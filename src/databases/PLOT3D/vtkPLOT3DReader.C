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

#include <vtkByteSwap.h>
#include <vtkErrorCode.h>
#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkIntArray.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkDataArrayCollection.h>

vtkStandardNewMacro(vtkPLOT3DReader);

#define VTK_RHOINF 1.0
#define VTK_CINF 1.0
#define VTK_PINF ((VTK_RHOINF*VTK_CINF) * (VTK_RHOINF*VTK_CINF) / this->Gamma)
#define VTK_CV (this->R / (this->Gamma-1.0))

vtkPLOT3DReader::vtkPLOT3DReader()
{
  this->XYZFileName = NULL;
  this->QFileName = NULL;
  this->FunctionFileName = NULL;
  this->BinaryFile = 1;
  this->HasByteCount = 0;
  this->FileSize = 0;
  this->MultiGrid = 0;
  this->ForceRead = 0;
  this->ByteOrder = FILE_BIG_ENDIAN;
  this->IBlanking = 0;
  this->TwoDimensionalGeometry = 0;
  this->DoublePrecision = 0;
  this->NumberOfGrids = 0;
  this->NumberOfPoints = 0;

  this->R = 1.0;
  this->Gamma = 1.4;
  this->Uvinf = 0.0;
  this->Vvinf = 0.0;
  this->Wvinf = 0.0;

  this->FunctionList = vtkIntArray::New();

  this->ScalarFunctionNumber = -1;
  this->SetScalarFunctionNumber(100);
  this->VectorFunctionNumber = -1;
  this->SetVectorFunctionNumber(202);

  this->GridNumber = 0;

  this->Density = NULL;
  this->Momentum = NULL;
  this->Energy = NULL;
  this->Properties = NULL;

  this->GridSizes = NULL;
  this->GridDimensions = NULL;
  this->GridOffsets = NULL;
  this->SolutionOffsets = NULL;

  this->SetNumberOfInputPorts(0);
} 

//----------------------------------------------------------------------------
vtkPLOT3DReader::~vtkPLOT3DReader()
{
  delete [] this->XYZFileName;
  delete [] this->QFileName;
  delete [] this->FunctionFileName;
  this->FunctionList->Delete();
}


//----------------------------------------------------------------------------
int vtkPLOT3DReader::CheckFile(FILE*& fp, const char* fname)
{
  if (this->BinaryFile)
    {
    fp = fopen(fname, "rb");
    }
  else
    {
    fp = fopen(fname, "r");
    }
  if ( fp == NULL)
    {
    this->SetErrorCode(vtkErrorCode::FileNotFoundError);
    vtkErrorMacro(<< "File: " << fname << " not found.");
    return VTK_ERROR;
    }
  return VTK_OK;
}

//----------------------------------------------------------------------------
int vtkPLOT3DReader::CheckGeometryFile(FILE*& xyzFp)
{
  if ( this->XYZFileName == NULL || this->XYZFileName[0] == '\0' )
    {
    this->SetErrorCode(vtkErrorCode::NoFileNameError);
    vtkErrorMacro(<< "Must specify geometry file");
    return VTK_ERROR;
    }
  return this->CheckFile(xyzFp, this->XYZFileName);
}

//----------------------------------------------------------------------------
int vtkPLOT3DReader::CheckSolutionFile(FILE*& qFp)
{
  if ( this->QFileName == NULL || this->QFileName[0] == '\0' )
    {
    this->SetErrorCode(vtkErrorCode::NoFileNameError);
    vtkErrorMacro(<< "Must specify solution (Q) file");
    return VTK_ERROR;
    }
  return this->CheckFile(qFp, this->QFileName);
}

//----------------------------------------------------------------------------
int vtkPLOT3DReader::CheckFunctionFile(FILE*& fFp)
{
  if ( this->FunctionFileName == NULL || this->FunctionFileName[0] == '\0' )
    {
    this->SetErrorCode(vtkErrorCode::NoFileNameError);
    vtkErrorMacro(<< "Must specify function file");
    return VTK_ERROR;
    }
  return this->CheckFile(fFp, this->FunctionFileName);
}

//----------------------------------------------------------------------------
// Skip Fortran style byte count.
void vtkPLOT3DReader::SkipByteCount(FILE* fp)
{
  if (this->BinaryFile && this->HasByteCount)
    {
    int tmp;
    if (fread(&tmp, sizeof(int), 1, fp) != 1)
      {
      vtkErrorMacro ("PLOT3DReader error reading file: " << this->XYZFileName
                     << " Premature EOF while reading skipping byte count.");
      fclose (fp);
      return;
      }
    }
}

//----------------------------------------------------------------------------
// Read a block of ints (ascii or binary) and return number read.
int vtkPLOT3DReader::ReadIntBlock(FILE* fp, int n, int* block)
{
  if (this->BinaryFile)
    {
    int retVal=static_cast<int>(fread(block, sizeof(int), n, fp));
    if (this->ByteOrder == FILE_LITTLE_ENDIAN)
      {
      vtkByteSwap::Swap4LERange(block, n);
      }
    else
      {
      vtkByteSwap::Swap4BERange(block, n);
      }
    return retVal;
    }
  else
    {
    int count = 0;
    for(int i=0; i<n; i++)
      {
      int num = fscanf(fp, "%d", &(block[i]));
      if ( num > 0 )
        {
        count++;
        }
      else
        {
        return 0;
        }
      }
    return count;
    }
}

template <class T>
int vtkPLOT3DReader_ReadScalarB(FILE *fp, int n, int byteOrder, T *block)
{
  int retVal = static_cast<int>(fread(block, sizeof(T), n, fp));
  if (byteOrder) // 1 == LITTLE_ENDIAN
    {
    if (sizeof(T) == 4)
      {
      vtkByteSwap::Swap4LERange(block, n);
      }
    else
      {
      vtkByteSwap::Swap8LERange(block, n);
      }
    }
  else
    {
    if (sizeof(T) == 4)
      {
      vtkByteSwap::Swap4BERange(block, n);
      }
    else
      {
      vtkByteSwap::Swap8BERange(block, n);
      }
    }
  return retVal;
}

template <class T>
int vtkPLOT3DReader_ReadScalarA(FILE *fp, int n, T *block)
{
  int count = 0;
  std::string format;
  if (sizeof(T) == 4)
    format = "%f";
  else 
    format = "%lf";
  for(int i=0; i<n; i++)
    {
    int num = fscanf(fp, format.c_str(), &(block[i]));
    if ( num > 0 )
      {
      count++;
      }
    else
      {
      return 0;
      }
    }
  return count;
}

int
vtkPLOT3DReader::ReadScalar(FILE *fp, int n, vtkDataArray *a)
{
  int retval;
  if(a->GetDataType() == VTK_FLOAT)
    {
    if (this->BinaryFile)
      {
      retval = vtkPLOT3DReader_ReadScalarB<float>(fp, n,
          this->ByteOrder,
          (float*)((vtkFloatArray*)a)->GetVoidPointer(0));
      }
    else
      {
      retval = vtkPLOT3DReader_ReadScalarA<float>(fp, n,
          (float*)((vtkFloatArray*)a)->GetVoidPointer(0));
      }
    }
  else
    {
    if (this->BinaryFile)
      {
      retval = vtkPLOT3DReader_ReadScalarB<double>(fp, n,
          this->ByteOrder,
          (double*)((vtkDoubleArray*)a)->GetVoidPointer(0));
      }
    else
      {
      retval = vtkPLOT3DReader_ReadScalarA<double>(fp, n,
        (double*)((vtkDoubleArray*)a)->GetVoidPointer(0));
      }
    }
  return retval;
}

template <class T>
int
vtkPLOT3DReader_ReadVector(FILE *fp, int n, int numDim, int byteOrder,
  int binaryFile, T *v)
{
  // Setting to 0 in case numDims == 2. We still need to
  // populate an array with 3 components but the code below
  // does not read the 3rd component (it doesn't exist
  // in the file)
  memset(v, 0, n*3*sizeof(T));

  int retVal = 0;
  T* buffer = new T[n];
  for (int component = 0; component < numDim; component++)
    {
    if (binaryFile)
      retVal += vtkPLOT3DReader_ReadScalarB<T>(fp, n, byteOrder, buffer);
    else
      retVal += vtkPLOT3DReader_ReadScalarA<T>(fp, n, buffer);

    for (int i=0; i<n; ++i)
      {
      v[3*i+component] = buffer[i];
      }
    }
  delete[] buffer;

  return retVal;
}

int
vtkPLOT3DReader::ReadVector(FILE *fp, int n, vtkDataArray *v)
{
  int retval;
  int numDim = this->TwoDimensionalGeometry ? 2 : 3;
  if(v->GetDataType() == VTK_FLOAT)
    {
    retval = vtkPLOT3DReader_ReadVector<float>(fp, n, numDim,
        this->ByteOrder, this->BinaryFile,
        (float*)((vtkFloatArray*)v)->GetVoidPointer(0));
    }
  else
    {
    retval = vtkPLOT3DReader_ReadVector<double>(fp, n, numDim,
        this->ByteOrder, this->BinaryFile,
        (double*)((vtkDoubleArray*)v)->GetVoidPointer(0));
    }
  return retval;
}

//----------------------------------------------------------------------------
// Read a block of floats (ascii or binary) and return number read.
void vtkPLOT3DReader::CalculateFileSize(FILE* fp)
{
  long curPos = ftell(fp);
  fseek(fp, 0, SEEK_END);
  this->FileSize = ftell(fp);
  fseek(fp, curPos, SEEK_SET);
}


//----------------------------------------------------------------------------
// Estimate the size of a grid (binary file only)
long vtkPLOT3DReader::EstimateSize(int ni, int nj, int nk)
{
  long size; // the header portion, 3 ints
  if (!this->TwoDimensionalGeometry)
    {
    size = 3*4;
    size += ni*nj*nk*3*4; // x, y, z
    }
  else
    {
    size = 2*4;
    size += ni*nj*nk*2*4; // x, y, z
    }
  if (this->HasByteCount)
    {
    size += 2*4; // the byte counts
    }
  if (this->IBlanking)
    {
    size += ni*nj*nk*4;
    }

  return size;
}

//----------------------------------------------------------------------------
int vtkPLOT3DReader::CanReadBinaryFile(const char* fname)
{
  FILE* xyzFp;

  if (!fname || fname[0] == '\0')
    {
    return 0;
    }

  if ( this->CheckFile(xyzFp, fname) != VTK_OK)
    {
    return 0;
    }

  this->CalculateFileSize(xyzFp);

  int numGrids = this->FindNumberOfGrids(xyzFp, 1);
  fclose(xyzFp);
  if (numGrids != 0)
    {
    return 1;
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkPLOT3DReader::GetNumberOfGrids()
{
  FILE* xyzFp;

  if ( this->CheckGeometryFile(xyzFp) != VTK_OK)
    {
    return 0;
    }
  this->CalculateFileSize(xyzFp);
  int numGrids = this->FindNumberOfGrids(xyzFp, 1);
  fclose(xyzFp);
  if (numGrids != 0)
    {
    return numGrids;
    }
  return 1;
}


//----------------------------------------------------------------------------
// Read the header and return the number of grids.
int vtkPLOT3DReader::FindNumberOfGrids(FILE* xyzFp, int verify)
{
  int numGrid=0;

  if ( this->MultiGrid )
    {
    this->SkipByteCount(xyzFp);
    this->ReadIntBlock(xyzFp, 1, &numGrid);
    this->SkipByteCount(xyzFp);
    }
  else
    {
    numGrid=1;
    }

  if (verify)
    {
    // We were told to make sure that the file can really contain
    // the number of grid in the header (we can only check this
    // if file is binary)
    int error=0;
    if ( this->BinaryFile )
      {
      // Store the beginning of first grid.
      long pos = ftell(xyzFp);

      long fileSize = 0;
      // Size of number of grids information.
      if ( this->MultiGrid )
        {
        fileSize += 4; // numGrids
        if (this->HasByteCount)
          {
          fileSize += 4*4; // byte counts for the header
          }
        }
      // Add the size of each grid.
      this->SkipByteCount(xyzFp);
      for(int i=0; i<numGrid; i++)
        {
        int ni, nj, nk;
        this->ReadIntBlock(xyzFp, 1, &ni);
        this->ReadIntBlock(xyzFp, 1, &nj);
        if (!this->TwoDimensionalGeometry)
          {
          this->ReadIntBlock(xyzFp, 1, &nk);
          }
        else
          {
          nk = 1;
          }
        fileSize += this->EstimateSize(ni, nj, nk);
        // If this number is larger than the file size, there
        // is something wrong.
        if ( fileSize > this->FileSize )
          {
          error = 1;
          break;
          }
        }
      this->SkipByteCount(xyzFp);
      // If this number is different than the file size, there
      // is something wrong.
      if ( fileSize != this->FileSize && !this->ForceRead)
        {
        this->SetErrorCode(vtkErrorCode::FileFormatError);
        error = 1;
        }

      fseek(xyzFp, pos, SEEK_SET);
      }
    else
      {
      if (numGrid == 0)
        {
        this->SetErrorCode(vtkErrorCode::FileFormatError);
        }
      }
    }
    
  return numGrid;
}

//----------------------------------------------------------------------------
int
vtkPLOT3DReader::ReadGeometryHeader(FILE* xyzFp, vtkInformationVector *outputVector)
{
  int numGrids = this->FindNumberOfGrids(xyzFp, 1);
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
    this->GridOffsets = new int[this->NumberOfGrids];
    this->GridSizes = new int[this->NumberOfGrids];
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

  if (0 <= this->GridNumber && this->GridNumber < this->NumberOfGrids)
    {
    int wext[6] = {0, this->GridDimensions[  3*this->GridNumber]-1,
                   0, this->GridDimensions[1+3*this->GridNumber]-1,
                   0, this->GridDimensions[2+3*this->GridNumber]-1};
    outputVector->GetInformationObject(0)->Set(
      vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), wext, 6);
    }
  else
    {
    vtkErrorMacro(<<"Specified grid not found!");
    return VTK_ERROR;
    }

  return VTK_OK;
}

int vtkPLOT3DReader::ReadQHeader(FILE* qFp)
{
  int numGrid = this->FindNumberOfGrids(qFp, 0);
  vtkDebugMacro("Q number of grids: " << numGrid);
  if (numGrid != this->NumberOfGrids)
    {
    return VTK_ERROR;
    }

  if (this->SolutionOffsets == NULL)
    {
    this->SolutionOffsets = new int[this->NumberOfGrids];
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
  int numGrid = this->FindNumberOfGrids(fp, 0);
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
void vtkPLOT3DReader::SetXYZFileName( const char* name )
{
  if ( this->XYZFileName && ! strcmp( this->XYZFileName, name ) )
    {
    return;
    }

  if ( this->XYZFileName )
    {
    delete [] this->XYZFileName;
    }

  if ( name )
    {
    this->XYZFileName = new char [ strlen( name ) + 1 ];
    strcpy( this->XYZFileName, name );
    }
  else
    {
    this->XYZFileName = 0;
    }

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkPLOT3DReader::SetScalarFunctionNumber(int num)
{
  if ( this->ScalarFunctionNumber == num)
    {
    return;
    }
  if (num >= 0)
    {
    // If this function is not in the list, add it.
    int found=0;
    for (int i=0; i < this->FunctionList->GetNumberOfTuples(); i++ )
      {
      if ( this->FunctionList->GetValue(i) == num )
        {
        found=1;
        }
      }
    if (!found)
      {
      this->AddFunction(num);
      }
    }
  this->ScalarFunctionNumber = num;
}

//----------------------------------------------------------------------------
void vtkPLOT3DReader::SetVectorFunctionNumber(int num)
{
  if ( this->VectorFunctionNumber == num)
    {
    return;
    }
  if (num >= 0)
    {
    // If this function is not in the list, add it.
    int found=0;
    for (int i=0; i < this->FunctionList->GetNumberOfTuples(); i++ )
      {
      if ( this->FunctionList->GetValue(i) == num )
        {
        found=1;
        }
      }
    if (!found)
      {
      this->AddFunction(num);
      }
    }
  this->VectorFunctionNumber = num;
}

//----------------------------------------------------------------------------
void vtkPLOT3DReader::RemoveFunction(int fnum)
{
  for (int i=0; i < this->FunctionList->GetNumberOfTuples(); i++ )
    {
    if ( this->FunctionList->GetValue(i) == fnum )
      {
      this->FunctionList->SetValue(i,-1);
      this->Modified();
      }
    }
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

  long curPos = ftell(xyzFp);
  this->CalculateFileSize(xyzFp);
  curPos = ftell(xyzFp);
  this->ReadGeometryHeader(xyzFp, outputVector);

  fclose(xyzFp);

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

  vtkStructuredGrid *output = vtkStructuredGrid::GetData(outputVector);

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

    vtkPointData *outPD = output->GetPointData();
    this->MapFunction(this->ScalarFunctionNumber, outPD);
    this->MapFunction(this->VectorFunctionNumber, outPD);
    if ( this->FunctionList->GetNumberOfTuples() > 0 )
      {
      int fnum;
      for (int tup=0; tup < this->FunctionList->GetNumberOfTuples(); tup++)
        {
        if ( (fnum=this->FunctionList->GetValue(tup)) >= 0 )
          {
          this->MapFunction(fnum, outPD);
          }
        }
      }
    }

#if 0
  if (this->FunctionFileName && this->FunctionFileName[0] != '\0') 
    {
    FILE* fFp;
    vtkIdList *arrayCounts;
    if ( this->CheckFunctionFile(fFp) != VTK_OK)
      {
      return;
      }
    
    if ( this->ReadFunctionHeader(fFp, arrayCounts) != VTK_OK )
      {
      fclose(fFp);
      return;
      }
    for(i=0; i<this->NumberOfOutputs; i++)
      {
      vtkStructuredGrid* nthOutput = this->GetOutput(i);

      int dims[6];
      int scalarId;
      nthOutput->GetWholeExtent(dims);
      nthOutput->SetExtent(dims);
      nthOutput->GetDimensions(dims);

      this->SkipByteCount(fFp);

      for(scalarId=0; scalarId<arrayCounts->GetId(i); scalarId++)
        {
        vtkFloatArray *scalars = vtkFloatArray::New();
        char fName[12];
        scalars->SetNumberOfComponents(1);
        scalars->SetNumberOfTuples(dims[0]*dims[1]*dims[2] );
        sprintf(fName, "Function%i", scalarId);
        scalars->SetName(fName);
        float *sen = scalars->GetPointer(0);
        if (this->ReadFloatBlock(fFp, dims[0]*dims[1]*dims[2], sen) == 0)
          {
          vtkErrorMacro("Encountered premature end-of-file while "
                        "reading the Function file (or the file is corrupt).");
          fclose(fFp);
          return;
          }
        nthOutput->GetPointData()->AddArray(scalars);
        scalars->Delete();
        }
      this->SkipByteCount(fFp);
      }
    fclose(fFp);
    }
#endif
  return 1;
}

//----------------------------------------------------------------------------
// Various PLOT3D functions.....................

void vtkPLOT3DReader::MapFunction(int fNumber, vtkPointData *outPD)
{
  switch (fNumber)
    {
    case -1: // Do nothing
      break;
    case 100: //Density
      this->ComputeDensity(outPD);
      break;

    case 110: //Pressure
      this->ComputePressure(outPD);
      break;

    case 120: //Temperature
      this->ComputeTemperature(outPD);
      break;

    case 130: //Enthalpy
      this->ComputeEnthalpy(outPD);
      break;

    case 140: //Internal Energy
      this->ComputeInternalEnergy(outPD);
      break;

    case 144: //Kinetic Energy
      this->ComputeKineticEnergy(outPD);
      break;

    case 153: //Velocity Magnitude
      this->ComputeVelocityMagnitude(outPD);
      break;

    case 163: //Stagnation energy
      this->ComputeStagnationEnergy(outPD);
      break;

    case 170: //Entropy
      this->ComputeEntropy(outPD);
      break;

    case 184: //Swirl
      this->ComputeSwirl(outPD);
      break;

    case 200: //Velocity
      this->ComputeVelocity(outPD);
      break;

    case 201: //Vorticity
      this->ComputeVorticity(outPD);
      break;

    case 202: //Momentum
      this->ComputeMomentum(outPD);
      break;

    case 210: //PressureGradient
      this->ComputePressureGradient(outPD);
      break;

    case 211: //Vorticity Magnitude
      this->ComputeVorticityMagnitude(outPD);
      break;

    case 212: //Strain Rate
      this->ComputeStrainRate(outPD);
      break;

    default:
      vtkErrorMacro(<<"No function number " << fNumber);
    }
}

void vtkPLOT3DReader::ComputeDensity(vtkPointData *outputPD)
{
  outputPD->AddArray(this->Density);
  outputPD->SetActiveScalars("Density");
  vtkDebugMacro(<<"Created density scalar");
}

//----------------------------------------------------------------------------
void vtkPLOT3DReader::ComputeTemperature(vtkPointData *outputPD)
{
  double *m, e, rr, u, v, w, v2, p, d, rrgas;
  vtkIdType i;
  vtkDataArray *temperature;

  //  Check that the required data is available
  //
  if ( this->Density == NULL || this->Momentum == NULL || 
       this->Energy == NULL )
    {
    vtkErrorMacro(<<"Cannot compute temperature");
    return;
    }

  vtkIdType numPts = this->Density->GetNumberOfTuples();
  temperature = this->NewFloatArray();
  temperature->SetNumberOfTuples(numPts);

  //  Compute the temperature
  //
  rrgas = 1.0 / this->R;
  for (i=0; i < numPts; i++) 
    {
    d = this->Density->GetComponent(i,0);
    d = (d != 0.0 ? d : 1.0);
    m = this->Momentum->GetTuple(i);
    e = this->Energy->GetComponent(i,0);
    rr = 1.0 / d;
    u = m[0] * rr;
    v = m[1] * rr;
    w = m[2] * rr;
    v2 = u*u + v*v + w*w;
    p = (this->Gamma-1.) * (e - 0.5 * d * v2);
    temperature->SetTuple1(i, p*rr*rrgas);
  }

  temperature->SetName("Temperature");
  outputPD->AddArray(temperature);
  outputPD->SetActiveScalars("Temperature");
  temperature->Delete();
  vtkDebugMacro(<<"Created temperature scalar");
}

//----------------------------------------------------------------------------
void vtkPLOT3DReader::ComputePressure(vtkPointData* outputPD)
{
  double *m, e, u, v, w, v2, p, d, rr;
  vtkIdType i;
  vtkDataArray *pressure;

  //  Check that the required data is available
  //
  if ( this->Density == NULL || this->Momentum == NULL ||
       this->Energy == NULL )
    {
    vtkErrorMacro(<<"Cannot compute pressure");
    return;
    }

  vtkIdType numPts = this->Density->GetNumberOfTuples();
  pressure = this->NewFloatArray();
  pressure->SetNumberOfTuples(numPts);

  //  Compute the pressure
  //
  for (i=0; i < numPts; i++) 
    {
    d = this->Density->GetComponent(i,0);
    d = (d != 0.0 ? d : 1.0);
    m = this->Momentum->GetTuple(i);
    e = this->Energy->GetComponent(i,0);
    rr = 1.0 / d;
    u = m[0] * rr;
    v = m[1] * rr;
    w = m[2] * rr;
    v2 = u*u + v*v + w*w;
    p = (this->Gamma-1.) * (e - 0.5 * d * v2);
    pressure->SetTuple1(i, p);
  }

  pressure->SetName("Pressure");
  outputPD->AddArray(pressure);
  outputPD->SetActiveScalars("Pressure");
  pressure->Delete();
  vtkDebugMacro(<<"Created pressure scalar");
}

//----------------------------------------------------------------------------
void vtkPLOT3DReader::ComputeEnthalpy(vtkPointData *outputPD)
{
  double *m, e, u, v, w, v2, d, rr;
  vtkIdType i;
  vtkDataArray *enthalpy;

  //  Check that the required data is available
  //
  if ( this->Density == NULL || this->Momentum == NULL ||
       this->Energy == NULL )
    {
    vtkErrorMacro(<<"Cannot compute enthalpy");
    return;
    }

  vtkIdType numPts = this->Density->GetNumberOfTuples();
  enthalpy = this->NewFloatArray();
  enthalpy->SetNumberOfTuples(numPts);

  //  Compute the enthalpy
  //
  for (i=0; i < numPts; i++) 
    {
    d = this->Density->GetComponent(i,0);
    d = (d != 0.0 ? d : 1.0);
    m = this->Momentum->GetTuple(i);
    e = this->Energy->GetComponent(i,0);
    rr = 1.0 / d;
    u = m[0] * rr;
    v = m[1] * rr;
    w = m[2] * rr;
    v2 = u*u + v*v + w*w;
    enthalpy->SetTuple1(i, this->Gamma*(e*rr - 0.5*v2));
  }
  enthalpy->SetName("Enthalpy");
  outputPD->AddArray(enthalpy);
  outputPD->SetActiveScalars("Enthalpy");
  enthalpy->Delete();
  vtkDebugMacro(<<"Created enthalpy scalar");
}

//----------------------------------------------------------------------------
void vtkPLOT3DReader::ComputeInternalEnergy(vtkPointData *outputPD)
{
  this->Energy->SetName("InternalEnergy");
  outputPD->AddArray(this->Energy);
  outputPD->SetActiveScalars("InternalEnergy");
  vtkDebugMacro(<<"Created internal energy scalar");
}

//----------------------------------------------------------------------------
void vtkPLOT3DReader::ComputeStagnationEnergy(vtkPointData *outputPD)
{
  this->Energy->SetName("StagnationEnergy");
  outputPD->AddArray(this->Energy);
  outputPD->SetActiveScalars("StagnationEnergy");
  vtkDebugMacro(<<"Created stagnation energy scalar");
}

//----------------------------------------------------------------------------
void vtkPLOT3DReader::ComputeKineticEnergy(vtkPointData *outputPD)
{
  double *m, u, v, w, v2, d, rr;
  vtkIdType i;
  vtkDataArray *kineticEnergy;

  //  Check that the required data is available
  //
  if ( this->Density == NULL || this->Momentum == NULL )
    {
    vtkErrorMacro(<<"Cannot compute kinetic energy");
    return;
    }

  vtkIdType numPts = this->Density->GetNumberOfTuples();
  kineticEnergy = this->NewFloatArray();
  kineticEnergy->SetNumberOfTuples(numPts);

  //  Compute the kinetic energy
  //
  for (i=0; i < numPts; i++)
    {
    d = this->Density->GetComponent(i,0);
    d = (d != 0.0 ? d : 1.0);
    m = this->Momentum->GetTuple(i);
    rr = 1.0 / d;
    u = m[0] * rr;
    v = m[1] * rr;
    w = m[2] * rr;
    v2 = u*u + v*v + w*w;
    kineticEnergy->SetTuple1(i, 0.5*v2);
  }
  kineticEnergy->SetName("KineticEnergy");
  outputPD->AddArray(kineticEnergy);
  outputPD->SetActiveScalars("KineticEnergy");
  kineticEnergy->Delete();
  vtkDebugMacro(<<"Created kinetic energy scalar");
}

//----------------------------------------------------------------------------
void vtkPLOT3DReader::ComputeVelocityMagnitude(vtkPointData *outputPD)
{
  double *m, u, v, w, v2, d, rr;
  vtkIdType i;
  vtkDataArray *velocityMag;

  //  Check that the required data is available
  //
  if ( this->Density == NULL || this->Momentum == NULL )
    {
    vtkErrorMacro(<<"Cannot compute velocity magnitude");
    return;
    }

  vtkIdType numPts = this->Density->GetNumberOfTuples();
  velocityMag = this->NewFloatArray();
  velocityMag->SetNumberOfTuples(numPts);

  //  Compute the velocity magnitude
  //
  for (i=0; i < numPts; i++)
    {
    d = this->Density->GetComponent(i,0);
    d = (d != 0.0 ? d : 1.0);
    m = this->Momentum->GetTuple(i);
    rr = 1.0 / d;
    u = m[0] * rr;
    v = m[1] * rr;
    w = m[2] * rr;
    v2 = u*u + v*v + w*w;
    velocityMag->SetTuple1(i, sqrt(v2));
  }
  velocityMag->SetName("VelocityMagnitude");
  outputPD->AddArray(velocityMag);
  outputPD->SetActiveScalars("VelocityMagnitude");
  velocityMag->Delete();
  vtkDebugMacro(<<"Created velocity magnitude scalar");
}

//----------------------------------------------------------------------------
void vtkPLOT3DReader::ComputeEntropy(vtkPointData *outputPD)
{
  double *m, u, v, w, v2, d, rr, s, p, e;
  vtkIdType i;
  vtkDataArray *entropy;

  //  Check that the required data is available
  //
  if ( this->Density == NULL || this->Momentum == NULL ||
       this->Energy == NULL )
    {
    vtkErrorMacro(<<"Cannot compute entropy");
    return;
    }

  vtkIdType numPts = this->Density->GetNumberOfTuples();
  entropy = this->NewFloatArray();
  entropy->SetNumberOfTuples(numPts);

  //  Compute the entropy
  //
  for (i=0; i < numPts; i++)
    {
    d = this->Density->GetComponent(i,0);
    d = (d != 0.0 ? d : 1.0);
    m = this->Momentum->GetTuple(i);
    e = this->Energy->GetComponent(i,0);
    rr = 1.0 / d;
    u = m[0] * rr;
    v = m[1] * rr;
    w = m[2] * rr;
    v2 = u*u + v*v + w*w;
    p = (this->Gamma-1.)*(e - 0.5*d*v2);
    s = VTK_CV * log((p/VTK_PINF)/pow(d/VTK_RHOINF,this->Gamma));
    entropy->SetTuple1(i,s);
  }
  entropy->SetName("Entropy");
  outputPD->AddArray(entropy);
  outputPD->SetActiveScalars("Entropy");
  entropy->Delete();
  vtkDebugMacro(<<"Created entropy scalar");
}

//----------------------------------------------------------------------------
void vtkPLOT3DReader::ComputeSwirl(vtkPointData *outputPD)
{
  vtkDataArray *vorticity;
  double d, rr, *m, u, v, w, v2, *vort, s;
  vtkIdType i;
  vtkDataArray *swirl;

  //  Check that the required data is available
  //
  if ( this->Density == NULL || this->Momentum == NULL )
    {
    vtkErrorMacro(<<"Cannot compute swirl");
    return;
    }

  vtkIdType numPts = this->Density->GetNumberOfTuples();
  swirl = this->NewFloatArray();
  swirl->SetNumberOfTuples(numPts);

  this->ComputeVorticity(outputPD);
  vorticity = outputPD->GetArray("Vorticity");
//
//  Compute the swirl
//
  for (i=0; i < numPts; i++)
    {
    d = this->Density->GetComponent(i,0);
    d = (d != 0.0 ? d : 1.0);
    m = this->Momentum->GetTuple(i);
    vort = vorticity->GetTuple(i);
    rr = 1.0 / d;
    u = m[0] * rr;
    v = m[1] * rr;
    w = m[2] * rr;
    v2 = u*u + v*v + w*w;
    if ( v2 != 0.0 )
      {
      s = (vort[0]*m[0] + vort[1]*m[1] + vort[2]*m[2]) / v2;
      }
    else
      {
      s = 0.0;
      }

    swirl->SetTuple1(i,s);
  }
  swirl->SetName("Swirl");
  outputPD->AddArray(swirl);
  outputPD->SetActiveScalars("Swirl");
  swirl->Delete();
  vtkDebugMacro(<<"Created swirl scalar");

}

//----------------------------------------------------------------------------
// Vector functions
void vtkPLOT3DReader::ComputeVelocity(vtkPointData *outputPD)
{
  double *m, v[3], d, rr;
  vtkIdType i;
  vtkDataArray *velocity;

  //  Check that the required data is available
  //
  if ( this->Density == NULL || this->Momentum == NULL )
    {
    vtkErrorMacro(<<"Cannot compute velocity");
    return;
    }

  vtkIdType numPts = this->Density->GetNumberOfTuples();
  velocity = this->NewFloatArray();
  velocity->SetNumberOfComponents(3);
  velocity->SetNumberOfTuples(numPts);

  //  Compute the velocity
  //
  for (i=0; i < numPts; i++)
    {
    d = this->Density->GetComponent(i,0);
    d = (d != 0.0 ? d : 1.0);
    m = this->Momentum->GetTuple(i);
    rr = 1.0 / d;
    v[0] = m[0] * rr;
    v[1] = m[1] * rr;
    v[2] = m[2] * rr;
    velocity->SetTuple(i, v);
  }
  velocity->SetName("Velocity");
  outputPD->AddArray(velocity);
  velocity->Delete();
  vtkDebugMacro(<<"Created velocity vector");
}

//----------------------------------------------------------------------------
void vtkPLOT3DReader::ComputeVorticity(vtkPointData *outputPD)
{
  vtkDataArray *velocity;
  vtkDataArray *vorticity;
  int dims[3], ijsize;
  vtkPoints *points;
  int i, j, k, idx, idx2, ii;
  double vort[3], xp[3], xm[3], vp[3], vm[3], factor;
  double xxi, yxi, zxi, uxi, vxi, wxi;
  double xeta, yeta, zeta, ueta, veta, weta;
  double xzeta, yzeta, zzeta, uzeta, vzeta, wzeta;
  double aj, xix, xiy, xiz, etax, etay, etaz, zetax, zetay, zetaz;

  //  Check that the required data is available
  //
  if ( (points=this->GetOutput()->GetPoints()) == NULL ||
       this->Density == NULL || this->Momentum == NULL ||
       this->Energy == NULL )
    {
    vtkErrorMacro(<<"Cannot compute vorticity");
    return;
    }

  vtkIdType numPts = this->Density->GetNumberOfTuples();
  vorticity = this->NewFloatArray();
  vorticity->SetNumberOfComponents(3);
  vorticity->SetNumberOfTuples(numPts);

  this->ComputeVelocity(outputPD);
  velocity = outputPD->GetArray("Velocity");

  this->GetOutput()->GetDimensions(dims);
  ijsize = dims[0]*dims[1];

  for (k=0; k<dims[2]; k++)
    {
    for (j=0; j<dims[1]; j++)
      {
      for (i=0; i<dims[0]; i++)
        {
        //  Xi derivatives.
        if ( dims[0] == 1 ) // 2D in this direction
          {
          factor = 1.0;
          for (ii=0; ii<3; ii++)
            {
            vp[ii] = vm[ii] = xp[ii] = xm[ii] = 0.0;
            }
          xp[0] = 1.0;
          }
        else if ( i == 0 )
          {
          factor = 1.0;
          idx = (i+1) + j*dims[0] + k*ijsize;
          idx2 = i + j*dims[0] + k*ijsize;
          points->GetPoint(idx,xp);
          points->GetPoint(idx2,xm);
          velocity->GetTuple(idx,vp);
          velocity->GetTuple(idx2,vm);
          }
        else if ( i == (dims[0]-1) )
          {
          factor = 1.0;
          idx = i + j*dims[0] + k*ijsize;
          idx2 = i-1 + j*dims[0] + k*ijsize;
          points->GetPoint(idx,xp);
          points->GetPoint(idx2,xm);
          velocity->GetTuple(idx,vp);
          velocity->GetTuple(idx2,vm);
          }
        else
          {
          factor = 0.5;
          idx = (i+1) + j*dims[0] + k*ijsize;
          idx2 = (i-1) + j*dims[0] + k*ijsize;
          points->GetPoint(idx,xp);
          points->GetPoint(idx2,xm);
          velocity->GetTuple(idx,vp);
          velocity->GetTuple(idx2,vm);
          }

        xxi = factor * (xp[0] - xm[0]);
        yxi = factor * (xp[1] - xm[1]);
        zxi = factor * (xp[2] - xm[2]);
        uxi = factor * (vp[0] - vm[0]);
        vxi = factor * (vp[1] - vm[1]);
        wxi = factor * (vp[2] - vm[2]);

        //  Eta derivatives.
        if ( dims[1] == 1 ) // 2D in this direction
          {
          factor = 1.0;
          for (ii=0; ii<3; ii++)
            {
            vp[ii] = vm[ii] = xp[ii] = xm[ii] = 0.0;
            }
          xp[1] = 1.0;
          }
        else if ( j == 0 )
          {
          factor = 1.0;
          idx = i + (j+1)*dims[0] + k*ijsize;
          idx2 = i + j*dims[0] + k*ijsize;
          points->GetPoint(idx,xp);
          points->GetPoint(idx2,xm);
          velocity->GetTuple(idx,vp);
          velocity->GetTuple(idx2,vm);
          }
        else if ( j == (dims[1]-1) )
          {
          factor = 1.0;
          idx = i + j*dims[0] + k*ijsize;
          idx2 = i + (j-1)*dims[0] + k*ijsize;
          points->GetPoint(idx,xp);
          points->GetPoint(idx2,xm);
          velocity->GetTuple(idx,vp);
          velocity->GetTuple(idx2,vm);
          }
        else
          {
          factor = 0.5;
          idx = i + (j+1)*dims[0] + k*ijsize;
          idx2 = i + (j-1)*dims[0] + k*ijsize;
          points->GetPoint(idx,xp);
          points->GetPoint(idx2,xm);
          velocity->GetTuple(idx,vp);
          velocity->GetTuple(idx2,vm);
          }


        xeta = factor * (xp[0] - xm[0]);
        yeta = factor * (xp[1] - xm[1]);
        zeta = factor * (xp[2] - xm[2]);
        ueta = factor * (vp[0] - vm[0]);
        veta = factor * (vp[1] - vm[1]);
        weta = factor * (vp[2] - vm[2]);

        //  Zeta derivatives.
        if ( dims[2] == 1 ) // 2D in this direction
          {
          factor = 1.0;
          for (ii=0; ii<3; ii++)
            {
            vp[ii] = vm[ii] = xp[ii] = xm[ii] = 0.0;
            }
          xp[2] = 1.0;
          }
        else if ( k == 0 )
          {
          factor = 1.0;
          idx = i + j*dims[0] + (k+1)*ijsize;
          idx2 = i + j*dims[0] + k*ijsize;
          points->GetPoint(idx,xp);
          points->GetPoint(idx2,xm);
          velocity->GetTuple(idx,vp);
          velocity->GetTuple(idx2,vm);
          }
        else if ( k == (dims[2]-1) )
          {
          factor = 1.0;
          idx = i + j*dims[0] + k*ijsize;
          idx2 = i + j*dims[0] + (k-1)*ijsize;
          points->GetPoint(idx,xp);
          points->GetPoint(idx2,xm);
          velocity->GetTuple(idx,vp);
          velocity->GetTuple(idx2,vm);
          }
        else
          {
          factor = 0.5;
          idx = i + j*dims[0] + (k+1)*ijsize;
          idx2 = i + j*dims[0] + (k-1)*ijsize;
          points->GetPoint(idx,xp);
          points->GetPoint(idx2,xm);
          velocity->GetTuple(idx,vp);
          velocity->GetTuple(idx2,vm);
          }

        xzeta = factor * (xp[0] - xm[0]);
        yzeta = factor * (xp[1] - xm[1]);
        zzeta = factor * (xp[2] - xm[2]);
        uzeta = factor * (vp[0] - vm[0]);
        vzeta = factor * (vp[1] - vm[1]);
        wzeta = factor * (vp[2] - vm[2]);

        // Now calculate the Jacobian.  Grids occasionally have
        // singularities, or points where the Jacobian is infinite (the
        // inverse is zero).  For these cases, we'll set the Jacobian to
        // zero, which will result in a zero vorticity.
        //
        aj =  xxi*yeta*zzeta+yxi*zeta*xzeta+zxi*xeta*yzeta
              -zxi*yeta*xzeta-yxi*xeta*zzeta-xxi*zeta*yzeta;
        if (aj != 0.0)
          {
          aj = 1. / aj;
          }

        //  Xi metrics.
        xix  =  aj*(yeta*zzeta-zeta*yzeta);
        xiy  = -aj*(xeta*zzeta-zeta*xzeta);
        xiz  =  aj*(xeta*yzeta-yeta*xzeta);

        //  Eta metrics.
        etax = -aj*(yxi*zzeta-zxi*yzeta);
        etay =  aj*(xxi*zzeta-zxi*xzeta);
        etaz = -aj*(xxi*yzeta-yxi*xzeta);

        //  Zeta metrics.
        zetax=  aj*(yxi*zeta-zxi*yeta);
        zetay= -aj*(xxi*zeta-zxi*xeta);
        zetaz=  aj*(xxi*yeta-yxi*xeta);

        //  Finally, the vorticity components.
        //
        vort[0]= xiy*wxi+etay*weta+zetay*wzeta - xiz*vxi-etaz*veta-zetaz*vzeta;
        vort[1]= xiz*uxi+etaz*ueta+zetaz*uzeta - xix*wxi-etax*weta-zetax*wzeta;
        vort[2]= xix*vxi+etax*veta+zetax*vzeta - xiy*uxi-etay*ueta-zetay*uzeta;
        idx = i + j*dims[0] + k*ijsize;
        vorticity->SetTuple(idx,vort);
        }
      }
    }
  vorticity->SetName("Vorticity");
  outputPD->AddArray(vorticity);
  outputPD->SetActiveVectors("Vorticity");
  vorticity->Delete();
  vtkDebugMacro(<<"Created vorticity vector");
}

void vtkPLOT3DReader::ComputeMomentum(vtkPointData *outputPD)
{
  this->Momentum->SetName("Momentum");
  outputPD->AddArray(this->Momentum);
  outputPD->SetActiveVectors("Momentum");
  vtkDebugMacro(<<"Created momentum vector");
}


//----------------------------------------------------------------------------
void vtkPLOT3DReader::ComputePressureGradient(vtkPointData *outputPD)
{
  vtkDataArray *pressure;
  vtkDataArray *gradient;
  int dims[3], ijsize;
  vtkPoints *points;
  int i, j, k, idx, idx2, ii;
  double g[3], xp[3], xm[3], pp, pm, factor;
  double xxi, yxi, zxi, pxi;
  double xeta, yeta, zeta, peta;
  double xzeta, yzeta, zzeta, pzeta;
  double aj, xix, xiy, xiz, etax, etay, etaz, zetax, zetay, zetaz;

  //  Check that the required data is available
  //
  if ( (points=this->GetOutput()->GetPoints()) == NULL ||
       this->Density == NULL || this->Momentum == NULL ||
       this->Energy == NULL )
    {
    vtkErrorMacro(<<"Cannot compute pressure gradient");
    return;
    }

  vtkIdType numPts = this->Density->GetNumberOfTuples();
  gradient = this->NewFloatArray();
  gradient->SetNumberOfComponents(3);
  gradient->SetNumberOfTuples(numPts);

  this->ComputePressure(outputPD);
  pressure = outputPD->GetArray("Pressure");

  this->GetOutput()->GetDimensions(dims);
  ijsize = dims[0]*dims[1];

  for (k=0; k<dims[2]; k++)
    {
    for (j=0; j<dims[1]; j++)
      {
      for (i=0; i<dims[0]; i++)
        {
        //  Xi derivatives.
        if ( dims[0] == 1 ) // 2D in this direction
          {
          factor = 1.0;
          for (ii=0; ii<3; ii++)
            {
            xp[ii] = xm[ii] = 0.0;
            }
          xp[0] = 1.0; pp = pm = 0.0;
          }
        else if ( i == 0 )
          {
          factor = 1.0;
          idx = (i+1) + j*dims[0] + k*ijsize;
          idx2 = i + j*dims[0] + k*ijsize;
          points->GetPoint(idx,xp);
          points->GetPoint(idx2,xm);
          pp = pressure->GetComponent(idx,0);
          pm = pressure->GetComponent(idx2,0);
          }
        else if ( i == (dims[0]-1) )
          {
          factor = 1.0;
          idx = i + j*dims[0] + k*ijsize;
          idx2 = i-1 + j*dims[0] + k*ijsize;
          points->GetPoint(idx,xp);
          points->GetPoint(idx2,xm);
          pp = pressure->GetComponent(idx,0);
          pm = pressure->GetComponent(idx2,0);
          }
        else
          {
          factor = 0.5;
          idx = (i+1) + j*dims[0] + k*ijsize;
          idx2 = (i-1) + j*dims[0] + k*ijsize;
          points->GetPoint(idx,xp);
          points->GetPoint(idx2,xm);
          pp = pressure->GetComponent(idx,0);
          pm = pressure->GetComponent(idx2,0);
          }

        xxi = factor * (xp[0] - xm[0]);
        yxi = factor * (xp[1] - xm[1]);
        zxi = factor * (xp[2] - xm[2]);
        pxi = factor * (pp - pm);

        //  Eta derivatives.
        if ( dims[1] == 1 ) // 2D in this direction
          {
          factor = 1.0;
          for (ii=0; ii<3; ii++)
            {
            xp[ii] = xm[ii] = 0.0;
            }
          xp[1] = 1.0; pp = pm = 0.0;
          }
        else if ( j == 0 )
          {
          factor = 1.0;
          idx = i + (j+1)*dims[0] + k*ijsize;
          idx2 = i + j*dims[0] + k*ijsize;
          points->GetPoint(idx,xp);
          points->GetPoint(idx2,xm);
          pp = pressure->GetComponent(idx,0);
          pm = pressure->GetComponent(idx2,0);
          }
        else if ( j == (dims[1]-1) )
          {
          factor = 1.0;
          idx = i + j*dims[0] + k*ijsize;
          idx2 = i + (j-1)*dims[0] + k*ijsize;
          points->GetPoint(idx,xp);
          points->GetPoint(idx2,xm);
          pp = pressure->GetComponent(idx,0);
          pm = pressure->GetComponent(idx2,0);
          }
        else
          {
          factor = 0.5;
          idx = i + (j+1)*dims[0] + k*ijsize;
          idx2 = i + (j-1)*dims[0] + k*ijsize;
          points->GetPoint(idx,xp);
          points->GetPoint(idx2,xm);
          pp = pressure->GetComponent(idx,0);
          pm = pressure->GetComponent(idx2,0);
          }

        xeta = factor * (xp[0] - xm[0]);
        yeta = factor * (xp[1] - xm[1]);
        zeta = factor * (xp[2] - xm[2]);
        peta = factor * (pp - pm);

        //  Zeta derivatives.
        if ( dims[2] == 1 ) // 2D in this direction
          {
          factor = 1.0;
          for (ii=0; ii<3; ii++)
            {
            xp[ii] = xm[ii] = 0.0;
            }
          xp[2] = 1.0; pp = pm = 0.0;
          }
        else if ( k == 0 )
          {
          factor = 1.0;
          idx = i + j*dims[0] + (k+1)*ijsize;
          idx2 = i + j*dims[0] + k*ijsize;
          points->GetPoint(idx,xp);
          points->GetPoint(idx2,xm);
          pp = pressure->GetComponent(idx,0);
          pm = pressure->GetComponent(idx2,0);
          }
        else if ( k == (dims[2]-1) )
          {
          factor = 1.0;
          idx = i + j*dims[0] + k*ijsize;
          idx2 = i + j*dims[0] + (k-1)*ijsize;
          points->GetPoint(idx,xp);
          points->GetPoint(idx2,xm);
          pp = pressure->GetComponent(idx,0);
          pm = pressure->GetComponent(idx2,0);
          }
        else
          {
          factor = 0.5;
          idx = i + j*dims[0] + (k+1)*ijsize;
          idx2 = i + j*dims[0] + (k-1)*ijsize;
          points->GetPoint(idx,xp);
          points->GetPoint(idx2,xm);
          pp = pressure->GetComponent(idx,0);
          pm = pressure->GetComponent(idx2,0);
          }

        xzeta = factor * (xp[0] - xm[0]);
        yzeta = factor * (xp[1] - xm[1]);
        zzeta = factor * (xp[2] - xm[2]);
        pzeta = factor * (pp - pm);

        //  Now calculate the Jacobian.  Grids occasionally have
        //  singularities, or points where the Jacobian is infinite (the
        //  inverse is zero).  For these cases, we'll set the Jacobian to
        //  zero, which will result in a zero vorticity.
        //
        aj =  xxi*yeta*zzeta+yxi*zeta*xzeta+zxi*xeta*yzeta
              -zxi*yeta*xzeta-yxi*xeta*zzeta-xxi*zeta*yzeta;
        if (aj != 0.0)
          {
          aj = 1. / aj;
          }

        //  Xi metrics.
        xix  =  aj*(yeta*zzeta-zeta*yzeta);
        xiy  = -aj*(xeta*zzeta-zeta*xzeta);
        xiz  =  aj*(xeta*yzeta-yeta*xzeta);

        //  Eta metrics.
        etax = -aj*(yxi*zzeta-zxi*yzeta);
        etay =  aj*(xxi*zzeta-zxi*xzeta);
        etaz = -aj*(xxi*yzeta-yxi*xzeta);

        //  Zeta metrics.
        zetax=  aj*(yxi*zeta-zxi*yeta);
        zetay= -aj*(xxi*zeta-zxi*xeta);
        zetaz=  aj*(xxi*yeta-yxi*xeta);

        //  Finally, the vorticity components.
        g[0]= xix*pxi+etax*peta+zetax*pzeta;
        g[1]= xiy*pxi+etay*peta+zetay*pzeta;
        g[2]= xiz*pxi+etaz*peta+zetaz*pzeta;

        idx = i + j*dims[0] + k*ijsize;
        gradient->SetTuple(idx,g);
        }
      }
    }
  gradient->SetName("PressureGradient");
  outputPD->AddArray(gradient);
  outputPD->SetActiveVectors("PressureGradient");
  gradient->Delete();
  vtkDebugMacro(<<"Created pressure gradient vector");
}

void vtkPLOT3DReader::ComputeVorticityMagnitude(vtkPointData* outputPD)
{
  // It's already computed
  if (outputPD->GetArray("VorticityMagnitude"))
    {
    return;
    }
  this->ComputeVorticity(outputPD);
  vtkDataArray* vorticity = outputPD->GetArray("Vorticity");
  vtkDataArray* vm = this->NewFloatArray();
  vtkIdType numPts = vorticity->GetNumberOfTuples();
  vm->SetNumberOfTuples(numPts);
  for (vtkIdType idx=0; idx<numPts; idx++)
    {
    double* vort = vorticity->GetTuple(idx);
    double magnitude = sqrt(vort[0]*vort[0]+
                            vort[1]*vort[1]+vort[2]*vort[2]);
    vm->SetTuple1(idx, magnitude);
    }
  vm->SetName("VorticityMagnitude");
  outputPD->AddArray(vm);
  vm->Delete();
}

void vtkPLOT3DReader::ComputeStrainRate(vtkPointData* outputPD)
{
  vtkDataArray *velocity;
  int dims[3], ijsize;
  int i, j, k, idx, idx2, ii;
  double stRate[3], xp[3], xm[3], vp[3], vm[3], factor;
  double xxi, yxi, zxi, uxi, vxi, wxi;
  double xeta, yeta, zeta, ueta, veta, weta;
  double xzeta, yzeta, zzeta, uzeta, vzeta, wzeta;
  double aj, xix, xiy, xiz, etax, etay, etaz, zetax, zetay, zetaz;

  //  Check that the required data is available
  //
  if (outputPD->GetArray("StrainRate"))
    {
    return;
    }
  if ( this->Density == NULL || this->Momentum == NULL )
    {
    vtkErrorMacro("Cannot compute strain rate.");
    return;
    }

  vtkIdType numPts = this->Density->GetNumberOfTuples();
  vtkDataArray* strainRate = this->NewFloatArray();
  strainRate->SetNumberOfComponents(3);
  strainRate->SetNumberOfTuples(numPts);
  strainRate->SetName("StrainRate");

  this->ComputeVelocity(outputPD);
  velocity = outputPD->GetArray("Velocity");
  if(!velocity)
    {
    vtkErrorMacro("Could not compute strain rate.");
    return;
    }

  vtkStructuredGrid *output = this->GetOutput();
  output->GetDimensions(dims);
  ijsize = dims[0]*dims[1];

  for (k=0; k<dims[2]; k++)
    {
    for (j=0; j<dims[1]; j++)
      {
      for (i=0; i<dims[0]; i++)
        {
        //  Xi derivatives.
        if ( dims[0] == 1 ) // 2D in this direction
          {
          factor = 1.0;
          for (ii=0; ii<3; ii++)
            {
            vp[ii] = vm[ii] = xp[ii] = xm[ii] = 0.0;
            }
          xp[0] = 1.0;
          }
        else if ( i == 0 )
          {
          factor = 1.0;
          idx = (i+1) + j*dims[0] + k*ijsize;
          idx2 = i + j*dims[0] + k*ijsize;
          output->GetPoint(idx,xp);
          output->GetPoint(idx2,xm);
          velocity->GetTuple(idx,vp);
          velocity->GetTuple(idx2,vm);
          }
        else if ( i == (dims[0]-1) )
          {
          factor = 1.0;
          idx = i + j*dims[0] + k*ijsize;
          idx2 = i-1 + j*dims[0] + k*ijsize;
          output->GetPoint(idx,xp);
          output->GetPoint(idx2,xm);
          velocity->GetTuple(idx,vp);
          velocity->GetTuple(idx2,vm);
          }
        else
          {
          factor = 0.5;
          idx = (i+1) + j*dims[0] + k*ijsize;
          idx2 = (i-1) + j*dims[0] + k*ijsize;
          output->GetPoint(idx,xp);
          output->GetPoint(idx2,xm);
          velocity->GetTuple(idx,vp);
          velocity->GetTuple(idx2,vm);
          }

        xxi = factor * (xp[0] - xm[0]);
        yxi = factor * (xp[1] - xm[1]);
        zxi = factor * (xp[2] - xm[2]);
        uxi = factor * (vp[0] - vm[0]);
        vxi = factor * (vp[1] - vm[1]);
        wxi = factor * (vp[2] - vm[2]);

        //  Eta derivatives.
        if ( dims[1] == 1 ) // 2D in this direction
          {
          factor = 1.0;
          for (ii=0; ii<3; ii++)
            {
            vp[ii] = vm[ii] = xp[ii] = xm[ii] = 0.0;
            }
          xp[1] = 1.0;
          }
        else if ( j == 0 )
          {
          factor = 1.0;
          idx = i + (j+1)*dims[0] + k*ijsize;
          idx2 = i + j*dims[0] + k*ijsize;
          output->GetPoint(idx,xp);
          output->GetPoint(idx2,xm);
          velocity->GetTuple(idx,vp);
          velocity->GetTuple(idx2,vm);
          }
        else if ( j == (dims[1]-1) )
          {
          factor = 1.0;
          idx = i + j*dims[0] + k*ijsize;
          idx2 = i + (j-1)*dims[0] + k*ijsize;
          output->GetPoint(idx,xp);
          output->GetPoint(idx2,xm);
          velocity->GetTuple(idx,vp);
          velocity->GetTuple(idx2,vm);
          }
        else
          {
          factor = 0.5;
          idx = i + (j+1)*dims[0] + k*ijsize;
          idx2 = i + (j-1)*dims[0] + k*ijsize;
          output->GetPoint(idx,xp);
          output->GetPoint(idx2,xm);
          velocity->GetTuple(idx,vp);
          velocity->GetTuple(idx2,vm);
          }


        xeta = factor * (xp[0] - xm[0]);
        yeta = factor * (xp[1] - xm[1]);
        zeta = factor * (xp[2] - xm[2]);
        ueta = factor * (vp[0] - vm[0]);
        veta = factor * (vp[1] - vm[1]);
        weta = factor * (vp[2] - vm[2]);

        //  Zeta derivatives.
        if ( dims[2] == 1 ) // 2D in this direction
          {
          factor = 1.0;
          for (ii=0; ii<3; ii++)
            {
            vp[ii] = vm[ii] = xp[ii] = xm[ii] = 0.0;
            }
          xp[2] = 1.0;
          }
        else if ( k == 0 )
          {
          factor = 1.0;
          idx = i + j*dims[0] + (k+1)*ijsize;
          idx2 = i + j*dims[0] + k*ijsize;
          output->GetPoint(idx,xp);
          output->GetPoint(idx2,xm);
          velocity->GetTuple(idx,vp);
          velocity->GetTuple(idx2,vm);
          }
        else if ( k == (dims[2]-1) )
          {
          factor = 1.0;
          idx = i + j*dims[0] + k*ijsize;
          idx2 = i + j*dims[0] + (k-1)*ijsize;
          output->GetPoint(idx,xp);
          output->GetPoint(idx2,xm);
          velocity->GetTuple(idx,vp);
          velocity->GetTuple(idx2,vm);
          }
        else
          {
          factor = 0.5;
          idx = i + j*dims[0] + (k+1)*ijsize;
          idx2 = i + j*dims[0] + (k-1)*ijsize;
          output->GetPoint(idx,xp);
          output->GetPoint(idx2,xm);
          velocity->GetTuple(idx,vp);
          velocity->GetTuple(idx2,vm);
          }

        xzeta = factor * (xp[0] - xm[0]);
        yzeta = factor * (xp[1] - xm[1]);
        zzeta = factor * (xp[2] - xm[2]);
        uzeta = factor * (vp[0] - vm[0]);
        vzeta = factor * (vp[1] - vm[1]);
        wzeta = factor * (vp[2] - vm[2]);

        // Now calculate the Jacobian.  Grids occasionally have
        // singularities, or points where the Jacobian is infinite (the
        // inverse is zero).  For these cases, we'll set the Jacobian to
        // zero, which will result in a zero vorticity.
        //
        aj =  xxi*yeta*zzeta+yxi*zeta*xzeta+zxi*xeta*yzeta
              -zxi*yeta*xzeta-yxi*xeta*zzeta-xxi*zeta*yzeta;
        if (aj != 0.0)
          {
          aj = 1. / aj;
          }

        //  Xi metrics.
        xix  =  aj*(yeta*zzeta-zeta*yzeta);
        xiy  = -aj*(xeta*zzeta-zeta*xzeta);
        xiz  =  aj*(xeta*yzeta-yeta*xzeta);

        //  Eta metrics.
        etax = -aj*(yxi*zzeta-zxi*yzeta);
        etay =  aj*(xxi*zzeta-zxi*xzeta);
        etaz = -aj*(xxi*yzeta-yxi*xzeta);

        //  Zeta metrics.
        zetax=  aj*(yxi*zeta-zxi*yeta);
        zetay= -aj*(xxi*zeta-zxi*xeta);
        zetaz=  aj*(xxi*yeta-yxi*xeta);

        //  Finally, the strain rate components.
        //
        stRate[0] = xix*uxi+etax*ueta+zetax*uzeta;
        stRate[1] = xiy*vxi+etay*veta+zetay*vzeta;
        stRate[2] = xiz*wxi+etaz*weta+zetaz*wzeta;
        idx = i + j*dims[0] + k*ijsize;
        strainRate->SetTuple(idx,stRate);
        }
      }
    }
  outputPD->AddArray(strainRate);
  strainRate->Delete();
}


//----------------------------------------------------------------------------
void vtkPLOT3DReader::SetByteOrderToBigEndian()
{
  this->ByteOrder = FILE_BIG_ENDIAN;
}

//----------------------------------------------------------------------------
void vtkPLOT3DReader::SetByteOrderToLittleEndian()
{
  this->ByteOrder = FILE_LITTLE_ENDIAN;
}

//----------------------------------------------------------------------------
const char *vtkPLOT3DReader::GetByteOrderAsString()
{
  if ( this->ByteOrder ==  FILE_LITTLE_ENDIAN)
    {
    return "LittleEndian";
    }
  else
    {
    return "BigEndian";
    }
}

//----------------------------------------------------------------------------
void vtkPLOT3DReader::AddFunction(int functionNumber)
{
  this->FunctionList->InsertNextValue(functionNumber);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkPLOT3DReader::RemoveAllFunctions()
{
  this->FunctionList->Reset();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkPLOT3DReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "XYZ File Name: " << 
    (this->XYZFileName ? this->XYZFileName : "(none)") << "\n";
  os << indent << "Q File Name: "<<
    (this->QFileName ? this->QFileName : "(none)") << "\n";
  os << indent << "Function File Name: "<<
    (this->FunctionFileName ? this->FunctionFileName : "(none)") << "\n";
  os << indent << "BinaryFile: " << this->BinaryFile << endl;
  os << indent << "HasByteCount: " << this->HasByteCount << endl;
  os << indent << "Gamma: " << this->Gamma << endl;
  os << indent << "R: " << this->R << endl;
  os << indent << "Uvinf: " << this->Uvinf << endl;
  os << indent << "Vvinf: " << this->Vvinf << endl;
  os << indent << "Wvinf: " << this->Wvinf << endl;
  os << indent << "ScalarFunctionNumber: " << this->ScalarFunctionNumber << endl;
  os << indent << "VectorFunctionNumber: " << this->VectorFunctionNumber << endl;
  os << indent << "MultiGrid: " << this->MultiGrid << endl;
  os << indent << "ForceRead: " << this->ForceRead << endl;
  os << indent << "IBlanking: " << this->IBlanking << endl;
  os << indent << "ByteOrder: " << this->ByteOrder << endl;
  os << indent << "TwoDimensionalGeometry: "
      << (this->TwoDimensionalGeometry?"on":"off") << endl;
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

  int dims[3];
  output->GetDimensions(dims);
  vtkDataArray *pointArray = this->NewFloatArray();
  pointArray->SetNumberOfComponents(3);
  pointArray->SetNumberOfTuples(this->NumberOfPoints);

  vtkPoints *newPts = vtkPoints::New();
  newPts->SetData(pointArray);
  pointArray->Delete();
  output->SetPoints(newPts);
  newPts->Delete();

  rewind(xyzFp);
  int offset = this->ComputeGridOffset(xyzFp);
  fseek(xyzFp, (long)(offset), SEEK_SET);

  this->SkipByteCount(xyzFp);
  if (this->ReadVector(xyzFp, this->NumberOfPoints, pointArray) == 0)
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
  if (this->IBlanking)
    {
    }
#endif
  fclose(xyzFp);
  return VTK_OK;
}

int
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

    for (int j = i+1; j <= this->GridNumber; j++)
      {
      int nd = this->TwoDimensionalGeometry ? 2 : 3;
      if (this->BinaryFile)
        {
        int bc = this->HasByteCount ? sizeof(int) : 0;
        if (this->IBlanking)
          {
          this->GridOffsets[j] = (this->GridOffsets[j-1] + (nd+1)*this->GridSizes[j-1]*4) + bc;
          }
        else
          {
          if (this->DoublePrecision)
            {
            this->GridOffsets[j] = (this->GridOffsets[j-1] + nd*this->GridSizes[j-1]*8) + 2*bc;
            }
          else
            {
            this->GridOffsets[j] = (this->GridOffsets[j-1] + nd*this->GridSizes[j-1]*4) + 2*bc;
            }
          }
        }
      else
        {
        int numberOfElements;
        if (this->IBlanking)
          {
          numberOfElements = (nd+1)*GridSizes[j-1];
          }
        else
          {
          numberOfElements = (nd)*GridSizes[j-1];
          }
        fseek(xyzFp,(long)(this->GridOffsets[j-1]),SEEK_SET);
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
  int offset = this->ComputeSolutionOffset(qFp);
  fseek (qFp, (long)(offset), SEEK_SET);

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

  this->Properties = newProp;
  this->Properties->Register(this);
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
  if ( this->ReadVector(qFp, numPts, newMomentum) == 0)
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

int
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
      // Number of scalars to  be read: 1 for density, 1 for energy and NUmDims for Momentum
      int ns = 1  + 1  + (this->TwoDimensionalGeometry ? 2 : 3);
      if (this->BinaryFile)
        {
        int bc = this->HasByteCount ? sizeof(int) : 0;

        if (this->DoublePrecision)
          {
          this->SolutionOffsets[j] = this->SolutionOffsets[0] +
                               4 * sizeof(double) + 2*bc +
                               ns * this->GridSizes[j-1]*sizeof(double) + 2*bc;
          }
        else
          {
          this->SolutionOffsets[j] = this->SolutionOffsets[0] +
                               4 * sizeof(float) + 2*bc +
                               ns * this->GridSizes[j-1]*sizeof(float) + 2*bc;
          }
        }
      else
        {
        int numberOfElements = 4 + ns*GridSizes[j-1];
        fseek(qFp,(long)this->SolutionOffsets[j-1],SEEK_SET);
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


vtkDataArray *
vtkPLOT3DReader::NewFloatArray()
{
  if (this->DoublePrecision)
    {
    return vtkDoubleArray::New();
    }
  else
    {
    return vtkFloatArray::New();
    }
}

