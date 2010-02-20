/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/
/****************************************************************************
** File      : VisItDataInterface_V2.h
**
** Purpose:
**
** Programmer: Jeremy Meredith
** Creation:   March 17, 2005
**
** Modifications:
**   Brad Whitlock, Fri Feb  6 14:15:15 PST 2009
**   Increased to version 2 and extended and added structures.
**
*****************************************************************************/

#ifndef VISIT_DATA_INTERFACE_V2_H
#define VISIT_DATA_INTERFACE_V2_H
#include <VisItInterfaceTypes_V2.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct VisIt_DataArray
{
    int         dataType; /* DATATYPE */
    int         owner;    /* OWNER */
    char       *cArray;
    int        *iArray;
    float      *fArray;
    double     *dArray;
} VisIt_DataArray;

typedef struct VisIt_NameList
{
    int          numNames;
    char **names;
} VisIt_NameList;

/*****************************************************************************
*                            MetaData types
*****************************************************************************/

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
    char    *name;
 
    VisIt_MeshType meshType;
    int            topologicalDimension;
    int            spatialDimension;

    int            numBlocks;
    char          *blockTitle;
    char          *blockPieceName;
    char         **blockNames;

    int            numGroups;
    char          *groupTitle;
    char          *groupPieceName;
    int           *groupIds;

    char *units;

    char *xLabel;
    char *yLabel;
    char *zLabel;
} VisIt_MeshMetaData;

typedef struct VisIt_VariableMetaData
{
    char *name;
    char *meshName;

    VisIt_VarCentering centering;
    VisIt_VarType      type;
    int                treatAsASCII; /* boolean */
} VisIt_VariableMetaData;

typedef struct VisIt_MaterialMetaData
{
    char  *name;
    char  *meshName;
    int          numMaterials;
    char **materialNames;
} VisIt_MaterialMetaData;

typedef struct VisIt_CurveMetaData
{
    char *name;

    char *xUnits;
    char *yUnits;

    char *xLabel;
    char *yLabel;
} VisIt_CurveMetaData;

typedef struct VisIt_ExpressionMetaData
{
    char    *name;
    char    *definition;
    VisIt_VarType  vartype;
} VisIt_ExpressionMetaData;

typedef struct VisIt_SpeciesMetaData
{
    char *name;
    char *meshName;
    char *materialName;
    int             nmaterialSpecies;
    VisIt_NameList *materialSpeciesNames;
} VisIt_SpeciesMetaData;

typedef struct VisIt_SimulationMetaData
{
    int      currentMode; /* SIMMODE */

    int      currentCycle;
    double   currentTime;

    int numMeshes;
    int numVariables;
    int numMaterials;
    int numCurves;
    int numExpressions;
    int numSpecies;

    VisIt_MeshMetaData       *meshes;
    VisIt_VariableMetaData   *variables;
    VisIt_MaterialMetaData   *materials;
    VisIt_CurveMetaData      *curves;
    VisIt_ExpressionMetaData *expressions;
    VisIt_SpeciesMetaData    *species;

    int numGenericCommands;
    int numCustomCommands;

    VisIt_SimulationControlCommand *genericCommands;
    VisIt_SimulationControlCommand *customCommands;
} VisIt_SimulationMetaData;

/*****************************************************************************
*                         Problem-sized data types
*****************************************************************************/

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

typedef struct VisIt_CSGZonelist
{
    int             nregs;
    VisIt_DataArray typeflags;     /* [nregs] type info about each region */
    VisIt_DataArray leftids;       /* [nregs] left operand region refs */
    VisIt_DataArray rightids;      /* [nregs] right operand region refs */

    int             nzones;        /* number of zones */
    VisIt_DataArray zonelist;      /* [nzones] region ids (complete regions) */

    /* START - RESERVED FOR FUTURE USE */
    VisIt_DataArray xform;         /* [lxforms] transformation coefficients */
    int             lxform;        /* length of xforms array */
    /* END - RESERVED FOR FUTURE USE */
} VisIt_CSGZonelist;

typedef struct VisIt_CSGMesh
{
    int             nbounds;       /* Total number of boundaries */
    VisIt_DataArray typeflags;     /* int[nbounds] boundary type info flags */
    VisIt_DataArray coeffs;        /* coefficients in the representation of
                                      each boundary */
    int             lcoeffs;       /* length of coeffs array */

    double          min_extents[3];
    double          max_extents[3];

    VisIt_CSGZonelist zones;
} VisIt_CSGMesh;

typedef struct VisIt_MeshData
{
    VisIt_MeshType          meshType;
    VisIt_CurvilinearMesh  *cmesh;
    VisIt_RectilinearMesh  *rmesh;
    VisIt_UnstructuredMesh *umesh;
    VisIt_PointMesh        *pmesh;
    VisIt_CSGMesh          *csgmesh;
} VisIt_MeshData;

typedef struct VisIt_MaterialData
{
    int nMaterials;
    int *materialNumbers;
    char **materialNames;

    int  nzones;
    VisIt_DataArray matlist;

    int             mixlen;
    VisIt_DataArray mix_mat;  /* int[mixlen]   */
    VisIt_DataArray mix_zone; /* int[mixlen]   */
    VisIt_DataArray mix_next; /* int[mixlen]   */
    VisIt_DataArray mix_vf;   /* float[mixlen] */
} VisIt_MaterialData;

typedef struct VisIt_SpeciesData
{
    int              nmaterialSpecies;     /* length of materialSpecies array. */
    VisIt_DataArray  materialSpecies;      /* int[nmaterialSpecies] tells how many 
                                              species in each mat*/
    VisIt_NameList  *materialSpeciesNames; /* Optional array sized nmaterialSpecies 
                                              that contains name lists where each 
                                              list contains the species names for
                                              one material. If this member is set  
                                              to NULL then species will be numbered.
                                            */
    int              ndims;
    int              dims[3];
    int              nspeciesMF;
    VisIt_DataArray  speciesMF;
    VisIt_DataArray  species;
    int              nmixedSpecies;
    VisIt_DataArray  mixedSpecies;
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

#ifndef VISIT_DECORATE
#define VISIT_DECORATE(NAME) VisIt_##NAME
#endif
#include <VisIt_DomainBoundaries.h>
#include <VisIt_DomainNesting.h>

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

/****************************************************************************/

/* Some functions that operate on the declared data types... here is where
   we'd have the long list of various accessor functions for the opaque types
   that we're manipulating

   Eventually split these functions into different headers since there will
   be multiple functions that operate on each type. For example, there will
   be alloc and accessor functions.
*/
int VisIt_DataArray_free(VisIt_DataArray *obj);
int VisIt_NameList_free(VisIt_NameList *obj);
int VisIt_SimulationControlCommand_free(VisIt_SimulationControlCommand *obj);
int VisIt_MeshMetaData_free(VisIt_MeshMetaData *obj);
int VisIt_VariableMetaData_free(VisIt_VariableMetaData *obj);
int VisIt_MaterialMetaData_free(VisIt_MaterialMetaData *obj);
int VisIt_CurveMetaData_free(VisIt_CurveMetaData *obj);
int VisIt_ExpressionMetaData_free(VisIt_ExpressionMetaData *obj);
int VisIt_SpeciesMetaData_free(VisIt_SpeciesMetaData *obj);
int VisIt_SimulationMetaData_free(VisIt_SimulationMetaData *obj);

int VisIt_CurvilinearMesh_free(VisIt_CurvilinearMesh *obj);
int VisIt_RectilinearMesh_free(VisIt_RectilinearMesh *obj);
int VisIt_UnstructuredMesh_free(VisIt_UnstructuredMesh *obj);
int VisIt_PointMesh_free(VisIt_PointMesh *obj);
int VisIt_CSGMesh_free(VisIt_CSGMesh *obj);
int VisIt_MeshData_free(VisIt_MeshData *obj);
int VisIt_CurveData_free(VisIt_CurveData *obj);
int VisIt_MaterialData_free(VisIt_MaterialData *obj);
int VisIt_SpeciesData_free(VisIt_SpeciesData *obj);
int VisIt_DomainList_free(VisIt_DomainList *obj);

/****************************************************************************/

#include <VisIt_DomainBoundaries.h>
#include <VisIt_DomainNesting.h>
#include <VisIt_VariableData.h>

#ifdef __cplusplus
}
#endif

#endif
