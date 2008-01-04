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
