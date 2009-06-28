/*
 * Memory range attribute operations, peformed on /dev/mem
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#ifndef _MEMRANGE_H
#define _MEMRANGE_H

/* Memory range attributes */
#define MDF_UNCACHEABLE		(1<<0)	/* region not cached */
#define MDF_WRITECOMBINE	(1<<1)	/* region supports "write combine"
					 * action */
#define MDF_WRITETHROUGH	(1<<2)	/* write-through cached */
#define MDF_WRITEBACK		(1<<3)	/* write-back cached */
#define MDF_WRITEPROTECT	(1<<4)	/* read-only region */
#define MDF_ATTRMASK		(0x00ffffff)

#define MDF_FIXBASE		(1<<24)	/* fixed base */
#define MDF_FIXLEN		(1<<25)	/* fixed length */
#define MDF_FIRMWARE		(1<<26)	/* set by firmware (XXX not useful?) */
#define MDF_ACTIVE		(1<<27)	/* currently active */
#define MDF_BOGUS		(1<<28)	/* we don't like it */
#define MDF_FIXACTIVE		(1<<29)	/* can't be turned off */
#define MDF_BUSY		(1<<30)	/* range is in use */

struct mem_range_desc {
	u_int64_t mr_base;
	u_int64_t mr_len;
	int     mr_flags;
	char    mr_owner[8];
};

struct mem_range_op {
	struct mem_range_desc *mo_desc;
	int     mo_arg[2];
#define MEMRANGE_SET_UPDATE	0
#define MEMRANGE_SET_REMOVE	1
	/* XXX want a flag that says "set and undo when I exit" */
};
#define MEMRANGE_GET	_IOWR('m', 50, struct mem_range_op)
#define MEMRANGE_SET	_IOW('m', 51, struct mem_range_op)

#ifdef _KERNEL

struct mem_range_softc;
struct mem_range_ops {
	void    (*init) __P((struct mem_range_softc * sc));
	int     (*set) __P((struct mem_range_softc * sc, struct mem_range_desc * mrd, int *arg));
	void    (*initAP) __P((struct mem_range_softc * sc));
};

struct mem_range_softc {
	struct mem_range_ops *mr_op;
	int     mr_cap;
	int     mr_ndesc;
	struct mem_range_desc *mr_desc;
};

extern struct mem_range_softc mem_range_softc;

extern int mem_range_attr_get __P((struct mem_range_desc * mrd, int *arg));
extern int mem_range_attr_set __P((struct mem_range_desc * mrd, int *arg));
extern void mem_range_AP_init __P((void));
#endif

#endif
