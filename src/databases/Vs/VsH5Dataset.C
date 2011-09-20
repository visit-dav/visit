/*
 * VsH5Dataset.cpp
 *
 *  Created on: Apr 27, 2010
 *      Author: mdurant
 */

#include "VsH5Dataset.h"
#include "VsH5Attribute.h"
#include "VsLog.h"
#include "VsSchema.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "VsMesh.h"
#include "VsUtils.h"
#include "VsVariable.h"
#include "VsVariableWithMesh.h"

VsH5Dataset::VsH5Dataset(VsRegistry* r, VsH5Object* parentObject, std::string datasetName, hid_t id):
  VsH5Object(r, parentObject, datasetName, id){
  
  dataType = H5Tget_native_type(H5Dget_type(id), H5T_DIR_DEFAULT);
  loadDims();
  
  registry->add(this);
}

VsH5Dataset::~VsH5Dataset() {  
  hid_t id = this->getId();
  if (id != -1) {
    H5Dclose(id);
    id = -1;
  }
  
  registry->remove(this);
}


// Total length
size_t VsH5Dataset::getLength() {
  size_t len = 1;
  for (size_t i = 0; i < dims.size(); ++i) {
    len *= dims[i];
  }
  return len;
}

void VsH5Dataset::loadDims() {
  hid_t space = H5Dget_space(getId());
  size_t rank = H5Sget_simple_extent_ndims(space);
  
  if (rank <= 0) {
    VsLog::errorLog() <<"VsH5Dataset::loadDims() - Rank was <= 0 for dataset: " <<getFullName() <<std::endl;
    return;
  }
  std::vector<hsize_t> sdim(rank);
  H5Sget_simple_extent_dims(space, &sdim[0], NULL);
  dims.resize(rank);
  for (size_t i = 0; i < rank; ++i) {
    dims[i] = sdim[i];
  }
}

std::vector<int> VsH5Dataset::getDims() {
  return dims;
}

void VsH5Dataset::write() {
  std::string offset = "";
  for (int i = 0; i < getDepth(); ++i) {
    offset += "  ";
  }
  
  VsLog::debugLog() << offset<<getFullName() <<"(" <<getId() <<")" <<std::endl;
}
