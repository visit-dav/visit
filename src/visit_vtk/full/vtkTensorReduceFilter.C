// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkTensorReduceFilter.h"

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkVisItUtility.h>


vtkStandardNewMacro(vtkTensorReduceFilter);


vtkTensorReduceFilter::vtkTensorReduceFilter()
{
  stride = 10;
  numEls = -1;
  origOnly = true;
}

void
vtkTensorReduceFilter::SetStride(int s)
{
  numEls = -1;
  stride = s;
}

void
vtkTensorReduceFilter::SetNumberOfElements(int n)
{
  stride = -1;
  numEls = n;
}

void
vtkTensorReduceFilter::SetLimitToOriginal(bool lto)
{
  origOnly = lto;
}

// ****************************************************************************
// Method: vtkTensorReduceFilter::RequestData
//
// Modifications:
//    Kathleen Bonnell, Tue Aug 30 11:11:56 PDT 2005 
//    Copy other Point and Cell data. 
//
//    Kathleen Biagas, Wed Sep 5 13:10:18 MST 2012 
//    Preserve coordinate and tensor data types.
//
//    Eric Brugger, Thu Jan 10 12:05:20 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
//    Allen Sanderson, Thu Jan 13 12:05:20 PST 2020
//    Copied from vtkVectorReduceFilter::RequestData(
//
// ****************************************************************************

int
vtkTensorReduceFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  vtkDebugMacro(<<"Executing vtkTensorReduceFilter");

  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  //
  // Initialize some frequently used values.
  //
  vtkDataSet   *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkCellData *inCd = input->GetCellData();
  vtkPointData *inPd = input->GetPointData();
  vtkCellData *outCd = output->GetCellData();
  vtkPointData *outPd = output->GetPointData();

  vtkDataArray *inCTensors = inCd->GetTensors();
  vtkDataArray *inPTensors = inPd->GetTensors();

  int npts = input->GetNumberOfPoints();
  int ncells = input->GetNumberOfCells();

  if (inPTensors == NULL && inCTensors == NULL)
    {
    vtkErrorMacro(<<"No tensors to reduce");
    return 1;
    }

  int inPType = (inPTensors ? inPTensors->GetDataType() : VTK_FLOAT);
  int inCType = (inCTensors ? inCTensors->GetDataType() : VTK_FLOAT);

  // Determine what the stride is.
  if (stride <= 0 && numEls <= 0)
    {
    vtkErrorMacro(<<"Invalid stride");
    return 1;
    }

  float actingStride = stride;
  if (actingStride <= 0)
    {
    int totalTensors = 0;
    if (inPTensors != NULL)
    {
        totalTensors += npts;
    }
    if (inCTensors != NULL)
    {
        totalTensors += ncells;
    }
    actingStride = ceil(((float) totalTensors) / ((float) numEls));
    }

  vtkPoints *outpts = vtkVisItUtility::NewPoints(input);
  vtkDataArray *outTensors;
  if (inPType == VTK_DOUBLE || inCType == VTK_DOUBLE)
      outTensors = vtkDoubleArray::New();
  else 
      outTensors = vtkFloatArray::New();

  int nComponents = 9;
  
  if (inPTensors != NULL)
    nComponents = std::min(nComponents, inPTensors->GetNumberOfComponents());
  else if (inCTensors != NULL)
    nComponents = std::min(nComponents, inCTensors->GetNumberOfComponents());

  outTensors->SetNumberOfComponents(nComponents);

  float nextToTake = 0.;
  int count = 0;
  if (inPTensors != NULL)
    {
    bool *foundcell = NULL;
    vtkDataArray *origCellArr =
      input->GetPointData()->GetArray("avtOriginalCellNumbers");
    vtkDataArray *origNodeArr =
      input->GetPointData()->GetArray("avtOriginalNodeNumbers");
    int ccmp = origCellArr ? origCellArr->GetNumberOfComponents() - 1 : -1;
    int ncmp = origNodeArr ? origNodeArr->GetNumberOfComponents() - 1 : -1;

    if (origOnly && origCellArr)
      {
      // Find needed size, allocate, and initialize the "found" array
      int max = 0;
      for (int i=0; i<npts; i++)
        if ((int)origCellArr->GetComponent(i,ccmp) > max)
          max = (int)origCellArr->GetComponent(i,ccmp);
      foundcell = new bool[max+1];
      for (int i=0; i<max+1; i++)
        foundcell[i] = false;
      }

    outPd->CopyAllocate(inPd, npts);
    outTensors->SetName(inPTensors->GetName());
    int index = 0;
    for (int i = 0 ; i < npts ; i++)
      {
      int orignode = i;
      int origcell = -1;
      if (origNodeArr)
        orignode = (int)origNodeArr->GetComponent(i,ncmp);
      if (origCellArr)
        origcell = (int)origCellArr->GetComponent(i,ccmp);

      if (origOnly && orignode<0)
        continue;

      if (foundcell && (origcell<0 || foundcell[origcell]))
        continue;

      if (index >= nextToTake)
        {
        nextToTake += actingStride;

        double v[9];
        inPTensors->GetTuple(i, v);

	int c;
	for( c=0; c<nComponents; ++c )
	{
	  if (v[c] != 0.)
	    break;
	}
	
        if (c < nComponents )
          {
            double pt[3];
            input->GetPoint(i, pt);
            outpts->InsertNextPoint(pt);
    
            if (inCType == VTK_DOUBLE || inPType == VTK_DOUBLE)
            {
                outTensors->InsertNextTuple(v);
            }
            else
            {
                float fv[9];
		for( c=0; c<nComponents; ++c )
		  fv[c] = vtkVisItUtility::SafeDoubleToFloat(v[c]);

                outTensors->InsertNextTuple(fv);
            }
            outPd->CopyData(inPd, i, count++);
          }
        }

      if (foundcell)
        foundcell[origcell] = true;

      index++;
      }
      outPd->Squeeze();
    }

  nextToTake = 0.;
  count = 0;
  if (inCTensors != NULL && inPTensors == NULL)
    {
    bool *foundcell = NULL;
    vtkDataArray *origCellArr =
      input->GetCellData()->GetArray("avtOriginalCellNumbers");
    int ccmp = origCellArr ? origCellArr->GetNumberOfComponents() - 1 : -1;

    if (origOnly && origCellArr)
    {
      // Find needed size, allocate, and initialize the "found" array
      int max = 0;
      for (int i=0; i<npts; i++)
        if ((int)origCellArr->GetComponent(i,ccmp) > max)
          max = (int)origCellArr->GetComponent(i,ccmp);
      foundcell = new bool[max+1];
      for (int i=0; i<max+1; i++)
        foundcell[i] = false;
    }

    outCd->CopyAllocate(inCd, ncells);
    outTensors->SetName(inCTensors->GetName());
    int index = 0;
    for (int i = 0 ; i < ncells ; i++)
      {
      int origcell = i;
      if (origCellArr)
        origcell = (int)origCellArr->GetComponent(i,ccmp);

      if (foundcell && (origcell<0 || foundcell[origcell]))
        continue;

      if (index >= nextToTake)
        {
        nextToTake += actingStride;

        vtkCell *cell = input->GetCell(i);
        double pt[3];
        vtkVisItUtility::GetCellCenter(cell,pt);
        outpts->InsertNextPoint(pt);

        double v[9];
        inCTensors->GetTuple(i, v);
        outTensors->InsertNextTuple(v);
        outCd->CopyData(inCd, i, count++);
        }

      if (foundcell)
        foundcell[origcell] = true;

      index++;
      }
      outCd->Squeeze();

    if (foundcell)
      delete[] foundcell;
    }

  int nOutPts = outpts->GetNumberOfPoints();
  output->SetPoints(outpts);
  outpts->Delete();
  if (inPTensors)
    output->GetPointData()->SetTensors(outTensors);
  else
    output->GetCellData()->SetTensors(outTensors);
  outTensors->Delete();

  output->Allocate(nOutPts);
  vtkIdType onevertex[1];
  for (int i = 0 ; i < nOutPts ; i++)
    {
    onevertex[0] = i;
    output->InsertNextCell(VTK_VERTEX, 1, onevertex);
    }

  return 1;
}

// ****************************************************************************
//  Method: vtkTensorReduceFilter::FillInputPortInformation
//
// ****************************************************************************

int
vtkTensorReduceFilter::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}

// ****************************************************************************
//  Method: vtkTensorReduceFilter::PrintSelf
//
// ****************************************************************************

void
vtkTensorReduceFilter::PrintSelf(ostream &os, vtkIndent indent)
{
   this->Superclass::PrintSelf(os, indent);
   os << indent << "Stride: " << this->stride << "\n";
   os << indent << "Target number of tensors: " << this->numEls << "\n";
   os << indent << "Limit to original cell/point: " << this->origOnly << "\n";
}
