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

#ifndef AIR_HAS_BEEN_INCLUDED
#define AIR_HAS_BEEN_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <float.h>

/*
******** TEEM_VERSION 
**
** TEEM_VERSION is a single (decimal) number that will always increase
** monotically, and the _MAJOR, _MINOR, _PATCH are also numbers that
** can be used to implement pre-processor logic about specifc
** versions.  The TEEM_VERSION_STRING is used in the (existing) char
** *airTeemVersion (added in version 1.9.0).  Yes, keeping these in
** sync is indeed a manual operation.
**
** NOTE: Significant API changes (aside from API additions) should NOT
** occur with changes in patch level, only with major or minor version
** changes.
**
** NOTE: ../CMakeLists.txt's TEEM_VERSION variables must be in sync
*/
#define TEEM_VERSION_MAJOR   1      /* must be 1 digit */
#define TEEM_VERSION_MINOR   9      /* 1 or 2 digits */
#define TEEM_VERSION_PATCH   0      /* 1 or 2 digits */
#define TEEM_VERSION         10900  /* can be easily compared numerically */
#define TEEM_VERSION_STRING "1.9.0" /* cannot be so easily compared */

/* NrrdIO-hack-000 */

#ifdef __cplusplus
extern "C" {
#endif

/* NrrdIO-hack-001 */
#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(air_EXPORTS) || defined(teem_EXPORTS)
#    define AIR_EXPORT extern __declspec(dllexport)
#  else
#    define AIR_EXPORT extern __declspec(dllimport)
#  endif
#define AIR_EXPORT2 AIR_EXPORT
#else /* TEEM_STATIC || UNIX */
# if __GNUC__ >= 4 && (defined(air_EXPORTS) || defined(teem_EXPORTS))
#  define AIR_EXPORT __attribute__ ((visibility("default")))
# else
#  define AIR_EXPORT extern
# endif
#define AIR_EXPORT2 extern
#endif

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
typedef signed __int64 airLLong;
typedef unsigned __int64 airULLong;
#define AIR_LLONG_FMT "%I64d"
#define AIR_ULLONG_FMT "%I64u"
#define AIR_LLONG(x) x##i64
#define AIR_ULLONG(x) x##ui64
#else
typedef signed long long airLLong;
typedef unsigned long long airULLong;
#define AIR_LLONG_FMT "%lld"
#define AIR_ULLONG_FMT "%llu"
#define AIR_LLONG(x) x##ll
#define AIR_ULLONG(x) x##ull
#endif

/* This is annoying, thanks to windows */
#define AIR_PI 3.14159265358979323846
#define AIR_E  2.71828182845904523536

#define AIR_STRLEN_SMALL (128+1)
#define AIR_STRLEN_MED   (256+1)
#define AIR_STRLEN_LARGE (512+1)
#define AIR_STRLEN_HUGE  (1024+1)

/* enum.c: enum value <--> string conversion utility */  
typedef struct {
  char name[AIR_STRLEN_SMALL];
               /* what are these things? */
  unsigned int M;
               /* If "val" is NULL, the the valid enum values are from 1 
                  to M (represented by strings str[1] through str[M]), and
                  the unknown/invalid value is 0.  If "val" is non-NULL, the
                  valid enum values are from val[1] to val[M] (but again, 
                  represented by strings str[1] through str[M]), and the
                  unknown/invalid value is val[0].  In both cases, str[0]
                  is the string to represent an unknown/invalid value */
  char (*str)[AIR_STRLEN_SMALL]; 
               /* "canonical" textual representation of the enum values */
  int *val;    /* non-NULL iff valid values in the enum are not [1..M], and/or
                  if value for unknown/invalid is not zero */
  char (*desc)[AIR_STRLEN_MED];
               /* desc[i] is a short description of the enum values represented
                  by str[i] (thereby starting with the unknown value), to be
                  used to by things like hest */
  char (*strEqv)[AIR_STRLEN_SMALL];  
               /* All the variations in strings recognized in mapping from
                  string to value (the values in valEqv).  This **MUST** be
                  terminated by a zero-length string ("") so as to signify
                  the end of the list.  This should not contain the string
                  for unknown/invalid.  If "strEqv" is NULL, then mapping
                  from string to value is done by traversing "str", and 
                  "valEqv" is ignored. */
  int *valEqv; /* The values corresponding to the strings in strEqv; there
                  should be one integer for each non-zero-length string in
                  strEqv: strEqv[i] is a valid string representation for
                  value valEqv[i]. This should not contain the value for
                  unknown/invalid.  This "valEqv" is ignored if "strEqv" is
                  NULL. */
  int sense;   /* require case matching on strings */
} airEnum;
AIR_EXPORT int airEnumUnknown(const airEnum *enm);
AIR_EXPORT int airEnumValCheck(const airEnum *enm, int val);
AIR_EXPORT const char *airEnumStr(const airEnum *enm, int val);
AIR_EXPORT const char *airEnumDesc(const airEnum *enm, int val);
AIR_EXPORT int airEnumVal(const airEnum *enm, const char *str);
AIR_EXPORT char *airEnumFmtDesc(const airEnum *enm, int val, int canon,
                                const char *fmt);

/*
******** airEndian enum
**
** for identifying how a file was written to disk, for those encodings
** where the raw data on disk is dependent on the endianness of the
** architecture.
*/
enum {
  airEndianUnknown,         /* 0: nobody knows */
  airEndianLittle = 1234,   /* 1234: Intel and friends */
  airEndianBig = 4321,      /* 4321: the rest */
  airEndianLast
};
/* endianAir.c */
AIR_EXPORT2 const airEnum *const airEndian;
AIR_EXPORT2 const int airMyEndian;

/* array.c: poor-man's dynamically resizable arrays */
typedef struct {
  void *data,         /* where the data is */
    **dataP;          /* (possibly NULL) address of user's data variable,
                         kept in sync with internal "data" variable */
  unsigned int len,   /* length of array: # units for which there is
                         considered to be data (which is <= total # units
                         allocated).  The # bytes which contain data is
                         len*unit.  Always updated (unlike "*lenP") */
    *lenP,            /* (possibly NULL) address of user's length variable,
                         kept in sync with internal "len" variable */
    incr,             /* the granularity of the changes in amount of space
                         allocated: when the length reaches a multiple of
                         "incr", then the array is resized */
    size;             /* array is allocated to have "size" increments, or,
                         size*incr elements, or, 
                         size*incr*unit bytes */
  size_t unit;        /* the size in bytes of one element in the array */
  int noReallocWhenSmaller;  /* as it says */

  /* the following are all callbacks useful for maintaining either an array
     of pointers (allocCB and freeCB) or array of structs (initCB and
     doneCB).  allocCB or initCB is called when the array length increases,
     and freeCB or doneCB when it decreases.  Any of them can be NULL if no
     such activity is desired.  allocCB sets values in the array (as in
     storing the return from malloc(); freeCB is called on values in the
     array (as in calling free()), and the values are cast to void*.  allocCB
     and freeCB don't care about the value of "unit" (though perhaps they
     should).  initCB and doneCB are called on the _addresses_ of elements in
     the array.  allocCB and initCB are called for the elements in ascending
     order in the array, and freeCB and doneCB are called in descending
     order.  allocCB and initCB are mutually exclusive- they can't both be
     non-NULL. Same goes for freeCB and doneCB */
  void *(*allocCB)(void);  /* values of new elements set to return of this */
  void *(*freeCB)(void *); /* called on the values of invalidated elements */
  void (*initCB)(void *);  /* called on addresses of new elements */
  void (*doneCB)(void *);  /* called on addresses of invalidated elements */

} airArray;
AIR_EXPORT airArray *airArrayNew(void **dataP, unsigned int *lenP, size_t unit,
                                 unsigned int incr);
AIR_EXPORT void airArrayStructCB(airArray *a, void (*initCB)(void *),
                                 void (*doneCB)(void *));
AIR_EXPORT void airArrayPointerCB(airArray *a, void *(*allocCB)(void),
                                  void *(*freeCB)(void *));
AIR_EXPORT void airArrayLenSet(airArray *a, unsigned int newlen);
AIR_EXPORT void airArrayLenPreSet(airArray *a, unsigned int newlen);
AIR_EXPORT unsigned int airArrayLenIncr(airArray *a, int delta);
AIR_EXPORT airArray *airArrayNix(airArray *a);
AIR_EXPORT airArray *airArrayNuke(airArray *a);

/* ---- BEGIN non-NrrdIO */

/* threadAir.c: simplistic wrapper functions for multi-threading  */
/*
********  airThreadCapable
**
** if non-zero: we have some kind of multi-threading available, either
** via pthreads, or via Windows stuff
*/
AIR_EXPORT2 const int airThreadCapable;

/*
******** airThreadNoopWarning
**
** When multi-threading is not available, and hence constructs like
** mutexes are not available, the operations on them will be
** no-ops. When this variable is non-zero, we fprintf(stderr) a
** warning to this effect when those constructs are used
*/
AIR_EXPORT2 int airThreadNoopWarning; 

/* opaque typedefs for OS-specific stuff */
typedef struct _airThread airThread;
typedef struct _airThreadMutex airThreadMutex;
typedef struct _airThreadCond airThreadCond;
typedef struct {
  unsigned int numUsers, numDone;
  airThreadMutex *doneMutex;
  airThreadCond *doneCond;
} airThreadBarrier;

AIR_EXPORT airThread *airThreadNew(void);
AIR_EXPORT int airThreadStart(airThread *thread, 
                              void *(*threadBody)(void *), void *arg);
AIR_EXPORT int airThreadJoin(airThread *thread, void **retP);
AIR_EXPORT airThread *airThreadNix(airThread *thread);

AIR_EXPORT airThreadMutex *airThreadMutexNew();
AIR_EXPORT int airThreadMutexLock(airThreadMutex *mutex);
AIR_EXPORT int airThreadMutexUnlock(airThreadMutex *mutex);
AIR_EXPORT airThreadMutex *airThreadMutexNix(airThreadMutex *mutex);

AIR_EXPORT airThreadCond *airThreadCondNew();
AIR_EXPORT int airThreadCondWait(airThreadCond *cond, airThreadMutex *mutex);
AIR_EXPORT int airThreadCondSignal(airThreadCond *cond);
AIR_EXPORT int airThreadCondBroadcast(airThreadCond *cond);
AIR_EXPORT airThreadCond *airThreadCondNix(airThreadCond *cond);

AIR_EXPORT airThreadBarrier *airThreadBarrierNew(unsigned numUsers);
AIR_EXPORT int airThreadBarrierWait(airThreadBarrier *barrier);
AIR_EXPORT airThreadBarrier *airThreadBarrierNix(airThreadBarrier *barrier);

/* ---- END non-NrrdIO */

/*
******** airFP enum
**
** the different kinds of floating point number afforded by IEEE 754,
** and the values returned by airFPClass_f().
**
** The values probably won't agree with those in #include's like
** ieee.h, ieeefp.h, fp_class.h.  This is because IEEE 754 hasn't
** defined standard values for these, so everyone does it differently.
** 
** This enum uses underscores (against Teem convention) to help
** legibility while also conforming to the spirit of the somewhat
** standard naming conventions
*/
enum {
  airFP_Unknown,               /*  0: nobody knows */
  airFP_SNAN,                  /*  1: signalling NaN */
  airFP_QNAN,                  /*  2: quiet NaN */
  airFP_POS_INF,               /*  3: positive infinity */
  airFP_NEG_INF,               /*  4: negative infinity */
  airFP_POS_NORM,              /*  5: positive normalized non-zero */
  airFP_NEG_NORM,              /*  6: negative normalized non-zero */
  airFP_POS_DENORM,            /*  7: positive denormalized non-zero */
  airFP_NEG_DENORM,            /*  8: negative denormalized non-zero */
  airFP_POS_ZERO,              /*  9: +0.0, positive zero */
  airFP_NEG_ZERO,              /* 10: -0.0, negative zero */
  airFP_Last                   /* after the last valid one */
};
/* 754.c: IEEE-754 related stuff values */
typedef union {
  unsigned int i;
  float f;
} airFloat;
typedef union {
  airULLong i;
  double d;
} airDouble;
AIR_EXPORT2 const int airMyQNaNHiBit;
AIR_EXPORT float airFPPartsToVal_f(unsigned int sign, 
                                   unsigned int expo, 
                                   unsigned int mant);
AIR_EXPORT void airFPValToParts_f(unsigned int *signP, 
                                  unsigned int *expoP, 
                                  unsigned int *mantP, float v);
AIR_EXPORT double airFPPartsToVal_d(unsigned int sign, 
                                    unsigned int expo,
                                    unsigned int mant0,
                                    unsigned int mant1);
AIR_EXPORT void airFPValToParts_d(unsigned int *signP, 
                                  unsigned int *expoP,
                                  unsigned int *mant0P,
                                  unsigned int *mant1P,
                                  double v);
AIR_EXPORT float airFPGen_f(int cls);
AIR_EXPORT double airFPGen_d(int cls);
AIR_EXPORT int airFPClass_f(float val);
AIR_EXPORT int airFPClass_d(double val);
AIR_EXPORT void airFPFprintf_f(FILE *file, float val);
AIR_EXPORT void airFPFprintf_d(FILE *file, double val);
AIR_EXPORT2 const airFloat airFloatQNaN;
AIR_EXPORT2 const airFloat airFloatSNaN;
AIR_EXPORT2 const airFloat airFloatPosInf;
AIR_EXPORT2 const airFloat airFloatNegInf;
AIR_EXPORT float airNaN(void);
AIR_EXPORT int airIsNaN(double d);
AIR_EXPORT int airIsInf_f(float f);
AIR_EXPORT int airIsInf_d(double d);
AIR_EXPORT int airExists(double d);

/* ---- BEGIN non-NrrdIO */

#define AIR_RANDMT_N 624
typedef struct {
  /* These need to be at least 32 bits */
  unsigned int state[AIR_RANDMT_N], /* internal state */
    *pNext,                         /* next value to get from state */
    left;                           /* number of values left before
                                       reload needed */
} airRandMTState;
/* randMT.c */
AIR_EXPORT2 airRandMTState *airRandMTStateGlobal;
AIR_EXPORT airRandMTState *airRandMTStateNew(unsigned int seed);
AIR_EXPORT airRandMTState *airRandMTStateNix(airRandMTState *state);
AIR_EXPORT void airSrandMT_r(airRandMTState *state, unsigned int seed);
AIR_EXPORT double airDrandMT_r(airRandMTState *state);       /* [0,1] */
AIR_EXPORT unsigned int airUIrandMT_r(airRandMTState *state);
AIR_EXPORT double airDrandMT53_r(airRandMTState *state);     /* [0,1) */
AIR_EXPORT void airSrandMT(unsigned int seed);
AIR_EXPORT double airDrandMT();

/* ---- END non-NrrdIO */

/*
******** airType
**
** Different types which air cares about.
** Currently only used in the command-line parsing, but perhaps will
** be used elsewhere in air later
*/
enum {
  airTypeUnknown,   /*  0 */
  airTypeBool,      /*  1 */
  airTypeInt,       /*  2 */
  airTypeUInt,      /*  3 */
  airTypeSize_t,    /*  4 */
  airTypeFloat,     /*  5 */
  airTypeDouble,    /*  6 */
  airTypeChar,      /*  7 */
  airTypeString,    /*  8 */
  airTypeEnum,      /*  9 */
  airTypeOther,     /* 10 */
  airTypeLast
};
#define AIR_TYPE_MAX   10
/* parseAir.c */
AIR_EXPORT double airAtod(const char *str);
AIR_EXPORT int airSingleSscanf(const char *str, const char *fmt, void *ptr);
AIR_EXPORT2 const airEnum *const airBool;
AIR_EXPORT unsigned int airParseStrB(int *out, const char *s,
                                     const char *ct, unsigned int n, 
                                     ... /* (nothing used) */);
AIR_EXPORT unsigned int airParseStrI(int *out, const char *s,
                                     const char *ct, unsigned int n,
                                     ... /* (nothing used) */);
AIR_EXPORT unsigned int airParseStrUI(unsigned int *out, const char *s,
                                      const char *ct, unsigned int n,
                                      ... /* (nothing used) */);
AIR_EXPORT unsigned int airParseStrZ(size_t *out, const char *s,
                                     const char *ct, unsigned int n,
                                     ... /* (nothing used) */);
AIR_EXPORT unsigned int airParseStrF(float *out, const char *s,
                                     const char *ct, unsigned int n,
                                     ... /* (nothing used) */);
AIR_EXPORT unsigned int airParseStrD(double *out, const char *s,
                                     const char *ct, unsigned int n,
                                     ... /* (nothing used) */);
AIR_EXPORT unsigned int airParseStrC(char *out, const char *s,
                                     const char *ct, unsigned int n,
                                     ... /* (nothing used) */);
AIR_EXPORT unsigned int airParseStrS(char **out, const char *s,
                                     const char *ct, unsigned int n,
                                     ... /* REQ'D even if n>1: int greedy */);
AIR_EXPORT unsigned int airParseStrE(int *out, const char *s,
                                     const char *ct, unsigned int n, 
                                     ... /* REQUIRED: airEnum *e */);
AIR_EXPORT2 unsigned int (*airParseStr[AIR_TYPE_MAX+1])(void *, const char *,
                                                       const char *,
                                                       unsigned int, ...);

/* string.c */
AIR_EXPORT char *airStrdup(const char *s);
AIR_EXPORT size_t airStrlen(const char *s);
AIR_EXPORT2 int airStrtokQuoting;
AIR_EXPORT char *airStrtok(char *s, const char *ct, char **last);
AIR_EXPORT unsigned int airStrntok(const char *s, const char *ct);
AIR_EXPORT char *airStrtrans(char *s, char from, char to);
AIR_EXPORT int airEndsWith(const char *s, const char *suff);
AIR_EXPORT char *airUnescape(char *s);
AIR_EXPORT char *airOneLinify(char *s);
AIR_EXPORT char *airToLower(char *str);
AIR_EXPORT char *airToUpper(char *str);
AIR_EXPORT unsigned int airOneLine(FILE *file, char *line, int size);

/* sane.c */
/*
******** airInsane enum
** 
** reasons for why airSanity() failed (specifically, the possible
** return values for airSanity()
*/
enum {
  airInsane_not,           /*  0: actually, all sanity checks passed */
  airInsane_endian,        /*  1: airMyEndian is wrong */
  airInsane_pInfExists,    /*  2: AIR_EXISTS(positive infinity) was true */
  airInsane_nInfExists,    /*  3: AIR_EXISTS(negative infinity) was true */
  airInsane_NaNExists,     /*  4: AIR_EXISTS(NaN) was true */
  airInsane_FltDblFPClass, /*  5: double -> float assignment messed up the
                               airFPClass_f() of the value */
  airInsane_QNaNHiBit,     /*  6: airMyQNaNHiBit is wrong */
  airInsane_dio,           /*  7: airMyDio set to something invalid */
  airInsane_32Bit,         /*  8: airMy32Bit is wrong */
  airInsane_UCSize,        /*  9: unsigned char isn't 8 bits */
  airInsane_FISize,        /* 10: sizeof(float), sizeof(int) not 4 */
  airInsane_DLSize         /* 11: sizeof(double), sizeof(airLLong) not 8 */
};
#define AIR_INSANE_MAX        11
AIR_EXPORT const char *airInsaneErr(int insane);
AIR_EXPORT int airSanity(void);

/* miscAir.c */
AIR_EXPORT2 const char *airTeemVersion;
AIR_EXPORT2 const char *airTeemReleaseDate;
AIR_EXPORT void *airNull(void);
AIR_EXPORT void *airSetNull(void **ptrP);
AIR_EXPORT void *airFree(void *ptr);
AIR_EXPORT FILE *airFopen(const char *name, FILE *std, const char *mode);
AIR_EXPORT FILE *airFclose(FILE *file);
AIR_EXPORT int airSinglePrintf(FILE *file, char *str, const char *fmt, ...);
AIR_EXPORT2 const int airMy32Bit;
/* ---- BEGIN non-NrrdIO */
AIR_EXPORT unsigned int airIndex(double min, double val, double max,
                                 unsigned int N);
AIR_EXPORT unsigned int airIndexClamp(double min, double val, double max,
                                      unsigned int N);
AIR_EXPORT airULLong airIndexULL(double min, double val, double max,
                                 airULLong N);
AIR_EXPORT airULLong airIndexClampULL(double min, double val, double max,
                                      airULLong N);
AIR_EXPORT2 const char airMyFmt_size_t[];
AIR_EXPORT char *airDoneStr(double start, double here, double end, char *str);
AIR_EXPORT double airTime();
AIR_EXPORT void airBinaryPrintUInt(FILE *file, int digits, unsigned int N);
AIR_EXPORT2 const char airTypeStr[AIR_TYPE_MAX+1][AIR_STRLEN_SMALL];
AIR_EXPORT2 const size_t airTypeSize[AIR_TYPE_MAX+1];
AIR_EXPORT int airILoad(void *v, int t);
AIR_EXPORT float airFLoad(void *v, int t);
AIR_EXPORT double airDLoad(void *v, int t);
AIR_EXPORT int airIStore(void *v, int t, int i);
AIR_EXPORT float airFStore(void *v, int t, float f);
AIR_EXPORT double airDStore(void *v, int t, double d);

/* math.c */
AIR_EXPORT void airNormalRand(double *z1, double *z2);
AIR_EXPORT void airNormalRand_r(double *z1, double *z2,
                                airRandMTState *state);
AIR_EXPORT unsigned int airRandInt(unsigned int N);
AIR_EXPORT unsigned int airRandInt_r(airRandMTState *state, unsigned int N);
AIR_EXPORT void airShuffle(unsigned int *buff, unsigned int N, int perm);
AIR_EXPORT void airShuffle_r(airRandMTState *state,
                             unsigned int *buff, unsigned int N,
                             int perm);
AIR_EXPORT double airCbrt(double);
AIR_EXPORT double airSgnPow(double, double);
AIR_EXPORT int airSgn(double);
AIR_EXPORT int airLog2(double n);
AIR_EXPORT double airErfc(double x);
AIR_EXPORT double airErf(double x);
AIR_EXPORT double airGaussian(double x, double mean, double stdv);
AIR_EXPORT double airBesselI0(double x);
AIR_EXPORT double airBesselI1(double x);
AIR_EXPORT double airLogBesselI0(double x);
AIR_EXPORT double airBesselI1By0(double x);
/* ---- END non-NrrdIO */

/* dio.c */
/*
******** airNoDio enum
**
** reasons for why direct I/O won't be used with a particular 
** file/pointer combination
*/
enum {
  airNoDio_okay,    /*  0: actually, you CAN do direct I/O */
  airNoDio_arch,    /*  1: Teem thinks this architecture can't do it */
  airNoDio_format,  /*  2: Teem thinks given data file format can't use it */
  airNoDio_std,     /*  3: DIO isn't possible for std{in|out|err} */
  airNoDio_fd,      /*  4: couldn't get underlying file descriptor */
  airNoDio_dioinfo, /*  5: calling fcntl() to get direct I/O info failed */
  airNoDio_small,   /*  6: requested size is too small */
  airNoDio_size,    /*  7: requested size not a multiple of d_miniosz */
  airNoDio_ptr,     /*  8: pointer not multiple of d_mem */
  airNoDio_fpos,    /*  9: current file position not multiple of d_miniosz */
  airNoDio_setfl,   /* 10: fcntl(fd, SETFL, FDIRECT) failed */
  airNoDio_test,    /* 11: couldn't memalign() even a small bit of memory */
  airNoDio_disable  /* 12: someone disabled it with airDisableDio */
};
#define AIR_NODIO_MAX  12
AIR_EXPORT const char *airNoDioErr(int noDio);
AIR_EXPORT2 const int airMyDio;
AIR_EXPORT2 int airDisableDio;
AIR_EXPORT void airDioInfo(int *align, int *min, int *max, int fd);
AIR_EXPORT int airDioTest(int fd, const void *ptr, size_t size);
AIR_EXPORT void *airDioMalloc(size_t size, int fd);
AIR_EXPORT size_t airDioRead(int fd, void *ptr, size_t size);
AIR_EXPORT size_t airDioWrite(int fd, const void *ptr, size_t size);

/* mop.c: clean-up utilities */
enum {
  airMopNever,
  airMopOnError,
  airMopOnOkay,
  airMopAlways
};
typedef void *(*airMopper)(void *);
typedef struct {
  void *ptr;         /* the thing to be processed */
  airMopper mop;     /* the function to which does the processing */
  int when;          /* from the airMopWhen enum */
} airMop;
AIR_EXPORT airArray *airMopNew(void);
AIR_EXPORT void airMopAdd(airArray *arr, void *ptr, airMopper mop, int when);
AIR_EXPORT void airMopSub(airArray *arr, void *ptr, airMopper mop);
AIR_EXPORT void airMopMem(airArray *arr, void *_ptrP, int when);
AIR_EXPORT void airMopUnMem(airArray *arr, void *_ptrP);
AIR_EXPORT void airMopPrint(airArray *arr, const void *_str, int when);
AIR_EXPORT void airMopDone(airArray *arr, int error);
AIR_EXPORT void airMopError(airArray *arr);
AIR_EXPORT void airMopOkay(airArray *arr);
AIR_EXPORT void airMopDebug(airArray *arr);

/*******     the interminable sea of defines and macros     *******/

#define AIR_TRUE 1
#define AIR_FALSE 0
#define AIR_WHITESPACE " \t\n\r\v\f"       /* K+R pg. 157 */

/*
******** AIR_UNUSED
**
** one way of reconciling "warning: unused parameter" with
** C's "error: parameter name omitted"
*/
#define AIR_UNUSED(x) (void)(x)

/*
******** AIR_CAST
**
** just a cast, but with the added ability to grep for it more easily,
** since casts should probably always be revisited and reconsidered.
*/
#define AIR_CAST(t, v) ((t)(v))

/*
******** AIR_ENDIAN, AIR_QNANHIBIT, AIR_DIO
**
** These reflect particulars of hardware which we're running on.
** The reason to have these in addition to TEEM_ENDIAN, TEEM_DIO, etc.,
** is that those are not by default defined for every source-file
** compilation: the Teem library has to define NEED_ENDIAN, NEED_DIO, etc,
** and these in turn generate appropriate compile command-line flags
** by Common.mk. By having these defined here, they become available
** to anyone who simply links against the air library (and includes air.h),
** with no command-line flags required, and no usage of Common.mk required.
*/
#define AIR_ENDIAN (airMyEndian)
#define AIR_QNANHIBIT (airMyQNaNHiBit)
#define AIR_DIO (airMyDio)
#define AIR_32BIT (airMy32Bit)

/*
******** AIR_NAN, AIR_QNAN, AIR_SNAN, AIR_POS_INF, AIR_NEG_INF
**
** its nice to have these values available without the cost of a 
** function call.
**
** NOTE: AIR_POS_INF and AIR_NEG_INF correspond to the _unique_
** bit-patterns which signify positive and negative infinity.  With
** the NaNs, however, they are only one of many possible
** representations.
*/
#define AIR_NAN  (airFloatQNaN.f)
#define AIR_QNAN (airFloatQNaN.f)
#define AIR_SNAN (airFloatSNaN.f)
#define AIR_POS_INF (airFloatPosInf.f)
#define AIR_NEG_INF (airFloatNegInf.f)

/* 
******** AIR_EXISTS
**
** is non-zero (true) only for values which are not NaN or +/-infinity
** 
** You'd think that (x == x) might work, but no no no, some optimizing
** compilers (e.g. SGI's cc) say "well of course they're equal, for all
** possible values".  Bastards!
**
** One of the benefits of IEEE 754 floating point numbers is that
** gradual underflow means that x = y <==> x - y = 0 for any (positive
** or negative) normalized or denormalized float.  Otherwise this
** macro could not be valid; some floating point conventions say that
** a zero-valued exponent means zero, regardless of the mantissa.
**
** However, there MAY be problems on machines which use extended
** (80-bit) floating point registers, such as Intel chips- where the
** same initial value 1) directly read from the register, versus 2)
** saved to memory and loaded back, may end up being different.  I
** have yet to produce this behavior, or convince myself it can't
** happen.  If you have problems, then use the version of the macro
** which is a function call to airExists_d(), and please email me:
** gk@bwh.harvard.edu
**
** The reason to #define AIR_EXISTS as airExists_d is that on some
** optimizing compilers, the !((x) - (x)) doesn't work.  This has been
** the case on Windows and 64-bit irix6 (64 bit) with -Ofast.  If
** airSanity fails because a special value "exists", then use the
** first version of AIR_EXISTS.
**
** There are two performance consequences of using airExists_d(x):
** 1) Its a function call (but WIN32 can __inline it)
** 2) (via AIR_EXISTS_D) It requires bit-wise operations on 64-bit
** ints, which might be terribly slow.
**
** The reason for using airExists_d and not airExists_f is for
** doubles > FLT_MAX: airExists_f would say these are infinity.
*/
#if defined(_WIN32) || defined(__ECC) /* NrrdIO-hack-002 */
#define AIR_EXISTS(x) (airExists(x))
#else
#define AIR_EXISTS(x) (!((x) - (x)))
#endif

/* ---- BEGIN non-NrrdIO */

/*
******** AIR_EXISTS_F(x)
**
** This is another way to check for non-specialness (not NaN, not
** +inf, not -inf) of a _float_, by making sure the exponent field
** isn't all ones.
**
** Unlike !((x) - (x)) or airExists(x), the argument to this macro
** MUST MUST MUST be a float, and the float must be of the standard
** 32-bit size, which must also be the size of an int.  The reason for
** this constraint is that macros are not functions, so there is no
** implicit cast or conversion to a single type.  Casting the address
** of the macro arg to an int* only works when the arg has the same
** size as an int.
**
** No cross-platform comparitive timings have been done to compare the
** speed of !((x) - (x)) versus airExists() versus AIR_EXISTS_F()
** 
** This macro is endian-safe.
*/
#define AIR_EXISTS_F(x) ((*(unsigned int*)&(x) & 0x7f800000) != 0x7f800000)

/*
******** AIR_EXISTS_D(x)
**
** like AIR_EXISTS_F(), but the argument here MUST be a double
*/
#define AIR_EXISTS_D(x) (                               \
  (*(airULLong*)&(x) & AIR_ULLONG(0x7ff0000000000000))  \
    != AIR_ULLONG(0x7ff0000000000000))

/*
******** AIR_ISNAN_F(x)
**
** detects if a float is NaN by looking at the bits, without relying on
** any of its arithmetic properties.  As with AIR_EXISTS_F(), this only
** works when the argument really is a float, and when floats are 4-bytes
*/
#define AIR_ISNAN_F(x) (((*(unsigned int*)&(x) & 0x7f800000)==0x7f800000) && \
                         (*(unsigned int*)&(x) & 0x007fffff))

/* ---- END non-NrrdIO */

/*
******** AIR_MAX(a,b), AIR_MIN(a,b), AIR_ABS(a)
**
** the usual
*/
#define AIR_MAX(a,b) ((a) > (b) ? (a) : (b))
#define AIR_MIN(a,b) ((a) < (b) ? (a) : (b))
#define AIR_ABS(a) ((a) > 0.0f ? (a) : -(a))

/*
******** AIR_COMPARE(a,b)
**
** the sort of compare that qsort() wants for ascending sort
*/
#define AIR_COMPARE(a,b) ((a) < (b)     \
                          ? -1          \
                          : ((a) > (b) \
                             ? 1        \
                             : 0))

/*
******** AIR_IN_OP(a,b,c), AIR_IN_CL(a,b,c)
**
** is true if the middle argument is in the open/closed interval
** defined by the first and third arguments
** 
** AIR_IN_OP is new name for old AIR_BETWEEN
** AIR_IN_CL is new name for odl AIR_INSIDE
*/
#define AIR_IN_OP(a,b,c) ((a) < (b) && (b) < (c))     /* closed interval */
#define AIR_IN_CL(a,b,c) ((a) <= (b) && (b) <= (c))   /* open interval */

/*
******** AIR_CLAMP(a,b,c)
**
** returns the middle argument, after being clamped to the closed
** interval defined by the first and third arguments
*/
#define AIR_CLAMP(a,b,c) ((b) < (a)        \
                           ? (a)           \
                           : ((b) > (c)    \
                              ? (c)        \
                              : (b)))

/*
******** AIR_MOD(i, N)
**
** returns that integer in [0, N-1] which is i plus a multiple of N. It
** may be unfortunate that the expression (i)%(N) appears three times;
** this should be inlined.  Or perhaps the compiler's optimizations
** (common sub-expression elimination) will save us.
**
** Note: integer divisions are not very fast on some modern chips;
** don't go silly using this one.
*/
#define AIR_MOD(i, N) ((i)%(N) >= 0 ? (i)%(N) : N + (i)%(N))

/*
******** AIR_LERP(w, a, b)
**
** returns a when w=0, and b when w=1, and linearly varies in between
*/
#define AIR_LERP(w, a, b) ((w)*((b) - (a)) + (a))

/*
******** AIR_AFFINE(i,x,I,o,O)
**
** given intervals [i,I], [o,O] and a value x which may or may not be
** inside [i,I], return the value y such that y stands in the same
** relationship to [o,O] that x does with [i,I].  Or:
**
**    y - o         x - i     
**   -------   =   -------
**    O - o         I - i
**
** It is the callers responsibility to make sure I-i and O-o are 
** both non-zero.  Strictly speaking, real problems arise only when
** when I-i is zero: division by zero generates either NaN or infinity
*/
#define AIR_AFFINE(i,x,I,o,O) ( \
((double)(O)-(o))*((double)(x)-(i)) / ((double)(I)-(i)) + (o))

/*
******** AIR_DELTA(i,x,I,o,O)
**
** given intervals [i,I] and [o,O], calculates the number y such that
** a change of x within [i,I] is proportional to a change of y within
** [o,O].  Or:
**
**      y             x     
**   -------   =   -------
**    O - o         I - i
**
** It is the callers responsibility to make sure I-i and O-o are 
** both non-zero
*/
#define AIR_DELTA(i,x,I,o,O) ( \
((double)(O)-(o))*((double)(x)) / ((double)(I)-(i)) )

/*
******** AIR_ROUNDUP, AIR_ROUNDDOWN
**
** rounds integers up or down; just wrappers around floor and ceil
*/
#define AIR_ROUNDUP(x)   ((int)(floor((x)+0.5)))
#define AIR_ROUNDDOWN(x) ((int)(ceil((x)-0.5)))

/*
******** _AIR_SIZE_T_CNV, _AIR_PTRDIFF_T_CNV, 
**
** Conversion sequence to use when printf/fprintf/sprintf-ing a value of
** type size_t or ptrdiff_t.  In C99, this is done with "%z" and "%t",
** respecitvely.
**
** This is not a useful macro for the world at large- only for Teem
** source files.  Why: we need to leave this as a bare string, so that
** we can exploit C's implicit string concatenation in forming a
** format string.  Therefore, unlike the definition of AIR_ENDIAN,
** AIR_DIO, etc, _AIR_SIZE_T_CNV can NOT just refer to a const variable
** (like airMyEndian).  Therefore, TEEM_32BIT has to be defined for
** ALL source files which want to use _AIR_SIZE_T_CNV, and to be
** conservative, that's all Teem files.  The converse is, since there is
** no expectation that other projects which use Teem will be defining
** TEEM_32BIT, this is not useful outside Teem, thus the leading _.
*/
#ifdef __APPLE__
#  define _AIR_SIZE_T_CNV "%lu"
#  define _AIR_PTRDIFF_T_CNV "%d"
#else
#  if TEEM_32BIT == 0
#    define _AIR_SIZE_T_CNV "%lu"
#    define _AIR_PTRDIFF_T_CNV "%ld"
#  elif TEEM_32BIT == 1
#    define _AIR_SIZE_T_CNV "%u"
#    define _AIR_PTRDIFF_T_CNV "%d"
#  else
#    define _AIR_SIZE_T_CNV "(no _AIR_SIZE_T_CNV w/out TEEM_32BIT %*d)"
#    define _AIR_PTRDIFF_T_CNV "(no _AIR_PTRDIFF_T_CNV w/out TEEM_32BIT %*d)"
#  endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* AIR_HAS_BEEN_INCLUDED */
