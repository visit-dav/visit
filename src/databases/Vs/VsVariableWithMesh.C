/*
 * VsVariableWithMesh.cpp
 *
 *  Created on: Apr 28, 2010
 *      Author: mdurant
 */

#include "VsVariableWithMesh.h"
#include "VsSchema.h"
#include "VsH5Dataset.h"
#include "VsLog.h"
#include "VsH5Attribute.h"
#include "VsUtils.h"

#include <string>
#include <map>
#include <vector>

VsVariableWithMesh::VsVariableWithMesh(VsH5Dataset* data):
  VsRegistryObject(data->registry) {
  indexOrder = VsSchema::compMinorCKey;
  dataset = data;
  timeGroup = NULL;
  
  registry->add(this);
}

VsVariableWithMesh::~VsVariableWithMesh() {
  registry->remove(this);
}

unsigned int VsVariableWithMesh::getNumSpatialDims() {
  return spatialIndices.size();
}

bool VsVariableWithMesh::isZonal() {
  return (centering == VsSchema::zonalCenteringKey);
}

bool VsVariableWithMesh::isCompMinor() {
  return ((indexOrder == VsSchema::compMinorCKey) ||
      (indexOrder == VsSchema::compMinorFKey));
}

bool VsVariableWithMesh::isCompMajor() {
  return ((indexOrder == VsSchema::compMajorCKey) ||
      (indexOrder == VsSchema::compMajorFKey));
}

// Get dims
std::vector<int> VsVariableWithMesh::getDims() {
  return dataset->getDims();
}

// Get hdf5 type
hid_t VsVariableWithMesh::getType() {
  return dataset->getType();
}

// Get length needed to store all elements in their format
size_t VsVariableWithMesh::getLength() {
  return dataset->getLength();
}

// Get name
std::string VsVariableWithMesh::getShortName () {
  return dataset->getShortName();
}

hid_t VsVariableWithMesh::getId() {
  return dataset->getId();
}

// Get path
std::string VsVariableWithMesh::getPath() {
  return dataset->getPath();
}

// Get full name
std::string VsVariableWithMesh::getFullName() {
  return dataset->getFullName();
}

// Find attribute by name, or return NULL if not found
VsH5Attribute* VsVariableWithMesh::getAttribute(const std::string name) {
  return dataset->getAttribute(name);
}

std::string VsVariableWithMesh::getStringAttribute(const std::string name) {
  VsH5Attribute* foundAtt = getAttribute(name);
  if (foundAtt == NULL)
    return "";
  std::string result = "";
  foundAtt->getStringValue(&result);
  return result;
}
//retrieve a particular spatial dimension index from the list
//returns -1 on failure
int VsVariableWithMesh::getSpatialDim(size_t index) {
  if ((index < 0) || (index > spatialIndices.size())) {
    return -1;
  }

  return spatialIndices[index];
}

void VsVariableWithMesh::write() {
  VsLog::debugLog() << getFullName() <<std::endl;
  VsLog::debugLog() << "    numSpatialDims  = " << getNumSpatialDims() << std::endl;
  VsLog::debugLog() << "    spatialIndices = [";
  for (unsigned int i = 0; i < getNumSpatialDims(); i++) {
    VsLog::debugLog() << spatialIndices[i];
    if (i + 1 < getNumSpatialDims()) {
      VsLog::debugLog() <<", ";
    }
  }
  VsLog::debugLog() <<"]" <<std::endl;
  VsLog::debugLog() << "    indexOrder = " << indexOrder << std::endl;
}

bool VsVariableWithMesh::initialize() {
  VsLog::debugLog() <<"VsVariableWithMesh::buildVariable() entering." <<std::endl;

  //We have two ways of specifying information for varWithMesh:
  // 1. VsSpatialIndices indicates which columns contain spatial data (synergia style)
  // 2. Spatial information is in the first "vsNumSpatialDims" columns (regular style)
  
  //we start with synergia style, and drop through to regular style on any errors
  bool numDimsSet = false;
  
  VsH5Attribute* spatialIndicesAtt = getAttribute(VsSchema::spatialIndicesAtt);
  if (spatialIndicesAtt) {
    VsLog::debugLog() <<"VsVariableWithMesh::buildVariable(): found spatialIndices, trying synergia style" <<std::endl;
    std::vector<int> in;
    herr_t err = spatialIndicesAtt->getIntVectorValue(&in);
    if (!err) {
      numDimsSet = true;
      this->spatialIndices = in;
      VsLog::debugLog() <<"VsVariableWithMesh::buildVariable(): Saved attribute in vm" <<std::endl;
    }
  }

  //NOTE: We load indexOrder regardless of whether we're in synergia style or not
  VsH5Attribute* indexOrderAtt = getAttribute(VsSchema::indexOrderAtt);
  if (indexOrderAtt) {
    VsLog::debugLog() <<"VsVariableWithMesh::buildVariable(): found indexOrder." <<std::endl;
    herr_t err = indexOrderAtt->getStringValue(&(this->indexOrder));
    if (err < 0) {
      VsLog::debugLog() << "VsH5Reader::makeVariableWithMeshMeta(...): '" <<
      getFullName() << "' error getting optional attribute '" <<
      VsSchema::indexOrderAtt << "'." << std::endl;
    }
  }
      
  //we tried and failed to load spatialIndices synergia style
  //so we drop back into the default - get the number of spatial dimensions
  //We then construct a spatialIndices array containing [0, 1, ..., numSpatialDims - 1]
  //So for a 3-d mesh we have [0, 1, 2]
  if (!numDimsSet) {
    VsLog::debugLog() <<"VsVariableWithMesh::buildVariable(): did not find spatialIndices, trying regular style." <<std::endl;
    VsH5Attribute* numDimsAtt = getAttribute(VsSchema::numSpatialDimsAtt);
    if (numDimsAtt) {
      std::vector<int> in;
      herr_t err = numDimsAtt->getIntVectorValue(&in);
      if (err < 0) {
        VsLog::debugLog() <<"VsVariableWithMesh::buildVariable(): VarWithMesh " <<getFullName();
        VsLog::debugLog() <<" does not have attribute " <<VsSchema::numSpatialDimsAtt <<"." <<std::endl;
        return false;
      }
      int numSpatialDims = in[0];
      
      //we construct a vector containing the proper spatialIndices
      this->spatialIndices.resize(numSpatialDims);
      for (int i = 0; i < numSpatialDims; i++) {
        this->spatialIndices[i] = i;
      }
        
      numDimsSet = true;
      VsLog::debugLog() << "VsVariableWithMesh::buildVariable(): numSpatialDims = " << this->getNumSpatialDims() << "." << std::endl;
    }
  }
  
  // Check that all set as needed
  if (!numDimsSet) {
    VsLog::debugLog() <<"VsVariableWithMesh::buildVariable(): Unable to determine spatial dimensions for var " <<getFullName() <<std::endl;
    return false;
  }

  //Get vsTimeGroup (optional attribute)
  VsH5Attribute* timeGroupAtt = dataset->getAttribute(VsSchema::timeGroupAtt);
  if (timeGroupAtt) {
    std::string timeGroupName;
    timeGroupAtt->getStringValue(&timeGroupName);
    timeGroup = registry->getGroup(timeGroupName);
  }
  
  //Get user-specified labels for components
  //labels is a comma-delimited list of strings
  VsH5Attribute* componentNamesAtt = dataset->getAttribute(VsSchema::labelsAtt);
  if (componentNamesAtt) {
    std::string names;
    componentNamesAtt->getStringValue(&names);

    tokenize(names, ',', this->labelNames);
  }
  
  return true;
}

// Get user-specified component names
std::string VsVariableWithMesh::getLabel (unsigned int i) {
  if ((i >= 0) && (i < labelNames.size()) && !labelNames[i].empty()) {
    return makeCanonicalName(getPath(), labelNames[i]);
  } 

  return "";
}

VsVariableWithMesh* VsVariableWithMesh::buildObject(VsH5Dataset* dataset) {
  VsVariableWithMesh* newVar = new VsVariableWithMesh(dataset);
  bool success = newVar->initialize();
  if (success) {
    return newVar;
  }
  
  delete(newVar);
  newVar = NULL;
  return NULL;
}

size_t VsVariableWithMesh::getNumComps() {
  std::vector<int> dims = getDims();
  if (dims.size() <= 0) {
    VsLog::errorLog() <<"VsVariableWithMesh::createComponents() - unable to get dimensions of variable?" << std::endl;
    return 0;
  }

  size_t lastDim = 0;
  if (isCompMinor())
    lastDim = dims[dims.size()-1];
  else lastDim = dims[0];
  
  return lastDim;
}

void VsVariableWithMesh::createComponents() {
  VsLog::debugLog() <<"VsVariableWithMesh::createComponents() - Entering" << std::endl;
  
  size_t numComps = getNumComps();
  
  for (size_t i = 0; i < numComps; ++i) {
    registry->registerComponent(getFullName(), i, getLabel(i));
  }

  VsLog::debugLog() <<"VsVariableWithMesh::createComponents() - Returning" << std::endl;
}
