/*
 * @file VsMDMesh.C
 *
 *  Created on: Apr 29, 2010
 *      Author: mdurant
 */

#include "VsMDMesh.h"

#include "VsSchema.h"
#include "VsMesh.h"
#include "VsUtils.h"
#include "VsLog.h"

#define __CLASS__ "VsMesh::"


VsMDMesh::VsMDMesh(VsMesh* firstMesh, std::string mdMeshName):
  VsRegistryObject(firstMesh->registry) {
  //Note: MD meshes do not have a path.  Their name is exactly what is
  //declared in the "vsMD" attribute This is because an MD mesh may be
  //made up of meshes in different paths, and we would have to choose
  //one over the other.  So, to avoid conflicts, we just use the name
  //as given
  numSpatialDims = (int)firstMesh->getNumSpatialDims();
  indexOrder = firstMesh->getIndexOrder();
  kind = firstMesh->getKind();
  name = mdMeshName;
  
  registry->add(this);
}

VsMDMesh::~VsMDMesh() {
  registry->remove(this);
}

void VsMDMesh::write() const   {
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << getFullName() << "  "
                    << "Kind: " << kind << "  "
                    << "Index Order: " << indexOrder << "  "
                    << "NumSpatialDims: " << numSpatialDims << "  "
                    << "Blocks:" << "  " << std::endl;

  for (unsigned int i = 0; i < blocks.size(); i++) {
    VsLog::debugLog() << "Block #" << i << " = " << blocks[i]->getFullName()
                      << std::endl;
  }
}

bool VsMDMesh::addBlock(VsMesh* newBlock) {
  if (newBlock->getIndexOrder() != indexOrder) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Rejected new block " << newBlock->getFullName()
                      << " because indexOrder did not match - ("
                      << newBlock->getIndexOrder() << " vs "
                      << indexOrder << ")" << std::endl;
    return false;
  }
  
  if (newBlock->getNumSpatialDims() != numSpatialDims) {
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Rejected new block " << newBlock->getFullName()
                    << " because numSpatialDims did not match" << std::endl;
    return false;
  }
  
  if (newBlock->getKind() != kind) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Rejected new block " << newBlock->getFullName()
                      << " because kind did not match (" << newBlock->getKind()
                      << " vs " << kind << ")" << std::endl;
    return false;
  }
  
  blocks.push_back(newBlock);
  newBlock->setMDMesh(this, (int)blocks.size() - 1);
  
  return true;
}

size_t VsMDMesh::getNumBlocks() const   {
  return blocks.size();
}

std::string VsMDMesh::getNameForBlock(size_t domain) const   {
  if (domain >= blocks.size())
  return NULL;

  VsMesh* foundMesh = blocks[domain];
  return foundMesh->getFullName();
}

VsMesh* VsMDMesh::getBlock(size_t domain) const  {
  if (domain >= blocks.size())
  return NULL;

  VsMesh* foundMesh = blocks[domain];
  return foundMesh;
}

std::string VsMDMesh::getMeshKind() const {
  return kind;
}

bool VsMDMesh::isUniformMesh() const  {
  return ((kind == VsSchema::Uniform::key) ||
          (kind == VsSchema::Uniform::key_deprecated));
}

bool VsMDMesh::isRectilinearMesh() const {
  return (kind == VsSchema::Rectilinear::key);
}

bool VsMDMesh::isUnstructuredMesh() const {
  return (kind == VsSchema::Unstructured::key);
}

bool VsMDMesh::isStructuredMesh() const {
  return (kind == VsSchema::structuredMeshKey);
}

size_t VsMDMesh::getNumSpatialDims() const {
  return numSpatialDims;
}

std::string VsMDMesh::getFullName() const {
  // Note: MD meshes do not have a path.  Their name is exactly what
  // is declared in the "vsMD" attribute This is because an MD mesh
  // may be made up of meshes in different paths, and we would have to
  // choose one over the other.  So, to avoid conflicts, we just use
  // the name as given
  return makeCanonicalName(name);
}
