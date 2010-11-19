/*  This file is part of mhmake.
 *
 *  Copyright (C) 2001-2009 Marc Haesen
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

/*
 *  RFC 1321 compliant MD5 implementation
 *
 *  Copyright (C) 2001-2003  Christophe Devine
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "stdafx.h"
#include "md5.h"

#ifdef _MSC_VER
#pragma warning (disable:4005) /* macro redefinition */
#endif

#ifdef _DEBUG
map<uint32,string> g_Md5Database;
#endif

#define GET_UINT32(n,b,i)                     \
{                                             \
  (n) = ( (uint32) (b)[(i)    ]       )       \
      | ( (uint32) (b)[(i) + 1] <<  8 )       \
      | ( (uint32) (b)[(i) + 2] << 16 )       \
      | ( (uint32) (b)[(i) + 3] << 24 );      \
}

#define PUT_UINT32(n,b,i)                     \
{                                             \
  (b)[(i)    ] = (uint8) ( (n)       );       \
  (b)[(i) + 1] = (uint8) ( (n) >>  8 );       \
  (b)[(i) + 2] = (uint8) ( (n) >> 16 );       \
  (b)[(i) + 3] = (uint8) ( (n) >> 24 );       \
}

void md5_starts( md5_context *ctx )
{
  ctx->total[0] = 0;
  ctx->total[1] = 0;

  ctx->state[0] = 0x67452301;
  ctx->state[1] = 0xEFCDAB89;
  ctx->state[2] = 0x98BADCFE;
  ctx->state[3] = 0x10325476;
#ifdef _DEBUG
  if (g_BuildMd5Db) ctx->Data="";
#endif
}

void md5_process( md5_context *ctx, uint8 data[64] )
{
  uint32 X[16], A, B, C, D;

  GET_UINT32( X[0],  data,  0 );
  GET_UINT32( X[1],  data,  4 );
  GET_UINT32( X[2],  data,  8 );
  GET_UINT32( X[3],  data, 12 );
  GET_UINT32( X[4],  data, 16 );
  GET_UINT32( X[5],  data, 20 );
  GET_UINT32( X[6],  data, 24 );
  GET_UINT32( X[7],  data, 28 );
  GET_UINT32( X[8],  data, 32 );
  GET_UINT32( X[9],  data, 36 );
  GET_UINT32( X[10], data, 40 );
  GET_UINT32( X[11], data, 44 );
  GET_UINT32( X[12], data, 48 );
  GET_UINT32( X[13], data, 52 );
  GET_UINT32( X[14], data, 56 );
  GET_UINT32( X[15], data, 60 );

#define S(x,n) ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)))

#define P(a,b,c,d,k,s,t)                              \
{                                                     \
  a += F(b,c,d) + X[k] + t; a = S(a,s) + b;           \
}

  A = ctx->state[0];
  B = ctx->state[1];
  C = ctx->state[2];
  D = ctx->state[3];

#define F(x,y,z) (z ^ (x & (y ^ z)))

  P( A, B, C, D,  0,  7, 0xD76AA478 );
  P( D, A, B, C,  1, 12, 0xE8C7B756 );
  P( C, D, A, B,  2, 17, 0x242070DB );
  P( B, C, D, A,  3, 22, 0xC1BDCEEE );
  P( A, B, C, D,  4,  7, 0xF57C0FAF );
  P( D, A, B, C,  5, 12, 0x4787C62A );
  P( C, D, A, B,  6, 17, 0xA8304613 );
  P( B, C, D, A,  7, 22, 0xFD469501 );
  P( A, B, C, D,  8,  7, 0x698098D8 );
  P( D, A, B, C,  9, 12, 0x8B44F7AF );
  P( C, D, A, B, 10, 17, 0xFFFF5BB1 );
  P( B, C, D, A, 11, 22, 0x895CD7BE );
  P( A, B, C, D, 12,  7, 0x6B901122 );
  P( D, A, B, C, 13, 12, 0xFD987193 );
  P( C, D, A, B, 14, 17, 0xA679438E );
  P( B, C, D, A, 15, 22, 0x49B40821 );

#undef F

#define F(x,y,z) (y ^ (z & (x ^ y)))

  P( A, B, C, D,  1,  5, 0xF61E2562 );
  P( D, A, B, C,  6,  9, 0xC040B340 );
  P( C, D, A, B, 11, 14, 0x265E5A51 );
  P( B, C, D, A,  0, 20, 0xE9B6C7AA );
  P( A, B, C, D,  5,  5, 0xD62F105D );
  P( D, A, B, C, 10,  9, 0x02441453 );
  P( C, D, A, B, 15, 14, 0xD8A1E681 );
  P( B, C, D, A,  4, 20, 0xE7D3FBC8 );
  P( A, B, C, D,  9,  5, 0x21E1CDE6 );
  P( D, A, B, C, 14,  9, 0xC33707D6 );
  P( C, D, A, B,  3, 14, 0xF4D50D87 );
  P( B, C, D, A,  8, 20, 0x455A14ED );
  P( A, B, C, D, 13,  5, 0xA9E3E905 );
  P( D, A, B, C,  2,  9, 0xFCEFA3F8 );
  P( C, D, A, B,  7, 14, 0x676F02D9 );
  P( B, C, D, A, 12, 20, 0x8D2A4C8A );

#undef F

#define F(x,y,z) (x ^ y ^ z)

  P( A, B, C, D,  5,  4, 0xFFFA3942 );
  P( D, A, B, C,  8, 11, 0x8771F681 );
  P( C, D, A, B, 11, 16, 0x6D9D6122 );
  P( B, C, D, A, 14, 23, 0xFDE5380C );
  P( A, B, C, D,  1,  4, 0xA4BEEA44 );
  P( D, A, B, C,  4, 11, 0x4BDECFA9 );
  P( C, D, A, B,  7, 16, 0xF6BB4B60 );
  P( B, C, D, A, 10, 23, 0xBEBFBC70 );
  P( A, B, C, D, 13,  4, 0x289B7EC6 );
  P( D, A, B, C,  0, 11, 0xEAA127FA );
  P( C, D, A, B,  3, 16, 0xD4EF3085 );
  P( B, C, D, A,  6, 23, 0x04881D05 );
  P( A, B, C, D,  9,  4, 0xD9D4D039 );
  P( D, A, B, C, 12, 11, 0xE6DB99E5 );
  P( C, D, A, B, 15, 16, 0x1FA27CF8 );
  P( B, C, D, A,  2, 23, 0xC4AC5665 );

#undef F

#define F(x,y,z) (y ^ (x | ~z))

  P( A, B, C, D,  0,  6, 0xF4292244 );
  P( D, A, B, C,  7, 10, 0x432AFF97 );
  P( C, D, A, B, 14, 15, 0xAB9423A7 );
  P( B, C, D, A,  5, 21, 0xFC93A039 );
  P( A, B, C, D, 12,  6, 0x655B59C3 );
  P( D, A, B, C,  3, 10, 0x8F0CCC92 );
  P( C, D, A, B, 10, 15, 0xFFEFF47D );
  P( B, C, D, A,  1, 21, 0x85845DD1 );
  P( A, B, C, D,  8,  6, 0x6FA87E4F );
  P( D, A, B, C, 15, 10, 0xFE2CE6E0 );
  P( C, D, A, B,  6, 15, 0xA3014314 );
  P( B, C, D, A, 13, 21, 0x4E0811A1 );
  P( A, B, C, D,  4,  6, 0xF7537E82 );
  P( D, A, B, C, 11, 10, 0xBD3AF235 );
  P( C, D, A, B,  2, 15, 0x2AD7D2BB );
  P( B, C, D, A,  9, 21, 0xEB86D391 );

#undef F

  ctx->state[0] += A;
  ctx->state[1] += B;
  ctx->state[2] += C;
  ctx->state[3] += D;
}

static uint8 md5_padding[64] =
{
 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void md5_update( md5_context *ctx, uint8 *input, uint32 length )
{
  uint32 left, fill;

  if( ! length ) return;

#ifdef _DEBUG
  if (g_BuildMd5Db && md5_padding!=input && ctx->msglen!=input)
    ctx->Data+=string((char*)input,length);
#endif

  left = ctx->total[0] & 0x3F;
  fill = 64 - left;

  ctx->total[0] += length;
  ctx->total[0] &= 0xFFFFFFFF;

  if( ctx->total[0] < length )
      ctx->total[1]++;

  if( left && length >= fill )
  {
    memcpy( (void *) (ctx->buffer + left),
            (void *) input, fill );
    md5_process( ctx, ctx->buffer );
    length -= fill;
    input  += fill;
    left = 0;
  }

  while( length >= 64 )
  {
    md5_process( ctx, input );
    length -= 64;
    input  += 64;
  }

  if( length )
  {
    memcpy( (void *) (ctx->buffer + left),
            (void *) input, length );
  }
}

uint32 *md5_finishbin( md5_context *ctx)
{
  uint32 last, padn;
  uint32 high, low;

  high = ( ctx->total[0] >> 29 )
        | ( ctx->total[1] <<  3 );
  low  = ( ctx->total[0] <<  3 );

  PUT_UINT32( low,  ctx->msglen, 0 );
  PUT_UINT32( high, ctx->msglen, 4 );

  last = ctx->total[0] & 0x3F;
  padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

  md5_update( ctx, md5_padding, padn );
  md5_update( ctx, ctx->msglen, 8 );

  return ctx->state;
}

void md5_finish( md5_context *ctx, uint8 digest[16] )
{
  md5_finishbin( ctx);
  PUT_UINT32( ctx->state[0], digest,  0 );
  PUT_UINT32( ctx->state[1], digest,  4 );
  PUT_UINT32( ctx->state[2], digest,  8 );
  PUT_UINT32( ctx->state[3], digest, 12 );
}

uint32 md5_finish32( md5_context *ctx)
{
  md5_finishbin( ctx);
  uint32 Md5_32=ctx->state[0]+ctx->state[1]+ctx->state[2]+ctx->state[3];
#ifdef _DEBUG
  if (g_BuildMd5Db) g_Md5Database[Md5_32]=ctx->Data;
#endif
  return Md5_32;
}

#ifdef _DEBUG
struct WRITEMD5DB
{
  ~WRITEMD5DB()
  {
    if (g_BuildMd5Db)
    {
      FILE *pFile=fopen("Md5.database","wb");
      map<uint32,string>::const_iterator It=g_Md5Database.begin();
      while (It!=g_Md5Database.end())
      {
        fprintf(pFile,"%08x: ",It->first);
        fwrite(It->second.c_str(),It->second.length(),1,pFile);
        fprintf(pFile,"\n");
        It++;
      }
      fclose(pFile);
    }
  }
};
static WRITEMD5DB WriteMd5Db;
#endif


#ifdef TEST

#include <stdlib.h>
#include <stdio.h>

/*
 * those are the standard RFC 1321 test vectors
 */

static char *msg[] =
{
    "",
    "a",
    "abc",
    "message digest",
    "abcdefghijklmnopqrstuvwxyz",
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
    "12345678901234567890123456789012345678901234567890123456789012" \
        "345678901234567890"
};

static char *val[] =
{
    "d41d8cd98f00b204e9800998ecf8427e",
    "0cc175b9c0f1b6a831c399e269772661",
    "900150983cd24fb0d6963f7d28e17f72",
    "f96b697d7cb7938d525a2f31aaf161d0",
    "c3fcd3d76192e4007dfb496cca67e13b",
    "d174ab98d277d9f5a5611c2c9f419d9f",
    "57edf4a22be3c955ac49da2e2107b67a"
};

int main( int argc, char *argv[] )
{
    FILE *f;
    int i, j;
    char output[33];
    md5_context ctx;
    unsigned char buf[1000];
    unsigned char md5sum[16];

    if( argc < 2 )
    {
        printf( "\n MD5 Validation Tests:\n\n" );

        for( i = 0; i < 7; i++ )
        {
            printf( " Test %d ", i + 1 );

            md5_starts( &ctx );
            md5_update( &ctx, (uint8 *) msg[i], strlen( msg[i] ) );
            md5_finish( &ctx, md5sum );

            for( j = 0; j < 16; j++ )
            {
                sprintf( output + j * 2, "%02x", md5sum[j] );
            }

            if( memcmp( output, val[i], 32 ) )
            {
                printf( "failed!\n" );
                return( 1 );
            }

            printf( "passed.\n" );
        }

        printf( "\n" );
    }
    else
    {
        if( ! ( f = fopen( argv[1], "rb" ) ) )
        {
            perror( "fopen" );
            return( 1 );
        }

        md5_starts( &ctx );

        while( ( i = fread( buf, 1, sizeof( buf ), f ) ) > 0 )
        {
            md5_update( &ctx, buf, i );
        }

        md5_finish( &ctx, md5sum );

        for( j = 0; j < 16; j++ )
        {
            printf( "%02x", md5sum[j] );
        }

        printf( "  %s\n", argv[1] );
    }

    return( 0 );
}

#endif

//#define APP
#ifdef APP
#include <stdio.h>

int main( int argc, char *argv[] )
{
  unsigned j;
  unsigned char md5sum[16];
  md5_context ctx;
  FILE *pFile=fopen(argv[1],"r");

  if (!pFile)
  {
    printf("Error opening file.\n");
    return 1;
  }

  md5_starts( &ctx );

  while (1)
  {
    char Buf[1024];
    size_t Ret=fread(Buf,1,sizeof(Buf),pFile);
    if (!Ret)
      break;
    md5_update( &ctx, (uint8 *) Buf, Ret);
  }
  md5_finish( &ctx, md5sum );

  for( j = 0; j < 16; j++ )
  {
    printf("%02x", md5sum[j] );
  }

}
#endif

