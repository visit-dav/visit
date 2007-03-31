// ************************************************************************* //
//                                avtParallel.h                              //
// ************************************************************************* //

#ifndef AVT_PARALLEL_H
#define AVT_PARALLEL_H

#include<pipeline_exports.h>

PIPELINE_API void    Barrier(void);
PIPELINE_API bool    Collect(float *, int);
PIPELINE_API bool    Collect(int *, int);
PIPELINE_API int     PAR_Rank(void);
PIPELINE_API int     PAR_Size(void);
PIPELINE_API void    SumIntAcrossAllProcessors(int&);
PIPELINE_API void    SumFloatAcrossAllProcessors(float&);
PIPELINE_API void    SumFloatArrayAcrossAllProcessors(float *, float *, int);
PIPELINE_API void    SumDoubleAcrossAllProcessors(double&);
PIPELINE_API void    SumDoubleArrayAcrossAllProcessors(double *, double *,int);
PIPELINE_API void    SumIntArrayAcrossAllProcessors(int *, int *, int);
PIPELINE_API bool    ThisProcessorHasMinimumValue(double);
PIPELINE_API void    UnifyMinMax(double *, int);
PIPELINE_API int     UnifyMaximumValue(int);


#endif


