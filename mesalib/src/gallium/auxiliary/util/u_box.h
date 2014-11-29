#ifndef UTIL_BOX_INLINES_H
#define UTIL_BOX_INLINES_H

#include "pipe/p_state.h"
#include "util/u_math.h"

static INLINE
void u_box_1d( unsigned x,
	       unsigned w,
	       struct pipe_box *box )
{
   box->x = x;
   box->y = 0;
   box->z = 0;
   box->width = w;
   box->height = 1;
   box->depth = 1;
}

static INLINE
void u_box_2d( unsigned x,
	       unsigned y,
	       unsigned w,
	       unsigned h,
	       struct pipe_box *box )
{
   box->x = x;
   box->y = y;
   box->z = 0;
   box->width = w;
   box->height = h;
   box->depth = 1;
}

static INLINE
void u_box_origin_2d( unsigned w,
		      unsigned h,
		      struct pipe_box *box )
{
   box->x = 0;
   box->y = 0;
   box->z = 0;
   box->width = w;
   box->height = h;
   box->depth = 1;
}

static INLINE
void u_box_2d_zslice( unsigned x,
		      unsigned y,
		      unsigned z,
		      unsigned w,
		      unsigned h,
		      struct pipe_box *box )
{
   box->x = x;
   box->y = y;
   box->z = z;
   box->width = w;
   box->height = h;
   box->depth = 1;
}

static INLINE
void u_box_3d( unsigned x,
	       unsigned y,
	       unsigned z,
	       unsigned w,
	       unsigned h,
	       unsigned d,
	       struct pipe_box *box )
{
   box->x = x;
   box->y = y;
   box->z = z;
   box->width = w;
   box->height = h;
   box->depth = d;
}

/* Clips @dst to width @w and height @h.
 * Returns -1 if the resulting box would be empty (then @dst is left unchanged).
 *          0 if nothing has been reduced.
 *          1 if width has been reduced.
 *          2 if height has been reduced.
 *          3 if both width and height have been reduced.
 * Aliasing permitted.
 */
static INLINE int
u_box_clip_2d(struct pipe_box *dst,
              const struct pipe_box *box, int w, int h)
{
   unsigned i;
   int a[2], b[2], dim[2];
   int *start, *end;
   int res = 0;

   if (!box->width || !box->height)
      return -1;
   dim[0] = w;
   dim[1] = h;
   a[0] = box->x;
   a[1] = box->y;
   b[0] = box->x + box->width;
   b[1] = box->y + box->height;

   for (i = 0; i < 2; ++i) {
      start = (a[i] <= b[i]) ? &a[i] : &b[i];
      end = (a[i] <= b[i]) ? &b[i] : &a[i];

      if (*end < 0 || *start >= dim[i])
         return -1;
      if (*start < 0) {
         *start = 0;
         res |= (1 << i);
      }
      if (*end > dim[i]) {
         *end = dim[i];
         res |= (1 << i);
      }
   }

   if (res) {
      dst->x = a[0];
      dst->y = a[1];
      dst->width = b[0] - a[0];
      dst->height = b[1] - a[1];
   }
   return res;
}

static INLINE int64_t
u_box_volume_3d(const struct pipe_box *box)
{
   return (int64_t)box->width * box->height * box->depth;
}

/* Aliasing of @dst permitted. */
static INLINE void
u_box_union_2d(struct pipe_box *dst,
               const struct pipe_box *a, const struct pipe_box *b)
{
   dst->x = MIN2(a->x, b->x);
   dst->y = MIN2(a->y, b->y);

   dst->width = MAX2(a->x + a->width, b->x + b->width) - dst->x;
   dst->height = MAX2(a->y + a->height, b->y + b->height) - dst->y;
}

/* Aliasing of @dst permitted. */
static INLINE void
u_box_union_3d(struct pipe_box *dst,
               const struct pipe_box *a, const struct pipe_box *b)
{
   dst->x = MIN2(a->x, b->x);
   dst->y = MIN2(a->y, b->y);
   dst->z = MIN2(a->z, b->z);

   dst->width = MAX2(a->x + a->width, b->x + b->width) - dst->x;
   dst->height = MAX2(a->y + a->height, b->y + b->height) - dst->y;
   dst->depth = MAX2(a->z + a->depth, b->z + b->depth) - dst->z;
}

static INLINE boolean
u_box_test_intersection_2d(const struct pipe_box *a,
                           const struct pipe_box *b)
{
   unsigned i;
   int a_l[2], a_r[2], b_l[2], b_r[2];

   a_l[0] = MIN2(a->x, a->x + a->width);
   a_r[0] = MAX2(a->x, a->x + a->width);
   a_l[1] = MIN2(a->y, a->y + a->height);
   a_r[1] = MAX2(a->y, a->y + a->height);

   b_l[0] = MIN2(b->x, b->x + b->width);
   b_r[0] = MAX2(b->x, b->x + b->width);
   b_l[1] = MIN2(b->y, b->y + b->height);
   b_r[1] = MAX2(b->y, b->y + b->height);

   for (i = 0; i < 2; ++i) {
      if (a_l[i] > b_r[i] || a_r[i] < b_l[i])
         return FALSE;
   }
   return TRUE;
}

static INLINE void
u_box_minify_2d(struct pipe_box *dst,
                const struct pipe_box *src, unsigned l)
{
   dst->x = src->x >> l;
   dst->y = src->y >> l;
   dst->width = MAX2(src->width >> l, 1);
   dst->height = MAX2(src->height >> l, 1);
}

#endif
