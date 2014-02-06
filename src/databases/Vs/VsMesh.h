/**
 * @file VsMesh.h
 *
 *  @class VsMesh
 *  @brief Superclass for all other types of meshes.
 *  
 *  Provides a common interface for all other types of meshes, 
 *  except VsMDMeshes.
 *
 *  Created on: Apr 28, 2010
 *      Author: mdurant
 */

#ifndef VSMESH_H_
#define VSMESH_H_

#include <string>
#include <vector>
#include <map>

#include "VsRegistryObject.h"

class VsMDMesh;
class VsDataset;
class VsGroup;
class VsAttribute;
class VsObject;

class VsMesh : public VsRegistryObject {
public:
  virtual ~VsMesh();
  
  void write() const;
  bool isFortranOrder() const;
  bool isCompMinor() const;
  bool isCompMajor() const;
  
  virtual bool isUniformMesh() const { return false; }
  virtual bool isRectilinearMesh() const { return false; }
  virtual bool isStructuredMesh() const { return false; }
  virtual bool isUnstructuredMesh() const { return false; }
  
  virtual std::string getShortName() const;
  virtual std::string getPath() const;
  
  std::string getFullName() const;
  std::string getIndexOrder() const;
  virtual std::string getKind() const = 0;
  size_t getNumSpatialDims() const;
  size_t getNumTopologicalDims() const;

  virtual bool isHighOrder() const;
  virtual std::string getSubCellLocationsDatasetName() const;
  virtual VsDataset* getSubCellLocationsDataset() const;

  static VsMesh* buildObject(VsDataset* dataset);
  static VsMesh* buildObject(VsGroup* group);
  
  std::string getAxisLabel(size_t axis) const;
  
  virtual void getCellDims(std::vector<int>& dims) const = 0;
  
  void setMDMesh(VsMDMesh* mdMesh, int dNumber);
  int getDomainNumber() const ;
  VsMDMesh* getMDMesh() const ;

  VsAttribute* getAttribute(std::string name) const;
  virtual bool hasTransform() const;
  virtual std::string getTransformName() const;
  virtual std::string getTransformedMeshName() const;

protected:
  VsMesh(VsObject* object);
  virtual bool initialize() = 0;
  bool initializeRoot();
  void getStringAttribute(std::string attName, std::string* value) const;
  
  /** Number of nodes per cell **/
  size_t nodesPerCell;

  /** The spatial dimensionality */
  size_t numSpatialDims;

  /** The topological dimensionality 
   * Note: will always be <= the spatial dimensionality
   * (i.e. can have lower-dimension object in a higher dimension
   * but not vice versa)
   */
  size_t numTopologicalDims;

  /** Index order (Fortran vs C style) */
  std::string indexOrder;

  VsObject* h5Object;
  
  /** Stuff for meshes that are subordinate blocks of MD Meshes**/
  int domainNumber;
  VsMDMesh* mdMesh;
};

#endif /* VSMESH_H_ */
