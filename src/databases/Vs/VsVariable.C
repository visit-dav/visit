/*
 * VsVariable.cpp
 *
 *  Created on: Apr 28, 2010
 *      Author: mdurant
 */

#include "VsVariable.h"
#include "VsSchema.h"
#include "VsDataset.h"
#include "VsLog.h"
#include "VsAttribute.h"
#include "VsMDVariable.h"
#include "VsMDMesh.h"
#include "VsUtils.h"
#include "VsMesh.h"

#include <map>
#include <vector>
#include <string>

#define __CLASS__ "VsVariable::"

VsVariable::VsVariable(VsDataset* data):
  VsRegistryObject(data->registry) {
  indexOrder = VsSchema::compMinorCKey; //default
  dataset = 0;
  centering = "nodal";
  meshName = "";
  meshMeta = NULL;
  timeGroup = NULL;

  dataset = data;
  registry->add(this);

  this->nodeOffsetAtt = NULL;
  this->nodeOffset.resize(3);
  for (size_t i = 0; i < 3; ++i) {
    this->nodeOffset[i] = 0;
  }
}

VsVariable::~VsVariable() {
  registry->remove(this);
}

bool VsVariable::isZonal() const {
  return (centering == VsSchema::zonalCenteringKey);
}

bool VsVariable::isNodal() const {
  return (centering == VsSchema::nodalCenteringKey);
}

bool VsVariable::isEdge() const {
  return (centering == VsSchema::edgeCenteringKey);
}

bool VsVariable::isFace() const {
  return (centering == VsSchema::faceCenteringKey);
}

bool VsVariable::isFortranOrder() const {
  return ((indexOrder == VsSchema::compMinorFKey) ||
          (indexOrder == VsSchema::compMajorFKey));
}

bool VsVariable::isCompMinor() const {
  return ((indexOrder == VsSchema::compMinorCKey) ||
          (indexOrder == VsSchema::compMinorFKey));
}

bool VsVariable::isCompMajor() const {
  return ((indexOrder == VsSchema::compMajorCKey) ||
          (indexOrder == VsSchema::compMajorFKey));
}

// Get dims
std::vector<int> VsVariable::getDims() const {
  return dataset->getDims();
}

// Get mesh name
std::string VsVariable::getMeshName() const {
  return meshName;
}

VsMesh* VsVariable::getMesh() const {
  return meshMeta;
}

bool VsVariable::hasTransform() const {
  return meshMeta && meshMeta->hasTransform();
}

// Get hdf5 type
hid_t VsVariable::getType() const {
  return dataset->getType();
}

// Get length needed to store all elements in their format
size_t VsVariable::getLength() const {
  return dataset->getLength();
}

// Get name
std::string VsVariable::getShortName () const {
  return dataset->getShortName();
}

hid_t VsVariable::getId() const {
  return dataset->getId();
}

// Get path
std::string VsVariable::getPath() const {
  return dataset->getPath();
}

// Get transformed name
std::string VsVariable::getFullTransformedName() const {
  return getFullName() + "_transform";
}

// Get full name
std::string VsVariable::getFullName() const {
  return dataset->getFullName();
}

// Find attribute by name, or return NULL if not found
VsAttribute* VsVariable::getAttribute(const std::string& name) const {
  VsLog::debugLog() << "getatt " << name <<dataset->getAttribute(name) << std::endl;
  return dataset->getAttribute(name);
}

std::string VsVariable::getStringAttribute(const std::string& name) const {
  VsLog::debugLog() << "getstringatt " << name <<dataset->getAttribute(name) << std::endl;
  VsAttribute* foundAtt = getAttribute(name);
  if (foundAtt == NULL)
    return "";
  std::string result = "";
  foundAtt->getStringValue(&result);
  return result;
}

void VsVariable::write() const {
  VsLog::debugLog() << getFullName() <<std::endl;
  VsLog::debugLog() << "    indexOrder = " << indexOrder << std::endl;
  VsLog::debugLog() << "    centering = " << centering << std::endl;
  VsLog::debugLog() << "    meshName = " << meshName << std::endl;
  VsLog::debugLog() << "    nodeOffset = ";
  for (size_t i = 0; i < nodeOffset.size(); ++i) {
    VsLog::debugLog() << nodeOffset[i] << " ";
  }
  VsLog::debugLog() << std::endl;
}

// Get user-specified component names
std::string VsVariable::getLabel (size_t i) const {
//  if ((i >= 0) && (i < labelNames.size()) && !labelNames[i].empty()) {
  if ((i < labelNames.size()) && !labelNames[i].empty()) {
    return makeCanonicalName(getPath(), labelNames[i]);
  } 

  return "";
}

VsVariable* VsVariable::buildObject(VsDataset* dataset) {
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
  VsAttribute* meshNameAtt = dataset->getAttribute(VsSchema::meshAtt);
  VsLog::debugLog() <<"VsVariable:: meshNameAtt." <<std::endl;
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
  VsLog::debugLog() <<"VsVariable:: meshMeta." <<std::endl;
  if (!this->meshMeta) {
    VsLog::errorLog() <<"VsVariable::initialize() - Unable to find mesh with name " <<this->meshName <<std::endl;
    VsLog::errorLog() <<"VsVariable::initialize(): returning failure" <<std::endl;
    return false;
  }

  //Get indexOrder (optional attribute)
  VsAttribute* indexOrderAtt = dataset->getAttribute(VsSchema::indexOrderAtt);
  VsLog::debugLog() <<"VsVariable:: indexorder." <<std::endl;
  if (indexOrderAtt) {
    indexOrderAtt->getStringValue(&(this->indexOrder));
  }

  //Get vsTimeGroup (optional attribute)
  VsLog::debugLog() <<"VsVariable:: timeGroupAtt: " <<VsSchema::timeGroupAtt<<std::endl;
  VsAttribute* timeGroupAtt = dataset->getAttribute(VsSchema::timeGroupAtt);
  VsLog::debugLog() <<"VsVariable:: timeGroupAtt: "<< timeGroupAtt <<std::endl;
  if (timeGroupAtt) {
    std::string timeGroupName;
    timeGroupAtt->getStringValue(&timeGroupName);
    timeGroup = registry->getGroup(timeGroupName);
  }
  
  //Get centering (optional attribute)
  VsAttribute* centeringAtt = dataset->getAttribute(VsSchema::centeringAtt);
  if (centeringAtt) {
    centeringAtt->getStringValue(&(this->centering));
  }

  this->nodeOffsetAtt = dataset->getAttribute(VsSchema::nodeOffsetAtt);
  if (this->nodeOffsetAtt) {
    std::vector<float> nodeOffsetFloat(3, 0);
    std::vector<int> nodeOffsetInt(3, 0);
    int err = this->nodeOffsetAtt->getDoubleVectorValue(&this->nodeOffset);
    if (err < 0) {
      int err2 = this->nodeOffsetAtt->getFloatVectorValue(&nodeOffsetFloat);
      if (err2 < 0) {
        int err3 = this->nodeOffsetAtt->getIntVectorValue(&nodeOffsetInt);
        if (err3 < 0) {
        VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Cannot get vector nodeOffset." << std::endl;
        }
        else {
          // nodeOffset is of type int vector
          // cast into double vectot
          for (size_t i = 0; i < nodeOffsetInt.size(); ++i) {
            this->nodeOffset[i] = nodeOffsetInt[i];
          }
        }
      }
      else {
        // nodeOffset is of type float vector
        // cast into double vector
        for (size_t i = 0; i < nodeOffsetFloat.size(); ++i) {
          this->nodeOffset[i] = nodeOffsetFloat[i];
        }
      }
    }

    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "vector nodeOffset is " 
                      << nodeOffset[0] << " " 
                      << nodeOffset[1] << " " 
                      << nodeOffset[2] << std::endl;

  }

  //Get user-specified labels for components
  //labels is a comma-delimited list of strings
  VsAttribute* componentNamesAtt = dataset->getAttribute(VsSchema::labelsAtt);
  if (componentNamesAtt) {
    std::string names;
    componentNamesAtt->getStringValue(&names);
    tokenize(names, ',', this->labelNames);
  }
  
  VsLog::debugLog() <<"VsVariable::initialize() returning success." <<std::endl;
  return true;
}

size_t VsVariable::getNumComps() const{
  VsLog::debugLog() << "VsVariable::getNumComps(): Entering." << std::endl;
  
  std::vector<int> dataDims = getDims();

  //load the mesh dimensions
  std::vector<int> meshDims;
  meshMeta->getCellDims(meshDims);
  
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
  if (meshMeta->isUnstructuredMesh() ) {
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

void VsVariable::createTransformedVariable() {
  VsLog::debugLog() <<"VsVariable::createTransformedVariable() - Creating transformed var name." <<std::endl;

  // Does this variable have a transformation?
  bool hasTransform = getMesh() && getMesh()->hasTransform();

  if (hasTransform) {
    VsLog::debugLog()<<"VsVariable::createTransformedVariable() - registering transformed variable: " + getFullTransformedName() <<std::endl;
    registry->registerTransformedVarName(getFullTransformedName(), getFullName());
  }

  VsLog::debugLog() <<"VsVariable::createTransformedVariable() - returning." <<std::endl;
}

void VsVariable::createComponents() {
  //Name & register components
  VsLog::debugLog() <<"VsVariable::createComponents() - Creating component names." <<std::endl;
  
  // Number of component of the var                                                                                                         
  size_t numComps = getNumComps();

  // Does this variable have a transformation?
  bool transformExists = this->hasTransform();
  
  //Note that single-component variables just use the base variable name
  //I.E. instead of a singleton named "var_0", we just call it "var"
  if (numComps > 1) {
    for (size_t i = 0; i < numComps; ++i) {
      registry->registerComponent(getFullName(), (int) i, getLabel(i));
      if (transformExists) {
        registry->registerComponent(getFullTransformedName(), (int)i, getLabel(i));
      }
    }
  }
}

std::vector<double> VsVariable::getNodeOffset() const
{
  return this->nodeOffset;
}

bool VsVariable::hasNodeOffset() const
{
  for (size_t i = 0; i < this->nodeOffset.size(); ++i) {
    if (this->nodeOffset[i] != 0) {
      return true;
    }
  }
  return false;
}
