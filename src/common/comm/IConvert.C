/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
