/*
 * VsStructuredMesh.cpp
 *
 *  Created on: Apr 29, 2010
 *      Author: mdurant
 */

#include "VsStructuredMesh.h"
#include "VsAttribute.h"
#include "VsSchema.h"
#include "VsDataset.h"
#include "VsLog.h"
#include "VsUtils.h"

#include <string>

#define __CLASS__ "VsStructuredMesh::"


VsStructuredMesh::VsStructuredMesh(VsDataset* data):VsMesh(data) {
  maskAtt = NULL;
}


VsStructuredMesh::~VsStructuredMesh() {
}


VsStructuredMesh* VsStructuredMesh::buildStructuredMesh(VsDataset* dataset)
{
  VsStructuredMesh* newMesh = new VsStructuredMesh(dataset);
  bool success = newMesh->initialize();
  
  if (success) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "returning success." <<std::endl;
    return newMesh;
  }
  
  delete (newMesh);
  newMesh = NULL;

  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "returning failure." <<std::endl;
  return NULL;
}


bool VsStructuredMesh::initialize()
{
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Entering" <<std::endl;

  VsDataset* dm = registry->getDataset(getFullName());

  std::vector< int > dims = dm->getDims();
  
  // Determine num spatial dims. For a structured mesh, it is the size
  // of the last component of the dataset.
  //int index = ((VsDataset*)h5Object)->getDims().size() - 1;

  if (dims.empty())
  {
    VsLog::debugLog()
      << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
      << "Failed to create mesh because dataset->dims.size() is zero."
      << std::endl;

    return false;
  }
  else if( dims.size() == 1 )
  {
    VsLog::warningLog()
      << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
      << "Special 1D case.  "
      << "This file attempts to declare a 1D structured mesh with "
      << "an array of size [numPoints] but with no spatial dimension.  "
      << "Whereas the normal dimensions would be [numPoints][spatialDim] "
      << "As such, assume the spatial dimenson is 1." << std::endl;
    numTopologicalDims = 1;
    numSpatialDims = 1;
  }
  else
  {
    if( isCompMinor() ) {
      numSpatialDims = dims[dims.size()-1];
      numTopologicalDims = 0;
      for (int i = 0; i < (dims.size() - 1); i++) {
        if (dims[i] > 1) {
          numTopologicalDims++;
        }
      }
    }
    else {
      numSpatialDims = dims[0];
      numTopologicalDims = 0;
      for (size_t i = 1; i < dims.size(); i++) {
        if (dims[i] > 1) {
          numTopologicalDims++;
        }
      } 
    }
  }

  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Structured mesh " << getShortName() << " has topological dimensionality = " 
                    << numTopologicalDims << std::endl;
  
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Mesh has num spatial dims = " << numSpatialDims
                    << std::endl;

  maskAtt = getAttribute(VsSchema::maskAtt);
  if (maskAtt) {
     VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                       << "Mesh has a mask" << std::endl;
  } else {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                       << "Mesh does not have a mask" << std::endl;
  }
    
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Exiting." << std::endl;

  return initializeRoot();
}


std::string VsStructuredMesh::getKind() const {
  return VsSchema::structuredMeshKey;
}


void VsStructuredMesh::getCellDims(std::vector<int>& dims) const
{
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Entering." << std::endl;
  
  // Read dataset's dims
  VsDataset* dm = registry->getDataset(getFullName());

  if (!dm) {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Error: dataset " << getFullName() << " "
                      << "not found. Returning." << std::endl;
    return;
  }
 
  dims = dm->getDims();

  //Structured is funny because dims is one of these:
  // 3D: [i][j][k][N] where N == 3
  // 2D: [i][j][N]    where 2 <= N <= 3
  // 1D: [i][N]       where 1 <= N <= 3
  // 1D: [i]

  // Special case were there is 1D data with only one coordinate, if
  // so for consistancy add in the spatial dimension.
  if( dims.size() == 1 )
  {
    dims.resize(dims.size()+1);

    if( isCompMinor() )
      dims[dims.size()-1] = 1;
    else
    {
      dims[dims.size()-1] = dims[0];
      dims[0] = 1;
    }
  }

  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Exiting." << std::endl;
}


void VsStructuredMesh::getNodeDims(std::vector<int>& dims) const
{
  getCellDims(dims);

  // Lop off the spatial dimension.
  if( dims.size() > 1 )
  {
    if( isCompMinor() )
      dims.resize(dims.size()-1);
    else
    {
      for( size_t i=0; i<dims.size()-1; ++i )
        dims[i] = dims[i+1];

      dims.resize(dims.size()-1);
    }
  }
}

std::string VsStructuredMesh::getMaskName() const
{
  if (!maskAtt) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "No mask attribute, returning empty string." << std::endl;
    return std::string("");
  }

  std::string maskName;
  int err = maskAtt->getStringValue(&maskName);
  if (err < 0) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Cannot get mask name from attribute, returning empty string." << std::endl;
    return std::string("");
  }

  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    <<"Returning makeCanonicalName of " <<maskName <<std::endl;
  return makeCanonicalName(getFullName(), maskName);
}
