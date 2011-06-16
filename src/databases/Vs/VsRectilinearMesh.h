/**
 * @file VsRectilinearMesh.h
 *      
 *  @class VsRectilinearMesh
 *  @brief Represents a rectilinear mesh.
 *
 *  Created on: Apr 29, 2010
 *      Author: mdurant
 */

#ifndef VSRECTILINEARMESH_H_
#define VSRECTILINEARMESH_H_

#include "VsMesh.h"
#include <hdf5.h>

class VsH5Dataset;
class VsH5Group;

class VsRectilinearMesh: public VsMesh {
public:
  virtual ~VsRectilinearMesh();
  
  hid_t getDataType();
  std::string getAxisDatasetName(int axisNumber);
  VsH5Dataset* getAxisDataset(int axisNumber);
  
  virtual bool isRectilinearMesh() { return true; }
  static VsRectilinearMesh* buildRectilinearMesh(VsH5Group* group);
  
  virtual std::string getKind();

  virtual void getMeshDataDims(std::vector<int>& dims);
  virtual void getNumMeshDims(std::vector<int>& dims);

private:
  VsRectilinearMesh(VsH5Group* group);
  virtual bool initialize();
};

#endif /* VSRECTILINEARMESH_H_ */
