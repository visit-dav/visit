#include<visit-config.h>
#include<convert.h>

#if(SIZEOF_LONG == 4)
#ifdef WORDS_BIGENDIAN
    static int long_src_format = B32;
#else
    static int long_src_format = L32;
#endif
#elif(SIZEOF_LONG == 8)
#ifdef WORDS_BIGENDIAN
    static int long_src_format = B64;
#else
    static int long_src_format = L64;
#endif
#else
#pragma error "Unsupported long size"
#endif

#define FUNCINDEX(s,d) ((((s) & 3) << 2) | ((d) & 3))

typedef int longconverter(long, unsigned char *);

/*********************************************************************
 *                      CONVERSION FUNCTIONS
 ********************************************************************/

/* 32 bit long B->L, L->B. */
static int
long32_Reverse_Endian(long val, unsigned char *outbuf)
{
    unsigned char *data = ((unsigned char *)&val) + 3;
    unsigned char *out = outbuf;

    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data--;
    *out = *data;

    return 4;
}

/* 32 bit long - no convert */
static int
long_32_To_32(long val, unsigned char *outbuf)
{
    unsigned char *data = ((unsigned char *)&val);
    unsigned char *out = outbuf;

    *out++ = *data++;
    *out++ = *data++;
    *out++ = *data++;
    *out = *data;

    return 4;
}

/* Big Endian 32 bit to Big Endian 64 bit */
static int
long_B32_To_B64(long val, unsigned char *outbuf)
{
    unsigned char *data = ((unsigned char *)&val);
    unsigned char *out = outbuf;

    *out++ = 0;
    *out++ = 0;
    *out++ = 0;
    *out++ = 0;
    *out++ = *data++;
    *out++ = *data++;
    *out++ = *data++;
    *out = *data;

    return 8;
}

/* Big Endian 32 bit to Little Endian 64 bit */
static int
long_B32_To_L64(long val, unsigned char *outbuf)
{
    unsigned char *data = ((unsigned char *)&val) + 3;
    unsigned char *out = outbuf;

    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data;
    *out++ = 0;
    *out++ = 0;
    *out++ = 0;
    *out = 0;

    return 8;
}

/* Little Endian 32 bit to Big Endian 64 bit */
static int
long_L32_To_B64(long val, unsigned char *outbuf)
{
    unsigned char *data = ((unsigned char *)&val) + 3;
    unsigned char *out = outbuf;

    *out++ = 0;
    *out++ = 0;
    *out++ = 0;
    *out++ = 0;
    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data--;
    *out = *data;

    return 8;
}

/* Little Endian 32 bit to Little Endian 64 bit */
static int
long_L32_To_L64(long val, unsigned char *outbuf)
{
    unsigned char *data = ((unsigned char *)&val);
    unsigned char *out = outbuf;

    *out++ = *data++;
    *out++ = *data++;
    *out++ = *data++;
    *out++ = *data;
    *out++ = 0;
    *out++ = 0;
    *out++ = 0;
    *out = 0;

    return 8;
}

/* 64 bit long B->L, L->B. */
static int
long64_Reverse_Endian(long val, unsigned char *outbuf)
{
    unsigned char *data = ((unsigned char *)&val) + 7;
    unsigned char *out = outbuf;

    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data--;
    *out = *data;

    return 8;
}

/* 64 bit long - no convert */
static int
long_64_To_64(long val, unsigned char *outbuf)
{
    unsigned char *data = ((unsigned char *)&val);
    unsigned char *out = outbuf;

    *out++ = *data++;
    *out++ = *data++;
    *out++ = *data++;
    *out++ = *data++;
    *out++ = *data++;
    *out++ = *data++;
    *out++ = *data++;
    *out = *data;

    return 8;
}

/* Big Endian 64 bit to Big Endian 32 bit */
static int
long_B64_To_B32(long val, unsigned char *outbuf)
{
    unsigned char *data = ((unsigned char *)&val) + 4;
    unsigned char *out = outbuf;

    *out++ = *data++;
    *out++ = *data++;
    *out++ = *data++;
    *out = *data;

    return 4;
}

/* Big Endian 64 bit to Little Endian 32 bit */
static int
long_B64_To_L32(long val, unsigned char *outbuf)
{
    unsigned char *data = ((unsigned char *)&val) + 7;
    unsigned char *out = outbuf;

    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data--;
    *out = *data;

    return 4;
}

/* Little Endian 64 bit to Big Endian 32 bit */
static int
long_L64_To_B32(long val, unsigned char *outbuf)
{
    unsigned char *data = ((unsigned char *)&val) + 3;
    unsigned char *out = outbuf;

    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data--;
    *out = *data;

    return 4;
}

/* Little Endian 64 bit to Little Endian 32 bit */
static int
long_L64_To_L32(long val, unsigned char *outbuf)
{
    unsigned char *data = ((unsigned char *)&val);
    unsigned char *out = outbuf;

    *out++ = *data++;
    *out++ = *data++;
    *out++ = *data++;
    *out = *data;

    return 4;
}

/* Lookup table for which converter function to use. */
static longconverter *lconvlist[] = {
long_32_To_32,
long_B32_To_B64,
long32_Reverse_Endian,
long_B32_To_L64,
long_B64_To_B32,
long_64_To_64,
long_B64_To_L32,
long64_Reverse_Endian,
long32_Reverse_Endian,
long_L32_To_B64,
long_32_To_32,
long_L32_To_L64,
long_L64_To_B32,
long64_Reverse_Endian,
long_L64_To_L32,
long_64_To_64
};

/*********************************************************************
 *
 * Purpose: Converts a long on the host platform to a long on the
 *          destination platform and writes the resulting bytes into
 *          buf. The size of the converted array is returned.
 *
 * Programmer: Brad Whitlock
 * Date:       Tue May 9 10:51:14 PDT 2000
 *
 * Input Arguments:
 *     val : The long value to be converted.
 *     dest_format : The destination platform's format. B32, B64, L32, L64.
 * Input / Output Arguments:
 *     buf : The buffer to hold the resulting bytes.
 *
 * Modifications:
 *
 ********************************************************************/
int
LongConvert(long val, unsigned char *buf, int dest_format)
{
    return (*(lconvlist[FUNCINDEX(long_src_format, dest_format)]))(val, buf);
}

int LongSourceFormat()
{
    return long_src_format;
}
