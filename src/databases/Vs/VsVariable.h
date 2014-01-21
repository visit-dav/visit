/**
 * @file VsVariable.h
 *
 * @class VsVariable
 * @brief A dataset of value data.  Usually used for fields.
 * 
 *  Created on: Apr 28, 2010
 *      Author: mdurant
 */

#ifndef VSVARIABLE_H_
#define VSVARIABLE_H_

#include <hdf5.h>
#include "VsRegistryObject.h"
#include "VsGroup.h"

#include <string>
#include <vector>
#include <map>

class VsAttribute;
class VsDataset;
class VsMesh;

class VsVariable : public VsRegistryObject {
public:
  virtual ~VsVariable();

  bool isZonal() const;
  bool isNodal() const;
  bool isEdge() const;
  bool isFace() const;

  bool isFortranOrder() const;
  bool isCompMinor() const;
  bool isCompMajor() const;

  std::vector<int> getDims() const;
  
  /** Get name of mesh that this variable belongs to. */
  std::string getMeshName() const;
  VsMesh* getMesh() const;

  /** Does this variable have a transformed version? */
  bool hasTransform() const;

  /** Get hdf5 type */
  hid_t getType() const;

  /** Get length needed to store all elements in their format */
  size_t getLength() const;

  /** Get short name */
  std::string getShortName() const;

  /** Get path */
  std::string getPath() const;
  
  /** Get full name */
  std::string getFullName() const;

  /** Find attribute by name, or return NULL if not found */
  VsAttribute* getAttribute(const std::string& name) const;

  std::string getStringAttribute(const std::string& name) const;

  /** Get user-specified name for a variable component. */
  std::string getLabel(size_t i) const;

  /** Supply debugging output. */
  virtual void write() const;
  
  /** Get user-specified name for a variable component. */

  std::string getCentering() const { return centering;} 
  std::string getIndexOrder() const { return indexOrder; }
  hid_t getId() const;
  
  VsGroup* getTimeGroup() const { return timeGroup; }
  
  /** Public method to initialize object.
   * Returns NULL on error.*/
  static VsVariable* buildObject(VsDataset* dataset);
 
  /** Get the transformed name */
  std::string getFullTransformedName() const;

  void createTransformedVariable();

  void createComponents();
  size_t getNumComps() const;

  /**
   * Get the node offset 
   * @return offset array
   */
  std::vector<double> getNodeOffset() const;

  /**
   * Inquire if field has node offsets
   * @return true of field has node offsets
   */
  bool hasNodeOffset() const;

protected:
  VsVariable(VsDataset* data);
  
  /** Private method to initialize object.
   * Returns false on error.*/
  bool initialize();
  
  /** Name of the mesh that this variable belongs to. */
  std::string meshName;
  VsMesh* meshMeta;

  /** Component order (compMajorF/compMinorF, compMajorC/compMinorC) */
  std::string indexOrder;

  /** Centering (nodal or zonal) */
  std::string centering;
  
  /** Time group */
  VsGroup* timeGroup;

  /** Dataset used to construct this object. */
  VsDataset* dataset;

  /** User-specified names of the components of this variable. */
  std::vector<std::string> labelNames;
  
  /** Fully qualified path to this object */
  std::string path;

  /** lower node offset array (optional) */
  VsAttribute* nodeOffsetAtt;

  /** node offsets with respect to base node */
  std::vector<double> nodeOffset;

};

#endif /* VSVARIABLE_H_ */
