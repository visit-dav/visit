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

/*
 * PDB.H - header file supporting PDBLIB routines
 *       -
 * Source Version: 9.0
 * Software Release #92-0043
 *
 */

#ifndef _LITE_PDB_H
#define _LITE_PDB_H

// re-define the PB interface so the PDB client code will call PDB lite
#define PD_ls             lite_PD_ls
#define PD_open           lite_PD_open
#define PD_close          lite_PD_close
#define PD_read           lite_PD_read
#define PD_write          lite_PD_write
#define PD_defstr         lite_PD_defstr
#define PD_inquire_entry  lite_PD_inquire_entry
#define PD_err            lite_PD_err
#define LAST              lite_LAST

#ifdef __cplusplus
extern "C" {
#endif

#include <setjmp.h> /* For the setjmp/long structure jmp_buf. */
#include <stdio.h>
#include <lite_score.h> /* For type definitions */ 

/* The MAXLINE macro is defined in score.h and taken from stdio.h's */
/* FILENAME_MAX.  HP's is only 14, so correct that here. */

#ifndef MAXLINE
  #define MAXLINE 1024
#else
  #if MAXLINE < 1024
     #undef MAXLINE
     #define MAXLINE 1024
  #endif
#endif

#define PDB_SYSTEM_VERSION  14
#define PDB_WRITE /*Include code to write to pdb files*/

/* PDBFile Layout:
 *       
 *       Header (ASCII)
 *         Identifying token
 *         Data Formats
 *         Structure Chart Address
 *         Symbol Table Address
 *       Data
 *       Structure Chart
 *       Symbol Table
 *       Extensions
 *       EOF
 */

#define NORMAL_ORDER    1
#define REVERSE_ORDER   2  /* this must not be -1 which turns out to be EOF */

#define IEEE_32_64   1        /* IEEE standard 32 bit float : 64 bit double */
#define INTEL_X86    2                        /* Intel 80x86 class machines */
#define CRAY_64      3                               /* CRAY class machines */
#define VAX_11       4                                /* VAX class machines */
#define IEEE_32_96   6        /* IEEE standard 32 bit float : 96 bit double */
#define NSTD         6       /* Number of standards currently in the system 
                                            should be same as last standard */

#define PD_READ   0
#define PD_WRITE  1
#define PD_APPEND 2
#define PD_OPEN   3
#define PD_CREATE 4
#define PD_CLOSE  5
#define PD_TRACE  6
#define PD_PRINT  7

#define ROW_MAJOR_ORDER     101
#define COLUMN_MAJOR_ORDER  102

#define PF_ALPHABETIC 1
#define PF_DISK_ORDER 2

#define X_AND_Y 1
#define X_ONLY  2
#define Y_ONLY  3

/*--------------------------------------------------------------------------*/
/* Macros                                                                    */
/*--------------------------------------------------------------------------*/

#define PN_sizeof(type, tab)  _lite_PD_lookup_size(type, tab)

#define PD_get_mode(file)        ((file)->mode)
#define PD_set_mode(file, v)     (file)->mode = (v)
#define PD_get_offset(file)      ((file)->default_offset)
#define PD_set_offset(file, v)   (file)->default_offset = (v)

/* #define PD_file_maximum_size(file) file->maximum_size           */
/* replaced by PD_get_max_file_size and PD_set_max_file_size below */

#define PD_get_max_file_size(file)    ((file)->maximum_size)
#define PD_set_max_file_size(file, v) (file)->maximum_size = (v)


#define PD_entry_type(ep)          ((ep)->type)
#define PD_entry_dimensions(ep)    ((ep)->dimensions)
#define PD_entry_address(ep)       ((ep)->blocks[0].diskaddr)
#define PD_entry_number(ep)        ((ep)->number)
#define PD_entry_indirects(ep)     ((ep)->indirects)
#define PD_entry_blocks(ep)        ((ep)->blocks)
#define PD_block_address(ep, n)    ((ep)->blocks[n].diskaddr)
#define PD_block_number(ep, n)     ((ep)->blocks[n].number)
#define PD_n_blocks(ep)         (lite_SC_arrlen((ep)->blocks)/sizeof(symblock))

#define PD_get_major_order(file)       ((file)->major_order)
#define PD_set_major_order(file, type) (file)->major_order = (type)

#define PD_has_directories(file)                                             \
 (lite_SC_def_lookup("Directory", file->chart) != NULL)

#define PD_inquire_type(file, name)                                          \
 (defstr *) lite_SC_def_lookup(name, file->chart)

#define PD_inquire_host_type(file, name)                                     \
 (defstr *) lite_SC_def_lookup(name, file->host_chart)

#define PD_inquire_table_type(table, name)                                   \
 (defstr *) lite_SC_def_lookup(name, table)

#define PD_hyper_refp(name, commnd)                                          \
   {char c, s[MAXLINE];                                                      \
    strcpy(s, name);                                                         \
    c = s[strlen(s) - 1];                                                    \
    if ((c == ')') || (c == ']'))                                            \
       (commnd);}

#define PD_hyper_ref(s, name)                                                \
   {char c;                                                                  \
    strcpy(s, name);                                                         \
    c = s[strlen(s) - 1];                                                    \
    if ((c == ')') || (c == ']'))                                            \
       {s[strlen(s) - 1] = '\0';                                             \
        lite_SC_lasttok(s, "[]()");};}

#define PD_INQUIRE_ATTRIBUTE(x)                                              \
    ((attribute *) lite_SC_def_lookup((x), file->attrtab))

#define PD_INQUIRE_ATTRIBUTE_VALUE(x)                                        \
    ((attribute_value *) lite_SC_def_lookup((x), file->attrtab))

#define PD_CAST_TYPE(t, d, vg, vl, err_fnc, err_msg, err_arg)                \
    {if (d->cast_offs < 0L)                                                  \
        t = d->type;                                                         \
     else                                                                    \
        {t = DEREF(vl + d->cast_offs);                                       \
         if (t == NULL)                                                      \
            {if (DEREF(vg) != NULL)                                          \
                {err_fnc(err_msg, err_arg);}                                 \
             else                                                            \
                t = d->type;};};}

/*--------------------------------------------------------------------------*/
/*                          TYPEDEFS AND STRUCTS                            */
/*--------------------------------------------------------------------------*/

struct s_data_alignment {
   int char_alignment;
   int ptr_alignment;
   int short_alignment;
   int int_alignment;
   int long_alignment;
   int float_alignment;
   int double_alignment;
   int struct_alignment;
};

typedef struct s_data_alignment data_alignment;


struct s_data_standard {
   int ptr_bytes;
   int short_bytes;
   int short_order;
   int int_bytes;
   int int_order;
   int long_bytes;
   int long_order;
   int float_bytes;
   long *float_format;
   int *float_order;
   int double_bytes;
   long *double_format;
   int *double_order;
};

typedef struct s_data_standard data_standard;


struct s_PDBfile {
   FILE *stream;
   char *name;
   char *type;                        /* used by files masqerading as PDB */
   HASHTAB *symtab;
   HASHTAB *chart;
   HASHTAB *host_chart;
   HASHTAB *attrtab;
   long maximum_size;                  /* for file family bookkeeping */
   char *previous_file;                /* for file family bookkeeping */
   int mode;
   int default_offset;
   data_standard *std;
   data_standard *host_std;
   data_alignment *align;
   data_alignment *host_align;
   char *current_prefix;
   int flushed;
   int virtual_internal;
   int system_version;
   int major_order;
   char *date;
   long headaddr;
   long symtaddr;
   long chrtaddr;
};

typedef struct s_PDBfile PDBfile;


/*
 * Dimension descriptor - describe an array dimension range
 * NOTE: changes here must be reflected in sxhook.c
 */
struct s_dimdes {
   long index_min;
   long index_max;
   long number;
   struct s_dimdes *next;
};

typedef struct s_dimdes dimdes;


/*
 * Member descriptor - describe a member efficiently
 * NOTE: changes here must be reflected in sxhook.c
 */
struct s_memdes {
   char *member;
   long member_offs;
   char *cast_memb;
   long cast_offs;
   char *type;
   char *base_type;
   char *name;
   dimdes *dimensions;
   long number;
   struct s_memdes *next;
};

typedef struct s_memdes memdes;

/*
 * Symbol table entry block - manage discontiguous data items
 * NOTE: changes here must be reflected in sxhook.c
 */
struct s_symblock {
   long number;
   long diskaddr;
};

typedef struct s_symblock symblock;


/*
 * Symbol table entry indirects
 * NOTE: changes here must be reflected in sxhook.c
 */
struct s_symindir {
   long addr;
   long n_ind_type;
   long arr_offs;
};

typedef struct s_symindir symindir;


/*
 * Symbol table entry - the def member of a hashel
 * NOTE: changes here must be reflected in sxhook.c
 */
struct s_syment {
   char *type;
   dimdes *dimensions;
   long number;
   symindir indirects;
   symblock *blocks;
};

typedef struct s_syment syment;


/*
 * Structure chart entry - the def member of a hashel
 * NOTE: changes here must be reflected in sxhook.c
 */
struct s_defstr {
   char *type;
   long size_bits;
   long size;
   int alignment;   /* type begins on byte boundary divisible by alignment */
   int n_indirects;                              /* number of indirections */
   int convert;
   int onescmp;      /* TRUE iff ones complement arithmetic - old machines */
   int unsgned;                  /* TRUE iff the integral type is unsigned */
   int order_flag;
   int *order;
   long *format;
   memdes *members;
};

typedef struct s_defstr defstr;

typedef memdes        *(*PFPmemdes)() ;
typedef memdes        *(*ReaderFuncType)(memdes *);
typedef memdes        *(*WriterFuncType)(PDBfile *,char *,defstr *);

/*
 * Indirect reference tag.
 */
struct s_PD_itag {
   long nitems;                              /* number of items pointed to */
   char *type;                                  /* type of item pointed to */
   long addr;                           /* address of the itag owning data */
   int flag;                            /* TRUE if this itag owns the data */
};

typedef struct s_PD_itag PD_itag;


/*
 * Hyper-index support structure
 */
struct s_dimind {
   long stride;
   long start;
   long stop;
   long step;
};

typedef struct s_dimind dimind;


struct s_attribute {
   char *name;
   char *type;
   lite_byte **data;
   long size;
   long indx;
};

typedef struct s_attribute attribute;


struct s_attribute_value {
   attribute *attr;
   long indx;
   struct s_attribute_value *next;
};

typedef struct s_attribute_value attribute_value;


/*
 * PD_IMAGE is a faithful shadow of the PG_image which is not defined yet
 * any changes to PG_image must be reflected here
 */
struct s_PD_image {
   int version_id;
   char *label;
   double xmin;
   double xmax;
   double ymin;
   double ymax;
   double zmin;
   double zmax;
   char *element_type;
   unsigned char *buffer;
   int kmax;
   int lmax;
   long size;
   int bits_pixel;
   char *palette;
};

typedef struct s_PD_image PD_image;

/*--------------------------------------------------------------------------*/
/*                            PDBLIB VARIABLES                              */
/*--------------------------------------------------------------------------*/

extern jmp_buf                _lite_PD_close_err ;
extern jmp_buf                _lite_PD_print_err ;
extern jmp_buf                _lite_PD_read_err ;
extern jmp_buf                _lite_PD_trace_err ;
extern char                lite_PD_err[] ;
extern int                lite_PD_buffer_size ;
extern int                lite_FORMAT_FIELDS ;
extern data_standard        lite_IEEEA_STD ;
extern data_standard        lite_IEEEB_STD ;
extern data_standard        lite_INTELA_STD ;
extern data_standard        lite_CRAY_STD ;
extern data_standard        lite_VAX_STD ;
extern data_standard        lite_DEF_STD ;
extern data_standard        *lite_INT_STANDARD ;
extern data_alignment        lite_M68000_ALIGNMENT ;
extern data_alignment         lite_INTELA_ALIGNMENT ;
extern data_alignment         lite_UNICOS_ALIGNMENT ;
extern data_alignment        lite_SPARC_ALIGNMENT ;
extern data_alignment        lite_RS6000_ALIGNMENT ;
extern data_alignment        lite_MIPS_ALIGNMENT ;
extern data_alignment         lite_DEF_ALIGNMENT ;
extern data_alignment        *lite_INT_ALIGNMENT ;
extern ReaderFuncType        lite_pdb_rd_hook;
extern WriterFuncType        lite_pdb_wr_hook;


/*--------------------------------------------------------------------------*/
/*                            PDBLIB FUNCTIONS                              */
/*--------------------------------------------------------------------------*/

extern int                lite_PD_cd (PDBfile*,char*);
extern int                lite_PD_close (PDBfile*);
extern int                lite_PD_convert (char**,char**,char*,char*,long,
                                         data_standard*,data_standard*,
                                         data_standard*,long*,long*,HASHTAB*,
                                         HASHTAB*,int,int);
extern dimdes *                lite_PD_copy_dims (dimdes*);
extern memdes *                lite_PD_copy_members (memdes*);
extern syment *                lite_PD_copy_syment (syment*);
extern defstr *                lite_PD_defncv (PDBfile*,char*,long,int);
extern char *                lite_PD_dereference (char*);
extern void                lite_PD_error (char*,int);
extern lite_byte *                lite_PD_get_attribute (PDBfile*,char*,char*);
extern long                lite_PD_hyper_number (PDBfile*,char*,syment*);
extern syment *                lite_PD_inquire_entry (PDBfile*,char*,int,char*);
extern char **                lite_PD_ls (PDBfile*,char*,char*,int*);
extern PDBfile *        lite_PD_open (char*,char*);
extern char *                lite_PD_pwd (PDBfile*);
extern int                lite_PD_read (PDBfile*,char*,lite_byte*);
extern int                lite_PD_read_alt (PDBfile*,char*,lite_byte*,long*);
extern int                lite_PD_read_as (PDBfile*,char*,char*,lite_byte*);
extern int                lite_PD_read_as_alt (PDBfile*,char*,char*,lite_byte*,long*);
extern defstr *                lite_PD_typedef (PDBfile*,char*,char*);
extern int                _lite_PD_align (long,char*,HASHTAB*,int*);
extern void                _lite_PD_check_casts (HASHTAB*,char**,long);
extern void                _lite_PD_clr_table (HASHTAB*,FreeFuncType);
extern long                _lite_PD_comp_num (dimdes*);
extern int                _lite_PD_compare_std (data_standard*,data_standard*,
                                              data_alignment*,data_alignment*);
extern int                _lite_PD_convert (char**,char**,long,int,defstr*,
                                          defstr*,data_standard*,long*,long*);
extern void                _lite_PD_convert_ascii (char*,char*,long,int,int);
extern data_alignment *        _lite_PD_copy_alignment (data_alignment*);
extern data_standard *        _lite_PD_copy_standard (data_standard*);
extern void                _lite_PD_d_install (char*,defstr*,HASHTAB*);
extern defstr *                _lite_PD_defstr_inst (char*,memdes*,int,int*,long*,
                                              HASHTAB*,HASHTAB*,
                                              data_alignment*,
                                              data_alignment*,int);
extern void                _lite_PD_e_install (char*,syment*,HASHTAB*);
extern syment *                _lite_PD_effective_ep (PDBfile*,char*,int,char*);
extern dimdes *                _lite_PD_ex_dims (char*,int,int);
extern long                _lite_PD_extract_field (char*,int,int,int,int*);
extern void                _lite_PD_fconvert (char**,char**,long,int,long*,int*,
                                           long*,int*,int,int,int);
extern char *                _lite_PD_fixname (PDBfile*,char*);
extern long                _lite_PD_hyper_number (PDBfile*,char*,long,dimdes*,
                                               long*);
extern int                _lite_PD_hyper_read (PDBfile*,char*,char*,syment*,
                                             lite_byte*);
extern void                _lite_PD_iconvert (char**,char**,long,long,int,long,
                                           int,int);
extern int                _lite_PD_indexed_read_as (PDBfile*,char*,char*,lite_byte*,
                                                  int,long*,syment*);
extern int                _lite_PD_indirection (char*);
extern void                _lite_PD_init_chrt (PDBfile*);
extern void                _lite_PD_init_dimind (dimind*,long,long,char*);
extern long                _lite_PD_lookup_size (char*,HASHTAB*);
extern defstr *                _lite_PD_lookup_type (char*,HASHTAB*);
extern char *                _lite_PD_member_base_type (char*);
extern long                _lite_PD_member_items (char*);
extern long                _lite_PD_member_location (char*,HASHTAB*,defstr*,
                                                  memdes**);
extern char *                _lite_PD_member_name (char*);
extern char *                _lite_PD_member_type (char*);
extern data_alignment *        _lite_PD_mk_alignment (char*);
extern defstr *                _lite_PD_mk_defstr (char*,memdes*,long,int,int,int,
                                            int*,long*);
extern memdes *                _lite_PD_mk_descriptor (char*,int);
extern dimdes *                _lite_PD_mk_dimensions (long,long);
extern PDBfile *        _lite_PD_mk_pdb (char*);
extern data_standard *        _lite_PD_mk_standard (void);
extern syment *                _lite_PD_mk_syment (char*,long,long,symindir*,dimdes*);
extern int                _lite_PD_null_pointer (char*,int);
extern int                _lite_PD_pio_close (FILE*);
extern int                _lite_PD_pio_printf (FILE*,char*,...);
extern int                _lite_PD_pio_seek (FILE*,long,int);
extern int                _lite_PD_prim_typep (char*,HASHTAB*,int);
extern int                _lite_PD_rd_chrt (PDBfile*);
extern int                _lite_PD_rd_extras (PDBfile*);
extern int                _lite_PD_rd_format (PDBfile*);
extern int                _lite_PD_rd_itag (PDBfile*,PD_itag*);
extern void                _lite_PD_rd_prim_extras (PDBfile*,int,int,char*);
extern long                _lite_PD_rd_syment (PDBfile*,syment*,char*,lite_byte*);
extern int                _lite_PD_rd_symt (PDBfile*);
extern char *                _lite_PD_rfgets (char*,int,FILE*);
extern void                _lite_PD_rl_alignment (data_alignment*);
extern void                _lite_PD_rl_defstr (defstr*);
extern void                _lite_PD_rl_descriptor (memdes*);
extern void                _lite_PD_rl_dimensions (dimdes*);
extern void                _lite_PD_rl_pdb (PDBfile*);
extern void                _lite_PD_rl_standard (data_standard*);
extern void                _lite_PD_rl_syment (syment*);
extern void                _lite_PD_rl_syment_d (syment*);
extern void                _lite_PD_setup_chart (HASHTAB*,data_standard*,
                                              data_standard*,data_alignment*,
                                              data_alignment*,int);
extern long                _lite_PD_skip_over (PDBfile*,long,int);
extern long                _lite_PD_str_size (memdes*,HASHTAB*);
extern int                _lite_PD_unp_bits (char*,char*,int,int,int,int,long,
                                           long);

#ifdef PDB_WRITE
extern int                lite_PD_mkdir (PDBfile*,char*);
extern int                lite_PD_write (PDBfile*,char*,char*,lite_byte*);
extern int                lite_PD_write_as (PDBfile*,char*,char*,char*,lite_byte*);
extern int                lite_PD_write_alt (PDBfile*,char*,char*,lite_byte*,int,
                                           long*);
extern int                lite_PD_write_as_alt (PDBfile*,char*,char*,char*,lite_byte*,
                                              int,long*);
extern PDBfile *        lite_PD_create (char*);
extern int                _lite_PD_adj_dimensions (PDBfile*,char*,syment*);
extern int                _lite_PD_add_block (PDBfile*,syment*,dimdes*);
extern int                _lite_PD_hyper_write (PDBfile*,char*,syment*,lite_byte*,
                                              char*);
extern void                _lite_PD_convert_attrtab (PDBfile*);
extern int                _lite_PD_wr_format (PDBfile*);
extern defstr *                lite_PD_defstr (PDBfile*,char*,...);
extern int                lite_PD_cast (PDBfile*,char*,char*,char*);
extern int                _lite_PD_extend_file (PDBfile*,long);
extern long                _lite_PD_wr_syment (PDBfile*,char*,long,char*,char*);
extern long                _lite_PD_number_refd (lite_byte*,char*,HASHTAB*);
extern int                _lite_PD_wr_itag (PDBfile*,long,char*,long,int);
extern void                _lite_PD_eod (PDBfile*);
extern int                lite_PD_target (data_standard*,data_alignment*);
extern int                lite_PD_flush (PDBfile*);
extern long                _lite_PD_wr_chrt (PDBfile*);
extern long                _lite_PD_wr_symt (PDBfile*);
extern int                _lite_PD_wr_extras (PDBfile*);
extern int                _lite_PD_rev_chrt (PDBfile*);
extern void                _lite_PD_wr_prim_extras (FILE*,HASHTAB*,int,int);
extern syment                 *lite_PD_defent_alt (PDBfile*,char*,char*,int,long*);
extern syment                *_lite_PD_defent (PDBfile*,char*,char*,long,dimdes*);
#endif /* PDB_WRITE */
   
#ifdef __cplusplus
}
#endif

#endif /* !_LITE_PDB_H */
