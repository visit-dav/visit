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

VsMesh::VsMesh(VsH5Object* object):VsRegistryObject(object->registry) {
  numSpatialDims = -1;
  indexOrder = VsSchema::compMinorCKey;

  if (!object) {
    VsLog::errorLog() <<"VsMesh::VsMesh() - object is null?" <<std::endl;
  }
  h5Object = object;

  mdMesh = NULL;
  domainNumber = -1;

  registry->add(this);
}

VsMesh::~VsMesh() {
  VsLog::debugLog() <<"Deleting VsMesh: " <<getFullName() <<std::endl;
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
  VsLog::debugLog() << getFullName() <<std::endl;
  VsLog::debugLog() << "    Kind: " <<getKind() <<std::endl;
  VsLog::debugLog() << "    Index Order: " << indexOrder << std::endl;
  VsLog::debugLog() << "    NumSpatialDims: " <<numSpatialDims <<std::endl;
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
    VsLog::warningLog() <<"VsMesh::buildObject() - dataset is null?  Returning null." <<std::endl;
    return NULL;
  }
  
  //What is the declared kind of this dataset?
  VsH5Attribute* kindAtt = dataset->getAttribute(VsSchema::kindAtt);
  if (!kindAtt) {
    VsLog::warningLog() <<"VsMesh::buildObject() - unable to find attribute " <<VsSchema::kindAtt
      <<".  Returning null for dataset: " <<dataset->getFullName() <<std::endl;
    return NULL;
  }
  
  std::string kind;
  kindAtt->getStringValue(&kind);
  VsLog::debugLog() <<"VsMesh::buildObject() - mesh dataset has kind: " <<kind <<std::endl;
  
  if (kind == VsSchema::structuredMeshKey) {
    return VsStructuredMesh::buildStructuredMesh(dataset);
  } else {
    VsLog::debugLog() <<"VsMesh::buildObject() - dataset is of unrecognized kind?" <<std::endl;
  }
    
  return NULL;
}

VsMesh* VsMesh::buildObject(VsH5Group* group) {
  if (!group) {
    VsLog::warningLog() <<"VsMesh::buildObject() - group is null?  Returning null." <<std::endl;
    return NULL;
  }
  
  //What is the declared kind of this group?
  VsH5Attribute* kindAtt = group->getAttribute(VsSchema::kindAtt);
  if (!kindAtt) {
    VsLog::warningLog() <<"VsMesh::buildObject() - unable to find attribute " <<VsSchema::kindAtt
      <<".  Returning null for group: " <<group->getFullName() <<std::endl;
    return NULL;
  }
  
  std::string kind;
  kindAtt->getStringValue(&kind);
  VsLog::debugLog() <<"VsMesh::buildObject() - mesh group has kind: " <<kind <<std::endl;

  if ((kind == VsSchema::Uniform::key_deprecated) || (kind == VsSchema::Uniform::key)) {
    return VsUniformMesh::buildUniformMesh(group);
  } else if (kind == VsSchema::Unstructured::key) {
    return VsUnstructuredMesh::buildUnstructuredMesh(group);
  } else if (kind == VsSchema::Rectilinear::key) {
    return VsRectilinearMesh::buildRectilinearMesh(group);
  } else {
    VsLog::debugLog() <<"VsMesh::buildObject() - group is of unrecognized kind?" <<std::endl;
  }
    
  return NULL;
}

bool VsMesh::initializeRoot() {
  //herr_t VsMesh::initialize(VsGMeta* gm, VsMeshMeta& mm) const {
  VsLog::debugLog() <<"VsMesh::initializeRoot() - Entering" <<std::endl;
  
  if (!h5Object) {
    VsLog::debugLog() <<"VsMesh::initializeRoot() - Failed to initialize mesh because h5Object is null." <<std::endl;
    return false;
  }

  //look for md mesh attribute
  //save for later
  //VsH5Attribute* mdAtt = h5Object->getAttribute(VsSchema::mdAtt);
  //if (mdAtt) {
  //  mdAtt->getStringValue(&(this->mdName));
  //}
  
  //Get & validate Index Order
  VsH5Attribute* indexOrderAtt = h5Object->getAttribute(VsSchema::indexOrderAtt);
  if (indexOrderAtt) {
    indexOrderAtt->getStringValue(&indexOrder);
  }
  
  if ((indexOrder != VsSchema::compMajorCKey) &&
      (indexOrder != VsSchema::compMajorFKey) &&
      (indexOrder != VsSchema::compMinorCKey) &&
      (indexOrder != VsSchema::compMinorFKey)) {
    VsLog::debugLog() <<"VsMesh::initializeRoot() - IndexOrder is invalid: " <<indexOrder;
    VsLog::debugLog() <<", using default value: " <<VsSchema::compMinorCKey <<std::endl;
    indexOrder = VsSchema::compMinorCKey;
  }
  
  // Done!
  VsLog::debugLog() <<"VsMesh::initializeRoot() - Returning success" <<std::endl;
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
      VsLog::debugLog() <<"VsMesh::getAxisLabel(" <<axis <<") - Requested axis number must be 0, 1, or 2." <<std::endl;
      break;
  }
  
  return answer;
}
