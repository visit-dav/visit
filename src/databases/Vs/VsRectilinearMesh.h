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

class VsDataset;
class VsGroup;

class VsRectilinearMesh: public VsMesh {
public:
  virtual ~VsRectilinearMesh();
  
  hid_t getDataType() const;
  std::string getAxisDatasetName(int axisNumber) const;
  VsDataset* getAxisDataset(int axisNumber) const;

  virtual bool isRectilinearMesh() const { return true; }
  static VsRectilinearMesh* buildRectilinearMesh(VsGroup* group);
  
  virtual std::string getKind() const;

  virtual void getCellDims(std::vector<int>& dims) const;
  virtual void getNodeDims(std::vector<int>& dims) const;

private:
  VsRectilinearMesh(VsGroup* group);
  virtual bool initialize();
};

#endif /* VSRECTILINEARMESH_H_ */
