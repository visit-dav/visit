/*
 * VsH5Group.cpp
 *
 *  Created on: Apr 27, 2010
 *      Author: mdurant
 */

#include "VsH5Group.h"
#include "VsH5Dataset.h"
#include "VsLog.h"
#include "VsSchema.h"
#include "VsMesh.h"
#include "VsUtils.h"
#include <hdf5.h>
#include <map>
#include "VsH5Attribute.h"

VsH5Group::VsH5Group(VsRegistry* r, VsH5Object* parentObject, std::string groupName, hid_t id):
  VsH5Object(r, parentObject, groupName, id) {
  
  registry->add(this);
}

VsH5Group::~VsH5Group() {
  VsLog::debugLog() <<"Deleting group: " <<getFullName() <<std::endl;
  
  hid_t id = this->getId();
  if (id != -1) {
    H5Gclose(id);
    id = -1;
  }
  
  registry->remove(this);
}

void VsH5Group::write() {
  //calculate how much indentation to use
  std::string offset = "";
  for (int i = 0; i < getDepth(); ++i)
    offset += "  ";
  
  VsLog::debugLog() << offset << getFullName() <<"(" <<getId() <<")" <<std::endl;
}
