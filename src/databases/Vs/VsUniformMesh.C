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
  
  numSpatialDims = -1;
  numTopologicalDims = -1;  
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
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Entering" <<std::endl;

  // Uniform meshes are defined by an lower and upper bounds and a
  // number of cells.
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

  std::vector<int> iNumCells;
  int err = numCellsAtt->getIntVectorValue(&iNumCells);
  if( err ) {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to get dimensionality from attribute: "
                      << numCellsAtt->getShortName() << std::endl;
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to initialize mesh, returning failure."
                      << std::endl;
    return false;
  }

  if( iNumCells.size() == 0 || 3 < iNumCells.size() ) {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Malformed number of cells attribute: "
                      << numCellsAtt->getShortName() << "  "
                      << "contains zero or more than three dimensions. "
                      << "Unable to initialize mesh, returning failure."
                      << std::endl;
    return false;
  }    

  if( (iNumCells.size() >= 1 && iNumCells[0] < 0) ||
      (iNumCells.size() >= 2 && iNumCells[1] < 0) ||
      (iNumCells.size() == 3 && iNumCells[2] < 0) ) {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Malformed number of cells attribute: "
                      << numCellsAtt->getShortName() << "  "
                      << "contains a negative number of cells. "
                      << "Unable to initialize mesh, returning failure."
                      << std::endl;
    return false;
  }    
      
  // Lowerbounds is required
  lowerBoundsAtt = getAttribute(VsSchema::Uniform::lowerBounds);
  
  if (!lowerBoundsAtt) {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Uniform mesh is missing attribute: "
                      << VsSchema::Uniform::lowerBounds
                      << ".  Looking for deprecated attribute: "
                      << VsSchema::Uniform::lowerBounds_deprecated << std::endl;

    lowerBoundsAtt = getAttribute(VsSchema::Uniform::lowerBounds_deprecated);
  }

  if (!lowerBoundsAtt) {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Uniform mesh is also missing deprecated attribute: "
                      << VsSchema::Uniform::lowerBounds_deprecated
                      << ".  Unable to initialize mesh, returning failure."
                      << std::endl;
    return false;
  }
  
  // Upperbounds is required
  upperBoundsAtt = getAttribute(VsSchema::Uniform::upperBounds);
  if (!upperBoundsAtt) {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Uniform mesh is missing attribute: "
                      << VsSchema::Uniform::upperBounds
                      << ".  Looking for deprecated attribute: "
                      << VsSchema::Uniform::upperBounds_deprecated << std::endl;
    upperBoundsAtt = getAttribute(VsSchema::Uniform::upperBounds_deprecated);
  }

  if (!upperBoundsAtt) {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Uniform mesh is also missing deprecated attribute: "
                      << VsSchema::Uniform::upperBounds_deprecated
                      << ".  Unable to initialize mesh, returning failure."
                      << std::endl;
    return false;
  }

  // Get the lower bounds spatial dimension.
  std::vector<double> dLowerBoundVals;
  err = lowerBoundsAtt->getDoubleVectorValue(&dLowerBoundVals);
  if( err )
  {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to get lower bounds from attribute: "
                      << lowerBoundsAtt->getShortName()
                      << ".  Unable to initialize mesh, returning failure."
                      << std::endl;
    return false;
  }

  if( iNumCells.size() != dLowerBoundVals.size() )
  {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Lower bounds and spatial dimensions do not match: "
                      <<  iNumCells.size() << "  " << dLowerBoundVals.size()
                      << ".  Unable to initialize mesh, returning failure."
                      << std::endl;
    return false;
  }

  // Get the upper bounds spatial dimension.
  std::vector<double> dUpperBoundVals;
  err = upperBoundsAtt->getDoubleVectorValue(&dUpperBoundVals);
  if( err )
  {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Unable to get upper bounds from attribute: "
                      << upperBoundsAtt->getShortName()
                      << ".  Unable to initialize mesh, returning failure."
                      << std::endl;
    return false;
  }

  if( iNumCells.size() != dUpperBoundVals.size() )
  {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Upper bounds and spatial dimensions do not match: "
                      <<  iNumCells.size() << "  " << dUpperBoundVals.size()
                      << ".  Unable to initialize mesh, returning failure."
                      << std::endl;
    return false;
  }

  // Malformed bounds 
  if( (iNumCells.size() >= 1 && iNumCells[0] > 0 &&
       dLowerBoundVals[0] == dUpperBoundVals[0]) ||

      (iNumCells.size() >= 2 && iNumCells[1] > 0 &&
       dLowerBoundVals[1] == dUpperBoundVals[1]) ||

      (iNumCells.size() == 3 && iNumCells[2] > 0 &&
       dLowerBoundVals[2] == dUpperBoundVals[2]) ) {
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Malformed mesh bounds and number of cells. "
                      << "Must have at least one cell along an axis"
                      << ".  Unable to initialize mesh, returning failure."
                      << std::endl;
    return false;
  }

  // For a uniform mesh, spatial dimensionality is the length of the
  // numCells array
  numSpatialDims = iNumCells.size();
  
  // ARS - Becasue of the way the data structures are used to hold
  // structured data in VTK and VisIt the topological dimension has to
  // equal the spatial dimension unless the last dim(s) are 0.

  // i.e. 0, 0, 2 = topological dims == 3
  // i.e. 2, 0, 0 = topological dims == 1
  
  // Determine the topological dimension using the number of cells.
  numTopologicalDims = 0;

  // Check for the last axis for a dimension greater than 0
  for (int i = 0; i < iNumCells.size(); i++) {
    if (iNumCells[i] > 0) {
      numTopologicalDims = i + 1;
    }
  }
  
  // Check each axis for a dimension greater than 1
  // for (int i = 0; i < iNumCells.size(); i++) {
  //   if (iNumCells[i] > 0) {
  //     numTopologicalDims++;
  //   }
  // }
  
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Uniform Mesh " << getShortName() << " "
                    << "has num spatial dims = "
                    << numSpatialDims << " and "
                    << "has num topological dims = "
                    << numTopologicalDims << std::endl;

  // ARS _ CURRENTLY THE START CELL IS IGNORED AS HOW IT IS TO USED IS
  // NOT FULLY DEFINED.

  // StartCell is optional
  startCellAtt = getAttribute(VsSchema::Uniform::startCell);

  if (!startCellAtt) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Uniform mesh does not have optional attribute: "
                      << VsSchema::Uniform::startCell
                      << ".  Looking for deprecated attribute: "
                      << VsSchema::Uniform::startCell_deprecated << std::endl;
    startCellAtt = getAttribute(VsSchema::Uniform::startCell_deprecated);
  }

  if (!startCellAtt) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Uniform mesh does not have deprecated attribute: "
                      << VsSchema::Uniform::startCell_deprecated
                      << ".  Using default start cell of 0." << std::endl;
  }
  else {
    std::vector<int> iStartCell;
    int err = startCellAtt->getIntVectorValue(&iStartCell);
    if (err) {
      VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "Unable to get dimensionality from attribute: "
                        << startCellAtt->getShortName()
                        << ".  Unable to initialize mesh, returning failure."
                        << std::endl;
      return false;
    }
// SS: These restrictions are commented out because we have cases that have mismatched dimensions but they should work anyway.
/*
    if( iNumCells.size() != iStartCell.size() )
    {
      VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "start cell and spatial dimensions do not match: "
                        <<  iNumCells.size() << "  " << iStartCell.size()
                        << ".  Unable to initialize mesh, returning failure."
                        << std::endl;
      return false;
    }

    if( (iStartCell.size() == 3 &&
         (iStartCell[2] < 0 || iNumCells[2] < iStartCell[2])) ||
        (iStartCell.size() == 2 &&
         (iStartCell[1] < 0 || iNumCells[1] < iStartCell[1])) ||
        (iStartCell.size() == 1 &&
         (iStartCell[0] < 0 || iNumCells[0] < iStartCell[0])) )
    {
      VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                        << "start cell bounds are greater than the cell bounds"
                        << ".  Unable to initialize mesh, returning failure."
                        << std::endl;
      return false;
    }
*/
    VsLog::errorLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                      << "Found a start cell for this mesh, "
                      << "but not sure how it should be used, "
                      << "ignoring it for now."
                      << std::endl;
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
  
  VsLog::debugLog() << "VsUniformMesh::getUpperBounds() - Returning " << err
                    << "." <<  std::endl;

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
  
  numCellsAtt->getIntVectorValue(&dims);
}


void VsUniformMesh::getNodeDims(std::vector<int>& dims) const
{
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Entering." <<  std::endl;

  // Determine the number of nodes which is the number of cells plus 1.
  numCellsAtt->getIntVectorValue(&dims);
  
  for( int i=0; i<dims.size(); ++i )
    dims[i] += 1;
}
