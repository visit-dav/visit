// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkReduceFilter.h"

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


vtkStandardNewMacro(vtkReduceFilter);


vtkReduceFilter::vtkReduceFilter()
{
  reduceType = rVectors;
  stride = 10;
  numEls = -1;
  origOnly = true;
}

void
vtkReduceFilter::SetStride(int s)
{
  numEls = -1;
  stride = s;
}

void
vtkReduceFilter::SetNumberOfElements(int n)
{
  stride = -1;
  numEls = n;
}

void
vtkReduceFilter::SetLimitToOriginal(bool lto)
{
  origOnly = lto;
}

void
vtkReduceFilter::ReduceVectors()
{
  reduceType = rVectors;
}


void
vtkReduceFilter::ReduceTensors()
{
  reduceType = rTensors;
}

// ****************************************************************************
// Method: vtkReduceFilter::RequestData
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
vtkReduceFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  vtkDebugMacro(<<"Executing vtkReduceFilter");

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

  vtkDataArray *inCObjects;
  vtkDataArray *inPObjects;

  if( reduceType == rVectors )
  {
      inCObjects = inCd->GetVectors();
      inPObjects = inPd->GetVectors();
  }
  else if( reduceType == rTensors )
  {
      inCObjects = inCd->GetTensors();
      inPObjects = inPd->GetTensors();
  }
  
  int npts = input->GetNumberOfPoints();
  int ncells = input->GetNumberOfCells();

  if (inPObjects == NULL && inCObjects == NULL)
  {
    vtkErrorMacro(<<"No objects to reduce");
    return 1;
  }

  int inPType = (inPObjects ? inPObjects->GetDataType() : VTK_FLOAT);
  int inCType = (inCObjects ? inCObjects->GetDataType() : VTK_FLOAT);

  // Determine what the stride is.
  if (stride <= 0 && numEls <= 0)
  {
    vtkErrorMacro(<<"Invalid stride");
    return 1;
  }

  float actingStride = stride;
  if (actingStride <= 0)
  {
    int totalObjects = 0;
    if (inPObjects != NULL)
    {
        totalObjects += npts;
    }
    if (inCObjects != NULL)
    {
        totalObjects += ncells;
    }
    actingStride = ceil(((float) totalObjects) / ((float) numEls));
  }

  vtkPoints *outpts = vtkVisItUtility::NewPoints(input);
  vtkDataArray *outObjects;
  if (inPType == VTK_DOUBLE || inCType == VTK_DOUBLE)
      outObjects = vtkDoubleArray::New();
  else 
      outObjects = vtkFloatArray::New();

  int nComponents;
  
  if (inPObjects != NULL)
    nComponents = inPObjects->GetNumberOfComponents();
  else if (inCObjects != NULL)
    nComponents = inCObjects->GetNumberOfComponents();

  float *fv = new float [nComponents];
  double *v = new double[nComponents];

  outObjects->SetNumberOfComponents(nComponents);

  float nextToTake = 0.;
  int count = 0;
  if (inPObjects != NULL)
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
    outObjects->SetName(inPObjects->GetName());
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

        inPObjects->GetTuple(i, v);

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
                outObjects->InsertNextTuple(v);
            }
            else
            {
                for( c=0; c<nComponents; ++c )
                  fv[c] = vtkVisItUtility::SafeDoubleToFloat(v[c]);

                outObjects->InsertNextTuple(fv);
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
  if (inCObjects != NULL && inPObjects == NULL)
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
    outObjects->SetName(inCObjects->GetName());
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

        inCObjects->GetTuple(i, v);
        outObjects->InsertNextTuple(v);
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

  delete [] fv;
  delete []  v;

  int nOutPts = outpts->GetNumberOfPoints();
  output->SetPoints(outpts);
  outpts->Delete();

  if (reduceType == rVectors && inPObjects)
    output->GetPointData()->SetVectors(outObjects);
  else if (reduceType == rVectors && inCObjects)
    output->GetCellData()->SetVectors(outObjects);

  else if (reduceType == rTensors && inPObjects)
    output->GetPointData()->SetTensors(outObjects);
  else if (reduceType == rTensors && inCObjects)
    output->GetCellData()->SetTensors(outObjects);

  outObjects->Delete();

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
//  Method: vtkReduceFilter::FillInputPortInformation
//
// ****************************************************************************

int
vtkReduceFilter::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}

// ****************************************************************************
//  Method: vtkReduceFilter::PrintSelf
//
// ****************************************************************************

void
vtkReduceFilter::PrintSelf(ostream &os, vtkIndent indent)
{
   this->Superclass::PrintSelf(os, indent);
   os << indent << "Stride: " << this->stride << "\n";
   os << indent << "Target number of objects: " << this->numEls << "\n";
   os << indent << "Limit to original cell/point: " << this->origOnly << "\n";
}
