/**
 * @file  VsH5Reader.h
 *
 * @class VsH5Reader
 *
 * @brief Interface for retrieving structure and data of an hdf5 file.
 * 
 * Top-level class in the VizSchema codebase.  Provides high-level accessors
 * to get data from an hdf5 file in the form of meshes, variables, and attributes.
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
 * VsH5Reader is a class for getting the vizschema metadata of
 * objects in and HDF5 file and also being able to return such objects.
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

  /**
   * Get the values of the whole variable (dataset).
   * Caller owns returned array and must allocate memory and delete it.
   * @param varname the variable name
   * @param indx index of the component
   * @param partnumber number of mesh part (along axis with leargest extent to load) for partial loading
   * @param numparts into how many parts should mesh be split along axis of longest extent
   * @param splitDims optional... unknown purpose
   */
  void* getVariableComponent(const std::string& varname, size_t indx,
                             size_t partnumber=0, size_t numparts=1,
                             size_t* splitDims=0);


  /**
   * Get attribute component of a mesh.
   * @param cname The name of the mesh component to retrieve
   * @param meta The metadata of the mesh
   * @param data The return value containing the requested attribute - ownership passes
   */
  //herr_t getAttMeshComponent (const std::string& cname, VsMesh& meta,
  //    void* data) const;

  /**
   * Get mesh of a var with mesh.
   * @param meta metadata for the var with mesh
   * @param data the array to be filled with mesh points
   */
  herr_t getVarWithMeshMesh(VsVariableWithMesh& meta, void* data,
                            size_t partStart, size_t partCount) const;

  /**
   * Get variable component of a var with mesh
   * Caller owns data and must allocate and delete it.
   * @param varname the variable name
   * @param idx index of component to read
   * @param data the returned filled array
   * @return 0 on success
   */
  herr_t getVarWithMeshComponent(const std::string& varname, size_t idx,
      void* data, size_t partStart, size_t partCount) const;

  private:
  // File id -opened once
 // hid_t fileId;
  
  VsH5File* fileData;
  
  static int numInstances;
  
  VsRegistry* registry;
};

#endif
#endif
