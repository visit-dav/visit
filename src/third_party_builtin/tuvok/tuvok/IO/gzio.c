/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2008 Scientific Computing and Imaging Institute,
   University of Utah.

   
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/
#include <assert.h>
#include <stdlib.h>
#include "gzio.h"

#ifdef WIN32
  #pragma warning( disable : 4706) // disable "assignment in conditional" warning 
#endif

#define CHUNK (1024 * 1024 * 64)

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
int
gz_inflate(FILE *source, FILE *dest)
{
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char *in = malloc(CHUNK);
    unsigned char *out = malloc(CHUNK);

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit2(&strm, -MAX_WBITS);
    if (ret != Z_OK) {
        free(in);
        free(out);
        return ret;
    }

    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = (uInt)fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)inflateEnd(&strm);
            free(in);
            free(out);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                free(in);
                free(out);
                return ret;
            }
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)inflateEnd(&strm);
                free(in);
                free(out);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    free(in);
    free(out);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* gzip flag byte */
enum GZFlags {
  GZ_TEXT      = 0x01,
  GZ_HDR_CRC   = 0x02,
  GZ_EXTRA     = 0x04,
  GZ_NAME      = 0x08,
  GZ_COMMENT   = 0x10,
  GZ_RESERVED1 = 0x10,
  GZ_RESERVED2 = 0x20,
  GZ_RESERVED3 = 0x40
};

void
gz_skip_header(FILE *fs)
{
    int byte;
    int flags;
    size_t i;

    byte = fgetc(fs);
    assert(byte == 0x1f);
    byte = fgetc(fs);
    assert(byte == 0x8b);

    byte = fgetc(fs); /* `method' byte */
    assert(byte = Z_DEFLATED);
    flags = fgetc(fs); /* gzip header flags, details `members' */

    /**  skip 6 bytes of worthless metadata? */
    for(i=0; i < 6; ++i) { fgetc(fs); }

    /** if there's an `extra' member, it's a length followed by a payload.
     * skip it. */
    if((flags & GZ_EXTRA) != 0) {
        size_t len = fgetc(fs);
        len += (fgetc(fs) << 8);

        // skip len bytes, or the rest of the file, whichever is first.
        while(len-- != 0 && fgetc(fs) != EOF);
    }

    /* if there's a `name' member, it's a null terminated string. */
    if((flags & GZ_NAME) != 0) {
        while((byte = fgetc(fs)) && byte != EOF);
    }
    /* likewise with the `comment' member. */
    if((flags & GZ_COMMENT) != 0) {
        while((byte = fgetc(fs)) && byte != EOF);
    }
    /* the CRC is somehow optional... we should probably store this so we can
     * check it later, but we ignore it for now ... */
    if((flags & GZ_HDR_CRC)) {
        fgetc(fs);
        fgetc(fs);
    }
}
