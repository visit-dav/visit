/**
 * @file VsFilter.h
 *
 * @class VsFilter
 *
 * @brief Static class for reading an hdf5 file
 *
 * VsFilter contains a set of static classes that interface
 * with hdf5 recursive reading routines.  VsFilter creates 
 * VsGroup, VsDataset, and VsAttribute objects to
 * represent the contents of the hdf5 file.
 *
 * Copyright &copy; 2008 by Tech-X Corporation
 */
#include <hdf5.h>

#ifndef VS_FILTER
#define VS_FILTER

// std includes
#include <string>
#include <vector>

// metadata
#include "VsFile.h"
#include <hdf5.h>

class VsRegistry;

class VsFilter {

public:
  static VsFile* readFile(VsRegistry* registry, std::string fileName);

  virtual ~VsFilter() {}
  
protected:
  VsFilter();

  // Visit links, group or dataset
  static int visitLinks(hid_t group, const char* name, const H5L_info_t* linfo, void* opdata);
  static int visitGroup(hid_t group, const char* name, void* opdata);
  static int visitDataset(hid_t group, const char* name, void* opdata);

  // Visit attribute
  static int visitAttrib(hid_t group, const char* name, const H5A_info_t* ai, void* opdata);
};

#endif

