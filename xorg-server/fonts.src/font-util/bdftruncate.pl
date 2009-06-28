#!/usr/bin/perl
#
# bdftruncate.pl -- Markus Kuhn <http://www.cl.cam.ac.uk/~mgk25/>
#
# This Perl script allows you to generate from an ISO10646-1 encoded
# BDF font other ISO10646-1 BDF fonts in which all characters above
# a threshold code value are stored unencoded.
#
# $ucs-fonts: bdftruncate.pl,v 1.7 2004-11-28 18:41:13+00 mgk25 Rel $

# Subroutine to identify whether the ISO 10646/Unicode character code
# ucs belongs into the East Asian Wide (W) or East Asian FullWidth
# (F) category as defined in Unicode Technical Report #11.
sub iswide ($) {
    my $ucs = shift(@_);

    return ($ucs >= 0x1100 &&
            ($ucs <= 0x115f ||                   # Hangul Jamo
             ($ucs >= 0x2e80 && $ucs <= 0xa4cf &&
              ($ucs & ~0x0011) != 0x300a && $ucs != 0x303f) || # CJK .. Yi
             ($ucs >= 0xac00 && $ucs <= 0xd7a3) || # Hangul Syllables
             ($ucs >= 0xf900 && $ucs <= 0xfaff) || # CJK Comp. Ideographs
             ($ucs >= 0xfe30 && $ucs <= 0xfe6f) || # CJK Comp. Forms
             ($ucs >= 0xff00 && $ucs <= 0xff5f) || # Fullwidth Forms
             ($ucs >= 0xffe0 && $ucs <= 0xffe6) ||
             ($ucs >= 0x20000 && $ucs <= 0x2ffff)));
}

# parse options
if ($ARGV[0] eq '-w' || $ARGV[0] eq '+w') {
    $removewide = $ARGV[0] eq '-w';
    shift @ARGV;
}

print STDERR <<End if $#ARGV != 0;

Usage: bdftruncate.pl [+w|-w] threshold <source.bdf >destination.bdf

Example:

   bdftruncate.pl 0x3200 <6x13.bdf >6x13t.bdf

will generate the file 6x13t.bdf in which all glyphs with codes
>= 0x3200 will only be stored unencoded (i.e., ENCODING -1).
Option -w removes East Asian Wide and East Asian FullWidth characters
(default if threshold <= 0x3200), and option +w keeps them.

End

exit 1 if $#ARGV != 0;

# read threshold value from command line
$threshold = $ARGV[0];
if ($threshold =~ /^(0[xX]|U[+-]?)([0-9a-fA-F]+)$/) {
    $threshold = hex($2);
} elsif (!($threshold =~ /^[0-9]+$/)) {
    die("Illegal threshold '$threshold'!\n");
}
$removewide = $threshold <= 0x3200 unless defined $removewide;

# filter file
while (<STDIN>) {
    if (/^ENCODING\s+(-?\d+)/ && 
	($1 >= $threshold || ($removewide && iswide($1)))) {
	print "ENCODING -1\n";
    } elsif (/^STARTFONT/) {
	print;
	print "COMMENT AUTOMATICALLY GENERATED FILE. DO NOT EDIT!\n";
	printf("COMMENT In this version of the font file, all characters >= " .
	       "U+%04X are\nCOMMENT not encoded to keep XFontStruct small.\n",
	       $threshold);
    } else {
	s/^COMMENT\s+\"(.*)\"$/COMMENT $1/;
        s/^COMMENT\s+\$[I]d: (.*)\$\s*$/COMMENT Derived from $1\n/;
	print;
    }
}
