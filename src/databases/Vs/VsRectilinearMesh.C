/*
 * VsRectilinearMesh.cpp
 *
 *  Created on: Apr 29, 2010
 *      Author: mdurant
 */

#include "VsRectilinearMesh.h"
#include "VsSchema.h"
#include "VsH5Dataset.h"
#include "VsH5Group.h"
#include "VsH5Attribute.h"
#include "VsLog.h"

VsRectilinearMesh::VsRectilinearMesh(VsH5Group* group):VsMesh(group) {
  
}

VsRectilinearMesh::~VsRectilinearMesh() {
}

hid_t VsRectilinearMesh::getDataType() {

  VsH5Dataset* axis0 = getAxisDataset(0);
  if (axis0 == NULL) {
    //ERROR!
    return H5T_NATIVE_DOUBLE; //?
  }

  return axis0->getType();
}

VsRectilinearMesh* VsRectilinearMesh::buildRectilinearMesh(VsH5Group* group) {
  VsRectilinearMesh* newMesh = new VsRectilinearMesh(group);
  bool success = newMesh->initialize();
  
  if (success) {
    VsLog::debugLog() <<"VsRectilinearMesh::buildRectilinearMesh() - returning success." <<std::endl;
    return newMesh;
  }
  
  delete (newMesh);
  newMesh = NULL;
  VsLog::debugLog() <<"VsRectilinearMesh::buildRectilinearMesh() - returning failure." <<std::endl;
  return NULL;
}

bool VsRectilinearMesh::initialize() {
  
  //Rectilinear meshes are defined by a series of arrays
  //Each array represents one spatial dimension
  //i.e. numSpatialDims = number of arrays
  VsH5Dataset* axis0 = getAxisDataset(0);
  VsH5Dataset* axis1 = getAxisDataset(1);
  VsH5Dataset* axis2 = getAxisDataset(2);
  if (!axis0) {
    VsLog::errorLog() <<"VsRectilinearMesh::initialize() - Can't determine spatial dimensionality - no axis0 dataset?" <<std::endl;
    numSpatialDims = -1;
    return false;
  }
  
  numSpatialDims = 1;
  if (axis1) {
    if (axis2) {
      numSpatialDims = 3;
    } else {
      numSpatialDims = 2;
    }
  }
  VsLog::debugLog() <<"VsRectilinearMesh::initialize() - Mesh has num spatial dims = " <<numSpatialDims <<std::endl;
  
  return initializeRoot();
}

std::string VsRectilinearMesh::getAxisDatasetName(int axisNumber) {
  if ((axisNumber < 0) || (axisNumber > 2)) {
    return "";
  }
  
  std::string axisKey;
  switch (axisNumber) {
    case 0: axisKey = VsSchema::Rectilinear::axis0Key;
       break;
    case 1: axisKey = VsSchema::Rectilinear::axis1Key;
       break;
    case 2: axisKey = VsSchema::Rectilinear::axis2Key;
       break;
    default:
       return "";
       break;
  }
  
  std::string axisName;
    
  //First see if the user has specified a name for the dataset
  VsH5Attribute* axisNameAtt = getAttribute(axisKey);
  if (axisNameAtt) {
    axisNameAtt->getStringValue(&axisName);
    if (!axisName.empty()) {
      return axisName;
    }
  }
  
  //if we didn't find a user supplied name, try the default name
  switch (axisNumber) {
    case 0: axisName = VsSchema::Rectilinear::axis0DefaultName;
       break;
    case 1: axisName = VsSchema::Rectilinear::axis1DefaultName;
       break;
    case 2: axisName = VsSchema::Rectilinear::axis2DefaultName;
       break;
    default:
       axisName = "";
       break;
  }

  //axisName 
  return axisName;
}

VsH5Dataset* VsRectilinearMesh::getAxisDataset(int axisNumber) {
  std::string axisDatasetName = getAxisDatasetName(axisNumber);
  if (axisDatasetName.empty()) {
    return NULL;
  }

  VsH5Dataset* answer = registry->getDataset(axisDatasetName);

  return answer; //could be NULL
}

std::string VsRectilinearMesh::getKind() {
  return VsSchema::Rectilinear::key;
}

size_t VsRectilinearMesh::getMeshDims(std::vector<int>* dims, bool useStride, std::vector<int> stride) {
  VsLog::debugLog() << "VsRectilinearMesh::getMeshDims() - Entering." << std::endl;
  
  //size of rectilinear mesh depends on the size of the component arrays
  
  //We require at least one axis
  VsH5Dataset* axis0Data = getAxisDataset(0);
  if (!axis0Data) {
    VsLog::debugLog() <<"VsRectilinearMesh::getMeshDims() - unable to find information for axis 0." <<std::endl;
    VsLog::debugLog() <<"VsRectilinearMesh::getMeshDims() - returning 0." <<std::endl;
    dims->resize(0);
    return 0;
  }
  dims->resize(1);
  (*dims)[0] = axis0Data->getDims().front();
  
  VsH5Dataset* axis1Data = getAxisDataset(1);
  if (axis1Data == NULL) {
    VsLog::debugLog() <<"VsRectilinearMesh::getMeshDims() - unable to find information for axis 1." <<std::endl;
    dims->resize(1);
  } else {
    dims->resize(2);
    (*dims)[1] = axis1Data->getDims().front();
    
    //If a second axis exists, check for a third axis
    VsH5Dataset* axis2Data = getAxisDataset(2);
    if (axis2Data == NULL) {
      VsLog::debugLog() <<"VsRectilinearMesh::getMeshDims() - unable to find information for axis 2." <<std::endl;
      dims->resize(2);
    } else {
      dims->resize(3);
      (*dims)[2] = axis2Data->getDims().front();
    }
  }
  
  size_t len = 1;
  for (size_t i = 0; i < dims->size(); ++i)
    len *= (*dims)[i];
  
  VsLog::debugLog() << "VsRectilinearMesh::getMeshDims(): Returning " <<len <<"." << std::endl;
  return len;
}
