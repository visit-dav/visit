#include <hdf5.h>
#include <visit-hdf5.h>
#if HDF5_VERSION_GE(1,8,1)
/**
 * @file  VsSchema.cpp
 *
 * @class VsSchema
 *
 * @brief Describes how instant datasets and meshes are found in the
 * file compliant with the schema.
 *
 * Copyright &copy; 2008 by Tech-X Corporation
 */

#include <VsSchema.h>

std::string VsSchema::mdAtt ="vsMD";

std::string VsSchema::typeAtt ="vsType";
std::string VsSchema::kindAtt = "vsKind";
std::string VsSchema::meshAtt = "vsMesh";

std::string VsSchema::centeringAtt ="vsCentering";
std::string VsSchema::indexOrderAtt = "vsIndexOrder";
std::string VsSchema::numSpatialDimsAtt = "vsNumSpatialDims";
std::string VsSchema::spatialIndicesAtt = "vsSpatialIndices";
std::string VsSchema::labelsAtt = "vsLabels";

std::string VsSchema::varKey = "variable";
std::string VsSchema::varWithMeshKey = "variableWithMesh";
std::string VsSchema::meshKey = "mesh";
std::string VsSchema::vsVarsKey = "vsVars";
std::string VsSchema::zonalCenteringKey = "zonal"; // Default is nodal

// Index ordering...
std::string VsSchema::compMajorCKey = "compMajorC";     //currently not supported
std::string VsSchema::compMinorCKey = "compMinorC";     //default ordering
std::string VsSchema::compMajorFKey = "compMajorF";     //currently not supported
std::string VsSchema::compMinorFKey = "compMinorF";     //supported

// Meshes

std::string VsSchema::structuredMeshKey = "structured";

std::string VsSchema::Uniform::key = "uniform";
std::string VsSchema::Uniform::deprecated_key = "uniformCartesian";
std::string VsSchema::Uniform::comp0 = "vsLowerBounds";
std::string VsSchema::Uniform::comp1 = "vsStartCell";
std::string VsSchema::Uniform::comp2 = "vsNumCells";
std::string VsSchema::Uniform::comp3 = "vsUpperBounds";

std::string VsSchema::Unstructured::key = "unstructured";
std::string VsSchema::Unstructured::defaultPolygonsName = "polygons";
std::string VsSchema::Unstructured::defaultPointsName = "points";
std::string VsSchema::Unstructured::vsPolygons = "vsPolygons";
std::string VsSchema::Unstructured::vsPoints = "vsPoints";
std::string VsSchema::Unstructured::vsPoints0 = "vsPoints0";
std::string VsSchema::Unstructured::vsPoints1 = "vsPoints1";
std::string VsSchema::Unstructured::vsPoints2 = "vsPoints2";
#endif
