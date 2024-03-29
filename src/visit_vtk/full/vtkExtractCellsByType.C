/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkExtractCellsByType.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkExtractCellsByType.h"

#include <visit-config.h>

#include "vtkPoints.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkCellArray.h"
#if LIB_VERSION_GE(VTK, 9,1,0)
#include <vtkCellArrayIterator.h>
#endif
#include "vtkPointSet.h"
#include "vtkPolyData.h"
#include "vtkUnstructuredGrid.h"
#include "vtkImageData.h"
#include "vtkStructuredGrid.h"
#include "vtkRectilinearGrid.h"
#include "vtkUniformGrid.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkExtractCellsByType);

#include <set>

// Special token marks any cell type
#define VTK_ANY_CELL_TYPE 1000000

struct vtkCellTypeSet : public std::set<unsigned int>
{
};

//----------------------------------------------------------------------------
vtkExtractCellsByType::vtkExtractCellsByType()
{
  this->CellTypes = new vtkCellTypeSet;
}

//----------------------------------------------------------------------------
vtkExtractCellsByType::~vtkExtractCellsByType()
{
  delete this->CellTypes;
}

//----------------------------------------------------------------------------
void vtkExtractCellsByType::AddCellType(unsigned int cellType)
{
  auto prevSize = this->CellTypes->size();
  this->CellTypes->insert(cellType);
  if ( prevSize != this->CellTypes->size() )
  {
    this->Modified();
  }
}

//----------------------------------------------------------------------------
void vtkExtractCellsByType::RemoveCellType(unsigned int cellType)
{
  auto prevSize = this->CellTypes->size();
  this->CellTypes->erase(cellType);
  this->CellTypes->erase(VTK_ANY_CELL_TYPE);
  if ( prevSize != this->CellTypes->size() )
  {
    this->Modified();
  }
}

//----------------------------------------------------------------------------
void vtkExtractCellsByType::RemoveAllCellTypes()
{
  if ( !this->CellTypes->empty() )
  {
    this->CellTypes->clear();
    this->Modified();
  }
}

//----------------------------------------------------------------------------
// Special value indicates that all cells are to be selected. This is better
// than populating from the list vtkCellType.h due to the associated
// maintenance burden.
#if USE_VTK_VERSION_OF_ADDALLCELLTYPES
void vtkExtractCellsByType::AddAllCellTypes()
{
  auto prevSize = this->CellTypes->size();
  this->CellTypes->insert(VTK_ANY_CELL_TYPE);
  if ( prevSize != this->CellTypes->size() )
  {
    this->Modified();
  }
}
#else
// KSB, using above 'special value' version, in conjuntion with
// 'RemoveCellType', ends up with NO cell types to extract.
// In order to extract all but LINES, for example.
void vtkExtractCellsByType::AddAllCellTypes()
{
  auto prevSize = this->CellTypes->size();
  for(unsigned int i = 0; i < VTK_NUMBER_OF_CELL_TYPES; ++i)
  {
      // not all 'i' in this loop are valid cell types, but
      // it shouldn't matter for this filter.
      this->CellTypes->insert(i);
  }
  if ( prevSize != this->CellTypes->size() )
  {
    this->Modified();
  }
}
#endif

//----------------------------------------------------------------------------
#if USE_VTK_VERSION_OF_EXTRACTCELLTYPE
bool vtkExtractCellsByType::ExtractCellType(unsigned int cellType)
{
  if ( this->CellTypes->find(cellType) != this->CellTypes->end() ||
       this->CellTypes->find(VTK_ANY_CELL_TYPE) != this->CellTypes->end() )
  {
    return true;
  }
  else
  {
    return false;
  }
}
#else
bool vtkExtractCellsByType::ExtractCellType(unsigned int cellType)
{
  if ( this->CellTypes->find(cellType) != this->CellTypes->end())
  {
    return true;
  }
  else
  {
    return false;
  }
}
#endif

//----------------------------------------------------------------------------
void vtkExtractCellsByType::
ExtractUnstructuredData(vtkDataSet *inDS, vtkDataSet *outDS)
{
  vtkPointData *inPD = inDS->GetPointData();
  vtkPointData *outPD = outDS->GetPointData();

  vtkIdType numPts = inDS->GetNumberOfPoints();

  // We are going some maps to indicate where the points and cells originated
  // from. Values <0 mean that the points or cells are not mapped to the
  // output.
  vtkIdType *ptMap = new vtkIdType [numPts];
  std::fill_n(ptMap,numPts,-1);

  // Now dispatch to specific unstructured type
  vtkIdType numNewPts = 0;
  if ( inDS->GetDataObjectType() == VTK_POLY_DATA )
  {
    this->ExtractPolyDataCells(inDS, outDS, ptMap, numNewPts);
  }

  else if ( inDS->GetDataObjectType() == VTK_UNSTRUCTURED_GRID )
  {
    this->ExtractUnstructuredGridCells(inDS, outDS, ptMap, numNewPts);
  }

  // Copy referenced input points to new points array
  outPD->CopyAllocate(inPD);
  vtkPointSet *inPtSet = vtkPointSet::SafeDownCast(inDS);
  vtkPointSet *outPtSet = vtkPointSet::SafeDownCast(outDS);
  vtkPoints *inPts = inPtSet->GetPoints();
  vtkPoints *outPts = vtkPoints::New();
  outPts->SetNumberOfPoints(numNewPts);
  for (vtkIdType ptId=0; ptId < numPts; ++ptId)
  {
    if ( ptMap[ptId] >= 0 )
    {
      outPts->SetPoint(ptMap[ptId], inPts->GetPoint(ptId));
      outPD->CopyData(inPD,ptId,ptMap[ptId]);
    }
  }
  outPtSet->SetPoints(outPts);

  // Clean up
  outPts->Delete();
  delete [] ptMap;
}

//----------------------------------------------------------------------------
// KSB August 29, 2019:
//   keep track of currentInputCellId and currentOutputCellId, separately
//   from the 'cellId' used by the verts,lines, etc. This is to ensure
//   proper copying celldata from input to output.
//
// Kathleen Biagas, Thu Aug 11, 2022
// Support VTK9: use vtkCellArrayIterator.
//

void
vtkExtractCellsByType::ExtractPolyDataCells(vtkDataSet *inDS,
    vtkDataSet *outDS, vtkIdType *ptMap, vtkIdType &numNewPts)
{
  vtkPolyData *input = vtkPolyData::SafeDownCast(inDS);
  vtkCellData *inCD = input->GetCellData();
  vtkPolyData *output = vtkPolyData::SafeDownCast(outDS);
  vtkCellData *outCD = output->GetCellData();

  // Treat the four cell arrays separately. If the array might have cells of
  // the specified types, then traverse it, copying the input cells to the
  // output cells. Also keep track of the point map.

  // The cellIds are numbered across the four arrays: verts, lines, polys,
  // strips. Have to carefully coordinate the cell ids with traversal of each
  // array.
  vtkIdType cellId, currentInputCellId = 0, currentOutputCellId = 0;
  vtkIdList *ptIds = vtkIdList::New();

  // Verts
  outCD->CopyAllocate(inCD);
  vtkIdType npts;
#if LIB_VERSION_LE(VTK, 8,1,0)
  vtkIdType *pts;
#else
  const vtkIdType *pts;
#endif
  if ( this->ExtractCellType(VTK_VERTEX) || this->ExtractCellType(VTK_POLY_VERTEX) )
  {
    vtkCellArray *verts = vtkCellArray::New();
#if LIB_VERSION_LE(VTK, 8,1,0)
    vtkCellArray *inVerts = input->GetVerts();
    for ( inVerts->InitTraversal(); inVerts->GetNextCell(npts,pts); ++currentInputCellId )
    {
#else
    auto inVerts = vtk::TakeSmartPointer(input->GetVerts()->NewIterator());
    for ( inVerts->GoToFirstCell(); !inVerts->IsDoneWithTraversal(); inVerts->GoToNextCell(), ++currentInputCellId )
    {
      inVerts->GetCurrentCell(npts,pts);
#endif
      if ( this->ExtractCellType(input->GetCellType(currentInputCellId)) )
      {
        ptIds->Reset();
        for (vtkIdType i=0; i<npts; ++i)
        {
          if ( ptMap[pts[i]] < 0 )
          {
            ptMap[pts[i]] = numNewPts++;
          }
          ptIds->InsertId(i,ptMap[pts[i]]);
        }
        cellId = verts->InsertNextCell(ptIds);
        outCD->CopyData(inCD,currentInputCellId,currentOutputCellId++);
      }
    }
    output->SetVerts(verts);
    verts->Delete();
  }
  else
  {
    currentInputCellId += input->GetNumberOfVerts();
  }

  // Lines
  if ( this->ExtractCellType(VTK_LINE) || this->ExtractCellType(VTK_POLY_LINE) )
  {
    vtkCellArray *lines = vtkCellArray::New();
#if LIB_VERSION_LE(VTK, 8,1,0)
    vtkCellArray *inLines = input->GetLines();
    for ( inLines->InitTraversal(); inLines->GetNextCell(npts,pts); ++currentInputCellId )
    {
#else
    auto inLines = vtk::TakeSmartPointer(input->GetLines()->NewIterator());
    for ( inLines->GoToFirstCell(); !inLines->IsDoneWithTraversal(); inLines->GoToNextCell(), ++currentInputCellId )
    {
      inLines->GetCurrentCell(npts,pts);
#endif
      if ( this->ExtractCellType(input->GetCellType(currentInputCellId)) )
      {
        ptIds->Reset();
        for (vtkIdType i=0; i<npts; ++i)
        {
          if ( ptMap[pts[i]] < 0 )
          {
            ptMap[pts[i]] = numNewPts++;
          }
          ptIds->InsertId(i,ptMap[pts[i]]);
        }
        cellId = lines->InsertNextCell(ptIds);
        outCD->CopyData(inCD,currentInputCellId,currentOutputCellId++);
      }
    }
    output->SetLines(lines);
    lines->Delete();
  }
  else
  {
    currentInputCellId += input->GetNumberOfLines();
  }

  // Polys
  if ( this->ExtractCellType(VTK_TRIANGLE) || this->ExtractCellType(VTK_QUAD) ||
       this->ExtractCellType(VTK_POLYGON) )
  {
    vtkCellArray *polys = vtkCellArray::New();
#if LIB_VERSION_LE(VTK, 8,1,0)
    vtkCellArray *inPolys = input->GetPolys();
    for ( inPolys->InitTraversal(); inPolys->GetNextCell(npts,pts); ++currentInputCellId )
    {
#else
    auto inPolys = vtk::TakeSmartPointer(input->GetPolys()->NewIterator());
    for ( inPolys->GoToFirstCell(); !inPolys->IsDoneWithTraversal(); inPolys->GoToNextCell(), ++currentInputCellId )
    {
      inPolys->GetCurrentCell(npts,pts);
#endif
      if ( this->ExtractCellType(input->GetCellType(currentInputCellId)) )
      {
        ptIds->Reset();
        for (vtkIdType i=0; i<npts; ++i)
        {
          if ( ptMap[pts[i]] < 0 )
          {
            ptMap[pts[i]] = numNewPts++;
          }
          ptIds->InsertId(i,ptMap[pts[i]]);
        }
        cellId = polys->InsertNextCell(ptIds);
        outCD->CopyData(inCD,currentInputCellId,currentOutputCellId++);
      }
    }
    output->SetPolys(polys);
    polys->Delete();
  }
  else
  {
    currentInputCellId += input->GetNumberOfPolys();
  }

  // Triangle strips
  if ( this->ExtractCellType(VTK_TRIANGLE_STRIP) )
  {
    vtkCellArray *strips = vtkCellArray::New();
#if LIB_VERSION_LE(VTK, 8,1,0)
    vtkCellArray *inStrips = input->GetStrips();
    // All cells are of type VTK_TRIANGLE_STRIP
    for ( inStrips->InitTraversal(); inStrips->GetNextCell(npts,pts); ++currentInputCellId )
    {
#else
    auto inStrips = vtk::TakeSmartPointer(input->GetStrips()->NewIterator());
    // All cells are of type VTK_TRIANGLE_STRIP
    for ( inStrips->GoToFirstCell(); !inStrips->IsDoneWithTraversal(); inStrips->GoToNextCell(), ++currentInputCellId )
    {
      inStrips->GetCurrentCell(npts,pts);
#endif
      ptIds->Reset();
      for (vtkIdType i=0; i<npts; ++i)
      {
        if ( ptMap[pts[i]] < 0 )
        {
          ptMap[pts[i]] = numNewPts++;
        }
        ptIds->InsertId(i,ptMap[pts[i]]);
      }
      cellId = strips->InsertNextCell(ptIds);
      outCD->CopyData(inCD,currentInputCellId,cellId);
    }
    output->SetStrips(strips);
    strips->Delete();
  }

  // Clean up
  ptIds->Delete();
}

//----------------------------------------------------------------------------
void
vtkExtractCellsByType::ExtractUnstructuredGridCells(vtkDataSet *inDS,
     vtkDataSet *outDS, vtkIdType *ptMap, vtkIdType &numNewPts)
{
  vtkUnstructuredGrid *input = vtkUnstructuredGrid::SafeDownCast(inDS);
  vtkCellData *inCD = input->GetCellData();
  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(outDS);
  vtkCellData *outCD = output->GetCellData();
  vtkIdType numCells = input->GetNumberOfCells();

  // Check for trivial cases: either all in or all out
  if ( input->IsHomogeneous() )
  {
    if ( this->ExtractCellType(input->GetCellType(0)) )
    {
      output->ShallowCopy(input);
    }
    else
    {
      output->Initialize();
    }
    return;
  }

  // Mixed collection of cells so simply loop over all cells, copying
  // appropriate types to the output. Along the way keep track of the points
  // that are used.
  vtkIdType i, cellId, newCellId, npts, ptId;
  vtkIdList *ptIds = vtkIdList::New();
  int cellType;
  output->Allocate(numCells);
  outCD->CopyAllocate(inCD);
  for (cellId=0; cellId < numCells; ++cellId)
  {
    cellType = input->GetCellType(cellId);
    if ( this->ExtractCellType(cellType) )
    {
      input->GetCellPoints(cellId, ptIds);
      npts = ptIds->GetNumberOfIds();
      for (i=0; i<npts; ++i)
      {
        ptId = ptIds->GetId(i);
        if ( ptMap[ptId] < 0 )
        {
          ptMap[ptId] = numNewPts++;
        }
        ptIds->InsertId(i,ptMap[ptId]);
      }
      newCellId = output->InsertNextCell(cellType,ptIds);
      outCD->CopyData(inCD,cellId,newCellId);
    }
  }

  // Clean up
  ptIds->Delete();
}

//----------------------------------------------------------------------------
int vtkExtractCellsByType::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkDataSet *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkDataSet *output = vtkDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // Handle the trivial case
  vtkIdType numCells = input->GetNumberOfCells();
  if ( this->CellTypes->empty() || numCells <= 0 )
  {
    output->Initialize(); //output is empty
    return 1;
  }

  // Dispatch to appropriate type. This filter does not directly handle
  // composite dataset types, composite types should be looped over by
  // the pipeline executive.
  if ( input->GetDataObjectType() == VTK_POLY_DATA ||
       input->GetDataObjectType() == VTK_UNSTRUCTURED_GRID )
  {
    this->ExtractUnstructuredData(input, output);
  }

  // Structured data has only one cell type per dataset
  else if ( input->GetDataObjectType() == VTK_IMAGE_DATA ||
            input->GetDataObjectType() == VTK_STRUCTURED_POINTS ||
            input->GetDataObjectType() == VTK_RECTILINEAR_GRID ||
            input->GetDataObjectType() == VTK_STRUCTURED_GRID ||
            input->GetDataObjectType() == VTK_UNIFORM_GRID ||
            input->GetDataObjectType() == VTK_HYPER_TREE_GRID )
  {
    if ( this->ExtractCellType(input->GetCellType(0)) )
    {
      output->ShallowCopy(input);
    }
    else
    {
      output->Initialize(); //output is empty
    }
  }

  else
  {
    vtkErrorMacro("Unknown dataset type");
    output->Initialize(); //output is empty
  }

  return 1;
}

//----------------------------------------------------------------------------
int vtkExtractCellsByType::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}

//----------------------------------------------------------------------------
void vtkExtractCellsByType::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  // Output the number of types specified
  os << indent << "Number of types specified: "
     << this->CellTypes->size() << "\n";
}
