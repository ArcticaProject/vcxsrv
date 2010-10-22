/*  This file is part of mhmake.
 *
 *  Copyright (C) 2001-2010 marha@sourceforge.net
 *
 *  Mhmake is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Mhmake is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Mhmake.  If not, see <http://www.gnu.org/licenses/>.
*/

/* $Rev$ */

#ifndef _MD5_H
#define _MD5_H

#ifndef uint8
#define uint8  unsigned char
#endif

#ifndef uint32
#define uint32 unsigned long int
#endif

typedef struct
{
    uint32 total[2];
    uint32 state[4];
    uint8 buffer[64];
    uint8 msglen[8];
#ifdef _DEBUG
    string Data;
#endif
}
md5_context;

typedef uint32 md5_val[4];

void md5_starts( md5_context *ctx );
void md5_update( md5_context *ctx, uint8 *input, size_t length );
void md5_finish( md5_context *ctx, uint8 digest[16] );
uint32 *md5_finishbin( md5_context *ctx);
uint32 md5_finish32( md5_context *ctx);

#ifdef _DEBUG
extern bool g_BuildMd5Db;
#endif

#endif /* md5.h */


