

/* see gzlib.c for authorship and revision notes */


#ifndef LIB_gzlib__TYPE

#define LIB_gzlib__TYPE 1

typedef struct gzlib_structdef gzlib_struct,*gzlib;

#endif


#ifndef LIB_gzlib__TYPEONLY

#ifndef LIB_gzlib__HERE

#define LIB_gzlib__HERE 1


/* notes from original gzlib.l; see gzlib.c for current notes: */

    /* gzlib.l            Lib wrapper to zlib library -- general compression */
    /* by Mark Duchaineau */
    /* */
    /* 2002-12-13: wrote */


    /* perform make/make clean on attached libraries/utilities */
    
    
#include "stdlib.h"


/* note: requires compile flag -I<zlib dir with zlib.h in it> */

#include "zlib.h"



    typedef void *(*gzlib_alloctype)(void *opaque,size_t size);
    typedef void (*gzlib_freetype)(void *opaque,void *ptr);


    /* session record for zlib compress/decompress activity */
    

/*
 * lib_struct gzlib
 */

struct gzlib_structdef {
        gzlib_alloctype alloc; /* application-supplied memory allocator */
        gzlib_freetype free;   /* application-supplied memory de-allocator */
        void *opaque;          /* application-supplied pass-thru value */
        int error_count;       /* number of errors since startup or error_clear */
        char error_msg[256];   /* most recent error message */
    };



    gzlib gzlib_create(gzlib_alloctype my_alloc,gzlib_freetype my_free,
        void *opaque);
    void gzlib_destroy(gzlib gz);
    int gzlib_deflate(gzlib gz,char *inbuf,int insize,
        char **r_outbuf,int *r_outsize);
    int gzlib_inflate(gzlib gz,char *inbuf,int insize,
        char **r_outbuf,int *r_outsize);
    void *gzlib_alloc(void *opaque,size_t size);
    void gzlib_free(void *opaque,void *ptr);


    


#endif


#endif

