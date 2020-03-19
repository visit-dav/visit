// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// 
// This was taken mostly from vtkMultiBlockPlot3DReader (VTK Version 6.1)
// by Kitware, http://www.vtk.org
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

#ifndef vtkPLOT3DReader_h
#define vtkPLOT3DReader_h

#include <vtkObject.h>
#include "PLOT3DFunctions.h"

class vtkDataArray;
class vtkDataSet;
class vtkStructuredGrid;
struct vtkPLOT3DReaderInternals;

class vtkPLOT3DReader : public vtkObject
{
public:
  static vtkPLOT3DReader *New();
  vtkTypeMacro(vtkPLOT3DReader,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Set/Get the PLOT3D geometry filename.
  void SetXYZFileName( const char* );
  vtkGetStringMacro(XYZFileName);

  // Description:
  // Set/Get the PLOT3D solution filename.
  vtkSetStringMacro(QFileName);
  vtkGetStringMacro(QFileName);

  // Description:
  // Is the file to be read written in binary format (as opposed
  // to ascii).
  vtkSetMacro(BinaryFile, bool);
  vtkGetMacro(BinaryFile, bool);
  vtkBooleanMacro(BinaryFile, bool);

  // Description:
  // Does the file to be read contain information about number of
  // grids. In some PLOT3D files, the first value contains the number
  // of grids (even if there is only 1). If reading such a file,
  // set this to true.
  vtkSetMacro(MultiGrid, bool);
  vtkGetMacro(MultiGrid, bool);
  vtkBooleanMacro(MultiGrid, bool);

  // Description:
  // Were the arrays written with leading and trailing byte counts ?
  // Usually, files written by a fortran program will contain these
  // byte counts whereas the ones written by C/C++ won't.
  // Newer Fortran files written as unformatted 'stream' will also
  // not have ByteCount.
  vtkSetMacro(HasByteCount, bool);
  vtkGetMacro(HasByteCount, bool);
  vtkBooleanMacro(HasByteCount, bool);

  // Description:
  // Is there iblanking (point visibility) information in the file.
  // If there is iblanking arrays, these will be read and assigned
  // to the PointVisibility array of the output.
  vtkSetMacro(IBlanking, bool);
  vtkGetMacro(IBlanking, bool);
  vtkBooleanMacro(IBlanking, bool);

  // Description:
  // If only two-dimensional data was written to the file,
  // turn this on.
  vtkSetMacro(TwoDimensionalGeometry, bool);
  vtkGetMacro(TwoDimensionalGeometry, bool);
  vtkBooleanMacro(TwoDimensionalGeometry, bool);

  // Description:
  // Is this file in double precision or single precision.
  // This only matters for binary files.
  // Default is single.
  vtkSetMacro(DoublePrecision, bool);
  vtkGetMacro(DoublePrecision, bool);
  vtkBooleanMacro(DoublePrecision, bool);

  // Description:
  // Set the byte order of the file (remember, more Unix workstations
  // write big endian whereas PCs write little endian). Default is
  // big endian (since most older PLOT3D files were written by
  // workstations).
  void SetByteOrderToBigEndian();
  void SetByteOrderToLittleEndian();
  vtkSetMacro(ByteOrder, int);
  vtkGetMacro(ByteOrder, int);
  const char *GetByteOrderAsString();

  // Description:
  // Set/Get the gas constant. Default is 1.0.
  vtkSetMacro(R,double);
  vtkGetMacro(R,double);

  // Set/Get the ratio of specific heats. Default is 1.4.
  vtkSetMacro(Gamma,double);
  vtkGetMacro(Gamma,double);

  int GetPrecision();
  int GetNumberOfPoints() { return this->NumberOfPoints; }

  int RequestInformation();


//BTX
  enum
  {
    FILE_BIG_ENDIAN=0,
    FILE_LITTLE_ENDIAN=1
  };
//ETX

//BEGIN LLNL ADDITIONS
  // Description:
  // This returns the number of grids in the current file. 
  int GetNumberOfGrids();

  // the grid number of interest (for multi-grid files)
  void SetGridNumber(int);
  vtkGetMacro(GridNumber, int);

  vtkDataArray *GetProperties() { return this->Properties; }

  // Reads the grid specified by GridNumber
  int ReadGrid();

  // Reads the output grid (current structured)
  vtkDataSet *GetOutput();

  // Reads the solution file for the specified function.
  // Function values stored in 'f'.
  int GetFunction_float(const char *funcName, float *f);
  int GetFunction_double(const char *funcName, double *f);

  double GetTime();

  // overflow files only
  // 
  vtkGetMacro(IsOverflow, bool);
  vtkGetMacro(OverflowNQ, int);
  vtkGetMacro(OverflowNQC, int);
//END LLNL ADDITIONS

protected:
  vtkPLOT3DReader();
  ~vtkPLOT3DReader();

  vtkDataArray* CreateFloatArray();

  int CheckFile(FILE*& fp, const char* fname);
  int CheckGeometryFile(FILE*& xyzFp);
  int CheckSolutionFile(FILE*& qFp);

  int SkipByteCount (FILE* fp);
  int ReadIntBlock  (FILE* fp, int n, int*   block);

  int ReadScalar(FILE* fp, int n, vtkDataArray* scalar);
  int ReadVector(FILE* fp, int n, int numDims, vtkDataArray* vector);

  int GetNumberOfBlocksInternal(FILE* xyzFp);

  int ReadGeometryHeader(FILE* fp);
  int ReadQHeader(FILE* fp);

  void CalculateFileSize(FILE* fp);

  int AutoDetectionCheck(FILE* fp);

  // Returns a vtkFloatArray or a vtkDoubleArray depending
  // on DoublePrecision setting
  vtkDataArray* NewFloatArray();

  //plot3d FileNames
  char *XYZFileName;
  char *QFileName;

  bool BinaryFile;
  bool HasByteCount;
  bool TwoDimensionalGeometry;
  bool MultiGrid;
  int ForceRead;
  int ByteOrder;
  bool IBlanking;
  bool DoublePrecision;

  long FileSize;

  //parameters used in computing derived functions
  double R;
  double Gamma;

  vtkPLOT3DReaderInternals* Internal;

// BEGIN LLNL ADDITIONS
  int NumberOfGrids;
  int NumberOfPoints;

  // for multi-grid, the grid we're interested in
  int   GridNumber;

  // storage used to cache grid offsets for multi-grid files
  long *GridSizes;
  long *GridOffsets;
  long *SolutionOffsets;
  int  *GridDimensions;
// END LLNL ADDITIONS

private:
  vtkPLOT3DReader(const vtkPLOT3DReader&);  // Not implemented.
  void operator=(const vtkPLOT3DReader&);  // Not implemented.

// BEGIN LLNL ADDITIONS
  long ComputeGridOffset(FILE *xyzFp);
  long ComputeSolutionOffset(FILE *qFp);
  int ReadGrid(FILE *xyzFp);
  int ReadSolutionProperties(FILE *qFp);

  template <class DataType>
  int GetFunction(PLOT3DFunctions<DataType> &, const char *, DataType *);

  vtkDataArray *Properties;

  bool IsOverflow;
  int  OverflowNQ;
  int  OverflowNQC;
  int  NumProperties;

  vtkStructuredGrid *output;
// END LLNL ADDITIONS
};

#endif


