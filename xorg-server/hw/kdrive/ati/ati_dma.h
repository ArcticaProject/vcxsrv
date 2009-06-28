/*
 * Copyright © 2004 Eric Anholt
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Eric Anholt not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Eric Anholt makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * ERIC ANHOLT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ERIC ANHOLT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _ATI_DMA_H_
#define _ATI_DMA_H_

#define CCE_DEBUG 1

#if !CCE_DEBUG
#define DMA_PACKET0(reg, count)						\
	(ATI_CCE_PACKET0 | (((count) - 1) << 16) | ((reg) >> 2))
#else
#define DMA_PACKET0(reg, count)						\
	(__packet0count = (count), __reg = (reg),			\
	ATI_CCE_PACKET0 | (((count) - 1) << 16) | ((reg) >> 2))
#endif
#define DMA_PACKET1(reg1, reg2)						\
	(ATI_CCE_PACKET1 |						\
	(((reg2) >> 2) << ATI_CCE_PACKET1_REG_2_SHIFT) |  ((reg1) >> 2))
#define DMA_PACKET3(type, count)					\
	((type) | (((count) - 1) << 16))

#if !CCE_DEBUG

#define RING_LOCALS	CARD32 *__head; int __count
#define BEGIN_DMA(n)							\
do {									\
	if ((atis->indirectBuffer->used + 4*(n)) >			\
	    atis->indirectBuffer->size) {				\
		ATIFlushIndirect(atis, 1);				\
	}								\
	__head = (CARD32 *)((char *)atis->indirectBuffer->address +	\
	    atis->indirectBuffer->used);				\
	__count = 0;							\
} while (0)
#define END_DMA() do {							\
	atis->indirectBuffer->used += __count * 4;			\
} while (0)

#else

#define RING_LOCALS	\
	CARD32 *__head; int __count, __total, __reg, __packet0count
#define BEGIN_DMA(n)							\
do {									\
	if ((atis->indirectBuffer->used + 4*(n)) >			\
	    atis->indirectBuffer->size) {				\
		ATIFlushIndirect(atis, 1);				\
	}								\
	__head = (CARD32 *)((char *)atis->indirectBuffer->address +	\
	    atis->indirectBuffer->used);				\
	__count = 0;							\
	__total = n;							\
	__reg = 0;								\
	__packet0count = 0;								\
} while (0)
#define END_DMA() do {							\
	if (__count != __total)						\
		FatalError("count != total (%d vs %d) at %s:%d\n",	 \
		     __count, __total, __FILE__, __LINE__);		\
	atis->indirectBuffer->used += __count * 4;			\
} while (0)

#endif

#define OUT_RING(val) do {						\
	__head[__count++] = (val);					\
} while (0)

#define OUT_RING_REG(reg, val) do {					\
	if (__reg != reg)						\
		FatalError("unexpected reg (0x%x vs 0x%x) at %s:%d\n",	\
		    reg, __reg, __FILE__, __LINE__);			\
	if (__packet0count-- <= 0)					\
		FatalError("overrun of packet0 at %s:%d\n",		\
		    __FILE__, __LINE__);				\
	__head[__count++] = (val);					\
	__reg += 4;							\
} while (0)

#define OUT_RING_F(x) OUT_RING(GET_FLOAT_BITS(x))

#define OUT_REG(reg, val)						\
do {									\
	OUT_RING(DMA_PACKET0(reg, 1));					\
	OUT_RING(val);							\
} while (0)

#define TIMEOUT_LOCALS struct timeval _target, _curtime

static inline Bool
tv_le(struct timeval *tv1, struct timeval *tv2)
{
	if (tv1->tv_sec < tv2->tv_sec ||
	    (tv1->tv_sec == tv2->tv_sec && tv1->tv_usec < tv2->tv_usec))
		return TRUE;
	else
		return FALSE;
}

#define WHILE_NOT_TIMEOUT(_timeout)					\
	gettimeofday(&_target, NULL);					\
	_target.tv_usec += ((_timeout) * 1000000);			\
	_target.tv_sec += _target.tv_usec / 1000000;			\
	_target.tv_usec = _target.tv_usec % 1000000;			\
	while (gettimeofday(&_curtime, NULL), tv_le(&_curtime, &_target))

#define TIMEDOUT()	(!tv_le(&_curtime, &_target))

dmaBuf *
ATIGetDMABuffer(ATIScreenInfo *atis);

void
ATIFlushIndirect(ATIScreenInfo *atis, Bool discard);

void
ATIDMASetup(ScreenPtr pScreen);

void
ATIDMATeardown(ScreenPtr pScreen);

#endif /* _ATI_DMA_H_ */
