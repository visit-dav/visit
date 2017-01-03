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
