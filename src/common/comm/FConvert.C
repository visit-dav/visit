#include<visit-config.h>
#include<convert.h>

#if(SIZEOF_FLOAT == 4)
#ifdef WORDS_BIGENDIAN
    static int float_src_format = B32;
#else
    static int float_src_format = L32;
#endif
#else
#pragma error "Unsupported float size"
#endif

#define FUNCINDEX(s,d) ((((s) & 3) << 2) | ((d) & 3))

typedef int floatconverter(float, unsigned char *);

/*********************************************************************
 *                      CONVERSION FUNCTIONS
 ********************************************************************/

/* 32 bit float B->L, L->B. */
static int
float32_Reverse_Endian(float val, unsigned char *outbuf)
{
    unsigned char *data = ((unsigned char *)&val) + 3;
    unsigned char *out = outbuf;

    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data--;
    *out = *data;

    return 4;
}

/* 32 bit float - no convert */
static int
float_32_To_32(float val, unsigned char *outbuf)
{
    unsigned char *data = ((unsigned char *)&val);
    unsigned char *out = outbuf;

    *out++ = *data++;
    *out++ = *data++;
    *out++ = *data++;
    *out = *data;

    return 4;
}

static int
float_unsupported(float, unsigned char *)
{
    return 0;
}

/* Lookup table for which converter function to use. */
static floatconverter *fconvlist[] = {
float_32_To_32,
float_unsupported,
float32_Reverse_Endian,
float_unsupported,
float_unsupported,
float_unsupported,
float_unsupported,
float_unsupported,
float32_Reverse_Endian,
float_unsupported,
float_32_To_32,
float_unsupported,
float_unsupported,
float_unsupported,
float_unsupported,
float_unsupported
};

/*********************************************************************
 *
 * Purpose: Converts a float on the host platform to a float on the
 *          destination platform and writes the resulting bytes into
 *          buf. The size of the converted array is returned.
 *
 * Programmer: Brad Whitlock
 * Date:       Tue May 9 10:51:14 PDT 2000
 *
 * Input Arguments:
 *     val : The float value to be converted.
 *     dest_format : The destination platform's format. B32, L32.
 * Input / Output Arguments:
 *     buf : The buffer to hold the resulting bytes.
 *
 * Modifications:
 *
 ********************************************************************/
int
FloatConvert(float val, unsigned char *buf, int dest_format)
{
    return (*(fconvlist[FUNCINDEX(float_src_format, dest_format)]))(val, buf);
}

int FloatSourceFormat()
{
    return float_src_format;
}
