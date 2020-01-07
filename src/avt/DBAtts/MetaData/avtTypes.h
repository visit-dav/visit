// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  avtTypes.h
// ****************************************************************************

#ifndef AVT_TYPES_H
#define AVT_TYPES_H

#include <dbatts_exports.h>

#include <vector>
#include <string>

enum avtVarType
{
    AVT_MESH                     = 0,
    AVT_SCALAR_VAR,             /* 1 */
    AVT_VECTOR_VAR,             /* 2 */
    AVT_TENSOR_VAR,             /* 3 */
    AVT_SYMMETRIC_TENSOR_VAR,   /* 4 */
    AVT_ARRAY_VAR,              /* 5 */
    AVT_LABEL_VAR,              /* 6 */
    AVT_MATERIAL,               /* 7 */
    AVT_MATSPECIES,             /* 8 */
    AVT_CURVE,                  /* 9 */
    AVT_UNKNOWN_TYPE            /* 10 */
};


enum avtSubsetType
{
    AVT_DOMAIN_SUBSET      = 0,
    AVT_GROUP_SUBSET,     /* 1 */
    AVT_MATERIAL_SUBSET,  /* 2 */
    AVT_ENUMSCALAR_SUBSET,/* 3 */
    AVT_UNKNOWN_SUBSET    /* 4 */
};


enum avtCentering
{
    AVT_NODECENT         = 0,
    AVT_ZONECENT,       /* 1 */
    AVT_NO_VARIABLE,    /* 2 */
    AVT_UNKNOWN_CENT    /* 3 */
};


enum avtExtentType
{
    AVT_ORIGINAL_EXTENTS      = 0,
    AVT_ACTUAL_EXTENTS,      /* 1 */
    AVT_SPECIFIED_EXTENTS,   /* 2 */
    AVT_UNKNOWN_EXTENT_TYPE  /* 3 */
};


enum avtMeshType
{
    AVT_RECTILINEAR_MESH      = 0,
    AVT_CURVILINEAR_MESH,    /* 1 */
    AVT_UNSTRUCTURED_MESH,   /* 2 */
    AVT_POINT_MESH,          /* 3 */
    AVT_SURFACE_MESH,        /* 4 */
    AVT_CSG_MESH,            /* 5 */
    AVT_AMR_MESH,            /* 6 */
    AVT_UNKNOWN_MESH         /* 7 */
};


enum avtGhostType
{
    AVT_NO_GHOSTS        = 0,
    AVT_HAS_GHOSTS,     /* 1 */
    AVT_CREATED_GHOSTS, /* 2 */
    AVT_MAYBE_GHOSTS    /* 3 */
};

//
// Note:
// These are used in a bit mask.
// If you need to extend the available types
// make sure to shift new enum values properly.
//
enum avtGhostsZonesPresent
{
    AVT_NO_GHOST_ZONES          = 0,
    AVT_BOUNDARY_GHOST_ZONES,  /* 1 */
    AVT_NESTING_GHOST_ZONES    /* 2 */
};

enum avtMeshCoordType
{
    AVT_XY      = 0,
    AVT_RZ,    /* 1 */
    AVT_ZR     /* 2 */
};

enum avtPrecisionType
{
    AVT_PRECISION_FLOAT     = 0,
    AVT_PRECISION_NATIVE,  /* 1 */
    AVT_PRECISION_DOUBLE   /* 2 */
};

enum avtBackendType
{
    AVT_BACKEND_VTK     = 0,
    AVT_BACKEND_VTKM   /* 1 */
};

enum SetState
{
    NoneUsed            = 0,
    SomeUsed,          /* 1 */
    AllUsed,           /* 2 */
    SomeUsedOtherProc, /* 3 */
    AllUsedOtherProc   /* 4 */
};


enum LoadBalanceScheme
{
    LOAD_BALANCE_UNKNOWN                       =-1,
    LOAD_BALANCE_CONTIGUOUS_BLOCKS_TOGETHER    = 0,
    LOAD_BALANCE_STRIDE_ACROSS_BLOCKS,        /* 1 */
    LOAD_BALANCE_RANDOM_ASSIGNMENT,           /* 2 */
    LOAD_BALANCE_DBPLUGIN_DYNAMIC,            /* 3 */
    LOAD_BALANCE_RESTRICTED,                  /* 4 */
    LOAD_BALANCE_ABSOLUTE,                    /* 5 */
    LOAD_BALANCE_STREAM                       /* 6 */
};

typedef std::vector<std::string> MaterialList;

DBATTS_API extern const char * AUXILIARY_DATA_DATA_EXTENTS;
DBATTS_API extern const char * AUXILIARY_DATA_EXTERNAL_FACELIST;
DBATTS_API extern const char * AUXILIARY_DATA_HISTOGRAM;
DBATTS_API extern const char * AUXILIARY_DATA_IDENTIFIERS;
DBATTS_API extern const char * AUXILIARY_DATA_MATERIAL;
DBATTS_API extern const char * AUXILIARY_DATA_POST_GHOST_MATERIAL;
DBATTS_API extern const char * AUXILIARY_DATA_SPECIES;
DBATTS_API extern const char * AUXILIARY_DATA_SPATIAL_EXTENTS;
DBATTS_API extern const char * AUXILIARY_DATA_MIXED_VARIABLE;
DBATTS_API extern const char * AUXILIARY_DATA_POST_GHOST_MIXED_VARIABLE;
DBATTS_API extern const char * AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION;
DBATTS_API extern const char * AUXILIARY_DATA_STREAMING_GHOST_GENERATION;
DBATTS_API extern const char * AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION;
DBATTS_API extern const char * AUXILIARY_DATA_GLOBAL_NODE_IDS;
DBATTS_API extern const char * AUXILIARY_DATA_GLOBAL_ZONE_IDS;

DBATTS_API std::string avtExtentType_ToString(avtExtentType);
DBATTS_API std::string avtExtentType_ToString(int);
DBATTS_API bool avtExtentType_FromString(const std::string &, avtExtentType &);

DBATTS_API std::string avtVarTypeToString(avtVarType);
DBATTS_API std::string avtPrecisionTypeToString(avtPrecisionType);
DBATTS_API std::string avtBackendTypeToString(avtBackendType);

DBATTS_API std::string LoadBalanceSchemeToString(LoadBalanceScheme);

DBATTS_API avtVarType GuessVarTypeFromNumDimsAndComps(int numSpatialDims,
                                                      int componentCount);
#endif


