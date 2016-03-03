/*
 * VsRegistry.cpp
 *
 *  Created on: Apr 30, 2010
 *      Author: mdurant
 */

#include "VsRegistry.h"
#include "VsDataset.h"
#include "VsGroup.h"
#include "VsMesh.h"
#include "VsUtils.h"
#include "VsLog.h"
#include "VsAttribute.h"
#include "VsSchema.h"
#include "VsVariable.h"
#include "VsVariableWithMesh.h"
#include "VsMDMesh.h"
#include "VsMDVariable.h"
#include "VsMesh.h"

#include <sstream>

VsRegistry::VsRegistry() {
  deletingObjects = false;
  timeValue = -1;
  cycle = -1;
}

VsRegistry::~VsRegistry() {
  deleteAllObjects();
}

void VsRegistry::deleteAllObjects() {
  deletingObjects = true;
  
  //Delete the highest-level objects first
  this->deleteAllMeshes();
  this->deleteAllVariables();
  this->deleteAllVariablesWithMesh();

  //Delete the low-level objects last
  this->deleteAllDatasets();
  this->deleteAllGroups();
  
  deletingObjects = false;
}

/*********** VsGroups ***********/
void VsRegistry::add(VsGroup* group) {
  //check for duplicate long name
  VsGroup* found = this->getGroup(group->getFullName());
  if (found) {
    VsLog::errorLog() <<"VsRegistry::add() - Group already exists with this long name: " <<group->getFullName() <<std::endl;
    return;
  }
  
  //check for duplicate short names
  found = this->getGroup(group->getShortName());
  if (found) {
    VsLog::warningLog() <<"VsRegistry::add() - Group already exists with this short name: " <<group->getShortName() <<std::endl;
  }
  
  allGroups[group->getFullName()] = group; 
  allGroupsShort[group->getShortName()] = group;
}

int VsRegistry::numGroups() {
  return (int)allGroups.size();
}

void VsRegistry::remove(VsGroup* group) {
  if (deletingObjects)
    return;

  allGroups.erase(group->getFullName());
  allGroupsShort.erase(group->getShortName());
}

VsGroup* VsRegistry::getGroup(const std::string& name) {
  std::string fullName = makeCanonicalName(name);

  std::map<std::string, VsGroup*>::iterator it = allGroups.find(fullName);
  if (it != allGroups.end()) {
    return (*it).second;
  }
  
  it = allGroupsShort.find(fullName);
  if (it != allGroupsShort.end()) {
    return (*it).second;
  }
  return NULL;
}

void VsRegistry::deleteAllGroups() {
  for (std::map<std::string, VsGroup*>::const_iterator it = allGroups.begin();
          it != allGroups.end(); it++) {
        VsGroup* group = it->second;
        delete(group);
    }

  allGroups.clear();
  allGroupsShort.clear();
}

void VsRegistry::writeAllGroups() const {
  VsLog::debugLog() <<"********** BEGIN VsGroups *********" <<std::endl;
  
  for (std::map<std::string, VsGroup*>::const_iterator it = allGroups.begin();
        it != allGroups.end(); it++) {
      VsGroup* group = it->second;
      group->write();
  }
    
  VsLog::debugLog() <<"********** END VsGroups *********" <<std::endl;
}

void VsRegistry::buildGroupObjects() {
  VsLog::debugLog() <<"VsRegistry::buildGroupObjects - Entering." <<std::endl;
  for (std::map<std::string, VsGroup*>::const_iterator it = allGroups.begin();
      it != allGroups.end(); it++) {
    VsGroup* group = it->second;
    VsLog::debugLog() <<"VsRegistry::buildGroupObjects - Building object " <<group->getFullName() <<std::endl;
    
    //What is the declared type of this group?
    VsAttribute* typeAtt = group->getAttribute(VsSchema::typeAtt);
    if (!typeAtt) {
      VsLog::warningLog() <<"VsRegistry::buildGroupObjects - unable to find attribute " <<VsSchema::typeAtt
        <<".  Skipping object " <<group->getFullName() <<std::endl;
      continue;
    }
    
    std::string type;
    typeAtt->getStringValue(&type);
    VsLog::debugLog() <<"VsRegistry::buildGroupObjects - group is of type " <<type <<std::endl;
    if (type == VsSchema::meshKey) {
      VsMesh::buildObject(group);
    } else if (type == VsSchema::vsVarsKey) {
      buildExpressions(group);
    } else if (type == VsSchema::timeKey) {
      loadTime(group);
    } else if (type == VsSchema::runInfoKey) {
      loadRunInfo(group);
    } else {
      VsLog::debugLog() <<"VsRegistry::buildGroupObjects - object is of unrecognized type " <<type <<std::endl;
    }
  }
    
  VsLog::debugLog() <<"VsRegistry::buildGroupObjects - Returning." <<std::endl;
}

void VsRegistry::loadTime(VsGroup* group) {
  VsLog::debugLog() <<"VsRegistry::loadTime() - Group is NULL?" << "'" << group << "'" <<std::endl;
  if (!group) {
    VsLog::debugLog() <<"VsRegistry::loadTime() - Group is NULL?" <<std::endl;
    return;
  }
  
  //try to load a value for "time"
  double foundTime = -1.0;
  VsAttribute* timeAtt = group->getAttribute(VsSchema::timeAtt);
  if (timeAtt) {
    std::vector<float> in;
    int err = timeAtt->getFloatVectorValue(&in);
    if (err < 0) {
      VsLog::debugLog() <<"VsRegistry::loadTime(): Error " <<err <<" while trying to load time attribute." <<std::endl;
    } else {
      foundTime = in[0];
      VsLog::debugLog() <<"VsRegistry::loadTime() - loaded time: " <<foundTime  <<std::endl;
    }
  }

  //try to load a value for "cycle"
  int foundCycle = -1;
  VsAttribute* cycleAtt = group->getAttribute(VsSchema::cycleAtt);
  if (cycleAtt) {
    std::vector<int> in;
    int err = cycleAtt->getIntVectorValue(&in);
    if (err < 0) {
      VsLog::debugLog() <<"VsRegistry::loadTime(): Error " <<err <<" while trying to load cycle attribute." <<std::endl;
    } else {
      foundCycle = in[0];
      VsLog::debugLog() <<"VsRegistry::loadTime() - loaded cycle: " <<foundCycle <<std::endl;
    }
  }
 
  //check for existing time data, and compare
  if ((foundTime != -1) && hasTime() && (foundTime != getTime())) {
    VsLog::warningLog() <<"VsRegistry::loadTime() - was asked to load time data again, but time data already exists." <<std::endl;
    VsLog::warningLog() <<"VsRegistry::loadTime() - and is in conflict: " <<foundTime <<" vs " <<getTime() <<std::endl;
  } else {
    timeValue = foundTime;
  }
  
  if ((foundCycle != -1) && hasCycle() && (foundCycle != getCycle())) {
    VsLog::warningLog() <<"VsRegistry::loadTime() - was asked to load cycle data again, but cycle data already exists." <<std::endl;
    VsLog::warningLog() <<"VsRegistry::loadTime() - and is in conflict: " <<foundCycle <<" vs " <<getCycle() <<std::endl;
  } else {
    cycle = foundCycle;
  }
}

void VsRegistry::loadRunInfo(VsGroup* group) {
  if (!group) {
    VsLog::debugLog() <<"VsRegistry::loadRunInfo() - Group is NULL?" <<std::endl;
    return;
  }

  VsLog::debugLog() <<"VsRegistry::loadRunInfo() - not loading any information at this time." <<std::endl;
}

/*********** VsDatasets***********/
void VsRegistry::add(VsDataset* dataset) {
  //check for duplicate long name
  VsDataset* found = this->getDataset(dataset->getFullName());
  if (found) {
    VsLog::errorLog() <<"VsRegistry::add() - Dataset already exists with this long name: " <<dataset->getFullName() <<std::endl;
    return;
  }
  
  //check for duplicate short names
  found = this->getDataset(dataset->getShortName());
  if (found) {
    VsLog::warningLog() <<"VsRegistry::add() - Dataset already exists with this short name: " <<dataset->getShortName() <<std::endl;
  }
  
  allDatasets[dataset->getFullName()] = dataset; 
  allDatasetsShort[dataset->getShortName()] = dataset;
}

int VsRegistry::numDatasets() {
  return (int)allDatasets.size();
}

void VsRegistry::remove(VsDataset* dataset) {
  if (deletingObjects)
    return;

  allDatasets.erase(dataset->getFullName());
  allDatasetsShort.erase(dataset->getShortName());
}

VsDataset* VsRegistry::getDataset(const std::string& name) {
  std::string fullName = makeCanonicalName(name);

  std::map<std::string, VsDataset*>::iterator it = allDatasets.find(fullName);
  if (it != allDatasets.end()) {
    return (*it).second;
  }
  
  it = allDatasetsShort.find(fullName);
  if (it != allDatasetsShort.end()) {
    return (*it).second;
  }
  return NULL;
}

void VsRegistry::deleteAllDatasets() {
  for (std::map<std::string, VsDataset*>::const_iterator it = allDatasets.begin();
        it != allDatasets.end(); it++) {
      VsDataset* dataset = it->second;
      delete(dataset);
  }

  allDatasets.clear();
  allDatasetsShort.clear();
}

void VsRegistry::writeAllDatasets() const {
  VsLog::debugLog() <<"********** BEGIN VsDatasets *********" <<std::endl;
  
  for (std::map<std::string, VsDataset*>::const_iterator it = allDatasets.begin();
        it != allDatasets.end(); it++) {
      VsDataset* dataset = it->second;
      dataset->write();
  }
    
  VsLog::debugLog() <<"********** END VsDatasets *********" <<std::endl;
}

void VsRegistry::buildDatasetObjects() {
  VsLog::debugLog() <<"VsRegistry::buildDatasetObjects() - Entering."  <<std::endl;

  //First pass just do meshes
  //But remember the variables for second pass
  std::vector<VsDataset*> varDatasets;
  std::vector<VsDataset*> varWithMeshDatasets;
  for (std::map<std::string, VsDataset*>::const_iterator it = allDatasets.begin();
     it != allDatasets.end(); it++)  {
    VsDataset* dataset = it->second;
    VsLog::debugLog() <<"VsRegistry::buildDatasetObjects() - looking at dataset " <<dataset->getFullName() <<std::endl;

    //Try to determine the type of the object
    std::string type;    
    VsAttribute* typeAtt = dataset->getAttribute(VsSchema::typeAtt);
    if (!typeAtt) {
      VsLog::warningLog() <<"VsRegistry::buildDatasetObjects() - unable to find attribute " <<VsSchema::typeAtt <<std::endl;
      
      //If the object contains the "vsMesh" attribute, then it is probably intended to be a variable
      //So continue with that assumption
      VsLog::warningLog() <<"VsRegistry::buildDatasetObjects() - Second chance - looking for attribute " <<VsSchema::meshAtt <<std::endl;
      VsAttribute* meshAtt = dataset->getAttribute(VsSchema::meshAtt);
      if (meshAtt) {
        VsLog::warningLog() <<"VsRegistry::buildDatasetObjects() - Found attribute " <<VsSchema::meshAtt <<" assuming that this is a variable." <<std::endl;
        type = VsSchema::varKey;
      } else {
        VsLog::warningLog() <<"VsRegistry::buildDatasetObjects() - Did not find attribute " <<VsSchema::meshAtt
                  <<", second chance option has failed.  Skipping object: " <<dataset->getFullName() <<std::endl;
        continue;
      }
    } else {
      typeAtt->getStringValue(&type);
    }
    
    VsLog::debugLog() <<"VsRegistry::buildDatasetObjects() - object is of type " <<type <<std::endl;
    if (type == VsSchema::meshKey) {
      VsMesh::buildObject(dataset);
    } else if (type == VsSchema::varKey) {
      VsLog::debugLog() <<"VsRegistry::buildDatasetObjects() - Variables are built in second pass. Skipping for now." <<std::endl;
      varDatasets.push_back(dataset);
    } else if (type == VsSchema::varWithMeshKey) {
      VsLog::debugLog() <<"VsRegistry::buildDatasetObjects() - VariableWithMesh are built in second pass. Skipping for now." <<std::endl;
      varWithMeshDatasets.push_back(dataset);
    } else {
      VsLog::debugLog() <<"VsRegistry::buildDatasetObjects() - object is of unknown type!" <<std::endl;
    }
  }

  //Second pass to do all variables
  for (std::vector<VsDataset*>::const_iterator it = varDatasets.begin();
       it != varDatasets.end(); it++)  {
    VsDataset* dataset = *it;
    
    VsVariable* var = VsVariable::buildObject(dataset);
    if (var && (var->getTimeGroup() != NULL)) {
      loadTime(var->getTimeGroup());
    }
  }

  //Second pass to do all variables with mesh
  for (std::vector<VsDataset*>::const_iterator it = varWithMeshDatasets.begin();
       it != varWithMeshDatasets.end(); it++)  {
    VsDataset* dataset = *it;
    VsLog::debugLog() <<"VsRegistry::buildDatasetObjects() - looking at varWithMesh dataset " <<dataset->getFullName() <<std::endl;
    
    VsVariableWithMesh* var = VsVariableWithMesh::buildObject(dataset);
    if (var && (var->getTimeGroup() != NULL)) {
      loadTime(var->getTimeGroup());
    }
  }
  
  VsLog::debugLog() <<"VsRegistry::buildDatasetObjects() - Returning."  <<std::endl;
 }

/*********** VsMeshes ***********/
void VsRegistry::add(VsMesh* mesh) {
  //check for duplicate long name
  VsMesh* found = this->getMesh(mesh->getFullName());
  if (found) {
    VsLog::errorLog() <<"VsRegistry::add() - Mesh already exists with this long name: " <<mesh->getFullName() <<std::endl;
    return;
  }
  
  //check for duplicate short names
  found = this->getMesh(mesh->getShortName());
  if (found) {
    VsLog::warningLog() <<"VsRegistry::add() - Mesh already exists with this short name: " <<mesh->getShortName() <<std::endl;
  }
  
  allMeshes[mesh->getFullName()] = mesh; 
  allMeshesShort[mesh->getShortName()] = mesh;
}

int VsRegistry::numMeshes() {
  return (int)allMeshes.size();
}

void VsRegistry::remove(VsMesh* mesh) {
  if (deletingObjects)
    return;

  allMeshes.erase(mesh->getFullName());
  allMeshesShort.erase(mesh->getShortName());
}

VsMesh* VsRegistry::getMesh(const std::string& name) {
  std::string fullName = makeCanonicalName(name);

  std::map<std::string, VsMesh*>::iterator it = allMeshes.find(fullName);
  if (it != allMeshes.end()) {
    return (*it).second;
  }
  
  it = allMeshesShort.find(fullName);
  if (it != allMeshesShort.end()) {
    return (*it).second;
  }
  return NULL;
}

void VsRegistry::deleteAllMeshes() {
  for (std::map<std::string, VsMesh*>::iterator it = allMeshes.begin();
        it != allMeshes.end(); it++) {
      VsMesh* mesh = it->second;
      delete(mesh);
  }
  
  allMeshes.clear();
  allMeshesShort.clear();
}

void VsRegistry::writeAllMeshes() const {
  if (allMeshes.empty()) {
    VsLog::debugLog() <<"*********** NO MESHES ********" <<std::endl;
    return;
  }
  
  VsLog::debugLog() <<"********** BEGIN VsMeshs *********" <<std::endl;
  
  for (std::map<std::string, VsMesh*>::const_iterator it = allMeshes.begin();
        it != allMeshes.end(); it++) {
      VsMesh* mesh = it->second;
      mesh->write();
  }
    
  VsLog::debugLog() <<"********** END VsMeshs *********" <<std::endl;
}

void VsRegistry::getAllMeshNames(std::vector<std::string>& names)  {
  std::map<std::string, VsMesh*>::const_iterator it;
  for (it = allMeshes.begin(); it != allMeshes.end(); ++it)
    names.push_back(it->first);
}

/************* MD MESHES **************/

VsMDMesh* VsRegistry::getMDMesh(const std::string& name) {
  // Make name fully qualified
  std::string fullName = makeCanonicalName(name);
  
  std::map<std::string, VsMDMesh*>::iterator it = allMDMeshes.find(fullName);
  if (it != allMDMeshes.end()) {
   return it->second;
  }
  
  return NULL;
}

void VsRegistry::add(VsMDMesh* mesh) {
  //check for duplicate name
  VsMDMesh* foundMesh = this->getMDMesh(mesh->getFullName());
  if (foundMesh) {
    VsLog::errorLog() <<"VsRegistry::add() - MD Mesh already exists with this long name: " <<mesh->getFullName() <<std::endl;
    return;
  }
  
  allMDMeshes[mesh->getFullName()] = mesh;
}

void VsRegistry::remove(VsMDMesh* mesh) {
  allMDMeshes.erase(mesh->getFullName());
}

void VsRegistry::getAllMDMeshNames(std::vector<std::string>& names)  {
  std::map<std::string, VsMDMesh*>::const_iterator it;
  for (it = allMDMeshes.begin(); it != allMDMeshes.end(); ++it)
    names.push_back(it->first);
}

void VsRegistry::writeAllMDMeshes() const {
  if (allMDMeshes.empty()) {
    VsLog::debugLog() <<"*********** NO MDMESHES ********" <<std::endl;
    return;
  }
  
  VsLog::debugLog() <<"*********** BEGIN MDMESHES ********" <<std::endl;
  std::map<std::string, VsMDMesh*>::const_iterator it;
  for (it = allMDMeshes.begin(); it != allMDMeshes.end(); ++it) {
    it->second->write();
  }
  
  VsLog::debugLog() <<"*********** END MDMESHES ********" <<std::endl;
}

int VsRegistry::numMDMeshes() {
  return (int)allMDMeshes.size();
}

VsMDMesh* VsRegistry::getMDParentForMesh(const std::string& name) {
  VsLog::debugLog() << "VsMDMesh::getMDParentForMesh(" <<name <<"): Entering." << std::endl;
  
  std::string fullName = makeCanonicalName(name);
  
  for (std::map<std::string, VsMDMesh*>::const_iterator it = allMDMeshes.begin(); it != allMDMeshes.end(); it++) {
    VsMDMesh* meshMeta = it->second;
    for (unsigned int i = 0; i < meshMeta->getNumBlocks(); i++) {
      if (fullName == makeCanonicalName(meshMeta->getBlock(i)->getFullName())) {
        VsLog::debugLog() << "VsMDMesh::getMDParentForMesh(" <<name <<"): Returning result." << std::endl;
        return meshMeta;
      }
    }
  }
  VsLog::debugLog() << "VsMDMesh::getMDParentForMesh(" <<name <<"): Returning NULL." << std::endl;
  return NULL;
}

VsMesh* VsRegistry::findSubordinateMDMesh(const std::string& name) {
  VsLog::debugLog() << "VsMDMesh::findSubordinateMDMesh(" <<name <<"): Entering." << std::endl;
  for (std::map<std::string, VsMDMesh*>::const_iterator it = allMDMeshes.begin(); it != allMDMeshes.end(); it++) {
    VsMDMesh* meshMeta = it->second;
    for (size_t i = 0; i < meshMeta->getNumBlocks(); i++) {
      if (meshMeta->getBlock(i)->getFullName() == name) {
        VsLog::debugLog() << "VsReader::findSubordinateMDMesh(" <<name <<"): Returning result." << std::endl;
        return meshMeta->getBlock(i);
      }
    }
  }
  VsLog::debugLog() << "VsMDMesh::findSubordinateMDMesh(" <<name <<"): Returning NULL." << std::endl;
  return NULL;
}

void VsRegistry::buildMDMeshes() {
  VsLog::debugLog() <<"VsRegistry::buildMDMeshes() - Entering." <<std::endl;
  
  // Go through list of all meshes and group the MD meshes together
  std::map<std::string, VsMesh*>::const_iterator it;
  for (it = allMeshes.begin(); it != allMeshes.end(); it++) {
    VsLog::debugLog() <<"VsRegistry::buildMDMeshes() - examining mesh " <<(*it).first <<std::endl;
    VsMesh* mesh = (*it).second;
    if (!mesh) {
      VsLog::errorLog() <<"VsRegistry::buildMDMeshes() - mesh is NULL?" <<std::endl;
      continue;
    }
    VsAttribute* mdMeshNameAttribute = mesh->getAttribute(VsSchema::mdAtt);
    std::string mdMeshName = "";
    if (mdMeshNameAttribute) {
      mdMeshNameAttribute->getStringValue(&mdMeshName);
    }
    if (!mdMeshName.empty()) {
      VsMDMesh* mdMesh = getMDMesh(mdMeshName);
      
      //If this is a new md mesh, create it
      if (mdMesh == NULL) {
        VsLog::debugLog() <<"VsRegistry::buildMDMeshes() - MD mesh not found with name " <<mdMeshName <<".  Creating." <<std::endl;
        mdMesh = new VsMDMesh(mesh, mdMeshName);
      }
      
      //add this group to the md mesh
      bool success = mdMesh->addBlock(mesh);
      if (!success) {
        VsLog::debugLog() <<"VsRegistry::buildMDMeshes() - failed to add block to md mesh.  Will remain as a stand-alone mesh." <<std::endl;
      }
    }
  }
  
  VsLog::debugLog() <<"VsRegistry::buildMDMeshes() - Exiting." <<std::endl;
}

/*********** TRANSFORMED VARIABLES ****************************/
void VsRegistry::buildTransformedVariables() {
  std::map<std::string, VsVariable*>::const_iterator it;
  for (it = allVariables.begin(); it != allVariables.end(); ++it) {
    VsLog::debugLog() <<"VsRegistry::buildTransformedVariables() - Creating transform names for Variable " <<it->first <<std::endl;
    it->second->createTransformedVariable();
  }

  std::map<std::string, VsVariableWithMesh*>::const_iterator it2;
  for (it2 = allVariablesWithMesh.begin(); it2 != allVariablesWithMesh.end(); ++it2) {
    VsLog::debugLog() <<"VsRegistry::buildTransformedVariables() - Creating transform names for Variable with Mesh " <<it2->first <<std::endl;
    it2->second->createTransformedVariableAndMesh();
  }
}

/*********** TRANSFORMED MESHES *******************************/
void VsRegistry::buildTransformedMeshes() {
  //go through all meshes looking for "vsTransform" flags
  VsLog::debugLog() <<"VsRegistry::buildTransformedMeshes() - Entering." <<std::endl;
  
  // Roopa: Check if there is a transformation specified for this
  // mesh. If so, register the transformed mesh here
  std::map<std::string, VsMesh*>::const_iterator it;
  for (it = allMeshes.begin(); it != allMeshes.end(); it++) {
    VsLog::debugLog() <<"VsRegistry::buildTransformedMeshes() - examining mesh " <<(*it).first <<std::endl;
    VsMesh* mesh = (*it).second;
    if (!mesh) {
      VsLog::errorLog() <<"VsRegistry::buildTransformedMeshes() - mesh is NULL?" <<std::endl;
      continue;
    }
    if (mesh->hasTransform()) {
      std::string transformedMeshName = mesh->getTransformedMeshName();
      VsLog::debugLog() <<"VsRegistry::buildTransformedMeshes() - mesh " <<(*it).first <<" has transformed mesh " <<transformedMeshName <<std::endl;
      registerTransformedMeshName(transformedMeshName, mesh->getFullName());
    }
  }
  
  VsLog::debugLog() <<"VsRegistry::buildTransformedMeshes() - Returning." <<std::endl;
}

/*********** VARIABLES ***********/
void VsRegistry::add(VsVariable* variable) {  
  //check for duplicate long name
  VsVariable* foundVariable = this->getVariable(variable->getFullName());
  if (foundVariable) {
    VsLog::errorLog() <<"VsRegistry::add() - Variable already exists with this long name: " <<variable->getFullName() <<std::endl;
    return;
  }
  
  //check for duplicate short names
  foundVariable = this->getVariable(variable->getShortName());
  if (foundVariable) {
    VsLog::warningLog() <<"VsRegistry::add() - Variable already exists with this short name: " <<variable->getShortName() <<std::endl;
  }
  
  allVariables[variable->getFullName()] = variable; 
  allVariablesShort[variable->getShortName()] = variable;
}

int VsRegistry::numVariables() {
  return (int)allVariables.size();
}

void VsRegistry::remove(VsVariable* variable) {
  if (deletingObjects)
    return;

  allVariables.erase(variable->getFullName());
  allVariablesShort.erase(variable->getShortName());
}

VsVariable* VsRegistry::getVariable(const std::string& name) {
  std::string fullName = makeCanonicalName(name);

  std::map<std::string, VsVariable*>::iterator it = allVariables.find(fullName);
  if (it != allVariables.end()) {
    return (*it).second;
  }
  
  it = allVariablesShort.find(fullName);
  if (it != allVariablesShort.end()) {
    return (*it).second;
  }
  return NULL;
}

void VsRegistry::deleteAllVariables() {
  for (std::map<std::string, VsVariable*>::const_iterator it = allVariables.begin();
        it != allVariables.end(); it++) {
      VsVariable* variable = it->second;
      delete(variable);
  }

  allVariables.clear();
  allVariablesShort.clear();
}

void VsRegistry::writeAllVariables() const {
  if (allVariables.empty()) {
    VsLog::debugLog() <<"*********** NO VSVARIABLES ********" <<std::endl;
    return;
  }
  
  VsLog::debugLog() <<"********** BEGIN VsVariables *********" <<std::endl;
  
  for (std::map<std::string, VsVariable*>::const_iterator it = allVariables.begin();
        it != allVariables.end(); it++) {
      VsVariable* variable = it->second;
      variable->write();
  }
    
  VsLog::debugLog() <<"********** END VsVariables *********" <<std::endl;
}

void VsRegistry::getAllVariableNames(std::vector<std::string>& names)  {
  std::map<std::string, VsVariable*>::const_iterator it;
  for (it = allVariables.begin(); it != allVariables.end(); ++it)
    names.push_back(it->first);
}


/*********** MD VARIABLES ***************/

VsMDVariable* VsRegistry::getMDVariable(const std::string& name) {
  // Make name fully qualified
  std::string fullName = makeCanonicalName(name);
  
  std::map<std::string, VsMDVariable*>::iterator it = allMDVariables.find(fullName);
  if (it != allMDVariables.end()) {
   return it->second;
  }
  
  return NULL;
}

void VsRegistry::add(VsMDVariable* var) {  //check for duplicate long name
  VsMDVariable* foundVariable = this->getMDVariable(var->getFullName());
  if (foundVariable) {
    VsLog::errorLog() <<"VsRegistry::add() - MD Variable already exists with this long name: " <<var->getFullName() <<std::endl;
    return;
  }
  
  allMDVariables[var->getFullName()] = var;
}

void VsRegistry::remove(VsMDVariable* var) {
  allMDVariables.erase(var->getFullName());
}

void VsRegistry::getAllMDVariableNames(std::vector<std::string>& names)  {
  std::map<std::string, VsMDVariable*>::const_iterator it;
  for (it = allMDVariables.begin(); it != allMDVariables.end(); ++it)
    names.push_back(it->first);
}

void VsRegistry::writeAllMDVariables() const {
  if (allMDVariables.empty()) {
    VsLog::debugLog() <<"*********** NO MDVARIABLES ********" <<std::endl;
    return;
  }
  
  VsLog::debugLog() <<"*********** BEGIN MDVARIABLES ********" <<std::endl;
  std::map<std::string, VsMDVariable*>::const_iterator it;
  for (it = allMDVariables.begin(); it != allMDVariables.end(); ++it) {
    it->second->write();
  }
  
  VsLog::debugLog() <<"*********** END MDVARIABLES ********" <<std::endl;
}

int VsRegistry::numMDVariables() {
  return (int)allMDVariables.size();
}

void VsRegistry::buildMDVars() {
  VsLog::debugLog() <<"VsRegistry::buildMDVars() - Entering." <<std::endl;
  
  // Go through list of all vars and group the MD vars together
  std::map<std::string, VsVariable*>::const_iterator it;
  for (it = allVariables.begin(); it != allVariables.end(); it++) {
    VsVariable* var = (*it).second;
    
    VsAttribute* mdVarAtt = var->getAttribute(VsSchema::mdAtt);
    if (!mdVarAtt) {
      continue;
    }
    
    std::string mdVarName;
    mdVarAtt->getStringValue(&mdVarName);
    
    if (!mdVarName.empty()) {
      VsMDVariable* mdVar = getMDVariable(mdVarName);
      if (!mdVar) {
        VsLog::debugLog() <<"VsRegistry::buildMDVars() - MD variable " + mdVarName + " not found, creating." <<std::endl;
        
        //the md var lives on an md mesh, so cross-reference to get the md mesh
        VsMDMesh* mdMesh = getMDParentForMesh(var->getMeshName());
        
        //did we find the appropriate md mesh?
        if (!mdMesh) {
          VsLog::debugLog() <<"VsRegistry::buildMDVars() - WARNING: variable " + var->getFullName() + " is declared as part of MD Var " + mdVarName + " but referenced mesh " + var->getMeshName() + " is not part of an MD mesh." <<std::endl;
          continue;
        } else {
          //note that the MD variable inherits index order and centering of first block
          mdVar = new VsMDVariable(var, mdMesh, mdVarName);
          
          VsLog::debugLog() <<"VsRegistry::buildMDVars() - Created new MD Variable named " << mdVarName <<" on mesh " <<mdMesh->getFullName() <<std::endl;
        }
      }

      if (!mdVar) {
        VsLog::debugLog() <<"VsRegistry::buildMDVars() - MD var object doesn't exist?" <<std::endl;
        continue;
      }
      //Finally, add the var to the MD var
      bool success = mdVar->addBlock(var);
      if (!success) {
        VsLog::warningLog() <<"VsRegistry::buildMDVars() - unable to add var to MD var, will remain stand-alone" <<std::endl;
      }
    }
  }
  VsLog::debugLog() <<"VsRegistry::buildMDVars() - Returning." <<std::endl;
}

VsVariable* VsRegistry::findSubordinateMDVar(const std::string& name) {
 VsLog::debugLog() << "VsRegistry::findSubordinateMDVar(" <<name <<"): Entering." << std::endl;
 for (std::map<std::string, VsMDVariable*>::const_iterator it = allMDVariables.begin(); it != allMDVariables.end(); it++) {
   VsMDVariable* varMeta = it->second;
   for (unsigned int i = 0; i < varMeta->getNumBlocks(); i++) {
     if (varMeta->getBlock(i)->getFullName() == name) {
       VsLog::debugLog() << "VsRegistry::findSubordinateMDVar(" <<name <<"): Returning result." << std::endl;
       return varMeta->getBlock(i);
     }
   }
 }
 VsLog::debugLog() << "VsRegistry::findSubordinateMDVar(" <<name <<"): Returning NULL." << std::endl;
 return NULL;
}

/*********** VARIBLES WITH MESH ***********/
void VsRegistry::add(VsVariableWithMesh* variable) {
  //check for duplicate long name
  VsVariableWithMesh* foundVariable = this->getVariableWithMesh(variable->getFullName());
  if (foundVariable) {
    VsLog::errorLog() <<"VsRegistry::add() - Variable with mesh already exists with this full name: " <<variable->getFullName() <<std::endl;
    return;
  }
  
  //check for duplicate short names
  foundVariable = this->getVariableWithMesh(variable->getShortName());
  if (foundVariable) {
    VsLog::errorLog() <<"VsRegistry::add() - Variable with mesh already exists with this short name: " <<variable->getShortName() <<std::endl;
  }
  
  allVariablesWithMesh[variable->getFullName()] = variable; 
  allVariablesWithMeshShort[variable->getShortName()] = variable;
}

int VsRegistry::numVariablesWithMesh() {
  return (int)allVariablesWithMesh.size();
}

void VsRegistry::remove(VsVariableWithMesh* variable) {  
  if (deletingObjects)
    return;

  allVariablesWithMesh.erase(variable->getFullName());
  allVariablesWithMeshShort.erase(variable->getShortName());
}

VsVariableWithMesh* VsRegistry::getVariableWithMesh(const std::string& name) {
  std::string fullName = makeCanonicalName(name);

  std::map<std::string, VsVariableWithMesh*>::iterator it = allVariablesWithMesh.find(fullName);
  if (it != allVariablesWithMesh.end()) {
    return (*it).second;
  }
  
  it = allVariablesWithMeshShort.find(fullName);
  if (it != allVariablesWithMeshShort.end()) {
    return (*it).second;
  }
  return NULL;
}

void VsRegistry::deleteAllVariablesWithMesh() {
  for (std::map<std::string, VsVariableWithMesh*>::const_iterator it = allVariablesWithMesh.begin();
        it != allVariablesWithMesh.end(); it++) {
      VsVariableWithMesh* variable = it->second;
      delete (variable);
  }
  
  allVariablesWithMesh.clear();
  allVariablesWithMeshShort.clear();
}

void VsRegistry::writeAllVariablesWithMesh() const {
  if (allVariablesWithMesh.empty()) {
    VsLog::debugLog() <<"********** NO VsVariablesWithMesh *********" <<std::endl;
    return;
  }
  
  VsLog::debugLog() <<"********** BEGIN VsVariablesWithMesh *********" <<std::endl;
  
  for (std::map<std::string, VsVariableWithMesh*>::const_iterator it = allVariablesWithMesh.begin();
        it != allVariablesWithMesh.end(); it++) {
      VsVariableWithMesh* variable = it->second;
      variable->write();
  }
    
  VsLog::debugLog() <<"********** END VsVariablesWithMesh *********" <<std::endl;
}

void VsRegistry::getAllVariableWithMeshNames(std::vector<std::string>& names)  {
  std::map<std::string, VsVariableWithMesh*>::const_iterator it;
  for (it = allVariablesWithMesh.begin(); it != allVariablesWithMesh.end(); ++it)
    names.push_back(it->first);
}

/********************* TRANSFORMED MESH NAMES ************************/
bool VsRegistry::registerTransformedMeshName(std::string transformedName, std::string origName) {
  //first, look for a match and report failure if the name is already registered
  std::string oName = getOriginalMeshName(transformedName);
  if (!oName.empty()) {
    if (origName != oName) {
      VsLog::debugLog() << "ERROR VsRegistry::registerTransformedMeshName() - " 
                        << transformedName << " is already registered to " 
                        << oName << std::endl;
      return false;
    } else {
      VsLog::debugLog() << "VsRegistry::registerTransformedMeshName() - received duplicate registration for " 
                        << origName << std::endl;
      VsLog::debugLog() << "VsRegistry::registerTransformedMeshName() - but all info matches, so it should be ok"
                        << std::endl;
      return true;
    }
  }

  // Ok, register the new name mapping
  transformedMeshNames[transformedName] = origName;
  VsLog::debugLog() << "VsRegistry::registerTransformedMeshName(" 
                    << transformedName << ", " << transformedMeshNames[transformedName]  
                    << ") - registration succeeded." << std::endl;
  return true;
}

std::string VsRegistry::getOriginalMeshName(std::string transformedMeshName) {
  // return the value if the name is registered
  return transformedMeshNames[transformedMeshName];
}

/******************* TRANSFORMED VARS *******************/
bool VsRegistry::registerTransformedVarName(std::string transformedName, std::string origName) {
  //first, look for a match and report failure if the name is already registered
  std::string oName = getOriginalVarName(transformedName);
  if (!oName.empty()) {
    if (origName != oName) {
      VsLog::debugLog() << "ERROR VsRegistry::registerTransformedVarName() - "
                        << transformedName << " is already registered to "
                        << oName << std::endl;
      return false;
    } else {
      VsLog::debugLog() << "VsRegistry::registerTransformedVarName() - received duplicate registration for "
                        << origName << std::endl;
      VsLog::debugLog() << "VsRegistry::registerTransformedVarName() - but all info matches, so it should be ok"
                        << std::endl;
      return true;
    }
  }

  // Ok, register the new name mapping
  transformedVarNames[transformedName] = origName;
  VsLog::debugLog() << "VsRegistry::registerTransformedVarName("
                    << transformedName << ", " << transformedVarNames[transformedName]
                    << ") - registration succeeded." << std::endl;
  return true;
}

std::string VsRegistry::getOriginalVarName(std::string transformedVarName) {
  // return the value if the name is registered
  return transformedVarNames[transformedVarName];
}

/******************* EXPRESSIONS ************************/
void VsRegistry::addExpression(const std::string& name, 
                               const std::string& value) {
  //check for duplicates
  std::map<std::string, std::string>::const_iterator it;
  it = allExpressions.find(name);
  if (it != allExpressions.end()) {
    VsLog::errorLog() <<"VsRegistry::addExpression(" <<name <<", " <<value <<") - Duplicate name!  Rejecting expression." <<std::endl;
    return;
  }

  allExpressions[name] = value;
}

/**
 * VsRegistry::buildExpressions()
 * 
 * A "VsVars" group represents a set of names and values
 * Each attribute in the group represents one expression
 * The name of the attribute is the name of the expression
 * The value of the attribute is the value of the expression
 */
void VsRegistry::buildExpressions(VsGroup* group) {
  if (!group) {
    return;
  }
  VsLog::debugLog() <<"VsRegistry::buildExpressions() - Entering with group " <<group->getFullName() <<std::endl;
  
  std::map<std::string, VsAttribute*>::const_iterator k;
  for (k = group->getAllAttributes().begin(); k != group->getAllAttributes().end(); ++k) {
    VsAttribute* att = (*k).second;
    if (att->getShortName() != VsSchema::typeAtt) {
      std::string s;
      int err = att->getStringValue(&s);
      if (err == 0) {
        addExpression(att->getShortName(), s);
      }
    }
  }
  VsLog::debugLog() <<"VsRegistry::buildExpressions() exiting." <<std::endl;
}

void VsRegistry::writeAllExpressions() const {
  if (allExpressions.empty()) {
    VsLog::debugLog() <<"******** No Expressions *********" <<std::endl;
    return;
  }

  VsLog::debugLog() <<"******** BEGIN Expressions *********" <<std::endl;
  std::map<std::string, std::string>::const_iterator it;
  for (it = allExpressions.begin(); it != allExpressions.end(); it++) {
    VsLog::debugLog() <<"(" <<(*it).first <<", " <<(*it).second <<")" <<std::endl;
  }

  VsLog::debugLog() <<"******** END Expressions *********" <<std::endl;
}

std::map<std::string, std::string>* VsRegistry::getAllExpressions() {
  return &allExpressions;
}

int VsRegistry::numExpressions() {
  return (int)allExpressions.size();
}

void VsRegistry::createComponents() {
  VsLog::debugLog() <<"VsRegistry::createComponents() - Entering" <<std::endl;
  
  std::map<std::string, VsMDVariable*>::const_iterator it;
  for (it = allMDVariables.begin(); it != allMDVariables.end(); ++it) {
    VsLog::debugLog() <<"VsRegistry::createComponents() - Creating components for MD Variable " <<it->first <<std::endl;
    it->second->createComponents();
  }

  std::map<std::string, VsVariable*>::const_iterator it2;
  for (it2 = allVariables.begin(); it2 != allVariables.end(); ++it2) {
    VsLog::debugLog() <<"VsRegistry::createComponents() - Creating components for Variable " <<it2->first <<std::endl;
    it2->second->createComponents();
  }

  std::map<std::string, VsVariableWithMesh*>::const_iterator it3;
  for (it3 = allVariablesWithMesh.begin(); it3 != allVariablesWithMesh.end(); ++it3) {
    VsLog::debugLog() <<"VsRegistry::createComponents() - Creating components for Variable With Mesh " <<it3->first <<std::endl;
    it3->second->createComponents();
  }
  
  VsLog::debugLog() <<"VsRegistry::createComponents() - Returning." <<std::endl;
}
  
void VsRegistry::registerComponent(const std::string& varName, 
                                   int componentNumber, 
                                   const std::string& userSuppliedName) {
  
  //If the user supplied a name, try to use it
  if (!userSuppliedName.empty()) {
    registerComponentInfo(userSuppliedName, varName, componentNumber);
  }
  
  //for backwards compatibility, register the "old-style" name too                                                                      
  std::string oldStyleName = getOldComponentName(varName, componentNumber);
  if (oldStyleName != userSuppliedName) {
    registerComponentInfo(oldStyleName, varName, componentNumber);
  }
}

bool VsRegistry::registerComponentInfo(const std::string& componentName, 
                                       const std::string& varName, 
                                       int componentNumber) {
  //yes, I should use a std::hash_map for this
  
  //first, look for a match and report failure if the name is already registered
  NamePair foundPair;
  getComponentInfo(componentName, &foundPair);
  if (!foundPair.first.empty()) {
    std::string temp = foundPair.first;
    int tempIndex = foundPair.second;
    if ((varName != temp) || (componentNumber != tempIndex)) {
      VsLog::debugLog() <<"ERROR VsReader::registerComponentInfo() - " <<componentName <<" is already registered to component " <<temp <<" index " <<tempIndex <<std::endl;
      return false;
    } else {
      VsLog::debugLog() <<"VsReader::registerComponentInfo() - received duplicate registration for " <<varName <<" and index " <<componentNumber <<std::endl;
      VsLog::debugLog() <<"VsReader::registerComponentInfo() - but all info matches, so it should be ok" <<std::endl;
      return true;
    }
  }
  
  //Ok, register the new name mapping
  std::pair<std::string, int> innerPair;
  innerPair.first = varName;
  innerPair.second = componentNumber;
  std::pair<std::string, std::pair<std::string, int> > newPair;
  newPair.first = componentName;
  newPair.second = innerPair;
  componentNames.push_back(newPair);
  //VsLog::debugLog() <<"newpair.first = " <<newPair.first <<std::endl;
  //VsLog::debugLog() <<"newpair.second.first = " <<newPair.second.first <<std::endl;
  //VsLog::debugLog() <<"newpair.second.second = " <<newPair.second.second <<std::endl;
  
  VsLog::debugLog() <<"VsReader::registerComponentInfo(" <<componentName <<", " <<varName <<", " <<componentNumber <<") - registration succeeded." <<std::endl;
  return true;
}

void VsRegistry::getComponentInfo(const std::string& componentName, 
                                  NamePair* namePair) {
  //yes, I should use a std::hash_map for this
  
  //look for a match and return the value if the name is registered
  for (unsigned int i = 0; i < componentNames.size(); i++) {
    std::pair<std::string, NamePair > foundPair = componentNames[i];
    if (foundPair.first == componentName) {
      VsLog::debugLog() <<"VsReader::getComponentInfo(" <<componentName <<") - Found registered name, returning. " <<std::endl;
      namePair->first = foundPair.second.first;
      namePair->second = foundPair.second.second;
      return;
    }
  }
  
  namePair->first = "";
  namePair->second = -1;
}

std::string VsRegistry::getComponentName(const std::string& varName, 
                                         int componentNumber) {
  for (unsigned int i = 0; i < componentNames.size(); i++) {
    std::pair<std::string, NamePair > foundPair = componentNames[i];
    NamePair tempNamePair = foundPair.second;
    if ((tempNamePair.first == varName) && (tempNamePair.second == componentNumber)) {
      VsLog::debugLog() <<"VsReader::getComponentInfo(" <<varName <<", " <<componentNumber <<") - Found matching name & index, returning. " <<std::endl;
      return foundPair.first;
    }
  }

  VsLog::debugLog() <<"VsReader::getComponentInfo(" <<varName <<", " <<componentNumber <<") - no match found." <<std::endl;
  return "";
}

void VsRegistry::getComponentInfo(const std::string& varName, 
                                  int componentNumber, NamePair* namePair) {
  //yes, I should use a std::hash_map for this
  
  NamePair tempNamePair;
  //look for a match and return the value if the name is registered
  for (unsigned int i = 0; i < componentNames.size(); i++) {
    std::pair<std::string, NamePair > foundPair = componentNames[i];
    tempNamePair = foundPair.second;
    if ((tempNamePair.first == varName) && (tempNamePair.second == componentNumber)) {
      VsLog::debugLog() <<"VsReader::getComponentInfo(" <<varName <<", " <<componentNumber <<") - Found matching name & index, returning. " <<std::endl;
      namePair->first = tempNamePair.first;
      namePair->second = tempNamePair.second;
      return;
    }
  }
  
  VsLog::debugLog() <<"VsReader::getComponentInfo(" <<varName <<", " <<componentNumber <<") - no match found." <<std::endl;
  
  namePair->first = "";
  namePair->second = -1;
}

std::string VsRegistry::getOldComponentName(const std::string& varName, 
                                            int componentIndex) {
  //generates an old-style name for the component
  //of the form "varName_index"
  
  std::string compName = varName;
  std::stringstream ss;
  ss << componentIndex;
  compName.append("_");
  compName.append(ss.str());
  
  return compName;
}

