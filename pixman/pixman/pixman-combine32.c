/*
 * Copyright © 2000 Keith Packard, member of The XFree86 Project, Inc.
 *             2005 Lars Knoll & Zack Rusin, Trolltech
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>
#include <string.h>

#include "pixman-private.h"
#include "pixman-combine32.h"

/* component alpha helper functions */

static void
combine_mask_ca (uint32_t *src, uint32_t *mask)
{
    uint32_t a = *mask;

    uint32_t x;
    uint16_t xa;

    if (!a)
    {
	*(src) = 0;
	return;
    }

    x = *(src);
    if (a == ~0)
    {
	x = x >> A_SHIFT;
	x |= x << G_SHIFT;
	x |= x << R_SHIFT;
	*(mask) = x;
	return;
    }

    xa = x >> A_SHIFT;
    UN8x4_MUL_UN8x4 (x, a);
    *(src) = x;
    
    UN8x4_MUL_UN8 (a, xa);
    *(mask) = a;
}

static void
combine_mask_value_ca (uint32_t *src, const uint32_t *mask)
{
    uint32_t a = *mask;
    uint32_t x;

    if (!a)
    {
	*(src) = 0;
	return;
    }

    if (a == ~0)
	return;

    x = *(src);
    UN8x4_MUL_UN8x4 (x, a);
    *(src) = x;
}

static void
combine_mask_alpha_ca (const uint32_t *src, uint32_t *mask)
{
    uint32_t a = *(mask);
    uint32_t x;

    if (!a)
	return;

    x = *(src) >> A_SHIFT;
    if (x == MASK)
	return;

    if (a == ~0)
    {
	x |= x << G_SHIFT;
	x |= x << R_SHIFT;
	*(mask) = x;
	return;
    }

    UN8x4_MUL_UN8 (a, x);
    *(mask) = a;
}

/*
 * There are two ways of handling alpha -- either as a single unified value or
 * a separate value for each component, hence each macro must have two
 * versions.  The unified alpha version has a 'u' at the end of the name,
 * the component version has a 'ca'.  Similarly, functions which deal with
 * this difference will have two versions using the same convention.
 */

static force_inline uint32_t
combine_mask (const uint32_t *src, const uint32_t *mask, int i)
{
    uint32_t s, m;

    if (mask)
    {
	m = *(mask + i) >> A_SHIFT;

	if (!m)
	    return 0;
    }

    s = *(src + i);

    if (mask)
	UN8x4_MUL_UN8 (s, m);

    return s;
}

static void
combine_clear (pixman_implementation_t *imp,
               pixman_op_t              op,
               uint32_t *                dest,
               const uint32_t *          src,
               const uint32_t *          mask,
               int                      width)
{
    memset (dest, 0, width * sizeof(uint32_t));
}

static void
combine_dst (pixman_implementation_t *imp,
	     pixman_op_t	      op,
	     uint32_t *		      dest,
	     const uint32_t *	      src,
	     const uint32_t *          mask,
	     int		      width)
{
    return;
}

static void
combine_src_u (pixman_implementation_t *imp,
               pixman_op_t              op,
               uint32_t *                dest,
               const uint32_t *          src,
               const uint32_t *          mask,
               int                      width)
{
    int i;

    if (!mask)
    {
	memcpy (dest, src, width * sizeof (uint32_t));
    }
    else
    {
	for (i = 0; i < width; ++i)
	{
	    uint32_t s = combine_mask (src, mask, i);

	    *(dest + i) = s;
	}
    }
}

static void
combine_over_u (pixman_implementation_t *imp,
                pixman_op_t              op,
                uint32_t *                dest,
                const uint32_t *          src,
                const uint32_t *          mask,
                int                      width)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t s = combine_mask (src, mask, i);
	uint32_t d = *(dest + i);
	uint32_t ia = ALPHA_8 (~s);

	UN8x4_MUL_UN8_ADD_UN8x4 (d, ia, s);
	*(dest + i) = d;
    }
}

static void
combine_over_reverse_u (pixman_implementation_t *imp,
                        pixman_op_t              op,
                        uint32_t *                dest,
                        const uint32_t *          src,
                        const uint32_t *          mask,
                        int                      width)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t s = combine_mask (src, mask, i);
	uint32_t d = *(dest + i);
	uint32_t ia = ALPHA_8 (~*(dest + i));
	UN8x4_MUL_UN8_ADD_UN8x4 (s, ia, d);
	*(dest + i) = s;
    }
}

static void
combine_in_u (pixman_implementation_t *imp,
              pixman_op_t              op,
              uint32_t *                dest,
              const uint32_t *          src,
              const uint32_t *          mask,
              int                      width)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t s = combine_mask (src, mask, i);
	uint32_t a = ALPHA_8 (*(dest + i));
	UN8x4_MUL_UN8 (s, a);
	*(dest + i) = s;
    }
}

static void
combine_in_reverse_u (pixman_implementation_t *imp,
                      pixman_op_t              op,
                      uint32_t *                dest,
                      const uint32_t *          src,
                      const uint32_t *          mask,
                      int                      width)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t s = combine_mask (src, mask, i);
	uint32_t d = *(dest + i);
	uint32_t a = ALPHA_8 (s);
	UN8x4_MUL_UN8 (d, a);
	*(dest + i) = d;
    }
}

static void
combine_out_u (pixman_implementation_t *imp,
               pixman_op_t              op,
               uint32_t *                dest,
               const uint32_t *          src,
               const uint32_t *          mask,
               int                      width)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t s = combine_mask (src, mask, i);
	uint32_t a = ALPHA_8 (~*(dest + i));
	UN8x4_MUL_UN8 (s, a);
	*(dest + i) = s;
    }
}

static void
combine_out_reverse_u (pixman_implementation_t *imp,
                       pixman_op_t              op,
                       uint32_t *                dest,
                       const uint32_t *          src,
                       const uint32_t *          mask,
                       int                      width)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t s = combine_mask (src, mask, i);
	uint32_t d = *(dest + i);
	uint32_t a = ALPHA_8 (~s);
	UN8x4_MUL_UN8 (d, a);
	*(dest + i) = d;
    }
}

static void
combine_atop_u (pixman_implementation_t *imp,
                pixman_op_t              op,
                uint32_t *                dest,
                const uint32_t *          src,
                const uint32_t *          mask,
                int                      width)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t s = combine_mask (src, mask, i);
	uint32_t d = *(dest + i);
	uint32_t dest_a = ALPHA_8 (d);
	uint32_t src_ia = ALPHA_8 (~s);

	UN8x4_MUL_UN8_ADD_UN8x4_MUL_UN8 (s, dest_a, d, src_ia);
	*(dest + i) = s;
    }
}

static void
combine_atop_reverse_u (pixman_implementation_t *imp,
                        pixman_op_t              op,
                        uint32_t *                dest,
                        const uint32_t *          src,
                        const uint32_t *          mask,
                        int                      width)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t s = combine_mask (src, mask, i);
	uint32_t d = *(dest + i);
	uint32_t src_a = ALPHA_8 (s);
	uint32_t dest_ia = ALPHA_8 (~d);

	UN8x4_MUL_UN8_ADD_UN8x4_MUL_UN8 (s, dest_ia, d, src_a);
	*(dest + i) = s;
    }
}

static void
combine_xor_u (pixman_implementation_t *imp,
               pixman_op_t              op,
               uint32_t *                dest,
               const uint32_t *          src,
               const uint32_t *          mask,
               int                      width)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t s = combine_mask (src, mask, i);
	uint32_t d = *(dest + i);
	uint32_t src_ia = ALPHA_8 (~s);
	uint32_t dest_ia = ALPHA_8 (~d);

	UN8x4_MUL_UN8_ADD_UN8x4_MUL_UN8 (s, dest_ia, d, src_ia);
	*(dest + i) = s;
    }
}

static void
combine_add_u (pixman_implementation_t *imp,
               pixman_op_t              op,
               uint32_t *                dest,
               const uint32_t *          src,
               const uint32_t *          mask,
               int                      width)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t s = combine_mask (src, mask, i);
	uint32_t d = *(dest + i);
	UN8x4_ADD_UN8x4 (d, s);
	*(dest + i) = d;
    }
}

static void
combine_saturate_u (pixman_implementation_t *imp,
                    pixman_op_t              op,
                    uint32_t *                dest,
                    const uint32_t *          src,
                    const uint32_t *          mask,
                    int                      width)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t s = combine_mask (src, mask, i);
	uint32_t d = *(dest + i);
	uint16_t sa, da;

	sa = s >> A_SHIFT;
	da = ~d >> A_SHIFT;
	if (sa > da)
	{
	    sa = DIV_UN8 (da, sa);
	    UN8x4_MUL_UN8 (s, sa);
	}
	;
	UN8x4_ADD_UN8x4 (d, s);
	*(dest + i) = d;
    }
}

/*
 * PDF blend modes:
 * The following blend modes have been taken from the PDF ISO 32000
 * specification, which at this point in time is available from
 * http://www.adobe.com/devnet/acrobat/pdfs/PDF32000_2008.pdf
 * The relevant chapters are 11.3.5 and 11.3.6.
 * The formula for computing the final pixel color given in 11.3.6 is:
 * αr × Cr = (1 – αs) × αb × Cb + (1 – αb) × αs × Cs + αb × αs × B(Cb, Cs)
 * with B() being the blend function.
 * Note that OVER is a special case of this operation, using B(Cb, Cs) = Cs
 *
 * These blend modes should match the SVG filter draft specification, as
 * it has been designed to mirror ISO 32000. Note that at the current point
 * no released draft exists that shows this, as the formulas have not been
 * updated yet after the release of ISO 32000.
 *
 * The default implementation here uses the PDF_SEPARABLE_BLEND_MODE and
 * PDF_NON_SEPARABLE_BLEND_MODE macros, which take the blend function as an
 * argument. Note that this implementation operates on premultiplied colors,
 * while the PDF specification does not. Therefore the code uses the formula
 * Cra = (1 – as) . Dca + (1 – ad) . Sca + B(Dca, ad, Sca, as)
 */

/*
 * Multiply
 * B(Dca, ad, Sca, as) = Dca.Sca
 */
static void
combine_multiply_u (pixman_implementation_t *imp,
                    pixman_op_t              op,
                    uint32_t *                dest,
                    const uint32_t *          src,
                    const uint32_t *          mask,
                    int                      width)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t s = combine_mask (src, mask, i);
	uint32_t d = *(dest + i);
	uint32_t ss = s;
	uint32_t src_ia = ALPHA_8 (~s);
	uint32_t dest_ia = ALPHA_8 (~d);

	UN8x4_MUL_UN8_ADD_UN8x4_MUL_UN8 (ss, dest_ia, d, src_ia);
	UN8x4_MUL_UN8x4 (d, s);
	UN8x4_ADD_UN8x4 (d, ss);

	*(dest + i) = d;
    }
}

static void
combine_multiply_ca (pixman_implementation_t *imp,
                     pixman_op_t              op,
                     uint32_t *                dest,
                     const uint32_t *          src,
                     const uint32_t *          mask,
                     int                      width)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t m = *(mask + i);
	uint32_t s = *(src + i);
	uint32_t d = *(dest + i);
	uint32_t r = d;
	uint32_t dest_ia = ALPHA_8 (~d);

	combine_mask_ca (&s, &m);

	UN8x4_MUL_UN8x4_ADD_UN8x4_MUL_UN8 (r, ~m, s, dest_ia);
	UN8x4_MUL_UN8x4 (d, s);
	UN8x4_ADD_UN8x4 (r, d);

	*(dest + i) = r;
    }
}

#define PDF_SEPARABLE_BLEND_MODE(name)					\
    static void								\
    combine_ ## name ## _u (pixman_implementation_t *imp,		\
			    pixman_op_t              op,		\
                            uint32_t *                dest,		\
			    const uint32_t *          src,		\
			    const uint32_t *          mask,		\
			    int                      width)		\
    {									\
	int i;								\
	for (i = 0; i < width; ++i) {					\
	    uint32_t s = combine_mask (src, mask, i);			\
	    uint32_t d = *(dest + i);					\
	    uint8_t sa = ALPHA_8 (s);					\
	    uint8_t isa = ~sa;						\
	    uint8_t da = ALPHA_8 (d);					\
	    uint8_t ida = ~da;						\
	    uint32_t result;						\
									\
	    result = d;							\
	    UN8x4_MUL_UN8_ADD_UN8x4_MUL_UN8 (result, isa, s, ida);	\
	    								\
	    *(dest + i) = result +					\
		(DIV_ONE_UN8 (sa * (uint32_t)da) << A_SHIFT) +		\
		(blend_ ## name (RED_8 (d), da, RED_8 (s), sa) << R_SHIFT) + \
		(blend_ ## name (GREEN_8 (d), da, GREEN_8 (s), sa) << G_SHIFT) + \
		(blend_ ## name (BLUE_8 (d), da, BLUE_8 (s), sa));	\
	}								\
    }									\
    									\
    static void								\
    combine_ ## name ## _ca (pixman_implementation_t *imp,		\
			     pixman_op_t              op,		\
                             uint32_t *                dest,		\
			     const uint32_t *          src,		\
			     const uint32_t *          mask,		\
			     int                     width)		\
    {									\
	int i;								\
	for (i = 0; i < width; ++i) {					\
	    uint32_t m = *(mask + i);					\
	    uint32_t s = *(src + i);					\
	    uint32_t d = *(dest + i);					\
	    uint8_t da = ALPHA_8 (d);					\
	    uint8_t ida = ~da;						\
	    uint32_t result;						\
            								\
	    combine_mask_ca (&s, &m);					\
            								\
	    result = d;							\
	    UN8x4_MUL_UN8x4_ADD_UN8x4_MUL_UN8 (result, ~m, s, ida);     \
            								\
	    result +=							\
	        (DIV_ONE_UN8 (ALPHA_8 (m) * (uint32_t)da) << A_SHIFT) +	\
	        (blend_ ## name (RED_8 (d), da, RED_8 (s), RED_8 (m)) << R_SHIFT) + \
	        (blend_ ## name (GREEN_8 (d), da, GREEN_8 (s), GREEN_8 (m)) << G_SHIFT) + \
	        (blend_ ## name (BLUE_8 (d), da, BLUE_8 (s), BLUE_8 (m))); \
	    								\
	    *(dest + i) = result;					\
	}								\
    }

/*
 * Screen
 * B(Dca, ad, Sca, as) = Dca.sa + Sca.da - Dca.Sca
 */
static inline uint32_t
blend_screen (uint32_t dca, uint32_t da, uint32_t sca, uint32_t sa)
{
    return DIV_ONE_UN8 (sca * da + dca * sa - sca * dca);
}

PDF_SEPARABLE_BLEND_MODE (screen)

/*
 * Overlay
 * B(Dca, Da, Sca, Sa) =
 *   if 2.Dca < Da
 *     2.Sca.Dca
 *   otherwise
 *     Sa.Da - 2.(Da - Dca).(Sa - Sca)
 */
static inline uint32_t
blend_overlay (uint32_t dca, uint32_t da, uint32_t sca, uint32_t sa)
{
    uint32_t rca;

    if (2 * dca < da)
	rca = 2 * sca * dca;
    else
	rca = sa * da - 2 * (da - dca) * (sa - sca);
    return DIV_ONE_UN8 (rca);
}

PDF_SEPARABLE_BLEND_MODE (overlay)

/*
 * Darken
 * B(Dca, Da, Sca, Sa) = min (Sca.Da, Dca.Sa)
 */
static inline uint32_t
blend_darken (uint32_t dca, uint32_t da, uint32_t sca, uint32_t sa)
{
    uint32_t s, d;

    s = sca * da;
    d = dca * sa;
    return DIV_ONE_UN8 (s > d ? d : s);
}

PDF_SEPARABLE_BLEND_MODE (darken)

/*
 * Lighten
 * B(Dca, Da, Sca, Sa) = max (Sca.Da, Dca.Sa)
 */
static inline uint32_t
blend_lighten (uint32_t dca, uint32_t da, uint32_t sca, uint32_t sa)
{
    uint32_t s, d;

    s = sca * da;
    d = dca * sa;
    return DIV_ONE_UN8 (s > d ? s : d);
}

PDF_SEPARABLE_BLEND_MODE (lighten)

/*
 * Color dodge
 * B(Dca, Da, Sca, Sa) =
 *   if Dca == 0
 *     0
 *   if Sca == Sa
 *     Sa.Da
 *   otherwise
 *     Sa.Da. min (1, Dca / Da / (1 - Sca/Sa))
 */
static inline uint32_t
blend_color_dodge (uint32_t dca, uint32_t da, uint32_t sca, uint32_t sa)
{
    if (sca >= sa)
    {
	return dca == 0 ? 0 : DIV_ONE_UN8 (sa * da);
    }
    else
    {
	uint32_t rca = dca * sa / (sa - sca);
	return DIV_ONE_UN8 (sa * MIN (rca, da));
    }
}

PDF_SEPARABLE_BLEND_MODE (color_dodge)

/*
 * Color burn
 * B(Dca, Da, Sca, Sa) =
 *   if Dca == Da
 *     Sa.Da
 *   if Sca == 0
 *     0
 *   otherwise
 *     Sa.Da.(1 - min (1, (1 - Dca/Da).Sa / Sca))
 */
static inline uint32_t
blend_color_burn (uint32_t dca, uint32_t da, uint32_t sca, uint32_t sa)
{
    if (sca == 0)
    {
	return dca < da ? 0 : DIV_ONE_UN8 (sa * da);
    }
    else
    {
	uint32_t rca = (da - dca) * sa / sca;
	return DIV_ONE_UN8 (sa * (MAX (rca, da) - rca));
    }
}

PDF_SEPARABLE_BLEND_MODE (color_burn)

/*
 * Hard light
 * B(Dca, Da, Sca, Sa) =
 *   if 2.Sca < Sa
 *     2.Sca.Dca
 *   otherwise
 *     Sa.Da - 2.(Da - Dca).(Sa - Sca)
 */
static inline uint32_t
blend_hard_light (uint32_t dca, uint32_t da, uint32_t sca, uint32_t sa)
{
    if (2 * sca < sa)
	return DIV_ONE_UN8 (2 * sca * dca);
    else
	return DIV_ONE_UN8 (sa * da - 2 * (da - dca) * (sa - sca));
}

PDF_SEPARABLE_BLEND_MODE (hard_light)

/*
 * Soft light
 * B(Dca, Da, Sca, Sa) =
 *   if (2.Sca <= Sa)
 *     Dca.(Sa - (1 - Dca/Da).(2.Sca - Sa))
 *   otherwise if Dca.4 <= Da
 *     Dca.(Sa + (2.Sca - Sa).((16.Dca/Da - 12).Dca/Da + 3)
 *   otherwise
 *     (Dca.Sa + (SQRT (Dca/Da).Da - Dca).(2.Sca - Sa))
 */
static inline uint32_t
blend_soft_light (uint32_t dca_org,
		  uint32_t da_org,
		  uint32_t sca_org,
		  uint32_t sa_org)
{
    double dca = dca_org * (1.0 / MASK);
    double da = da_org * (1.0 / MASK);
    double sca = sca_org * (1.0 / MASK);
    double sa = sa_org * (1.0 / MASK);
    double rca;

    if (2 * sca < sa)
    {
	if (da == 0)
	    rca = dca * sa;
	else
	    rca = dca * sa - dca * (da - dca) * (sa - 2 * sca) / da;
    }
    else if (da == 0)
    {
	rca = 0;
    }
    else if (4 * dca <= da)
    {
	rca = dca * sa +
	    (2 * sca - sa) * dca * ((16 * dca / da - 12) * dca / da + 3);
    }
    else
    {
	rca = dca * sa + (sqrt (dca * da) - dca) * (2 * sca - sa);
    }
    return rca * MASK + 0.5;
}

PDF_SEPARABLE_BLEND_MODE (soft_light)

/*
 * Difference
 * B(Dca, Da, Sca, Sa) = abs (Dca.Sa - Sca.Da)
 */
static inline uint32_t
blend_difference (uint32_t dca, uint32_t da, uint32_t sca, uint32_t sa)
{
    uint32_t dcasa = dca * sa;
    uint32_t scada = sca * da;

    if (scada < dcasa)
	return DIV_ONE_UN8 (dcasa - scada);
    else
	return DIV_ONE_UN8 (scada - dcasa);
}

PDF_SEPARABLE_BLEND_MODE (difference)

/*
 * Exclusion
 * B(Dca, Da, Sca, Sa) = (Sca.Da + Dca.Sa - 2.Sca.Dca)
 */

/* This can be made faster by writing it directly and not using
 * PDF_SEPARABLE_BLEND_MODE, but that's a performance optimization */

static inline uint32_t
blend_exclusion (uint32_t dca, uint32_t da, uint32_t sca, uint32_t sa)
{
    return DIV_ONE_UN8 (sca * da + dca * sa - 2 * dca * sca);
}

PDF_SEPARABLE_BLEND_MODE (exclusion)

#undef PDF_SEPARABLE_BLEND_MODE

/*
 * PDF nonseperable blend modes are implemented using the following functions
 * to operate in Hsl space, with Cmax, Cmid, Cmin referring to the max, mid
 * and min value of the red, green and blue components.
 *
 * LUM (C) = 0.3 × Cred + 0.59 × Cgreen + 0.11 × Cblue
 *
 * clip_color (C):
 *   l = LUM (C)
 *   min = Cmin
 *   max = Cmax
 *   if n < 0.0
 *     C = l + ( ( ( C – l ) × l ) ⁄ ( l – min ) )
 *   if x > 1.0
 *     C = l + ( ( ( C – l ) × ( 1 – l ) ) ⁄ ( max – l ) )
 *   return C
 *
 * set_lum (C, l):
 *   d = l – LUM (C)
 *   C += d
 *   return clip_color (C)
 *
 * SAT (C) = CH_MAX (C) - CH_MIN (C)
 *
 * set_sat (C, s):
 *  if Cmax > Cmin
 *    Cmid = ( ( ( Cmid – Cmin ) × s ) ⁄ ( Cmax – Cmin ) )
 *    Cmax = s
 *  else
 *    Cmid = Cmax = 0.0
 *  Cmin = 0.0
 *  return C
 */

/* For premultiplied colors, we need to know what happens when C is
 * multiplied by a real number. LUM and SAT are linear:
 *
 *    LUM (r × C) = r × LUM (C)		SAT (r * C) = r * SAT (C)
 *
 * If we extend clip_color with an extra argument a and change
 *
 *        if x >= 1.0
 *
 * into
 *
 *        if x >= a
 *
 * then clip_color is also linear:
 *
 *    r * clip_color (C, a) = clip_color (r_c, ra);
 *
 * for positive r.
 *
 * Similarly, we can extend set_lum with an extra argument that is just passed
 * on to clip_color:
 *
 *   r * set_lum ( C, l, a)
 *
 *   = r × clip_color ( C + l - LUM (C), a)
 *
 *   = clip_color ( r * C + r × l - r * LUM (C), r * a)
 *
 *   = set_lum ( r * C, r * l, r * a)
 *
 * Finally, set_sat:
 *
 *    r * set_sat (C, s) = set_sat (x * C, r * s)
 *
 * The above holds for all non-zero x, because the x'es in the fraction for
 * C_mid cancel out. Specifically, it holds for x = r:
 *
 *    r * set_sat (C, s) = set_sat (r_c, rs)
 *
 */

/* So, for the non-separable PDF blend modes, we have (using s, d for
 * non-premultiplied colors, and S, D for premultiplied:
 *
 *   Color:
 *
 *     a_s * a_d * B(s, d)
 *   = a_s * a_d * set_lum (S/a_s, LUM (D/a_d), 1)
 *   = set_lum (S * a_d, a_s * LUM (D), a_s * a_d)
 *
 *
 *   Luminosity:
 *
 *     a_s * a_d * B(s, d)
 *   = a_s * a_d * set_lum (D/a_d, LUM(S/a_s), 1)
 *   = set_lum (a_s * D, a_d * LUM(S), a_s * a_d)
 *
 *
 *   Saturation:
 *
 *     a_s * a_d * B(s, d)
 *   = a_s * a_d * set_lum (set_sat (D/a_d, SAT (S/a_s)), LUM (D/a_d), 1)
 *   = set_lum (a_s * a_d * set_sat (D/a_d, SAT (S/a_s)),
 *                                        a_s * LUM (D), a_s * a_d)
 *   = set_lum (set_sat (a_s * D, a_d * SAT (S), a_s * LUM (D), a_s * a_d))
 *
 *   Hue:
 *
 *     a_s * a_d * B(s, d)
 *   = a_s * a_d * set_lum (set_sat (S/a_s, SAT (D/a_d)), LUM (D/a_d), 1)
 *   = set_lum (set_sat (a_d * S, a_s * SAT (D)), a_s * LUM (D), a_s * a_d)
 *
 */

#define CH_MIN(c) (c[0] < c[1] ? (c[0] < c[2] ? c[0] : c[2]) : (c[1] < c[2] ? c[1] : c[2]))
#define CH_MAX(c) (c[0] > c[1] ? (c[0] > c[2] ? c[0] : c[2]) : (c[1] > c[2] ? c[1] : c[2]))
#define LUM(c) ((c[0] * 30 + c[1] * 59 + c[2] * 11) / 100)
#define SAT(c) (CH_MAX (c) - CH_MIN (c))

#define PDF_NON_SEPARABLE_BLEND_MODE(name)				\
    static void								\
    combine_ ## name ## _u (pixman_implementation_t *imp,		\
			    pixman_op_t op,				\
                            uint32_t *dest,				\
			    const uint32_t *src,				\
			    const uint32_t *mask,			\
			    int width)					\
    {									\
	int i;								\
	for (i = 0; i < width; ++i)					\
	{								\
	    uint32_t s = combine_mask (src, mask, i);			\
	    uint32_t d = *(dest + i);					\
	    uint8_t sa = ALPHA_8 (s);					\
	    uint8_t isa = ~sa;						\
	    uint8_t da = ALPHA_8 (d);					\
	    uint8_t ida = ~da;						\
	    uint32_t result;						\
	    uint32_t sc[3], dc[3], c[3];					\
            								\
	    result = d;							\
	    UN8x4_MUL_UN8_ADD_UN8x4_MUL_UN8 (result, isa, s, ida);	\
	    dc[0] = RED_8 (d);						\
	    sc[0] = RED_8 (s);						\
	    dc[1] = GREEN_8 (d);					\
	    sc[1] = GREEN_8 (s);					\
	    dc[2] = BLUE_8 (d);						\
	    sc[2] = BLUE_8 (s);						\
	    blend_ ## name (c, dc, da, sc, sa);				\
            								\
	    *(dest + i) = result +					\
		(DIV_ONE_UN8 (sa * (uint32_t)da) << A_SHIFT) +		\
		(DIV_ONE_UN8 (c[0]) << R_SHIFT) +			\
		(DIV_ONE_UN8 (c[1]) << G_SHIFT) +			\
		(DIV_ONE_UN8 (c[2]));					\
	}								\
    }

static void
set_lum (uint32_t dest[3], uint32_t src[3], uint32_t sa, uint32_t lum)
{
    double a, l, min, max;
    double tmp[3];

    a = sa * (1.0 / MASK);

    l = lum * (1.0 / MASK);
    tmp[0] = src[0] * (1.0 / MASK);
    tmp[1] = src[1] * (1.0 / MASK);
    tmp[2] = src[2] * (1.0 / MASK);

    l = l - LUM (tmp);
    tmp[0] += l;
    tmp[1] += l;
    tmp[2] += l;

    /* clip_color */
    l = LUM (tmp);
    min = CH_MIN (tmp);
    max = CH_MAX (tmp);

    if (min < 0)
    {
	if (l - min == 0.0)
	{
	    tmp[0] = 0;
	    tmp[1] = 0;
	    tmp[2] = 0;
	}
	else
	{
	    tmp[0] = l + (tmp[0] - l) * l / (l - min);
	    tmp[1] = l + (tmp[1] - l) * l / (l - min);
	    tmp[2] = l + (tmp[2] - l) * l / (l - min);
	}
    }
    if (max > a)
    {
	if (max - l == 0.0)
	{
	    tmp[0] = a;
	    tmp[1] = a;
	    tmp[2] = a;
	}
	else
	{
	    tmp[0] = l + (tmp[0] - l) * (a - l) / (max - l);
	    tmp[1] = l + (tmp[1] - l) * (a - l) / (max - l);
	    tmp[2] = l + (tmp[2] - l) * (a - l) / (max - l);
	}
    }

    dest[0] = tmp[0] * MASK + 0.5;
    dest[1] = tmp[1] * MASK + 0.5;
    dest[2] = tmp[2] * MASK + 0.5;
}

static void
set_sat (uint32_t dest[3], uint32_t src[3], uint32_t sat)
{
    int id[3];
    uint32_t min, max;

    if (src[0] > src[1])
    {
	if (src[0] > src[2])
	{
	    id[0] = 0;
	    if (src[1] > src[2])
	    {
		id[1] = 1;
		id[2] = 2;
	    }
	    else
	    {
		id[1] = 2;
		id[2] = 1;
	    }
	}
	else
	{
	    id[0] = 2;
	    id[1] = 0;
	    id[2] = 1;
	}
    }
    else
    {
	if (src[0] > src[2])
	{
	    id[0] = 1;
	    id[1] = 0;
	    id[2] = 2;
	}
	else
	{
	    id[2] = 0;
	    if (src[1] > src[2])
	    {
		id[0] = 1;
		id[1] = 2;
	    }
	    else
	    {
		id[0] = 2;
		id[1] = 1;
	    }
	}
    }

    max = dest[id[0]];
    min = dest[id[2]];
    if (max > min)
    {
	dest[id[1]] = (dest[id[1]] - min) * sat / (max - min);
	dest[id[0]] = sat;
	dest[id[2]] = 0;
    }
    else
    {
	dest[0] = dest[1] = dest[2] = 0;
    }
}

/*
 * Hue:
 * B(Cb, Cs) = set_lum (set_sat (Cs, SAT (Cb)), LUM (Cb))
 */
static inline void
blend_hsl_hue (uint32_t c[3],
               uint32_t dc[3],
               uint32_t da,
               uint32_t sc[3],
               uint32_t sa)
{
    c[0] = sc[0] * da;
    c[1] = sc[1] * da;
    c[2] = sc[2] * da;
    set_sat (c, c, SAT (dc) * sa);
    set_lum (c, c, sa * da, LUM (dc) * sa);
}

PDF_NON_SEPARABLE_BLEND_MODE (hsl_hue)

/*
 * Saturation:
 * B(Cb, Cs) = set_lum (set_sat (Cb, SAT (Cs)), LUM (Cb))
 */
static inline void
blend_hsl_saturation (uint32_t c[3],
                      uint32_t dc[3],
                      uint32_t da,
                      uint32_t sc[3],
                      uint32_t sa)
{
    c[0] = dc[0] * sa;
    c[1] = dc[1] * sa;
    c[2] = dc[2] * sa;
    set_sat (c, c, SAT (sc) * da);
    set_lum (c, c, sa * da, LUM (dc) * sa);
}

PDF_NON_SEPARABLE_BLEND_MODE (hsl_saturation)

/*
 * Color:
 * B(Cb, Cs) = set_lum (Cs, LUM (Cb))
 */
static inline void
blend_hsl_color (uint32_t c[3],
                 uint32_t dc[3],
                 uint32_t da,
                 uint32_t sc[3],
                 uint32_t sa)
{
    c[0] = sc[0] * da;
    c[1] = sc[1] * da;
    c[2] = sc[2] * da;
    set_lum (c, c, sa * da, LUM (dc) * sa);
}

PDF_NON_SEPARABLE_BLEND_MODE (hsl_color)

/*
 * Luminosity:
 * B(Cb, Cs) = set_lum (Cb, LUM (Cs))
 */
static inline void
blend_hsl_luminosity (uint32_t c[3],
                      uint32_t dc[3],
                      uint32_t da,
                      uint32_t sc[3],
                      uint32_t sa)
{
    c[0] = dc[0] * sa;
    c[1] = dc[1] * sa;
    c[2] = dc[2] * sa;
    set_lum (c, c, sa * da, LUM (sc) * da);
}

PDF_NON_SEPARABLE_BLEND_MODE (hsl_luminosity)

#undef SAT
#undef LUM
#undef CH_MAX
#undef CH_MIN
#undef PDF_NON_SEPARABLE_BLEND_MODE

/* All of the disjoint/conjoint composing functions
 *
 * The four entries in the first column indicate what source contributions
 * come from each of the four areas of the picture -- areas covered by neither
 * A nor B, areas covered only by A, areas covered only by B and finally
 * areas covered by both A and B.
 * 
 * Disjoint			Conjoint
 * Fa		Fb		Fa		Fb
 * (0,0,0,0)	0		0		0		0
 * (0,A,0,A)	1		0		1		0
 * (0,0,B,B)	0		1		0		1
 * (0,A,B,A)	1		min((1-a)/b,1)	1		max(1-a/b,0)
 * (0,A,B,B)	min((1-b)/a,1)	1		max(1-b/a,0)	1
 * (0,0,0,A)	max(1-(1-b)/a,0) 0		min(1,b/a)	0
 * (0,0,0,B)	0		max(1-(1-a)/b,0) 0		min(a/b,1)
 * (0,A,0,0)	min(1,(1-b)/a)	0		max(1-b/a,0)	0
 * (0,0,B,0)	0		min(1,(1-a)/b)	0		max(1-a/b,0)
 * (0,0,B,A)	max(1-(1-b)/a,0) min(1,(1-a)/b)	 min(1,b/a)	max(1-a/b,0)
 * (0,A,0,B)	min(1,(1-b)/a)	max(1-(1-a)/b,0) max(1-b/a,0)	min(1,a/b)
 * (0,A,B,0)	min(1,(1-b)/a)	min(1,(1-a)/b)	max(1-b/a,0)	max(1-a/b,0)
 *
 * See  http://marc.info/?l=xfree-render&m=99792000027857&w=2  for more
 * information about these operators.
 */

#define COMBINE_A_OUT 1
#define COMBINE_A_IN  2
#define COMBINE_B_OUT 4
#define COMBINE_B_IN  8

#define COMBINE_CLEAR   0
#define COMBINE_A       (COMBINE_A_OUT | COMBINE_A_IN)
#define COMBINE_B       (COMBINE_B_OUT | COMBINE_B_IN)
#define COMBINE_A_OVER  (COMBINE_A_OUT | COMBINE_B_OUT | COMBINE_A_IN)
#define COMBINE_B_OVER  (COMBINE_A_OUT | COMBINE_B_OUT | COMBINE_B_IN)
#define COMBINE_A_ATOP  (COMBINE_B_OUT | COMBINE_A_IN)
#define COMBINE_B_ATOP  (COMBINE_A_OUT | COMBINE_B_IN)
#define COMBINE_XOR     (COMBINE_A_OUT | COMBINE_B_OUT)

/* portion covered by a but not b */
static uint8_t
combine_disjoint_out_part (uint8_t a, uint8_t b)
{
    /* min (1, (1-b) / a) */

    b = ~b;                 /* 1 - b */
    if (b >= a)             /* 1 - b >= a -> (1-b)/a >= 1 */
	return MASK;        /* 1 */
    return DIV_UN8 (b, a);     /* (1-b) / a */
}

/* portion covered by both a and b */
static uint8_t
combine_disjoint_in_part (uint8_t a, uint8_t b)
{
    /* max (1-(1-b)/a,0) */
    /*  = - min ((1-b)/a - 1, 0) */
    /*  = 1 - min (1, (1-b)/a) */

    b = ~b;                 /* 1 - b */
    if (b >= a)             /* 1 - b >= a -> (1-b)/a >= 1 */
	return 0;           /* 1 - 1 */
    return ~DIV_UN8(b, a);    /* 1 - (1-b) / a */
}

/* portion covered by a but not b */
static uint8_t
combine_conjoint_out_part (uint8_t a, uint8_t b)
{
    /* max (1-b/a,0) */
    /* = 1-min(b/a,1) */

    /* min (1, (1-b) / a) */

    if (b >= a)             /* b >= a -> b/a >= 1 */
	return 0x00;        /* 0 */
    return ~DIV_UN8(b, a);    /* 1 - b/a */
}

/* portion covered by both a and b */
static uint8_t
combine_conjoint_in_part (uint8_t a, uint8_t b)
{
    /* min (1,b/a) */

    if (b >= a)             /* b >= a -> b/a >= 1 */
	return MASK;        /* 1 */
    return DIV_UN8 (b, a);     /* b/a */
}

#define GET_COMP(v, i)   ((uint16_t) (uint8_t) ((v) >> i))

#define ADD(x, y, i, t)							\
    ((t) = GET_COMP (x, i) + GET_COMP (y, i),				\
     (uint32_t) ((uint8_t) ((t) | (0 - ((t) >> G_SHIFT)))) << (i))

#define GENERIC(x, y, i, ax, ay, t, u, v)				\
    ((t) = (MUL_UN8 (GET_COMP (y, i), ay, (u)) +			\
            MUL_UN8 (GET_COMP (x, i), ax, (v))),			\
     (uint32_t) ((uint8_t) ((t) |					\
                           (0 - ((t) >> G_SHIFT)))) << (i))

static void
combine_disjoint_general_u (uint32_t *      dest,
                            const uint32_t *src,
                            const uint32_t *mask,
                            int            width,
                            uint8_t        combine)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t s = combine_mask (src, mask, i);
	uint32_t d = *(dest + i);
	uint32_t m, n, o, p;
	uint16_t Fa, Fb, t, u, v;
	uint8_t sa = s >> A_SHIFT;
	uint8_t da = d >> A_SHIFT;

	switch (combine & COMBINE_A)
	{
	default:
	    Fa = 0;
	    break;

	case COMBINE_A_OUT:
	    Fa = combine_disjoint_out_part (sa, da);
	    break;

	case COMBINE_A_IN:
	    Fa = combine_disjoint_in_part (sa, da);
	    break;

	case COMBINE_A:
	    Fa = MASK;
	    break;
	}

	switch (combine & COMBINE_B)
	{
	default:
	    Fb = 0;
	    break;

	case COMBINE_B_OUT:
	    Fb = combine_disjoint_out_part (da, sa);
	    break;

	case COMBINE_B_IN:
	    Fb = combine_disjoint_in_part (da, sa);
	    break;

	case COMBINE_B:
	    Fb = MASK;
	    break;
	}
	m = GENERIC (s, d, 0, Fa, Fb, t, u, v);
	n = GENERIC (s, d, G_SHIFT, Fa, Fb, t, u, v);
	o = GENERIC (s, d, R_SHIFT, Fa, Fb, t, u, v);
	p = GENERIC (s, d, A_SHIFT, Fa, Fb, t, u, v);
	s = m | n | o | p;
	*(dest + i) = s;
    }
}

static void
combine_disjoint_over_u (pixman_implementation_t *imp,
                         pixman_op_t              op,
                         uint32_t *                dest,
                         const uint32_t *          src,
                         const uint32_t *          mask,
                         int                      width)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t s = combine_mask (src, mask, i);
	uint16_t a = s >> A_SHIFT;

	if (s != 0x00)
	{
	    uint32_t d = *(dest + i);
	    a = combine_disjoint_out_part (d >> A_SHIFT, a);
	    UN8x4_MUL_UN8_ADD_UN8x4 (d, a, s);

	    *(dest + i) = d;
	}
    }
}

static void
combine_disjoint_in_u (pixman_implementation_t *imp,
                       pixman_op_t              op,
                       uint32_t *                dest,
                       const uint32_t *          src,
                       const uint32_t *          mask,
                       int                      width)
{
    combine_disjoint_general_u (dest, src, mask, width, COMBINE_A_IN);
}

static void
combine_disjoint_in_reverse_u (pixman_implementation_t *imp,
                               pixman_op_t              op,
                               uint32_t *                dest,
                               const uint32_t *          src,
                               const uint32_t *          mask,
                               int                      width)
{
    combine_disjoint_general_u (dest, src, mask, width, COMBINE_B_IN);
}

static void
combine_disjoint_out_u (pixman_implementation_t *imp,
                        pixman_op_t              op,
                        uint32_t *                dest,
                        const uint32_t *          src,
                        const uint32_t *          mask,
                        int                      width)
{
    combine_disjoint_general_u (dest, src, mask, width, COMBINE_A_OUT);
}

static void
combine_disjoint_out_reverse_u (pixman_implementation_t *imp,
                                pixman_op_t              op,
                                uint32_t *                dest,
                                const uint32_t *          src,
                                const uint32_t *          mask,
                                int                      width)
{
    combine_disjoint_general_u (dest, src, mask, width, COMBINE_B_OUT);
}

static void
combine_disjoint_atop_u (pixman_implementation_t *imp,
                         pixman_op_t              op,
                         uint32_t *                dest,
                         const uint32_t *          src,
                         const uint32_t *          mask,
                         int                      width)
{
    combine_disjoint_general_u (dest, src, mask, width, COMBINE_A_ATOP);
}

static void
combine_disjoint_atop_reverse_u (pixman_implementation_t *imp,
                                 pixman_op_t              op,
                                 uint32_t *                dest,
                                 const uint32_t *          src,
                                 const uint32_t *          mask,
                                 int                      width)
{
    combine_disjoint_general_u (dest, src, mask, width, COMBINE_B_ATOP);
}

static void
combine_disjoint_xor_u (pixman_implementation_t *imp,
                        pixman_op_t              op,
                        uint32_t *                dest,
                        const uint32_t *          src,
                        const uint32_t *          mask,
                        int                      width)
{
    combine_disjoint_general_u (dest, src, mask, width, COMBINE_XOR);
}

static void
combine_conjoint_general_u (uint32_t *      dest,
                            const uint32_t *src,
                            const uint32_t *mask,
                            int            width,
                            uint8_t        combine)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t s = combine_mask (src, mask, i);
	uint32_t d = *(dest + i);
	uint32_t m, n, o, p;
	uint16_t Fa, Fb, t, u, v;
	uint8_t sa = s >> A_SHIFT;
	uint8_t da = d >> A_SHIFT;

	switch (combine & COMBINE_A)
	{
	default:
	    Fa = 0;
	    break;

	case COMBINE_A_OUT:
	    Fa = combine_conjoint_out_part (sa, da);
	    break;

	case COMBINE_A_IN:
	    Fa = combine_conjoint_in_part (sa, da);
	    break;

	case COMBINE_A:
	    Fa = MASK;
	    break;
	}

	switch (combine & COMBINE_B)
	{
	default:
	    Fb = 0;
	    break;

	case COMBINE_B_OUT:
	    Fb = combine_conjoint_out_part (da, sa);
	    break;

	case COMBINE_B_IN:
	    Fb = combine_conjoint_in_part (da, sa);
	    break;

	case COMBINE_B:
	    Fb = MASK;
	    break;
	}

	m = GENERIC (s, d, 0, Fa, Fb, t, u, v);
	n = GENERIC (s, d, G_SHIFT, Fa, Fb, t, u, v);
	o = GENERIC (s, d, R_SHIFT, Fa, Fb, t, u, v);
	p = GENERIC (s, d, A_SHIFT, Fa, Fb, t, u, v);

	s = m | n | o | p;

	*(dest + i) = s;
    }
}

static void
combine_conjoint_over_u (pixman_implementation_t *imp,
                         pixman_op_t              op,
                         uint32_t *                dest,
                         const uint32_t *          src,
                         const uint32_t *          mask,
                         int                      width)
{
    combine_conjoint_general_u (dest, src, mask, width, COMBINE_A_OVER);
}

static void
combine_conjoint_over_reverse_u (pixman_implementation_t *imp,
                                 pixman_op_t              op,
                                 uint32_t *                dest,
                                 const uint32_t *          src,
                                 const uint32_t *          mask,
                                 int                      width)
{
    combine_conjoint_general_u (dest, src, mask, width, COMBINE_B_OVER);
}

static void
combine_conjoint_in_u (pixman_implementation_t *imp,
                       pixman_op_t              op,
                       uint32_t *                dest,
                       const uint32_t *          src,
                       const uint32_t *          mask,
                       int                      width)
{
    combine_conjoint_general_u (dest, src, mask, width, COMBINE_A_IN);
}

static void
combine_conjoint_in_reverse_u (pixman_implementation_t *imp,
                               pixman_op_t              op,
                               uint32_t *                dest,
                               const uint32_t *          src,
                               const uint32_t *          mask,
                               int                      width)
{
    combine_conjoint_general_u (dest, src, mask, width, COMBINE_B_IN);
}

static void
combine_conjoint_out_u (pixman_implementation_t *imp,
                        pixman_op_t              op,
                        uint32_t *                dest,
                        const uint32_t *          src,
                        const uint32_t *          mask,
                        int                      width)
{
    combine_conjoint_general_u (dest, src, mask, width, COMBINE_A_OUT);
}

static void
combine_conjoint_out_reverse_u (pixman_implementation_t *imp,
                                pixman_op_t              op,
                                uint32_t *                dest,
                                const uint32_t *          src,
                                const uint32_t *          mask,
                                int                      width)
{
    combine_conjoint_general_u (dest, src, mask, width, COMBINE_B_OUT);
}

static void
combine_conjoint_atop_u (pixman_implementation_t *imp,
                         pixman_op_t              op,
                         uint32_t *                dest,
                         const uint32_t *          src,
                         const uint32_t *          mask,
                         int                      width)
{
    combine_conjoint_general_u (dest, src, mask, width, COMBINE_A_ATOP);
}

static void
combine_conjoint_atop_reverse_u (pixman_implementation_t *imp,
                                 pixman_op_t              op,
                                 uint32_t *                dest,
                                 const uint32_t *          src,
                                 const uint32_t *          mask,
                                 int                      width)
{
    combine_conjoint_general_u (dest, src, mask, width, COMBINE_B_ATOP);
}

static void
combine_conjoint_xor_u (pixman_implementation_t *imp,
                        pixman_op_t              op,
                        uint32_t *                dest,
                        const uint32_t *          src,
                        const uint32_t *          mask,
                        int                      width)
{
    combine_conjoint_general_u (dest, src, mask, width, COMBINE_XOR);
}


/* Component alpha combiners */

static void
combine_clear_ca (pixman_implementation_t *imp,
                  pixman_op_t              op,
                  uint32_t *                dest,
                  const uint32_t *          src,
                  const uint32_t *          mask,
                  int                      width)
{
    memset (dest, 0, width * sizeof(uint32_t));
}

static void
combine_src_ca (pixman_implementation_t *imp,
                pixman_op_t              op,
                uint32_t *                dest,
                const uint32_t *          src,
                const uint32_t *          mask,
                int                      width)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t s = *(src + i);
	uint32_t m = *(mask + i);

	combine_mask_value_ca (&s, &m);

	*(dest + i) = s;
    }
}

static void
combine_over_ca (pixman_implementation_t *imp,
                 pixman_op_t              op,
                 uint32_t *                dest,
                 const uint32_t *          src,
                 const uint32_t *          mask,
                 int                      width)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t s = *(src + i);
	uint32_t m = *(mask + i);
	uint32_t a;

	combine_mask_ca (&s, &m);

	a = ~m;
	if (a)
	{
	    uint32_t d = *(dest + i);
	    UN8x4_MUL_UN8x4_ADD_UN8x4 (d, a, s);
	    s = d;
	}

	*(dest + i) = s;
    }
}

static void
combine_over_reverse_ca (pixman_implementation_t *imp,
                         pixman_op_t              op,
                         uint32_t *                dest,
                         const uint32_t *          src,
                         const uint32_t *          mask,
                         int                      width)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t d = *(dest + i);
	uint32_t a = ~d >> A_SHIFT;

	if (a)
	{
	    uint32_t s = *(src + i);
	    uint32_t m = *(mask + i);

	    UN8x4_MUL_UN8x4 (s, m);
	    UN8x4_MUL_UN8_ADD_UN8x4 (s, a, d);

	    *(dest + i) = s;
	}
    }
}

static void
combine_in_ca (pixman_implementation_t *imp,
               pixman_op_t              op,
               uint32_t *                dest,
               const uint32_t *          src,
               const uint32_t *          mask,
               int                      width)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t d = *(dest + i);
	uint16_t a = d >> A_SHIFT;
	uint32_t s = 0;

	if (a)
	{
	    uint32_t m = *(mask + i);

	    s = *(src + i);
	    combine_mask_value_ca (&s, &m);

	    if (a != MASK)
		UN8x4_MUL_UN8 (s, a);
	}

	*(dest + i) = s;
    }
}

static void
combine_in_reverse_ca (pixman_implementation_t *imp,
                       pixman_op_t              op,
                       uint32_t *                dest,
                       const uint32_t *          src,
                       const uint32_t *          mask,
                       int                      width)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t s = *(src + i);
	uint32_t m = *(mask + i);
	uint32_t a;

	combine_mask_alpha_ca (&s, &m);

	a = m;
	if (a != ~0)
	{
	    uint32_t d = 0;

	    if (a)
	    {
		d = *(dest + i);
		UN8x4_MUL_UN8x4 (d, a);
	    }

	    *(dest + i) = d;
	}
    }
}

static void
combine_out_ca (pixman_implementation_t *imp,
                pixman_op_t              op,
                uint32_t *                dest,
                const uint32_t *          src,
                const uint32_t *          mask,
                int                      width)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t d = *(dest + i);
	uint16_t a = ~d >> A_SHIFT;
	uint32_t s = 0;

	if (a)
	{
	    uint32_t m = *(mask + i);

	    s = *(src + i);
	    combine_mask_value_ca (&s, &m);

	    if (a != MASK)
		UN8x4_MUL_UN8 (s, a);
	}

	*(dest + i) = s;
    }
}

static void
combine_out_reverse_ca (pixman_implementation_t *imp,
                        pixman_op_t              op,
                        uint32_t *                dest,
                        const uint32_t *          src,
                        const uint32_t *          mask,
                        int                      width)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t s = *(src + i);
	uint32_t m = *(mask + i);
	uint32_t a;

	combine_mask_alpha_ca (&s, &m);

	a = ~m;
	if (a != ~0)
	{
	    uint32_t d = 0;

	    if (a)
	    {
		d = *(dest + i);
		UN8x4_MUL_UN8x4 (d, a);
	    }

	    *(dest + i) = d;
	}
    }
}

static void
combine_atop_ca (pixman_implementation_t *imp,
                 pixman_op_t              op,
                 uint32_t *                dest,
                 const uint32_t *          src,
                 const uint32_t *          mask,
                 int                      width)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t d = *(dest + i);
	uint32_t s = *(src + i);
	uint32_t m = *(mask + i);
	uint32_t ad;
	uint16_t as = d >> A_SHIFT;

	combine_mask_ca (&s, &m);

	ad = ~m;

	UN8x4_MUL_UN8x4_ADD_UN8x4_MUL_UN8 (d, ad, s, as);

	*(dest + i) = d;
    }
}

static void
combine_atop_reverse_ca (pixman_implementation_t *imp,
                         pixman_op_t              op,
                         uint32_t *                dest,
                         const uint32_t *          src,
                         const uint32_t *          mask,
                         int                      width)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t d = *(dest + i);
	uint32_t s = *(src + i);
	uint32_t m = *(mask + i);
	uint32_t ad;
	uint16_t as = ~d >> A_SHIFT;

	combine_mask_ca (&s, &m);

	ad = m;

	UN8x4_MUL_UN8x4_ADD_UN8x4_MUL_UN8 (d, ad, s, as);

	*(dest + i) = d;
    }
}

static void
combine_xor_ca (pixman_implementation_t *imp,
                pixman_op_t              op,
                uint32_t *                dest,
                const uint32_t *          src,
                const uint32_t *          mask,
                int                      width)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t d = *(dest + i);
	uint32_t s = *(src + i);
	uint32_t m = *(mask + i);
	uint32_t ad;
	uint16_t as = ~d >> A_SHIFT;

	combine_mask_ca (&s, &m);

	ad = ~m;

	UN8x4_MUL_UN8x4_ADD_UN8x4_MUL_UN8 (d, ad, s, as);

	*(dest + i) = d;
    }
}

static void
combine_add_ca (pixman_implementation_t *imp,
                pixman_op_t              op,
                uint32_t *                dest,
                const uint32_t *          src,
                const uint32_t *          mask,
                int                      width)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t s = *(src + i);
	uint32_t m = *(mask + i);
	uint32_t d = *(dest + i);

	combine_mask_value_ca (&s, &m);

	UN8x4_ADD_UN8x4 (d, s);

	*(dest + i) = d;
    }
}

static void
combine_saturate_ca (pixman_implementation_t *imp,
                     pixman_op_t              op,
                     uint32_t *                dest,
                     const uint32_t *          src,
                     const uint32_t *          mask,
                     int                      width)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t s, d;
	uint16_t sa, sr, sg, sb, da;
	uint16_t t, u, v;
	uint32_t m, n, o, p;

	d = *(dest + i);
	s = *(src + i);
	m = *(mask + i);

	combine_mask_ca (&s, &m);

	sa = (m >> A_SHIFT);
	sr = (m >> R_SHIFT) & MASK;
	sg = (m >> G_SHIFT) & MASK;
	sb =  m             & MASK;
	da = ~d >> A_SHIFT;

	if (sb <= da)
	    m = ADD (s, d, 0, t);
	else
	    m = GENERIC (s, d, 0, (da << G_SHIFT) / sb, MASK, t, u, v);

	if (sg <= da)
	    n = ADD (s, d, G_SHIFT, t);
	else
	    n = GENERIC (s, d, G_SHIFT, (da << G_SHIFT) / sg, MASK, t, u, v);

	if (sr <= da)
	    o = ADD (s, d, R_SHIFT, t);
	else
	    o = GENERIC (s, d, R_SHIFT, (da << G_SHIFT) / sr, MASK, t, u, v);

	if (sa <= da)
	    p = ADD (s, d, A_SHIFT, t);
	else
	    p = GENERIC (s, d, A_SHIFT, (da << G_SHIFT) / sa, MASK, t, u, v);

	*(dest + i) = m | n | o | p;
    }
}

static void
combine_disjoint_general_ca (uint32_t *      dest,
                             const uint32_t *src,
                             const uint32_t *mask,
                             int            width,
                             uint8_t        combine)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t s, d;
	uint32_t m, n, o, p;
	uint32_t Fa, Fb;
	uint16_t t, u, v;
	uint32_t sa;
	uint8_t da;

	s = *(src + i);
	m = *(mask + i);
	d = *(dest + i);
	da = d >> A_SHIFT;

	combine_mask_ca (&s, &m);

	sa = m;

	switch (combine & COMBINE_A)
	{
	default:
	    Fa = 0;
	    break;

	case COMBINE_A_OUT:
	    m = (uint32_t)combine_disjoint_out_part ((uint8_t) (sa >> 0), da);
	    n = (uint32_t)combine_disjoint_out_part ((uint8_t) (sa >> G_SHIFT), da) << G_SHIFT;
	    o = (uint32_t)combine_disjoint_out_part ((uint8_t) (sa >> R_SHIFT), da) << R_SHIFT;
	    p = (uint32_t)combine_disjoint_out_part ((uint8_t) (sa >> A_SHIFT), da) << A_SHIFT;
	    Fa = m | n | o | p;
	    break;

	case COMBINE_A_IN:
	    m = (uint32_t)combine_disjoint_in_part ((uint8_t) (sa >> 0), da);
	    n = (uint32_t)combine_disjoint_in_part ((uint8_t) (sa >> G_SHIFT), da) << G_SHIFT;
	    o = (uint32_t)combine_disjoint_in_part ((uint8_t) (sa >> R_SHIFT), da) << R_SHIFT;
	    p = (uint32_t)combine_disjoint_in_part ((uint8_t) (sa >> A_SHIFT), da) << A_SHIFT;
	    Fa = m | n | o | p;
	    break;

	case COMBINE_A:
	    Fa = ~0;
	    break;
	}

	switch (combine & COMBINE_B)
	{
	default:
	    Fb = 0;
	    break;

	case COMBINE_B_OUT:
	    m = (uint32_t)combine_disjoint_out_part (da, (uint8_t) (sa >> 0));
	    n = (uint32_t)combine_disjoint_out_part (da, (uint8_t) (sa >> G_SHIFT)) << G_SHIFT;
	    o = (uint32_t)combine_disjoint_out_part (da, (uint8_t) (sa >> R_SHIFT)) << R_SHIFT;
	    p = (uint32_t)combine_disjoint_out_part (da, (uint8_t) (sa >> A_SHIFT)) << A_SHIFT;
	    Fb = m | n | o | p;
	    break;

	case COMBINE_B_IN:
	    m = (uint32_t)combine_disjoint_in_part (da, (uint8_t) (sa >> 0));
	    n = (uint32_t)combine_disjoint_in_part (da, (uint8_t) (sa >> G_SHIFT)) << G_SHIFT;
	    o = (uint32_t)combine_disjoint_in_part (da, (uint8_t) (sa >> R_SHIFT)) << R_SHIFT;
	    p = (uint32_t)combine_disjoint_in_part (da, (uint8_t) (sa >> A_SHIFT)) << A_SHIFT;
	    Fb = m | n | o | p;
	    break;

	case COMBINE_B:
	    Fb = ~0;
	    break;
	}
	m = GENERIC (s, d, 0, GET_COMP (Fa, 0), GET_COMP (Fb, 0), t, u, v);
	n = GENERIC (s, d, G_SHIFT, GET_COMP (Fa, G_SHIFT), GET_COMP (Fb, G_SHIFT), t, u, v);
	o = GENERIC (s, d, R_SHIFT, GET_COMP (Fa, R_SHIFT), GET_COMP (Fb, R_SHIFT), t, u, v);
	p = GENERIC (s, d, A_SHIFT, GET_COMP (Fa, A_SHIFT), GET_COMP (Fb, A_SHIFT), t, u, v);

	s = m | n | o | p;

	*(dest + i) = s;
    }
}

static void
combine_disjoint_over_ca (pixman_implementation_t *imp,
                          pixman_op_t              op,
                          uint32_t *                dest,
                          const uint32_t *          src,
                          const uint32_t *          mask,
                          int                      width)
{
    combine_disjoint_general_ca (dest, src, mask, width, COMBINE_A_OVER);
}

static void
combine_disjoint_in_ca (pixman_implementation_t *imp,
                        pixman_op_t              op,
                        uint32_t *                dest,
                        const uint32_t *          src,
                        const uint32_t *          mask,
                        int                      width)
{
    combine_disjoint_general_ca (dest, src, mask, width, COMBINE_A_IN);
}

static void
combine_disjoint_in_reverse_ca (pixman_implementation_t *imp,
                                pixman_op_t              op,
                                uint32_t *                dest,
                                const uint32_t *          src,
                                const uint32_t *          mask,
                                int                      width)
{
    combine_disjoint_general_ca (dest, src, mask, width, COMBINE_B_IN);
}

static void
combine_disjoint_out_ca (pixman_implementation_t *imp,
                         pixman_op_t              op,
                         uint32_t *                dest,
                         const uint32_t *          src,
                         const uint32_t *          mask,
                         int                      width)
{
    combine_disjoint_general_ca (dest, src, mask, width, COMBINE_A_OUT);
}

static void
combine_disjoint_out_reverse_ca (pixman_implementation_t *imp,
                                 pixman_op_t              op,
                                 uint32_t *                dest,
                                 const uint32_t *          src,
                                 const uint32_t *          mask,
                                 int                      width)
{
    combine_disjoint_general_ca (dest, src, mask, width, COMBINE_B_OUT);
}

static void
combine_disjoint_atop_ca (pixman_implementation_t *imp,
                          pixman_op_t              op,
                          uint32_t *                dest,
                          const uint32_t *          src,
                          const uint32_t *          mask,
                          int                      width)
{
    combine_disjoint_general_ca (dest, src, mask, width, COMBINE_A_ATOP);
}

static void
combine_disjoint_atop_reverse_ca (pixman_implementation_t *imp,
                                  pixman_op_t              op,
                                  uint32_t *                dest,
                                  const uint32_t *          src,
                                  const uint32_t *          mask,
                                  int                      width)
{
    combine_disjoint_general_ca (dest, src, mask, width, COMBINE_B_ATOP);
}

static void
combine_disjoint_xor_ca (pixman_implementation_t *imp,
                         pixman_op_t              op,
                         uint32_t *                dest,
                         const uint32_t *          src,
                         const uint32_t *          mask,
                         int                      width)
{
    combine_disjoint_general_ca (dest, src, mask, width, COMBINE_XOR);
}

static void
combine_conjoint_general_ca (uint32_t *      dest,
                             const uint32_t *src,
                             const uint32_t *mask,
                             int            width,
                             uint8_t        combine)
{
    int i;

    for (i = 0; i < width; ++i)
    {
	uint32_t s, d;
	uint32_t m, n, o, p;
	uint32_t Fa, Fb;
	uint16_t t, u, v;
	uint32_t sa;
	uint8_t da;

	s = *(src + i);
	m = *(mask + i);
	d = *(dest + i);
	da = d >> A_SHIFT;

	combine_mask_ca (&s, &m);

	sa = m;

	switch (combine & COMBINE_A)
	{
	default:
	    Fa = 0;
	    break;

	case COMBINE_A_OUT:
	    m = (uint32_t)combine_conjoint_out_part ((uint8_t) (sa >> 0), da);
	    n = (uint32_t)combine_conjoint_out_part ((uint8_t) (sa >> G_SHIFT), da) << G_SHIFT;
	    o = (uint32_t)combine_conjoint_out_part ((uint8_t) (sa >> R_SHIFT), da) << R_SHIFT;
	    p = (uint32_t)combine_conjoint_out_part ((uint8_t) (sa >> A_SHIFT), da) << A_SHIFT;
	    Fa = m | n | o | p;
	    break;

	case COMBINE_A_IN:
	    m = (uint32_t)combine_conjoint_in_part ((uint8_t) (sa >> 0), da);
	    n = (uint32_t)combine_conjoint_in_part ((uint8_t) (sa >> G_SHIFT), da) << G_SHIFT;
	    o = (uint32_t)combine_conjoint_in_part ((uint8_t) (sa >> R_SHIFT), da) << R_SHIFT;
	    p = (uint32_t)combine_conjoint_in_part ((uint8_t) (sa >> A_SHIFT), da) << A_SHIFT;
	    Fa = m | n | o | p;
	    break;

	case COMBINE_A:
	    Fa = ~0;
	    break;
	}

	switch (combine & COMBINE_B)
	{
	default:
	    Fb = 0;
	    break;

	case COMBINE_B_OUT:
	    m = (uint32_t)combine_conjoint_out_part (da, (uint8_t) (sa >> 0));
	    n = (uint32_t)combine_conjoint_out_part (da, (uint8_t) (sa >> G_SHIFT)) << G_SHIFT;
	    o = (uint32_t)combine_conjoint_out_part (da, (uint8_t) (sa >> R_SHIFT)) << R_SHIFT;
	    p = (uint32_t)combine_conjoint_out_part (da, (uint8_t) (sa >> A_SHIFT)) << A_SHIFT;
	    Fb = m | n | o | p;
	    break;

	case COMBINE_B_IN:
	    m = (uint32_t)combine_conjoint_in_part (da, (uint8_t) (sa >> 0));
	    n = (uint32_t)combine_conjoint_in_part (da, (uint8_t) (sa >> G_SHIFT)) << G_SHIFT;
	    o = (uint32_t)combine_conjoint_in_part (da, (uint8_t) (sa >> R_SHIFT)) << R_SHIFT;
	    p = (uint32_t)combine_conjoint_in_part (da, (uint8_t) (sa >> A_SHIFT)) << A_SHIFT;
	    Fb = m | n | o | p;
	    break;

	case COMBINE_B:
	    Fb = ~0;
	    break;
	}
	m = GENERIC (s, d, 0, GET_COMP (Fa, 0), GET_COMP (Fb, 0), t, u, v);
	n = GENERIC (s, d, G_SHIFT, GET_COMP (Fa, G_SHIFT), GET_COMP (Fb, G_SHIFT), t, u, v);
	o = GENERIC (s, d, R_SHIFT, GET_COMP (Fa, R_SHIFT), GET_COMP (Fb, R_SHIFT), t, u, v);
	p = GENERIC (s, d, A_SHIFT, GET_COMP (Fa, A_SHIFT), GET_COMP (Fb, A_SHIFT), t, u, v);

	s = m | n | o | p;

	*(dest + i) = s;
    }
}

static void
combine_conjoint_over_ca (pixman_implementation_t *imp,
                          pixman_op_t              op,
                          uint32_t *                dest,
                          const uint32_t *          src,
                          const uint32_t *          mask,
                          int                      width)
{
    combine_conjoint_general_ca (dest, src, mask, width, COMBINE_A_OVER);
}

static void
combine_conjoint_over_reverse_ca (pixman_implementation_t *imp,
                                  pixman_op_t              op,
                                  uint32_t *                dest,
                                  const uint32_t *          src,
                                  const uint32_t *          mask,
                                  int                      width)
{
    combine_conjoint_general_ca (dest, src, mask, width, COMBINE_B_OVER);
}

static void
combine_conjoint_in_ca (pixman_implementation_t *imp,
                        pixman_op_t              op,
                        uint32_t *                dest,
                        const uint32_t *          src,
                        const uint32_t *          mask,
                        int                      width)
{
    combine_conjoint_general_ca (dest, src, mask, width, COMBINE_A_IN);
}

static void
combine_conjoint_in_reverse_ca (pixman_implementation_t *imp,
                                pixman_op_t              op,
                                uint32_t *                dest,
                                const uint32_t *          src,
                                const uint32_t *          mask,
                                int                      width)
{
    combine_conjoint_general_ca (dest, src, mask, width, COMBINE_B_IN);
}

static void
combine_conjoint_out_ca (pixman_implementation_t *imp,
                         pixman_op_t              op,
                         uint32_t *                dest,
                         const uint32_t *          src,
                         const uint32_t *          mask,
                         int                      width)
{
    combine_conjoint_general_ca (dest, src, mask, width, COMBINE_A_OUT);
}

static void
combine_conjoint_out_reverse_ca (pixman_implementation_t *imp,
                                 pixman_op_t              op,
                                 uint32_t *                dest,
                                 const uint32_t *          src,
                                 const uint32_t *          mask,
                                 int                      width)
{
    combine_conjoint_general_ca (dest, src, mask, width, COMBINE_B_OUT);
}

static void
combine_conjoint_atop_ca (pixman_implementation_t *imp,
                          pixman_op_t              op,
                          uint32_t *                dest,
                          const uint32_t *          src,
                          const uint32_t *          mask,
                          int                      width)
{
    combine_conjoint_general_ca (dest, src, mask, width, COMBINE_A_ATOP);
}

static void
combine_conjoint_atop_reverse_ca (pixman_implementation_t *imp,
                                  pixman_op_t              op,
                                  uint32_t *                dest,
                                  const uint32_t *          src,
                                  const uint32_t *          mask,
                                  int                      width)
{
    combine_conjoint_general_ca (dest, src, mask, width, COMBINE_B_ATOP);
}

static void
combine_conjoint_xor_ca (pixman_implementation_t *imp,
                         pixman_op_t              op,
                         uint32_t *                dest,
                         const uint32_t *          src,
                         const uint32_t *          mask,
                         int                      width)
{
    combine_conjoint_general_ca (dest, src, mask, width, COMBINE_XOR);
}

void
_pixman_setup_combiner_functions_32 (pixman_implementation_t *imp)
{
    /* Unified alpha */
    imp->combine_32[PIXMAN_OP_CLEAR] = combine_clear;
    imp->combine_32[PIXMAN_OP_SRC] = combine_src_u;
    imp->combine_32[PIXMAN_OP_DST] = combine_dst;
    imp->combine_32[PIXMAN_OP_OVER] = combine_over_u;
    imp->combine_32[PIXMAN_OP_OVER_REVERSE] = combine_over_reverse_u;
    imp->combine_32[PIXMAN_OP_IN] = combine_in_u;
    imp->combine_32[PIXMAN_OP_IN_REVERSE] = combine_in_reverse_u;
    imp->combine_32[PIXMAN_OP_OUT] = combine_out_u;
    imp->combine_32[PIXMAN_OP_OUT_REVERSE] = combine_out_reverse_u;
    imp->combine_32[PIXMAN_OP_ATOP] = combine_atop_u;
    imp->combine_32[PIXMAN_OP_ATOP_REVERSE] = combine_atop_reverse_u;
    imp->combine_32[PIXMAN_OP_XOR] = combine_xor_u;
    imp->combine_32[PIXMAN_OP_ADD] = combine_add_u;
    imp->combine_32[PIXMAN_OP_SATURATE] = combine_saturate_u;

    /* Disjoint, unified */
    imp->combine_32[PIXMAN_OP_DISJOINT_CLEAR] = combine_clear;
    imp->combine_32[PIXMAN_OP_DISJOINT_SRC] = combine_src_u;
    imp->combine_32[PIXMAN_OP_DISJOINT_DST] = combine_dst;
    imp->combine_32[PIXMAN_OP_DISJOINT_OVER] = combine_disjoint_over_u;
    imp->combine_32[PIXMAN_OP_DISJOINT_OVER_REVERSE] = combine_saturate_u;
    imp->combine_32[PIXMAN_OP_DISJOINT_IN] = combine_disjoint_in_u;
    imp->combine_32[PIXMAN_OP_DISJOINT_IN_REVERSE] = combine_disjoint_in_reverse_u;
    imp->combine_32[PIXMAN_OP_DISJOINT_OUT] = combine_disjoint_out_u;
    imp->combine_32[PIXMAN_OP_DISJOINT_OUT_REVERSE] = combine_disjoint_out_reverse_u;
    imp->combine_32[PIXMAN_OP_DISJOINT_ATOP] = combine_disjoint_atop_u;
    imp->combine_32[PIXMAN_OP_DISJOINT_ATOP_REVERSE] = combine_disjoint_atop_reverse_u;
    imp->combine_32[PIXMAN_OP_DISJOINT_XOR] = combine_disjoint_xor_u;

    /* Conjoint, unified */
    imp->combine_32[PIXMAN_OP_CONJOINT_CLEAR] = combine_clear;
    imp->combine_32[PIXMAN_OP_CONJOINT_SRC] = combine_src_u;
    imp->combine_32[PIXMAN_OP_CONJOINT_DST] = combine_dst;
    imp->combine_32[PIXMAN_OP_CONJOINT_OVER] = combine_conjoint_over_u;
    imp->combine_32[PIXMAN_OP_CONJOINT_OVER_REVERSE] = combine_conjoint_over_reverse_u;
    imp->combine_32[PIXMAN_OP_CONJOINT_IN] = combine_conjoint_in_u;
    imp->combine_32[PIXMAN_OP_CONJOINT_IN_REVERSE] = combine_conjoint_in_reverse_u;
    imp->combine_32[PIXMAN_OP_CONJOINT_OUT] = combine_conjoint_out_u;
    imp->combine_32[PIXMAN_OP_CONJOINT_OUT_REVERSE] = combine_conjoint_out_reverse_u;
    imp->combine_32[PIXMAN_OP_CONJOINT_ATOP] = combine_conjoint_atop_u;
    imp->combine_32[PIXMAN_OP_CONJOINT_ATOP_REVERSE] = combine_conjoint_atop_reverse_u;
    imp->combine_32[PIXMAN_OP_CONJOINT_XOR] = combine_conjoint_xor_u;

    imp->combine_32[PIXMAN_OP_MULTIPLY] = combine_multiply_u;
    imp->combine_32[PIXMAN_OP_SCREEN] = combine_screen_u;
    imp->combine_32[PIXMAN_OP_OVERLAY] = combine_overlay_u;
    imp->combine_32[PIXMAN_OP_DARKEN] = combine_darken_u;
    imp->combine_32[PIXMAN_OP_LIGHTEN] = combine_lighten_u;
    imp->combine_32[PIXMAN_OP_COLOR_DODGE] = combine_color_dodge_u;
    imp->combine_32[PIXMAN_OP_COLOR_BURN] = combine_color_burn_u;
    imp->combine_32[PIXMAN_OP_HARD_LIGHT] = combine_hard_light_u;
    imp->combine_32[PIXMAN_OP_SOFT_LIGHT] = combine_soft_light_u;
    imp->combine_32[PIXMAN_OP_DIFFERENCE] = combine_difference_u;
    imp->combine_32[PIXMAN_OP_EXCLUSION] = combine_exclusion_u;
    imp->combine_32[PIXMAN_OP_HSL_HUE] = combine_hsl_hue_u;
    imp->combine_32[PIXMAN_OP_HSL_SATURATION] = combine_hsl_saturation_u;
    imp->combine_32[PIXMAN_OP_HSL_COLOR] = combine_hsl_color_u;
    imp->combine_32[PIXMAN_OP_HSL_LUMINOSITY] = combine_hsl_luminosity_u;

    /* Component alpha combiners */
    imp->combine_32_ca[PIXMAN_OP_CLEAR] = combine_clear_ca;
    imp->combine_32_ca[PIXMAN_OP_SRC] = combine_src_ca;
    /* dest */
    imp->combine_32_ca[PIXMAN_OP_OVER] = combine_over_ca;
    imp->combine_32_ca[PIXMAN_OP_OVER_REVERSE] = combine_over_reverse_ca;
    imp->combine_32_ca[PIXMAN_OP_IN] = combine_in_ca;
    imp->combine_32_ca[PIXMAN_OP_IN_REVERSE] = combine_in_reverse_ca;
    imp->combine_32_ca[PIXMAN_OP_OUT] = combine_out_ca;
    imp->combine_32_ca[PIXMAN_OP_OUT_REVERSE] = combine_out_reverse_ca;
    imp->combine_32_ca[PIXMAN_OP_ATOP] = combine_atop_ca;
    imp->combine_32_ca[PIXMAN_OP_ATOP_REVERSE] = combine_atop_reverse_ca;
    imp->combine_32_ca[PIXMAN_OP_XOR] = combine_xor_ca;
    imp->combine_32_ca[PIXMAN_OP_ADD] = combine_add_ca;
    imp->combine_32_ca[PIXMAN_OP_SATURATE] = combine_saturate_ca;

    /* Disjoint CA */
    imp->combine_32_ca[PIXMAN_OP_DISJOINT_CLEAR] = combine_clear_ca;
    imp->combine_32_ca[PIXMAN_OP_DISJOINT_SRC] = combine_src_ca;
    imp->combine_32_ca[PIXMAN_OP_DISJOINT_DST] = combine_dst;
    imp->combine_32_ca[PIXMAN_OP_DISJOINT_OVER] = combine_disjoint_over_ca;
    imp->combine_32_ca[PIXMAN_OP_DISJOINT_OVER_REVERSE] = combine_saturate_ca;
    imp->combine_32_ca[PIXMAN_OP_DISJOINT_IN] = combine_disjoint_in_ca;
    imp->combine_32_ca[PIXMAN_OP_DISJOINT_IN_REVERSE] = combine_disjoint_in_reverse_ca;
    imp->combine_32_ca[PIXMAN_OP_DISJOINT_OUT] = combine_disjoint_out_ca;
    imp->combine_32_ca[PIXMAN_OP_DISJOINT_OUT_REVERSE] = combine_disjoint_out_reverse_ca;
    imp->combine_32_ca[PIXMAN_OP_DISJOINT_ATOP] = combine_disjoint_atop_ca;
    imp->combine_32_ca[PIXMAN_OP_DISJOINT_ATOP_REVERSE] = combine_disjoint_atop_reverse_ca;
    imp->combine_32_ca[PIXMAN_OP_DISJOINT_XOR] = combine_disjoint_xor_ca;

    /* Conjoint CA */
    imp->combine_32_ca[PIXMAN_OP_CONJOINT_CLEAR] = combine_clear_ca;
    imp->combine_32_ca[PIXMAN_OP_CONJOINT_SRC] = combine_src_ca;
    imp->combine_32_ca[PIXMAN_OP_CONJOINT_DST] = combine_dst;
    imp->combine_32_ca[PIXMAN_OP_CONJOINT_OVER] = combine_conjoint_over_ca;
    imp->combine_32_ca[PIXMAN_OP_CONJOINT_OVER_REVERSE] = combine_conjoint_over_reverse_ca;
    imp->combine_32_ca[PIXMAN_OP_CONJOINT_IN] = combine_conjoint_in_ca;
    imp->combine_32_ca[PIXMAN_OP_CONJOINT_IN_REVERSE] = combine_conjoint_in_reverse_ca;
    imp->combine_32_ca[PIXMAN_OP_CONJOINT_OUT] = combine_conjoint_out_ca;
    imp->combine_32_ca[PIXMAN_OP_CONJOINT_OUT_REVERSE] = combine_conjoint_out_reverse_ca;
    imp->combine_32_ca[PIXMAN_OP_CONJOINT_ATOP] = combine_conjoint_atop_ca;
    imp->combine_32_ca[PIXMAN_OP_CONJOINT_ATOP_REVERSE] = combine_conjoint_atop_reverse_ca;
    imp->combine_32_ca[PIXMAN_OP_CONJOINT_XOR] = combine_conjoint_xor_ca;

    imp->combine_32_ca[PIXMAN_OP_MULTIPLY] = combine_multiply_ca;
    imp->combine_32_ca[PIXMAN_OP_SCREEN] = combine_screen_ca;
    imp->combine_32_ca[PIXMAN_OP_OVERLAY] = combine_overlay_ca;
    imp->combine_32_ca[PIXMAN_OP_DARKEN] = combine_darken_ca;
    imp->combine_32_ca[PIXMAN_OP_LIGHTEN] = combine_lighten_ca;
    imp->combine_32_ca[PIXMAN_OP_COLOR_DODGE] = combine_color_dodge_ca;
    imp->combine_32_ca[PIXMAN_OP_COLOR_BURN] = combine_color_burn_ca;
    imp->combine_32_ca[PIXMAN_OP_HARD_LIGHT] = combine_hard_light_ca;
    imp->combine_32_ca[PIXMAN_OP_SOFT_LIGHT] = combine_soft_light_ca;
    imp->combine_32_ca[PIXMAN_OP_DIFFERENCE] = combine_difference_ca;
    imp->combine_32_ca[PIXMAN_OP_EXCLUSION] = combine_exclusion_ca;

    /* It is not clear that these make sense, so make them noops for now */
    imp->combine_32_ca[PIXMAN_OP_HSL_HUE] = combine_dst;
    imp->combine_32_ca[PIXMAN_OP_HSL_SATURATION] = combine_dst;
    imp->combine_32_ca[PIXMAN_OP_HSL_COLOR] = combine_dst;
    imp->combine_32_ca[PIXMAN_OP_HSL_LUMINOSITY] = combine_dst;
}
