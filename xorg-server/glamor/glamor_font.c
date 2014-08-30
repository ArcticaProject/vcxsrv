/*
 * Copyright © 2014 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include "glamor_priv.h"
#include "glamor_font.h"
#include <dixfontstr.h>

static int glamor_font_generation;
static int glamor_font_private_index;
static int glamor_font_screen_count;

glamor_font_t *
glamor_font_get(ScreenPtr screen, FontPtr font)
{
    glamor_screen_private *glamor_priv = glamor_get_screen_private(screen);

    glamor_font_t       *privates;
    glamor_font_t       *glamor_font;
    int                 overall_width, overall_height;
    int                 num_rows;
    int                 num_cols;
    int                 glyph_width_pixels;
    int                 glyph_width_bytes;
    int                 glyph_height;
    int                 row, col;
    unsigned char       c[2];
    CharInfoPtr         glyph;
    unsigned long       count;

    if (glamor_priv->glsl_version < 130)
        return NULL;

    privates = FontGetPrivate(font, glamor_font_private_index);
    if (!privates) {
        privates = calloc(glamor_font_screen_count, sizeof (glamor_font_t));
        if (!privates)
            return NULL;
        FontSetPrivate(font, glamor_font_private_index, privates);
    }

    glamor_font = &privates[screen->myNum];

    if (glamor_font->realized)
        return glamor_font;

    glamor_font->realized = TRUE;

    /* Figure out how many glyphs are in the font */
    num_cols = font->info.lastCol - font->info.firstCol + 1;
    num_rows = font->info.lastRow - font->info.firstRow + 1;

    /* Figure out the size of each glyph */
    glyph_width_pixels = font->info.maxbounds.rightSideBearing - font->info.minbounds.leftSideBearing;
    glyph_height = font->info.maxbounds.ascent + font->info.maxbounds.descent;

    glyph_width_bytes = (glyph_width_pixels + 7) >> 3;

    glamor_font->glyph_width_pixels = glyph_width_pixels;
    glamor_font->glyph_width_bytes = glyph_width_bytes;
    glamor_font->glyph_height = glyph_height;

    overall_width = glyph_width_bytes * num_cols;
    overall_height = glyph_height * num_rows;

    /* Check whether the font has a default character */
    c[0] = font->info.lastRow + 1;
    c[1] = font->info.lastCol + 1;
    (*font->get_glyphs)(font, 1, c, TwoD16Bit, &count, &glyph);

    glamor_font->default_char = count ? glyph : NULL;
    glamor_font->default_row = font->info.defaultCh >> 8;
    glamor_font->default_col = font->info.defaultCh;

    glamor_priv = glamor_get_screen_private(screen);
    glamor_make_current(glamor_priv);

    glGenTextures(1, &glamor_font->texture_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, glamor_font->texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    /* Allocate storage */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, overall_width, overall_height,
                 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    /* Paint all of the glyphs */
    for (row = 0; row < num_rows; row++) {
        for (col = 0; col < num_cols; col++) {
            c[0] = row + font->info.firstRow;
            c[1] = col + font->info.firstCol;

            (*font->get_glyphs)(font, 1, c, TwoD16Bit, &count, &glyph);

            if (count)
                glTexSubImage2D(GL_TEXTURE_2D, 0, col * glyph_width_bytes, row * glyph_height,
                                GLYPHWIDTHBYTES(glyph), GLYPHHEIGHTPIXELS(glyph),
                                GL_RED_INTEGER, GL_UNSIGNED_BYTE, glyph->bits);
        }
    }

    return glamor_font;
}

static Bool
glamor_realize_font(ScreenPtr screen, FontPtr font)
{
    return TRUE;
}

static Bool
glamor_unrealize_font(ScreenPtr screen, FontPtr font)
{
    glamor_screen_private       *glamor_priv;
    glamor_font_t               *privates = FontGetPrivate(font, glamor_font_private_index);
    glamor_font_t               *glamor_font;
    int                         s;

    if (!privates)
        return TRUE;

    glamor_font = &privates[screen->myNum];

    if (!glamor_font->realized)
        return TRUE;

    /* Unrealize the font, freeing the allocated texture */
    glamor_font->realized = FALSE;

    glamor_priv = glamor_get_screen_private(screen);
    glamor_make_current(glamor_priv);
    glDeleteTextures(1, &glamor_font->texture_id);

    /* Check to see if all of the screens are  done with this font
     * and free the private when that happens
     */
    for (s = 0; s < glamor_font_screen_count; s++)
        if (privates[s].realized)
            return TRUE;

    free(privates);
    FontSetPrivate(font, glamor_font_private_index, NULL);
    return TRUE;
}

Bool
glamor_font_init(ScreenPtr screen)
{
    glamor_screen_private *glamor_priv = glamor_get_screen_private(screen);

    if (glamor_priv->glsl_version < 130)
        return TRUE;

    if (glamor_font_generation != serverGeneration) {
        glamor_font_private_index = AllocateFontPrivateIndex();
        if (glamor_font_private_index == -1)
            return FALSE;
        glamor_font_screen_count = 0;
        glamor_font_generation = serverGeneration;
    }

    if (screen->myNum >= glamor_font_screen_count)
        glamor_font_screen_count = screen->myNum + 1;

    screen->RealizeFont = glamor_realize_font;
    screen->UnrealizeFont = glamor_unrealize_font;
    return TRUE;
}
