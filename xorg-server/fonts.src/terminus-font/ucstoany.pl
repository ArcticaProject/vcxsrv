#!/usr/bin/perl

push @ARGV, "";

if ($ARGV[0] eq "--help")
{
print STDERR <<EOT;
usage:	ucstoany.pl [-f [+u|-u]]|[+f [+g|-g]] [+o|-o NAME] [+b]
	[+[CHAR]] [--] INPUT REGISTRY ENCODING [TABLE...]

-f	Filter mode - discard characters with unicode FFFF.
	This is the default.
  +u	Encode characters with index >= 32 with their unicodes.
	Default for ISO10646-1 output.
  -u	Encode all characters with their indexes (FFFF counts).
	Default for any other output.

+f	Fillout mode - encode with unicodes, emit characters
	with unicode FFFF as the default character.
  +g	Exchange the characters in range 00...1F with these at
	C0...DF. Default for 8-pixel wide fonts with 256...512
	characters starting with 00A3.
  -g	Do not exchange. Default for all other fonts.

+o	Output to INPUT-REGISTRY-ENCODING (using the same name
	as ucs2any, but preserving the INPUT directory).
-o NAME	Output to NAME.

+b	Use binary mode for output (the default is text mode).
	No effect on POSIX systems.

+[CHAR]	Set the default character to CHAR (decimal value). If
	no CHAR is specified, the one from INPUT is used.

--	Terminate the option list.

INPUT	Any BDF file.

TABLE	An unicode table. Each line must either be blank or
	contain exactly one hexadecimal unicode consisting of
	maximum 4 digits. If no TABLE(s) are specified, the
	standard input is read.

If no output is specified, the standard output is used.

If no default character is specified, 65533 (FFFD) is used for
unicode and fillout modes, 46 (period) for index mode.

Any options not specified in the above order are treated as
non-option arguments.
EOT
	exit 0;
}

if ($ARGV[0] eq "--version")
{
print STDERR <<EOT;
ucstoany.pl 0.1.1, Copyright (C) 2008 Dimitar Toshkov Zhekov

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Report bugs to jimmy\@is-vn.bg
EOT
	exit 0;
}

if ($ARGV[0] eq "+f")
{
	$control = 0;
	shift @ARGV;

	if ($ARGV[0] eq "+g")
	{
		$exchange = 1;
		shift @ARGV;
	}
	elsif ($ARGV[0] eq "-g")
	{
		$exchange = 0;
		shift @ARGV;
	}
}
else
{
	$exchange = 0;
	if($ARGV[0] eq "-f") { shift @ARGV; }

	if ($ARGV[0] eq "+u")
	{
		$control = 32;
		shift @ARGV;
	}
	elsif ($ARGV[0] eq "-u")
	{
		$control = 65536;
		shift @ARGV;
	}
}

if ($ARGV[0] eq "+o")
{
	$output = "";
	shift @ARGV;
}
elsif ($ARGV[0] eq "-o")
{
	shift @ARGV;
	$#ARGV > 0 || die("$0: -o requires an argument\n");
	$output = $ARGV[0];
	shift @ARGV;
}
elsif ($ARGV[0] =~ /^-o(.*)$/)
{
	$output = $1;
	shift @ARGV;
}

if ($ARGV[0] eq "+b")
{
	$binary = 1;
	shift @ARGV;
}
else
{
	$binary = 0;
	if ($ARGV[0] eq "-b") { shift @ARGV; }
}

if ($ARGV[0] =~ /\+\s*([0-9]*)\s*$/)
{
	$default = $1;
	shift @ARGV;
}

if ($ARGV[0] eq '--') { shift @ARGV; }
elsif ($ARGV[0] =~ /^([-+][0-9a-z])$/) { print STDERR "$0: suspicuous $1, use -- to terminate the option list\n"; }
pop @ARGV;

if ($#ARGV == 2) { $ARGV[3] = "-"; }
elsif ($#ARGV < 2) { die("$0: invalid arguments, try --help\n"); }

open(BDF, "<$ARGV[0]") || die("$0: $ARGV[0]: $!\n");

while (<BDF>)
{
	$header .= $_;
	last if /^CHARS\s/;
}

while (<BDF>)
{
	if (/^STARTCHAR\s+(.+)$/) { $startchar = $1; }
	elsif (/^ENCODING\s+(.+)$/) { $encoding = $1; }
	$buffer .= $_;
	if (/^ENDCHAR$/)
	{
		$startchar ne "" || die("$0: $ARGV[0]: ENDCHAR without STARTCHAR\n");
		$encoding ne "" || die("$0: $ARGV[0]: no ENCODING for $startchar\n");
		$bitmap{$encoding} = $buffer;
		$buffer = $startchar = $encoding = "";
	}
	elsif (!defined($exchange) && /^BBX\s+([0-9]+)/ && $1 != 8) { $exchange = 0; }
}

close BDF;

$charset = "-$ARGV[1]-$ARGV[2]";
if (!defined($control)) { $control = ($charset =~ /^-iso10646-1$/i) ? 32 : 65536; }

$chars = 0;
for ($index = 3; $index <= $#ARGV; $index++)
{
	open(UNI, "<$ARGV[$index]") || die("$0: $ARGV[$index]: $!\n");
	while (<UNI>)
	{
		next if /^\s*$/;
		/^([0-9a-fA-F]{1,4})$/ || die("$0: $ARGV[$index]: invalid unicode $_\n");
		push @unimap, hex($1);
		if (!$control || hex($1) != 65535) { $chars++; }
	}
	close UNI;
}
$chars > 0 || die("$0: empty unicode table(s)\n");

if (!defined($exchange)) { $exchange = $chars >= 256 && $chars <= 512 && $unimap[0] != 65533; }
if ($exchange)
{
	@unimap >= 0xE0 || die("$0: not enough characters for exchange\n");
	for ($index = 0x00; $index < 0x20; $index++)
	{
		$_ = $unimap[$index];
		$unimap[$index] = $unimap[$index + 0xC0];
		$unimap[$index + 0xC0] = $_;
	}
}

if (!defined($default)) { $default = $control == 65536 ? 46 : 65533; }
elsif ($default eq "")
{
	$header =~ /^DEFAULT_CHAR\s(.+)$/m || die("$0: $ARGV[0]: unable to obtain DEFAULT_CHAR\n");
	$default = $1;
}

if (!defined($output)) { $output = "-"; }
elsif ($output eq "") { if ($ARGV[0] =~ /^(.*).bdf$/) { $output = "$1$charset.bdf" ; } else { $output = "$ARGV[0]$charset"; } }

$header =~ s/^(FONT\s.*)-.*-.*$/$1$charset/m || die("$0: $output: unable to change FONT registry-encoding\n");
$header =~ s/^(CHARSET_REGISTRY\s).*$/$1"$ARGV[1]"/m || die("$0: $output: unable to change CHARSET_REGISTRY\n");
$header =~ s/^(CHARSET_ENCODING\s).*$/$1"$ARGV[2]"/m || die("$0: $output: unable to change CHARSET_ENCODING\n");
$header =~ s/^(DEFAULT_CHAR\s).*$/$1$default/m || die("$0: $output: unable to change DEFAULT_CHAR\n");
$header =~ s/^(CHARS\s).*$/$1$chars/m || die("$0: $output: unable to change CHARS\n");

sub int { $int = $!; }
sub bye
{
	close OUT;
	$output ne "-" && unlink($output) != 1 && print STDERR "$0: $output: $!\n";
	die("@_");
}

open(OUT, ">$output") || die("$0: $output: $!\n");
$SIG{INT} = 'int';
if ($binary) { binmode(OUT) || bye("$0: $output: $!\n"); }
print OUT $header;

for ($index = 0; $index < @unimap; $index++)
{
	$_ = $unimap[$index];
	$encoding = $index >= $control ? $_ : $index;
	if ($_ == 65535) { if ($control) { next; } else { $_ = $default ; } }
	$bitmap{$_} =~ s/^(ENCODING\s).*$/$1$encoding/m || bye("$0: $output: unable to change encoding for $_\n");
	print OUT $bitmap{$_};
}

print OUT $buffer;
close OUT || bye("$0: $output: $!\n");
defined($int) && bye("$0: $int\n");
