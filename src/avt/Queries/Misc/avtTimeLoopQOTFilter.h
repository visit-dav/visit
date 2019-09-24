/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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

// ************************************************************************* //
//                       avtTimeLoopQOTFilter.h                            //
// ************************************************************************* //

#ifndef AVT_TIME_LOOP_QOT_FILTER_H
#define AVT_TIME_LOOP_QOT_FILTER_H

#include <query_exports.h>

#include <avtQueryOverTimeFilter.h>

#include <avtDatasetToDatasetFilter.h>
#include <avtTimeLoopFilter.h>

#include <QueryOverTimeAttributes.h>
#include <SILRestrictionAttributes.h>

#include <string>

class vtkRectilinearGrid;

// ****************************************************************************
//  Class: avtTimeLoopQOTFilter
//
//  Purpose:
//    Performs a query over time. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 19, 2004
//
//  Modifications:
//    Brad Whitlock, Wed Apr 14 14:56:45 PST 2004
//    Fixed for Windows.
//
//    Kathleen Bonnell, Tue May  4 14:21:37 PDT 2004
//    Removed SilUseSet in favor of SILRestrictionAttributes. 
//
//    Kathleen Bonnell, Thu Jan  6 11:12:35 PST 2005 
//    Added inheritance from avtTimeLoopFilter, which handles the stepping
//    through time.  Removed PostExecute method.  Added CreateFinalOutput,
//    ExecutionSuccessful (required by new inheritance).  Added qRes, times
//    (used to be stored by avtDataSetQuery::PerformQueryInTime).  Added
//    sucess and finalOutputCreated.
//
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005
//    Added CreatePolys method, and members useTimeForXAxis, nResultsToStore. 
//
//    Kathleen Bonnell, Thu Jul 27 17:43:38 PDT 2006 
//    Curves now represented as 1D RectilinearGrid, Renamed CreatedPolys to
//    CreateRGRid.
//
//    Hank Childs, Thu Feb  8 09:52:25 PST 2007
//    Try to correctly quote how many additional filters there will be
//    for one time step.
//
//    Kathleen Bonnell, Wed Nov 28 16:33:22 PST 2007 
//    Added member 'label', to store a shorter y-axis label than the query
//    name if desired. 
//
//    Kathleen Bonnell, Tue Jul  8 15:48:11 PDT 2008
//    Add useVarForYAxis.
//
//    Hank Childs, Fri Dec 24 17:59:02 PST 2010
//    Add method FilterSupportsTimeParallelization.
//
//    Kathleen Bonnell, Thu Feb 17 09:50:28 PST 2011
//    Replace CreateRGrid with CreateTree, to allow multiple outputs.
//
//    Alister Maguire, Wed May 23 09:21:45 PDT 2018
//    Added cacheIdx and useCache to allow plotting pick curves
//    from cached picks. 
//
//    Alister Maguire, Mon Sep 23 11:13:48 MST 2019
//    Changed name from avtQueryOverTimeFilter to avtTimeLoopQOTFilter, 
//    and added avtQueryOverTimeFilter inheritance. 
//
// ****************************************************************************

class QUERY_API avtTimeLoopQOTFilter : public avtTimeLoopFilter,
                                       public avtQueryOverTimeFilter
{
  public:
                          avtTimeLoopQOTFilter(const AttributeGroup*);
    virtual              ~avtTimeLoopQOTFilter();

    static avtFilter     *Create(const AttributeGroup*);

    virtual const char   *GetType(void)  { return "avtTimeLoopQOTFilter"; };
    virtual const char   *GetDescription(void) { return "Querying over Time"; };

    virtual bool          FilterSupportsTimeParallelization(void);

  protected:
    doubleVector          qRes;
    doubleVector          times;
    bool                  success;
    bool                  finalOutputCreated;
    std::string           label;

    bool                  useTimeForXAxis;
    bool                  useVarForYAxis;
    int                   nResultsToStore;
    int                   numAdditionalFilters;
    int                   cacheIdx;
    bool                  useCache;

    virtual void          Execute(void);
    virtual void          UpdateDataObjectInfo(void);

    virtual int           AdditionalPipelineFilters(void) 
                                            { return numAdditionalFilters; };

    virtual void          CreateFinalOutput(void);
    virtual bool          ExecutionSuccessful(void) { return success; } ;
    avtDataTree_p         CreateTree(const doubleVector &, 
                                     const doubleVector &,
                                     stringVector &,
                                     const bool);
};


#endif


