// ************************************************************************* //
//                                 avtTypes.h                                //
// ************************************************************************* //

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
    AVT_MATERIAL,               /* 5 */
    AVT_MATSPECIES,             /* 6 */
    AVT_UNKNOWN_TYPE            /* 7 */
};


enum avtSubsetType
{
    AVT_DOMAIN_SUBSET      = 0,
    AVT_GROUP_SUBSET,     /* 1 */
    AVT_MATERIAL_SUBSET,  /* 2 */
    AVT_UNKNOWN_SUBSET    /* 3 */
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
    AVT_UNKNOWN_MESH         /* 5 */
};


enum avtGhostType
{
    AVT_NO_GHOSTS      = 0,
    AVT_HAS_GHOSTS,   /* 1 */
    AVT_MAYBE_GHOSTS  /* 2 */
};


enum SetState
{
    NoneUsed    = 0,
    SomeUsed,  /* 1 */
    AllUsed    /* 2 */
}; 


typedef std::vector<std::string> MaterialList;


DBATTS_API extern const char * AUXILIARY_DATA_DATA_EXTENTS;
DBATTS_API extern const char * AUXILIARY_DATA_EXTERNAL_FACELIST;
DBATTS_API extern const char * AUXILIARY_DATA_MATERIAL;
DBATTS_API extern const char * AUXILIARY_DATA_SPECIES;
DBATTS_API extern const char * AUXILIARY_DATA_SPATIAL_EXTENTS;
DBATTS_API extern const char * AUXILIARY_DATA_MIXED_VARIABLE;
DBATTS_API extern const char * AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION;

#endif


