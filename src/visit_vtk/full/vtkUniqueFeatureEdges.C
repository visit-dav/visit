// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkUniqueFeatureEdges.h"

#include <visit-config.h>

#include <vtkCellArray.h>
#if LIB_VERSION_GE(VTK, 9,1,0)
#include <vtkCellArrayIterator.h>
#endif
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkEdgeTable.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMath.h>
#include <vtkMergePoints.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolygon.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkTriangleStrip.h>
#include <vtkUnsignedCharArray.h>

vtkStandardNewMacro(vtkUniqueFeatureEdges);

// Construct object with feature angle = 30; all types of edges, except
// manifold edges, are extracted
vtkUniqueFeatureEdges::vtkUniqueFeatureEdges()
{
  this->FeatureAngle = 30.0;
  this->BoundaryEdges = true;
  this->FeatureEdges = true;
  this->NonManifoldEdges = true;
  this->ManifoldEdges = false;
  this->Locator = NULL;
}

vtkUniqueFeatureEdges::~vtkUniqueFeatureEdges()
{
  if ( this->Locator )
    {
    this->Locator->UnRegister(this);
    this->Locator = NULL;
    }
}

// ***************************************************************************
// Generate feature edges for mesh
//
//  This is pretty much exactly like vtkFeatureEdges, with the addtion
//  of a vtkEdgeTable to prevent inserting duplicate edges.  Also, compare
//  ghostLevels[cellId] against output->UpdateGhostLevel instead of 0.
//  And there is no edge 'coloring'.   KSB
//
//  Modifications:
//    Kathleen Bonnell, Mon Oct 29 13:22:36 PST 2001
//    Make lineIds, npts, pts of type vtkIdType to match VTK 4.0 API.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkNormals has been deprecated in VTK 4.0, use vtkFloatArray instead.
//
//    Kathleen Bonnell, Mon May 20 17:01:31 PDT 2002
//    Fix memory leak. (delete edgeTable).
//
//    Hank Childs, Fri Jul 30 08:02:44 PDT 2004
//    Copy over field data.
//
//    Hank Childs, Fri Aug 27 15:15:20 PDT 2004
//    Rename ghost data array.
//
//    Kathleen Biagas, Thu Sep 6 11:07:21 MST 2012
//    Preserve coordinate data type.
//
//    Eric Brugger, Wed Jan  9 12:32:37 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
//    Kathleen Biagas, Thu Aug 11, 2022
//    Support VTK9: use vtkCellArrayIterator.
//
// ****************************************************************************

int vtkUniqueFeatureEdges::RequestData(
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
  vtkPolyData *input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkPoints *inPts;
  vtkPoints *newPts;
  vtkCellArray *newLines;
  vtkPolyData *Mesh;
  int i;
  vtkIdType j, numNei;
  vtkIdType numBEdges, numNonManifoldEdges, numFedges, numManifoldEdges;
  double n[3], x1[3], x2[3];
  double cosAngle = 0;
  vtkIdType lineIds[2];
  vtkIdType npts;
#if LIB_VERSION_LE(VTK, 8,1,0)
  vtkIdType *pts;
#else
  const vtkIdType *pts;
#endif
  vtkCellArray *inPolys, *newPolys;
  vtkDataArray *polyNormals = NULL;
  vtkIdType numPts, numCells, numPolys, numStrips, nei;
  vtkIdList *neighbors;
  vtkIdType p1, p2, newId;
  vtkPointData *pd=input->GetPointData(), *outPD=output->GetPointData();
  vtkCellData *cd=input->GetCellData(), *outCD=output->GetCellData();
  unsigned char* ghostLevels=0;
  vtkEdgeTable *edgeTable;
  unsigned char  updateLevel = static_cast<unsigned char>(
    outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS()));

  vtkDebugMacro(<<"Executing feature edges");

  vtkDataArray* temp = 0;
  if (cd)
    {
    temp = cd->GetArray("avtGhostZones");
    }
  if ( (!temp) || (temp->GetDataType() != VTK_UNSIGNED_CHAR)
       || (temp->GetNumberOfComponents() != 1))
    {
    vtkDebugMacro("No appropriate ghost levels field available.");
    }
  else
    {
    ghostLevels = ((vtkUnsignedCharArray*)temp)->GetPointer(0);
    }

  //  Check input
  //
  inPts=input->GetPoints();
  numCells = input->GetNumberOfCells();
  numPolys = input->GetNumberOfPolys();
  numStrips = input->GetNumberOfStrips();
  if ( (numPts=input->GetNumberOfPoints()) < 1 || !inPts ||
       (numPolys < 1 && numStrips < 1) )
    {
    //vtkErrorMacro(<<"No input data!");
    return 1;
    }

  if ( !this->BoundaryEdges && !this->NonManifoldEdges &&
       !this->FeatureEdges && !this->ManifoldEdges )
    {
    vtkDebugMacro(<<"All edge types turned off!");
    }

  // Build cell structure.  Might have to triangulate the strips.
  Mesh = vtkPolyData::New();
  Mesh->SetPoints(inPts);
  inPolys=input->GetPolys();
  if ( numStrips > 0 )
    {
    newPolys = vtkCellArray::New();
    if ( numPolys > 0 )
      {
      newPolys->DeepCopy(inPolys);
      }
    else
      {
      newPolys->Allocate(newPolys->EstimateSize(numStrips,5));
      }
#if LIB_VERSION_LE(VTK, 8,1,0)
    vtkCellArray *inStrips = input->GetStrips();
    for ( inStrips->InitTraversal(); inStrips->GetNextCell(npts,pts); )
      {
#else
    auto inStrips = vtk::TakeSmartPointer(input->GetStrips()->NewIterator());
    for (inStrips->GoToFirstCell(); !inStrips->IsDoneWithTraversal(); inStrips->GoToNextCell())
      {
      inStrips->GetCurrentCell(npts,pts);
#endif
      vtkTriangleStrip::DecomposeStrip(npts, pts, newPolys);
      }
    Mesh->SetPolys(newPolys);
    newPolys->Delete();
    }
  else
    {
    newPolys = inPolys;
    Mesh->SetPolys(newPolys);
    }
  Mesh->BuildLinks();

  // Allocate storage for lines/points (arbitrary allocation sizes)
  //
  newPts = vtkPoints::New(inPts->GetDataType());
  newPts->Allocate(numPts/10,numPts);
  newLines = vtkCellArray::New();
  newLines->Allocate(numPts/10);
  edgeTable = vtkEdgeTable::New();
  edgeTable->InitEdgeInsertion(numPts);

  outPD->CopyAllocate(pd, numPts);
  outCD->CopyAllocate(cd, numCells);
  GetOutput()->GetFieldData()->ShallowCopy(GetInput()->GetFieldData());

  // Get our locator for merging points
  //
  if ( this->Locator == NULL )
    {
    this->CreateDefaultLocator();
    }
  this->Locator->InitPointInsertion (newPts, input->GetBounds());

  // Loop over all polygons generating boundary, non-manifold,
  // and feature edges
  //
  if ( this->FeatureEdges )
    {
    polyNormals = inPts->GetData()->NewInstance();
    polyNormals->SetNumberOfComponents(3);
    polyNormals->Allocate(newPolys->GetNumberOfCells());

#if LIB_VERSION_LE(VTK, 8,1,0)
	vtkIdType cellId = 0;
    for (newPolys->InitTraversal(); newPolys->GetNextCell(npts,pts); cellId++)
      {
      vtkPolygon::ComputeNormal(inPts,npts,pts,n);
      polyNormals->InsertTuple(cellId,n);
      }
#else
    auto npIter = vtk::TakeSmartPointer(newPolys->NewIterator());
    for (npIter->GoToFirstCell(); !npIter->IsDoneWithTraversal(); npIter->GoToNextCell())
      {
      npIter->GetCurrentCell(npts,pts);
      vtkPolygon::ComputeNormal(inPts,npts,pts,n);
      polyNormals->InsertTuple(npIter->GetCurrentCellId(),n);
      }
#endif

    cosAngle = cos( vtkMath::RadiansFromDegrees( this->FeatureAngle ) );
    }

  neighbors = vtkIdList::New();
  neighbors->Allocate(VTK_CELL_SIZE);

  int abort=0;
  vtkIdType progressInterval=numCells/20+1;

  numBEdges = numNonManifoldEdges = numFedges = numManifoldEdges = 0;
#if LIB_VERSION_LE(VTK, 8,1,0)
  vtkIdType cellId;
  for (cellId=0, newPolys->InitTraversal();
       newPolys->GetNextCell(npts,pts) && !abort; cellId++)
    {
#else
  auto npIter = vtk::TakeSmartPointer(newPolys->NewIterator());
  for (npIter->GoToFirstCell(); !npIter->IsDoneWithTraversal() && !abort; npIter->GoToNextCell())
    {
    npIter->GetCurrentCell(npts,pts);
    vtkIdType cellId = npIter->GetCurrentCellId();
#endif
    if ( ! (cellId % progressInterval) ) //manage progress / early abort
      {
      this->UpdateProgress ((double)cellId / numCells);
      abort = this->GetAbortExecute();
      }

    for (i=0; i < npts; i++)
      {
      p1 = pts[i];
      p2 = pts[(i+1)%npts];

      Mesh->GetCellEdgeNeighbors(cellId,p1,p2, neighbors);
      numNei = neighbors->GetNumberOfIds();

      if ( this->BoundaryEdges && numNei < 1 )
        {
        if (ghostLevels && ghostLevels[cellId] > updateLevel)
          {
          continue;
          }
        else
          {
          numBEdges++;
          }
        }

      else if ( this->NonManifoldEdges && numNei > 1 )
        {
        // check to make sure that this edge hasn't been created before
        for (j=0; j < numNei; j++)
          {
          if ( neighbors->GetId(j) < cellId )
            {
            break;
            }
          }
        if ( j >= numNei )
          {
          if (ghostLevels && ghostLevels[cellId] > updateLevel)
            {
            continue;
            }
          else
            {
            numNonManifoldEdges++;
            }
          }
        else
          {
          continue;
          }
        }
      else if ( this->FeatureEdges &&
                numNei == 1 && (nei=neighbors->GetId(0)) > cellId )
        {
        if ( vtkMath::Dot(polyNormals->GetTuple(nei),
                          polyNormals->GetTuple(cellId)) <= cosAngle )
          {
          if (ghostLevels && ghostLevels[cellId] > updateLevel)
            {
            continue;
            }
          else
            {
            numFedges++;
            }
          }
        else
          {
          continue;
          }
        }
      else if ( this->ManifoldEdges )
        {
        if (ghostLevels && ghostLevels[cellId] > updateLevel)
          {
          continue;
          }
        else
          {
          numManifoldEdges++;
          }
        }
      else
        {
        continue;
        }

      // Add edge to output
      Mesh->GetPoint(p1, x1);
      Mesh->GetPoint(p2, x2);

      if ( this->Locator->InsertUniquePoint(x1, lineIds[0]) )
        {
        outPD->CopyData (pd,p1,lineIds[0]);
        }

      if ( this->Locator->InsertUniquePoint(x2, lineIds[1]) )
        {
        outPD->CopyData (pd,p2,lineIds[1]);
        }
      if (edgeTable->IsEdge(lineIds[0], lineIds[1]) == -1)
        {
        edgeTable->InsertEdge(lineIds[0], lineIds[1]);
        newId = newLines->InsertNextCell(2,lineIds);
        outCD->CopyData (cd,cellId,newId);
        }
      }
    }

  vtkDebugMacro(<<"Created " << numBEdges << " boundary edges, "
                << numNonManifoldEdges << " non-manifold edges, "
                << numFedges << " feature edges, "
                << numManifoldEdges << " manifold edges");

  //  Update ourselves.
  //
  if ( this->FeatureEdges )
    {
    polyNormals->Delete();
    }

  Mesh->Delete();

  edgeTable->Delete();
  output->SetPoints(newPts);
  newPts->Delete();
  neighbors->Delete();

  output->SetLines(newLines);
  newLines->Delete();

  return 1;
}

void vtkUniqueFeatureEdges::CreateDefaultLocator()
{
  if ( this->Locator == NULL )
    {
    this->Locator = vtkMergePoints::New();
    this->Locator->Register(this);
    this->Locator->Delete();
    }
}

// Specify a spatial locator for merging points. By
// default an instance of vtkMergePoints is used.
void vtkUniqueFeatureEdges::SetLocator(vtkPointLocator *locator)
{
  if ( this->Locator != locator )
    {
    if ( this->Locator )
      {
      this->Locator->UnRegister(this);
      this->Locator = NULL;
      }
    this->Locator = locator;
    if ( this->Locator )
      {
      this->Locator->Register(this);
      }
    this->Modified();
    }
}

// ****************************************************************************
//  Modifications:
//    Eric Brugger, Wed Jan  9 12:32:37 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
// ****************************************************************************

vtkMTimeType vtkUniqueFeatureEdges::GetMTime()
{
  vtkMTimeType mTime = this->Superclass::GetMTime();
  vtkMTimeType time;

  if ( this->Locator != NULL )
    {
    time = this->Locator->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }
  return mTime;
}

// ****************************************************************************
//  Modifications:
//    Eric Brugger, Wed Jan  9 12:32:37 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
// ****************************************************************************

int vtkUniqueFeatureEdges::RequestUpdateExtent(
  vtkInformation *request,
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  int numPieces = (int) outInfo->Get(
    vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES());
  int ghostLevel = (int) outInfo->Get(
    vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS());
  if (numPieces > 1)
    {
    inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(),
                ghostLevel + 1);
    }
  return 1;
}

void vtkUniqueFeatureEdges::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Feature Angle: "
               << this->FeatureAngle << "\n";
  os << indent << "Boundary Edges: "
               << (this->BoundaryEdges ? "On\n" : "Off\n");
  os << indent << "Feature Edges: "
               << (this->FeatureEdges ? "On\n" : "Off\n");
  os << indent << "Non-Manifold Edges: "
               << (this->NonManifoldEdges ? "On\n" : "Off\n");
  os << indent << "Manifold Edges: "
               << (this->ManifoldEdges ? "On\n" : "Off\n");

  if ( this->Locator )
    {
    os << indent << "Locator: " << this->Locator << "\n";
    }
  else
    {
    os << indent << "Locator: (none)\n";
    }
}
