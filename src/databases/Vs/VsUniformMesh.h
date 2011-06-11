/**
 * @file VsUniformMesh.h
 *      
 *  @class VsUniformMesh
 *  @brief Represents a uniform mesh
 *
 *  Created on: Apr 29, 2010
 *      Author: mdurant
 */

#ifndef VSUNIFORMMESH_H_
#define VSUNIFORMMESH_H_

#include "VsMesh.h"
#include <hdf5.h>

class VsH5Group;

class VsUniformMesh: public VsMesh {
public:
  virtual ~VsUniformMesh();
  hid_t getDataType();
  
  virtual bool isUniformMesh() { return true; }
  virtual std::string getKind();
  
  static VsUniformMesh* buildUniformMesh(VsH5Group* group);

  herr_t getLowerBounds(std::vector<float>* fVals);
  herr_t getUpperBounds(std::vector<float>* fVals);
  herr_t getStartCell(std::vector<int>* startCell);

  virtual void getMeshDataDims(std::vector<int>& dims);
  virtual void getNumMeshDims(std::vector<int>& dims);
  
private:
  //required attributes
  VsH5Attribute* numCellsAtt;
  VsH5Attribute* lowerBoundsAtt;
  VsH5Attribute* upperBoundsAtt;
  VsH5Attribute* startCellAtt;
    
  VsUniformMesh(VsH5Group* group);
  virtual bool initialize();
  
};

#endif /* VSUNIFORMMESH_H_ */
