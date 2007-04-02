/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/
/****************************************************************************
** File      : VisItDataInterface_V1.h
**
** Purpose:
**
** Programmer: Jeremy Meredith
** Creation:   March 17, 2005
**
** Modifications:
**   Shelly Prevost, Tue Sep 12 15:42:17 PDT 2006
**   Changed struct VisIt_SimulationControlCommand name field from a
**   const to a variable.
**
*****************************************************************************/

#ifndef VISIT_DATA_INTERFACE_V1_H
#define VISIT_DATA_INTERFACE_V1_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Mesh Types */
#define VISIT_MESHTYPE_RECTILINEAR        0
#define VISIT_MESHTYPE_CURVILINEAR        1
#define VISIT_MESHTYPE_UNSTRUCTURED       2
#define VISIT_MESHTYPE_POINT              3
#define VISIT_MESHTYPE_SURFACE            4
#define VISIT_MESHTYPE_UNKNOWN            5

/* Centering */
#define VISIT_VARCENTERING_NODE           0
#define VISIT_VARCENTERING_ZONE           1

/* Variable Type */
#define VISIT_VARTYPE_MESH                0
#define VISIT_VARTYPE_SCALAR              1
#define VISIT_VARTYPE_VECTOR              2
#define VISIT_VARTYPE_TENSOR              3
#define VISIT_VARTYPE_SYMMETRIC_TENSOR    4
#define VISIT_VARTYPE_MATERIAL            5
#define VISIT_VARTYPE_MATSPECIES          6
#define VISIT_VARTYPE_CURVE               7
#define VISIT_VARTYPE_UNKNOWN             8

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

/* Array Owner */
#define VISIT_OWNER_SIM                   0
#define VISIT_OWNER_VISIT                 1

/* Cell Types */
#define VISIT_CELL_BEAM                   0
#define VISIT_CELL_TRI                    1
#define VISIT_CELL_QUAD                   2
#define VISIT_CELL_TET                    3
#define VISIT_CELL_PYR                    4
#define VISIT_CELL_WEDGE                  5
#define VISIT_CELL_HEX                    6
#define VISIT_CELL_POINT                  7

typedef struct VisIt_DataArray
{
    int         dataType; /* DATATYPE */
    int         owner;    /* OWNER */
    char       *cArray;
    int        *iArray;
    float      *fArray;
    double     *dArray;
} VisIt_DataArray;

typedef struct VisIt_SimulationControlCommand
{
    char       *name;
    int         argType;   /* CMDARG */
    int         enabled;   /* boolean */
    char       *signal;
    char       *className;
    char       *parent;
    char       *text;
    char       *value;
    char       *uiType;
    int         isOn;
} VisIt_SimulationControlCommand;

typedef struct VisIt_MeshMetaData
{
    const char *name;
 
    int         meshType;  /* MESHTYPE */
    int         topologicalDimension;
    int         spatialDimension;

    int         numBlocks;
    const char *blockTitle;
    const char *blockPieceName;

    int         numGroups;
    const char *groupTitle;
    const char *groupPieceName;
    int        *groupIds;

    const char *units;

    const char *xLabel;
    const char *yLabel;
    const char *zLabel;
} VisIt_MeshMetaData;

typedef struct VisIt_ScalarMetaData
{
    const char *name;
    const char *meshName;

    int         centering;    /* VARCENTERING */
    int         treatAsASCII; /* boolean */
} VisIt_ScalarMetaData;

typedef struct VisIt_MaterialMetaData
{
    const char  *name;
    const char  *meshName;
    int          numMaterials;
    const char **materialNames;
} VisIt_MaterialMetaData;

typedef struct VisIt_CurveMetaData
{
    const char *name;

    const char *xUnits;
    const char *yUnits;

    const char *xLabel;
    const char *yLabel;
} VisIt_CurveMetaData;

typedef struct VisIt_ExpressionMetaData
{
    const char *name;
    const char *definition;
    int         vartype;      /* VARTYPE */
} VisIt_ExpressionMetaData;

typedef struct VisIt_SimulationMetaData
{
    int      currentMode; /* SIMMODE */

    int      currentCycle;
    double   currentTime;

    int numMeshes;
    int numScalars;
    int numMaterials;
    int numCurves;
    int numExpressions;

    VisIt_MeshMetaData       *meshes;
    VisIt_ScalarMetaData     *scalars;
    VisIt_MaterialMetaData   *materials;
    VisIt_CurveMetaData      *curves;
    VisIt_ExpressionMetaData *expressions;

    int numGenericCommands;
    int numCustomCommands;

    VisIt_SimulationControlCommand *genericCommands;
    VisIt_SimulationControlCommand *customCommands;
} VisIt_SimulationMetaData;

typedef struct VisIt_CurvilinearMesh
{
    int ndims;
    int dims[3];
    int baseIndex[3];
    int minRealIndex[3];
    int maxRealIndex[3];
    VisIt_DataArray xcoords;
    VisIt_DataArray ycoords;
    VisIt_DataArray zcoords;
} VisIt_CurvilinearMesh;

typedef struct VisIt_RectilinearMesh
{
    int ndims;
    int dims[3];
    int baseIndex[3];
    int minRealIndex[3];
    int maxRealIndex[3];
    VisIt_DataArray xcoords;
    VisIt_DataArray ycoords;
    VisIt_DataArray zcoords;
} VisIt_RectilinearMesh;

typedef struct VisIt_UnstructuredMesh
{
    int ndims;
    int nnodes;
    int nzones;

    int firstRealZone;
    int lastRealZone;

    VisIt_DataArray xcoords;
    VisIt_DataArray ycoords;
    VisIt_DataArray zcoords;

    int connectivityLen;
    VisIt_DataArray connectivity;
} VisIt_UnstructuredMesh;

typedef struct VisIt_PointMesh
{
    int ndims;
    int nnodes;

    VisIt_DataArray xcoords;
    VisIt_DataArray ycoords;
    VisIt_DataArray zcoords;
} VisIt_PointMesh;

typedef struct VisIt_MeshData
{
    int                     meshType; /* MESHTYPE */
    VisIt_CurvilinearMesh  *cmesh;
    VisIt_RectilinearMesh  *rmesh;
    VisIt_UnstructuredMesh *umesh;
    VisIt_PointMesh        *pmesh;
} VisIt_MeshData;

typedef struct VisIt_ScalarData
{
    int len;
    VisIt_DataArray data;
} VisIt_ScalarData;

typedef struct VisIt_MixedScalarData
{
    int len;
    VisIt_DataArray data;
} VisIt_MixedScalarData;

typedef struct VisIt_MaterialData
{
    int nMaterials;
    int *materialNumbers;
    const char **materialNames;

    int  nzones;
    VisIt_DataArray matlist;

    int    mixlen;
    int   *mix_mat;
    int   *mix_zone;
    int   *mix_next;
    float *mix_vf;
} VisIt_MaterialData;

typedef struct VisIt_SpeciesData
{
    /* FILL IN */
    int dummy;
} VisIt_SpeciesData;

typedef struct VisIt_CurveData
{
    int len;
    VisIt_DataArray x;
    VisIt_DataArray y;
} VisIt_CurveData;

typedef struct VisIt_DomainList
{
    int nTotalDomains;
    int nMyDomains;
    VisIt_DataArray myDomains;
} VisIt_DomainList;

typedef struct
{
    /* Reader methods */
    VisIt_SimulationMetaData *(*GetMetaData)();
    VisIt_MeshData           *(*GetMesh)(int,const char*);
    VisIt_MaterialData       *(*GetMaterial)(int,const char*);
    VisIt_SpeciesData        *(*GetSpecies)(int,const char*);
    VisIt_ScalarData         *(*GetScalar)(int,const char*);
    VisIt_CurveData          *(*GetCurve)(const char*);
    VisIt_MixedScalarData    *(*GetMixedScalar)(int,const char*);
    VisIt_DomainList         *(*GetDomainList)();
} VisIt_SimulationCallback;

typedef struct
{
    /* Writer methods. */
    int (*WriteBegin)(const char *);
    int (*WriteEnd)(const char *);
    int (*WriteCurvilinearMesh)(const char *, int, VisIt_CurvilinearMesh*, const VisIt_MeshMetaData *);
    int (*WriteRectilinearMesh)(const char *, int, VisIt_RectilinearMesh*, const VisIt_MeshMetaData *);
    int (*WritePointMesh)(const char *, int, VisIt_PointMesh*, const VisIt_MeshMetaData *);
    int (*WriteUnstructuredMesh)(const char *, int, VisIt_UnstructuredMesh*, const VisIt_MeshMetaData *);
    int (*WriteDataArray)(const char *, const char *, int, int, void *, int, int, const VisIt_ScalarMetaData *);
} VisIt_SimulationWriterCallback;

/* Helper Methods */

static VisIt_DataArray VisIt_CreateDataArrayFromChar(int o, char *c)
{
    VisIt_DataArray da;
    da.dataType = VISIT_DATATYPE_CHAR;
    da.owner    = o;
    da.cArray   = c;
    return da;
}

static VisIt_DataArray VisIt_CreateDataArrayFromInt(int o, int *i)
{
    VisIt_DataArray da;
    da.dataType = VISIT_DATATYPE_INT;
    da.owner    = o;
    da.iArray   = i;
    return da;
}

static VisIt_DataArray VisIt_CreateDataArrayFromFloat(int o, float *f)
{
    VisIt_DataArray da;
    da.dataType = VISIT_DATATYPE_FLOAT;
    da.owner    = o;
    da.fArray   = f;
    return da;
}

static VisIt_DataArray VisIt_CreateDataArrayFromDouble(int o, double *d)
{
    VisIt_DataArray da;
    da.dataType = VISIT_DATATYPE_DOUBLE;
    da.owner    = o;
    da.dArray   = d;
    return da;
}

#ifdef __cplusplus
}
#endif

#endif
