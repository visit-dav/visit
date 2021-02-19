// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// Originating Author:  Richard Cook 925-423-9605 c.2011-01-03
// This file encapsulates the old "dumpfile-based" code, which is capable of 
// very detailed analysis, but cannot work in parallel.

#include <avtSTSDFileFormat.h>
#include "avtparaDISOptions.h"
#include <vtkUnstructuredGrid.h>
#include <string>
#include <vtkFloatArray.h>
#include "ParaDISFileSet.h" 
#include "paradis_c_interface.h"

struct Dumpfile: public ParaDISFileSet {
 public:
 
  Dumpfile(const char *filename, const DBOptionsAttributes *rdatts); 
  ~Dumpfile(); 

  void Clear(void); 
  //void Init(std::string filename, const DBOptionsAttributes *rdatts); 
  bool FileIsValid(void);
  virtual vtkDataSet *GetMesh(std::string meshname);
  virtual vtkDataArray *GetVar(std::string varname); 
  virtual void *GetAuxiliaryData(const char *var, const char *type,
                         DestructorFunction &df);
  uint8_t BurgersTypeToIndex(int btype); 

  /*!
    paraDIS data SERIAL
  */
  std::vector<int> mSegmentBurgerTypes; 
  std::vector<std::string> mNodeNeighborValues, mSegmentBurgerTypeNames, mMetaArmTypes;
  
  double mExtents[6]; // xxyyzz
  uint32_t mNumMetaArmSegments; // computed in GetMesh() to avoid having to recompute at GetVar() 
  int mVerbosity; 
  //int mMaterialSetChoice; 
  std::string mFilename; 
  std::string mDebugFile; 
}; 
