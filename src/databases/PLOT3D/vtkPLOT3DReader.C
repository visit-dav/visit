/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// 
// This was adapted vtkMultiBlockPlot3DReader, VTK Version 6.1,
// by Kitware http://www.vtk.org
//
// Subclassing vtkMultiBlockPlot3DReader no longer feasible due to the
// large number of methods that need to be overridden to work within
// VisIt's pipeline.
//
// A lot of modifications were made to remove vtk pipeline infrastructure,
// reduce memory footprint and to serve up one block at a time.
//
// Function calculations have been templated to work on native arrays,
// and moved into their own class (PLOT3DFunctions).
//

#include "vtkPLOT3DReader.h"

#include <vtkByteSwap.h>
#include <vtkDoubleArray.h>
#include <vtkIdList.h>
#include <vtkErrorCode.h>
#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkStructuredGrid.h>

#include "vtkPLOT3DReaderInternals.h"
#include "PLOT3DFunctions.h"

#include <float.h> // for DBL_MAX
#include <string>
using std::string;


vtkStandardNewMacro(vtkPLOT3DReader);


template <class DataType>
class vtkPLOT3DArrayReader
{
public:
  vtkPLOT3DArrayReader() : ByteOrder(
    vtkPLOT3DReader::FILE_BIG_ENDIAN)
    {
    }

  int ReadScalar(FILE* fp, int n, DataType* scalar)
    {
      int retVal = static_cast<int>(fread(scalar, sizeof(DataType), n, fp));
      if (this->ByteOrder == vtkPLOT3DReader::FILE_LITTLE_ENDIAN)
        {
        if (sizeof(DataType) == 4)
          {
          vtkByteSwap::Swap4LERange(scalar, n);
          }
        else
          {
          vtkByteSwap::Swap8LERange(scalar, n);
          }
        }
      else
        {
        if (sizeof(DataType) == 4)
          {
          vtkByteSwap::Swap4BERange(scalar, n);
          }
        else
          {
          vtkByteSwap::Swap8BERange(scalar, n);
          }
        }
      return retVal;
    }

  int ReadVector(FILE* fp, int n, int numDims, DataType* vector)
    {
      // Setting to 0 in case numDims == 0. We still need to
      // populate an array with 3 components but the code below
      // does not read the 3rd component (it doesn't exist
      // in the file)
      memset(vector, 0, n*3*sizeof(DataType));

      int retVal = 0;
      DataType* buffer = new DataType[n];
      for (int component = 0; component < numDims; component++)
        {
        retVal += this->ReadScalar(fp, n, buffer);
        for (int i=0; i<n; i++)
          {
          vector[3*i+component] = buffer[i];
          }
        }
      delete[] buffer;

      return retVal;
    }

  int ByteOrder;
};

vtkPLOT3DReader::vtkPLOT3DReader()
{
  this->Internal = new vtkPLOT3DReaderInternals;

  this->XYZFileName = NULL;
  this->QFileName = NULL;
  this->BinaryFile = true;
  this->HasByteCount = false;
  this->FileSize = 0;
  this->MultiGrid = false;
  this->ByteOrder = FILE_BIG_ENDIAN;
  this->IBlanking = false;
  this->TwoDimensionalGeometry = false;
  this->DoublePrecision = false;

  this->R = 1.0;
  this->Gamma = 1.4;

  // BEGIN LLNL ADDITIONS
  this->NumberOfGrids = 0;
  this->NumberOfPoints = 0;
  this->GridNumber = 0;

  this->Properties = NULL;

  this->GridSizes = NULL;
  this->GridDimensions = NULL;
  this->GridOffsets = NULL;
  this->SolutionOffsets = NULL;

  // overflow info
  this->IsOverflow = false;
  this->OverflowNQ = 0;
  this->OverflowNQC = 0;
  this->NumProperties = 4;


  output = vtkStructuredGrid::New();
  // END LLNL ADDITIONS
}

vtkPLOT3DReader::~vtkPLOT3DReader()
{
  delete [] this->XYZFileName;
  delete [] this->QFileName;

  delete this->Internal;

  // BEGIN LLNL ADDITIONS
  if (this->GridSizes)
    delete [] this->GridSizes;
  if (this->GridDimensions)
    delete [] this->GridDimensions;
  if (this->GridOffsets)
    delete [] this->GridOffsets;
  if (this->SolutionOffsets)
    delete [] this->SolutionOffsets;

  output->Delete();
  // END LLNL ADDITIONS
}

int vtkPLOT3DReader::AutoDetectionCheck(FILE* fp)
{
  this->Internal->CheckBinaryFile(fp);

  if (!this->Internal->BinaryFile)
    {
    vtkDebugMacro("Auto-detection only works with binary files.");
    if (this->BinaryFile)
      {
      vtkWarningMacro("This appears to be an ASCII file. Please make sure "
                      "that all settings are correct to read it correctly.");
      }
    this->Internal->ByteOrder = this->ByteOrder;
    this->Internal->HasByteCount = this->HasByteCount;
    this->Internal->MultiGrid = this->MultiGrid;
    this->Internal->NumberOfDimensions = this->TwoDimensionalGeometry ? 2 : 3;
    this->Internal->Precision = this->DoublePrecision ? 8 : 4;
    this->Internal->IBlanking = this->IBlanking;
    return 1;
    }

  int success = this->Internal->CheckByteOrder(fp);
  if (!success)
    vtkDebugMacro("Auto detection failed at byte order");

  if (success)
    {
    success =  this->Internal->CheckByteCount(fp);
    if (!success)
      vtkDebugMacro("Auto detection failed at byte count");
    }

  if (success)
    {
    if (!this->Internal->HasByteCount)
      {
      success = this->Internal->CheckCFile(fp, this->FileSize);
      if (!success)
        vtkDebugMacro("Auto detection failed checking C File");
      }
    else
      {
      success = this->Internal->CheckMultiGrid(fp);
      if (!success)
        vtkDebugMacro("Auto detection failed checking Multi Grid");
      if (success)
        {
        success = this->Internal->Check2DGeom(fp);
        if (!success)
          vtkDebugMacro("Auto detection failed checking 2D geometry");
        }
      if (success)
        {
        success = this->Internal->CheckBlankingAndPrecision(fp);
        if (!success)
          vtkDebugMacro("Auto detection failed blanking and precision");
        }
      }
    }

  if (!success)
    {
    success = 1;
    this->Internal->BinaryFile = this->BinaryFile;
    this->Internal->ByteOrder = this->ByteOrder;
    this->Internal->HasByteCount = this->HasByteCount;
    this->Internal->MultiGrid = this->MultiGrid;
    this->Internal->NumberOfDimensions = this->TwoDimensionalGeometry ? 2 : 3;
    this->Internal->Precision = this->DoublePrecision ? 8 : 4;
    this->Internal->IBlanking = this->IBlanking;
    }
  return success;
}

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
    vtkErrorMacro(<< "File: " << fname << " not found.");
    return VTK_ERROR;
    }
  return VTK_OK;
}

int vtkPLOT3DReader::CheckGeometryFile(FILE*& xyzFp)
{
  if ( this->XYZFileName == NULL || this->XYZFileName[0] == '\0'  )
    {
    vtkErrorMacro(<< "Must specify geometry file");
    return VTK_ERROR;
    }
  return this->CheckFile(xyzFp, this->XYZFileName);
}

int vtkPLOT3DReader::CheckSolutionFile(FILE*& qFp)
{
  if ( this->QFileName == NULL || this->QFileName[0] == '\0' )
    {
    vtkErrorMacro(<< "Must specify geometry file");
    return VTK_ERROR;
    }
  return this->CheckFile(qFp, this->QFileName);
}

// Skip Fortran style byte count.
int vtkPLOT3DReader::SkipByteCount(FILE* fp)
{
  if (this->Internal->BinaryFile && this->Internal->HasByteCount)
    {
    int tmp;
    if (fread(&tmp, sizeof(int), 1, fp) != 1)
      {
      vtkErrorMacro ("vtkPLOT3DReader error reading file: " << this->XYZFileName
                     << " Premature EOF while reading skipping byte count.");
      fclose (fp);
      return 0;
      }
    if (this->Internal->ByteOrder == vtkPLOT3DReader::FILE_LITTLE_ENDIAN)
      {
      vtkByteSwap::Swap4LERange(&tmp, 1);
      }
    else
      {
      vtkByteSwap::Swap4BERange(&tmp, 1);
      }

    return tmp;
    }
  return 0;
}

// Read a block of ints (ascii or binary) and return number read.
int vtkPLOT3DReader::ReadIntBlock(FILE* fp, int n, int* block)
{
  if (this->Internal->BinaryFile)
    {
    int retVal=static_cast<int>(fread(block, sizeof(int), n, fp));
    if (this->Internal->ByteOrder == FILE_LITTLE_ENDIAN)
      {
      vtkByteSwap::Swap4LERange(block, n);
      }
    else
      {
      vtkByteSwap::Swap4BERange(block, n);
      }
    return retVal == n;
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
    return count == n;
    }
}

vtkDataArray* vtkPLOT3DReader::NewFloatArray()
{
  if (this->Internal->Precision == 4)
    {
    return vtkFloatArray::New();
    }
  else
    {
    return vtkDoubleArray::New();
    }
}

int vtkPLOT3DReader::ReadScalar(FILE* fp, int n, vtkDataArray* scalar)
{
  if (this->Internal->BinaryFile)
    {
    if (this->Internal->Precision == 4)
      {
      vtkPLOT3DArrayReader<float> arrayReader;
      arrayReader.ByteOrder = this->Internal->ByteOrder;
      vtkFloatArray* floatArray = static_cast<vtkFloatArray*>(scalar);
      return arrayReader.ReadScalar(fp, n, floatArray->GetPointer(0));
      }
    else
      {
      vtkPLOT3DArrayReader<double> arrayReader;
      arrayReader.ByteOrder = this->Internal->ByteOrder;
      vtkDoubleArray* doubleArray = static_cast<vtkDoubleArray*>(scalar);
      return arrayReader.ReadScalar(fp, n, doubleArray->GetPointer(0));
      }
    }
  else
    {
    if (this->Internal->Precision == 4)
      {
      vtkFloatArray* floatArray = static_cast<vtkFloatArray*>(scalar);
      float* values = floatArray->GetPointer(0);

      int count = 0;
      for(int i=0; i<n; i++)
        {
        int num = fscanf(fp, "%f", &(values[i]));
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
    else
      {
      vtkDoubleArray* doubleArray = static_cast<vtkDoubleArray*>(scalar);
      double* values = doubleArray->GetPointer(0);

      int count = 0;
      for(int i=0; i<n; i++)
        {
        int num = fscanf(fp, "%lf", &(values[i]));
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
}

int vtkPLOT3DReader::ReadVector(FILE* fp, int n, int numDims, vtkDataArray* vector)
{
  if (this->Internal->BinaryFile)
    {
    if (this->Internal->Precision == 4)
      {
      vtkPLOT3DArrayReader<float> arrayReader;
      arrayReader.ByteOrder = this->Internal->ByteOrder;
      vtkFloatArray* floatArray = static_cast<vtkFloatArray*>(vector);
      return arrayReader.ReadVector(fp, n, numDims, floatArray->GetPointer(0));
      }
    else
      {
      vtkPLOT3DArrayReader<double> arrayReader;
      arrayReader.ByteOrder = this->Internal->ByteOrder;
      vtkDoubleArray* doubleArray = static_cast<vtkDoubleArray*>(vector);
      return arrayReader.ReadVector(fp, n, numDims, doubleArray->GetPointer(0));
      }
    }
  else
    {
    // Initialize the 3rd component to 0 in case the input file is
    // 2D
    vector->FillComponent(2, 0);

    int count = 0;

    if (this->Internal->Precision == 4)
      {
      vtkFloatArray* floatArray = static_cast<vtkFloatArray*>(vector);

      vtkFloatArray* tmpArray = vtkFloatArray::New();
      tmpArray->Allocate(n);
      for (int component = 0; component < numDims; component++)
        {
        count += this->ReadScalar(fp, n, tmpArray);
        for (int i=0; i<n; i++)
          {
          floatArray->SetValue(3*i+component, tmpArray->GetValue(i));
          }
        }
      tmpArray->Delete();
      }
    else
      {
      vtkDoubleArray* doubleArray = static_cast<vtkDoubleArray*>(vector);

      vtkDoubleArray* tmpArray = vtkDoubleArray::New();
      tmpArray->Allocate(n);
      for (int component = 0; component < numDims; component++)
        {
        count += this->ReadScalar(fp, n, tmpArray);
        for (int i=0; i<n; i++)
          {
          doubleArray->SetValue(3*i+component, tmpArray->GetValue(i));
          }
        }
      tmpArray->Delete();
      }

    return count;
    }
}

// Read a block of floats (ascii or binary) and return number read.
void vtkPLOT3DReader::CalculateFileSize(FILE* fp)
{
  long curPos = ftell(fp);
  fseek(fp, 0, SEEK_END);
  this->FileSize = ftell(fp);
  fseek(fp, curPos, SEEK_SET);
}


// Read the header and return the number of grids.
int vtkPLOT3DReader::GetNumberOfBlocksInternal(FILE* xyzFp)
{
  int numGrid = 0;

  if ( this->Internal->MultiGrid )
    {
    this->SkipByteCount(xyzFp);
    this->ReadIntBlock(xyzFp, 1, &numGrid);
    this->SkipByteCount(xyzFp);
    }
  else
    {
    numGrid=1;
    }

  return numGrid;
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
    int numGrids = this->GetNumberOfBlocksInternal(xyzFp);
    fclose(xyzFp);
    return numGrids;
    }
  return this->NumberOfGrids;
}


int
vtkPLOT3DReader::ReadGeometryHeader(FILE* xyzFp)
{
  int numGrids = this->GetNumberOfBlocksInternal(xyzFp);
  vtkDebugMacro("Geometry number of grids: " << numGrids);
  if ( numGrids == 0 )
    {
    this->NumberOfGrids = 0; 
    return VTK_ERROR;
    }

  if (this->NumberOfGrids == 0)
    this->NumberOfGrids = numGrids;
  else if (numGrids != this->NumberOfGrids)
    return VTK_ERROR;
  if (this->GridOffsets == NULL)
    {
    this->GridOffsets    = new long[this->NumberOfGrids];
    this->GridSizes      = new long[this->NumberOfGrids];
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
  return VTK_OK;
}

int vtkPLOT3DReader::ReadQHeader(FILE* qFp)
{
  int numGrid = this->GetNumberOfBlocksInternal(qFp);
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
    }

  int bytes = this->SkipByteCount(qFp);
  if (bytes > 0 &&
      bytes == (numGrid*this->Internal->NumberOfDimensions+2)*4)
    {
     this->IsOverflow = true;
    }
  else
    {
    this->IsOverflow = false;
    }
 
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
      vtkErrorMacro("Geometry and data dimensions do not match. "
                    "Data file may be corrupt.");
      return VTK_ERROR;
      }
    }

  if (this->IsOverflow)
    {
    this->ReadIntBlock(qFp, 1, &this->OverflowNQ);
    this->ReadIntBlock(qFp, 1, &this->OverflowNQC);
    }
  else 
    {
    this->OverflowNQ = 5;
    this->OverflowNQC = 0;
    }

  // Get to the location of the fsmach
  int endbytes = this->SkipByteCount(qFp);
  if(bytes != endbytes)
  {
      vtkErrorMacro("ReadQHeader " << bytes << " != " << endbytes);
      return VTK_ERROR;
  }

  this->SolutionOffsets[0] = ftell(qFp);
  if (this->IsOverflow)
    {
    int count = this->SkipByteCount(qFp);
    this->NumProperties = (count-4)/this->Internal->Precision + 1;
    fseek(qFp, this->SolutionOffsets[0], SEEK_SET);
    }

  return VTK_OK;
}

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

  this->Internal->NeedToCheckXYZFile = true;
  this->Modified();
}

int vtkPLOT3DReader::RequestInformation()
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

  this->ReadGeometryHeader(xyzFp);

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
int
vtkPLOT3DReader::ReadGrid()
{
  FILE* xyzFp;

  output->Initialize();
  if ( this->CheckGeometryFile(xyzFp) != VTK_OK)
    {
    return 0;
    }

  if ( this->ReadGeometryHeader(xyzFp) != VTK_OK )
    {
    vtkErrorMacro("Error reading geometry file.");
    fclose(xyzFp);
    return 0;
    }

  if (this->ReadGrid(xyzFp) != VTK_OK)
    {
    vtkErrorMacro("Error reading geometry file.");
    fclose(xyzFp);
    return 0;
    }
  fclose(xyzFp);
  return 1;
}


int
vtkPLOT3DReader::ReadGrid(FILE *xyzFp)
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
            this->GridOffsets[j] = (this->GridOffsets[j-1] +        // starting offset
                nd*this->GridSizes[j-1]*this->Internal->Precision + // coordinate sizes
                this->GridSizes[j-1]*4 +                            // iblank sizes
                2*bc);                                              // start/end byte counts
          }
        else
          {
            this->GridOffsets[j] = (this->GridOffsets[j-1] +        // starting offset
                nd*this->GridSizes[j-1]*this->Internal->Precision + // coordinate sizes
                2*bc);                                              // start/end byte counts
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
  int start = this->SkipByteCount(qFp);

  // read parameters
  vtkDataArray *newProp = this->NewFloatArray();
  newProp->SetNumberOfTuples(this->NumProperties);
  newProp->SetName("Properties");
  if (this->ReadScalar(qFp, 4, newProp) == 0)
    {
    vtkErrorMacro("Encountered premature end-of-file while reading "
                  "the q file for properties (or the file is corrupt).");
    newProp->Delete();
    return VTK_ERROR;
    }

  if (this->IsOverflow && this->NumProperties > 4)
    {
    // We create a dummy array to use with ReadScalar
    vtkDataArray* dummyArray = newProp->NewInstance();
    dummyArray->SetVoidArray(newProp->GetVoidPointer(4), 3, 1);

    // Read GAMINF, BETA, TINF
    if ( this->ReadScalar(qFp, 3, dummyArray) == 0)
      {
      vtkErrorMacro("Encountered premature end-of-file while reading "
                    "the q file (or the file is corrupt).");
      fclose(qFp);
      dummyArray->Delete();
      newProp->Delete();
      return 0;
      }

    // igam is an int
    int igam;
    this->ReadIntBlock(qFp, 1, &igam);
    newProp->SetTuple1(7, igam);

    dummyArray->SetVoidArray(newProp->GetVoidPointer(8), 3, 1);
    // Read the rest of properties
    if ( this->ReadScalar(qFp, this->NumProperties - 8, dummyArray) == 0)
      {
      vtkErrorMacro("Encountered premature end-of-file while reading "
                    "the q file (or the file is corrupt).");
      fclose(qFp);
      dummyArray->Delete();
      newProp->Delete();
      return 0;
      }
    dummyArray->Delete();
    }
  int end = this->SkipByteCount(qFp);
  if(start != end)
  {
      vtkErrorMacro("ReadSolutionProperties error.");
  }

  this->Properties = newProp;
  this->Properties->Register(this);
  newProp->Delete();
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
      // Number of scalars to  be read: 
      //     1 for this->Density 
      //     1 for this->Energy 
      //     NumDims for Momentum
      int ns = 1  + 1  + this->Internal->NumberOfDimensions;
      if (this->IsOverflow)
      {
          // Add NumDims for gamma if nq >= 6
          if (this->OverflowNQ >= 6)
              ns += 1;
          // Add Species Density
          ns += this->OverflowNQC; 
          if ((this->OverflowNQ - 6 - this->OverflowNQC) > 0)
            ns += this->OverflowNQ - 6 - this->OverflowNQC;
      }
      if (this->Internal->BinaryFile)
        {
        int n = 4;
        if (this->IsOverflow)
          {
          fseek(qFp, this->SolutionOffsets[0], SEEK_SET);
          int count = this->SkipByteCount(qFp);
          n = (count-4)/this->Internal->Precision + 1;
          }
        this->NumProperties = n;
        rewind(qFp);
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
vtkPLOT3DReader::GetOutput()
{
  return output;
}

void
vtkPLOT3DReader::SetGridNumber(int gn)
{
  if (0 <= gn && gn < this->NumberOfGrids)
    {
    this->GridNumber = gn;
    this->Modified();
    }
  else
    {
    vtkErrorMacro(<<"Invalid Grid number specified! max: " <<  this->NumberOfGrids -1);
    }
}


void vtkPLOT3DReader::SetByteOrderToBigEndian()
{
  this->ByteOrder = FILE_BIG_ENDIAN;
}

void vtkPLOT3DReader::SetByteOrderToLittleEndian()
{
  this->ByteOrder = FILE_LITTLE_ENDIAN;
}

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

void vtkPLOT3DReader::PrintSelf(ostream& os, vtkIndent indent)
{
  os << indent << "XYZ File Name: " <<
    (this->XYZFileName ? this->XYZFileName : "(none)") << "\n";
  os << indent << "Q File Name: " <<
    (this->QFileName ? this->QFileName : "(none)") << "\n";
  os << indent << "BinaryFile: " << this->BinaryFile << endl;
  os << indent << "HasByteCount: " << this->HasByteCount << endl;
  os << indent << "Gamma: " << this->Gamma << endl;
  os << indent << "R: " << this->R << endl;
  os << indent << "MultiGrid: " << this->MultiGrid << endl;
  os << indent << "IBlanking: " << this->IBlanking << endl;
  os << indent << "ByteOrder: " << this->ByteOrder << endl;
  os << indent << "TwoDimensionalGeometry: " << (this->TwoDimensionalGeometry?"on":"off")
     << endl;
  os << indent << "Double Precision:" << this->DoublePrecision << endl;
}

template <typename DataType>
int
TReadAsciiScalar(FILE *fp, const int n, DataType *values)
{
  string fmt;
  if (sizeof(DataType) == 4)
    fmt = "%f";
  else
    fmt = "%lf";

  int count = 0;
  for(int i=0; i<n; i++)
    {
    int num = fscanf(fp, fmt.c_str(), &(values[i]));
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

template <typename DataType>
int
TReadAsciiVector(FILE *fp, const int n, const int numDims,
DataType *values)
{
  memset(values, 0, n*3*sizeof(DataType));
  int count = 0;
  DataType *temp = new DataType[n];
  for (int comp = 0; comp < numDims; ++comp)
    {   
    count += TReadAsciiScalar(fp, n, temp);
    for (int i = 0; i < n; ++i)
        values[3*i+comp] = temp[i];
    }
  delete [] temp;
  return count;
}



int
ExtractNumber(const char *field)
{
    int d = -1;
    std::string f(field);
    size_t pos = f.find("#");
    if (pos != std::string::npos)
    {
        f = f.substr(pos+1);
        char tmp[100];
        sscanf(field, "%d %s", &d, tmp);
    }
    return d;
}

int
vtkPLOT3DReader::GetFunction_float(const char *funcName, float *f)
{
    PLOT3DFunctions<float> P3DF_float;
    return GetFunction(P3DF_float, funcName, f);
}

int
vtkPLOT3DReader::GetFunction_double(const char *funcName, double *f)
{
    PLOT3DFunctions<double> P3DF_double;
    return GetFunction(P3DF_double, funcName, f);
}

// ---------------------------------------------------------------------
//  Templated (on float/double) to retrieve PLOT3D functions.
//
//  This method has early returns: when an error is encountered, or
//  enough data has been read from the solution file to calculate
//  the desired function.
// ---------------------------------------------------------------------

template <class DataType>
int
vtkPLOT3DReader::GetFunction(PLOT3DFunctions<DataType> &P3DF_helper, const char *funcName, DataType *f)
{
  if (!this->QFileName || this->QFileName[0] == '\0')
      return VTK_ERROR;

  FILE* qFp;
  if ( this->CheckSolutionFile(qFp) != VTK_OK)
    {
    return VTK_ERROR;
    }

  if ( this->ReadQHeader(qFp) != VTK_OK )
    {
    fclose(qFp);
    return VTK_ERROR;
    }

  if(this->ReadSolutionProperties(qFp) == VTK_ERROR)
    {
    fclose(qFp);
    return VTK_ERROR;
    }

  string sFName(funcName);
  this->SkipByteCount(qFp);

  int numPts  = this->GridSizes[this->GridNumber];
  int numDims = this->Internal->NumberOfDimensions;

  vtkPLOT3DArrayReader<DataType> arrayReader;
  arrayReader.ByteOrder = this->Internal->ByteOrder;


  // DENSITY
  DataType *density = new DataType[numPts];
  int success;
  if (this->Internal->BinaryFile)
    success = arrayReader.ReadScalar(qFp, numPts, density);
  else 
    success = TReadAsciiScalar<DataType>(qFp, numPts, density);

  if (!success)
    {
    vtkErrorMacro("Encountered premature end-of-file while reading "
                  "the q file for Density (or the file is corrupt).");
    delete [] density;
    return VTK_ERROR;
    }

  if (sFName == "Density")
    {
    for (int i = 0; i < numPts; ++i)
      f[i] = density[i];
    delete [] density;
    fclose(qFp);
    return VTK_OK;
    }

  // MOMENTUM
  DataType *momentum = new DataType[numPts*3];
  if (this->Internal->BinaryFile)
    {
    success = arrayReader.ReadVector(qFp, numPts, numDims, momentum);
    }
  else 
    {
    success = TReadAsciiVector<DataType>(qFp, numPts, numDims, momentum);
    }

  if (!success)
    {
    vtkErrorMacro("Encountered premature end-of-file while reading "
                  "the q file for Momentum (or the file is corrupt).");
    delete [] momentum;
    delete [] density;
    fclose(qFp);
    return VTK_ERROR;
    }

  bool finished = false;
  if (sFName == "Momentum")
    {
    for (int i = 0; i < numPts*3; ++i)
      f[i] = momentum[i];
    finished = true; 
    }
  else if (sFName == "KineticEnergy")
    {
    P3DF_helper.ComputeKineticEnergy(
        numPts, f, 
        density, momentum);
    finished = true;
    }
  else if (sFName == "Velocity")
    {
    P3DF_helper.ComputeVelocity(
        numPts, f, 
        density, momentum);
    finished = true;
    }
  else if (sFName == "Vorticity")
    {
    DataType *velocity  = new DataType[3*numPts];
    P3DF_helper.ComputeVelocity(
        numPts, velocity, 
        density, momentum);

    P3DF_helper.ComputeVorticity(
        numPts, f, 
        (DataType*)this->output->GetPoints()->GetData()->GetVoidPointer(0),
        velocity,
        this->output->GetDimensions());

    delete [] velocity;
    finished = true;
    }
  else if (sFName == "Swirl")
    {
    DataType *velocity  = new DataType[3*numPts];
    DataType *vorticity = new DataType[3*numPts];
    P3DF_helper.ComputeVelocity(
        numPts, velocity, 
        density, momentum);
    P3DF_helper.ComputeVorticity(
        numPts, vorticity, 
        (DataType*)this->output->GetPoints()->GetData()->GetVoidPointer(0),
        velocity,
        this->output->GetDimensions());
   
    P3DF_helper.ComputeSwirl(
        numPts, f, 
        density, momentum, vorticity);

    delete [] vorticity;
    delete [] velocity;
    finished = true; 
    }
  else if (sFName == "StrainRate")
    {
    DataType *velocity  = new DataType[3*numPts];
    P3DF_helper.ComputeVelocity(
        numPts, velocity, 
        density, momentum);

    P3DF_helper.ComputeStrainRate(
        numPts, f, 
        (DataType*)this->output->GetPoints()->GetData()->GetVoidPointer(0),
        velocity,
        this->output->GetDimensions());

    delete [] velocity;
    finished = true;
    }

  if (finished)
    {
    delete [] momentum;
    delete [] density;
    fclose(qFp);
    return VTK_OK;
    }

  // ENERGY
  DataType *energy = new DataType[numPts];

  if (this->Internal->BinaryFile)
    {
    success = arrayReader.ReadScalar(qFp, numPts, energy);
    }
  else 
    {
    success = TReadAsciiScalar(qFp, numPts, energy);
    }

  if (!success)
    {
    vtkErrorMacro("Encountered premature end-of-file while reading "
                  "the q file for Energy (or the file is corrupt).");
    delete [] energy;
    delete [] momentum;
    delete [] density;
    fclose(qFp);
    return VTK_ERROR;
    }

  if (sFName == "StagnationEnergy" ||
      sFName == "InternalEnergy")
    {
    for (int i = 0; i < numPts; ++i)
      f[i] = energy[i];
    delete [] energy;
    delete [] momentum;
    delete [] density;
    fclose(qFp);
    return VTK_OK;
    }

  // Non overflow derived quantities
  if (sFName == "Pressure")
    {
    P3DF_helper.ComputePressure(
        numPts, f, 
        density, momentum, energy, this->Gamma);
    finished = true; 
    }
  else if (sFName == "PressureGradient")
    {
    DataType *pressure = new DataType[numPts];
    P3DF_helper.ComputePressure(
        numPts, pressure, 
        density, momentum, energy, this->Gamma);

    P3DF_helper.ComputePressureGradient(
        numPts, f, 
        (DataType*)this->output->GetPoints()->GetData()->GetVoidPointer(0),
        pressure,
        this->output->GetDimensions());

    delete [] pressure;
    finished = true; 
    }
  else if (sFName == "Temperature")
    {
    P3DF_helper.ComputeTemperature(
        numPts, f, 
        density, momentum, energy, this->R, this->Gamma);
    finished = true; 
    }
  else if (sFName == "Enthalpy")
    {
    P3DF_helper.ComputeEnthalpy(
        numPts, f, 
        density, momentum, energy, this->Gamma);
    finished = true; 
    }
  else if (sFName == "Entropy")
    {
    P3DF_helper.ComputeEntropy(
        numPts, f, 
        density, momentum, energy, this->R, this->Gamma);
    finished = true; 
    }

  if (finished)
    {
    delete [] energy;
    delete [] momentum;
    delete [] density;
    fclose(qFp);
    return VTK_OK;
    }


  // OVERFLOW

  if (this->IsOverflow)
    {
    // First, do we really need to read any of this?
    string name(funcName);
    string onames("Gamma PressureCoefficient MachNumber SoundSpeed");
    bool doGamma = (onames.find(name) != string::npos);
    bool doSpecies = ( name.find("Species") != string::npos ||
                       name.find("Spec Dens") != string::npos ||
                       name.find("Turb") != string::npos);
   
    DataType *temp = new DataType[numPts]; 
    if(doGamma || (doSpecies && this->OverflowNQ >=6))
      {
      if (this->Internal->BinaryFile)
        {
        success = arrayReader.ReadScalar(qFp, numPts, temp);
        }
      else
        {
        success = TReadAsciiScalar(qFp, numPts, temp);
        }
      }
    if (!success)
      {
      vtkErrorMacro("Encountered premature end-of-file while reading "
                    "the q file for Gamma (or the file is corrupt).");
      delete [] temp;
      delete [] energy;
      delete [] momentum;
      delete [] density;
      fclose(qFp);
      return VTK_ERROR;
      }

    if (doGamma)
      {  
      if (name == "Gamma")
        {
        for (int i = 0; i < numPts; ++i)
          f[i] = temp[i];
        }
      else if (name == "PressureCoefficient")
        {
        P3DF_helper.ComputePressureCoefficient(
            numPts, f, 
            density, momentum, energy, temp, 
            (DataType)this->Properties->GetComponent(0, 4),
            (DataType)this->Properties->GetComponent(0, 0));
        }
      else if (name == "MachNumber")
        {
        P3DF_helper.ComputeMachNumber(
            numPts, f, 
            density, momentum, energy, temp);
        }
      else if (name == "SoundSpeed")
        {
        P3DF_helper.ComputeSoundSpeed(
            numPts, f, 
            density, momentum, energy, temp);
        }
      delete [] temp;
      delete [] energy;
      delete [] momentum;
      delete [] density;
      fclose(qFp);
      return VTK_OK;
      }  
    else
      {
      int fn = ExtractNumber(funcName);
      bool spec    = name.find("Species")   != string::npos;
      bool specrho = name.find("Spec Dens") != string::npos;
      bool turb    = name.find("Turb")      != string::npos;

      for (int j = 0; j < this->OverflowNQC && !finished; ++j)
        {
        if (this->Internal->BinaryFile)
          {
          success = arrayReader.ReadScalar(qFp, numPts, temp);
          }
        else
          {
          success = TReadAsciiScalar(qFp, numPts, temp);
          }
        if (!success)
          {
          vtkErrorMacro("Encountered premature end-of-file while reading "
                        "the q file for Gamma (or the file is corrupt).");
          delete [] temp;
          delete [] energy;
          delete [] momentum;
          delete [] density;
          fclose(qFp);
          return VTK_ERROR;
          }
        if(spec && fn == j+1)
          {
          for (int i = 0; i < numPts; ++i)
            f[i] = temp[i];
          finished = true;
          }
        else if(specrho && fn == j+1)
          {
          double r;
          for (int i = 0; i < numPts; ++i)
            {
            r = density[i];
            r = (r != 0.0 ? r : 1.0);
            f[i] = temp[i]/r;
            }
          finished = true;
          }
        }
      if (turb)
        {
        for(int a=0; a<this->OverflowNQ-6-this->OverflowNQC && !finished; a++)
          {
          if (this->Internal->BinaryFile)
            {
            success = arrayReader.ReadScalar(qFp, numPts, temp);
            }
          else
            {
            success = TReadAsciiScalar(qFp, numPts, temp);
            }
          if (!success)
            {
            vtkErrorMacro("Encountered premature end-of-file while reading "
                          "the q file (or the file is corrupt).");
            delete [] temp;
            delete [] energy;
            delete [] momentum;
            delete [] density;
            fclose(qFp);
            return VTK_ERROR;
            }
          if(fn == a+1)
            {
            for (int i = 0; i < numPts; ++i)
              f[i] = temp[i];
            finished = true;
            }
          } // for
        }
      delete [] temp;
      delete [] energy;
      delete [] momentum;
      delete [] density;
      fclose(qFp);
      return VTK_OK;
      }  
    } // is overflow
  fclose(qFp);
  return VTK_ERROR;
}


int vtkPLOT3DReader::GetPrecision() 
{ 
  return this->Internal->Precision;
}


double
vtkPLOT3DReader::GetTime()
{
  double time = -DBL_MAX; // INVALID_TIME
  if (this->Properties && 
      this->Properties->GetMTime() > this->GetMTime() &&
      this->Properties->GetNumberOfTuples() > 3)
    {
    time = this->Properties->GetTuple1(3);
    }
  return time;
}
