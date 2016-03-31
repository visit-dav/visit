/*****************************************************************************
*
* Copyright (c) 2000 - 2016, Lawrence Livermore National Security, LLC
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
//                          avtPolylineAddEndPointsFilter.C                  //
// ************************************************************************* //

#include <avtPolylineAddEndPointsFilter.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkDataSet.h>
#include <vtkPolyData.h>
#include <vtkConeSource.h>
#include <vtkSphereSource.h>
#include <vtkAppendPolyData.h>
#include <avtDataset.h>

#include <DebugStream.h>
#include <VisItException.h>
#include <TimingsManager.h>

#include <set>
#include <deque>

// ****************************************************************************
//  Method: avtPolylineAddEndPointsFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 27 11:53:59 PDT 2009
//
// ****************************************************************************

avtPolylineAddEndPointsFilter::avtPolylineAddEndPointsFilter() : avtDataTreeIterator()
{
}

// ****************************************************************************
//  Method: avtPolylineAddEndPointsFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 27 11:53:59 PDT 2009
//
// ****************************************************************************

avtPolylineAddEndPointsFilter::~avtPolylineAddEndPointsFilter()
{
}

// ****************************************************************************
//  Method: avtPolylineAddEndPointsFilter::ExecuteData
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
avtPolylineAddEndPointsFilter::ExecuteData(avtDataRepresentation *inDR)
{
    //
    // Get the VTK data set.
    //
    vtkDataSet *inDS = inDR->GetDataVTK();

    if (inDS->GetDataObjectType() != VTK_POLY_DATA)
    {
        // We only work on line data
        EXCEPTION1(VisItException, "avtPolylineAddEndPointsFilter::ExecuteDataTree "
                                   "-- Did not get polydata");
    }

    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() != 1)
    {
        return inDR;
    }

    vtkAppendPolyData *append = vtkAppendPolyData::New();

    vtkDataArray *activeScalars = inDS->GetPointData()->GetScalars();
    vtkDataArray *activeRadius;

    double range[2], scale;

    if( varyRadius && radiusVar != "" && radiusVar != "\0" )
    {
      if (radiusVar != "default" && radiusVar != activeScalars->GetName() )
        activeRadius = inDS->GetPointData()->GetArray( radiusVar.c_str() );
      else
        activeRadius = activeScalars;

      activeRadius->GetRange( range, 0 );

      if( (range[1] - range[0]) == 0.0 )
        range[1] = range[0] + 1.0;

      scale = (radiusFactor-1) / (range[1]-range[0]);
    }

    vtkPolyData *data   = vtkPolyData::SafeDownCast( inDS );
    vtkCellArray *lines = data->GetLines();
    vtkPoints *points   = data->GetPoints();
   
    vtkIdType numPts;
    vtkIdType *ptIndexs;

    append->AddInputData( data );

    while( lines->GetNextCell( numPts, ptIndexs ) )
    {
      vtkPolyData *outPD = NULL;

      double p0[3], p1[3];

      avtVector vec;

      for( int i=Heads; i<=Tails; ++i )
      {
        if( type == i || type == Both )
        {
          int tip, tail;
        
          if( i == Heads )
          {
            tip  = numPts - 1;
            tail = numPts - 2;
          }
          else if( i == Tails )
          {
            tip  = 0;
            tail = 1;
          }
          
          points->GetPoint( ptIndexs[tip], p0 );

          double scaledRadius = radius;
          
          if( varyRadius && radiusVar != "" && radiusVar != "\0" )
          {
            scaledRadius *=
              (1.0 + (activeRadius->GetComponent( ptIndexs[tip], 0 ) -
                      range[0]) * scale);
          }

          if( style == Spheres )
          {
            vtkSphereSource *sphere = vtkSphereSource::New();
            
            sphere->SetRadius( scaledRadius );
            sphere->SetPhiResolution( resolution );
            sphere->SetThetaResolution( resolution );
            sphere->SetCenter( p0 );
            sphere->Update();
            
            outPD = sphere->GetOutput();
          }
          else if( style == Cones )
          {
            points->GetPoint( ptIndexs[tail], p1 );
            
            vec = avtVector( p0[0]-p1[0], p0[1]-p1[1],  p0[2]-p1[2] );
            vec.normalize();
            
            p0[0] += (vec * scaledRadius * ratio / 2.0).x;
            p0[1] += (vec * scaledRadius * ratio / 2.0).y;
            p0[2] += (vec * scaledRadius * ratio / 2.0).z;
            
            vtkConeSource *cone = vtkConeSource::New();
            
            cone->SetRadius( scaledRadius );
            cone->SetHeight( scaledRadius * ratio );
            cone->SetResolution( resolution );
            cone->SetCenter( p0 );
            cone->SetDirection( vec.x, vec.y, vec.z );
            cone->CappingOn();
            cone->Update();
            
            outPD = cone->GetOutput();
          }
          
          int npts = outPD->GetPoints()->GetNumberOfPoints();

          // Copy over all of the point data from the lines to the
          // cones so the append filter will append correctly.
          int nArrays = inDS->GetPointData()->GetNumberOfArrays();

          for( int j=0; j<nArrays; ++j )
          {
            vtkDataArray *array = inDS->GetPointData()->GetArray(j);

            vtkDoubleArray *scalars = vtkDoubleArray::New();
            scalars->Allocate(npts);
            scalars->SetName(array->GetName());

            if( array->GetName() == activeScalars->GetName() )
            {
              outPD->GetPointData()->SetScalars(scalars);
              outPD->GetPointData()->SetActiveScalars(scalars->GetName());
            }
            else
              outPD->GetPointData()->AddArray(scalars);
              
            double scalar = array->GetComponent( ptIndexs[tip], 0 );
        
            for( int k=0; k<npts; ++k )
              scalars->InsertTuple1(k, scalar);

            scalars->Delete();
          }

          append->AddInputData( outPD );
          
          outPD->Delete();
        }
      }
    }      
    
    // Update.
    append->Update();
    vtkPolyData *outPD = append->GetOutput();
    outPD->Register(NULL);
    append->Delete();
   
    // Restore the active scalars.
    if( activeScalars )
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
//  Method: avtPolylineAddEndPointsFilter::UpdateDataObjectInfo
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
avtPolylineAddEndPointsFilter::UpdateDataObjectInfo(void)
{
    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 1)
        GetOutput()->GetInfo().GetValidity().InvalidateZones();
}
