/*
 * VsUniformMesh.cpp
 *
 *  Created on: Apr 29, 2010
 *      Author: mdurant
 */

#include "VsUniformMesh.h"
#include "VsAttribute.h"
#include "VsSchema.h"
#include "VsGroup.h"
#include "VsLog.h"
#include "VsUtils.h"

#include <string>

#define __CLASS__ "VsUniformMesh::"


VsUniformMesh::VsUniformMesh(VsGroup* group):VsMesh(group) {
  numCellsAtt = NULL;
  startCellAtt = NULL;
  lowerBoundsAtt = NULL;
  upperBoundsAtt = NULL;
}


VsUniformMesh::~VsUniformMesh() {
}

VsUniformMesh* VsUniformMesh::buildUniformMesh(VsGroup* group)
{
  VsUniformMesh* newMesh = new VsUniformMesh(group);
  bool success = newMesh->initialize();
  
  if (success) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "returning success." << std::endl;
    return newMesh;
  }

  delete (newMesh);
  newMesh = NULL;
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "returning failure." << std::endl;
  return NULL;
}


bool VsUniformMesh::initialize()
{
  // ARS - Get the spatial dimensionality based on the lower and upper
  // bounds NOT the dimensionality of the topology.

  // Determine the topological dimensionality

  // For a uniform mesh, spatial dimensionality is the length of the
  // numCells array
  numCellsAtt = getAttribute(VsSchema::Uniform::numCells);

  if (!numCellsAtt) {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Uniform mesh is missing attribute: "
                      << VsSchema::Uniform::numCells << std::endl;
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Looking for deprecated attribute: "
                      << VsSchema::Uniform::numCells_deprecated << std::endl;
    numCellsAtt = getAttribute(VsSchema::Uniform::numCells_deprecated);
  }

  if (!numCellsAtt) {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Uniform mesh is also missing deprecated attribute: "
                      << VsSchema::Uniform::numCells_deprecated << std::endl;
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to initialize mesh, returning failure."
                      << std::endl;
    return false;
  }

  std::vector<int> dims;

  int err = numCellsAtt->getIntVectorValue(&dims);
  if (!err) {
    numSpatialDims = dims.size();
  } else {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to get dimensionality from attribute: "
                      << numCellsAtt->getShortName() << std::endl;
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to initialize mesh, returning failure."
                      << std::endl;
    return false;
  }

  //Num topological dims is equal to the count of dims
  //that are > 1
  numTopologicalDims = 0;
  for (size_t i = 0; i < dims.size(); i++) {
    if (dims[i] > 1) {
      numTopologicalDims++;
    }
  } 

  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Uniform mesh " <<getShortName() << " has topological dimensionality " 
                    << numTopologicalDims <<std::endl;
  
  // Lowerbounds is required
  lowerBoundsAtt = getAttribute(VsSchema::Uniform::lowerBounds);
  if (!lowerBoundsAtt) {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Uniform mesh is missing attribute: "
                      << VsSchema::Uniform::lowerBounds << std::endl;
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Looking for deprecated attribute: "
                      << VsSchema::Uniform::lowerBounds_deprecated << std::endl;
    lowerBoundsAtt = getAttribute(VsSchema::Uniform::lowerBounds_deprecated);
  }

  if (!lowerBoundsAtt) {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Uniform mesh is also missing deprecated attribute: "
                      << VsSchema::Uniform::lowerBounds_deprecated << std::endl;
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to initialize mesh, returning failure."
                      << std::endl;
    return false;
  }
  
  // Upperbounds is required
  upperBoundsAtt = getAttribute(VsSchema::Uniform::upperBounds);
  if (!upperBoundsAtt) {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Uniform mesh is missing attribute: "
                      << VsSchema::Uniform::upperBounds << std::endl;
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Looking for deprecated attribute: "
                      << VsSchema::Uniform::upperBounds_deprecated << std::endl;
    upperBoundsAtt = getAttribute(VsSchema::Uniform::upperBounds_deprecated);
  }

  if (!upperBoundsAtt) {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Uniform mesh is also missing deprecated attribute: "
                      << VsSchema::Uniform::upperBounds_deprecated << std::endl;
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to initialize mesh, returning failure."
                      << std::endl;
    return false;
  }

  // Get the spatial bounds. Get it from the lower bounds and make
  // sure the upper bounds matches.
  std::vector<double> dVals;

  // Get the lower bounds spatial dimension.
  err = lowerBoundsAtt->getDoubleVectorValue(&dVals);

  if( err )
  {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to get lower bounds from attribute: "
                      << lowerBoundsAtt->getShortName() << std::endl;
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to initialize mesh, returning failure."
                      << std::endl;
    numSpatialDims = -1;
    return false;
  }

  // Now get the upper bounds spatial dimension.
  err = upperBoundsAtt->getDoubleVectorValue(&dVals);

  if( err )
  {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to get upper bounds from attribute: "
                      << lowerBoundsAtt->getShortName() << std::endl;
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to initialize mesh, returning failure."
                      << std::endl;
    numSpatialDims = -1;
    return false;
  }

  if( numSpatialDims != dVals.size() )
  {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Lower and Upper spatial dimensions do not match: "
                      << numSpatialDims << "  " << dVals.size()
                      << std::endl;
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to initialize mesh, returning failure."
                      << std::endl;
    numSpatialDims = -1;
    return false;
  }

  // StartCell is optional
  startCellAtt = getAttribute(VsSchema::Uniform::startCell);
  if (!startCellAtt) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Uniform mesh does not have optional attribute: "
                      << VsSchema::Uniform::startCell << std::endl;
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Looking for deprecated attribute: "
                      << VsSchema::Uniform::startCell_deprecated << std::endl;
    startCellAtt = getAttribute(VsSchema::Uniform::startCell_deprecated);
  }

  if (!startCellAtt) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Uniform mesh does not have deprecated attribute: "
                      << VsSchema::Uniform::startCell_deprecated << std::endl;
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Using default start cell of 0." << std::endl;
  }

  return initializeRoot();
}


hid_t VsUniformMesh::getDataType() const {

  if (lowerBoundsAtt->getType() == H5T_NATIVE_DOUBLE)
    return H5T_NATIVE_DOUBLE;

  if (upperBoundsAtt->getType() == H5T_NATIVE_DOUBLE)
    return H5T_NATIVE_DOUBLE;

  return H5T_NATIVE_FLOAT;
}


std::string VsUniformMesh::getKind() const {
  return VsSchema::Uniform::key;
}


int VsUniformMesh::getLowerBounds(std::vector<float>* fVals) {
  hid_t type = lowerBoundsAtt->getType();

  int err = 0;
  
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
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Error " << err
                      << " in reading attribute '"
                      << lowerBoundsAtt->getShortName() << "'." <<  std::endl;
  }
  
  VsLog::debugLog() << "VsUniformMesh::getLowerBounds() - Returning " << err
                    << "." <<  std::endl;
  return err;
}


int VsUniformMesh::getUpperBounds(std::vector<float>* fVals) {
  hid_t type = upperBoundsAtt->getType();

  int err = 0;
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
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Error " << err
                      << " in reading attribute '"
                      << upperBoundsAtt->getShortName() << "'." <<  std::endl;
  }
  
  return err;
}


int VsUniformMesh::getStartCell(std::vector<int>* startCell) {
  if (startCellAtt == NULL) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Mesh does not have optional attribute: "
                      << VsSchema::Uniform::startCell <<  std::endl;
    return -1;
  }
  
  int err = startCellAtt->getIntVectorValue(startCell);

  if (err < 0) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Error " << err
                      << " in reading attribute '"
                      << startCellAtt->getShortName() << "'." <<  std::endl;
  }
  return err;
}


void VsUniformMesh::getCellDims(std::vector<int>& dims) const
{
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Entering." <<  std::endl;
  
  // Read dims of totalNumCells attribute
  numCellsAtt->getIntVectorValue(&dims);
}


void VsUniformMesh::getNodeDims(std::vector<int>& dims) const
{
  // Get the number of cells
  getCellDims(dims);

  // The number of nodes will be one more than the number of cells;
  for (unsigned int i = 0; i < dims.size(); i++)
    ++dims[i];
}
