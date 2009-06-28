/*
 * Copyright © 2005 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#include "xglglx.h"
#include "xglglxext.h"
#include "glapitable.h"

#ifndef NGLXEXTLOG

static struct _glapi_table *nativeRenderTable = 0;

static FILE *logFp = 0;

static Bool logVertexAttribs = FALSE;

static struct VertexAttribCount {
    int  n;
    char *name;
} vCnt[] = {
    { 0, "glArrayElement"	},
    { 0, "glCallList"		},
    { 0, "glCallLists"		},
    { 0, "glColor3bv"		},
    { 0, "glColor3dv"		},
    { 0, "glColor3fv"		},
    { 0, "glColor3iv"		},
    { 0, "glColor3sv"		},
    { 0, "glColor3ubv"		},
    { 0, "glColor3uiv"		},
    { 0, "glColor3usv"		},
    { 0, "glColor4bv"		},
    { 0, "glColor4dv"		},
    { 0, "glColor4fv"		},
    { 0, "glColor4iv"		},
    { 0, "glColor4sv"		},
    { 0, "glColor4ubv"		},
    { 0, "glColor4uiv"		},
    { 0, "glColor4usv"		},
    { 0, "glEdgeFlagv"		},
    { 0, "glEvalCoord1dv"	},
    { 0, "glEvalCoord1fv"	},
    { 0, "glEvalCoord2dv"	},
    { 0, "glEvalCoord2fv"	},
    { 0, "glEvalPoint1"		},
    { 0, "glEvalPoint2"		},
    { 0, "glIndexdv"		},
    { 0, "glIndexfv"		},
    { 0, "glIndexiv"		},
    { 0, "glIndexsv"		},
    { 0, "glIndexubv"		},
    { 0, "glMaterialf"		},
    { 0, "glMaterialfv"		},
    { 0, "glMateriali"		},
    { 0, "glMaterialiv"		},
    { 0, "glNormal3bv"		},
    { 0, "glNormal3dv"		},
    { 0, "glNormal3fv"		},
    { 0, "glNormal3iv"		},
    { 0, "glNormal3sv"		},
    { 0, "glTexCoord1dv"	},
    { 0, "glTexCoord1fv"	},
    { 0, "glTexCoord1iv"	},
    { 0, "glTexCoord1sv"	},
    { 0, "glTexCoord2dv"	},
    { 0, "glTexCoord2fv"	},
    { 0, "glTexCoord2iv"	},
    { 0, "glTexCoord2sv"	},
    { 0, "glTexCoord3dv"	},
    { 0, "glTexCoord3fv"	},
    { 0, "glTexCoord3iv"	},
    { 0, "glTexCoord3sv"	},
    { 0, "glTexCoord4dv"	},
    { 0, "glTexCoord4fv"	},
    { 0, "glTexCoord4iv"	},
    { 0, "glTexCoord4sv"	},
    { 0, "glVertex2dv"		},
    { 0, "glVertex2fv"		},
    { 0, "glVertex2iv"		},
    { 0, "glVertex2sv"		},
    { 0, "glVertex3dv"		},
    { 0, "glVertex3fv"		},
    { 0, "glVertex3iv"		},
    { 0, "glVertex3sv"		},
    { 0, "glVertex4dv"		},
    { 0, "glVertex4fv"		},
    { 0, "glVertex4iv"		},
    { 0, "glVertex4sv"		},
    { 0, "glMultiTexCoord1dv"	},
    { 0, "glMultiTexCoord1fv"	},
    { 0, "glMultiTexCoord1iv"	},
    { 0, "glMultiTexCoord1sv"	},
    { 0, "glMultiTexCoord2dv"	},
    { 0, "glMultiTexCoord2fv"	},
    { 0, "glMultiTexCoord2iv"	},
    { 0, "glMultiTexCoord2sv"	},
    { 0, "glMultiTexCoord3dv"	},
    { 0, "glMultiTexCoord3fv"	},
    { 0, "glMultiTexCoord3iv"	},
    { 0, "glMultiTexCoord3sv"	},
    { 0, "glMultiTexCoord4dv"	},
    { 0, "glMultiTexCoord4fv"	},
    { 0, "glMultiTexCoord4iv"	},
    { 0, "glMultiTexCoord4sv"	},
    { 0, "glFogCoordfv"		},
    { 0, "glFogCoorddv"		},
    { 0, "glSecondaryColor3bv"	},
    { 0, "glSecondaryColor3dv"	},
    { 0, "glSecondaryColor3fv"	},
    { 0, "glSecondaryColor3iv"	},
    { 0, "glSecondaryColor3sv"	},
    { 0, "glSecondaryColor3ubv"	},
    { 0, "glSecondaryColor3uiv"	},
    { 0, "glSecondaryColor3usv"	}
};

#define arrayElementIndex	0
#define callListIndex		1
#define callListsIndex		2
#define color3bvIndex		3
#define color3dvIndex		4
#define color3fvIndex		5
#define color3ivIndex		6
#define color3svIndex		7
#define color3ubvIndex		8
#define color3uivIndex		9
#define color3usvIndex		10
#define color4bvIndex		11
#define color4dvIndex		12
#define color4fvIndex		13
#define color4ivIndex		14
#define color4svIndex		15
#define color4ubvIndex		16
#define color4uivIndex		17
#define color4usvIndex		18
#define edgeFlagvIndex		19
#define evalCoord1dvIndex	20
#define evalCoord1fvIndex	21
#define evalCoord2dvIndex	22
#define evalCoord2fvIndex	23
#define evalPoint1Index		24
#define evalPoint2Index		25
#define indexdvIndex		26
#define indexfvIndex		27
#define indexivIndex		28
#define indexsvIndex		29
#define indexubvIndex		30
#define materialfIndex		31
#define materialfvIndex		32
#define materialiIndex		33
#define materialivIndex		34
#define normal3bvIndex		35
#define normal3dvIndex		36
#define normal3fvIndex		37
#define normal3ivIndex		38
#define normal3svIndex		39
#define texCoord1dvIndex	40
#define texCoord1fvIndex	41
#define texCoord1ivIndex	42
#define texCoord1svIndex	43
#define texCoord2dvIndex	44
#define texCoord2fvIndex	45
#define texCoord2ivIndex	46
#define texCoord2svIndex	47
#define texCoord3dvIndex	48
#define texCoord3fvIndex	49
#define texCoord3ivIndex	50
#define texCoord3svIndex	51
#define texCoord4dvIndex	52
#define texCoord4fvIndex	53
#define texCoord4ivIndex	54
#define texCoord4svIndex	55
#define vertex2dvIndex		56
#define vertex2fvIndex		57
#define vertex2ivIndex		58
#define vertex2svIndex		59
#define vertex3dvIndex		60
#define vertex3fvIndex		61
#define vertex3ivIndex		62
#define vertex3svIndex		63
#define vertex4dvIndex		64
#define vertex4fvIndex		65
#define vertex4ivIndex		66
#define vertex4svIndex		67
#define multiTexCoord1dvIndex	68
#define multiTexCoord1fvIndex	69
#define multiTexCoord1ivIndex	70
#define multiTexCoord1svIndex	71
#define multiTexCoord2dvIndex	72
#define multiTexCoord2fvIndex	73
#define multiTexCoord2ivIndex	74
#define multiTexCoord2svIndex	75
#define multiTexCoord3dvIndex	76
#define multiTexCoord3fvIndex	77
#define multiTexCoord3ivIndex	78
#define multiTexCoord3svIndex	79
#define multiTexCoord4dvIndex	80
#define multiTexCoord4fvIndex	81
#define multiTexCoord4ivIndex	82
#define multiTexCoord4svIndex	83
#define fogCoordfvIndex		84
#define fogCoorddvIndex		85
#define secondaryColor3bvIndex	86
#define secondaryColor3dvIndex	87
#define secondaryColor3fvIndex	88
#define secondaryColor3ivIndex	89
#define secondaryColor3svIndex	90
#define secondaryColor3ubvIndex 91
#define secondaryColor3uivIndex 92
#define secondaryColor3usvIndex 93

static void
logAccum (GLenum  op,
	  GLfloat value)
{
    fprintf (logFp, "glAccum (0x%x, %f)\n", op, value);
    (*nativeRenderTable->Accum) (op, value);
}

static void
logAlphaFunc (GLenum   func,
	      GLclampf ref)
{
    fprintf (logFp, "glAlphaFunc (0x%x, %f)\n", func, ref);
    (*nativeRenderTable->AlphaFunc) (func, ref);
}

static GLboolean
logAreTexturesResident (GLsizei	     n,
			const GLuint *textures,
			GLboolean    *residences)
{
    fprintf (logFp, "glAreTexturesResident (%d, %p, %p)\n", n, textures,
	     residences);
    return (*nativeRenderTable->AreTexturesResident) (n, textures,
						      residences);
}

static void
logArrayElement (GLint i)
{
    vCnt[arrayElementIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glArrayElement (%d)\n", i);
    (*nativeRenderTable->ArrayElement) (i);
}

static void
logBegin (GLenum mode)
{
    fprintf (logFp, "glBegin (0x%x)\n", mode);
    (*nativeRenderTable->Begin) (mode);
}

static void
logBindTexture (GLenum target,
		GLuint texture)
{
    fprintf (logFp, "glBindTexture (0x%x, %u)\n", target, texture);
    (*nativeRenderTable->BindTexture) (target, texture);
}

static void
logBitmap (GLsizei	 width,
	   GLsizei	 height,
	   GLfloat	 xorig,
	   GLfloat	 yorig,
	   GLfloat	 xmove,
	   GLfloat	 ymove,
	   const GLubyte *bitmap)
{
    fprintf (logFp, "glBitmap (%d, %d, %f, %f, %f, %f, %p)\n",
	     width, height, xorig, yorig, xmove, ymove, bitmap);
    (*nativeRenderTable->Bitmap) (width, height, xorig, yorig,
				  xmove, ymove, bitmap);
}

static void
logBlendFunc (GLenum sfactor,
	      GLenum dfactor)
{
    fprintf (logFp, "glBlendFunc (0x%x, 0x%x)\n", sfactor, dfactor);
    (*nativeRenderTable->BlendFunc) (sfactor, dfactor);
}

static void
logCallList (GLuint list)
{
    vCnt[callListIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glCallList (%u)\n", list);
    (*nativeRenderTable->CallList) (list);
}

static void
logCallLists (GLsizei	 n,
	      GLenum	 type,
	      const void *lists)
{
    vCnt[callListsIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glCallLists (%d, 0x%x, %p)\n", n, type, lists);
    (*nativeRenderTable->CallLists) (n, type, lists);
}

static void
logClear (GLbitfield mask)
{
    fprintf (logFp, "glClear (0x%x)\n", mask);
    (*nativeRenderTable->Clear) (mask);
}

static void
logClearAccum (GLfloat red,
	       GLfloat green,
	       GLfloat blue,
	       GLfloat alpha)
{
    fprintf (logFp, "glClearAccum (%f, %f, %f, %f)\n",
	     red, green, blue, alpha);
    (*nativeRenderTable->ClearAccum) (red, green, blue, alpha);
}

static void
logClearColor (GLclampf red,
	       GLclampf green,
	       GLclampf blue,
	       GLclampf alpha)
{
    fprintf (logFp, "glClearColor (%f, %f, %f, %f)\n",
	     red, green, blue, alpha);
    (*nativeRenderTable->ClearColor) (red, green, blue, alpha);
}

static void
logClearDepth (GLclampd depth)
{
    fprintf (logFp, "glClearDepth (%f)\n", depth);
    (*nativeRenderTable->ClearDepth) (depth);
}

static void
logClearIndex (GLfloat c)
{
    fprintf (logFp, "glClearIndex (%f)\n", c);
    (*nativeRenderTable->ClearIndex) (c);
}

static void
logClearStencil (GLint s)
{
    fprintf (logFp, "glClearStencil (%d)\n", s);
    (*nativeRenderTable->ClearStencil) (s);
}

static void
logClipPlane (GLenum	     plane,
	      const GLdouble *equation)
{
    fprintf (logFp, "glClipPlane (0x%x, %p)\n", plane, equation);
    (*nativeRenderTable->ClipPlane) (plane, equation);
}

static void
logColor3bv (const GLbyte *v)
{
    vCnt[color3bvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glColor3bv (%p)\n", v);
    (*nativeRenderTable->Color3bv) (v);
}

static void
logColor3dv (const GLdouble *v)
{
    vCnt[color3dvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glColor3dv (%p)\n", v);
    (*nativeRenderTable->Color3dv) (v);
}

static void
logColor3fv (const GLfloat *v)
{
    vCnt[color3fvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glColor3fv (%p)\n", v);
    (*nativeRenderTable->Color3fv) (v);
}

static void
logColor3iv (const GLint *v)
{
    vCnt[color3ivIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glColor3iv (%p)\n", v);
    (*nativeRenderTable->Color3iv) (v);
}

static void
logColor3sv (const GLshort *v)
{
    vCnt[color3svIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glColor3sv (%p)\n", v);
    (*nativeRenderTable->Color3sv) (v);
}

static void
logColor3ubv (const GLubyte *v)
{
    vCnt[color3ubvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glColor3ubv (%p)\n", v);
    (*nativeRenderTable->Color3ubv) (v);
}

static void
logColor3uiv (const GLuint *v)
{
    vCnt[color3uivIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glColor3uiv (%p)\n", v);
    (*nativeRenderTable->Color3uiv) (v);
}

static void
logColor3usv (const GLushort *v)
{
    vCnt[color3usvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glColor3usv (%p)\n", v);
    (*nativeRenderTable->Color3usv) (v);
}

static void
logColor4bv (const GLbyte *v)
{
    vCnt[color4bvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glColor4bv (%p)\n", v);
    (*nativeRenderTable->Color4bv) (v);
}

static void
logColor4dv (const GLdouble *v)
{
    vCnt[color4dvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glColor4dv (%p)\n", v);
    (*nativeRenderTable->Color4dv) (v);
}

static void
logColor4fv (const GLfloat *v)
{
    vCnt[color4fvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glColor4fv (%p)\n", v);
    (*nativeRenderTable->Color4fv) (v);
}

static void
logColor4iv (const GLint *v)
{
    vCnt[color4ivIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glColor4iv (%p)\n", v);
    (*nativeRenderTable->Color4iv) (v);
}

static void
logColor4sv (const GLshort *v)
{
    vCnt[color4svIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glColor4sv (%p)\n", v);
    (*nativeRenderTable->Color4sv) (v);
}

static void
logColor4ubv (const GLubyte *v)
{
    vCnt[color4ubvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glColor4ubv (%p)\n", v);
    (*nativeRenderTable->Color4ubv) (v);
}

static void
logColor4uiv(const GLuint *v)
{
    vCnt[color4uivIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glColor4uiv (%p)\n", v);
    (*nativeRenderTable->Color4uiv) (v);
}

static void
logColor4usv (const GLushort *v)
{
    vCnt[color4usvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glColor4usv (%p)\n", v);
    (*nativeRenderTable->Color4usv) (v);
}

static void
logColorMask (GLboolean red,
	      GLboolean green,
	      GLboolean blue,
	      GLboolean alpha)
{
    fprintf (logFp, "glColorMask (%d, %d, %d, %d)\n", red, green, blue, alpha);
    (*nativeRenderTable->ColorMask) (red, green, blue, alpha);
}

static void
logColorMaterial (GLenum face,
		  GLenum mode)
{
    fprintf (logFp, "glColorMaterial (0x%x, 0x%x)\n", face, mode);
    (*nativeRenderTable->ColorMaterial) (face, mode);
}

static void
logColorPointer (GLint	    size,
		 GLenum	    type,
		 GLsizei    stride,
		 const void *pointer)
{
    fprintf (logFp, "glColorPointer (%d, 0x%x, %d, %p)\n",
	     size, type, stride, pointer);
    (*nativeRenderTable->ColorPointer) (size, type, stride, pointer);
}

static void
logCopyPixels (GLint   x,
	       GLint   y,
	       GLsizei width,
	       GLsizei height,
	       GLenum  type)
{
    fprintf (logFp, "glCopyPixels (%d, %d, %d, %d, 0x%x)\n",
	     x, y, width, height, type);
    (*nativeRenderTable->CopyPixels) (x, y, width, height, type);
}

static void
logCopyTexImage1D (GLenum  target,
		   GLint   level,
		   GLenum  internalFormat,
		   GLint   x,
		   GLint   y,
		   GLsizei width,
		   GLint   border)
{
    fprintf (logFp, "glCopyTexImage1D (0x%x, %d, 0x%x, %d, %d, %d, %d)\n",
	     target, level, internalFormat, x, y, width, border);
    (*nativeRenderTable->CopyTexImage1D) (target, level, internalFormat,
					  x, y, width, border);
}

static void
logCopyTexImage2D (GLenum  target,
		   GLint   level,
		   GLenum  internalFormat,
		   GLint   x,
		   GLint   y,
		   GLsizei width,
		   GLsizei height,
		   GLint   border)
{
    fprintf (logFp, "glCopyTexImage2D (0x%x, %d, 0x%x, %d, %d, %d, %d, %d)\n",
	     target, level, internalFormat, x, y, width, height, border);
    (*nativeRenderTable->CopyTexImage2D) (target, level, internalFormat,
					  x, y, width, height, border);
}

static void
logCopyTexSubImage1D (GLenum  target,
		      GLint   level,
		      GLint   xoffset,
		      GLint   x,
		      GLint   y,
		      GLsizei width)
{
    fprintf (logFp, "glCopyTexSubImage1D (0x%x, %d, %d, %d, %d, %d)\n",
	     target, level, xoffset, x, y, width);
    (*nativeRenderTable->CopyTexSubImage1D) (target, level, xoffset, x, y,
					     width);
}

static void
logCopyTexSubImage2D (GLenum  target,
		      GLint   level,
		      GLint   xoffset,
		      GLint   yoffset,
		      GLint   x,
		      GLint   y,
		      GLsizei width,
		      GLsizei height)
{
    fprintf (logFp, "glCopyTexSubImage2D (0x%x, %d, %d, %d, %d, %d, %d, %d)\n",
	     target, level, xoffset, yoffset, x, y, width, height);
    (*nativeRenderTable->CopyTexSubImage2D) (target, level,
					     xoffset, yoffset, x, y,
					     width, height);
}

static void
logCullFace (GLenum mode)
{
    fprintf (logFp, "glCullFace (0x%x)\n", mode);
    (*nativeRenderTable->CullFace) (mode);
}

static void
logDeleteLists (GLuint  list,
		GLsizei range)
{
    fprintf (logFp, "glDeleteLists (%d, %d)\n", list, range);
    (*nativeRenderTable->DeleteLists) (list, range);
}

static void
logDeleteTextures (GLsizei n, const GLuint *textures)
{
    fprintf (logFp, "glDeleteTextures (%d, %p)\n", n, textures);
    (*nativeRenderTable->DeleteTextures) (n, textures);
}

static void
logDepthFunc (GLenum func)
{
    fprintf (logFp, "glDepthFunc (0x%x)\n", func);
    (*nativeRenderTable->DepthFunc) (func);
}

static void
logDepthMask (GLboolean flag)
{
    fprintf (logFp, "glDepthMask (%d)\n", flag);
    (*nativeRenderTable->DepthMask) (flag);
}

static void
logDepthRange (GLclampd zNear,
	       GLclampd zFar)
{
    fprintf (logFp, "glDepthRange (%f, %f)\n", zNear, zFar);
    (*nativeRenderTable->DepthRange) (zNear, zFar);
}

static void
logDisable (GLenum cap)
{
    fprintf (logFp, "glDisable (0x%x)\n", cap);
    (*nativeRenderTable->Disable) (cap);
}

static void
logDisableClientState (GLenum array)
{
    fprintf (logFp, "glDisableClientState (0x%x)\n", array);
    (*nativeRenderTable->DisableClientState) (array);
}

static void
logDrawArrays (GLenum  mode,
	       GLint   first,
	       GLsizei count)
{
    fprintf (logFp, "glDrawArrays (0x%x, %d, %d)\n", mode, first, count);
    (*nativeRenderTable->DrawArrays) (mode, first, count);
}

static void
logDrawBuffer (GLenum mode)
{
    fprintf (logFp, "glDrawBuffer (0x%x)\n", mode);
    (*nativeRenderTable->DrawBuffer) (mode);
}

static void
logDrawElements (GLenum     mode,
		 GLsizei    count,
		 GLenum	    type,
		 const void *indices)
{
    fprintf (logFp, "glDrawElements (0x%x, %d, 0x%x, %p)\n",
	     mode, count, type, indices);
    (*nativeRenderTable->DrawElements) (mode, count, type, indices);
}

static void
logDrawPixels (GLsizei	  width,
	       GLsizei    height,
	       GLenum	  format,
	       GLenum	  type,
	       const void *pixels)
{
    fprintf (logFp, "glDrawPixels (%d, %d, 0x%x, 0x%x, %p)\n",
	     width, height, format, type, pixels);
    (*nativeRenderTable->DrawPixels) (width, height, format, type, pixels);
}

static void
logEdgeFlagPointer (GLsizei    stride,
		    const void *pointer)
{
    fprintf (logFp, "glEdgeFlagPointer (%d, %p)", stride, pointer);
    (*nativeRenderTable->EdgeFlagPointer) (stride, pointer);
}

static void
logEdgeFlagv (const GLboolean *flag)
{
    vCnt[edgeFlagvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glEdgeFlagv (%p)\n", flag);
    (*nativeRenderTable->EdgeFlagv) (flag);
}

static void
logEnable (GLenum cap)
{
    fprintf (logFp, "glEnable (0x%x)\n", cap);
    (*nativeRenderTable->Enable) (cap);
}

static void
logEnableClientState (GLenum array)
{
    fprintf (logFp, "glEnableClientState (0x%x)\n", array);
    (*nativeRenderTable->EnableClientState) (array);
}

static void
logEnd (void)
{
    int i;

    for (i = 0; i < sizeof (vCnt) / sizeof (vCnt[0]); i++)
    {
	if (vCnt[i].n)
	{
	    fprintf (logFp, "  %s: %d\n", vCnt[i].name, vCnt[i].n);
	    vCnt[i].n = 0;
	}
    }

    fprintf (logFp, "glEnd ()\n" );
    (*nativeRenderTable->End) ();
}

static void
logEndList (void)
{
    fprintf (logFp, "glEndList ()\n" );
    (*nativeRenderTable->EndList) ();
}

static void
logEvalCoord1dv (const GLdouble *u)
{
    vCnt[evalCoord1dvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glEvalCoord1dv (%p)\n", u);
    (*nativeRenderTable->EvalCoord1dv) (u);
}

static void
logEvalCoord1fv (const GLfloat *u)
{
    vCnt[evalCoord1fvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glEvalCoord1fv (%p)\n", u);
    (*nativeRenderTable->EvalCoord1fv) (u);
}

static void
logEvalCoord2dv (const GLdouble *u)
{
    vCnt[evalCoord2dvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glEvalCoord2dv (%p)\n", u);
    (*nativeRenderTable->EvalCoord2dv) (u);
}

static void
logEvalCoord2fv (const GLfloat *u)
{
    vCnt[evalCoord1fvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glEvalCoord2fv (%p)\n", u);
    (*nativeRenderTable->EvalCoord2fv) (u);
}

static void
logEvalMesh1 (GLenum mode,
	      GLint  i1,
	      GLint  i2)
{
    fprintf (logFp, "glEvalMesh1 (0x%x, %d, %d)\n", mode, i1, i2);
    (*nativeRenderTable->EvalMesh1) (mode, i1, i2 );
}

static void
logEvalMesh2 (GLenum mode,
	      GLint  i1,
	      GLint  i2,
	      GLint  j1,
	      GLint  j2)
{
    fprintf (logFp, "glEvalMesh2 (0x%x, %d, %d, %d, %d)\n",
	     mode, i1, i2, j1, j2);
    (*nativeRenderTable->EvalMesh2) (mode, i1, i2, j1, j2);
}

static void
logEvalPoint1 (GLint i)
{
    vCnt[evalPoint1Index].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glEvalPoint1 (%d)\n", i);
    (*nativeRenderTable->EvalPoint1) (i);
}

static void
logEvalPoint2 (GLint i, GLint j)
{
    vCnt[evalPoint2Index].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glEvalPoint2 (%d, %d)\n", i, j);
    (*nativeRenderTable->EvalPoint2) (i, j);
}

static void
logFeedbackBuffer (GLsizei size,
		   GLenum  type,
		   GLfloat *buffer)
{
    fprintf (logFp, "glFeedbackBuffer (%d, 0x%x, %p)\n", size, type, buffer);
    (*nativeRenderTable->FeedbackBuffer) (size, type, buffer);
}

static void
logFinish (void)
{
    fprintf (logFp, "glFinish ()\n");
    (*nativeRenderTable->Finish) ();
}

static void
logFlush (void)
{
    fprintf (logFp, "glFlush ()\n");
    (*nativeRenderTable->Flush) ();
}

static void
logFogf (GLenum  pname,
	 GLfloat param)
{
    fprintf (logFp, "glFogf (0x%x, %f)\n", pname, param);
    (*nativeRenderTable->Fogf) (pname, param);
}

static void
logFogfv (GLenum	pname,
	  const GLfloat *params)
{
    fprintf (logFp, "glFogfv (0x%x, %p)\n", pname, params);
    (*nativeRenderTable->Fogfv) (pname, params);
}

static void
logFogi (GLenum pname,
	 GLint  param)
{
    fprintf (logFp, "glFogi (0x%x, %d)\n", pname, param);
    (*nativeRenderTable->Fogi) (pname, param);
}

static void
logFogiv (GLenum      pname,
	  const GLint *params)
{
    fprintf (logFp, "glFogiv (0x%x, %p)\n", pname, params);
    (*nativeRenderTable->Fogiv) (pname, params);
}

static void
logFrontFace (GLenum mode)
{
    fprintf (logFp, "glFrontFace (0x%x)\n", mode);
    (*nativeRenderTable->FrontFace) (mode);
}

static void
logFrustum (GLdouble left,
	    GLdouble right,
	    GLdouble bottom,
	    GLdouble top,
	    GLdouble zNear,
	    GLdouble zFar)
{
    fprintf (logFp, "glFrustum (%f, %f, %f, %f, %f, %f)\n",
	     left, right, bottom, top, zNear, zFar);
    (*nativeRenderTable->Frustum) (left, right, bottom, top, zNear, zFar);
}

static GLuint
logGenLists (GLsizei range)
{
    fprintf (logFp, "glGenLists (%d)\n", range);
    return (*nativeRenderTable->GenLists) (range);
}

static void
logGenTextures (GLsizei n,
		GLuint *textures)
{
    fprintf (logFp, "glGenTextures (%d, %p)\n", n, textures);
    (*nativeRenderTable->GenTextures) (n, textures);
}
static void
logGetBooleanv (GLenum	  pname,
		GLboolean *params)
{
    fprintf (logFp, "glGetBooleanv (0x%x, %p)\n", pname, params);
    (*nativeRenderTable->GetBooleanv) (pname, params);
}

static void
logGetClipPlane (GLenum   plane,
		 GLdouble *equation)
{
    fprintf (logFp, "glGetClipPlane (0x%x, %p)\n", plane, equation);
    (*nativeRenderTable->GetClipPlane) (plane, equation);
}

static void
logGetDoublev (GLenum   pname,
	       GLdouble *params)
{
    fprintf (logFp, "glGetDoublev (0x%x, %p)\n", pname, params);
    (*nativeRenderTable->GetDoublev) (pname, params);
}

static GLenum
logGetError (void)
{
    fprintf (logFp, "glGetError ()\n");
    return (*nativeRenderTable->GetError) ();
}

static void
logGetFloatv (GLenum  pname,
	      GLfloat *params)
{
    fprintf (logFp, "glGetFloatv (0x%x, %p)\n", pname, params);
    (*nativeRenderTable->GetFloatv) (pname, params);
}

static void
logGetIntegerv (GLenum pname,
		GLint  *params)
{
    fprintf (logFp, "glGetIntegerv (0x%x, %p)\n", pname, params);
    (*nativeRenderTable->GetIntegerv) (pname, params);
}

static void
logGetLightfv (GLenum  light,
	       GLenum  pname,
	       GLfloat *params)
{
    fprintf (logFp, "glGetLightfv (0x%x, 0x%x, %p)\n", light, pname, params);
    (*nativeRenderTable->GetLightfv) (light, pname, params);
}

static void
logGetLightiv (GLenum light,
	       GLenum pname,
	       GLint  *params)
{
    fprintf (logFp, "glGetLightiv (0x%x, 0x%x, %p)\n",
	     light, pname, params);
    (*nativeRenderTable->GetLightiv) (light, pname, params);
}

static void
logGetMapdv (GLenum   target,
	     GLenum   query,
	     GLdouble *v)
{
    fprintf (logFp, "glGetMapdv (0x%x, 0x%x, %p)\n", target, query, v);
    (*nativeRenderTable->GetMapdv) (target, query, v);
}

static void
logGetMapfv (GLenum  target,
	     GLenum  query,
	     GLfloat *v)
{
    fprintf (logFp, "glGetMapfv (0x%x, 0x%x, %p)\n", target, query, v);
    (*nativeRenderTable->GetMapfv) (target, query, v);
}

static void
logGetMapiv (GLenum target,
	     GLenum query,
	     GLint  *v)
{
    fprintf (logFp, "glGetMapiv (0x%x, 0x%x, %p)\n", target, query, v);
    (*nativeRenderTable->GetMapiv) (target, query, v);
}

static void
logGetMaterialfv (GLenum  face,
		  GLenum  pname,
		  GLfloat *params)
{
    fprintf (logFp, "glGetMaterialfv (0x%x, 0x%x, %p)\n", face, pname, params);
    (*nativeRenderTable->GetMaterialfv) (face, pname, params);
}

static void
logGetMaterialiv (GLenum face,
		  GLenum pname,
		  GLint  *params)
{
    fprintf (logFp, "glGetMaterialiv (0x%x, 0x%x, %p)\n", face, pname, params);
    (*nativeRenderTable->GetMaterialiv) (face, pname, params);
}

static void
logGetPixelMapfv (GLenum  map,
		  GLfloat *values)
{
    fprintf (logFp, "glGetPixelMapfv (0x%x, %p)\n", map, values);
    (*nativeRenderTable->GetPixelMapfv) (map, values);
}

static void
logGetPixelMapuiv (GLenum map,
		   GLuint *values)
{
    fprintf (logFp, "glGetPixelMapuiv (0x%x, %p)\n", map, values);
    (*nativeRenderTable->GetPixelMapuiv) (map, values);
}

static void
logGetPixelMapusv (GLenum   map,
		   GLushort *values)
{
    fprintf (logFp, "glGetPixelMapusv (0x%x, %p)\n", map, values);
    (*nativeRenderTable->GetPixelMapusv) (map, values);
}

static void
logGetPointerv (GLenum pname,
		GLvoid **params)
{
    fprintf (logFp, "glGetPointerv (0x%x, %p)\n", pname, params);
    (*nativeRenderTable->GetPointerv) (pname, params);
}

static void
logGetPolygonStipple (GLubyte *mask)
{
    fprintf (logFp, "glGetPolygonStipple (%p)\n", mask);
    (*nativeRenderTable->GetPolygonStipple) (mask);
}

static const GLubyte *
logGetString (GLenum name)
{
    fprintf (logFp, "glGetString (0x%x)\n", name);
    return (*nativeRenderTable->GetString) (name);
}

static void
logGetTexEnvfv (GLenum  target,
		GLenum  pname,
		GLfloat *params)
{
    fprintf (logFp, "glGetTexEnvfv (0x%x, 0x%x, %p)\n", target, pname, params);
    (*nativeRenderTable->GetTexEnvfv) (target, pname, params);
}

static void
logGetTexEnviv (GLenum target,
		GLenum pname,
		GLint  *params)
{
    fprintf (logFp, "glGetTexEnviv (0x%x, 0x%x, %p)\n", target, pname, params);
    (*nativeRenderTable->GetTexEnviv) (target, pname, params);
}

static void
logGetTexGendv (GLenum   coord,
		GLenum   pname,
		GLdouble *params)
{
    fprintf (logFp, "glGetTexGendv (0x%x, 0x%x, %p)\n", coord, pname, params);
    (*nativeRenderTable->GetTexGendv) (coord, pname, params);
}

static void
logGetTexGenfv (GLenum  coord,
		GLenum  pname,
		GLfloat *params)
{
    fprintf (logFp, "glGetTexGenfv (0x%x, 0x%x, %p)\n", coord, pname, params);
    (*nativeRenderTable->GetTexGenfv) (coord, pname, params);
}

static void
logGetTexGeniv (GLenum coord,
		GLenum pname,
		GLint  *params)
{
    fprintf (logFp, "glGetTexGeniv (0x%x, 0x%x, %p)\n", coord, pname, params);
    (*nativeRenderTable->GetTexGeniv) (coord, pname, params);
}

static void
logGetTexImage (GLenum target,
		GLint  level,
		GLenum format,
		GLenum type,
		void   *pixels)
{
    fprintf (logFp, "glGetTexImage (0x%x, %d, 0x%x, 0x%x, %p)\n",
	     target, level, format, type, pixels);
    (*nativeRenderTable->GetTexImage) (target, level, format, type,
				       pixels);
}
static void
logGetTexLevelParameterfv (GLenum  target,
			   GLint   level,
			   GLenum  pname,
			   GLfloat *params)
{
    fprintf (logFp, "glGetTexLevelParameterfv (0x%x, %d, 0x%x, %p)\n",
	     target, level, pname, params);
    (*nativeRenderTable->GetTexLevelParameterfv) (target, level,
						  pname, params);
}

static void
logGetTexLevelParameteriv (GLenum target,
			   GLint  level,
			   GLenum pname,
			   GLint  *params)
{
    fprintf (logFp, "glGetTexLevelParameteriv (0x%x, %d, 0x%x, %p)\n",
	     target, level, pname, params);
    (*nativeRenderTable->GetTexLevelParameteriv) (target, level,
						  pname, params);
}

static void
logGetTexParameterfv (GLenum  target,
		      GLenum  pname,
		      GLfloat *params)
{
    fprintf (logFp, "glGetTexParameterfv (0x%x, 0x%x, %p)\n",
	     target, pname, params);
    (*nativeRenderTable->GetTexParameterfv) (target, pname, params);
}

static void
logGetTexParameteriv (GLenum target,
		      GLenum pname,
		      GLint  *params)
{
    fprintf (logFp, "glGetTexParameteriv (0x%x, 0x%x, %p)\n",
	     target, pname, params);
    (*nativeRenderTable->GetTexParameteriv) (target, pname, params);
}

static void
logHint (GLenum target,
	 GLenum mode)
{
    fprintf (logFp, "glHint (0x%x, 0x%x)\n", target, mode);
    (*nativeRenderTable->Hint) (target, mode);
}

static void
logIndexMask (GLuint mask)
{
    fprintf (logFp, "glIndexMask (%d)\n", mask);
    (*nativeRenderTable->IndexMask) (mask);
}

static void
logIndexPointer (GLenum	    type,
		 GLsizei    stride,
		 const void *pointer)
{
    fprintf (logFp, "glIndexPointer (0x%x, %d, %p)\n", type, stride, pointer);
    (*nativeRenderTable->IndexPointer) (type, stride, pointer);
}

static void
logIndexdv (const GLdouble *c)
{
    vCnt[indexdvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glIndexdv (%p)\n", c);
    (*nativeRenderTable->Indexdv) (c);
}

static void
logIndexfv (const GLfloat *c)
{
    vCnt[indexfvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glIndexfv (%p)\n", c);
    (*nativeRenderTable->Indexfv) (c);
}

static void
logIndexiv (const GLint *c)
{
    vCnt[indexivIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glIndexiv (%p)\n", c);
    (*nativeRenderTable->Indexiv) (c);
}

static void
logIndexsv (const GLshort *c)
{
    vCnt[indexsvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glIndexsv (%p)\n", c);
    (*nativeRenderTable->Indexsv) (c);
}

static void
logIndexubv (const GLubyte *c)
{
    vCnt[indexubvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glIndexubv (%p)\n", c);
    (*nativeRenderTable->Indexubv) (c);
}

static void
logInitNames (void)
{
    fprintf (logFp, "glInitNames ()\n" );
    (*nativeRenderTable->InitNames) ();
}

static void
logInterleavedArrays (GLenum	 format,
		      GLsizei	 stride,
		      const void *pointer)
{
    fprintf (logFp, "glInterleavedArrays (0x%x, %d, %p)\n",
	     format, stride, pointer);
    (*nativeRenderTable->InterleavedArrays) (format, stride, pointer);
}

static GLboolean
logIsEnabled (GLenum cap)
{
    fprintf (logFp, "glIsEnabled (0x%x)\n", cap);
    return (*nativeRenderTable->IsEnabled) (cap);
}

static GLboolean
logIsList (GLuint list)
{
    fprintf (logFp, "glIsList (%d)\n", list);
    return (*nativeRenderTable->IsList) (list);
}

static GLboolean
logIsTexture (GLuint texture)
{
    fprintf (logFp, "glIsTexture (%d)\n", texture);
    return (*nativeRenderTable->IsTexture) (texture);
}

static void
logLightModelf (GLenum  pname,
		GLfloat param)
{
    fprintf (logFp, "glLightModelf (0x%x, %f)\n", pname, param);
    (*nativeRenderTable->LightModelf) (pname, param);
}

static void
logLightModelfv (GLenum	       pname,
		 const GLfloat *params)
{
    fprintf (logFp, "glLightModelfv (0x%x, %p)\n", pname, params);
    (*nativeRenderTable->LightModelfv) (pname, params);
}

static void
logLightModeli (GLenum pname,
		GLint  param)
{
    fprintf (logFp, "glLightModeli (0x%x, %d)\n", pname, param);
    (*nativeRenderTable->LightModeli) (pname, param);
}

static void
logLightModeliv (GLenum	     pname,
		 const GLint *params)
{
    fprintf (logFp, "glLightModeliv (0x%x, %p)\n", pname, params);
    (*nativeRenderTable->LightModeliv) (pname, params);
}

static void
logLightf (GLenum  light,
	   GLenum  pname,
	   GLfloat param)
{
    fprintf (logFp, "glLightf (0x%x, 0x%x, %f)\n", light, pname, param);
    (*nativeRenderTable->Lightf) (light, pname, param);
}

static void
logLightfv (GLenum	  light,
	    GLenum	  pname,
	    const GLfloat *params)
{
    fprintf (logFp, "glLightfv (0x%x, 0x%x, %p)\n", light, pname, params);
    (*nativeRenderTable->Lightfv) (light, pname, params);
}

static void
logLighti (GLenum light,
	   GLenum pname,
	   GLint  param)
{
    fprintf (logFp, "glLighti (0x%x, 0x%x, %d)\n", light, pname, param);
    (*nativeRenderTable->Lighti) (light, pname, param);
}

static void
logLightiv (GLenum	light,
	    GLenum	pname,
	    const GLint *params)
{
    fprintf (logFp, "glLightiv (0x%x, 0x%x, %p)\n", light, pname, params);
    (*nativeRenderTable->Lightiv) (light, pname, params);
}

static void
logLineStipple (GLint    factor,
		GLushort pattern)
{
    fprintf (logFp, "glLineStipple (%d, %d)\n", factor, pattern);
    (*nativeRenderTable->LineStipple) (factor, pattern);
}

static void
logLineWidth (GLfloat width)
{
    fprintf (logFp, "glLineWidth (%f)\n", width);
    (*nativeRenderTable->LineWidth) (width);
}

static void
logListBase (GLuint base)
{
    fprintf (logFp, "glListBase (%d)\n", base);
    (*nativeRenderTable->ListBase) (base);
}

static void
logLoadIdentity (void)
{
    fprintf (logFp, "glLoadIdentity ()\n");
    (*nativeRenderTable->LoadIdentity) ();
}

static void
logLoadMatrixd (const GLdouble *m)
{
    fprintf (logFp, "glLoadMatrixd (%p)\n", m);
    (*nativeRenderTable->LoadMatrixd) (m);
}

static void
logLoadMatrixf (const GLfloat *m)
{
    fprintf (logFp, "glLoadMatrixf (%p)\n", m);
    (*nativeRenderTable->LoadMatrixf) (m);
}

static void
logLoadName (GLuint name)
{
    fprintf (logFp, "glLoadName (%d)\n", name);
    (*nativeRenderTable->LoadName) (name);
}

static void
logLogicOp (GLenum opcode)
{
    fprintf (logFp, "glLogicOp(0x%x)\n", opcode);
    (*nativeRenderTable->LogicOp) (opcode);
}

static void
logMap1d (GLenum	 target,
	  GLdouble	 u1,
	  GLdouble	 u2,
	  GLint		 stride,
	  GLint		 order,
	  const GLdouble *points)
{
    fprintf (logFp, "glMap1d (0x%x, %f, %f, %d, %d, %p)\n",
	     target, u1, u2, stride, order, points);
    (*nativeRenderTable->Map1d) (target, u1, u2, stride, order, points);
}

static void
logMap1f (GLenum	target,
	  GLfloat	u1,
	  GLfloat	u2,
	  GLint		stride,
	  GLint		order,
	  const GLfloat *points)
{
    fprintf (logFp, "glMap1f (0x%x, %f, %f, %d, %d, %p)\n",
	     target, u1, u2, stride, order, points);
    (*nativeRenderTable->Map1f) (target, u1, u2, stride, order, points);
}

static void
logMap2d (GLenum	 target,
	  GLdouble	 u1,
	  GLdouble	 u2,
	  GLint		 ustride,
	  GLint		 uorder,
	  GLdouble	 v1,
	  GLdouble	 v2,
	  GLint		 vstride,
	  GLint		 vorder,
	  const GLdouble *points)
{
    fprintf (logFp, "glMap2d (0x%x, %f, %f, %d, %d, %f, %f, %d, %d, %p)\n",
	     target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
    (*nativeRenderTable->Map2d) (target, u1, u2, ustride, uorder, v1, v2,
				 vstride, vorder, points);
}

static void
logMap2f (GLenum	target,
	  GLfloat	u1,
	  GLfloat	u2,
	  GLint		ustride,
	  GLint		uorder,
	  GLfloat	v1,
	  GLfloat	v2,
	  GLint		vstride,
	  GLint		vorder,
	  const GLfloat *points)
{
    fprintf (logFp, "glMap2f (0x%x, %f, %f, %d, %d, %f, %f, %d, %d, %p)\n",
	     target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
    (*nativeRenderTable->Map2f) (target, u1, u2, ustride, uorder, v1, v2,
				 vstride, vorder, points);
}

static void
logMapGrid1d (GLint    un,
	      GLdouble u1,
	      GLdouble u2)
{
    fprintf (logFp, "glMapGrid1d (%d, %f, %f)\n", un, u1, u2);
    (*nativeRenderTable->MapGrid1d) (un, u1, u2);
}

static void
logMapGrid1f (GLint   un,
	      GLfloat u1,
	      GLfloat u2)
{
    fprintf (logFp, "glMapGrid1f (%d, %f, %f)\n", un, u1, u2);
    (*nativeRenderTable->MapGrid1f) (un, u1, u2);
}

static void
logMapGrid2d (GLint    un,
	      GLdouble u1,
	      GLdouble u2,
	      GLint    vn,
	      GLdouble v1,
	      GLdouble v2)
{
    fprintf (logFp, "glMapGrid2d (%d, %f, %f, %d, %f, %f)\n",
	     un, u1, u2, vn, v1, v2);
    (*nativeRenderTable->MapGrid2d) (un, u1, u2, vn, v1, v2);
}

static void
logMapGrid2f (GLint   un,
	      GLfloat u1,
	      GLfloat u2,
	      GLint   vn,
	      GLfloat v1,
	      GLfloat v2)
{
    fprintf (logFp, "glMapGrid2f (%d, %f, %f, %d, %f, %f)\n",
	     un, u1, u2, vn, v1, v2);
    (*nativeRenderTable->MapGrid2f) (un, u1, u2, vn, v1, v2);
}

static void
logMaterialf (GLenum  face,
	      GLenum  pname,
	      GLfloat param)
{
    vCnt[materialfIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glMaterialf (0x%x, 0x%x, %f)\n", face, pname, param);
    (*nativeRenderTable->Materialf) (face, pname, param);
}

static void
logMaterialfv (GLenum	     face,
	       GLenum	     pname,
	       const GLfloat *params)
{
    vCnt[materialfvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glMaterialfv (0x%x, 0x%x, %p)\n",
		 face, pname, params);
    (*nativeRenderTable->Materialfv) (face, pname, params);
}

static void
logMateriali (GLenum face,
	      GLenum pname,
	      GLint  param)
{
    vCnt[materialiIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glMateriali (0x%x, 0x%x, %d)\n", face, pname, param);
    (*nativeRenderTable->Materiali) (face, pname, param);
}

static void
logMaterialiv (GLenum	   face,
	       GLenum	   pname,
	       const GLint *params)
{
    vCnt[materialivIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glMaterialiv (0x%x, 0x%x, %p)\n",
		 face, pname, params);
    (*nativeRenderTable->Materialiv) (face, pname, params);
}

static void
logMatrixMode (GLenum mode)
{
    fprintf (logFp, "glMatrixMode (0x%x)\n", mode);
    (*nativeRenderTable->MatrixMode) (mode);
}

static void
logMultMatrixd (const GLdouble *m)
{
    fprintf (logFp, "glMultMatrixd (%p)\n", m);
    (*nativeRenderTable->MultMatrixd) (m);
}

static void
logMultMatrixf (const GLfloat *m)
{
    fprintf (logFp, "glMultMatrixf (%p)\n", m);
    (*nativeRenderTable->MultMatrixf) (m);
}

static void
logNewList (GLuint list,
	    GLenum mode)
{
    fprintf (logFp, "glNewList (%d, 0x%x)\n", list, mode);
    (*nativeRenderTable->NewList) (list, mode);
}

static void
logNormal3bv (const GLbyte *v)
{
    vCnt[normal3bvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glNormal3bv (%p)\n", v);
    (*nativeRenderTable->Normal3bv) (v);
}

static void
logNormal3dv (const GLdouble *v)
{
    vCnt[normal3dvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glNormal3dv (%p)\n", v);
    (*nativeRenderTable->Normal3dv) (v);
}

static void
logNormal3fv (const GLfloat *v)
{
    vCnt[normal3fvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glNormal3fv (%p)\n", v);
    (*nativeRenderTable->Normal3fv) (v);
}

static void
logNormal3iv (const GLint *v)
{
    vCnt[normal3ivIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glNormal3iv (%p)\n", v);
    (*nativeRenderTable->Normal3iv) (v);
}

static void
logNormal3sv (const GLshort *v)
{
    vCnt[normal3svIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glNormal3sv (%p)\n", v);
    (*nativeRenderTable->Normal3sv) (v);
}

static void
logNormalPointer (GLenum     type,
		  GLsizei    stride,
		  const void *pointer)
{
    fprintf (logFp, "glNormalPointer (0x%x, %d, %p)\n", type, stride, pointer);
    (*nativeRenderTable->NormalPointer) (type, stride, pointer);
}

static void
logOrtho (GLdouble left,
	  GLdouble right,
	  GLdouble bottom,
	  GLdouble top,
	  GLdouble zNear,
	  GLdouble zFar)
{
    fprintf (logFp, "glOrtho (%f, %f, %f, %f, %f, %f)\n",
	     left, right, bottom, top, zNear, zFar);
    (*nativeRenderTable->Ortho) (left, right, bottom, top, zNear, zFar);
}

static void
logPassThrough (GLfloat token)
{
    fprintf (logFp, "glPassThrough (%f)\n", token);
    (*nativeRenderTable->PassThrough) (token);
}

static void
logPixelMapfv (GLenum	     map,
	       GLsizei	     mapsize,
	       const GLfloat *values)
{
    fprintf (logFp, "glPixelMapfv (0x%x, %d, %p)\n", map, mapsize, values);
    (*nativeRenderTable->PixelMapfv) (map, mapsize, values);
}

static void
logPixelMapuiv (GLenum	     map,
		GLsizei	     mapsize,
		const GLuint *values)
{
    fprintf (logFp, "glPixelMapuiv (0x%x, %d, %p)\n", map, mapsize, values);
    (*nativeRenderTable->PixelMapuiv) (map, mapsize, values);
}

static void
logPixelMapusv (GLenum	       map,
		GLsizei	       mapsize,
		const GLushort *values)
{
    fprintf (logFp, "glPixelMapusv (0x%x, %d, %p)\n", map, mapsize, values);
    (*nativeRenderTable->PixelMapusv) (map, mapsize, values);
}

static void
logPixelStoref (GLenum  pname,
		GLfloat param)
{
    fprintf (logFp, "glPixelStoref (0x%x, %f)\n", pname, param);
    (*nativeRenderTable->PixelStoref) (pname, param);
}

static void
logPixelStorei (GLenum pname,
		GLint  param)
{
    fprintf (logFp, "glPixelStorei (0x%x, %d)\n", pname, param);
    (*nativeRenderTable->PixelStorei) (pname, param);
}

static void
logPixelTransferf (GLenum pname, GLfloat param)
{
    fprintf (logFp, "glPixelTransferf (0x%x, %f)\n", pname, param);
    (*nativeRenderTable->PixelTransferf) (pname, param);
}

static void
logPixelTransferi (GLenum pname,
		   GLint  param)
{
    fprintf (logFp, "glPixelTransferi (0x%x, %d)\n", pname, param);
    (*nativeRenderTable->PixelTransferi) (pname, param);
}

static void
logPixelZoom (GLfloat xfactor,
	      GLfloat yfactor)
{
    fprintf (logFp, "glPixelZoom (%f, %f)\n", xfactor, yfactor);
    (*nativeRenderTable->PixelZoom) (xfactor, yfactor);
}

static void
logPointSize (GLfloat size)
{
    fprintf (logFp, "glPointSize" );
    (*nativeRenderTable->PointSize) (size);
}

static void
logPolygonMode (GLenum face,
		GLenum mode)
{
    fprintf (logFp, "glPolygonMode (0x%x, 0x%x)\n", face, mode );
    (*nativeRenderTable->PolygonMode) (face, mode);
}

static void
logPolygonOffset (GLfloat factor,
		  GLfloat units)
{
    fprintf (logFp, "glPolygonOffset (%f, %f)\n", factor, units);
    (*nativeRenderTable->PolygonOffset) (factor, units);
}

static void
logPolygonStipple (const GLubyte *mask)
{
    fprintf (logFp, "glPolygonStipple (%p)\n", mask);
    (*nativeRenderTable->PolygonStipple) (mask);
}

static void
logPopAttrib (void)
{
    fprintf (logFp, "glPopAttrib ()\n");
    (*nativeRenderTable->PopAttrib) ();
}

static void
logPopClientAttrib (void)
{
    fprintf (logFp, "glPopClientAttrib ()\n" );
    (*nativeRenderTable->PopClientAttrib) ();
}

static void
logPopMatrix (void)
{
    fprintf (logFp, "glPopMatrix ()\n" );
    (*nativeRenderTable->PopMatrix) ();
}

static void
logPopName (void)
{
    fprintf (logFp, "glPopName ()\n");
    (*nativeRenderTable->PopName) ();
}

static void
logPrioritizeTextures (GLsizei	      n,
		       const GLuint   *textures,
		       const GLclampf *priorities)
{
    fprintf (logFp, "glPrioritizeTextures (%d, %p, %p)\n",
	     n, textures, priorities);
    (*nativeRenderTable->PrioritizeTextures) (n, textures, priorities);
}

static void
logPushAttrib (GLbitfield mask)
{
    fprintf (logFp, "glPushAttrib (0x%x)\n", mask);
    (*nativeRenderTable->PushAttrib) (mask);
}

static void
logPushClientAttrib (GLbitfield mask)
{
    fprintf (logFp, "glPushClientAttrib (0x%x)\n", mask);
    (*nativeRenderTable->PushClientAttrib) (mask);
}

static void
logPushMatrix (void)
{
    fprintf (logFp, "glPushMatrix ()\n" );
    (*nativeRenderTable->PushMatrix) ();
}

static void
logPushName (GLuint name)
{
    fprintf (logFp, "glPushName (%d)\n", name);
    (*nativeRenderTable->PushName) (name);
}

static void
logRasterPos2dv (const GLdouble *v)
{
    fprintf (logFp, "glRasterPos2dv (%p)\n", v);
    (*nativeRenderTable->RasterPos2dv) (v);
}

static void
logRasterPos2fv (const GLfloat *v)
{
    fprintf (logFp, "glRasterPos2dv (%p)\n", v);
    (*nativeRenderTable->RasterPos2fv) (v);
}

static void
logRasterPos2iv (const GLint *v)
{
    fprintf (logFp, "glRasterPos2iv (%p)\n", v);
    (*nativeRenderTable->RasterPos2iv) (v);
}

static void
logRasterPos2sv (const GLshort *v)
{
    fprintf (logFp, "glRasterPos2sv (%p)\n", v);
    (*nativeRenderTable->RasterPos2sv) (v);
}

static void
logRasterPos3dv (const GLdouble *v)
{
    fprintf (logFp, "glRasterPos3dv (%p)\n", v);
    (*nativeRenderTable->RasterPos3dv) (v);
}

static void
logRasterPos3fv (const GLfloat *v)
{
    fprintf (logFp, "glRasterPos3fv (%p)\n", v);
    (*nativeRenderTable->RasterPos3fv) (v);
}

static void
logRasterPos3iv (const GLint *v)
{
    fprintf (logFp, "glRasterPos3iv (%p)\n", v);
    (*nativeRenderTable->RasterPos3iv) (v);
}

static void
logRasterPos3sv (const GLshort *v)
{
    fprintf (logFp, "glRasterPos3sv (%p)\n", v);
    (*nativeRenderTable->RasterPos3sv) (v);
}

static void
logRasterPos4dv (const GLdouble *v)
{
    fprintf (logFp, "glRasterPos4dv (%p)\n", v);
    (*nativeRenderTable->RasterPos4dv) (v);
}

static void
logRasterPos4fv (const GLfloat *v)
{
    fprintf (logFp, "glRasterPos4fv (%p)\n", v);
    (*nativeRenderTable->RasterPos4fv) (v);
}

static void
logRasterPos4iv (const GLint *v)
{
    fprintf (logFp, "glRasterPos4iv (%p)\n", v);
    (*nativeRenderTable->RasterPos4iv) (v);
}

static void
logRasterPos4sv (const GLshort *v)
{
    fprintf (logFp, "glRasterPos4sv (%p)\n", v);
    (*nativeRenderTable->RasterPos4sv) (v);
}

static void
logReadBuffer (GLenum mode)
{
    fprintf (logFp, "glReadBuffer (0x%x)\n", mode);
    (*nativeRenderTable->ReadBuffer) (mode);
}

static void
logReadPixels (GLint   x,
	       GLint   y,
	       GLsizei width,
	       GLsizei height,
	       GLenum  format,
	       GLenum  type,
	       void    *pixels)
{
    fprintf (logFp, "glReadPixels (%d, %d, %d, %d, 0x%x, 0x%x, %p)\n",
	     x, y, width, height, format, type, pixels);
    (*nativeRenderTable->ReadPixels) (x, y, width, height, format, type,
				      pixels);
}

static void
logRectdv (const GLdouble *v1,
	   const GLdouble *v2)
{
    fprintf (logFp, "glRectdv (%p, %p)\n", v1, v2);
    (*nativeRenderTable->Rectdv) (v1, v2);
}

static void
logRectfv (const GLfloat *v1,
	   const GLfloat *v2)
{
    fprintf (logFp, "glRectfv (%p, %p)\n", v1, v2);
    (*nativeRenderTable->Rectfv) (v1, v2);
}

static void
logRectiv (const GLint *v1,
	   const GLint *v2)
{
    fprintf (logFp, "glRectiv (%p, %p)\n", v1, v2);
    (*nativeRenderTable->Rectiv) (v1, v2);
}

static void
logRectsv (const GLshort *v1,
	   const GLshort *v2)
{
    fprintf (logFp, "glRectsv (%p, %p)\n", v1, v2);
    (*nativeRenderTable->Rectsv) (v1, v2);
}

static GLint
logRenderMode (GLenum mode)
{
    fprintf (logFp, "glRenderMode (0x%x)\n", mode);
    return (*nativeRenderTable->RenderMode) (mode);
}

static void
logRotated (GLdouble angle,
	    GLdouble x,
	    GLdouble y,
	    GLdouble z)
{
    fprintf (logFp, "glRotated (%f, %f, %f, %f)\n", angle, x, y, z);
    (*nativeRenderTable->Rotated) (angle, x, y, z);
}

static void
logRotatef (GLfloat angle,
	    GLfloat x,
	    GLfloat y,
	    GLfloat z)
{
    fprintf (logFp, "glRotatef (%f, %f, %f, %f)\n", angle, x, y, z);
    (*nativeRenderTable->Rotatef) (angle, x, y, z);
}

static void
logScaled (GLdouble x,
	   GLdouble y,
	   GLdouble z)
{
    fprintf (logFp, "glScaled (%f, %f, %f)\n", x, y, z);
    (*nativeRenderTable->Scaled) (x, y, z);
}

static void
logScalef (GLfloat x,
	   GLfloat y,
	   GLfloat z)
{
    fprintf (logFp, "glScalef (%f, %f, %f)\n", x, y, z);
    (*nativeRenderTable->Scalef) (x, y, z);
}

static void
logScissor (GLint   x,
	    GLint   y,
	    GLsizei width,
	    GLsizei height)
{
    fprintf (logFp, "glScissor (%d, %d, %d, %d)\n", x, y, width, height);
    (*nativeRenderTable->Scissor) (x, y, width, height);
}

static void
logSelectBuffer (GLsizei size,
		 GLuint  *buffer)
{
    fprintf (logFp, "glSelectBuffer (%d, %p)\n", size, buffer);
    (*nativeRenderTable->SelectBuffer) (size, buffer);
}

static void
logShadeModel (GLenum mode)
{
    fprintf (logFp, "glShadeModel (0x%x)\n", mode);
    (*nativeRenderTable->ShadeModel) (mode);
}

static void
logStencilFunc (GLenum func,
		GLint  ref,
		GLuint mask)
{
    fprintf (logFp, "glStencilFunc (0x%x, %d, %d)\n", func, ref, mask);
    (*nativeRenderTable->StencilFunc) (func, ref, mask);
}

static void
logStencilMask (GLuint mask)
{
    fprintf (logFp, "glStencilMask (0x%x)\n", mask);
    (*nativeRenderTable->StencilMask) (mask);
}

static void
logStencilOp (GLenum fail,
	      GLenum zfail,
	      GLenum zpass)
{
    fprintf (logFp, "glStencilOp (0x%x, 0x%x, 0x%x)\n", fail, zfail, zpass);
    (*nativeRenderTable->StencilOp) (fail, zfail, zpass);
}

static void
logTexCoord1dv (const GLdouble *v)
{
    vCnt[texCoord1dvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glTexCoord1dv (%p)\n", v);
    (*nativeRenderTable->TexCoord1dv) (v);
}

static void
logTexCoord1fv (const GLfloat *v)
{
    vCnt[texCoord1fvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glTexCoord1fv (%p)\n", v);
    (*nativeRenderTable->TexCoord1fv) (v);
}

static void
logTexCoord1iv (const GLint *v)
{
    vCnt[texCoord1ivIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glTexCoord1iv (%p)\n", v);
    (*nativeRenderTable->TexCoord1iv) (v);
}

static void
logTexCoord1sv (const GLshort *v)
{
    vCnt[texCoord1svIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glTexCoord1sv (%p)\n", v);
    (*nativeRenderTable->TexCoord1sv) (v);
}

static void
logTexCoord2dv (const GLdouble *v)
{
    vCnt[texCoord2dvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glTexCoord2dv (%p)\n", v);
    (*nativeRenderTable->TexCoord2dv) (v);
}

static void
logTexCoord2fv (const GLfloat *v)
{
    vCnt[texCoord2fvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glTexCoord2fv (%p)\n", v);
    (*nativeRenderTable->TexCoord2fv) (v);
}

static void
logTexCoord2iv (const GLint *v)
{
    vCnt[texCoord2ivIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glTexCoord2iv (%p)\n", v);
    (*nativeRenderTable->TexCoord2iv) (v);
}

static void
logTexCoord2sv (const GLshort *v)
{
    vCnt[texCoord2svIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glTexCoord2sv (%p)\n", v);
    (*nativeRenderTable->TexCoord2sv) (v);
}


static void
logTexCoord3dv (const GLdouble *v)
{
    vCnt[texCoord3dvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glTexCoord3dv (%p)\n", v);
    (*nativeRenderTable->TexCoord3dv) (v);
}

static void
logTexCoord3fv (const GLfloat *v)
{
    vCnt[texCoord3fvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glTexCoord3fv (%p)\n", v);
    (*nativeRenderTable->TexCoord3fv) (v);
}

static void
logTexCoord3iv (const GLint *v)
{
    vCnt[texCoord3ivIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glTexCoord3iv (%p)\n", v);
    (*nativeRenderTable->TexCoord3iv) (v);
}

static void
logTexCoord3sv (const GLshort *v)
{
    vCnt[texCoord3svIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glTexCoord3sv (%p)\n", v);
    (*nativeRenderTable->TexCoord3sv) (v);
}

static void
logTexCoord4dv (const GLdouble *v)
{
    vCnt[texCoord4dvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glTexCoord4dv (%p)\n", v);
    (*nativeRenderTable->TexCoord4dv) (v);
}

static void
logTexCoord4fv (const GLfloat *v)
{
    vCnt[texCoord4fvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glTexCoord4fv (%p)\n", v);
    (*nativeRenderTable->TexCoord4fv) (v);
}

static void
logTexCoord4iv (const GLint *v)
{
    vCnt[texCoord4ivIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glTexCoord4iv (%p)\n", v);
    (*nativeRenderTable->TexCoord4iv) (v);
}

static void
logTexCoord4sv (const GLshort *v)
{
    vCnt[texCoord4svIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glTexCoord4sv (%p)\n", v);
    (*nativeRenderTable->TexCoord4sv) (v);
}

static void
logTexCoordPointer (GLint      size,
		    GLenum     type,
		    GLsizei    stride,
		    const void *pointer)
{
    fprintf (logFp, "glTexCoordPointer (%d, 0x%x, %d, %p)\n",
	     size, type, stride, pointer);
    (*nativeRenderTable->TexCoordPointer) (size, type, stride, pointer);
}

static void
logTexEnvf (GLenum  target,
	    GLenum  pname,
	    GLfloat param)
{
    fprintf (logFp, "glTexEnvf (0x%x, 0x%x, %f)\n", target, pname, param);
    (*nativeRenderTable->TexEnvf) (target, pname, param);
}

static void
logTexEnvfv (GLenum	   target,
	     GLenum	   pname,
	     const GLfloat *params)
{
    fprintf (logFp, "glTexEnvfv (0x%x, 0x%x, %p)\n", target, pname, params);
    (*nativeRenderTable->TexEnvfv) (target, pname, params);
}

static void
logTexEnvi (GLenum target,
	    GLenum pname,
	    GLint  param)
{
    fprintf (logFp, "glTexEnvi (0x%x, 0x%x, %d)\n", target, pname, param);
    (*nativeRenderTable->TexEnvi) (target, pname, param);
}

static void
logTexEnviv (GLenum	 target,
	     GLenum	 pname,
	     const GLint *params)
{
    fprintf (logFp, "glTexEnviv (0x%x, 0x%x, %p)\n", target, pname, params);
    (*nativeRenderTable->TexEnviv) (target, pname, params);
}

static void
logTexGend (GLenum   coord,
	    GLenum   pname,
	    GLdouble param)
{
    fprintf (logFp, "glTexGend (0x%x, 0x%x, %f)\n", coord, pname, param);
    (*nativeRenderTable->TexGend) (coord, pname, param);
}

static void
logTexGendv (GLenum	    coord,
	     GLenum	    pname,
	     const GLdouble *params)
{
    fprintf (logFp, "glTexGendv (0x%x, 0x%x, %p)\n", coord, pname, params);
    (*nativeRenderTable->TexGendv) (coord, pname, params);
}

static void
logTexGenf (GLenum  coord,
	    GLenum  pname,
	    GLfloat param)
{
    fprintf (logFp, "glTexGenf (0x%x, 0x%x, %f)\n", coord, pname, param);
    (*nativeRenderTable->TexGenf) (coord, pname, param);
}

static void
logTexGenfv (GLenum	   coord,
	     GLenum	   pname,
	     const GLfloat *params)
{
    fprintf (logFp, "glTexGenfv (0x%x, 0x%x, %p)\n", coord, pname, params);
    (*nativeRenderTable->TexGenfv) (coord, pname, params);
}

static void
logTexGeni (GLenum coord,
	    GLenum pname,
	    GLint  param)
{
    fprintf (logFp, "glTexGeni (0x%x, 0x%x, %d)\n", coord, pname, param);
    (*nativeRenderTable->TexGeni) (coord, pname, param);
}

static void
logTexGeniv (GLenum	 coord,
	     GLenum	 pname,
	     const GLint *params)
{
    fprintf (logFp, "glTexGeniv (0x%x, 0x%x, %p)\n", coord, pname, params);
    (*nativeRenderTable->TexGeniv) (coord, pname, params);
}

static void
logTexImage1D (GLenum	  target,
	       GLint	  level,
	       GLint	  internalformat,
	       GLsizei	  width,
	       GLint	  border,
	       GLenum	  format,
	       GLenum	  type,
	       const void *pixels)
{
    fprintf (logFp, "glTexImage1D (0x%x, %d, %d, %d, %d, 0x%x, 0x%x, %p)\n",
	     target, level, internalformat, width, border, format, type,
	     pixels);
    (*nativeRenderTable->TexImage1D) (target, level, internalformat,
				      width, border, format, type,
				      pixels);
}

static void
logTexImage2D (GLenum     target,
	       GLint	  level,
	       GLint	  internalformat,
	       GLsizei	  width,
	       GLsizei	  height,
	       GLint	  border,
	       GLenum	  format,
	       GLenum	  type,
	       const void *pixels)
{
    fprintf (logFp, "glTexImage2D (0x%x, %d, %d, %d, %d, %d, "
	     "0x%x, 0x%x, %p)\n", target, level, internalformat,
	     width, height, border, format, type, pixels);
    (*nativeRenderTable->TexImage2D) (target, level, internalformat,
				      width, height, border, format, type,
				      pixels);
}

static void
logTexParameterf (GLenum  target,
		  GLenum  pname,
		  GLfloat param)
{
    fprintf (logFp, "glTexParameterf (0x%x, 0x%x, %f)\n",
	     target, pname, param);
    (*nativeRenderTable->TexParameterf) (target, pname, param);
}

static void
logTexParameterfv (GLenum	 target,
		   GLenum	 pname,
		   const GLfloat *params)
{
    fprintf (logFp, "glTexParameterfv (0x%x, 0x%x, %p)\n",
	     target, pname, params);
    (*nativeRenderTable->TexParameterfv) (target, pname, params);
}

static void
logTexParameteri (GLenum target,
		  GLenum pname,
		  GLint  param)
{
    fprintf (logFp, "glTexParameteri (0x%x, 0x%x, 0x%x)\n",
	     target, pname, param);
    (*nativeRenderTable->TexParameteri) (target, pname, param);
}

static void
logTexParameteriv (GLenum      target,
		   GLenum      pname,
		   const GLint *params)
{
    fprintf (logFp, "glTexParameteriv (0x%x, 0x%x, %p)\n",
	     target, pname, params);
    (*nativeRenderTable->TexParameteriv) (target, pname, params);
}

static void
logTexSubImage1D (GLenum     target,
		  GLint	     level,
		  GLint	     xoffset,
		  GLsizei    width,
		  GLenum     format,
		  GLenum     type,
		  const void *pixels)
{
    fprintf (logFp, "glTexSubImage1D (0x%x, %d, %d, %d, 0x%x, 0x%x, %p)\n",
	     target, level, xoffset, width, format, type, pixels);
    (*nativeRenderTable->TexSubImage1D) (target, level, xoffset, width,
					 format, type, pixels);
}

static void
logTexSubImage2D (GLenum     target,
		  GLint	     level,
		  GLint	     xoffset,
		  GLint	     yoffset,
		  GLsizei    width,
		  GLsizei    height,
		  GLenum     format,
		  GLenum     type,
		  const void *pixels)
{
    fprintf (logFp, "glTexSubImage2D (0x%x, %d, %d, %d, %d, %d, "
	     "0x%x, 0x%x, %p)\n", target, level, xoffset, yoffset,
	     width, height, format, type, pixels);
    (*nativeRenderTable->TexSubImage2D) (target, level, xoffset, yoffset,
					 width, height, format, type,
					 pixels);
}

static void
logTranslated (GLdouble x,
	       GLdouble y,
	       GLdouble z)
{
    fprintf (logFp, "glTranslated (%f, %f, %f)\n", x, y, z);
    (*nativeRenderTable->Translated) (x, y, z);
}

static void
logTranslatef (GLfloat x,
	       GLfloat y,
	       GLfloat z)
{
    fprintf (logFp, "glTranslatef (%f, %f, %f)\n", x, y, z);
    (*nativeRenderTable->Translatef) (x, y, z);
}

static void
logVertex2dv (const GLdouble *v)
{
    vCnt[vertex2dvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glVertex2dv (%p)\n", v);
    (*nativeRenderTable->Vertex2dv) (v);
}

static void
logVertex2fv (const GLfloat *v)
{
    vCnt[vertex2fvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glVertex2dv (%p)\n", v);
    (*nativeRenderTable->Vertex2fv) (v);
}

static void
logVertex2iv (const GLint *v)
{
    vCnt[vertex2ivIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glVertex2iv (%p)\n", v);
    (*nativeRenderTable->Vertex2iv) (v);
}

static void
logVertex2sv (const GLshort *v)
{
    vCnt[vertex2svIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glVertex2sv (%p)\n", v);
    (*nativeRenderTable->Vertex2sv) (v);
}

static void
logVertex3dv (const GLdouble *v)
{
    vCnt[vertex3dvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glVertex3dv (%p)\n", v);
    (*nativeRenderTable->Vertex3dv) (v);
}

static void
logVertex3fv (const GLfloat *v)
{
    vCnt[vertex3fvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glVertex3fv (%p)\n", v);
    (*nativeRenderTable->Vertex3fv) (v);
}

static void
logVertex3iv (const GLint *v)
{
    vCnt[vertex3ivIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glVertex3iv (%p)\n", v);
    (*nativeRenderTable->Vertex3iv) (v);
}

static void
logVertex3sv (const GLshort *v)
{
    vCnt[vertex3svIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glVertex3sv (%p)\n", v);
    (*nativeRenderTable->Vertex3sv) (v);
}

static void
logVertex4dv (const GLdouble *v)
{
    vCnt[vertex4dvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glVertex4dv (%p)\n", v);
    (*nativeRenderTable->Vertex4dv) (v);
}

static void
logVertex4fv (const GLfloat *v)
{
    vCnt[vertex4fvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glVertex4fv (%p)\n", v);
    (*nativeRenderTable->Vertex4fv) (v);
}

static void
logVertex4iv (const GLint *v)
{
    vCnt[vertex4ivIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glVertex4iv (%p)\n", v);
    (*nativeRenderTable->Vertex4iv) (v);
}

static void
logVertex4sv (const GLshort *v)
{
    vCnt[vertex4svIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glVertex4sv (%p)\n", v);
    (*nativeRenderTable->Vertex4sv) (v);
}

static void
logVertexPointer (GLint	     size,
		  GLenum     type,
		  GLsizei    stride,
		  const void *pointer)
{
    fprintf (logFp, "glVertexPointer (%d, 0x%x, %d, %p)\n",
	     size, type, stride, pointer);
    (*nativeRenderTable->VertexPointer) (size, type, stride, pointer);
}

static void
logViewport (GLint   x,
	     GLint   y,
	     GLsizei width,
	     GLsizei height)
{
    fprintf (logFp, "glViewport (%d %d %d %d)\n", x, y, width, height);
    (*nativeRenderTable->Viewport) (x, y, width, height);
}

static void
logBlendColor (GLclampf red,
	       GLclampf green,
	       GLclampf blue,
	       GLclampf alpha)
{
    fprintf (logFp, "glBlendColor (%f, %f, %f, %f)\n",
	     red, green, blue, alpha);
    (*nativeRenderTable->BlendColor) (red, green, blue, alpha);
}

static void
logBlendEquation (GLenum mode)
{
    fprintf (logFp, "glBlendEquation (0x%x)\n", mode);
    (*nativeRenderTable->BlendEquation) (mode);
}

static void
logColorTable (GLenum       target,
	       GLenum       internalformat,
	       GLsizei      width,
	       GLenum       format,
	       GLenum	    type,
	       const GLvoid *table)
{
    fprintf (logFp, "glColorTable (0x%x, 0x%x, %d, 0x%x, 0x%x, %p)\n",
	     target, internalformat, width, format, type, table);
    (*nativeRenderTable->ColorTable) (target, internalformat, width,
				      format, type, table);
}

static void
logColorTableParameterfv (GLenum	target,
			  GLenum	pname,
			  const GLfloat *params)
{
    fprintf (logFp, "glColorTableParameterfv (0x%x, 0x%x, %p)\n",
	     target, pname, params);
    (*nativeRenderTable->ColorTableParameterfv) (target, pname, params);
}

static void
logColorTableParameteriv (GLenum      target,
			  GLenum      pname,
			  const GLint *params)
{
    fprintf (logFp, "glColorTableParameterfv (0x%x, 0x%x, %p)\n",
	     target, pname, params);
    (*nativeRenderTable->ColorTableParameteriv) (target, pname, params);
}

static void
logCopyColorTable (GLenum  target,
		   GLenum  internalformat,
		   GLint   x,
		   GLint   y,
		   GLsizei width)
{
    fprintf (logFp, "glCopyColorTable (0x%x, 0x%x, %d, %d, %d)\n",
	     target, internalformat, x, y, width);
    (*nativeRenderTable->CopyColorTable) (target, internalformat,
					  x, y, width);
}

static void
logGetColorTable (GLenum target,
		  GLenum format,
		  GLenum type,
		  GLvoid *table)
{
    fprintf (logFp, "glGetColorTable (0x%x, 0x%x, 0x%x, %p)\n",
	     target, format, type, table);
    (*nativeRenderTable->GetColorTable) (target, format, type, table);
}

static void
logGetColorTableParameterfv (GLenum  target,
			     GLenum  pname,
			     GLfloat *params)
{
    fprintf (logFp, "glGetColorTableParameterfv (0x%x, 0x%x, %p)\n",
	     target, pname, params);
    (*nativeRenderTable->GetColorTableParameterfv) (target, pname, params);
}

static void
logGetColorTableParameteriv (GLenum target,
			     GLenum pname,
			     GLint  *params)
{
    fprintf (logFp, "glGetColorTableParameteriv (0x%x, 0x%x, %p)\n",
	     target, pname, params);
    (*nativeRenderTable->GetColorTableParameteriv) (target, pname, params);
}

static void
logColorSubTable (GLenum       target,
		  GLsizei      start,
		  GLsizei      count,
		  GLenum       format,
		  GLenum       type,
		  const GLvoid *data)
{
    fprintf (logFp, "glColorSubTable (0x%x, %d, %d, 0x%x, 0x%x, %p)\n",
	     target, start, count, format, type, data);
    (*nativeRenderTable->ColorSubTable) (target, start, count,
					 format, type, data);
}

static void
logCopyColorSubTable (GLenum  target,
		      GLsizei start,
		      GLint   x,
		      GLint   y,
		      GLsizei width)
{
    fprintf (logFp, "glCopyColorSubTable (0x%x, %d, %d, %d, %d)\n",
	     target, start, x, y, width);
    (*nativeRenderTable->CopyColorSubTable) (target, start, x, y, width);
}

static void
logConvolutionFilter1D (GLenum	     target,
			GLenum	     internalformat,
			GLsizei	     width,
			GLenum	     format,
			GLenum	     type,
			const GLvoid *image)
{
    fprintf (logFp, "glConvolutionFilter1D (0x%x, 0x%x, %d, 0x%x, 0x%x, %p)\n",
	     target, internalformat, width, format, type, image);
    (*nativeRenderTable->ConvolutionFilter1D) (target, internalformat,
					       width, format, type, image);
}

static void
logConvolutionFilter2D (GLenum	     target,
			GLenum	     internalformat,
			GLsizei	     width,
			GLsizei	     height,
			GLenum	     format,
			GLenum	     type,
			const GLvoid *image)
{
    fprintf (logFp, "glConvolutionFilter2D (0x%x, 0x%x, %d, %d, "
	     "0x%x, 0x%x, %p)\n", target, internalformat, width, height,
	     format, type, image);
    (*nativeRenderTable->ConvolutionFilter2D) (target, internalformat,
					       width, height, format,
					       type, image);
}

static void
logConvolutionParameterf (GLenum  target,
			  GLenum  pname,
			  GLfloat param)
{
    fprintf (logFp, "glConvolutionParameterf (0x%x, 0x%x, %f)\n",
	     target, pname, param);
    (*nativeRenderTable->ConvolutionParameterf) (target, pname, param);
}

static void
logConvolutionParameterfv (GLenum	 target,
			   GLenum	 pname,
			   const GLfloat *params)
{
    fprintf (logFp, "glConvolutionParameterfv (0x%x, 0x%x, %p)\n",
	     target, pname, params);
    (*nativeRenderTable->ConvolutionParameterfv) (target, pname, params);
}

static void
logConvolutionParameteri (GLenum target,
			  GLenum pname,
			  GLint  param)
{
    fprintf (logFp, "glConvolutionParameterf (0x%x, 0x%x, %d)\n",
	     target, pname, param);
    (*nativeRenderTable->ConvolutionParameteri) (target, pname, param);
}

static void
logConvolutionParameteriv (GLenum      target,
			   GLenum      pname,
			   const GLint *params)
{
    fprintf (logFp, "glConvolutionParameteriv (0x%x, 0x%x, %p)\n",
	     target, pname, params);
    (*nativeRenderTable->ConvolutionParameteriv) (target, pname, params);
}

static void
logCopyConvolutionFilter1D (GLenum  target,
			    GLenum  internalformat,
			    GLint   x,
			    GLint   y,
			    GLsizei width)
{
    fprintf (logFp, "glCopyConvolutionFilter1D (0x%x, 0x%x, %d, %d, %d)\n",
	     target, internalformat, x, y, width);
    (*nativeRenderTable->CopyConvolutionFilter1D) (target, internalformat,
						   x, y, width);
}

static void
logCopyConvolutionFilter2D (GLenum  target,
			    GLenum  internalformat,
			    GLint   x,
			    GLint   y,
			    GLsizei width,
			    GLsizei height)
{
    fprintf (logFp, "glCopyConvolutionFilter2D (0x%x, 0x%x, %d, %d, %d, %d)\n",
	     target, internalformat, x, y, width, height);
    (*nativeRenderTable->CopyConvolutionFilter2D) (target, internalformat,
						   x, y, width, height);
}

static void
logGetConvolutionFilter (GLenum target,
			 GLenum format,
			 GLenum type,
			 GLvoid *image)
{
    fprintf (logFp, "glGetConvolutionFilter (0x%x, 0x%x, 0x%x, %p)\n",
	     target, format, type, image);
    (*nativeRenderTable->GetConvolutionFilter) (target, format, type,
						image);
}

static void
logGetConvolutionParameterfv (GLenum  target,
			      GLenum  pname,
			      GLfloat *params)
{
    fprintf (logFp, "glGetConvolutionParameterfv (0x%x, 0x%x, %p)\n",
	     target, pname, params);
    (*nativeRenderTable->GetConvolutionParameterfv) (target, pname,
						     params);
}

static void
logGetConvolutionParameteriv (GLenum target,
			      GLenum pname,
			      GLint  *params)
{
    fprintf (logFp, "glGetConvolutionParameteriv (0x%x, 0x%x, %p)\n",
	     target, pname, params);
    (*nativeRenderTable->GetConvolutionParameteriv) (target, pname,
						     params);
}

static void
logGetSeparableFilter (GLenum target,
		       GLenum format,
		       GLenum type,
		       GLvoid *row,
		       GLvoid *column,
		       GLvoid *span)
{
    fprintf (logFp, "glGetSeparableFilter (0x%x, 0x%x, 0x%x, %p, %p, %p)\n",
	     target, format, type, row, column, span);
    (*nativeRenderTable->GetSeparableFilter) (target, format, type,
					      row, column, span);
}

static void
logSeparableFilter2D (GLenum	   target,
		      GLenum	   internalformat,
		      GLsizei	   width,
		      GLsizei	   height,
		      GLenum	   format,
		      GLenum	   type,
		      const GLvoid *row,
		      const GLvoid *column)
{
    fprintf (logFp, "glSeparableFilter2D (0x%x, 0x%x, %d, %d, "
	     "0x%x, 0x%x, %p, %p)\n", target, internalformat, width, height,
	     format, type, row, column);
    (*nativeRenderTable->SeparableFilter2D) (target, internalformat,
					     width, height, format,
					     type, row, column);
}

static void
logGetHistogram (GLenum	   target,
		 GLboolean reset,
		 GLenum	   format,
		 GLenum	   type,
		 GLvoid	   *values)
{
    fprintf (logFp, "glGetHistogram (0x%x, %d, 0x%x, 0x%x, %p)\n",
	     target, reset, format, type, values);
    (*nativeRenderTable->GetHistogram) (target, reset, format, type,
					values);
}

static void
logGetHistogramParameterfv (GLenum  target,
			    GLenum  pname,
			    GLfloat *params)
{
    fprintf (logFp, "glGetHistogramParameterfv (0x%x, 0x%x, %p)\n",
	     target, pname, params);
    (*nativeRenderTable->GetHistogramParameterfv) (target, pname, params);
}

static void
logGetHistogramParameteriv (GLenum target,
			    GLenum pname,
			    GLint  *params)
{
    fprintf (logFp, "glGetHistogramParameteriv (0x%x, 0x%x, %p)\n",
	     target, pname, params);
    (*nativeRenderTable->GetHistogramParameteriv) (target, pname, params);
}

static void
logGetMinmax (GLenum	target,
	      GLboolean reset,
	      GLenum	format,
	      GLenum	type,
	      GLvoid	*values)
{
    fprintf (logFp, "glGetMinmax (0x%x, %d, 0x%x, 0x%x, %p)\n",
	     target, reset, format, type, values);
    (*nativeRenderTable->GetMinmax) (target, reset, format, type, values);
}

static void
logGetMinmaxParameterfv (GLenum  target,
			 GLenum  pname,
			 GLfloat *params)
{
    fprintf (logFp, "GetMinmaxParameterfv (0x%x, 0x%x, %p)\n",
	     target, pname, params);
    (*nativeRenderTable->GetMinmaxParameterfv) (target, pname, params);
}

static void
logGetMinmaxParameteriv (GLenum target,
			 GLenum pname,
			 GLint  *params)
{
    fprintf (logFp, "GetMinmaxParameteriv (0x%x, 0x%x, %p)\n",
	     target, pname, params);
    (*nativeRenderTable->GetMinmaxParameteriv) (target, pname, params);
}

static void
logHistogram (GLenum	target,
	      GLsizei	width,
	      GLenum	internalformat,
	      GLboolean sink)
{
    fprintf (logFp, "glHistogram (0x%x, %d, 0x%x, %d)\n",
	     target, width, internalformat, sink);
    (*nativeRenderTable->Histogram) (target, width, internalformat, sink);
}

static void
logMinmax (GLenum    target,
	   GLenum    internalformat,
	   GLboolean sink)
{
    fprintf (logFp, "glMinmax (0x%x, 0x%x, %d)\n",
	     target, internalformat, sink);
    (*nativeRenderTable->Minmax) (target, internalformat, sink);
}

static void
logResetHistogram (GLenum target)
{
    fprintf (logFp, "glResetHistogram (0x%x)\n", target);
    (*nativeRenderTable->ResetHistogram) (target);
}

static void
logResetMinmax (GLenum target)
{
    fprintf (logFp, "glResetMinmax (0x%x)\n", target);
    (*nativeRenderTable->ResetMinmax) (target);
}

static void
logCopyTexSubImage3D (GLenum  target,
		      GLint   level,
		      GLint   xoffset,
		      GLint   yoffset,
		      GLint   zoffset,
		      GLint   x,
		      GLint   y,
		      GLsizei width,
		      GLsizei height)
{
    fprintf (logFp, "glCopyTexSubImage3D (0x%x, %d, %d, %d, %d, %d, %d, "
	     "%d, %d)\n", target, level, xoffset, yoffset, zoffset,
	     x, y, width, height);
    (*nativeRenderTable->CopyTexSubImage3D) (target, level,
					     xoffset, yoffset, zoffset,
					     x, y, width, height);
}

static void
logTexImage3D (GLenum	    target,
	       GLint	    level,
	       GLint	    internalformat,
	       GLsizei	    width,
	       GLsizei	    height,
	       GLsizei	    depth,
	       GLint	    border,
	       GLenum	    format,
	       GLenum	    type,
	       const GLvoid *pixels)
{
    fprintf (logFp, "glTexImage3D (0x%x, %d, %d, %d, %d, %d, %d, "
	     "0x%x, 0x%x, %p)\n", target, level, internalformat,
	     width, height, depth, border, format, type, pixels);
    (*nativeRenderTable->TexImage3D) (target, level, internalformat,
				      width, height, depth, border,
				      format, type, pixels);
}

static void
logTexSubImage3D (GLenum       target,
		  GLint	       level,
		  GLint	       xoffset,
		  GLint	       yoffset,
		  GLint	       zoffset,
		  GLsizei      width,
		  GLsizei      height,
		  GLsizei      depth,
		  GLenum       format,
		  GLenum       type,
		  const GLvoid *pixels)
{
    fprintf (logFp, "glTexSubImage3D (0x%x, %d, %d, %d, %d, %d, %d, %d, "
	     "0x%x, 0x%x, %p)\n", target, level, xoffset, yoffset, zoffset,
	     width, height, depth, format, type, pixels);
    (*nativeRenderTable->TexSubImage3D) (target, level,
					 xoffset, yoffset, zoffset,
					 width, height, depth,
					 format, type, pixels);
}

/* GL_ARB_multitexture */

static void
logActiveTextureARB (GLenum texture)
{
    fprintf (logFp, "glActiveTextureARB (0x%x)\n", texture);
    (*nativeRenderTable->ActiveTextureARB) (texture);
}

static void
logClientActiveTextureARB (GLenum texture)
{
    fprintf (logFp, "glClientActiveTextureARB (0x%x)\n", texture);
    (*nativeRenderTable->ClientActiveTextureARB) (texture);
}

static void
logMultiTexCoord1dvARB (GLenum	       target,
			const GLdouble *v)
{
    vCnt[multiTexCoord1dvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glMultiTexCoord1dvARB (0x%x, %p)\n", target, v);
    (*nativeRenderTable->MultiTexCoord1dvARB) (target, v);
}

static void
logMultiTexCoord1fvARB (GLenum	      target,
			const GLfloat *v)
{
    vCnt[multiTexCoord1fvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glMultiTexCoord1fvARB (0x%x, %p)\n", target, v);
    (*nativeRenderTable->MultiTexCoord1fvARB) (target, v);
}

static void
logMultiTexCoord1ivARB (GLenum	    target,
			const GLint *v)
{
    vCnt[multiTexCoord1ivIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glMultiTexCoord1ivARB (0x%x, %p)\n", target, v);
    (*nativeRenderTable->MultiTexCoord1ivARB) (target, v);
}

static void
logMultiTexCoord1svARB (GLenum	      target,
			const GLshort *v)
{
    vCnt[multiTexCoord1svIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glMultiTexCoord1svARB (0x%x, %p)\n", target, v);
    (*nativeRenderTable->MultiTexCoord1svARB) (target, v);
}

static void
logMultiTexCoord2dvARB (GLenum	       target,
			const GLdouble *v)
{
    vCnt[multiTexCoord2dvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glMultiTexCoord2dvARB (0x%x, %p)\n", target, v);
    (*nativeRenderTable->MultiTexCoord2dvARB) (target, v);
}

static void
logMultiTexCoord2fvARB (GLenum	      target,
			const GLfloat *v)
{
    vCnt[multiTexCoord2fvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glMultiTexCoord2fvARB (0x%x, %p)\n", target, v);
    (*nativeRenderTable->MultiTexCoord2fvARB) (target, v);
}

static void
logMultiTexCoord2ivARB (GLenum	    target,
			const GLint *v)
{
    vCnt[multiTexCoord2ivIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glMultiTexCoord2ivARB (0x%x, %p)\n", target, v);
    (*nativeRenderTable->MultiTexCoord2ivARB) (target, v);
}

static void
logMultiTexCoord2svARB (GLenum	      target,
			const GLshort *v)
{
    vCnt[multiTexCoord2svIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glMultiTexCoord2svARB (0x%x, %p)\n", target, v);
    (*nativeRenderTable->MultiTexCoord2svARB) (target, v);
}

static void
logMultiTexCoord3dvARB (GLenum	       target,
			const GLdouble *v)
{
    vCnt[multiTexCoord3dvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glMultiTexCoord3dvARB (0x%x, %p)\n", target, v);
    (*nativeRenderTable->MultiTexCoord3dvARB) (target, v);
}

static void
logMultiTexCoord3fvARB (GLenum	      target,
			const GLfloat *v)
{
    vCnt[multiTexCoord3fvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glMultiTexCoord3fvARB (0x%x, %p)\n", target, v);
    (*nativeRenderTable->MultiTexCoord3fvARB) (target, v);
}

static void
logMultiTexCoord3ivARB (GLenum	    target,
			const GLint *v)
{
    vCnt[multiTexCoord3ivIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glMultiTexCoord3ivARB (0x%x, %p)\n", target, v);
    (*nativeRenderTable->MultiTexCoord3ivARB) (target, v);
}

static void
logMultiTexCoord3svARB (GLenum	      target,
			const GLshort *v)
{
    vCnt[multiTexCoord3svIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glMultiTexCoord3svARB (0x%x, %p)\n", target, v);
    (*nativeRenderTable->MultiTexCoord3svARB) (target, v);
}

static void
logMultiTexCoord4dvARB (GLenum	       target,
			const GLdouble *v)
{
    vCnt[multiTexCoord4dvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glMultiTexCoord4dvARB (0x%x, %p)\n", target, v);
    (*nativeRenderTable->MultiTexCoord4dvARB) (target, v);
}

static void
logMultiTexCoord4fvARB (GLenum	      target,
			const GLfloat *v)
{
    vCnt[multiTexCoord4fvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glMultiTexCoord4fvARB (0x%x, %p)\n", target, v);
    (*nativeRenderTable->MultiTexCoord4fvARB) (target, v);
}

static void
logMultiTexCoord4ivARB (GLenum	    target,
			const GLint *v)
{
    vCnt[multiTexCoord4ivIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glMultiTexCoord4ivARB (0x%x, %p)\n", target, v);
    (*nativeRenderTable->MultiTexCoord4ivARB) (target, v);
}

static void
logMultiTexCoord4svARB (GLenum	      target,
			const GLshort *v)
{
    vCnt[multiTexCoord4svIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glMultiTexCoord4svARB (0x%x, %p)\n", target, v);
    (*nativeRenderTable->MultiTexCoord4svARB) (target, v);
}


/* GL_ARB_multisample */

static void
logSampleCoverageARB (GLclampf  value,
		      GLboolean invert)
{
    fprintf (logFp, "glSampleCoverageARB (%f, %d)\n", value, invert);
    (*nativeRenderTable->SampleCoverageARB) (value, invert);
}


/* GL_EXT_texture_object */

static GLboolean
logAreTexturesResidentEXT (GLsizei	n,
			   const GLuint *textures,
			   GLboolean	*residences)
{
    fprintf (logFp, "glAreTexturesResidentEXT (%d, %p, %p)\n",
	     n, textures, residences);
    return (*nativeRenderTable->AreTexturesResidentEXT) (n, textures,
							 residences);
}
static void
logGenTexturesEXT (GLsizei n,
		   GLuint  *textures)
{
    fprintf (logFp, "glGenTexturesEXT (%d, %p)\n", n, textures);
    (*nativeRenderTable->GenTexturesEXT) (n, textures);
}

static GLboolean
logIsTextureEXT (GLuint texture)
{
    fprintf (logFp, "glIsTextureEXT (%d)\n", texture);
    return (*nativeRenderTable->IsTextureEXT) (texture);
}


/* GL_SGIS_multisample */

static void
logSampleMaskSGIS (GLclampf  value,
		   GLboolean invert)
{
    fprintf (logFp, "glSampleMaskSGIS (%f, %d)\n", value, invert);
    (*nativeRenderTable->SampleMaskSGIS) (value, invert);
}

static void
logSamplePatternSGIS (GLenum pattern)
{
    fprintf (logFp, "glSamplePatternSGIS (0x%x)\n", pattern);
    (*nativeRenderTable->SamplePatternSGIS) (pattern);
}


/* GL_EXT_point_parameters */

static void
logPointParameterfEXT (GLenum  pname,
		       GLfloat param)
{
    fprintf (logFp, "glPointParameterfEXT (0x%x, %f)\n", pname, param);
    (*nativeRenderTable->PointParameterfEXT) (pname, param);
}

static void
logPointParameterfvEXT (GLenum	      pname,
			const GLfloat *params)
{
    fprintf (logFp, "glPointParameterfvEXT (0x%x, %p)\n", pname, params);
    (*nativeRenderTable->PointParameterfvEXT) (pname, params);
}


/* GL_MESA_window_pos */

static void
logWindowPos3fMESA (GLfloat x,
		    GLfloat y,
		    GLfloat z)
{
    fprintf (logFp, "glWindowPos3fMESA (%f, %f, %f)\n", x, y, z);
    (*nativeRenderTable->WindowPos3fMESA) (x, y, z);
}


/* GL_EXT_blend_func_separate */

static void
logBlendFuncSeparateEXT (GLenum sfactorRGB,
			 GLenum dfactorRGB,
			 GLenum sfactorAlpha,
			 GLenum dfactorAlpha)
{
    fprintf (logFp, "glBlendFuncSeparateEXT (0x%x, 0x%x, 0x%x, 0x%x)\n",
	     sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
    (*nativeRenderTable->BlendFuncSeparateEXT) (sfactorRGB,
						dfactorRGB,
						sfactorAlpha,
						dfactorAlpha);
}


/* GL_EXT_fog_coord */

static void
logFogCoordfvEXT (const GLfloat *coord)
{
    vCnt[fogCoordfvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glFogCoordfvEXT (%p)\n", coord);
    (*nativeRenderTable->FogCoordfvEXT) (coord);
}

static void
logFogCoorddvEXT (const GLdouble *coord)
{
    vCnt[fogCoorddvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glFogCoorddvEXT (%p)\n", coord);
    (*nativeRenderTable->FogCoorddvEXT) (coord);
}

static void
logFogCoordPointerEXT (GLenum	    type,
		       GLsizei	    stride,
		       const GLvoid *pointer)
{
    fprintf (logFp, "glFogCoordPointerEXT (0x%x, %d, %p)\n",
	     type, stride, pointer);
    (*nativeRenderTable->FogCoordPointerEXT) (type, stride, pointer);
}


/* GL_EXT_secondary_color */

static void
logSecondaryColor3bvEXT (const GLbyte *v)
{
    vCnt[secondaryColor3bvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glSecondaryColor3bvEXT (%p)\n", v);
    (*nativeRenderTable->SecondaryColor3bvEXT) (v);
}

static void
logSecondaryColor3dvEXT (const GLdouble *v)
{
    vCnt[secondaryColor3dvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glSecondaryColor3dvEXT (%p)\n", v);
    (*nativeRenderTable->SecondaryColor3dvEXT) (v);
}

static void
logSecondaryColor3fvEXT (const GLfloat *v)
{
    vCnt[secondaryColor3fvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glSecondaryColor3fvEXT (%p)\n", v);
    (*nativeRenderTable->SecondaryColor3fvEXT) (v);
}

static void
logSecondaryColor3ivEXT (const GLint *v)
{
    vCnt[secondaryColor3ivIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glSecondaryColor3ivEXT (%p)\n", v);
    (*nativeRenderTable->SecondaryColor3ivEXT) (v);
}

static void
logSecondaryColor3svEXT (const GLshort *v)
{
    vCnt[secondaryColor3svIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glSecondaryColor3svEXT (%p)\n", v);
    (*nativeRenderTable->SecondaryColor3svEXT) (v);
}

static void
logSecondaryColor3ubvEXT (const GLubyte *v)
{
    vCnt[secondaryColor3ubvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glSecondaryColor3ubvEXT (%p)\n", v);
    (*nativeRenderTable->SecondaryColor3ubvEXT) (v);
}

static void
logSecondaryColor3uivEXT (const GLuint *v)
{
    vCnt[secondaryColor3uivIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glSecondaryColor3uivEXT (%p)\n", v);
    (*nativeRenderTable->SecondaryColor3uivEXT) (v);
}

static void
logSecondaryColor3usvEXT (const GLushort *v)
{
    vCnt[secondaryColor3usvIndex].n++;
    if (logVertexAttribs)
	fprintf (logFp, "glSecondaryColor3usvEXT (%p)\n", v);
    (*nativeRenderTable->SecondaryColor3usvEXT) (v);
}

static void
logSecondaryColorPointerEXT (GLint	  size,
			     GLenum	  type,
			     GLsizei	  stride,
			     const GLvoid *pointer)
{
    fprintf (logFp, "glSecondaryColorPointerEXT (%d, 0x%x, %d, %p)\n",
	     size, type, stride, pointer);
    (*nativeRenderTable->SecondaryColorPointerEXT) (size, type,
						    stride, pointer);
}


/* GL_NV_point_sprite */

static void
logPointParameteriNV (GLenum pname,
		      GLint  param)
{
    fprintf (logFp, "glPointParameteriNV (0x%x, %d)\n", pname, param);
    (*nativeRenderTable->PointParameteriNV) (pname, param);
}

static void
logPointParameterivNV (GLenum	   pname,
		       const GLint *params)
{
    fprintf (logFp, "glPointParameterivNV (0x%x, %p)\n", pname, params);
    (*nativeRenderTable->PointParameterivNV) (pname, params);
}


/* GL_EXT_stencil_two_side */

static void
logActiveStencilFaceEXT (GLenum face)
{
    fprintf (logFp, "glActiveStencilFaceEXT (0x%x)\n", face);
    (*nativeRenderTable->ActiveStencilFaceEXT) (face);
}


/* GL_EXT_framebuffer_object */

static GLboolean
logIsRenderbufferEXT (GLuint renderbuffer)
{
    fprintf (logFp, "glIsRenderbufferEXT (%d)\n", renderbuffer);
    return (*nativeRenderTable->IsRenderbufferEXT) (renderbuffer);
}

static void
logBindRenderbufferEXT (GLenum target,
			GLuint renderbuffer)
{
    fprintf (logFp, "glBindRenderbufferEXT (0x%x, %d)\n",
	     target, renderbuffer);
    (*nativeRenderTable->BindRenderbufferEXT) (target, renderbuffer);
}

static void
logDeleteRenderbuffersEXT (GLsizei	n,
			   const GLuint *renderbuffers)
{
    fprintf (logFp, "glDeleteRenderbuffersEXT (%d, %p)\n", n, renderbuffers);
    (*nativeRenderTable->DeleteRenderbuffersEXT) (n, renderbuffers);
}

static void
logGenRenderbuffersEXT (GLsizei n,
			GLuint  *renderbuffers)
{
    fprintf (logFp, "glGenRenderbuffersEXT (%d, %p)\n", n, renderbuffers);
    (*nativeRenderTable->GenRenderbuffersEXT) (n, renderbuffers);
}

static void
logRenderbufferStorageEXT (GLenum  target,
			   GLenum  internalformat,
			   GLsizei width,
			   GLsizei height)
{
    fprintf (logFp, "glRenderbufferStorageEXT (0x%x, 0x%x, %d, %d)\n",
	     target, internalformat, width, height);
    (*nativeRenderTable->RenderbufferStorageEXT) (target,
						  internalformat,
						  width, height);
}

static void
logGetRenderbufferParameterivEXT (GLenum target,
				  GLenum pname,
				  GLint  *params)
{
    fprintf (logFp, "glGetRenderbufferParameterivEXT (0x%x, 0x%x, %p)\n",
	     target, pname, params);
    (*nativeRenderTable->GetRenderbufferParameterivEXT) (target,
							 pname,
							 params);
}

static GLboolean
logIsFramebufferEXT (GLuint framebuffer)
{
    fprintf (logFp, "glIsFramebufferEXT (%d)\n", framebuffer);
    return (*nativeRenderTable->IsFramebufferEXT) (framebuffer);
}

static void
logBindFramebufferEXT (GLenum target,
		       GLuint framebuffer)
{
    fprintf (logFp, "glBindFramebufferEXT (0x%x, %d)\n", target, framebuffer);
    (*nativeRenderTable->BindFramebufferEXT) (target, framebuffer);
}

static void
logDeleteFramebuffersEXT (GLsizei      n,
			  const GLuint *framebuffers)
{
    fprintf (logFp, "glDeleteFramebuffersEXT (%d, %p)\n", n, framebuffers);
    (*nativeRenderTable->DeleteFramebuffersEXT) (n, framebuffers);
}

static void
logGenFramebuffersEXT (GLsizei n,
		       GLuint  *framebuffers)
{
    fprintf (logFp, "glGenFramebuffersEXT (%d, %p)\n", n, framebuffers);
    (*nativeRenderTable->GenFramebuffersEXT) (n, framebuffers);
}

static GLenum
logCheckFramebufferStatusEXT (GLenum target)
{
    fprintf (logFp, "glCheckFramebufferStatusEXT (0x%x)\n", target);
    return (*nativeRenderTable->CheckFramebufferStatusEXT) (target);
}

static void
logFramebufferTexture1DEXT (GLenum target,
			    GLenum attachment,
			    GLenum textarget,
			    GLuint texture,
			    GLint  level)
{
    fprintf (logFp, "glFramebufferTexture1DEXT (0x%x, 0x%x, 0x%x, %d, %d)\n",
	     target, attachment, textarget, texture, level);
    (*nativeRenderTable->FramebufferTexture1DEXT) (target, attachment,
						   textarget, texture,
						   level);
}

static void
logFramebufferTexture2DEXT (GLenum target,
			    GLenum attachment,
			    GLenum textarget,
			    GLuint texture,
			    GLint  level)
{
    fprintf (logFp, "glFramebufferTexture2DEXT (0x%x, 0x%x, 0x%x, %d, %d)\n",
	     target, attachment, textarget, texture, level);
    (*nativeRenderTable->FramebufferTexture2DEXT) (target, attachment,
						   textarget, texture,
						   level);
}

static void
logFramebufferTexture3DEXT (GLenum target,
			    GLenum attachment,
			    GLenum textarget,
			    GLuint texture,
			    GLint  level,
			    GLint  zoffset)
{
    fprintf (logFp, "glFramebufferTexture3DEXT (0x%x, 0x%x, 0x%x, "
	     "%d, %d, %d)\n", target, attachment, textarget, texture,
	     level, zoffset);
    (*nativeRenderTable->FramebufferTexture3DEXT) (target, attachment,
						   textarget, texture,
						   level, zoffset);
}

static void
logFramebufferRenderbufferEXT (GLenum target,
			       GLenum attachment,
			       GLenum buffertarget,
			       GLuint renderbuffer)
{
    fprintf (logFp, "glFramebufferRenderbufferEXT (0x%x, 0x%x, 0x%x, %d)\n",
	     target, attachment, buffertarget, renderbuffer);
    (*nativeRenderTable->FramebufferRenderbufferEXT) (target,
						      attachment,
						      buffertarget,
						      renderbuffer);
}

static void
logGetFramebufferAttachmentParameterivEXT (GLenum target,
					   GLenum attach,
					   GLenum pname,
					   GLint  *params)
{
    fprintf (logFp, "glGetFramebufferAttachmentParameterivEXT (0x%x, "
	     "0x%x, 0x%x, %p)\n", target, attach, pname, params);
    (*nativeRenderTable->GetFramebufferAttachmentParameterivEXT) (target,
								  attach,
								  pname,
								  params);
}

static void
logGenerateMipmapEXT (GLenum target)
{
    fprintf (logFp, "glGenerateMipmapEXT (0x%x)\n", target);
    (*nativeRenderTable->GenerateMipmapEXT) (target);
}

static struct _glapi_table __logRenderTable = {
    logNewList,
    logEndList,
    logCallList,
    logCallLists,
    logDeleteLists,
    logGenLists,
    logListBase,
    logBegin,
    logBitmap,
    0, /* glColor3b */
    logColor3bv,
    0, /* glColor3d */
    logColor3dv,
    0, /* glColor3f */
    logColor3fv,
    0, /* glColor3i */
    logColor3iv,
    0, /* glColor3s */
    logColor3sv,
    0, /* glColor3ub */
    logColor3ubv,
    0, /* glColor3ui */
    logColor3uiv,
    0, /* glColor3us */
    logColor3usv,
    0, /* glColor4b */
    logColor4bv,
    0, /* glColor4d */
    logColor4dv,
    0, /* glColor4f */
    logColor4fv,
    0, /* glColor4i */
    logColor4iv,
    0, /* glColor4s */
    logColor4sv,
    0, /* glColor4ub */
    logColor4ubv,
    0, /* glColor4ui */
    logColor4uiv,
    0, /* glColor4us */
    logColor4usv,
    0, /* glEdgeFlag */
    logEdgeFlagv,
    logEnd,
    0, /* glIndexd */
    logIndexdv,
    0, /* glIndexf */
    logIndexfv,
    0, /* glIndexi */
    logIndexiv,
    0, /* glIndexs */
    logIndexsv,
    0, /* glNormal3b */
    logNormal3bv,
    0, /* glNormal3d */
    logNormal3dv,
    0, /* glNormal3f */
    logNormal3fv,
    0, /* glNormal3i */
    logNormal3iv,
    0, /* glNormal3s */
    logNormal3sv,
    0, /* glRasterPos2d */
    logRasterPos2dv,
    0, /* glRasterPos2f */
    logRasterPos2fv,
    0, /* glRasterPos2i */
    logRasterPos2iv,
    0, /* glRasterPos2s */
    logRasterPos2sv,
    0, /* glRasterPos3d */
    logRasterPos3dv,
    0, /* glRasterPos3f */
    logRasterPos3fv,
    0, /* glRasterPos3i */
    logRasterPos3iv,
    0, /* glRasterPos3s */
    logRasterPos3sv,
    0, /* glRasterPos4d */
    logRasterPos4dv,
    0, /* glRasterPos4f */
    logRasterPos4fv,
    0, /* glRasterPos4i */
    logRasterPos4iv,
    0, /* glRasterPos4s */
    logRasterPos4sv,
    0, /* glRectd */
    logRectdv,
    0, /* glRectf */
    logRectfv,
    0, /* glRecti */
    logRectiv,
    0, /* glRects */
    logRectsv,
    0, /* glTexCoord1d */
    logTexCoord1dv,
    0, /* glTexCoord1f */
    logTexCoord1fv,
    0, /* glTexCoord1i */
    logTexCoord1iv,
    0, /* glTexCoord1s */
    logTexCoord1sv,
    0, /* glTexCoord2d */
    logTexCoord2dv,
    0, /* glTexCoord2f */
    logTexCoord2fv,
    0, /* glTexCoord2i */
    logTexCoord2iv,
    0, /* glTexCoord2s */
    logTexCoord2sv,
    0, /* glTexCoord3d */
    logTexCoord3dv,
    0, /* glTexCoord3f */
    logTexCoord3fv,
    0, /* glTexCoord3i */
    logTexCoord3iv,
    0, /* glTexCoord3s */
    logTexCoord3sv,
    0, /* glTexCoord4d */
    logTexCoord4dv,
    0, /* glTexCoord4f */
    logTexCoord4fv,
    0, /* glTexCoord4i */
    logTexCoord4iv,
    0, /* glTexCoord4s */
    logTexCoord4sv,
    0, /* glVertex2d */
    logVertex2dv,
    0, /* glVertex2f */
    logVertex2fv,
    0, /* glVertex2i */
    logVertex2iv,
    0, /* glVertex2s */
    logVertex2sv,
    0, /* glVertex3d */
    logVertex3dv,
    0, /* glVertex3f */
    logVertex3fv,
    0, /* glVertex3i */
    logVertex3iv,
    0, /* glVertex3s */
    logVertex3sv,
    0, /* glVertex4d */
    logVertex4dv,
    0, /* glVertex4f */
    logVertex4fv,
    0, /* glVertex4i */
    logVertex4iv,
    0, /* glVertex4s */
    logVertex4sv,
    logClipPlane,
    logColorMaterial,
    logCullFace,
    logFogf,
    logFogfv,
    logFogi,
    logFogiv,
    logFrontFace,
    logHint,
    logLightf,
    logLightfv,
    logLighti,
    logLightiv,
    logLightModelf,
    logLightModelfv,
    logLightModeli,
    logLightModeliv,
    logLineStipple,
    logLineWidth,
    logMaterialf,
    logMaterialfv,
    logMateriali,
    logMaterialiv,
    logPointSize,
    logPolygonMode,
    logPolygonStipple,
    logScissor,
    logShadeModel,
    logTexParameterf,
    logTexParameterfv,
    logTexParameteri,
    logTexParameteriv,
    logTexImage1D,
    logTexImage2D,
    logTexEnvf,
    logTexEnvfv,
    logTexEnvi,
    logTexEnviv,
    logTexGend,
    logTexGendv,
    logTexGenf,
    logTexGenfv,
    logTexGeni,
    logTexGeniv,
    logFeedbackBuffer,
    logSelectBuffer,
    logRenderMode,
    logInitNames,
    logLoadName,
    logPassThrough,
    logPopName,
    logPushName,
    logDrawBuffer,
    logClear,
    logClearAccum,
    logClearIndex,
    logClearColor,
    logClearStencil,
    logClearDepth,
    logStencilMask,
    logColorMask,
    logDepthMask,
    logIndexMask,
    logAccum,
    logDisable,
    logEnable,
    logFinish,
    logFlush,
    logPopAttrib,
    logPushAttrib,
    logMap1d,
    logMap1f,
    logMap2d,
    logMap2f,
    logMapGrid1d,
    logMapGrid1f,
    logMapGrid2d,
    logMapGrid2f,
    0, /* glEvalCoord1d */
    logEvalCoord1dv,
    0, /* glEvalCoord1f */
    logEvalCoord1fv,
    0, /* glEvalCoord2d */
    logEvalCoord2dv,
    0, /* glEvalCoord2f */
    logEvalCoord2fv,
    logEvalMesh1,
    logEvalPoint1,
    logEvalMesh2,
    logEvalPoint2,
    logAlphaFunc,
    logBlendFunc,
    logLogicOp,
    logStencilFunc,
    logStencilOp,
    logDepthFunc,
    logPixelZoom,
    logPixelTransferf,
    logPixelTransferi,
    logPixelStoref,
    logPixelStorei,
    logPixelMapfv,
    logPixelMapuiv,
    logPixelMapusv,
    logReadBuffer,
    logCopyPixels,
    logReadPixels,
    logDrawPixels,
    logGetBooleanv,
    logGetClipPlane,
    logGetDoublev,
    logGetError,
    logGetFloatv,
    logGetIntegerv,
    logGetLightfv,
    logGetLightiv,
    logGetMapdv,
    logGetMapfv,
    logGetMapiv,
    logGetMaterialfv,
    logGetMaterialiv,
    logGetPixelMapfv,
    logGetPixelMapuiv,
    logGetPixelMapusv,
    logGetPolygonStipple,
    logGetString,
    logGetTexEnvfv,
    logGetTexEnviv,
    logGetTexGendv,
    logGetTexGenfv,
    logGetTexGeniv,
    logGetTexImage,
    logGetTexParameterfv,
    logGetTexParameteriv,
    logGetTexLevelParameterfv,
    logGetTexLevelParameteriv,
    logIsEnabled,
    logIsList,
    logDepthRange,
    logFrustum,
    logLoadIdentity,
    logLoadMatrixf,
    logLoadMatrixd,
    logMatrixMode,
    logMultMatrixf,
    logMultMatrixd,
    logOrtho,
    logPopMatrix,
    logPushMatrix,
    logRotated,
    logRotatef,
    logScaled,
    logScalef,
    logTranslated,
    logTranslatef,
    logViewport,
    logArrayElement,
    logBindTexture,
    logColorPointer,
    logDisableClientState,
    logDrawArrays,
    logDrawElements,
    logEdgeFlagPointer,
    logEnableClientState,
    logIndexPointer,
    0, /* glIndexub */
    logIndexubv,
    logInterleavedArrays,
    logNormalPointer,
    logPolygonOffset,
    logTexCoordPointer,
    logVertexPointer,
    logAreTexturesResident,
    logCopyTexImage1D,
    logCopyTexImage2D,
    logCopyTexSubImage1D,
    logCopyTexSubImage2D,
    logDeleteTextures,
    logGenTextures,
    logGetPointerv,
    logIsTexture,
    logPrioritizeTextures,
    logTexSubImage1D,
    logTexSubImage2D,
    logPopClientAttrib,
    logPushClientAttrib,
    logBlendColor,
    logBlendEquation,
    0, /* glDrawRangeElements */
    logColorTable,
    logColorTableParameterfv,
    logColorTableParameteriv,
    logCopyColorTable,
    logGetColorTable,
    logGetColorTableParameterfv,
    logGetColorTableParameteriv,
    logColorSubTable,
    logCopyColorSubTable,
    logConvolutionFilter1D,
    logConvolutionFilter2D,
    logConvolutionParameterf,
    logConvolutionParameterfv,
    logConvolutionParameteri,
    logConvolutionParameteriv,
    logCopyConvolutionFilter1D,
    logCopyConvolutionFilter2D,
    logGetConvolutionFilter,
    logGetConvolutionParameterfv,
    logGetConvolutionParameteriv,
    logGetSeparableFilter,
    logSeparableFilter2D,
    logGetHistogram,
    logGetHistogramParameterfv,
    logGetHistogramParameteriv,
    logGetMinmax,
    logGetMinmaxParameterfv,
    logGetMinmaxParameteriv,
    logHistogram,
    logMinmax,
    logResetHistogram,
    logResetMinmax,
    logTexImage3D,
    logTexSubImage3D,
    logCopyTexSubImage3D,
    logActiveTextureARB,
    logClientActiveTextureARB,
    0, /* glMultiTexCoord1dARB */
    logMultiTexCoord1dvARB,
    0, /* glMultiTexCoord1fARB */
    logMultiTexCoord1fvARB,
    0, /* glMultiTexCoord1iARB */
    logMultiTexCoord1ivARB,
    0, /* glMultiTexCoord1sARB */
    logMultiTexCoord1svARB,
    0, /* glMultiTexCoord2dARB */
    logMultiTexCoord2dvARB,
    0, /* glMultiTexCoord2fARB */
    logMultiTexCoord2fvARB,
    0, /* glMultiTexCoord2iARB */
    logMultiTexCoord2ivARB,
    0, /* glMultiTexCoord2sARB */
    logMultiTexCoord2svARB,
    0, /* glMultiTexCoord3dARB */
    logMultiTexCoord3dvARB,
    0, /* glMultiTexCoord3fARB */
    logMultiTexCoord3fvARB,
    0, /* glMultiTexCoord3iARB */
    logMultiTexCoord3ivARB,
    0, /* glMultiTexCoord3sARB */
    logMultiTexCoord3svARB,
    0, /* glMultiTexCoord4dARB */
    logMultiTexCoord4dvARB,
    0, /* glMultiTexCoord4fARB */
    logMultiTexCoord4fvARB,
    0, /* glMultiTexCoord4iARB */
    logMultiTexCoord4ivARB,
    0, /* glMultiTexCoord4sARB */
    logMultiTexCoord4svARB,
    0, /* glLoadTransposeMatrixfARB */
    0, /* glLoadTransposeMatrixdARB */
    0, /* glMultTransposeMatrixfARB */
    0, /* glMultTransposeMatrixdARB */
    logSampleCoverageARB,
    0, /* glDrawBuffersARB */
    0, /* glPolygonOffsetEXT */
    0, /* glGetTexFilterFuncSGIS */
    0, /* glTexFilterFuncSGIS */
    0, /* glGetHistogramEXT */
    0, /* glGetHistogramParameterfvEXT */
    0, /* glGetHistogramParameterivEXT */
    0, /* glGetMinmaxEXT */
    0, /* glGetMinmaxParameterfvEXT */
    0, /* glGetMinmaxParameterivEXT */
    0, /* glGetConvolutionFilterEXT */
    0, /* glGetConvolutionParameterfvEXT */
    0, /* glGetConvolutionParameterivEXT */
    0, /* glGetSeparableFilterEXT */
    0, /* glGetColorTableSGI */
    0, /* glGetColorTableParameterfvSGI */
    0, /* glGetColorTableParameterivSGI */
    0, /* glPixelTexGenSGIX */
    0, /* glPixelTexGenParameteriSGIS */
    0, /* glPixelTexGenParameterivSGIS */
    0, /* glPixelTexGenParameterfSGIS */
    0, /* glPixelTexGenParameterfvSGIS */
    0, /* glGetPixelTexGenParameterivSGIS */
    0, /* glGetPixelTexGenParameterfvSGIS */
    0, /* glTexImage4DSGIS */
    0, /* glTexSubImage4DSGIS */
    logAreTexturesResidentEXT,
    logGenTexturesEXT,
    logIsTextureEXT,
    0, /* glDetailTexFuncSGIS */
    0, /* glGetDetailTexFuncSGIS */
    0, /* glSharpenTexFuncSGIS */
    0, /* glGetSharpenTexFuncSGIS */
    logSampleMaskSGIS,
    logSamplePatternSGIS,
    0, /* glColorPointerEXT */
    0, /* glEdgeFlagPointerEXT */
    0, /* glIndexPointerEXT */
    0, /* glNormalPointerEXT */
    0, /* glTexCoordPointerEXT */
    0, /* glVertexPointerEXT */
    0, /* glSpriteParameterfSGIX */
    0, /* glSpriteParameterfvSGIX */
    0, /* glSpriteParameteriSGIX */
    0, /* glSpriteParameterivSGIX */
    logPointParameterfEXT,
    logPointParameterfvEXT,
    0, /* glGetInstrumentsSGIX */
    0, /* glInstrumentsBufferSGIX */
    0, /* glPollInstrumentsSGIX */
    0, /* glReadInstrumentsSGIX */
    0, /* glStartInstrumentsSGIX */
    0, /* glStopInstrumentsSGIX */
    0, /* glFrameZoomSGIX */
    0, /* glTagSampleBufferSGIX */
    0, /* glReferencePlaneSGIX */
    0, /* glFlushRasterSGIX */
    0, /* glGetListParameterfvSGIX */
    0, /* glGetListParameterivSGIX */
    0, /* glListParameterfSGIX */
    0, /* glListParameterfvSGIX */
    0, /* glListParameteriSGIX */
    0, /* glListParameterivSGIX */
    0, /* glFragmentColorMaterialSGIX */
    0, /* glFragmentLightfSGIX */
    0, /* glFragmentLightfvSGIX */
    0, /* glFragmentLightiSGIX */
    0, /* glFragmentLightivSGIX */
    0, /* glFragmentLightModelfSGIX */
    0, /* glFragmentLightModelfvSGIX */
    0, /* glFragmentLightModeliSGIX */
    0, /* glFragmentLightModelivSGIX */
    0, /* glFragmentMaterialfSGIX */
    0, /* glFragmentMaterialfvSGIX */
    0, /* glFragmentMaterialiSGIX */
    0, /* glFragmentMaterialivSGIX */
    0, /* glGetFragmentLightfvSGIX */
    0, /* glGetFragmentLightivSGIX */
    0, /* glGetFragmentMaterialfvSGIX */
    0, /* glGetFragmentMaterialivSGIX */
    0, /* glLightEnviSGIX */
    0, /* glVertexWeightfEXT */
    0, /* glVertexWeightfvEXT */
    0, /* glVertexWeightPointerEXT */
    0, /* glFlushVertexArrayRangeNV */
    0, /* glVertexArrayRangeNV */
    0, /* glCombinerParameterfvNV */
    0, /* glCombinerParameterfNV */
    0, /* glCombinerParameterivNV */
    0, /* glCombinerParameteriNV */
    0, /* glCombinerInputNV */
    0, /* glCombinerOutputNV */
    0, /* glFinalCombinerInputNV */
    0, /* glGetCombinerInputParameterfvNV */
    0, /* glGetCombinerInputParameterivNV */
    0, /* glGetCombinerOutputParameterfvNV */
    0, /* glGetCombinerOutputParameterivNV */
    0, /* glGetFinalCombinerInputParameterfvNV */
    0, /* glGetFinalCombinerInputParameterivNV */
    0, /* glResizeBuffersMESA */
    0, /* glWindowPos2dMESA */
    0, /* glWindowPos2dvMESA */
    0, /* glWindowPos2fMESA */
    0, /* glWindowPos2fvMESA */
    0, /* glWindowPos2iMESA */
    0, /* glWindowPos2ivMESA */
    0, /* glWindowPos2sMESA */
    0, /* glWindowPos2svMESA */
    0, /* glWindowPos3dMESA */
    0, /* glWindowPos3dvMESA */
    logWindowPos3fMESA,
    0, /* glWindowPos3fvMESA */
    0, /* glWindowPos3iMESA */
    0, /* glWindowPos3ivMESA */
    0, /* glWindowPos3sMESA */
    0, /* glWindowPos3svMESA */
    0, /* glWindowPos4dMESA */
    0, /* glWindowPos4dvMESA */
    0, /* glWindowPos4fMESA */
    0, /* glWindowPos4fvMESA */
    0, /* glWindowPos4iMESA */
    0, /* glWindowPos4ivMESA */
    0, /* glWindowPos4sMESA */
    0, /* glWindowPos4svMESA */
    logBlendFuncSeparateEXT,
    0, /* glIndexMaterialEXT */
    0, /* glIndexFuncEXT */
    0, /* glLockArraysEXT */
    0, /* glUnlockArraysEXT */
    0, /* glCullParameterdvEXT */
    0, /* glCullParameterfvEXT */
    0, /* glHintPGI */
    0, /* glFogCoordfEXT */
    logFogCoordfvEXT,
    0, /* glFogCoorddEXT */
    logFogCoorddvEXT,
    logFogCoordPointerEXT,
    0, /* glGetColorTableEXT */
    0, /* glGetColorTableParameterivEXT */
    0, /* glGetColorTableParameterfvEXT */
    0, /* glTbufferMask3DFX */
    0, /* glCompressedTexImage3DARB */
    0, /* glCompressedTexImage2DARB */
    0, /* glCompressedTexImage1DARB */
    0, /* glCompressedTexSubImage3DARB */
    0, /* glCompressedTexSubImage2DARB */
    0, /* glCompressedTexSubImage1DARB */
    0, /* glGetCompressedTexImageARB */
    0, /* glSecondaryColor3bEXT */
    logSecondaryColor3bvEXT,
    0, /* glSecondaryColor3dEXT */
    logSecondaryColor3dvEXT,
    0, /* glSecondaryColor3fEXT */
    logSecondaryColor3fvEXT,
    0, /* glSecondaryColor3iEXT */
    logSecondaryColor3ivEXT,
    0, /* glSecondaryColor3sEXT */
    logSecondaryColor3svEXT,
    0, /* glSecondaryColor3ubEXT */
    logSecondaryColor3ubvEXT,
    0, /* glSecondaryColor3uiEXT */
    logSecondaryColor3uivEXT,
    0, /* glSecondaryColor3usEXT */
    logSecondaryColor3usvEXT,
    logSecondaryColorPointerEXT,
    0, /* glAreProgramsResidentNV */
    0, /* glBindProgramNV */
    0, /* glDeleteProgramsNV */
    0, /* glExecuteProgramNV */
    0, /* glGenProgramsNV */
    0, /* glGetProgramParameterdvNV */
    0, /* glGetProgramParameterfvNV */
    0, /* glGetProgramivNV */
    0, /* glGetProgramStringNV */
    0, /* glGetTrackMatrixivNV */
    0, /* glGetVertexAttribdvARB */
    0, /* glGetVertexAttribfvARB */
    0, /* glGetVertexAttribivARB */
    0, /* glGetVertexAttribPointervNV */
    0, /* glIsProgramNV */
    0, /* glLoadProgramNV */
    0, /* glProgramParameter4dNV */
    0, /* glProgramParameter4dvNV */
    0, /* glProgramParameter4fNV */
    0, /* glProgramParameter4fvNV */
    0, /* glProgramParameters4dvNV */
    0, /* glProgramParameters4fvNV */
    0, /* glRequestResidentProgramsNV */
    0, /* glTrackMatrixNV */
    0, /* glVertexAttribPointerNV */
    0, /* glVertexAttrib1dARB */
    0, /* glVertexAttrib1dvARB */
    0, /* glVertexAttrib1fARB */
    0, /* glVertexAttrib1fvARB */
    0, /* glVertexAttrib1sARB */
    0, /* glVertexAttrib1svARB */
    0, /* glVertexAttrib2dARB */
    0, /* glVertexAttrib2dvARB */
    0, /* glVertexAttrib2fARB */
    0, /* glVertexAttrib2fvARB */
    0, /* glVertexAttrib2sARB */
    0, /* glVertexAttrib2svARB */
    0, /* glVertexAttrib3dARB */
    0, /* glVertexAttrib3dvARB */
    0, /* glVertexAttrib3fARB */
    0, /* glVertexAttrib3fvARB */
    0, /* glVertexAttrib3sARB */
    0, /* glVertexAttrib3svARB */
    0, /* glVertexAttrib4dARB */
    0, /* glVertexAttrib4dvARB */
    0, /* glVertexAttrib4fARB */
    0, /* glVertexAttrib4fvARB */
    0, /* glVertexAttrib4sARB */
    0, /* glVertexAttrib4svARB */
    0, /* glVertexAttrib4NubARB */
    0, /* glVertexAttrib4NubvARB */
    0, /* glVertexAttribs1dvNV */
    0, /* glVertexAttribs1fvNV */
    0, /* glVertexAttribs1svNV */
    0, /* glVertexAttribs2dvNV */
    0, /* glVertexAttribs2fvNV */
    0, /* glVertexAttribs2svNV */
    0, /* glVertexAttribs3dvNV */
    0, /* glVertexAttribs3fvNV */
    0, /* glVertexAttribs3svNV */
    0, /* glVertexAttribs4dvNV */
    0, /* glVertexAttribs4fvNV */
    0, /* glVertexAttribs4svNV */
    0, /* glVertexAttribs4ubvNV */
    logPointParameteriNV,
    logPointParameterivNV,
    0, /* glMultiDrawArraysEXT */
    0, /* glMultiDrawElementsEXT */
    logActiveStencilFaceEXT,
    0, /* glDeleteFencesNV */
    0, /* glGenFencesNV */
    0, /* glIsFenceNV */
    0, /* glTestFenceNV */
    0, /* glGetFenceivNV */
    0, /* glFinishFenceNV */
    0, /* glSetFenceNV */
    0, /* glVertexAttrib4bvARB */
    0, /* glVertexAttrib4ivARB */
    0, /* glVertexAttrib4ubvARB */
    0, /* glVertexAttrib4usvARB */
    0, /* glVertexAttrib4uivARB */
    0, /* glVertexAttrib4NbvARB */
    0, /* glVertexAttrib4NsvARB */
    0, /* glVertexAttrib4NivARB */
    0, /* glVertexAttrib4NusvARB */
    0, /* glVertexAttrib4NuivARB */
    0, /* glVertexAttribPointerARB */
    0, /* glEnableVertexAttribArrayARB */
    0, /* glDisableVertexAttribArrayARB */
    0, /* glProgramStringARB */
    0, /* glProgramEnvParameter4dARB */
    0, /* glProgramEnvParameter4dvARB */
    0, /* glProgramEnvParameter4fARB */
    0, /* glProgramEnvParameter4fvARB */
    0, /* glProgramLocalParameter4dARB */
    0, /* glProgramLocalParameter4dvARB */
    0, /* glProgramLocalParameter4fARB */
    0, /* glProgramLocalParameter4fvARB */
    0, /* glGetProgramEnvParameterdvARB */
    0, /* glGetProgramEnvParameterfvARB */
    0, /* glGetProgramLocalParameterdvARB */
    0, /* glGetProgramLocalParameterfvARB */
    0, /* glGetProgramivARB */
    0, /* glGetProgramStringARB */
    0, /* glProgramNamedParameter4fNV */
    0, /* glProgramNamedParameter4dNV */
    0, /* glProgramNamedParameter4fvNV */
    0, /* glProgramNamedParameter4dvNV */
    0, /* glGetProgramNamedParameterfvNV */
    0, /* glGetProgramNamedParameterdvNV */
    0, /* glBindBufferARB */
    0, /* glBufferDataARB */
    0, /* glBufferSubDataARB */
    0, /* glDeleteBuffersARB */
    0, /* glGenBuffersARB */
    0, /* glGetBufferParameterivARB */
    0, /* glGetBufferPointervARB */
    0, /* glGetBufferSubDataARB */
    0, /* glIsBufferARB */
    0, /* glMapBufferARB */
    0, /* glUnmapBufferARB */
    0, /* glDepthBoundsEXT */
    0, /* glGenQueriesARB */
    0, /* glDeleteQueriesARB */
    0, /* glIsQueryARB */
    0, /* glBeginQueryARB */
    0, /* glEndQueryARB */
    0, /* glGetQueryivARB */
    0, /* glGetQueryObjectivARB */
    0, /* glGetQueryObjectuivARB */
    0, /* glMultiModeDrawArraysIBM */
    0, /* glMultiModeDrawElementsIBM */
    0, /* glBlendEquationSeparateEXT */
    0, /* glDeleteObjectARB */
    0, /* glGetHandleARB */
    0, /* glDetachObjectARB */
    0, /* glCreateShaderObjectARB */
    0, /* glShaderSourceARB */
    0, /* glCompileShaderARB */
    0, /* glCreateProgramObjectARB */
    0, /* glAttachObjectARB */
    0, /* glLinkProgramARB */
    0, /* glUseProgramObjectARB */
    0, /* glValidateProgramARB */
    0, /* glUniform1fARB */
    0, /* glUniform2fARB */
    0, /* glUniform3fARB */
    0, /* glUniform4fARB */
    0, /* glUniform1iARB */
    0, /* glUniform2iARB */
    0, /* glUniform3iARB */
    0, /* glUniform4iARB */
    0, /* glUniform1fvARB */
    0, /* glUniform2fvARB */
    0, /* glUniform3fvARB */
    0, /* glUniform4fvARB */
    0, /* glUniform1ivARB */
    0, /* glUniform2ivARB */
    0, /* glUniform3ivARB */
    0, /* glUniform4ivARB */
    0, /* glUniformMatrix2fvARB */
    0, /* glUniformMatrix3fvARB */
    0, /* glUniformMatrix4fvARB */
    0, /* glGetObjectParameterfvARB */
    0, /* glGetObjectParameterivARB */
    0, /* glGetInfoLogARB */
    0, /* glGetAttachedObjectsARB */
    0, /* glGetUniformLocationARB */
    0, /* glGetActiveUniformARB */
    0, /* glGetUniformfvARB */
    0, /* glGetUniformivARB */
    0, /* glGetShaderSourceARB */
    0, /* glBindAttribLocationARB */
    0, /* glGetActiveAttribARB */
    0, /* glGetAttribLocationARB */
    0, /* glGetVertexAttribdvNV */
    0, /* glGetVertexAttribfvNV */
    0, /* glGetVertexAttribivNV */
    0, /* glVertexAttrib1dNV */
    0, /* glVertexAttrib1dvNV */
    0, /* glVertexAttrib1fNV */
    0, /* glVertexAttrib1fvNV */
    0, /* glVertexAttrib1sNV */
    0, /* glVertexAttrib1svNV */
    0, /* glVertexAttrib2dNV */
    0, /* glVertexAttrib2dvNV */
    0, /* glVertexAttrib2fNV */
    0, /* glVertexAttrib2fvNV */
    0, /* glVertexAttrib2sNV */
    0, /* glVertexAttrib2svNV */
    0, /* glVertexAttrib3dNV */
    0, /* glVertexAttrib3dvNV */
    0, /* glVertexAttrib3fNV */
    0, /* glVertexAttrib3fvNV */
    0, /* glVertexAttrib3sNV */
    0, /* glVertexAttrib3svNV */
    0, /* glVertexAttrib4dNV */
    0, /* glVertexAttrib4dvNV */
    0, /* glVertexAttrib4fNV */
    0, /* glVertexAttrib4fvNV */
    0, /* glVertexAttrib4sNV */
    0, /* glVertexAttrib4svNV */
    0, /* glVertexAttrib4ubNV */
    0, /* glVertexAttrib4ubvNV */
    0, /* glGenFragmentShadersATI */
    0, /* glBindFragmentShaderATI */
    0, /* glDeleteFragmentShaderATI */
    0, /* glBeginFragmentShaderATI */
    0, /* glEndFragmentShaderATI */
    0, /* glPassTexCoordATI */
    0, /* glSampleMapATI */
    0, /* glColorFragmentOp1ATI */
    0, /* glColorFragmentOp2ATI */
    0, /* glColorFragmentOp3ATI */
    0, /* glAlphaFragmentOp1ATI */
    0, /* glAlphaFragmentOp2ATI */
    0, /* glAlphaFragmentOp3ATI */
    0, /* glSetFragmentShaderConstantATI */
    logIsRenderbufferEXT,
    logBindRenderbufferEXT,
    logDeleteRenderbuffersEXT,
    logGenRenderbuffersEXT,
    logRenderbufferStorageEXT,
    logGetRenderbufferParameterivEXT,
    logIsFramebufferEXT,
    logBindFramebufferEXT,
    logDeleteFramebuffersEXT,
    logGenFramebuffersEXT,
    logCheckFramebufferStatusEXT,
    logFramebufferTexture1DEXT,
    logFramebufferTexture2DEXT,
    logFramebufferTexture3DEXT,
    logFramebufferRenderbufferEXT,
    logGetFramebufferAttachmentParameterivEXT,
    logGenerateMipmapEXT,
    0, /* glStencilFuncSeparate */
    0, /* glStencilOpSeparate */
    0, /* glStencilMaskSeparate */
    0, /* glGetQueryObjecti64vEXT */
    0  /* glGetQueryObjectui64vEXT */
};

static Bool isCurrent = FALSE;

static void (*flushContextCache) (void);
static void (*setRenderTables)   (struct _glapi_table *table);

static void
GlxLogFlushContextCache (void)
{
    if (isCurrent)
    {
	fprintf (logFp, "LOSE CURRENT\n");
	isCurrent = FALSE;
    }

    (*flushContextCache) ();
}

static void
GlxLogSetRenderTables (struct _glapi_table *table)
{
    nativeRenderTable = table;

    if (table)
    {
	fprintf (logFp, "FORCE CURRENT\n");
	isCurrent = TRUE;

	(*setRenderTables) (&__logRenderTable);
    }
    else
    {
	(*setRenderTables) (0);
    }
}

void
xglInitGlxLog (void)
{
    if (logFp)
	return;

    if (__xglGLXLogFp)
    {
	logFp = __xglGLXLogFp;

	flushContextCache = __xglGLXFunc.flushContextCache;
	setRenderTables   = __xglGLXFunc.setRenderTables;

	__xglGLXFunc.flushContextCache = GlxLogFlushContextCache;
	__xglGLXFunc.setRenderTables   = GlxLogSetRenderTables;
    }
}

#endif
