/*
 * VsVariableWithMesh.cpp
 *
 *  Created on: Apr 28, 2010
 *      Author: mdurant
 */

#include "VsVariableWithMesh.h"
#include "VsSchema.h"
#include "VsDataset.h"
#include "VsLog.h"
#include "VsAttribute.h"
#include "VsUtils.h"

#include <string>
#include <map>
#include <vector>

#define __CLASS__ "VsStructuredMesh::"


VsVariableWithMesh::VsVariableWithMesh(VsDataset* data):
  VsRegistryObject(data->registry) {
  indexOrder = VsSchema::compMinorCKey;
  dataset = data;
  timeGroup = NULL;
  
  registry->add(this);
}

VsVariableWithMesh::~VsVariableWithMesh() {
  registry->remove(this);
}

size_t VsVariableWithMesh::getNumSpatialDims() const {
  return spatialIndices.size();
}

bool VsVariableWithMesh::isZonal() const {
  return (centering == VsSchema::zonalCenteringKey);
}

bool VsVariableWithMesh::isNodal() const {
  return (centering == VsSchema::nodalCenteringKey);
}

bool VsVariableWithMesh::isEdge() const {
  return (centering == VsSchema::edgeCenteringKey);
}

bool VsVariableWithMesh::isFace() const {
  return (centering == VsSchema::faceCenteringKey);
}

bool VsVariableWithMesh::isCompMinor() const {
  return ((indexOrder == VsSchema::compMinorCKey) ||
      (indexOrder == VsSchema::compMinorFKey));
}

bool VsVariableWithMesh::isCompMajor() const {
  return ((indexOrder == VsSchema::compMajorCKey) ||
      (indexOrder == VsSchema::compMajorFKey));
}

std::string VsVariableWithMesh::getFullTransformedName() const {
  return getFullName() + "_transform";
}

bool VsVariableWithMesh::hasTransform() const {
  return (!getTransformName().empty());
}

std::string VsVariableWithMesh::getTransformName() const {
  //Look for the vsTransform attribute
  //and either retrieve the value or leave the name empty
  std::string transformName;
  VsAttribute* transformNameAtt = getAttribute(VsSchema::transformKey);
  if (transformNameAtt) {
    transformNameAtt->getStringValue(&transformName);
  }
  
  //Make sure this is a recognized value
  //All other methods use the return value of this method as a go/no-go test
  //So this is the best place to catch bad values
  if ((transformName != VsSchema::zrphiTransformKey) &&
      (transformName != VsSchema::zrphiTransformKey_deprecated)) {
    VsLog::errorLog() <<"VsVariableWithMesh::getTransformName() - Unrecognized value for key "
    << VsSchema::transformKey << " - " <<transformName <<std::endl;
    transformName = "";
  }
  
  return transformName;
}

std::string VsVariableWithMesh::getTransformedMeshName() const {
  //Look for the vsTransformName key
  std::string transformedMeshName;
  VsAttribute* transformedMeshNameAtt = getAttribute(VsSchema::transformedMeshKey);
  if (transformedMeshNameAtt) {
    transformedMeshNameAtt->getStringValue(&transformedMeshName);
    if (!transformedMeshName.empty()) {
      //We want to make the tranformed mesh appear at the same file level
      //as the original mesh.
      //So, when we calculate the canonical name, use the PATH, not the FULL NAME
      transformedMeshName = makeCanonicalName(getPath(), transformedMeshName);
    }
  }
  
  // if we didn't find a user supplied name, create a name
  if (transformedMeshName.empty()) {
    transformedMeshName = getFullName() + "_transform";
    transformedMeshName = makeCanonicalName(transformedMeshName);
  }
  
  return transformedMeshName;
}

void VsVariableWithMesh::createTransformedVariableAndMesh() {
  VsLog::debugLog() <<"VsVariableWithMesh::createTransformedVariableAndMesh() - Creating transformed var name." <<std::endl;
  
  // Does this variable have a transformation?
  if (hasTransform()) {
    VsLog::debugLog()<<"VsVariableWithMesh::createTransformedVariableAndMesh() - registering transformed variable: " + getFullTransformedName() <<std::endl;
    registry->registerTransformedVarName(getFullTransformedName(), getFullName());
    
    //And register the transformed mesh name to match
    registry->registerTransformedMeshName(getFullTransformedName(), getFullName());
  }
  
  VsLog::debugLog() <<"VsVariableWithMesh::createTransformedVariable() - returning." <<std::endl;
}

// Get dims
std::vector<int> VsVariableWithMesh::getDims() const
{
  return dataset->getDims();
}

void VsVariableWithMesh::getCellDims(std::vector<int>& dims) const
{
  dims = dataset->getDims();
}

void VsVariableWithMesh::getNodeDims(std::vector<int>& dims) const
{
  dims.resize(1);

  dims[0] = (int)getNumPoints();
}

size_t VsVariableWithMesh::getNumPoints() const
{
  if( isCompMinor() )
    return dataset->getDims()[0];
  else
    return dataset->getDims()[1];
}

// Get hdf5 type
hid_t VsVariableWithMesh::getType() const {
  return dataset->getType();
}

// Get length needed to store all elements in their format
size_t VsVariableWithMesh::getLength() const {
  return dataset->getLength();
}

// Get name
std::string VsVariableWithMesh::getShortName () const {
  return dataset->getShortName();
}

hid_t VsVariableWithMesh::getId() const {
  return dataset->getId();
}

// Get path
std::string VsVariableWithMesh::getPath() const {
  return dataset->getPath();
}

// Get full name
std::string VsVariableWithMesh::getFullName() const {
  return dataset->getFullName();
}

// Find attribute by name, or return NULL if not found
VsAttribute* VsVariableWithMesh::getAttribute(const std::string& name) const {
  return dataset->getAttribute(name);
}

std::string VsVariableWithMesh::getStringAttribute(const std::string& name) const {

  std::string result("");

  VsAttribute* foundAtt = getAttribute(name);
  if (foundAtt)
    foundAtt->getStringValue(&result);

  return result;
}
//retrieve a particular spatial dimension index from the list
//returns -1 on failure
int VsVariableWithMesh::getSpatialDim(size_t index) const {
  if (index > spatialIndices.size()) {
    return -1;
  }

  return spatialIndices[index];
}

void VsVariableWithMesh::write() const {
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << getFullName() << "  "
                    << "indexOrder = " << indexOrder << "  "
                    << "numSpatialDims  = " << getNumSpatialDims() << "  "
                    << "spatialIndices = [";

  for (unsigned int i = 0; i < getNumSpatialDims(); i++) {
    VsLog::debugLog() << spatialIndices[i];
    if (i + 1 < getNumSpatialDims()) {
      VsLog::debugLog() <<", ";
    }
  }
  VsLog::debugLog() <<"]" << std::endl;
}

bool VsVariableWithMesh::initialize() {
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << " entering." << std::endl;

  //We have two ways of specifying information for varWithMesh:
  // 1. VsSpatialIndices indicates which columns contain spatial data
  // (synergia style)
  // 2. Spatial information is in the first "vsNumSpatialDims" columns
  // (regular style)
  //we start with synergia style, and drop through to regular style on
  //any errors
  bool numDimsSet = false;
  
  VsAttribute* spatialIndicesAtt = getAttribute(VsSchema::spatialIndicesAtt);
  if (spatialIndicesAtt) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "found spatialIndices, trying synergia style"
                      << std::endl;

    std::vector<int> in;
    size_t err = spatialIndicesAtt->getIntVectorValue(&in);
    if (!err) {
      numDimsSet = true;
      this->spatialIndices = in;
      VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Saved attribute in vm" << std::endl;
    }
  }

  //NOTE: We load indexOrder regardless of whether we're in synergia
  //style or not
  VsAttribute* indexOrderAtt = getAttribute(VsSchema::indexOrderAtt);
  if (indexOrderAtt) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "found indexOrder." << std::endl;
    int err = indexOrderAtt->getStringValue(&(this->indexOrder));
    if (err < 0) {
      VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                        << getFullName()
                        << "' error getting optional attribute '"
                        << VsSchema::indexOrderAtt << "'." << std::endl;
    }
  }
      
  //we tried and failed to load spatialIndices synergia style so we
  //drop back into the default - get the number of spatial dimensions
  //We then construct a spatialIndices array containing [0, 1, ...,
  //numSpatialDims - 1] So for a 3-d mesh we have [0, 1, 2]
  if (!numDimsSet) {
      VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "did not find spatialIndices, trying regular style."
                        << std::endl;
      VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Looking for attribute: "
                        << VsSchema::numSpatialDimsAtt << std::endl;

    VsAttribute* numDimsAtt = getAttribute(VsSchema::numSpatialDimsAtt);
    if (!numDimsAtt) {

      VsLog::warningLog()
        << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
        << "Did not find attribute: "
        << VsSchema::numSpatialDimsAtt << "  "
        << "Looking for deprecated attribute: "
        << VsSchema::numSpatialDimsAtt_deprecated << std::endl;

      numDimsAtt = getAttribute(VsSchema::numSpatialDimsAtt_deprecated);
    }
    if (numDimsAtt) {
      std::vector<int> in;
      int err = numDimsAtt->getIntVectorValue(&in);
      if (err < 0) {
        VsLog::debugLog()
          << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
          << getFullName() << " does not have attribute "
          << VsSchema::numSpatialDimsAtt << "." << std::endl;

        return false;
      }
      int numSpatialDims = in[0];
      
      //we construct a vector containing the proper spatialIndices
      this->spatialIndices.resize(numSpatialDims);
      for (int i = 0; i < numSpatialDims; i++) {
        this->spatialIndices[i] = i;
      }
        
      numDimsSet = true;
      VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "numSpatialDims = " << this->getNumSpatialDims() << "."
                        << std::endl;
    } else {
      VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Did not find deprecated attribute either: "
                        << VsSchema::numSpatialDimsAtt_deprecated << std::endl;
    }
  }
  
  // Check that all set as needed
  if (!numDimsSet) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to determine spatial dimensions for var "
                      << getFullName() << std::endl;
    return false;
  }

  //Get vsTimeGroup (optional attribute)
  VsAttribute* timeGroupAtt = dataset->getAttribute(VsSchema::timeGroupAtt);
  if (timeGroupAtt) {
    std::string timeGroupName;
    timeGroupAtt->getStringValue(&timeGroupName);
    timeGroup = registry->getGroup(timeGroupName);
  }
  
  //Get user-specified labels for components
  //labels is a comma-delimited list of strings
  VsAttribute* componentNamesAtt = dataset->getAttribute(VsSchema::labelsAtt);
  if (componentNamesAtt) {
    std::string names;
    componentNamesAtt->getStringValue(&names);

    tokenize(names, ',', this->labelNames);
  }
  
  return true;
}

// Get user-specified component names
std::string VsVariableWithMesh::getLabel (size_t i) const {
//  if ((i >= 0) && (i < labelNames.size()) && !labelNames[i].empty()) {
  if ((i < labelNames.size()) && !labelNames[i].empty()) {
    return makeCanonicalName(getPath(), labelNames[i]);
  } 

  return "";
}

VsVariableWithMesh* VsVariableWithMesh::buildObject(VsDataset* dataset) {
  VsVariableWithMesh* newVar = new VsVariableWithMesh(dataset);
  bool success = newVar->initialize();
  if (success) {
    return newVar;
  }
  
  delete(newVar);
  newVar = NULL;
  return NULL;
}

size_t VsVariableWithMesh::getNumComps() const {
  std::vector<int> dims = getDims();
  if (dims.size() <= 0) {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to get dimensions of variable?" << std::endl;
    return 0;
  }

  size_t lastDim = 0;
  if (isCompMinor())
    lastDim = dims[dims.size()-1];
  else lastDim = dims[0];
  
  return lastDim;
}

void VsVariableWithMesh::createComponents() {
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Entering" << std::endl;
  
  size_t numComps = getNumComps();
  
  //We should only create component names if we have more than one component
  //But i'm going to leave it as-is for now...
  bool transformExists = hasTransform();
  for (size_t i = 0; i < numComps; ++i) {
    registry->registerComponent(getFullName(), (int)i, getLabel(i));
    if (transformExists) {
      registry->registerComponent(getFullTransformedName(), (int)i, getLabel(i));
    }
  }

  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Returning" << std::endl;
}
