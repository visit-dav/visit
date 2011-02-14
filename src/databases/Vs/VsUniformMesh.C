/*
 * VsUniformMesh.cpp
 *
 *  Created on: Apr 29, 2010
 *      Author: mdurant
 */

#include "VsUniformMesh.h"
#include "VsH5Attribute.h"
#include "VsSchema.h"
#include "VsH5Group.h"
#include "VsLog.h"
#include "VsUtils.h"

VsUniformMesh::VsUniformMesh(VsH5Group* group):VsMesh(group) {
  numCellsAtt = NULL;
  startCellAtt = NULL;
  lowerBoundsAtt = NULL;
  upperBoundsAtt = NULL;
}

VsUniformMesh::~VsUniformMesh() {
}

VsUniformMesh* VsUniformMesh::buildUniformMesh(VsH5Group* group) {
  VsUniformMesh* newMesh = new VsUniformMesh(group);
  bool success = newMesh->initialize();
  
  if (success) {
    VsLog::debugLog() <<"VsUniformMesh::buildUniformMesh() - returning success." <<std::endl;
    return newMesh;
  }

  delete (newMesh);
  newMesh = NULL;
  VsLog::debugLog() <<"VsUniformMesh::buildUniformMesh() - returning failure." <<std::endl;
  return NULL;
}


bool VsUniformMesh::initialize() {
  //determine the spatial dimensionality
  //For a uniform mesh, spatial dimensionality is the length of the numCells array
  numCellsAtt = getAttribute(VsSchema::Uniform::numCells);
  if (!numCellsAtt) {
    VsLog::errorLog() <<"VsUniformMesh::initialize() - Uniform mesh is missing attribute: " <<VsSchema::Uniform::numCells <<std::endl;
    VsLog::debugLog() <<"VsUniformMesh::initialize() - Looking for deprecated attribute: " <<VsSchema::Uniform::numCells_deprecated <<std::endl;
    numCellsAtt = getAttribute(VsSchema::Uniform::numCells_deprecated);
  }
  if (!numCellsAtt) {
    VsLog::errorLog() <<"VsUniformMesh::initialize() - Uniform mesh is also missing deprecated attribute: " <<VsSchema::Uniform::numCells_deprecated <<std::endl;
    VsLog::debugLog() <<"VsUniformMesh::initialize() - Unable to initialize mesh, returning failure." <<std::endl;
    return false;
  }
  
  std::vector<int> dims;
  herr_t err = numCellsAtt->getIntVectorValue(&dims);
  if (!err) {
    numSpatialDims = dims.size();
  } else {
    VsLog::errorLog() <<"VsUniformMesh::initialize() - Unable to get dimensionality from attribute: " <<numCellsAtt->getShortName() <<std::endl;
    VsLog::errorLog() <<"VsUniformMesh::initialize() - Unable to initialize mesh, returning failure." <<std::endl;
    numSpatialDims = -1;
    return false;
  }
  
  // Lowerbounds is required
  lowerBoundsAtt = getAttribute(VsSchema::Uniform::lowerBounds);
  if (!lowerBoundsAtt) {
    VsLog::errorLog() <<"VsUniformMesh::initialize() - Uniform mesh is missing attribute: " <<VsSchema::Uniform::lowerBounds <<std::endl;
    VsLog::errorLog() <<"VsUniformMesh::initialize() - Looking for deprecated attribute: " <<VsSchema::Uniform::lowerBounds_deprecated <<std::endl;
    lowerBoundsAtt = getAttribute(VsSchema::Uniform::lowerBounds_deprecated);
  }
  if (!lowerBoundsAtt) {
    VsLog::errorLog() <<"VsUniformMesh::initialize() - Uniform mesh is also missing deprecated attribute: " <<VsSchema::Uniform::lowerBounds_deprecated <<std::endl;
    VsLog::errorLog() <<"VsUniformMesh::initialize() - Unable to initialize mesh, returning failure." <<std::endl;
    return false;
  }
  
  // Upperbounds is required
  upperBoundsAtt = getAttribute(VsSchema::Uniform::upperBounds);
  if (!upperBoundsAtt) {
    VsLog::errorLog() <<"VsUniformMesh::initialize() - Uniform mesh is missing attribute: " <<VsSchema::Uniform::upperBounds <<std::endl;
    VsLog::errorLog() <<"VsUniformMesh::initialize() - Looking for deprecated attribute: " <<VsSchema::Uniform::upperBounds_deprecated <<std::endl;
    upperBoundsAtt = getAttribute(VsSchema::Uniform::upperBounds_deprecated);
  }
  if (!upperBoundsAtt) {
    VsLog::errorLog() <<"VsUniformMesh::initialize() - Uniform mesh is also missing deprecated attribute: " <<VsSchema::Uniform::upperBounds_deprecated <<std::endl;
    VsLog::errorLog() <<"VsUniformMesh::initialize() - Unable to initialize mesh, returning failure." <<std::endl;
    return false;
  }

  // StartCell is optional
  startCellAtt = getAttribute(VsSchema::Uniform::startCell);
  if (!startCellAtt) {
    VsLog::debugLog() <<"VsUniformMesh::initialize() - Uniform mesh does not have optional attribute: " <<VsSchema::Uniform::startCell <<std::endl;
    VsLog::debugLog() <<"VsUniformMesh::initialize() - Looking for deprecated attribute: " <<VsSchema::Uniform::startCell_deprecated <<std::endl;
    startCellAtt = getAttribute(VsSchema::Uniform::startCell_deprecated);
  }
  if (!startCellAtt) {
    VsLog::debugLog() <<"VsUniformMesh::initialize() - Uniform mesh also does not have deprecated attribute: " <<VsSchema::Uniform::startCell_deprecated <<std::endl;
    VsLog::debugLog() <<"VsUniformMesh::initialize() - Using default start cell of 0." <<std::endl;
  }
  
  return initializeRoot();
}

hid_t VsUniformMesh::getDataType()  {
  return lowerBoundsAtt->getType();
}

std::string VsUniformMesh::getKind() {
  return VsSchema::Uniform::key;
}

herr_t VsUniformMesh::getLowerBounds(std::vector<float>* fVals) {
  hid_t type = lowerBoundsAtt->getType();

  herr_t err = 0;
  
  if (isDoubleType(type)) {
    std::vector<double> dVals;
    err = lowerBoundsAtt->getDoubleVectorValue(&dVals);
    fVals->resize(dVals.size());
    for (size_t i = 0; i < dVals.size(); i++) {
      (*fVals)[i] = (float)(dVals[i]);
    }
  } else if (isFloatType(type)) {
    err = lowerBoundsAtt->getFloatVectorValue(fVals);
  }
  
  if (err < 0) {
    VsLog::debugLog() << "VsUniformMesh::getLowerBounds(): error " << err <<
    " in reading attribute '" <<lowerBoundsAtt->getShortName() << "'." << std::endl;
  }
  
  VsLog::debugLog() << "VsUniformMesh::getLowerBounds() - Returning " <<err <<"." << std::endl;
  return err;
}

herr_t VsUniformMesh::getUpperBounds(std::vector<float>* fVals) {
  hid_t type = upperBoundsAtt->getType();

  herr_t err = 0;
  if (isDoubleType(type)) {
    std::vector<double> dVals;
    err = upperBoundsAtt->getDoubleVectorValue(&dVals);
    fVals->resize(dVals.size());
    for (size_t i = 0; i < dVals.size(); i++) {
      (*fVals)[i] = (float)(dVals[i]);
    }
  } else if (isFloatType(type)) {
    err = upperBoundsAtt->getFloatVectorValue(fVals);
  }
  
  if (err < 0) {
    VsLog::debugLog() << "VsUniformMesh::getUpperBounds(): error " << err <<
    " in reading attribute '" << upperBoundsAtt->getShortName() << "'." << std::endl;
  }
  
  VsLog::debugLog() << "VsUniformMesh::getUpperBounds() - Returning " <<err <<"." << std::endl;
  return err;
}

herr_t VsUniformMesh::getStartCell(std::vector<int>* startCell) {
  if (startCellAtt == NULL) {
    VsLog::debugLog() << "VsUniformMesh::getStartCell() - Mesh does not have optional attribute: " <<VsSchema::Uniform::startCell << std::endl;
    return -1;
  }
  
  herr_t err = startCellAtt->getIntVectorValue(startCell);
  if (err < 0) {
    VsLog::debugLog() << "VsUniformMesh::getStartCell(): error " << err <<
    " in reading attribute '" << startCellAtt->getShortName() << "'." << std::endl;
  }
  
  VsLog::debugLog() << "VsUniformMesh::getStartCell() - Returning " <<err <<"." << std::endl;
  return err;
}

size_t VsUniformMesh::getMeshDims(std::vector<int>* dims, bool useStride, std::vector<int> stride) {
  VsLog::debugLog() << "VsUniformMesh::getMeshDims(): Entering." << std::endl;
  
  // Read dims of totalNumCells attribute
  numCellsAtt->getIntVectorValue(dims);
  
  if (useStride) {
    VsLog::debugLog() <<"VsUniformMesh::getMeshDims() - Adjusting size of mesh using stride." <<std::endl;
    for (unsigned int i = 0; i < dims->size(); i++) {
      int value = (*dims)[i];
      VsLog::debugLog() <<"VsUniformMesh::getMeshDims() - dims[" <<i <<"] = " <<value <<" stride[" <<i <<"] = " <<stride[i] <<std::endl;
      value = value / stride[i];
      
      //we round UP if there is a remainder
      // NOT FOR UNIFORM
      //if ((*dims)[i] % stride[i] != 0) {
      //  VsLog::debugLog() <<"VsH5Reader::getMeshDims() - Added 1 because there was a remainder." <<std::endl;
      //      value ++;
      //}
      
      if (value < 1)
        value = 1;
      
      (*dims)[i] = value;
      VsLog::debugLog() <<"VsUniformMesh::getMeshDims() - dims[" <<i <<"] was adjusted to " <<(*dims)[i] <<std::endl;
    }
  }
  
  size_t len = 1;
  for (size_t i = 0; i < dims->size(); ++i)
    len *= (*dims)[i];
  
  VsLog::debugLog() << "VsUniformMesh::getMeshDims(): Returning " <<len <<"." << std::endl;
  return len;
}
