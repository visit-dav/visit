/* pnmrw.h - header file for PBM/PGM/PPM read/write library
**
** Copyright (C) 1988, 1989, 1991 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#ifndef _PNMRW_H_
#define _PNMRW_H_

/* CONFIGURE: On some systems, malloc.h doesn't declare these, so we have
** to do it.  On other systems, for example HP/UX, it declares them
** incompatibly.  And some systems, for example Dynix, don't have a
** malloc.h at all.  A sad situation.  If you have compilation problems
** that point here, feel free to tweak or remove these declarations.
*/
#include <malloc.h>
/* End of configurable definitions. */


/* Definitions to make PBMPLUS work with either ANSI C or C Classic. */

#if __STDC__
#define ARGS(alist) alist
#else /*__STDC__*/
#define ARGS(alist) ()
#define const
#endif /*__STDC__*/


/* Types. */

typedef unsigned char bit;
#define PBM_WHITE 0
#define PBM_BLACK 1
#define PBM_FORMAT_TYPE(f) ((f) == PBM_FORMAT || (f) == RPBM_FORMAT ? PBM_TYPE : -1)

typedef unsigned char gray;
#define PGM_MAXMAXVAL 255
#define PGM_FORMAT_TYPE(f) ((f) == PGM_FORMAT || (f) == RPGM_FORMAT ? PGM_TYPE : PBM_FORMAT_TYPE(f))

typedef gray pixval;
#define PPM_MAXMAXVAL PGM_MAXMAXVAL
typedef struct
    {
    pixval r, g, b;
    } pixel;
#define PPM_GETR(p) ((p).r)
#define PPM_GETG(p) ((p).g)
#define PPM_GETB(p) ((p).b)
#define PPM_ASSIGN(p,red,grn,blu) do { (p).r = (red); (p).g = (grn); (p).b = (blu); } while ( 0 )
#define PPM_EQUAL(p,q) ( (p).r == (q).r && (p).g == (q).g && (p).b == (q).b )
#define PPM_FORMAT_TYPE(f) ((f) == PPM_FORMAT || (f) == RPPM_FORMAT ? PPM_TYPE : PGM_FORMAT_TYPE(f))

typedef pixel xel;
typedef pixval xelval;
#define PNM_MAXMAXVAL PPM_MAXMAXVAL
#define PNM_GET1(x) PPM_GETB(x)
#define PNM_ASSIGN1(x,v) PPM_ASSIGN(x,0,0,v)
#define PNM_EQUAL(x,y) PPM_EQUAL(x,y)
#define PNM_FORMAT_TYPE(f) PPM_FORMAT_TYPE(f)


/* Magic constants. */

#define PBM_MAGIC1 'P'
#define PBM_MAGIC2 '1'
#define RPBM_MAGIC2 '4'
#define PBM_FORMAT (PBM_MAGIC1 * 256 + PBM_MAGIC2)
#define RPBM_FORMAT (PBM_MAGIC1 * 256 + RPBM_MAGIC2)
#define PBM_TYPE PBM_FORMAT

#define PGM_MAGIC1 'P'
#define PGM_MAGIC2 '2'
#define RPGM_MAGIC2 '5'
#define PGM_FORMAT (PGM_MAGIC1 * 256 + PGM_MAGIC2)
#define RPGM_FORMAT (PGM_MAGIC1 * 256 + RPGM_MAGIC2)
#define PGM_TYPE PGM_FORMAT

#define PPM_MAGIC1 'P'
#define PPM_MAGIC2 '3'
#define RPPM_MAGIC2 '6'
#define PPM_FORMAT (PPM_MAGIC1 * 256 + PPM_MAGIC2)
#define RPPM_FORMAT (PPM_MAGIC1 * 256 + RPPM_MAGIC2)
#define PPM_TYPE PPM_FORMAT


/* Color scaling macro -- to make writing ppmtowhatever easier. */

#define PPM_DEPTH(newp,p,oldmaxval,newmaxval) \
    PPM_ASSIGN( (newp), \
    ( (int) PPM_GETR(p) * (newmaxval) + (oldmaxval) / 2 ) / (oldmaxval), \
    ( (int) PPM_GETG(p) * (newmaxval) + (oldmaxval) / 2 ) / (oldmaxval), \
    ( (int) PPM_GETB(p) * (newmaxval) + (oldmaxval) / 2 ) / (oldmaxval) )


/* Luminance macro. */

#define PPM_LUMIN(p) ( 0.299 * PPM_GETR(p) + 0.587 * PPM_GETG(p) + 0.114 * PPM_GETB(p) )


/* Declarations of pnmrw routines. */

void pnm_init2 ARGS(( char* pn ));

char** pm_allocarray ARGS(( int cols, int rows, int size ));
#define pnm_allocarray( cols, rows ) ((xel**) pm_allocarray( cols, rows, sizeof(xel) ))
char* pm_allocrow ARGS(( int cols, int size ));
#define pnm_allocrow( cols ) ((xel*) pm_allocrow( cols, sizeof(xel) ))
void pm_freearray ARGS(( char** its, int rows ));
#define pnm_freearray( xels, rows ) pm_freearray( (char**) xels, rows )
void pm_freerow ARGS(( char* itrow ));
#define pnm_freerow( xelrow ) pm_freerow( (char*) xelrow )

xel** pnm_readpnm ARGS(( FILE* file, int* colsP, int* rowsP, xelval* maxvalP, int* formatP ));
int pnm_readpnminit ARGS(( FILE* file, int* colsP, int* rowsP, xelval* maxvalP, int* formatP ));
int pnm_readpnmrow ARGS(( FILE* file, xel* xelrow, int cols, xelval maxval, int format ));

int pnm_writepnm ARGS(( FILE* file, xel** xels, int cols, int rows, xelval maxval, int format, int forceplain ));
int pnm_writepnminit ARGS(( FILE* file, int cols, int rows, xelval maxval, int format, int forceplain ));
int pnm_writepnmrow ARGS(( FILE* file, xel* xelrow, int cols, xelval maxval, int format, int forceplain ));

extern xelval pnm_pbmmaxval;
/* This is the maxval used when a PNM program reads a PBM file.  Normally
** it is 1; however, for some programs, a larger value gives better results
*/


/* File open/close that handles "-" as stdin and checks errors. */

FILE* pm_openr ARGS(( char* name ));
FILE* pm_openw ARGS(( char* name ));
int pm_closer ARGS(( FILE* f ));
int pm_closew ARGS(( FILE* f ));


/* Colormap stuff. */

typedef struct colorhist_item* colorhist_vector;
struct colorhist_item
    {
    pixel color;
    int value;
    };

typedef struct colorhist_list_item* colorhist_list;
struct colorhist_list_item
    {
    struct colorhist_item ch;
    colorhist_list next;
    };

typedef colorhist_list* colorhash_table;

colorhist_vector ppm_computecolorhist ARGS(( pixel** pixels, int cols, int rows, int maxcolors, int* colorsP ));
/* Returns a colorhist *colorsP long (with space allocated for maxcolors. */

void ppm_addtocolorhist ARGS(( colorhist_vector chv, int* colorsP, int maxcolors, pixel* colorP, int value, int position ));

void ppm_freecolorhist ARGS(( colorhist_vector chv ));

colorhash_table ppm_computecolorhash ARGS(( pixel** pixels, int cols, int rows, int maxcolors, int* colorsP ));

int
ppm_lookupcolor ARGS(( colorhash_table cht, pixel* colorP ));

colorhist_vector ppm_colorhashtocolorhist ARGS(( colorhash_table cht, int maxcolors ));
colorhash_table ppm_colorhisttocolorhash ARGS(( colorhist_vector chv, int colors ));

int ppm_addtocolorhash ARGS(( colorhash_table cht, pixel* colorP, int value ));
/* Returns -1 on failure. */

colorhash_table ppm_alloccolorhash ARGS(( void ));

void ppm_freecolorhash ARGS(( colorhash_table cht ));

/* Other function declarations */
void pnm_promoteformat ARGS(( xel** xels, int cols, int rows, xelval maxval,
int format, xelval newmaxval, int newformat ));
void pnm_promoteformatrow ARGS(( xel* xelrow, int cols, xelval maxval, 
int format, xelval newmaxval, int newformat ));

xel pnm_backgroundxel ARGS(( xel** xels, int cols, int rows, xelval maxval, int format ));
xel pnm_backgroundxelrow ARGS(( xel* xelrow, int cols, xelval maxval, int format ));
xel pnm_whitexel ARGS(( xelval maxval, int format ));
xel pnm_blackxel ARGS(( xelval maxval, int format ));
void pnm_invertxel ARGS(( xel* xP, xelval maxval, int format ));

#endif /*_PNMRW_H_*/
