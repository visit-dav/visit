

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


