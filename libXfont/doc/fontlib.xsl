<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">
  <!--
       Copyright 2009 Sun Microsystems, Inc.  All rights reserved.

       Permission is hereby granted, free of charge, to any person obtaining a
       copy of this software and associated documentation files (the
       "Software"), to deal in the Software without restriction, including
       without limitation the rights to use, copy, modify, merge, publish,
       distribute, and/or sell copies of the Software, and to permit persons
       to whom the Software is furnished to do so, provided that the above
       copyright notice(s) and this permission notice appear in all copies of
       the Software and that both the above copyright notice(s) and this
       permission notice appear in supporting documentation.

       THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
       OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
       MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
       OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
       HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL
       INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
       FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
       NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
       WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

       Except as contained in this notice, the name of a copyright holder
       shall not be used in advertising or otherwise to promote the sale, use
       or other dealings in this Software without prior written authorization
       of the copyright holder.
   -->

  <xsl:param name="html.stylesheet" select="'fontlib.css'"/>
  <xsl:param name="chunker.output.indent">yes</xsl:param>
  <xsl:param name="html.extra.head.links" select="1"></xsl:param>
  <xsl:param name="saxon.character.representation" select="'entity;decimal'"></xsl:param>
  <xsl:param name="function.parens" select="0"></xsl:param>
</xsl:stylesheet>
