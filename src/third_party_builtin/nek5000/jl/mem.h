#ifndef MEM_H
#define MEM_H

/* requires:
     <stddef.h> for size_t, offsetof
     <stdlib.h> for malloc, calloc, realloc, free
     <string.h> for memcpy
     "c99.h"
     "fail.h"
*/

#if !defined(C99_H) || !defined(FAIL_H)
#error "mem.h" requires "c99.h" and "fail.h"
#endif

/* 
   All memory management goes through the wrappers defined in this
   header. Diagnostics can be turned on with
     -DPRINT_MALLOCS=1
   Then all memory management operations will be printed to stdout.
   
   Most memory management occurs through use of the "array" type,
   defined below, which defines a generic dynamically-sized array
   that grows in bursts. The "buffer" type is a "char" array and
   is often passed around by code to provide a common area for
   scratch work.
*/

#ifndef PRINT_MALLOCS
#  define PRINT_MALLOCS 0
#else
#  include <stdio.h>
#  ifndef comm_gbl_id
#    define comm_gbl_id PREFIXED_NAME(comm_gbl_id)
#    define comm_gbl_np PREFIXED_NAME(comm_gbl_np)
#    include "types.h"
     extern uint comm_gbl_id, comm_gbl_np;
#  endif
#endif

/*--------------------------------------------------------------------------
   Memory Allocation Wrappers to Catch Out-of-memory
  --------------------------------------------------------------------------*/

void *smalloc(size_t size, const char *file, unsigned line);


void *scalloc(
  size_t nmemb, size_t size, const char *file, unsigned line);


void *srealloc(
  void *restrict ptr, size_t size, const char *file, unsigned line);


#define tmalloc(type, count) \
  ((type*) smalloc((count)*sizeof(type),__FILE__,__LINE__) )
#define tcalloc(type, count) \
  ((type*) scalloc((count),sizeof(type),__FILE__,__LINE__) )
#define trealloc(type, ptr, count) \
  ((type*) srealloc((ptr),(count)*sizeof(type),__FILE__,__LINE__) )

#if PRINT_MALLOCS
void sfree(void *restrict ptr, const char *file, unsigned line);
#define free(x) sfree(x,__FILE__,__LINE__)
#endif

/*--------------------------------------------------------------------------
   A dynamic array
  --------------------------------------------------------------------------*/
struct array { void *ptr; size_t n,max; };
#define null_array {0,0,0}
void array_init_(struct array *a, size_t max, size_t size,
                 const char *file, unsigned line);
void array_resize_(struct array *a, size_t max, size_t size,
                   const char *file, unsigned line);
void *array_reserve_(struct array *a, size_t min, size_t size,
                     const char *file, unsigned line);

#define array_free(a) (free((a)->ptr))
#define array_init(T,a,max) array_init_(a,max,sizeof(T),__FILE__,__LINE__)
#define array_resize(T,a,max) array_resize_(a,max,sizeof(T),__FILE__,__LINE__)
#define array_reserve(T,a,min) array_reserve_(a,min,sizeof(T),__FILE__,__LINE__)

void array_cat_(size_t size, struct array *d, const void *s, size_t n,
                       const char *file, unsigned line);

#define array_cat(T,d,s,n) array_cat_(sizeof(T),d,s,n,__FILE__,__LINE__)

/*--------------------------------------------------------------------------
   Buffer = char array
  --------------------------------------------------------------------------*/
typedef struct array buffer;
#define null_buffer null_array
#define buffer_init(b,max) array_init(char,b,max)
#define buffer_resize(b,max) array_resize(char,b,max)
#define buffer_reserve(b,max) array_reserve(char,b,max)
#define buffer_free(b) array_free(b)

/*--------------------------------------------------------------------------
   Alignment routines
  --------------------------------------------------------------------------*/
#define ALIGNOF(T) offsetof(struct { char c; T x; }, x)
size_t align_as_(size_t a, size_t n) ;
#define align_as(T,n) align_as_(ALIGNOF(T),n)
#define align_ptr(T,base,offset) ((T*)((char*)(base)+align_as(T,offset)))
#endif

