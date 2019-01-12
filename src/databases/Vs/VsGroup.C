/*
 * VsGroup.cpp
 *
 *  Created on: Apr 27, 2010
 *      Author: mdurant
 */

#include "VsGroup.h"
#include "VsDataset.h"
#include "VsLog.h"
#include "VsSchema.h"
#include "VsMesh.h"
#include "VsUtils.h"
#include <hdf5.h>
#include <map>
#include "VsAttribute.h"

VsGroup::VsGroup(VsRegistry* r, VsObject* parentObject, std::string groupName, hid_t id):
  VsObject(r, parentObject, groupName, id) {
  
  registry->add(this);
}

VsGroup::~VsGroup() {
  VsLog::debugLog() <<"Deleting group: " <<getFullName() <<std::endl;
  
  hid_t id = this->getId();
  if (id != -1) {
    H5Gclose(id);
    id = -1;
  }
  
  registry->remove(this);
}

void VsGroup::write() const {
  //calculate how much indentation to use
  std::string offset = "";
  for (int i = 0; i < getDepth(); ++i)
    offset += "  ";
  
  VsLog::debugLog() << offset << getFullName() <<"(" <<getId() <<")" <<std::endl;
}
