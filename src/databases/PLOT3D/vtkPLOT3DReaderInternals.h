// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// 
// This was taken entirely from vtkMultiBlockPlot3DReaderInternals,
// VTK Version 6.1, by Kitware, http://www.vtk.org
//
// Couldn't use their file directly due to inclusion of 
// vtkMultiBlockPlot3DReader header file
//
// Changes from their version:
// The storage of StructuredGrid blocks was removed.
//


#ifndef vtkPLOT3DReaderInternals_h
#define vtkPLOT3DReaderInternals_h

#include <vtkPLOT3DReader.h>

struct vtkPLOT3DReaderInternals
{
  int BinaryFile;
  int ByteOrder;
  int HasByteCount;
  int MultiGrid;
  int NumberOfDimensions;
  int Precision; // in bytes
  int IBlanking;

  bool NeedToCheckXYZFile;

  vtkPLOT3DReaderInternals() :
    BinaryFile(1),
    ByteOrder(vtkPLOT3DReader::FILE_BIG_ENDIAN),
    HasByteCount(1),
    MultiGrid(0),
    NumberOfDimensions(3),
    Precision(4),
    IBlanking(0),
    NeedToCheckXYZFile(true)
    {
    }

  int ReadInts(FILE* fp, int n, int* val);
  void CheckBinaryFile(FILE *fp);
  int CheckByteOrder(FILE* fp);
  int CheckByteCount(FILE* fp);
  int CheckMultiGrid(FILE* fp);
  int Check2DGeom(FILE* fp);
  int CheckBlankingAndPrecision(FILE* fp);
  int CheckCFile(FILE* fp, long fileSize);
  long CalculateFileSize(int mgrid,
                         int precision, // in bytes
                         int blanking,
                         int ndims,
                         int hasByteCount,
                         int nGrids,
                         int* gridDims);
  long CalculateFileSizeForBlock(int precision, // in bytes
                                 int blanking,
                                 int ndims,
                                 int hasByteCount,
                                 int* gridDims);
};
#endif
