/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItCellDataToPointData.cxx,v $
  Language:  C++
  Date:      $Date: 2002/12/17 02:05:38 $
  Version:   $Revision: 1.24 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkVisItCellDataToPointData.h"

#include "vtkAccessors.h"
#include "vtkCellData.h"
#include "vtkDataSet.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkRectilinearGrid.h"
#include "vtkStructuredGrid.h"

vtkStandardNewMacro(vtkVisItCellDataToPointData);

// Instantiate object so that cell data is not passed to output.
vtkVisItCellDataToPointData::vtkVisItCellDataToPointData()
{
  this->PassCellData = false;
}

#define VTK_MAX_CELLS_PER_POINT 4096

// **************************************************************************** 
//  Modifications:
//    Hank Childs, Wed Mar  9 16:23:01 PST 2005
//    Fix minor UMR.
//
//    Dave Pugmire, Wed Jul  30 16:57:23 EST 2008
//    Avoid using fastTrack code for non-scalar data. The fastTrack code does
//    not handle it right.
//
//    Kathleen Biagas, Thu Sep 6 11:05:01 MST 2012
//    Added templatized helper method to handle double-precision.
//
// **************************************************************************** 

template <class Accessor> inline void
vtkVisItCellDataToPointData_Copy(int ptId, double weight, int *ids, int nids,
    Accessor var_out, Accessor var_in)
{
    double val = 0.;
    for (int m = 0 ; m < nids ; ++m)
    {
        val += weight * var_in.GetComponent(ids[m]);
    }
    var_out.SetComponent(ptId, val);
}

// **************************************************************************** 
//  Modifications:
//    Eric Brugger, Wed Jan  9 14:48:17 PST 2013
//    Modified to inherit from vtkDataSetAlgorithm.
//
// **************************************************************************** 

int vtkVisItCellDataToPointData::RequestData(
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
  vtkDataSet  *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkDataSet *output = vtkDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkIdType cellId, ptId, i, j, k, l;
  vtkIdType numCells, numPts;
  vtkCellData *inPD=input->GetCellData();
  vtkPointData *outPD=output->GetPointData();
  vtkIdList *cellIds;
  double weight;
  double *weights;

  vtkDebugMacro(<<"Mapping cell data to point data");

  // First, copy the input to the output as a starting point
  output->CopyStructure( input );

  cellIds = vtkIdList::New();
  cellIds->Allocate(VTK_MAX_CELLS_PER_POINT);

  if ( (numPts=input->GetNumberOfPoints()) < 1 )
    {
    vtkErrorMacro(<<"No input point data!");
    cellIds->Delete();
    return 1;
    }
  weights = new double[VTK_MAX_CELLS_PER_POINT];
  
  // Pass the point data first. The fields and attributes
  // which also exist in the cell data of the input will
  // be over-written during CopyAllocate
  output->GetPointData()->PassData(input->GetPointData());

  // notice that inPD and outPD are vtkCellData and vtkPointData; respectively.
  // It's weird, but it works.
  outPD->CopyAllocate(inPD,numPts);

  bool canFastTrackStructured = false;
  int dims[3] = { 0, 0, 0 };
  if (input->GetDataObjectType() == VTK_RECTILINEAR_GRID)
  {
    ((vtkRectilinearGrid *) input)->GetDimensions(dims);
    canFastTrackStructured = true;
  }
  else if (input->GetDataObjectType() == VTK_STRUCTURED_GRID)
  {
    ((vtkStructuredGrid *) input)->GetDimensions(dims);
    canFastTrackStructured = true;
  }

  // Let's not worry about degenerate cases -- or even 2D.
  if (dims[0] <= 1 || dims[1] <= 1 || dims[2] <= 1)
  {
    canFastTrackStructured = false;
  }

  // Only handle floating point/double and make sure our assumptions about
  // the variable's names are valid.
  int nvals = inPD->GetNumberOfArrays();
  for (i = 0 ; i < nvals ; i++)
  {
    if (inPD->GetArray(i)->GetName() == NULL)
      canFastTrackStructured = false;
    else if (strcmp(inPD->GetArray(i)->GetName(), "") == 0)
      canFastTrackStructured = false;

    if (outPD->GetArray(inPD->GetArray(i)->GetName()) == NULL)
      canFastTrackStructured = false;
    if (!(inPD->GetArray(i)->GetDataType() == VTK_FLOAT ||
          inPD->GetArray(i)->GetDataType() == VTK_DOUBLE))
      canFastTrackStructured = false;

    // fastTrack code doesn't support non-scalar data, so for now, avoid it.
    if (inPD->GetArray(i)->GetNumberOfComponents() != 1 )
        canFastTrackStructured = false;
  }
       
  int abort=0;
  vtkIdType progressInterval=numPts/20 + 1;
  if (canFastTrackStructured)
    {
    const int iOffset = 1;
    const int jOffset = (dims[0]-1);
    const int kOffset = (dims[0]-1)*(dims[1]-1);
    double weights[4] = { 1., 0.5, 0.25, 0.125 };
    int nids_array[4] = { 1, 2, 4, 8 };
    int ids[8];

    const int maxArrays = 128;
    vtkDataArray *vars_in[maxArrays];
    vtkDataArray *vars_out[maxArrays];
    for (i = 0 ; i < nvals ; i++)
    {
       // This should be automatic, but some arrays do not seem to correctly
       // know how many tuples they have.
       outPD->GetArray(inPD->GetArray(i)->GetName())
                              ->SetNumberOfTuples(output->GetNumberOfPoints());
       vars_in[i] = inPD->GetArray(i);
       vars_out[i] = outPD->GetArray(inPD->GetArray(i)->GetName());
    }
    for (i = nvals ; i < maxArrays ; i++)
    {
        vars_in[i] = NULL;
        vars_out[i] = NULL;
    }

    for (k = 0 ; k < dims[2] ; k++)
    {
      for (j = 0 ; j < dims[1] ; j++)
      {
        for (i = 0 ; i < dims[0] ; i++)
        {
          int numBad = 0;
          if (i == 0 || i == dims[0]-1)
            numBad++;
          if (j == 0 || j == dims[1]-1)
            numBad++;
          if (k == 0 || k == dims[2]-1)
            numBad++;

          double weight = weights[3-numBad];
          int nids = nids_array[3-numBad];

          int id = 0;
          if (i > 0 && j > 0 && k > 0)
             ids[id++] = (k-1)*kOffset+(j-1)*jOffset+(i-1)*iOffset;
          if (i < (dims[0]-1) && j > 0 && k > 0)
             ids[id++] = (k-1)*kOffset+(j-1)*jOffset+i*iOffset;
          if (i > 0 && j < (dims[1]-1) && k > 0)
             ids[id++] = (k-1)*kOffset+j*jOffset+(i-1)*iOffset;
          if (i < (dims[0]-1) && j < (dims[1]-1) && k > 0)
             ids[id++] = (k-1)*kOffset+j*jOffset+i*iOffset;
          if (i > 0 && j > 0 && k < (dims[2]-1))
             ids[id++] = k*kOffset+(j-1)*jOffset+(i-1)*iOffset;
          if (i < (dims[0]-1) && j > 0 && k < (dims[2]-1))
             ids[id++] = k*kOffset+(j-1)*jOffset+i*iOffset;
          if (i > 0 && j < (dims[1]-1) && k < (dims[2]-1))
             ids[id++] = k*kOffset+j*jOffset+(i-1)*iOffset;
          if (i < (dims[0]-1) && j < (dims[1]-1) && k < (dims[2]-1))
             ids[id++] = k*kOffset+j*jOffset+i*iOffset;
          
          int ptId = k*dims[0]*dims[1] + j*dims[0] + i;
          for (l = 0 ; l < nvals ; l++)
          {
              // only floats/doubles take the fast-path.
              if (vars_in[l]->GetDataType() == VTK_FLOAT)
              {
                  vtkVisItCellDataToPointData_Copy(ptId, weight, ids, nids,
                      vtkDirectAccessor<float>(vars_out[l]), 
                      vtkDirectAccessor<float>(vars_in[l]));
              }
              else if (vars_in[l]->GetDataType() == VTK_DOUBLE)
              {
                  vtkVisItCellDataToPointData_Copy(ptId, weight, ids, nids,
                      vtkDirectAccessor<double>(vars_out[l]), 
                      vtkDirectAccessor<double>(vars_in[l]));
              }
          }
        }
      }
    } // End for

    } // End if
  else
    {
    for (ptId=0; ptId < numPts && !abort; ptId++)
      {
      if ( !(ptId % progressInterval) )
        {
        this->UpdateProgress((float)ptId/numPts);
        abort = GetAbortExecute();
        }
  
      input->GetPointCells(ptId, cellIds);
      numCells = cellIds->GetNumberOfIds();
      if ( numCells > 0 )
        {
        weight = 1.0 / numCells;
        for (cellId=0; cellId < numCells; cellId++)
          {
          weights[cellId] = weight;
          }
        outPD->InterpolatePoint(inPD, ptId, cellIds, weights);
        }
      else
        {
        outPD->NullPoint(ptId);
        }
      }
    }

  if ( this->PassCellData )
    {
    output->GetCellData()->PassData(input->GetCellData());
    }

  cellIds->Delete();
  delete [] weights;

  return 1;
}

void vtkVisItCellDataToPointData::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Pass Cell Data: " << (this->PassCellData ? "On\n" : "Off\n");
}
