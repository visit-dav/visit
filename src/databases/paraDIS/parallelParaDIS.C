/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
* Originating Author:  Richard Cook 925-423-9605 c.2010-12-29
*  This file is to provide support for the new parallel file format created 
*  by the ParaDIS team in consultation with Rich Cook, in order to 
*  speed up visualization and enable larger data to be visualized. 
*****************************************************************************/

#include "RC_cpp_lib/stringutil.h"
#include "paraDIS_lib/paradis.h"
#include <DebugStream.h>
#include "parallelParaDIS.h"
#include <vector>
#include <errno.h>
#include <string.h>

#ifdef PARALLEL
#include <avtParallel.h>
#endif
#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>
#include <avtMaterial.h>

#ifdef WIN32
#include <stdio.h>
#define F_SLASH_STRING "/"
#define B_SLASH_STRING "\\"
#define DEV_CHAR ":"
#else
#include <libgen.h>
#endif
#ifdef WIN32
#include <direct.h> /* for _getcwd */
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

using namespace std; 

// ************************************************************************* //
//                            parallelParDIS.C                          //
// ************************************************************************* //

// ****************************************************************************
// parse_dirname
//
//  Purpose: to parse a directory name from the given path.  
//           Added because windows does not have 'libgen.h' (dirname).
//  Stolen from S3D reader. 
//
// ****************************************************************************
static string
parse_dirname(char *wholePath)
{
#ifndef WIN32
    return string(dirname(wholePath)) + "/";
#else
    string wholePathString(wholePath);
    int len = wholePathString.size();
    if (wholePathString[len-1] == DEV_CHAR[0])
        return wholePathString;
    int pos = len;
    string es(".");
    string searchFor = (string)F_SLASH_STRING + (string)B_SLASH_STRING + (string)DEV_CHAR;
    while (pos == len)
    {
        pos = wholePathString.find_last_of(searchFor, --len);
    }
    if (pos == -1)
        return es;
    else if (pos == 0)
        pos++;
    if (wholePathString[pos] == DEV_CHAR[0])
    {
       pos++;
    }
    string rets = wholePathString.substr(0, pos);
    return rets + "\\";
#endif
}



//===============================================
void FileSet::AddVar(string varname, string vartype, int components) {
  mDataArrayNames.push_back(varname); 
  mDataArrayTypes.push_back(vartype); 
  mComponentsPerVar.push_back(components); 
  int bytesInVar = components*VarTypeToBytes(vartype);
  mBytesPerVar.push_back(bytesInVar); 
  mBytesPerElem += bytesInVar; 
  debug4 << "Got variable: \""<< varname << "\", type \""<< vartype << "\" and num components = " << components << ", bytes in var = " << bytesInVar << endl; 
  return;
}

//===============================================
bool FileSet::HaveVar(string varname) {
  return VarNum(varname) != -1; 
}  

//===============================================
int FileSet::VarNum(string varname) {
  int loc = 0, endloc = mDataArrayNames.size();
  while (loc != endloc) {
    if (mDataArrayNames[loc] == varname) {
      debug5 << "VarNum("<<varname<<") = " << loc << endl;
      return loc;
    }
    ++loc;
  }
  return -1; // not found
}    

//===============================================
string FileSet::VarType(string varname) {
  int loc = 0, endloc = mDataArrayNames.size();
  while (loc != endloc) {
    if (mDataArrayNames[loc] == varname) {
      debug5 << "VarType("<<varname<<") = " <<mDataArrayTypes[loc]  << endl;
      return mDataArrayTypes[loc];
    }
    ++loc;
  }
  return "NOTFOUND"; // not found
}    
 
//===============================================
long FileSet::NumElems(void) {
  long fileno = 0, numelems = 0; 
  for (fileno=0; fileno < (long)mElemsPerFile.size(); fileno++){
    numelems += mElemsPerFile[fileno]; 
  }
  debug5 << "NumElems() = " << numelems << endl;
  return numelems; 
}

//===============================================
int FileSet::VarComponents(string varname) {
  int varnum = VarNum(varname); 
  int numc = -1; 
  if (varnum != -1) numc = mComponentsPerVar[varnum]; 
  debug5 << "VarComponents("<<varname<<") = " << numc << endl;
  return numc; 
}


//===============================================
int FileSet::VarTokenPositionInElement(string varname) {
  int varnum = VarNum(varname); 
  if (varnum == -1) return -1; 
  int i = 0, position = 0; 
  while (i < varnum) {
    position += mComponentsPerVar[i++];
  }
  debug5 << "VarTokenPositionInElement("<<varname<<") = " << position << endl;
  return position; 
}

//===============================================
// Returns the size in bytes of the given data type,  "D", "F",  "I", or "L"
inline int FileSet::VarTypeToBytes(string typeName) {
  if (typeName == "D") return mDataTypeSizes[0]; 
  if (typeName == "F") return mDataTypeSizes[1]; 
  if (typeName == "I") return mDataTypeSizes[2]; 
  if (typeName == "L") return mDataTypeSizes[3];
  string msg = string("VarTypeToBytes:  Invalid type: \"")+typeName+"\"";
  EXCEPTION1(VisItException, msg.c_str()); 
  return 0; 
}

//===============================================
// Returns how many bytes to skip per element to read the variable
int FileSet::VarBytePositionInElement(string varname) {
  int varnum = VarNum(varname); 
  if (varnum == -1) return -1; 
  int i = 0, position = 0; 
  while (i < varnum) {
    position += mBytesPerVar[i++];
  }
  debug5 << "VarBytePositionInElement("<<varname<<") = " << position << endl;
  return position; 
}


//=====  END FileSet ======================================

//======================================================
/*
  ElementFetcher constructor
*/
ElementFetcher::ElementFetcher (std::string elementName, FileSet *fileset): 
  mFileSet(fileset),  mOutputData(NULL), mOutputIndex(0), 
  mElementName(elementName) {
  
#ifdef PARALLEL
  /*!  I keep this here to remind myself how to use MPI.  LOL
    MPI_Comm_rank(MPI_COMM_WORLD, &mProcNum); 
    MPI_Comm_size(MPI_COMM_WORLD, &mNumProcs);  
  */
  // This is the Visit Way: 
  mProcNum = PAR_Rank(); 
  mNumProcs = PAR_Size(); 
#else
  mProcNum = 0; 
  mNumProcs = 1; 
#endif

  long numelems = mFileSet -> NumElems(); 
  mElemsPerProc = ((float)numelems)/(float)mNumProcs; // close enough
  mStartElement = mProcNum * mElemsPerProc; 
  mEndElement = (mProcNum + 1)*mElemsPerProc-1;
  if (mEndElement > numelems-1) { 
    mEndElement = numelems-1;
  }
  debug2<< "ElementFetcher::ComputeWorkingSubset: Processor " << mProcNum << " mStartElement = " << mStartElement << ", mEndElement = " << mEndElement << ", mElemsPerProc = " << mElemsPerProc << ", totalElems = " << numelems << endl; 
  
  return; 
}

//======================================================
/*
  ElementFetcher::IterateOverFiles()
 */
void ElementFetcher::IterateOverFiles(void *output) {

  mOutputData = output; 
  
  // ITERATE OVER FILES
  // look at each file in order, and determine if it contains elements we care about.  If it does, then read them and place them into the mesh. 
  long fileno = 0 /*, outputItemIndex = 0*/,
    elementsCompleted = 0; //elements from previous files
  for (fileno = 0;
       fileno < (long)mFileSet->mFileNames.size() &&
         elementsCompleted < mEndElement + 1;
       fileno++) {
    //  int elemsInFile = mFileSet->mElemsPerFile[fileno];
    long elementsToReadInFile = mFileSet->mElemsPerFile[fileno];
    if (elementsCompleted + elementsToReadInFile > mStartElement) {
      // at least one element in the file is of interest
      long fileOffset = 0;
      if (mStartElement >= elementsCompleted) {
        // the mStartElement is somewhere in our file
        fileOffset = mStartElement - elementsCompleted;
        elementsCompleted = mStartElement; // mark offset as being complete
      }

      elementsToReadInFile -= fileOffset;

      if (elementsCompleted + elementsToReadInFile > mEndElement) {
        // the mEndElement is inside our file; don't read all file elements
        elementsToReadInFile = mEndElement - elementsCompleted + 1;
      }

      if (mFileSet->mFilesAreBinary) {
        GetElemsFromBinaryFile(mFileSet->mFileNames[fileno], fileOffset, elementsToReadInFile);
      } else {
        GetElemsFromTextFile(mFileSet->mFileNames[fileno], fileOffset, elementsToReadInFile);
      }
    }
    elementsCompleted += elementsToReadInFile;
  }
  return;
}


/* 
   ParallelData::GetBinaryMeshPoints
   Fill "points" with a bunch of pairs of points from the given binary data file at the given offset, starting with firstPointIndex and continuing until elementsToRead are done.  Read pointsPerElement points per element.  
*/ 
void ElementFetcher::GetElemsFromBinaryFile(std::string filename, long fileOffset, long elementsToRead){
  debug2 << "ElementFetcher::GetElemsFromBinaryFile(" << filename << ", "<< fileOffset<<", "<< elementsToRead << ")" << endl; 
  FILE *fp = fopen(filename.c_str(), "rb"); 
  if (!fp) {
    string msg = string("Error, cannot open file: ")+filename;
    EXCEPTION1(VisItException, msg.c_str()); 
  }
  long byteOffset = mFileSet->mBytesPerElem * fileOffset; 
  if (fseek(fp, byteOffset, SEEK_SET) == -1) {
    string msg = string("Error, cannot seek ")+intToString(byteOffset)+" bytes into file: "+filename+string(" (")+strerror(errno)+string(")");
    fclose(fp);
    EXCEPTION1(VisItException, msg.c_str()); 
  }

  long bufferLength = mFileSet->mBytesPerElem*1000; 
  char *buffer = new char[bufferLength]; 

  long bytesToRead = elementsToRead * mFileSet->mBytesPerElem; 

  while (bytesToRead) { 
    long chunkBytes = bytesToRead > bufferLength ? bufferLength: bytesToRead; 
    long chunkElems = chunkBytes/mFileSet->mBytesPerElem; 
    if (fread((void*)buffer, mFileSet->mBytesPerElem, chunkElems, fp) == (unsigned int)-1) /* TODO: check fix for fread return is size_t which cannot be -1 */{
      string msg = string("Error, cannot read ")+intToString(chunkBytes)+" bytes from file: "+filename+string(" (")+strerror(errno)+string(")");
      fclose(fp);
      EXCEPTION1(VisItException, msg.c_str()); 
    }
    char *bufp = buffer; 
    //long elemNum = 0; 
    while (chunkElems--) {
      InterpretBinaryElement(bufp); 
      bufp += mFileSet->mBytesPerElem; 
    }
    bytesToRead -= chunkBytes; 
  }
  debug2 << "ElementFetcher::GetElemsFromBinaryFile complete " << endl;
  fclose(fp);
  return; 
}
/* 
   ParallelData::GetTextMeshPoints
   Fill "points" with a bunch of pairs of points from the given text data file at the given offset, starting with firstPointIndex and continuing until elementsToRead are done.  Read pointsPerElement points per element.  
*/ 
void ElementFetcher::GetElemsFromTextFile(std::string filename, long fileOffset, long elementsToRead){
  ifstream datafile (filename.c_str()); 
  string line; 
  debug4 << "ParallelData::GetTextMeshPoints for file " << filename << ", mOutputIndex = " << mOutputIndex << ", fileOffset = " << fileOffset << " and elementsToRead is " << elementsToRead << endl; 
  if (!datafile.is_open()) {
    string msg = string("Error, cannot open file: ")+filename;
    EXCEPTION1(VisItException, msg.c_str()); 
  }

  long linenum = 0;
  for (linenum=0; linenum < fileOffset && datafile.good(); linenum++) {
    getline(datafile, line); 
  }
  if (!datafile.good()) {
    string msg = string("Error in file ")+filename+string(", line ")+intToString(linenum)+string(": cannot seek to line ")+intToString(fileOffset);
    EXCEPTION1(VisItException, msg.c_str()); 
  }
  
  long elementNum = 0; 
  for (elementNum =0; elementNum < elementsToRead && datafile.good(); linenum++, elementNum++) {
    getline(datafile, line); 
    InterpretTextElement(line, linenum); 
  }
  if (elementNum < elementsToRead) {
    string msg = string("Error in file ")+filename+string(", line ")+intToString(linenum)+string(": unknown error ");
    EXCEPTION1(VisItException, msg.c_str()); 
  }
  
  return; 
}

//=============================================================
/*
  MeshElementFetcher::GetMeshElems
*/

vtkDataSet * MeshElementFetcher::GetMeshElems(void) {
  /*
    First, we have to populate the points, regardless of whether this is a point mesh (nodes) or a line mesh (segments)
  */ 

  mElementDataType = mFileSet->mDataArrayTypes[0]; 

  vtkPoints *points =  vtkPoints::New();
  long numelems = mEndElement - mStartElement + 1; 
  long  pointsPerElement = 0;  (void) pointsPerElement;
  
  if (mElementName == "nodes") {
    points->SetNumberOfPoints(numelems);
    pointsPerElement = 1; 
  }
  else if  (mElementName == "segments") {
    points->SetNumberOfPoints(numelems*2);
    pointsPerElement = 2; 
  }
  else { 
    EXCEPTION1(VisItException, "Unknown mesh name requested"); 
  }

  // Fill in the points
  IterateOverFiles((void*)points); 

  vtkUnstructuredGrid *theMesh = vtkUnstructuredGrid::New();

  theMesh->SetPoints(points);  
  points->Delete(); // decrement reference count

  if (mElementName == "segments") {
    // create the elements
    long segnum = 0; 
    vtkIdType nodeIndices[2]; 
    for (segnum =0; segnum < mEndElement-mStartElement+1; segnum++) {
      nodeIndices[0] = segnum*2;
      nodeIndices[1] = segnum*2+1;
      theMesh->InsertNextCell(VTK_LINE, 2, nodeIndices);
    }
  } // end segment mesh creation
  else { // if (mElementName == "nodes") 
    // create a node mesh
    long nodenum = 0; 
    vtkIdType nodeIndex[1]; //has to be array type
    while (nodenum  < mEndElement-mStartElement) {
      nodeIndex[0] = nodenum; 
      debug5 << "Inserting node index " << nodenum << " = " <<  nodeIndex[0] << endl;        
      theMesh->InsertNextCell(VTK_VERTEX, 1, nodeIndex);       
      ++nodenum;
    }
  }      
  debug1 << "ParallelData::GetMesh: Ending GetMesh" << endl; 
  return theMesh;
}

//=============================================================
/*
  MeshElementFetcher:: InterpretTextElement
  Interprets the line as a segment or a node and adds it to mOutputData.  
  increments mOutputIndex by one if a node, two if a segment. 
 */

inline void MeshElementFetcher:: InterpretTextElement(std::string line, long linenum) {
  
  debug5 << "MeshElementFetcher::InterpretTextElement parsing line " << linenum << ": \"" << line << "\"" << endl;  
  vector<string> linetokens;
  float location[3] ; 
  linetokens = Split(line); 
  if (linetokens.size() < (size_t)3*mPointsPerElement) {
    string msg = string("Error in line ")+intToString(linenum)+string(": not enough tokens in line ");
    EXCEPTION1(VisItException, msg.c_str()); 
  }
  long lineIndex = 0; 
  for (lineIndex = 0; lineIndex < 3*mPointsPerElement; ) {
    location[0] = strtod(linetokens[lineIndex++].c_str(), NULL); 
    location[1] = strtod(linetokens[lineIndex++].c_str(), NULL); 
    location[2] = strtod(linetokens[lineIndex++].c_str(), NULL); 
    debug5 << "Setting point " << mOutputIndex << " to (" << location[0] << ", " << location[1] << ", " << location[2] << ")" << endl; 
    ((vtkPoints *)mOutputData)->SetPoint(mOutputIndex++, location); 
    
  }
  return; 
}

//=============================================================
/*
  MeshElementFetcher:: InterpretBinaryElement
  Interprets the buffer as a segment or a node and adds it to mOutputData.  
  increments mOutputIndex by the correct amount 
 */
inline void MeshElementFetcher::InterpretBinaryElement(char *elementData){
  float location[3]; 
  int numpts = mPointsPerElement; 
  //int component = 0; 
  while (numpts--) {
    if (mElementDataType == "D") {
      double *dp = (double*)elementData; 
      location[0] = dp[0]; 
      location[1] = dp[1]; 
      location[2] = dp[2]; 
      elementData += 3*sizeof(double); 
    } else {
      memcpy(location, elementData, 3*sizeof(float)); 
      elementData += 3*sizeof(float); 
    }   
    debug5 << "Setting point " << mOutputIndex << " to (" << location[0] << ", " << location[1] << ", " << location[2] << ")" << endl; 
    ((vtkPoints *)mOutputData)->SetPoint(mOutputIndex++, location); 
    
  }
  return; 
}

//=============================================================
/*
  VarElementFetcher::GetMaterialElems
*/

int * 
VarElementFetcher::GetMaterialElems(void) {
  debug3 << "VarElementFetcher::GetMaterialElems()" << endl;

  string virtualName = mElementName; 
  if (virtualName == "Burgers type" ) {
    virtualName = "BurgersVec"; // to ensure reading the correct variable
  } else {
    debug1 << "Error:  unknown material: " << virtualName << endl; 
    return NULL; 
  }

  mElementDataType = mFileSet->VarType(virtualName); 
  mNumVarComponents = mFileSet->VarComponents(virtualName); 
  mVarTokenPositionInElement = mFileSet->VarTokenPositionInElement(virtualName); 
  mVarBytePositionInElement = mFileSet->VarBytePositionInElement(virtualName); 

  debug4 << "mNumVarComponents = " << mNumVarComponents << ", mVarTokenPositionInElement = " << mVarTokenPositionInElement << ", mVarBytePositionInElement = " << mVarBytePositionInElement << endl; 

  mVarBuffer = new float[mNumVarComponents]; 
  long numelems = mEndElement - mStartElement + 1;
  int *matId = new int[numelems];

  IterateOverFiles((void*)matId); 
  
  delete mVarBuffer; 
  mVarBuffer = NULL; 
  
  return matId; 
}

//=============================================================
/*
  VarElementFetcher::GetVarElems
*/

vtkDataArray * VarElementFetcher::GetVarElems(void) {
  debug2 << "Beginning VarElementFetcher::GetVarElems" << endl;   
  
  string virtualName = mElementName; 
  mElementDataType = mFileSet->VarType(virtualName); 
  mNumVarComponents = mFileSet->VarComponents(virtualName); 
  mVarTokenPositionInElement = mFileSet->VarTokenPositionInElement(virtualName); 
  mVarBytePositionInElement = mFileSet->VarBytePositionInElement(virtualName); 

  debug4 << "mNumVarComponents = " << mNumVarComponents << ", mVarTokenPositionInElement = " << mVarTokenPositionInElement << ", mVarBytePositionInElement = " << mVarBytePositionInElement << endl; 
  mVarBuffer = new float[mNumVarComponents]; 
  //long index = 0;
  //float f=0.0; 

  vtkFloatArray *tuples = vtkFloatArray::New(); 

  long numelems = mEndElement - mStartElement + 1;
  tuples->SetNumberOfComponents(mNumVarComponents); 
  
  tuples->SetNumberOfTuples(numelems); 

  IterateOverFiles((void*)tuples); 

  delete mVarBuffer; 
  mVarBuffer = NULL; 
  return tuples; 
}


//=============================================================
/*
  VarElementFetcher:: InterpretTextElement
  Interprets the line as a segment or a node and adds it to mOutputData.  
  increments mOutputIndex by one if a node, two if a segment. 
 */
void VarElementFetcher:: InterpretTextElement(std::string line, long linenum) {
  debug5 << "VarElementFetcher:: InterpretTextElement parsing line " << linenum << ": \"" << line << "\"" << endl;  
  vector<string> linetokens;
  linetokens = Split(line); 
  if (linetokens.size() < (size_t)mVarTokenPositionInElement+mNumVarComponents) {
    string msg = string("Error in line ")+intToString(linenum)+string(": not enough tokens in line to get variable values");
    EXCEPTION1(VisItException, msg.c_str()); 
  }
  debug5 << "Element " << mOutputIndex << " value = ("; 
  long i=0; 
  for (i=0; i < mNumVarComponents; ++i ) {
    mVarBuffer[i] = strtod(linetokens[i+mVarTokenPositionInElement].c_str(), NULL);
    debug5 << mVarBuffer[i] << ", ";     
  }
  debug5 << ")" << endl;
  if (mElementName == "Burgers type" ) {
    ((int*)mOutputData)[mOutputIndex++] = InterpretBurgersType(mVarBuffer); 
  } else {
    ((vtkFloatArray *)mOutputData)->SetTuple(mOutputIndex++, mVarBuffer); 
  }
  
  return;
}


//=============================================================
/*
  MeshElementFetcher:: InterpretBinaryElement
  Interprets the buffer as a segment or a node and adds it to mOutputData.  
  increments mOutputIndex by the correct amount 
 */
inline void VarElementFetcher::InterpretBinaryElement(char *elementData){
  long i=0;
  elementData += mVarBytePositionInElement;
  debug5 << "Element " << mOutputIndex << " value = ("; 
  for (i=0; i < mNumVarComponents; ++i ) {
    if (mElementDataType == "D") {
      mVarBuffer[i] = ((double*)elementData)[i]; 
    } else if (mElementDataType == "F") {
      mVarBuffer[i] = ((float*)elementData)[i]; 
    } else if (mElementDataType == "I") {
      mVarBuffer[i] = ((int*)elementData)[i]; 
    } else if (mElementDataType == "L") {
      mVarBuffer[i] = ((long*)elementData)[i]; 
    } 
    debug5 << mVarBuffer[i] << ", "; 
  }
   
  debug5 << ")" << endl;

  if (mElementName == "Burgers type" ) {
    ((int*)mOutputData)[mOutputIndex++] = InterpretBurgersType(mVarBuffer); 
  } else {
    ((vtkFloatArray *)mOutputData)->SetTuple(mOutputIndex++, mVarBuffer); 
  }
  return; 
}


//=============================================================
/*
  ParallelData initializer
 */
void ParallelData::Clear(void) {
  mMetaDataFileName = "";
  mDataDescription = ""; 
  mNodeFiles.Clear(); 
  mSegmentFiles.Clear(); 
  return; 
}
//=============================================================
/*
  ParallelData destructor
 */
ParallelData::~ParallelData() {
  return; 
}

//=============================================================
/*
  ParallelData:: ParseMetaDataFile
  
 */
bool ParallelData:: ParseMetaDataFile(void) {
  debug2 << "ParallelData:: ParseMetaDataFile " << mMetaDataFileName << endl;
  string cwd = "./"; 
  char buf[2048];
  if (!getcwd(buf, 2048)) {
    debug1 << "Warning: cannot get current working directory -- this will cause errors if reading a meta data file in another directory other than your current working directory." << endl; 
  } else {
    cwd = string(buf) + "/"; 
  }
#ifndef WIN32
  if (mMetaDataFileName[0] != '/' )
#else
  if (mMetaDataFileName.size() > 1 && mMetaDataFileName[1] != ':' )
#endif
  {
    debug2 << "mMetaDataFileName " << mMetaDataFileName; 
    mMetaDataFileName = cwd + "/" + mMetaDataFileName; 
    debug2 << " changed to " << mMetaDataFileName << endl;  
  }
  strncpy(buf, mMetaDataFileName.c_str(), 2047); 
  string datadir = parse_dirname(buf); 

  string line;
  vector<string> linetokens;
  bool goodversion = false;  (void) goodversion;
  FileSet *fileset = &mNodeFiles; 
  ifstream myfile (mMetaDataFileName.c_str());
  if (!myfile.is_open()) {
    return false; 
  }
    
  bool parsingDescription = false; 
  long linenum = 0;
  while ( ++linenum && myfile.good() ) {
    getline (myfile,line);
    debug3 << "Parsing line " << linenum << ": \"" << line << "\"" << endl;  
    linetokens = Split(line); 

    //=======================================================
    if (!linetokens.size() || linetokens[0][0] == '#') {
      debug2 << "Comment or blank line found" << endl; 
      continue; 
    }

    //=======================================================
    if (line.find("PARADIS") != string::npos && 
        line.find("PARALLEL") != string::npos && 
        line.find("1.0") != string::npos) {
      debug2 << "Found good version" << endl; 
      goodversion = true; 
      continue; 
    }

    //=======================================================
    if (linetokens[0] == "DESCRIPTION") {
      string:: size_type quote = line.find("\""); 
      if (quote == string::npos) {
        debug1 << "Bad description line" << endl; 
        return false; 
      }
      parsingDescription = true; // keep parsing until endquote found!
      line = line.substr(quote+1);
    }
    if (parsingDescription) {
      string:: size_type quote = line.find("\"");
      if (quote != string::npos) {
        line = line.substr(0, quote); 
        parsingDescription = false; 
      }
      mDataDescription += line;     
      continue; 
    }

    //=======================================================
    if (linetokens[0] == "NODE") {
      if (linetokens.size() < 3 || linetokens[1] != "FILES") {
        debug1 << "Bad NODE line" << endl; 
        return false; 
      }
      fileset = &mNodeFiles; 
      continue; 
    }
    
    //=======================================================
    if (linetokens[0] == "SEGMENT") {
      if (linetokens.size() < 3 || linetokens[1] != "FILES") {
        debug1 << "Bad SEGMENT line" << endl; 
        return false; 
      }
      fileset = &mSegmentFiles; 
      continue; 
    }
    
    //=======================================================
    if (linetokens[0] == "TEXT") {
      fileset->mFilesAreBinary = false; 
      continue; 
    } 

    //=======================================================
    if (linetokens[0] == "BINARY") {
      long numtokens = linetokens.size(); 
      long tokennum; 
      for (tokennum = 1; tokennum < numtokens; ++tokennum) {
        vector<string> typetuple = Split(linetokens[tokennum], '='); 
        if (typetuple.size() != 2) {
          debug1 << "Bad BINARY tuple: " << linetokens[tokennum] << endl; 
          return false; 
        }
        long typeSize = atol(typetuple[1].c_str()); 
        long typenum = -1; 
        if (typetuple[0] == "D") {
          debug3 << "sizeof(D-double)=" << typeSize << endl; 
          typenum = 0; 
        } else if (typetuple[0] == "F") {
          debug3 << "sizeof(F-float)=" << typeSize << endl; 
          typenum = 1; 
        } else if (typetuple[0] == "I") {
          debug3 << "sizeof(I-int)=" << typeSize << endl; 
          typenum = 2; 
        } else if (typetuple[0] == "L") {
          debug3 << "sizeof(L-long)=" << typeSize << endl; 
          typenum = 3; 
        } else {
          debug1 << "Bad BINARY tuple: " << linetokens[tokennum] << endl; 
          return false; 
        }         
        fileset->mDataTypeSizes[typenum] = typeSize; 
      }
      fileset->mFilesAreBinary = true; 
      continue;
    }
    
    //=======================================================
    if (linetokens[0] == "FORMAT") {
      if (!ParseFormatString(line, fileset)) {
        return false; 
      }
      continue;
    }

    //=======================================================
    if (linetokens[0] == "FILE") {
      if (linetokens.size() != 4) {
        debug1 << "Bad FILE line: " << line << endl; 
        return false; 
      }        
      string filename = linetokens[1]; 
      if (filename[0] != '/'){ // this won't work on Windows
        filename = datadir + filename; 
      }
      fileset->mFileNames.push_back(filename); 
      fileset->mElemsPerFile.push_back(atol(linetokens[3].c_str())); 
      continue;
    }

    // If we get here, there is a problem with the current line -- it is indecipherable
    debug1 << "Error: indecipherable line in file:  " << line << endl; 
    return false; 
  }
  myfile.close();

  debug1 << "Parallel ParaDIS metadatafile parsed successfully" << endl; 

  return true; 
}


/*
  This function takes a format string from the metadata file as input.
  It creates the appropriate metadat from the string.  
  The string must not be terminated by a '\n'.
*/
bool ParallelData::ParseFormatString(string formatString, FileSet *fileset) { 
  debug4 << "ParallelData::ParseFormatString called" << endl; 
  vector<string> formatStringTokens = Split(formatString); 
  long numtokens = formatStringTokens.size(); 
  if (numtokens < 2) {
    debug1 << "Not enough tokens in FORMAT line in nodal metadata -- expect at least two, including FORMAT token\n"; 
    return false; 
  }
  long tokenNum = 1; 
  for (tokenNum = 1; tokenNum < numtokens; ++tokenNum) {
    string token = formatStringTokens[tokenNum];     
    std::string::size_type startparen = token.find("("), 
      colon = token.find(":"), 
      endparen =token.find(")"); 
    if (startparen == string::npos || colon == string::npos || endparen == string::npos) {
      debug1 << string("Bad format token: \"")+token+string("\"\n"); 
      return false; 
    }
    string varname = token.substr(0,startparen), 
      vartype = token.substr(startparen+1, colon-startparen-1), 
      componentstr = token.substr(colon+1, endparen-colon-1); 

    long components = atol(componentstr.c_str()); 
    fileset->AddVar(varname, vartype, components); 
  }
  return true;    
}

/*
GetMesh()
*/
vtkDataSet *ParallelData::GetMesh(std::string meshname) {
  debug2 << "ParallelData::GetMesh("<<meshname<<")"<<endl;
  FileSet *fileset = NULL; 
  if (meshname == "segments") {
    fileset = &mSegmentFiles; 
  } else if (meshname == "nodes") {
    fileset = &mNodeFiles; 
  }
  if (!fileset) {
    EXCEPTION1(VisItException, (string("ParallelData::GetMesh is unable to find a fileset that contains the mesh \"")+meshname+"\".  :-(").c_str()); 
  }
  MeshElementFetcher mef(meshname, fileset); 
  return mef.GetMeshElems(); 
}

/*
  GetVar()
*/
vtkDataArray *ParallelData::GetVar(string varname) {
  debug2 <<"ParallelData::GetVar(" << varname << ")" << endl; 

  FileSet *fileset = NULL; 
  if (mNodeFiles.HaveVar(varname)) {
    fileset = &mNodeFiles;
  } else if (varname == "Burgers type" || mSegmentFiles.HaveVar(varname)) {
    fileset = &mSegmentFiles;
  } 
  if (!fileset) {
    debug1 << "WARNING ParallelData::GetVar is unable to find a fileset that contains the variable requested" << endl; 
    return NULL; 
  }

  VarElementFetcher vef(varname, fileset); 
  return vef.GetVarElems(); 
}

/*!
  ParallelData::GetAuxiliaryData
*/
void *
ParallelData::GetAuxiliaryData(const char *var, const char *type,
                             DestructorFunction &df) {
  debug3 << "ParallelData::GetAuxiliaryData("<<var<<", "<<type<< ") returning NULL"<<endl;
  if (strcmp(type, AUXILIARY_DATA_MATERIAL) != 0) {
    return NULL;
  }

  string varname(var); 
  FileSet *fileset = NULL; 
  if (mNodeFiles.HaveVar(varname)) {
    fileset = &mNodeFiles;
  } else if (varname == "Burgers type" || mSegmentFiles.HaveVar(varname)) {
    fileset = &mSegmentFiles;
  } 
  if (!fileset) {
    debug1 << "WARNING ParallelData::GetVar is unable to find a fileset that contains the variable requested" << endl; 
    return NULL; 
  }

  VarElementFetcher vef(varname, fileset); 
  long numelems = vef.mEndElement - vef.mStartElement + 1; 
  avtMaterial *mat = 
    new avtMaterial(mBurgersTypes.size(), mBurgersTypes, 
                    numelems, vef.GetMaterialElems(), 0, 
                    NULL, NULL, NULL, NULL);  
  df = avtMaterial::Destruct;

  return mat; 
} 
