/*
 * VsVariable.cpp
 *
 *  Created on: Apr 28, 2010
 *      Author: mdurant
 */

#include "VsVariable.h"
#include "VsSchema.h"
#include "VsH5Dataset.h"
#include "VsLog.h"
#include "VsH5Attribute.h"
#include "VsMDVariable.h"
#include "VsMDMesh.h"
#include "VsUtils.h"
#include "VsMesh.h"

#include <map>
#include <vector>
#include <string>

VsVariable::VsVariable(VsH5Dataset* data):
  VsRegistryObject(data->registry) {
  indexOrder = VsSchema::compMinorCKey; //default
  dataset = 0;
  centering = "nodal";
  meshName = "";
  meshMeta = NULL;
  timeGroup = NULL;

  dataset = data;
  registry->add(this);
}

VsVariable::~VsVariable() {
  registry->remove(this);
}

bool VsVariable::isZonal() {
  return (centering == VsSchema::zonalCenteringKey);
}

bool VsVariable::isFortranOrder() {
  return ((indexOrder == VsSchema::compMinorFKey) ||
          (indexOrder == VsSchema::compMajorFKey));
}

bool VsVariable::isCompMinor() {
  return ((indexOrder == VsSchema::compMinorCKey) ||
          (indexOrder == VsSchema::compMinorFKey));
}

bool VsVariable::isCompMajor() {
  return ((indexOrder == VsSchema::compMajorCKey) ||
          (indexOrder == VsSchema::compMajorFKey));
}

// Get dims
std::vector<int> VsVariable::getDims() {
  return dataset->getDims();
}

// Get mesh name
std::string VsVariable::getMeshName() {
  return meshName;
}

VsMesh* VsVariable::getMesh() {
  return meshMeta;
}

// Get hdf5 type
hid_t VsVariable::getType() {
  return dataset->getType();
}

// Get length needed to store all elements in their format
size_t VsVariable::getLength() {
  return dataset->getLength();
}

// Get name
std::string VsVariable::getShortName () {
  return dataset->getShortName();
}

hid_t VsVariable::getId() {
  return dataset->getId();
}

// Get path
std::string VsVariable::getPath() {
  return dataset->getPath();
}

// Get full name
std::string VsVariable::getFullName() {
  return dataset->getFullName();
}

// Find attribute by name, or return NULL if not found
VsH5Attribute* VsVariable::getAttribute(const std::string name) {
  return dataset->getAttribute(name);
}

std::string VsVariable::getStringAttribute(const std::string name) {
  VsH5Attribute* foundAtt = getAttribute(name);
  if (foundAtt == NULL)
    return "";
  std::string result = "";
  foundAtt->getStringValue(&result);
  return result;
}

void VsVariable::write() {
  VsLog::debugLog() << getFullName() <<std::endl;
  VsLog::debugLog() << "    indexOrder = " << indexOrder << std::endl;
  VsLog::debugLog() << "    centering = " << centering << std::endl;
  VsLog::debugLog() << "    meshName = " << meshName << std::endl;
}

// Get user-specified component names
std::string VsVariable::getLabel (unsigned int i) {
  if ((i >= 0) && (i < labelNames.size()) && !labelNames[i].empty()) {
    return makeCanonicalName(getPath(), labelNames[i]);
  } 

  return "";
}

VsVariable* VsVariable::buildObject(VsH5Dataset* dataset) {
  VsVariable* newVar = new VsVariable(dataset);
  bool success = newVar->initialize();
  if (success) {
    return newVar;
  }
  
  delete(newVar);
  newVar = NULL;
  return NULL;
}

bool VsVariable::initialize() {
  VsLog::debugLog() <<"VsVariable::initialize() - Entering." <<std::endl;
  
  //Get the name of our mesh (required attribute)
  VsH5Attribute* meshNameAtt = dataset->getAttribute(VsSchema::meshAtt);
  if (!meshNameAtt) {
    VsLog::errorLog() << "VsVariable::initialize(): error getting required attribute '" 
      <<VsSchema::meshAtt << "." << std::endl;
    VsLog::errorLog() <<"VsVariable::initialize(): returning failure" <<std::endl;
    return false;
  }
  
  std::string meshName;
  meshNameAtt->getStringValue(&meshName);
  this->meshName = makeCanonicalName(dataset->getPath(), meshName);
  
  //Does mesh exist?
  this->meshMeta = registry->getMesh(this->meshName);
  if (!this->meshMeta) {
    VsLog::errorLog() <<"VsVariable::initialize() - Unable to find mesh with name " <<this->meshName <<std::endl;
    VsLog::errorLog() <<"VsVariable::initialize(): returning failure" <<std::endl;
    return false;
  }

  //Get indexOrder (optional attribute)
  VsH5Attribute* indexOrderAtt = dataset->getAttribute(VsSchema::indexOrderAtt);
  if (indexOrderAtt) {
    indexOrderAtt->getStringValue(&(this->indexOrder));
  }

  //Get vsTimeGroup (optional attribute)
  VsH5Attribute* timeGroupAtt = dataset->getAttribute(VsSchema::timeGroupAtt);
  if (timeGroupAtt) {
    std::string timeGroupName;
    timeGroupAtt->getStringValue(&timeGroupName);
    timeGroup = registry->getGroup(timeGroupName);
  }
  
  //Get centering (optional attribute)
  VsH5Attribute* centeringAtt = dataset->getAttribute(VsSchema::centeringAtt);
  if (centeringAtt) {
    centeringAtt->getStringValue(&(this->centering));
  }

  //Get user-specified labels for components
  //labels is a comma-delimited list of strings
  VsH5Attribute* componentNamesAtt = dataset->getAttribute(VsSchema::labelsAtt);
  if (componentNamesAtt) {
    std::string names;
    componentNamesAtt->getStringValue(&names);
    
    tokenize(names, ',', this->labelNames);
  }
  
  VsLog::debugLog() <<"VsVariable::initialize() returning success." <<std::endl;
  return true;
}

size_t VsVariable::getNumComps() {
  VsLog::debugLog() << "VsVariable::getNumComps(): Entering." << std::endl;
  
  std::vector<int> dataDims = getDims();

  //load the mesh dimensions
  std::vector<int> meshDims;
  meshMeta->getMeshDataDims(meshDims);
  
  //did we get a reasonable value?
  if (meshDims.empty()) {
    VsLog::debugLog() <<"VsVariable::getNumComps(): Unable to get mesh dimensions for mesh." <<std::endl;
    VsLog::debugLog() <<"VsVariable::getNumComps(): Returning 0." <<std::endl;
    return 0;
  }

  //we do not want to adjust the size of the variable array here
  // Because we're trying to determine how many components the variable has
  // and so we don't know if we would need to adjust ALL of the dimensions
  // or just all-but-one.
  //adjustSize_vector(&ddims, stride, VsLog::debugLog());
  
  
  // Debugging information
  VsLog::debugLog() << "VsVariable::getNumComps(): ";
  VsLog::debugLog() << "For data, " << getFullName() << ", dataDims =";
  for (size_t i=0; i<dataDims.size(); ++i) VsLog::debugLog() << " " << dataDims[i];
  VsLog::debugLog() << "." << std::endl;;
  VsLog::debugLog() << "VsVariable::getNumComps():";
  VsLog::debugLog() << "For mesh, " << getMeshName() << ", meshDims =";
  for (size_t i=0; i<meshDims.size(); ++i) VsLog::debugLog() << " " << meshDims[i];
  VsLog::debugLog() << "." << std::endl;
  
  size_t res = 0;
  // For unstructured meshes
  if (meshMeta->isUnstructuredMesh()) {
    if ((meshDims.size() != 1) && (meshDims.size() != 2)) {
      VsLog::debugLog() << "VsVariable::getNumComps(): error: dataset '" << getFullName() <<
      "' does not have 1 or 2 dimensions.  Number of components unknown." <<
      std::endl;
      VsLog::debugLog() << "VsVariable::getNumComps(): Returning 0." << std::endl;
      return 0;
    }
    
    //is ddims a single component?
    if (dataDims.size() == 1) {
      VsLog::debugLog() << "VsVariable::getNumComps(): This dataset is 1-d, so 1 component." <<std::endl;
      //yes, it's a simple list of values
      res = 1;
    }
    else {
      if (isCompMajor()) {
        res = dataDims.front();
      }
      else { //compMinor
        res = dataDims.back();
      }
      VsLog::debugLog() << "VsVariable::getNumComps(): This dataset has " <<res <<" components." <<std::endl;
    }
    goto dimwarn;
  }
  
  // Compare dims
  if (meshMeta->isStructuredMesh()) {
    // Structure mesh has an extra dimension for the coordinate index
    if (meshDims.size() > 1) meshDims.resize(meshDims.size() - 1);
  }
  if (meshDims.size() == dataDims.size()) {
    res = 1;
    goto dimwarn;
  }
  if ((dataDims.size() == 1)
      || ((dataDims.size() == 2) && (dataDims[0] == 1))) {
    res = 1;
    goto dimwarn;
  }
  if (meshDims.size() != (dataDims.size() - 1) ) {
    VsLog::debugLog() << "VsVariable::getNumComps(): error - mesh '" << getMeshName() <<
    "' has dimensions of size, " << meshDims.size() << ", while dataset '" <<
    getFullName() << "' has dimensions of size, " << dataDims.size() <<
    ", which does not equal mdims.size() or mdims.size() + 1." << std::endl;
    VsLog::debugLog() << "VsVariable::getNumComps(): Returning 0." << std::endl;
    return 0;
  }
  
  // check that each of mesh sizes are correct (for compMinor data here)
  if (!isCompMajor()) {
    res = dataDims.back();
  } else {
    res = dataDims.front();
  }
  
dimwarn:
  VsLog::debugLog() << "VsVariable::getNumComps(): warning the size of the "
  "variable '" << getFullName() << "' not validated.  Future work." << std::endl;
  VsLog::debugLog() << "VsVariable::getNumComps(): Returning " <<res <<" components." << std::endl;
  return res;
  
}

void VsVariable::createComponents() {
  //Name & register components
  VsLog::debugLog() <<"VsVariable::createComponents() - Creating component names." <<std::endl;
  
  // Number of component of the var                                                                                                         
  size_t numComps = getNumComps();
  
  //Note that single-component variables just use the base variable name
  //I.E. instead of a singleton named "var_0", we just call it "var"
  if (numComps > 1) {
    for (size_t i = 0; i < numComps; ++i) {
      registry->registerComponent(getFullName(), i, getLabel(i));
    }
  }
}
