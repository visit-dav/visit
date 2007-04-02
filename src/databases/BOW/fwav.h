

/********** fwav.h created by lib **********/


#ifndef LIB_fwav__TYPE

#define LIB_fwav__TYPE 1

#ifndef LIB_gzlib__TYPEONLY

#define LIB_gzlib__TYPEONLY 1

#include "gzlib.h"

#undef LIB_gzlib__TYPEONLY

#else

#include "gzlib.h"

#endif

typedef struct fwav_structdef fwav_struct,*fwav;

#endif


#ifndef LIB_fwav__TYPEONLY

#ifndef LIB_fwav__HERE

#define LIB_fwav__HERE 1

void lib_init_fwav();

void lib_quit_fwav();


    /* fwav.l         linear wavelet transform library for bricks of floats */
    /* by Mark Duchaineau (free but copyrighted, see LibGen/COPYING) */
    /* */
    /* 09-21-00: based on 12-11-99 Wav/wav.l library */
    /* 09-24-00: allow non power-of-two bricks; added fwavtest.l application */
    /* 09-25-00: fixed stran() failure to init _zero */
    /* 12-08-02: new API to be thread safe, use app-supplied mem allocation */
    /* 12-13-02: API change adds opaque arg to fwav_create() */


    /* lib_include "stdlib.h" */
#include "stdlib.h"

    /* lib_include "Gzlib/gzlib.l" */
#include "gzlib.h"



    fwav fwav_create(int xs,int ys,int zs,float *thresh,
        gzlib_alloctype my_alloc,gzlib_freetype my_free,void *opaque);
    void fwav_destroy(fwav fw);
    void fwav_atran(fwav fw,float *v);
    void fwav_stran(fwav fw,float *v);


#endif


#endif

