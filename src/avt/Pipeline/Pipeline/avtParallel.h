// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                avtParallel.h                              //
// ************************************************************************* //

#ifndef AVT_PARALLEL_H
#define AVT_PARALLEL_H

#include <pipeline_exports.h>

#include <vector>
#include <string>
class AttributeGroup;

#ifdef PARALLEL
// we define VISIT_MPI_COMM this way to avoid having to include mpi.h
// to get the def'n for MPI_Comm datatype
#define VISIT_MPI_COMM (*((MPI_Comm*) VISIT_MPI_COMM_PTR))
PIPELINE_API extern void *VISIT_MPI_COMM_PTR;
#endif

PIPELINE_API void    PAR_Exit(void);
PIPELINE_API void    PAR_Init(int &argc, char **&argv);
PIPELINE_API int     PAR_Rank(void);
PIPELINE_API int     PAR_Size(void);
PIPELINE_API bool    PAR_UIProcess(void);
PIPELINE_API void    PAR_WaitForDebugger(void);
PIPELINE_API bool    PAR_SetComm(void *);
PIPELINE_API void    PullInMPI_IOSymbols();

// NOTE: Think about deprecating these in favor of avtParallelContext.
PIPELINE_API void    Barrier(void);
PIPELINE_API bool    Collect(float *, int);
PIPELINE_API bool    Collect(double *, int);
PIPELINE_API bool    Collect(int *, int);
PIPELINE_API void    SumIntAcrossAllProcessors(int&);
PIPELINE_API void    SumLongAcrossAllProcessors(long&);
PIPELINE_API void    SumFloatAcrossAllProcessors(float&);
PIPELINE_API void    SumFloatArrayAcrossAllProcessors(float *, float *, int);
PIPELINE_API void    SumFloatArray(float *, float *, int);
PIPELINE_API void    SumDoubleAcrossAllProcessors(double&);
PIPELINE_API void    SumDoubleArrayAcrossAllProcessors(double *, double *,int);
PIPELINE_API void    SumDoubleArray(double *, double *,int);
PIPELINE_API void    SumDoubleArrayInPlace(double *, int);
PIPELINE_API void    SumIntArrayAcrossAllProcessors(int *, int *, int);
PIPELINE_API void    SumLongLongArrayAcrossAllProcessors(VISIT_LONG_LONG*, VISIT_LONG_LONG*, int);
PIPELINE_API bool    ThisProcessorHasMinimumValue(double);
PIPELINE_API bool    ThisProcessorHasMaximumValue(double);
PIPELINE_API void    UnifyMinMax(double *, int, int=0);
PIPELINE_API int     UnifyMaximumValue(int);
PIPELINE_API float   UnifyMaximumValue(float);
PIPELINE_API double  UnifyMaximumValue(double);
PIPELINE_API int     UnifyMinimumValue(int);
PIPELINE_API float   UnifyMinimumValue(float);
PIPELINE_API double  UnifyMinimumValue(double);
PIPELINE_API void    UnifyMaximumValue(std::vector<int>&, std::vector<int>&);
PIPELINE_API void    UnifyMinimumFloatArrayAcrossAllProcessors(float *, float *, int);
PIPELINE_API void    UnifyMaximumFloatArrayAcrossAllProcessors(float *, float *, int);
PIPELINE_API void    UnifyMinimumDoubleArrayAcrossAllProcessors(double *, double *, int);
PIPELINE_API void    UnifyMaximumDoubleArrayAcrossAllProcessors(double *, double *, int);

PIPELINE_API void    BroadcastInt(int &i);
PIPELINE_API void    BroadcastLongLong(VISIT_LONG_LONG &i);
PIPELINE_API void    BroadcastIntArray(int *array, int nArray);
PIPELINE_API void    BroadcastIntVector(std::vector<int>&, int myrank);
PIPELINE_API void    BroadcastIntVectorFromAny(std::vector<int> &, int, int);
PIPELINE_API void    BroadcastBool(bool &b);
PIPELINE_API void    BroadcastBoolVector(std::vector<bool> &b, int myrank);
PIPELINE_API void    BroadcastDouble(double &i);
PIPELINE_API void    BroadcastDoubleArray(double *array, int nArray);
PIPELINE_API void    BroadcastDoubleArrayFromAny(double *, int, int);
PIPELINE_API void    BroadcastDoubleVector(std::vector<double>&, int myrank);
PIPELINE_API void    BroadcastDoubleVectorFromAny(std::vector<double> &, int, int);
PIPELINE_API void    BroadcastString(std::string &s, int myrank);
PIPELINE_API void    BroadcastStringVector(std::vector<std::string>&,
                                           int myrank);
PIPELINE_API void    BroadcastStringVectorVector(std::vector<std::vector<std::string> >&,
                                                 int myrank);
PIPELINE_API bool    GetListToRootProc(std::vector<std::string> &, int);

PIPELINE_API void    CollectIntArraysOnRootProc(int *&, int *&, int *, int);
PIPELINE_API void    CollectDoubleArraysOnRootProc(double *&, int *&, double *, int);

PIPELINE_API int     GetUniqueMessageTag();
PIPELINE_API void    GetUniqueMessageTags(int *tags, int ntags);
PIPELINE_API int     GetUniqueStaticMessageTag();

PIPELINE_API void    GetAttToRootProc(AttributeGroup &, int);
PIPELINE_API void    GetFloatArrayToRootProc(float *, int, bool &);
PIPELINE_API void    GetDoubleArrayToRootProc(double *, int, bool &);
PIPELINE_API void    WaitAll(std::vector<int> &reqs, std::vector<int> &status );
PIPELINE_API void    WaitSome(std::vector<int> &reqs, std::vector<int> &done, std::vector<int> &status );
PIPELINE_API void    TestSome(std::vector<int> &reqs, std::vector<int> &done, std::vector<int> &status );
PIPELINE_API void    CancelRequest(void *req);

#endif
