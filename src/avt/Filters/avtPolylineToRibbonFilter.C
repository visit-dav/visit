/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkDataSet.h>
#include <vtkPolyData.h>
#include <vtkRibbonFilter.h>
#include <vtkAppendPolyData.h>
#include <avtDataset.h>

#include <DebugStream.h>
#include <VisItException.h>
#include <TimingsManager.h>

#include <set>
#include <deque>

// ****************************************************************************
//  Method: avtPolylineToRibbonFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 27 11:53:59 PDT 2009
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
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 27 11:53:59 PDT 2009
//
// ****************************************************************************

avtPolylineToRibbonFilter::~avtPolylineToRibbonFilter()
{
}

// ****************************************************************************
//  Method: avtPolylineToRibbonFilter::ExecuteData
//
//  Purpose:
//      Groups connected line cells into polyline cells.
//
//  Arguments:
//      inDR       The input data representation.
//
//  Returns:       The output data representation.
//
//  Note: The cell data copying is untested.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 27 11:53:59 PDT 2009
//
//  Modifications:
//
//    Tom Fogal, Mon Apr 26 17:27:44 MDT 2010
//    Break out of a loop to prevent incrementing a singular iterator.
//    Use `empty' instead of 'size'.
//
//    Eric Brugger, Mon Jul 21 13:51:51 PDT 2014
//    Modified the class to work with avtDataRepresentation.
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

    vtkPolyData *data = vtkPolyData::SafeDownCast( inDS );

    double width;

    if( widthSizeType == 0 )
      width = widthAbsolute;
    else
      width = widthBBox * boundingBoxSize;
    

    vtkRibbonFilter *ribbonFilter = vtkRibbonFilter::New();

    ribbonFilter->SetWidth( width );
    ribbonFilter->ReleaseDataFlagOn();
    
    if( widthVarEnabled && widthVar != "" && widthVar != "\0" )
    {
      if (widthVar != "default")
        data->GetPointData()->SetActiveScalars( widthVar.c_str() );
        
      ribbonFilter->SetVaryWidth( true );
      ribbonFilter->SetWidthFactor( widthVarFactor );
    }
    else
      ribbonFilter->SetVaryWidth( false );

    // If we need to trim either end, create a new trimmed polyline
    // and run the ribbon on this geometry.
    // if( atts.GetDisplayBeginFlag() || atts.GetDisplayEndFlag())
    // {
    //     vtkCellArray *lines = data->GetLines();
    //     vtkIdType *segments = lines->GetPointer();
    
    //     vtkIdType *segptr = segments;
    //     vtkAppendPolyData *append = vtkAppendPolyData::New();
        
    //     for (int i=0; i<data->GetNumberOfLines(); ++i)
    //     {
    //         vtkPolyData *pd = MakeNewPolyline(data, segptr);

    //         if (pd != NULL)
    //         {
    //             append->AddInputData(pd);
    //             pd->Delete();
    //         }
    //     }
    //     ribbonFilter->SetInputConnection(append->GetOutputPort());
    //     append->Delete();
    // }
    // else
      ribbonFilter->SetInputData(data);

    // Create the ribbon polydata.
    ribbonFilter->Update();

    vtkDataSet *outDS = ribbonFilter->GetOutput();

    // Restore the active scalars.
    if( activeScalars )
    {
      data->GetPointData()->SetActiveScalars(activeScalars->GetName());
      outDS->GetPointData()->SetActiveScalars(activeScalars->GetName());
    }
    
    // Crearte the output data rep.
    avtDataRepresentation *outDR =
      new avtDataRepresentation( outDS, inDR->GetDomain(), inDR->GetLabel() );

    ribbonFilter->Delete();

    return outDR;
}


// ****************************************************************************
//  Method: avtPolylineToRibbonFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Indicate that this invalidates the zone numberings.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Sep  3 11:45:19 PDT 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtPolylineToRibbonFilter::UpdateDataObjectInfo(void)
{
    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 1)
        GetOutput()->GetInfo().GetValidity().InvalidateZones();
}
