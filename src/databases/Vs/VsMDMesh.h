/**
 * @file VsMDMesh.h
 *      
 *  @class VsMDMesh
 *  @brief Represents a multi-domain mesh - made up of other VsMesh objects
 *  
 *  A VsMDMesh allows the grouping of multiple VsMesh objects into a single mesh.
 *
 *  Created on: Apr 29, 2010
 *      Author: mdurant
 */

#ifndef VSMDMESH_H_
#define VSMDMESH_H_

#include <string>
#include <vector>
#include <map>

#include "VsRegistryObject.h"

class VsMesh;

class VsMDMesh : public VsRegistryObject {
public:
  VsMDMesh(VsMesh* firstMesh, std::string mdMeshName);
  virtual ~VsMDMesh();
  void write() const;
  bool addBlock(VsMesh* newBlock);
  size_t getNumBlocks() const;
  std::string getNameForBlock(size_t domain) const;
  VsMesh* getBlock(size_t domain) const;
  std::string getMeshKind() const;
  bool isUniformMesh() const;
  bool isRectilinearMesh() const;
  bool isUnstructuredMesh() const;
  bool isStructuredMesh() const;
  size_t getNumSpatialDims() const;
  std::string getFullName() const;

private:
  /** List of all subordinate blocks in this mesh
   * Order is important (block 0 = blocks[0], etc)*/
  std::vector<VsMesh*> blocks;

  /** The kind of the mesh (e.g., structured, uniform, ... */
  std::string kind;

  /** The spatial dimensionality */
  size_t numSpatialDims;

  /** Index order (Fortran vs C style) */
  std::string indexOrder;

  /* name */
  std::string name;
  
};

#endif /* VSMDMESH_H_ */
