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
//                          avtPolylineCleanupFilter.C                        //
// ************************************************************************* //

#include <avtPolylineCleanupFilter.h>

#include <vtkCleanPolyData.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

// ****************************************************************************
//  Method: avtPolylineCleanupFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Cyrus Harrison
//  Creation:   Mon Nov  7 11:21:19 PST 2016
//
// ****************************************************************************

avtPolylineCleanupFilter::avtPolylineCleanupFilter() : avtDataTreeIterator()
{
}

// ****************************************************************************
//  Method: avtPolylineCleanupFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Cyrus Harrison
//  Creation:   Mon Nov  7 11:21:19 PST 2016
//
// ****************************************************************************

avtPolylineCleanupFilter::~avtPolylineCleanupFilter()
{
}

// ****************************************************************************
//  Method: avtPolylineCleanupFilter::ExecuteData
//
//  Purpose:
//    Cleans duplicate points from a poly line using the
//    vtkCleanPolyData filter.
//
//  Arguments:
//      inDR       The input data representation.
//
//  Returns:       The output data representation.
//
//  Note: The cell data copying is untested.
//
//  Programmer:  Cyrus Harrison
//  Creation:    Mon Nov  7 11:21:19 PST 2016
//
//  Modifications:
//    Kathleen Biagas, Thu Jun 20 11:17:52 PDT 2019
//    Remove unnecessary handling of active scalars, vtkCleanPolyData
//    doesn't modify them.
//
// ****************************************************************************

avtDataRepresentation *
avtPolylineCleanupFilter::ExecuteData(avtDataRepresentation *inDR)
{
    //
    // Get the VTK data set.
    //
    vtkDataSet *inDS = inDR->GetDataVTK();

    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() != 1)
    {
        return inDR;
    }

    // Clean duplicate points from the polydata.
    vtkCleanPolyData *cleanFilter = vtkCleanPolyData::New();

    cleanFilter->SetInputData(vtkPolyData::SafeDownCast(inDS));
    cleanFilter->Update();

    // Get the output.
    vtkPolyData *outPD = cleanFilter->GetOutput();
    outPD->Register(NULL);

    // Create the output data rep.
    avtDataRepresentation *outDR =
        new avtDataRepresentation(outPD, inDR->GetDomain(), inDR->GetLabel());

    return outDR;
}


// ****************************************************************************
//  Method: avtPolylineCleanupFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Indicate that this invalidates the zone numberings.
//
//  Programmer:  Cyrus Harrison
//  Creation:    Mon Nov  7 11:21:19 PST 2016
//
// ****************************************************************************

void
avtPolylineCleanupFilter::UpdateDataObjectInfo(void)
{
    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 1)
        GetOutput()->GetInfo().GetValidity().InvalidateZones();
}
