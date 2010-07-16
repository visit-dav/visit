/**
 * @file VsFilter.h
 *
 * @class VsFilter
 *
 * @brief Static class for reading an hdf5 file
 *
 * VsFilter contains a set of static classes that interface
 * with hdf5 recursive reading routines.  VsFilter creates 
 * VsH5Group, VsH5Dataset, and VsH5Attribute objects to
 * represent the contents of the hdf5 file.
 *
 * Copyright &copy; 2008 by Tech-X Corporation
 */
#include <hdf5.h>
#include <visit-hdf5.h>
#if HDF5_VERSION_GE(1, 8, 1)

#ifndef VS_FILTER
#define VS_FILTER

// std includes
#include <string>
#include <vector>

// metadata
#include "VsH5File.h"
#include <hdf5.h>

class VsRegistry;

class VsFilter {

public:
  static VsH5File* readFile(VsRegistry* registry, std::string fileName);

  virtual ~VsFilter() {}
  
protected:
  VsFilter();

  // Visit links, group or dataset
  static herr_t visitLinks(hid_t group, const char* name, const H5L_info_t* linfo, void* opdata);
  static herr_t visitGroup(hid_t group, const char* name, void* opdata);
  static herr_t visitDataset(hid_t group, const char* name, void* opdata);

  // Visit attribute
  static herr_t visitAttrib(hid_t group, const char* name, const H5A_info_t* ai, void* opdata);
};

#endif
#endif

