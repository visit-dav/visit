// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.
struct vtkCellTypeSet : public std::set<unsigned int>
{
};

//----------------------------------------------------------------------------
vtkRemoveCellsByType::vtkRemoveCellsByType()
{
  this->CellTypes = new vtkCellTypeSet;
}

//----------------------------------------------------------------------------
vtkRemoveCellsByType::~vtkRemoveCellsByType()
{
  delete this->CellTypes;
}


//----------------------------------------------------------------------------
void vtkRemoveCellsByType::RemoveCellType(unsigned int cellType)
{
  auto prevSize = this->CellTypes->size();
  this->CellTypes->insert(cellType);
  if ( prevSize != this->CellTypes->size() )
  {
    this->Modified();
  }
}

//----------------------------------------------------------------------------
void vtkRemoveCellsByType::ClearCellTypesToBeRemoved()
{
  if ( !this->CellTypes->empty() )
  {
    this->CellTypes->clear();
    this->Modified();
  }
}

//----------------------------------------------------------------------------
bool vtkRemoveCellsByType::ExtractCellType(unsigned int cellType)
{
  if ( this->CellTypes->find(cellType) != this->CellTypes->end())
  {
    return false;
  }
  else
  {
    return true;
  }
}

//----------------------------------------------------------------------------
void vtkRemoveCellsByType::
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
void vtkRemoveCellsByType::
ExtractPolyDataCells(vtkDataSet *inDS, vtkDataSet *outDS,
                     vtkIdType *ptMap, vtkIdType &numNewPts)
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
  vtkIdType cellId, currentCellId = 0;
  vtkIdList *ptIds = vtkIdList::New();

  // Verts
  outCD->CopyAllocate(inCD);
  vtkIdType i, npts, *pts;
  vtkCellArray *inVerts = input->GetVerts();
  if ( this->ExtractCellType(VTK_VERTEX) || this->ExtractCellType(VTK_POLY_VERTEX) )
  {
    vtkCellArray *verts = vtkCellArray::New();
    for ( inVerts->InitTraversal(); inVerts->GetNextCell(npts,pts); ++currentCellId )
    {
      if ( this->ExtractCellType(input->GetCellType(currentCellId)) )
      {
        ptIds->Reset();
        for (i=0; i<npts; ++i)
        {
          if ( ptMap[pts[i]] < 0 )
          {
            ptMap[pts[i]] = numNewPts++;
          }
          ptIds->InsertId(i,ptMap[pts[i]]);
        }
        cellId = verts->InsertNextCell(ptIds);
        outCD->CopyData(inCD,currentCellId,cellId);
      }
    }
    output->SetVerts(verts);
    verts->Delete();
  }
  else
  {
    currentCellId += inVerts->GetNumberOfCells();
  }

  // Lines
  vtkCellArray *inLines = input->GetLines();
  if ( this->ExtractCellType(VTK_LINE) || this->ExtractCellType(VTK_POLY_LINE) )
  {
    vtkCellArray *lines = vtkCellArray::New();
    for ( inLines->InitTraversal(); inLines->GetNextCell(npts,pts); ++currentCellId )
    {
      if ( this->ExtractCellType(input->GetCellType(currentCellId)) )
      {
        ptIds->Reset();
        for (i=0; i<npts; ++i)
        {
          if ( ptMap[pts[i]] < 0 )
          {
            ptMap[pts[i]] = numNewPts++;
          }
          ptIds->InsertId(i,ptMap[pts[i]]);
        }
        cellId = lines->InsertNextCell(ptIds);
        outCD->CopyData(inCD,currentCellId,cellId);
      }
    }
    output->SetLines(lines);
    lines->Delete();
  }
  else
  {
    currentCellId += inLines->GetNumberOfCells();
  }

  // Polys
  vtkCellArray *inPolys = input->GetPolys();
  if ( this->ExtractCellType(VTK_TRIANGLE) || this->ExtractCellType(VTK_QUAD) ||
       this->ExtractCellType(VTK_POLYGON) )
  {
    vtkCellArray *polys = vtkCellArray::New();
    for ( inPolys->InitTraversal(); inPolys->GetNextCell(npts,pts); ++currentCellId )
    {
      if ( this->ExtractCellType(input->GetCellType(currentCellId)) )
      {
        ptIds->Reset();
        for (i=0; i<npts; ++i)
        {
          if ( ptMap[pts[i]] < 0 )
          {
            ptMap[pts[i]] = numNewPts++;
          }
          ptIds->InsertId(i,ptMap[pts[i]]);
        }
        cellId = polys->InsertNextCell(ptIds);
        outCD->CopyData(inCD,currentCellId,cellId);
      }
    }
    output->SetPolys(polys);
    polys->Delete();
  }
  else
  {
    currentCellId += inPolys->GetNumberOfCells();
  }

  // Triangle strips
  vtkCellArray *inStrips = input->GetStrips();
  if ( this->ExtractCellType(VTK_TRIANGLE_STRIP) )
  {
    vtkCellArray *strips = vtkCellArray::New();
    // All cells are of type VTK_TRIANGLE_STRIP
    for ( inStrips->InitTraversal(); inStrips->GetNextCell(npts,pts); ++currentCellId )
    {
      ptIds->Reset();
      for (i=0; i<npts; ++i)
      {
        if ( ptMap[pts[i]] < 0 )
        {
          ptMap[pts[i]] = numNewPts++;
        }
        ptIds->InsertId(i,ptMap[pts[i]]);
      }
      cellId = strips->InsertNextCell(ptIds);
      outCD->CopyData(inCD,currentCellId,cellId);
    }
    output->SetStrips(strips);
    strips->Delete();
  }

  // Clean up
  ptIds->Delete();
}

//----------------------------------------------------------------------------
void vtkRemoveCellsByType::
ExtractUnstructuredGridCells(vtkDataSet *inDS, vtkDataSet *outDS,
                             vtkIdType *ptMap, vtkIdType &numNewPts)
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
int vtkRemoveCellsByType::RequestData(
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
if(this->CellTypes->empty())
cerr << "Extract cells has no cell types, creating empty output " << endl;
if (numCells <= 0)
cerr << "Extract cells numCells <=0, creating empty output" << endl;
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
      output->Initialize(); //output is empty
    }
    else
    {
      output->ShallowCopy(input);
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
int vtkRemoveCellsByType::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}

//----------------------------------------------------------------------------
void vtkRemoveCellsByType::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  // Output the number of types specified
  os << indent << "Number of types specified: "
     << this->CellTypes->size() << "\n";
}
