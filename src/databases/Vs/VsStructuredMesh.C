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
  
  numSpatialDims = -1;
  numTopologicalDims = -1;  
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

  // Structured meshes are defined by an array containing the node
  // coordinates.
  std::vector< int > dims = dm->getDims();

  for( int i=0; i<dims.size(); ++i )
  {
    if( dims[i] <= 0 )
    {
      VsLog::debugLog()
        << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
        << "Failed to create mesh because dataset->dims[" << i
        << "] = " << dims[i] << " and is less than or equal to zero."
        << std::endl;

      return false;
    }  
  }
    
  if (dims.size() == 0 || 4 < dims.size())
  {
    VsLog::debugLog()
      << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
      << "Failed to create mesh because dataset->dims.size() is zero "
      << "or contains more than four dimensions."
      << std::endl;

    return false;
  }
  // Special case of 1D coordinates sans the spatial dimension.
  else if( dims.size() == 1 )
  {
    numSpatialDims = 1;
    numTopologicalDims = (dims[0] > 1); 

    VsLog::warningLog()
      << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
      << "Special 1D case.  "
      << "This file attempts to declare a 1D structured mesh with "
      << "an array of size [numPoints] but with no spatial dimension.  "
      << "Whereas the normal dimensions would be [numPoints][spatialDim] "
      << "As such, assume the spatial dimenson is 1." << std::endl;
  }  
  else
  {
    // ARS - Becasue of the way the data structures are used to hold
    // structured data in VTK and VisIt the topological dimension has to
    // equal the spatial dimension unless the last dim(s) are 1.

    // i.e. 1, 2, 3 = topological dims == 3
    // i.e. 3, 2, 1 = topological dims == 2
  
    if( isCompMinor() ) { // the comp minor ordering is X Y Z N
      numSpatialDims = dims[dims.size()-1];
      
      // Determine the topological dimension using the number of nodes.
      
      // Check for the last axis for a dimension greater than 1
      for( int i=0; i<(dims.size()-1); ++i ) {
        if( dims[i] > 1 ) {
          numTopologicalDims = i + 1;
        }
      }
        
      // Check each axis for a dimension greater than 1
      // numTopologicalDims = 0;
      // for( int i=0; i<(dims.size()-1); ++i ) {
      //   if (dims[i] > 1) {
      //     numTopologicalDims++;
      //   }
      // }
    }
    else { // the comp major ordering is N X Y Z
      numSpatialDims = dims[0];

      // Determine the topological dimension using the number of nodes.
      numTopologicalDims = 0;
      
      // Check for the last axis for a dimension greater than 1
      for( int i=1; i<dims.size(); ++i ) {
        if( dims[i] > 1 ) {
          numTopologicalDims = i;
        }
      }

      // Check each axis for a dimension greater than 1
      // for( int i=1; i<dims.size(); ++i ) {
      //   if (dims[i] > 1) {
      //     numTopologicalDims++;
      //   }
      // } 
    }
  }

    if( numSpatialDims < 1 || 3 < numSpatialDims )
    {
      VsLog::debugLog()
        << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
        << "Failed to create mesh because the numSpatialDims, "
        << numSpatialDims << " is less than 1 or greater than 3."
        << std::endl;
      
      return false;
    } 

    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Structured Mesh " << getShortName() << " "
                    << "has num spatial dims = "
                    << numSpatialDims << " and "
                    << "has num topological dims = "
                    << numTopologicalDims << std::endl;

  // Get the mask attribute.
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


void VsStructuredMesh::getNodeDims(std::vector<int>& dims) const
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

  // Lop off the spatial dimension.
  if( dims.size() > 1 )
  {    
    if( isCompMinor() ) {
      // Comp minor structured data dims is one of these:
      // 3D: [i][j][k][N] where N == 3
      // 2D: [i][j][N]    where 2 <= N <= 3
      // 1D: [i][N]       where 1 <= N <= 3
      // 1D: [i]
      
      // Lop off the spatial dimension which is last.
      dims.resize(dims.size()-1);
    }

    else
    {
      // Comp major structured data dims is one of these:
      // 3D: [N][i][j][k] where N == 3
      // 2D: [N][i][j]    where 2 <= N <= 3
      // 1D: [N][i]       where 1 <= N <= 3
      // 1D: [i]

      // The spatial dim is first so shift the dims over.
      for( size_t i=0; i<dims.size()-1; ++i )
        dims[i] = dims[i+1];

      // Lop off the spatial dimension which was first.
      dims.resize(dims.size()-1);
    }
  }

  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Exiting." << std::endl;
}


void VsStructuredMesh::getCellDims(std::vector<int>& dims) const
{
  // Determine the number of cells which is the number of nodes less 1.
  getNodeDims( dims );
  
  for( int i=0; i<dims.size(); ++i )
    dims[i] -= 1;
}


std::string VsStructuredMesh::getMaskName() const
{
  if (!maskAtt) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "No mask attribute, returning empty string."
                      << std::endl;
    
    return std::string("");
  }

  std::string maskName;
  int err = maskAtt->getStringValue(&maskName);

  if (err < 0) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Cannot get mask name from attribute, "
                      << "returning empty string." << std::endl;
    
    return std::string("");
  }

  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    <<"Returning makeCanonicalName of " <<maskName <<std::endl;
  
  return makeCanonicalName(getFullName(), maskName);
}
