#include "fileutils.h"
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/* return 0 on success, perr, or -1 (internal failure) value on failure */
int mkdir_recursive(const char *targetdir){
  char parentdir[1024]; 
  char *lastslash = NULL;
  int err = 0;
  strcpy(parentdir, targetdir); 
  lastslash = strrchr(parentdir, '/'); 
  if (lastslash && lastslash != parentdir) { 
    /* target directory not at root or is relative path, but contains a '/'
       check if we need to recursively create parent dirs */
    DIR *dir = opendir(parentdir); 
    *lastslash = 0; 
    if (!dir) {
      int err = mkdir_recursive(parentdir);
      if (err) return err; 
    }
    else closedir(dir); 
  }
  /* at this point, the parent directory exists */
  err = mkdir(targetdir, 0777); 
  if (err) {
    if (err == -1) {
      if  (errno == EEXIST) {
    return 0; 
      }
      else {
    return errno; 
      }
    }
    else return err; 
  }
  return 0; 
}
     
  
/* read in a loop until done or fread returns zero (EOF)
   return total number of bytes read
*/
size_t fread_loop(void *bufp, size_t elemSize, size_t elems2Read, FILE *theFile) {
  size_t totalRead = fread(bufp, elemSize, elems2Read, theFile);
  bufp = (void*)((char*)bufp + totalRead); 
  while (elems2Read > totalRead) {
    size_t numread = fread(bufp, elemSize, elems2Read - totalRead, theFile); 
    if (numread < (elems2Read - totalRead)) {
      /* EOF or Error */
      if (feof(theFile) != 0) {
        fprintf(stderr, "Warning:  Premature EOF in fread_loop.\n"); 
      }
      if (ferror(theFile) != 0) {
        fprintf(stderr, "Warning:  Stream Error in fread_loop.\n"); 
      }
      if (!numread) {
        return totalRead;
      }
    }
    bufp = (void*)((char*)bufp + numread); 
    totalRead += numread; 
  }
  return totalRead; 
}
