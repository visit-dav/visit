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

class VsGroup;

class VsUniformMesh: public VsMesh {
public:
  virtual ~VsUniformMesh();
  hid_t getDataType() const;
  
  virtual bool isUniformMesh() const { return true; }
  virtual std::string getKind() const;
  
  static VsUniformMesh* buildUniformMesh(VsGroup* group);

  int getLowerBounds(std::vector<float>* fVals);
  int getUpperBounds(std::vector<float>* fVals);
  int getStartCell(std::vector<int>* startCell);

  virtual void getCellDims(std::vector<int>& dims) const;
  virtual void getNodeDims(std::vector<int>& dims) const;
  
private:
  //required attributes
  VsAttribute* numCellsAtt;
  VsAttribute* lowerBoundsAtt;
  VsAttribute* upperBoundsAtt;
  VsAttribute* startCellAtt;
    
  VsUniformMesh(VsGroup* group);
  virtual bool initialize();
  
};

#endif /* VSUNIFORMMESH_H_ */
