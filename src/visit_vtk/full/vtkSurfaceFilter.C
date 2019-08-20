// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  Class:   vtkSurfaceFilter
//
//  Purpose:
//    Derived type of vtkDataSetToDataSetFilter.
//
//  Notes:  
//
//  Programmer:  Kathleen S. Bonnell
//
//  Creation:  14 March 2001
//
//  Modifications:
//
// ****************************************************************************

#include "vtkSurfaceFilter.h"

#include <vtkCell.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkIdList.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVisItUtility.h>

// ****************************************************************************
//  Modifications:
//    Kathleen Bonnell, Wed Mar  6 15:14:29 PST 2002
//    Replace 'New' method with Macro to match VTK 4.0 API.
// ****************************************************************************

vtkStandardNewMacro(vtkSurfaceFilter);
vtkCxxSetObjectMacro(vtkSurfaceFilter, inScalars, vtkDataArray); 


// ****************************************************************************
// Construct with 
vtkSurfaceFilter::vtkSurfaceFilter()
{
  this->inScalars = NULL;
}


// ****************************************************************************
// Destructor
vtkSurfaceFilter::~vtkSurfaceFilter()
{
  this->SetinScalars(NULL);
}


// ****************************************************************************
//  Method: vtkSurfaceFilter::RequestData
//
//  Purpose:
//    vtk Required method. Updates state of the filter by
//
//  Arguments:  None
//
//  Returns:    None
//
//  Assumptions and Comments:
//
//  Programmer: Kathleen S. Bonnell
//  Creation:   12 March 2001
//
//  Modifications:
//
// ****************************************************************************
int
vtkSurfaceFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  //
  // Initialize some frequently used values.
  //
  vtkDataSet *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (inScalars == NULL)
  {
      vtkErrorMacro(<<"Cannot execute, scalars not set");
      return 1;
  }

  if (input->GetDataObjectType() == VTK_RECTILINEAR_GRID)
      this->ExecuteRectilinearGrid( (vtkRectilinearGrid *) input, output);
  else
      this->ExecutePointSet( (vtkPointSet*) input, output);

  return 1;
} // RequestData


// ****************************************************************************
//  Method: vtkSurfaceFilter::FillInputPortInformation
//
// ****************************************************************************

int
vtkSurfaceFilter::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
} // FillInputPortInformation


// ****************************************************************************
//  Method: vtkSurfaceFilter::PrintSelf
//
//  Purpose:  Prints pertinent information regarding the state of this class
//            to the given output stream.
//
//  Arguments:
//    os      The output stream to which the information is printed
//    indent  The amount of spaces to indent.
//
//  Returns:  None
//
//  Assumptions and Comments:
//    Calls the superclass method first.
//
//  Programmer: Kathleen S. Bonnell
//  Creation:   5 October 2000
//
//  Modifications:
//
// ****************************************************************************
void 
vtkSurfaceFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "zVals :    ";
  inScalars->PrintSelf(os, indent); 
  os << indent << "\n";
}


// ****************************************************************************
//  Method: vtkSurfaceFilter::ExecuteRectilinearGrid
//
//  Purpose:  
//    Execution method for rectilinear grid input types.
//
//  Arguments: rg   the rectilinear grid input
//
//  Returns:   None 
//
//  Assumptions and Comments:
//
//  Programmer: Kathleen S. Bonnell
//  Creation:   5 October 2000
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Kathleen Biagas, Thu Aug 30 16:26:44 MST 2012
//    Preserve coordinate type.
//
// ****************************************************************************
void 
vtkSurfaceFilter::ExecuteRectilinearGrid(vtkRectilinearGrid *rg,
  vtkUnstructuredGrid *output)
{
  vtkDebugMacro(<<"ExecuteRectilinearGrid::");

  int numPoints = rg->GetNumberOfPoints();
  int numCells = rg->GetNumberOfCells();
  int *cellTypes = new int [numCells];
  vtkCellArray *cells = vtkCellArray::New();

  vtkPoints *outPoints = vtkVisItUtility::NewPoints(rg);
  outPoints->SetNumberOfPoints(numPoints);

  int dims[3];
  rg->GetDimensions(dims);
  double point[3]; 
  double val;
  for (int i = 0; i < numPoints; ++i)
  {
    rg->GetPoint(i, point);
    val = inScalars->GetTuple1(i); 
    if (dims[2] == 1) // replace z coordinates with val
    {
        point[2] = val;
    }
    else if (dims[1] == 1) // replace y coordinates with val
    {
        point[1] = val;
    }
    else  // replace x coordinates with val
    {
        point[0] = val;
    }
    outPoints->SetPoint(i, point);
  }

  vtkCell * aCell = NULL;
  for (int j = 0; j < numCells; ++j)
  {
    aCell = rg->GetCell(j);
    cellTypes[j] = aCell->GetCellType();
    cells->InsertNextCell(aCell);
  }

  output->SetPoints(outPoints);
  output->SetCells(cellTypes, cells);
  output->GetPointData()->PassData(rg->GetPointData());
  output->GetCellData()->PassData(rg->GetCellData());

  delete [] cellTypes;
  outPoints->Delete();
  cells->Delete();

} // ExecuteRectilinearGrid

      
// ****************************************************************************
//  Method: vtkSurfaceFilter::ExecutePointSet
//
//  Purpose:
//    Execution method for rectilinear grid input types.
//
//  Arguments: rg   the rectilinear grid input
//
//  Returns:   None
//
//  Assumptions and Comments:
//
//  Programmer: Kathleen S. Bonnell
//  Creation:   5 October 2000
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Thu Sep 12 19:12:35 PDT 2002
//    Fix memory leak.
//
//    Kathleen Biagas, Thu Aug 30 16:26:44 MST 2012
//    Preserve coordinate type.
//
// ****************************************************************************
void 
vtkSurfaceFilter::ExecutePointSet(vtkPointSet *ps, vtkUnstructuredGrid *output)
{
  vtkDebugMacro(<<"ExecutePointSet::");

  vtkPoints *inPoints = ps->GetPoints();

  if ( inScalars->GetNumberOfTuples() != inPoints->GetNumberOfPoints() )
  {
      vtkErrorMacro(<<"Number of scalars must match number of points!");
      return;
  }

  vtkPoints *outPoints = vtkPoints::New(inPoints->GetDataType());
  outPoints->SetNumberOfPoints(inPoints->GetNumberOfPoints());

  double point[3];
  for (int i = 0; i < inScalars->GetNumberOfTuples(); ++i)
  {
     inPoints->GetPoint(i, point);
     point[2] = inScalars->GetTuple1(i);
     outPoints->SetPoint(i, point); 
  }
  int numCells = ps->GetNumberOfCells();
  int *cellTypes = new int [numCells];
  vtkCellArray *cells = vtkCellArray::New();
  vtkCell * aCell = NULL; /// TODO: is switch handling all cases?
  for (int j = 0; j < numCells; ++j)
  {
    switch(ps->GetDataObjectType())
    {
        case VTK_POLY_DATA : 
             aCell = ((vtkPolyData*)ps)->GetCell(j); break;
        case VTK_STRUCTURED_GRID : 
             aCell = ((vtkStructuredGrid*)ps)->GetCell(j); break;
        case VTK_UNSTRUCTURED_GRID : 
             aCell = ((vtkUnstructuredGrid*)ps)->GetCell(j); break;
    }
    cellTypes[j] = aCell->GetCellType();
    cells->InsertNextCell(aCell);
  }

  output->SetPoints(outPoints);
  output->SetCells(cellTypes, cells);
  output->GetPointData()->PassData(ps->GetPointData());
  output->GetCellData()->PassData(ps->GetCellData());

  // free memory used locally
  delete [] cellTypes;
  outPoints->Delete();
  cells->Delete();

} // ExecutePointSet
