/**
 * @file  VsReader.h
 *
 * @class VsReader
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

#ifndef VS_READER_H_
#define VS_READER_H_

#include <hdf5.h>
#include <string>

class VsVariable;
class VsVariableWithMesh;
class VsMDMesh;
class VsMesh;
class VsMDVariable;
class VsFile;
class VsRegistry;
class VsUnstructuredMesh;
class VsStructuredMesh;
class VsUniformMesh;
class VsRectilinearMesh;
class VsDataset;

/**
 * VsReader is a class for getting the vizschema metadata of objects
 * in and HDF5 file and also being able to return such objects.
 */
class VsReader {

public:
  /**
   * Constructor.
   * @param filename the name of the HDF5 file
   * @param r Empty VsRegistry object
   */
  VsReader(const std::string& filename, VsRegistry* r);

  /**
   * Destructor.
   */
  ~VsReader();

  /**
   * Get the values of the whole dataset.
   * Caller owns returned array and must allocate memory and delete it.
   * @param dataset - dataset
   * @param data - pointer to the data
   * @param indexOrder - optional, FORTRAN or C and Minor or Major
   * @param components - optional, -2 (no components) -1 (read all components)
   * @param srcMins - optional, mins (start) array, up to three dimension
   * @param srcCounts - optional, counts array, up to three dimension
   * @param srcStrides - optional, stride array, up to three dimension

   * @param mdims - optional, rank of the memory space
   * @param destSizes - optional, memory size array, up to three dimension
   * @param destMins - optional, mins (start) array, up to three dimension
   * @param destCounts - optional, counts array, up to three dimension
   * @param destStrides - optional, stride array, up to three dimension
   **/
  int getData( VsDataset* dataset,
               void* data,
                     
               // Use these variables for adjusting
               // the read memory space.
               std::string indexOrder = std::string(""),
               int components = 0,
               int* srcMins = 0,
               int* srcCounts = 0,
               int* srcStrides = 0,

               // Use these variables for adjusting
               // the write memory space.
               int  mdims = 0,
               int* destSizes = 0,
               int* destMins = 0,
               int* destCounts = 0,
               int* destStrides = 0 ) const;    

private:
  VsFile* fileData;
  
  static int numInstances;
  
  VsRegistry* registry;
};

#endif // VS_READER_H_
