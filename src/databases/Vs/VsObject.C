/*
 * VsObject.cpp
 *
 *  Created on: Apr 27, 2010
 *      Author: mdurant
 */

#include "VsObject.h"
#include "VsUtils.h"
#include "VsAttribute.h"
#include "VsGroup.h"
#include "VsDataset.h"
#include "VsLog.h"
#include <vector>

VsObject::VsObject(VsRegistry* r, VsObject* parentObject, 
                   const std::string& objectName, hid_t id):
  VsRegistryObject(r) {

  parent = parentObject;
  name = makeCanonicalName(objectName);
  resourceId = id;
  
  if (parent) {
    depth = parent->getDepth() + 1;
    fullName = makeCanonicalName(parentObject->getFullName(), objectName);
  } else {
    depth = 0;
    fullName = makeCanonicalName(objectName);
  }
  
}

VsObject::~VsObject() {
  std::map<std::string, VsAttribute*>::const_iterator it;
  for (it = attributes.begin(); it != attributes.end(); it++) {
    VsAttribute* att = (*it).second;
    delete(att);
  }
  
  attributes.clear();
}

const std::map<std::string, VsAttribute*>& VsObject::getAllAttributes() const {
  return attributes;
}

std::string VsObject::getShortName() const {
  return name;
}

std::string VsObject::getFullName() const {
  return fullName;
}

std::string VsObject::getPath() const {
  if (parent) {
    return parent->getFullName();
  }
  
  return "/";
}

hid_t VsObject::getId() const {
  return resourceId;
}

int VsObject::getDepth() const {
  return depth;
}

void VsObject::addAttribute(const std::string name, hid_t id) {
  VsLog::debugLog() << "VsObject::addAttribute '" << name << "', id: "<< id<< std::endl;
  VsAttribute* att = new VsAttribute(this, name, id);
  attributes[name] = att;
}

VsAttribute* VsObject::getAttribute (const std::string name) const {

//  return attributes.find(name)->second;

  std::map<std::string, VsAttribute*>::const_iterator it = attributes.find(name);
  if (it != attributes.end()) {
    VsLog::debugLog() << "VsObject::getAttribute " << name <<" "<<it->second<< std::endl;
    return (*it).second;
  }
  VsLog::debugLog() << "VsObject::getAttribute: '" << name << "' not found" << std::endl;
  
  return NULL;
}
