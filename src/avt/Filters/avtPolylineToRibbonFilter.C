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
//                          avtPolylineToRibbonFilter.C                      //
// ************************************************************************* //

#include <avtPolylineToRibbonFilter.h>

#include <vtkRibbonFilter.h>
#include <vtkAppendPolyData.h>
#include <vtkCellData.h>
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
//    Kathleen Biagas, Thu Jun 20 11:37:25 PDT 2019
//    1) Handle cell-centered data.
//    2) Only use the append filter if input contained more than just lines,
//       and remove the lines before adding to append.
//    3) Change how widthVar is sent to the vtk filter to avoid mucking with
//       ActiveScalars.
//    4) Removed CleanFilter as the input has already been cleaned before this
//       filter is called.
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

    vtkPolyData *data = vtkPolyData::SafeDownCast(inDS);

    // Create the ribbon polydata.
    vtkRibbonFilter *ribbonFilter = vtkRibbonFilter::New();
    ribbonFilter->SetInputData(data);
    ribbonFilter->SetWidth( width );
    ribbonFilter->ReleaseDataFlagOn();
    ribbonFilter->SetVaryWidth( varyWidth );
    ribbonFilter->ReleaseDataFlagOn();

    if (varyWidth && widthVar != "" && widthVar != "\0")
    {
        if (widthVar != "default")
        {
            int fieldAssociation = vtkDataObject::FIELD_ASSOCIATION_POINTS;
            if (data->GetCellData()->HasArray(widthVar.c_str()))
            {
                fieldAssociation = vtkDataObject::FIELD_ASSOCIATION_CELLS;
            }
            ribbonFilter->SetInputArrayToProcess(0, 0,0, fieldAssociation,
                                                 widthVar.c_str());
        }
    }
    ribbonFilter->Update();
    vtkPolyData *outPD = NULL;

    // Append the original data and ribbon polydata if needed
    if (data->GetNumberOfLines() < data->GetNumberOfCells())
    {
        // seems to work better removing the lines before the append
        // especially if avtPolylineAddEndPointsFilter has been used
        // prior to this filter. The cell-data arrays get mixed up
        // when lines are still in the data.
        vtkPolyData *noLines = data->NewInstance();
        noLines->ShallowCopy(data);
        noLines->SetLines(NULL);
        noLines->RemoveDeletedCells();

        vtkAppendPolyData *append = vtkAppendPolyData::New();
        append->AddInputData(noLines);
        append->AddInputData(ribbonFilter->GetOutput());
        append->Update();

        outPD = append->GetOutput();
        outPD->Register(NULL);
        append->Delete();
        noLines->Delete();
    }
    else
    {
        outPD = ribbonFilter->GetOutput();
        outPD->Register(NULL);
    }

    ribbonFilter->Delete();


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
