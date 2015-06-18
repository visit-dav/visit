/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPLOT3DReader.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPLOT3DReader - read PLOT3D data files
// .SECTION Description
// vtkPLOT3DReader is a derived class of vtkMultiBlockPLOT3DReader.
// A few methods have been re-implmented in order to allow only a single
// block to be read and served up at a time.
// It still creates a vtkMultiBlockDataSet, but this is only populated
// with a single dataset.

#ifndef __vtkPLOT3DReader_h
#define __vtkPLOT3DReader_h

#include <vtkMultiBlockPLOT3DReader.h>
#include <vtkIdList.h> // Needed for internal method

class vtkDataSet;

class vtkPLOT3DReader : public vtkMultiBlockPLOT3DReader
{
public:
  static vtkPLOT3DReader *New();
  vtkTypeMacro(vtkPLOT3DReader,vtkMultiBlockPLOT3DReader);
  void PrintSelf(ostream& os, vtkIndent indent);


  // Description:
  // This returns the number of outputs this reader will produce.
  // This number is equal to the number of grids in the current
  // file. This method has to be called before getting any output
  // if the number of outputs will be greater than 1 (the first
  // output is always the same).
  int GetNumberOfGrids();

  // the grid number of interest (for multi-grid files)
  vtkSetMacro(GridNumber, int);
  vtkGetMacro(GridNumber, int);

  vtkDataArray *GetProperties() { return this->Properties; }

  // Convenience method to extract the single dataset from the
  // vtkMultiBlockDataSet output.
  vtkDataSet *GetCurrentBlock();


protected:
  vtkPLOT3DReader();
  ~vtkPLOT3DReader();

  int  RequestInformation(vtkInformation*,
                          vtkInformationVector **,
                          vtkInformationVector *);
  int  RequestData(vtkInformation*,
                   vtkInformationVector **,
                   vtkInformationVector *);

  int ReadGeometryHeader(FILE* fp, vtkInformationVector *);
  int ReadQHeader(FILE* fp);
  int ReadFunctionHeader(FILE *fp, vtkIdList*& counts);


  int NumberOfGrids;
  int NumberOfPoints;

  long FileSize;


  // for multi-grid, the grid we're interested in
  int   GridNumber;

  // storage used to cache grid offsets for multi-grid files
  long *GridSizes;
  long *GridOffsets;
  long *SolutionOffsets;
  int  *GridDimensions;

private:
  vtkPLOT3DReader(const vtkPLOT3DReader&);  // Not implemented.
  void operator=(const vtkPLOT3DReader&);  // Not implemented.

  long ComputeGridOffset(FILE *xyzFp);
  long ComputeSolutionOffset(FILE *qFp);
  int ReadGrid(FILE *xyzFp, vtkStructuredGrid *output);
  int ReadSolution(FILE *qFp, vtkStructuredGrid *output);
  int ReadSolutionProperties(FILE *qFp);

  vtkDataArray *Density;
  vtkDataArray *Momentum;
  vtkDataArray *Energy;
  vtkDataArray *Properties;
};

#endif


