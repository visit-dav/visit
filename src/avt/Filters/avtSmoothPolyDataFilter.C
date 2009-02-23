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
//                         avtSmoothPolyDataFilter.C                         //
// ************************************************************************* //

#include <avtSmoothPolyDataFilter.h>

#include <vtkDataSet.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkPolyData.h>
#include <vtkGeometryFilter.h>

#include <avtDataset.h>

#include <DebugStream.h>
#include <VisItException.h>


using     std::string;


// ****************************************************************************
//  Method: avtSmoothPolyDataFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtSmoothPolyDataFilter::avtSmoothPolyDataFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtSmoothPolyDataFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtSmoothPolyDataFilter::~avtSmoothPolyDataFilter()
{
    ;
}


// ****************************************************************************
//  Method:  avtSmoothPolyDataFilter::SetSmoothingLevel
//
//  Purpose:
//    Set the smoothing level (0, 1, or 2)
//
//  Arguments:
//    sl         the smoothing level
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 10, 2002
//
// ****************************************************************************

void
avtSmoothPolyDataFilter::SetSmoothingLevel(int sl)
{
    smoothingLevel = sl;
}


// ****************************************************************************
//  Method: avtSmoothPolyDataFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the SmoothPolyData filter.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//      <unused>   The label.
//
//  Returns:       The output geometry.
//
//  Programmer: Jeremy Meredith
//  Creation:   December  6, 2002
//
//  Modifications:
//    Jeremy Meredith, Mon Feb 23 10:41:33 EST 2009
//    If we didn't get poly data, don't throw an error, convert it.
//    Also, do that check after our no-op checks to save time.
//
// ****************************************************************************

vtkDataSet *
avtSmoothPolyDataFilter::ExecuteData(vtkDataSet *inDS, int, string)
{
    // Detect no-ops
    if (smoothingLevel == 0 ||
        GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() != 2 ||
        GetInput()->GetInfo().GetAttributes().GetSpatialDimension()     != 3)
    {
        return inDS;
    }

    // We only work on surface data
    if (inDS->GetDataObjectType() != VTK_POLY_DATA)
    {
        vtkGeometryFilter *geom = vtkGeometryFilter::New();
        geom->SetInput(inDS);
        inDS = geom->GetOutput();
    }

    //
    // Set up and apply the filter
    //
    vtkSmoothPolyDataFilter *smoothPolyData = vtkSmoothPolyDataFilter::New();
    smoothPolyData->SetInput((vtkPolyData*)inDS);
    vtkPolyData *newDS = vtkPolyData::New();
    smoothPolyData->SetOutput(newDS);

    smoothPolyData->SetConvergence(0.0);   // always do requested # iterations
    smoothPolyData->SetFeatureAngle(60.0);
    smoothPolyData->SetEdgeAngle(25.0);
    smoothPolyData->SetFeatureEdgeSmoothing(1);
    smoothPolyData->SetBoundarySmoothing(1);

    //
    // Right now we support 3 different levels -- off, fast, and nice/high
    //
    switch (smoothingLevel)
    {
      case 0:
        smoothPolyData->SetNumberOfIterations(0);
        break;
      case 1:
        smoothPolyData->SetRelaxationFactor(1.0);
        smoothPolyData->SetNumberOfIterations(1);
        break;
      case 2:
        smoothPolyData->SetRelaxationFactor(0.05);
        smoothPolyData->SetNumberOfIterations(100);
        break;
    }

    newDS->Update();

    vtkDataSet *outDS = NULL;
    if (newDS->GetNumberOfCells() > 0)
    {
        outDS = newDS;
    }

    ManageMemory(outDS);
    newDS->Delete();
    smoothPolyData->Delete();

    return outDS;
}
