/**
 * @file VsStructuredMesh.h
 * 
 *  @class VsStructuredMesh
 *  @brief Represents a structured mesh
 *
 *  Created on: Apr 29, 2010
 *      Author: mdurant
 */

#ifndef VSSTRUCTUREDMESH_H_
#define VSSTRUCTUREDMESH_H_

#include "VsMesh.h"
#include <hdf5.h>

class VsDataset;

class VsStructuredMesh: public VsMesh {
public:
  virtual ~VsStructuredMesh();
  
  virtual bool isStructuredMesh() const { return true; }
  virtual std::string getKind() const;
  
  static VsStructuredMesh* buildStructuredMesh(VsDataset* data);

  virtual void getCellDims(std::vector<int>& dims) const;
  virtual void getNodeDims(std::vector<int>& dims) const;

  /**
   * Get the mask variable name
   * @return name or empty string if no mask
   */
  virtual std::string getMaskName() const;

private:
  VsStructuredMesh(VsDataset* data);
  virtual bool initialize();
 
  /** name of the mask array (optional) */
  VsAttribute* maskAtt;
};

#endif /* VSSTRUCTUREDMESH_H_ */
