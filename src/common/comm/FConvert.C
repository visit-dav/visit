/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
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
