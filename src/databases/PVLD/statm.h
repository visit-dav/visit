
#ifndef GET_STATM_H
#define GET_STATM_H

#ifdef __cplusplus 
extern "C" {
#endif
void getstatm_(int *totsize, int *rss, int* share, int *text, int *lib, 
               int *data, int* dt );
#ifdef __cplusplus 
}
#endif

#endif
