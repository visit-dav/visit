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

class VsH5Dataset;

class VsStructuredMesh: public VsMesh {
public:
  virtual ~VsStructuredMesh();
  
  virtual bool isStructuredMesh() { return true; }
  virtual std::string getKind();
  
  static VsStructuredMesh* buildStructuredMesh(VsH5Dataset* data);

  virtual void getMeshDataDims(std::vector<int>& dims);
  virtual void getNumMeshDims(std::vector<int>& dims);

private:
  VsStructuredMesh(VsH5Dataset* data);
  virtual bool initialize();
};

#endif /* VSSTRUCTUREDMESH_H_ */
