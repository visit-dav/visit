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

VsMDMesh::VsMDMesh(VsMesh* firstMesh, std::string mdMeshName):
  VsRegistryObject(firstMesh->registry) {
  //Note: MD meshes do not have a path.  Their name is exactly what is declared in the "vsMD" attribute
  //This is because an MD mesh may be made up of meshes in different paths, and we would have to choose one over the other.
  //So, to avoid conflicts, we just use the name as given
  numSpatialDims = firstMesh->getNumSpatialDims();
  indexOrder = firstMesh->getIndexOrder();
  kind = firstMesh->getKind();
  name = mdMeshName;
  
  registry->add(this);
}

VsMDMesh::~VsMDMesh() {
  registry->remove(this);
}

void VsMDMesh::write()   {
  VsLog::debugLog() << getFullName() <<std::endl;
  VsLog::debugLog() << "    Index Order: " << indexOrder << std::endl;
  VsLog::debugLog() << "    Spatial Dimensionality: " << numSpatialDims <<std::endl;
  VsLog::debugLog() << "    Kind: " << kind <<std::endl;
  VsLog::debugLog() << "    Blocks:" <<std::endl;
  for (unsigned int i = 0; i < blocks.size(); i++) {
    VsLog::debugLog() << "      Block #" << i << " = " <<blocks[i]->getFullName() <<std::endl;
  }
}

bool VsMDMesh::addBlock(VsMesh* newBlock) {
  if (newBlock->getIndexOrder() != indexOrder) {
    VsLog::debugLog() << "VsMDMesh rejected new block " + newBlock->getFullName() + " because indexOrder did not match - (" + newBlock->getIndexOrder() + " vs " + indexOrder + ")";
    return false;
  }
  
  if (newBlock->getNumSpatialDims() != numSpatialDims) {
    VsLog::debugLog() <<"VsMDMesh rejected new block " + newBlock->getFullName() + " because numSpatialDims did not match";
    return false;
  }
  
  if (newBlock->getKind() != kind) {
    VsLog::debugLog() <<"VsMDMesh rejected new block " + newBlock->getFullName() + " because kind did not match (" + newBlock->getKind() + " vs " + kind + ")";
    return false;
  }
  
  blocks.push_back(newBlock);
  newBlock->setMDMesh(this, blocks.size() - 1);
  
  return true;
}

unsigned int VsMDMesh::getNumBlocks()   {
  return blocks.size();
}

std::string VsMDMesh::getNameForBlock(unsigned int domain)   {
  if (domain >= blocks.size())
  return NULL;

  VsMesh* foundMesh = blocks[domain];
  return foundMesh->getFullName();
}

VsMesh* VsMDMesh::getBlock(unsigned int domain)   {
  if (domain >= blocks.size())
  return NULL;

  VsMesh* foundMesh = blocks[domain];
  return foundMesh;
}

std::string VsMDMesh::getMeshKind()   {
  return kind;
}

bool VsMDMesh::isUniformMesh()   {
  return ((kind == VsSchema::Uniform::key) || (kind == VsSchema::Uniform::deprecated_key));
}

bool VsMDMesh::isRectilinearMesh()   {
  return (kind == VsSchema::Rectilinear::key);
}

bool VsMDMesh::isUnstructuredMesh()   {
  return (kind == VsSchema::Unstructured::key);
}

bool VsMDMesh::isStructuredMesh()   {
  return (kind == VsSchema::structuredMeshKey);
}

int VsMDMesh::getNumSpatialDims()   {
  return numSpatialDims;
}

std::string VsMDMesh::getFullName() {
  //Note: MD meshes do not have a path.  Their name is exactly what is declared in the "vsMD" attribute                                                       
  //This is because an MD mesh may be made up of meshes in different paths, and we would have to choose one over the other.                                   
  //So, to avoid conflicts, we just use the name as given                                                                                                     
  return makeCanonicalName(name);
}
