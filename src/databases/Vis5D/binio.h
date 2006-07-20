/*    
 * Vis5D system for visualizing five dimensional gridded data sets.
 * Copyright (C) 1990 - 2000 Bill Hibbard, Johan Kellum, Brian Paul,
 * Dave Santek, and Andre Battaiola.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * As a special exception to the terms of the GNU General Public
 * License, you are permitted to link Vis5D with (and distribute the
 * resulting source and executables) the LUI library (copyright by
 * Stellar Computer Inc. and licensed for distribution with Vis5D),
 * the McIDAS library, and/or the NetCDF library, where those
 * libraries are governed by the terms of their own licenses.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


/*
 * Functions to do binary I/O of floats, ints, etc. with byte swapping
 * as needed.
 */


#ifndef BINIO_H
#define BINIO_H


/* Include files which define SEEK_SET, O_RD_ONLY, etc. */
/* and prototype open(), close(), lseek(), etc. */
#include <unistd.h>
#include <fcntl.h>

/* SGJ: use extern "C" if included from a C++ file: */
#ifdef __cplusplus
extern "C" {
#endif                /* __cplusplus */

extern void flip4( const unsigned int *src, unsigned int *dest, int n );

extern void flip2( const unsigned short *src, unsigned short *dest, int n );


#ifdef _CRAY
  extern void cray_to_ieee_array( long *dest, const float *source, int n );
  extern void ieee_to_cray_array( float *dest, const long *source, int n );
#endif


/**********************************************************************/
/*****                     Read Functions                         *****/
/**********************************************************************/


extern int read_bytes( int f, void *b, int n );

extern int read_int2_array( int f, short *iarray, int n );

extern int read_uint2_array( int f, unsigned short *iarray, int n );

extern int read_int4( int f, int *i );

extern int read_int4_array( int f, int *iarray, int n );

extern int read_float4( int f, float *x );

extern int read_float4_array( int f, float *x, int n );

extern int read_block( int f, void *data, int elements, int elsize );



/**********************************************************************/
/*****                         Write Functions                    *****/
/**********************************************************************/


extern int write_bytes( int f, const void *b, int n );

extern int write_int2_array( int f, const short *iarray, int n );

extern int write_uint2_array( int f, const unsigned short *iarray, int n );

extern int write_int4( int f, int i );

extern int write_int4_array( int f, const int *iarray, int n );

extern int write_float4( int f, float x );

extern int write_float4_array( int f, const float *x, int n );

extern int write_block( int f, const void *data, int elements, int elsize );

#ifdef __cplusplus
}                               /* extern "C" */
#endif                /* __cplusplus */

#endif
