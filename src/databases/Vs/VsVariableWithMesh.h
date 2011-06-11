/**
 * @file VsVariableWithMesh.h
 *
 *  @class VsVariableWithMesh
 *  @brief A dataset with combined position and value data.  Usually used for particle data.
 *
 *  Created on: Apr 28, 2010
 *      Author: mdurant
 */


#ifndef VSVARIABLEWITHMESH_H_
#define VSVARIABLEWITHMESH_H_

#include "VsRegistryObject.h"

#include <vector>
#include <string>
#include <hdf5.h>
#include <map>

class VsH5Attribute;
class VsH5Dataset;

class VsVariableWithMesh : public VsRegistryObject {
public:
  virtual ~VsVariableWithMesh();
  
  /** Get the number of spatial dimensions */
  unsigned int getNumSpatialDims();

  /** Retrieve a particular spatial dimension index from the list
    returns -1 on failure */
  int getSpatialDim(size_t index);

  /** Retrieve the entire list of spatial indices */
  std::vector<int> getSpatialIndices();

  /** Supply debugging output */
  void write();
  
  bool isZonal();
  bool isCompMinor();
  bool isCompMajor();

  /** Get dimensions of associated dataset. */
  std::vector<int> getDims();

  unsigned int getNumPoints();

  virtual void getMeshDataDims(std::vector<int>& dims);
  virtual void getNumMeshDims(std::vector<int>& dims);

  /** Get hdf5 type */
  hid_t getType();

  /** Get length needed to store all elements in their format */
  size_t getLength();

  /** Get short name */
  std::string getShortName();

  /** Get path */
  std::string getPath();
  
  /** Get fully qualified name */
  std::string getFullName();

  /** Find attribute by name, or return NULL if not found. */
  VsH5Attribute* getAttribute(const std::string name);

  /** Retrieve the value of the attribute with the given name */
  std::string getStringAttribute(const std::string name);

  /** Retrieve the user-specified label for component number i */
  std::string getLabel(unsigned int i);
  
  /** Get the centering string*/
  std::string getCentering() { return centering; }

  /** Get the index order string */
  std::string getIndexOrder() { return indexOrder; }
 
  /** Retrieve the hdf5 id of this object. */
  hid_t getId();
  
  /** Public method to construct a VsVariableWithMesh object.
   * Returns NULL if an error is encountered.
   */
  static VsVariableWithMesh* buildObject(VsH5Dataset* dataset);

  void createComponents();
  size_t getNumComps();
  
  VsH5Group* getTimeGroup() { return timeGroup; }

private:
  /** Private constructor */
  VsVariableWithMesh(VsH5Dataset* dataset);

  /** Initializes the data members of the object.
   * Returns false if an error is found. */
  bool initialize();

  /** Index order, if defined (compMajorF/compMinorF, compMajorC/compMinorC) */
  std::string indexOrder;

  /** Centering, if defined (zonal or nodal) */
  std::string centering;

  /** Dataset object from which this object was created. */
  VsH5Dataset* dataset;

  /** Time group */
  VsH5Group* timeGroup;

  /** List of user-specified names for the components of this variable */
  std::vector<std::string> labelNames;
  
  /** Fully qualified path to this object */
  std::string path;
  
  /** A list of the indices of the spatial dims */
  std::vector<int> spatialIndices;
};

#endif /* VSVARIABLEWITHMESH_H_ */
