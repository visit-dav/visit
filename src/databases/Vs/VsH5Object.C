/*
 * VsH5Object.cpp
 *
 *  Created on: Apr 27, 2010
 *      Author: mdurant
 */

#include "VsH5Object.h"
#include "VsUtils.h"
#include "VsH5Attribute.h"
#include "VsH5Group.h"
#include "VsH5Dataset.h"
#include <vector>

VsH5Object::VsH5Object(VsRegistry* r, VsH5Object* parentObject, std::string objectName, hid_t id):
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

VsH5Object::~VsH5Object() {
  std::map<std::string, VsH5Attribute*>::const_iterator it;
  for (it = attributes.begin(); it != attributes.end(); it++) {
    VsH5Attribute* att = (*it).second;
    delete(att);
  }
  
  attributes.clear();
}

std::map<std::string, VsH5Attribute*>* VsH5Object::getAllAttributes() {
  return &attributes;
}

std::string VsH5Object::getShortName() {
  return name;
}

std::string VsH5Object::getFullName() {
  return fullName;
}

std::string VsH5Object::getPath() {
  if (parent) {
    return parent->getFullName();
  }
  
  return "/";
}

hid_t VsH5Object::getId() {
  return resourceId;
}

int VsH5Object::getDepth() {
  return depth;
}

void VsH5Object::addAttribute(std::string name, hid_t id) {
  VsH5Attribute* att = new VsH5Attribute(this, name, id);

  attributes[name] = att;
  //TODO check for duplicates
  //TODO check for validity
}

VsH5Attribute* VsH5Object::getAttribute (std::string name) {
  std::map<std::string, VsH5Attribute*>::iterator it = attributes.find(name);
  if (it != attributes.end()) {
    return (*it).second;
  }
  
  return NULL;
}
