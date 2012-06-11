/*
 * VsUnstructuredMesh.C
 *
 *  Created on: Apr 29, 2010
 *      Author: mdurant
 */

#include "VsUnstructuredMesh.h"
#include "VsSchema.h"
#include "VsH5Dataset.h"
#include "VsUtils.h"
#include "VsH5Group.h"
#include "VsLog.h"

#define __CLASS__ "VsUnstructuredMesh::"

VsUnstructuredMesh::VsUnstructuredMesh(VsH5Group* group):VsMesh(group) {
  numPoints = 0;
  numCells = 0;
  splitPoints = false;
}

VsUnstructuredMesh::~VsUnstructuredMesh() {
}

//Tweak for Nautilus
std::string VsUnstructuredMesh::getNodeCorrectionDatasetName() {
  return makeCanonicalName(getFullName(), "localToGlobalNodeMapping");
}

bool VsUnstructuredMesh::hasNodeCorrectionData() {
  std::string datasetName = getNodeCorrectionDatasetName();

  //Look for a node correction list
  VsH5Dataset* nodeCorrectionDataset = registry->getDataset(datasetName);

  return (nodeCorrectionDataset != NULL);
}
//end tweak

unsigned int VsUnstructuredMesh::getNumPoints() {
  return numPoints;
}

unsigned int VsUnstructuredMesh::getNumCells() {
  return numCells;
}

bool VsUnstructuredMesh::usesSplitPoints()  {
  return splitPoints;
}

std::string VsUnstructuredMesh::getPointsDatasetName()  {
  //First see if the user has specified a name for the dataset
  std::string pointsName;
  getStringAttribute(VsSchema::Unstructured::vsPoints, &pointsName);
  if (!pointsName.empty()) {
    return makeCanonicalName(getFullName(), pointsName);
  }

  //if we didn't find vsPoints, try the default name
  return makeCanonicalName(getFullName(),
                           VsSchema::Unstructured::defaultPointsName);
}

std::string VsUnstructuredMesh::getPointsDatasetName(int i)  {
  
  std::string attributeName;
  switch (i) {
    case 0: attributeName = VsSchema::Unstructured::vsPoints0;
      break;
    case 1: attributeName = VsSchema::Unstructured::vsPoints1;
      break;
    case 2: attributeName = VsSchema::Unstructured::vsPoints2;
      break;
    default:

      VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "  
                        << "Requested index (" << i << ") is out of range."
                        << std::endl;
      return "";
  }
  
  std::string fullName;
  getStringAttribute(attributeName, &fullName); //might be empty
  //if the name was found, make sure it's fully qualified
  if (!fullName.empty()) {
    fullName = makeCanonicalName(getFullName(), fullName);
  }

  return fullName;
}

hid_t VsUnstructuredMesh::getDataType() {
  VsH5Dataset* pointsDataset = getPointsDataset();
  if (pointsDataset != NULL) {
    return pointsDataset->getType();
  }

  VsH5Dataset* points0Dataset = getPointsDataset(0);
  if (points0Dataset != NULL) {
    return points0Dataset->getType();
  }
  
  return H5T_NATIVE_FLOAT;
}

VsH5Dataset* VsUnstructuredMesh::getPointsDataset(int i)  {
  std::string pointsName = getPointsDatasetName(i);
  if (pointsName.empty()) {
    return NULL;
  }

  VsH5Dataset* answer = registry->getDataset(pointsName);

  return answer; //could be NULL
}

VsH5Dataset* VsUnstructuredMesh::getPointsDataset()  {
  std::string pointsName = getPointsDatasetName();
  if (pointsName.empty()) {
    return NULL;
  }

  VsH5Dataset* answer = registry->getDataset(pointsName);

  return answer; //could be NULL
}

std::string VsUnstructuredMesh::getPolygonsDatasetName()  {
  //First see if the user has specified a name for the dataset
  std::string polygonsName;
  getStringAttribute(VsSchema::Unstructured::vsPolygons, &polygonsName);
  if (!polygonsName.empty()) {
    return makeCanonicalName(getFullName(), polygonsName);
  }

  return makeCanonicalName(getFullName(), VsSchema::Unstructured::defaultPolygonsName);
}

VsH5Dataset* VsUnstructuredMesh::getPolygonsDataset()  {
  std::string polygonsName = getPolygonsDatasetName();
  if (polygonsName.empty()) {
    return NULL;
  }

  return registry->getDataset(polygonsName);
}

std::string VsUnstructuredMesh::getPolyhedraDatasetName()  {
  //First see if the user has specified a name for the dataset
  std::string polyhedraName;
  getStringAttribute(VsSchema::Unstructured::vsPolyhedra, &polyhedraName);
  if (!polyhedraName.empty()) {
    return makeCanonicalName(getFullName(), polyhedraName);
  }

  return makeCanonicalName(getFullName(), VsSchema::Unstructured::defaultPolyhedraName);
}

VsH5Dataset* VsUnstructuredMesh::getPolyhedraDataset()  {
  std::string polyhedraName = getPolyhedraDatasetName();
  if (polyhedraName.empty()) {
    return NULL;
  }

  return registry->getDataset(polyhedraName);
}

std::string VsUnstructuredMesh::getLinesDatasetName()   {
  //First see if the user has specified a name for the dataset
  std::string linesName;
  getStringAttribute(VsSchema::Unstructured::vsLines, &linesName);
  if (!linesName.empty()) {
    return makeCanonicalName(getFullName(), linesName);
  }
  
  return makeCanonicalName(getFullName(), VsSchema::Unstructured::defaultLinesName);
}

VsH5Dataset* VsUnstructuredMesh::getLinesDataset()   {
  std::string linesName = getLinesDatasetName();
  if (linesName.empty()) {
    return NULL;
  }

  return registry->getDataset(linesName);
}

std::string VsUnstructuredMesh::getTrianglesDatasetName()   {
  //First see if the user has specified a name for the dataset
  std::string trianglesName;
  getStringAttribute(VsSchema::Unstructured::vsTriangles, &trianglesName);
  if (!trianglesName.empty()) {
    return makeCanonicalName(getFullName(), trianglesName);
  }

  return makeCanonicalName(getFullName(), VsSchema::Unstructured::defaultTrianglesName);
}

VsH5Dataset* VsUnstructuredMesh::getTrianglesDataset()   {
  std::string trianglesName = getTrianglesDatasetName();
  if (trianglesName.empty()) {
    return NULL;
  }

  return registry->getDataset(trianglesName);
}

std::string VsUnstructuredMesh::getQuadrilateralsDatasetName()   {
  //First see if the user has specified a name for the dataset
  std::string quadrilateralsName;
  getStringAttribute(VsSchema::Unstructured::vsQuadrilaterals, &quadrilateralsName);
  if (!quadrilateralsName.empty()) {
    return makeCanonicalName(getFullName(), quadrilateralsName);
  }

  return makeCanonicalName(getFullName(), VsSchema::Unstructured::defaultQuadrilateralsName);
}

VsH5Dataset* VsUnstructuredMesh::getQuadrilateralsDataset()   {
  std::string quadrilateralsName = getQuadrilateralsDatasetName();
  if (quadrilateralsName.empty()) {
    return NULL;
  }

  return registry->getDataset(quadrilateralsName);
}

std::string VsUnstructuredMesh::getTetrahedralsDatasetName()   {
  //First see if the user has specified a name for the dataset
  std::string tetrahedralsName;
  getStringAttribute(VsSchema::Unstructured::vsTetrahedrals, &tetrahedralsName);
  if (!tetrahedralsName.empty()) {
    return makeCanonicalName(getFullName(), tetrahedralsName);
  }

  return makeCanonicalName(getFullName(), VsSchema::Unstructured::defaultTetrahedralsName);
}

VsH5Dataset* VsUnstructuredMesh::getTetrahedralsDataset()   {
  std::string tetrahedralsName = getTetrahedralsDatasetName();
  if (tetrahedralsName.empty()) {
    return NULL;
  }

  return registry->getDataset(tetrahedralsName);
}

std::string VsUnstructuredMesh::getPyramidsDatasetName()   {
  //First see if the user has specified a name for the dataset
  std::string pyramidsName;
  getStringAttribute(VsSchema::Unstructured::vsPyramids, &pyramidsName);
  if (!pyramidsName.empty()) {
    return makeCanonicalName(getFullName(), pyramidsName);
  }

  return makeCanonicalName(getFullName(), VsSchema::Unstructured::defaultPyramidsName);
}

VsH5Dataset* VsUnstructuredMesh::getPyramidsDataset()   {
  std::string pyramidsName = getPyramidsDatasetName();
  if (pyramidsName.empty()) {
    return NULL;
  }

  return registry->getDataset(pyramidsName);
}

std::string VsUnstructuredMesh::getPrismsDatasetName()   {
  //First see if the user has specified a name for the dataset
  std::string prismsName;
  getStringAttribute(VsSchema::Unstructured::vsPrisms, &prismsName);
  if (!prismsName.empty()) {
    return makeCanonicalName(getFullName(), prismsName);
  }

  return makeCanonicalName(getFullName(), VsSchema::Unstructured::defaultPrismsName);
}

VsH5Dataset* VsUnstructuredMesh::getPrismsDataset()   {
  std::string prismsName = getPrismsDatasetName();
  if (prismsName.empty()) {
    return NULL;
  }

  return registry->getDataset(prismsName);
}

std::string VsUnstructuredMesh::getHexahedralsDatasetName()   {
  //First see if the user has specified a name for the dataset
  std::string hexahedralsName;
  getStringAttribute(VsSchema::Unstructured::vsHexahedrals, &hexahedralsName);
  if (!hexahedralsName.empty()) {
    return makeCanonicalName(getFullName(), hexahedralsName);
  }

  return makeCanonicalName(getFullName(), VsSchema::Unstructured::defaultHexahedralsName);
}

VsH5Dataset* VsUnstructuredMesh::getHexahedralsDataset()   {
  std::string hexahedralsName = getHexahedralsDatasetName();
  if (hexahedralsName.empty()) {
    return NULL;
  }

  return registry->getDataset(hexahedralsName);
}

VsUnstructuredMesh* VsUnstructuredMesh::buildUnstructuredMesh(VsH5Group* group) {
  VsUnstructuredMesh* newMesh = new VsUnstructuredMesh(group);
  bool success = newMesh->initialize();
  
  if (success) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "  
                      << "Returning success." << std::endl;
    return newMesh;
  }

  delete (newMesh);
  newMesh = NULL;
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "  
                      << "Returning failure." << std::endl;
  return NULL;
}


bool VsUnstructuredMesh::initialize() {
  //For an unstructured mesh, spatial dimensionality is... complicated
  VsH5Dataset* pointsDataset = getPointsDataset();
  if (pointsDataset != NULL) {
    splitPoints = false;

    if( isCompMinor() )
    {
      numPoints = pointsDataset->getDims()[0];
      numSpatialDims = pointsDataset->getDims()[1];
      //guess that topological == spatial
      numTopologicalDims = numSpatialDims;
    }
    else
    {
      numSpatialDims = pointsDataset->getDims()[0];
      //guess that topological == spatial
      numTopologicalDims = numSpatialDims;
      numPoints = pointsDataset->getDims()[1];
    }
  }
  else {
    splitPoints = true;

    //it's possible that we have multiple points datasets
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "  
                      << "Path = " <<getPath() << "  "
                      << "vsPoints0 = " <<getPointsDatasetName(0) << "  "
                      << "vsPoints1 = " <<getPointsDatasetName(1) << "  "
                      << "vsPoints2 = " <<getPointsDatasetName(2) << std::endl;
    
    VsH5Dataset* points0 = getPointsDataset(0);
    VsH5Dataset* points1 = getPointsDataset(1);
    VsH5Dataset* points2 = getPointsDataset(2);
 
    if (!points0) {
    VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "  
                      << "Unable to load points data.  Returning false."
                      << std::endl;
      return false;
    }
    
    //spatial dimensionality = number of vspoints datasets
    numSpatialDims = 1;
    numTopologicalDims = 1;
    if (points1) {
      if (points2) {
        numSpatialDims = 3;
        numTopologicalDims = 3;
      } else {
        numSpatialDims = 2;
        numTopologicalDims = 2;
      }
    }

    numPoints = points0->getDims()[0];
  }

  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "Unstructured mesh " <<getShortName() <<" has num topological dims = " 
                    << numTopologicalDims <<std::endl;

  if( isPointMesh() )
  {
    numCells = numPoints;
  }
  else
  {
    // For now users can have only one connectivity dataset.
    VsH5Dataset* connectivityMeta = 0;
    std::string connectivityDatasetName;

    if( (connectivityMeta = getLinesDataset())) {
      connectivityDatasetName = getLinesDatasetName();
    } else if( (connectivityMeta = getPolygonsDataset()) ) {
      connectivityDatasetName = getPolygonsDatasetName();
    } else if( (connectivityMeta = getTrianglesDataset()) ) {
      connectivityDatasetName = getTrianglesDatasetName();
    } else if( (connectivityMeta = getQuadrilateralsDataset()) ) {
      connectivityDatasetName = getQuadrilateralsDatasetName();
    } else if( (connectivityMeta = getPolyhedraDataset()) ) {
      connectivityDatasetName = getPolyhedraDatasetName();
    } else if( (connectivityMeta = getTetrahedralsDataset()) ) {
      connectivityDatasetName = getTetrahedralsDatasetName();
    } else if( (connectivityMeta = getPyramidsDataset()) ) {
      connectivityDatasetName = getPyramidsDatasetName();
    } else if( (connectivityMeta = getPrismsDataset()) ) {
      connectivityDatasetName = getPrismsDatasetName();
    } else if( (connectivityMeta = getHexahedralsDataset()) ){
      connectivityDatasetName = getHexahedralsDatasetName();
    }

    VsH5Dataset* connectivityDataset =
      registry->getDataset(connectivityDatasetName);
    
    std::vector<int> connectivityDims = connectivityMeta->getDims();
    
    if( isCompMinor() )
    {
      numCells = connectivityDims[0];
    }
    else
    {
      numCells = connectivityDims[1];
    }
  }

  return initializeRoot();
}

bool VsUnstructuredMesh::isPointMesh() {
  return ((getPolygonsDataset() == NULL) &&
          (getPolyhedraDataset() == NULL) &&
    
          (getLinesDataset() == NULL) &&
          (getTrianglesDataset() == NULL) &&
          (getQuadrilateralsDataset() == NULL) &&
          
          (getTetrahedralsDataset() == NULL) &&
          (getPyramidsDataset() == NULL) &&
          (getPrismsDataset() == NULL) &&
          (getHexahedralsDataset() == NULL));
}

std::string VsUnstructuredMesh::getKind() {
  return VsSchema::Unstructured::key;
}

void VsUnstructuredMesh::getMeshDataDims(std::vector<int>& dims)
{
  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "entering" << std::endl;
  
  // Unstructured mesh is meshDataDims is: [#points][#spatialDims]
  dims.resize(2);

  if( isCompMinor() )
  {
    dims[0] = numPoints;
    dims[1] = numSpatialDims;
  }
  else
  {
    dims[0] = numSpatialDims;
    dims[1] = numPoints;
  }

  VsLog::debugLog() << __CLASS__ << __FUNCTION__ << "  " << __LINE__ << "  "
                    << "exiting" << std::endl;  
}
