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
   * @param newStride The stride to use when loading data
   */
  VsH5Reader(const std::string& filename, std::vector<int> settings, VsRegistry* r);

  /**
   * Destructor.
   */
  ~VsH5Reader();

  /**
   * Get the values of the whole variable (dataset).
   * Caller owns returned array and must allocate memory and delete it.
   * @param varname the variable name
   * @param data the returned filled array
   */
  herr_t getVariable(const std::string& varname, void* data) const;
  herr_t getVariableComponent(const std::string& varname, size_t indx, void* data);
  herr_t getVariableWithMesh(const std::string& varname, void* data) const;

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
      size_t partnumber=0, size_t numparts=1, size_t* splitDims=0);

  /**
   * Get mesh's dimensions.
   * @param name The name of the mesh to retrieve
   * @param dims The return value - a vector of integer dimensions
   */
  //size_t getMeshDims (VsMesh* meshMeta, std::vector<int>* dims) const;

  /**
   * Get dataset component of a mesh.
   * @param cname The name of the mesh component to retrieve
   * @param meta The metadata for the mesh
   * @param data The return value containing the requested dataset - ownership passes
   */
  herr_t getDatasetMeshComponent (VsH5Dataset* dataset, VsMesh& meta,
      void* data) const;
  herr_t getSplitMeshData(VsUnstructuredMesh& mmeta, void* data) const;

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
  herr_t getVarWithMeshMesh(VsVariableWithMesh& meta,
      void* data) const;

  herr_t getVarWithMeshMesh(VsVariableWithMesh& meta,
      void* data, size_t partStart, size_t partCount) const;

  /**
   * Get variable component of a var with mesh
   * Caller owns data and must allocate and delete it.
   * @param varname the variable name
   * @param idx index of component to read
   * @param data the returned filled array
   * @return 0 on success
   */
  herr_t getVarWithMeshComponent(const std::string& varname, size_t idx,
      void* data) const;

  herr_t getVarWithMeshComponent(const std::string& varname, size_t idx,
      void* data, size_t partStart, size_t partCount) const;

  /**
   * Should the reader use stride values to reduce the amount of data loaded?
   */
  bool useStride;

  private:
  // The stride to use when loading data
  std::vector<int> stride;

  // File id -opened once
 // hid_t fileId;
  
  VsH5File* fileData;
  
  static int numInstances;
  
  VsRegistry* registry;
};

#endif
#endif

