#include <hdf5.h>
#include <visit-hdf5.h>
#if HDF5_VERSION_GE(1, 8, 1)
/**
 * @file  VsMeta.h
 *
 * @class VsMeta
 *
 * @brief VsMeta represents vizschema metadata
 *
 * Copyright &copy; 2007 by Tech-X Corporation
 */

#ifndef VS_META_H
#define VS_META_H

#include <string>
#include <map>
#include <iostream>
#include <vector>

// HDF5
#include <hdf5.h>

#include <VsH5Meta.h>
#include <VsSchema.h>

struct VsVariableMeta {

  // Constructor
  VsVariableMeta() {
    indexOrder = VsSchema::compMinorCKey; //default
    dataset = 0;
    centering = "nodal";
  }

  // Name of the mesh
  std::string mesh;

  // Component order, if defined
  std::string indexOrder;

  bool isZonal() const {
    return (centering == VsSchema::zonalCenteringKey);
  }

  bool isCompMinor() const {
    if ((indexOrder == VsSchema::compMinorCKey) ||
        (indexOrder == VsSchema::compMinorFKey))
    return true;

    return false;
  }

  bool isCompMajor() const {
    if ((indexOrder == VsSchema::compMajorCKey) ||
        (indexOrder == VsSchema::compMajorFKey))
    return true;

    return false;
  }

  // Get user-specified component names
  std::vector<std::string> labelNames;
  std::string getLabel (unsigned int i) const {
    if ((i >= 0) && (i < labelNames.size()) && !labelNames[i].empty()) {
      return makeCanonicalName(path, labelNames[i]);
    }

    return "";
  }

  // Centering, if defined
  std::string centering;

  // Dataset
  VsDMeta* dataset;

  //fully qualified path to this object
  std::string path;

  // Get dims
  std::vector<int> getDims() const {
    return dataset->dims;
  }

  // Get mesh name
  std::string getMesh() const {
    return mesh;
  }

  // Get hdf5 type
  hid_t getType() const {
    return dataset->type;
  }

  // Get length needed to store all elements in their format
  size_t getLength() const {
    return dataset->getLength();
  }

  // Get name
  std::string getName () const {
    return dataset->name;
  }

  // Get full name
  std::string getFullName() const {
    return dataset->getFullName();
  }

  // Find attribute by name, or return NULL if not found
  const VsAMeta* find(const std::string name) const {
    return dataset->find(name);
  }

  std::string getStringAttribute(const std::string name) const {
    const VsAMeta* foundAtt = find(name);
    if (foundAtt == NULL)
    return "";
    std::string result = "";
    herr_t err = getAttributeHelper((foundAtt)->aid, &result, 0, 0);
    if (err < 0 ) {
      result = "";
    }
    /*
     if (result.empty()) {
     std::string msg = "VsVariableMeta::getStringAttribute(" + attName + ") - failed to find attribute in object " + getFullName();
     //debugStrmRef << msg << std::endl;
     }
     else {
     std::string msg = "VsMeta::getStringAttribute(" + attName + ") - found attribute in object " + getFullName();
     msg += " - value is " + result;
     //debugStrmRef << msg << std::endl;
     }*/

    return result;
  }
  // Write
  void write(std::ostream& os) const {
    if (dataset) dataset->write(os);
    os << "    indexOrder = " << indexOrder << std::endl;
    os << "    centering = " << centering << std::endl;
    os << "    mesh = " << mesh << std::endl;
  }

};

struct VsMDVariableMeta {

  /** List of all subordinate blocks in this var
   * Order is important (block 0 = blocks[0], etc)*/
  std::vector<VsVariableMeta*> blocks;

  // Constructor
  VsMDVariableMeta(std::string meshString, std::string centeringString, std::string indexOrderString) {
    mesh = meshString;
    indexOrder = indexOrderString;
    centering = centeringString;

    //default behaviors
    //indexOrder = VsSchema::compMinorCKey;
    //centering = "nodal";
  }

  //Destructor
  ~VsMDVariableMeta() {
    for (std::vector<VsVariableMeta*>::iterator it = blocks.begin(); it != blocks.end(); it++) {
      delete (*it);
    }
    blocks.clear();
  }

  //we look for user-specified component names in each subordinate variable
  //we return the first one we find
  std::string getLabel(int componentIndex) const {
    std::string name;
    name.clear();
    for (unsigned int i = 0; i < blocks.size(); i++) {
      name = blocks[i]->getLabel(componentIndex);
      if (!name.empty())
      return name;
    }

    return "";
  }

  std::string name;

  // Name of the mesh
  std::string mesh;

  // Component order, if defined
  std::string indexOrder;

  bool isZonal() const {
    return (centering == VsSchema::zonalCenteringKey);
  }

  // Centering, if defined
  std::string centering;

  //fully qualified path to this object
  std::string path;

  std::string addBlock(VsVariableMeta* newBlock) {
    //all blocks must match
    //Special note here: the mesh of newBlock will be the NON-MD mesh
    //But all we know is the name of the MD mesh
    //In theory we can look up the subordinate mesh here,
    //but I'm leaving it as a todo for now
    ///TODO: look up subordinate mesh name
    //     if (newBlock->mesh != mesh)
    //        return "VsMDVariableMeta rejected new block " + newBlock->getFullName() + " because meshes did not match (" + newBlock->mesh + " vs " + mesh + ")";

    if (newBlock->centering != centering)
    return "VsMDVariableMeta rejected new block " + newBlock->getFullName() + " because centering did not match (" + newBlock->centering + " vs " + centering + ")";

    if (newBlock->indexOrder != indexOrder)
    return "VsMDVariableMeta rejected new block " + newBlock->getFullName() + " because indexOrder did not match (" + newBlock->indexOrder + " vs " + indexOrder + ")";

    blocks.push_back(newBlock);

    return "";
  }

  unsigned int getNumBlocks() const {
    return blocks.size();
  }

  std::string getNameForBlock(unsigned int domain) const {
    if (domain >= blocks.size())
    return NULL;

    VsVariableMeta* foundVar = blocks[domain];
    return foundVar->getFullName();
  }

  VsVariableMeta* getBlock(unsigned int domain) const {
    if (domain >= blocks.size())
    return NULL;

    VsVariableMeta* foundVar = blocks[domain];
    return foundVar;
  }

  // Get mesh name
  std::string getMesh() const {
    return mesh;
  }

  // Get hdf5 type
  hid_t getType() const {
    if (!blocks.empty()) {
      return blocks[0]->getType();
    }

    return 0;
  }
  /*
   // Get length needed to store all elements in their format
   size_t getLength() const {
   return dataset->getLength();
   }
   */
  // Get name
  std::string getName() const {
    return name;
  }

  // Get full name
  std::string getFullName() const {
    return makeCanonicalName(path, name);
  }

  // Write
  void write(std::ostream& os) const {
    os <<"   MDVar: " <<getFullName() <<std::endl;
    for (unsigned int i = 0; i < blocks.size(); i++) {
      os <<"   block #" <<i <<std::endl;
      blocks[i]->write(os);
    }
    os << "    indexOrder = " << indexOrder << std::endl;
    os << "    centering = " << centering << std::endl;
    os << "    MD mesh = " << mesh << std::endl;
  }

};
// Metadata for particles should include number of spatial dims.
struct VsVariableWithMeshMeta {

  // Constructor
  VsVariableWithMeshMeta() {
    indexOrder = VsSchema::compMinorCKey;
    dataset = 0;
  }

  // We now maintain a list of the indices of the spatial dims
  std::vector<int> spatialIndices;

  unsigned int getNumSpatialDims() const {
    return spatialIndices.size();
  }

  //retrieve a particular spatial dimension index from the list
  //returns -1 on failure
  int getSpatialDim(size_t index) const {
    if ((index < 0) || (index > spatialIndices.size())) {
      return -1;
    }

    return spatialIndices[index];
  }

  //retrieve the entire list of spatial indices
  std::vector<int> getSpatialIndices() const {
    return spatialIndices;
  }

  bool isCompMinor() const {
    if ((indexOrder == VsSchema::compMinorCKey) ||
        (indexOrder == VsSchema::compMinorFKey))
    return true;

    return false;
  }

  bool isCompMajor() const {
    if ((indexOrder == VsSchema::compMajorCKey) ||
        (indexOrder == VsSchema::compMajorFKey))
    return true;

    return false;
  }

  // Order if defined
  std::string indexOrder;

  // Dataset
  VsDMeta* dataset;

  // Get dims
  std::vector<int> getDims() const {
    return dataset->dims;
  }

  // Get type
  hid_t getType() const {
    return dataset->type;
  }

  // Get length
  size_t getLength() const {
    return dataset->getLength();
  }

  // Get name
  std::string getName () const {
    return dataset->name;
  }

  // Get user-specified component names
  std::vector<std::string> labelNames;
  std::string getLabel (unsigned int i) const {
    if ((i >= 0) && (i < labelNames.size()) && !labelNames[i].empty()) {
      return makeCanonicalName(getPath(), labelNames[i]);
    }

    return "";
  }

  //get full name
  std::string getFullName() const {
    return dataset->getFullName();
  }

  //get path
  std::string getPath() const {
    return dataset->path;
  }

  void write(std::ostream& os) const {
    if (dataset) dataset->write(os);
    os << "    numSpatialDims  = " << getNumSpatialDims() << std::endl;
    os << "    spatialIndices = [";
    for (unsigned int i = 0; i < getNumSpatialDims(); i++) {
      os << spatialIndices[i];
      if (i + 1 < getNumSpatialDims()) {
        os <<", ";
      }
    }
    os <<"]" <<std::endl;
    os << "    indexOrder = " << indexOrder << std::endl;
  }

};

// Metadata for geometry should include a list of its components.
struct VsMeshMeta {

  VsMeshMeta() {
    numSpatialDims = 0;
    indexOrder = VsSchema::compMinorCKey;
  }

  void write(std::ostream& os) const {
    os << "    Index Order: " << indexOrder << std::endl;
    os << "    NumSpatialDims: " <<numSpatialDims <<std::endl;
    os << "    Datasets:" << std::endl;
    std::map<std::string, VsDMeta*>::const_iterator i;
    for (i = dComps.begin(); i != dComps.end(); ++i)
    os << "      " << i->first << std::endl;

    os << "    Attributes:" << std::endl;
    std::map<std::string, VsAMeta*>::const_iterator k;
    for (k = aComps.begin(); k != aComps.end(); ++k)
    os << "      " << k->first << std::endl;

  }

  bool isFortranOrder() const {
    if ((indexOrder == VsSchema::compMinorFKey) ||
        (indexOrder == VsSchema::compMajorFKey))
    return true;

    return false;
  }

  bool isCompMinor() const {
    if ((indexOrder == VsSchema::compMinorCKey) ||
        (indexOrder == VsSchema::compMinorFKey))
    return true;

    return false;
  }

  bool isCompMajor() const {
    if ((indexOrder == VsSchema::compMajorCKey) ||
        (indexOrder == VsSchema::compMajorFKey))
    return true;

    return false;
  }

  bool isUniformMesh() const {
    return ((kind == VsSchema::Uniform::key) || (kind == VsSchema::Uniform::deprecated_key));
  }

  bool isUnstructuredMesh() const {
    return (kind == VsSchema::Unstructured::key);
  }

  bool isStructuredMesh() const {
    return (kind == VsSchema::structuredMeshKey);
  }

  /** The kind of the mesh (e.g., structured, uniform, ... */
  std::string kind;

  /** The spatial dimensionality */
  size_t numSpatialDims;

  /** Index order (Fortran vs C style) */
  std::string indexOrder;

  /* name */
  std::string name;

  /* path */
  std::string path;

  /** Map of names of data sets to the metadata for that dataset */
  std::map<std::string, VsDMeta*> dComps;

  /** Map of names of attributes to the metadata for that attribute */
  std::map<std::string, VsAMeta*> aComps;

  std::string getFullName() const {
    return makeCanonicalName(path, name);
  }

  std::string getStringAttribute(std::string attName) const {
    std::map<std::string, VsAMeta*>::const_iterator it = aComps.find(attName);
    std::string result = "";
    if (it != aComps.end()) {
      VsAMeta* foundAtt = it->second;
      herr_t err = getAttributeHelper((foundAtt)->aid, &result, 0, 0);
      if (err < 0 ) {
        result = "";
      }
    }
    /*
     if (result.empty()) {
     std::string msg = "VsMeta::getStringAttribute(" + attName + ") - failed to find attribute in object " + getFullName();
     debugStrmRef << msg << std::endl;
     }
     else {
     std::string msg = "VsMeta::getStringAttribute(" + attName + ") - found attribute in object " + getFullName();
     msg += " - value is " + result;
     debugStrmRef << msg << std::endl;
     }
     */
    return result;
  }

  VsDMeta* getDataset(std::string name) const {

    std::map<std::string, VsDMeta*>::const_iterator i;
    for (i = dComps.begin(); i != dComps.end(); ++i) {
      if (i->first == name)
      return i->second;
    }

    return NULL;
  }

  VsAMeta* getAttribute(std::string name) const {
    std::map<std::string, VsAMeta*>::const_iterator i;
    for (i = aComps.begin(); i != aComps.end(); ++i) {
      if (i->first == name)
      return i->second;
    }

    return NULL;
  }

  int numnodes;
};

struct VsUniformMesh : public VsMeshMeta {

  hid_t getDataType() const {

    VsAMeta* lowerBounds = getAttribute(VsSchema::Uniform::lowerBounds);
    if (lowerBounds == NULL) {
      //ERROR!
      return H5T_NATIVE_DOUBLE; //?
    }

    return lowerBounds->type;
  }

};

struct VsUnstructuredMesh : public VsMeshMeta {

  bool usesSplitPoints() const {
    if (getPointsDatasetName().empty())
    return true;

    return false;
  }

  std::string getPointsDatasetName() const {
    //First see if the user has specified a name for the dataset
    std::string pointsName = getStringAttribute(VsSchema::Unstructured::vsPoints);
    if (!pointsName.empty()) {
      return pointsName;
    }

    //if we didn't find vsPoints, try the default name
    pointsName = getStringAttribute(VsSchema::Unstructured::defaultPointsName);
    if (!pointsName.empty()) {
      return pointsName;
    }

    return "";
  }

  hid_t getDataType() const {
    VsDMeta* pointsDataset = getPointsDataset();
    if (pointsDataset != NULL) {
      return pointsDataset->type;
    }

    //TODO: cache the type of points0, points1, points2?
    //TODO: make sure all are the same type
    return H5T_NATIVE_FLOAT;
  }

  std::string getPointsDatasetName(int i) const {
    //Ok, try vspoints0, vspoints1, vspoints2
    std::string attributeName = VsSchema::Unstructured::vsPoints0;
    if (i == 1)
    attributeName = VsSchema::Unstructured::vsPoints1;
    else if (i == 2)
    attributeName = VsSchema::Unstructured::vsPoints2;

    std::string fullName = getStringAttribute(attributeName); //might be empty
    //if the name was found, make sure it's fully qualified
    if (!fullName.empty()) {
      fullName = makeCanonicalName(path, fullName);
    }

    return fullName;
  }

  //NOTE - cannot be used to get vspoints0, vspoints1, vspoints2
  //because they are not dataset members of this group
  VsDMeta* getPointsDataset() const {
    std::string pointsName = getPointsDatasetName();
    if (pointsName.empty()) {
      return NULL;
    }

    //We look inside our own datasets
    VsDMeta* answer = getDataset(pointsName);

    //At this point
    return answer; //could be NULL
  }

  /*
   VsDMeta* getPointsDataset() {
   return getPointsDataset(0);
   }

   VsDMeta* getPointsDataset(int i) {
   std::string pointsName = getPointsDatasetName(i);
   if (pointsName.empty()) {
   return NULL;
   }

   //first we look inside our own datasets
   VsDMeta* answer = getDataset(pointsName);

   //next we try the global registry
   if (answer == NULL) {
   answer = VsH5Meta::getDataset(pointsName);
   }

   return answer; //could be NULL
   }*/

  std::string getPolygonsDatasetName() const {
    //First see if the user has specified a name for the dataset
    std::string polygonsName = getStringAttribute(VsSchema::Unstructured::vsPolygons);
    if (!polygonsName.empty()) {
      return polygonsName;
    }

    //if we didn't find anything, try the default name
    polygonsName = getStringAttribute(VsSchema::Unstructured::defaultPolygonsName);

    return polygonsName; //could be empty
  }

  VsDMeta* getPolygonsDataset() const {
    std::string polygonsName = getPolygonsDatasetName();
    if (polygonsName.empty()) {
      return NULL;
    }

    return getDataset(polygonsName);
  }

  std::string getPolyhedraDatasetName() const {
    //First see if the user has specified a name for the dataset
    std::string polyhedraName = getStringAttribute(VsSchema::Unstructured::vsPolyhedra);
    if (!polyhedraName.empty()) {
      return polyhedraName;
    }

    //if we didn't find anything, try the default name
    polyhedraName = getStringAttribute(VsSchema::Unstructured::defaultPolyhedraName);

    return polyhedraName; //could be empty
  }

  VsDMeta* getPolyhedraDataset() const {
    std::string polyhedraName = getPolyhedraDatasetName();
    if (polyhedraName.empty()) {
      return NULL;
    }

    return getDataset(polyhedraName);
  }

  std::string getLinesDatasetName() const {
    //First see if the user has specified a name for the dataset
    std::string linesName = getStringAttribute(VsSchema::Unstructured::vsLines);
    if (!linesName.empty()) {
      return linesName;
    }

    //if we didn't find anything, try the default name
    linesName = getStringAttribute(VsSchema::Unstructured::defaultLinesName);

    return linesName; //could be empty
  }

  VsDMeta* getLinesDataset() const {
    std::string linesName = getLinesDatasetName();
    if (linesName.empty()) {
      return NULL;
    }

    return getDataset(linesName);
  }

  std::string getTrianglesDatasetName() const {
    //First see if the user has specified a name for the dataset
    std::string trianglesName = getStringAttribute(VsSchema::Unstructured::vsTriangles);
    if (!trianglesName.empty()) {
      return trianglesName;
    }

    //if we didn't find anything, try the default name
    trianglesName = getStringAttribute(VsSchema::Unstructured::defaultTrianglesName);

    return trianglesName; //could be empty
  }

  VsDMeta* getTrianglesDataset() const {
    std::string trianglesName = getTrianglesDatasetName();
    if (trianglesName.empty()) {
      return NULL;
    }

    return getDataset(trianglesName);
  }

  std::string getQuadrilateralsDatasetName() const {
    //First see if the user has specified a name for the dataset
    std::string quadrilateralsName = getStringAttribute(VsSchema::Unstructured::vsQuadrilaterals);
    if (!quadrilateralsName.empty()) {
      return quadrilateralsName;
    }

    //if we didn't find anything, try the default name
    quadrilateralsName = getStringAttribute(VsSchema::Unstructured::defaultQuadrilateralsName);

    return quadrilateralsName; //could be empty
  }

  VsDMeta* getQuadrilateralsDataset() const {
    std::string quadrilateralsName = getQuadrilateralsDatasetName();
    if (quadrilateralsName.empty()) {
      return NULL;
    }

    return getDataset(quadrilateralsName);
  }

  std::string getTetrahedralsDatasetName() const {
    //First see if the user has specified a name for the dataset
    std::string tetrahedralsName = getStringAttribute(VsSchema::Unstructured::vsTetrahedrals);
    if (!tetrahedralsName.empty()) {
      return tetrahedralsName;
    }

    //if we didn't find anything, try the default name
    tetrahedralsName = getStringAttribute(VsSchema::Unstructured::defaultTetrahedralsName);

    return tetrahedralsName; //could be empty
  }

  VsDMeta* getTetrahedralsDataset() const {
    std::string tetrahedralsName = getTetrahedralsDatasetName();
    if (tetrahedralsName.empty()) {
      return NULL;
    }

    return getDataset(tetrahedralsName);
  }

  std::string getPyramidsDatasetName() const {
    //First see if the user has specified a name for the dataset
    std::string pyramidsName = getStringAttribute(VsSchema::Unstructured::vsPyramids);
    if (!pyramidsName.empty()) {
      return pyramidsName;
    }

    //if we didn't find anything, try the default name
    pyramidsName = getStringAttribute(VsSchema::Unstructured::defaultPyramidsName);

    return pyramidsName; //could be empty
  }

  VsDMeta* getPyramidsDataset() const {
    std::string pyramidsName = getPyramidsDatasetName();
    if (pyramidsName.empty()) {
      return NULL;
    }

    return getDataset(pyramidsName);
  }

  std::string getPrismsDatasetName() const {
    //First see if the user has specified a name for the dataset
    std::string prismsName = getStringAttribute(VsSchema::Unstructured::vsPrisms);
    if (!prismsName.empty()) {
      return prismsName;
    }

    //if we didn't find anything, try the default name
    prismsName = getStringAttribute(VsSchema::Unstructured::defaultPrismsName);

    return prismsName; //could be empty
  }

  VsDMeta* getPrismsDataset() const {
    std::string prismsName = getPrismsDatasetName();
    if (prismsName.empty()) {
      return NULL;
    }

    return getDataset(prismsName);
  }

  std::string getHexahedralsDatasetName() const {
    //First see if the user has specified a name for the dataset
    std::string hexahedralsName = getStringAttribute(VsSchema::Unstructured::vsHexahedrals);
    if (!hexahedralsName.empty()) {
      return hexahedralsName;
    }

    //if we didn't find anything, try the default name
    hexahedralsName = getStringAttribute(VsSchema::Unstructured::defaultHexahedralsName);

    return hexahedralsName; //could be empty
  }

  VsDMeta* getHexahedralsDataset() const {
    std::string hexahedralsName = getHexahedralsDatasetName();
    if (hexahedralsName.empty()) {
      return NULL;
    }

    return getDataset(hexahedralsName);
  }

  bool isPointMesh() const {
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
};

// Metadata for geometry should include a list of its components.
struct VsMDMeshMeta {

  VsMDMeshMeta(std::string meshName, std::string meshKind, int dims, std::string meshIndexOrder = VsSchema::compMinorCKey) {
    //Note: MD meshes do not have a path.  Their name is exactly what is declared in the "vsMD" attribute
    //This is because an MD mesh may be made up of meshes in different paths, and we would have to choose one over the other.
    //So, to avoid conflicts, we just use the name as given
    numSpatialDims = dims;
    indexOrder = meshIndexOrder;
    kind = meshKind;
    name = meshName;
  }

  //Destructor
  ~VsMDMeshMeta() {
    for(std::vector<VsMeshMeta*>::iterator it = blocks.begin(); it!= blocks.end(); it++) {
      delete (*it);
    }
    blocks.clear();
  }

  void write(std::ostream& os) const {
    os << "    Index Order: " << indexOrder << std::endl;
    os << "    Spatial Dimensionality: " << numSpatialDims <<std::endl;
    os << "    Kind: " << kind <<std::endl;
    os << "    Blocks:" <<std::endl;
    for (unsigned int i = 0; i < blocks.size(); i++) {
      os << "      Block #" << i << std::endl;
      blocks[i]->write(os);
    }
  }

  std::string addBlock(VsMeshMeta* newBlock) {
    if (newBlock->indexOrder != indexOrder)
    return "VsMDMeshMeta rejected new block " + newBlock->getFullName() + " because indexOrder did not match - (" + newBlock->indexOrder + " vs " + indexOrder + ")";

    if (newBlock->numSpatialDims != numSpatialDims)
    return "VsMDMeshMeta rejected new block " + newBlock->getFullName() + " because numSpatialDims did not match";

    if (newBlock->kind != kind)
    return "VsMDMeshMeta rejected new block " + newBlock->getFullName() + " because kind did not match (" + newBlock->kind + " vs " + kind + ")";

    blocks.push_back(newBlock);

    return "";
  }

  unsigned int getNumBlocks() const {
    return blocks.size();
  }

  std::string getNameForBlock(unsigned int domain) const {
    if (domain >= blocks.size())
    return NULL;

    VsMeshMeta* foundMesh = blocks[domain];
    return foundMesh->getFullName();
  }

  VsMeshMeta* getBlock(unsigned int domain) const {
    if (domain >= blocks.size())
    return NULL;

    VsMeshMeta* foundMesh = blocks[domain];
    return foundMesh;
  }

  std::string getMeshKind() const {
    return kind;
  }

  bool isUniformMesh() const {
    return ((kind == VsSchema::Uniform::key) || (kind == VsSchema::Uniform::deprecated_key));
  }

  bool isUnstructuredMesh() const {
    return (kind == VsSchema::Unstructured::key);
  }

  bool isStructuredMesh() const {
    return (kind == VsSchema::structuredMeshKey);
  }
  int getDims() const {
    return numSpatialDims;
  }

  /** The kind of the mesh (e.g., structured, uniform, ... */
  std::string kind;

  /** The spatial dimensionality */
  size_t numSpatialDims;

  /** Index order (Fortran vs C style) */
  std::string indexOrder;

  /* name */
  std::string name;

  /** List of all subordinate blocks in this mesh
   * Order is important (block 0 = blocks[0], etc)*/
  std::vector<VsMeshMeta*> blocks;

  std::string getFullName() const {
    //Note: MD meshes do not have a path.  Their name is exactly what is declared in the "vsMD" attribute
    //This is because an MD mesh may be made up of meshes in different paths, and we would have to choose one over the other.
    //So, to avoid conflicts, we just use the name as given
    return makeCanonicalName(name);
  }
};

struct VsMeta {

  VsMeta () {
  }

  ~VsMeta() {
    clear();
  }

  // List of group-meshes
  std::map<std::string, VsMeshMeta*> meshes;
  VsMeshMeta* getMesh(const std::string& name) const {
    //if the name does not start with a "/",
    // it is not fully qualified
    // and we assume it is in the root group
    // If not found in root, we search the namespace
    // and if we find that mesh, and it's unique, we return it
    // If there is a name conflict, we return the "closest" object

    // Make name fully qualified
    std::string fullName = makeCanonicalName(name);

    //     std::cerr << "Meshes - Looking for " << fullName << " from " << name << std::endl;
    //     std::map<std::string, VsMeshMeta*>::const_iterator i;
    //     for (i = meshes.begin(); i != meshes.end(); ++i) std::cerr << (const std::string)i->first << std::endl;


    //look for fully qualified name
    std::map<std::string, VsMeshMeta*>::const_iterator it = meshes.find(fullName);
    if (it != meshes.end()) {
      return (VsMeshMeta*)(it->second);
    }

    //fully qualified name does not show up
    //search by simple name
    // - but be careful for naming conflicts -
    VsMeshMeta* foundMesh = 0;
    for (it = meshes.begin(); it != meshes.end(); ++it) {
      if (name.compare(it->second->getFullName()) == 0) {
        //found a match
        //but is there already a match?
        //if so, choose the one with a path that is closer to the requested full name
        // i.e. if we requested /a/b/c
        // and we have found /a/c and /d/c
        // we want /a/c because it's "closer" to /a/b/c
        if (foundMesh != 0) {
          std::string candidate = it->second->getFullName();
          if (candidate == getClosestName(foundMesh->getFullName(), candidate, fullName)) {
            //the candidate name is closer to the target name than foundMesh
            // so switch
            foundMesh = it->second;
          }
          else {
            //candidate name is farther from the target than foundMesh
            //so keep foundMesh
          }
        }
        else {
          //we don't have a foundMesh yet, so just save it and continue
          foundMesh = it->second;
        }
      }
    }

    return foundMesh;
  }

  //md meshes
  std::map<std::string, VsMDMeshMeta*> mdMeshes;
  VsMDMeshMeta* getMDMesh(const std::string& name) const {
    //if the name does not start with a "/",
    // it is not fully qualified
    // and we assume it is in the root group
    // If not found in root, we search the namespace
    // and if we find that mesh, and it's unique, we return it
    // If there is a name conflict, we return the "closest" object

    // Make name fully qualified
    std::string fullName = makeCanonicalName(name);

    //look for fully qualified name
    std::map<std::string, VsMDMeshMeta*>::const_iterator it = mdMeshes.find(fullName);
    if (it != mdMeshes.end()) {
      return (VsMDMeshMeta*)(it->second);
    }

    //fully qualified name does not show up
    //search by simple name
    // - but be careful for naming conflicts -
    VsMDMeshMeta* foundMesh = 0;
    for (it = mdMeshes.begin(); it != mdMeshes.end(); ++it) {
      if (name.compare(it->second->getFullName()) == 0) {
        //found a match
        //but is there already a match?
        //if so, choose the one with a path that is closer to the requested full name
        // i.e. if we requested /a/b/c
        // and we have found /a/c and /d/c
        // we want /a/c because it's "closer" to /a/b/c
        if (foundMesh != 0) {
          std::string candidate = it->second->getFullName();
          if (candidate == getClosestName(foundMesh->getFullName(), candidate, fullName)) {
            //the candidate name is closer to the target name than foundMesh
            // so switch
            foundMesh = it->second;
          }
          else {
            //candidate name is farther from the target than foundMesh
            //so keep foundMesh
          }
        }
        else {
          //we don't have a foundMesh yet, so just save it and continue
          foundMesh = it->second;
        }
      }
    }

    return foundMesh;
  }

  //md meshes
  std::map<std::string, VsMDVariableMeta*> mdVars;
  VsMDVariableMeta* getMDVar(const std::string& name) const {
    //if the name does not start with a "/",
    // it is not fully qualified
    // and we assume it is in the root group
    // If not found in root, we search the namespace
    // and if we find that mesh, and it's unique, we return it
    // If there is a name conflict, we return the "closest" object

    // Make name fully qualified
    std::string fullName = makeCanonicalName(name);

    //look for fully qualified name
    std::map<std::string, VsMDVariableMeta*>::const_iterator it = mdVars.find(fullName);
    if (it != mdVars.end()) {
      return (VsMDVariableMeta*)(it->second);
    }

    //fully qualified name does not show up
    //search by simple name
    // - but be careful for naming conflicts -
    VsMDVariableMeta* foundVar = 0;
    for (it = mdVars.begin(); it != mdVars.end(); ++it) {
      if (name.compare(it->second->getFullName()) == 0) {
        //found a match
        //but is there already a match?
        //if so, choose the one with a path that is closer to the requested full name
        // i.e. if we requested /a/b/c
        // and we have found /a/c and /d/c
        // we want /a/c because it's "closer" to /a/b/c
        if (foundVar != 0) {
          std::string candidate = it->second->getFullName();
          if (candidate == getClosestName(foundVar->getFullName(), candidate, fullName)) {
            //the candidate name is closer to the target name than foundMesh
            // so switch
            foundVar = it->second;
          }
          else {
            //candidate name is farther from the target than foundMesh
            //so keep foundMesh
          }
        }
        else {
          //we don't have a foundMesh yet, so just save it and continue
          foundVar = it->second;
        }
      }
    }

    return foundVar;
  }

  // List of variables
  std::map<std::string, VsVariableMeta*> vars;
  const VsVariableMeta* getVar(const std::string& name) const {

    // Make name fully qualified
    std::string fullName = makeCanonicalName(name);

    //look for fully qualified name
    std::map<std::string, VsVariableMeta*>::const_iterator it = vars.find(fullName);
    if (it != vars.end()) {
      return (const VsVariableMeta*) it->second;
    }

    //fully qualified name does not show up
    //search by simple name
    // - but be careful for naming conflicts -
    // update 09.02.26 marc
    // I'm pretty sure this can never happen as part of the VisIt plugin
    // because VisIt pre-filters names based on what we supply
    const VsVariableMeta* foundVar = 0;
    for (it = vars.begin(); it != vars.end(); ++it) {
      if (name.compare(it->second->getName()) == 0) {
        //found a match
        //but is there already a match?
        //if so, choose the one with a path that is closer to the requested full name
        // i.e. if we requested /a/b/c
        // and we have found /a/c and /d/c
        // we want /a/c because it's "closer" to /a/b/c
        if (foundVar != 0) {
          std::string candidate = it->second->getFullName();
          if (candidate == getClosestName(foundVar->getFullName(), candidate, fullName)) {
            //the candidate name is closer to the target name than foundVar
            // so switch
            foundVar = it->second;
          }
          else {
            //candidate name is farther from the target than foundVar
            //so keep foundVar
          }
        }
        else {
          //we don't have a foundVar yet, so just save it and continue
          foundVar = it->second;
        }
      }
    }

    return foundVar;
  }

  // List of dataset-variable with meshes
  std::map<std::string, VsVariableWithMeshMeta*> varsWithMesh;
  const VsVariableWithMeshMeta* getVarWithMesh(const std::string& name) const {
    //if the name does not start with a "/",
    // it is not fully qualified
    // and we assume it is in the root group
    // If not found in root, we search the namespace
    // and if we find that mesh, and it's unique, we return it
    // If there is a name conflict, we return the "closest" name

    // Make name fully qualified
    std::string fullName = makeCanonicalName(name);

    //look for fully qualified name
    std::map<std::string, VsVariableWithMeshMeta*>::const_iterator it =
    varsWithMesh.find(fullName);
    if (it != varsWithMesh.end()) {
      return (const VsVariableWithMeshMeta*)(it->second);
    }

    //fully qualified name does not show up
    //search by simple name
    // - but be careful for naming conflicts -
    const VsVariableWithMeshMeta* foundVar = 0;
    for (it = varsWithMesh.begin(); it != varsWithMesh.end(); ++it) {
      if (name.compare(it->second->getName()) == 0) {
        //found a match
        //but is there already a match?
        //if so, choose the one with a path that is closer to the requested full name
        // i.e. if we requested /a/b/c
        // and we have found /a/c and /d/c
        // we want /a/c because it's "closer" to /a/b/c
        if (foundVar != 0) {
          std::string candidate = it->second->getFullName();
          if (candidate == getClosestName(foundVar->getFullName(), candidate, fullName)) {
            //the candidate name is closer to the target name than foundVar
            // so switch
            foundVar = it->second;
          }
          else {
            //candidate name is farther from the target than foundVar
            //so keep foundVar
          }
        }
        else {
          //we don't have a foundVar yet, so just save it and continue
          foundVar = it->second;
        }
      }
    }

    return foundVar;
  }

  // List of of derived variables (vsVars)
  std::map<std::string, std::string> vsVars;

  // Clean itself
  void clear() {

    // Need to delete all pointers
    std::map<std::string, VsMeshMeta*>::const_iterator i;
    for (i=meshes.begin(); i != meshes.end(); ++i)
    delete i->second;

    for (std::map<std::string, VsVariableMeta*>::const_iterator k=vars.begin(); k != vars.end(); ++k)
    delete k->second;

    for (std::map<std::string, VsVariableWithMeshMeta*>::const_iterator j=varsWithMesh.begin(); j != varsWithMesh.end(); ++j)
    delete j->second;

    for (std::map<std::string, VsMDVariableMeta*>::const_iterator h=mdVars.begin(); h != mdVars.end(); ++h)
    delete h->second;

    for (std::map<std::string, VsMDMeshMeta*>::const_iterator j=mdMeshes.begin(); j != mdMeshes.end(); ++j)
    delete j->second;

    // Clean lists
    meshes.clear();
    vars.clear();
    varsWithMesh.clear();
    mdVars.clear();
    mdMeshes.clear();
  }

  // Write
  void write(std::ostream& os) const {
    os <<"VsMeta::write() - dumping header data from input files." <<std::endl;
    os <<"VsMeta::write() ***************************************" <<std::endl;
    if (meshes.size()) os << "meshes"<<std::endl;
    for (std::map<std::string, VsMeshMeta*>::const_iterator i=meshes.begin(); i != meshes.end(); ++i) {
      os <<i->first <<std::endl;
      i->second->write(os);
    }

    if (mdMeshes.size()) os << "mdMeshes"<<std::endl;
    for (std::map<std::string, VsMDMeshMeta*>::const_iterator i=mdMeshes.begin(); i != mdMeshes.end(); ++i) {
      os <<i->first <<std::endl;
      i->second->write(os);
    }

    std::map<std::string, VsVariableMeta*>::const_iterator k;
    if (vars.size()) os << "vars" << std::endl;
    for (k=vars.begin(); k != vars.end(); ++k) {
      k->second->write(os);
    }

    if (mdVars.size()) os << "mdVars"<<std::endl;
    for (std::map<std::string, VsMDVariableMeta*>::const_iterator i=mdVars.begin(); i != mdVars.end(); ++i) {
      os <<i->first <<std::endl;
      i->second->write(os);
    }

    std::map<std::string, VsVariableWithMeshMeta*>::const_iterator j;
    if (varsWithMesh.size()) os << "varsWithMesh" << std::endl;
    for (j=varsWithMesh.begin(); j != varsWithMesh.end(); ++j) {
      j->second->write(os);
    }

    std::map<std::string, std::string>::const_iterator l;
    if (vsVars.size()) os << "vsVars" << std::endl;
    for (l=vsVars.begin(); l != vsVars.end(); ++l) {
      os << "    " << l->first << " = " << l->second << std::endl;
    }
    os <<"VsMeta::write() ***************************" <<std::endl;
    os <<"VsMeta::write() - done dumping header data." <<std::endl;
  }

};

#endif
#endif

