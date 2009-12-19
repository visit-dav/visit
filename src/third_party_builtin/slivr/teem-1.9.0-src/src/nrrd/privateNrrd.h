/*
  Teem: Tools to process and visualize scientific data and images
  Copyright (C) 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  (LGPL) as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  The terms of redistributing and/or modifying this software also
  include exceptions to the LGPL that facilitate static linking.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef NRRD_PRIVATE_HAS_BEEN_INCLUDED
#define NRRD_PRIVATE_HAS_BEEN_INCLUDED

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define _NRRD_TEXT_INCR 1024

/* ---- BEGIN non-NrrdIO */

#if NRRD_RESAMPLE_FLOAT
#  define nrrdResample_nrrdType nrrdTypeFloat
#  define EVALN evalN_f               /* NrrdKernel method */
#else
#  define nrrdResample_nrrdType nrrdTypeDouble
#  define EVALN evalN_d               /* NrrdKernel method */
#endif

/* to access whatever nrrd there may be in in a NrrdIter */
#define _NRRD_ITER_NRRD(iter) ((iter)->nrrd ? (iter)->nrrd : (iter)->ownNrrd)

/* ---- END non-NrrdIO */

/*
** _NRRD_SPACING
**
** returns nrrdDefSpacing if the argument doesn't exist, otherwise
** returns the argument
*/
#define _NRRD_SPACING(spc) (AIR_EXISTS(spc) ? spc: nrrdDefSpacing)

typedef union {
  char **CP;
  int *I;
  unsigned int *UI;
  size_t *ST;
  double *D;
  const void *P;
  double (*V)[NRRD_SPACE_DIM_MAX];
} _nrrdAxisInfoSetPtrs;

typedef union {
  char **CP;
  int *I;
  unsigned int *UI;
  size_t *ST;
  double *D;
  void *P;
  double (*V)[NRRD_SPACE_DIM_MAX];
} _nrrdAxisInfoGetPtrs;

/* keyvalue.c */
extern int _nrrdKeyValueWrite(FILE *file, char **stringP, const char *prefix,
                              const char *key, const char *value);

/* formatXXX.c */
extern const char *_nrrdFormatURLLine0;
extern const char *_nrrdFormatURLLine1;
extern const NrrdFormat _nrrdFormatNRRD;
extern const NrrdFormat _nrrdFormatPNM;
extern const NrrdFormat _nrrdFormatPNG;
extern const NrrdFormat _nrrdFormatVTK;
extern const NrrdFormat _nrrdFormatText;
extern const NrrdFormat _nrrdFormatEPS;
extern int _nrrdHeaderCheck(Nrrd *nrrd, NrrdIoState *nio, int checkSeen);
extern int _nrrdFormatNRRD_whichVersion(const Nrrd *nrrd, NrrdIoState *nio);

/* encodingXXX.c */
extern const NrrdEncoding _nrrdEncodingRaw;
extern const NrrdEncoding _nrrdEncodingAscii;
extern const NrrdEncoding _nrrdEncodingHex;
extern const NrrdEncoding _nrrdEncodingGzip;
extern const NrrdEncoding _nrrdEncodingBzip2;

/* read.c */
extern int _nrrdCalloc(Nrrd *nrrd, NrrdIoState *nio, FILE *file);
extern char _nrrdFieldSep[];

/* arrays.c */
extern const int _nrrdFieldValidInImage[NRRD_FIELD_MAX+1];
extern const int _nrrdFieldValidInText[NRRD_FIELD_MAX+1];
extern const int _nrrdFieldOnePerAxis[NRRD_FIELD_MAX+1];
extern const char _nrrdEnumFieldStr[NRRD_FIELD_MAX+1][AIR_STRLEN_SMALL];
extern const int _nrrdFieldRequired[NRRD_FIELD_MAX+1];

/* simple.c */
extern char *_nrrdContentGet(const Nrrd *nin);
extern int _nrrdContentSet_nva(Nrrd *nout, const char *func,
                               char *content, const char *format,
                               va_list arg);
extern int _nrrdContentSet_va(Nrrd *nout, const char *func,
                              char *content, const char *format, ...);
extern int _nrrdFieldCheckSpaceInfo(const Nrrd *nrrd, int useBiff);
extern int (*_nrrdFieldCheck[NRRD_FIELD_MAX+1])(const Nrrd *nrrd, int useBiff);
extern void _nrrdSplitSizes(size_t *pieceSize, size_t *pieceNum, 
                            Nrrd *nrrd, unsigned int listDim);
extern void _nrrdSpaceVecCopy(double dst[NRRD_SPACE_DIM_MAX], 
                              const double src[NRRD_SPACE_DIM_MAX]);
extern void _nrrdSpaceVecScaleAdd2(double sum[NRRD_SPACE_DIM_MAX], 
                                   double sclA, 
                                   const double vecA[NRRD_SPACE_DIM_MAX],
                                   double sclB, 
                                   const double vecB[NRRD_SPACE_DIM_MAX]);
extern void _nrrdSpaceVecScale(double out[NRRD_SPACE_DIM_MAX], 
                               double scl, 
                               const double vec[NRRD_SPACE_DIM_MAX]);
extern double _nrrdSpaceVecNorm(int sdim,
                                const double vec[NRRD_SPACE_DIM_MAX]);
extern void _nrrdSpaceVecSetNaN(double vec[NRRD_SPACE_DIM_MAX]);


/* axis.c */
extern int _nrrdKindAltered(int kindIn, int resampling);
extern void _nrrdAxisInfoCopy(NrrdAxisInfo *dest, const NrrdAxisInfo *src,
                              int bitflag);
extern void _nrrdAxisInfoInit(NrrdAxisInfo *axis);
extern void _nrrdAxisInfoNewInit(NrrdAxisInfo *axis);
extern int _nrrdCenter(int center);
extern int _nrrdCenter2(int center, int def);

/* convert.c */
extern void (*_nrrdConv[][NRRD_TYPE_MAX+1])(void *, const void *, size_t);

/* read.c */
extern char _nrrdFieldStr[NRRD_FIELD_MAX+1][AIR_STRLEN_SMALL];
extern char _nrrdRelativePathFlag[];
extern char _nrrdFieldSep[];
extern char _nrrdNoSpaceVector[];
extern char _nrrdTextSep[];
/* ---- BEGIN non-NrrdIO */
extern int _nrrdReshapeUpGrayscale(Nrrd *nimg);
/* ---- END non-NrrdIO */
extern void _nrrdSplitName(char **dirP, char **baseP, const char *name);

/* write.c */
extern int _nrrdFieldInteresting (const Nrrd *nrrd, NrrdIoState *nio,
                                  int field);
extern void _nrrdSprintFieldInfo(char **strP, char *prefix,
                                 const Nrrd *nrrd, NrrdIoState *nio,
                                 int field);
extern void _nrrdFprintFieldInfo(FILE *file, char *prefix,
                                 const Nrrd *nrrd, NrrdIoState *nio,
                                 int field);
/* ---- BEGIN non-NrrdIO */
extern int _nrrdReshapeDownGrayscale(Nrrd *nimg);
/* ---- END non-NrrdIO */

/* parseNrrd.c */
extern int _nrrdReadNrrdParseField(NrrdIoState *nio, int useBiff);

/* methodsNrrd.c */
extern void nrrdPeripheralInit(Nrrd *nrrd);
extern int nrrdPeripheralCopy(Nrrd *nout, const Nrrd *nin);
extern int _nrrdCopy(Nrrd *nout, const Nrrd *nin, int bitflag);
extern int _nrrdSizeCheck(const size_t *size, unsigned int dim, int useBiff);
extern void _nrrdTraverse(Nrrd *nrrd);

#if TEEM_ZLIB
#include <zlib.h>

/* gzio.c */
extern gzFile _nrrdGzOpen(FILE* fd, const char *mode);
extern int _nrrdGzClose(gzFile file);
extern int _nrrdGzRead(gzFile file, voidp buf, unsigned int len,
                       unsigned int* read);
extern int _nrrdGzWrite(gzFile file, const voidp buf, unsigned int len,
                        unsigned int* written);
#endif

/* ---- BEGIN non-NrrdIO */
/* ccmethods.c */
extern unsigned int _nrrdCC_settle(unsigned int *map, unsigned int len);
extern unsigned int _nrrdCC_eclass(unsigned int *map, unsigned int len,
                                   airArray *eqvArr);

/* apply1D.c */
extern double _nrrdApplyDomainMin(const Nrrd *nmap, int ramps, int mapAxis);
extern double _nrrdApplyDomainMax(const Nrrd *nmap, int ramps, int mapAxis);

/* ---- END non-NrrdIO */

#ifdef __cplusplus
}
#endif

#endif /* NRRD_PRIVATE_HAS_BEEN_INCLUDED */
