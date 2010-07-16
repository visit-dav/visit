/*
 * VsStructuredMesh.cpp
 *
 *  Created on: Apr 29, 2010
 *      Author: mdurant
 */

#include "VsStructuredMesh.h"
#include "VsH5Attribute.h"
#include "VsSchema.h"
#include "VsH5Dataset.h"
#include "VsLog.h"

VsStructuredMesh::VsStructuredMesh(VsH5Dataset* data):VsMesh(data) {
}

VsStructuredMesh::~VsStructuredMesh() {
}

VsStructuredMesh* VsStructuredMesh::buildStructuredMesh(VsH5Dataset* dataset) {
  VsStructuredMesh* newMesh = new VsStructuredMesh(dataset);
  bool success = newMesh->initialize();
  
  if (success) {
    VsLog::debugLog() <<"VsStructuredMesh::buildStructuredMesh() - returning success." <<std::endl;
    return newMesh;
  }
  
  delete (newMesh);
  newMesh = NULL;
  VsLog::debugLog() <<"VsStructuredMesh::buildStructuredMesh() - returning failure." <<std::endl;
  return NULL;
}

bool VsStructuredMesh::initialize() {
  VsLog::debugLog() <<"VsStructuredMesh::initialize() - Entering" <<std::endl;
  
  //determine num spatial dims
  //For a structured mesh, it is the size of the last component of the dataset
  int index = ((VsH5Dataset*)h5Object)->getDims().size() - 1;
  if (index < 0) {
    VsLog::debugLog() <<"VsStructuredMesh::initialize() - Failed to create mesh because dataset->dims.size() is zero." <<std::endl;
    return false;
  } else if (index == 0) {
    VsLog::warningLog() <<"VsStructuredMesh::initialize() - in special 1-d case." <<std::endl;
    VsLog::warningLog() <<"VsStructuredMesh::initialize() - This file attempts to declare a 1-d structured mesh." <<std::endl;
    VsLog::warningLog() <<"VsStructuredMesh::initialize() - As an array of size [numPoints]" <<std::endl;
    VsLog::warningLog() <<"VsStructuredMesh::initialize() - Whereas the normal dimensions would be [numPoints][1]" <<std::endl;
    VsLog::warningLog() <<"VsStructuredMesh::initialize() - Although this is acceptable by vizschema documentation," <<std::endl;
    VsLog::warningLog() <<"VsStructuredMesh::initialize() - it is not implemented at the moment." <<std::endl;
    return false;
  } else {
    numSpatialDims = ((VsH5Dataset*)h5Object)->getDims()[index];
  }
  
  VsLog::debugLog() <<"VsStructuredMesh::initialize() - Mesh has num spatial dims = " <<numSpatialDims <<std::endl;
    
  VsLog::debugLog() <<"VsStructuredMesh::initialize() - Success." <<std::endl;
  return initializeRoot();
}

std::string VsStructuredMesh::getKind() {
  return VsSchema::structuredMeshKey;
}

size_t VsStructuredMesh::getMeshDims(std::vector<int>* dims, bool useStride, std::vector<int> stride) {
  VsLog::debugLog() << "VsStructuredMesh::getMeshDims(): Entering." << std::endl;
  
  // Read dataset's dims
  VsH5Dataset* dm = registry->getDataset(getFullName());
  if (!dm) {
    VsLog::errorLog() << "VsStructuredMesh::getMeshDims(): Error: dataset " << getFullName()
    << " not found" << std::endl;
    VsLog::errorLog() << "VsStructuredMesh::getMeshDims(): Returning 0." << std::endl;
    return 0;
  }
 
  *dims = dm->getDims();
  
  //Structured is funny because dims is one of these:
  // 3d: [i][j][k][3]
  // 2d: [i][j][2]
  // 1d: [i][1] or [i]
  if (useStride) {
    VsLog::debugLog() <<"VsStructuredMesh::getMeshDims() - Adjusting size of mesh using stride." <<std::endl;
    
    //we do dims - 1 because structured meshes store their coordinates in the last dimension
    // and we don't want to resize it
    for (unsigned int i = 0; i < dims->size() - 1; i++) {
      int value = (*dims)[i];
      VsLog::debugLog() <<"VsStructuredMesh::getMeshDims() - dims[" <<i <<"] = " <<value <<" stride[" <<i <<"] = " <<stride[i] <<std::endl;
      
      value = value / stride[i];
      
      //we round UP if there is a remainder
      if ((*dims)[i] % stride[i] != 0) {
        VsLog::debugLog() <<"VsStructuredMesh::getMeshDims() - Added 1 because there was a remainder." <<std::endl;
        value ++;
      }
      
      if (value < 1)
        value = 1;
      
      (*dims)[i] = value;
      VsLog::debugLog() <<"VsStructuredMesh::getMeshDims() - dims[" <<i <<"] was adjusted to " <<(*dims)[i] <<std::endl;
    }
  }
  
  size_t len = 1;
  for (size_t i = 0; i < dims->size(); ++i)
    len *= (*dims)[i];
  
  VsLog::debugLog() << "VsStructuredMesh::getMeshDims(): Returning " <<len <<"." << std::endl;
  return len;
}
