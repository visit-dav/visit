// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkVisItTensorGlyph.h"

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataObject.h>
#include <vtkDataSet.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

vtkStandardNewMacro(vtkVisItTensorGlyph)

//----------------------------------------------------------------------------
vtkVisItTensorGlyph::vtkVisItTensorGlyph()
{
}

//----------------------------------------------------------------------------
vtkVisItTensorGlyph::~vtkVisItTensorGlyph() = default;

// ****************************************************************************
//  Modifications:
//
//   Hank Childs, Fri Feb 15 11:43:54 PST 2008
//   Strengthen tests for mismatched data types.  Also fix possible memory
//   leak.
//
//   Hank Childs, Fri Mar 12 12:15:49 PST 2010
//   Incorporate fix from Seth Johnson of UMich for scaling.
//
//   Kathleen Biagas, Tue June 16, 2026
//   Inherit from vtkTensorGlyph. Perform necessary VisIt modifications after
//   parent class finishes processing.
//
// ****************************************************************************

int
vtkVisItTensorGlyph::RequestData(vtkInformation *request,
                                 vtkInformationVector **inputVector,
                                 vtkInformationVector *outputVector)
{
  int retval = this->Superclass::RequestData(request, inputVector, outputVector);

  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *sourceInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  if (inInfo == nullptr || sourceInfo == nullptr || outInfo == nullptr)
  {
    return retval;
  }

  vtkDataSet *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *source = vtkPolyData::SafeDownCast(
    sourceInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (input == nullptr || source == nullptr || output == nullptr)
  {
    return retval;
  }

  // ensure avtOriginalNodeNumbers and avtOriginalCellNumbers will be
  // available in the output
  vtkPointData *pd = input->GetPointData();
  vtkDataArray *inOrigNodes = pd->GetArray("avtOriginalNodeNumbers");
  vtkDataArray *inOrigCells = pd->GetArray("avtOriginalCellNumbers");
  if (inOrigNodes == nullptr && inOrigCells == nullptr)
  {
    return retval;
  }

  vtkCellData *outCD = output->GetCellData();
  const vtkIdType numPts = input->GetNumberOfPoints();
  const vtkIdType numSourceCells = source->GetNumberOfCells();
  const vtkIdType outputNumCells = output->GetNumberOfCells();
  const vtkIdType numDirs = (this->GetThreeGlyphs() ? 3 : 1) *
                            (this->GetSymmetric() ? 2 : 1);

  vtkDataArray *outOrigNodes = nullptr;
  vtkDataArray *outOrigCells = nullptr;

  if (inOrigNodes != nullptr)
  {
    outOrigNodes = inOrigNodes->NewInstance();
    if(outOrigNodes != nullptr)
    {
      outOrigNodes->SetName(inOrigNodes->GetName());
      outOrigNodes->SetNumberOfComponents(inOrigNodes->GetNumberOfComponents());
      outOrigNodes->Allocate(inOrigNodes->GetNumberOfComponents() * outputNumCells);
    }
  }

  if (inOrigCells != nullptr)
  {
    outOrigCells = inOrigCells->NewInstance();
    if (outOrigCells != nullptr)
    {
      outOrigCells->SetName(inOrigCells->GetName());
      outOrigCells->SetNumberOfComponents(inOrigCells->GetNumberOfComponents());
      outOrigCells->Allocate(inOrigCells->GetNumberOfComponents() * outputNumCells);
    }
  }

  vtkIdType generatedCells = 0;
  for (vtkIdType inPtId = 0; inPtId < numPts && generatedCells < outputNumCells; ++inPtId)
  {
    double *inNode = (inOrigNodes != nullptr) ? inOrigNodes->GetTuple(inPtId) : nullptr;
    double *inCell = (inOrigCells != nullptr) ? inOrigCells->GetTuple(inPtId) : nullptr;

    for (vtkIdType cellId = 0; cellId < numSourceCells && generatedCells < outputNumCells; ++cellId)
    {
      for (vtkIdType dir = 0; dir < numDirs && generatedCells < outputNumCells; ++dir)
      {
        if (outOrigNodes != nullptr)
        {
          outOrigNodes->InsertNextTuple(inNode);
        }
        if (outOrigCells != nullptr)
        {
          outOrigCells->InsertNextTuple(inCell);
        }
        ++generatedCells;
      }
    }
  }

  if (outOrigNodes != nullptr)
  {
    outCD->AddArray(outOrigNodes);
    outOrigNodes->Delete();
  }
  if (outOrigCells != nullptr)
  {
    outCD->AddArray(outOrigCells);
    outOrigCells->Delete();
  }


  // Fix scaling issue
  if(this->ColorGlyphs && (this->ColorMode == COLOR_BY_EIGENVALUES) &&
     (this->ScaleFactor > 0.0))
  {
    vtkDataArray *scalars  = output->GetPointData()->GetScalars();
    if(scalars)
    {
      for(vtkIdType i  = 0; i < scalars->GetNumberOfTuples(); ++i)
        scalars->SetTuple1(i, scalars->GetTuple1(i)/this->ScaleFactor);
    }
  }

  return retval;
}
