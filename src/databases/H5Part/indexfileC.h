#ifndef _INDEX_FILEC_H
#define _INDEX_FILEC_H

#include <visit-config.h>

#ifdef HAVE_LIBFASTBIT

#ifdef __cplusplus
/* turn off name mangling if this is compiled with c++ compiler */
extern "C" {
#endif

#include <sys/types.h>

typedef caddr_t H5UCFile;

#define H5_FLOAT32 0
#define H5_FLOAT 0
#define H5_FLOAT64 1
#define H5_DOUBLE 1
#define H5_INT32 2
#define H5_INT64 3
#define H5_BYTE 4
#define H5_ERROR -1
  
H5UCFile H5UC_Open(char *filename);
void H5UC_Close(H5UCFile f);
int H5UC_getNumVariables(H5UCFile f);
void H5UC_getVariableNames(H5UCFile f,char **namelist,int maxlen);
char **H5UC_getVariableNamesAlloc(H5UCFile f,int *num); // allocate names
int64_t H5UC_getNumSteps(H5UCFile f);
double H5UC_getMinStep(H5UCFile f);
double H5UC_getMaxStep(H5UCFile f);
int64_t H5UC_getDatasetSize(int rank,int *dims,int datatype); // generic size calculator
int64_t H5UC_getVariableInfo(H5UCFile f, /* <in> */
                        char *name, /* <in> */
                        int64_t timestep, /* <in> */
                        int *rank, /* <out> */
                        int64_t *dims, /* <out> */
                        int *datatype); /* <out> (ignore if null) */
void H5UC_getData(H5UCFile f,char *varname,int64_t time,void *data);

#ifdef __cplusplus
}
#endif

#endif

#endif
