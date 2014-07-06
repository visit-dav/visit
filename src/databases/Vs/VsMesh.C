/*
 * VsMesh.cpp
 *
 *  Created on: Apr 28, 2010
 *      Author: mdurant
 */

#include "VsMesh.h"
#include "VsDataset.h"
#include "VsAttribute.h"
#include "VsUnstructuredMesh.h"
#include "VsStructuredMesh.h"
#include "VsRectilinearMesh.h"
#include "VsUniformMesh.h"
#include "VsGroup.h"
#include "VsSchema.h"
#include "VsLog.h"
#include "VsUtils.h"
#include "VsMDMesh.h"

#define __CLASS__ "VsMesh::"


VsMesh::VsMesh(VsObject* object):VsRegistryObject(object->registry) {
  numSpatialDims = -1;
  numTopologicalDims = -1;
  indexOrder = VsSchema::compMinorCKey;

  if (!object) {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "object is null?" << std::endl;
  }
  h5Object = object;

  mdMesh = NULL;
  domainNumber = -1;

  registry->add(this);
}

VsMesh::~VsMesh() {
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << getFullName() << std::endl;
  registry->remove(this);
}

bool VsMesh::isHighOrder() const {
    VsAttribute* att = getAttribute(VsSchema::VsSubCellLocationsAtt);

    if(att) {
        return true;
    }

    return false;
}

bool VsMesh::hasTransform() const {
  std::string transformName = getTransformName();
  if ((transformName == VsSchema::zrphiTransformKey) && (numSpatialDims == 3)) {
    return true;
  } else if ((transformName == VsSchema::zrphiTransformKey_deprecated) && (numSpatialDims == 3)) {
    return true;
  }

  return false;
}

std::string VsMesh::getTransformName() const {
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
    VsLog::errorLog() <<"VsMesh::getTransformName() - Unrecognized value for key "
    << VsSchema::transformKey << " - " <<transformName <<std::endl;
    transformName = "";
  }
  
  return transformName;
}

std::string VsMesh::getTransformedMeshName() const {
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

bool VsMesh::isFortranOrder() const {
  return ((indexOrder == VsSchema::compMinorFKey) ||
          (indexOrder == VsSchema::compMajorFKey));
}

bool VsMesh::isCompMinor() const {
  return ((indexOrder == VsSchema::compMinorCKey) ||
          (indexOrder == VsSchema::compMinorFKey));
}

bool VsMesh::isCompMajor() const {
  return ((indexOrder == VsSchema::compMajorCKey) ||
          (indexOrder == VsSchema::compMajorFKey));
}

void VsMesh::write() const {
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << getFullName() << "  "
                    << "Kind: " << getKind() << "  "
                    << "Index Order: " << indexOrder << "  "
                    << "NumSpatialDims: " << numSpatialDims << "  "
                    << "NumTopologicalDims: " << numTopologicalDims << std::endl;
}

size_t VsMesh::getNumSpatialDims() const {
  return numSpatialDims;
}

size_t VsMesh::getNumTopologicalDims() const {
  return numTopologicalDims;
}

std::string VsMesh::getPath() const {
  return h5Object->getPath();
}

std::string VsMesh::getShortName() const {
  return h5Object->getShortName();
}

std::string VsMesh::getFullName() const {
  return h5Object->getFullName();
}

std::string VsMesh::getIndexOrder() const {
  return indexOrder;
}

void VsMesh::setMDMesh(VsMDMesh* md, int dNumber) {
  mdMesh = md;
  domainNumber = dNumber;
}

int VsMesh::getDomainNumber() const {
  return domainNumber;
}

VsMDMesh* VsMesh::getMDMesh() const {
  return mdMesh;
}

void VsMesh::getStringAttribute(std::string attName, std::string* value) const {
  VsAttribute* att = getAttribute(attName);
  if (att) {
    att->getStringValue(value);
  } else {
    value->clear();
  }
}

VsAttribute* VsMesh::getAttribute(std::string name) const {
  return h5Object->getAttribute(name);
}

VsMesh* VsMesh::buildObject(VsDataset* dataset) {
  if (!dataset) {
    VsLog::warningLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "dataset is null?  Returning null." << std::endl;
    return NULL;
  }
  
  //What is the declared kind of this dataset?
  VsAttribute* kindAtt = dataset->getAttribute(VsSchema::kindAtt);
  if (!kindAtt) {
    VsLog::warningLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Unable to find attribute " << VsSchema::kindAtt
                        << ".  Returning null for dataset: "
                        << dataset->getFullName() << std::endl;
    return NULL;
  }
  
  std::string kind;
  kindAtt->getStringValue(&kind);
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Mesh dataset has kind: " <<kind << std::endl;
  
  if (kind == VsSchema::structuredMeshKey) {
    return VsStructuredMesh::buildStructuredMesh(dataset);
  } else {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Dataset is of unrecognized kind?" << std::endl;
  }
    
  return NULL;
}

VsMesh* VsMesh::buildObject(VsGroup* group) {
  if (!group) {
    VsLog::warningLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Group is null?  Returning null." << std::endl;
    return NULL;
  }
  
  //What is the declared kind of this group?
  VsAttribute* kindAtt = group->getAttribute(VsSchema::kindAtt);
  if (!kindAtt) {
    VsLog::warningLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Unable to find attribute " << VsSchema::kindAtt
                        << ".  Returning null for group: "
                        << group->getFullName() << std::endl;
    return NULL;
  }
  
  std::string kind;
  kindAtt->getStringValue(&kind);
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Mesh group has kind: " <<kind << std::endl;

  if ((kind == VsSchema::Uniform::key_deprecated) ||
      (kind == VsSchema::Uniform::key)) {
    return VsUniformMesh::buildUniformMesh(group);
  } else if (kind == VsSchema::Unstructured::key) {
    return VsUnstructuredMesh::buildUnstructuredMesh(group);
  } else if (kind == VsSchema::Rectilinear::key) {
    return VsRectilinearMesh::buildRectilinearMesh(group);
  } else {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Group is of unrecognized kind?" << std::endl;
  }
    
  return NULL;
}

bool VsMesh::initializeRoot() {
  VsLog::debugLog() <<"VsMesh::initializeRoot() - Entering" << std::endl;
  
  if (!h5Object) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Failed to initialize mesh because h5Object is null."
                      << std::endl;
    return false;
  }

  //look for md mesh attribute
  //save for later
  //VsAttribute* mdAtt = h5Object->getAttribute(VsSchema::mdAtt);
  //if (mdAtt) {
  //  mdAtt->getStringValue(&(this->mdName));
  //}
  
  //Get & validate Index Order
  VsAttribute* indexOrderAtt =
    h5Object->getAttribute(VsSchema::indexOrderAtt);

  if (indexOrderAtt) {
    indexOrderAtt->getStringValue(&indexOrder);
  }
  
  if ((indexOrder != VsSchema::compMajorCKey) &&
      (indexOrder != VsSchema::compMajorFKey) &&
      (indexOrder != VsSchema::compMinorCKey) &&
      (indexOrder != VsSchema::compMinorFKey)) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "IndexOrder is invalid: " << indexOrder
                      << ", using default value: " << VsSchema::compMinorCKey
                      << std::endl;
    indexOrder = VsSchema::compMinorCKey;
  }
  
  // Done!
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Returning success" << std::endl;
  return true;
}

std::string VsMesh::getAxisLabel(size_t axis) const {
  std::string axisNames;
  this->getStringAttribute(VsSchema::axisLabelsAtt, &axisNames);
 
  //axisNames must be a comma-delimited string
  if (!axisNames.empty()) {
    std::vector<std::string> names;
    tokenize(axisNames, ',', names);
    if (names.size() > axis) {
      return names[axis];
    }
  }

  //the default values
  std::string answer = "?";
  switch (axis) {
    case 0: answer = "x"; break;
    case 1: answer = "y"; break;
    case 2: answer = "z"; break;
    default:
      VsLog::warningLog()
        << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
        << "Requested axis  (" <<axis <<") number must be 0, 1, or 2."
        << std::endl;
      break;
  }
  
  return answer;
}

std::string VsMesh::getSubCellLocationsDatasetName() const {
  std::string nodesName;
  getStringAttribute(VsSchema::VsSubCellLocationsAtt, &nodesName);
  if(!nodesName.empty()) {
      return makeCanonicalName(getFullName(), nodesName);
  }
  return ""; //TODO: check on fix for non-void return
}


VsDataset* VsMesh::getSubCellLocationsDataset() const  {
  std::string nodesName = getSubCellLocationsDatasetName();
  if (nodesName.empty()) {
    return NULL;
  }

  VsDataset* answer = registry->getDataset(nodesName);

  return answer; //could be NULL
}
