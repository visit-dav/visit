#include <stddef.h>
#include <stdlib.h>
#include <math.h>    /* for cos, fabs */
#include <float.h>
#include <string.h>
#include "c99.h"
#include "name.h"
#include "fail.h"
#include "mem.h"

void *smalloc(size_t size, const char *file, unsigned line)
{
  void *restrict res = malloc(size);
  #if PRINT_MALLOCS
  fprintf(stdout,"MEM: proc %04d: %p = malloc(%ld) @ %s(%u)\n",
          (int)comm_gbl_id,res,(long)size,file,line), fflush(stdout);
  #endif
  if(!res && size)
    fail(1,file,line,"allocation of %ld bytes failed\n",(long)size);
  return res;
}

void *scalloc(
  size_t nmemb, size_t size, const char *file, unsigned line)
{
  void *restrict res = calloc(nmemb, size);
  #if PRINT_MALLOCS
  fprintf(stdout,"MEM: proc %04d: %p = calloc(%ld) @ %s(%u)\n",
          (int)comm_gbl_id,res,(long)size*nmemb,file,line), fflush(stdout);
  #endif
  if(!res && nmemb)
    fail(1,file,line,"allocation of %ld bytes failed\n",
           (long)size*nmemb);
  return res;
}

void *srealloc(
  void *restrict ptr, size_t size, const char *file, unsigned line)
{
  void *restrict res = realloc(ptr, size);
  #if PRINT_MALLOCS
  if(res!=ptr) {
    if(ptr)
      fprintf(stdout,"MEM: proc %04d: %p freed by realloc @ %s(%u)\n",
              (int)comm_gbl_id,ptr,file,line), fflush(stdout);
    fprintf(stdout,"MEM: proc %04d: %p = realloc of %p to %lu @ %s(%u)\n",
            (int)comm_gbl_id,res,ptr,(long)size,file,line), fflush(stdout);
  } else
    fprintf(stdout,"MEM: proc %04d: %p realloc'd to %lu @ %s(%u)\n",
            (int)comm_gbl_id,res,(long)size,file,line), fflush(stdout);
  #endif
  if(!res && size)
    fail(1,file,line,"allocation of %ld bytes failed\n",(long)size);
  return res;
}

#if PRINT_MALLOCS
void sfree(void *restrict ptr, const char *file, unsigned line)
{
  free(ptr);
  fprintf(stdout,"MEM: proc %04d: %p freed @ %s(%u)\n",
          (int)comm_gbl_id,ptr,file,line), fflush(stdout);
}
#endif


void array_init_(struct array *a, size_t max, size_t size,
                        const char *file, unsigned line)
{
  a->n=0, a->max=max, a->ptr=smalloc(max*size,file,line);
}
void array_resize_(struct array *a, size_t max, size_t size,
                          const char *file, unsigned line)
{
  a->max=max, a->ptr=srealloc(a->ptr,max*size,file,line);
}
void *array_reserve_(struct array *a, size_t min, size_t size,
                            const char *file, unsigned line)
{
  size_t max = a->max;
  if(max<min) {
    max+=max/2+1;
    if(max<min) max=min;
    array_resize_(a,max,size,file,line);
  }
  return a->ptr;
}


void array_cat_(size_t size, struct array *d, const void *s, size_t n,
                       const char *file, unsigned line)
{
  char *out = array_reserve_(d,d->n+n,size, file,line);
  memcpy(out+d->n*size, s, n*size);
  d->n+=n;
}


size_t align_as_(size_t a, size_t n) { return (n+a-1)/a*a; }

