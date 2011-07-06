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

#include <string>

#define __CLASS__ "VsStructuredMesh::"

using namespace std;


VsStructuredMesh::VsStructuredMesh(VsH5Dataset* data):VsMesh(data) {
}


VsStructuredMesh::~VsStructuredMesh() {
}


VsStructuredMesh* VsStructuredMesh::buildStructuredMesh(VsH5Dataset* dataset)
{
  string methodSig("VsStructuredMesh::buildStructuredMesh() - ");

  VsStructuredMesh* newMesh = new VsStructuredMesh(dataset);
  bool success = newMesh->initialize();
  
  if (success) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "returning success." <<endl;
    return newMesh;
  }
  
  delete (newMesh);
  newMesh = NULL;

  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "returning failure." <<endl;
  return NULL;
}


bool VsStructuredMesh::initialize()
{
  string methodSig("VsStructuredMesh::initialize() - ");
    
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Entering" <<endl;

  VsH5Dataset* dm = registry->getDataset(getFullName());

  vector< int > dims = dm->getDims();
  
  // Determine num spatial dims. For a structured mesh, it is the size
  // of the last component of the dataset.
  int index = ((VsH5Dataset*)h5Object)->getDims().size() - 1;

  if (dims.empty())
  {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
      << "Failed to create mesh because dataset->dims.size() is zero." << endl;
    return false;
  }
  else if( dims.size() == 1 )
  {
    VsLog::warningLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Special 1D case." << endl
      << "This file attempts to declare a 1D structured mesh with" << endl
      << "an array of size [numPoints] but with no spatial dimension." << endl
      << "Whereas the normal dimensions would be [numPoints][spatialDim]"
      << endl
      << "As such, assume the spatial dimenson is 1." << endl;

    numSpatialDims = 1;
  }
  else
  {
    if( isCompMinor() )
      numSpatialDims = dims[dims.size()-1];
    else
      numSpatialDims = dims[0];
  }
  
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Mesh has num spatial dims = " << numSpatialDims << endl;
    
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Exiting." << endl;

  return initializeRoot();
}


std::string VsStructuredMesh::getKind() {
  return VsSchema::structuredMeshKey;
}


void VsStructuredMesh::getMeshDataDims(std::vector<int>& dims)
{
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Entering." << endl;
  
  // Read dataset's dims
  VsH5Dataset* dm = registry->getDataset(getFullName());

  if (!dm) {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Error: dataset " << getFullName()
                      << " not found. Returning." << endl;
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
                    << "Exiting." << endl;
}


void VsStructuredMesh::getNumMeshDims(std::vector<int>& dims)
{
  getMeshDataDims(dims);

  // Lop off the spatial dimension.
  if( dims.size() > 1 )
  {
    if( isCompMinor() )
      dims.resize(dims.size()-1);
    else
    {
      for( int i=0; i<dims.size()-1; ++i )
        dims[i] = dims[i+1];

      dims.resize(dims.size()-1);
    }
  }
}
