/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
#include <vtkDataSet.h>
#include <vtkDataObject.h>
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
//  Programmer: Kathleen Bonnell
//  Creation:   October 29, 2004
//
//  Modifications:
//
// ****************************************************************************

avtStaggeringFilter::avtStaggeringFilter()
{
  debug5 <<"avtStaggeringFilter::avtStaggeringFilter()" <<std::endl;
  
}


// ****************************************************************************
//  Method: avtStaggeringFilter destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 29, 2004
//
//  Modifications:
//
// ****************************************************************************

avtStaggeringFilter::~avtStaggeringFilter()
{
  debug5 <<"avtStaggeringFilter::~avtStaggeringFilter()" <<std::endl;
}

// ****************************************************************************
//  Method: avtStaggeringFilter::ExecuteData
//
//  Purpose:
//      Returns input. 
//
//  Arguments:
//      inDS      The input dataset.
//      <unused>  The domain number.
//      <unused>  The label.
//
//  Returns:      The output dataset.
//
//  Programmer:   Kathleen Bonnell
//  Creation:     October 29, 2004
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtStaggeringFilter::ExecuteData(vtkDataSet *inDS, int, std::string)
{
  
  if (!inDS) {
    return inDS;
  }
  
  vtkDataSet* outDS = NULL;
  
  debug5 <<"avtStaggeringFilter::ExecuteData()" <<std::endl;
  // Check for offset information attached to the data
  // For a Pseudocolor plot, we only need to consult the SCALARS.
  // In addition, we have no concept of offsets for cell-centered data
  // so we only need to check the Point data.
  vtkDataArray* velData = inDS->GetPointData()->GetScalars();
  //if (!velData) {
  //  velData = inDS->GetCellData()->GetScalars();
  //}
  
  if (!velData) {
    debug5 <<"avtStaggeringFilter::ExecuteData() - Unable to find scalar data on mesh, returning inptut dataset." <<std::endl;
    return inDS;
  }
    
  vtkInformation* info = velData->GetInformation();
      
  if (!info->Has(avtVariableCache::OFFSET_3())) {
    debug5 <<"avtStaggeringFilter::ExecuteData() - scalars have NO offset information, returning input dataset." <<std::endl;
    return inDS;
  }
      
  //Extract the offset information from the dataset
  double* nodeOffset = info->Get(avtVariableCache::OFFSET_3());
  debug5 <<"avtStaggeringFilter::ExecuteData() - offset is " <<nodeOffset[0] <<", " <<nodeOffset[1] <<", " <<nodeOffset[2] <<std::endl;
      
  int dstype = inDS->GetDataObjectType();
  switch (dstype)
  {
    case VTK_RECTILINEAR_GRID:
      //rv = ReflectRectilinear((vtkRectilinearGrid *) ds, dim);
      debug5 <<"avtStaggeringFilter::ExecuteData() - This is a RECTILINEAR grid." <<std::endl;
      break;
    case VTK_STRUCTURED_GRID:
    {
      debug5 <<"avtStaggeringFilter::ExecuteData() - This is a STRUCTURED grid." <<std::endl;
      
      vtkStructuredGrid* grid = vtkStructuredGrid::SafeDownCast(inDS);
      int* dimensions = grid->GetDimensions();
      
      vtkStructuredGrid* newGrid = vtkStructuredGrid::New();
      newGrid->ShallowCopy(grid);
        
      vtkPoints* oldPoints = grid->GetPoints();
      vtkIdType numPoints = oldPoints->GetNumberOfPoints();
      vtkPoints* newPoints = vtkPoints::New();
      newPoints->SetNumberOfPoints(numPoints);
      newGrid->SetPoints(newPoints);
      
      int* dims = newGrid->GetDimensions();
      int mulSizes[3];
      debug5 <<"avtStaggeringFilter::ExecuteData() - mesh has dimensions " <<dims[0] <<", " <<dims[1] <<", " <<dims[2] <<std::endl;
      mulSizes[0] = 1;
      mulSizes[1] = dims[0];
      mulSizes[2] = dims[0] * dims[1];
      debug5 <<"avtStaggeringFilter::ExecuteData() - mulSizes = " <<mulSizes[0] <<", " <<mulSizes[1] <<", " <<mulSizes[2] <<std::endl;
        
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
        debug5 <<"avtStaggeringFilter::ExecuteData() - point " <<k <<" = (" <<originalXYZ[0] <<", " <<originalXYZ[1] <<", " <<originalXYZ[2] <<")" <<std::endl;
          
        double newXYZ[3];
        newXYZ[0] = originalXYZ[0];
        newXYZ[1] = originalXYZ[1];
        newXYZ[2] = originalXYZ[2];
        // Apply node offsets
        for (size_t j1 = 0; j1 < 3; j1++) {
          //debug5 <<"j1 = " <<j1 <<std::endl;
          //debug5 <<"nodeOffset[j1] = " <<nodeOffset[j1] <<std::endl;
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
            
        debug5 <<"avtStaggeringFilter::ExecuteData() - displaced point[" <<k <<"] = " <<newXYZ[0] <<", " <<newXYZ[1] <<", " <<newXYZ[2] <<std::endl;

        //Now check to make sure that this is within the original spatial bounds
        bool withinBounds = true;
        if (withinBounds) {
          newPoints->SetPoint(k, newXYZ[0], newXYZ[1], newXYZ[2]);
        } else {
          newPoints->SetPoint(k, originalXYZ[0], originalXYZ[1], originalXYZ[2]);
        }
      }
          
      outDS = newGrid;
      ManageMemory(outDS);            
    }
    break;
          
    case VTK_UNSTRUCTURED_GRID:
      debug5 <<"avtStaggeringFilter::ExecuteData() -This is an UNSTRUCTURED grid." <<std::endl;
      break;
    case VTK_POLY_DATA:
      debug5 <<"avtStaggeringFilter::ExecuteData() -This is a POLY_DATA grid." <<std::endl;
      //rv = ReflectPointSet((vtkPointSet *) ds, dim);
      break;
    default:
      debug1 << "avtStaggeringFilter::ExecuteData() - Unknown data type = " << dstype << endl;
      break;
  }  //end switch(dstype)
      
      /*
       vtkTransformFilter* transform = vtkTransformFilter::New();
       XXX* t = XXX::New();
       t->SetDisplacement(displacement);
       t->SetInput(displacement);
       transform->SetTransform(t);
       t->Delete();
       
       transform->SetInput((vtkPointSet *)inDS);
       
       outDS = transform->GetOutput();
       outDS->Update();
       
       ManageMemory(outDS);
       transform->Delete();
       */
    
  if (outDS) {
    return outDS;
  }
    
  return inDS;
}  

// ****************************************************************************
//  Method: avtStaggeringFilter::UpdateDataObjectInfo
//
//  Purpose:  Sets flags in the pipeline.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 29, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtStaggeringFilter::UpdateDataObjectInfo(void)
{
  debug5 <<"avtStaggeringFilter::UpdateDataObjectInfo()" <<std::endl;

//    GetOutput()->GetInfo().GetAttributes().SetTopologicalDimension(0);
//    GetOutput()->GetInfo().GetAttributes().SetKeepNodeZoneArrays(keepNodeZone);
}


// ****************************************************************************
//  Method: avtStaggeringFilter::ModifyContract
//
//  Purpose:  Turns on Node/Zone numbers when appropriate. 
// 
//  Programmer: Kathleen Bonnell 
//  Creation:   October 29, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jun 10 13:37:09 PDT 2005
//    Verify the existence of a valid variable before attempting to retrieve
//    its centering.
//
//    Kathleen Bonnell, Tue Jul 14 13:42:37 PDT 2009
//    Added test for MayRequireNodes for turning Node numbers on.
//
// ****************************************************************************

avtContract_p
avtStaggeringFilter::ModifyContract(avtContract_p contract)
{
  debug5 <<"avtStaggeringFilter::ModifyContract()" <<std::endl;

  return contract;
}
