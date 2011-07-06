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

#include <string>

#define __CLASS__ "VsUniformMesh::"

using namespace std;


VsUniformMesh::VsUniformMesh(VsH5Group* group):VsMesh(group) {
  numCellsAtt = NULL;
  startCellAtt = NULL;
  lowerBoundsAtt = NULL;
  upperBoundsAtt = NULL;
}


VsUniformMesh::~VsUniformMesh() {
}

VsUniformMesh* VsUniformMesh::buildUniformMesh(VsH5Group* group)
{
  VsUniformMesh* newMesh = new VsUniformMesh(group);
  bool success = newMesh->initialize();
  
  if (success) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  " << "returning success." << endl;
    return newMesh;
  }

  delete (newMesh);
  newMesh = NULL;
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  " << "returning failure." << endl;
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
                      << VsSchema::Uniform::numCells << endl;
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Looking for deprecated attribute: "
                      << VsSchema::Uniform::numCells_deprecated << endl;
    numCellsAtt = getAttribute(VsSchema::Uniform::numCells_deprecated);
  }

  if (!numCellsAtt) {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Uniform mesh is also missing deprecated attribute: "
                      << VsSchema::Uniform::numCells_deprecated << endl;
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to initialize mesh, returning failure."
                      << endl;
    return false;
  }

  std::vector<int> dims;

  herr_t err = numCellsAtt->getIntVectorValue(&dims);
  if (!err) {
    numSpatialDims = dims.size();
  } else {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to get dimensionality from attribute: "
                      << numCellsAtt->getShortName() << endl;
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to initialize mesh, returning failure."
                      << endl;
    return false;
  }

  int numTopologyDims = dims.size();
  
  // Lowerbounds is required
  lowerBoundsAtt = getAttribute(VsSchema::Uniform::lowerBounds);
  if (!lowerBoundsAtt) {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Uniform mesh is missing attribute: "
                      << VsSchema::Uniform::lowerBounds << endl;
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Looking for deprecated attribute: "
                      << VsSchema::Uniform::lowerBounds_deprecated << endl;
    lowerBoundsAtt = getAttribute(VsSchema::Uniform::lowerBounds_deprecated);
  }

  if (!lowerBoundsAtt) {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Uniform mesh is also missing deprecated attribute: "
                      << VsSchema::Uniform::lowerBounds_deprecated << endl;
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to initialize mesh, returning failure."
                      << endl;
    return false;
  }
  
  // Upperbounds is required
  upperBoundsAtt = getAttribute(VsSchema::Uniform::upperBounds);
  if (!upperBoundsAtt) {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Uniform mesh is missing attribute: "
                      << VsSchema::Uniform::upperBounds << endl;
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Looking for deprecated attribute: "
                      << VsSchema::Uniform::upperBounds_deprecated << endl;
    upperBoundsAtt = getAttribute(VsSchema::Uniform::upperBounds_deprecated);
  }

  if (!upperBoundsAtt) {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Uniform mesh is also missing deprecated attribute: "
                      << VsSchema::Uniform::upperBounds_deprecated << endl;
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to initialize mesh, returning failure."
                      << endl;
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
                      << lowerBoundsAtt->getShortName() << endl;
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to initialize mesh, returning failure."
                      << endl;
    numSpatialDims = -1;
    return false;
  }

  numSpatialDims = dVals.size();

  // Now get the upper bounds spatial dimension.
  err = upperBoundsAtt->getDoubleVectorValue(&dVals);

  if( err )
  {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to get upper bounds from attribute: "
                      << lowerBoundsAtt->getShortName() << endl;
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to initialize mesh, returning failure."
                      << endl;
    numSpatialDims = -1;
    return false;
  }

  if( numSpatialDims != dVals.size() )
  {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Lower and Upper spatial dimensions do not match: "
                      << numSpatialDims << "  " << dVals.size()
                      << endl;
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to initialize mesh, returning failure."
                      << endl;
    numSpatialDims = -1;
    return false;
  }

  // StartCell is optional
  startCellAtt = getAttribute(VsSchema::Uniform::startCell);
  if (!startCellAtt) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Uniform mesh does not have optional attribute: "
                      << VsSchema::Uniform::startCell << endl;
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Looking for deprecated attribute: "
                      << VsSchema::Uniform::startCell_deprecated << endl;
    startCellAtt = getAttribute(VsSchema::Uniform::startCell_deprecated);
  }

  if (!startCellAtt) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Uniform mesh also does not have deprecated attribute: "
                      << VsSchema::Uniform::startCell_deprecated << endl;
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Using default start cell of 0." << endl;
  }

  return initializeRoot();
}


hid_t VsUniformMesh::getDataType()  {

  if (lowerBoundsAtt->getType() == H5T_NATIVE_DOUBLE)
    return H5T_NATIVE_DOUBLE;

  if (upperBoundsAtt->getType() == H5T_NATIVE_DOUBLE)
    return H5T_NATIVE_DOUBLE;

  return H5T_NATIVE_FLOAT;
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
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Error " << err
                      << " in reading attribute '"
                      << lowerBoundsAtt->getShortName() << "'." <<  endl;
  }
  
  VsLog::debugLog() << "VsUniformMesh::getLowerBounds() - Returning " << err
                    << "." <<  endl;
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
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Error " << err
                      << " in reading attribute '"
                      << upperBoundsAtt->getShortName() << "'." <<  endl;
  }
  
  return err;
}


herr_t VsUniformMesh::getStartCell(std::vector<int>* startCell) {
  if (startCellAtt == NULL) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Mesh does not have optional attribute: "
                      << VsSchema::Uniform::startCell <<  endl;
    return -1;
  }
  
  herr_t err = startCellAtt->getIntVectorValue(startCell);

  if (err < 0) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Error " << err
                      << " in reading attribute '"
                      << startCellAtt->getShortName() << "'." <<  endl;
  }
  return err;
}


void VsUniformMesh::getMeshDataDims(std::vector<int>& dims)
{
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Entering." <<  endl;
  
  // Read dims of totalNumCells attribute
  numCellsAtt->getIntVectorValue(&dims);
}


void VsUniformMesh::getNumMeshDims(std::vector<int>& dims)
{
  // Get the number of cells
  getMeshDataDims(dims);

  // The number of nodes will be one more than the number of cells;
  for (unsigned int i = 0; i < dims.size(); i++)
    ++dims[i];
}
