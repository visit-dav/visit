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

class VsGroup;

class VsUnstructuredMesh: public VsMesh {
public:
  virtual ~VsUnstructuredMesh();
  
  size_t getNumPoints() const;
  size_t getNumCells() const;

  static VsUnstructuredMesh* buildUnstructuredMesh(VsGroup* group);
  
  virtual bool isUnstructuredMesh() const { return true; }
  virtual std::string getKind() const;
  
  //Tweak for Nautilus
  bool hasNodeCorrectionData() const;
  std::string getNodeCorrectionDatasetName() const;
  //end tweak

  bool usesSplitPoints() const;
  std::string getPointsDatasetName() const;
  std::string getPointsDatasetName(int i) const;
  hid_t getDataType() const;

  VsDataset* getPointsDataset() const;
  VsDataset* getPointsDataset(int i) const;
  std::string getPolygonsDatasetName() const;
  VsDataset* getPolygonsDataset() const;
  std::string getPolyhedraDatasetName() const;
  VsDataset* getPolyhedraDataset() const;
  std::string getLinesDatasetName() const;
  VsDataset* getLinesDataset() const;
  std::string getTrianglesDatasetName() const;
  VsDataset* getTrianglesDataset() const;
  std::string getQuadrilateralsDatasetName() const;
  VsDataset* getQuadrilateralsDataset() const;
  std::string getTetrahedralsDatasetName() const;
  VsDataset* getTetrahedralsDataset() const;
  std::string getPyramidsDatasetName() const;
  VsDataset* getPyramidsDataset() const;
  std::string getPrismsDatasetName() const;
  VsDataset* getPrismsDataset() const;
  std::string getHexahedralsDatasetName() const;
  VsDataset* getHexahedralsDataset() const;
  bool isPointMesh() const;
  
  virtual void getCellDims(std::vector<int>& dims) const;
  
private:
  VsUnstructuredMesh(VsGroup* group);
  virtual bool initialize();
  size_t numPoints;
  size_t numCells;
  bool splitPoints;
  
};

#endif /* VSUNSTRUCTUREDMESH_H_ */
