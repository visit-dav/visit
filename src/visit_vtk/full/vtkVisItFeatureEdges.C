/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkFeatureEdges.cxx,v $
  Language:  C++
  Date:      $Date: 2002/09/03 12:52:23 $
  Version:   $Revision: 1.61 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include <vtkVisItFeatureEdges.h>

#include <visit-config.h> // For LIB_VERSION_LE

#include <vtkCellArray.h>
#if LIB_VERSION_GE(VTK, 9,1,0)
#include <vtkCellArrayIterator.h>
#endif
#include <vtkCellData.h>
#include <vtkFloatArray.h>
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

vtkStandardNewMacro(vtkVisItFeatureEdges);

// Construct object with feature angle = 30; all types of edges, except
// manifold edges, are extracted and colored.
vtkVisItFeatureEdges::vtkVisItFeatureEdges()
{
  this->FeatureAngle = 30.0;
  this->BoundaryEdges = true;
  this->FeatureEdges = true;
  this->NonManifoldEdges = true;
  this->ManifoldEdges = false;
  this->Coloring = true;
  this->Locator = NULL;
}

vtkVisItFeatureEdges::~vtkVisItFeatureEdges()
{
  if ( this->Locator )
    {
    this->Locator->UnRegister(this);
    this->Locator = NULL;
    }
}

// ****************************************************************************
//  Method:  vtkVisItFeatureEdges::RequestData
//
//  Purpose:
//    Generate feature edges for mesh.
//
//  Modifications:
//    Jeremy Meredith, Fri Apr 11 15:35:02 PDT 2003
//    I added correct code to deal with ghost levels (per Kathleen's
//    suggestions).
//
//    Hank Childs, Fri Aug 27 15:15:20 PDT 2004
//    Renamed ghost data array.  Also remove vestiges of "levels" concept.
//
//    Eric Brugger, Wed Jan  9 13:07:36 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
//    Kathleen Biagas, Thu Aug 11, 2022
//    Support VTK9: use vtkCellArrayIterator and const for pts.
//
// ****************************************************************************

int vtkVisItFeatureEdges::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  vtkPolyData *input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkPoints *inPts;
  vtkPoints *newPts;
  vtkFloatArray *newScalars = NULL;
  vtkCellArray *newLines;
  vtkPolyData *Mesh;
  int i;
  vtkIdType j, numNei;
  vtkIdType numBEdges, numNonManifoldEdges, numFedges, numManifoldEdges;
  double scalar;
  double n[3], x1[3], x2[3];
  double cosAngle = 0;
  vtkIdType lineIds[2];
  vtkIdType npts = 0;
#if LIB_VERSION_LE(VTK, 8,1,0)
  vtkIdType *pts = 0;
#else
  const vtkIdType *pts = 0;
#endif
  vtkCellArray *inPolys, *newPolys;
  vtkFloatArray *polyNormals = NULL;
  vtkIdType numPts, numCells, numPolys, numStrips, nei;
  vtkIdList *neighbors;
  vtkIdType p1, p2, newId;
  vtkPointData *pd=input->GetPointData(), *outPD=output->GetPointData();
  vtkCellData *cd=input->GetCellData(), *outCD=output->GetCellData();
  unsigned char* ghostLevels=0;

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
    for ( inStrips->GoToFirstCell(); !inStrips->IsDoneWithTraversal(); inStrips->GoToNextCell())
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
  newPts = vtkPoints::New(input->GetPoints()->GetDataType());
  newPts->Allocate(numPts/10,numPts);
  newLines = vtkCellArray::New();
  newLines->Allocate(numPts/10);
  if ( this->Coloring )
    {
    newScalars = vtkFloatArray::New();
    newScalars->Allocate(numCells/10,numCells);
    outCD->CopyScalarsOff();
    }

  outPD->CopyAllocate(pd, numPts);
  outCD->CopyAllocate(cd, numCells);

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
    polyNormals = vtkFloatArray::New();
    polyNormals->SetNumberOfComponents(3);
    polyNormals->Allocate(3*newPolys->GetNumberOfCells());

#if LIB_VERSION_LE(VTK, 8,1,0)
    vtkIdType cellId = 0;
    for (newPolys->InitTraversal(); newPolys->GetNextCell(npts,pts); cellId++)
      {
#else
    auto iter = vtk::TakeSmartPointer(newPolys->NewIterator());
    for (iter->GoToFirstCell(); !iter->IsDoneWithTraversal(); iter->GoToNextCell())
      {
      vtkIdType cellId = iter->GetCurrentCellId();
      iter->GetCurrentCell(npts,pts);
#endif
      vtkPolygon::ComputeNormal(inPts,npts,pts,n);
      polyNormals->InsertTuple(cellId,n);
      }

    cosAngle = cos( vtkMath::RadiansFromDegrees( this->FeatureAngle ) );
    }

  neighbors = vtkIdList::New();
  neighbors->Allocate(VTK_CELL_SIZE);

  int abort=0;
  vtkIdType progressInterval=numCells/20+1;

  numBEdges = numNonManifoldEdges = numFedges = numManifoldEdges = 0;
#if LIB_VERSION_LE(VTK, 8,1,0)
  vtkIdType cellId = 0;
  for (newPolys->InitTraversal(); newPolys->GetNextCell(npts,pts) && !abort; cellId++)
    {
#else
  auto iter = vtk::TakeSmartPointer(newPolys->NewIterator());
  for (iter->GoToFirstCell(); !iter->IsDoneWithTraversal() && !abort; iter->GoToNextCell())
    {
    vtkIdType cellId = iter->GetCurrentCellId();
    iter->GetCurrentCell(npts,pts);
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
        if (ghostLevels && ghostLevels[cellId] > 0)
          {
          continue;
          }
        else
          {
          numBEdges++;
          scalar = 0.0;
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
          if (ghostLevels && ghostLevels[cellId] > 0)
            {
            continue;
            }
          else
            {
            numNonManifoldEdges++;
            scalar = 0.222222;
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
        double neiTuple[3];
        double cellTuple[3];
        polyNormals->GetTuple(nei, neiTuple);
        polyNormals->GetTuple(cellId, cellTuple);
        if ( vtkMath::Dot(neiTuple, cellTuple) <= cosAngle )
          {
          if (ghostLevels && ghostLevels[cellId] > 0)
            {
            continue;
            }
          else
            {
            numFedges++;
            scalar = 0.444444;
            }
          }
        else
          {
          continue;
          }
        }
      else if ( this->ManifoldEdges )
        {
        if (ghostLevels && ghostLevels[cellId] > 0)
          {
          continue;
          }
        else
          {
          numManifoldEdges++;
          scalar = 0.666667;
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

      newId = newLines->InsertNextCell(2,lineIds);
      outCD->CopyData (cd,cellId,newId);
      if ( this->Coloring )
        {
        newScalars->InsertTuple(newId, &scalar);
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

  output->SetPoints(newPts);
  newPts->Delete();
  neighbors->Delete();

  output->SetLines(newLines);
  newLines->Delete();

  if ( this->Coloring )
    {
    outCD->SetScalars(newScalars);
    newScalars->Delete();
    }

  return 1;
}

void vtkVisItFeatureEdges::CreateDefaultLocator()
{
  if ( this->Locator == NULL )
    {
    this->Locator = vtkMergePoints::New();
    }
}

// Specify a spatial locator for merging points. By
// default an instance of vtkMergePoints is used.
void vtkVisItFeatureEdges::SetLocator(vtkPointLocator *locator)
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

// ****************************************************************************
//  Modifications:
//    Eric Brugger, Wed Jan  9 13:07:36 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
// ****************************************************************************

vtkMTimeType vtkVisItFeatureEdges::GetMTime()
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
//    Eric Brugger, Wed Jan  9 13:07:36 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
// ****************************************************************************

int vtkVisItFeatureEdges::RequestUpdateExtent(
  vtkInformation *request,
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  int numPieces =
    outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES());
  int ghostLevel =
    outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS());

  if (numPieces > 1)
    {
    inInfo->Set(
      vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(),
                ghostLevel + 1);
    }
  return 1;
}

void vtkVisItFeatureEdges::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Feature Angle: " << this->FeatureAngle << "\n";
  os << indent << "Boundary Edges: " << (this->BoundaryEdges ? "On\n" : "Off\n");
  os << indent << "Feature Edges: " << (this->FeatureEdges ? "On\n" : "Off\n");
  os << indent << "Non-Manifold Edges: " << (this->NonManifoldEdges ? "On\n" : "Off\n");
  os << indent << "Manifold Edges: " << (this->ManifoldEdges ? "On\n" : "Off\n");
  os << indent << "Coloring: " << (this->Coloring ? "On\n" : "Off\n");

  if ( this->Locator )
    {
    os << indent << "Locator: " << this->Locator << "\n";
    }
  else
    {
    os << indent << "Locator: (none)\n";
    }
}
