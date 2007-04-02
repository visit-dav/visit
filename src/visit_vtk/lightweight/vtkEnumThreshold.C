/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkThreshold.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkEnumThreshold.h"

#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkUnstructuredGrid.h"

vtkCxxRevisionMacro(vtkEnumThreshold, "$Revision: 1.68 $");
vtkStandardNewMacro(vtkEnumThreshold);

//  Modifications:  
//    Jeremy Meredith, Tue Aug 22 16:20:41 EDT 2006
//    Taken from 5.0.0.a vtkThreshold source, renamed to vtkEnumThreshold, and
//    made it work on an enumerated scalar selection.
vtkEnumThreshold::vtkEnumThreshold()
{
    // by default process active point scalars
    this->SetInputArrayToProcess(0,0,0,
                                 vtkDataObject::FIELD_ASSOCIATION_POINTS_THEN_CELLS,
                                 vtkDataSetAttributes::SCALARS);

    enumerationMap = NULL;
}

vtkEnumThreshold::~vtkEnumThreshold()
{
}

//  Modifications:  
//    Jeremy Meredith, Tue Aug 22 16:20:41 EDT 2006
//    Taken from 5.0.0.a vtkThreshold source, renamed to vtkEnumThreshold, and
//    made it work on an enumerated scalar selection.
int vtkEnumThreshold::RequestData(
                              vtkInformation *vtkNotUsed(request),
                              vtkInformationVector **inputVector,
                              vtkInformationVector *outputVector)
{
    // get the info objects
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    // get the input and ouptut
    vtkDataSet *input = vtkDataSet::SafeDownCast(
                                                 inInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
                                                                    outInfo->Get(vtkDataObject::DATA_OBJECT()));

    vtkIdType cellId, newCellId;
    vtkIdList *cellPts, *pointMap;
    vtkIdList *newCellPts;
    vtkCell *cell;
    vtkPoints *newPoints;
    int i, ptId, newId, numPts;
    int numCellPts;
    double x[3];
    vtkPointData *pd=input->GetPointData(), *outPD=output->GetPointData();
    vtkCellData *cd=input->GetCellData(), *outCD=output->GetCellData();
    int keepCell, usePointScalars;

    vtkDebugMacro(<< "Executing threshold filter");
  
    vtkDataArray *inScalars = this->GetInputArrayToProcess(0,inputVector);
  
    if (!inScalars)
    {
        vtkDebugMacro(<<"No scalar data to threshold");
        return 1;
    }

    outPD->CopyAllocate(pd);
    outCD->CopyAllocate(cd);

    numPts = input->GetNumberOfPoints();
    output->Allocate(input->GetNumberOfCells());
    newPoints = vtkPoints::New();
    newPoints->Allocate(numPts);

    pointMap = vtkIdList::New(); //maps old point ids into new
    pointMap->SetNumberOfIds(numPts);
    for (i=0; i < numPts; i++)
    {
        pointMap->SetId(i,-1);
    }

    newCellPts = vtkIdList::New();     

    // are we using pointScalars?
    usePointScalars = (inScalars->GetNumberOfTuples() == numPts);
  
    // Check that the scalars of each cell satisfy the threshold criterion
    for (cellId=0; cellId < input->GetNumberOfCells(); cellId++)
    {
        cell = input->GetCell(cellId);
        cellPts = cell->GetPointIds();
        numCellPts = cell->GetNumberOfPoints();
    
        if ( usePointScalars )
        {
            if (true) // was "this->allScalars in vtkEnumThreshold
            {
                keepCell = 1;
                for ( i=0; keepCell && (i < numCellPts); i++)
                {
                    ptId = cellPts->GetId(i);
                    keepCell = this->EvaluateComponents( inScalars, ptId );
                }
            }
        }
        else //use cell scalars
        {
            keepCell = this->EvaluateComponents( inScalars, cellId );
        }
    
        if (  numCellPts > 0 && keepCell )
        {
            // satisfied thresholding (also non-empty cell, i.e. not VTK_EMPTY_CELL)
            for (i=0; i < numCellPts; i++)
            {
                ptId = cellPts->GetId(i);
                if ( (newId = pointMap->GetId(ptId)) < 0 )
                {
                    input->GetPoint(ptId, x);
                    newId = newPoints->InsertNextPoint(x);
                    pointMap->SetId(ptId,newId);
                    outPD->CopyData(pd,ptId,newId);
                }
                newCellPts->InsertId(i,newId);
            }
            newCellId = output->InsertNextCell(cell->GetCellType(),newCellPts);
            outCD->CopyData(cd,cellId,newCellId);
            newCellPts->Reset();
        } // satisfied thresholding
    } // for all cells

    vtkDebugMacro(<< "Extracted " << output->GetNumberOfCells() 
                  << " number of cells.");

    // now clean up / update ourselves
    pointMap->Delete();
    newCellPts->Delete();
  
    output->SetPoints(newPoints);
    newPoints->Delete();

    output->Squeeze();

    return 1;
}

//  Modifications:  
//    Jeremy Meredith, Tue Aug 22 16:20:41 EDT 2006
//    Taken from 5.0.0.a vtkThreshold source, renamed to vtkEnumThreshold, and
//    made it work on an enumerated scalar selection.
int vtkEnumThreshold::EvaluateComponents( vtkDataArray *scalars, vtkIdType id )
{
    int numComp = scalars->GetNumberOfComponents();
    int c;

    double val = scalars->GetComponent(id, 0);
    int keepCell = false;
    if (val >= 0 && val <= maxEnumerationValue && enumerationMap[int(val)])
        keepCell = true;

    return keepCell;
}

int vtkEnumThreshold::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}

void vtkEnumThreshold::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


//  Modifications:  
//    Jeremy Meredith, Tue Aug 22 16:20:41 EDT 2006
//    Taken from 5.0.0.a vtkThreshold source, renamed to vtkEnumThreshold, and
//    made it work on an enumerated scalar selection.
void vtkEnumThreshold::SetEnumerationValues(const std::vector<int> &vals)
{
    enumerationValues = vals;
}

//  Modifications:  
//    Jeremy Meredith, Tue Aug 22 16:20:41 EDT 2006
//    Taken from 5.0.0.a vtkThreshold source, renamed to vtkEnumThreshold, and
//    made it work on an enumerated scalar selection.
void vtkEnumThreshold::SetEnumerationSelection(const std::vector<bool> &sel)
{
    enumerationSelection = sel;

    if (enumerationMap)
    {
        delete[] enumerationMap;
        enumerationMap = NULL;
    }

    int i;
    maxEnumerationValue = 0;
    for (i=0; i<enumerationValues.size(); i++)
    {
        if (enumerationValues[i] < 0)
        {
            vtkErrorMacro(<<"Negative values in enumeration are not allowed.");
            return;
        }
        if (enumerationValues[i] > maxEnumerationValue)
            maxEnumerationValue = enumerationValues[i];
    }
    if (maxEnumerationValue > 1e7)
    {
        vtkErrorMacro(<<"Extraordinarily large value in enumeration (>1e7).");
        return;
    }

    enumerationMap = new unsigned char[maxEnumerationValue+1];
    for (i=0; i<=maxEnumerationValue; i++)
    {
        enumerationMap[i] = 0;
    }

    for (i=0; i<enumerationValues.size(); i++)
    {
        if (enumerationSelection[i])
            enumerationMap[enumerationValues[i]] = 1;
    }
}

