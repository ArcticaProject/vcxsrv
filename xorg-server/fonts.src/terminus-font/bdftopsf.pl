#!/usr/bin/perl

push @ARGV, "";

if ($ARGV[0] eq "--help")
{
print STDERR <<EOT;
usage:	bdftopsf [[+h] [+u|-u] [+1|-1]]|[-h [+r|-r]]
	[+o|-o NAME] [--] [INPUT [TABLE...]]

+h	PSF mode - write a PSF header. This is the default.
  +u	Write an unicode table. Default for fonts with more
	than 256 characters.
  -u	Don't write unicode table. Default for fonts with 256
	or less characters.
  +1	Use PSF1. Default for fonts with 256 or 512 characters,
	width 8 and height less than 256.
  -1	Use PSF2. Default for all other fonts.

-h	RAW mode - write no PSF header (and no unicode table,
	of course).
  +r	Reject 512 character fonts. This is the default. Most
	systems won't handle these properly without some sort
	of header.
  -r	Allow 512 character fonts.

+o	Output to INPUT.psf or INPUT.raw (replacing the .bdf in
	INPUT if any).
-o NAME	Output to NAME.

--	Terminate the option list.

INPUT	RAW and PSF1: a 256 or 512 character BDF file with
	width = 8 and height between 1 and 255; PSF2: a BDF
	file with width between 1 and 128, height between 1 and
	256 and number of characters between 1 and 65536. If no
	input is specified, the standard input is used.

TABLE	A duplicate unicodes table. Each line must either be
	blank or contain two hexadecimal unicodes, each
	consisting of maximum 4 digits. Used for PSF with
	unicode table only, otherwise treated as error.

If no output is specified, the standard output is used.

Any options not specified in the above order are treated as
non-option arguments.
EOT
	exit 0;
}

if ($ARGV[0] eq "--version")
{
print STDERR <<EOT;
bdftopsf.pl 0.2.1, Copyright (C) 2008 Dimitar Toshkov Zhekov

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

if ($ARGV[0] eq "-h")
{
	$header = 0;
	$suffix = ".raw";
	$unicode = 0;
	$version = 0;
	shift @ARGV;

	if ($ARGV[0] eq "-r")
	{
		$reject = 0;
		shift @ARGV;
	}
	else
	{
		$reject = 1;
		if ($ARGV[0] eq "+r") { shift @ARGV; }
	}
}
else
{
	$header = 1;
	$suffix = ".psf";
	$reject = 0;
	if ($ARGV[0] eq "+h") { shift @ARGV; }

	if ($ARGV[0] eq "+u")
	{
		$unicode = 1;
		shift @ARGV;
	}
	elsif ($ARGV[0] eq "-u")
	{
		$unicode = 0;
		shift @ARGV;
	}

	if ($ARGV[0] eq "+1")
	{
		$version = 0;
		shift @ARGV;
	}
	elsif ($ARGV[0] eq "-1")
	{
		$version = 1;
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

if ($ARGV[0] eq "--") { shift @ARGV; }
elsif ($ARGV[0] =~ /^([-+][0-9a-z])$/) { print STDERR "$0: suspicuous $1, use -- to terminate the option list\n"; }
pop @ARGV;

if ($#ARGV < 0) { $ARGV[0] = "-"; }

open(BDF, "<$ARGV[0]") || die("$0: $ARGV[0]: $!\n");

while (<BDF>)
{
	if (/^FONTBOUNDINGBOX\s+([0-9]+)\s+([0-9]+).*$/)
	{
		$width = $1;
		$height = $2;
	}
	elsif (/^CHARS\s+([0-9]+)$/)
	{
		$chars = $1;
		last;
	}
}

(defined($width) && defined($chars)) || die("$0: $ARGV[0]: missing width/height or CHARS\n");
# psf2 can handle almost anything, but there must be some reasonable limits
($width > 0 && $width <= 128) || die("$0: $ARGV[0]: width $width out of range\n");
($height > 0 && $height <= 256) || die("$0: $ARGV[0]: height $height out of range\n");
($chars > 0 && $chars <= 65536) || die("$0: $ARGV[0]: CHARS $chars out of range\n");

$minimal = $width != 8 || $height >= 256 || ($chars != 256 && $chars != 512);
$header != 0 || $minimal == 0 || die("$0: $ARGV[0]: RAW format: invalid width $width, height $height or CHARS $chars\n");
$chars == 256 || $reject == 0 || die("$0: $ARGV[0]: RAW format: CHARS $chars rejected, use -r to accept\n", $chars);

if (!defined($unicode)) { $unicode = $chars > 256; }
if (!defined($version)) { $version = $minimal; }
else { $version >= $minimal || die("$0: $ARGV[0]: requested version ", $version + 1,  ", required ", $minimal + 1, "\n"); }

if (!defined($output)) { $output = "-"; }
elsif ($output eq "") { if ($ARGV[0] =~ /^(.*).bdf$/) { $output = "$1$suffix" ; } else { $output = "$ARGV[0]$suffix"; } }

$linesize = ($width + 7) >> 3;
$charsize = $linesize * $height;

sub int { $int = $!; }
sub bye
{
	close OUT;
	$output ne "-" && unlink($output) != 1 && print STDERR "$0: $output: $!\n";
	die("@_");
}

open(OUT, ">$output") || die("$0: $output: $!\n");
$SIG{INT} = 'int';
binmode(OUT) || bye("$0: $output: $!\n");

if ($header != 0)
{
	if ($version == 0) { printf OUT "%c%c%c%c", 0x36, 0x04, ($chars == 512) + $unicode * 2, $height; }
	else
	{
		printf OUT "%c%c%c%c", 0x72, 0xB5, 0x4A, 0x86;
		printf OUT "%c%c%c%c", 0x00, 0x00, 0x00, 0x00;
		printf OUT "%c%c%c%c", 0x20, 0x00, 0x00, 0x00;
		printf OUT "%c%c%c%c", $unicode, 0x00, 0x00, 0x00;
		printf OUT "%c%c%c%c", $chars & 0xFF, $chars >> 8, 0x00, 0x00;
		printf OUT "%c%c%c%c", $charsize & 0xFF, ($charsize >> 8) & 0xFF, $charsize >> 16, 0x00;
		printf OUT "%c%c%c%c", $height & 0xFF, $height >> 8, 0x00, 0x00;
		printf OUT "%c%c%c%c", $width & 0xFF, $width >> 8, 0x00, 0x00;
	}
}

$lines = 0;
while (<BDF>)
{
	$bytes = 0;
	while (/^([0-9a-fA-F]{2})(([0-9a-fA-F]{2})*)$/)
	{
		printf OUT "%c", hex($1);
		$bytes++;
		$_ = $2;
	}
	if ($bytes != 0)
	{
		$lines++;
		$bytes == $linesize || bye("$0: $ARGV[0]: invalid number of bytes $bytes on data line $lines\n");
	}
	else
	{
		if (/^ENCODING\s+([0-9]+)$/)
		{
			push @unimap, $1;
			if ($1 != 65535) { $unidup{$1}[0] = $1; }
		}
		elsif (/^ENDFONT$/) { last; }
	}
}

close BDF;

if ($#ARGV > 0)
{
	if ($unicode)
	{
		do
		{
			shift @ARGV;

			open(DUP, "<$ARGV[0]") || bye("$0: $ARGV[0]: $!\n");
			while (<DUP>)
			{
				next if /^\s*$/;
				/^([0-9a-fA-F]{1,4})\s+([0-9a-fA-F]{1,4})$/ || bye("$0: $ARGV[0]: invalid unicode(s) $_\n");
				$duplicate = hex($2);
				hex($1) != $duplicate || bye("$0: $ARGV[0]: invalid duplicate entry $_\n");
				foreach (@{$unidup{hex($1)}}) { if ($_ == $duplicate) { $duplicate = 65535; last; } }
				if ($duplicate != 65535) { push @{$unidup{hex($1)}}, $duplicate; }
			}
			close DUP;
		} while ($#ARGV > 0);
	}
	else { bye("$0: invalid number of arguments\n"); }
}

@unimap == $chars || bye("$0: $output: invalid number of chars @unimap\n");
@unimap * $charsize == $lines * $linesize || bye("$0: $output: invalid number of data lines $lines\n");

if ($unicode)
{
	foreach (@unimap)
	{
		foreach (@{$unidup{$_}}) {
			if ($version == 0) { printf OUT "%c%c", $_ & 0xFF, $_ >> 8; }
			elsif ($_ <= 0x7F) { printf OUT "%c", $_; }
			else
			{
				if ($_ <= 0x7FF) { printf OUT "%c", 0xC0 + ($_ >> 6); }
				else { printf OUT "%c%c", 0xE0 + ($_ >> 12), 0x80 + (($_ >> 6) & 0x3F); }
				printf OUT "%c", 0x80 + ($_ & 0x3F);
			}
		}
		printf OUT "%c", 0xFF;
		if ($version == 0) { printf OUT "%c", 0xFF; }
	}
}

close OUT || bye("$0: $output: $!\n");
defined($int) && bye("$0: $int\n");
