.\" $Xorg: macros.t,v 1.3 2000/08/17 19:42:51 cpqbld Exp $
.\" macros.t -- macros for X Consortium documents
.\" Revised and commented by smarks 93.12.20.
.\"
.\" global setup: set ragged right, assign string variables
.\"
.na
.ie n \{\
.ds Q \&"
.ds U \&"
.ds - \%--
.\}
.el \{\
.ds Q `\h'-\w'\^'u'`
.ds U '\h'-\w'\^'u''
.ds - \(em
.\}
.\"
.\" --- Ds --- displayed text (like .DS) with no keep
.\" .Ds is obsolete.  Change to something from this table:
.\" for this	use instead
.\" .Ds		.ID
.\" .Ds n	.LD		(where "n" is a number)
.\" (Numbers don't work in these macros, so ".Ds 5"
.\" comes out the same as ".Ds 0".)
.\"
.de Ds
.nf
.\\$1D \\$2 \\$1
.ft 1
.ps \\n(PS
.if \\n(VS>=40 .vs \\n(VSu
.if \\n(VS<=39 .vs \\n(VSp
..
.de D
.ID \\$1
..
.de 0D
.LD
..
.\" backward compatibility for the Xt spec
.de 5D
.LD
..
.\"
.\" --- De --- obsolete: use .DE instead
.\"
.de De
.DE
..
.\"
.\" --- FD ---
.\"
.de FD
.LP
.KS
.TA .5i 3i
.ta .5i 3i
.nf
..
.\"
.\" --- FN ---
.\"
.de FN
.fi
.KE
.LP
..
.\"
.\" --- IN --- send an index entry to the stderr
.\"
.de IN
.tm \\n%:\\$1:\\$2:\\$3
..
.\"
.\" --- C{ ---
.\"
.de C{
.KS
.nf
.D
.\"
.\"	choose appropriate monospace font
.\"	the imagen conditional, 480,
.\"	may be changed to L if LB is too
.\"	heavy for your eyes...
.\"
.ie "\\*(.T"480" .ft L
.el .ie "\\*(.T"300" .ft L
.el .ie "\\*(.T"202" .ft PO
.el .ie "\\*(.T"aps" .ft CW
.el .ft R
.ps \\n(PS
.ie \\n(VS>40 .vs \\n(VSu
.el .vs \\n(VSp
..
.\"
.\" --- C} ---
.\"
.de C}
.DE
.R
..
.\"
.\" --- Pn --- like PN, but use $2; $1 and $3 abut
.\"
.de Pn
.IN \\$2
.ie t \\$1\fB\^\\$2\^\fR\\$3
.el \\$1\fI\^\\$2\^\fP\\$3
..
.\"
.\" --- PN --- put $1 in boldface and add index entry; $2 abuts
.\"
.de PN
.IN \\$1
.ie t \fB\^\\$1\^\fR\\$2
.el \fI\^\\$1\^\fP\\$2
..
.\"
.\" --- hI --- add index entry for $1 as header file
.\"
.de hI
.IN         <\\$1>
.IN Files   <\\$1>
.IN Headers <\\$1>
..
.\"
.\" --- hN --- put $1 in boldface as header and add index entry; $2 abuts
.\"
.de hN
.hI \\$1
.ie t <\fB\\$1\fR>\\$2
.el <\fI\\$1\fP>\\$2
..
.\"
.\" --- NT ---
.\"
.de NT
.br
.ne 7
.ds NO Note
.if \\n(.$ .ds NO \\$1
.ie n .sp
.el .sp 10p
.ce
\\*(NO
.ie n .sp
.el .sp 5p
.if '\\$1'C' .ce 99
.if '\\$2'C' .ce 99
.\" .QS/.QE macros don't exist in older versions of -ms
.ie \\n(GS .QS
.el \{\
.	in +5n
.	ll -5n
.\}
.R
..
.\"
.\" --- NE --- Note End (doug kraft 3/85)
.\"
.de NE
.ce 0
.ie \\n(GS .QE
.el \{\
.	in -5n
.	ll +5n
.\}
.ie n .sp
.el .sp 10p
..
.\"
.\" --- nH --- numbered header (like NH) but with automatic TOC entry
.\" usage: .nH level "section title, preferable in quotes"
.\"
.de nH
.NH \\$1
\\$2
.XS
\\*(SN \\$2
.XE
..
.\"
.\" --- sM --- put start-marker in margin
.\"
.de sM
.KS
.sp 1
\\h'-0.5i'\\L'-1v'\\v'1p'\\l'1v'\\v'1v-1p'
.sp -1
..
.\"
.\" --- eM --- put end-marker in margin
.\"
.de eM
.sp -1
\\h'-0.5i'\\L'-1v'\\v'1v+1p'\\l'1v'\\v'-1p'
.sp 1
.KE
..
.\"
.\" --- YZ --- finish up; $1 is the starting page number of the TOC
.\"
.de YZ
.		\" Force there to be an even number of pages, so the table of
.		\" contents doesn't end up on the back of the last page in
.		\" the case of duplex printing.
.if o .bp
.		\" Emit a .pn directive with one plus the last page number.
		\" This will be the number of the first page of the index.
.nr YZ \\n%+1
.tm .pn \\n(YZ
.		\" Issue the table of contents, setting roman numerals,
.		\" and redefining the footer to use them.
.bp \\$1
.af PN i
.EF ''\\\\\\\\n(PN''
.OF ''\\\\\\\\n(PN''
.		\" Why all the backslashes?  This string is evaluated
.		\" three times: 1) during the definition of this macro,
.		\" 2) when the .EF and .OF macros are expanded, and 3)
.		\" when the bottom-of-page trap is invoked.  Thus,
.		\" eight backslashes are reduced to one in the final output.
.PX
..
