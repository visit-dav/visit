#include<visit-config.h>
#include<convert.h>

#if(SIZEOF_DOUBLE == 8)
#ifdef WORDS_BIGENDIAN
    static int double_src_format = B64;
#else
    static int double_src_format = L64;
#endif
#else
#pragma error "Unsupported double size"
#endif

#define FUNCINDEX(s,d) ((((s) & 3) << 2) | ((d) & 3))

typedef int doubleconverter(double, unsigned char *);

/*********************************************************************
 *                      CONVERSION FUNCTIONS
 ********************************************************************/

/* 64 bit double B->L, L->B. */
static int
double64_Reverse_Endian(double val, unsigned char *outbuf)
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

/* 64 bit double - no convert */
static int
double_64_To_64(double val, unsigned char *outbuf)
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

static int
double_unsupported(double, unsigned char *)
{
    return 0;
}

/* Lookup table for which converter function to use. */
static doubleconverter *dconvlist[] = {
double_unsupported,
double_unsupported,
double_unsupported,
double_unsupported,
double_unsupported,
double_64_To_64,
double_unsupported,
double64_Reverse_Endian,
double_unsupported,
double_unsupported,
double_unsupported,
double_unsupported,
double_unsupported,
double64_Reverse_Endian,
double_unsupported,
double_64_To_64
};

/*********************************************************************
 *
 * Purpose: Converts a double on the host platform to a double on the
 *          destination platform and writes the resulting bytes into
 *          buf. The size of the converted array is returned.
 *
 * Programmer: Brad Whitlock
 * Date:       Tue May 9 10:51:14 PDT 2000
 *
 * Input Arguments:
 *     val : The double value to be converted.
 *     dest_format : The destination platform's format. B64, L64.
 * Input / Output Arguments:
 *     buf : The buffer to hold the resulting bytes.
 *
 * Modifications:
 *
 ********************************************************************/
int
DoubleConvert(double val, unsigned char *buf, int dest_format)
{
    return (*(dconvlist[FUNCINDEX(double_src_format, dest_format)]))(val, buf);
}

int DoubleSourceFormat()
{
    return double_src_format;
}
