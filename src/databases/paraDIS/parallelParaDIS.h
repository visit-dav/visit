// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// Originating Author:  Richard Cook 925-423-9605 c.2010-12-29
//  This file is to provide support for the new parallel file format created 
//  by the ParaDIS team in consultation with Rich Cook, in order to 
//  speed up visualization and enable larger data to be visualized. 

#include <vector>
#include <string>
#include <DebugStream.h>
#include <avtSTSDFileFormat.h>
#include "avtparaDISOptions.h"
#include <vtkUnstructuredGrid.h>
#include "ParaDISFileSet.h" 
// ************************************************************************* //
//                            parallelParDIS.h                           //
// ************************************************************************* //

/* 
  //===============================================
  struct FileSet
   The node files and the segment file metadata is identical, so to avoid duplicating code I created this struct.  
   An "elem" is either a node or segment.
  //===============================================
 */ 
struct FileSet {
  //===============================================
  FileSet() { Clear(); }
  //===============================================
  void Clear(void) {
    mNumFiles = 0; 
    mFileNames.clear(); 
    mElemsPerFile.clear(); 
    mBytesPerElem = 0; 
    mFilesAreBinary = false; 
    mDataTypeSizes[0] = mDataTypeSizes[2] = 8; // doubles and longs
    mDataTypeSizes[1] = mDataTypeSizes[3] = 4; //floats and ints
    return; 
  }
  //===============================================
  void AddVar(std::string varname, std::string vartype, int components);
  //===============================================
  bool HaveVar(std::string varname);
  //===============================================
  int VarNum(std::string varname); 
  //===============================================
  std::string VarType(std::string varname); 
  //===============================================
  long NumElems(void); 
  //===============================================
  int VarComponents(std::string varname); 
  //===============================================
  // Returns how many tokens to skip per element to read the variable
  int VarTokenPositionInElement(std::string varname); 
  //===============================================
  // Returns how many bytes to skip per element to read the variable
  int VarBytePositionInElement(std::string varname); 
  //===============================================
  int VarTypeToBytes(std::string typeName); // lookup type in mDataTypeSizes
  //===============================================

  int mNumFiles; 
  std::vector<std::string> mFileNames; 
  std::vector<long> mElemsPerFile; 
  int mBytesPerElem; // to enable reading binary files in large chunks
  bool mFilesAreBinary; 
  int mDataTypeSizes[4]; // sizes of double, float, long, int
  std::vector<std::string> mDataArrayNames; // for display in VisIt
  std::vector<std::string> mDataArrayTypes; // "D"ouble "F"loat "I"nt or "L"ong
  std::vector<int> mBytesPerVar; // including all its components
  std::vector<int> mComponentsPerVar; // components per elem in array, 1 for scalars, or 3 for vectors always I think
}; 

/*
  This is a base class for fetching elements from a FileSet.  It exists to avoid duplicating code that depends on file format, which could very well change pretty soon. 
*/
class ElementFetcher {
 public:
  ElementFetcher (std::string elementName, FileSet *fileset); 
    

  /*!
    // IterateOverFiles: main loop to get elements
  */
  void IterateOverFiles(void *output); 
  /*!
    GetElemsFromTextFile()
    called from IterateOverFiles()
  */
  void GetElemsFromTextFile(std::string filename, long fileOffset, long elems2Read);
  
  /*!
    InterpretTextElement()
    called from GetElemsFromTextFile(), implemented in subclass; interprets a text line representing an element and places it in the right place in mOutputData, incrementing mOutputIndex as well of course.  
  */
  virtual void InterpretTextElement(std::string line, long linenum) = 0; 
  
  /*!
    GetElemsFromBinaryFile()
    called from IterateOverFiles()
  */
  void GetElemsFromBinaryFile(std::string filename, long fileOffset, long elemsToRead);  
  /*!
    InterpretBinaryElement()
    called from GetElemsFromTextFile(), implemented in subclass; interprets the data chunk representing an element and places it in the right place in mOutputData, incrementing mOutputIndex as well of course.  
  */
  virtual void InterpretBinaryElement(char *elementData) = 0;  
  
  int mProcNum, mNumProcs;
  FileSet *mFileSet; 
  void *mOutputData; // something like a vtkDataSet or set of points
  long mOutputIndex; //running count of where we are in the output
  int mElemsPerProc, mStartElement, mEndElement; 
  std::string mElementName, mElementDataType; 
}; 

/*!
  ==================================================
  class MeshElementFetcher
  ==================================================
*/
  class MeshElementFetcher: public ElementFetcher {
 public:
  MeshElementFetcher(std::string meshname, FileSet *fileset): 
    ElementFetcher(meshname, fileset){
    if (meshname == "segments") {
      mPointsPerElement = 2; 
    } else {
      mPointsPerElement = 1; 
    }
    return; 
  }
  vtkDataSet *GetMeshElems(void); 
  virtual void InterpretTextElement(std::string line, long linenum);
  virtual void InterpretBinaryElement(char *elementData);

  int mPointsPerElement;
}; 
/*!
  ==================================================
  VarElementFetcher:  For fetching variables from the dataset
  ==================================================
*/
class VarElementFetcher: public ElementFetcher {
 public:
  VarElementFetcher(std::string varname, FileSet *fileset):
  ElementFetcher(varname, fileset), 
    mNumVarComponents(-1), 
    mVarTokenPositionInElement(-1), mVarBytePositionInElement(-1), 
    mVarBuffer(NULL)
      {
        return; 
      }
  int *GetMaterialElems(void);
  
  vtkDataArray *GetVarElems(void); 
  virtual void InterpretTextElement(std::string line, long linenum);
  
  virtual void InterpretBinaryElement(char *elementData);
  
  int mNumVarComponents, // generally 1 (scalar) or 3 (vector)
    mVarTokenPositionInElement, // first var component  position in element
    mVarBytePositionInElement;  // first var component  position in element
  float *mVarBuffer; // for reading one variable in InterpretTextElement
  
}; 

struct ParallelData: public ParaDISFileSet {
 public: 
 ParallelData(const char *filename = NULL) {
   this->Clear(); 
   if (filename)
     mMetaDataFileName = filename;
   else 
     mMetaDataFileName = "";
   return;
 }
  ~ParallelData();

  void Clear(void); 

  bool ParseMetaDataFile(void); 
  bool ParseFormatString(std::string formatString, FileSet *fileset);
  virtual vtkDataSet *GetMesh(std::string meshname);
  void GetBinaryMeshPoints(vtkPoints *points, long firstPointIndex, std::string filename, long elementOffset, long elementsToRead, int pointsPerElement);  
  void GetTextMeshPoints(vtkPoints *points, long firstPointIndex, std::string filename, long elementOffset, long elementsToRead, int pointsPerElement);  
  virtual vtkDataArray *GetVar(std::string varname); 

  virtual void *GetAuxiliaryData(const char *var, const char *type,
                         DestructorFunction &df);

  // data members:
  std::string mMetaDataFileName; 
  std::string mDataDescription; 
  FileSet mNodeFiles, mSegmentFiles; 
  
}; 

