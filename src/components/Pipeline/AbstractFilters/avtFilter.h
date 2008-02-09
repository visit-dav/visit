/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

// ************************************************************************* //
//                               avtFilter.h                                 //
// ************************************************************************* //

#ifndef AVT_FILTER_H
#define AVT_FILTER_H

#include <pipeline_exports.h>

#include <vector>

#include <avtDataObjectSource.h>
#include <avtDataObjectSink.h>


class     AttributeGroup;

class     avtDynamicAttribute;
class     avtMetaData;
class     avtWebpage;


// ****************************************************************************
//  Method: avtFilter
//
//  Purpose:
//      A filter is an object that does not originate or terminate a pipeline.
//      It defines what it looks like to propagate an Update to an upstream
//      filter and also incorporates the idea of "Executes", which are
//      defined by all derived types of filters.
// 
//  Programmer: Hank Childs
//  Creation:   May 30, 2001
//
//  Modifications:
//  
//    Kathleen Bonnell, Wed Oct  3 08:53:21 PDT 2001
//    Added TryCurrent and GetCurrent Data/Spatial Extents.
//
//    Hank Childs, Wed Oct 24 14:21:18 PDT 2001
//    Added PreExecute and PostExecute from avtSIMODataTreeIterator.
//
//    Hank Childs, Fri Mar 15 15:25:33 PST 2002 
//    Add support for attributes.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
//    Hank Childs, Fri Dec  3 14:22:42 PST 2004
//    Add variable name argument to SearchDataForDataExtents.
//
//    Hank Childs, Tue Jun  7 14:55:40 PDT 2005
//    Add friend status to the facade filter.
//
//    Hank Childs, Thu Dec 21 09:17:43 PST 2006
//    Add support for debug dumps.
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//    All filters can now report that they understand these grids, though
//    the virtual default in this base class is that each does not.
//
//    Hank Childs, Fri Jun 15 11:37:59 PDT 2007
//    Added a static data member to keep track of the number of filters
//    currently executing (which also informs the amount of sub-pipelines
//    that are executing, which allows for formatting the debug dump).

// ****************************************************************************

class PIPELINE_API avtFilter
    : virtual public avtDataObjectSource, virtual public avtDataObjectSink
{
    friend class                        avtFacadeFilter;

  public:
                                        avtFilter();
    virtual                            ~avtFilter();

    virtual const char                 *GetType(void) = 0;
    virtual const char                 *GetDescription(void) { return NULL; };

    virtual bool                        Equivalent(const AttributeGroup *)
                                            { return false; };

    virtual bool                        Update(avtContract_p);

    virtual avtOriginatingSource       *GetOriginatingSource(void);
    virtual avtQueryableSource         *GetQueryableSource(void);
    avtContract_p          GetGeneralContract(void);
    virtual void                        ReleaseData(void);

    static void                         DebugDump(bool d) {debugDump = d;}

  protected:
    bool                                modified;
    bool                                inExecute;

    static bool                         debugDump;
    static int                          numInExecute;
    avtWebpage                         *webpage;
    std::vector<avtDynamicAttribute *>  dynamicAttributes;

    virtual void                        Execute(void) = 0;
    avtContract_p          ModifyContractAndDoBookkeeping(
                                               avtContract_p);
    virtual avtContract_p  ModifyContract(
                                               avtContract_p);

    virtual void                        ChangedInput(void);
    virtual void                        InitializeFilter(void);
    virtual void                        VerifyInput(void);
    virtual int                         AdditionalPipelineFilters(void);

    void                                PassOnDataObjectInfo(void);
    virtual void                        UpdateDataObjectInfo(void);

    virtual bool                        FilterUnderstandsTransformedRectMesh();

    virtual void                        PreExecute(void);
    virtual void                        PostExecute(void);
    virtual void                        ExamineContract(
                                                   avtContract_p);

    avtMetaData                        *GetMetaData(void);

    void                                UpdateProgress(int, int);

    bool                                TryDataExtents(double *,
                                                       const char * = NULL);
    void                                GetDataExtents(double *,
                                                       const char * = NULL);
    bool                                TrySpatialExtents(double *);
    void                                GetSpatialExtents(double *);
    bool                                TryCurrentDataExtents(double *);
    void                                GetCurrentDataExtents(double *);
    bool                                TryCurrentSpatialExtents(double *);
    void                                GetCurrentSpatialExtents(double *);
    virtual void                        SearchDataForDataExtents(double *,
                                                                 const char *);

    void                                RegisterDynamicAttribute(
                                                        avtDynamicAttribute *);
    void                                ResolveDynamicAttributes(void);

    void                                DumpDataObject(avtDataObject_p,
                                                      const char *);
    void                                DumpContract(avtContract_p,
                                                     const char *);
    void                                InitializeWebpage(void);
    void                                FinalizeWebpage(void);
};


#endif


