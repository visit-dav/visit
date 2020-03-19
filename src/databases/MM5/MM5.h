// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef MM5_HEADER_H
#define MM5_HEADER_H

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
