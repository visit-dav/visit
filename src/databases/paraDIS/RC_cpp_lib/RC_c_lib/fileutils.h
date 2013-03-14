/* $Id: fileutils.h,v 1.3 2006/03/31 01:04:37 rcook Exp $ */

#ifndef __RCFILEUTIL_H
#define __RCFILEUTIL_H
#include <stdlib.h>
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif
/* utilities to deal with files and directories */

size_t fread_loop(void *bufp, size_t elemSize, size_t elems2Read, FILE *fp);
int mkdir_recursive(const char *dirname); 


#ifdef __cplusplus
}
#endif
#endif
