// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_INTERFACE_TYPES_H
#define VISIT_INTERFACE_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef int visit_handle;

#define VISIT_INVALID_HANDLE -1

#define VISIT_ERROR  0
#define VISIT_OKAY   1
#define VISIT_NODATA 2

/* Mesh Types */
typedef enum {
    VISIT_MESHTYPE_UNKNOWN          = 0,
    VISIT_MESHTYPE_RECTILINEAR      = 1,
    VISIT_MESHTYPE_CURVILINEAR      = 2,
    VISIT_MESHTYPE_UNSTRUCTURED     = 3,
    VISIT_MESHTYPE_POINT            = 4,
    VISIT_MESHTYPE_CSG              = 5,
    VISIT_MESHTYPE_AMR              = 6
} VisIt_MeshType;

/* Centering */
typedef enum {
    VISIT_VARCENTERING_NODE = 0,
    VISIT_VARCENTERING_ZONE
} VisIt_VarCentering;

/* Variable Type */
typedef enum {
    /* Unknown variable type */
    VISIT_VARTYPE_UNKNOWN          = 0,
    /* Field variable types*/
    VISIT_VARTYPE_SCALAR           = 1,
    VISIT_VARTYPE_VECTOR           = 2,
    VISIT_VARTYPE_TENSOR           = 3,
    VISIT_VARTYPE_SYMMETRIC_TENSOR = 4,
    VISIT_VARTYPE_MATERIAL         = 5,
    VISIT_VARTYPE_MATSPECIES       = 6,
    VISIT_VARTYPE_LABEL            = 7,
    VISIT_VARTYPE_ARRAY            = 8,
    /* Mesh variable types (expressions only) */
    VISIT_VARTYPE_MESH             = 100,
    VISIT_VARTYPE_CURVE            = 101
} VisIt_VarType;

typedef enum
{
    VISIT_ENUMTYPE_NONE        = 0,
    VISIT_ENUMTYPE_BY_VALUE    = 1,
    VISIT_ENUMTYPE_BY_RANGE    = 2,
    VISIT_ENUMTYPE_BY_BITMASK  = 3,
    VISIT_ENUMTYPE_BY_NCHOOSER = 4
} VisIt_EnumType;
  
/* Command Argument Type */
#define VISIT_CMDARG_NONE                 0
#define VISIT_CMDARG_INT                  1
#define VISIT_CMDARG_FLOAT                2
#define VISIT_CMDARG_STRING               3

/* Simulation Mode */
#define VISIT_SIMMODE_UNKNOWN             0
#define VISIT_SIMMODE_RUNNING             1
#define VISIT_SIMMODE_STOPPED             2

/* Data Type */
#define VISIT_DATATYPE_CHAR               0
#define VISIT_DATATYPE_INT                1
#define VISIT_DATATYPE_FLOAT              2
#define VISIT_DATATYPE_DOUBLE             3
#define VISIT_DATATYPE_LONG               4
#define VISIT_DATATYPE_STRING             10

/* Array Owner */
#define VISIT_OWNER_SIM                   0
#define VISIT_OWNER_VISIT                 1
#define VISIT_OWNER_COPY                  2
#define VISIT_OWNER_VISIT_EX              3

/* Array memory layout */
#define VISIT_MEMORY_CONTIGUOUS           0
#define VISIT_MEMORY_STRIDED              1

/* Cell Types */
#define VISIT_CELL_BEAM                   0
#define VISIT_CELL_TRI                    1
#define VISIT_CELL_QUAD                   2
#define VISIT_CELL_TET                    3
#define VISIT_CELL_PYR                    4
#define VISIT_CELL_WEDGE                  5
#define VISIT_CELL_HEX                    6
#define VISIT_CELL_POINT                  7
#define VISIT_CELL_POLYHEDRON             8

#define VISIT_CELL_QUADRATIC_EDGE         20
#define VISIT_CELL_QUADRATIC_TRI          21
#define VISIT_CELL_QUADRATIC_QUAD         22
#define VISIT_CELL_QUADRATIC_TET          23
#define VISIT_CELL_QUADRATIC_PYR          24
#define VISIT_CELL_QUADRATIC_WEDGE        25
#define VISIT_CELL_QUADRATIC_HEX          26

#define VISIT_CELL_BIQUADRATIC_TRI        27
#define VISIT_CELL_BIQUADRATIC_QUAD       28
#define VISIT_CELL_TRIQUADRATIC_HEX       29
#define VISIT_CELL_QUADRATIC_LINEAR_QUAD         30
#define VISIT_CELL_QUADRATIC_LINEAR_WEDGE        31
#define VISIT_CELL_BIQUADRATIC_QUADRATIC_WEDGE   32
#define VISIT_CELL_BIQUADRATIC_QUADRATIC_HEX     33

/* Coordinate modes */
#define VISIT_COORD_MODE_SEPARATE         0
#define VISIT_COORD_MODE_INTERLEAVED      1

/* Ghost cell types */
#define VISIT_GHOSTCELL_REAL                  0
#define VISIT_GHOSTCELL_INTERIOR_BOUNDARY     1
#define VISIT_GHOSTCELL_EXTERIOR_BOUNDARY     2
#define VISIT_GHOSTCELL_ENHANCED_CONNECTIVITY 4
#define VISIT_GHOSTCELL_REDUCED_CONNECTIVITY  8
#define VISIT_GHOSTCELL_BLANK                 16
#define VISIT_GHOSTCELL_REFINED_AMR_CELL      32

/* Ghost node types */
#define VISIT_GHOSTNODE_REAL                  0
#define VISIT_GHOSTNODE_INTERIOR_BOUNDARY     1
#define VISIT_GHOSTNODE_BLANK                 2
#define VISIT_GHOSTNODE_COARSE_SIDE           3
#define VISIT_GHOSTNODE_FINE_SIDE             4

/* Definitions for CSG boundary types 
   Designed so low-order 16 bits are unused
   and can be included in silo.inc for Fortran 

   The last few characters of the symbol are intended
   to indicate the representational form of the surface type

   G = generalized form  (n values, depends on surface type)
   P = point (3 values, x,y,z in 3D, 2 values in 2D x,y)
   N = normal (3 values, Nx,Ny,Nz in 3D, 2 values in 2D Nx,Ny)
   R = radius (1 value)
   A = angle (1 value in degrees)
   L = length (1 value)
   X = x-intercept (1 value)
   Y = y-intercept (1 value)
   Z = z-intercept (1 value)
   K = arbitrary integer
   F = planar face defined by point-normal pair (6 values)
   */
#define VISIT_CSG_QUADRIC_G         16777216   /* 0x01000000 */
#define VISIT_CSG_SPHERE_PR         33619968   /* 0x02010000 */
#define VISIT_CSG_ELLIPSOID_PRRR    33685504   /* 0x02020000 */
#define VISIT_CSG_PLANE_G           50331648   /* 0x03000000 */
#define VISIT_CSG_PLANE_X           50397184   /* 0x03010000 */
#define VISIT_CSG_PLANE_Y           50462720   /* 0x03020000 */
#define VISIT_CSG_PLANE_Z           50528256   /* 0x03030000 */
#define VISIT_CSG_PLANE_PN          50593792   /* 0x03040000 */
#define VISIT_CSG_PLANE_PPP         50659328   /* 0x03050000 */
#define VISIT_CSG_CYLINDER_PNLR     67108864   /* 0x04000000 */
#define VISIT_CSG_CYLINDER_PPR      67174400   /* 0x04010000 */
#define VISIT_CSG_BOX_XYZXYZ        83886080   /* 0x05000000 */
#define VISIT_CSG_CONE_PNLA         100663296  /* 0x06000000 */
#define VISIT_CSG_CONE_PPA          100728832  /* 0x06010000 */
#define VISIT_CSG_POLYHEDRON_KF     117440512  /* 0x07000000 */
#define VISIT_CSG_HEX_6F            117506048  /* 0x07010000 */
#define VISIT_CSG_TET_4F            117571584  /* 0x07020000 */
#define VISIT_CSG_PYRAMID_5F        117637120  /* 0x07030000 */
#define VISIT_CSG_PRISM_5F          117702656  /* 0x07040000 */

/* Definitions for 2D CSG boundary types */
#define VISIT_CSG_QUADRATIC_G       134217728  /* 0x08000000 */
#define VISIT_CSG_CIRCLE_PR         150994944  /* 0x09000000 */
#define VISIT_CSG_ELLIPSE_PRR       151060480  /* 0x09010000 */
#define VISIT_CSG_LINE_G            167772160  /* 0x0A000000 */
#define VISIT_CSG_LINE_X            167837696  /* 0x0A010000 */
#define VISIT_CSG_LINE_Y            167903232  /* 0x0A020000 */
#define VISIT_CSG_LINE_PN           167968768  /* 0x0A030000 */
#define VISIT_CSG_LINE_PP           168034304  /* 0x0A040000 */
#define VISIT_CSG_BOX_XYXY          184549376  /* 0x0B000000 */
#define VISIT_CSG_ANGLE_PNLA        201326592  /* 0x0C000000 */
#define VISIT_CSG_ANGLE_PPA         201392128  /* 0x0C010000 */
#define VISIT_CSG_POLYGON_KP        218103808  /* 0x0D000000 */
#define VISIT_CSG_TRI_3P            218169344  /* 0x0D010000 */
#define VISIT_CSG_QUAD_4P           218234880  /* 0x0D020000 */

/* Definitions for CSG Region operators */
#define VISIT_CSG_INNER             2130706432 /* 0x7F000000 */
#define VISIT_CSG_OUTER             2130771968 /* 0x7F010000 */
#define VISIT_CSG_ON                2130837504 /* 0x7F020000 */
#define VISIT_CSG_UNION             2130903040 /* 0x7F030000 */
#define VISIT_CSG_INTERSECT         2130968576 /* 0x7F040000 */
#define VISIT_CSG_DIFF              2131034112 /* 0x7F050000 */
#define VISIT_CSG_COMPLIMENT        2131099648 /* 0x7F060000 */
/* XFORM and SWEEP are not yet implemented */
#define VISIT_CSG_XFORM             2131165184 /* 0x7F070000 */
#define VISIT_CSG_SWEEP             2131230720 /* 0x7F080000 */

/* Image formats */
#define VISIT_IMAGEFORMAT_BMP    0
#define VISIT_IMAGEFORMAT_JPEG   1
#define VISIT_IMAGEFORMAT_PNG    2
#define VISIT_IMAGEFORMAT_POVRAY 3
#define VISIT_IMAGEFORMAT_PPM    4
#define VISIT_IMAGEFORMAT_RGB    5
#define VISIT_IMAGEFORMAT_TIFF   6
#define VISIT_IMAGEFORMAT_EXR    7

/* Export option keys */
#define VISIT_EXPORT_WRITE_USING_GROUPS "EXPORT_WRITE_USING_GROUPS"
#define VISIT_EXPORT_GROUP_SIZE         "EXPORT_GROUP_SIZE"

/* Cinema spec types.*/
#define VISIT_CINEMA_SPEC_A 0
#define VISIT_CINEMA_SPEC_C 2
#define VISIT_CINEMA_SPEC_D 3

/* Cinema camera types */
#define VISIT_CINEMA_CAMERA_STATIC    0
#define VISIT_CINEMA_CAMERA_PHI_THETA 1

#ifdef __cplusplus
}
#endif
#endif
