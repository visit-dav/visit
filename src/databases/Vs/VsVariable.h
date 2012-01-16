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

#include <string>
#include <vector>
#include <map>

class VsH5Attribute;
class VsH5Dataset;
class VsMesh;

class VsVariable : public VsRegistryObject {
public:
  virtual ~VsVariable();

  bool isZonal();

  bool isFortranOrder();
  bool isCompMinor();
  bool isCompMajor();

  std::vector<int> getDims();
  
  /** Get name of mesh that this variable belongs to. */
  std::string getMeshName();
  VsMesh* getMesh();

  /** Get hdf5 type */
  hid_t getType();

  /** Get length needed to store all elements in their format */
  size_t getLength();

  /** Get short name */
  std::string getShortName();

  /** Get path */
  std::string getPath();
  
  /** Get full name */
  std::string getFullName();

  /** Find attribute by name, or return NULL if not found */
  VsH5Attribute* getAttribute(const std::string& name);

  std::string getStringAttribute(const std::string& name);

  /** Supply debugging output. */
  virtual void write();
  
  /** Get user-specified name for a variable component. */
  std::string getLabel(unsigned int i);
  
  std::string getCentering() { return centering; }
  std::string getIndexOrder() { return indexOrder; }
  hid_t getId();
  
  VsH5Group* getTimeGroup() { return timeGroup; }
  
  /** Public method to initialize object.
   * Returns NULL on error.*/
  static VsVariable* buildObject(VsH5Dataset* dataset);

  void createComponents();
  size_t getNumComps();

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

  /**
   * Get the mesh name mangled with the node offsets
   * @return meshName if there are no node offset, or 
   *         meshName_withNodeOffset_<offset0>_<offset1>_<offset2>
   *         if there are offsets
   */
  std::string getNodeOffsetMeshName() const;

protected:
  VsVariable(VsH5Dataset* data);
  
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
  VsH5Group* timeGroup;

  /** Dataset used to construct this object. */
  VsH5Dataset* dataset;

  /** User-specified names of the components of this variable. */
  std::vector<std::string> labelNames;
  
  /** Fully qualified path to this object */
  std::string path;

  /** lower node offset array (optional) */
  VsH5Attribute* nodeOffsetAtt;

  /** node offsets with respect to base node */
  std::vector<double> nodeOffset;

  /** The name with the node offset mangled */
  std::string nodeOffsetMeshName;

};

#endif /* VSVARIABLE_H_ */
