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

// ************************************************************************* //
//                          avtPolylineToRibbonFilter.C                      //
// ************************************************************************* //

#include <avtPolylineToRibbonFilter.h>

#include <vtkRibbonFilter.h>
#include <vtkAppendPolyData.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

// ****************************************************************************
//  Method: avtPolylineToRibbonFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Allen Sanderson
//  Creation:   Feb 12 2016
//
// ****************************************************************************

avtPolylineToRibbonFilter::avtPolylineToRibbonFilter() : avtDataTreeIterator()
{
}

// ****************************************************************************
//  Method: avtPolylineToRibbonFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Allen Sanderson
//  Creation:   Feb 12 2016
//
// ****************************************************************************

avtPolylineToRibbonFilter::~avtPolylineToRibbonFilter()
{
}

// ****************************************************************************
//  Method: avtPolylineToRibbonFilter::ExecuteData
//
//  Purpose:
//    Creates a ribbon from a polyline
//
//  Arguments:
//      inDR       The input data representation.
//
//  Returns:       The output data representation.
//
//  Programmer: Allen Sanderson
//  Creation:   Feb 12 2016
//
//  Modifications:
//    Eric Brugger, Thu Oct 20 14:51:51 PDT 2016
//    I added code to remove duplicate points from the lines since the 
//    vtkRibbonFilter exits on any lines that have duplicate points.
//
// ****************************************************************************

avtDataRepresentation *
avtPolylineToRibbonFilter::ExecuteData(avtDataRepresentation *inDR)
{
    //
    // Get the VTK data set.
    //
    vtkDataSet *inDS = inDR->GetDataVTK();

    if (inDS->GetDataObjectType() != VTK_POLY_DATA)
    {
        // We only work on line data
        EXCEPTION1(VisItException, "avtPolylineToRibbonFilter::ExecuteDataTree "
                                   "-- Did not get polydata");
    }

    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() != 1)
    {
        return inDR;
    }

    vtkDataArray *activeScalars = inDS->GetPointData()->GetScalars();

    vtkPolyData *data = vtkPolyData::SafeDownCast(inDS);

    // Create the ribbon polydata.
    vtkRibbonFilter *ribbonFilter = vtkRibbonFilter::New();

    ribbonFilter->SetWidth( width );
    ribbonFilter->ReleaseDataFlagOn();
    ribbonFilter->SetVaryWidth( varyWidth );
    ribbonFilter->ReleaseDataFlagOn();

    if (varyWidth && widthVar != "" && widthVar != "\0")
    {
        if (widthVar != "default")
            data->GetPointData()->SetActiveScalars(widthVar.c_str());
    }

    ribbonFilter->SetInputData(data);

    ribbonFilter->Update();

    // Append the original data and ribbon polydata
    vtkAppendPolyData *append = vtkAppendPolyData::New();

    append->AddInputData(data);
    append->AddInputData(ribbonFilter->GetOutput());
    
    ribbonFilter->Delete();

    append->Update();

    // Get the output.
    vtkPolyData *outPD = append->GetOutput();
    outPD->Register(NULL);
    append->Delete();
    
    // Remove the lines.
    outPD->SetLines(NULL);
    outPD->RemoveDeletedCells();

    // Restore the active scalars.
    if (activeScalars)
    {
        data->GetPointData()->SetActiveScalars(activeScalars->GetName());
        outPD->GetPointData()->SetActiveScalars(activeScalars->GetName());
    }

    // Create the output data rep.
    avtDataRepresentation *outDR =
        new avtDataRepresentation( outPD, inDR->GetDomain(), inDR->GetLabel() );

    return outDR;
}


// ****************************************************************************
//  Method: avtPolylineToRibbonFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Indicate that this invalidates the zone numberings.
//
//  Programmer: Allen Sanderson
//  Creation:   Feb 12 2016
//
// ****************************************************************************

void
avtPolylineToRibbonFilter::UpdateDataObjectInfo(void)
{
    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 1)
        GetOutput()->GetInfo().GetValidity().InvalidateZones();
}
