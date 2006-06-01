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



/*
 * gzlib.{h,c}          Lib wrapper to zlib library -- general compression
 * by Mark Duchaineau
 *
 * 2002-12-13: wrote
 * 2003-10-09: took snapshot of libgen code, converted to nonliib version
 */


#include "gzlib.h"
#include <snprintf.h>

static voidpf zlib_alloc_func(voidpf opaque, uInt items, uInt size);
static void zlib_free_func(voidpf opaque, voidpf address);


gzlib gzlib_create(gzlib_alloctype my_alloc,gzlib_freetype my_free,
    void *opaque)
{
    gzlib gz;

    if (!my_alloc || !my_free) { my_alloc=gzlib_alloc; my_free=gzlib_free; }
    gz=(gzlib)(*my_alloc)(opaque,sizeof(gzlib_struct));
    if (!gz) return gz;
    gz->alloc=my_alloc;
    gz->free=my_free;
    gz->opaque=opaque;
    gz->error_count=0;
    gz->error_msg[0]='\0';
    return gz;
}


void gzlib_destroy(gzlib gz)
{
    (*gz->free)(gz->opaque,(void *)gz);
}


int gzlib_deflate(gzlib gz,char *inbuf,int insize,
    char **r_outbuf,int *r_outsize)
{
    int outsize;
    z_stream zs;
    int err;
    char *outbuf;

    /* allocate large enough output buffer in zlib's preferred flavor */
    outsize=insize+(insize>>6)+32; /* insize+0.1%+12 or more */
    outbuf=(char *)(*gz->alloc)(gz->opaque,outsize);
    if (!outbuf) {
        gz->error_count++;
        SNPRINTF(gz->error_msg, 256, "out of memory");
        return -1;
    }

    /* compress inbuf to outbuf */
    zs.next_in=(Bytef *)inbuf;
    zs.avail_in=(uInt)insize;
    zs.next_out=(Bytef *)outbuf;
    zs.avail_out=(uInt)outsize;
    zs.zalloc=zlib_alloc_func;
    zs.zfree=zlib_free_func;
    zs.opaque=(voidpf)gz;
    err=deflateInit(&zs,Z_DEFAULT_COMPRESSION);
    if (err!=Z_OK) {
        (*gz->free)(gz->opaque,(void *)outbuf);
        gz->error_count++;
        SNPRINTF(gz->error_msg, 256, "deflateInit failed");
        return -1;
    }
    err=deflate(&zs,Z_FINISH);
    if (err!=Z_STREAM_END) {
        (*gz->free)(gz->opaque,(void *)outbuf);
        gz->error_count++;
        SNPRINTF(gz->error_msg, 256, "deflate failed");
        return -1;
    }
    err=deflateEnd(&zs);
    if (err!=Z_OK) {
        (*gz->free)(gz->opaque,(void *)outbuf);
        gz->error_count++;
        SNPRINTF(gz->error_msg, 256, "deflateEnd failed");
        return -1;
    }
    outsize=(int)zs.total_out;

    *r_outbuf=outbuf; *r_outsize=outsize;
    return 0;
}


int gzlib_inflate(gzlib gz,char *inbuf,int insize,
    char **r_outbuf,int *r_outsize)
{
    int outsize;
    char *outbuf;
    z_stream zs;
    int err;

    /* allocate large enough output buffer if none supplied */
    outsize= *r_outsize;
    outbuf= *r_outbuf;
    if (!outbuf) {
        outbuf=(char *)(*gz->alloc)(gz->opaque,outsize);
        if (!outbuf) {
            gz->error_count++;
            SNPRINTF(gz->error_msg, 256, "out of memory");
            return -1;
        }
    }

    /* decompress inbuf to outbuf */
    zs.next_in=(Bytef *)inbuf;
    zs.avail_in=(uInt)insize;
    zs.next_out=(Bytef *)outbuf;
    zs.avail_out=(uInt)outsize;
    zs.zalloc=zlib_alloc_func;
    zs.zfree=zlib_free_func;
    zs.opaque=(voidpf)gz;
    err=inflateInit(&zs);
    if (err!=Z_OK) {
        (*gz->free)(gz->opaque,(void *)outbuf);
        gz->error_count++;
        SNPRINTF(gz->error_msg, 256, "inflateInit failed");
        return -1;
    }
    err=inflate(&zs,Z_FINISH);
    if (err!=Z_STREAM_END) {
        (*gz->free)(gz->opaque,(void *)outbuf);
        gz->error_count++;
        SNPRINTF(gz->error_msg, 256, "inflate failed");
        return -1;
    }
    outsize=(int)zs.total_out;
    err=inflateEnd(&zs);
    if (err!=Z_OK) {
        (*gz->free)(gz->opaque,(void *)outbuf);
        gz->error_count++;
        SNPRINTF(gz->error_msg, 256, "inflateEnd failed");
        return -1;
    }

    *r_outbuf=outbuf; *r_outsize=outsize;
    return 0;
}


void *gzlib_alloc(void *opaque,size_t size)
{
    return malloc(size);
}


void gzlib_free(void *opaque,void *ptr)
{
    free(ptr);
}



/* ========== internal routines ========== */


static voidpf zlib_alloc_func(voidpf opaque, uInt items, uInt size)
{
    gzlib gz;

    gz=(gzlib)opaque;
    return (voidpf)(*gz->alloc)(gz->opaque,items*size);
}


static void zlib_free_func(voidpf opaque, voidpf address)
{
    gzlib gz;

    gz=(gzlib)opaque;
    (*gz->free)(gz->opaque,(void *)address);
}


