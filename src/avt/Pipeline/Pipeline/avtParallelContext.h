/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/
#ifndef AVT_PARALLEL_CONTEXT_H
#define AVT_PARALLEL_CONTEXT_H
#include <pipeline_exports.h>
#include <vector>
#include <string>
#ifdef PARALLEL
#include <mpi.h>
#endif

class AttributeGroup;

// ****************************************************************************
// Class: avtParallelContext
//
// Purpose:
//   A context that can be used for parallel operations.
//
// Notes:    This class does the typical AVT parallel operations but does it
//           on ranks specified by an internal communicator. By accessing parallel
//           operations through this class, algorithms will use the right
//           communicator without us having to expose MPI communicators all 
//           over the place.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug  4 15:43:45 PDT 2015
//
// Modifications:
//
// ****************************************************************************

class PIPELINE_API avtParallelContext
{
public:
    avtParallelContext();
    avtParallelContext(const avtParallelContext &);
    ~avtParallelContext();
    void operator = (const avtParallelContext &);

#ifdef PARALLEL
    // Direct access to communicator (for now).
    MPI_Comm GetCommunicator();
    void     SetCommunicator(MPI_Comm, bool owns = false);
#endif

    /**
      Rank within the group.
     */
    int Rank();

    /**
      Number of ranks within this rank's group.
     */
    int Size();

    /**
      Rank of this group within the total number of groups. This will
      be set to 0 if the default avtParallelContext is used.
     */
    int GroupRank();

    /**
      The number of groups into which the global group was partitioned. This will
      be set to 1 if the default avtParallelContext is used.
     */
    int GroupSize();

    /**
      Rank within the global group.
     */
    int GlobalRank();

    /**
      Size of the global group.
     */
    int GlobalSize();

    /**
      Create a new parallel context consisting of a set of ranks.
     */
    avtParallelContext CreateGroup(const std::vector<int> &ranks);

    /**
      Create a new parallel context where ranks are divided into groups of N
     */
    avtParallelContext CreateGroupsOfN(int N);

    /**
      Assume that we have group ids [0,nGroups], let's make a new parallel 
      context where this rank joins a specified group. If multiple ranks passed
      a groupId of 5 then those ranks would join the new communicator. Other 
      groupIds would join other communicators.
     */
    avtParallelContext Split(int groupId, int nGroups);

    //  Transplanted avtParallel.h functions that operate on
    //  the local communicator.

    void    Barrier(void);
    bool    Collect(float *, int);
    bool    Collect(double *, int);
    bool    Collect(int *, int);
    void    SumIntAcrossAllProcessors(int&);
    void    SumLongAcrossAllProcessors(long&);
    void    SumFloatAcrossAllProcessors(float&);
    void    SumDoubleAcrossAllProcessors(double&);
    void    SumFloatArrayAcrossAllProcessors(float *, float *, int);
    void    SumFloatArray(float *, float *, int);
    void    SumDoubleArrayAcrossAllProcessors(double *, double *,int);
    void    SumDoubleArray(double *, double *,int);
    void    SumDoubleArrayInPlace(double *, int);
    void    SumIntArrayAcrossAllProcessors(int *, int *, int);
    void    SumLongLongArrayAcrossAllProcessors(VISIT_LONG_LONG*, VISIT_LONG_LONG*, int);
    bool    ThisProcessorHasMinimumValue(double);
    bool    ThisProcessorHasMaximumValue(double);
    void    UnifyMinMax(double *, int, int=0);
    int     UnifyMaximumValue(int);
    float   UnifyMaximumValue(float);
    double  UnifyMaximumValue(double);
    int     UnifyMinimumValue(int);
    float   UnifyMinimumValue(float);
    double  UnifyMinimumValue(double);
    void    UnifyMaximumValue(std::vector<int>&, std::vector<int>&);
    void    UnifyMinimumFloatArrayAcrossAllProcessors(float *, float *, int);
    void    UnifyMaximumFloatArrayAcrossAllProcessors(float *, float *, int);
    void    UnifyMinimumDoubleArrayAcrossAllProcessors(double *, double *, int);
    void    UnifyMaximumDoubleArrayAcrossAllProcessors(double *, double *, int);

    void    BroadcastInt(int &i);
    void    BroadcastLongLong(VISIT_LONG_LONG &i);
    void    BroadcastIntArray(int *array, int nArray);
    void    BroadcastIntVector(std::vector<int>&, int myrank);
    void    BroadcastIntVectorFromAny(std::vector<int> &, int, int);
    void    BroadcastBool(bool &b);
    void    BroadcastBoolVector(std::vector<bool> &b, int myrank);
    void    BroadcastDouble(double &i);
    void    BroadcastDoubleArray(double *array, int nArray);
    void    BroadcastDoubleArrayFromAny(double *, int, int);
    void    BroadcastDoubleVector(std::vector<double>&, int myrank);
    void    BroadcastDoubleVectorFromAny(std::vector<double> &, int, int);
    void    BroadcastString(std::string &s, int myrank);
    void    BroadcastStringVector(std::vector<std::string>&,
                                           int myrank);
    void    BroadcastStringVectorVector(std::vector<std::vector<std::string> >&,
                                                 int myrank);
    void    BroadcastAttributes(AttributeGroup &atts);

    bool    GetListToRootProc(std::vector<std::string> &, int);

    void    CollectIntArraysOnRank(int *&, int *&, int *, int, int root);
    void    CollectIntArraysOnRootProc(int *&, int *&, int *, int);
    void    CollectDoubleArraysOnRootProc(double *&, int *&, double *, int);

    int     GetUniqueMessageTag();
    void    GetUniqueMessageTags(int *tags, int ntags);
    int     GetUniqueStaticMessageTag();

    void    GetAttToRootProc(AttributeGroup &, int);
    void    GetFloatArrayToRootProc(float *, int, bool &);
    void    GetDoubleArrayToRootProc(double *, int, bool &);
    void    WaitAll(std::vector<int> &reqs, std::vector<int> &status );
    void    WaitSome(std::vector<int> &reqs, std::vector<int> &done, std::vector<int> &status );
    void    TestSome(std::vector<int> &reqs, std::vector<int> &done, std::vector<int> &status );
    void    CancelRequest(void *req);

 
    static void Init();
private:
    class PrivateData;
    PrivateData *d;
};

#endif
