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

class VsAttribute;
class VsDataset;

class VsVariableWithMesh : public VsRegistryObject {
public:
  virtual ~VsVariableWithMesh();
  
  /** Get the number of spatial dimensions */
  size_t getNumSpatialDims() const;

  /** Retrieve a particular spatial dimension index from the list
    returns -1 on failure */
  int getSpatialDim(size_t index) const;

  /** Retrieve the entire list of spatial indices */
  std::vector<int> getSpatialIndices() const;
  
  /** Handles tranformation attributes for the "mesh" part of this object */
  virtual bool hasTransform() const;
  std::string getTransformName() const;
  std::string getTransformedMeshName() const;
  
  /** Handles transformation attributes for the "variable" part of this object */
  std::string getFullTransformedName() const;
  void createTransformedVariableAndMesh();
  
  /** Supply debugging output */
  void write() const;
  
  bool isZonal() const;
  bool isNodal() const;
  bool isEdge() const;
  bool isFace() const;
  bool isCompMinor() const;
  bool isCompMajor() const;

  /** Get dimensions of associated dataset. */
  std::vector<int> getDims() const;

  size_t getNumPoints() const;

  virtual void getCellDims(std::vector<int>& dims) const;
  virtual void getNodeDims(std::vector<int>& dims) const;

  /** Get hdf5 type */
  hid_t getType() const;

  /** Get length needed to store all elements in their format */
  size_t getLength() const;

  /** Get short name */
  std::string getShortName() const;

  /** Get path */
  std::string getPath() const;
  
  /** Get fully qualified name */
  std::string getFullName() const;

  /** Find attribute by name, or return NULL if not found. */
  VsAttribute* getAttribute(const std::string& name) const;

  /** Retrieve the value of the attribute with the given name */
  std::string getStringAttribute(const std::string& name) const;

  /** Retrieve the user-specified label for component number i */
  std::string getLabel(size_t i) const;
  
  /** Get the centering string*/
  std::string getCentering() const { return centering; }

  /** Get the index order string */
  std::string getIndexOrder() const { return indexOrder; }
 
  /** Retrieve the hdf5 id of this object. */
  hid_t getId() const;
  
  /** Public method to construct a VsVariableWithMesh object.
   * Returns NULL if an error is encountered.
   */
  static VsVariableWithMesh* buildObject(VsDataset* dataset);

  void createComponents();
  size_t getNumComps() const;
  
  VsGroup* getTimeGroup() const { return timeGroup; }

private:
  /** Private constructor */
  VsVariableWithMesh(VsDataset* dataset);

  /** Initializes the data members of the object.
   * Returns false if an error is found. */
  bool initialize();

  /** Index order, if defined (compMajorF/compMinorF, compMajorC/compMinorC) */
  std::string indexOrder;

  /** Centering, if defined (zonal or nodal) */
  std::string centering;

  /** Dataset object from which this object was created. */
  VsDataset* dataset;

  /** Time group */
  VsGroup* timeGroup;

  /** List of user-specified names for the components of this variable */
  std::vector<std::string> labelNames;
  
  /** Fully qualified path to this object */
  std::string path;
  
  /** A list of the indices of the spatial dims */
  std::vector<int> spatialIndices;
};

#endif /* VSVARIABLEWITHMESH_H_ */
