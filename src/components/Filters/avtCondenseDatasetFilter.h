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
//                        avtCondenseDatasetFilter.h                         //
// ************************************************************************* //

#ifndef AVT_CONDENSE_DATASET_FILTER_H
#define AVT_CONDENSE_DATASET_FILTER_H

#include <filters_exports.h>

#include <avtStreamer.h>


class vtkPolyDataRelevantPointsFilter;
class vtkUnstructuredGridRelevantPointsFilter;
class vtkDataSet;


// ****************************************************************************
//  Class: avtCondenseDatasetFilter
//
//  Purpose:
//      Condenses the size of the dataset by removing irrelevant points and
//      data arrays.
//  
//  Programmer: Kathleen Bonnell 
//  Creation:   November 07, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Apr 10 10:49:10 PDT 2001 
//    Changed ExecuteDomain to ExecuteData.
//
//    Hank Childs, Wed Jun  6 09:09:33 PDT 2001
//    Removed CalcDomainList and Equivalent.
//
//    Hank Childs, Fri Jul 25 21:21:08 PDT 2003
//    Renamed from avtRelevantPointsFilter.
//
//    Kathleen Bonnell, Wed Nov 12 18:26:21 PST 2003 
//    Added a flag that tells this filter to keep avt and vtk data arrays
//    around, and a method for setting the flag. 
//
//    Kathleen Bonnell, Wed Apr 14 17:51:36 PDT 2004 
//    Added a flag that tells this filter to force usage of relevant points 
//    filter (bypassHeuristic).  
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
// ****************************************************************************

class AVTFILTERS_API avtCondenseDatasetFilter : public avtStreamer
{
  public:
                         avtCondenseDatasetFilter();
    virtual             ~avtCondenseDatasetFilter();

    virtual const char  *GetType(void) { return "avtCondenseDatasetFilter"; };
    virtual const char  *GetDescription(void) 
                             { return "Removing unneeded points"; };

    virtual void         ReleaseData(void);
    void                 KeepAVTandVTK(bool val) {keepAVTandVTK = val; };
    void                 BypassHeuristic(bool val) {bypassHeuristic = val; };

  protected:
    vtkPolyDataRelevantPointsFilter         *rpfPD;
    vtkUnstructuredGridRelevantPointsFilter *rpfUG;

    virtual vtkDataSet  *ExecuteData(vtkDataSet *, int, std::string);
    virtual bool         FilterUnderstandsTransformedRectMesh();

  private:
    bool                 keepAVTandVTK;
    bool                 bypassHeuristic;
};


#endif


