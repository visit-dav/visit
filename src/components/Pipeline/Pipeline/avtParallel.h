// ************************************************************************* //
//                                avtParallel.h                              //
// ************************************************************************* //

#ifndef AVT_PARALLEL_H
#define AVT_PARALLEL_H

#include<pipeline_exports.h>

#include <vector>
#include <string>
class AttributeGroup;

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
PIPELINE_API bool    ThisProcessorHasMaximumValue(double);
PIPELINE_API void    UnifyMinMax(double *, int, int=0);
PIPELINE_API int     UnifyMaximumValue(int);

PIPELINE_API void    BroadcastInt(int &i);
PIPELINE_API void    BroadcastIntVector(std::vector<int>&, int myrank);
PIPELINE_API void    BroadcastString(std::string &s, int myrank);
PIPELINE_API void    BroadcastStringVector(std::vector<std::string>&,
                                           int myrank);
PIPELINE_API void    BroadcastStringVectorVector(std::vector<std::vector<std::string> >&,
                                                 int myrank);
PIPELINE_API bool    GetListToRootProc(std::vector<std::string> &, int);

PIPELINE_API int     GetUniqueMessageTag();

PIPELINE_API void    GetAttToRootProc(AttributeGroup &, int);
PIPELINE_API void    GetFloatArrayToRootProc(float *, int, bool &);

#endif


