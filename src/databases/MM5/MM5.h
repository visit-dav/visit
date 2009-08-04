#ifndef MM5_HEADER_H
#define MM5_HEADER_H
/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include <stdio.h>
/***************************************************************************
 * This file contains definitions for an API to read MM5 files.
 *
 * Programmer: Brad Whitlock
 * Date: Tue Jul 11 16:41:48 PST 2006
 *
 * Modifications:
 *
 **************************************************************************/

typedef struct
{
    int   BHI[50][20];      /* Integer array */
    float BHR[20][20];      /* Real array */
    char  BHIC[50][20][80]; /* Description of BHI */
    char  BHRC[20][20][80]; /* Description of BHR */
} mm5_big_header_t;

typedef struct
{
    int   ndim;
    int   start_index[4];
    int   end_index[4];
    float xtime;
    char  staggering[4];
    char  ordering[4];
    char  current_date[24];
    char  name[9];
    char  unit[25];
    char  description[46];
} mm5_sub_header_t;

typedef struct
{
    mm5_sub_header_t header;
    unsigned long    file_offset;
} mm5_fieldinfo_t;

typedef struct
{
    mm5_fieldinfo_t  *fields;
    int               nfields;
    int               nfields_alloc;
} mm5_fieldlist_t;

typedef struct
{
    FILE             *file;
    char             *filename;
    mm5_big_header_t  header;
    mm5_fieldlist_t  *fields_over_time;
    int               ntimes;
    int               swap_endian;
} mm5_file_t;


void            *mm5_malloc(size_t sz);
void             mm5_free(void *ptr);

mm5_file_t      *mm5_file_open(const char *filename);
void             mm5_file_close(mm5_file_t *f);
mm5_fieldinfo_t *mm5_file_find_field(mm5_file_t *f, const char *varname, int ts);
int              mm5_file_read_field(mm5_file_t *, const char *, int, float *);
void             mm5_file_print_structure(mm5_file_t *f, FILE *out);

#endif
