// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PARADISFILESET
#define PARADISFILESET 1

#include <string>

class ParaDISFileSet {
 public:
  ParaDISFileSet() {
    mBurgersTypes.push_back(std::string("100 arm type"));    
    mBurgersTypes.push_back(std::string("010 arm type"));
    mBurgersTypes.push_back(std::string("001 arm type"));
    mBurgersTypes.push_back(std::string("+++ arm type"));
    mBurgersTypes.push_back(std::string("++- arm type"));
    mBurgersTypes.push_back(std::string("+-+ arm type"));
    mBurgersTypes.push_back(std::string("-++ arm type"));
    mBurgersTypes.push_back(std::string("unknown arm type"));
    
    return; 
  } 

  virtual vtkDataSet *GetMesh(std::string meshname)=0;
  virtual vtkDataArray *GetVar(std::string varname)=0; 
  virtual void *GetAuxiliaryData(const char *var, const char *type,
                         DestructorFunction &df)=0;

  std::vector<std::string> mBurgersTypes;

}; 

#endif
