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
  void write();
  bool addBlock(VsMesh* newBlock);
  unsigned int getNumBlocks();
  std::string getNameForBlock(unsigned int domain);
  VsMesh* getBlock(unsigned int domain);
  std::string getMeshKind();
  bool isUniformMesh();
  bool isRectilinearMesh();
  bool isUnstructuredMesh();
  bool isStructuredMesh();
  int getNumSpatialDims();
  std::string getFullName();
  
  /** List of all subordinate blocks in this mesh
   * Order is important (block 0 = blocks[0], etc)*/
  std::vector<VsMesh*> blocks;  //TODO make private
private:


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
