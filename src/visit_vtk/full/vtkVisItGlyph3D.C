/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItGlyph3D.cxx,v $
  Language:  C++
  Date:      $Date: 2002/12/17 02:05:38 $
  Version:   $Revision: 1.107 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include <vtkVisItGlyph3D.h>

#include <vtkAppendPolyData.h>
#include <vtkCell.h>
#include <vtkDataSet.h>
#include <vtkCellData.h>
#include <vtkExecutive.h>
#include <vtkFloatArray.h>
#include <vtkIdList.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkTransform.h>
#include <vtkTrivialProducer.h>
#include <vtkUnsignedCharArray.h>

vtkStandardNewMacro(vtkVisItGlyph3D);

// Construct object with scaling on, scaling mode is by scalar value,
// scale factor = 1.0, the range is (0,1), orient geometry is on, and
// orientation is by vector. Clamping and indexing are turned off. No
// initial sources are defined.
vtkVisItGlyph3D::vtkVisItGlyph3D()
{
  this->Scaling = true;
  this->ColorMode = VTK_COLOR_BY_SCALE;
  this->ScaleMode = VTK_SCALE_BY_SCALAR;
  this->ScaleFactor = 1.0;
  this->Range[0] = 0.0;
  this->Range[1] = 1.0;
  this->Orient = true;
  this->VectorMode = VTK_USE_VECTOR;
  this->Clamping = false;
  this->IndexMode = VTK_INDEXING_OFF;
  this->GeneratePointIds = false;
  this->TreatVectorsAs2D = false;
  this->PointIdsName = NULL;
  this->SetPointIdsName("InputPointIds");
  this->SetNumberOfInputPorts(2);
  this->InputScalarsSelection = NULL;
  this->InputVectorsSelection = NULL;
  this->InputNormalsSelection = NULL;

  this->ScalarsForColoring = NULL;
  this->ScalarsForScaling = NULL;
  this->VectorsForColoring = NULL;
  this->VectorsForScaling = NULL;
  this->TensorsForScaling = NULL;

  this->UseFullFrameScaling = 0;
  this->FullFrameScaling[0] = 1.;
  this->FullFrameScaling[1] = 1.;
  this->FullFrameScaling[2] = 1.;
}

vtkVisItGlyph3D::~vtkVisItGlyph3D()
{
  if (this->PointIdsName)
  {
    delete []PointIdsName;
  }
  this->SetInputScalarsSelection(NULL);
  this->SetInputVectorsSelection(NULL);
  this->SetInputNormalsSelection(NULL);

  this->SetScalarsForColoring(NULL);
  this->SetScalarsForScaling(NULL);
  this->SetVectorsForColoring(NULL);
  this->SetVectorsForScaling(NULL);
  this->SetTensorsForScaling(NULL);
}

// ****************************************************************************
//  Method: vtkVisItGlyph3D::RequestData
//
//  Modifications:
//    Kathleen Bonnell, Thu Aug 19 15:29:46 PDT 2004
//    Allow user to set different scalars/vectors for coloring and scaling,
//    but still perserved default vtkGlyph3D behavior if those ivars aren't
//    populated.
//
//    Hank Childs, Fri Aug 27 15:15:20 PDT 2004
//    Renamed ghost data arrays.
//
//    Kathleen Bonnell, Tue Oct 12 16:42:16 PDT 2004
//    Ensure that avtOriginalCellNumbers and avtOriginalNodeNumbers arrays
//    get copied to output (if they exist in input).
//
//    Kathleen Bonnell, Fri Nov 12 11:50:33 PST 2004
//    Retrieve VectorsForScaling.
//
//    Hank Childs, Fri Sep 14 09:54:13 PDT 2007
//    Add support for treating vectors as 2D.
//
//    Hank Childs, Fri Jan 29 16:25:29 PST 2010
//    Add support for treating vectors as 2D ... for data that is 10^15.
//
//    Hank Childs, Sun Jun 27 12:18:41 PDT 2010
//    Only glyph points that have VTK_VERTEXs.  Also pass edges, quads, etc
//    through the filter.
//
//    John Schmidt, Thu Nov 15 13:08:21 MST 2012
//    Added capability to scale by a 3x3 tensor.  Use in scaling the Box
//    glyph.
//
//    Eric Brugger, Thu Jan 10 13:05:08 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
//    Jeremy Meredith, Thu Aug 22 15:07:57 EDT 2013
//    Fix full frame correction for vector glyphs.  The old correction
//    was applied at the wrong place in the transforms, so it was applied
//    before the glyphs were oriented correctly.  I changed it so it
//    would apply after the orientation -- though that has the consequence
//    of changing the orientation -- so I also explicitly re-applied the
//    full frame correction to the orientation.  So the shape has the
//    distortion removed, but the orientation properly leaves it in.
//
//    Jeremy Meredith, Fri Aug 23 12:01:38 EDT 2013
//    Added back the original full frame correction for non-vector glyphs.
//
//    Kathleen Biagas, Thu May 30 12:15:07 PDT 2019
//    Allow cell-centered data for coloring/scaling.
//
//    Kathleen Biagas, Fri Sep 13 09:36:22 PDT 2019
//    When scaling by a tensor use inTensors_forScaling,
//    not inScalars_forScaling.
//
//    Kathleen Biagas, Wed Nov  6 11:39:09 PST 2019
//    Properly access CellData by cellIdx, not inPtId.
//    Make indentation consistent with vtk-8 coding styles.
//
// ****************************************************************************

int
vtkVisItGlyph3D::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  vtkDebugMacro(<<"Executing vtkVisItGlyph3D");

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

  vtkPointData *pd;
  vtkCellData  *cd;
  vtkDataArray *inScalars = NULL;
  vtkDataArray *inScalars_forColoring = NULL;
  vtkDataArray *inScalars_forScaling = NULL;

  vtkDataArray *inVectors = NULL;
  vtkDataArray *inVectors_forColoring = NULL;
  vtkDataArray *inVectors_forScaling = NULL;
  vtkDataArray *inTensors_forScaling = NULL;

  // flags indicating pointdata(true) or celldata(false)
  bool inScalars_pd             = true;
  bool inScalars_forColoring_pd = true;
  bool inScalars_forScaling_pd  = true;
  bool inVectors_pd             = true;
  bool inVectors_forColoring_pd = true;
  bool inVectors_forScaling_pd  = true;
  bool inTensors_forScaling_pd  = true;
  bool inNormals_pd  = true;

  int requestedGhostLevel;
  unsigned char* inGhostLevels=0;
  vtkDataArray *inNormals = NULL, *sourceNormals = NULL;
  vtkIdType numPts, numSourcePts, numSourceCells, i;
  int index;
  vtkPoints *sourcePts = NULL;
  vtkPoints *newPts;
  vtkDataArray *newScalars=NULL;
  vtkDataArray *newVectors=NULL;
  vtkDataArray *newNormals=NULL;
  double x[3], v[3];
  double vNew[3], s = 0.0, vMag = 0.0, value;
  vtkTransform *trans = vtkTransform::New();
  vtkMatrix4x4 *def_mat = vtkMatrix4x4::New();
  vtkCell *cell;
  vtkIdList *cellPts;
  int npts;
  vtkIdList *pts;
  vtkIdType ptIncr, cellId;
  int haveVectors, haveNormals;
  double scalex,scaley,scalez, den;
  vtkPolyData *outPD = vtkPolyData::New();
  vtkPointData *outputPD = outPD->GetPointData();
  vtkCellData *outputCD = outPD->GetCellData();

  vtkDataArray *inOrigNodes = NULL;
  vtkDataArray *inOrigCells = NULL;
  vtkDataArray *outOrigNodes = NULL;
  vtkDataArray *outOrigCells = NULL;

  int numberOfSources = this->GetNumberOfInputConnections(1);
  vtkPolyData *defaultSource = NULL;
  vtkIdTypeArray *pointIds=0;

  vtkDebugMacro(<<"Generating glyphs");

  pts = vtkIdList::New();
  pts->Allocate(VTK_CELL_SIZE);

  if (!input)
  {
    vtkErrorMacro(<<"No input");
    return 1;
  }

  pd = input->GetPointData();
  cd = input->GetCellData();

  inScalars = pd->GetScalars(this->InputScalarsSelection);
  if(inScalars == NULL)
  {
    inScalars_pd = false;
    inScalars = cd->GetScalars(this->InputScalarsSelection);
  }

  inVectors = pd->GetVectors(this->InputVectorsSelection);
  if(inVectors == NULL)
  {
    inVectors_pd = false;
    inVectors = cd->GetVectors(this->InputVectorsSelection);
  }

  inNormals = pd->GetNormals(this->InputNormalsSelection);
  if(inNormals == NULL)
  {
    inNormals_pd = false;
    inNormals = cd->GetNormals(this->InputNormalsSelection);
  }

  inScalars_forColoring = pd->GetArray(this->ScalarsForColoring);
  if (inScalars_forColoring == NULL)
  {
    inScalars_forColoring_pd = false;
    inScalars_forColoring = cd->GetArray(this->ScalarsForColoring);
  }
  inScalars_forScaling  = pd->GetArray(this->ScalarsForScaling);
  if (inScalars_forScaling == NULL)
  {
    inScalars_forScaling_pd = false;
    inScalars_forScaling = cd->GetArray(this->ScalarsForScaling);
  }
  inVectors_forColoring = pd->GetArray(this->VectorsForColoring);
  if (inVectors_forColoring == NULL)
  {
    inVectors_forColoring_pd = false;
    inVectors_forColoring = cd->GetArray(this->VectorsForColoring);
  }
  inVectors_forScaling  = pd->GetArray(this->VectorsForScaling);
  if (inVectors_forScaling == NULL)
  {
    inVectors_forScaling_pd = false;
    inVectors_forScaling = cd->GetArray(this->VectorsForScaling);
  }
  inTensors_forScaling  = pd->GetArray(this->TensorsForScaling);
  if (inTensors_forScaling == NULL)
  {
    inTensors_forScaling_pd = false;
    inTensors_forScaling = cd->GetArray(this->TensorsForScaling);
  }

  inOrigNodes = pd->GetArray("avtOriginalNodeNumbers");
  inOrigCells = pd->GetArray("avtOriginalCellNumbers");

  vtkDataArray* temp = 0;
  if (pd)
  {
    temp = pd->GetArray("avtGhostZones");
  }
  if ( (!temp) || (temp->GetDataType() != VTK_UNSIGNED_CHAR)
    || (temp->GetNumberOfComponents() != 1))
  {
    vtkDebugMacro("No appropriate ghost levels field available.");
  }
  else
  {
    inGhostLevels =static_cast<vtkUnsignedCharArray *>(temp)->GetPointer(0);
  }

  requestedGhostLevel =
    outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS());

  numPts = input->GetNumberOfPoints();
  if (numPts < 1)
  {
    vtkDebugMacro(<<"No points to glyph!");
    pts->Delete();
    trans->Delete();
    return 1;
  }

  // Check input for consistency
  //
  if ( (den = this->Range[1] - this->Range[0]) == 0.0 )
  {
    den = 1.0;
  }
  if ( this->VectorMode != VTK_VECTOR_ROTATION_OFF &&
       ((this->VectorMode == VTK_USE_VECTOR && inVectors_forScaling != NULL) ||
        (this->VectorMode == VTK_USE_NORMAL && inNormals != NULL)) )
  {
    haveVectors = 1;
  }
  else if ( this->VectorMode != VTK_VECTOR_ROTATION_OFF &&
       ((this->VectorMode == VTK_USE_VECTOR && inVectors != NULL) ||
        (this->VectorMode == VTK_USE_NORMAL && inNormals != NULL)) )
  {
    haveVectors = 1;
  }
  else
  {
    haveVectors = 0;
  }

  if ( (this->IndexMode == VTK_INDEXING_BY_SCALAR &&
       (!inScalars && !inScalars_forScaling)) ||
       (this->IndexMode == VTK_INDEXING_BY_VECTOR &&
       (((!inVectors && !inVectors_forScaling) &&
           this->VectorMode == VTK_USE_VECTOR) ||
        (!inNormals && this->VectorMode == VTK_USE_NORMAL))) )
  {
    if ( this->GetSource(0) == NULL )
    {
      vtkErrorMacro(<<"Indexing on but don't have data to index with");
      pts->Delete();
      trans->Delete();
      def_mat->Delete();
      return 1;
    }
    else
    {
      vtkWarningMacro(<<"Turning indexing off: no data to index with");
      this->IndexMode = VTK_INDEXING_OFF;
    }
  }

  // Allocate storage for output PolyData
  //
  outputPD->CopyScalarsOff();
  outputPD->CopyVectorsOff();
  outputPD->CopyNormalsOff();

  if (!this->GetSource(0))
  {
    defaultSource = vtkPolyData::New();
    defaultSource->Allocate();
    vtkPoints *defaultPoints = vtkPoints::New();
    defaultPoints->Allocate(6);
    defaultPoints->InsertNextPoint(0, 0, 0);
    defaultPoints->InsertNextPoint(1, 0, 0);
    vtkIdType defaultPointIds[2];
    defaultPointIds[0] = 0;
    defaultPointIds[1] = 1;
    defaultSource->SetPoints(defaultPoints);
    defaultSource->InsertNextCell(VTK_LINE, 2, defaultPointIds);
    this->SetSourceData(defaultSource);
    defaultSource->Delete();
    defaultSource = NULL;
    defaultPoints->Delete();
    defaultPoints = NULL;
  }

  if ( this->IndexMode != VTK_INDEXING_OFF )
  {
    pd = NULL;
    numSourcePts = numSourceCells = 0;
    haveNormals = 1;
    for (numSourcePts=numSourceCells=i=0; i < numberOfSources; i++)
    {
      if ( this->GetSource(i) != NULL )
      {
        numSourcePts += this->GetSource(i)->GetNumberOfPoints();
        numSourceCells += this->GetSource(i)->GetNumberOfCells();
        if ( !(sourceNormals = this->GetSource(i)->GetPointData()->GetNormals()) )
        {
          haveNormals = 0;
        }
      }
    }
  }
  else
  {
    sourcePts = this->GetSource(0)->GetPoints();
    numSourcePts = sourcePts->GetNumberOfPoints();
    numSourceCells = this->GetSource(0)->GetNumberOfCells();

    sourceNormals = this->GetSource(0)->GetPointData()->GetNormals();
    if ( sourceNormals )
    {
      haveNormals = 1;
    }
    else
    {
      haveNormals = 0;
    }

    // Prepare to copy output.
    pd = this->GetSource(0)->GetPointData();
    outputPD->CopyAllocate(pd,numPts*numSourcePts);
  }

  newPts = vtkPoints::New();
  newPts->Allocate(numPts*numSourcePts);
  if ( this->GeneratePointIds )
  {
    pointIds = vtkIdTypeArray::New();
    pointIds->SetName(this->PointIdsName);
    pointIds->Allocate(numPts*numSourcePts);
    outputPD->AddArray(pointIds);
  }
  if ( this->ColorMode == VTK_COLOR_BY_SCALAR && inScalars_forColoring)
  {
    newScalars = inScalars_forColoring->NewInstance();
    newScalars->SetNumberOfComponents(inScalars_forColoring->GetNumberOfComponents());
    newScalars->Allocate(inScalars_forColoring->GetNumberOfComponents()*
                         numPts*numSourcePts);
    newScalars->SetName(inScalars_forColoring->GetName());
  }
  else if ( this->ColorMode == VTK_COLOR_BY_SCALAR && inScalars)
  {
    newScalars = inScalars->NewInstance();
    newScalars->SetNumberOfComponents(inScalars->GetNumberOfComponents());
    newScalars->Allocate(inScalars->GetNumberOfComponents()*numPts*numSourcePts);
    newScalars->SetName(inScalars->GetName());
  }
  else if ( (this->ColorMode == VTK_COLOR_BY_SCALE) && inScalars)
  {
    newScalars = vtkFloatArray::New();
    newScalars->Allocate(numPts*numSourcePts);
    newScalars->SetName("GlyphScale");
    if (this->ScaleMode == VTK_SCALE_BY_SCALAR)
    {
      newScalars->SetName(inScalars->GetName());
    }
  }
  else if ( (this->ColorMode == VTK_COLOR_BY_VECTOR) && inVectors_forColoring)
  {
    newScalars = vtkFloatArray::New();
    newScalars->Allocate(numPts*numSourcePts);
    newScalars->SetName("VectorMagnitude");
  }
  else if ( (this->ColorMode == VTK_COLOR_BY_VECTOR) && haveVectors)
  {
    newScalars = vtkFloatArray::New();
    newScalars->Allocate(numPts*numSourcePts);
    newScalars->SetName("VectorMagnitude");
  }
  if ( haveVectors )
  {
    newVectors = vtkFloatArray::New();
    newVectors->SetNumberOfComponents(3);
    newVectors->Allocate(3*numPts*numSourcePts);
    newVectors->SetName("GlyphVector");
  }
  if ( haveNormals )
  {
    newNormals = vtkFloatArray::New();
    newNormals->SetNumberOfComponents(3);
    newNormals->Allocate(3*numPts*numSourcePts);
    newNormals->SetName("Normals");
  }
  if ( inOrigNodes )
  {
    outOrigNodes = inOrigNodes->NewInstance();
    outOrigNodes->SetNumberOfComponents(inOrigNodes->GetNumberOfComponents());
    outOrigNodes->Allocate(inOrigNodes->GetNumberOfComponents()*numSourceCells*numPts);
    outOrigNodes->SetName(inOrigNodes->GetName());
  }
  if ( inOrigCells )
  {
    outOrigCells = inOrigCells->NewInstance();
    outOrigCells->SetNumberOfComponents(inOrigCells->GetNumberOfComponents());
    outOrigCells->Allocate(inOrigCells->GetNumberOfComponents()*numSourceCells*numPts);
    outOrigCells->SetName(inOrigCells->GetName());
  }

  int connSize = 0;
  int numVerts = 0;
  int numCells = input->GetNumberOfCells();
  vtkIdList *ptIds = vtkIdList::New();
  for (i = 0 ; i < numCells ; i++)
  {
    vtkIdType c = input->GetCellType(i);
    if (c == VTK_VERTEX)
      numVerts++;
    else if (c == VTK_POLY_VERTEX)
    {
      input->GetCellPoints(i, ptIds);
      numVerts += ptIds->GetNumberOfIds();
    }
    else
    {
      input->GetCellPoints(i, ptIds);
      connSize += ptIds->GetNumberOfIds()+1;
    }
  }
  // Setting up for calls to PolyData::InsertNextCell()
  if (this->IndexMode != VTK_INDEXING_OFF )
  {
    outPD->Allocate(3*numPts*numSourceCells,numPts*numSourceCells);
  }
  else
  {
    outPD->Allocate(this->GetSource(0),3*numPts*numSourceCells,numPts*numSourceCells);
  }

  // Traverse all Input points, transforming Source points and copying
  // point attributes.
  //
  ptIncr=0;
  for (int cellIdx = 0 ; cellIdx < numCells ; cellIdx++)
  {
    vtkIdType c = input->GetCellType(cellIdx);
    if (c != VTK_VERTEX && c != VTK_POLY_VERTEX)
       continue;
    input->GetCellPoints(cellIdx, ptIds);
    // only 1 iteration for VTK_VERTEX, multiple for VTK_POLY_VERTEX
    for (int j = 0 ; j < ptIds->GetNumberOfIds() ; j++)
    {
      vtkIdType inPtId = ptIds->GetId(j);
      scalex = scaley = scalez = 1.0;
      if ( (cellIdx % 10000) == 0 )
      {
        this->UpdateProgress(static_cast<double>(inPtId)/numPts);
        if (this->GetAbortExecute())
        {
          break;
        }
      }

      // Get the scalar and vector data
      if ( inTensors_forScaling )
      {
        if (this->ScaleMode == VTK_SCALE_BY_TENSOR)
        {
          // def_mat is Identity at its creation, only change needed elements.
          double* tensor = NULL;
          if (inTensors_forScaling_pd)
            tensor = inTensors_forScaling->GetTuple9(inPtId);
          else
            tensor = inTensors_forScaling->GetTuple9(cellIdx);
          def_mat->SetElement(0,0,tensor[0]);
          def_mat->SetElement(0,1,tensor[1]);
          def_mat->SetElement(0,2,tensor[2]);

          def_mat->SetElement(1,0,tensor[3]);
          def_mat->SetElement(1,1,tensor[4]);
          def_mat->SetElement(1,2,tensor[5]);

          def_mat->SetElement(2,0,tensor[6]);
          def_mat->SetElement(2,1,tensor[7]);
          def_mat->SetElement(2,2,tensor[8]);
        }
      }
      else if ( inScalars_forScaling )
      {
        if (inScalars_forScaling_pd)
            s = inScalars_forScaling->GetComponent(inPtId, 0);
        else
            s = inScalars_forScaling->GetComponent(cellIdx, 0);
        if ( this->ScaleMode == VTK_SCALE_BY_SCALAR ||
             this->ScaleMode == VTK_DATA_SCALING_OFF )
        {
          scalex = scaley = scalez = s;
        }
      }
      else if ( inScalars )
      {
        if (inScalars_pd)
          s = inScalars->GetComponent(inPtId, 0);
        else
          s = inScalars->GetComponent(cellIdx, 0);
        if ( this->ScaleMode == VTK_SCALE_BY_SCALAR ||
             this->ScaleMode == VTK_DATA_SCALING_OFF )
        {
          scalex = scaley = scalez = s;
        }
      }

      if ( haveVectors )
      {
        if ( this->VectorMode == VTK_USE_NORMAL )
        {
          if (inNormals_pd)
            inNormals->GetTuple(inPtId, v);
          else
            inNormals->GetTuple(cellIdx, v);
        }
        else if (inVectors_forScaling)
        {
          if (inVectors_forScaling_pd)
            inVectors_forScaling->GetTuple(inPtId, v);
          else
            inVectors_forScaling->GetTuple(cellIdx, v);
        }
        else
        {
          if (inVectors_pd)
            inVectors->GetTuple(inPtId, v);
          else
            inVectors->GetTuple(cellIdx, v);
        }
        if (this->TreatVectorsAs2D)
        {
          v[2] = 0.;
        }
        vMag = vtkMath::Norm(v);

        // We're going to "undo" the full frame
        // scaling later to remove the visual
        // distortion, but that will distort
        // the vector; we need to keep the
        // full frame scaling in here somehow:
        if (this->UseFullFrameScaling)
        {
          v[0] *= this->FullFrameScaling[0];
          v[1] *= this->FullFrameScaling[1];
          v[2] *= this->FullFrameScaling[2];
        }

        if ( this->ScaleMode == VTK_SCALE_BY_VECTORCOMPONENTS )
        {
          scalex = v[0];
          scaley = v[1];
          scalez = v[2];
        }
        else if ( this->ScaleMode == VTK_SCALE_BY_VECTOR )
        {
          scalex = scaley = scalez = vMag;
        }
      }

      // Clamp data scale if enabled
      if ( this->Clamping )
      {
        scalex = (scalex < this->Range[0] ? this->Range[0] :
                 (scalex > this->Range[1] ? this->Range[1] : scalex));
        scalex = (scalex - this->Range[0]) / den;
        scaley = (scaley < this->Range[0] ? this->Range[0] :
                 (scaley > this->Range[1] ? this->Range[1] : scaley));
        scaley = (scaley - this->Range[0]) / den;
        scalez = (scalez < this->Range[0] ? this->Range[0] :
                 (scalez > this->Range[1] ? this->Range[1] : scalez));
        scalez = (scalez - this->Range[0]) / den;
      }

      // Compute index into table of glyphs
      index = 0;
      if ( this->IndexMode == VTK_INDEXING_OFF )
      {
        index = 0;
      }
      else
      {
        if ( this->IndexMode == VTK_INDEXING_BY_SCALAR )
        {
          value = s;
        }
        else
        {
          value = vMag;
        }

        index = (int) ((double)(value - this->Range[0]) * numberOfSources / den);
        index = (index < 0 ? 0 :
                (index >= numberOfSources ? (numberOfSources-1) : index));

        if ( this->GetSource(index) != NULL )
        {
          sourcePts = this->GetSource(index)->GetPoints();
          sourceNormals = this->GetSource(index)->GetPointData()->GetNormals();
          numSourcePts = sourcePts->GetNumberOfPoints();
          numSourceCells = this->GetSource(index)->GetNumberOfCells();
        }
      }

      // Make sure we're not indexing into empty glyph
      if ( this->GetSource(index) == NULL )
      {
        continue;
      }

      // Check ghost points.
      // If we are processing a piece, we do not want to duplicate
      // glyphs on the borders.  The corrct check here is:
      // ghostLevel > 0.  I am leaving this over glyphing here because
      // it make a nice example (sphereGhost.tcl) to show the
      // point ghost levels with the glyph filter.  I am not certain
      // of the usefullness of point ghost levels over 1, but I will have
      // to think about it.
      if (inGhostLevels && inGhostLevels[inPtId] > requestedGhostLevel)
      {
        continue;
      }

      // Now begin copying/transforming glyph
      trans->Identity();

      // Make 100% sure this is putting the glyphs at Z=0 for 2D.  Floating
      // point error can creep in with 10^15 extents.
      if (this->TreatVectorsAs2D)
      {
        trans->Scale(1,1,0);
      }

      double *inNode = NULL;
      double *inCell = NULL;
      if (inOrigNodes)
      {
        inNode = inOrigNodes->GetTuple(inPtId);
      }
      if (inOrigCells)
      {
        inCell = inOrigCells->GetTuple(inPtId);
      }

      // Copy all topology (transformation independent)
      for (cellId=0; cellId < numSourceCells; cellId++)
      {
        cell = this->GetSource(index)->GetCell(cellId);
        cellPts = cell->GetPointIds();
        npts = cellPts->GetNumberOfIds();
        for (pts->Reset(), i=0; i < npts; i++)
        {
          pts->InsertId(i,cellPts->GetId(i) + ptIncr);
        }
        outPD->InsertNextCell(cell->GetCellType(),pts);
        if (outOrigNodes)
        {
          outOrigNodes->InsertNextTuple(inNode);
        }
        if (outOrigCells)
        {
          outOrigCells->InsertNextTuple(inCell);
        }
      }

      // translate Source to Input point
      input->GetPoint(inPtId, x);
      trans->Translate(x[0], x[1], x[2]);

      // Transform Source by Tensor
      trans->Concatenate(def_mat);

      if ( haveVectors )
      {
        // If we are using full frame scaling then add an additional
        // transform to undo what fullframe will do.  However, it
        // does change the angle of the vector as well, which is
        // why we needed to modify v[] in this case above.
        if (this->UseFullFrameScaling)
        {
          trans->Scale(1. / this->FullFrameScaling[0],
                       1. / this->FullFrameScaling[1],
                       1. / this->FullFrameScaling[2]);
        }

        // Copy Input vector
        for (i=0; i < numSourcePts; i++)
        {
          newVectors->InsertTuple(i+ptIncr, v);
        }
        if (this->Orient && (vMag > 0.0))
        {
          // if there is no y or z component
          if ( v[1] == 0.0 && v[2] == 0.0 )
          {
            if (v[0] < 0) //just flip x if we need to
            {
              trans->RotateWXYZ(180.0,0,1,0);
            }
          }
          else
          {
            // In full frame mode, we changed v, but kept its
            // original magnitude.
            // This specific calculation, however, is only
            // valid if we use v's actual magnitude.
            double realvMag = vtkMath::Norm(v);
            vNew[0] = (v[0]+realvMag) / 2.0;
            vNew[1] = v[1] / 2.0;
            vNew[2] = v[2] / 2.0;
            trans->RotateWXYZ(180.0,vNew[0],vNew[1],vNew[2]);
          }
        }
      }

      // determine scale factor from scalars if appropriate
      if ( inScalars_forColoring || inScalars )
      {
        // Copy scalar value
        if (this->ColorMode == VTK_COLOR_BY_SCALE)
        {
          for (i=0; i < numSourcePts; i++)
          {
            newScalars->InsertTuple(i+ptIncr, &scalex); // = scaley = scalez
          }
        }
        else if (this->ColorMode == VTK_COLOR_BY_SCALAR && inScalars_forColoring)
        {
          for (i=0; i < numSourcePts; i++)
          {
            if(inScalars_forColoring_pd)
              outputPD->CopyTuple(inScalars_forColoring, newScalars, inPtId, ptIncr+i);
            else
              outputPD->CopyTuple(inScalars_forColoring, newScalars, cellIdx, ptIncr+i);
          }
        }
        else if (this->ColorMode == VTK_COLOR_BY_SCALAR && inScalars)
        {
          for (i=0; i < numSourcePts; i++)
          {
            if(inScalars_pd)
              outputPD->CopyTuple(inScalars, newScalars, inPtId, ptIncr+i);
            else
              outputPD->CopyTuple(inScalars, newScalars, cellIdx, ptIncr+i);
          }
        }
      }

      if (inVectors_forColoring && this->ColorMode == VTK_COLOR_BY_VECTOR)
      {
        if (inVectors_forColoring_pd)
          inVectors_forColoring->GetTuple(inPtId, v);
        else
          inVectors_forColoring->GetTuple(cellIdx, v);
        vMag = vtkMath::Norm(v);
        for (i=0; i < numSourcePts; i++)
        {
          newScalars->InsertTuple(i+ptIncr, &vMag);
        }
      }
      else if (haveVectors && this->ColorMode == VTK_COLOR_BY_VECTOR)
      {
        for (i=0; i < numSourcePts; i++)
        {
          newScalars->InsertTuple(i+ptIncr, &vMag);
        }
      }

      // scale data if appropriate
      if ( this->Scaling )
      {
        if ( this->ScaleMode == VTK_DATA_SCALING_OFF )
        {
          scalex = scaley = scalez = this->ScaleFactor;
        }
        else
        {
          scalex *= this->ScaleFactor;
          scaley *= this->ScaleFactor;
          scalez *= this->ScaleFactor;
        }

        if ( scalex == 0.0 )
        {
          scalex = 1.0e-10;
        }
        if ( scaley == 0.0 )
        {
          scaley = 1.0e-10;
        }
        if ( scalez == 0.0 )
        {
          scalez = 1.0e-10;
        }
        trans->Scale(scalex,scaley,scalez);
      }

      if (!haveVectors)
      {
        // If we are using full frame scaling then add an additional
        // transform to undo what fullframe will do.
        // Note that we applied a different correction for vectors,
        // so only apply this one for non-vectors.
        if (this->UseFullFrameScaling)
        {
          trans->Scale(1. / this->FullFrameScaling[0],
                       1. / this->FullFrameScaling[1],
                       1. / this->FullFrameScaling[2]);
        }
      }

      // multiply points and normals by resulting matrix
      trans->TransformPoints(sourcePts,newPts);

      if ( haveNormals )
      {
        trans->TransformNormals(sourceNormals,newNormals);
      }

      // Copy point data from source (if possible)
      if ( pd )
      {
        for (i=0; i < numSourcePts; i++)
        {
          outputPD->CopyData(pd,i,ptIncr+i);
        }
      }

      // If point ids are to be generated, do it here
      if ( this->GeneratePointIds )
      {
        for (i=0; i < numSourcePts; i++)
        {
          pointIds->InsertNextValue(inPtId);
        }
      }

      ptIncr += numSourcePts;
    }
  }

  // Update ourselves and release memory
  //
  outPD->SetPoints(newPts);
  newPts->Delete();

  if (newScalars)
  {
    outputPD->SetScalars(newScalars);
    newScalars->Delete();
  }

  if (newVectors)
  {
    outputPD->SetVectors(newVectors);
    newVectors->Delete();
  }

  if (newNormals)
  {
    outputPD->SetNormals(newNormals);
    newNormals->Delete();
  }
  if (outOrigNodes)
  {
    outputCD->AddArray(outOrigNodes);
    outOrigNodes->Delete();
  }
  if (outOrigCells)
  {
    outputCD->AddArray(outOrigCells);
    outOrigCells->Delete();
  }

  outPD->Squeeze();
  trans->Delete();
  def_mat->Delete();
  pts->Delete();

  if (connSize > 0 && (input->GetDataObjectType() == VTK_POLY_DATA ||
                       input->GetDataObjectType() == VTK_UNSTRUCTURED_GRID))
  {
    vtkPolyData *in_polydata = (vtkPolyData *) input;

    // we have non-verts ... make a separate output for the non-verts and then
    // append them together.
    vtkPolyData *outPD2 = vtkPolyData::New();
    outPD2->SetPoints(in_polydata->GetPoints());
    outPD2->GetPointData()->ShallowCopy(in_polydata->GetPointData());
    // ignore cell data ... it won't match up with glyphed verts anyway.
    outPD2->Allocate(connSize);
    for (i = 0 ; i < numCells ; i++)
    {
      vtkIdType c = input->GetCellType(i);
      if (c == VTK_VERTEX || c == VTK_POLY_VERTEX)
        continue;
      input->GetCellPoints(i, ptIds);
      outPD2->InsertNextCell(c, ptIds);
    }
    vtkAppendPolyData *appender = vtkAppendPolyData::New();
    appender->AddInputData(outPD2);
    appender->AddInputData(outPD);
    appender->Update();
    output->ShallowCopy(appender->GetOutput());
    outPD2->Delete();
    appender->Delete();
  }
  else
     output->ShallowCopy(outPD);
  outPD->Delete();
  ptIds->Delete();

  return 1;
}

//----------------------------------------------------------------------------
// Specify a source object at a specified table location.
void vtkVisItGlyph3D::SetSourceConnection(int id, vtkAlgorithmOutput* algOutput)
{
  if (id < 0)
  {
    vtkErrorMacro("Bad index " << id << " for source.");
    return;
  }

  int numConnections = this->GetNumberOfInputConnections(1);
  if (id < numConnections)
  {
    this->SetNthInputConnection(1, id, algOutput);
  }
  else if (id == numConnections && algOutput)
  {
    this->AddInputConnection(1, algOutput);
  }
  else if (algOutput)
  {
    vtkWarningMacro("The source id provided is larger than the maximum "
                    "source id, using " << numConnections << " instead.");
    this->AddInputConnection(1, algOutput);
  }
}

// ****************************************************************************
//  Method: vtkVisItGlyph3D::RequestUpdateExtent
//
//  Modifications:
//    Eric Brugger, Thu Jan 10 13:05:08 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
// ****************************************************************************

int
vtkVisItGlyph3D::RequestUpdateExtent(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *sourceInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  if (sourceInfo)
  {
    sourceInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER(),
                    0);
    sourceInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES(),
                    1);
    sourceInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(),
                    0);
  }
  inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER(),
              outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()));
  inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES(),
              outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES()));
  inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(),
              outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS()));
  inInfo->Set(vtkStreamingDemandDrivenPipeline::EXACT_EXTENT(), 1);

  return 1;
}

// ****************************************************************************
//  Method: vtkVisItGlyph3D::FillInputPortInformation
//
// ****************************************************************************

int
vtkVisItGlyph3D::FillInputPortInformation(int port, vtkInformation *info)
{
  if (port == 0)
  {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
    return 1;
  }
  else if (port == 1)
  {
    info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
    return 1;
  }
  return 0;
}

// Specify a source object at a specified table location.
void
vtkVisItGlyph3D::SetSourceData(int id, vtkPolyData *pd)
{
  int numConnections = this->GetNumberOfInputConnections(1);

  if (id < 0 || id > numConnections)
  {
    vtkErrorMacro("Bad index " << id << " for source.");
    return;
  }

  vtkTrivialProducer* tp = 0;
  if (pd)
  {
    tp = vtkTrivialProducer::New();
    tp->SetOutput(pd);
  }

  if (id < numConnections)
  {
    if (tp)
    {
      this->SetNthInputConnection(1, id, tp->GetOutputPort());
    }
    else
    {
      this->SetNthInputConnection(1, id, 0);
    }
  }
  else if (id == numConnections && tp)
  {
    this->AddInputConnection(1, tp->GetOutputPort());
  }

  if (tp)
  {
    tp->Delete();
  }
}

// Get a pointer to a source object at a specified table location.
vtkPolyData *
vtkVisItGlyph3D::GetSource(int id)
{
  if ( id < 0 || id >= this->GetNumberOfInputConnections(1) )
  {
    return NULL;
  }
  else
  {
    return vtkPolyData::SafeDownCast(
      this->GetExecutive()->GetInputData(1, id));
  }
}

// ****************************************************************************
//  Method: vtkVisItGlyph3D::PrintSelf
//
// ****************************************************************************

void
vtkVisItGlyph3D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Generate Point Ids "
     << (this->GeneratePointIds ? "On\n" : "Off\n");

  os << indent << "PointIdsName: " << (this->PointIdsName ? this->PointIdsName
       : "(none)") << "\n";

  os << indent << "Color Mode: " << this->GetColorModeAsString() << endl;

  if ( this->GetNumberOfInputConnections(1) < 2 )
  {
    if ( this->GetSource(0) != NULL )
    {
      os << indent << "Source: (" << this->GetSource(0) << ")\n";
    }
    else
    {
      os << indent << "Source: (none)\n";
    }
  }
  else
  {
    os << indent << "A table of " << this->GetNumberOfInputConnections(1) << " glyphs has been defined\n";
  }

  os << indent << "Scaling: " << (this->Scaling ? "On\n" : "Off\n");

  os << indent << "Scale Mode: ";
  if ( this->ScaleMode == VTK_SCALE_BY_SCALAR )
  {
    os << "Scale by scalar\n";
  }
  else if ( this->ScaleMode == VTK_SCALE_BY_VECTOR )
  {
    os << "Scale by vector\n";
  }
  else
  {
    os << "Data scaling is turned off\n";
  }

  os << indent << "Scale Factor: " << this->ScaleFactor << "\n";
  os << indent << "Clamping: " << (this->Clamping ? "On\n" : "Off\n");
  os << indent << "Range: (" << this->Range[0] << ", " << this->Range[1] << ")\n";
  os << indent << "Orient: " << (this->Orient ? "On\n" : "Off\n");
  os << indent << "Orient Mode: " << (this->VectorMode == VTK_USE_VECTOR ?
                                       "Orient by vector\n" : "Orient by normal\n");
  os << indent << "Index Mode: ";
  if ( this->IndexMode == VTK_INDEXING_BY_SCALAR )
  {
    os << "Index by scalar value\n";
  }
  else if ( this->IndexMode == VTK_INDEXING_BY_VECTOR )
  {
    os << "Index by vector value\n";
  }
  else
  {
    os << "Indexing off\n";
  }
  os << indent << "InputScalarsSelection: "
     << (this->InputScalarsSelection ? this->InputScalarsSelection : "(none)") << "\n";
  os << indent << "InputVectorsSelection: "
     << (this->InputVectorsSelection ? this->InputVectorsSelection : "(none)") << "\n";
  os << indent << "InputNormalsSelection: "
     << (this->InputNormalsSelection ? this->InputNormalsSelection : "(none)") << "\n";
}

int
vtkVisItGlyph3D::SetFullFrameScaling(int useIt, const double *s)
{
  int ret = ((useIt != this->UseFullFrameScaling) ||
             (s[0] != this->FullFrameScaling[0]) ||
             (s[1] != this->FullFrameScaling[1]) ||
             (s[2] != this->FullFrameScaling[2])) ? 1 : 0;

  this->UseFullFrameScaling = useIt;

  if(useIt)
  {
    this->FullFrameScaling[0] = s[0];
    this->FullFrameScaling[1] = s[1];
    this->FullFrameScaling[2] = s[2];
  }
  else
  {
    this->FullFrameScaling[0] = 1.;
    this->FullFrameScaling[1] = 1.;
    this->FullFrameScaling[2] = 1.;
  }

  if(ret==1)
    this->Modified();

  return ret;
}
