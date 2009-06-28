#ifndef __TDA8425_H__
#define __TDA8425_H__

#include "xf86i2c.h"

typedef struct {
	I2CDevRec d;
	
	int mux;
	int stereo;
	int v_left;
	int v_right;
	int bass;
	int treble;
	int src_sel;
	Bool mute;
	} TDA8425Rec, *TDA8425Ptr;

#define TDA8425_ADDR_1   0x82

/* the third parameter is meant to force detection of tda8425.
   This is because tda8425 is write-only and complete implementation
   of I2C protocol is not always available. Besides address there is no good
   way to autodetect it so we have to _know_ it is there anyway */
   
TDA8425Ptr Detect_tda8425(I2CBusPtr b, I2CSlaveAddr addr,Bool force);
Bool tda8425_init(TDA8425Ptr t);
void tda8425_setaudio(TDA8425Ptr t);
void tda8425_mute(TDA8425Ptr t, Bool mute);

#define TDA8425SymbolsList  \
		"Detect_tda8425", \
		"tda8425_init", \
		"tda8425_setaudio", \
		"tda8425_mute"

#define xf86_Detect_tda8425   ((TDA8425Ptr (*)(I2CBusPtr, I2CSlaveAddr,Bool))LoaderSymbol("Detect_tda8425"))
#define xf86_tda8425_init     ((Bool (*)(TDA8425Ptr))LoaderSymbol("tda8425_init"))
#define xf86_tda8425_setaudio ((void (*)(TDA8425Ptr))LoaderSymbol("tda8425_setaudio"))
#define xf86_tda8425_mute     ((void (*)(TDA8425Ptr, Bool))LoaderSymbol("tda8425_mute"))

#endif
