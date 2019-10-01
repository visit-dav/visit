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
//                       avtDirectDatabaseQOTFilter.h                            //
// ************************************************************************* //

#ifndef AVT_DIRECT_DATABASE_QOT_FILTER_H
#define AVT_DIRECT_DATABASE_QOT_FILTER_H

#include <query_exports.h>

#include <avtDatasetToDatasetFilter.h>

#include <QueryOverTimeAttributes.h>
#include <SILRestrictionAttributes.h>

#include <avtQueryOverTimeFilter.h>

#include <string>

class vtkRectilinearGrid;
class vtkPolyData;

// ****************************************************************************
//  Class: avtDirectDatabaseQOTFilter
//
//  Purpose:
//      Perform a query over time by communciating directly with the database
//      readers.
//
//      Using this filter requires that the avtDataRequest be updated to 
//      request a query over time dataset (QOTDataset), which can be 
//      accomplished by setting the retrieveQOTDataset within avtDataRequest.
//
//      Using this filter is much faster than using the TimeLoopQOTFilter,
//      but there are some significant drawbacks that must be noted:
//
//          1. This filter can only retrieve "original" data, meaning that
//             the query will be performed before any other plots and filters
//             are applied (excluding the expression filter). 
//          2. The QOTDataset is limited in its ability to process expressions.
//
//      When using "actual" data or complex expressions, the TimeLoopQOTFilter
//      should be relied on. 
//
//  Programmer: Alister Maguire
//  Creation:   Tue Sep 24 13:46:56 MST 2019  
//
//  Modifications:
//
// ****************************************************************************

class QUERY_API avtDirectDatabaseQOTFilter : public avtQueryOverTimeFilter
{
  public:
                             avtDirectDatabaseQOTFilter(const AttributeGroup*);
    virtual                 ~avtDirectDatabaseQOTFilter();

    static avtFilter        *Create(const AttributeGroup*);

    virtual const char      *GetType(void)  
                               { return "avtDirectDatabaseQOTFilter"; };
    virtual const char      *GetDescription(void) 
                               { return "Querying over Time"; };

  protected:
    bool                     success;
    bool                     finalOutputCreated; 
    bool                     useTimeForXAxis;
    bool                     useVarForYAxis;
    std::string              YLabel;

    virtual void             Execute(void);
    virtual void             UpdateDataObjectInfo(void);

    virtual bool             ExecutionSuccessful(void) 
                               { return success; };

    vtkPolyData             *VerifyAndRefineTimesteps(vtkPolyData *);

    avtDataTree_p            ConstructCurveTree(vtkPolyData *,
                                                const bool);
};

#endif
