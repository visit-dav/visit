/*
 * VsMDVariable.C
 *
 *  Created on: Apr 29, 2010
 *      Author: mdurant
 */

#include "VsMDVariable.h"
#include "VsVariable.h"
#include "VsLog.h"
#include "VsUtils.h"
#include "VsSchema.h"
#include "VsMDMesh.h"
#include "VsMesh.h"

VsMDVariable::VsMDVariable(VsVariable* firstVar, VsMDMesh* mdMesh, std::string mdVarName):
  VsRegistryObject(firstVar->registry) {
  //std::string n, std::string meshString, std::string centeringString, std::string indexOrderString) {

  mesh = mdMesh->getFullName();
  name = mdVarName;
  indexOrder = firstVar->getIndexOrder();
  centering = firstVar->getCentering();
  numComponents = -1;
  
  //MD Var should have same number of blocks as MD Mesh
  blocks.resize(mdMesh->getNumBlocks());
    
  registry->add(this);
}

VsMDVariable::~VsMDVariable() {
//  for (std::vector<VsVariable*>::iterator it = blocks.begin(); it != blocks.end(); it++) {
//    delete (*it);
//  }
  blocks.clear();
  
  registry->remove(this);
}

//we look for user-specified component names in each subordinate variable
//we return the first one we find
std::string VsMDVariable::getLabel(int componentIndex) const{
  std::string name;
  name.clear();
  for (unsigned int i = 0; i < blocks.size(); i++) {
    name = blocks[i]->getLabel(componentIndex);
    if (!name.empty())
    return name;
  }

  return "";
}

bool VsMDVariable::isZonal() const {
  return (centering == VsSchema::zonalCenteringKey);
}

bool VsMDVariable::isNodal() const {
  return (centering == VsSchema::nodalCenteringKey);
}

bool VsMDVariable::isEdge() const {
  return (centering == VsSchema::edgeCenteringKey);
}

bool VsMDVariable::isFace() const {
  return (centering == VsSchema::faceCenteringKey);
}

bool VsMDVariable::addBlock(VsVariable* newBlock) {
  //all blocks must match
  //Special note here: the mesh of newBlock will be the NON-MD mesh
  //But all we know is the name of the MD mesh
  //In theory we can look up the subordinate mesh here,
  //but I'm leaving it as a todo for now
  ///TODO: look up subordinate mesh name
  //     if (newBlock->mesh != mesh)
  //        return "VsMDVariableMeta rejected new block " + newBlock->getFullName() + " because meshes did not match (" + newBlock->mesh + " vs " + mesh + ")";

  if (newBlock->getCentering() != centering) {
    VsLog::debugLog() <<"VsMDVariable rejected new block " + newBlock->getFullName() + " because centering did not match (" + newBlock->getCentering() + " vs " + centering + ")";
    return false;
  }
  
  if (newBlock->getIndexOrder() != indexOrder) {
    VsLog::debugLog() <<"VsMDVariable rejected new block " + newBlock->getFullName() + " because indexOrder did not match (" + newBlock->getIndexOrder() + " vs " + indexOrder + ")";
    return false;
  }
  
  //What domain number is this variable?
  VsMesh* varMesh = newBlock->getMesh();
  if (!varMesh) {
    VsLog::errorLog() <<"VsMDVariable rejected new block " + newBlock->getFullName() + " because it did not have a mesh." <<std::endl;
    return false;
  }
  int blockNumber = varMesh->getDomainNumber();
  if ((blockNumber < 0) || ((size_t)blockNumber >= blocks.size())) {
    VsLog::errorLog() <<"VsMDVariable rejected new block " + newBlock->getFullName() + " because the given domain number is out of bounds: " <<blockNumber <<std::endl;
    return false;
  }
  
  ///TODO - check for conflict?
  blocks[blockNumber] = newBlock;
  //blocks.push_back(newBlock);
  return true;
}

size_t VsMDVariable::getNumBlocks() const {
  return blocks.size();
}

std::string VsMDVariable::getNameForBlock(size_t domain) const {
  if (domain >= blocks.size())
  return NULL;

  VsVariable* foundVar = blocks[domain];
  return foundVar->getFullName();
}

VsVariable* VsMDVariable::getBlock(size_t domain) const {
  if (domain >= blocks.size())
  return NULL;

  VsVariable* foundVar = blocks[domain];
  return foundVar;
}

// Get mesh name
std::string VsMDVariable::getMesh() const {
  return mesh;
}

std::string VsMDVariable::getCentering() const {
  return centering;
}

// Get hdf5 type
hid_t VsMDVariable::getType() const {
  if (!blocks.empty()) {
    return blocks[0]->getType();
  }

  return 0;
}

// Get name
std::string VsMDVariable::getName() const {
  return name;
}

// Get full name
std::string VsMDVariable::getFullName() const {
  return makeCanonicalName(path, name);
}

// Write
void VsMDVariable::write() const {
  VsLog::debugLog() <<"   MDVar: " <<getFullName() <<std::endl;
  for (unsigned int i = 0; i < blocks.size(); i++) {
    VsLog::debugLog() <<"   block #" <<i <<" = " <<blocks[i]->getFullName() <<std::endl;
  }
  VsLog::debugLog() << "    indexOrder = " << indexOrder << std::endl;
  VsLog::debugLog() << "    centering = " << centering << std::endl;
  VsLog::debugLog() << "    MD mesh = " << mesh << std::endl;
}


size_t VsMDVariable::getNumComps() const {
  VsLog::debugLog() << "VsMDVariable::getNumComps() - Entering for variable " <<getFullName() <<std::endl;

  if (numComponents == -1) {
    // Look up all subordinate vars
    // Get num components for each (should all be the same)
    // Return the smallest (in case they're not all the same)
    for (unsigned int i = 0; i < getNumBlocks(); i++) {
      VsVariable* varMeta = getBlock(i);
      if (varMeta == NULL) {
        VsLog::debugLog() <<"VsMDVariable::getNumComps() - Unable to find block #"
        <<i <<"?  Returning 0." <<std::endl;
        return 0;
      }
      int comps = (int)varMeta->getNumComps();
      
      //first time through the loop we just initialize the comparison variable
      if (numComponents == -1) {
        numComponents = comps;
      } else {
        //the rest of the times through the loop we do some comparisons and error checking
        if (comps < numComponents) {
          VsLog::debugLog() <<"VsMDVariable::getNumComps() - mismatch between number of components in subordinate blocks" <<std::endl;
          numComponents = comps;
        } else if (comps > numComponents) {
          VsLog::debugLog() <<"VsMDVariable::getNumComps() - mismatch between number of components in subordinate blocks" <<std::endl;
          //note that we do NOT adjust fewestComponents here
        }
      }
    }
  }
  
  VsLog::debugLog() <<"VsMDVariable::getNumComps() - Returning " <<numComponents <<"." <<std::endl;
  return numComponents;
}

void VsMDVariable::createComponents() {
  VsLog::debugLog() <<"VsMDVariable::createComponents() - Entering" <<std::endl;
      
  int numComps = (int)getNumComps();
  if (numComps > 1) {
    for (int i = 0; i < numComps; i++) {
      registry->registerComponent(getFullName(), i, getLabel(i));
    }
  }

  VsLog::debugLog() <<"VsMDVariable::createComponents() - Returning" <<std::endl;
}

