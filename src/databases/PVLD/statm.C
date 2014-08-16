/*  A c subroutine called by Fortran code to 
    obtain the memory status of the current process.

    Author: Xudong Xiao, Ph.D., Corvid Technologies, Inc.
    Date  : Mar-29-2010

    Notes:
          This C subroutine visit the proc/pid/statm to 
      obtain the sizes needed.

      This code also assumes the page size is 4KB.

*/
    
#define PAGE_SIZE 4
#include <sys/types.h>
#ifndef _WIN32
#include <unistd.h>
#endif

/* subroutine return all the sizes in KB  */
/*  totsize: total program size
    rss: resident set size
    share: shared pages
    text : text(code)
    lib  : library
    data : data/stack
    dt   : unused in linux
*/
#include<stdio.h>

#include <statm.h>
#include <snprintf.h>
#ifdef _WIN32
#include <process.h>
#endif

void getstatm_(unsigned int *totsize, unsigned int *rss, unsigned int* share,
    unsigned int *text, unsigned int *lib, unsigned int *data, unsigned int* dt)
{
  char buf[40];
  
  *totsize = 0;
  *rss = 0;
  *share = 0;
  *text  = 0;
  *lib   = 0;
  *data  = 0;
  *dt    = 0;
#ifdef _WIN32
  SNPRINTF(buf,40, "/proc/%u/statm", (unsigned)_getpid());
#else
  SNPRINTF(buf,40, "/proc/%u/statm", (unsigned)getpid());
#endif
  FILE *fp = fopen(buf,"r");
  if(fp){
    fscanf(fp, "%u %u %u %u %u %u %u",totsize,rss, share,text,lib, data, dt);
    (*totsize) *= PAGE_SIZE;
    (*rss) *= PAGE_SIZE;
    (*share) *= PAGE_SIZE;
    (*text) *= PAGE_SIZE;
    (*lib) *= PAGE_SIZE;
    (*data) *= PAGE_SIZE;
    (*dt) *= PAGE_SIZE;
    fclose(fp);
  }
  return;
}

#ifdef DEBUG_STATM 
int main(){
  unsigned int totsize,rss, share, text,lib,data,dt;
  getstatm_(&totsize,&rss,&share,&text, &lib,&data,&dt);
  printf("%u, %u, %u, %u, %u, %u, %u\n",totsize, rss, share, text, lib, data, dt);
  return 0;
}
#endif
      
    

