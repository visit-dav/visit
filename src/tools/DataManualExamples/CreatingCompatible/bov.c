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

#include <stdio.h>
#include <visit-config.h>

#define NX 20
#define NY 30
#define NZ 40

int main(int argc, char *argv[])
{
    int i,j,k,index = 0;
    float values[NZ][NY][NX];
    FILE *f = NULL;

    /* Fill up the array */
    for(k = 0; k < NZ; ++k)
        for(j = 0; j < NY; ++j)
            for(i = 0; i < NX; ++i)
                values[k][j][i] = index++;

    /* Write the BOV data file */
    f = fopen("bov.values", "wb");
    if(f == NULL)
    {
        fprintf(stderr, "Could not write data file\n");
        return -1;
    }
    fwrite((void *)values, sizeof(float), NX*NY*NZ, f);
    fclose(f);

    /* Write the BOV header file. */
    f = fopen("bov.bov", "wb");
    if(f == NULL)
    {
        fprintf(stderr, "Could not write header file\n");
        return -1;
    }
    fprintf(f, "TIME: 0.0\n");
    fprintf(f, "DATA_FILE: bov.values\n");
    fprintf(f, "DATA_SIZE: %d %d %d\n", NX, NY, NZ);
    fprintf(f, "DATA_FORMAT: FLOAT\n");
    fprintf(f, "VARIABLE: var\n");
#ifdef WORDS_BIGENDIAN
    fprintf(f, "DATA_ENDIAN: BIG\n");
#else
    fprintf(f, "DATA_ENDIAN: LITTLE\n");
#endif
    fprintf(f, "CENTERING: zonal\n");
    fprintf(f, "BRICK_ORIGIN: 0.0 0.0 0.0\n");
    fprintf(f, "BRICK_SIZE: 10.0 10.0 10.0\n");
    fclose(f);

    return 0;
}
