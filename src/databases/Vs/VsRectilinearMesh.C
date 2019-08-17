/*
 * VsRectilinearMesh.cpp
 *
 *  Created on: Apr 29, 2010
 *      Author: mdurant
 */

#include "VsRectilinearMesh.h"
#include "VsSchema.h"
#include "VsDataset.h"
#include "VsGroup.h"
#include "VsAttribute.h"
#include "VsLog.h"
#include "VsUtils.h"

#include <string>

#define __CLASS__ "VsRectilinearMesh::"


VsRectilinearMesh::VsRectilinearMesh(VsGroup* group):VsMesh(group) {
    numSpatialDims = -1;
    numTopologicalDims = -1;
}

VsRectilinearMesh::~VsRectilinearMesh() {}


VsRectilinearMesh* VsRectilinearMesh::buildRectilinearMesh(VsGroup* group) {

  VsRectilinearMesh* newMesh = new VsRectilinearMesh(group);
  bool success = newMesh->initialize();

  if (success) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Returning success." << std::endl;
    return newMesh;
  }

  delete (newMesh);
  newMesh = NULL;

  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Returning NULL." << std::endl;
  return NULL;
}

bool VsRectilinearMesh::initialize() {

  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Entering" <<std::endl;

  // Rectilinear meshes are defined by a series of arrays containing
  // the node coordinates. Each array represents one spatial dimension
  // i.e. numSpatialDims = number of arrays.

  // This checks for missing axis 0.
  if (!getAxisDataset(0)) {
      VsLog::errorLog()
        << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
        << "Can't determine spatial dimensionality - no axis "
        << 0 << " '" << getAxisDatasetName(0) << "' dataset?" << std::endl;

      return false;
  }

  // This checks for missing axis 1 but have axis 2.
  else if (!getAxisDataset(1) && getAxisDataset(2)) {
      VsLog::errorLog()
        << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
        << "Can't determine spatial dimensionality - no axis "
        << 1 << " '" << getAxisDatasetName(1) << "' dataset?" << std::endl;

      return false;
  }

  numTopologicalDims = 0;

  // Get each axis that contributes to the spatial and topological dim.
  for( int i=0; i<3; ++i ) {

    VsDataset* axis = getAxisDataset(i);

    if (axis) {
      std::vector<int> axisDims = axis->getDims();

      if (axisDims.size() != 1) {
        if( axisDims.size() == 2 && axisDims[1] == 1 ) {
          VsLog::errorLog()
            << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
            << "Expected 1-d dataset for Axis "
            << i << " '" << getAxisDatasetName(i) << "', "
            << "actually have " << axisDims.size() << "  "
            << "ignoring because the second dimension is 1." << std::endl;
        } else {
          VsLog::errorLog()
            << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
            << "Expected 1-d dataset for Axis "
            << i << " '" << getAxisDatasetName(i) << "', "
            << "actually have " << axisDims.size() << "." << std::endl;
          return false;
        }
      }
      if (axisDims[0] == 0) {
        VsLog::debugLog()
          << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
          << "Expected data in dataset for Axis "
          << i << " '" << getAxisDatasetName(i) << "', "
          << "actually have none." << std::endl;
      }
      numSpatialDims = i + 1;

      // ARS - Because of the way the data structures are used to hold
      // structured data in VTK and VisIt the topological dimension has to
      // equal the spatial dimension unless the last dim(s) are 1.

      // i.e. 1, 1, 2 = topological dims == 3
      // i.e. 2, 1, 1 = topological dims == 1

      // Check for the last axis for a dimension greater than 1
      if ( axisDims[0] > 1 ) {
        numTopologicalDims = i + 1;
      }

      // Check each axis for a dimension greater than 1
      // if (axisDims[0] > 1) {
      //        numTopologicalDims++;
      // }
    }
    else
    {
      break;
    }
  }

  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Rectilinear Mesh " << getShortName() << " "
                    << "has num spatial dims = "
                    << numSpatialDims << " and "
                    << "has num topological dims = "
                    << numTopologicalDims << std::endl;

  return initializeRoot();
}


hid_t VsRectilinearMesh::getDataType() const{

  if (!getAxisDataset(0)) {
    VsLog::errorLog()
      << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
      << "Can't determine data type - no axis "
      << 0 << " '" << getAxisDatasetName(0) << "' dataset?" << std::endl;
    return H5T_NATIVE_DOUBLE; //?
  }

  for( int i=0; i<3; ++i )
  {
    VsDataset* axis = getAxisDataset(i);

    if (axis && axis->getType() == H5T_NATIVE_DOUBLE)
      return H5T_NATIVE_DOUBLE;
  }

  return H5T_NATIVE_FLOAT;
}


std::string VsRectilinearMesh::getAxisDatasetName(int axisNumber) const {
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
  VsAttribute* axisNameAtt = getAttribute(axisKey);
  if (axisNameAtt) {
    axisNameAtt->getStringValue(&axisName);
    if (!axisName.empty()) {
      return makeCanonicalName(getFullName(), axisName);
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
  return makeCanonicalName(getFullName(), axisName);
}


VsDataset* VsRectilinearMesh::getAxisDataset(int axisNumber) const {
  std::string axisDatasetName = getAxisDatasetName(axisNumber);
  if (axisDatasetName.empty()) {
    return NULL;
  }

  VsDataset* answer = registry->getDataset(axisDatasetName);

  return answer; //could be NULL
}


std::string VsRectilinearMesh::getKind() const {
  return VsSchema::Rectilinear::key;
}


void VsRectilinearMesh::getNodeDims(std::vector<int>& dims) const
{
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Entering." <<  std::endl;

  // The size of rectilinear mesh depends on the numSpatialDims
  dims.resize(numSpatialDims);

  for( int i=0; i<numSpatialDims; ++i )
  {
    dims[i] = getAxisDataset(i)->getDims()[0];
  }
}


void VsRectilinearMesh::getCellDims(std::vector<int>& dims) const
{
  // Determine the number of cells which is the number of nodes less 1.
  getNodeDims( dims );

  for( int i=0; i<dims.size(); ++i )
    dims[i] -= 1;
}
