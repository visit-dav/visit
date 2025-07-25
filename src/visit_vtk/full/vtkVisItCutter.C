// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkVisItCutter.h"

#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkUnstructuredGridBase.h>


vtkObjectFactoryNewMacro(vtkVisItCutter)

//------------------------------------------------------------------------------
vtkVisItCutter::vtkVisItCutter(vtkImplicitFunction* cf) : vtkCutter(cf)
{
  this->UnstructuredGridBypass = 0;
}

//------------------------------------------------------------------------------
vtkVisItCutter::~vtkVisItCutter()
{
}

// Cut through data generating surface.
int vtkVisItCutter::RequestData(
  vtkInformation* request,
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  // get the info objects
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkDataSet* input = vtkDataSet::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData* output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (!this->CutFunction)
  {
    vtkErrorMacro("No cut function specified");
    return 0;
  }

  if (!input)
  {
    return 0;
  }

  if (input->GetNumberOfPoints() < 1 || this->GetNumberOfContours() < 1)
  {
    return 1;
  }

  //
  // this begins the change in logic from vtkCutter::RequestData, to work around a bug
  //
  if (this->UnstructuredGridBypass && vtkUnstructuredGridBase::SafeDownCast(input))
  {
      // bypasses the 'executePlaneCutter' for UnstructuredGrids
      // Necessary when Slicing polygons and polyhedra, as the executePlaneCutter
      // method utilizes vtkPlaneCutter, which doesn't process CellData
      // correctly for these cell types.
      this->UnstructuredGridCutter(input, output);
  }
  else if (vtkPolyData::SafeDownCast(input))
  {
      vtkPolyData *pd = vtkPolyData::SafeDownCast(input);
      if(pd->GetNumberOfVerts() == pd->GetNumberOfCells())
      {
          // call vtkCutter::RequestData to handle the data for a pointset.
          return this->Superclass::RequestData(request, inputVector, outputVector);
      }
      else
      {
          // bypasses the 'executePlaneCutter' for PolyData, due to failure
          // when slice plane lies along boundary of the data,
          this->DataSetCutter(input, output);
      }

  }
  else
  {
      // call vtkCutter::RequestData to handle the data.
      return this->Superclass::RequestData(request, inputVector, outputVector);
  }

  return 1;
}

