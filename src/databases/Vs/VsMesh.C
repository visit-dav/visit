/*
 * VsMesh.cpp
 *
 *  Created on: Apr 28, 2010
 *      Author: mdurant
 */

#include "VsMesh.h"
#include "VsH5Dataset.h"
#include "VsH5Attribute.h"
#include "VsUnstructuredMesh.h"
#include "VsStructuredMesh.h"
#include "VsRectilinearMesh.h"
#include "VsUniformMesh.h"
#include "VsH5Group.h"
#include "VsSchema.h"
#include "VsLog.h"
#include "VsUtils.h"
#include "VsMDMesh.h"

#define __CLASS__ "VsMesh::"

using namespace std;

VsMesh::VsMesh(VsH5Object* object):VsRegistryObject(object->registry) {
  numSpatialDims = -1;
  indexOrder = VsSchema::compMinorCKey;

  if (!object) {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "object is null?" << endl;
  }
  h5Object = object;

  mdMesh = NULL;
  domainNumber = -1;

  registry->add(this);
}

VsMesh::~VsMesh() {
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << getFullName() << endl;
  registry->remove(this);
}

bool VsMesh::isFortranOrder() {
  return ((indexOrder == VsSchema::compMinorFKey) ||
          (indexOrder == VsSchema::compMajorFKey));
}

bool VsMesh::isCompMinor() {
  return ((indexOrder == VsSchema::compMinorCKey) ||
          (indexOrder == VsSchema::compMinorFKey));
}

bool VsMesh::isCompMajor() {
  return ((indexOrder == VsSchema::compMajorCKey) ||
          (indexOrder == VsSchema::compMajorFKey));
}

void VsMesh::write() {
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << getFullName() << "  "
                    << "Kind: " << getKind() << "  "
                    << "Index Order: " << indexOrder << "  "
                    << "NumSpatialDims: " << numSpatialDims << endl;
}

size_t VsMesh::getNumSpatialDims() {
  return numSpatialDims;
}

std::string VsMesh::getPath() {
  return h5Object->getPath();
}

std::string VsMesh::getShortName() {
  return h5Object->getShortName();
}

std::string VsMesh::getFullName() {
  return h5Object->getFullName();
}

std::string VsMesh::getIndexOrder() {
  return indexOrder;
}

void VsMesh::setMDMesh(VsMDMesh* md, int dNumber) {
  mdMesh = md;
  domainNumber = dNumber;
}

int VsMesh::getDomainNumber() {
  return domainNumber;
}

VsMDMesh* VsMesh::getMDMesh() {
  return mdMesh;
}

void VsMesh::getStringAttribute(std::string attName, std::string* value) {
  VsH5Attribute* att = getAttribute(attName);
  if (att) {
    att->getStringValue(value);
  } else {
    value->clear();
  }
}

VsH5Attribute* VsMesh::getAttribute(std::string name) {
  return h5Object->getAttribute(name);
}

VsMesh* VsMesh::buildObject(VsH5Dataset* dataset) {
  if (!dataset) {
    VsLog::warningLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "dataset is null?  Returning null." << endl;
    return NULL;
  }
  
  //What is the declared kind of this dataset?
  VsH5Attribute* kindAtt = dataset->getAttribute(VsSchema::kindAtt);
  if (!kindAtt) {
    VsLog::warningLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Unable to find attribute " << VsSchema::kindAtt
                        << ".  Returning null for dataset: "
                        << dataset->getFullName() << endl;
    return NULL;
  }
  
  std::string kind;
  kindAtt->getStringValue(&kind);
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Mesh dataset has kind: " <<kind << endl;
  
  if (kind == VsSchema::structuredMeshKey) {
    return VsStructuredMesh::buildStructuredMesh(dataset);
  } else {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Dataset is of unrecognized kind?" << endl;
  }
    
  return NULL;
}

VsMesh* VsMesh::buildObject(VsH5Group* group) {
  if (!group) {
    VsLog::warningLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Group is null?  Returning null." << endl;
    return NULL;
  }
  
  //What is the declared kind of this group?
  VsH5Attribute* kindAtt = group->getAttribute(VsSchema::kindAtt);
  if (!kindAtt) {
    VsLog::warningLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Unable to find attribute " << VsSchema::kindAtt
                        << ".  Returning null for group: "
                        << group->getFullName() << endl;
    return NULL;
  }
  
  std::string kind;
  kindAtt->getStringValue(&kind);
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Mesh group has kind: " <<kind << endl;

  if ((kind == VsSchema::Uniform::key_deprecated) ||
      (kind == VsSchema::Uniform::key)) {
    return VsUniformMesh::buildUniformMesh(group);
  } else if (kind == VsSchema::Unstructured::key) {
    return VsUnstructuredMesh::buildUnstructuredMesh(group);
  } else if (kind == VsSchema::Rectilinear::key) {
    return VsRectilinearMesh::buildRectilinearMesh(group);
  } else {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Group is of unrecognized kind?" << endl;
  }
    
  return NULL;
}

bool VsMesh::initializeRoot() {
  //herr_t VsMesh::initialize(VsGMeta* gm, VsMeshMeta& mm) const {
  VsLog::debugLog() <<"VsMesh::initializeRoot() - Entering" << endl;
  
  if (!h5Object) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Failed to initialize mesh because h5Object is null."
                      << endl;
    return false;
  }

  //look for md mesh attribute
  //save for later
  //VsH5Attribute* mdAtt = h5Object->getAttribute(VsSchema::mdAtt);
  //if (mdAtt) {
  //  mdAtt->getStringValue(&(this->mdName));
  //}
  
  //Get & validate Index Order
  VsH5Attribute* indexOrderAtt =
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
                      << endl;
    indexOrder = VsSchema::compMinorCKey;
  }
  
  // Done!
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Returning success" << endl;
  return true;
}

std::string VsMesh::getAxisLabel(unsigned int axis) {
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
      VsLog::warningLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Requested axis  (" <<axis
                          <<") number must be 0, 1, or 2." << endl;
      break;
  }
  
  return answer;
}
