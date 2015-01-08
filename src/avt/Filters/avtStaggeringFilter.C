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
//                           avtStaggeringFilter.C                          //
// ************************************************************************* //

#include <avtStaggeringFilter.h>
#include <avtVariableCache.h>

#include <vtkDataSet.h>
#include <vtkInformation.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <DebugStream.h>
#include <vtkStructuredGrid.h>
#include <string>

// ****************************************************************************
//  Method: avtStaggeringFilter constructor
//
//  Programmer: Marc Durant
//  Creation:   April 24, 2012
//
//  Modifications:
//
// ****************************************************************************

avtStaggeringFilter::avtStaggeringFilter()
{
}

// ****************************************************************************
//  Method: avtStaggeringFilter destructor
//
//  Programmer: Marc Durant
//  Creation:   April 24, 2012
//
//  Modifications:
//
// ****************************************************************************

avtStaggeringFilter::~avtStaggeringFilter()
{
}

// ****************************************************************************
//  Method: avtStaggeringFilter::ExecuteData
//
//  Purpose:
//      Examines input dataset for node offset information.  If found,
//      creates a copy of the mesh and alters positions to match node
//      offsets.  If no offsets, or offsets are zero, returns input dataset.
//
//  Arguments:
//      inDR      The input data representation.
//
//  Returns:      The input data representation adjusted for node offsets
//
//  Programmer:   Marc Durant
//  Creation:     April 24, 2012
//
//  Modifications:
//    Eric Brugger, Tue Jul 22 08:56:41 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

avtDataRepresentation *
avtStaggeringFilter::ExecuteData(avtDataRepresentation *inDR)
{
    //
    // Get the VTK data set.
    //
    vtkDataSet *inDS = inDR->GetDataVTK();
  
  if (!inDS) {
    return inDR;
  }
  
  vtkDataSet* outDS = NULL;
  
  // Check for offset information attached to the data
  // For a Pseudocolor plot, we only need to consult the SCALARS.
  // In addition, we have no concept of offsets for cell-centered data
  // so we only need to check the Point data.
  vtkDataArray* velData = inDS->GetPointData()->GetScalars();
  
  if (!velData) {
    debug5 <<"avtStaggeringFilter::ExecuteData() - Unable to find scalar data on mesh, returning input dataset." <<std::endl;
    return inDR;
  }
    
  vtkInformation* info = velData->GetInformation();
      
  if (!info->Has(avtVariableCache::OFFSET_3())) {
    debug5 <<"avtStaggeringFilter::ExecuteData() - scalars have NO offset information, returning input dataset." <<std::endl;
    return inDR;
  }
      
  //Extract the offset information from the dataset
  double* nodeOffset = info->Get(avtVariableCache::OFFSET_3());
  debug5 <<"avtStaggeringFilter::ExecuteData() - offset is " <<nodeOffset[0] <<", " <<nodeOffset[1] <<", " <<nodeOffset[2] <<std::endl;
      
  int dstype = inDS->GetDataObjectType();
  switch (dstype)
  {
    case VTK_RECTILINEAR_GRID:
      //rv = ReflectRectilinear((vtkRectilinearGrid *) ds, dim);
      debug5 <<"avtStaggeringFilter::ExecuteData() - This is a RECTILINEAR grid, which is not currently implemented." <<std::endl;
      break;
    case VTK_STRUCTURED_GRID:
    {
      vtkStructuredGrid* grid = vtkStructuredGrid::SafeDownCast(inDS);
      
      vtkStructuredGrid* newGrid = vtkStructuredGrid::New();
      newGrid->ShallowCopy(grid);
        
      vtkPoints* oldPoints = grid->GetPoints();
      vtkIdType numPoints = oldPoints->GetNumberOfPoints();
      vtkPoints* newPoints = vtkPoints::New();
      newPoints->SetNumberOfPoints(numPoints);
      newGrid->SetPoints(newPoints);
      
      int* dims = newGrid->GetDimensions();
      int mulSizes[3];
      mulSizes[0] = 1;
      mulSizes[1] = dims[0];
      mulSizes[2] = dims[0] * dims[1];
        
      int index[3];
      //Scan through all points and adjust location
      //NOTE - this only works with POSITIVE OFFSETS
      for (vtkIdType k = 0; k < numPoints; k++) {
        // Set the base coordinates
        for (size_t j = 0; j < 3; j++) {
          index[j] = (k / mulSizes[j]) % dims[j];
        }
            
        double originalXYZ[3];
        oldPoints->GetPoint(k, &originalXYZ[0]);
          
        double newXYZ[3];
        newXYZ[0] = originalXYZ[0];
        newXYZ[1] = originalXYZ[1];
        newXYZ[2] = originalXYZ[2];
        // Apply node offsets
        for (size_t j1 = 0; j1 < 3; j1++) {
          //debug5 <<"j1 = " <<j1 <<std::endl;
          //debug5 <<"nodeOffset[j1] = " <<nodeOffset[j1] <<std::endl;
          // Must be inside domain, no offset if hitting the boundary
          if (index[j1] < dims[j1] - 1) {
            int kPlus = k + mulSizes[j1];
            double otherXYZ[3];
            oldPoints->GetPoint(kPlus, &otherXYZ[0]);
            //debug5 <<"avtStaggeringFilter::ExecuteData() - neighbor point point(#" <<kPlus <<") = (" <<otherXYZ[0] <<", " <<otherXYZ[1] <<", " <<otherXYZ[2] <<")" <<std::endl;
            for (size_t j2 = 0; j2 < 3; j2++) {
              //debug5 <<"avtStaggeringFilter::ExecuteData() - otherXYZ[" <<j2 <<"] = " <<otherXYZ[j2] <<std::endl;
              //debug5 <<"avtStaggeringFilter::ExecuteData() - xyz[" <<j2 <<"] = " <<xyz[j2] <<std::endl;
              double difference = otherXYZ[j2] - originalXYZ[j2];
              double displacement = difference * nodeOffset[j1];
              //debug5 <<"avtStaggeringFilter::ExecuteData() - j2 = " <<j2 <<" difference = " <<difference <<"  displacement = " <<displacement <<std::endl;
              newXYZ[j2] += displacement;
              //debug5 <<"avtStaggeringFilter::ExecuteData() - newXYZ = " <<newXYZ[j2] <<std::endl;
            }
          }
        }
            
        newPoints->SetPoint(k, newXYZ[0], newXYZ[1], newXYZ[2]);
      }
          
      outDS = newGrid;
    }
    break;
          
    case VTK_UNSTRUCTURED_GRID:
      debug5 <<"avtStaggeringFilter::ExecuteData() -This is an UNSTRUCTURED grid, not currently implemented." <<std::endl;
      break;
    case VTK_POLY_DATA:
      debug5 <<"avtStaggeringFilter::ExecuteData() -This is a POLY_DATA grid, not currently implemented." <<std::endl;
      //rv = ReflectPointSet((vtkPointSet *) ds, dim);
      break;
    default:
      debug1 << "avtStaggeringFilter::ExecuteData() - Unknown data type = " << dstype << endl;
      break;
  }  //end switch(dstype)
      
  if (outDS) {
    avtDataRepresentation *outDR = new avtDataRepresentation(outDS,
        inDR->GetDomain(), inDR->GetLabel());

    outDS->Delete();

    return outDR;
  }
    
  return inDR;
}  

// ****************************************************************************
//  Method: avtStaggeringFilter::UpdateDataObjectInfo
//
//  Purpose:  Sets flags in the pipeline.
//
//  Programmer: Marc Durant
//  Creation:   April 24, 2012 
//
//  Modifications:
//
// ****************************************************************************

void
avtStaggeringFilter::UpdateDataObjectInfo(void)
{
//    GetOutput()->GetInfo().GetAttributes().SetTopologicalDimension(0);
//    GetOutput()->GetInfo().GetAttributes().SetKeepNodeZoneArrays(keepNodeZone);
}


// ****************************************************************************
//  Method: avtStaggeringFilter::ModifyContract
//
//  Purpose:  Does nothing
// 
//  Programmer: Marc Durant 
//  Creation:   April 24, 2012 
//
//  Modifications:
//
// ****************************************************************************

avtContract_p
avtStaggeringFilter::ModifyContract(avtContract_p contract)
{
  return contract;
}

// ****************************************************************************
// Method: avtStaggeringFilter::PostExecute
//
// Purpose:
//   Override the original data extents since we want the filters downstream
//   to think that the dataset without the missing data elements is the "original"
//   dataset.
//
// Programmer: Marc Durant
// Creation:   April 26, 2012
//
// Modifications:
//
// ****************************************************************************

void
avtStaggeringFilter::PostExecute(void)
{
    // Call the base class's PostExecute. Set the spatial dimension to zero to
    // bypass a check in avtDataObjectToDatasetFilter::PostExecute that causes
    // all unstructured meshes with tdim<sdim to become polydata. We don't want
    // that so work around it by setting tdim==sdim. Then we do the PostExecute
    // and restore the old value.
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    int sdim = outAtts.GetSpatialDimension();
    int tdim = outAtts.GetTopologicalDimension();
    if (tdim < sdim && sdim >= 2) {
        outAtts.SetTopologicalDimension(sdim);
    }
    avtDataTreeIterator::PostExecute();
    if (tdim < sdim && sdim >= 2) {
        outAtts.SetTopologicalDimension(tdim);
    }
}

// ****************************************************************************
//  Method:  avtStaggeringFilter::FilterUnderstandsTransformedRectMesh
//
//  Purpose:
//    If this filter returns true, this means that it correctly deals
//    with rectilinear grids having an implied transform set in the
//    data attributes.  It can do this conditionally if desired.
//
//  Arguments:
//    none
//
//  Programmer:  Marc Durant
//  Creation:    April 26, 2012
//
// ****************************************************************************

bool
avtStaggeringFilter::FilterUnderstandsTransformedRectMesh()
{
    //A little tricky, because technically we don't handle transformed meshes,
    //but we also don't want to force a rect->unstructured transition
    //Final solution is don't add the filter if it's not needed.
    //Short-term solution is to return true so we don't mess up filters downstream
    return true;
}
