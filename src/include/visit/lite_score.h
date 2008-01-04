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

/*
 * SCORE.H - include file for PACT standard core package
 *
 * Source Version: 2.0
 * Software Release #92-0043
 *
 */

#ifndef _LITE_SCORE_H
#define _LITE_SCORE_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Standard include files...
 */

#include <limits.h>                /*needed for SHRT_MAX*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>



/*--------------------------------------------------------------------------*/
/*                           DEFINED CONSTANTS                              */
/*--------------------------------------------------------------------------*/

#ifndef FALSE
#  define FALSE                        0
#  define TRUE                        1
#endif

#define ERR_FREE                   2            /*error free ret flg for longjmp*/
#define ABORT                     3            /*error return flag for longjmps*/

#define BINARY_MODE_R                "rb"
#define BINARY_MODE_RPLUS         "r+b"
#define BINARY_MODE_WPLUS         "w+b"
#define LRG_TXT_BUFFER                4096
#define NODOC                        0
#define HSZLARGE                521
#define SC_BITS_lite_byte                8

#define SC_CHAR_I                1
#define SC_SHORT_I               2
#define SC_INTEGER_I             3
#define SC_LONG_I                4
#define SC_FLOAT_I               5
#define SC_DOUBLE_I              6
#define SC_REAL_I                7
#define SC_STRING_I              8
#define SC_POINTER_I             9
#define SC_PCONS_P_I             10
#define SC_VOID_I                11
#define SC_SHORT_P_I                   12
#define SC_INTEGER_P_I           13
#define SC_LONG_P_I              14
#define SC_FLOAT_P_I             15
#define SC_REAL_P_I              16
#define SC_DOUBLE_P_I            17
#define SC_PCONS_I               18
#define SC_STRUCT_I              19        /*is this pointer type? */
#define SC_UNKNOWN_I             20
#define SC_CHAR_8_I              21        /*for F77 users and interfacers */
#define SC_USER_I                22        /*first user defined index available */

#define UNCOLLECT               SHRT_MAX
#define MAXLINE                            FILENAME_MAX

#undef PRINT
#define PRINT (*putln)

#undef CONVERT
#define CONVERT (*SC_convert_hook)

#undef SIZEOF
#define SIZEOF (*SC_sizeof_hook)

#define io_close   (*lite_io_close_hook)
#define io_flush   (*lite_io_flush_hook)
#define io_open    (*lite_io_open_hook)
#define io_printf  (*lite_io_printf_hook)
#define io_read    (*lite_io_read_hook)
#define io_seek    (*lite_io_seek_hook)
#define io_setvbuf (*lite_io_setvbuf_hook)
#define io_tell    (*lite_io_tell_hook)
#define io_write   (*lite_io_write_hook)


#undef MAKE
#undef MAKE_N
#undef REMAKE
#undef REMAKE_N
#undef SFREE
#undef SFREE_N

/*--------------------------------------------------------------------------*/
/*                           PROCEDURAL MACROS                              */
/*--------------------------------------------------------------------------*/

#ifndef MIN
#  define MIN(X,Y)        ((X)<(Y)?(X):(Y))
#endif

#ifndef MAX
#  define MAX(X,Y)        ((X)>(Y)?(X):(Y))
#endif

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

#define DEREF(x) (*(char **) (x))

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/* FMAKE - memory allocation and bookkeeping macro */

#define FMAKE(x, name) ((x *) lite_SC_alloc(1L, (long) sizeof(x), name))

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/* FMAKE_N - allocate a block of type x and return a pointer to it */

#define FMAKE_N(x, n, name) ((x *)lite_SC_alloc((long)n,(long)sizeof(x),name))

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/* MAKE_N - allocate a block of type x and return a pointer to it */

#define MAKE_N(x, n) ((x *) lite_SC_alloc((long) n, (long) sizeof(x), NULL))

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/* REMAKE_N - reallocate a block of type x and return a pointer to it */

#define REMAKE_N(p, x, n)                                                    \
   (p = (x *) lite_SC_realloc((lite_byte *) p, (long) (n), (long) sizeof(x)))

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/* SFREE - release memory and do bookkeeping */

#define SFREE(x)                                                             \
   {lite_SC_free(x);                                                         \
    x = NULL;}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                         TYPEDEFS AND STRUCTS                             */
/*--------------------------------------------------------------------------*/


typedef void                lite_byte ;
typedef void                (*FreeFuncType)(void *) ;
typedef void                *((*MallocFuncType)(size_t));
typedef void                *((*ReallocFuncType)(void *,size_t));
typedef void                (*PFVoid)() ;
typedef int                (*PFInt)() ;
typedef long                (*PFLong)() ;
typedef double                (*PFDouble)() ;

/*
 * Pcons definition
 *    - useful for linked lists, association lists and whatnot
 *    - the type fields are for bookkeeping that may be crucial to
 *    - other packages
 */
typedef struct s_pcons {
   char                *car_type;
   lite_byte         *car;
   char         *cdr_type;
   lite_byte         *cdr;
} pcons;

/*
 * Hashel definition
 *          - hashel have two members, the name and def
 *          - the name is a string on which the function hash operates
 *          - type is a string which names the type of object associated
 *          - with the name and type checking on the object can be used
 *          - to determine appropriate casts for accessing the object
 *          - def is a pointer to an object, usually a struct with
 *          - the necessary members for the specific application
 */
typedef struct s_hashel {
   char         *name;
   char         *type;
   lite_byte         *def;
   struct s_hashel *next; /* next entry in chain */
} hashel ;

/*
 * Hash table type.
 */
typedef struct s_hashtab {
   int                 size;
   int                 nelements;
   int                 docp;
   hashel         **table;
} HASHTAB ;

typedef union u_SC_address {
   long                diskaddr;
   PFInt         funcaddr;
   char         *memaddr;
} SC_address ;

typedef FILE *        (*PFfopen)(char*,char*);
typedef long         (*PFftell)(void*);
typedef size_t        (*PFfread)(lite_byte*,size_t,size_t,void*);
typedef size_t        (*PFfwrite)(void*,size_t,size_t,void*);
typedef int        (*PFsetvbuf)(void*,char*,int,size_t);
typedef int        (*PFfclose)(void*);
typedef int        (*PFfseek)(void*,long,int);
typedef int        (*PFfprintf)(void*,char*,...);
typedef int        (*PFungetc)(int,void*);
typedef int        (*PFfflush)(void*);

/*--------------------------------------------------------------------------*/
/*                         VARIABLE DECLARATIONS                            */
/*--------------------------------------------------------------------------*/

extern int                lite_SC_c_sp_alloc;
extern int                lite_SC_c_sp_free;
extern int                lite_SC_c_sp_diff;
extern int                lite_SC_c_sp_max;
extern int *                lite_LAST;
extern char *                lite_SC_CHAR_S ;

/* IO hooks */
extern PFfclose lite_io_close_hook;
extern PFfflush lite_io_flush_hook;
extern PFfopen lite_io_open_hook;
extern PFfprintf lite_io_printf_hook;
extern PFfread lite_io_read_hook;
extern PFfseek lite_io_seek_hook;
extern PFsetvbuf lite_io_setvbuf_hook;
extern PFftell lite_io_tell_hook;
extern PFfwrite lite_io_write_hook;



/*--------------------------------------------------------------------------*/
/*                          FUNCTION DECLARATIONS                           */
/*--------------------------------------------------------------------------*/

extern lite_byte *                lite_SC_alloc (long,long,char*);
extern long                lite_SC_arrlen (lite_byte*);
extern char *                lite_SC_date (void);
extern lite_byte *                lite_SC_def_lookup (char*,HASHTAB*);
extern char **                lite_SC_dump_hash (HASHTAB*,char*,int);
extern char *                lite_SC_firsttok (char*,char*);
extern int                lite_SC_free (lite_byte*);
extern int                lite_SC_hash (char*,int);
extern void                lite_SC_hash_clr (HASHTAB*);
extern char **                lite_SC_hash_dump (HASHTAB*,char*);
extern int                lite_SC_hash_rem (char*,HASHTAB*);
extern hashel *                lite_SC_install (char*,lite_byte*,char*,HASHTAB*);
extern char *                lite_SC_lasttok (char*,char*);
extern hashel *                lite_SC_lookup (char*,HASHTAB*);
extern HASHTAB *        lite_SC_make_hash_table (int,int);
extern int                lite_SC_mark (lite_byte*,int);
extern lite_byte *                lite_SC_realloc (lite_byte*,long,long);
extern int                lite_SC_ref_count (lite_byte*);
extern int                lite_SC_regx_match (char*,char*);
extern void                lite_SC_rl_hash_table (HASHTAB*);
extern int                lite_SC_stoi (char*);
extern long                lite_SC_stol (char*);
extern void                lite_SC_string_sort (char**,int);
extern char *                lite_SC_strrev (char*);
extern char *                lite_SC_strsavef (char*,char*);
extern hashel *                _lite_SC_install (char*,lite_byte*,char*,HASHTAB*);
extern char *                _lite_SC_pr_tok (char*,char*);
extern long int                _lite_SC_strtol (char*,char**,int);

#if !defined(HAVE_MEMMOVE) && !defined(_WIN32)
extern void *                memmove (void*,const void*,size_t) ;
#endif

#ifdef __cplusplus
}
#endif

#endif /* !_LITE_SCORE_H */
