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

#include <MM5.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

/****************************************************************************
 * Function: mm5p_null_terminate_string
 *
 * Purpose: 
 *   Null terminate a fixed width string.
 *
 * Arguments:
 *   s : The string.
 *   slen : The string's maximum length.
 *
 * Programmer: Brad Whitlock
 * Creation:   Wed Jul 12 11:00:04 PDT 2006
 *
 * Modifications:
 *   
 ***************************************************************************/

void
mm5p_null_terminate_string(char *s, int slen)
{
    char *ptr = s + slen - 1;
    while(ptr >= s && *ptr == ' ')
        *ptr-- = '\0';
}

/****************************************************************************
 * Function: mm5p_swap_endian_32
 *
 * Purpose: 
 *   Swaps the endian representation for a buffer if 32 bit values.
 *
 * Arguments:
 *   ptr : Pointer to the buffer.
 *   nelem : The number of 32 bit items.
 *
 * Programmer: Brad Whitlock
 * Creation:   Wed Jul 12 11:00:04 PDT 2006
 *
 * Modifications:
 *   
 ***************************************************************************/
void mm5p_swap_endian_32(unsigned char *ptr, int nelem)
{
    unsigned char tmp;
    int i;
#define _SWAP(a,b) tmp=a; a=b; b=tmp;
    for(i = 0; i < nelem; ++i)
    {
        _SWAP(ptr[0], ptr[3]);
        _SWAP(ptr[1], ptr[2]);
        ptr += 4;
    }
#undef _SWAP
}

/****************************************************************************
 * Function: mm5p_read_int
 *
 * Purpose: 
 *   Reads an integer from the a file.
 *
 * Arguments:
 *   f : The file.
 *
 * Returns:    An integer from the file.
 *
 * Programmer: Brad Whitlock
 * Creation:   Wed Jul 12 11:00:04 PDT 2006
 *
 * Modifications:
 *   
 ***************************************************************************/
int mm5p_read_int(FILE *f)
{
    int ret = 0;
    fread((void *)&ret, sizeof(int), 1, f);
    return ret;
}

/****************************************************************************
 * Function: mm5p_skip_record_header
 *
 * Purpose: 
 *   Skips the record header but returns its value.
 *
 * Arguments:
 *   f : The file.
 *
 * Returns:    The value stored in the record header
 *
 * Programmer: Brad Whitlock
 * Creation:   Wed Jul 12 11:00:04 PDT 2006
 *
 * Modifications:
 *   
 ***************************************************************************/
int mm5p_skip_record_header(FILE *f)
{
    int ret = mm5p_read_int(f);
    /*printf("mm5p_skip_record_header: ret = %x\n", ret);*/
    return ret;
}

/****************************************************************************
 * Function: mm5p_skip_record_trailer
 *
 * Purpose: 
 *   Skips the record trailer but returns its value.
 *
 * Arguments:
 *   f : The file.
 *
 * Programmer: Brad Whitlock
 * Creation:   Wed Jul 12 11:00:04 PDT 2006
 *
 * Modifications:
 *   
 ***************************************************************************/
int mm5p_skip_record_trailer(FILE *f)
{
    int ret = mm5p_read_int(f);
    /*printf("mm5p_skip_record_trailer: ret = %x\n", ret);*/
    return ret;
}

/****************************************************************************
 * Function: mm5p_print_fixed_width_string
 *
 * Purpose: 
 *   Prints a fixed width string to a stream.
 *
 * Arguments:
 *   f : The file.
 *   s : The string
 *   slen : The string's maximum length.
 *
 * Programmer: Brad Whitlock
 * Creation:   Wed Jul 12 11:00:04 PDT 2006
 *
 * Modifications:
 *   
 ***************************************************************************/
void mm5p_print_fixed_width_string(FILE *f, const char *s, int slen)
{
    int i;
    if(s[slen-1] != '\0')
    {
        fprintf(f, "\"");
        for(i = 0; i < slen; ++i)
            fprintf(f, "%c", s[i]);
        fprintf(f, "\"");
    }
    else
        fprintf(f, "\"%s\"", s);
}

/****************************************************************************
 * Function: mm5p_big_header_read
 *
 * Purpose: 
 *   Reads the MM5 big header.
 *
 * Arguments:
 *  bh          : The destination header object.
 *  f           : The file.
 *  swap_endian : Whether we should swap endian.
 *
 * Programmer: Brad Whitlock
 * Creation:   Wed Jul 12 11:00:04 PDT 2006
 *
 * Modifications:
 *   
 ***************************************************************************/
int mm5p_big_header_read(mm5_big_header_t *bh, FILE *f, int swap_endian)
{
    int i, j;

    mm5p_skip_record_header(f);

    fread(bh, sizeof(mm5_big_header_t), 1, f);

    /* Swap endians if necessary. */
    if(swap_endian)
    {
        mm5p_swap_endian_32((unsigned char *)bh->BHI, 50*20);
        mm5p_swap_endian_32((unsigned char *)bh->BHR, 20*20);
    }
    mm5p_skip_record_trailer(f);

    for(i = 0; i < 50; ++i)
        for(j = 0; j < 20; ++j)
            mm5p_null_terminate_string(bh->BHIC[i][j], 80);

    for(i = 0; i < 20; ++i)
        for(j = 0; j < 20; ++j)
            mm5p_null_terminate_string(bh->BHRC[i][j], 80);

    return 1;
}

/****************************************************************************
 * Function: mm5p_big_header_print
 *
 * Purpose: 
 *   Prints the MM5 big header.
 *
 * Arguments:
 *   bh : The big header.
 *   f  : The file.
 *
 * Programmer: Brad Whitlock
 * Creation:   Wed Jul 12 11:00:04 PDT 2006
 *
 * Modifications:
 *    Jeremy Meredith, Thu Aug  7 13:42:07 EDT 2008
 *    index into bh->BHIC for strings is a double index
 *   
 ***************************************************************************/
void mm5p_big_header_print(mm5_big_header_t *bh, FILE *f)
{
    int i, j;
    fprintf(f, "mm5_big_header_t = {\n");
    fprintf(f, "    BHI = {\n");
    for(j = 0; j < 50; ++j)
    {
        fprintf(f, "        ");
        for(i = 0; i < 20; ++i)
        {
            fprintf(f, ", %d", bh->BHI[j][i]);
        }
        fprintf(f, "\n");
    }
    fprintf(f, "    }\n");

    fprintf(f, "    BHR = {\n");
    for(j = 0; j < 20; ++j)
    {
        fprintf(f, "        ");
        for(i = 0; i < 20; ++i)
        {
            fprintf(f, ", %g", bh->BHR[j][i]);
        }
        fprintf(f, "\n");
    }
    fprintf(f, "    }\n");

    fprintf(f, "    BHIC = {\n");
    for(j = 0; j < 50; ++j)
        for(i = 0; i < 20; ++i)
        {
            fprintf(f, "        [%d][%d] = \"%s\"\n", j, i, bh->BHIC[j][i]);
        }
    fprintf(f, "    }\n");

    fprintf(f, "    BHRC = {\n");
    for(j = 0; j < 20; ++j)
        for(i = 0; i < 20; ++i)
        {
            fprintf(f, "        [%d][%d] = \"%s\"\n", j, i, bh->BHRC[j][i]);
        }
    fprintf(f, "    }\n");

    fprintf(f, "}\n");
}

/****************************************************************************
 * Function: mm5p_sub_header_read
 *
 * Purpose: 
 *   Reads an MM5 sub-header.
 *
 * Arguments:
 *   sh : The sub-header.
 *   f  : The file
 *   swap_endian : Whether to swap endian.
 *
 * Programmer: Brad Whitlock
 * Creation:   Wed Jul 12 11:00:04 PDT 2006
 *
 * Modifications:
 *   
 ***************************************************************************/
int mm5p_sub_header_read(mm5_sub_header_t *sh, FILE *f, int swap_endian)
{
    mm5p_skip_record_header(f);

    fread(sh, sizeof(mm5_sub_header_t), 1, f);

    mm5p_skip_record_trailer(f);

    /* Swap endians if necessary. */
    if(swap_endian)
    {
        mm5p_swap_endian_32((unsigned char *)&sh->ndim, 1);
        mm5p_swap_endian_32((unsigned char *)sh->start_index, 4);
        mm5p_swap_endian_32((unsigned char *)sh->end_index, 4);
        mm5p_swap_endian_32((unsigned char *)&sh->xtime, 1);
    }

    mm5p_null_terminate_string(sh->staggering, 4);
    mm5p_null_terminate_string(sh->ordering, 4);
    mm5p_null_terminate_string(sh->current_date, 24);
    mm5p_null_terminate_string(sh->name, 9);
    mm5p_null_terminate_string(sh->unit, 25);
    mm5p_null_terminate_string(sh->description, 46);

    return 1;
}

/****************************************************************************
 * Function: mm5p_sub_header_print
 *
 * Purpose: 
 *   Prints the MM5 sub header.
 *
 * Arguments:
 *   sh : The sub header
 *   f  : The file.
 *
 * Programmer: Brad Whitlock
 * Creation:   Wed Jul 12 11:00:04 PDT 2006
 *
 * Modifications:
 *   
 ***************************************************************************/
void mm5p_sub_header_print(mm5_sub_header_t *sh, FILE *f)
{
    fprintf(f, "mm5_sub_header_t = {\n");
    fprintf(f, "    ndim = %d\n", sh->ndim);
    fprintf(f, "    start_index = {%d,%d,%d,%d}\n",
        sh->start_index[0],sh->start_index[1],sh->start_index[2],sh->start_index[3]);
    fprintf(f, "    end_index = {%d,%d,%d,%d}\n",
        sh->end_index[0],sh->end_index[1],sh->end_index[2],sh->end_index[3]);
    fprintf(f, "    xtime = %g\n", sh->xtime);
    fprintf(f, "    staggering = ");
    mm5p_print_fixed_width_string(f, sh->staggering, 4);

    fprintf(f, "\n    ordering = ");
    mm5p_print_fixed_width_string(f, sh->ordering, 4);

    fprintf(f, "\n    current_date = ");
    mm5p_print_fixed_width_string(f, sh->current_date, 24);

    fprintf(f, "\n    name = ");
    mm5p_print_fixed_width_string(f, sh->name, 9);

    fprintf(f, "\n    unit = ");
    mm5p_print_fixed_width_string(f, sh->unit, 25);

    fprintf(f, "\n    description = ");
    mm5p_print_fixed_width_string(f, sh->description, 46);

    fprintf(f, "\n}\n");
}

/****************************************************************************
 * Function: mm5p_fieldlist_add_fieldinfo
 *
 * Purpose: 
 *   Adds a field to a field list and returns a pointer to the field.
 *
 * Arguments:
 *   fl : The field list.
 *
 * Returns:    A pointer to the field.
 *
 * Programmer: Brad Whitlock
 * Creation:   Wed Jul 12 11:00:04 PDT 2006
 *
 * Modifications:
 *   
 ***************************************************************************/
mm5_fieldinfo_t *
mm5p_fieldlist_add_fieldinfo(mm5_fieldlist_t *fl)
{
    mm5_fieldinfo_t *retval = NULL;

    if(fl->nfields + 1 >= fl->nfields_alloc)
    {
        mm5_fieldinfo_t *ptr = NULL;
        ptr = (mm5_fieldinfo_t *)mm5_malloc(
            sizeof(mm5_fieldinfo_t) * (fl->nfields_alloc + 20));
        memcpy(ptr, fl->fields, fl->nfields_alloc * sizeof(mm5_fieldinfo_t));
        mm5_free(fl->fields);
        fl->nfields_alloc += 20;
        fl->fields = ptr;
    }

    retval = &fl->fields[fl->nfields];
    ++fl->nfields;

    return retval;
}

/******************************* PUBLIC ************************************/

/****************************************************************************
 * Function: mm5_malloc
 *
 * Purpose: 
 *   Allocates memory for the MM5 API.
 *
 * Arguments:
 *   sz : The number of bytes to allocate.
 *
 * Returns:    A pointer to the allocated storage.
 *
 * Note:       
 *
 * Programmer: Brad Whitlock
 * Creation:   Wed Jul 12 11:00:04 PDT 2006
 *
 * Modifications:
 *   
 ***************************************************************************/
void *
mm5_malloc(size_t sz)
{
    void *ptr = malloc(sz);
    memset(ptr, 0, sz);
    return ptr;
}

/****************************************************************************
 * Function: mm5_free
 *
 * Purpose: 
 *   Frees bytes allocated by mm5_malloc.
 *
 * Arguments:
 *   ptr : The pointer to free.
 *
 * Programmer: Brad Whitlock
 * Creation:   Wed Jul 12 11:00:04 PDT 2006
 *
 * Modifications:
 *   
 ***************************************************************************/
void
mm5_free(void *ptr)
{
    if(ptr != NULL)
        free(ptr);
}

/****************************************************************************
 * Function: mm5_file_close
 *
 * Purpose: 
 *   Closes an MM5 file.
 *
 * Arguments:
 *   f : The file to close.
 *
 * Programmer: Brad Whitlock
 * Creation:   Wed Jul 12 11:00:04 PDT 2006
 *
 * Modifications:
 *   
 ***************************************************************************/
void
mm5_file_close(mm5_file_t *f)
{
    if(f != NULL)
    {
        int i;
        if(f->file != NULL)
            fclose(f->file);

        mm5_free(f->filename);
        for(i = 0; i < f->ntimes; ++i)
            mm5_free(f->fields_over_time[i].fields);
        mm5_free(f);
    }
}

/****************************************************************************
 * Function: mm5_file_open
 *
 * Purpose: 
 *   Opens an MM5 file.
 *
 * Arguments:
 *   filename : The name of the file to open.
 *
 * Returns:    A pointer to an mm5_file_t object or NULL if the file could
 *             not be opened.
 *
 * Programmer: Brad Whitlock
 * Creation:   Wed Jul 12 11:00:04 PDT 2006
 *
 * Modifications:
 *   
 ***************************************************************************/
mm5_file_t *
mm5_file_open(const char *filename)
{
    int op, rh_size, n_elements, inserting_time_info = 0, first = 1;
    mm5_file_t *retval = NULL;
    FILE *f = NULL;

    /* Try and open the file. */
    f = fopen(filename, "rb");
    if(f == NULL)
        return NULL;

    /* We were able to open the file so allocate an mm5_file_t to return. */
    retval = (mm5_file_t *)mm5_malloc(sizeof(mm5_file_t));
    retval->file = f;
    retval->filename = (char *)mm5_malloc(strlen(filename) + 1);
    strcpy(retval->filename, filename);

    /* Read the headers out of the file.*/
    do
    {
        /* Read the record header size. */
        rh_size = mm5p_skip_record_header(f);
        if(feof(f))
            break;

        if(first == 1)
        {
            if(rh_size != 4)
                retval->swap_endian = 1;
            first = 0;             
        }

        /* Read the record type. */
        op = -1;
        op = mm5p_read_int(f);
        if(retval->swap_endian)
        {
            mm5p_swap_endian_32((unsigned char *)&rh_size, 1);
            mm5p_swap_endian_32((unsigned char *)&op, 1);
        }
        mm5p_skip_record_trailer(f);

        if(op == 0)
        {
            /* Read the main header. */
            mm5p_big_header_read(&retval->header, f, retval->swap_endian);
        }
        else if(op == 1)
        {
            mm5_fieldinfo_t *field = NULL;

            /* Make sure that there are enough time entries. */
            if(inserting_time_info == 0)
            {
                /* Add a new time entry. */
                mm5_fieldlist_t *newlist = 
                    (mm5_fieldlist_t*)mm5_malloc((retval->ntimes + 1) * 
                    sizeof(mm5_fieldlist_t));
                if(retval->ntimes > 0)
                {
                    memcpy(newlist, retval->fields_over_time,
                           retval->ntimes * sizeof(mm5_fieldlist_t));
                }
                mm5_free(retval->fields_over_time);
                retval->fields_over_time = newlist;

                inserting_time_info = 1;
                ++retval->ntimes;
            }

            /* Make sure that there are enough spaces in the fields array. */
            field = mm5p_fieldlist_add_fieldinfo(
                 &retval->fields_over_time[retval->ntimes-1]);

            /* Read the sub header. */
            mm5p_sub_header_read(&field->header, f, retval->swap_endian);

            /* Skip past the field but remember where it was. */
            n_elements = field->header.end_index[0] *
                         field->header.end_index[1] *
                         field->header.end_index[2] *
                         field->header.end_index[3];

            mm5p_skip_record_header(f);
            field->file_offset = ftell(f);
            fseek(f, n_elements * sizeof(float), SEEK_CUR);
            mm5p_skip_record_trailer(f);
        }
        else if(op == 2)
        {
            /* End of time period. */
            inserting_time_info = 0;
        }
        else
        {
            /* We have an unknown opcode. Assume that we read some unsupported
               record. Since we know the size, we can skip past it.
             */
            rh_size -= 8;
            fseek(f, rh_size, SEEK_CUR);
            /*fprintf(out, "Bad record type. Skipping %d bytes.\n", rh_size);*/
        }
    } while(!feof(f));

    return retval;
}

/****************************************************************************
 * Function: mm5_file_find_field
 *
 * Purpose: 
 *   Finds the named field for the current time step and returns a pointer.
 *
 * Arguments:
 *   f : The MM5 file.
 *   varname : The name of the variable to find.
 *   ts      : The index of the time state in which to find the variable.
 *
 * Returns:    A pointer to the field or NULL.
 *
 * Programmer: Brad Whitlock
 * Creation:   Wed Jul 12 11:00:04 PDT 2006
 *
 * Modifications:
 *   
 ***************************************************************************/
mm5_fieldinfo_t *
mm5_file_find_field(mm5_file_t *f, const char *varname, int ts)
{
    int i;
    if(ts < 0 || ts >= f->ntimes)
        return NULL;

    for(i = 0; i < f->fields_over_time[ts].nfields; ++i)
    {
        mm5_fieldinfo_t *field = &f->fields_over_time[ts].fields[i];
        if(strcmp(field->header.name, varname) == 0)
            return field;
    }

    return NULL;
}

/****************************************************************************
 * Function: mm5_file_read_field
 *
 * Purpose: 
 *   Reads a field directly into storage allocated by the caller.
 *
 * Arguments:
 *   f : The MM% file.
 *   varname : The variable to read.
 *   ts : The time state for which we're reading the data.
 *   dest : The destination array.
 *
 * Returns:    1 on success and 0 on failure.
 *
 * Programmer: Brad Whitlock
 * Creation:   Wed Jul 12 11:00:04 PDT 2006
 *
 * Modifications:
 *   
 ***************************************************************************/
int mm5_file_read_field(mm5_file_t *f, const char *varname, int ts, float *dest)
{
    const mm5_fieldinfo_t *field = NULL;

    field = mm5_file_find_field(f, varname, ts);
    if(field != 0)
    {
        size_t n_elements = field->header.end_index[0] *
            field->header.end_index[1] *
            field->header.end_index[2] *
            field->header.end_index[3];

        /* Seek to the start of the file and then to the variable. */
        fseek(f->file, 0, SEEK_SET);
        fseek(f->file, field->file_offset, SEEK_CUR);

        /* Read the variable into the array. */
        fread((void*)dest, sizeof(float), n_elements, f->file);
        /* Swap endian.*/
        if(f->swap_endian)
            mm5p_swap_endian_32((unsigned char *)dest, n_elements);

        return 1;
    }

    return 0;
}

/****************************************************************************
 * Function: mm5_file_print_structure
 *
 * Purpose: 
 *   Print the MM5 file structure.
 *
 * Arguments:
 *   f   : The file.
 *   out : The stream used for printing.
 *
 * Programmer: Brad Whitlock
 * Creation:   Wed Jul 12 11:00:04 PDT 2006
 *
 * Modifications:
 *   
 ***************************************************************************/
void
mm5_file_print_structure(mm5_file_t *f, FILE *out)
{
    int ts, i;
    mm5p_big_header_print(&f->header, out);
    for(ts = 0; ts < f->ntimes; ++ts)
    {
        for(i = 0; i < f->fields_over_time[ts].nfields; ++i)
            mm5p_sub_header_print(&f->fields_over_time[ts].fields[i].header, out);
        fprintf(out, "*****************************************\n");
    }
}
