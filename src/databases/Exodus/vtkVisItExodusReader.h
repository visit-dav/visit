// THIS IS VTK CODE THAT IS NOT PUT IN THE visit_vtk DIRECTORY, SINCE IT
// CONTAINS REFERENCES TO THE EXODUS LIBRARY, AND THAT LIBRARY IS NOT 
// LINKED AGAINST THE VIEWER. 

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItExodusReader.h,v $
  Language:  C++
  Date:      $Date: 2000/12/10 20:08:25 $
  Version:   $Revision: 1.14 $


Copyright (c) 1993-2001 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkVisItExodusReader - Read exodus 2 files .ex2
// .SECTION Description
// vtkVisItExodusReader is a unstructured grid source object that reads ExodusII
// files.  Most of the meta data associated with the file is loaded when 
// UpdateInformation is called.  This includes information like Title, number
// of blocks, number and names of arrays. This data can be retrieved from 
// methods in this reader. Separate arrays that are meant to be a single 
// vector, are combined internally for convenience.  To be combined, the array 
// names have to be identical except for a trailing X,Y and Z (or x,y,z).  By 
// default all cell and point arrays are loaded.  However, the user can flag 
// arrays not to load with the methods "SetPointDataArrayLoadFlag" and
// "SetCellDataArrayLoadFlag".  The reader responds to piece requests
// by loading only a range of the possible blocks.  Unused points
// are filtered out internally.

// Modifications:
//
//   Hank Childs, Sat Apr 17 07:29:22 PDT 2004
//   Added support for reading times.
//
//   Hank Childs, Sun Jun 27 10:31:18 PDT 2004
//   Added support for reading global node numbers.
//
//   Eric Brugger, Fri Mar  9 14:10:43 PST 2007
//   Added support for reading element block names.
//

#ifndef __vtkVisItExodusReader_h
#define __vtkVisItExodusReader_h

#include <stdio.h>
#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkIntArray.h"
#include "vtkStringArray.h"

class vtkVisItExodusReader : public vtkUnstructuredGridAlgorithm 
{
public:
  static vtkVisItExodusReader *New();
  vtkTypeMacro(vtkVisItExodusReader,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify file name of the Exodus file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Which TimeStep to read.  Indexed from 1.
  vtkSetClampMacro(TimeStep, int, 1, VTK_LARGE_INTEGER);
  vtkGetMacro(TimeStep, int);
  
  // Description:
  // Extra cell data array that can be generated.  By default, this array
  // is off.  The value of the array is the integer id found
  // in the exodus file. The name of the array is "BlockId".
  vtkSetMacro(GenerateBlockIdCellArray, int);
  vtkGetMacro(GenerateBlockIdCellArray, int);
  vtkBooleanMacro(GenerateBlockIdCellArray, int);

  // Description:
  // Extra cell data array that can be generated.  By default, this array
  // is off.  The value of the array is the integer id of the cell
  // The id is local to the block containing the element.
  // The name of the array is "ElementId".
  vtkSetMacro(GenerateElementIdArray, int);
  vtkGetMacro(GenerateElementIdArray, int);
  vtkBooleanMacro(GenerateElementIdArray, int);
  
  // Description:
  // Extra cell data array that can be generated.  By default, this array
  // is off.  The value of the array is the integer id of the cell
  // The id is relative to the entire data set (all blocks).
  // The name of the array is "ElementGlobalId".
  vtkSetMacro(GenerateElementGlobalIdArray, int);
  vtkGetMacro(GenerateElementGlobalIdArray, int);
  vtkBooleanMacro(GenerateElementGlobalIdArray, int);
  
  // Description:
  // Extra node data array that can be generated.  By default, this array
  // is off.  The value of the array is the integer id of the node.
  // The id is relative to the entire data set (all blocks).
  // The name of the array is "avtGlobalNodeId".
  vtkSetMacro(GenerateNodeGlobalIdArray, int);
  vtkGetMacro(GenerateNodeGlobalIdArray, int);
  vtkBooleanMacro(GenerateNodeGlobalIdArray, int);
  
  // Description:
  // Extra point data array that can be generated.  By default, this array
  // is off.  The value of the array is the integer id of the node.
  // The id is relative to the entire data set.
  // The name of the array is "NodeId".
  vtkSetMacro(GenerateNodeIdArray, int);
  vtkGetMacro(GenerateNodeIdArray, int);
  vtkBooleanMacro(GenerateNodeIdArray, int);
  
  // Description:
  // Access to meta data generated by UpdateInformation.
  vtkGetStringMacro(Title);
  vtkGetMacro(Dimensionality, int);
  vtkGetMacro(NumberOfNodes, int);
  vtkGetMacro(NumberOfElements, int);
  vtkGetMacro(NumberOfBlocks, int);
  vtkGetMacro(NumberOfTimeSteps, int);
  vtkGetMacro(NumberOfPointDataArrays, int);
  vtkGetMacro(NumberOfCellDataArrays, int);
  int GetNumberOfElementsInBlock(int blockIdx);
  int GetBlockId(int blockIdx);
  const char *GetBlockName(int blockIdx);
  const char *GetPointDataArrayName(int arrayIdx);
  int GetPointDataArrayNumberOfComponents(int arrayIdx);
  const char *GetCellDataArrayName(int arrayIdx);
  int GetCellDataArrayNumberOfComponents(int arrayIdx);
  
  // Descriptions:
  // By default all arrays are loaded.  These methods allow the user to select
  // which arrays they want to load.  You can get information about the arrays
  // by first caling UpdateInformation, and useing GetPointDataArrayName ...
  void SetPointDataArrayLoadFlag(int index, int flag);
  int GetPointDataArrayLoadFlag(int index);
  void SetCellDataArrayLoadFlag(int index, int flag);
  int GetCellDataArrayLoadFlag(int index);
  
  void  LoadTimes(void);
  float GetTime(int ts) {  return (this->Times != NULL && ts >= 0 &&
                                   ts < this->NumberOfTimeSteps
                           ? Times[ts]
                           : 0.); };
protected:
  vtkVisItExodusReader();
  ~vtkVisItExodusReader();
  
  int RequestInformation(vtkInformation *, vtkInformationVector **, 
                          vtkInformationVector*);
  int RequestData(vtkInformation *, vtkInformationVector **, 
                   vtkInformationVector*);

  void ReadGeometry(int exoid, vtkUnstructuredGrid *);
  void ReadCells(int exoid, vtkUnstructuredGrid *);
  void ReadPoints(int exoid, vtkUnstructuredGrid *);
  void ReadArrays(int exoid, vtkUnstructuredGrid *);
  vtkDataArray *ReadPointDataArray(int exoid, int varIndex);
  vtkDataArray *ReadPointDataVector(int exoid, int varIndex, int dim);
  vtkDataArray *ReadCellDataArray(int exoid, int varIndex);
  vtkDataArray *ReadCellDataVector(int exoid, int varIndex, int dim);
  void GenerateExtraArrays(vtkUnstructuredGrid *);

  void CheckForProblems(vtkUnstructuredGrid *);
  
  // Used internally for allocating memory for meta data.
  // Simplyfy combines arrays into vectors.
  void SetNumberOfPointDataArrays(int num);
  void SetNumberOfCellDataArrays(int num);
  int SimplifyArrayNames(char **ArrayNames, int *ArrayNumComps,
                         int NumScalarArrays);
  // This method efficiently keeps track of which points are being used,
  // and sets up the PointMapOutIn (as well as pointMapInOut) without 
  // duplicates. The InOutMap is passed in as an array to help inline.
  int *GetOutPointId(int *inPtId, int *pointMapInOutArray);
  //vtkIdType *GetOutPointId(vtkIdType *inPtId, vtkIdType *pointMapInOutArray);
  
  // Paramemters for controlling what is read in.
  char *FileName;
  int TimeStep;
  int GenerateBlockIdCellArray;
  int GenerateElementIdArray;
  int GenerateElementGlobalIdArray;
  int GenerateNodeIdArray;
  int GenerateNodeGlobalIdArray;

  // Keep the file name that was used to generate the geometry.
  // This is to avoid rereading the geometry when only time step has changed.
  char *GeometryFileName;
  vtkSetStringMacro(GeometryFileName);
  vtkGetStringMacro(GeometryFileName);
  vtkUnstructuredGrid *GeometryCache;
  // Information about which blocks were loaded.
  int StartBlock;
  int EndBlock;
  // Point mapping to save a subset of the points.
  // Index is the output id, value is the input id.
  vtkIdList *PointMapOutIn;
  
  // Information specific for exodus files.
  char *Title;
  int Dimensionality;
  int NumberOfBlocks;
  int NumberOfNodes;
  int NumberOfElements;
  int NumberOfTimeSteps;
  float *Times;
  vtkIntArray *NumberOfBlockElements;
  vtkIntArray *BlockIds;
  vtkStringArray *BlockNames;
  vtkIntArray *CellVarTruthTable;
  bool alreadyDoneExecuteInfo;
  
  // Actual number of arrays that will be generated.
  int NumberOfPointDataArrays;
  // Space allocated in the arrays.
  int PointDataArrayLength;
  // Exodus names of the arrays.
  char **PointDataArrayNames;
  // 1 or Dimensionality of the data set.
  int *PointDataArrayNumberOfComponents;
  // Whether to load the array.
  int *PointDataArrayFlags;  
  
  // Actual number of arrays that will be generated.
  int NumberOfCellDataArrays;
  // Space allocated in the arrays.
  int CellDataArrayLength;
  // Exodus names of the arrays.
  char **CellDataArrayNames;  
  // 1 or Dimensionality of the data set.
  int *CellDataArrayNumberOfComponents;
  // Whether to load the array.
  int *CellDataArrayFlags;  

private:
  vtkVisItExodusReader(const vtkVisItExodusReader&);
  void operator=(const vtkVisItExodusReader&);
};

#endif
