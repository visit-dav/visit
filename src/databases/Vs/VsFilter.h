#include <hdf5.h>
#include <visit-hdf5.h>
#if HDF5_VERSION_GE(1, 8, 1)
/**
 * @file  VsFilter.h
 *
 * @class VsFilter
 *
 * @brief Interface for inspecting an HDF5 file
 *
 * VsFilter is class for extracting mesh groups, variable datasets,
 * varWithMeshes datasets and mesh datasets and getting their attributes.
 *
 * Copyright &copy; 2008 by Tech-X Corporation
 */

#ifndef VS_FILTER
#define VS_FILTER

// std includes
#include <iostream>
#include <string>
#include <vector>

// metadata
#include <VsH5Meta.h>

#include <hdf5.h>

class VsFilter {

public:

  //  Constructor does nothing
  VsFilter(std::ostream& dbgstrm);

  // This constructor opens a file and creates generic HDF5 metadata
  VsFilter(hid_t fId, std::ostream& dbgstrm);

  virtual ~VsFilter() {}

  // Set a file
  void setFile(hid_t fId);

  // Write up
  void write() const;

  // Get a pointer to internal data
  const VsH5Meta* getH5Meta() {
    return &h5meta;
  }

protected:

  // Visit links, group or dataset
  static herr_t visitLinks(hid_t group, const char* name,
      const H5L_info_t* linfo, void* opdata);

  static herr_t visitGroup(hid_t group, const char* name, void* opdata);
  static herr_t visitDataset(hid_t group, const char* name, void* opdata);

  // Visit attribute
  static herr_t visitAttrib(hid_t group, const char* name,
      const H5A_info_t* ai, void* opdata);

  // Find all groups which are meshes and all datasets that are meshes,
  // variables and variables with meshes and attach their attributes to them
  // Also attach datasets to the groups of interest (meshes).
  // This function looks only for vsType attributes and checks if its value
  // is in the schema.  Then it adds the entity with the correct vsType
  // to meta.
  void makeH5Meta();

  /** Reference to our stream for debugging information */
  std::ostream& debugStrmRef;

  // File id -opened once
  hid_t fileId;

  // List of vs entities
  VsH5Meta h5meta;

};

#endif
#endif

