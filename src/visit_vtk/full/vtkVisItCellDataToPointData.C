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

#include "vtkCellData.h"
#include "vtkDataSet.h"
#include "vtkIdList.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkRectilinearGrid.h"
#include "vtkStructuredGrid.h"

vtkCxxRevisionMacro(vtkVisItCellDataToPointData, "$Revision: 1.24 $");
vtkStandardNewMacro(vtkVisItCellDataToPointData);

// Instantiate object so that cell data is not passed to output.
vtkVisItCellDataToPointData::vtkVisItCellDataToPointData()
{
  this->PassCellData = 0;
}

#define VTK_MAX_CELLS_PER_POINT 4096

// **************************************************************************** 
//  Modifications:
//
//    Hank Childs, Wed Mar  9 16:23:01 PST 2005
//    Fix minor UMR.
//
// **************************************************************************** 

void vtkVisItCellDataToPointData::Execute()
{
  vtkIdType cellId, ptId, i, j, k, l, m;
  vtkIdType numCells, numPts;
  vtkDataSet *input= this->GetInput();
  vtkDataSet *output= this->GetOutput();
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
    return;
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

  // Only handle floating point and make sure that are assumptions about
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
    if (inPD->GetArray(i)->GetDataType() != VTK_FLOAT)
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
    float *vars_in[maxArrays];
    float *vars_out[maxArrays];
    for (i = 0 ; i < nvals ; i++)
    {
       // This should be automatic, but some arrays do not seem to correctly
       // know how many tuples they have.
       outPD->GetArray(inPD->GetArray(i)->GetName())
                              ->SetNumberOfTuples(output->GetNumberOfPoints());
       vars_in[i] = (float *) inPD->GetArray(i)->GetVoidPointer(0);
       vars_out[i] = (float *) outPD->GetArray(inPD->GetArray(i)->GetName())
                                                           ->GetVoidPointer(0);
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
             vars_out[l][ptId] = 0.;
             for (m = 0 ; m < nids ; m++)
             {
               vars_out[l][ptId] += weight*vars_in[l][ids[m]];
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
}

void vtkVisItCellDataToPointData::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Pass Cell Data: " << (this->PassCellData ? "On\n" : "Off\n");
}
