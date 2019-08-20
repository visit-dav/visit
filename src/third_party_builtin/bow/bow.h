// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

/********** bow.h created by lib **********/


#ifndef LIB_bow__TYPE

#define LIB_bow__TYPE 1

#ifndef LIB_gzlib__TYPEONLY

#define LIB_gzlib__TYPEONLY 1

#include "gzlib.h"

#undef LIB_gzlib__TYPEONLY

#else

#include "gzlib.h"

#endif

#ifndef LIB_fwav__TYPEONLY

#define LIB_fwav__TYPEONLY 1

#include "fwav.h"

#undef LIB_fwav__TYPEONLY

#else

#include "fwav.h"

#endif

typedef struct bowglobal_structdef bowglobal_struct,*bowglobal;

typedef struct bowinfo_structdef bowinfo_struct,*bowinfo;

#endif


#ifndef LIB_bow__TYPEONLY

#ifndef LIB_bow__HERE

#define LIB_bow__HERE 1

void lib_init_bow();

void lib_quit_bow();


    /* bow.l                                   brick-of-wavelet support */
    /* by Mark Duchaineau */
    /* */
    /* 09-25-00: created code in atoms2bow.l */
    /* 09-26-00: split into separate library; added notran option */
    /* 09-27-00: better rounding; added x0,y0,z0 info */
    /* 09-28-00: added ti,bi,vi info; added "catenated" bows; */
    /*           wrapped DEBUG code; changed bof_write_bof() to bow_test() */
    /* 09-29-00: cleaning/debugging */
    /* 09-30-00: added bowcat.l application; bowinfo usage */
    /* 10-19-00: added bowinfo.l application */
    /* 12-06-02: added bof2bow.l application, moved bowcrop here */
    /* 12-08-02: use of new fwav.l API and app-supplied mem allocation */
    /* 12-09-02: cleaning/debugging, added bowtest.l application */
    /* 12-12-02: made tmp file names more distinct to avoid conflicts; */
    /*           made /var/tmp the default if BOW_TMPDIR not set in envp */
    /* 12-13-02: fixed major bug in bowglobal_create (no return!); */
    /*           use of gzlib.l to replace system("gzip...") usage */
    /* 2004-09-24: improved debugging message management */
    /* 2005-03-15: fixed notran flag for ti=0 (only allow ti>=0, */
    /*              add one before negating) */
    /* 2005-03-25: work-around gcc -O2 bug in FIN (similarly FOUT, my_floor) */


    /* lib_include "stdlib.h" */
#include "stdlib.h"

    /* lib_include "stdio.h" */
#include "stdio.h"

    /* lib_include "sys/types.h" */
#include "sys/types.h"

    /* lib_include "sys/stat.h" */
#include "sys/stat.h"

    /* avoids AIX name conflict */
    
#define finfo finfo_BOWFIX
    /* lib_include "unistd.h" */
#ifndef _WIN32
#include "unistd.h"
#endif
    
#undef finfo
    /* lib_include "fcntl.h" */
#include "fcntl.h"



    /* lib_include "Gzlib/gzlib.l" */
#include "gzlib.h"



    /* lib_archive "Fwav/fwav.l" */
#include "fwav.h"


#include <bow_exports.h>
    
#define BOWGLOBAL_ERROR_MAX 8


    

/*
 * lib_struct bowglobal
 */

struct bowglobal_structdef {
        gzlib_alloctype alloc; /* application-supplied memory allocator */
        gzlib_freetype free;   /* application-supplied memory de-allocator */
        void *opaque;          /* application-supplied pass-through value */
        int error_count;       /* number of errors since startup or error_clear */
        char *error_msg;       /* most recent error message */
        char error_msgs[BOWGLOBAL_ERROR_MAX][256]; /* recent errors (wraps) */
        int no_copy;           /* 0=make copy of brick, 1=overwrite brick */
    };



    

/*
 * lib_struct bowinfo
 */

struct bowinfo_structdef {               /* information on bofs stored in bow */
        int version;           /* bow format version ('A','B',...) */
        int numbow;            /* number of bricks stored */
        int *size;             /* compressed bytes per brick */
        int *offset;           /* byte position within bow */
        int *notran;           /* 0=do transform, 1=no transform */
        int *ti;               /* timestep */
        int *bi;               /* input-data block index */
        int *vi;               /* variable index (0==wgt) */
        int *xs;               /* brick x size */
        int *ys;               /*   y size */
        int *zs;               /*   z size */
        int *x0;               /* brick x origin location in global brick */
        int *y0;               /*   y origin */
        int *z0;               /*   z origin */
        float *v0;             /* min value from bof2bow call (NOT actual min) */
        float *v1;             /* max value from bof2bow call (NOT actual max) */
    };



    bowglobal BOW_API bowglobal_create(gzlib_alloctype my_alloc,
        gzlib_freetype my_free,void *opaque);

    void BOW_API bowglobal_destroy(bowglobal bg);

    int BOW_API bof2bow(bowglobal bg,int ti,int bi,int vi,int xs,int ys,int zs,
        int x0,int y0,int z0,float *vtab,float v0,float v1,float q,
        char **rbow,int *rbowsize);

    BOW_API float *  bow2bof(bowglobal bg,char *bow,int bowi);

    bowinfo BOW_API bow_getbowinfo(bowglobal bg,char *bow);

    void BOW_API bow_freebowinfo(bowglobal bg,bowinfo binf);

    BOW_API char * bow_cat(bowglobal bg,int numbow,char **bowtab);

    int BOW_API bow_test(bowglobal bg,int ti,int bi,int vi,int xs,int ys,int zs,
        int x0,int y0,int z0,float *vsum,float v0,float v1,char *bow);

    void BOW_API bow_ntitab(bowglobal bg,int xs,int ys,int zs,int *ntab,int *rntimax);


#endif


#endif

