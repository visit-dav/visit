// ************************************************************************* //
//                            avtPLOT3DFileFormat.C                          //
// ************************************************************************* //

// FIRST INCLUDE VTK CLASSES

// NOTE THAT THIS IS VTK's PLOT3D READER WITH OUR BUG FIXES AND ENHANCEMENT
// FOR READING IBLANKING.  THIS SHOULD ONLY BE INCLUDED IN VISIT'S
// REPOSITORY UNTIL THE KITWARE REPOSITORY HAS THESE CHANGES
/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItPLOT3DReader.h,v $
  Language:  C++
  Date:      $Date: 2002/01/22 15:38:17 $
  Version:   $Revision: 1.48 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItPLOT3DReader - read PLOT3D data files
// .SECTION Description
// vtkVisItPLOT3DReader is a reader object that reads PLOT3D formatted files and 
// generates a structured grid on output. PLOT3D is a computer graphics 
// program designed to visualize the grids and solutions of computational 
// fluid dynamics. Please see the "PLOT3D User's Manual" available from 
// NASA Ames Research Center, Moffett Field CA.
//
// PLOT3D files consist of a grid file (also known as XYZ file), an 
// optional solution file (also known as a Q file), and an optional function 
// file that contains user created data. The Q file contains solution 
// information as follows: the four parameters free stream mach number 
// (Fsmach), angle of attack (Alpha), Reynolds number (Re), and total 
// integration time (Time). In addition, the solution file contains 
// the flow density (scalar), flow momentum (vector), and flow energy (scalar).
//
// The reader can generate additional scalars and vectors (or "functions")
// from this information. To use vtkVisItPLOT3DReader, you must specify the 
// particular function number for the scalar and vector you want to visualize.
// This implementation of the reader provides the following functions. The
// scalar functions are:
//    -1  - don't read or compute any scalars
//    100 - density
//    110 - pressure
//    120 - temperature
//    130 - enthalpy
//    140 - internal energy
//    144 - kinetic energy
//    153 - velocity magnitude
//    163 - stagnation energy
//    170 - entropy
//    184 - swirl.
//
// The vector functions are:
//    -1  - don't read or compute any vectors
//    200 - velocity
//    201 - vorticity
//    202 - momentum
//    210 - pressure gradient.
//
// (Other functions are described in the PLOT3D spec, but only those listed are
// implemented here.) Note that by default, this reader creates the density 
// scalar (100) and momentum vector (202) as output. (These are just read in
// from the solution file.) Please note that the validity of computation is
// a function of this class's gas constants (R, Gamma) and the equations used.
// They may not be suitable for your computational domain.
//
// Additionally, you can read other data and associate it as a vtkDataArray
// into the output's point attribute data. Use the method AddFunction()
// to list all the functions that you'd like to read. AddFunction() accepts
// an integer parameter that defines the function number.
//
// The format of the function file is as follows. An integer indicating 
// number of grids, then an integer specifying number of functions per each 
// grid. This is followed by the (integer) dimensions of each grid in the 
// file. Finally, for each grid, and for each function, a float value per 
// each point in the current grid. Note: if both a function from the function
// file is specified, as well as a scalar from the solution file (or derived
// from the solution file), the function file takes precedence.

#ifndef __vtkVisItPLOT3DReader_h
#define __vtkVisItPLOT3DReader_h

#include <stdio.h>
#include <visit_vtk_exports.h>
#include "vtkStructuredGridSource.h"

class vtkIntArray;
class vtkFloatArray;
class vtkPointData;
class vtkPoints;
class vtkStructuredGrid;

// file formats
#define VTK_WHOLE_SINGLE_GRID_NO_IBLANKING 0
#define VTK_WHOLE_MULTI_GRID_NO_IBLANKING 2
#define VTK_WHOLE_MULTI_GRID_WITH_IBLANKING 3

class VISIT_VTK_API vtkVisItPLOT3DReader : public vtkStructuredGridSource 
{
public:
  static vtkVisItPLOT3DReader *New();
  vtkTypeRevisionMacro(vtkVisItPLOT3DReader,vtkStructuredGridSource);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify the PLOT3D file format to use
  vtkSetClampMacro(FileFormat,int,0,7);
  vtkGetMacro(FileFormat,int);

  // Description:
  // Set/Get the PLOT3D geometry FileName.
  vtkSetStringMacro(XYZFileName);
  vtkGetStringMacro(XYZFileName);

  // Description:
  // Set/Get the PLOT3D solution FileName.
  vtkSetStringMacro(QFileName);
  vtkGetStringMacro(QFileName);

  // Description:
  // Set/Get the PLOT3D function FileName.
  vtkSetStringMacro(FunctionFileName);
  vtkGetStringMacro(FunctionFileName);

  // Description:
  // Set/Get the PLOT3D vector FileName.
  vtkSetStringMacro(VectorFunctionFileName);
  vtkGetStringMacro(VectorFunctionFileName);

  // Description:
  // Specify the grid to read.
  vtkSetMacro(GridNumber,int);
  vtkGetMacro(GridNumber,int);

  // Description:
  // Specify the scalar function to extract. If ==(-1), then no scalar 
  // function is extracted.
  vtkSetMacro(ScalarFunctionNumber,int);
  vtkGetMacro(ScalarFunctionNumber,int);

  // Description:
  // Specify the vector function to extract. If ==(-1), then no vector
  // function is extracted.
  vtkSetMacro(VectorFunctionNumber,int);
  vtkGetMacro(VectorFunctionNumber,int);

  // Description:
  // Specify additional functions to read. These are placed into the
  // point data as data arrays. Later on they can be used by labeling
  // them as scalars, etc.
  void AddFunction(int functionNumber);
  void RemoveFunction(int);
  void RemoveAllFunctions();

  // these are read from PLOT3D file
  // Description:
  // Get the free-stream mach number.
  vtkGetMacro(Fsmach,float);

  // Description:
  // Get the angle of attack.
  vtkGetMacro(Alpha,float);

  // Description:
  // Get the Reynold's number.
  vtkGetMacro(Re,float);

  // Description:
  // Get the total integration time.
  vtkGetMacro(Time,float);

  // Description:
  // Set/Get the gas constant.
  vtkSetMacro(R,float);
  vtkGetMacro(R,float);

  // Description:
  // Set/Get the ratio of specific heats.
  vtkSetMacro(Gamma,float);
  vtkGetMacro(Gamma,float);

  // Description:
  // Set/Get the x-component of the free-stream velocity.
  vtkSetMacro(Uvinf,float);
  vtkGetMacro(Uvinf,float);

  // Description:
  // Set/Get the y-component of the free-stream velocity.
  vtkSetMacro(Vvinf,float);
  vtkGetMacro(Vvinf,float);

  // Description:
  // Set/Get the z-component of the free-stream velocity.
  vtkSetMacro(Wvinf,float);
  vtkGetMacro(Wvinf,float);

  // Description:
  // Get the number of grids. This is valid only after a
  // read has been performed.
  vtkGetMacro(NumberOfGrids, int);

protected:
  vtkVisItPLOT3DReader();
  ~vtkVisItPLOT3DReader();

  void ExecuteInformation();
  void Execute();
  int GetFileType(FILE *fp);

  //plot3d FileNames
  int FileFormat; //various PLOT3D formats
  char *XYZFileName;
  char *QFileName;
  char *FunctionFileName;
  char *VectorFunctionFileName;

  //flags describing data to be read
  int GridNumber; //for multi-grid files, the one we're interested in
  int ScalarFunctionNumber;
  int VectorFunctionNumber;
  int FunctionFileFunctionNumber;
  void MapFunction(int fNumber,vtkPointData *outputPD);

  //functions to read that are not scalars or vectors
  vtkIntArray *FunctionList;

  //temporary variables used during read
  float *TempStorage;
  int NumberOfPoints;
  int NumberOfGrids;

  //supplied in PLOT3D file
  float Fsmach;
  float Alpha;
  float Re;
  float Time;

  //parameters used in computing derived functions
  float R; 
  float Gamma;
  float Uvinf;
  float Vvinf;
  float Wvinf;
  
  //methods to read data
  int ReadBinaryGrid(FILE *fp,vtkStructuredGrid *output);
  int ReadBinaryGridDimensions(FILE *fp, vtkStructuredGrid *output);
  int ReadBinarySolution(FILE *fp, vtkStructuredGrid *output);
  int ReadBinaryFunctionFile(FILE *fp, vtkStructuredGrid *output);
  int ReadBinaryVectorFunctionFile(FILE *fp, vtkStructuredGrid *output);

  vtkPoints *Grid;
  vtkFloatArray *Density;
  vtkFloatArray *Energy;
  vtkFloatArray *Momentum;

  // derived functions from data in PLOT3D files
  void ComputeDensity(vtkPointData *outputPD);
  void ComputePressure(vtkPointData *outputPD);
  void ComputeTemperature(vtkPointData *outputPD);
  void ComputeEnthalpy(vtkPointData *outputPD);
  void ComputeInternalEnergy(vtkPointData *outputPD);
  void ComputeKineticEnergy(vtkPointData *outputPD);
  void ComputeVelocityMagnitude(vtkPointData *outputPD);
  void ComputeStagnationEnergy(vtkPointData *outputPD);
  void ComputeEntropy(vtkPointData *outputPD);
  void ComputeSwirl(vtkPointData *outputPD);

  void ComputeVelocity(vtkPointData *outputPD);
  void ComputeVorticity(vtkPointData *outputPD);
  void ComputeMomentum(vtkPointData *outputPD);
  void ComputePressureGradient(vtkPointData *outputPD);

private:
  vtkVisItPLOT3DReader(const vtkVisItPLOT3DReader&);  // Not implemented.
  void operator=(const vtkVisItPLOT3DReader&);  // Not implemented.
};

#endif

// NOTE THAT THIS IS VTK's PLOT3D READER WITH OUR BUG FIXES AND ENHANCEMENT
// FOR READING IBLANKING.  THIS SHOULD ONLY BE INCLUDED IN VISIT'S
// REPOSITORY UNTIL THE KITWARE REPOSITORY HAS THESE CHANGES
/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItPLOT3DReader.cxx,v $
  Language:  C++
  Date:      $Date: 2002/01/22 15:38:17 $
  Version:   $Revision: 1.64 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <vtkByteSwap.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkStructuredGrid.h>

#include <ctype.h>
#include <math.h>

vtkCxxRevisionMacro(vtkVisItPLOT3DReader, "$Revision: 1.64 $");
vtkStandardNewMacro(vtkVisItPLOT3DReader);

#define VTK_BINARY 0
#define VTK_ASCII 1

#define VTK_RHOINF 1.0
#define VTK_CINF 1.0
#define VTK_PINF ((VTK_RHOINF*VTK_CINF) * (VTK_RHOINF*VTK_CINF) / this->Gamma)
#define VTK_CV (this->R / (this->Gamma-1.0))

vtkVisItPLOT3DReader::vtkVisItPLOT3DReader()
{
  this->FileFormat = VTK_WHOLE_SINGLE_GRID_NO_IBLANKING;

  this->XYZFileName = NULL;
  this->QFileName = NULL;
  this->FunctionFileName = NULL;
  this->VectorFunctionFileName = NULL;

  this->FunctionList = vtkIntArray::New();

  this->GridNumber = 0;
  this->ScalarFunctionNumber = 100;
  this->VectorFunctionNumber = 202;
  this->FunctionFileFunctionNumber = -1;

  this->Fsmach = 0.0;
  this->Alpha = 0.0;
  this->Re = 0.0;
  this->Time = 0.0;

  this->R = 1.0;
  this->Gamma = 1.4;
  this->Uvinf = 0.0;
  this->Vvinf = 0.0;
  this->Wvinf = 0.0;

  this->Grid = NULL;
  this->Energy = NULL;
  this->Density = NULL;
  this->Momentum = NULL;

  this->NumberOfGrids = 0;
} 

vtkVisItPLOT3DReader::~vtkVisItPLOT3DReader()
{
  if ( this->XYZFileName )
    {
    delete [] this->XYZFileName;
    }
  if ( this->QFileName )
    {
    delete [] this->QFileName;
    }
  if ( this->FunctionFileName )
    {
    delete [] this->FunctionFileName;
    }
  if ( this->VectorFunctionFileName )
    {
    delete [] this->VectorFunctionFileName;
    }
  this->FunctionList->Delete();
}

void vtkVisItPLOT3DReader::RemoveFunction(int fnum)
{
  for (int i=0; i < this->FunctionList->GetNumberOfTuples(); i++ )
    {
    if ( this->FunctionList->GetValue(i) == fnum )
      {
      this->FunctionList->SetValue(i,-1);
      }
    }
}

void vtkVisItPLOT3DReader::ExecuteInformation()
{
  FILE *xyzFp;
  int error = 0;
  vtkStructuredGrid *output = this->GetOutput();

  // must go through all the same checks as actual read.
  if ( this->XYZFileName == NULL )
    {
    vtkErrorMacro(<< "Must specify geometry file");
    return;
    }
  if ( (xyzFp = fopen(this->XYZFileName, "r")) == NULL)
    {
    vtkErrorMacro(<< "File: " << this->XYZFileName << " not found");
    return;
    }
  if ( this->GetFileType(xyzFp) == VTK_ASCII )
    {
    vtkWarningMacro("reading ascii grid files currently not supported");
    // error = this->ReadASCIIGrid(xyzFp);
    }
  else
    {
    fclose(xyzFp);
    xyzFp = fopen(this->XYZFileName, "rb");
    // reads the whole extent
    error = this->ReadBinaryGridDimensions(xyzFp,output);
    fclose(xyzFp);
    }
  
  if ( error )
    {
    vtkErrorMacro(<<"Error reading XYZ file");
    return;
    }
}

void vtkVisItPLOT3DReader::Execute()
{
  FILE *xyzFp, *QFp, *funcFp;
  int error = 0;
  vtkStructuredGrid *output = this->GetOutput();
  vtkPointData *outputPD = output->GetPointData();
  
  // Initialize output and read geometry
  //
  if ( this->XYZFileName == NULL )
    {
    output->Initialize();
    vtkErrorMacro(<< "Must specify geometry file");
    return;
    }
  if ( (xyzFp = fopen(this->XYZFileName, "r")) == NULL)
    {
    output->Initialize();
    vtkErrorMacro(<< "File: " << this->XYZFileName << " not found");
    return;
    }
  if ( this->GetFileType(xyzFp) == VTK_ASCII )
    {
    vtkWarningMacro("reading ascii grid files currently not supported");
    // error = this->ReadASCIIGrid(xyzFp);
    }
  else
    {
    fclose(xyzFp);
    xyzFp = fopen(this->XYZFileName, "rb");
    error = this->ReadBinaryGrid(xyzFp,output);
    fclose(xyzFp);
    }
  
  if ( error )
    {
    output->Initialize();
    vtkErrorMacro(<<"Error reading XYZ file");
    return;
    }

  // Read solution file (if available and requested)
  //
  if ( this->QFileName && 
  ((this->FunctionFileName == NULL && this->ScalarFunctionNumber >= 0) ||
  (this->VectorFunctionFileName == NULL && this->VectorFunctionNumber >= 0)) )
    {
    if ( (QFp = fopen(this->QFileName, "r")) == NULL)
      {
      output->Initialize();
      vtkErrorMacro(<< "File: " << this->QFileName << " not found");
      return;
      }

    if ( this->GetFileType(QFp) == VTK_ASCII )
      {
      vtkWarningMacro("reading ascii solution files currently not supported");
      // error = this->ReadASCIISolution(QFp);
      }
    else
      {
      fclose(QFp);
      QFp = fopen(this->QFileName, "rb");
      error = this->ReadBinarySolution(QFp,output);
      fclose(QFp);
      }
    
    if ( error )
      {
      output->Initialize();
      vtkErrorMacro(<<"Error reading solution file");
      return;
      }

    // Read solutions as general point attribute data
    if ( this->FunctionList->GetNumberOfTuples() > 0 )
      {
      int fnum;
      for (int i=0; i < this->FunctionList->GetNumberOfTuples(); i++)
        {
        if ( (fnum=this->FunctionList->GetValue(i)) >= 0 )
          {
          this->MapFunction(fnum,outputPD);
          }
        }
      }

    this->MapFunction(this->ScalarFunctionNumber,outputPD);
    this->MapFunction(this->VectorFunctionNumber,outputPD);
    }

  // Read function file (if available)
  //
  if ( this->FunctionFileName != NULL)
    {
    if ( (funcFp = fopen(this->FunctionFileName, "r")) == NULL)
      {
      output->Initialize();
      vtkErrorMacro(<< "File: " << this->FunctionFileName << " not found");
      return;
      }

    if ( this->GetFileType(funcFp) == VTK_ASCII )
      {
      vtkWarningMacro("reading ASCII function files currently not supported");
      // error = this->ReadASCIIFunctionFile(funcFp);
      }
    else
      {
      fclose(funcFp);
      funcFp = fopen(this->FunctionFileName, "rb");
      error = this->ReadBinaryFunctionFile(funcFp,output);
      fclose(funcFp);
      }
    
    if ( error )
      {
      vtkErrorMacro(<<"Error reading function file");
      return;
      }
    }

  // Read vector function file (if available)
  //
  if ( this->VectorFunctionFileName != NULL )
    {
    if ( (funcFp = fopen(this->VectorFunctionFileName, "r")) == NULL)
      {
      output->Initialize();
      vtkErrorMacro(<< "File: " << this->VectorFunctionFileName << " not found");
      return;
      }

    if ( this->GetFileType(funcFp) == VTK_ASCII )
      {
      vtkWarningMacro("reading ASCII vector function files currently not supported");
      // error = this->ReadASCIIFunctionFile(funcFp);
      }
    else
      {
      fclose(funcFp);
      funcFp = fopen(this->VectorFunctionFileName, "rb");
      error = this->ReadBinaryVectorFunctionFile(funcFp,output);
      fclose(funcFp);
      }
    
    if ( error )
      {
      output->Initialize();
      vtkErrorMacro(<<"Error reading vector function file");
      return;
      }
    }

  // Reading is finished; free any extra memory. Data objects that comprise the
  // output will not be released with the UnRegister() method since they are
  // registered more than once.
  //
  delete [] this->TempStorage;
  this->TempStorage = NULL;

  this->Grid->UnRegister(this);
  this->Grid = NULL;

  if ( this->Density ) 
    {
    this->Density->UnRegister(this);
    this->Density = NULL;
    }

  if ( this->Energy ) 
    {
    this->Energy->UnRegister(this);
    this->Energy = NULL;
    }

  if ( this->Momentum ) 
    {
    this->Momentum->UnRegister(this);
    this->Momentum = NULL;
    }
}

int vtkVisItPLOT3DReader::ReadBinaryGrid(FILE *fp,vtkStructuredGrid *output)
{
  vtkPoints *newPts;
  int dim[3];
  int i, gridFound, offset, gridSize;
  float x[3];
  
  if ( this->FileFormat == VTK_WHOLE_MULTI_GRID_NO_IBLANKING ||
       this->FileFormat == VTK_WHOLE_MULTI_GRID_WITH_IBLANKING )
    {
    if (fread(&(this->NumberOfGrids), sizeof(int), 1, fp) < 1 )
      {
      return 1;
      }
    vtkByteSwap::Swap4BE(&(this->NumberOfGrids));
    }
  else
    {
    this->NumberOfGrids = 1;
    }

  // Loop over grids, reading one that has been specified
  //
  for (gridFound=0, offset=0, i=0; i<this->NumberOfGrids; i++) 
    {
    //read dimensions
    if ( fread (dim, sizeof(int), 3, fp) < 3 )
      {
      return 1;
      }
    vtkByteSwap::Swap4BERange(dim,3);
    
    gridSize = dim[0] * dim[1] * dim[2];

    if ( i < this->GridNumber ) 
      {
      if ( this->FileFormat == VTK_WHOLE_MULTI_GRID_WITH_IBLANKING )
        {
        offset += 4*gridSize;
        }
      else
        {
        offset += 3*gridSize;
        }
      }
    else if ( i == this->GridNumber ) 
      {
      gridFound = 1;
      this->NumberOfPoints = gridSize;
      output->SetDimensions(dim);
      }
    }

  if ( ! gridFound )
    {
    vtkErrorMacro (<<"Specified grid not found!");
    return 1;
    }
    
  //allocate temporary storage to read into + points
  this->TempStorage = new float[3*this->NumberOfPoints];
  newPts = vtkPoints::New();
  newPts->SetNumberOfPoints(this->NumberOfPoints);

  //seek to correct spot and read grid
  fseek (fp, (long)(offset*sizeof(float)), 1);

  //even if the file has iblanking, only read in the points fields (so 3, not 4).
  if ( fread(this->TempStorage, sizeof(float), 3*this->NumberOfPoints, fp) < (unsigned long)3*this->NumberOfPoints ) 
    {
    newPts->Delete();
    delete [] this->TempStorage;
    return 1;
    }
  else //successful read, load coordinates in points object
    {
    vtkByteSwap::Swap4BERange(this->TempStorage,3*this->NumberOfPoints);
    for (i=0; i < this->NumberOfPoints; i++)
      {
      x[0] = this->TempStorage[i];
      x[1] = this->TempStorage[this->NumberOfPoints+i];
      x[2] = this->TempStorage[2*this->NumberOfPoints+i];
      newPts->SetPoint(i,x);
      }
    }
  //
  // Now send data to ourselves
  //
  this->Grid = newPts;
  this->Grid->Register(this);
  output->SetPoints(newPts);
  newPts->Delete();

  vtkDebugMacro(<<"Read " << this->NumberOfPoints << " points");
  return 0;
}

// for UpdateInformation
int vtkVisItPLOT3DReader::ReadBinaryGridDimensions(FILE *fp,
                                              vtkStructuredGrid *output)
{
  int dim[3];
  int i, offset, gridSize;
  
  if ( this->FileFormat == VTK_WHOLE_MULTI_GRID_NO_IBLANKING ||
       this->FileFormat == VTK_WHOLE_MULTI_GRID_WITH_IBLANKING )
    {
    if (fread(&(this->NumberOfGrids), sizeof(int), 1, fp) < 1 )
      {
      return 1;
      }
    vtkByteSwap::Swap4BE(&(this->NumberOfGrids));
    }
  else
    {
    this->NumberOfGrids = 1;
    }
  //
  // Loop over grids, reading one that has been specified
  //
  for (offset=0, i=0; i<this->NumberOfGrids; i++) 
    {
    //read dimensions
    if ( fread (dim, sizeof(int), 3, fp) < 3 )
      {
      return 1;
      }
    vtkByteSwap::Swap4BERange(dim,3);
    
    gridSize = dim[0] * dim[1] * dim[2];

    if ( i < this->GridNumber ) 
      {
      offset += 3*gridSize;
      }
    else if ( i == this->GridNumber ) 
      {
      this->NumberOfPoints = gridSize;
      output->SetWholeExtent(0,dim[0]-1, 0,dim[1]-1, 0,dim[2]-1);
      return 0;
      }
    }
  // could not find grid
  return 1;
}

int vtkVisItPLOT3DReader::ReadBinarySolution(FILE *fp,vtkStructuredGrid *output)
{
  vtkFloatArray *newDensity, *newEnergy;
  vtkFloatArray *newMomentum;
  int dim[3];
  int i, gridFound, offset, gridSize;
  float m[3], params[4];
  int numGrids, numPts = 0;

  if ( this->FileFormat == VTK_WHOLE_MULTI_GRID_NO_IBLANKING ||
       this->FileFormat == VTK_WHOLE_MULTI_GRID_WITH_IBLANKING )
    {
    if ( fread (&numGrids, sizeof(int), 1, fp) < 1 )
      {
      return 1;
      }
    vtkByteSwap::Swap4BE(&numGrids);
    }
  else
    {
    numGrids = 1;
    }

  if ( numGrids != this->NumberOfGrids )
    {
    vtkErrorMacro(<<"Data mismatch in solution file!");
    return 1;
    }
  //
  // Loop over dimensions, reading grid dimensions that have been specified
  //
  for (gridFound=0, offset=0, i=0; i<numGrids; i++) 
    {
    //read dimensions
    if ( fread (dim, sizeof(int), 3, fp) < 3 )
      {
      return 1;
      }
    vtkByteSwap::Swap4BERange(dim,3);
    gridSize = dim[0] * dim[1] * dim[2];

    if ( i < this->GridNumber ) 
      {
      offset += 4; // skip condition values for grid, fix from Tom Johnson
      offset += 5*gridSize;
      }
    else if ( i == this->GridNumber ) 
      {
      gridFound = 1;
      numPts = gridSize;
      output->SetDimensions(dim);
      }
    }

  if ( ! gridFound )
    {
    vtkErrorMacro (<<"Specified grid not found!");
    return 1;
    }
    
  if ( numPts != this->NumberOfPoints )
    {
    vtkErrorMacro (<<"Data mismatch in solution file!");
    delete [] this->TempStorage;
    return 1;
    }
    
  //seek to correct spot and read solution
  fseek (fp, (long)(offset*sizeof(float)), 1);

  //read solution parameters
  if ( fread (params, sizeof(float), 4, fp) < 4 )
    {
    return 1;
    }
  vtkByteSwap::Swap4BERange(params,4);
  this->Fsmach = params[0];
  this->Alpha = params[1];
  this->Re = params[2];
  this->Time = params[3];

  //allocate temporary storage to copy density data into
  newDensity = vtkFloatArray::New();
  newDensity->SetNumberOfTuples(numPts);
  newDensity->SetName("Density");
  newEnergy = vtkFloatArray::New();
  newEnergy->SetNumberOfTuples(numPts);
  newEnergy->SetName("Energy");
  newMomentum = vtkFloatArray::New();
  newMomentum->SetNumberOfComponents(3);
  newMomentum->SetNumberOfTuples(numPts);
  newMomentum->SetName("Momentum");

  if (fread(this->TempStorage, sizeof(float), numPts, fp) < 
      (unsigned int)numPts ) 
    {
    newDensity->Delete();
    newMomentum->Delete();
    newEnergy->Delete();
    delete [] this->TempStorage;
    return 1;
    }
  else //successful read
    {
    vtkByteSwap::Swap4BERange(this->TempStorage,numPts);
    for (i=0; i < this->NumberOfPoints; i++) 
      {
      newDensity->SetValue(i,this->TempStorage[i]);
      }
    }

  if (fread(this->TempStorage, sizeof(float), 3*this->NumberOfPoints, fp) < 
      (unsigned int)3*this->NumberOfPoints ) 
    {
    newDensity->Delete();
    newMomentum->Delete();
    newEnergy->Delete();
    delete [] this->TempStorage;
    return 1;
    }
  else //successful read, load coordinates into vector object
    {
    vtkByteSwap::Swap4BERange(this->TempStorage,3*this->NumberOfPoints);
    for (i=0; i < this->NumberOfPoints; i++)
      {
      m[0] = this->TempStorage[i];
      m[1] = this->TempStorage[this->NumberOfPoints+i];
      m[2] = this->TempStorage[2*this->NumberOfPoints+i];
      newMomentum->SetTuple(i,m);
      }
    }

  if (fread(this->TempStorage, sizeof(float), numPts, fp) < 
      (unsigned int)numPts) 
    {
    newDensity->Delete();
    newMomentum->Delete();
    newEnergy->Delete();
    delete [] this->TempStorage;
    return 1;
    }
  else //successful read
    {
    vtkByteSwap::Swap4BERange(this->TempStorage,numPts);
    for (i=0; i < this->NumberOfPoints; i++) 
      {
      newEnergy->SetValue(i,this->TempStorage[i]);
      }
    }

  // Register data for use by computation functions
  //
  this->Density = newDensity;
  this->Density->SetName("Density");
  this->Density->Register(this);
  newDensity->Delete();

  this->Momentum = newMomentum;
  this->Momentum->SetName("Momentum");
  this->Momentum->Register(this);
  newMomentum->Delete();

  this->Energy = newEnergy;
  this->Energy->SetName("Energy");
  this->Energy->Register(this);
  newEnergy->Delete();

  return 0;
}

int vtkVisItPLOT3DReader::ReadBinaryFunctionFile(FILE *fp,vtkStructuredGrid *output)
{
  int numGrids;

  output = output;
  if ( this->FileFormat == VTK_WHOLE_MULTI_GRID_NO_IBLANKING ||
       this->FileFormat == VTK_WHOLE_MULTI_GRID_WITH_IBLANKING )
    {
    if ( fread (&numGrids, sizeof(int), 1, fp) < 1 )
      {
      return 1;
      }
    vtkByteSwap::Swap4BE(&numGrids);
    }
  else
    {
    numGrids = 1;
    }

  if ( numGrids != this->NumberOfGrids )
    {
    vtkErrorMacro(<<"Data mismatch in function file!");
    return 1;
    }

  return 0;
}

int vtkVisItPLOT3DReader::ReadBinaryVectorFunctionFile(FILE *fp,vtkStructuredGrid *output)
{
  int numGrids;

  output = output;
  if ( this->FileFormat == VTK_WHOLE_MULTI_GRID_NO_IBLANKING ||
       this->FileFormat == VTK_WHOLE_MULTI_GRID_WITH_IBLANKING )
    {
    if ( fread (&numGrids, sizeof(int), 1, fp) < 1 )
      {
      return 1;
      }
    vtkByteSwap::Swap4BE(&numGrids);
    }
  else
    {
    numGrids = 1;
    }

  if ( numGrids != this->NumberOfGrids )
    {
    vtkErrorMacro(<<"Data mismatch in vector function file!");
    return 1;
    }

  return 0;
}

//
// Various PLOT3D functions.....................
//
void vtkVisItPLOT3DReader::MapFunction(int fNumber,vtkPointData *outputPD)
{
  switch (fNumber)
    {
    case -1: //empty mapping
      break;

    case 100: //Density
      this->ComputeDensity(outputPD);
      break;

    case 110: //Pressure
      this->ComputePressure(outputPD);
      break;

    case 120: //Temperature
      this->ComputeTemperature(outputPD);
      break;

    case 130: //Enthalpy
      this->ComputeEnthalpy(outputPD);
      break;

    case 140: //Internal Energy
      this->ComputeInternalEnergy(outputPD);
      break;

    case 144: //Kinetic Energy
      this->ComputeKineticEnergy(outputPD);
      break;

    case 153: //Velocity Magnitude
      this->ComputeVelocityMagnitude(outputPD);
      break;

    case 163: //Stagnation energy
      this->ComputeStagnationEnergy(outputPD);
      break;

    case 170: //Entropy
      this->ComputeEntropy(outputPD);
      break;

    case 184: //Swirl
      this->ComputeSwirl(outputPD);
      break;

    case 200: //Velocity
      this->ComputeVelocity(outputPD);
      break;

    case 201: //Vorticity
      this->ComputeVorticity(outputPD);
      break;

    case 202: //Momentum
      this->ComputeMomentum(outputPD);
      break;

    case 210: //PressureGradient
      this->ComputePressureGradient(outputPD);
      break;

    default:
      vtkErrorMacro(<<"No function number " << fNumber);
    }
}

void vtkVisItPLOT3DReader::ComputeDensity(vtkPointData *outputPD)
{
  outputPD->AddArray(this->Density);
  outputPD->SetActiveScalars("Density");
  vtkDebugMacro(<<"Created density scalar");
}

void vtkVisItPLOT3DReader::ComputeTemperature(vtkPointData *outputPD)
{
  float *m, e, rr, u, v, w, v2, p, d, rrgas;
  int i;
  vtkFloatArray *temperature;

  //  Check that the required data is available
  //
  if ( this->Density == NULL || this->Momentum == NULL || 
  this->Energy == NULL )
    {
    vtkErrorMacro(<<"Cannot compute temperature");
    return;
    }

  temperature = vtkFloatArray::New();
  temperature->SetNumberOfTuples(this->NumberOfPoints);

  //  Compute the temperature
  //
  rrgas = 1.0 / this->R;
  for (i=0; i < this->NumberOfPoints; i++) 
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
    temperature->SetValue(i, p*rr*rrgas);
  }

  temperature->SetName("Temperature");
  outputPD->AddArray(temperature);
  outputPD->SetActiveScalars("Temperature");
  
  temperature->Delete();
  vtkDebugMacro(<<"Created temperature scalar");
}

void vtkVisItPLOT3DReader::ComputePressure(vtkPointData *outputPD)
{
  float *m, e, u, v, w, v2, p, d, rr;
  int i;
  vtkFloatArray *pressure;

  //  Check that the required data is available
  //
  if ( this->Density == NULL || this->Momentum == NULL || 
  this->Energy == NULL )
    {
    vtkErrorMacro(<<"Cannot compute pressure");
    return;
    }

  pressure = vtkFloatArray::New();
  pressure->SetNumberOfTuples(this->NumberOfPoints);

  //  Compute the pressure
  //
  for (i=0; i < this->NumberOfPoints; i++) 
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
    pressure->SetValue(i, p);
  }

  pressure->SetName("Pressure");
  outputPD->AddArray(pressure);
  outputPD->SetActiveScalars("Pressure");
  pressure->Delete();
  vtkDebugMacro(<<"Created pressure scalar");
}

void vtkVisItPLOT3DReader::ComputeEnthalpy(vtkPointData *outputPD)
{
  float *m, e, u, v, w, v2, d, rr;
  int i;
  vtkFloatArray *enthalpy;

  //  Check that the required data is available
  //
  if ( this->Density == NULL || this->Momentum == NULL || 
  this->Energy == NULL )
    {
    vtkErrorMacro(<<"Cannot compute enthalpy");
    return;
    }

  enthalpy = vtkFloatArray::New();
  enthalpy->SetNumberOfTuples(this->NumberOfPoints);

  //  Compute the enthalpy
  //
  for (i=0; i < this->NumberOfPoints; i++) 
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
    enthalpy->SetValue(i, this->Gamma*(e*rr - 0.5*v2));
  }
  enthalpy->SetName("Enthalpy");
  outputPD->AddArray(enthalpy);
  outputPD->SetActiveScalars("Enthalpy");
  enthalpy->Delete();
  vtkDebugMacro(<<"Created enthalpy scalar");
}

void vtkVisItPLOT3DReader::ComputeInternalEnergy(vtkPointData *outputPD)
{
  outputPD->AddArray(this->Energy);
  outputPD->SetActiveScalars("Energy");

  vtkDebugMacro(<<"Created energy scalar");
}

void vtkVisItPLOT3DReader::ComputeKineticEnergy(vtkPointData *outputPD)
{
  float *m, u, v, w, v2, d, rr;
  int i;
  vtkFloatArray *kineticEnergy;

  //  Check that the required data is available
  //
  if ( this->Density == NULL || this->Momentum == NULL )
    {
    vtkErrorMacro(<<"Cannot compute kinetic energy");
    return;
    }

  kineticEnergy = vtkFloatArray::New();
  kineticEnergy->SetNumberOfTuples(this->NumberOfPoints);

  //  Compute the kinetic energy
  //
  for (i=0; i < this->NumberOfPoints; i++) 
    {
    d = this->Density->GetComponent(i,0);
    d = (d != 0.0 ? d : 1.0);
    m = this->Momentum->GetTuple(i);
    rr = 1.0 / d;
    u = m[0] * rr;        
    v = m[1] * rr;        
    w = m[2] * rr;        
    v2 = u*u + v*v + w*w;
    kineticEnergy->SetValue(i, 0.5*v2);
  }
  kineticEnergy->SetName("Kinetic Energy");
  outputPD->AddArray(kineticEnergy);
  outputPD->SetActiveScalars("Kinetic Energy");
  kineticEnergy->Delete();
  vtkDebugMacro(<<"Created kinetic energy scalar");
}

void vtkVisItPLOT3DReader::ComputeVelocityMagnitude(vtkPointData *outputPD)
{
  float *m, u, v, w, v2, d, rr;
  int i;
  vtkFloatArray *velocityMag;

  //  Check that the required data is available
  //
  if ( this->Density == NULL || this->Momentum == NULL ||
  this->Energy == NULL )
    {
    vtkErrorMacro(<<"Cannot compute velocity magnitude");
    return;
    }

  velocityMag = vtkFloatArray::New();
  velocityMag->SetNumberOfTuples(this->NumberOfPoints);

  //  Compute the velocity magnitude
  //
  for (i=0; i < this->NumberOfPoints; i++) 
    {
    d = this->Density->GetComponent(i,0);
    d = (d != 0.0 ? d : 1.0);
    m = this->Momentum->GetTuple(i);
    rr = 1.0 / d;
    u = m[0] * rr;        
    v = m[1] * rr;        
    w = m[2] * rr;        
    v2 = u*u + v*v + w*w;
    velocityMag->SetValue(i, sqrt((double)v2));
  }
  velocityMag->SetName("Velocity Magnitude");
  outputPD->AddArray(velocityMag);
  outputPD->SetActiveScalars("Velocity Magnitude");
  velocityMag->Delete();
  vtkDebugMacro(<<"Created velocity magnitude scalar");
}

void vtkVisItPLOT3DReader::ComputeStagnationEnergy(vtkPointData *outputPD)
{
  outputPD->AddArray(this->Energy);
  outputPD->SetActiveScalars("Energy");
  
  vtkDebugMacro(<<"Created stagnation energy scalar");
}

void vtkVisItPLOT3DReader::ComputeEntropy(vtkPointData *outputPD)
{
  float *m, u, v, w, v2, d, rr, s, p, e;
  int i;
  vtkFloatArray *entropy;

  //  Check that the required data is available
  //
  if ( this->Density == NULL || this->Momentum == NULL ||
  this->Energy == NULL )
    {
    vtkErrorMacro(<<"Cannot compute entropy");
    return;
    }

  entropy = vtkFloatArray::New();
  entropy->SetNumberOfTuples(this->NumberOfPoints);

  //  Compute the entropy
  //
  for (i=0; i < this->NumberOfPoints; i++) 
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
    s = VTK_CV * log((p/VTK_PINF)/pow((double)d/VTK_RHOINF,(double)this->Gamma));
    entropy->SetValue(i,s);
  }
  entropy->SetName("Entropy");
  outputPD->AddArray(entropy);
  outputPD->SetActiveScalars("Entropy");
  entropy->Delete();
  vtkDebugMacro(<<"Created entropy scalar");
}

void vtkVisItPLOT3DReader::ComputeSwirl(vtkPointData *outputPD)
{
  vtkDataArray *currentVector;
  vtkDataArray *vorticity;
  float d, rr, *m, u, v, w, v2, *vort, s;
  int i;
  vtkFloatArray *swirl;

  //  Check that the required data is available
  //
  if ( this->Density == NULL || this->Momentum == NULL ||
  this->Energy == NULL )
    {
    vtkErrorMacro(<<"Cannot compute swirl");
    return;
    }

  swirl = vtkFloatArray::New();
  swirl->SetNumberOfTuples(this->NumberOfPoints);

  currentVector = outputPD->GetVectors();
  if (currentVector)
    {
    currentVector->Register(this);
    }

  this->ComputeVorticity(outputPD);
  vorticity = outputPD->GetVectors();
//
//  Compute the swirl
//
  for (i=0; i < this->NumberOfPoints; i++) 
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

    swirl->SetValue(i,s);
  }
  swirl->SetName("Swirl");
  outputPD->AddArray(swirl);
  outputPD->SetActiveScalars("Swirl");
  swirl->Delete();
  vtkDebugMacro(<<"Created swirl scalar");

  // reset current vector
  if (currentVector)
    {
    outputPD->SetVectors(currentVector);
    currentVector->UnRegister(this);
    }
}

// Vector functions
void vtkVisItPLOT3DReader::ComputeVelocity(vtkPointData *outputPD)
{
  float *m, v[3], d, rr;
  int i;
  vtkFloatArray *velocity;

  //  Check that the required data is available
  //
  if ( this->Density == NULL || this->Momentum == NULL ||
  this->Energy == NULL )
    {
    vtkErrorMacro(<<"Cannot compute velocity");
    return;
    }

  velocity = vtkFloatArray::New();
  velocity->SetNumberOfComponents(3);
  velocity->SetNumberOfTuples(this->NumberOfPoints);

  //  Compute the velocity
  //
  for (i=0; i < this->NumberOfPoints; i++) 
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
  outputPD->SetActiveVectors("Velocity");
  velocity->Delete();
  vtkDebugMacro(<<"Created velocity vector");
}

void vtkVisItPLOT3DReader::ComputeVorticity(vtkPointData *outputPD)
{
  vtkDataArray *velocity;
  vtkFloatArray *vorticity;
  int dims[3], ijsize;
  vtkPoints *points;
  int i, j, k, idx, idx2, ii;
  float vort[3], xp[3], xm[3], vp[3], vm[3], factor;
  float xxi, yxi, zxi, uxi, vxi, wxi;
  float xeta, yeta, zeta, ueta, veta, weta;
  float xzeta, yzeta, zzeta, uzeta, vzeta, wzeta;
  float aj, xix, xiy, xiz, etax, etay, etaz, zetax, zetay, zetaz;

  //  Check that the required data is available
  //
  if ( (points=this->GetOutput()->GetPoints()) == NULL || 
       this->Density == NULL || this->Momentum == NULL || 
       this->Energy == NULL )
    {
    vtkErrorMacro(<<"Cannot compute vorticity");
    return;
    }

  vorticity = vtkFloatArray::New();
  vorticity->SetNumberOfComponents(3);
  vorticity->SetNumberOfTuples(this->NumberOfPoints);

  this->ComputeVelocity(outputPD);
  velocity = outputPD->GetVectors();

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

void vtkVisItPLOT3DReader::ComputeMomentum(vtkPointData *outputPD)
{
  this->Momentum->SetName("Momentum");
  outputPD->SetVectors(this->Momentum);
  vtkDebugMacro(<<"Created momentum vector");
}

void vtkVisItPLOT3DReader::ComputePressureGradient(vtkPointData *outputPD)
{
  vtkDataArray *currentScalar;
  vtkDataArray *pressure;
  vtkFloatArray *gradient;
  int dims[3], ijsize;
  vtkPoints *points;
  int i, j, k, idx, idx2, ii;
  float g[3], xp[3], xm[3], pp, pm, factor;
  float xxi, yxi, zxi, pxi;
  float xeta, yeta, zeta, peta;
  float xzeta, yzeta, zzeta, pzeta;
  float aj, xix, xiy, xiz, etax, etay, etaz, zetax, zetay, zetaz;

  //  Check that the required data is available
  //
  if ( (points=this->GetOutput()->GetPoints()) == NULL || 
       this->Density == NULL || this->Momentum == NULL || 
       this->Energy == NULL )
    {
    vtkErrorMacro(<<"Cannot compute pressure gradient");
    return;
    }

  gradient = vtkFloatArray::New();
  gradient->SetNumberOfComponents(3);
  gradient->SetNumberOfTuples(this->NumberOfPoints);

  currentScalar = outputPD->GetScalars();
  if (currentScalar)
    {
    currentScalar->Register(this);
    }
  this->ComputePressure(outputPD);
  pressure = outputPD->GetScalars();

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
  gradient->SetName("Pressure Gradient");
  outputPD->AddArray(gradient);
  outputPD->SetActiveVectors("Pressure Gradient");
  gradient->Delete();
  vtkDebugMacro(<<"Created pressure gradient vector");

  // reset current scalar
  if (currentScalar)
    {
    outputPD->SetScalars(currentScalar);
    currentScalar->UnRegister(this);
    }
}

int vtkVisItPLOT3DReader::GetFileType(FILE *fp)
{
  char fourBytes[4];
  int type, i;

  //  Read a little from the file to figure what type it is.
  //
  fgets (fourBytes, 4, fp);
  for (i=0, type=VTK_ASCII; i<4 && type == VTK_ASCII; i++)
    {
    if ( ! isprint(fourBytes[i]) )
      {
      type = VTK_BINARY;
      }
    }

  // Reset file for reading
  //
  rewind (fp);
  return type;
}

void vtkVisItPLOT3DReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "XYZ File Name: " << 
    (this->XYZFileName ? this->XYZFileName : "(none)") << "\n";
  os << indent << "Q File Name: " <<
    (this->QFileName ? this->QFileName : "(none)") << "\n";
  os << indent << "Function File Name: " << 
    (this->FunctionFileName ? this->FunctionFileName : "(none)") << "\n";

  os << indent << "File Format: " << this->FileFormat << "\n";

  os << indent << "Grid Number: " << this->GridNumber << "\n";
  os << indent << "Scalar Function Number: " 
     << this->ScalarFunctionNumber << "\n";

  if ( this->VectorFunctionFileName )
    {
    os << indent << "Vector Function Filename: " <<
          this->VectorFunctionFileName << "\n";
    }
  else
    {
    os << indent << "Vector Function Filename: (none)\n";
    }

  os << indent << "Vector Function Number: " 
     << this->VectorFunctionNumber << "\n";
  os << indent << "Function Number: " 
     << this->FunctionFileFunctionNumber << "\n";

  os << indent << "Free Stream Mach Number: " << this->Fsmach << "\n";
  os << indent << "Alpha: " << this->Alpha << "\n";
  os << indent << "Reynolds Number " << this->Re << "\n";
  os << indent << "Total Integration Time: " << this->Time << "\n";

  os << indent << "R: " << this->R << "\n";
  os << indent << "Gamma: " << this->Gamma << "\n";
  os << indent << "UVinf: " << this->Uvinf << "\n";
  os << indent << "VVinf: " << this->Vvinf << "\n";
  os << indent << "WVinf: " << this->Wvinf << "\n";

  os << indent << "Number Of Grids: " << this->NumberOfGrids << "\n";
}

void 
vtkVisItPLOT3DReader::AddFunction(int functionNumber)
{
  this->FunctionList->InsertNextValue(functionNumber);
}

void 
vtkVisItPLOT3DReader::RemoveAllFunctions()
{
  this->FunctionList->Reset();
}


// NOW START AVT CODE

#include <avtPLOT3DFileFormat.h>

#include <vector>
#include <string>

#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkStructuredGrid.h>

#include <avtDatabaseMetaData.h>

#include <BadIndexException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidVariableException.h>


using std::vector;
using std::string;


// ****************************************************************************
//  Method: avtPLOT3DFileFormat constructor
//
//  Arguments:
//      fname    The name of the PLOT3D file.
//
//  Programmer:  Hank Childs
//  Creation:    May 3, 2002
//
//  Modifications:
//
//    Hank Childs, Mon Apr  7 18:37:59 PDT 2003
//    Do not read in the file in the constructor.
//
// ****************************************************************************

avtPLOT3DFileFormat::avtPLOT3DFileFormat(const char *fname)
    : avtSTMDFileFormat(&fname, 1)
{
    char *s_file = NULL;
    char *x_file = NULL;
    if (strstr(fname, ".x") != NULL)
    {
        char soln_file[1024];
        char *q = strstr(fname, ".x");
        strncpy(soln_file, fname, q-fname);
        strcpy(soln_file + (q-fname), ".q");
        AddFile(soln_file);
        x_file = filenames[0];
        s_file = filenames[1];
    }
    else if (strstr(fname, ".q") != NULL)
    {
        char points_file[1024];
        char *x = strstr(fname, ".q");
        strncpy(points_file, fname, x-fname);
        strcpy(points_file + (x-fname), ".x");
        AddFile(points_file);
        x_file = filenames[1];
        s_file = filenames[0];
    }
    else
    {
        //
        // We cannot identify either file as a points file or a solution file,
        // so give up.
        //
        EXCEPTION0(ImproperUseException);
    }
    
    reader = vtkVisItPLOT3DReader::New();
    reader->SetXYZFileName(x_file);
    reader->SetQFileName(s_file);
    reader->SetFileFormat(VTK_WHOLE_MULTI_GRID_WITH_IBLANKING);
}


// ****************************************************************************
//  Method: avtPLOT3DFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   May 3, 2002
//
// ****************************************************************************

avtPLOT3DFileFormat::~avtPLOT3DFileFormat()
{
    reader->Delete();
    reader = NULL;
}


// ****************************************************************************
//  Method: avtPLOT3DFileFormat::GetMesh
//
//  Purpose:
//      Returns the grid associated with a domain number.
//
//  Arguments:
//      dom     The domain number.
//      name    The mesh name.
//
//  Programmer: Hank Childs
//  Creation:   May 3, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 16:31:30 PST 2002    
//    Use NewInstance instead of MakeObject, to match vtk's new api.
//
// ****************************************************************************

vtkDataSet *
avtPLOT3DFileFormat::GetMesh(int dom, const char *name)
{
    if (dom < 0 || dom >= reader->GetNumberOfGrids())
    {
        EXCEPTION2(BadIndexException, dom, reader->GetNumberOfGrids());
    }

    if (strcmp(name, "mesh") != 0)
    {
        EXCEPTION1(InvalidVariableException, name);
    }

    reader->SetGridNumber(dom);
    reader->SetScalarFunctionNumber(-1);
    reader->SetVectorFunctionNumber(-1);
    reader->GetOutput()->Update();
    vtkDataSet *rv = (vtkDataSet *) reader->GetOutput()->NewInstance();
    rv->ShallowCopy(reader->GetOutput());

    return rv;
}


// ****************************************************************************
//  Method: avtPLOT3DFileFormat::GetVar
//
//  Purpose:
//      Returns the variable associated with a domain number.
//
//  Arguments:
//      dom       The domain number.
//      name      The variable name.
//
//  Programmer:   Hank Childs
//  Creation:     May 3, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 16:31:30 PST 2002    
//    Use NewInstance instead of MakeObject, to match vtk's new api.
//
// ****************************************************************************

vtkDataArray *
avtPLOT3DFileFormat::GetVar(int dom, const char *name)
{
    int var = -1;
    reader->SetVectorFunctionNumber(-1);
    if (strcmp(name, "density") == 0)
    {
        var = 100;
    }
    else if (strcmp(name, "pressure") == 0)
    {
        var = 110;
    }
    else if (strcmp(name, "temperature") == 0)
    {
        var = 120;
    }
    else if (strcmp(name, "enthalpy") == 0)
    {
        var = 130;
    }
    else if (strcmp(name, "internal_energy") == 0)
    {
        var = 140;
    }
    else if (strcmp(name, "kinetic_energy") == 0)
    {
        var = 144;
    }
    else if (strcmp(name, "velocity_magnitude") == 0)
    {
        var = 153;
    }
    else if (strcmp(name, "stagnation_energy") == 0)
    {
        var = 163;
    }
    else if (strcmp(name, "entropy") == 0)
    {
        var = 170;
    }
    else if (strcmp(name, "swirl") == 0)
    {
        var = 184;
    }

    if (var < 0)
    {
        EXCEPTION1(InvalidVariableException, name);
    }

    reader->SetScalarFunctionNumber(var);
    reader->SetGridNumber(dom);
    reader->GetOutput()->Update();

    vtkDataArray *dat = reader->GetOutput()->GetPointData()->GetScalars();
    if (dat == NULL)
    {
        debug1 << "Internal error -- variable should be point data!" << endl;
        EXCEPTION0(ImproperUseException);
    }

    vtkDataArray *rv = dat->NewInstance();
    rv->DeepCopy(dat);
    rv->SetName(name);

    return rv;
}


// ****************************************************************************
//  Method: avtPLOT3DFileFormat::GetVectorVar
//
//  Purpose:
//      Returns the vector variable associated with a domain number.
//
//  Arguments:
//      dom       The domain number.
//      name      The variable name.
//
//  Programmer:   Hank Childs
//  Creation:     May 6, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 16:31:30 PST 2002    
//    Use NewInstance instead of MakeObject, to match vtk's new api.
//
// ****************************************************************************

vtkDataArray *
avtPLOT3DFileFormat::GetVectorVar(int dom, const char *name)
{
    int var = -1;
    reader->SetScalarFunctionNumber(-1);
    if (strcmp(name, "velocity") == 0)
    {
        var = 200;
    }
    else if (strcmp(name, "vorticity") == 0)
    {
        var = 201;
    }
    else if (strcmp(name, "momentum") == 0)
    {
        var = 202;
    }
    else if (strcmp(name, "pressure_gradient") == 0)
    {
        var = 210;
    }

    if (var < 0)
    {
        EXCEPTION1(InvalidVariableException, name);
    }

    reader->SetVectorFunctionNumber(var);
    reader->SetGridNumber(dom);
    reader->GetOutput()->Update();

    vtkDataArray *dat = reader->GetOutput()->GetPointData()->GetVectors();
    if (dat == NULL)
    {
        debug1 << "Internal error -- variable should be point data!" << endl;
        EXCEPTION0(ImproperUseException);
    }

    vtkDataArray *rv = dat->NewInstance();
    rv->DeepCopy(dat);
    rv->SetName(name);

    return rv;
}


// ****************************************************************************
//  Method: avtPLOT3DFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Sets the database meta-data for this PLOT3D file.
//
//  Programmer: Hank Childs
//  Creation:   May 3, 2002
//
// ****************************************************************************

void
avtPLOT3DFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "mesh";
    mesh->meshType = AVT_CURVILINEAR_MESH;
    mesh->numBlocks = reader->GetNumberOfGrids();
    mesh->blockOrigin = 0;
    mesh->spatialDimension = 3;
    mesh->topologicalDimension = 3;
    mesh->hasSpatialExtents = false;
    md->Add(mesh);

    const int NUM_SCALARS = 10;
    char *scalar_names[NUM_SCALARS] = { "density", "pressure", "temperature", 
         "enthalpy", "internal_energy", "kinetic_energy", "velocity_magnitude",
         "stagnation_energy", "entropy", "swirl" };

    int i;
    for (i = 0 ; i < NUM_SCALARS ; i++)
    {
        avtScalarMetaData *sd1 = new avtScalarMetaData;
        sd1->name = scalar_names[i];
        sd1->meshName = "mesh";
        sd1->centering = AVT_NODECENT;
        sd1->hasDataExtents = false;
        md->Add(sd1);
    }

    const int NUM_VECTORS = 4;
    char *vector_names[4] = { "velocity", "vorticity", "momentum",
                               "pressure_gradient" };
    for (i = 0 ; i < NUM_VECTORS ; i++)
    {
        avtVectorMetaData *vd1 = new avtVectorMetaData;
        vd1->name = vector_names[i];
        vd1->meshName = "mesh";
        vd1->centering = AVT_NODECENT;
        vd1->hasDataExtents = false;
        vd1->varDim = 3;
        md->Add(vd1);
    }
}


