/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItCutter.cxx,v $
  Language:  C++
  Date:      $Date: 2003/09/23 15:02:08 $
  Version:   $Revision: 1.73 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkVisItCutter.h"

#include <visit-config.h> // For LIB_VERSION_LE

#include <vtkCellArray.h>
#if LIB_VERSION_GE(VTK, 9,1,0)
#include <vtkCellArrayIterator.h>
#endif
#include <vtkCellData.h>
#include <vtkContourValues.h>
#include <vtkDataSet.h>
#include <vtkDoubleArray.h>
#include <vtkGenericCell.h>
#include <vtkImplicitFunction.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMergePoints.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVisItUtility.h>

#include <math.h>

vtkStandardNewMacro(vtkVisItCutter);
vtkCxxSetObjectMacro(vtkVisItCutter,CutFunction,vtkImplicitFunction);

void CellContour(vtkCell *cell, double value, vtkDataArray *cellScalars,
                 vtkPointLocator *locator, vtkCellArray *newVerts,
                 vtkCellArray *newLines, vtkCellArray *newPolys,
                 vtkPointData *inPd, vtkPointData *outPd,
                 vtkCellData *inCd, vtkIdType cellId,
                 vtkCellData *vert_outCd, vtkCellData *line_outCd,
                 vtkCellData *poly_outCd);

// Construct with user-specified implicit function; initial value of 0.0; and
// generating cut scalars turned off.
vtkVisItCutter::vtkVisItCutter(vtkImplicitFunction *cf)
{
  this->ContourValues = vtkContourValues::New();
  this->SortBy = VTK_SORT_BY_VALUE;
  this->CutFunction = cf;
  this->GenerateCutScalars = false;
  this->Locator = NULL;
}

vtkVisItCutter::~vtkVisItCutter()
{
  this->ContourValues->Delete();
  this->SetCutFunction(NULL);
  if ( this->Locator )
    {
    this->Locator->UnRegister(this);
    this->Locator = NULL;
    }
}

// Overload standard modified time function. If cut functions is modified,
// or contour values modified, then this object is modified as well.
vtkMTimeType
vtkVisItCutter::GetMTime()
{
  vtkMTimeType mTime = this->vtkPolyDataAlgorithm::GetMTime();
  vtkMTimeType contourValuesMTime = this->ContourValues->GetMTime();
  vtkMTimeType time;

  mTime = ( contourValuesMTime > mTime ? contourValuesMTime : mTime );

  if ( this->CutFunction != NULL )
    {
    time = this->CutFunction->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }

  if ( this->Locator != NULL )
    {
    time = this->Locator->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }

  return mTime;
}

// ****************************************************************************
//  Method: vtkLineoutFilter::RequestData.
//
//  Purpose:
//    Cut through data generating surface.
//
//  Modifications:
//    Eric Brugger, Thu Jan 10 11:47:09 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
// ****************************************************************************

int
vtkVisItCutter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  vtkDebugMacro(<< "Executing cutter");

  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  //
  // Initialize some frequently used values.
  //
  input  = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (!input)
    {
    vtkErrorMacro("No input specified");
    return 1;
    }

  if (!this->CutFunction)
    {
    vtkErrorMacro("No cut function specified");
    return 1;
    }

  if ( input->GetNumberOfPoints() < 1 )
    {
    vtkErrorMacro("Input data set is empty");
    return 1;
    }

  if (input->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    {
    vtkDebugMacro(<< "Executing Unstructured Grid Cutter");
    this->UnstructuredGridCutter();
    }
  else
    {
    vtkDebugMacro(<< "Executing DataSet Cutter");
    this->DataSetCutter();
    }

  return 1;
}

// ****************************************************************************
//  Method: vtkVisItCutter::FillInputPortInformation
//
// ****************************************************************************

int
vtkVisItCutter::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}

// ***************************************************************************
//  Method: vtkVisItCutter::DataSetCutter
//
//  Modifications:
//    Kathleen Bonnell, Wed Apr 27 18:47:18 PDT 2005
//    Call new method ContourCell, so that CellData can be handled correctly.
//    Concatenate the different output CellData Objects in the proper order:
//    Verts, Lines, Polys.
//
//    Eric Brugger, Thu Jan 10 11:47:09 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
// ***************************************************************************

void
vtkVisItCutter::DataSetCutter()
{
  vtkIdType cellId, i;
  int iter;
  vtkPoints *cellPts;
  vtkDoubleArray *cellScalars;
  vtkGenericCell *cell;
  vtkCellArray *newVerts, *newLines, *newPolys;
  vtkPoints *newPoints;
  vtkDoubleArray *cutScalars;
  double value, s;
  vtkIdType estimatedSize, numCells=input->GetNumberOfCells();
  vtkIdType numPts=input->GetNumberOfPoints();
  int numCellPts;
  vtkPointData *inPD, *outPD;
  vtkCellData *inCD=input->GetCellData(), *outCD=output->GetCellData();
  vtkCellData *vert_outCD = vtkCellData::New();
  vtkCellData *line_outCD = vtkCellData::New();
  vtkCellData *poly_outCD = vtkCellData::New();
  vtkIdList *cellIds;
  int numContours=this->ContourValues->GetNumberOfContours();
  int abortExecute=0;


  cellScalars=vtkDoubleArray::New();

  // Create objects to hold output of contour operation
  //
  estimatedSize = (vtkIdType) pow ((double) numCells, .75) * numContours;
  estimatedSize = estimatedSize / 1024 * 1024; //multiple of 1024
  if (estimatedSize < 1024)
    {
    estimatedSize = 1024;
    }

  newPoints = vtkVisItUtility::NewPoints(input);
  newPoints->Allocate(estimatedSize,estimatedSize/2);
  newVerts = vtkCellArray::New();
  newVerts->Allocate(estimatedSize,estimatedSize/2);
  newLines = vtkCellArray::New();
  newLines->Allocate(estimatedSize,estimatedSize/2);
  newPolys = vtkCellArray::New();
  newPolys->Allocate(estimatedSize,estimatedSize/2);
  cutScalars = vtkDoubleArray::New();
  cutScalars->SetNumberOfTuples(numPts);


  // Interpolate data along edge. If generating cut scalars, do necessary setup
  if ( this->GenerateCutScalars )
    {
    inPD = vtkPointData::New();
    inPD->ShallowCopy(input->GetPointData());//copies original attributes
    inPD->SetScalars(cutScalars);
    }
  else
    {
    inPD = input->GetPointData();
    }
  outPD = output->GetPointData();
  outPD->InterpolateAllocate(inPD,estimatedSize,estimatedSize/2);
  outCD->CopyAllocate(inCD,estimatedSize,estimatedSize/2);
  vert_outCD->CopyAllocate(inCD,estimatedSize,estimatedSize/2);
  line_outCD->CopyAllocate(inCD,estimatedSize,estimatedSize/2);
  poly_outCD->CopyAllocate(inCD,estimatedSize,estimatedSize/2);

  // locator used to merge potentially duplicate points
  if ( this->Locator == NULL )
    {
    this->CreateDefaultLocator();
    }
  this->Locator->InitPointInsertion (newPoints, input->GetBounds());

  // Loop over all points evaluating scalar function at each point
  //
  for ( i=0; i < numPts; i++ )
    {
    s = this->CutFunction->FunctionValue(input->GetPoint(i));
    cutScalars->SetComponent(i,0,s);
    }

  // Compute some information for progress methods
  //
  cell = vtkGenericCell::New();
  vtkIdType numCuts = numContours*numCells;
  vtkIdType progressInterval = numCuts/20 + 1;
  int cut=0;

  if ( this->SortBy == VTK_SORT_BY_CELL )
    {
    // Loop over all contour values.  Then for each contour value,
    // loop over all cells.
    //
    for (iter=0; iter < numContours && !abortExecute; iter++)
      {
      // Loop over all cells; get scalar values for all cell points
      // and process each cell.
      //
      for (cellId=0; cellId < numCells && !abortExecute; cellId++)
        {
        if ( !(++cut % progressInterval) )
          {
          vtkDebugMacro(<<"Cutting #" << cut);
          this->UpdateProgress ((double)cut/numCuts);
          abortExecute = this->GetAbortExecute();
          }

        input->GetCell(cellId,cell);
        cellPts = cell->GetPoints();
        cellIds = cell->GetPointIds();

        numCellPts = cellPts->GetNumberOfPoints();
        cellScalars->SetNumberOfTuples(numCellPts);
        for (i=0; i < numCellPts; i++)
          {
          s = cutScalars->GetComponent(cellIds->GetId(i),0);
          cellScalars->SetTuple(i,&s);
          }

        value = this->ContourValues->GetValue(iter);
        CellContour(cell, value, cellScalars, this->Locator, newVerts,
                    newLines, newPolys, inPD, outPD, inCD, cellId,
                    vert_outCD, line_outCD, poly_outCD);

        } // for all cells
      } // for all contour values
    } // sort by cell

  else // VTK_SORT_BY_VALUE:
    {
    // Loop over all cells; get scalar values for all cell points
    // and process each cell.
    //
    for (cellId=0; cellId < numCells && !abortExecute; cellId++)
      {
      input->GetCell(cellId,cell);
      cellPts = cell->GetPoints();
      cellIds = cell->GetPointIds();

      numCellPts = cellPts->GetNumberOfPoints();
      cellScalars->SetNumberOfTuples(numCellPts);
      for (i=0; i < numCellPts; i++)
        {
        s = cutScalars->GetComponent(cellIds->GetId(i),0);
        cellScalars->SetTuple(i,&s);
        }

      // Loop over all contour values.
      for (iter=0; iter < numContours && !abortExecute; iter++)
        {
        if ( !(++cut % progressInterval) )
          {
          vtkDebugMacro(<<"Cutting #" << cut);
          this->UpdateProgress ((double)cut/numCuts);
          abortExecute = this->GetAbortExecute();
          }
        value = this->ContourValues->GetValue(iter);
        CellContour(cell, value, cellScalars, this->Locator, newVerts,
                    newLines, newPolys, inPD, outPD, inCD, cellId,
                    vert_outCD, line_outCD, poly_outCD);
        } // for all contour values
      } // for all cells
    } // sort by value

  // Update ourselves.  Because we don't know upfront how many verts, lines,
  // polys we've created, take care to reclaim memory.
  //
  cell->Delete();
  cellScalars->Delete();
  cutScalars->Delete();

  if ( this->GenerateCutScalars )
    {
    inPD->Delete();
    }

  output->SetPoints(newPoints);
  newPoints->Delete();

  vtkIdType newCellId = 0;
  if (newVerts->GetNumberOfCells())
    {
    output->SetVerts(newVerts);
    for (i = 0; i < newVerts->GetNumberOfCells(); i++, newCellId++)
        outCD->CopyData(vert_outCD, i, newCellId);
    }
  newVerts->Delete();
  vert_outCD->Delete();

  if (newLines->GetNumberOfCells())
    {
    output->SetLines(newLines);
    for (i = 0; i < newLines->GetNumberOfCells(); i++, newCellId++)
        outCD->CopyData(line_outCD, i, newCellId);
    }
  newLines->Delete();
  line_outCD->Delete();

  if (newPolys->GetNumberOfCells())
    {
    output->SetPolys(newPolys);
    for (i = 0; i < newPolys->GetNumberOfCells(); i++, newCellId++)
        outCD->CopyData(poly_outCD, i, newCellId);
    }
  newPolys->Delete();
  poly_outCD->Delete();

  this->Locator->Initialize();//release any extra memory
  output->Squeeze();
}

// ***************************************************************************
//  Method: vtkVisItCutter::UnstructuredGridCutter
//
//  Modifications:
//    Kathleen Bonnell, Fri May 13 15:03:26 PDT 2005
//    Fix memory leak.
//
//    Eric Brugger, Thu Jan 10 11:47:09 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
//    Kathleen Biagas, Thu Aug 11, 2022
//    Support VTK9: use vtkCellArrayIterator.
//
// ***************************************************************************

void
vtkVisItCutter::UnstructuredGridCutter()
{
  vtkDataArray *cellScalars;
  vtkCellArray *newVerts, *newLines, *newPolys;
  vtkPoints *newPoints;
  vtkDoubleArray *cutScalars;
  double value, s;
  vtkIdType estimatedSize, numCells=input->GetNumberOfCells();
  vtkIdType numPts=input->GetNumberOfPoints();
  vtkPointData *inPD, *outPD;
  vtkCellData *inCD=input->GetCellData(), *outCD=output->GetCellData();
  vtkCellData *vert_outCD = vtkCellData::New();
  vtkCellData *line_outCD = vtkCellData::New();
  vtkCellData *poly_outCD = vtkCellData::New();
  int numContours = this->ContourValues->GetNumberOfContours();
  int abortExecute = 0;

  double range[2];


  // Create objects to hold output of contour operation
  //
  estimatedSize = (vtkIdType) pow ((double) numCells, .75) * numContours;
  estimatedSize = estimatedSize / 1024 * 1024; //multiple of 1024
  if (estimatedSize < 1024)
    {
      estimatedSize = 1024;
    }

  newPoints = vtkVisItUtility::NewPoints(input);
  newPoints->Allocate(estimatedSize,estimatedSize/2);
  newVerts = vtkCellArray::New();
  newVerts->Allocate(estimatedSize,estimatedSize/2);
  newLines = vtkCellArray::New();
  newLines->Allocate(estimatedSize,estimatedSize/2);
  newPolys = vtkCellArray::New();
  newPolys->Allocate(estimatedSize,estimatedSize/2);
  cutScalars = vtkDoubleArray::New();
  cutScalars->SetNumberOfTuples(numPts);

  // Interpolate data along edge. If generating cut scalars, do necessary setup
  if ( this->GenerateCutScalars )
    {
    inPD = vtkPointData::New();
    inPD->ShallowCopy(input->GetPointData());//copies original attributes
    inPD->SetScalars(cutScalars);
    }
  else
    {
    inPD = input->GetPointData();
    }
  outPD = output->GetPointData();
  outPD->InterpolateAllocate(inPD,estimatedSize,estimatedSize/2);
  outCD->CopyAllocate(inCD,estimatedSize,estimatedSize/2);
  vert_outCD->CopyAllocate(inCD,estimatedSize,estimatedSize/2);
  line_outCD->CopyAllocate(inCD,estimatedSize,estimatedSize/2);
  poly_outCD->CopyAllocate(inCD,estimatedSize,estimatedSize/2);

  // locator used to merge potentially duplicate points
  if ( this->Locator == NULL )
    {
    this->CreateDefaultLocator();
    }
  this->Locator->InitPointInsertion (newPoints, input->GetBounds());

  // Loop over all points evaluating scalar function at each point
  //
  for ( vtkIdType i=0; i < numPts; i++ )
    {
    s = this->CutFunction->FunctionValue(input->GetPoint(i));
    cutScalars->SetComponent(i,0,s);
    }

  // Compute some information for progress methods
  //
  vtkIdType numCuts = numContours*numCells;
  vtkIdType progressInterval = numCuts/20 + 1;
  int cut=0;

  vtkUnstructuredGrid *grid = vtkUnstructuredGrid::SafeDownCast(input);
#if LIB_VERSION_LE(VTK, 8,1,0)
  vtkIdType *cellArrayPtr = grid->GetCells()->GetPointer();
#else
  auto cellIter = vtk::TakeSmartPointer(grid->GetCells()->NewIterator());
  vtkIdType numCellPts;
  const vtkIdType *ptIds;
#endif
  double *scalarArrayPtr = cutScalars->GetPointer(0);
  double tempScalar;
  cellScalars = cutScalars->NewInstance();
  cellScalars->SetNumberOfComponents(cutScalars->GetNumberOfComponents());
  cellScalars->Allocate(VTK_CELL_SIZE*cutScalars->GetNumberOfComponents());

  if ( this->SortBy == VTK_SORT_BY_CELL )
    {
    // Loop over all contour values.  Then for each contour value,
    // loop over all cells.
    //
    for (int iter=0; iter < numContours && !abortExecute; iter++)
      {
      // Loop over all cells; get scalar values for all cell points
      // and process each cell.
      //
#if LIB_VERSION_LE(VTK, 8,1,0)
      for (vtkIdType cellId=0, cellArrayIt=0; cellId < numCells && !abortExecute; cellId++)
#else
      for (cellIter->GoToFirstCell(); !cellIter->IsDoneWithTraversal() && !abortExecute; cellIter->GoToNextCell())
#endif
        {
        if ( !(++cut % progressInterval) )
          {
          vtkDebugMacro(<<"Cutting #" << cut);
          this->UpdateProgress ((double)cut/numCuts);
          abortExecute = this->GetAbortExecute();
          }
#if LIB_VERSION_LE(VTK, 8,1,0)
        vtkIdType numCellPts = cellArrayPtr[cellArrayIt];
        cellArrayIt++;

        //find min and max values in scalar data
        range[0] = scalarArrayPtr[cellArrayPtr[cellArrayIt]];
        range[1] = scalarArrayPtr[cellArrayPtr[cellArrayIt]];
        cellArrayIt++;

        for (vtkIdType i = 1; i < numCellPts; i++)
          {
          tempScalar = scalarArrayPtr[cellArrayPtr[cellArrayIt]];
          cellArrayIt++;
#else
        cellIter->GetCurrentCell(numCellPts, ptIds);

        //find min and max values in scalar data
        range[0] = scalarArrayPtr[ptIds[0]];
        range[1] = scalarArrayPtr[ptIds[0]];

        for (vtkIdType i = 1; i < numCellPts; i++)
          {
          tempScalar = scalarArrayPtr[ptIds[i]];
#endif
          if (tempScalar <= range[0])
            {
            range[0] = tempScalar;
            } //if tempScalar <= min range value
          if (tempScalar >= range[1])
            {
            range[1] = tempScalar;
            } //if tempScalar >= max range value
          } // for all points in this cell

        int needCell = 0;
        double val = this->ContourValues->GetValue(iter);
        if (val >= range[0] && val <= range[1])
          {
          needCell = 1;
          }

        if (needCell)
          {
#if LIB_VERSION_GE(VTK, 9,1,0)
          vtkIdType cellId = cellIter->GetCurrentCellId();
#endif
          vtkCell *cell = input->GetCell(cellId);
          vtkIdList *cellIds = cell->GetPointIds();
          cutScalars->GetTuples(cellIds,cellScalars);
          // Loop over all contour values.
          for (int iter2=0; iter2 < numContours && !abortExecute; iter2++)
            {
            if ( !(++cut % progressInterval) )
              {
              vtkDebugMacro(<<"Cutting #" << cut);
              this->UpdateProgress ((double)cut/numCuts);
              abortExecute = this->GetAbortExecute();
              }
            value = this->ContourValues->GetValue(iter2);
            CellContour(cell, value, cellScalars, this->Locator, newVerts,
                    newLines, newPolys, inPD, outPD, inCD, cellId,
                    vert_outCD, line_outCD, poly_outCD);
            }
          }

        } // for all cells
      } // for all contour values
    } // sort by cell

  else // SORT_BY_VALUE:
    {
    // Loop over all cells; get scalar values for all cell points
    // and process each cell.
    //
#if LIB_VERSION_LE(VTK, 8,1,0)
    for (vtkIdType cellId=0, cellArrayIt=0; cellId < numCells && !abortExecute; cellId++)
      {
      vtkIdType numCellPts = cellArrayPtr[cellArrayIt];
      cellArrayIt++;

      //find min and max values in scalar data
      range[0] = scalarArrayPtr[cellArrayPtr[cellArrayIt]];
      range[1] = scalarArrayPtr[cellArrayPtr[cellArrayIt]];
      cellArrayIt++;

      for (vtkIdType i = 1; i < numCellPts; i++)
        {
        tempScalar = scalarArrayPtr[cellArrayPtr[cellArrayIt]];
        cellArrayIt++;
#else
    for (cellIter->GoToFirstCell(); !cellIter->IsDoneWithTraversal() && !abortExecute; cellIter->GoToNextCell())
      {
      cellIter->GetCurrentCell(numCellPts, ptIds);

      //find min and max values in scalar data
      range[0] = scalarArrayPtr[ptIds[0]];
      range[1] = scalarArrayPtr[ptIds[0]];

      for (vtkIdType i = 1; i < numCellPts; i++)
        {
        tempScalar = scalarArrayPtr[ptIds[i]];
#endif
        if (tempScalar <= range[0])
          {
          range[0] = tempScalar;
          } //if tempScalar <= min range value
        if (tempScalar >= range[1])
          {
          range[1] = tempScalar;
          } //if tempScalar >= max range value
        } // for all points in this cell

      int needCell = 0;
      for (int cont = 0; cont < numContours; ++cont)
        {
        double val = this->ContourValues->GetValue(cont);
        if (val >= range[0] && val <= range[1])
          {
          needCell = 1;
          break;
          }
        }

      if (needCell)
        {
#if LIB_VERSION_GE(VTK, 9,1,0)
        vtkIdType cellId = cellIter->GetCurrentCellId();
#endif
        vtkCell *cell = input->GetCell(cellId);
        vtkIdList *cellIds = cell->GetPointIds();
        cutScalars->GetTuples(cellIds,cellScalars);
        // Loop over all contour values.
        for (int iter=0; iter < numContours && !abortExecute; iter++)
          {
          if ( !(++cut % progressInterval) )
            {
            vtkDebugMacro(<<"Cutting #" << cut);
            this->UpdateProgress ((double)cut/numCuts);
            abortExecute = this->GetAbortExecute();
            }
          value = this->ContourValues->GetValue(iter);

          CellContour(cell, value, cellScalars, this->Locator, newVerts,
                      newLines, newPolys, inPD, outPD, inCD, cellId,
                      vert_outCD, line_outCD, poly_outCD);
          } // for all contour values

        } // if need cell
      } // for all cells
    } // sort by value

  // Update ourselves.  Because we don't know upfront how many verts, lines,
  // polys we've created, take care to reclaim memory.
  //
  cellScalars->Delete();
  cutScalars->Delete();

  if ( this->GenerateCutScalars )
    {
    inPD->Delete();
    }

  output->SetPoints(newPoints);
  newPoints->Delete();

  int newCellId = 0;
  if (newVerts->GetNumberOfCells())
    {
    output->SetVerts(newVerts);
    for (vtkIdType i = 0; i < newVerts->GetNumberOfCells(); i++, newCellId++)
        outCD->CopyData(vert_outCD, i, newCellId);
    }
  newVerts->Delete();
  vert_outCD->Delete();

  if (newLines->GetNumberOfCells())
    {
    output->SetLines(newLines);
    for (vtkIdType i = 0; i < newLines->GetNumberOfCells(); i++, newCellId++)
        outCD->CopyData(line_outCD, i, newCellId);
    }
  newLines->Delete();
  line_outCD->Delete();

  if (newPolys->GetNumberOfCells())
    {
    output->SetPolys(newPolys);
    for (vtkIdType i = 0; i < newPolys->GetNumberOfCells(); i++, newCellId++)
        outCD->CopyData(poly_outCD, i, newCellId);
    }
  newPolys->Delete();
  poly_outCD->Delete();

  this->Locator->Initialize();//release any extra memory
  output->Squeeze();
}


// Specify a spatial locator for merging points. By default,
// an instance of vtkMergePoints is used.

void
vtkVisItCutter::SetLocator(vtkPointLocator *locator)
{
  if ( this->Locator == locator )
    {
    return;
    }
  if ( this->Locator )
    {
    this->Locator->UnRegister(this);
    this->Locator = NULL;
    }
  if ( locator )
    {
    locator->Register(this);
    }
  this->Locator = locator;
  this->Modified();
}

void vtkVisItCutter::CreateDefaultLocator()
{
  if ( this->Locator == NULL )
    {
    this->Locator = vtkMergePoints::New();
    this->Locator->Register(this);
    this->Locator->Delete();
    }
}

// ***************************************************************************
//  Function: CellContour
//
//  Modifications:
//    Kathleen Bonnell, Wed Apr 27 18:47:18 PDT 2005
//    Using 'Contour' can create havoc with the CellData array, because
//    cells returning different contour cell types can overwrite the data
//    previsouly generated.  Create separate vtkCellData's for each cell
//    type generated (verts, lines, cells), send the appropriate one to
//    each call to Contour (based on celltype being used).  Concatenate them
//    at the end in the proper order: Verts, Lines, Polys.
//
//    Kathleen Bonnell, Fri Jun  9 07:11:34 PDT 2006
//    Kitware changed the 'contour' method for each cell type, but their
//    fix still doesn't handle out-of-order cells.  Pass an empty cell array
//    when appropriate, to work around their 'fix' (and prevent possible MSE).
//
//    Hank Childs, Sat Sep 29 07:42:27 PDT 2007
//    Previously, each of the known VTK cell types were enumerated.  This
//    created problems when new cell types were encountered (specifically for
//    me with a hex-20).  Instead of enumerating cell types, focus on their
//    cell dimension.
//
// ***************************************************************************

void
CellContour(vtkCell *cell, double value,
            vtkDataArray *cellScalars,
            vtkPointLocator *locator,
            vtkCellArray *newVerts,
            vtkCellArray *newLines,
            vtkCellArray *newPolys,
            vtkPointData *inPD, vtkPointData *outPD,
            vtkCellData *inCD, vtkIdType cellId,
            vtkCellData *vert_outCD,
            vtkCellData *line_outCD,
            vtkCellData *poly_outCD)
{
  vtkCellArray *empty = vtkCellArray::New();
  int nP = newPolys->GetNumberOfCells();
  switch (cell->GetCellDimension())
    {
    case 0:
    case 1:
      // These cell types only create verts when contoured, so
      // pass the contour method the outCD for verts.
      cell->Contour(value, cellScalars, locator, newVerts, empty,
                    empty, inPD, outPD, inCD, cellId, vert_outCD);
      break;

    case 2:
      // These cell types only create lines when contoured, so
      // pass the contour method the outCD for lines.
      cell->Contour(value, cellScalars, locator, empty, newLines,
                    empty, inPD, outPD, inCD, cellId, line_outCD);
      break;

    case 3:
      // These cell types only create polys when contoured, so
      // pass the contour method the outCD for polys.
      cell->Contour(value, cellScalars, locator, empty, empty,
                    newPolys, inPD, outPD, inCD, cellId, poly_outCD);
      break;

    default:
      break;
    } // switch cell-type

    // Work around bug in VTK 5.0.  You should be able to remove this code
    // when VTK is upgraded.
    if (cell->GetCellType() == VTK_QUADRATIC_HEXAHEDRON)
    {
      int newNP = newPolys->GetNumberOfCells();
      for (int i = nP ; i < newNP ; i++)
        poly_outCD->CopyData(inCD, cellId, i);
    }
    // End work around

    empty->Delete();
}

// ****************************************************************************
//  Method: vtkVisItCutter::PrintSelf
//
// ****************************************************************************

void vtkVisItCutter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Cut Function: " << this->CutFunction << "\n";

  os << indent << "Sort By: " << this->GetSortByAsString() << "\n";

  if ( this->Locator )
    {
    os << indent << "Locator: " << this->Locator << "\n";
    }
  else
    {
    os << indent << "Locator: (none)\n";
    }

  this->ContourValues->PrintSelf(os,indent);

  os << indent << "Generate Cut Scalars: "
     << (this->GenerateCutScalars ? "On\n" : "Off\n");
}
