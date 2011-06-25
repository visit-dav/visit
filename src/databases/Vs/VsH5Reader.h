/**
 * @file  VsH5Reader.h
 *
 * @class VsH5Reader
 *
 * @brief Interface for retrieving structure and data of an hdf5 file.
 * 
 * Top-level class in the VizSchema codebase.  Provides high-level
 * accessors to get data from an hdf5 file in the form of meshes,
 * variables, and attributes.
 *
 * Copyright &copy; 2008 by Tech-X Corporation
 */

#include <hdf5.h>
#include <visit-hdf5.h>

#if HDF5_VERSION_GE(1, 8, 1)

#ifndef VS_H5_READER
#define VS_H5_READER

#include <VsFilter.h>
#include <hdf5.h>
#include <string>

class VsVariable;
class VsVariableWithMesh;
class VsMDMesh;
class VsMesh;
class VsMDVariable;
class VsH5File;
class VsRegistry;
class VsUnstructuredMesh;
class VsStructuredMesh;
class VsUniformMesh;
class VsRectilinearMesh;


/**
 * VsH5Reader is a class for getting the vizschema metadata of objects
 * in and HDF5 file and also being able to return such objects.
 */
class VsH5Reader {

public:
  /**
   * Constructor.
   * @param filename the name of the HDF5 file
   */
  VsH5Reader(const std::string& filename, VsRegistry* r);

  /**
   * Destructor.
   */
  ~VsH5Reader();

  /**
   * Get the values of the whole dataset.
   * Caller owns returned array and must allocate memory and delete it.
   * @param dataset - dataset
   * @param data - pointer to the data
   * @param indexOrder - FORTRAN or C and Minor or Major
   * @param components - -2 (no components) -1 (read all components)
   * @param mins - optional mins (start) array for up to three dimension
   * @param counts - optional counts array for up to three dimension
   * @param strides - optional stride array for up to three dimension
   **/
  herr_t getDataSet( VsH5Dataset* dataset,
                     void* data,
                     std::string indexOrder = std::string(""),
                     int components = 0,
                     int* srcMins = 0,
                     int* srcCounts = 0,
                     int* srcStrides = 0,
                     int  mdims = 0,
                     int* destSizes = 0,
                     int* destMins = 0,
                     int* destCounts = 0,
                     int* destStrides = 0 ) const;    

private:
  VsH5File* fileData;
  
  static int numInstances;
  
  VsRegistry* registry;
};

#endif

#endif
