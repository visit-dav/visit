#include<visit-config.h>
#include<convert.h>

#if(SIZEOF_INT == 4)
#ifdef WORDS_BIGENDIAN
    static int int_src_format = B32;
#else
    static int int_src_format = L32;
#endif
#elif(SIZEOF_INT == 8)
#ifdef WORDS_BIGENDIAN
    static int int_src_format = B64;
#else
    static int int_src_format = L64;
#endif
#else
#pragma error "Unsupported int size"
#endif

#define FUNCINDEX(s,d) ((((s) & 3) << 2) | ((d) & 3))

typedef int intconverter(int, unsigned char *);

/*********************************************************************
 *                      CONVERSION FUNCTIONS
 ********************************************************************/

/* 32 bit int B->L, L->B. */
static int
int32_Reverse_Endian(int val, unsigned char *outbuf)
{
    unsigned char *data = ((unsigned char *)&val) + 3;
    unsigned char *out = outbuf;

    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data--;
    *out = *data;

    return 4;
}

/* 32 bit int - no convert */
static int
int_32_To_32(int val, unsigned char *outbuf)
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
int_B32_To_B64(int val, unsigned char *outbuf)
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
int_B32_To_L64(int val, unsigned char *outbuf)
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
int_L32_To_B64(int val, unsigned char *outbuf)
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
int_L32_To_L64(int val, unsigned char *outbuf)
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

/* 64 bit int B->L, L->B. */
static int
int64_Reverse_Endian(int val, unsigned char *outbuf)
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

/* 64 bit int - no convert */
static int
int_64_To_64(int val, unsigned char *outbuf)
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
int_B64_To_B32(int val, unsigned char *outbuf)
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
int_B64_To_L32(int val, unsigned char *outbuf)
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
int_L64_To_B32(int val, unsigned char *outbuf)
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
int_L64_To_L32(int val, unsigned char *outbuf)
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
static intconverter *iconvlist[] = {
int_32_To_32,
int_B32_To_B64,
int32_Reverse_Endian,
int_B32_To_L64,
int_B64_To_B32,
int_64_To_64,
int_B64_To_L32,
int64_Reverse_Endian,
int32_Reverse_Endian,
int_L32_To_B64,
int_32_To_32,
int_L32_To_L64,
int_L64_To_B32,
int64_Reverse_Endian,
int_L64_To_L32,
int_64_To_64
};

/*********************************************************************
 *
 * Purpose: Converts an integer on the host platform to an integer
 *          on the destination platform and writes the resulting
 *          bytes into buf. The size of the converted array is returned.
 *
 * Programmer: Brad Whitlock
 * Date:       Tue May 9 10:51:14 PDT 2000
 *
 * Input Arguments:
 *     val : The integer value to be converted.
 *     dest_format : The destination platform's format. B32, B64, L32, L64.
 * Input / Output Arguments:
 *     buf : The buffer to hold the resulting bytes.
 *
 * Modifications:
 *
 ********************************************************************/
int
IntConvert(int val, unsigned char *buf, int dest_format)
{
    return (*(iconvlist[FUNCINDEX(int_src_format, dest_format)]))(val, buf);
}

int IntSourceFormat()
{
    return int_src_format;
}
