/**
 * @file  VsSchema.h
 *
 * @class VsSchema
 * @brief Contains all VizSchema key words.
 *
 * Copyright &copy; 2008 by Tech-X Corporation
 */
#include <hdf5.h>
#ifndef VS_SCHEMA
#define VS_SCHEMA

#include <string>

struct VsSchema {

  // MD elements
  static std::string mdAtt;

  // Elements of schema
  static std::string typeAtt;
  static std::string kindAtt;
  static std::string meshAtt;
  static std::string maskAtt;
  static std::string nodeOffsetAtt;
  static std::string centeringAtt; // This is deprecated
  static std::string cellOffsetAtt; // Instead of offsetAtt
  static std::string indexOrderAtt; //component major/minor, index C/Fortran; compMinorC is default
  static std::string numSpatialDimsAtt;
  static std::string numSpatialDimsAtt_deprecated;
  static std::string spatialIndicesAtt;
  static std::string labelsAtt;
  static std::string axisLabelsAtt;
  static std::string VsSubCellLocationsAtt;

  static std::string varKey;
  static std::string vsVarsKey;
  static std::string varWithMeshKey;
  static std::string meshKey;
  static std::string nodalCenteringKey;// Default
  static std::string edgeCenteringKey;
  static std::string faceCenteringKey;
  static std::string zonalCenteringKey;
  static std::string structuredMeshKey;

  // Index ordering...
  static std::string compMajorCKey;
  static std::string compMinorCKey;
  static std::string compMajorFKey;
  static std::string compMinorFKey;
  
  // Transforms
  static std::string transformKey;
  static std::string transformedMeshKey;  
  static std::string zrphiTransformKey;
  static std::string zrphiTransformKey_deprecated;
  
  struct Uniform {
    static std::string key;
    static std::string key_deprecated;
    static std::string lowerBounds;
    static std::string lowerBounds_deprecated;
    static std::string startCell;
    static std::string startCell_deprecated;
    static std::string numCells;
    static std::string numCells_deprecated;
    static std::string upperBounds;
    static std::string upperBounds_deprecated;
  };

  struct Rectilinear {
    static std::string key;

    static std::string axis0Key;
    static std::string axis0DefaultName;
    
    static std::string axis1Key;
    static std::string axis1DefaultName;
    
    static std::string axis2Key;
    static std::string axis2DefaultName;
  };

  struct Unstructured {
    static std::string key;

    static std::string defaultPolygonsName; //polygons
    static std::string defaultPolyhedraName; //polyhedra
    static std::string defaultPointsName; //points
    static std::string defaultLinesName; //Lines
    static std::string defaultTrianglesName; //Triangles
    static std::string defaultQuadrilateralsName; //Quadrilaterals
    static std::string defaultTetrahedralsName; //Tetrahedral
    static std::string defaultPyramidsName; //Pyramids
    static std::string defaultPrismsName; //Prisms
    static std::string defaultHexahedralsName; //Hexahedrals

    static std::string vsPolygons; //polygons
    static std::string vsPolyhedra; //polyhedra
    static std::string vsPoints; //points
    static std::string vsLines; //Lines
    static std::string vsTriangles; //Triangles
    static std::string vsQuadrilaterals; //Quadrilaterals
    static std::string vsTetrahedrals; //Tetrahedral
    static std::string vsPyramids; //Pyramids
    static std::string vsPrisms; //Prisms
    static std::string vsHexahedrals; //Hexahedrals
    static std::string vsPoints0; //points
    static std::string vsPoints1; //points
    static std::string vsPoints2; //points
  };

  //time
  static std::string timeKey;
  static std::string timeAtt;
  static std::string cycleAtt;
  static std::string timeGroupAtt;
  
  //run info
  static std::string runInfoKey;
  static std::string softwareAtt;
};

#endif

