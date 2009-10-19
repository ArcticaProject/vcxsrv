
#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <stdio.h>
#include <X11/X.h>
#include "os.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "shared/xf86Axp.h"

axpDevice lnxGetAXP(void);

typedef struct 
 { char* sysName; 
   char* sysVari; 
   char* cpu; 
   axpDevice sys; }
AXP;

static AXP axpList[] = {
  { "Tsunami", NULL, NULL, TSUNAMI },
  { "Eiger", NULL, NULL, TSUNAMI }, 
  {"Noname", NULL, NULL, LCA },
  { "AlphaBook1", NULL, NULL, LCA }, 
  {"EB66", NULL, NULL, LCA}, 
  {"EB64+",NULL,NULL, APECS}, 
  {"Noritake",NULL,"EV5",CIA},
  {"Noritake",NULL,"EV56",CIA},
  {"Noritake",NULL,NULL,APECS},
  {"XL",NULL,NULL,APECS},              
  {"Avanti",NULL,NULL,APECS},
  {"Mikasa",NULL,"EV5",CIA},
  {"Mikasa",NULL,"EV56",CIA},
  {"Mikasa",NULL,NULL,APECS},
  {"EB164","EB164",NULL,CIA},
  {"EB164","PC164", NULL,CIA},
  {"EB164","LX164",NULL, PYXIS},
  {"EB164","SX164",NULL, PYXIS},
  {"EB164","RX164",NULL, POLARIS},
  {"Alcor",NULL,NULL,CIA},
  {"Takara",NULL,NULL,CIA},
  {"Sable",NULL, "EV5",T2_GAMMA},
  {"Sable",NULL,"EV56",T2_GAMMA},
  {"Sable",NULL,NULL,T2},
  {"Rawhide",NULL,NULL,MCPCIA},
  {"Jensen",NULL,NULL,JENSEN},
  {"Miata",NULL,NULL,PYXIS_CIA},
  {"Ruffian",NULL,NULL,PYXIS_CIA},
  {"Nautilus",NULL,NULL,IRONGATE},
  {NULL,NULL,NULL,SYS_NONE}
};


axpDevice
lnxGetAXP(void)
{
  FILE *file;
  int count = 0;
  char res[256];
  char cpu[255];
  char systype[255];
  char sysvari[255];
  if (!(file = fopen("/proc/cpuinfo","r")))
    return SYS_NONE;
  do {
    if (!fgets(res,0xff,file)) return SYS_NONE;
    switch (count) {
    case 1:
      sscanf(res, "cpu model : %s",cpu);
      DebugF("CPU %s\n",cpu);
      break;
    case 5:
      sscanf(res, "system type : %s",systype);
      DebugF("system type : %s\n",systype);
      break;
    case 6:
      sscanf(res, "system variation : %s",sysvari);
      DebugF("system variation: %s\n",sysvari);
      break;
    }
    count++;
  } while (count < 8);
  
  fclose(file);
  
  count = 0;
  
  do {
    if (!axpList[count].sysName || !strcmp(axpList[count].sysName,systype)) {
      if (axpList[count].sysVari && strcmp(axpList[count].sysVari,sysvari)) {
	count++;
	continue;
      };
      if (axpList[count].cpu && strcmp(axpList[count].cpu,cpu)) {
	count++;
	continue;
      }
      return axpList[count].sys;
    } 
	count++;
  } while (1);
}

/*
 * pciconfig_iobase wrappers and dynamic i/o selection
 */
#include "lnx.h"
#include <unistd.h>
#include <errno.h>

/* glibc versions (single hose only) */
extern void _outb(char val, unsigned long port);
extern void _outw(short val, unsigned long port);
extern void _outl(int val, unsigned long port);
extern unsigned int _inb(unsigned long port);
extern unsigned int _inw(unsigned long port);
extern unsigned int _inl(unsigned long port);

extern void _dense_outb(char, unsigned long);
extern void _dense_outw(short, unsigned long);
extern void _dense_outl(int, unsigned long);
extern unsigned int _dense_inb(unsigned long);
extern unsigned int _dense_inw(unsigned long);
extern unsigned int _dense_inl(unsigned long);

_X_EXPORT void (*_alpha_outb)(char, unsigned long) = _outb;
_X_EXPORT void (*_alpha_outw)(short, unsigned long) = _outw;
_X_EXPORT void (*_alpha_outl)(int, unsigned long) = _outl;
_X_EXPORT unsigned int (*_alpha_inb)(unsigned long) = _inb;
_X_EXPORT unsigned int (*_alpha_inw)(unsigned long) = _inw;
_X_EXPORT unsigned int (*_alpha_inl)(unsigned long) = _inl;

static long _alpha_iobase_query(unsigned, int, int, int);
long (*_iobase)(unsigned, int, int, int) = _alpha_iobase_query;

static long
_alpha_iobase(unsigned flags, int hose, int bus, int devfn)
{
  if (bus < 0) {
    bus = hose;
    flags |= IOBASE_FROM_HOSE;
  }

  return syscall(__NR_pciconfig_iobase, flags, bus, devfn);
}

static long
_alpha_iobase_legacy(unsigned flags, int hose, int bus, int devfn)
{
  if (hose > 0) return -ENODEV;
  if (flags & IOBASE_DENSE_MEM) return _bus_base();
  if (flags & IOBASE_SPARSE_MEM) return _bus_base_sparse();
  return 0;
}

static long 
_alpha_iobase_query(unsigned flags, int hose, int bus, int devfn)
{
  /*
   * Only use iobase if the syscall is supported *and* it's
   * a dense io system
   */
  if (_alpha_iobase(IOBASE_DENSE_IO, 0, 0, 0) > 0) {
    /*
     * The syscall worked and it's a dense io system - take over the
     * io subsystem
     */
    _iobase = _alpha_iobase;

    /* 
     * Only take over the inx/outx functions if this is a dense I/O
     * system *and* addressing domains are being used. The dense I/O
     * routines expect I/O to be mapped (as done in xf86MapLegacyIO)
     */
    _alpha_outb = _dense_outb;
    _alpha_outw = _dense_outw;
    _alpha_outl = _dense_outl;
    _alpha_inb = _dense_inb;
    _alpha_inw = _dense_inw;
    _alpha_inl = _dense_inl;
  } else _iobase = _alpha_iobase_legacy;

  return _iobase(flags, hose, bus, devfn);
}

