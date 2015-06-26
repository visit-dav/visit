/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPLOT3DReader.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkPLOT3DReader.h"

#include <vtkDataArray.h>
#include <vtkErrorCode.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkIntArray.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkStructuredGrid.h>
#include <vtkMultiBlockPLOT3DReaderInternals.h>

vtkStandardNewMacro(vtkPLOT3DReader);


vtkPLOT3DReader::vtkPLOT3DReader()
{
  this->NumberOfGrids = 0;
  this->NumberOfPoints = 0;
  this->GridNumber = 0;

  this->Density = NULL;
  this->Momentum = NULL;
  this->Energy = NULL;
  this->Properties = NULL;

  this->GridSizes = NULL;
  this->GridDimensions = NULL;
  this->GridOffsets = NULL;
  this->SolutionOffsets = NULL;
  // parent class sets up scalar function 100 and vector 202 ALWAYS, but
  // we only want what user requests, so remove what parent set up.
  this->SetScalarFunctionNumber(-1);
  this->SetVectorFunctionNumber(-1);
  this->RemoveAllFunctions();
} 

//----------------------------------------------------------------------------
vtkPLOT3DReader::~vtkPLOT3DReader()
{
  if (this->GridSizes)
    delete [] this->GridSizes;
  if (this->GridDimensions)
    delete [] this->GridDimensions;
  if (this->GridOffsets)
    delete [] this->GridOffsets;
  if (this->SolutionOffsets)
    delete [] this->SolutionOffsets;

  if (this->Density)
    {
    this->Density->Delete();
    this->Density = NULL;
    }
  if (this->Momentum)
    {
    this->Momentum->Delete();
    this->Momentum = NULL;
    }
  if (this->Energy)
    {
    this->Energy->Delete();
    this->Energy = NULL;
    }
  if (this->Properties)
    {
    this->Properties->Delete();
    this->Properties = NULL;
    }
}


//----------------------------------------------------------------------------
int vtkPLOT3DReader::GetNumberOfGrids()
{
  if (this->NumberOfGrids == 0)
    {
    FILE* xyzFp;

    if ( this->CheckGeometryFile(xyzFp) != VTK_OK)
      {
      return 0;
      }
    this->CalculateFileSize(xyzFp);
    int numGrids = this->GetNumberOfBlocksInternal(xyzFp, 0);
    fclose(xyzFp);
    return numGrids;
    }
  return this->NumberOfGrids;
}


//----------------------------------------------------------------------------
int
vtkPLOT3DReader::ReadGeometryHeader(FILE* xyzFp, vtkInformationVector *outputVector)
{
  int numGrids = this->GetNumberOfBlocksInternal(xyzFp, 0);
  vtkDebugMacro("Geometry number of grids: " << numGrids);
  if ( numGrids == 0 )
    {
    this->NumberOfGrids = 0; 
    // Bad file, set all extents to invalid.
    int wext[6] = {0, -1, 0, -1, 0, -1};
    outputVector->GetInformationObject(0)->Set(
      vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), wext, 6);
    return VTK_ERROR;
    }

  if (this->NumberOfGrids == 0)
    this->NumberOfGrids = numGrids;
  else if (numGrids != this->NumberOfGrids)
    return VTK_ERROR;
  if (this->GridOffsets == NULL)
    {
    this->GridOffsets = new long[this->NumberOfGrids];
    this->GridSizes = new long[this->NumberOfGrids];
    this->GridDimensions = new int[3*this->NumberOfGrids];

    for (int i = 0; i < this->NumberOfGrids; i++)
        GridOffsets[i] = -1;

    this->SkipByteCount(xyzFp);
    for(int i=0; i < this->NumberOfGrids; i++)
      {
      int ni, nj, nk=1;
      this->ReadIntBlock(xyzFp, 1, &ni);
      this->ReadIntBlock(xyzFp, 1, &nj);
      if (!this->TwoDimensionalGeometry)
        {
        this->ReadIntBlock(xyzFp, 1, &nk);
        }

      this->GridDimensions[0 + 3*i] = ni;
      this->GridDimensions[1 + 3*i] = nj;
      this->GridDimensions[2 + 3*i] = nk;
      vtkDebugMacro("Geometry, block " << i << " dimensions: "
                    << ni << " " << nj << " " << nk);
      }
    this->SkipByteCount(xyzFp); //do we do this now?
    this->GridOffsets[0] = ftell(xyzFp);

    for (int i=0; i<this->NumberOfGrids; i++)
      {
      GridSizes[i] = GridDimensions[3*i]*GridDimensions[1+3*i]*GridDimensions[2+3*i];
      }
    }

  if (0 > this->GridNumber || this->GridNumber >= this->NumberOfGrids)
    {
    vtkErrorMacro(<<"Specified grid not found!");
    return VTK_ERROR;
    }

  return VTK_OK;
}

int vtkPLOT3DReader::ReadQHeader(FILE* qFp)
{
  int numGrid = this->GetNumberOfBlocksInternal(qFp, 0);
  vtkDebugMacro("Q number of grids: " << numGrid);
  if (numGrid != this->NumberOfGrids)
    {
    return VTK_ERROR;
    }

  if (this->SolutionOffsets == NULL)
    {
    this->SolutionOffsets = new long[this->NumberOfGrids];
    for (int i = 0; i < numGrid; ++i)
      this->SolutionOffsets[i] = -1;

    this->SkipByteCount(qFp);

    for(int i = 0; i < numGrid; ++i)
      {
      int ni, nj, nk=1;
      this->ReadIntBlock(qFp, 1, &ni);
      this->ReadIntBlock(qFp, 1, &nj);
      if (!this->TwoDimensionalGeometry)
        {
        this->ReadIntBlock(qFp, 1, &nk);
        }
      vtkDebugMacro("Q, block " << i << " dimensions: "
                    << ni << " " << nj << " " << nk);
      if (this->GridDimensions[    3*i] != ni ||
          this->GridDimensions[1 + 3*i] != nj ||
          this->GridDimensions[2 + 3*i] != nk)
        {
        this->SetErrorCode(vtkErrorCode::FileFormatError);
        vtkErrorMacro("Geometry and data dimensions do not match. "
                      "Data file may be corrupt.");
        return VTK_ERROR;
        }
      }
      this->SkipByteCount(qFp);
      // Get to the location of the fsmach
      this->SkipByteCount(qFp);
      this->SolutionOffsets[0] = ftell(qFp);
    }
  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkPLOT3DReader::ReadFunctionHeader(FILE* fp, vtkIdList*& counts)
{
  int numGrid = this->GetNumberOfBlocksInternal(fp, 0);
  vtkDebugMacro("Function number of grids: " << numGrid);
  if ( numGrid != this->NumberOfGrids )
    {
    return VTK_ERROR;
    }

  this->SkipByteCount(fp);
  counts = vtkIdList::New();
  for(int i=0; i<numGrid; i++)
    {
    int ni, nj, nk, ns;
    this->ReadIntBlock(fp, 1, &ni);
    this->ReadIntBlock(fp, 1, &nj);
    this->ReadIntBlock(fp, 1, &nk);
    this->ReadIntBlock(fp, 1, &ns);
    vtkDebugMacro("Function, block " << i << " dimensions: "
                  << ni << " " << nj << " " << nk
                  << ", " << ns << "Scalars");
    counts->InsertNextId(ns);
    if (this->GridDimensions[    3*i] != ni ||
        this->GridDimensions[1 + 3*i] != nj ||
        this->GridDimensions[2 + 3*i] != nk)
      {
      this->SetErrorCode(vtkErrorCode::FileFormatError);
      vtkErrorMacro("Geometry and data dimensions do not match. "
                    "Data file may be corrupt.");
      return VTK_ERROR;
      }
    }
  this->SkipByteCount(fp);
  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkPLOT3DReader::RequestInformation(
   vtkInformation *vtkNotUsed(request),
   vtkInformationVector **vtkNotUsed(inputVector),
   vtkInformationVector *outputVector)
{
  FILE* xyzFp;

  if ( this->CheckGeometryFile(xyzFp) != VTK_OK)
    {
    return 0;
    }
  this->CalculateFileSize(xyzFp);
 
  if (!this->AutoDetectionCheck(xyzFp))
    {
    vtkWarningMacro("AutoDetectionCheck failed, you may need to specify file format options manually.");
    }
  rewind(xyzFp);

  this->ReadGeometryHeader(xyzFp, outputVector);

  fclose(xyzFp);

  if (this->QFileName && this->QFileName[0] != '\0')
    {
    FILE* qFp;
    if ( this->CheckSolutionFile(qFp) != VTK_OK)
      {
      return 0;
      }

    if ( this->ReadQHeader(qFp) != VTK_OK )
      {
      fclose(qFp);
      return 0;
      }
    if ( this->ReadSolutionProperties(qFp) != VTK_OK )
      {
      fclose(qFp);
      return 0;
      }
    fclose(qFp);
    }
  return 1;
}

//----------------------------------------------------------------------------
int vtkPLOT3DReader::RequestData(
   vtkInformation *vtkNotUsed(request),
   vtkInformationVector **vtkNotUsed(inputVector),
   vtkInformationVector *outputVector)
{
  this->SetErrorCode(vtkErrorCode::NoError);

  FILE* xyzFp;

  vtkInformation *info = outputVector->GetInformationObject(0);
  vtkDataObject *doOutput = info->Get(vtkDataObject::DATA_OBJECT());
  vtkMultiBlockDataSet *mb = vtkMultiBlockDataSet::SafeDownCast(doOutput);
  if (!mb)
    {
    return 0;
    }
  this->SetErrorCode(vtkErrorCode::NoError);
  vtkSmartPointer<vtkStructuredGrid> output = vtkSmartPointer<vtkStructuredGrid>::New();

  if ( this->CheckGeometryFile(xyzFp) != VTK_OK)
    {
    return 0;
    }

  if ( this->ReadGeometryHeader(xyzFp, outputVector) != VTK_OK )
    {
    vtkErrorMacro("Error reading geometry file.");
    fclose(xyzFp);
    return 0;
    }

  if (this->ReadGrid(xyzFp, output) != VTK_OK)
    {
    vtkErrorMacro("Error reading geometry file.");
    fclose(xyzFp);
    return 0;
    }

  // Now read the solution.
  if (this->QFileName && this->QFileName[0] != '\0')
    {
    FILE* qFp;
    if ( this->CheckSolutionFile(qFp) != VTK_OK)
      {
      return 0;
      }

    if ( this->ReadQHeader(qFp) != VTK_OK )
      {
      fclose(qFp);
      return 0;
      }

    if (this->ReadSolution(qFp, output) != VTK_OK)
      {
      vtkErrorMacro("Error reading solution file.");
      fclose(qFp);
      return 0;
      }

    // parent class uses Density (fn == 100), Stagnation Energy (fn == 163)
    // and Momentum (fn == 202) to computate others, and expects these
    // arrays to exist in the output, so add them here, then remove them if
    // not requested.
    bool removeDensity = true;
    bool removeSE = true;
    bool removeMomentum = true;

    // we use the same array for 'Internal' and 'Stagnation' energy, but
    // MapFunction looks for the name 'StagnationEnergy' 

    this->Energy->SetName("StagnationEnergy");
    bool changeSEName = false;

    output->GetPointData()->AddArray(this->Density);
    output->GetPointData()->AddArray(this->Energy);
    output->GetPointData()->AddArray(this->Momentum);
    if ( this->FunctionList->GetNumberOfTuples() > 0 )
      {
      int fnum;
      for (int tup=0; tup < this->FunctionList->GetNumberOfTuples(); tup++)
        {
        if ( (fnum=this->FunctionList->GetValue(tup)) >= 0 )
          {
          if (fnum == 100)
            {
            removeDensity = false;
            }
          else if (fnum == 140)
            {
            removeSE = false;
            changeSEName = true;
            }
          else if (fnum == 163)
            {
            removeSE = false;
            }
          else if (fnum == 202)
            {
            removeMomentum = false;
            }
          else
            {
            this->MapFunction(fnum, output);
            }
          }
        }
      }
    else
      {
      if (this->ScalarFunctionNumber != -1)
        {
        if (this->ScalarFunctionNumber == 100)
          {
          removeDensity = false;
          }
        else if (this->ScalarFunctionNumber == 140)
          {
          removeSE = false;
          changeSEName = true;
          }
        else if (this->ScalarFunctionNumber == 163)
          {
          removeSE = false;
          }
        else
          {
          this->MapFunction(this->ScalarFunctionNumber, output);
          }
        }
      if (this->VectorFunctionNumber != -1)
        {
        if (this->VectorFunctionNumber == 202)
          removeMomentum = false;
        else
          this->MapFunction(this->VectorFunctionNumber, output);
        }
      }
    if (removeDensity)
      output->GetPointData()->RemoveArray("Density");

    if (removeSE)
    {
      output->GetPointData()->RemoveArray("StagnationEnergy");
    }
    else if (changeSEName)
    {
      output->GetPointData()->GetArray("StagnationEnergy")->SetName("InternalEnergy");
    }

    if (removeMomentum)
        output->GetPointData()->RemoveArray("Momentum");
    }
  mb->SetBlock(0, output);
  return 1;
}


//----------------------------------------------------------------------------
void vtkPLOT3DReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


int
vtkPLOT3DReader::ReadGrid(FILE *xyzFp, vtkStructuredGrid *output)
{
  if (0 <= this->GridNumber && this->GridNumber < this->NumberOfGrids)
    {
    this->NumberOfPoints = this->GridSizes[this->GridNumber];
    output->SetDimensions(this->GridDimensions[  3*this->GridNumber],
                          this->GridDimensions[1+3*this->GridNumber],
                          this->GridDimensions[2+3*this->GridNumber]);
    }
  else
    {
    vtkErrorMacro (<<"Specified grid not found!");
    return VTK_ERROR;
    }

  vtkDataArray *pointArray = this->NewFloatArray();
  pointArray->SetNumberOfComponents(3);
  pointArray->SetNumberOfTuples(this->NumberOfPoints);

  vtkPoints *newPts = vtkPoints::New();
  newPts->SetData(pointArray);
  pointArray->Delete();
  output->SetPoints(newPts);
  newPts->Delete();

  rewind(xyzFp);
  long offset = this->ComputeGridOffset(xyzFp);
  fseek(xyzFp, offset, SEEK_SET);

  this->SkipByteCount(xyzFp);
  int d = this->Internal->NumberOfDimensions;
  if (this->ReadVector(xyzFp, this->NumberOfPoints, d, pointArray) == 0)
    {
    vtkErrorMacro("Encountered premature end-of-file while reading "
                  "the geometry file (or the file is corrupt).");
    this->SetErrorCode(vtkErrorCode::PrematureEndOfFileError);
    fclose(xyzFp);
    return VTK_ERROR;
    }

  // This is where we would read IBlanking information, but VisIt currently 
  //  doesn't do anything with it
  // 
#if 0
  if (this->Internal->IBlanking)
    {
    }
#endif
  fclose(xyzFp);
  return VTK_OK;
}

long
vtkPLOT3DReader::ComputeGridOffset(FILE *xyzFp)
{
  // If offset known return it else compute.
  if (this->GridOffsets[this->GridNumber] <= 0)
    {
    int i;
    //  Starting from current grid back up until last known offset.
    for (i = this->GridNumber; i>0 && this->GridOffsets[i] < 0; i--)
      {
      ;
      }

    long nd = this->Internal->NumberOfDimensions;
    long bc = this->Internal->HasByteCount ? sizeof(int) : 0;

    for (int j = i+1; j <= this->GridNumber; j++)
      {
      if (this->Internal->BinaryFile)
        {
        if (this->Internal->IBlanking)
          {
          this->GridOffsets[j] = (this->GridOffsets[j-1] + (nd+1)*this->GridSizes[j-1]*4) + bc;
          }
        else
          {
            this->GridOffsets[j] = (this->GridOffsets[j-1] + nd*this->GridSizes[j-1]*this->Internal->Precision) + 2*bc;
          }
        }
      else
        {
        int numberOfElements;
        if (this->Internal->IBlanking)
          {
          numberOfElements = (nd+1)*GridSizes[j-1];
          }
        else
          {
          numberOfElements = (nd)*GridSizes[j-1];
          }
        fseek(xyzFp,this->GridOffsets[j-1],SEEK_SET);
        vtkDataArray *numbersToSkip = this->NewFloatArray();
        numbersToSkip->SetNumberOfTuples(numberOfElements);
        this->ReadScalar(xyzFp,numberOfElements,numbersToSkip);
        numbersToSkip->Delete();
        this->GridOffsets[j] = ftell(xyzFp);
        }
      }
    }
  return this->GridOffsets[this->GridNumber];
}

int
vtkPLOT3DReader::ReadSolutionProperties(FILE *qFp)
{
  if (!(this->GridNumber >= 0) && (this->GridNumber < this->NumberOfGrids))
    {
    vtkErrorMacro (<<"Specified grid not found!");
    return VTK_ERROR;
    }

  //seek to correct spot and read solution
  rewind(qFp);
  long offset = this->ComputeSolutionOffset(qFp);
  fseek (qFp, offset, SEEK_SET);

  // read parameters
  vtkDataArray *newProp = this->NewFloatArray();
  newProp->SetNumberOfTuples(4);
  newProp->SetName("Properties");
  if (this->ReadScalar(qFp, 4, newProp) == 0)
    {
    vtkErrorMacro("Encountered premature end-of-file while reading "
                  "the q file for properties (or the file is corrupt).");
    this->SetErrorCode(vtkErrorCode::PrematureEndOfFileError);
    newProp->Delete();
    return VTK_ERROR;
    }
  this->SkipByteCount(qFp);
  //if (this->GridNumber == 0)
    {
    this->Properties = newProp;
    this->Properties->Register(this);
    }
  newProp->Delete();
  return VTK_OK;
}

int
vtkPLOT3DReader::ReadSolution(FILE *qFp, vtkStructuredGrid *output)
{
  if(this->ReadSolutionProperties(qFp) == VTK_ERROR)
      return VTK_ERROR;

  this->SkipByteCount(qFp);

  int numPts = this->GridSizes[this->GridNumber];
  vtkDataArray* newDensity = this->NewFloatArray();
  newDensity->SetNumberOfComponents(1);
  newDensity->SetNumberOfTuples(numPts);
  newDensity->SetName("Density");
  if ( this->ReadScalar(qFp, numPts, newDensity) == 0)
    {
    vtkErrorMacro("Encountered premature end-of-file while reading "
                  "the q file for density (or the file is corrupt).");
    this->SetErrorCode(vtkErrorCode::PrematureEndOfFileError);
    newDensity->Delete();
    fclose(qFp);
    return VTK_ERROR;
    }

  vtkDataArray* newMomentum = this->NewFloatArray();
  newMomentum->SetNumberOfComponents(3);
  newMomentum->SetNumberOfTuples(numPts);
  newMomentum->SetName("Momentum");
  int d = this->TwoDimensionalGeometry ? 2 : 3;
  if ( this->ReadVector(qFp, numPts, d, newMomentum) == 0)
    {
    vtkErrorMacro("Encountered premature end-of-file while reading "
                  "the q file for density (or the file is corrupt).");
    this->SetErrorCode(vtkErrorCode::PrematureEndOfFileError);
    newDensity->Delete();
    newMomentum->Delete();
    fclose(qFp);
    return VTK_ERROR;
    }

  vtkDataArray* newEnergy = this->NewFloatArray();
  newEnergy->SetNumberOfComponents(1);
  newEnergy->SetNumberOfTuples(numPts);
  newEnergy->SetName("StagnationEnergy");
  if (this->ReadScalar(qFp, numPts, newEnergy) == 0)
    {
    vtkErrorMacro("Encountered premature end-of-file while reading "
                      "the q file (or the file is corrupt).");
    newDensity->Delete();
    newMomentum->Delete();
    newEnergy->Delete();
    fclose(qFp);
    return VTK_ERROR;
    }

  this->Density = newDensity;
  this->Density->Register(this);
  newDensity->Delete();
  this->Momentum = newMomentum;
  this->Momentum->Register(this);
  newMomentum->Delete();
  this->Energy = newEnergy;
  this->Energy->Register(this);
  newEnergy->Delete();

  fclose(qFp);
  return VTK_OK;
}

long
vtkPLOT3DReader::ComputeSolutionOffset(FILE *qFp)
{
  int i;

  //  If solution offset not known, compute it.
  if (this->SolutionOffsets[this->GridNumber] <= 0)
    {
    // back up until last known offset.
    for (i = this->GridNumber; i > 0 && this->SolutionOffsets[i]< 0; i--)
      {
      ;
      }

    for (int j = i+1; j<= this->GridNumber; j++)
      {
      // Number of scalars to  be read: 1 for density, 1 for energy and
      // NumDims for Momentum
      int ns = 1  + 1  + this->Internal->NumberOfDimensions;
      if (this->Internal->BinaryFile)
        {
        int n = 4;
        long bc = this->Internal->HasByteCount ? sizeof(int) : 0;
        int mult = this->Internal->Precision;
        this->SolutionOffsets[j] = this->SolutionOffsets[j-1] +
                             n * mult + 2*bc +
                             ns * this->GridSizes[j-1]*mult + 2*bc;
        }
      else
        {
        int numberOfElements = 4 + ns*GridSizes[j-1];
        fseek(qFp,this->SolutionOffsets[j-1],SEEK_SET);
        vtkDataArray *numbersToSkip = this->NewFloatArray();
        numbersToSkip->SetNumberOfTuples(numberOfElements);
        this->ReadScalar(qFp,numberOfElements,numbersToSkip);
        numbersToSkip->Delete();
        this->SolutionOffsets[j] = ftell(qFp);
        }
      }
    }
  return this->SolutionOffsets[this->GridNumber];
}

vtkDataSet*
vtkPLOT3DReader::GetCurrentBlock()
{
  vtkMultiBlockDataSet *mb = GetOutput();
  vtkDataSet *rv = NULL;
  if (mb != NULL)
    {
    vtkDataObject *obj = mb->GetBlock(0);
    if (obj != NULL)
      {
      rv = vtkStructuredGrid::SafeDownCast(obj);
      }
    }
  return rv;
}
