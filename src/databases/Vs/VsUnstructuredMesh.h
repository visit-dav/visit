/**
 * @file VsUnstructuredMesh.h
 *      
 *  @class VsUnstructuredMesh
 *  @brief Represents an unstructured mesh
 *
 *  Created on: Apr 29, 2010
 *      Author: mdurant
 */

#ifndef VSUNSTRUCTUREDMESH_H_
#define VSUNSTRUCTUREDMESH_H_

#include "VsMesh.h"
#include <hdf5.h>
#include <string>

class VsH5Group;

class VsUnstructuredMesh: public VsMesh {
public:
  virtual ~VsUnstructuredMesh();
  
  unsigned int getNumPoints();
  unsigned int getNumCells();

  static VsUnstructuredMesh* buildUnstructuredMesh(VsH5Group* group);
  
  virtual bool isUnstructuredMesh() { return true; }
  virtual std::string getKind();
  
  bool usesSplitPoints();
  std::string getPointsDatasetName();
  std::string getPointsDatasetName(int i);
  hid_t getDataType();

  VsH5Dataset* getPointsDataset();
  VsH5Dataset* getPointsDataset(int i);
  std::string getPolygonsDatasetName();
  VsH5Dataset* getPolygonsDataset();
  std::string getPolyhedraDatasetName();
  VsH5Dataset* getPolyhedraDataset();
  std::string getLinesDatasetName();
  VsH5Dataset* getLinesDataset();
  std::string getTrianglesDatasetName();
  VsH5Dataset* getTrianglesDataset();
  std::string getQuadrilateralsDatasetName();
  VsH5Dataset* getQuadrilateralsDataset();
  std::string getTetrahedralsDatasetName();
  VsH5Dataset* getTetrahedralsDataset();
  std::string getPyramidsDatasetName();
  VsH5Dataset* getPyramidsDataset();
  std::string getPrismsDatasetName();
  VsH5Dataset* getPrismsDataset();
  std::string getHexahedralsDatasetName();
  VsH5Dataset* getHexahedralsDataset();
  bool isPointMesh();
  
  virtual void getMeshDataDims(std::vector<int>& dims);
  
private:
  VsUnstructuredMesh(VsH5Group* group);
  virtual bool initialize();
  unsigned int numPoints;
  unsigned int numCells;
  bool splitPoints;
  
};

#endif /* VSUNSTRUCTUREDMESH_H_ */
