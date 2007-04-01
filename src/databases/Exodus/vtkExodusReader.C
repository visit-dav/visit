// THIS IS VTK CODE THAT IS NOT PUT IN THE visit_vtk DIRECTORY, SINCE IT
// CONTAINS REFERENCES TO THE EXODUS LIBRARY, AND THAT LIBRARY IS NOT 
// LINKED AGAINST THE VIEWER. 

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkExodusReader.cxx,v $
  Language:  C++
  Date:      $Date: 2001/06/28 18:49:49 $
  Version:   $Revision: 1.16 $


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
#include <vector>
#include <string>
#include "vtkExodusReader.h"
#include <vtkCell.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkIdList.h>
#include <vtkIntArray.h>
#include <vtkFloatArray.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <ctype.h>
#include "netcdf.h"
#include "exodusII.h"


// Things todo.
// 1: Detect XYZ vector and combine.
// 2: Have an option to turn off loading of cell or point arrays.

// 2.5: Load arrays.

// 3: Put in option to have point data array that is index of the point.
// 4: Put in option of element arrays to be global vs local (to element) indexing.
// 5: Filter elements based on piece request.

// Do not reload points and elements when time step chagnes, 
// or the array selection changes.

// Load a range of blocks.
// Filter points that are not used.
// Change Range to be set by update extent.



//----------------------------------------------------------------------------
//  Modifications:
//    Kathleen Bonnell, Wed Mar  6 17:41:23 PST 2002
//    Replace 'New' method with Macro to match VTK 4.0 API.
//----------------------------------------------------------------------------

vtkStandardNewMacro(vtkExodusReader);


//----------------------------------------------------------------------------
//  Modifications:
//    Hank Childs, Sat Apr 17 07:44:05 PDT 2004
//    Initialize times.
//
//    Hank Childs, Sun Jun 27 10:31:18 PDT 2004
//    Initialize GenerateNodeGlobalIdArray.
//
//    Hank Childs, Wed Jul 14 07:49:06 PDT 2004
//    Initialize alreadyDoneExecuteInfo.
//
//----------------------------------------------------------------------------
// Description:
// Instantiate object with NULL filename.
vtkExodusReader::vtkExodusReader()
{
  this->FileName = NULL;
  this->TimeStep = 1;
  this->GenerateBlockIdCellArray = 0;
  this->GenerateElementIdArray = 0;
  this->GenerateElementGlobalIdArray = 0;
  this->GenerateNodeIdArray = 0;
  this->GenerateNodeGlobalIdArray = 0;
  
  this->Title = NULL;
  this->NumberOfBlocks = 0;
  this->NumberOfNodes = 0;
  this->NumberOfTimeSteps = 0;
  this->NumberOfPointDataArrays = 0;
  this->NumberOfCellDataArrays = 0;
  this->NumberOfBlockElements = vtkIntArray::New();
  this->CellVarTruthTable = vtkIntArray::New();
  this->BlockIds = vtkIntArray::New();  
  this->PointDataArrayLength = 0;
  this->PointDataArrayNames = NULL;
  this->PointDataArrayNumberOfComponents = NULL;
  this->PointDataArrayFlags = NULL;
  this->CellDataArrayLength = 0;
  this->CellDataArrayNames = NULL;
  this->CellDataArrayNumberOfComponents = NULL;
  this->CellDataArrayFlags = NULL;

  this->GeometryFileName = NULL;
  this->GeometryCache = vtkUnstructuredGrid::New();

  this->StartBlock = this->EndBlock = -1;
  this->PointMapOutIn = vtkIdList::New();

  this->Times = NULL;
  this->alreadyDoneExecuteInfo = false;
}


//----------------------------------------------------------------------------
//  Modifications:
//    Hank Childs, Sat Apr 17 07:44:05 PDT 2004
//    Destruct times.
//----------------------------------------------------------------------------
vtkExodusReader::~vtkExodusReader()
{
  this->SetFileName(NULL);

  this->SetNumberOfPointDataArrays(0);
  this->SetNumberOfCellDataArrays(0);
  this->NumberOfBlockElements->Delete();
  this->NumberOfBlockElements = NULL;
  this->BlockIds->Delete();
  this->BlockIds = NULL;
  this->CellVarTruthTable->Delete();
  this->CellVarTruthTable = NULL;
  
  if (this->Title)
    {
    delete [] this->Title;
    this->Title = NULL;
    }
  if (this->Times)
    {
    delete [] this->Times;
    this->Times = NULL;
    }
    
  this->SetGeometryFileName(NULL);
  this->GeometryCache->Delete();
  this->GeometryCache = NULL;
  this->PointMapOutIn->Delete();
  this->PointMapOutIn = NULL;
}

//----------------------------------------------------------------------------
void vtkExodusReader::SetNumberOfPointDataArrays(int num)
{
  int idx;
  
  if (this->PointDataArrayLength == num)
    {
    this->NumberOfPointDataArrays = num;
    return;
    }
  
  // Free the previous names.
  for (idx = 0; idx < this->PointDataArrayLength; ++idx)
    {
    delete [] this->PointDataArrayNames[idx];
    this->PointDataArrayNames[idx] = NULL;
    }
  
  // Delete the array.
  if (this->PointDataArrayNames)
    {
    delete [] this->PointDataArrayNames;
    this->PointDataArrayNames = NULL;
    }
  if (this->PointDataArrayNumberOfComponents)
    {
    delete [] this->PointDataArrayNumberOfComponents;
    this->PointDataArrayNumberOfComponents = NULL;
    }
  if (this->PointDataArrayFlags)
    {
    delete [] this->PointDataArrayFlags;
    this->PointDataArrayFlags = NULL;
    }
    
  this->NumberOfPointDataArrays = this->PointDataArrayLength = num;
  if (num == 0)
    {
    return;
    }
  
  // allocate new arrays
  this->PointDataArrayNames = new char*[num];
  this->PointDataArrayNumberOfComponents = new int[num];
  this->PointDataArrayFlags = new int[num];
  for (idx = 0; idx < num; ++idx)
    {
    this->PointDataArrayNames[idx] = new char[MAX_STR_LENGTH+1];
    this->PointDataArrayNumberOfComponents[idx] = 1;
    this->PointDataArrayFlags[idx] = 1;
    }
}

//----------------------------------------------------------------------------
// Combine separate XYZ arrays into vectors.
// To do this, we have to analize names.
int vtkExodusReader::SimplifyArrayNames(char **ArrayNames,
                                        int *ArrayNumComps,
                                        int NumScalarArrays)
{
  int idx1, idx2;
  int len;
  int combineFlag;
  int count;
  
  if (this->Dimensionality != 2 && this->Dimensionality != 3)
    {
    return NumScalarArrays;
    }
  
  idx1 = idx2 = 0;
  count = 0;
  while (idx1 < NumScalarArrays)
    {
    combineFlag = 1;
    len = strlen(ArrayNames[idx1]);
    // Any test fails means we cannot create a vector.
    if (idx1 > NumScalarArrays - this->Dimensionality)
      { // We do not have enough remaining arrays.
      combineFlag = 0;
      }
    else if (strncmp(ArrayNames[idx1],
                     ArrayNames[idx1+1], len - 1) != 0)
      { // The names do not match.
      combineFlag = 0;
      }
    else if (this->Dimensionality == 3 && 
             strncmp(ArrayNames[idx1],
                     ArrayNames[idx1+2], len - 1) != 0)
      { // The names do not match.
      combineFlag = 0;
      }
    else if (ArrayNames[idx1][len-1] != 'X' && 
             ArrayNames[idx1][len-1] != 'x')
      { // The first array name does not end with an X.
      combineFlag = 0;
      }
    else if (ArrayNames[idx1+1][len-1] != 'Y' && 
             ArrayNames[idx1+1][len-1] != 'y')
      { // The second array name does not end with an Y.
      combineFlag = 0;
      }
    else if (this->Dimensionality == 3 &&
             ArrayNames[idx1+2][len-1] != 'Z' && 
             ArrayNames[idx1+2][len-1] != 'z')
      { // The third array name does not end with an Z.
      combineFlag = 0;
      }
    
    if (combineFlag)
      {
      strncpy(ArrayNames[idx2], ArrayNames[idx1], len-1);
      ArrayNames[idx2][len-1] = '\0';
      ArrayNumComps[idx2] = this->Dimensionality;
      idx1 += this->Dimensionality;
      ++idx2;
      }
    else
      {
      strcpy(ArrayNames[idx2], ArrayNames[idx1]);
      ArrayNumComps[idx2] = 1;
      ++idx1;
      ++idx2;
      }
    ++count;
    }
  return count;
}


//----------------------------------------------------------------------------
void vtkExodusReader::SetNumberOfCellDataArrays(int num)
{
  int idx;
  
  if (this->CellDataArrayLength == num)
    {
    this->NumberOfCellDataArrays = num;
    return;
    }
  
  // Free the previous names.
  for (idx = 0; idx < this->CellDataArrayLength; ++idx)
    {
    delete [] this->CellDataArrayNames[idx];
    this->CellDataArrayNames[idx] = NULL;
    }
  
  // Delete the array.
  if (this->CellDataArrayNames)
    {
    delete [] this->CellDataArrayNames;
    this->CellDataArrayNames = NULL;
    }
  if (this->CellDataArrayNumberOfComponents)
    {
    delete [] this->CellDataArrayNumberOfComponents;
    this->CellDataArrayNumberOfComponents = NULL;
    }
  if (this->CellDataArrayFlags)
    {
    delete [] this->CellDataArrayFlags;
    this->CellDataArrayFlags = NULL;
    }
    
  this->NumberOfCellDataArrays = this->CellDataArrayLength = num;
  if (num == 0)
    {
    return;
    }
  
  // allocate new arrays
  this->CellDataArrayNames = new char*[num];
  this->CellDataArrayNumberOfComponents = new int[num];
  this->CellDataArrayFlags = new int[num];
  for (idx = 0; idx < num; ++idx)
    {
    this->CellDataArrayNames[idx] = new char[MAX_STR_LENGTH+1];
    this->CellDataArrayNumberOfComponents[idx] = 1;
    this->CellDataArrayFlags[idx] = 1;
    }
}


void vtkExodusReader::LoadTimes()
{
  float version;
  int CPU_word_size, IO_word_size;
  float fdum;
  char *cdum = NULL;
  CPU_word_size = 0; 
  IO_word_size = 0;
  
  int exoid = ex_open(this->FileName, EX_READ, &CPU_word_size, &IO_word_size,
                  &version);
  if (this->NumberOfTimeSteps == 0)
  {
     int error = ex_inquire(exoid, EX_INQ_TIME, &this->NumberOfTimeSteps,
                        &fdum,cdum);
     if (error < 0)
       {
       vtkErrorMacro("Error: " << error 
                     << " while reading number of time steps from file " 
                     << this->FileName);
       }
  }
  if (this->Times != NULL)
      delete [] Times;
  this->Times = new float[this->NumberOfTimeSteps];
  ex_get_all_times(exoid, this->Times);
  
  ex_close(exoid);
}


//----------------------------------------------------------------------------
// Modifications:
//
//   Hank Childs, Sat Apr 17 07:29:22 PDT 2004
//   Read in the times as well.
//
//   Hank Childs, Wed Jul 14 07:35:25 PDT 2004
//   No longer load the times, since that is so costly.  Also, only call this
//   function one time.
//
//----------------------------------------------------------------------------

void vtkExodusReader::ExecuteInformation()
{
  int exoid;
  int error;
  int CPU_word_size, IO_word_size;
  float version;
  float fdum;
  int num_node_sets, num_side_sets;
  int num_node_vars, num_ele_vars;
  //char *coord_names[3];
  int i;
  int num_elem_in_block;
  int num_nodes_per_elem;
  int num_attr;
  int *ids;
  char *cdum = NULL;
  char elem_type[MAX_STR_LENGTH+1];
  
  if (alreadyDoneExecuteInfo)
      return;
  alreadyDoneExecuteInfo = true;

  CPU_word_size = 0;  // float or double.
  IO_word_size = 0;
  
  exoid = ex_open(this->FileName, EX_READ, &CPU_word_size, &IO_word_size,
                  &version);
  if (exoid < 0)
    {
    vtkErrorMacro("Problem reading information from file " << this->FileName);
    return;
    }
  
  if (this->Title == NULL)
    {
    this->Title = new char[MAX_LINE_LENGTH+1];
    }
  error = ex_get_init(exoid, this->Title, &this->Dimensionality, 
                      &this->NumberOfNodes, &this->NumberOfElements, 
                      &this->NumberOfBlocks, &num_node_sets, &num_side_sets);

  if (error < 0)
    {
    vtkErrorMacro("Error: " << error << " initializing exodus file " << this->FileName);
    }
  
  // Read coordinate names. What will we do with these?
  //for (i = 0; i < num_dim; ++i)
  //  {
  //  coord_names[i] = new char [MAX_STR_LENGTH+1];
  //  }
  //error = ex_get_coord_names(exoid, coord_names);
  //for (i = 0; i < num_dim; ++i)
  //  {
  //  delete [] coord_names[i];
  //  coord_names[i] = NULL;
  //  }
  
  // Read element block paramemters.
  this->NumberOfBlockElements->Reset();
  this->BlockIds->Reset();
  this->BlockIds->SetNumberOfValues(this->NumberOfBlocks);
  ids = this->BlockIds->GetPointer((int)(0));
  error = ex_get_elem_blk_ids (exoid, ids);
  for (i = 0; i < this->NumberOfBlocks; ++i)
    {
    error = ex_get_elem_block (exoid, ids[i], elem_type,
                               &(num_elem_in_block),
                               &(num_nodes_per_elem), &(num_attr));
    if (error < 0)
      {
      vtkErrorMacro("Error: " << error << " reading block information from file " 
                    << this->FileName);
      }
    
    this->NumberOfBlockElements->InsertValue(i, num_elem_in_block);
    }

  // Read the attribute array information.
  error = ex_get_var_param(exoid, "n", &(num_node_vars));
  if (error < 0)
    {
    vtkErrorMacro("Error: " << error << " while reading number of point array from file " 
                  << this->FileName);
    }
  this->SetNumberOfPointDataArrays(num_node_vars);
  if (num_node_vars > 0)
    {
    error = ex_get_var_names(exoid, "n", num_node_vars,
                             this->PointDataArrayNames);
    if (error < 0)
      {
      vtkErrorMacro("Error: " << error << " while reading point array names from file " 
                    << this->FileName);
      }
    this->NumberOfPointDataArrays = 
      this->SimplifyArrayNames(this->PointDataArrayNames, 
                               this->PointDataArrayNumberOfComponents,
                               this->NumberOfPointDataArrays);
    }
  
  
  // Cell array names
  error = ex_get_var_param(exoid, "e", &(num_ele_vars));
  if (error < 0)
    {
    vtkErrorMacro("Error: " << error 
                  << " while reading number of element array from file " 
                  << this->FileName);
    }
  this->SetNumberOfCellDataArrays(num_ele_vars);
  if (num_ele_vars > 0)
    {
    // not all cell variables exist over all element blocks.  A "truth table"
    // will say whether a variable is defined for a certain block.
    this->CellVarTruthTable->Resize(num_ele_vars*this->NumberOfBlocks);
    int *ptr = CellVarTruthTable->GetPointer(0);
    ex_get_elem_var_tab(exoid, this->NumberOfBlocks, num_ele_vars, ptr);

    error = ex_get_var_names (exoid, "e", num_ele_vars, this->CellDataArrayNames);
    if (error < 0)
      {
      vtkErrorMacro("Error: " << error 
                    << " while reading element array names from file " 
                    << this->FileName);
      }

    this->NumberOfCellDataArrays = 
      this->SimplifyArrayNames(this->CellDataArrayNames, 
                               this->CellDataArrayNumberOfComponents,
                               this->NumberOfCellDataArrays);
    }
  
  // Read the number of time steps available.
  error = ex_inquire(exoid, EX_INQ_TIME, &this->NumberOfTimeSteps, &fdum,cdum);
  if (error < 0)
    {
    vtkErrorMacro("Error: " << error << " while reading number of time steps from file " 
                  << this->FileName);
    }
  error = ex_close(exoid);
  if (error < 0)
    {
    vtkErrorMacro("Error: " << error << " closing file " 
                  << this->FileName);
    }
  
  if (this->StartBlock < 0)
    {
    this->StartBlock = 0;
    this->EndBlock = this->NumberOfBlocks - 1;
    }
}

//----------------------------------------------------------------------------
// Modifications:
//
//   Hank Childs, Wed Jul 14 07:35:25 PDT 2004
//   No longer call CheckForProblems -- it doesn't fix anything if there is
//   a problem and takes a long time to execute.
//
//----------------------------------------------------------------------------
void vtkExodusReader::Execute()
{
  int exoid;
  int CPU_word_size, IO_word_size;
  float version;
  char title[MAX_LINE_LENGTH+1];
  int num_dim, num_nodes, num_elem, num_elem_blk;
  int num_node_sets, num_side_sets;
  vtkUnstructuredGrid *output = this->GetOutput();
  int piece, numPieces, startBlock, endBlock;
  
  CPU_word_size = 0;  // float or double.
  IO_word_size = 0;
  
  exoid = ex_open(this->FileName, EX_READ, &CPU_word_size, &IO_word_size,
                  &version);
  if (exoid < 0)
    {
    vtkErrorMacro("Problem opening file " << this->FileName);
    return;
    }
  
  ex_get_init(exoid, title, &num_dim, &num_nodes, &num_elem, 
              &num_elem_blk, &num_node_sets, &num_side_sets);
  // Lets make user these match the informations version.
  if (this->Dimensionality != num_dim || this->NumberOfNodes != num_nodes ||
      this->NumberOfElements != num_elem || this->NumberOfBlocks !=num_elem_blk)
    {
    vtkErrorMacro("Information mismatch.");
    return;
    }
  
  piece = output->GetUpdatePiece();
  numPieces = output->GetUpdateNumberOfPieces();
  if (piece < 0 || piece > numPieces || numPieces < 1)
    { // Requested nothing.
    return;
    }
  
  startBlock = this->NumberOfBlocks * piece / numPieces;
  endBlock = (this->NumberOfBlocks * (piece+1) / numPieces) - 1;

  if (startBlock > endBlock)
    { // Empty piece
    this->StartBlock = startBlock;
    this->EndBlock = endBlock;
    this->GeometryCache->ReleaseData();
    return;
    }
  
  if (this->GeometryFileName == NULL || 
      startBlock != this->StartBlock || endBlock != this->EndBlock ||
      strcmp(this->FileName, this->GeometryFileName) != 0)
    {
    this->StartBlock = startBlock;
    this->EndBlock = endBlock;
    this->ReadGeometry(exoid);
    }
  else
    {
    output->ShallowCopy(this->GeometryCache);
    }
  
  // Read in the arrays.
  // If we are execution then either the file, the time step or array 
  // selections changed.  In all cases we have to reload the arrays.
  this->ReadArrays(exoid);

  // Extra arrays that the reader can generate.
  this->GenerateExtraArrays();
  
  // Reclaim any extra space.
  output->Squeeze();
  
  // Read element block attributes.  
  ex_close(exoid);
}


//----------------------------------------------------------------------------
void vtkExodusReader::ReadGeometry(int exoid)
{
  // It would be better if the cache released when the file name changed.
  this->GeometryCache->ReleaseData();
  this->PointMapOutIn->Reset();
  this->SetGeometryFileName(NULL);
  
  this->ReadCells(exoid);
  this->ReadPoints(exoid);
  
  if (this->GetOutput()->GetReleaseDataFlag() == 0)
    {
    this->SetGeometryFileName(this->FileName);
    this->GeometryCache->ShallowCopy(this->GetOutput());
    }
}



//----------------------------------------------------------------------------
//
// Modifications:
//   Kathleen Bonnell, Mon Oct 29 15:27:41 PST 2001
//   Use vtkIdType for outPtCount to match VTK 4.0 API for retrieving the
//   number of Nodes.
//
//   Hank Childs, Sat Jul 10 12:19:28 PDT 2004
//   Remove virtual function calls for performance.
//
//   Hank Childs, Thu Jul 29 15:30:40 PDT 2004
//   Fix bug with setting up the cell list.
//
//----------------------------------------------------------------------------
void vtkExodusReader::ReadCells(int exoid)
{
  int i, j, k;
  std::vector<int> num_elem_in_block;
  std::vector<int> num_nodes_per_elem;
  std::vector<int> num_attr;
  std::vector<std::string> elem_type;
  char all_caps_elem_type[MAX_STR_LENGTH+1];
  std::vector<int *> connect;
  vtkUnstructuredGrid *output = this->GetOutput();
  int cellType;
  int cellNumPoints;
  int *pointMapInOutArray;
  int inId, outId, *pId;
  vtkIdType outPtCount;
  int len;

  // I think we can do better than this.
  //output->Allocate(this->NumberOfElements);  
  
  // Set up the point maps.
  // Remember which points we have used.
  outPtCount = 0;
  pointMapInOutArray = new int[this->NumberOfNodes];
  pId = pointMapInOutArray;
  if (this->StartBlock > 0 || this->EndBlock < this->NumberOfBlocks-1)
    { // Initialize to use none of them.
    for (inId = 0; inId < this->NumberOfNodes; ++inId)
      {
      *pId++ = -1;
      }
    }
  else
    {
    // Since we are reading the full data set, then preset the maps
    // to avoid shuffleing the points.
    outPtCount = this->NumberOfNodes;
    this->PointMapOutIn->SetNumberOfIds(this->NumberOfNodes);
    for (inId = 0; inId < this->NumberOfNodes; ++inId)
      {
      *pId++ = inId;
      this->PointMapOutIn->SetId(inId, inId);
      }    
    }
  
  // Although information already read these ids ...
  //this->BlockIds->Reset();
  //this->BlockIds->SetNumberOfValues(this->NumberOfBlocks);
  //ids = this->BlockIds->GetPointer((const int)(0));
  //error = ex_get_elem_blk_ids (exoid, ids);

  // Initialize using the type of cells.  
  // A block contains contains only one type of cell.
  // We will go through and get all of the connectivities out of the file
  // first, so we will know how big of an array to allocate this in to.
  for (i = this->StartBlock; i <= this->EndBlock; ++i)
    {
    // Although we read most of this information in ExecuteInformation,
    // we did not save the element types for the blocks.
    char elem_type_tmp[MAX_STR_LENGTH+1];
    int num_elem_in_cur_block, num_nodes_per_cur_elem, cur_num_attr;
    ex_get_elem_block(exoid, this->BlockIds->GetValue(i), elem_type_tmp,
                      &num_elem_in_cur_block, &num_nodes_per_cur_elem,
                      &cur_num_attr);
    elem_type.push_back(elem_type_tmp);
    num_elem_in_block.push_back(num_elem_in_cur_block);
    num_nodes_per_elem.push_back(num_nodes_per_cur_elem);
    num_attr.push_back(cur_num_attr);
    if (num_elem_in_cur_block == 0)
      {
      connect.push_back(NULL);
      }
    else
      {
      int *tmp_conn = new int [num_nodes_per_cur_elem*num_elem_in_cur_block];
      ex_get_elem_conn (exoid, this->BlockIds->GetValue(i), tmp_conn);
      connect.push_back(tmp_conn);
      }
    }

  // Calculate how big the dataset will have to be to hold this and create the
  // VTK unstructured grid.
  int totalSize = 0;
  int numCells = 0;
  for (i = 0; i < connect.size(); i++)
    {
    totalSize += (num_nodes_per_elem[i]+1)*(num_elem_in_block[i]);
    numCells += num_elem_in_block[i];
    }

  vtkIdTypeArray *nlist = vtkIdTypeArray::New();
  nlist->SetNumberOfValues(totalSize);
  vtkIdType *nl = nlist->GetPointer(0);

  vtkUnsignedCharArray *cellTypes = vtkUnsignedCharArray::New();
  cellTypes->SetNumberOfValues(numCells);
  unsigned char *ct = cellTypes->GetPointer(0);

  vtkIntArray *cellLocations = vtkIntArray::New();
  cellLocations->SetNumberOfValues(numCells);
  int *cl = cellLocations->GetPointer(0);

  int idx = 0;
  int currentIndex = 0;
  for (i = this->StartBlock, idx=0 ; i <= this->EndBlock; i++, idx++)
    {
    // cellNumPoints may be smaller than num_nodes_per_elem 
    // because of higher order cells.
    len = strlen(elem_type[idx].c_str());
    for (j = 0 ; j < len ; j++)
      {
      all_caps_elem_type[j] = toupper(elem_type[idx].c_str()[j]);
      }
    if (strncmp(all_caps_elem_type, "HEX", 3) == 0)
      {
      cellType = VTK_HEXAHEDRON;
      cellNumPoints = 8;
      }
    else if (strncmp(all_caps_elem_type, "QUA", 3) == 0)
      {
      cellType = VTK_QUAD;
      cellNumPoints = 4;
      }
    else if (strncmp(all_caps_elem_type,"SPH",3)==0 
             || strncmp(all_caps_elem_type,"CIR",3)==0)
      {
      cellType = VTK_VERTEX;
      cellNumPoints = 1;
      }
    else if (strncmp(all_caps_elem_type, "TRU", 3) == 0)
      {
      cellType = VTK_LINE;
      cellNumPoints = 2;
      }
    else if (strncmp(all_caps_elem_type, "TRI", 3) == 0)
      {
      cellType = VTK_TRIANGLE;
      cellNumPoints = 3;
      }
    else if (strncmp(all_caps_elem_type, "TET", 3) == 0)
      {
      cellType = VTK_TETRA;
      cellNumPoints = 4;
      }
    else if (strncmp(all_caps_elem_type, "WED", 3) == 0)
      {
      cellType = VTK_WEDGE;
      cellNumPoints = 6;
      }
    else
      {
      vtkErrorMacro("Cannot handle type: " << elem_type[idx].c_str() 
                    << " with " << num_nodes_per_elem[idx] << " nodes.");
      continue;
      }
      
    // Now save the cells in a cell array.
    int *pConnect = connect[idx];
    for (j = 0; j < num_elem_in_block[idx]; ++j)
      {
      *ct++ = cellType;
      *cl++ = currentIndex;
      *nl++ = num_nodes_per_elem[i];
      for (k = 0; k < cellNumPoints; ++k)
        {
        // Translate inId to outId and build up point map.
        inId = pConnect[k]-1;  // Exodus stores ids starting form 1 not 0.
        outId = pointMapInOutArray[inId];
        if (outId == -1)
          { // We have not seen this point before.
          outId = pointMapInOutArray[inId] = outPtCount++;
          this->PointMapOutIn->InsertId(outId, inId);
          }
        *nl++ = outId;
        }
      // Skip to next element (may skip more than 8 nodes).
      currentIndex += 1+cellNumPoints;
      pConnect += num_nodes_per_elem[idx];
      }
    }
  
  for (i = 0 ; i < connect.size() ; i++)
    if (connect[i] != NULL)
        delete [] connect[i];

  vtkCellArray *cells = vtkCellArray::New();
  cells->SetCells(numCells, nlist);
  nlist->Delete();

  output->SetCells(cellTypes, cellLocations, cells);
  cellTypes->Delete();
  cellLocations->Delete();
  cells->Delete();

  // From now on we only need the OutIn map.
  delete [] pointMapInOutArray;
  pointMapInOutArray = NULL;
}



//----------------------------------------------------------------------------
//
// Modifications:
//   Kathleen Bonnell, Mon Oct 29 15:27:41 PST 2001
//   Use vtkIdType for outPtCount to match VTK 4.0 API. 
//
//   Hank Childs, Sun Jun 27 10:31:18 PDT 2004
//   Read in the global node ids as well.
//
//   Hank Childs, Sat Jul 10 12:19:28 PDT 2004
//   Remove virtual function calls for performance.
//
//----------------------------------------------------------------------------
void vtkExodusReader::ReadPoints(int exoid)
{
  float *x, *y, *z;
  vtkUnstructuredGrid *output = this->GetOutput();
  vtkPoints *newPoints;
  int inId, outId;
  vtkIdType outPtCount;
  
  // Read points
  x = new float[this->NumberOfNodes];
  y = new float[this->NumberOfNodes];
  if (this->Dimensionality >= 3)
    {
    z = new float[this->NumberOfNodes];
    }
  else
    {
    z = NULL;
    }
  ex_get_coord(exoid, x, y, z);
  // Do we need to reorder these axes based on name?  Probably not.
  newPoints = vtkPoints::New();
  // Although we have to read all of the points,  
  // we are only going to save the points the points that we used.
  outPtCount = this->PointMapOutIn->GetNumberOfIds();
  newPoints->SetNumberOfPoints(outPtCount);
  float *ptr = (float *) newPoints->GetVoidPointer(0);
  for (outId  = 0; outId < outPtCount; ++outId)
    {
    inId = this->PointMapOutIn->GetId(outId);
    // Sanity check
    if (inId < 0 || inId >= this->NumberOfNodes)
      {
      vtkErrorMacro("Point id out of range.");
      }
    
    ptr[3*outId]   = x[inId];
    ptr[3*outId+1] = y[inId];
    ptr[3*outId+2] = (z == NULL ? 0. : z[inId]);
    }
  output->SetPoints(newPoints);
  delete [] x;
  delete [] y;
  if (z)
    {
    delete [] z;
    }
  newPoints->Delete();
  newPoints = NULL;
  
  if (this->GenerateNodeGlobalIdArray != 0)
    {
    int *ids = new int[this->NumberOfNodes];
    ex_get_node_num_map(exoid, ids);

    vtkIntArray *arr = vtkIntArray::New();
    arr->SetName("avtGlobalNodeId");
    outPtCount = this->PointMapOutIn->GetNumberOfIds();
    arr->SetNumberOfTuples(outPtCount);

    for (outId = 0 ; outId < outPtCount ; outId++)
      {
      inId = this->PointMapOutIn->GetId(outId);

      if (inId < 0 || inId >= this->NumberOfNodes)
        {
        vtkErrorMacro("Point id out of range");
        inId = 0;
        }

      arr->SetValue(outId, ids[inId]);
      }

    GetOutput()->GetPointData()->AddArray(arr);
    arr->Delete();
    delete [] ids;
    }
  
  if (output->GetReleaseDataFlag() == 0)
    {
    this->SetGeometryFileName(this->FileName);
    this->GeometryCache->ShallowCopy(output);
    }
}



//----------------------------------------------------------------------------
void vtkExodusReader::ReadArrays(int exoid)
{
  vtkDataArray *array;
  int dim, arrayIdx, idx;
  int vectorFlag;
  int scalarFlag;
  vtkUnstructuredGrid *output = this->GetOutput();
  
  // Read point arrays.
  // The first vector array encounters is set to vectors,
  // and the first array encountered is set to scalars.
  vectorFlag = scalarFlag = 0;
  arrayIdx = 1;
  for (idx = 0; idx < this->NumberOfPointDataArrays; ++idx)
    {
    dim = this->PointDataArrayNumberOfComponents[idx];
    if (this->PointDataArrayFlags[idx])
      { // User wants to load this.
      if (dim == 1)
        {
        array = this->ReadPointDataArray(exoid, arrayIdx);
        if (array == NULL)
          {
          vtkErrorMacro("Problem reading node array " 
                        << this->PointDataArrayNames[idx]);
          // Do not try loading this again
          this->PointDataArrayFlags[idx] = 0;
          }
        else
          {
          array->SetName(this->PointDataArrayNames[idx]);
          if (scalarFlag)
            { // Already have scalars.  Add as array.
            output->GetPointData()->AddArray(array);
            }
          else
            {
            output->GetPointData()->SetScalars(array);
            scalarFlag = 1;
            }
          }
        }
      else if (dim == 2 || dim == 3)
        {
        array = this->ReadPointDataVector(exoid, arrayIdx, dim);
        if (array == NULL)
          {
          vtkErrorMacro("Problem reading node array " 
                        << this->PointDataArrayNames[idx]);
          // Do not try loading this again
          this->PointDataArrayFlags[idx] = 0;
          }
        else
          {
          array->SetName(this->PointDataArrayNames[idx]);
          if (vectorFlag)
            { // Already have vectors.  Add as array.
            output->GetPointData()->AddArray(array);
            }
          else
            {
            output->GetPointData()->SetVectors(array);
            vectorFlag = 1;
            }
          }
        }
      if (array)
        {
        array->Delete();
        array = NULL;
        }
      }
    arrayIdx += dim;
    }

  // Read cell arrays.
  // The first vector array encounters is set to vectors,
  // and the first array encountered is set to scalars.
  vectorFlag = scalarFlag = 0;
  arrayIdx = 1;
  for (idx = 0; idx < this->NumberOfCellDataArrays; ++idx)
    {
    dim = this->CellDataArrayNumberOfComponents[idx];
    if (this->CellDataArrayFlags[idx])
      { // User wants to load this.
      if (dim == 1)
        {
        array = this->ReadCellDataArray(exoid, arrayIdx);
        if (array == NULL)
          {
          vtkErrorMacro("Problem reading element array " 
                        << this->CellDataArrayNames[idx]);
          // Do not try loading this again
          this->CellDataArrayFlags[idx] = 0;
          }
        else
          {
          array->SetName(this->CellDataArrayNames[idx]);
          if (scalarFlag)
            { // Already have scalars.  Add as array.
            output->GetCellData()->AddArray(array);
            }
          else
            {
            output->GetCellData()->SetScalars(array);
            scalarFlag = 1;
            }
          }
        }
      else if ((dim == 2 || dim == 3))
        {
        array = this->ReadCellDataVector(exoid, arrayIdx, dim);
        if (array == NULL)
          {
          vtkErrorMacro("Problem reading element array " 
                        << this->CellDataArrayNames[idx]);
          // Do not try loading this again
          this->CellDataArrayFlags[idx] = 0;
          }
        else
          {
          array->SetName(this->CellDataArrayNames[idx]);
          if (vectorFlag)
            { // Already have vectors.  Add as array.
            output->GetCellData()->AddArray(array);
            }
          else
            {
            output->GetCellData()->SetVectors(array);
            vectorFlag = 1;
            }
          }
        }
      if (array)
        {
        array->Delete();
        array = NULL;
        }
      }
    arrayIdx += dim;
    }
}


//----------------------------------------------------------------------------
// Read the first three vectors and put in vectors as displacement.
//
// Modifications:
//   Kathleen Bonnell, Mon Oct 29 15:27:41 PST 2001
//   Use vtkIdType for outPtCount, pointMapOutInArray, to match VTK 4.0 API. 
//
//----------------------------------------------------------------------------
vtkDataArray *vtkExodusReader::ReadPointDataArray(int exoid, int varIndex)
{
  int error;
  float *x;
  float *pv;
  vtkFloatArray *array;
  int inId, outId;
  vtkIdType outPtCount;
  vtkIdType *pointMapOutInArray;
  
  x = new float[this->NumberOfNodes];
  
  error = ex_get_nodal_var (exoid, this->TimeStep, varIndex, this->NumberOfNodes, x);
  if (error < 0)
    {
    vtkErrorMacro("Error: " << error << " while reading array  from " 
                  << this->FileName);
    delete [] x;
    return NULL;
    }
  
  // Since we are only keeping a portion of the points/values, use a mapping.
  outPtCount = this->PointMapOutIn->GetNumberOfIds();
  pointMapOutInArray = this->PointMapOutIn->GetPointer(0);
  
  array = vtkFloatArray::New();
  array->SetNumberOfValues(outPtCount);
  // Fill in the array.
  pv = (float*)(array->GetVoidPointer(0));
  for (outId = 0; outId < outPtCount; ++outId)
    {
    inId = pointMapOutInArray[outId];
    *pv++= x[inId];
    }
  delete [] x;
  
  return array;
}

//----------------------------------------------------------------------------
// Read the first three vectors and put in vectors as displacement.
//
// Modifications:
//   Kathleen Bonnell, Mon Oct 29 15:27:41 PST 2001
//   Use vtkIdType for outPtCount, pointMapOutInArray to match VTK 4.0 API.
//
//----------------------------------------------------------------------------
vtkDataArray *vtkExodusReader::ReadPointDataVector(int exoid, int startIdx, 
                                                   int dim)
{
  int error;
  float *x, *y, *z;
  float *pv;
  vtkFloatArray *vectors;
  int inId, outId;
  vtkIdType outPtCount;
  vtkIdType *pointMapOutInArray;

  if (dim != 2 && dim != 3)
    {
    vtkErrorMacro("We only handle 2d and 3d vectors.");
    return NULL;
    }
  
  x = new float[this->NumberOfNodes];
  error = ex_get_nodal_var(exoid, this->TimeStep, startIdx,
                           this->NumberOfNodes, x);
  if (error < 0)
    {
    vtkErrorMacro("Error: " << error << " while reading vector componenet from " 
                  << this->FileName);
    delete [] x;
    return NULL;
    }
  
  y = new float[this->NumberOfNodes];
  error = ex_get_nodal_var(exoid, this->TimeStep, startIdx+1,
                           this->NumberOfNodes, y);
  if (error < 0)
    {
    vtkErrorMacro("Error: " << error << " while reading vector componenet from " 
                  << this->FileName);
    delete [] x;
    delete [] y;
    return NULL;
    }

  z = NULL;
  if (dim > 2)
    {
    z = new float[this->NumberOfNodes];
    error = ex_get_nodal_var (exoid, this->TimeStep, startIdx+2, this->NumberOfNodes, z);
    if (error < 0)
      {
      vtkErrorMacro("Error: " << error << " while reading vector componenet from " 
                    << this->FileName);
      delete [] x;
      delete [] y;
      delete [] z;
      return NULL;
      }
    }
  
  // Since we are only keeping a portion of the points/values, use a mapping.
  outPtCount = this->PointMapOutIn->GetNumberOfIds();
  pointMapOutInArray = this->PointMapOutIn->GetPointer(0);
  
  vectors = vtkFloatArray::New();
  vectors->SetNumberOfComponents(3);
  vectors->SetNumberOfTuples(outPtCount);
  // Fill in the vectors
  pv = (float*)(vectors->GetVoidPointer(0));
  for (outId = 0; outId < outPtCount; ++outId)
    {
    inId = pointMapOutInArray[outId];
    *pv++= x[inId];
    *pv++= y[inId];
    if(z)
      {
      *pv++= z[inId];
      }
    else
      {
      *pv++ = 0.0;
      }
    }
  delete [] x;
  delete [] y;
  if (z)
    {
    delete [] z;
    }
  
  return vectors;
}

  
//----------------------------------------------------------------------------
// Read the first three vectors and put in vectors as displacement.
vtkDataArray *vtkExodusReader::ReadCellDataArray(int exoid, int varIndex)
{
  int error, idx, blockIdx, num;
  float *x;
  float *px;
  float *pv;
  vtkFloatArray *array;
  int numElemInPiece;
  
  // Find the total number of elements in the blocks we are loading.
  numElemInPiece = 0;
  for (blockIdx = this->StartBlock; blockIdx <= this->EndBlock; ++blockIdx)
    {
    num = this->NumberOfBlockElements->GetValue(blockIdx);
    numElemInPiece += num;
    }
  
  // Create the arra to collect the block elemet information.
  array = vtkFloatArray::New();
  array->SetNumberOfValues(numElemInPiece);
  pv = (float*)(array->GetVoidPointer(0));

  // Loop through the blocks.
  for (blockIdx = this->StartBlock; blockIdx <= this->EndBlock; ++blockIdx)
    {
    num = this->NumberOfBlockElements->GetValue(blockIdx);
    if (num <= 0)
      {
      continue;
      }
    int index = blockIdx*this->CellDataArrayLength + varIndex;
    if (this->CellVarTruthTable->GetValue(index) == 0)
      {
      for (idx = 0; idx < num; ++idx)
        {
        *pv++= 0.;
        }
      continue;
      }
    // Read in the arrary for this block.
    x = new float[num];
    
    error = ex_get_elem_var (exoid, this->TimeStep, varIndex, 
                             this->BlockIds->GetValue(blockIdx), num, x);
    if (error < 0)
      {
      vtkErrorMacro("Error: " << error << " while reading element array from " 
                    << this->FileName << ", time step = " << this->TimeStep
                    << ", variable index = " << varIndex << ", block id = "
                    << this->BlockIds->GetValue(blockIdx) << ", length = "
                    << num);
      delete [] x;
      array->Delete();
      return NULL;
      }
    // Copy to final array.
    px = x;
    for (idx = 0; idx < num; ++idx)
      {
      *pv++= *px++;
      }
    delete [] x;
    x = NULL;
    }
    
  return array;
}

//----------------------------------------------------------------------------
// Read the first three vectors and put in vectors as displacement.
vtkDataArray *vtkExodusReader::ReadCellDataVector(int exoid, int startIdx, 
                                                  int dim)
{
  int error, idx, blockIdx, num;
  float *x, *y, *z;
  float *px, *py, *pz;
  float *pv;
  vtkFloatArray *array;
  int numElemInPiece;
  
  if (dim != 2 && dim != 3)
    {
    vtkErrorMacro("Only 2d and 3d arrays please.");
    return NULL;
    }
  
  // Find the total number of elements in the blocks we are loading.
  numElemInPiece = 0;
  for (blockIdx = this->StartBlock; blockIdx <= this->EndBlock; ++blockIdx)
    {
    num = this->NumberOfBlockElements->GetValue(blockIdx);
    numElemInPiece += num;
    }  

  // Create the array to collect the block element information.
  array = vtkFloatArray::New();
  array->SetNumberOfComponents(3);
  array->SetNumberOfTuples(numElemInPiece);
  pv = (float*)(array->GetVoidPointer(0));
  z = NULL;
  
  // Loop through the blocks.
  for (blockIdx = this->StartBlock; blockIdx <= this->EndBlock; ++blockIdx)
    {
    num = this->NumberOfBlockElements->GetValue(blockIdx);
    // Read in the arrary for this block.
    x = new float[num];
    error = ex_get_elem_var (exoid, this->TimeStep, startIdx, 
                             this->BlockIds->GetValue(blockIdx), num, x);
    if (error < 0)
      {
      vtkErrorMacro("Error: " << error << " while reading element array  from " 
                    << this->FileName);
      delete [] x;
      array->Delete();
      return NULL;
      }
    y = new float[num];
    error = ex_get_elem_var (exoid, this->TimeStep, startIdx+1, 
                             this->BlockIds->GetValue(blockIdx), num, y);
    if (error < 0)
      {
      vtkErrorMacro("Error: " << error << " while reading element array  from " 
                    << this->FileName);
      
      }
    if (dim > 2)
      {
      z = new float[num];
      error = ex_get_elem_var (exoid, this->TimeStep, startIdx+2, 
                               this->BlockIds->GetValue(blockIdx), num, z);
      if (error < 0)
        {
        vtkErrorMacro("Error: " << error << " while reading element array  from " 
                      << this->FileName);
        delete [] x;
        delete [] y;
        delete [] z;
        array->Delete();
        return NULL;
        }
      }
    // Copy to final array.
    px = x;
    py = y;
    pz = z;
    for (idx = 0; idx < num; ++idx)
      {
      *pv++= *px++;
      *pv++= *py++;
      if (pz)
        {
        *pv++= *pz++;
        }
      else
        {
        *pv++= 0.0;
        }
      }
    delete [] x;
    x = NULL;
    delete [] y;
    y = NULL;
    if (z)
      {
      delete [] z;
      z = NULL;
      }
    }
    
  return array;
}

//----------------------------------------------------------------------------
void vtkExodusReader::GenerateExtraArrays()
{
  vtkIntArray *array;
  vtkUnstructuredGrid *output = this->GetOutput();
  int numCells = output->GetNumberOfCells();
  int numPts = output->GetNumberOfPoints();
  int count, id;
  int blockId;
  int i, j, numBlockElem;
  int offset;
  
  // Node Id
  if (this->GenerateNodeIdArray)
    {
    array = vtkIntArray::New();
    array->SetNumberOfValues(numPts);
    count = 0;
    for (id = 0; id < numPts; ++id)
      {
      // Rember the 1 shift to make the id start at 1.
      array->SetValue(count++,this->PointMapOutIn->GetId(id)+1);
      }
    array->SetName("NodeId");
    output->GetPointData()->AddArray(array);
    array->Delete();
    array = NULL;
    }

  // Block Id
  if (this->GenerateBlockIdCellArray)
    {
    array = vtkIntArray::New();
    array->SetNumberOfValues(numCells);
    count = 0;
    for (i = this->StartBlock; i <= this->EndBlock; ++i)
      {
      numBlockElem = this->NumberOfBlockElements->GetValue(i);
      blockId = this->BlockIds->GetValue(i);
      for (j = 0; j < numBlockElem; ++j)
        {
        array->SetValue(count++,blockId);
        }
      }
    array->SetName("BlockId");
    output->GetCellData()->AddArray(array);
    array->Delete();
    array = NULL;
    }

  // Element Id
  if (this->GenerateElementIdArray)
    {
    array = vtkIntArray::New();
    array->SetNumberOfValues(numCells);
    count = 0;
    for (i = this->StartBlock; i <= this->EndBlock; ++i)
      {
      numBlockElem = this->NumberOfBlockElements->GetValue(i);
      for (j = 0; j < numBlockElem; ++j)
        {
        array->SetValue(count++,j+1);
        }
      }
    array->SetName("ElementId");
    output->GetCellData()->AddArray(array);
    array->Delete();
    array = NULL;
    }

  // Element Global Id
  if (this->GenerateElementGlobalIdArray)
    {
    // Determine the offset (the number of elements skipped).
    offset = 0;
    for (i = 0; i < this->StartBlock; ++i)
      {
      offset += this->NumberOfBlockElements->GetValue(i);
      }
    // Create the array/
    array = vtkIntArray::New();
    array->SetNumberOfValues(numCells);
    count = 0;
    for (i = this->StartBlock; i <= this->EndBlock; ++i)
      {
      numBlockElem = this->NumberOfBlockElements->GetValue(i);
      for (j = 0; j < numBlockElem; ++j)
        {
        array->SetValue(count++,offset+j+1);
        }
      offset += numBlockElem;
      }
    array->SetName("ElementGlobalId");
    output->GetCellData()->AddArray(array);
    array->Delete();
    array = NULL;
    }
}


  
//----------------------------------------------------------------------------
int vtkExodusReader::GetNumberOfElementsInBlock(int blockIdx)
{
  if (blockIdx < 0 || blockIdx >= this->NumberOfBlocks)
    {
    vtkErrorMacro("Block index " << blockIdx 
                  << " is out of range.  The Current number of blocks is : " 
                  << this->NumberOfBlocks);
    return 0;
    }
  
  return this->NumberOfBlockElements->GetValue(blockIdx);
}

//----------------------------------------------------------------------------
int vtkExodusReader::GetBlockId(int blockIdx)
{
  if (blockIdx < 0 || blockIdx >= this->NumberOfBlocks)
    {
    vtkErrorMacro("Block index " << blockIdx 
                  << " is out of range.  The Current number of blocks is : " 
                  << this->NumberOfBlocks);
    return 0;
    }
  
  return this->BlockIds->GetValue(blockIdx);
}

//----------------------------------------------------------------------------
const char *vtkExodusReader::GetPointDataArrayName(int arrayIdx)
{
  if (arrayIdx < 0 || arrayIdx >= this->NumberOfPointDataArrays)
    {
    vtkErrorMacro("PointData array  index " << arrayIdx 
                  << " is out of range.  The Current number of point data arrays is : " 
                  << this->NumberOfPointDataArrays);
    return NULL;
    }
  return this->PointDataArrayNames[arrayIdx];
}

//----------------------------------------------------------------------------
int vtkExodusReader::GetPointDataArrayNumberOfComponents(int arrayIdx)
{
  if (arrayIdx < 0 || arrayIdx >= this->NumberOfPointDataArrays)
    {
    vtkErrorMacro("PointData array  index " << arrayIdx 
                  << " is out of range.  The Current number of point data arrays is : " 
                  << this->NumberOfPointDataArrays);
    return 0;
    }
  return this->PointDataArrayNumberOfComponents[arrayIdx];
}

//----------------------------------------------------------------------------
int vtkExodusReader::GetPointDataArrayLoadFlag(int arrayIdx)
{
  if (arrayIdx < 0 || arrayIdx >= this->NumberOfPointDataArrays)
    {
    vtkErrorMacro("PointData array  index " << arrayIdx 
                  << " is out of range.  The Current number of point data arrays is : " 
                  << this->NumberOfPointDataArrays);
    return 0;
    }
  return this->PointDataArrayFlags[arrayIdx];
}

//----------------------------------------------------------------------------
void vtkExodusReader::SetPointDataArrayLoadFlag(int arrayIdx, int flag)
{
  if (arrayIdx < 0 || arrayIdx >= this->NumberOfPointDataArrays)
    {
    vtkErrorMacro("PointData array  index " << arrayIdx 
                  << " is out of range.  The Current number of point data arrays is : " 
                  << this->NumberOfPointDataArrays);
    return;
    }
  if (this->PointDataArrayFlags[arrayIdx] == flag)
    {
    return;
    }
  this->Modified();
  this->PointDataArrayFlags[arrayIdx] = flag;
}


    
//----------------------------------------------------------------------------
const char *vtkExodusReader::GetCellDataArrayName(int arrayIdx)
{
  if (arrayIdx < 0 || arrayIdx >= this->NumberOfCellDataArrays)
    {
    vtkErrorMacro("CellData array  index " << arrayIdx 
                  << " is out of range.  The Current number of cell data arrays is : " 
                  << this->NumberOfCellDataArrays);
    return NULL;
    }
  return this->CellDataArrayNames[arrayIdx];
}

//----------------------------------------------------------------------------
int vtkExodusReader::GetCellDataArrayNumberOfComponents(int arrayIdx)
{
  if (arrayIdx < 0 || arrayIdx >= this->NumberOfCellDataArrays)
    {
    vtkErrorMacro("CellData array  index " << arrayIdx 
                  << " is out of range.  The Current number of cell data arrays is : " 
                  << this->NumberOfCellDataArrays);
    return 0;
    }
  return this->CellDataArrayNumberOfComponents[arrayIdx];
}

    
//----------------------------------------------------------------------------
int vtkExodusReader::GetCellDataArrayLoadFlag(int arrayIdx)
{
  if (arrayIdx < 0 || arrayIdx >= this->NumberOfCellDataArrays)
    {
    vtkErrorMacro("CellData array  index " << arrayIdx 
                  << " is out of range.  The Current number of cell data arrays is : " 
                  << this->NumberOfCellDataArrays);
    return 0;
    }
  return this->CellDataArrayFlags[arrayIdx];
}    

//----------------------------------------------------------------------------
void vtkExodusReader::SetCellDataArrayLoadFlag(int arrayIdx, int flag)
{
  if (arrayIdx < 0 || arrayIdx >= this->NumberOfCellDataArrays)
    {
    vtkErrorMacro("CellData array  index " << arrayIdx 
                  << " is out of range.  The Current number of cell data arrays is : " 
                  << this->NumberOfCellDataArrays);
    return;
    }
  if (this->CellDataArrayFlags[arrayIdx] == flag)
    {
    return;
    }
  this->Modified();
  this->CellDataArrayFlags[arrayIdx] = flag;
}


//----------------------------------------------------------------------------
void vtkExodusReader::CheckForProblems()
{
  vtkUnstructuredGrid *output = this->GetOutput();
  int numPts = output->GetNumberOfPoints();
  int numCells = output->GetNumberOfCells();
  int i, num;
  vtkDataArray *array;
  int id, ptId;
  vtkCell *cell;
  vtkIdList *ptIds;
  
  // Check cell arrays to see if they have the right number of values.
  num = output->GetCellData()->GetNumberOfArrays();
  for (i = 0; i < num; ++i)
    {
    array = output->GetCellData()->GetArray(i);
    if (array->GetNumberOfTuples() != numCells)
      {
      vtkErrorMacro("CellArray " << array->GetName() << " has "
                    << array->GetNumberOfTuples() 
                    << " tuples, but should have " << numCells);
      }
    }
  
  // Check point arrays to see if they have the right number of values.
  num = output->GetPointData()->GetNumberOfArrays();
  for (i = 0; i < num; ++i)
    {
    array = output->GetPointData()->GetArray(i);
    if (array->GetNumberOfTuples() != numPts)
      {
      vtkErrorMacro("PointArray " << array->GetName() << " has " 
                    << array->GetNumberOfTuples() 
                    << " tuples, but should have " << numPts);
      }
    }
  
  // Check all o fthe cells point ids to see if they are valid.
  for (id = 0; id < numCells; ++id)
    {
    cell = output->GetCell(id);
    ptIds = cell->GetPointIds();
    num = ptIds->GetNumberOfIds();
    for (i = 0; i < num; ++i)
      {
      ptId = ptIds->GetId(i);
      if (ptId < 0 || ptId >= numPts)
        {
        vtkErrorMacro("Bad pt id " << ptId << " in cell "
                      << id);
        }
      }
    }
}

    
  
  
  
//----------------------------------------------------------------------------
void vtkExodusReader::PrintSelf(ostream& os, vtkIndent indent)
{
  int idx;
  
  this->Superclass::PrintSelf(os,indent);

  if (this->GenerateBlockIdCellArray)
    {
    os << indent << "GenerateBlockIdCellArray: On\n";
    }
  else
    {
    os << indent << "GenerateBlockIdCellArray: Off\n";
    }

  if (this->GenerateElementIdArray)
    {
    os << indent << "GenerateElementIdArray: On\n";
    }
  else
    {
    os << indent << "GenerateElementIdArray: Off\n";
    }

  if (this->GenerateElementGlobalIdArray)
    {
    os << indent << "GenerateElementGlobalIdArray: On\n";
    }
  else
    {
    os << indent << "GenerateElementGlobalIdArray: Off\n";
    }

  if (this->GenerateNodeIdArray)
    {
    os << indent << "GenerateNodeIdArray: On\n";
    }
  else
    {
    os << indent << "GenerateNodeIdArray: Off\n";
    }  
  
  os << indent << "File Name: " 
     << (this->FileName ? this->FileName : "(none)") << "\n";
  os << indent << "Title: " 
     << (this->Title ? this->Title : "(none)") << "\n";

  os << indent << "Dimensionality: " << this->Dimensionality << "\n";
  os << indent << "NumberOfNodes: " << this->NumberOfNodes << "\n";
  os << indent << "NumberOfElements: " << this->NumberOfElements << "\n";
  os << indent << "NumberOfBlocks: " << this->NumberOfBlocks << "\n";
  for (idx = 0; idx < this->NumberOfBlocks; ++idx)
    {
    os << indent << "  " << this->NumberOfBlockElements->GetValue(idx)
       << " elements in block " << this->BlockIds->GetValue(idx) << "\n";
    }
  os << indent << "BlockRange: " << this->StartBlock << ", " << this->EndBlock << endl;
  os << indent << "NumberOfTimeSteps: " << this->NumberOfTimeSteps << "\n";
  os << indent << "NumberOfPointDataArrays: " << this->NumberOfPointDataArrays << "\n";
  for (idx = 0; idx < this->NumberOfPointDataArrays; ++idx)
    {
    os << indent << "  " << this->PointDataArrayNames[idx];
    if (this->PointDataArrayNumberOfComponents[idx] != 1)
      {
      os << " " << this->PointDataArrayNumberOfComponents[idx] << " components";
      }
    if (this->PointDataArrayFlags[idx] == 0)
      {
      os << " do not load";
      }
    os << endl;
    }
  os << indent << "NumberOfCellDataArrays: " << this->NumberOfCellDataArrays << "\n";
  for (idx = 0; idx < this->NumberOfCellDataArrays; ++idx)
    {
    os << indent << "  " << this->CellDataArrayNames[idx];
    if (this->CellDataArrayNumberOfComponents[idx] != 1)
      {
      os << " " << this->CellDataArrayNumberOfComponents[idx] << " components";
      }
    if (this->CellDataArrayFlags[idx] == 0)
      {
      os << " do not load";
      }
    os << endl;
    }
}



