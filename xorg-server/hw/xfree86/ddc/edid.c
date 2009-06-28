
/* edid.c:  retrieve EDID record from raw DDC1 data stream: data 
 * is contained in an array of unsigned int each unsigned int 
 * contains one bit if bit is 0 unsigned int has to be zero else 
 * unsigned int > 0 
 * 
 * Copyright 1998 by Egbert Eich <Egbert.Eich@Physik.TU-Darmstadt.DE>
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

static int find_start(unsigned int *);
static unsigned char * find_header(unsigned char *);
static unsigned char * resort(unsigned char *);

unsigned char *
GetEDID_DDC1(unsigned int *s_ptr)
{
    unsigned char *d_block, *d_pos;
    unsigned int *s_pos, *s_end;
    int s_start;
    int i,j;
    s_start = find_start(s_ptr);
    if (s_start==-1) return NULL;
    s_end = s_ptr + NUM;
    s_pos = s_ptr + s_start;
    d_block=xalloc(EDID1_LEN);
    if (!d_block) return NULL;
    d_pos = d_block;
    for (i=0;i<EDID1_LEN;i++) {
	for (j=0;j<8;j++) {
	    *d_pos <<= 1;
	    if (*s_pos) {
		*d_pos |= 0x01;
	    }
	    s_pos++; if (s_pos == s_end) s_pos=s_ptr;
	};
	s_pos++; if (s_pos == s_end) s_pos=s_ptr;
	d_pos++;
    }
    xfree(s_ptr);
    if (d_block && DDC_checksum(d_block,EDID1_LEN)) return NULL;
    return (resort(d_block));
}

int
DDC_checksum(unsigned char *block, int len)
{
    int i, result = 0;
    int not_null = 0;
    
    for (i=0;i<len;i++) {
	not_null |= block[i];
	result += block[i];
    }
    
#ifdef DEBUG
    if (result & 0xFF) ErrorF("DDC checksum not correct\n");
    if (!not_null) ErrorF("DDC read all Null\n");
#endif

    /* catch the trivial case where all bytes are 0 */
    if (!not_null) return 1;

    return (result&0xFF);
}

static int
find_start(unsigned int *ptr)
{
    unsigned int comp[9], test[9];
    int i,j;
  
    for (i=0;i<9;i++){
	comp[i] = *(ptr++);
	test[i] = 1;
    }
    for (i=0;i<127;i++){
	for (j=0;j<9;j++){
	    test[j] = test[j] & !(comp[j] ^ *(ptr++));
	}
    }
    for (i=0;i<9;i++)
	if (test[i]) return (i+1);
    return (-1);
}

static unsigned char *
find_header(unsigned char *block)
{
    unsigned char *ptr, *head_ptr, *end;
    unsigned char header[]={0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00};
 
    ptr = block;
    end = block + EDID1_LEN;
    while (ptr<end) {
	int i;
	head_ptr = ptr;
	for (i=0;i<8;i++){
	    if (header[i] != *(head_ptr++)) break;
	    if (head_ptr == end) head_ptr = block;
	}
	if (i==8) break;
	ptr++; 
    }
    if (ptr == end) return (NULL);
    return (ptr);
}

static unsigned char *
resort(unsigned char *s_block)
{
    unsigned char *d_new, *d_ptr, *d_end, *s_ptr, *s_end;
    unsigned char tmp;

    s_end = s_block + EDID1_LEN;
    d_new = xalloc(EDID1_LEN);
    if (!d_new) return NULL;
    d_end = d_new + EDID1_LEN;

    s_ptr = find_header(s_block);
    if (!s_ptr) return NULL;
    for (d_ptr=d_new;d_ptr<d_end;d_ptr++){
	tmp = *(s_ptr++);
	*d_ptr = tmp; 
	if (s_ptr == s_end) s_ptr = s_block;
    }
    xfree(s_block);
    return (d_new);
}


