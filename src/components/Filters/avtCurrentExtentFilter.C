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
//                         avtCurrentExtentFilter.h                          // 
// ************************************************************************* // 


#include <avtCurrentExtentFilter.h>

#include <avtDataAttributes.h>
#include <avtDatasetExaminer.h>
#include <avtExtents.h>


// ****************************************************************************
//  Method: avtCurrentExtentFilter::Execute
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 2, 2001 
//
// ****************************************************************************

void
avtCurrentExtentFilter::Execute(void)
{
    SetOutputDataTree(GetInputDataTree());
}


// ****************************************************************************
//  Method: avtCurrentExtentFilter::UpdateDataObjectInfo
//
//  Purpose:  Retrieves the actual data/spatial extents from the
//            input and stores them in output's info.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 3, 2001 
//
//  Modifications:
//
//    Hank Childs, Fri Mar 15 18:11:12 PST 2002
//    Account for avtDatasetExaminer.
//
//    Hank Childs, Thu Jul 17 17:40:24 PDT 2003
//    Treat 2D vectors as 3D since VTK will assume that vectors are 3D.
//
//    Hank Childs, Tue Feb 24 14:23:03 PST 2004
//    Account for multiple variables.
//
//    Kathleen Bonnell, Thu Mar 11 11:16:17 PST 2004 
//    DataExtents now always have only 2 components. 
//
// ****************************************************************************

void
avtCurrentExtentFilter::UpdateDataObjectInfo(void)
{
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    avtDataset_p ds = GetTypedInput();

    int nVars = atts.GetNumberOfVariables();
    double de[2];
    for (int i = 0 ; i < nVars ; i++)
    {
        const char *vname = atts.GetVariableName(i).c_str();
    
        bool foundDE = avtDatasetExaminer::GetDataExtents(ds, de, vname);
        if (foundDE)
        {
            outAtts.GetCumulativeCurrentDataExtents(vname)->Merge(de);
        }
    }

    double se[6];
    bool foundSE = avtDatasetExaminer::GetSpatialExtents(ds, se);
    if (foundSE)
    {
        outAtts.GetCumulativeCurrentSpatialExtents()->Merge(se);
    }
}

// ****************************************************************************
//  Method:  avtCurrentExtentFilter::FilterUnderstandsTransformedRectMesh
//
//  Purpose:
//    If this filter returns true, this means that it correctly deals
//    with rectilinear grids having an implied transform set in the
//    data attributes.  It can do this conditionally if desired.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 15, 2007
//
// ****************************************************************************
bool
avtCurrentExtentFilter::FilterUnderstandsTransformedRectMesh()
{
    // there were some changes made at lower levels which make
    // this filter safe
    return true;
}
