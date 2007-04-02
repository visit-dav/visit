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

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkContourValues.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkGenericCell.h>
#include <vtkImplicitFunction.h>
#include <vtkMergePoints.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>

#include <math.h>

vtkCxxRevisionMacro(vtkVisItCutter, "$Revision: 1.73 $");
vtkStandardNewMacro(vtkVisItCutter);
vtkCxxSetObjectMacro(vtkVisItCutter,CutFunction,vtkImplicitFunction);

void CellContour(vtkCell *cell, float value, vtkDataArray *cellScalars,
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
  this->GenerateCutScalars = 0;
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
unsigned long vtkVisItCutter::GetMTime()
{
  unsigned long mTime=this->vtkDataSetToPolyDataFilter::GetMTime();
  unsigned long contourValuesMTime=this->ContourValues->GetMTime();
  unsigned long time;
 
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

// Cut through data generating surface.
//
void vtkVisItCutter::Execute()
{

  vtkDebugMacro(<< "Executing cutter");

  vtkDataSet *input = this->GetInput();

  if (!input)
    {
    vtkErrorMacro("No input specified");
    return;
    }
  
  if (!this->CutFunction)
    {
    vtkErrorMacro("No cut function specified");
    return;
    }

  if ( input->GetNumberOfPoints() < 1 )
    {
    vtkErrorMacro("Input data set is empty");
    return;
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
}

// ***************************************************************************
//  Modifications:
//    Kathleen Bonnell, Wed Apr 27 18:47:18 PDT 2005
//    Call new method ContourCell, so that CellData can be handled correctly.
//    Concatenate the different output CellData Objects in the proper order: 
//    Verts, Lines, Polys.
//
// ***************************************************************************
void vtkVisItCutter::DataSetCutter()
{
  vtkIdType cellId, i;
  int iter;
  vtkPoints *cellPts;
  vtkFloatArray *cellScalars;
  vtkGenericCell *cell;
  vtkCellArray *newVerts, *newLines, *newPolys;
  vtkPoints *newPoints;
  vtkFloatArray *cutScalars;
  float value, s;
  vtkPolyData *output = this->GetOutput();
  vtkDataSet *input=this->GetInput();
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

  
  cellScalars=vtkFloatArray::New();

  // Create objects to hold output of contour operation
  //
  estimatedSize = (vtkIdType) pow ((double) numCells, .75) * numContours;
  estimatedSize = estimatedSize / 1024 * 1024; //multiple of 1024
  if (estimatedSize < 1024)
    {
    estimatedSize = 1024;
    }

  newPoints = vtkPoints::New();
  newPoints->Allocate(estimatedSize,estimatedSize/2);
  newVerts = vtkCellArray::New();
  newVerts->Allocate(estimatedSize,estimatedSize/2);
  newLines = vtkCellArray::New();
  newLines->Allocate(estimatedSize,estimatedSize/2);
  newPolys = vtkCellArray::New();
  newPolys->Allocate(estimatedSize,estimatedSize/2);
  cutScalars = vtkFloatArray::New();
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
          this->UpdateProgress ((float)cut/numCuts);
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
          this->UpdateProgress ((float)cut/numCuts);
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
//  Modifications:
//    Kathleen Bonnell, Fri May 13 15:03:26 PDT 2005
//    Fix memory leak.
//
// ***************************************************************************

void vtkVisItCutter::UnstructuredGridCutter()
{
  vtkIdType cellId, i;
  int iter;
  vtkFloatArray *cellScalars;
  vtkCellArray *newVerts, *newLines, *newPolys;
  vtkPoints *newPoints;
  vtkFloatArray *cutScalars;
  float value, s;
  vtkPolyData *output = this->GetOutput();
  vtkDataSet *input = this->GetInput();
  vtkIdType estimatedSize, numCells=input->GetNumberOfCells();
  vtkIdType numPts=input->GetNumberOfPoints();
  vtkIdType cellArrayIt = 0;
  int numCellPts;
  vtkPointData *inPD, *outPD;
  vtkCellData *inCD=input->GetCellData(), *outCD=output->GetCellData();
  vtkCellData *vert_outCD = vtkCellData::New();
  vtkCellData *line_outCD = vtkCellData::New();
  vtkCellData *poly_outCD = vtkCellData::New();
  vtkIdList *cellIds;
  int numContours = this->ContourValues->GetNumberOfContours();
  int abortExecute = 0;

  float range[2];


  // Create objects to hold output of contour operation
  //
  estimatedSize = (vtkIdType) pow ((double) numCells, .75) * numContours;
  estimatedSize = estimatedSize / 1024 * 1024; //multiple of 1024
  if (estimatedSize < 1024)
    {
      estimatedSize = 1024;
    }

  newPoints = vtkPoints::New();
  newPoints->Allocate(estimatedSize,estimatedSize/2);
  newVerts = vtkCellArray::New();
  newVerts->Allocate(estimatedSize,estimatedSize/2);
  newLines = vtkCellArray::New();
  newLines->Allocate(estimatedSize,estimatedSize/2);
  newPolys = vtkCellArray::New();
  newPolys->Allocate(estimatedSize,estimatedSize/2);
  cutScalars = vtkFloatArray::New();
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
  vtkIdType numCuts = numContours*numCells;
  vtkIdType progressInterval = numCuts/20 + 1;
  int cut=0;

  vtkUnstructuredGrid *grid = (vtkUnstructuredGrid *)input;
  vtkIdType *cellArrayPtr = grid->GetCells()->GetPointer();
  float *scalarArrayPtr = cutScalars->GetPointer(0);
  float tempScalar;
  cellScalars = cutScalars->NewInstance();
  cellScalars->SetNumberOfComponents(cutScalars->GetNumberOfComponents());
  cellScalars->Allocate(VTK_CELL_SIZE*cutScalars->GetNumberOfComponents());
 
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
          this->UpdateProgress ((float)cut/numCuts);
          abortExecute = this->GetAbortExecute();
          }

        numCellPts = cellArrayPtr[cellArrayIt];
        cellArrayIt++;
        
        //find min and max values in scalar data
        range[0] = scalarArrayPtr[cellArrayPtr[cellArrayIt]];
        range[1] = scalarArrayPtr[cellArrayPtr[cellArrayIt]];
        cellArrayIt++;
        
        for (i = 1; i < numCellPts; i++)
          {
          tempScalar = scalarArrayPtr[cellArrayPtr[cellArrayIt]];
          cellArrayIt++;
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
        float val = this->ContourValues->GetValue(iter);
        if (val >= range[0] && val <= range[1]) 
          {
          needCell = 1;
          }

        if (needCell) 
          {
          vtkCell *cell = input->GetCell(cellId);
          cellIds = cell->GetPointIds();
          cutScalars->GetTuples(cellIds,cellScalars);
          // Loop over all contour values.
          for (iter=0; iter < numContours && !abortExecute; iter++)
            {
            if ( !(++cut % progressInterval) )
              {
              vtkDebugMacro(<<"Cutting #" << cut);
              this->UpdateProgress ((float)cut/numCuts);
              abortExecute = this->GetAbortExecute();
              }
            value = this->ContourValues->GetValue(iter);
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
    for (cellId=0; cellId < numCells && !abortExecute; cellId++)
      {
      numCellPts = cellArrayPtr[cellArrayIt];
      cellArrayIt++;
          
      //find min and max values in scalar data
      range[0] = scalarArrayPtr[cellArrayPtr[cellArrayIt]];
      range[1] = scalarArrayPtr[cellArrayPtr[cellArrayIt]];
      cellArrayIt++;
          
      for (i = 1; i < numCellPts; i++)
        {
        tempScalar = scalarArrayPtr[cellArrayPtr[cellArrayIt]];
        cellArrayIt++;
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
        float val = this->ContourValues->GetValue(cont);
        if (val >= range[0] && val <= range[1]) 
          {
          needCell = 1;
          break;
          }
        }
          
      if (needCell) 
        {
        vtkCell *cell = input->GetCell(cellId);
        cellIds = cell->GetPointIds();
        cutScalars->GetTuples(cellIds,cellScalars);
        // Loop over all contour values.
        for (iter=0; iter < numContours && !abortExecute; iter++)
          {
          if ( !(++cut % progressInterval) )
            {
            vtkDebugMacro(<<"Cutting #" << cut);
            this->UpdateProgress ((float)cut/numCuts);
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


// Specify a spatial locator for merging points. By default, 
// an instance of vtkMergePoints is used.
void vtkVisItCutter::SetLocator(vtkPointLocator *locator)
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
//  Modifications:
//    Kathleen Bonnell, Wed Apr 27 18:47:18 PDT 2005
//    Using 'Contour' can create havoc with the CellData array, because 
//    cells returning different contour cell types can overwrite the data   
//    previsouly generated.  Create separate vtkCellData's for each cell
//    type generated (verts, lines, cells), send the appropriate one to
//    each call to Contour (based on celltype being used).  Concatenate them
//    at the end in the proper order: Verts, Lines, Polys.
//
// ***************************************************************************
void
CellContour(vtkCell *cell, float value, 
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
  switch (cell->GetCellType())
    {
    case VTK_VERTEX:
    case VTK_POLY_VERTEX:
    case VTK_LINE:
    case VTK_POLY_LINE:
      // These cell types only create verts when contoured, so
      // pass the contour method the outCD for verts.
      cell->Contour(value, cellScalars, locator, newVerts, newLines, 
                    newPolys, inPD, outPD, inCD, cellId, vert_outCD);
      break;
    case VTK_TRIANGLE:
    case VTK_TRIANGLE_STRIP:
    case VTK_POLYGON:
    case VTK_PIXEL:
    case VTK_QUAD:
      // These cell types only create lines when contoured, so
      // pass the contour method the outCD for lines.
      cell->Contour(value, cellScalars, locator, newVerts, newLines, 
                    newPolys, inPD, outPD, inCD, cellId, line_outCD);
      break;
    case VTK_TETRA:
    case VTK_VOXEL:
    case VTK_HEXAHEDRON:
    case VTK_WEDGE:
    case VTK_PYRAMID:
      // These cell types only create polys when contoured, so
      // pass the contour method the outCD for polys.
      cell->Contour(value, cellScalars, locator, newVerts, newLines, 
                    newPolys, inPD, outPD, inCD, cellId, poly_outCD);
      break;
    default:
      break;
    } // switch cell-type
}

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
