/* xf86DDC.c 
 * 
 * Copyright 1998,1999 by Egbert Eich <Egbert.Eich@Physik.TU-Darmstadt.DE>
 */
#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "misc.h"
#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86DDC.h"
#include "ddcPriv.h"
#include <string.h>

#define RETRIES 4

static unsigned char *EDIDRead_DDC1(
    ScrnInfoPtr pScrn,
    DDC1SetSpeedProc,
    unsigned int (*)(ScrnInfoPtr)
);

static Bool TestDDC1(
    ScrnInfoPtr pScrn,
    unsigned int (*)(ScrnInfoPtr)
);

static unsigned int *FetchEDID_DDC1(
    ScrnInfoPtr,
    register unsigned int (*)(ScrnInfoPtr)
);

static unsigned char* EDID1Read_DDC2(
    int scrnIndex, 
    I2CBusPtr pBus
);

static unsigned char * DDCRead_DDC2(
    int scrnIndex,
    I2CBusPtr pBus, 
    int start, 
    int len
);

typedef enum {
    DDCOPT_NODDC1,
    DDCOPT_NODDC2,
    DDCOPT_NODDC
} DDCOpts;

static const OptionInfoRec DDCOptions[] = {
    { DDCOPT_NODDC1,	"NoDDC1",	OPTV_BOOLEAN,	{0},	FALSE },
    { DDCOPT_NODDC2,	"NoDDC2",	OPTV_BOOLEAN,	{0},	FALSE },
    { DDCOPT_NODDC,	"NoDDC",	OPTV_BOOLEAN,	{0},	FALSE },
    { -1,		NULL,		OPTV_NONE,	{0},	FALSE },
};

/**
 * Attempts to probe the monitor for EDID information, if NoDDC and NoDDC1 are
 * unset.  EDID information blocks are interpreted and the results returned in
 * an xf86MonPtr.
 *
 * This function does not affect the list of modes used by drivers -- it is up
 * to the driver to decide policy on what to do with EDID information.
 *
 * @return pointer to a new xf86MonPtr containing the EDID information.
 * @return NULL if no monitor attached or failure to interpret the EDID.
 */
xf86MonPtr 
xf86DoEDID_DDC1(
    int scrnIndex, DDC1SetSpeedProc DDC1SetSpeed, 
    unsigned int (*DDC1Read)(ScrnInfoPtr)
)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    unsigned char *EDID_block = NULL;
    xf86MonPtr tmp = NULL;
    int sigio;
    /* Default DDC and DDC1 to enabled. */
    Bool noddc = FALSE, noddc1 = FALSE;
    OptionInfoPtr options;

    options = xnfalloc(sizeof(DDCOptions));
    (void)memcpy(options, DDCOptions, sizeof(DDCOptions));
    xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, options);

    xf86GetOptValBool(options, DDCOPT_NODDC, &noddc);
    xf86GetOptValBool(options, DDCOPT_NODDC1, &noddc1);
    xfree(options);
    
    if (noddc || noddc1)
	return NULL;
    
    sigio = xf86BlockSIGIO();
    EDID_block = EDIDRead_DDC1(pScrn,DDC1SetSpeed,DDC1Read);
    xf86UnblockSIGIO(sigio);

    if (EDID_block){
	tmp = xf86InterpretEDID(scrnIndex,EDID_block);
    }
#ifdef DEBUG
	else ErrorF("No EDID block returned\n");
    if (!tmp)
	ErrorF("Cannot interpret EDID block\n");
#endif
	return tmp;
}

/**
 * Attempts to probe the monitor for EDID information, if NoDDC and NoDDC2 are
 * unset.  EDID information blocks are interpreted and the results returned in
 * an xf86MonPtr.
 *
 * This function does not affect the list of modes used by drivers -- it is up
 * to the driver to decide policy on what to do with EDID information.
 *
 * @return pointer to a new xf86MonPtr containing the EDID information.
 * @return NULL if no monitor attached or failure to interpret the EDID.
 */
xf86MonPtr
xf86DoEDID_DDC2(int scrnIndex, I2CBusPtr pBus)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    unsigned char *EDID_block = NULL;
    xf86MonPtr tmp = NULL;
    /* Default DDC and DDC2 to enabled. */
    Bool noddc = FALSE, noddc2 = FALSE;
    OptionInfoPtr options;

    options = xnfalloc(sizeof(DDCOptions));
    (void)memcpy(options, DDCOptions, sizeof(DDCOptions));
    xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, options);

    xf86GetOptValBool(options, DDCOPT_NODDC, &noddc);
    xf86GetOptValBool(options, DDCOPT_NODDC2, &noddc2);
    xfree(options);
    
    if (noddc || noddc2)
	return NULL;

    EDID_block = EDID1Read_DDC2(scrnIndex,pBus);

    if (EDID_block){
	tmp = xf86InterpretEDID(scrnIndex,EDID_block);
    } else {
#ifdef DEBUG
	ErrorF("No EDID block returned\n");
#endif
	return NULL;
    }
#ifdef DEBUG
    if (!tmp)
	ErrorF("Cannot interpret EDID block\n");
    else
        ErrorF("Sections to follow: %i\n",tmp->no_sections);
#endif
    
    return tmp;
}

/* 
 * read EDID record , pass it to callback function to interpret.
 * callback function will store it for further use by calling
 * function; it will also decide if we need to reread it 
 */
static unsigned char *
EDIDRead_DDC1(ScrnInfoPtr pScrn, DDC1SetSpeedProc DDCSpeed, 
              unsigned int (*read_DDC)(ScrnInfoPtr))
{
    unsigned char *EDID_block = NULL;
    int count = RETRIES;

    if (!read_DDC) { 
	xf86DrvMsg(pScrn->scrnIndex, X_PROBED, 
		   "chipset doesn't support DDC1\n");
	return NULL; 
    };

    if (TestDDC1(pScrn,read_DDC)==-1) { 
	xf86DrvMsg(pScrn->scrnIndex, X_PROBED, "No DDC signal\n"); 
	return NULL; 
    };

    if (DDCSpeed) DDCSpeed(pScrn,DDC_FAST);
    do {
	EDID_block = GetEDID_DDC1(FetchEDID_DDC1(pScrn,read_DDC)); 
	count --;
    } while (!EDID_block && count);
    if (DDCSpeed) DDCSpeed(pScrn,DDC_SLOW);

    return EDID_block;
}

/* test if DDC1  return 0 if not */
static Bool
TestDDC1(ScrnInfoPtr pScrn, unsigned int (*read_DDC)(ScrnInfoPtr))
{
    int old, count;

    old = read_DDC(pScrn);
    count = HEADER * BITS_PER_BYTE;
    do {
	/* wait for next retrace */
	if (old != read_DDC(pScrn)) break;
    } while(count--);
    return (count);
}

/* fetch entire EDID record; DDC bit needs to be masked */
static unsigned int * 
FetchEDID_DDC1(register ScrnInfoPtr pScrn,
	       register unsigned int (*read_DDC)(ScrnInfoPtr))
{
    int count = NUM;
    unsigned int *ptr, *xp;

    ptr=xp=xalloc(sizeof(int)*NUM); 

    if (!ptr)  return NULL;
    do {
	/* wait for next retrace */
	*xp = read_DDC(pScrn);
	xp++;
    } while(--count);
    return (ptr);
}

static unsigned char*
EDID1Read_DDC2(int scrnIndex, I2CBusPtr pBus)
{
    return  DDCRead_DDC2(scrnIndex, pBus, 0, EDID1_LEN);
}

static unsigned char *
DDCRead_DDC2(int scrnIndex, I2CBusPtr pBus, int start, int len)
{
    I2CDevPtr dev;
    unsigned char W_Buffer[2];
    int w_bytes;
    unsigned char *R_Buffer;
    int i;
    
    /*
     * Slow down the bus so that older monitors don't 
     * miss things.
     */
    pBus->RiseFallTime = 20;
    
    if (!(dev = xf86I2CFindDev(pBus, 0x00A0))) {
	dev = xf86CreateI2CDevRec();
	dev->DevName = "ddc2";
	dev->SlaveAddr = 0xA0;
	dev->ByteTimeout = 2200; /* VESA DDC spec 3 p. 43 (+10 %) */
	dev->StartTimeout = 550;
	dev->BitTimeout = 40;
	dev->AcknTimeout = 40;

	dev->pI2CBus = pBus;
	if (!xf86I2CDevInit(dev)) {
	    xf86DrvMsg(scrnIndex, X_PROBED, "No DDC2 device\n");
	    return NULL;
	}
    }
    if (start < 0x100) {
	w_bytes = 1;
	W_Buffer[0] = start;
    } else {
	w_bytes = 2;
	W_Buffer[0] = start & 0xFF;
	W_Buffer[1] = (start & 0xFF00) >> 8;
    }
    R_Buffer = xcalloc(1,sizeof(unsigned char) 
					* (len));
    for (i=0; i<RETRIES; i++) {
	if (xf86I2CWriteRead(dev, W_Buffer,w_bytes, R_Buffer,len)) {
	    if (!DDC_checksum(R_Buffer,len))
		return R_Buffer;

#ifdef DEBUG
	    else ErrorF("Checksum error in EDID block\n");
#endif
	}
#ifdef DEBUG
	else ErrorF("Error reading EDID block\n");
#endif
    }
    
    xf86DestroyI2CDevRec(dev,TRUE);
    xfree(R_Buffer);
    return NULL;
}
