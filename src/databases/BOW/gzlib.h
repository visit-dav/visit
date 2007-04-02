/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/



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

