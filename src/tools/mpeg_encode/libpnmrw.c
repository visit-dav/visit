/* libpnmrw.c - PBM/PGM/PPM read/write library
 **
 ** Copyright (C) 1988, 1989, 1991, 1992 by Jef Poskanzer.
 **
 ** Permission to use, copy, modify, and distribute this software and its
 ** documentation for any purpose and without fee is hereby granted, provided
 ** that the above copyright notice appear in all copies and that both that
 ** copyright notice and this permission notice appear in supporting
 ** documentation.  This software is provided "as is" without express or
 ** implied warranty.
 */

#define pm_error(x) exit(1)


#if defined(SVR2) || defined(SVR3) || defined(SVR4)
#define SYSV
#endif
#if ! ( defined(BSD) || defined(SYSV) || defined(MSDOS) )
/* CONFIGURE: If your system is >= 4.2BSD, set the BSD option; if you're a
 ** System V site, set the SYSV option; and if you're IBM-compatible, set
** MSDOS.  If your compiler is ANSI C, you're probably better off setting
** SYSV - all it affects is string handling.
*/
#define BSD
/* #define SYSV */
/* #define MSDOS */
#endif

#include <errno.h>
#include <stdio.h>
#include "libpnmrw.h"

/* if don't have string.h, try strings.h */
#include <string.h>
#ifdef rindex
#undef rindex
#endif
#define rindex(s,c) strrchr(s,c)


/* Definitions. */

#define pbm_allocarray( cols, rows ) ((bit**) pm_allocarray( cols, rows, sizeof(bit) ))
#define pbm_allocrow( cols ) ((bit*) pm_allocrow( cols, sizeof(bit) ))
#define pbm_freearray( bits, rows ) pm_freearray( (char**) bits, rows )
#define pbm_freerow( bitrow ) pm_freerow( (char*) bitrow )
#define pgm_allocarray( cols, rows ) ((gray**) pm_allocarray( cols, rows, sizeof(gray) ))
#define pgm_allocrow( cols ) ((gray*) pm_allocrow( cols, sizeof(gray) ))
#define pgm_freearray( grays, rows ) pm_freearray( (char**) grays, rows )
#define pgm_freerow( grayrow ) pm_freerow( (char*) grayrow )
#define ppm_allocarray( cols, rows ) ((pixel**) pm_allocarray( cols, rows, sizeof(pixel) ))
#define ppm_allocrow( cols ) ((pixel*) pm_allocrow( cols, sizeof(pixel) ))
#define ppm_freearray( pixels, rows ) pm_freearray( (char**) pixels, rows )
#define ppm_freerow( pixelrow ) pm_freerow( (char*) pixelrow )


/* Variables. */

static char* progname;


/* Variable-sized arrays. */

char*
  pm_allocrow( cols, size )
int cols;
int size;
{
  register char* itrow;

  itrow = (char*) malloc( cols * size );
  if ( itrow == (char*) 0 )
    {
      (void) fprintf(
             stderr, "%s: out of memory allocating a row\n", progname );
      return (char*) 0;
    }
  return itrow;
}

void
  pm_freerow( itrow )
char* itrow;
{
  free( itrow );
}

char**
  pm_allocarray( cols, rows, size )
int cols, rows;
int size;
{
  char** its;
  int i;

  its = (char**) malloc( rows * sizeof(char*) );
  if ( its == (char**) 0 )
    {
      (void) fprintf(
             stderr, "%s: out of memory allocating an array\n", progname );
      return (char**) 0;
    }
  its[0] = (char*) malloc( rows * cols * size );
  if ( its[0] == (char*) 0 )
    {
      (void) fprintf(
             stderr, "%s: out of memory allocating an array\n", progname );
      free( (char*) its );
      return (char**) 0;
    }
  for ( i = 1; i < rows; ++i )
    its[i] = &(its[0][i * cols * size]);
  return its;
}

void
  pm_freearray( its, rows )
char** its;
int rows;
{
  free( its[0] );
  free( its );
}


/* File open/close that handles "-" as stdin and checks errors. */

static void
pm_perror(char *reason)
{
  extern int errno;
  char* e;

  e = strerror(errno);

  if ( reason != 0 && reason[0] != '\0' )
    (void) fprintf( stderr, "%s: %s - %s\n", progname, reason, e );
  else
    (void) fprintf( stderr, "%s: %s\n", progname, e );
}

FILE*
  pm_openr( name )
char* name;
{
  FILE* f;

  if ( strcmp( name, "-" ) == 0 )
    f = stdin;
  else
    {
      f = fopen( name, "rb" );
      if ( f == NULL )
    {
      pm_perror( name );
      return (FILE*) 0;
    }
    }
  return f;
}

FILE*
  pm_openw( name )
char* name;
{
  FILE* f;

  f = fopen( name, "wb" );
  if ( f == NULL ) {
    pm_perror( name );
    return (FILE*) 0;
  }
  return f;
}

int
  pm_closer( f )
FILE* f;
{
  if ( ferror( f ) )
    {
      (void) fprintf(
             stderr, "%s: a file read error occurred at some point\n",
             progname );
      return -1;
    }
  if ( f != stdin )
    if ( fclose( f ) != 0 )
      {
    pm_perror( "fclose" );
    return -1;
      }
  return 0;
}

int
  pm_closew( f )
FILE* f;
{
  fflush( f );
  if ( ferror( f ) )
    {
      (void) fprintf(
             stderr, "%s: a file write error occurred at some point\n",
             progname );
      return -1;
    }
  if ( f != stdout )
    if ( fclose( f ) != 0 )
      {
    pm_perror( "fclose" );
    return -1;
      }
  return 0;
}

static int
  pbm_getc( file )
FILE* file;
{
  register int ich;

  ich = getc( file );
  if ( ich == EOF )
    {
      (void) fprintf( stderr, "%s: EOF / read error\n", progname );
      return EOF;
    }
    
  if ( ich == '#' )
    {
      do
    {
      ich = getc( file );
      if ( ich == EOF )
        {
          (void) fprintf( stderr, "%s: EOF / read error\n", progname );
          return EOF;
        }
    }
      while ( ich != '\n' && ich != '\r' );
    }

  return ich;
}

static bit
  pbm_getbit( file )
FILE* file;
{
  register int ich;

  do
    {
      ich = pbm_getc( file );
      if ( ich == EOF )
    return -1;
    }
  while ( ich == ' ' || ich == '\t' || ich == '\n' || ich == '\r' );

  if ( ich != '0' && ich != '1' )
    {
      (void) fprintf(
             stderr, "%s: junk in file where bits should be\n", progname );
      return -1;
    }

  return ( ich == '1' ) ? 1 : 0;
}

static int
  pbm_readmagicnumber( file )
FILE* file;
{
  int ich1, ich2;

  ich1 = getc( file );
  if ( ich1 == EOF )
    {
      (void) fprintf(
             stderr, "%s: EOF / read error reading magic number\n", progname );
      return -1;
    }
  ich2 = getc( file );
  if ( ich2 == EOF )
    {
      (void) fprintf(
             stderr, "%s: EOF / read error reading magic number\n", progname );
      return -1;
    }
  return ich1 * 256 + ich2;
}

static int
  pbm_getint( file )
FILE* file;
{
  register int ich;
  register int i;

  do
    {
      ich = pbm_getc( file );
      if ( ich == EOF )
    return -1;
    }
  while ( ich == ' ' || ich == '\t' || ich == '\n' || ich == '\r' );

  if ( ich < '0' || ich > '9' )
    {
      (void) fprintf(
             stderr, "%s: junk in file where an integer should be\n", progname );
      return -1;
    }

  i = 0;
  do
    {
      i = i * 10 + ich - '0';
      ich = pbm_getc( file );
      if ( ich == EOF )
    return -1;
    }
  while ( ich >= '0' && ich <= '9' );

  return i;
}

static int
  pbm_readpbminitrest( file, colsP, rowsP )
FILE* file;
int* colsP;
int* rowsP;
{
  /* Read size. */
  *colsP = pbm_getint( file );
  *rowsP = pbm_getint( file );
  if ( *colsP == -1 || *rowsP == -1 )
    return -1;
  return 0;
}

static int
  pbm_getrawbyte( file )
FILE* file;
{
  register int iby;

  iby = getc( file );
  if ( iby == EOF )
    {
      (void) fprintf( stderr, "%s: EOF / read error\n", progname );
      return -1;
    }
  return iby;
}

static int
  pbm_readpbmrow( file, bitrow, cols, format )
FILE* file;
bit* bitrow;
int cols, format;
{
  register int col, bitshift, b;
  register int item;
  register bit* bP;

  switch ( format )
    {
    case PBM_FORMAT:
      for ( col = 0, bP = bitrow; col < cols; ++col, ++bP )
    {
      b = pbm_getbit( file );
      if ( b == -1 )
        return -1;
      *bP = b;
    }
      break;

    case RPBM_FORMAT:
      bitshift = -1;
      for ( col = 0, bP = bitrow; col < cols; ++col, ++bP )
    {
      if ( bitshift == -1 )
        {
          item = pbm_getrawbyte( file );
          if ( item == -1 )
        return -1;
          bitshift = 7;
        }
      *bP = ( item >> bitshift ) & 1;
      --bitshift;
    }
      break;

    default:
      (void) fprintf( stderr, "%s: can't happen\n", progname );
      return -1;
    }
  return 0;
}

static void
  pbm_writepbminit( file, cols, rows, forceplain )
FILE* file;
int cols, rows;
int forceplain;
{
  if ( ! forceplain )
    (void) fprintf(
           file, "%c%c\n%d %d\n", PBM_MAGIC1, RPBM_MAGIC2, cols, rows );
  else
    (void) fprintf(
           file, "%c%c\n%d %d\n", PBM_MAGIC1, PBM_MAGIC2, cols, rows );
}

static void
  pbm_writepbmrowraw( file, bitrow, cols )
FILE* file;
bit* bitrow;
int cols;
{
  register int col, bitshift;
  register unsigned char item;
  register bit* bP;

  bitshift = 7;
  item = 0;
  for ( col = 0, bP = bitrow; col < cols; ++col, ++bP )
    {
      if ( *bP )
    item += 1 << bitshift;
      --bitshift;
      if ( bitshift == -1 )
    {
      (void) putc( item, file );
      bitshift = 7;
      item = 0;
    }
    }
  if ( bitshift != 7 )
    (void) putc( item, file );
}

static void
  pbm_writepbmrowplain( file, bitrow, cols )
FILE* file;
bit* bitrow;
int cols;
{
  register int col, charcount;
  register bit* bP;

  charcount = 0;
  for ( col = 0, bP = bitrow; col < cols; ++col, ++bP )
    {
      if ( charcount >= 70 )
    {
      (void) putc( '\n', file );
      charcount = 0;
    }
      putc( *bP ? '1' : '0', file );
      ++charcount;
    }
  (void) putc( '\n', file );
}

static void
  pbm_writepbmrow( file, bitrow, cols, forceplain )
FILE* file;
bit* bitrow;
int cols;
int forceplain;
{
  if ( ! forceplain )
    pbm_writepbmrowraw( file, bitrow, cols );
  else
    pbm_writepbmrowplain( file, bitrow, cols );
}

static int
  pgm_readpgminitrest( file, colsP, rowsP, maxvalP )
FILE* file;
int* colsP;
int* rowsP;
gray* maxvalP;
{
  int maxval;

  /* Read size. */
  *colsP = pbm_getint( file );
  *rowsP = pbm_getint( file );
  if ( *colsP == -1 || *rowsP == -1 )
    return -1;

  /* Read maxval. */
  maxval = pbm_getint( file );
  if ( maxval == -1 )
    return -1;
  if ( maxval > PGM_MAXMAXVAL )
    {
      (void) fprintf( stderr, "%s: maxval is too large\n", progname );
      return -1;
    }
  *maxvalP = maxval;
  return 0;
}

#if __STDC__
static int
  pgm_readpgmrow( FILE* file, gray* grayrow, int cols, gray maxval, int format )
#else /*__STDC__*/
static int
  pgm_readpgmrow( file, grayrow, cols, maxval, format )
FILE* file;
gray* grayrow;
int cols;
gray maxval;
int format;
#endif /*__STDC__*/
{
  register int col, val;
  register gray* gP;
  /*
    bit* bitrow;
    register bit* bP;
    */

  switch ( format )
    {
    case PGM_FORMAT:
      for ( col = 0, gP = grayrow; col < cols; ++col, ++gP )
    {
      val = pbm_getint( file );
      if ( val == -1 )
        return -1;
      *gP = val;
    }
      break;
    
    case RPGM_FORMAT:
      if ( fread( grayrow, 1, cols, file ) != cols )
    {
      (void) fprintf( stderr, "%s: EOF / read error\n", progname );
      return -1;
    }
      break;

    default:
      (void) fprintf( stderr, "%s: can't happen\n", progname );
      return -1;
    }
  return 0;
}

#if __STDC__
static void
  pgm_writepgminit( FILE* file, int cols, int rows, gray maxval, int forceplain )
#else /*__STDC__*/
static void
  pgm_writepgminit( file, cols, rows, maxval, forceplain )
FILE* file;
int cols, rows;
gray maxval;
int forceplain;
#endif /*__STDC__*/
{
  if ( maxval <= 255 && ! forceplain )
    fprintf(
        file, "%c%c\n%d %d\n%d\n", PGM_MAGIC1, RPGM_MAGIC2,
        cols, rows, maxval );
  else
    fprintf(
        file, "%c%c\n%d %d\n%d\n", PGM_MAGIC1, PGM_MAGIC2,
        cols, rows, maxval );
}

static void
  putus( n, file )
unsigned short n;
FILE* file;
{
  if ( n >= 10 )
    putus( n / 10, file );
  putc( n % 10 + '0', file );
}

static int
  pgm_writepgmrowraw( file, grayrow, cols, maxval )
FILE* file;
gray* grayrow;
int cols;
gray maxval;
{
  if ( fwrite( grayrow, 1, cols, file ) != cols )
    {
      (void) fprintf( stderr, "%s: write error\n", progname );
      return -1;
    }
  return 0;
}

static int
  pgm_writepgmrowplain( file, grayrow, cols, maxval )
FILE* file;
gray* grayrow;
int cols;
gray maxval;
{
  register int col, charcount;
  register gray* gP;

  charcount = 0;
  for ( col = 0, gP = grayrow; col < cols; ++col, ++gP )
    {
      if ( charcount >= 65 )
    {
      (void) putc( '\n', file );
      charcount = 0;
    }
      else if ( charcount > 0 )
    {
      (void) putc( ' ', file );
      ++charcount;
    }
      putus( (unsigned short) *gP, file );
      charcount += 3;
    }
  if ( charcount > 0 )
    (void) putc( '\n', file );
  return 0;
}

#if __STDC__
static int
  pgm_writepgmrow( FILE* file, gray* grayrow, int cols, gray maxval, int forceplain )
#else /*__STDC__*/
static int
  pgm_writepgmrow( file, grayrow, cols, maxval, forceplain )
FILE* file;
gray* grayrow;
int cols;
gray maxval;
int forceplain;
#endif /*__STDC__*/
{
  if ( maxval <= 255 && ! forceplain )
    return pgm_writepgmrowraw( file, grayrow, cols, maxval );
  else
    return pgm_writepgmrowplain( file, grayrow, cols, maxval );
}

static int
  ppm_readppminitrest( file, colsP, rowsP, maxvalP )
FILE* file;
int* colsP;
int* rowsP;
pixval* maxvalP;
{
  int maxval;

  /* Read size. */
  *colsP = pbm_getint( file );
  *rowsP = pbm_getint( file );
  if ( *colsP == -1 || *rowsP == -1 )
    return -1;

  /* Read maxval. */
  maxval = pbm_getint( file );
  if ( maxval == -1 )
    return -1;
  if ( maxval > PPM_MAXMAXVAL )
    {
      (void) fprintf( stderr, "%s: maxval is too large\n", progname );
      return -1;
    }
  *maxvalP = maxval;
  return 0;
}

#if __STDC__
static int
  ppm_readppmrow( FILE* file, pixel* pixelrow, int cols, pixval maxval, int format )
#else /*__STDC__*/
static int
  ppm_readppmrow( file, pixelrow, cols, maxval, format )
FILE* file;
pixel* pixelrow;
int cols, format;
pixval maxval;
#endif /*__STDC__*/
{
  register int col;
  register pixel* pP;
  register int r, g, b;
  gray* grayrow;
  register gray* gP;
  /*
    bit* bitrow;
    register bit* bP;
    */

  switch ( format )
    {
    case PPM_FORMAT:
      for ( col = 0, pP = pixelrow; col < cols; ++col, ++pP )
    {
      r = pbm_getint( file );
      g = pbm_getint( file );
      b = pbm_getint( file );
      if ( r == -1 || g == -1 || b == -1 )
        return -1;
      PPM_ASSIGN( *pP, r, g, b );
    }
      break;

    case RPPM_FORMAT:
      grayrow = pgm_allocrow( 3 * cols );
      if ( grayrow == (gray*) 0 )
    return -1;
      if ( fread( grayrow, 1, 3 * cols, file ) != 3 * cols )
    {
      (void) fprintf( stderr, "%s: EOF / read error\n", progname );
      return -1;
    }
      for ( col = 0, gP = grayrow, pP = pixelrow; col < cols; ++col, ++pP )
    {
      r = *gP++;
      g = *gP++;
      b = *gP++;
      PPM_ASSIGN( *pP, r, g, b );
    }
      pgm_freerow( grayrow );
      break;

    default:
      (void) fprintf( stderr, "%s: can't happen\n", progname );
      return -1;
    }
  return 0;
}

#if __STDC__
static void
  ppm_writeppminit( FILE* file, int cols, int rows, pixval maxval, int forceplain )
#else /*__STDC__*/
static void
  ppm_writeppminit( file, cols, rows, maxval, forceplain )
FILE* file;
int cols, rows;
pixval maxval;
int forceplain;
#endif /*__STDC__*/
{
  if ( maxval <= 255 && ! forceplain )
    fprintf(
        file, "%c%c\n%d %d\n%d\n", PPM_MAGIC1, RPPM_MAGIC2,
        cols, rows, maxval );
  else
    fprintf(
        file, "%c%c\n%d %d\n%d\n", PPM_MAGIC1, PPM_MAGIC2,
        cols, rows, maxval );
}

static int
  ppm_writeppmrowraw( file, pixelrow, cols, maxval )
FILE* file;
pixel* pixelrow;
int cols;
pixval maxval;
{
  register int col;
  register pixel* pP;
  gray* grayrow;
  register gray* gP;

  grayrow = pgm_allocrow( 3 * cols );
  if ( grayrow == (gray*) 0 )
    return -1;
  for ( col = 0, pP = pixelrow, gP = grayrow; col < cols; ++col, ++pP )
    {
      *gP++ = PPM_GETR( *pP );
      *gP++ = PPM_GETG( *pP );
      *gP++ = PPM_GETB( *pP );
    }
  if ( fwrite( grayrow, 1, 3 * cols, file ) != 3 * cols )
    {
      (void) fprintf( stderr, "%s: write error\n", progname );
      return -1;
    }
  pgm_freerow( grayrow );
  return 0;
}

static int
  ppm_writeppmrowplain( file, pixelrow, cols, maxval )
FILE* file;
pixel* pixelrow;
int cols;
pixval maxval;
{
  register int col, charcount;
  register pixel* pP;
  register pixval val;

  charcount = 0;
  for ( col = 0, pP = pixelrow; col < cols; ++col, ++pP )
    {
      if ( charcount >= 65 )
    {
      (void) putc( '\n', file );
      charcount = 0;
    }
      else if ( charcount > 0 )
    {
      (void) putc( ' ', file );
      (void) putc( ' ', file );
      charcount += 2;
    }
      val = PPM_GETR( *pP );
      putus( val, file );
      (void) putc( ' ', file );
      val = PPM_GETG( *pP );
      putus( val, file );
      (void) putc( ' ', file );
      val = PPM_GETB( *pP );
      putus( val, file );
      charcount += 11;
    }
  if ( charcount > 0 )
    (void) putc( '\n', file );
  return 0;
}

#if __STDC__
static int
  ppm_writeppmrow( FILE* file, pixel* pixelrow, int cols, pixval maxval, int forceplain )
#else /*__STDC__*/
static int
  ppm_writeppmrow( file, pixelrow, cols, maxval, forceplain )
FILE* file;
pixel* pixelrow;
int cols;
pixval maxval;
int forceplain;
#endif /*__STDC__*/
{
  if ( maxval <= 255 && ! forceplain )
    return ppm_writeppmrowraw( file, pixelrow, cols, maxval );
  else
    return ppm_writeppmrowplain( file, pixelrow, cols, maxval );
}

void
  pnm_init2( pn )
char* pn;
{
  /* Save program name. */
  progname = pn;
}

xelval pnm_pbmmaxval = 1;

int
  pnm_readpnminit( file, colsP, rowsP, maxvalP, formatP )
FILE* file;
int* colsP;
int* rowsP;
int* formatP;
xelval* maxvalP;
{
  gray gmaxval;

  /* Check magic number. */
  *formatP = pbm_readmagicnumber( file );
  if ( *formatP == -1 )
    return -1;
  switch ( PNM_FORMAT_TYPE(*formatP) )
    {
    case PPM_TYPE:
      if ( ppm_readppminitrest( file, colsP, rowsP, (pixval*) maxvalP ) < 0 )
    return -1;
      break;

    case PGM_TYPE:
      if ( pgm_readpgminitrest( file, colsP, rowsP, &gmaxval ) < 0 )
    return -1;
      *maxvalP = (xelval) gmaxval;
      break;

    case PBM_TYPE:
      if ( pbm_readpbminitrest( file, colsP, rowsP ) < 0 )
    return -1;
      *maxvalP = pnm_pbmmaxval;
      break;

    default:
      (void) fprintf(
             stderr, "%s: bad magic number - not a ppm, pgm, or pbm file\n",
             progname );
      return -1;
    }
  return 0;
}

#if __STDC__
int
  pnm_readpnmrow( FILE* file, xel* xelrow, int cols, xelval maxval, int format )
#else /*__STDC__*/
int
  pnm_readpnmrow( file, xelrow, cols, maxval, format )
FILE* file;
xel* xelrow;
xelval maxval;
int cols, format;
#endif /*__STDC__*/
{
  register int col;
  register xel* xP;
  gray* grayrow;
  register gray* gP;
  bit* bitrow;
  register bit* bP;

  switch ( PNM_FORMAT_TYPE(format) )
    {
    case PPM_TYPE:
      if ( ppm_readppmrow( file, (pixel*) xelrow, cols, (pixval) maxval, format ) < 0 )
    return -1;
      break;

    case PGM_TYPE:
      grayrow = pgm_allocrow( cols );
      if ( grayrow == (gray*) 0 )
    return -1;
      if ( pgm_readpgmrow( file, grayrow, cols, (gray) maxval, format ) < 0 )
    return -1;
      for ( col = 0, xP = xelrow, gP = grayrow; col < cols; ++col, ++xP, ++gP )
    PNM_ASSIGN1( *xP, *gP );
      pgm_freerow( grayrow );
      break;

    case PBM_TYPE:
      bitrow = pbm_allocrow( cols );
      if ( bitrow == (bit*) 0 )
    return -1;
      if ( pbm_readpbmrow( file, bitrow, cols, format ) < 0 )
    {
      pbm_freerow( bitrow );
      return -1;
    }
      for ( col = 0, xP = xelrow, bP = bitrow; col < cols; ++col, ++xP, ++bP )
    PNM_ASSIGN1( *xP, *bP == PBM_BLACK ? 0: pnm_pbmmaxval );
      pbm_freerow( bitrow );
      break;

    default:
      (void) fprintf( stderr, "%s: can't happen\n", progname );
      return -1;
    }
  return 0;
}

xel**
  pnm_readpnm( file, colsP, rowsP, maxvalP, formatP )
FILE* file;
int* colsP;
int* rowsP;
int* formatP;
xelval* maxvalP;
{
  xel** xels;
  int row;

  if ( pnm_readpnminit( file, colsP, rowsP, maxvalP, formatP ) < 0 )
    return (xel**) 0;

  xels = pnm_allocarray( *colsP, *rowsP );
  if ( xels == (xel**) 0 )
    return (xel**) 0;

  for ( row = 0; row < *rowsP; ++row )
    if ( pnm_readpnmrow( file, xels[row], *colsP, *maxvalP, *formatP ) < 0 )
      {
    pnm_freearray( xels, *rowsP );
    return (xel**) 0;
      }

  return xels;
}

#if __STDC__
int
  pnm_writepnminit( FILE* file, int cols, int rows, xelval maxval, int format, int forceplain )
#else /*__STDC__*/
int
  pnm_writepnminit( file, cols, rows, maxval, format, forceplain )
FILE* file;
int cols, rows, format;
xelval maxval;
int forceplain;
#endif /*__STDC__*/
{
  switch ( PNM_FORMAT_TYPE(format) )
    {
    case PPM_TYPE:
      ppm_writeppminit( file, cols, rows, (pixval) maxval, forceplain );
      break;

    case PGM_TYPE:
      pgm_writepgminit( file, cols, rows, (gray) maxval, forceplain );
      break;

    case PBM_TYPE:
      pbm_writepbminit( file, cols, rows, forceplain );
      break;

    default:
      (void) fprintf( stderr, "%s: can't happen\n", progname );
      return -1;
    }
  return 0;
}

#if __STDC__
int
  pnm_writepnmrow( FILE* file, xel* xelrow, int cols, xelval maxval, int format, int forceplain )
#else /*__STDC__*/
int
  pnm_writepnmrow( file, xelrow, cols, maxval, format, forceplain )
FILE* file;
xel* xelrow;
int cols, format;
xelval maxval;
int forceplain;
#endif /*__STDC__*/
{
  register int col;
  register xel* xP;
  gray* grayrow;
  register gray* gP;
  bit* bitrow;
  register bit* bP;

  switch ( PNM_FORMAT_TYPE(format) )
    {
    case PPM_TYPE:
      if ( ppm_writeppmrow( file, (pixel*) xelrow, cols, (pixval) maxval, forceplain ) < 0 )
    return -1;
      break;

    case PGM_TYPE:
      grayrow = pgm_allocrow( cols );
      if ( grayrow == (gray*) 0 )
    return -1;
      for ( col = 0, gP = grayrow, xP = xelrow; col < cols; ++col, ++gP, ++xP )
    *gP = PNM_GET1( *xP );
      if ( pgm_writepgmrow( file, grayrow, cols, (gray) maxval, forceplain ) < 0 )
    {
      pgm_freerow( grayrow );
      return -1;
    }
      pgm_freerow( grayrow );
      break;

    case PBM_TYPE:
      bitrow = pbm_allocrow( cols );
      if ( bitrow == (bit*) 0 )
    return -1;
      for ( col = 0, bP = bitrow, xP = xelrow; col < cols; ++col, ++bP, ++xP )
    *bP = PNM_GET1( *xP ) == 0 ? PBM_BLACK : PBM_WHITE;
      pbm_writepbmrow( file, bitrow, cols, forceplain );
      pbm_freerow( bitrow );
      break;

    default:
      (void) fprintf( stderr, "%s: can't happen\n", progname );
      return -1;
    }
  return 0;
}

#if __STDC__
int
  pnm_writepnm( FILE* file, xel** xels, int cols, int rows, xelval maxval, int format, int forceplain )
#else /*__STDC__*/
int
  pnm_writepnm( file, xels, cols, rows, maxval, format, forceplain )
FILE* file;
xel** xels;
xelval maxval;
int cols, rows, format;
int forceplain;
#endif /*__STDC__*/
{
  int row;

  if ( pnm_writepnminit( file, cols, rows, maxval, format, forceplain ) < 0 )
    return -1;

  for ( row = 0; row < rows; ++row )
    if ( pnm_writepnmrow( file, xels[row], cols, maxval, format, forceplain ) < 0 )
      return -1;
  return 0;
}


/* Colormap stuff. */

#define HASH_SIZE 20023

#define ppm_hashpixel(p) ( ( ( (long) PPM_GETR(p) * 33023 + (long) PPM_GETG(p) * 30013 + (long) PPM_GETB(p) * 27011 ) & 0x7fffffff ) % HASH_SIZE )

colorhist_vector
  ppm_computecolorhist( pixels, cols, rows, maxcolors, colorsP )
pixel** pixels;
int cols, rows, maxcolors;
int* colorsP;
{
  colorhash_table cht;
  colorhist_vector chv;

  cht = ppm_computecolorhash( pixels, cols, rows, maxcolors, colorsP );
  if ( cht == (colorhash_table) 0 )
    return (colorhist_vector) 0;
  chv = ppm_colorhashtocolorhist( cht, maxcolors );
  ppm_freecolorhash( cht );
  return chv;
}

void
  ppm_addtocolorhist( chv, colorsP, maxcolors, colorP, value, position )
colorhist_vector chv;
pixel* colorP;
int* colorsP;
int maxcolors, value, position;
{
  int i, j;

  /* Search colorhist for the color. */
  for ( i = 0; i < *colorsP; ++i )
    if ( PPM_EQUAL( chv[i].color, *colorP ) )
      {
    /* Found it - move to new slot. */
    if ( position > i )
      {
        for ( j = i; j < position; ++j )
          chv[j] = chv[j + 1];
      }
    else if ( position < i )
      {
        for ( j = i; j > position; --j )
          chv[j] = chv[j - 1];
      }
    chv[position].color = *colorP;
    chv[position].value = value;
    return;
      }
  if ( *colorsP < maxcolors )
    {
      /* Didn't find it, but there's room to add it; so do so. */
      for ( i = *colorsP; i > position; --i )
    chv[i] = chv[i - 1];
      chv[position].color = *colorP;
      chv[position].value = value;
      ++(*colorsP);
    }
}

colorhash_table
  ppm_computecolorhash( pixels, cols, rows, maxcolors, colorsP )
pixel** pixels;
int cols, rows, maxcolors;
int* colorsP;
{
  colorhash_table cht;
  register pixel* pP;
  colorhist_list chl;
  int col, row, hash;

  cht = ppm_alloccolorhash( );
  if ( cht == (colorhash_table) 0 )
    return (colorhash_table) 0;
  *colorsP = 0;

  /* Go through the entire image, building a hash table of colors. */
  for ( row = 0; row < rows; ++row )
    for ( col = 0, pP = pixels[row]; col < cols; ++col, ++pP )
      {
    hash = ppm_hashpixel( *pP );
    for ( chl = cht[hash]; chl != (colorhist_list) 0; chl = chl->next )
      if ( PPM_EQUAL( chl->ch.color, *pP ) )
        break;
    if ( chl != (colorhist_list) 0 )
      ++(chl->ch.value);
    else
      {
        if ( ++(*colorsP) > maxcolors )
          {
        ppm_freecolorhash( cht );
        return (colorhash_table) 0;
          }
        chl = (colorhist_list) malloc( sizeof(struct colorhist_list_item) );
        if ( chl == 0 )
          {
        (void) fprintf(
                   stderr, "%s: out of memory computing hash table\n",
                   progname );
        ppm_freecolorhash( cht );
        return (colorhash_table) 0;
          }
        chl->ch.color = *pP;
        chl->ch.value = 1;
        chl->next = cht[hash];
        cht[hash] = chl;
      }
      }
    
  return cht;
}

colorhash_table
  ppm_alloccolorhash( )
{
  colorhash_table cht;
  int i;

  cht = (colorhash_table) malloc( HASH_SIZE * sizeof(colorhist_list) );
  if ( cht == 0 )
    {
      (void) fprintf(
             stderr, "%s: out of memory allocating hash table\n", progname );
      return (colorhash_table) 0;
    }

  for ( i = 0; i < HASH_SIZE; ++i )
    cht[i] = (colorhist_list) 0;

  return cht;
}

int
  ppm_addtocolorhash( cht, colorP, value )
colorhash_table cht;
pixel* colorP;
int value;
{
  register int hash;
  register colorhist_list chl;

  chl = (colorhist_list) malloc( sizeof(struct colorhist_list_item) );
  if ( chl == 0 )
    return -1;
  hash = ppm_hashpixel( *colorP );
  chl->ch.color = *colorP;
  chl->ch.value = value;
  chl->next = cht[hash];
  cht[hash] = chl;
  return 0;
}

colorhist_vector
  ppm_colorhashtocolorhist( cht, maxcolors )
colorhash_table cht;
int maxcolors;
{
  colorhist_vector chv;
  colorhist_list chl;
  int i, j;

  /* Now collate the hash table into a simple colorhist array. */
  chv = (colorhist_vector) malloc( maxcolors * sizeof(struct colorhist_item) );
  /* (Leave room for expansion by caller.) */
  if ( chv == (colorhist_vector) 0 )
    {
      (void) fprintf(
             stderr, "%s: out of memory generating histogram\n", progname );
      return (colorhist_vector) 0;
    }

  /* Loop through the hash table. */
  j = 0;
  for ( i = 0; i < HASH_SIZE; ++i )
    for ( chl = cht[i]; chl != (colorhist_list) 0; chl = chl->next )
      {
    /* Add the new entry. */
    chv[j] = chl->ch;
    ++j;
      }

  /* All done. */
  return chv;
}

colorhash_table
  ppm_colorhisttocolorhash( chv, colors )
colorhist_vector chv;
int colors;
{
  colorhash_table cht;
  int i, hash;
  pixel color;
  colorhist_list chl;

  cht = ppm_alloccolorhash( );
  if ( cht == (colorhash_table) 0 )
    return (colorhash_table) 0;

  for ( i = 0; i < colors; ++i )
    {
      color = chv[i].color;
      hash = ppm_hashpixel( color );
      for ( chl = cht[hash]; chl != (colorhist_list) 0; chl = chl->next )
    if ( PPM_EQUAL( chl->ch.color, color ) )
      {
        (void) fprintf(
               stderr, "%s: same color found twice - %d %d %d\n", progname,
               PPM_GETR(color), PPM_GETG(color), PPM_GETB(color) );
        ppm_freecolorhash( cht );
        return (colorhash_table) 0;
      }
      chl = (colorhist_list) malloc( sizeof(struct colorhist_list_item) );
      if ( chl == (colorhist_list) 0 )
    {
      (void) fprintf( stderr, "%s: out of memory\n", progname );
      ppm_freecolorhash( cht );
      return (colorhash_table) 0;
    }
      chl->ch.color = color;
      chl->ch.value = i;
      chl->next = cht[hash];
      cht[hash] = chl;
    }

  return cht;
}

int
  ppm_lookupcolor( cht, colorP )
colorhash_table cht;
pixel* colorP;
{
  int hash;
  colorhist_list chl;

  hash = ppm_hashpixel( *colorP );
  for ( chl = cht[hash]; chl != (colorhist_list) 0; chl = chl->next )
    if ( PPM_EQUAL( chl->ch.color, *colorP ) )
      return chl->ch.value;

  return -1;
}

void
  ppm_freecolorhist( chv )
colorhist_vector chv;
{
  free( (char*) chv );
}

void
  ppm_freecolorhash( cht )
colorhash_table cht;
{
  int i;
  colorhist_list chl, chlnext;

  for ( i = 0; i < HASH_SIZE; ++i )
    for ( chl = cht[i]; chl != (colorhist_list) 0; chl = chlnext )
      {
    chlnext = chl->next;
    free( (char*) chl );
      }
  free( (char*) cht );
}




/* added from libpnm3.c: */

/* libpnm3.c - pnm utility library part 3
 **
 ** Copyright (C) 1989, 1991 by Jef Poskanzer.
 **
 ** Permission to use, copy, modify, and distribute this software and its
 ** documentation for any purpose and without fee is hereby granted, provided
 ** that the above copyright notice appear in all copies and that both that
 ** copyright notice and this permission notice appear in supporting
 ** documentation.  This software is provided "as is" without express or
 ** implied warranty.
 */

#if __STDC__
xel
  pnm_backgroundxel( xel** xels, int cols, int rows, xelval maxval, int format )
#else /*__STDC__*/
xel
  pnm_backgroundxel( xels, cols, rows, maxval, format )
xel** xels;
int cols, rows, format;
xelval maxval;
#endif /*__STDC__*/
{
  xel bgxel, ul, ur, ll, lr;

  /* Guess a good background value. */
  ul = xels[0][0];
  ur = xels[0][cols-1];
  ll = xels[rows-1][0];
  lr = xels[rows-1][cols-1];

  /* First check for three corners equal. */
  if ( PNM_EQUAL( ul, ur ) && PNM_EQUAL( ur, ll ) )
    bgxel = ul;
  else if ( PNM_EQUAL( ul, ur ) && PNM_EQUAL( ur, lr ) )
    bgxel = ul;
  else if ( PNM_EQUAL( ul, ll ) && PNM_EQUAL( ll, lr ) )
    bgxel = ul;
  else if ( PNM_EQUAL( ur, ll ) && PNM_EQUAL( ll, lr ) )
    bgxel = ur;
  /* Nope, check for two corners equal. */
  else if ( PNM_EQUAL( ul, ur ) || PNM_EQUAL( ul, ll ) ||
       PNM_EQUAL( ul, lr ) )
    bgxel = ul;
  else if ( PNM_EQUAL( ur, ll ) || PNM_EQUAL( ur, lr ) )
    bgxel = ur;
  else if ( PNM_EQUAL( ll, lr ) )
    bgxel = ll;
  else
    {
      /* Nope, we have to average the four corners.  This breaks the
       ** rules of pnm, but oh well.  Let's try to do it portably. */
      switch ( PNM_FORMAT_TYPE(format) )
    {
    case PPM_TYPE:
      PPM_ASSIGN( bgxel,
             PPM_GETR(ul) + PPM_GETR(ur) + PPM_GETR(ll) + PPM_GETR(lr) / 4,
             PPM_GETG(ul) + PPM_GETG(ur) + PPM_GETG(ll) + PPM_GETG(lr) / 4,
             PPM_GETB(ul) + PPM_GETB(ur) + PPM_GETB(ll) + PPM_GETB(lr) / 4 );
      break;

    case PGM_TYPE:
      {
        gray gul, gur, gll, glr;
        gul = (gray) PNM_GET1( ul );
        gur = (gray) PNM_GET1( ur );
        gll = (gray) PNM_GET1( ll );
        glr = (gray) PNM_GET1( lr );
        PNM_ASSIGN1( bgxel, ( ( gul + gur + gll + glr ) / 4 ) );
        break;
      }

    case PBM_TYPE:
      pm_error(
           "pnm_backgroundxel: four bits no two of which equal each other??" );

    default:
      pm_error( "can't happen" );
    }
    }

  return bgxel;
}

#if __STDC__
xel
  pnm_backgroundxelrow( xel* xelrow, int cols, xelval maxval, int format )
#else /*__STDC__*/
xel
  pnm_backgroundxelrow( xelrow, cols, maxval, format )
xel* xelrow;
int cols, format;
xelval maxval;
#endif /*__STDC__*/
{
  xel bgxel, l, r;

  /* Guess a good background value. */
  l = xelrow[0];
  r = xelrow[cols-1];

  /* First check for both corners equal. */
  if ( PNM_EQUAL( l, r ) )
    bgxel = l;
  else
    {
      /* Nope, we have to average the two corners.  This breaks the
       ** rules of pnm, but oh well.  Let's try to do it portably. */
      switch ( PNM_FORMAT_TYPE(format) )
    {
    case PPM_TYPE:
      PPM_ASSIGN( bgxel, PPM_GETR(l) + PPM_GETR(r) / 2,
             PPM_GETG(l) + PPM_GETG(r) / 2, PPM_GETB(l) + PPM_GETB(r) / 2 );
      break;

    case PGM_TYPE:
      {
        gray gl, gr;
        gl = (gray) PNM_GET1( l );
        gr = (gray) PNM_GET1( r );
        PNM_ASSIGN1( bgxel, ( ( gl + gr ) / 2 ) );
        break;
      }

    case PBM_TYPE:
      {
        int col, blacks;

        /* One black, one white.  Gotta count. */
        for ( col = 0, blacks = 0; col < cols; ++col )
          {
        if ( PNM_GET1( xelrow[col] ) == 0 )
          ++blacks;
          }
        if ( blacks >= cols / 2 )
          PNM_ASSIGN1( bgxel, 0 );
        else
          PNM_ASSIGN1( bgxel, pnm_pbmmaxval );
        break;
      }

    default:
      pm_error( "can't happen" );
    }
    }

  return bgxel;
}

#if __STDC__
xel
  pnm_whitexel( xelval maxval, int format )
#else /*__STDC__*/
xel
  pnm_whitexel( maxval, format )
xelval maxval;
int format;
#endif /*__STDC__*/
{
  xel x;

  switch ( PNM_FORMAT_TYPE(format) )
    {
    case PPM_TYPE:
      PPM_ASSIGN( x, maxval, maxval, maxval );
      break;

    case PGM_TYPE:
      PNM_ASSIGN1( x, maxval );
      break;

    case PBM_TYPE:
      PNM_ASSIGN1( x, pnm_pbmmaxval );
      break;

    default:
      pm_error( "can't happen" );
    }

  return x;
}

#if __STDC__
xel
  pnm_blackxel( xelval maxval, int format )
#else /*__STDC__*/
xel
  pnm_blackxel( maxval, format )
xelval maxval;
int format;
#endif /*__STDC__*/
{
  xel x;

  switch ( PNM_FORMAT_TYPE(format) )
    {
    case PPM_TYPE:
      PPM_ASSIGN( x, 0, 0, 0 );
      break;

    case PGM_TYPE:
      PNM_ASSIGN1( x, (xelval) 0 );
      break;

    case PBM_TYPE:
      PNM_ASSIGN1( x, (xelval) 0 );
      break;

    default:
      pm_error( "can't happen" );
    }

  return x;
}

#if __STDC__
void
  pnm_invertxel( xel* xP, xelval maxval, int format )
#else /*__STDC__*/
void
  pnm_invertxel( xP, maxval, format )
xel* xP;
xelval maxval;
int format;
#endif /*__STDC__*/
{
  switch ( PNM_FORMAT_TYPE(format) )
    {
    case PPM_TYPE:
      PPM_ASSIGN(
         *xP, maxval - PPM_GETR( *xP ),
         maxval - PPM_GETG( *xP ), maxval - PPM_GETB( *xP ) );
      break;

    case PGM_TYPE:
      PNM_ASSIGN1( *xP, (gray) maxval - (gray) PNM_GET1( *xP ) );
      break;

    case PBM_TYPE:
      PNM_ASSIGN1( *xP, ( PNM_GET1( *xP ) == 0 ) ? pnm_pbmmaxval : 0 );
      break;

    default:
      pm_error( "can't happen" );
    }
}

#if __STDC__
void
  pnm_promoteformat( xel** xels, int cols, int rows, xelval maxval, int format, xelval newmaxval, int newformat )
#else /*__STDC__*/
void
  pnm_promoteformat( xels, cols, rows, maxval, format, newmaxval, newformat )
xel** xels;
xelval maxval, newmaxval;
int cols, rows, format, newformat;
#endif /*__STDC__*/
{
  int row;

  for ( row = 0; row < rows; ++row )
    pnm_promoteformatrow(
             xels[row], cols, maxval, format, newmaxval, newformat );
}

#if __STDC__
void
  pnm_promoteformatrow( xel* xelrow, int cols, xelval maxval, int format, xelval newmaxval, int newformat )
#else /*__STDC__*/
void
  pnm_promoteformatrow( xelrow, cols, maxval, format, newmaxval, newformat )
xel* xelrow;
xelval maxval, newmaxval;
int cols, format, newformat;
#endif /*__STDC__*/
{
  register int col;
  register xel* xP;

  if ( ( PNM_FORMAT_TYPE(format) == PPM_TYPE &&
    ( PNM_FORMAT_TYPE(newformat) == PGM_TYPE ||
     PNM_FORMAT_TYPE(newformat) == PBM_TYPE ) ) ||
      ( PNM_FORMAT_TYPE(format) == PGM_TYPE &&
       PNM_FORMAT_TYPE(newformat) == PBM_TYPE ) )
    pm_error( "pnm_promoteformatrow: can't promote downwards!" );

  /* Are we promoting to the same type? */
  if ( PNM_FORMAT_TYPE(format) == PNM_FORMAT_TYPE(newformat) )
    {
      if ( PNM_FORMAT_TYPE(format) == PBM_TYPE )
    return;
      if ( newmaxval < maxval )
    pm_error(
         "pnm_promoteformatrow: can't decrease maxval - try using pnmdepth" );
      if ( newmaxval == maxval )
    return;
      /* Increase maxval. */
      switch ( PNM_FORMAT_TYPE(format) )
    {
    case PGM_TYPE:
      for ( col = 0, xP = xelrow; col < cols; ++col, ++xP )
        PNM_ASSIGN1(
            *xP, (int) PNM_GET1(*xP) * newmaxval / maxval );
      break;

    case PPM_TYPE:
      for ( col = 0, xP = xelrow; col < cols; ++col, ++xP )
        PPM_DEPTH( *xP, *xP, maxval, newmaxval );
      break;

    default:
      pm_error( "shouldn't happen" );
    }
      return;
    }

  /* We must be promoting to a higher type. */
  switch ( PNM_FORMAT_TYPE(format) )
    {
    case PBM_TYPE:
      switch ( PNM_FORMAT_TYPE(newformat) )
    {
    case PGM_TYPE:
      for ( col = 0, xP = xelrow; col < cols; ++col, ++xP )
        if ( PNM_GET1(*xP) == 0 )
          PNM_ASSIGN1( *xP, 0 );
        else
          PNM_ASSIGN1( *xP, newmaxval );
      break;

    case PPM_TYPE:
      for ( col = 0, xP = xelrow; col < cols; ++col, ++xP )
        if ( PNM_GET1(*xP) == 0 )
          PPM_ASSIGN( *xP, 0, 0, 0 );
        else
          PPM_ASSIGN( *xP, newmaxval, newmaxval, newmaxval );
      break;

    default:
      pm_error( "can't happen" );
    }
      break;

    case PGM_TYPE:
      switch ( PNM_FORMAT_TYPE(newformat) )
    {
    case PPM_TYPE:
      if ( newmaxval < maxval )
        pm_error(
             "pnm_promoteformatrow: can't decrease maxval - try using pnmdepth" );
      if ( newmaxval == maxval )
        {
          for ( col = 0, xP = xelrow; col < cols; ++col, ++xP )
        PPM_ASSIGN(
               *xP, PNM_GET1(*xP), PNM_GET1(*xP), PNM_GET1(*xP) );
        }
      else
        {            /* Increase maxval. */
          for ( col = 0, xP = xelrow; col < cols; ++col, ++xP )
        PPM_ASSIGN(
               *xP, (int) PNM_GET1(*xP) * newmaxval / maxval,
               (int) PNM_GET1(*xP) * newmaxval / maxval,
               (int) PNM_GET1(*xP) * newmaxval / maxval );
        }
      break;

    default:
      pm_error( "can't happen" );
    }
      break;

    default:
      pm_error( "can't happen" );
    }
}


