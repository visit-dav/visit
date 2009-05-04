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
#include "vtkStructuredGridSource.h"
#include <visit-config.h>

class vtkIntArray;
class vtkFloatArray;
class vtkPointData;
class vtkPoints;
class vtkStructuredGrid;



#define SINGLE_GRID 0
#define MULTI_GRID 1

#define BINARY 0
#define ASCII 1

#define NATIVE_ENDIAN 0
#define VTK_BIG_ENDIAN 1
#define VTK_LITTLE_ENDIAN 2

#define STRUCTURED 0
#define UNSTRUCTURED 1

#define NO_IBLANKING 0
#define IBLANKING 1

#define THREE 0
#define TWO 1

#define SINGLE_PRECISION 0
#define DOUBLE_PRECISION 1

#define UNCOMPRESSED_ASCII 0
#define COMPRESSED_ASCII 1

#define C_BINARY 0
#define FORTRAN_BINARY 1


class vtkVisItPLOT3DReader : public vtkStructuredGridSource 
{
public:
  static vtkVisItPLOT3DReader *New();
  vtkTypeRevisionMacro(vtkVisItPLOT3DReader,vtkStructuredGridSource);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify the PLOT3D file format to use
  vtkSetClampMacro(Endianness,int,0,2);
  vtkGetMacro(Endianness,int);

  //Description:
  // Whether or not the file is single or multi-grid file. 
  vtkSetClampMacro(GridType,int,0,1);
  vtkGetMacro(GridType,int);  
  
  // Description:
  // Whether or not the file is ASCII or binary.
  vtkSetClampMacro(FileType,int,0,1);
  vtkGetMacro(FileType,int);

  // Description:
  // Whetehr or not the file is structured or unstructured.
  vtkSetClampMacro(FileStructure,int,0,1);
  vtkGetMacro(FileStructure,int);
     
  // Description:
  // Whether or not the file is single- or double-precision.
  vtkSetClampMacro(Precision,int,0,1);
  vtkGetMacro(Precision,int);


  // Description:
  // Whether or not the PLOT3D file is in C Binary or Fortran Binary.
  vtkSetClampMacro(BinaryType,int,0,1);
  vtkGetMacro(BinaryType,int);

  // Description:
  // Whether or not a PLOT3D ASCII file is compressed.  
  vtkSetClampMacro(Compression,int,0,1);
  vtkGetMacro(Compression,int);
  
  // Description:
  // Whether or not the data in file is 2D or 3D
  vtkSetClampMacro(Dimension,int,0,1);
  vtkGetMacro(Dimension,int);  

  // Description:
  // Whether or not the file is Iblanked.   
  vtkSetClampMacro(Iblanking,int,0,1);
  vtkGetMacro(Iblanking,int);

  // Specify the grid index currently looking at.
  vtkSetMacro(GridNumber,int);
  vtkGetMacro(GridNumber,int);

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
  // Set/Get the PLOT3D function FileName.
  vtkSetStringMacro(VP3DFileName);
  vtkGetStringMacro(VP3DFileName);
  
  // Description:
  // Set/Get the PLOT3D vector FileName.
  vtkSetStringMacro(VectorFunctionFileName);
  vtkGetStringMacro(VectorFunctionFileName);

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
  vtkSetMacro(NumberOfGrids, int);

  
 
  //  Description:
  //  Read the Visit PLOT3D file and collect PLOT3D properties.
  void CollectInfo(char*, char*);
  int getInfoLine(char * line, int size, FILE*fp); 

  // Description:
  // Read the number of grids provided in PLOT3D file.  
  int FindNumberOfGrids(FILE *);

  //  Read data from file regardless of file type.
  int ReadNumbers(FILE *fp, unsigned int numbersToRead, float * output);
  int ReadNumbers(FILE *fp, unsigned int numbersToRead, int * output);
protected:
  vtkVisItPLOT3DReader();
  ~vtkVisItPLOT3DReader();

  void ExecuteInformation();
  void Execute();
//  int GetFileType(FILE *fp);

  //plot3d FileNames
  char *XYZFileName;
  char *QFileName;
  char *FunctionFileName;
  char *VectorFunctionFileName;

  char *VP3DFileName;
  
  //flags describing data to be read
  int GridNumber; //for multi-grid files, the one we're interested in
  int ScalarFunctionNumber;
  int VectorFunctionNumber;
  int FunctionFileFunctionNumber;
  void MapFunction(int fNumber,vtkPointData *outputPD);

  int GridType;
  int FileType;
  int Endianness;
  int FileStructure;
  int Iblanking;
  int Dimension;
  int Precision;
  int Compression;
  int BinaryType;
  

  // storage used to cache grid offsets multi-grid files
  int * GridSizes;
  int * GridOffsets;
  int * SolutionOffsets;
  int * GridDimensions;  

  
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

  //  Compute the offsets before performing a read.
  int ComputeGridOffset(FILE *fp);
  int ComputeSolutionOffset(FILE *fp);  

  //  Reader functions 
  int ReadGrid(FILE *fp,vtkStructuredGrid *output);
  int ReadGridDimensions(FILE *fp, vtkStructuredGrid *output);
  int ReadSolution(FILE *fp, vtkStructuredGrid *output);
  int ReadFunctionFile(FILE *fp, vtkStructuredGrid *output);
  int ReadVectorFunctionFile(FILE *fp, vtkStructuredGrid *output);

  
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
  //  set up for the default PLOT3D file type
  this->GridType = SINGLE_GRID;
  this->FileType = BINARY;
  this->BinaryType = C_BINARY;
  this->Endianness = NATIVE_ENDIAN;
  this->Iblanking = NO_IBLANKING;
  this->Precision = SINGLE_PRECISION;
  this->FileStructure = STRUCTURED;
  this->Dimension = THREE;
  this->Compression = UNCOMPRESSED_ASCII; 

  this->XYZFileName = NULL;
  this->QFileName = NULL;
  this->VP3DFileName = NULL;
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

  this->GridSizes = NULL;
  this->GridDimensions = NULL;
  this->GridOffsets = NULL;
  this->SolutionOffsets = NULL;
} 

vtkVisItPLOT3DReader::~vtkVisItPLOT3DReader()
{
  if (this->XYZFileName)
    {
    delete [] this->XYZFileName;
    }
  if (this->QFileName)
    {
    delete [] this->QFileName;
    }
  if (this->VP3DFileName)
    {
      delete [] this->VP3DFileName;
    } 
  if (this->FunctionFileName)
    {
    delete [] this->FunctionFileName;
    }
  if (this->VectorFunctionFileName)
    {
    delete [] this->VectorFunctionFileName;
    }
  this->FunctionList->Delete();
 
  if (this->GridSizes)
  {
    delete [] this->GridSizes;
  }
  if (this->GridDimensions)
  {
    delete [] this->GridDimensions;
  }
  if (this->GridOffsets)
  {
    delete [] GridOffsets;
  }
  if (this->SolutionOffsets)
  {
    delete [] SolutionOffsets;
  }
}




void vtkVisItPLOT3DReader::RemoveFunction(int fnum)
{
  for (int i=0; i < this->FunctionList->GetNumberOfTuples(); i++)
  {
    if (this->FunctionList->GetValue(i) == fnum)
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
  if (this->XYZFileName == NULL)
    {
    vtkErrorMacro(<< "Must specify geometry file");
    return;
    }
  if ((xyzFp = fopen(this->XYZFileName, "r")) == NULL)
    {
    vtkErrorMacro(<< "File: " << this->XYZFileName << " not found");
    return;
    }

  error = this->ReadGridDimensions(xyzFp,output);
  fclose (xyzFp);
  
  if (error)
    {
    vtkErrorMacro(<<"Error reading XYZ file");
    return;
    }
}


bool
MatchesSubstring(const char *c1, const char *c2)
{
    return (strncmp(c1, c2, strlen(c2)) == 0);
}

// ****************************************************************************
//  Method: vtkVisItPLOT3DReader::CollectInfo
//
//  Arguments;
//      x_file:  grid file name
//      x_file:  solution file name 
//
//  Programmer:  Abel Gezahegne 
//  Creation:    Sept. 9, 2004
//
//  Modifications:
//
//    Hank Childs, Tue Jan 18 13:19:28 PST 2005
//    Added support for extended pathnames.  Also beefed up substring matching.
//
//    Jeremy Meredith, Tue Feb 22 13:27:06 PST 2005
//    Made lineSize a const.  Compilers other than g++ (like xlC) choke on
//    automatic array variable allocation of sizes that are non-const, even
//    if they are provably known at compile time.
//
//    Hank Childs, Thu Jun  8 14:04:15 PDT 2006
//    Change BIG_ENDIAN to VTK_BIG_ENDIAN because of namespace conflict with
//    endian.h on SUSE.
//
// ****************************************************************************

void vtkVisItPLOT3DReader::CollectInfo(char *x_file, char *s_file)
{
  const int lineSize = 1024;
  int fileFound = 1;
  char infoLine [lineSize];
  FILE * vp3dFp;
  if (this->VP3DFileName == NULL)
  {
    fileFound = 0;
  }

  if ((vp3dFp = fopen(this->VP3DFileName,"r")) == NULL)
  {
    fileFound = 0;
  }


  //  Note the order of these checks is important because we are using
  //  strstr.  ie  check for BINARY must not come before C_BINARY
  if (fileFound)
  {
    //
    // We may need the path of the vp3d file later, so get it now.
    //
    const char *fap = this->VP3DFileName; // fap = file-and-path
    int len = strlen(fap);
    int lastSlash = -1;
    for (int i = len-1; i >= 0; i--)
    {
        if (fap[i] == VISIT_SLASH_CHAR)
        {
            lastSlash = i;
            break;
        }
    }
    char path[1024] = { '\0' };
    if (lastSlash != -1)
    {
        strcpy(path, fap);
        path[lastSlash] = '\0';
    }

    while (getInfoLine(infoLine,lineSize,vp3dFp))
    {
      if (MatchesSubstring(infoLine,"SINGLE_GRID"))
      {
        GridType = SINGLE_GRID;
      }
      else if (MatchesSubstring(infoLine,"MULTI_GRID"))
      {
        GridType = MULTI_GRID;
      }
      else if (MatchesSubstring(infoLine,"NATIVE_ENDIAN"))
      {
        Endianness = NATIVE_ENDIAN;
      }
      else if (MatchesSubstring(infoLine,"BIG_ENDIAN"))
      {  
        Endianness = VTK_BIG_ENDIAN;
      }
      else if (MatchesSubstring(infoLine,"LITTLE_ENDIAN"))
      {
        Endianness = VTK_LITTLE_ENDIAN;
      }
      else if (MatchesSubstring(infoLine,"STRUCTURED"))
      {
        FileStructure = STRUCTURED;
      }
      else if (MatchesSubstring(infoLine,"UNSTRUCTURED"))
      {
        FileStructure = UNSTRUCTURED;
      }
      else if (MatchesSubstring(infoLine,"NO_IBLANKING"))
      {
        Iblanking = NO_IBLANKING;
      }
      else if (MatchesSubstring(infoLine,"IBLANKING"))
      {
        Iblanking = IBLANKING;
      }
      else if (MatchesSubstring(infoLine,"3D"))
      {
        Dimension = THREE;
      }
      else if (MatchesSubstring(infoLine,"2D"))
      {
        Dimension = TWO;
      }
      else if (MatchesSubstring(infoLine,"SINGLE_PRECISION"))
      {
        Precision = SINGLE_PRECISION;
      }
      else if (MatchesSubstring(infoLine,"DOUBLE_PRECISION"))
      {
        Precision = DOUBLE_PRECISION;
      }
      else if (MatchesSubstring(infoLine,"COMPRESSED_ASCII"))
      {
        Compression = COMPRESSED_ASCII;
      }
      else if (MatchesSubstring(infoLine,"UNCOMPRESSED_ASCII"))
      {
        Compression = UNCOMPRESSED_ASCII;
      }
      else if (MatchesSubstring(infoLine,"C_BINARY"))
      {
        BinaryType = C_BINARY;
      }
      else if (MatchesSubstring(infoLine,"FORTRAN_BINARY"))
      {
        BinaryType = FORTRAN_BINARY;
      }
      else if (MatchesSubstring(infoLine,"BINARY"))
      {
        FileType = BINARY;
      }
      else if (MatchesSubstring(infoLine,"ASCII"))
      {
        FileType = ASCII;
      }
      else if (MatchesSubstring(infoLine,"GRID"))
      {
        char tmp[1024];
        sscanf(infoLine + strlen("GRID"),"%s",tmp);
        if (tmp[0] == '/')
            strcpy(x_file, tmp);
        else
            sprintf(x_file, "%s/%s", path, tmp);
      }
      else if (MatchesSubstring(infoLine,"SOLUTION"))
      { 
        char tmp[1024];
        sscanf(infoLine + strlen("SOLUTION"),"%s",tmp);
        if (tmp[0] == '/')
            strcpy(s_file, tmp);
        else
            sprintf(s_file, "%s/%s", path, tmp);
      }
    }
    fclose(vp3dFp);
  }
}



// ****************************************************************************
//  Method: vtkVisItPLOT3DReader::getInfoLine
//
//  Arguments:
//      line     storage for a line read from a vp3d file.
//      size     number of characters to read for one line.
//      fp       file stream for the vp3d file
//
//  Programmer:  Abel Gezahegne
//  Creation:    Sept 9, 2004
//
// ****************************************************************************

int vtkVisItPLOT3DReader::getInfoLine(char * line, int size, FILE*fp)
{
  //  skip all comments and empty lines in the file.  
  do{
    fscanf(fp,"%*[^0-9a-zA-Z#]s");
    line = fgets(line,size,fp);
  }while(line && line[0] =='#');

  
  if (line) 
    return 1;
  else 
    return 0;
}



void vtkVisItPLOT3DReader::Execute()
{
  FILE *xyzFp, *QFp, *funcFp;
  int error = 0;
  vtkStructuredGrid *output = this->GetOutput();
  vtkPointData *outputPD = output->GetPointData();
 
  //Initialize output and read geometry
  //

  if (this->XYZFileName == NULL)
    {
    output->Initialize();
    vtkErrorMacro(<< "Must specify geometry file");
    return;
    }
  if ((xyzFp = fopen(this->XYZFileName, "r")) == NULL)
    {
    output->Initialize();
    vtkErrorMacro(<< "File: " << this->XYZFileName << " not found");
    return;
    }
  if (this->FileType == ASCII)
  {
    error = this->ReadGrid(xyzFp,output);
    fclose(xyzFp);
  }
  else
  {
    fclose(xyzFp);
    xyzFp = fopen(this->XYZFileName, "rb");
    error = this->ReadGrid(xyzFp,output);
    fclose(xyzFp);
  }
  
  if (error)
  {
    output->Initialize();
    vtkErrorMacro(<<"Error reading XYZ file");
    return;
  }

  // Read solution file (if available and requested)
  //
  if (this->QFileName && 
  ((this->FunctionFileName == NULL && this->ScalarFunctionNumber >= 0) ||
  (this->VectorFunctionFileName == NULL && this->VectorFunctionNumber >= 0)))
    {

    if ((QFp = fopen(this->QFileName, "r")) == NULL)
      {
      output->Initialize();
      vtkErrorMacro(<< "File: " << this->QFileName << " not found");
      return;
      }

    if (this->FileType == ASCII) 
    {
      error = this->ReadSolution(QFp,output);
      fclose(QFp);
    }
    else
    {
      fclose(QFp);
      QFp = fopen(this->QFileName, "rb");
      error = this->ReadSolution(QFp,output);
      fclose(QFp);
    }
    
    if (error)
    {
      output->Initialize();
      vtkErrorMacro(<<"Error reading solution file");
      return;
    }

    // Read solutions as general point attribute data
    
    if (this->FunctionList->GetNumberOfTuples() > 0)
      {
      int fnum;
      for (int i=0; i < this->FunctionList->GetNumberOfTuples(); i++)
        {
        if ((fnum=this->FunctionList->GetValue(i)) >= 0)
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
  if (this->FunctionFileName != NULL)
    {
    if ((funcFp = fopen(this->FunctionFileName, "r")) == NULL)
      {
      output->Initialize();
      vtkErrorMacro(<< "File: " << this->FunctionFileName << " not found");
      return;
      }

    if (this->FileType == ASCII) 
    {
      error = this->ReadFunctionFile(funcFp,output);
      fclose(funcFp);
    }
    else
    {
      fclose(funcFp);
      funcFp = fopen(this->FunctionFileName, "rb");
      error = this->ReadFunctionFile(funcFp,output);
      fclose(funcFp);
    }
    
    if (error)
      {
      vtkErrorMacro(<<"Error reading function file");
      return;
      }
    }

  // Read vector function file (if available)
  //
  if (this->VectorFunctionFileName != NULL)
    {
    if ((funcFp = fopen(this->VectorFunctionFileName, "r")) == NULL)
      {
      output->Initialize();
      vtkErrorMacro(<< "File: "<<this->VectorFunctionFileName <<" not found");
      return;
      }

    if (this->FileType == ASCII)
    {
      error = this->ReadVectorFunctionFile(funcFp,output);
      fclose(funcFp);
    }
    else
    {
      fclose(funcFp);
      funcFp = fopen(this->VectorFunctionFileName, "rb");
      error = this->ReadVectorFunctionFile(funcFp,output);
      fclose(funcFp);
    }
    
    if (error)
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

  if (this->Density) 
    {
    this->Density->UnRegister(this);
    this->Density = NULL;
    }

  if (this->Energy) 
    {
    this->Energy->UnRegister(this);
    this->Energy = NULL;
    }

  if (this->Momentum) 
    {
    this->Momentum->UnRegister(this);
    this->Momentum = NULL;
    }
}



int vtkVisItPLOT3DReader::ReadGrid(FILE *fp,vtkStructuredGrid *output)
{
  vtkPoints *newPts;
  int i, gridFound, offset, gridSize;
  int error = 0;
  float x[3];

  rewind(fp);
  this->NumberOfGrids = this->FindNumberOfGrids(fp);

  // If First time reading grids then set up the initial offset and the
  // grid size and dimensions for later reference. 
  if (GridOffsets == NULL)
  {
    GridOffsets = new int[this->NumberOfGrids];
    GridSizes = new int[this->NumberOfGrids];
    GridDimensions = new int[3*this->NumberOfGrids];

    for (int i = 0; i < this->NumberOfGrids; i++)
        GridOffsets[i] = -1;

    if (this->Dimension == THREE)
      error = ReadNumbers(fp,3*this->NumberOfGrids,GridDimensions);
    else 
      error = ReadNumbers(fp,2*this->NumberOfGrids,GridDimensions);

    if (error)
    {
      delete [] GridOffsets;
      delete [] GridSizes;
      delete [] GridDimensions;
      return 1;
    }

    GridOffsets[0] = ftell(fp);
  }

  
  for (gridFound=0, offset=0, i=0; i<this->NumberOfGrids; i++)
  {
    if (this->Dimension == THREE)
      gridSize =GridDimensions[3*i]*GridDimensions[1+3*i]*GridDimensions[2+3*i];
    else
      gridSize = GridDimensions[2*i]*GridDimensions[1+2*i];

    GridSizes[i] = gridSize;
  }

  if (0 <= this->GridNumber && this->GridNumber < this->NumberOfGrids)
  {
    gridFound =  1;
    this->NumberOfPoints = GridSizes[this->GridNumber];
    if (this->Dimension == THREE) 
    {
      output->SetDimensions( GridDimensions[3*this->GridNumber],
                              GridDimensions[1+3*this->GridNumber],
                              GridDimensions[2+3*this->GridNumber]);
    }
    else
    {
      output->SetDimensions( GridDimensions[2*this->GridNumber],
                              GridDimensions[1+2*this->GridNumber], 1);
    }
  }

  if (! gridFound)
  {
    vtkErrorMacro (<<"Specified grid not found!");
    return 1;
  }


//allocate temporary storage to read into + points
  this->TempStorage = new float[3*this->NumberOfPoints];
  newPts = vtkPoints::New();
  newPts->SetNumberOfPoints(this->NumberOfPoints);


  offset = ComputeGridOffset(fp);
  fseek (fp, (long)(offset),SEEK_SET);


  if (this->Dimension == THREE) 
    error = ReadNumbers(fp,3*this->NumberOfPoints,this->TempStorage);
  else
    error = ReadNumbers(fp,2*this->NumberOfPoints,this->TempStorage);

  if (error)
  {
    newPts->Delete();
    delete[] this->TempStorage;
    return 1;
  }
  else 
  {
    //  The Iblank values are read and discarded.  This reader does not
    //  yet support true iblanking.

    if (this->Iblanking == IBLANKING)
    {
      int * iblankValues = new int [this->NumberOfPoints];
      error = ReadNumbers(fp,this->NumberOfPoints,iblankValues);
      if (error)
      {
        newPts->Delete();
        delete[] this->TempStorage;
        delete[] iblankValues;
        return 1;
      }
    }

    //  After read mark the current position as offset for reading next grid.  
    if (this->GridNumber +1 < this->NumberOfGrids)
    {
      GridOffsets[this->GridNumber + 1] = ftell(fp);
    }


    for (i=0; i < this->NumberOfPoints; i++)
    {
      x[0] = this->TempStorage[i];
      x[1] = this->TempStorage[i + this->NumberOfPoints];
      if (Dimension == THREE)
      {
        x[2] = this->TempStorage[i + 2*this->NumberOfPoints];
      }
      else
      {
        x[2] = 0;
      }
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
int vtkVisItPLOT3DReader::ReadGridDimensions(FILE *fp,
                                              vtkStructuredGrid *output)
{
  int *dim= new int[3*this->NumberOfGrids ];
  int gridSize;

  rewind(fp);
  if (this->GridType == MULTI_GRID)
  {
    // Skip the first number (number of grids) in file. 
    int numGrids;
    ReadNumbers(fp,1,&numGrids);
  }

  if (this->Dimension == THREE)
  {
    if (ReadNumbers(fp,3*this->NumberOfGrids,dim))
    {
      return 1;
    }
    gridSize = dim[3 * this->GridNumber] * dim[1 + 3 * this->GridNumber] 
                                         * dim[2 + 3 * this->GridNumber];

    output->SetWholeExtent( 0, dim[3 * this->GridNumber] -1,
                             0, dim[1 + 3 * this->GridNumber] -1,
                             0, dim[2 + 3 * this->GridNumber] -1);
  }
  else
  {
    if (ReadNumbers(fp,2*this->NumberOfGrids,dim))
    {
      return 1;
    }
    gridSize = dim[ 2 * this->GridNumber] * dim[ 1 + 2 * GridNumber];
    output->SetWholeExtent(0, dim[2 * this->GridNumber] -1,
                            0, dim[1 + 2 * this->GridNumber] -1, 0, 1);
  }

  this->NumberOfPoints = gridSize;
  return 0;
}



// ****************************************************************************
//  Method: vtkVisItPLOT3DReader::FindNumberOfGrids
//
//  Arguments;
//      fp:      grid file stream
//
//  Programmer:  Abel Gezahegne
//  Creation:    Sept. 12, 2004
//
// ****************************************************************************

int vtkVisItPLOT3DReader::FindNumberOfGrids(FILE *fp)
{
  int numGrids;
  if (this->GridType == MULTI_GRID || this->Compression == COMPRESSED_ASCII) 
  {
    int error = ReadNumbers(fp,1,&numGrids);
    if (error)
    {
      return 1;
    }
  }
  else
    numGrids = 1;

  return numGrids;
}


int vtkVisItPLOT3DReader::ReadSolution(FILE *fp,vtkStructuredGrid *output)
{
  vtkFloatArray *newDensity, *newEnergy;
  vtkFloatArray *newMomentum;
  int i, gridFound, offset;
  float m[3], params[4];
  int numGrids, numPts = 0;

  int error = 0;

  numGrids = this->FindNumberOfGrids(fp);

  if (numGrids != this->NumberOfGrids)
  {
    vtkErrorMacro(<<"Data mismatch in solution file!");
    return 1;
  }


  // If first time reading soluton file then find the initial offset 
  // as well as find the gridsize(s) and Dmensions.
  //
  if (SolutionOffsets == NULL)
  {
    SolutionOffsets = new int[this->NumberOfGrids];
    GridSizes = new int [this->NumberOfGrids];
    GridDimensions = new int [3*this->NumberOfGrids];

    for (int i = 0; i < this->NumberOfGrids; i++) 
    {
      SolutionOffsets[i] = -1;
    }

    if (this->Dimension == THREE)
    {
      error = ReadNumbers(fp,3*this->NumberOfGrids, GridDimensions);
    }
    else if (this->Dimension == TWO)
    {
      error = ReadNumbers(fp,2*this->NumberOfGrids, GridDimensions);
    }

    if (error)
    {
      delete[] SolutionOffsets;
      delete[] GridSizes;
      delete[] GridDimensions;
      return 1;
    }
    SolutionOffsets[0] = ftell(fp);
  }


  
  for (i =0; i < this->NumberOfGrids; i++)
  {
    if (this->Dimension == THREE)
    {
      GridSizes[i] = GridDimensions[3 * i] * GridDimensions[1 + 3 * i]
                                           * GridDimensions[2 + 3 * i];
    } 
    else if (this->Dimension == TWO)
    {  
      GridSizes[i] = GridDimensions[2 * i] * GridDimensions[1 + 2 * i];
    }
  }
  
  if (0 <= this->GridNumber && this->GridNumber < this->NumberOfGrids) 
  {
    gridFound = 1;
    numPts = GridSizes[this->GridNumber];
    
    if (this->Dimension == THREE)
    {
      output->SetDimensions(GridDimensions[3 * GridNumber], 
                             GridDimensions[1 + 3 * GridNumber], 
                             GridDimensions[2 + 3 * GridNumber]);
    }
    else if (this->Dimension == TWO)
    {
      output->SetDimensions(GridDimensions[2 * GridNumber], 
                             GridDimensions[1 + 2 * GridNumber], 1);
    }
  }

  if (! gridFound)
  {
    vtkErrorMacro (<<"Specified grid not found!");
    return 1;
  }

  if (numPts != this->NumberOfPoints)
  {
    vtkErrorMacro (<<"Data mismatch in solution file!");
    delete [] this->TempStorage;
    return 1;
  }


  //seek to correct spot and read solution
  offset = ComputeSolutionOffset(fp);
  fseek (fp, (long)(offset), SEEK_SET);

  //read solution parameters

  error = ReadNumbers(fp,4,params);
  if (error)
  {
    return 1;
  }
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


  error = ReadNumbers(fp,numPts,this->TempStorage);
  if (error) 
  {
    newDensity->Delete();
    newMomentum->Delete();
    newEnergy->Delete();
    delete [] this->TempStorage;
    return 1;
  }
  else //successful read
  {
    for (i=0; i < this->NumberOfPoints; i++) 
    {
      newDensity->SetValue(i,this->TempStorage[i]);
    }
  }

  if (this->Dimension == THREE)
  {
    error = ReadNumbers(fp,3*this->NumberOfPoints,this->TempStorage);
  }
  else
  {
    error = ReadNumbers(fp,2*this->NumberOfPoints,this->TempStorage);
  }
  
  if (error) 
  {
    newDensity->Delete();
    newMomentum->Delete();
    newEnergy->Delete();
    delete [] this->TempStorage;
    return 1;
  }
  else //successful read, load coordinates into vector object
  {
    for (i=0; i < this->NumberOfPoints; i++)
    {
      m[0] = this->TempStorage[i];
      m[1] = this->TempStorage[this->NumberOfPoints+i];
      if (this->Dimension == THREE)
      {
        m[2] = this->TempStorage[2*this->NumberOfPoints+i];
      }
      else
      {
        m[2] = 0;
      }      
      newMomentum->SetTuple(i,m);
    }
  }

  error = ReadNumbers(fp,numPts,this->TempStorage);
  if (error) 
  {
    newDensity->Delete();
    newMomentum->Delete();
    newEnergy->Delete();
    delete [] this->TempStorage;
    return 1;
  }
  else //successful read
  {
    if (this->GridNumber + 1 < this->NumberOfGrids)
    {
      SolutionOffsets[this->GridNumber + 1] = ftell(fp);
    }
    for (i=0; i < this->NumberOfPoints; i++) 
    {
      newEnergy->SetValue(i,this->TempStorage[i]);
    }
  }

  // Register data for use by computation functions

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


int vtkVisItPLOT3DReader::ReadFunctionFile(FILE *fp,vtkStructuredGrid *output)
{
  int numGrids;
  output = output;     // what is this for?
  
  numGrids = this->FindNumberOfGrids(fp);
  
  if (numGrids != this->NumberOfGrids)
    {
    vtkErrorMacro(<<"Data mismatch in function file!");
    return 1;
    }

  return 0;
}


int vtkVisItPLOT3DReader::ReadVectorFunctionFile(FILE *fp,
                                                  vtkStructuredGrid *output)
{
  int numGrids= this->FindNumberOfGrids(fp);

  output = output;
  if (numGrids != this->NumberOfGrids)
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
  double *m;
  float e, rr, u, v, w, v2, p, d, rrgas;
  int i;
  vtkFloatArray *temperature;

  //  Check that the required data is available
  //
  if (this->Density == NULL || this->Momentum == NULL || 
  this->Energy == NULL)
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
  double *m;
  float e, u, v, w, v2, p, d, rr;
  int i;
  vtkFloatArray *pressure;

  //  Check that the required data is available
  //
  if (this->Density == NULL || this->Momentum == NULL || 
  this->Energy == NULL)
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
  double *m;
  float e, u, v, w, v2, d, rr;
  int i;
  vtkFloatArray *enthalpy;

  //  Check that the required data is available
  //
  if (this->Density == NULL || this->Momentum == NULL || 
  this->Energy == NULL)
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
  double *m;
  float u, v, w, v2, d, rr;
  int i;
  vtkFloatArray *kineticEnergy;

  //  Check that the required data is available
  //
  if (this->Density == NULL || this->Momentum == NULL)
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
  double *m;
  float u, v, w, v2, d, rr;
  int i;
  vtkFloatArray *velocityMag;

  //  Check that the required data is available
  //
  if (this->Density == NULL || this->Momentum == NULL ||
  this->Energy == NULL)
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
  double *m;
  float u, v, w, v2, d, rr, s, p, e;
  int i;
  vtkFloatArray *entropy;

  //  Check that the required data is available
  //
  if (this->Density == NULL || this->Momentum == NULL ||
  this->Energy == NULL)
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
    s =VTK_CV * log((p/VTK_PINF)/pow((double)d/VTK_RHOINF,(double)this->Gamma));
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
  float d, rr, u, v, w, v2, s;
  double *vort, *m;
  int i;
  vtkFloatArray *swirl;

  //  Check that the required data is available
  //
  if (this->Density == NULL || this->Momentum == NULL ||
  this->Energy == NULL)
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
    if (v2 != 0.0) 
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
  double *m;
  float v[3], d, rr;
  int i;
  vtkFloatArray *velocity;

  //  Check that the required data is available
  //
  if (this->Density == NULL || this->Momentum == NULL ||
  this->Energy == NULL)
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
  double xp[3], xm[3], vp[3], vm[3];
  float vort[3], factor;
  float xxi, yxi, zxi, uxi, vxi, wxi;
  float xeta, yeta, zeta, ueta, veta, weta;
  float xzeta, yzeta, zzeta, uzeta, vzeta, wzeta;
  float aj, xix, xiy, xiz, etax, etay, etaz, zetax, zetay, zetaz;

  //  Check that the required data is available
  //
  if ((points=this->GetOutput()->GetPoints()) == NULL || 
       this->Density == NULL || this->Momentum == NULL || 
       this->Energy == NULL)
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
        if (dims[0] == 1) // 2D in this direction
          {
          factor = 1.0;
          for (ii=0; ii<3; ii++)
            {
            vp[ii] = vm[ii] = xp[ii] = xm[ii] = 0.0;
            }
          xp[0] = 1.0;
          }
        else if (i == 0) 
          {
          factor = 1.0;
          idx = (i+1) + j*dims[0] + k*ijsize;
          idx2 = i + j*dims[0] + k*ijsize;
          points->GetPoint(idx,xp);
          points->GetPoint(idx2,xm);
          velocity->GetTuple(idx,vp);
          velocity->GetTuple(idx2,vm);
          } 
        else if (i == (dims[0]-1)) 
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
        if (dims[1] == 1) // 2D in this direction
          {
          factor = 1.0;
          for (ii=0; ii<3; ii++)
            {
            vp[ii] = vm[ii] = xp[ii] = xm[ii] = 0.0;
            }
          xp[1] = 1.0;
          }
        else if (j == 0) 
          {
          factor = 1.0;
          idx = i + (j+1)*dims[0] + k*ijsize;
          idx2 = i + j*dims[0] + k*ijsize;
          points->GetPoint(idx,xp);
          points->GetPoint(idx2,xm);
          velocity->GetTuple(idx,vp);
          velocity->GetTuple(idx2,vm);
          } 
        else if (j == (dims[1]-1)) 
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
        if (dims[2] == 1) // 2D in this direction
          {
          factor = 1.0;
          for (ii=0; ii<3; ii++)
            {
            vp[ii] = vm[ii] = xp[ii] = xm[ii] = 0.0;
            }
          xp[2] = 1.0;
          }
        else if (k == 0) 
          {
          factor = 1.0;
          idx = i + j*dims[0] + (k+1)*ijsize;
          idx2 = i + j*dims[0] + k*ijsize;
          points->GetPoint(idx,xp);
          points->GetPoint(idx2,xm);
          velocity->GetTuple(idx,vp);
          velocity->GetTuple(idx2,vm);
          } 
        else if (k == (dims[2]-1)) 
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
  double xp[3], xm[3];
  float g[3], pp, pm, factor;
  float xxi, yxi, zxi, pxi;
  float xeta, yeta, zeta, peta;
  float xzeta, yzeta, zzeta, pzeta;
  float aj, xix, xiy, xiz, etax, etay, etaz, zetax, zetay, zetaz;

  //  Check that the required data is available
  //
  if ((points=this->GetOutput()->GetPoints()) == NULL || 
       this->Density == NULL || this->Momentum == NULL || 
       this->Energy == NULL)
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
        if (dims[0] == 1) // 2D in this direction
          {
          factor = 1.0;
          for (ii=0; ii<3; ii++)
            {
            xp[ii] = xm[ii] = 0.0;
            }
          xp[0] = 1.0; pp = pm = 0.0;
          }
        else if (i == 0) 
          {
          factor = 1.0;
          idx = (i+1) + j*dims[0] + k*ijsize;
          idx2 = i + j*dims[0] + k*ijsize;
          points->GetPoint(idx,xp);
          points->GetPoint(idx2,xm);
          pp = pressure->GetComponent(idx,0);
          pm = pressure->GetComponent(idx2,0);
          } 
        else if (i == (dims[0]-1)) 
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
        if (dims[1] == 1) // 2D in this direction
          {
          factor = 1.0;
          for (ii=0; ii<3; ii++)
            {
            xp[ii] = xm[ii] = 0.0;
            }
          xp[1] = 1.0; pp = pm = 0.0;
          }
        else if (j == 0) 
          {
          factor = 1.0;
          idx = i + (j+1)*dims[0] + k*ijsize;
          idx2 = i + j*dims[0] + k*ijsize;
          points->GetPoint(idx,xp);
          points->GetPoint(idx2,xm);
          pp = pressure->GetComponent(idx,0);
          pm = pressure->GetComponent(idx2,0);
          } 
        else if (j == (dims[1]-1)) 
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
        if (dims[2] == 1) // 2D in this direction
          {
          factor = 1.0;
          for (ii=0; ii<3; ii++)
            {
            xp[ii] = xm[ii] = 0.0;
            }
          xp[2] = 1.0; pp = pm = 0.0;
          }
        else if (k == 0) 
          {
          factor = 1.0;
          idx = i + j*dims[0] + (k+1)*ijsize;
          idx2 = i + j*dims[0] + k*ijsize;
          points->GetPoint(idx,xp);
          points->GetPoint(idx2,xm);
          pp = pressure->GetComponent(idx,0);
          pm = pressure->GetComponent(idx2,0);
          } 
        else if (k == (dims[2]-1)) 
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

void vtkVisItPLOT3DReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "XYZ File Name: " << 
    (this->XYZFileName ? this->XYZFileName : "(none)") << "\n";
  os << indent << "Q File Name: " <<
    (this->QFileName ? this->QFileName : "(none)") << "\n";
  os << indent << "Function File Name: " << 
    (this->FunctionFileName ? this->FunctionFileName : "(none)") << "\n";

  os << indent << "Grid Number: " << this->GridNumber << "\n";
  os << indent << "Scalar Function Number: " 
     << this->ScalarFunctionNumber << "\n";

  if (this->VectorFunctionFileName)
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


void vtkVisItPLOT3DReader::AddFunction(int functionNumber)
{
  this->FunctionList->InsertNextValue(functionNumber);
}


void vtkVisItPLOT3DReader::RemoveAllFunctions()
{
  this->FunctionList->Reset();
}





// ****************************************************************************
//  Method: vtkVisItPLOT3DReader::ReadNumbers
//
//  Arguments;
//      fp:             file stream from which to read numbers
//      numbersToRead:  amount of numbers to read.
//      output:         location to store the numbers.
//
//  Programmer:  Abel Gezahegne
//  Creation:    Sept. 12, 2004
//
//  Modifications:
//    Jeremy Meredith, Tue Feb 22 12:57:17 PST 2005
//    Since 'output' is a float*, not a float*&, we cannot meaningfully
//    allocate space for it inside this method.  I disallowed it.
//
//    Hank Childs, Thu Jun  8 14:04:15 PDT 2006
//    Change BIG_ENDIAN to VTK_BIG_ENDIAN because of namespace conflict with
//    endian.h on SUSE.
//
// ****************************************************************************

int vtkVisItPLOT3DReader::ReadNumbers( FILE *fp, unsigned int numbersToRead, 
                                        float * output)
{
  if (output == NULL)
  {
    return 1;
  }

  if (this->FileType == ASCII && this->Compression == UNCOMPRESSED_ASCII)
  {
    for (unsigned int i = 0; i < numbersToRead; i++)
    {
      fscanf(fp,"%f",output + i);
    }
  }
 
  else if (this->FileType == ASCII && this->Compression == COMPRESSED_ASCII)
  {
    float multiplier;
    float multiplicand;
    char garbage[1024];
    for (unsigned int i = 0; i < numbersToRead;)
    {
      fscanf(fp,"%f%[^0-9.-]s",&multiplier,garbage);
      if (strchr(garbage,'*'))
      {
        fscanf(fp,"%f%[^0-9.-]s",&multiplicand,garbage);
        for (int j = 0; j < (int)multiplier; j++)
        {
          output[i++] = multiplicand;
        }
      }
      else
      {
        output[i++] = multiplier;
      }
    }
  }
  else if (this->FileType == BINARY)
  {
    bool isLittleEndian = true;
    #ifdef WORDS_BIGENDIAN
      isLittleEndian = true;
    #endif

    if (this->Precision == SINGLE_PRECISION)
    {
      if ((fread(output, sizeof(float),numbersToRead,fp))< numbersToRead)
      {
        return 1;
      }
      else
      {
        if ((this->Endianness == VTK_BIG_ENDIAN && isLittleEndian) ||
           (this->Endianness == VTK_LITTLE_ENDIAN && !isLittleEndian))
        {
          vtkByteSwap::Swap4BERange(output,numbersToRead);
        }
      }
    }
    else if (this->Precision == DOUBLE_PRECISION)
    {
      double * doubleOutput = new double[numbersToRead];
      if ((fread(doubleOutput,sizeof(double),numbersToRead,fp))<numbersToRead)
      {
        return 1;
      }
      else
      {
        if (( this->Endianness == VTK_BIG_ENDIAN && isLittleEndian) ||
            ( this->Endianness == VTK_LITTLE_ENDIAN && !isLittleEndian))
        {
          vtkByteSwap::Swap8BERange(doubleOutput,numbersToRead);
        }

        //  convert the doubles to floats. 
        for (unsigned int i = 0; i< numbersToRead; i++)
        {
          output[i] = (float)doubleOutput[i];
        }
      }
    }
  }
  return 0;
}



// ****************************************************************************
//  Method: vtkVisItPLOT3DReader::ReadNumbers
//
//  Arguments;
//      fp:             file stream from which to read numbers
//      numbersToRead:  amount of numbers to read.
//      output:         location to store the numbers.
//
//  Programmer:  Abel Gezahegne
//  Creation:    Sept. 12, 2004
//
//  Modifications:
//    Jeremy Meredith, Tue Feb 22 12:57:17 PST 2005
//    Check for a NULL output array.
//
//    Hank Childs, Thu Jun  8 14:04:15 PDT 2006
//    Change BIG_ENDIAN to VTK_BIG_ENDIAN because of namespace conflict with
//    endian.h on SUSE.
//
// ****************************************************************************


int vtkVisItPLOT3DReader::ReadNumbers( FILE *fp, unsigned int numbersToRead, 
                                        int * output)
{
  if (output == NULL)
  {
    return 1;
  }

  if (this->FileType == ASCII && this->Compression == UNCOMPRESSED_ASCII)
  {
    for (unsigned int i = 0; i < numbersToRead; i++)
    {
      fscanf(fp,"%d",output + i);
    }
  }
  else if (this->FileType == ASCII && this->Compression == COMPRESSED_ASCII)
  {
    int multiplier;
    int multiplicand;
    char garbage[1024];
    for (unsigned int i = 0; i < numbersToRead;)
    {
      fscanf(fp,"%d%[^0-9.-]s",&multiplier,garbage);
      if (strchr(garbage,'*'))
      {
        fscanf(fp,"%d%[^0-9.-]s",&multiplicand,garbage);
        for (int j = 0; j < (int)multiplier; j++)
        {
          output[i++] = multiplicand;
        }
      }
      else
      {
        output[i++] = multiplier;
      }
    }
  }
  else if (this->FileType ==  BINARY)
  {
    if (fread(output, sizeof(int),numbersToRead,fp)< numbersToRead)
    {    
       return 1;
    }
    else
    {
      bool isLittleEndian = true;
      #ifdef WORDS_BIGENDIAN
        isLittleEndian = false;
      #endif
      if ((this->Endianness == VTK_BIG_ENDIAN && isLittleEndian) ||
          (this->Endianness == VTK_LITTLE_ENDIAN && !isLittleEndian))
      {
        vtkByteSwap::Swap4BERange(output,numbersToRead);
      }
    }
  }
  return 0;
}







// ****************************************************************************
//  Method: vtkVisItPLOT3DReader::ComputeGridOffset
//
//  Arguments;
//      fp:             file stream for grid file
//
//  Programmer:  Abel Gezahegne
//  Creation:    Sept. 12, 2004
//
//  Modifications:
//    Jeremy Meredith, Tue Feb  8 18:17:05 PST 2005
//    Allocated numbersToSkip before reading into it.
//
// ****************************************************************************

int vtkVisItPLOT3DReader::ComputeGridOffset(FILE *fp)
{
  int i;

  // If offset known return it else compute. 
  if (GridOffsets[this->GridNumber] >0) 
  {
    return GridOffsets[this->GridNumber];
  }
  else
  {
    //  Starting from current grid back up until last known offset. 
    for (i = this->GridNumber; i>0 && GridOffsets[i] < 0; i--) 
    {  
     ;
    }

    for (int j = i+1; j <= this->GridNumber; j++)
    {
      int * numbersToSkip;
      if (this->FileType == BINARY)
      {
        if (this->Iblanking == IBLANKING)
        {
          GridOffsets[j] = (GridOffsets[0] + 4*GridSizes[j-1]*4);
        }
        else
        {
          GridOffsets[j] = (GridOffsets[0] + 3*GridSizes[j-1]*4);
        }
      }
      else
      {
        int numberOfElements;
        if (this->Iblanking == IBLANKING)
        {
          numberOfElements = 4*GridSizes[j-1];
        }
        else
        {
          numberOfElements = 3*GridSizes[j-1];
        }
        numbersToSkip = new int[numberOfElements];
        fseek(fp,(long)(GridOffsets[j-1]),SEEK_SET);
        ReadNumbers(fp,numberOfElements,numbersToSkip);
        GridOffsets[j] = ftell(fp);
      }
    }
    return GridOffsets[this->GridNumber];
  }
}



// ****************************************************************************
//  Method: vtkVisItPLOT3DReader::ComputeSolutionOffset
//
//  Arguments;
//      fp:             file stream for solution file 
//
//  Programmer:  Abel Gezahegne
//  Creation:    Sept. 12, 2004
//
//  Modifications:
//    Jeremy Meredith, Tue Feb  8 18:17:05 PST 2005
//    Allocated numbersToSkip before reading into it.
//
// ****************************************************************************

int vtkVisItPLOT3DReader::ComputeSolutionOffset(FILE *fp)
{
  int i;

  //  If solution offset known return it.
  if (SolutionOffsets[this->GridNumber] > 0)
  {
    return SolutionOffsets[this->GridNumber];
  }
  else
  {
    // back up until last known offset.
    for (i = this->GridNumber; i > 0 && SolutionOffsets[i]< 0; i--)
    {
     ;
    }

    for (int j = i+1; j<= this->GridNumber; j++)
    {
      float * numbersToSkip;
      if (this->FileType == BINARY)
      {
        SolutionOffsets[j] = SolutionOffsets[0] + 4 * sizeof(float) + 
                             5 * GridSizes[j-1]*sizeof(float);
      }
      else
      {
        int numberOfElements;
        numberOfElements = 4 + 5*GridSizes[j-1];
        fseek(fp,(long)SolutionOffsets[j-1],SEEK_SET);
        numbersToSkip = new float[numberOfElements];
        ReadNumbers(fp,numberOfElements,numbersToSkip);
        SolutionOffsets[j] = ftell(fp);
      }
    }
    return SolutionOffsets[this->GridNumber];
  }
}




// NOW START AVT CODE

#include <avtPLOT3DFileFormat.h>

#include <vector>
#include <string>

#include <vtkDataArray.h>
#include <vtkPointData.h>

#include <Expression.h>

#include <avtDatabaseMetaData.h>

#include <BadIndexException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidVariableException.h>
#include <InvalidFilesException.h>

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
//    Abel Gezahegne, Sept 12, 2004
//    Add the Visit plot3d file extension. 
//
// ****************************************************************************

avtPLOT3DFileFormat::avtPLOT3DFileFormat(const char *fname)
    : avtSTMDFileFormat(&fname, 1)
{
    char *s_file = NULL;
    char *x_file = NULL;
    char *v_file = NULL;
  
    if (strstr(fname, ".vp3d") != NULL)
    {
        v_file = filenames[0];
    }
    else if (strstr(fname, ".x") != NULL)
    {
        char soln_file[1024];
        const char *q = strstr(fname, ".x");
        strncpy(soln_file, fname, q-fname);
        strcpy(soln_file + (q-fname), ".q");
          
        AddFile(soln_file);
        x_file = filenames[0];
        s_file = filenames[1];
    }
    else if (strstr(fname, ".q") != NULL)
    {
        char points_file[1024];
        const char *x = strstr(fname, ".q");
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
        debug1 << "PLOT3D reader giving up with file identification of " 
               << fname << endl;
        EXCEPTION0(ImproperUseException);
    }

    reader = vtkVisItPLOT3DReader::New();

    reader->SetVP3DFileName(v_file); 
    reader->SetXYZFileName(x_file);
    reader->SetQFileName(s_file);
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


// *****************************************************************************
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
//
//  Modifications:
//     Abel Gezahenge,  Sept 12, 2004
//     Read information from Visit Plot3d file; read global expressions and
//     account for 2 and 3D files.  
//  
//    Jeremy Meredith, Thu Aug  7 15:47:31 EDT 2008
//    Use const char*'s for string literlas.
//
// ****************************************************************************

void
avtPLOT3DFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "mesh";
    mesh->meshType = AVT_CURVILINEAR_MESH;

    if (reader->GetVP3DFileName() != NULL)
    {
        char points_file[1024];
        char soln_file[1024];
        char * fname = reader->GetVP3DFileName();
        char *x = strstr(fname ,".vp3d");

        strncpy(points_file, fname, x-fname);
        strcpy(points_file + (x-fname), ".x");

        strncpy(soln_file, fname,x-fname);
        strcpy(soln_file + (x-fname),".q");

        reader->CollectInfo(points_file,soln_file);
        AddFile(points_file);
        AddFile(soln_file);
        reader->SetXYZFileName(filenames[1]);
        reader->SetQFileName(filenames[2]);
    }


    FILE *xyzFp;
    if (reader->GetFileType() == BINARY)
    {
        if ((xyzFp = fopen(reader->GetXYZFileName() ,"rb"))== NULL)
        {
            EXCEPTION1(InvalidFilesException, reader->GetXYZFileName()); 
            return;
        }
    }
        else
    {
        if ((xyzFp = fopen(reader->GetXYZFileName() ,"r"))== NULL)
        {   
            EXCEPTION1(InvalidFilesException, reader->GetXYZFileName());
            return;
        }
    }
        
        reader->SetNumberOfGrids(reader->FindNumberOfGrids(xyzFp));

    mesh->numBlocks = reader->GetNumberOfGrids();  
    mesh->blockOrigin = 0;

    if (reader->GetDimension() == THREE)
    {
        mesh->spatialDimension = 3;
        mesh->topologicalDimension = 3;
    }
    else
    {
        mesh->spatialDimension = 2;
        mesh->topologicalDimension =2;
    }

    mesh->hasSpatialExtents = false;
    md->Add(mesh);

    const int NUM_SCALARS = 10;
    const char *scalar_names[NUM_SCALARS] = { "density", "pressure", 
         "temperature", "enthalpy", "internal_energy", "kinetic_energy", 
         "velocity_magnitude", "stagnation_energy", "entropy", "swirl" };

    int i;
    for (i = 0; i < NUM_SCALARS; i++)
    {
        avtScalarMetaData *sd1 = new avtScalarMetaData;
        sd1->name = scalar_names[i];
        sd1->meshName = "mesh";
        sd1->centering = AVT_NODECENT;
        sd1->hasDataExtents = false;
        md->Add(sd1);
    }

    const int NUM_VECTORS = 4;
    const char *vector_names[4] = { "velocity", "vorticity", "momentum",
                                    "pressure_gradient"};
    for (i = 0; i < NUM_VECTORS; i++)
    {
        avtVectorMetaData *vd1 = new avtVectorMetaData;
        vd1->name = vector_names[i];
        vd1->meshName = "mesh";
        vd1->centering = AVT_NODECENT;
        vd1->hasDataExtents = false;
        vd1->varDim = 3;
        md->Add(vd1);
    }


   //  Read the four constant values Free-stream mach number, Angle of attack,
   //  Reynold's number, and Intergration time.  
   //  Note that only the numbers for the first grid are used even if a multi
   //  grid file may contain unique values for each grid.  

    FILE * QFp;

    if (reader->GetQFileName() != NULL)
    {
      int error = 0;
      if (reader->GetFileType() == BINARY)
      {
        if ((QFp = fopen(reader->GetQFileName(),"rb")) == NULL)
        {
          error = 1;
        }
      }
      else
      {
        if ((QFp = fopen(reader->GetQFileName(),"r")) == NULL)
        {
          error = 1;
        }
      }
      
      if (! error)
      {
        int skip = reader->GetNumberOfGrids();
        if (reader->GetDimension() == THREE)
          skip *= 3;
        else
          skip *= 2;
        
        if (reader->GetGridType() == MULTI_GRID)
          skip += 1;

        int *numbersToSkip = new int [skip];
        bool error1 = reader->ReadNumbers(QFp,skip,numbersToSkip);
        if (!error1)
        {
          Expression exp;
          float param[4];
          char def[1024];

          reader->ReadNumbers(QFp,4,param);
          exp.SetName("Free-stream mach number");
          sprintf(def,"%f",param[0]);
          exp.SetDefinition(def);
          exp.SetType(Expression::Unknown);
          md->AddExpression(&exp);

          exp.SetName("Angle of attack");
          sprintf(def,"%f",param[1]);
          exp.SetDefinition(def);
          exp.SetType(Expression::Unknown);
          md->AddExpression(&exp);

          exp.SetName("Reynold's number");
          sprintf(def,"%f",param[2]);
          exp.SetDefinition(def);
          exp.SetType(Expression::Unknown);
          md->AddExpression(&exp);

          exp.SetName("Integration time");
          sprintf(def,"%f",param[3]);
          exp.SetDefinition(def);
          exp.SetType(Expression::Unknown);
          md->AddExpression(&exp);
        }
        fclose(QFp);
      }
    }
}



