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
}


VsRectilinearMesh::~VsRectilinearMesh() {
}


hid_t VsRectilinearMesh::getDataType() const{

  VsDataset* axis0 = getAxisDataset(0);

  if (!axis0) {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Can't determine data type - no axis0 dataset?"
                      << std::endl;

    return H5T_NATIVE_DOUBLE; //?
  }
  
  if (axis0 && axis0->getType() == H5T_NATIVE_DOUBLE)
    return H5T_NATIVE_DOUBLE;

  VsDataset* axis1 = getAxisDataset(1);
  if (axis1 && axis1->getType() == H5T_NATIVE_DOUBLE)
    return H5T_NATIVE_DOUBLE;

  VsDataset* axis2 = getAxisDataset(2);
  if (axis2 && axis2->getType() == H5T_NATIVE_DOUBLE)
    return H5T_NATIVE_DOUBLE;

  return H5T_NATIVE_FLOAT;
}


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

  // Rectilinear meshes are defined by a series of arrays. Each array
  // represents one spatial dimension i.e. numSpatialDims = number of
  // arrays.
  VsDataset* axis0 = getAxisDataset(0);
  VsDataset* axis1 = getAxisDataset(1);
  VsDataset* axis2 = getAxisDataset(2);

  if (!axis0) {
    VsLog::errorLog()
      << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
      << "Can't determine spatial dimensionality - no axis0 dataset?"
      << std::endl;

    numSpatialDims = -1;
    return false;
  }
  
  numSpatialDims = 1;
  if (axis1) {
    numSpatialDims = 2;
    if (axis2) {
      numSpatialDims = 3;
    }
  }

  //Calculate topological dims
  numTopologicalDims = 0;
  std::vector<int> axis0Dims = axis0->getDims();
  if (axis0Dims.size() != 1) {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Expected 1-d dataset for Axis 0, actually have " <<axis0Dims.size() <<std::endl;
    numTopologicalDims = numSpatialDims;
  } else {
    if (axis0Dims[0] > 1) {
      numTopologicalDims++;
    }

    //Check axis 1 (if it exists)
    if (axis1) {
      std::vector<int> axis1Dims = axis1->getDims();
      if (axis1Dims.size() != 1) {
        VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                          << "Expected 1-d dataset for Axis 1, actually have " <<axis1Dims.size() <<std::endl;
        numTopologicalDims = numSpatialDims;
      } else {
        if (axis1Dims[0] > 1) {
          numTopologicalDims++;
        }

        //Check axis 1 (if it exists)
        if (axis2) {
          std::vector<int> axis2Dims = axis2->getDims();
          if (axis2Dims.size() != 1) {
            VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                              << "Expected 1-d dataset for Axis 2, actually have " <<axis2Dims.size() <<std::endl;
            numTopologicalDims = numSpatialDims;
          } else {
            if (axis2Dims[0] > 1) {
              numTopologicalDims++;
            }
          }
        } //end if axis2
      }
    } //end if axis1
  }

  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Rectilinear Mesh " <<getShortName() <<" has num topological dims = "
                    << numTopologicalDims << std::endl;

  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Mesh has num spatial dims = "
                    << numSpatialDims << std::endl;

  return initializeRoot();
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

void VsRectilinearMesh::getCellDims(std::vector<int>& dims) const
{
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Entering." <<  std::endl;
  
  // The size of rectilinear mesh depends on the size of the component
  // arrays.
  
  // Require at least one axis
  VsDataset* axis0Data = getAxisDataset(0);
  if (!axis0Data) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to find information for axis 0." << "  "
                      << "Returning 0." << std::endl;
    dims.resize(0);
    return;
  }

  dims.resize(1);
  dims[0] = axis0Data->getDims().front();
  
  // If a first axis exists, check for a second axis
  VsDataset* axis1Data = getAxisDataset(1);
  if (axis1Data == NULL) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to find information for axis 1." << std::endl;
    return;
  }

  dims.resize(2);
  dims[1] = axis1Data->getDims().front();
  
  // If a second axis exists, check for a third axis
  VsDataset* axis2Data = getAxisDataset(2);
  if (axis2Data == NULL) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to find information for axis 2." << std::endl;
    return;
  }

  dims.resize(3);
  dims[2] = axis2Data->getDims().front();
  
//   size_t len = 1;
//   for (size_t i = 0; i < dims->size(); ++i)
//     len *= (*dims)[i];
  
//     VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
//                       << "Returning " <<len <<"." <<  std::endl;
//   return len;
}


void VsRectilinearMesh::getNodeDims(std::vector<int>& dims) const
{
  // Simple case get the data dims and return;
  getCellDims( dims );
}
