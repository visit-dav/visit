/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

// ************************************************************************* //
//                            avtSimV2FileFormat.C                           //
// ************************************************************************* //

#include <avtSimV2FileFormat.h>

#include <map>
#include <string>
#include <vector>
#include <snprintf.h>

#include <avtDatabaseMetaData.h>
#include <avtGhostData.h>
#include <avtIOInformation.h>
#include <avtMixedVariable.h>
#include <avtSimulationInformation.h>
#include <avtSimulationCommandSpecification.h>
#include <avtStructuredDomainBoundaries.h>
#include <avtStructuredDomainNesting.h>
#include <avtSpecies.h>
#include <avtVariableCache.h>

#include <Expression.h>
#include <DebugStream.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>

#include <visitstream.h>
                                                    
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkIdTypeArray.h>
#include <vtkLongArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVisItUtility.h>

using std::string;
using std::vector;

#ifdef PARALLEL
#include <mpi.h>
#include <avtParallel.h>
#endif

#ifndef MDSERVER
#include <VisItControlInterfaceRuntime.h>
#include <VisItDataInterfaceRuntime.h>

// MetaData
#include <simv2_CommandMetaData.h>
#include <simv2_CurveMetaData.h>
#include <simv2_ExpressionMetaData.h>
#include <simv2_MaterialMetaData.h>
#include <simv2_MeshMetaData.h>
#include <simv2_NameList.h>
#include <simv2_SimulationMetaData.h>
#include <simv2_SpeciesMetaData.h>
#include <simv2_VariableMetaData.h>

// Data
#include <simv2_CurveData.h>
#include <simv2_DomainBoundaries.h>
#include <simv2_DomainList.h>
#include <simv2_DomainNesting.h>
#include <simv2_MaterialData.h>
#include <simv2_SpeciesData.h>
#include <simv2_VariableData.h>

#include <PolyhedralSplit.h>

vtkDataSet *SimV2_GetMesh_Curvilinear(visit_handle h);
vtkDataSet *SimV2_GetMesh_Rectilinear(visit_handle h);
vtkDataSet *SimV2_GetMesh_Unstructured(int, visit_handle h, PolyhedralSplit **);
vtkDataSet *SimV2_GetMesh_Point(visit_handle h);
vtkDataSet *SimV2_GetMesh_CSG(visit_handle h);

const char *AUXILIARY_DATA_POLYHEDRAL_SPLIT = "POLYHEDRAL_SPLIT";
#endif

// ****************************************************************************
//  Method: avtSimV2 constructor
//
//  Programmer: Brad Whitlock
//  Creation:   March 17, 2005
//
//  Modifications:
//
// ****************************************************************************

avtSimV2FileFormat::avtSimV2FileFormat(const char *filename)
    : avtSTMDFileFormat(&filename, 1)
{
#ifdef MDSERVER
    ifstream in(filename);

    simInfo.SetHost("");
    simInfo.SetPort(-1);
    simInfo.SetSecurityKey("");

    char buff[256];
    while (in >> buff)
    {
        if (strcmp(buff, "host")==0)
        {
            in >> buff;
            simInfo.SetHost(buff);
        }
        else if (strcmp(buff, "port")==0)
        {
            int port;
            in >> port;
            simInfo.SetPort(port);
        }
        else if (strcmp(buff, "key")==0)
        {
            in >> buff;
            simInfo.SetSecurityKey(buff);
        }
        else
        {
            in.get(); // assume one delimiter character

            char val[2048];
            in.getline(val, 2048);
            simInfo.GetOtherNames().push_back(buff);
            simInfo.GetOtherValues().push_back(val);
        }
    }

    in.close();

    if (simInfo.GetHost()=="")
    {
        EXCEPTION2(InvalidFilesException,filename,
                   "Did not find 'host' in the file.");
    }
    if (simInfo.GetSecurityKey()=="")
    {
        EXCEPTION2(InvalidFilesException,filename,
                   "Did not find 'key' in the file.");
    }
    if (simInfo.GetPort()==-1)
    {
        EXCEPTION2(InvalidFilesException,filename,
                   "Did not find 'port' in the file.");
    }
#endif
}

// ****************************************************************************
//  Method: avtSimV2FileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   August 11, 2004
//
// ****************************************************************************

void
avtSimV2FileFormat::FreeUpResources(void)
{
}

// ****************************************************************************
// Method: avtSimV2FileFormat::ActivateTimestep
//
// Purpose: 
//   Calls ActivateTimestep on all of the simulation processors.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 16 16:51:20 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
avtSimV2FileFormat::ActivateTimestep()
{
#ifndef MDSERVER
    simv2_invoke_ActivateTimestep();
#endif
}

#ifndef MDSERVER
// ****************************************************************************
// Method: AddMeshMetaData
//
// Purpose: 
//   Populates avtMeshMetaData using a handle to SimV2's MeshMetaData
//
// Arguments:
//   mHandle : A handle to a MeshMetaData object.
//
// Returns:    An initialized avtMeshMetaData object.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar  9 11:11:27 PST 2010
//
// Modifications:
//   Brad Whitlock, Tue Jun  8 11:51:09 PDT 2010
//   I fixed an error where the wrong mesh metadata was used for domain 
//   piece name.
//
//   Brad Whitlock, Thu Dec  2 14:41:13 PST 2010
//   I fixed a problem where a freed units string was being referenced.
//
//   Brad Whitlock, Wed Aug 17 12:20:43 PDT 2011
//   I added support for node/cell origin and spatial extents.
//
// ****************************************************************************

void
AddMeshMetaData(avtDatabaseMetaData *md, visit_handle h)
{
    avtMeshMetaData *mesh = 0;

    // Get the mesh type
    int meshType = VISIT_MESHTYPE_UNKNOWN;
    if(simv2_MeshMetaData_getMeshType(h, &meshType) == VISIT_OKAY)
    {
        // Get the mesh name
        char *meshName = NULL;
        if(simv2_MeshMetaData_getName(h, &meshName) == VISIT_OKAY)
        {
            int tdim = 0, sdim = 0;
            if(simv2_MeshMetaData_getTopologicalDimension(h, &tdim)
               == VISIT_OKAY &&
               simv2_MeshMetaData_getSpatialDimension(h, &sdim)
               == VISIT_OKAY)
            {
                mesh = new avtMeshMetaData;
                mesh->name = meshName;
                mesh->topologicalDimension = tdim;
                mesh->spatialDimension = sdim;
                mesh->hasSpatialExtents = false;

                switch (meshType)
                {
                case VISIT_MESHTYPE_RECTILINEAR:
                    mesh->meshType = AVT_RECTILINEAR_MESH;
                    break;
                case VISIT_MESHTYPE_CURVILINEAR:
                    mesh->meshType = AVT_CURVILINEAR_MESH;
                    break;
                case VISIT_MESHTYPE_UNSTRUCTURED:
                    mesh->meshType = AVT_UNSTRUCTURED_MESH;
                    break;
                case VISIT_MESHTYPE_POINT:
                    mesh->meshType = AVT_POINT_MESH;
                    break;
                case VISIT_MESHTYPE_CSG:
                    mesh->meshType = AVT_CSG_MESH;
                    break;
                case VISIT_MESHTYPE_AMR:
                    mesh->meshType = AVT_AMR_MESH;
                    break;
                default:
                    delete mesh;
                    simv2_FreeObject(h);
                    EXCEPTION1(ImproperUseException,
                               "Invalid mesh type in MeshMetaData.");
                    break;
                }
            }

            free(meshName);
        }
    }

    if(mesh == 0)
        return;

    int nDomains = 1;
    if(simv2_MeshMetaData_getNumDomains(h, &nDomains) == VISIT_OKAY)
        mesh->numBlocks = nDomains;

    char *domainTitle = NULL;
    if(simv2_MeshMetaData_getDomainTitle(h, &domainTitle) == VISIT_OKAY)
    {
        mesh->blockTitle = std::string(domainTitle);
        free(domainTitle);
    }

    char *domainPieceName = NULL;
    if(simv2_MeshMetaData_getDomainPieceName(h, &domainPieceName) == VISIT_OKAY)
    {
        mesh->blockPieceName = std::string(domainPieceName);
        free(domainPieceName);
    }

    int nDomainNames = 0;
    if(simv2_MeshMetaData_getNumDomainName(h, &nDomainNames) == VISIT_OKAY)
    {
        for(int i = 0; i < nDomainNames; ++i)
        {
            char *name = NULL;
            if(simv2_MeshMetaData_getDomainName(h, i, &name) == VISIT_OKAY)
            {
                mesh->blockNames.push_back(name);
                free(name);
            }
            else
                mesh->blockNames.push_back("");
        }
    }

    int nGroups = 1;
    if(simv2_MeshMetaData_getNumGroups(h, &nGroups) == VISIT_OKAY)
        mesh->numGroups = nGroups;

    char *groupTitle = NULL;
    if(nGroups > 0 &&
       simv2_MeshMetaData_getGroupTitle(h, &groupTitle) == VISIT_OKAY)
    {
        mesh->groupTitle = std::string(groupTitle);
        free(groupTitle);
    }

    char *groupPieceName = NULL;
    if(nGroups > 0 &&
       simv2_MeshMetaData_getGroupPieceName(h, &groupPieceName) == VISIT_OKAY)
    {
        mesh->groupPieceName = std::string(groupPieceName);
        free(groupPieceName);
    }
 
    int groupLen = (mesh->meshType == AVT_AMR_MESH) ? 
        mesh->numBlocks : mesh->numGroups;
    mesh->groupIds.resize(groupLen);
    for (int g = 0; g<groupLen; g++)
    {
        int id = 0;
        simv2_MeshMetaData_getGroupId(h, g, &id);
        mesh->groupIds[g] = id;
    }

    // Get axis labels
    char *xLabel = NULL, *yLabel = NULL, *zLabel = NULL;
    if(simv2_MeshMetaData_getXLabel(h, &xLabel) == VISIT_OKAY)
    {
        mesh->xLabel = std::string(xLabel);
        free(xLabel);
    }
    if(simv2_MeshMetaData_getYLabel(h, &yLabel) == VISIT_OKAY)
    {
        mesh->yLabel = std::string(yLabel);
        free(yLabel);
    }
    if(simv2_MeshMetaData_getZLabel(h, &zLabel) == VISIT_OKAY)
    {
        mesh->zLabel = std::string(zLabel);
        free(zLabel);
    }

    // Get axis units
    char *xUnits = NULL, *yUnits = NULL, *zUnits = NULL;
    if(simv2_MeshMetaData_getXUnits(h, &xUnits) == VISIT_OKAY)
    {
        mesh->xUnits = std::string(xUnits);
        free(xUnits);
    }
    if(simv2_MeshMetaData_getYUnits(h, &yUnits) == VISIT_OKAY)
    {
        mesh->yUnits = std::string(yUnits);
        free(yUnits);
    }
    if(simv2_MeshMetaData_getZUnits(h, &zUnits) == VISIT_OKAY)
    {
        mesh->zUnits = std::string(zUnits);
        free(zUnits);
    }

    // Get cell origin.
    int origin = 0;
    if(simv2_MeshMetaData_getCellOrigin(h, &origin) == VISIT_OKAY)
        mesh->cellOrigin = origin;

    // Get node origin.
    if(simv2_MeshMetaData_getNodeOrigin(h, &origin) == VISIT_OKAY)
        mesh->nodeOrigin = origin;

    // Get the spatial extents.
    int hasSE = 0;
    if(simv2_MeshMetaData_getHasSpatialExtents(h, &hasSE) == VISIT_OKAY)
    {
        double extents[6] = {0.,0.,0.,0.,0.,0.};
        if(hasSE > 0 &&
           simv2_MeshMetaData_getSpatialExtents(h, extents) == VISIT_OKAY)
        {
            mesh->hasSpatialExtents = true;
            mesh->minSpatialExtents[0] = extents[0];
            mesh->minSpatialExtents[1] = extents[2];
            mesh->minSpatialExtents[2] = extents[4];
            mesh->maxSpatialExtents[0] = extents[1];
            mesh->maxSpatialExtents[1] = extents[3];
            mesh->maxSpatialExtents[2] = extents[5];
        }
    }

    md->Add(mesh);
}

// ****************************************************************************
// Method: RestrictMaterialIndices
//
// Purpose: 
//   Check the input variable metadata for its materialName to see if any of
//   the material names are present in the material for the variable's mesh.
//   If so, return a vector of the indices of the material names.
//
// Arguments:
//   h : the metadata handle.
//   mmd : The material metadata.
//
// Returns:    A vector of indices.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 18 14:55:04 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

intVector
RestrictMaterialIndices(visit_handle h, const avtMaterialMetaData *mmd)
{
    intVector restrictedMats;

    if(mmd != NULL)
    {
        int nmat = 0;
        if(simv2_VariableMetaData_getNumMaterialName(h, &nmat) == VISIT_OKAY)
        {
            for(int m = 0; m < nmat; ++m)
            {
                char *matname = NULL;
                if(simv2_VariableMetaData_getMaterialName(h, m, &matname) == VISIT_OKAY)
                {
                    // Find index of matname in mmd's material names.
                    for(size_t idx = 0; idx < mmd->materialNames.size(); ++idx)
                    {
                        if(mmd->materialNames[idx] == std::string(matname))
                        {
                            restrictedMats.push_back(idx);
                            break;
                        }
                    }
                    free(matname);
                }
            }
        }
    }

    return restrictedMats;
}

// ****************************************************************************
// Method: AddVariableMetaData
//
// Purpose: 
//   Adds a variable metadata to the database metadata.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar  9 11:34:15 PST 2010
//
// Modifications:
//   Brad Whitlock, Thu Dec 15 14:51:15 PST 2011
//   Reenable array variables. Also add support for hideFromGUI attribute.
//
//   Brad Whitlock, Wed Jul 18 14:58:01 PDT 2012
//   Enable variables to be restricted to material subsets of the mesh.
//
// ****************************************************************************

void
AddVariableMetaData(avtDatabaseMetaData *md, visit_handle h)
{  
    char *name = NULL, *meshName = NULL;
    if(simv2_VariableMetaData_getName(h, &name) == VISIT_OKAY)
    {
        if(simv2_VariableMetaData_getMeshName(h, &meshName) == VISIT_OKAY)
        {
            int type, cntr;
            if(simv2_VariableMetaData_getType(h, &type) == VISIT_OKAY &&
               simv2_VariableMetaData_getCentering(h, &cntr) == VISIT_OKAY)
            {
                char *u = NULL;
                bool hasUnits = false;
                std::string units;
                if(simv2_VariableMetaData_getUnits(h, &u) == VISIT_OKAY)
                {
                    units = u;
                    hasUnits = !units.empty();
                    free(u);
                }

                avtCentering centering;
                if(cntr == VISIT_VARCENTERING_NODE)
                    centering = AVT_NODECENT;
                else
                    centering = AVT_ZONECENT;

                int iHideFromGUI = 0;
                bool hideFromGUI = false;
                if(simv2_VariableMetaData_getHideFromGUI(h, &iHideFromGUI) ==  VISIT_OKAY)
                    hideFromGUI = iHideFromGUI > 0;

                // See if the mesh for this variable has a material so we can 
                // potentially restrict this variable to certain material regions.
                const avtMaterialMetaData *mmd = NULL;
                TRY
                {
                    std::string matObjectName = md->MaterialOnMesh(meshName);
                    mmd = md->GetMaterial(matObjectName);
                }
                CATCHALL
                { 
                }
                ENDTRY

                // Create the appropriate metadata based on the variable type.
                if(type == VISIT_VARTYPE_SCALAR)
                {
                    int treatAsASCII = 0;
                    simv2_VariableMetaData_getTreatAsASCII(h, &treatAsASCII);

                    avtScalarMetaData *scalar = new avtScalarMetaData;
                    scalar->name = name;
                    scalar->originalName = name;
                    scalar->meshName = meshName;
                    scalar->centering = centering;
                    scalar->treatAsASCII = treatAsASCII != 0;
                    scalar->hasDataExtents = false;
                    scalar->units = units;
                    scalar->hasUnits = hasUnits;
                    scalar->hideFromGUI = hideFromGUI;
                    scalar->matRestricted = RestrictMaterialIndices(h, mmd);

                    md->Add(scalar);
                }
                else if(type == VISIT_VARTYPE_VECTOR)
                {
                    avtVectorMetaData *vector = new avtVectorMetaData;
                    vector->name = name;
                    vector->originalName = name;
                    vector->meshName = meshName;
                    vector->centering = centering;
                    vector->units = units;
                    vector->hasUnits = hasUnits;
                    vector->hideFromGUI = hideFromGUI;
                    vector->matRestricted = RestrictMaterialIndices(h, mmd);

                    md->Add(vector);
                }
                else if(type == VISIT_VARTYPE_TENSOR)
                {
                    avtTensorMetaData *tensor = new avtTensorMetaData;
                    tensor->name = name;
                    tensor->originalName = name;
                    tensor->meshName = meshName;
                    tensor->centering = centering;
                    tensor->units = units;
                    tensor->hasUnits = hasUnits;
                    tensor->hideFromGUI = hideFromGUI;
                    tensor->matRestricted = RestrictMaterialIndices(h, mmd);

                    md->Add(tensor);
                }
                else if(type == VISIT_VARTYPE_SYMMETRIC_TENSOR)
                {
                    avtSymmetricTensorMetaData *tensor = new avtSymmetricTensorMetaData;
                    tensor->name = name;
                    tensor->originalName = name;
                    tensor->meshName = meshName;
                    tensor->centering = centering;
                    tensor->units = units;
                    tensor->hasUnits = hasUnits;
                    tensor->hideFromGUI = hideFromGUI;
                    tensor->matRestricted = RestrictMaterialIndices(h, mmd);

                    md->Add(tensor);
                }
                else if(type == VISIT_VARTYPE_LABEL)
                {
                    avtLabelMetaData *label = new avtLabelMetaData;
                    label->name = name;
                    label->originalName = name;
                    label->meshName = meshName;
                    label->centering = centering;
                    label->hideFromGUI = hideFromGUI;
                    label->matRestricted = RestrictMaterialIndices(h, mmd);

                    md->Add(label);
                }
                else if(type == VISIT_VARTYPE_ARRAY)
                {
                    int nComponents = 1;
                    simv2_VariableMetaData_getNumComponents(h, &nComponents);

                    avtArrayMetaData *arr = new avtArrayMetaData;
                    arr->name = name;
                    arr->originalName = name;
                    arr->meshName = meshName;
                    arr->centering = centering;
                    arr->nVars = nComponents;
                    for(int c = 0; c < nComponents; ++c)
                    { 
                        char compname[100];
                        sprintf(compname, "%d", c);
                        arr->compNames.push_back(compname);
                    }
                    arr->units = units;
                    arr->hasUnits = hasUnits;
                    arr->hideFromGUI = hideFromGUI;
                    arr->matRestricted = RestrictMaterialIndices(h, mmd);

                    md->Add(arr);
                }
            }
            free(meshName);
        }
        free(name);
    }
}

// ****************************************************************************
// Method: AddMaterialMetaData
//
// Purpose: 
//   Add metadata for a material
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar  9 12:13:56 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
AddMaterialMetaData(avtDatabaseMetaData *md, visit_handle h)
{
    char *name = NULL, *meshName = NULL;
    if(simv2_MaterialMetaData_getName(h, &name) == VISIT_OKAY)
    {
        if(simv2_MaterialMetaData_getMeshName(h, &meshName) == VISIT_OKAY)
        {
            int nMat = 0;
            if(simv2_MaterialMetaData_getNumMaterialName(h, &nMat) == VISIT_OKAY)
            {
                avtMaterialMetaData *material = new avtMaterialMetaData;
                material->name = name;
                material->originalName = name;
                material->meshName = meshName;
                material->numMaterials = nMat;
                material->materialNames.clear();

                for (int m = 0; m < nMat; m++)
                {
                    char *matname = NULL;
                    if(simv2_MaterialMetaData_getMaterialName(h, m, &matname) == VISIT_OKAY)
                    {
                        material->materialNames.push_back(matname);
                        free(matname);
                    }
                    else
                        material->materialNames.push_back("");
                }

                md->Add(material);
            }
            free(meshName);
        }
        free(name);
    }
}

// ****************************************************************************
// Method: NameListToStringVector
//
// Purpose: 
//   Converts a name list to a string vector.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar  9 13:06:44 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
NameListToStringVector(visit_handle h, stringVector &vec)
{
    int n;
    if(simv2_NameList_getNumName(h, &n) == VISIT_OKAY)
    {
        for(int i = 0; i < n; ++i)
        {
            char *name = NULL;
            if(simv2_NameList_getName(h, i, &name) == VISIT_OKAY)
            {
                vec.push_back(name);
                free(name);
            }
        }
    }
}

// ****************************************************************************
// Method: AddSpeciesMetaData
//
// Purpose: 
//   Adds species metadata.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar  9 13:07:12 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
AddSpeciesMetaData(avtDatabaseMetaData *md, visit_handle h)
{
    char *name = NULL, *meshName = NULL, *matName = NULL;
    if(simv2_SpeciesMetaData_getName(h, &name) == VISIT_OKAY)
    {
        if(simv2_SpeciesMetaData_getMeshName(h, &meshName) == VISIT_OKAY)
        {
            if(simv2_SpeciesMetaData_getMaterialName(h, &matName) == VISIT_OKAY)
            {
                int nSpecies = 0;
                if(simv2_SpeciesMetaData_getNumSpeciesName(h, &nSpecies) == VISIT_OKAY)
                {
                    vector<int>   numSpecies;
                    vector<vector<string> > speciesNames;
                    for(int i = 0; i < nSpecies; ++i)
                    {
                        visit_handle s;
                        if(simv2_SpeciesMetaData_getSpeciesName(h, i, &s) == VISIT_OKAY)
                        {
                            stringVector onelist;
                            NameListToStringVector(s, onelist);

                            numSpecies.push_back(static_cast<int>(onelist.size()));
                            speciesNames.push_back(onelist);
                        }
                    }

                    avtSpeciesMetaData *species = new avtSpeciesMetaData(name,
                        meshName, matName, static_cast<int>(numSpecies.size()), numSpecies, 
                        speciesNames);

                    md->Add(species);
                }            
                free(matName);
            }
            free(meshName);
        }
        free(name);
    }
}

// ****************************************************************************
// Method: AddCurveMetaData
//
// Purpose: 
//   Adds curve metadata.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar  9 13:22:56 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
AddCurveMetaData(avtDatabaseMetaData *md, visit_handle h)
{
    char *name = NULL;
    if(simv2_CurveMetaData_getName(h, &name) == VISIT_OKAY)
    {
        avtCurveMetaData *curve = new avtCurveMetaData;
        curve->name = name;
        curve->originalName = name;
        free(name);

        // Get axis labels
        char *xLabel = NULL, *yLabel = NULL;
        if(simv2_CurveMetaData_getXLabel(h, &xLabel) == VISIT_OKAY)
        {
            curve->xLabel = xLabel;
            free(xLabel);
        }
        if(simv2_CurveMetaData_getYLabel(h, &yLabel) == VISIT_OKAY)
        {
            curve->yLabel = yLabel;
            free(yLabel);
        }

        // Get axis units
        char *xUnits = NULL, *yUnits = NULL;
        if(simv2_CurveMetaData_getXUnits(h, &xUnits) == VISIT_OKAY)
        {
            curve->xUnits = xUnits;
            free(xUnits);
        }
        if(simv2_CurveMetaData_getYUnits(h, &yUnits) == VISIT_OKAY)
        {
            curve->yUnits = yUnits;
            free(yUnits);
        }

        md->Add(curve);
    }
}

// ****************************************************************************
// Method: AddExpressionMetaData
//
// Purpose: 
//   
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar  9 13:28:23 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
AddExpressionMetaData(avtDatabaseMetaData *md, visit_handle h)
{
    char *name = NULL, *definition = NULL;
    if(simv2_ExpressionMetaData_getName(h, &name) == VISIT_OKAY)
    {
        if(simv2_ExpressionMetaData_getDefinition(h, &definition) == VISIT_OKAY)
        {
            int vartype;
            if(simv2_ExpressionMetaData_getType(h, &vartype) == VISIT_OKAY)
            {
                Expression *newexp = new Expression;
                newexp->SetName(name);
                newexp->SetDefinition(definition);
                if(vartype == VISIT_VARTYPE_MESH)
                    newexp->SetType(Expression::Mesh);
                else if(vartype == VISIT_VARTYPE_SCALAR)
                    newexp->SetType(Expression::ScalarMeshVar);
                else if(vartype == VISIT_VARTYPE_VECTOR)
                    newexp->SetType(Expression::VectorMeshVar);
                else if(vartype == VISIT_VARTYPE_TENSOR)
                    newexp->SetType(Expression::TensorMeshVar);
                else if(vartype == VISIT_VARTYPE_SYMMETRIC_TENSOR)
                    newexp->SetType(Expression::SymmetricTensorMeshVar);
                else if(vartype == VISIT_VARTYPE_MATERIAL)
                    newexp->SetType(Expression::Material);
                else if(vartype == VISIT_VARTYPE_MATSPECIES)
                    newexp->SetType(Expression::Species);
                else if(vartype == VISIT_VARTYPE_CURVE)
                    newexp->SetType(Expression::CurveMeshVar);
                else
                    newexp->SetType(Expression::Unknown);

                md->AddExpression(newexp);
            }
            free(definition);
        }
        free(name);
    }
}

// ****************************************************************************
// Method: CommandMetaDataToCommandSpec
//
// Purpose: 
//   Populates avtSimulationCommandSpecification from CommandMetaData.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar  9 13:46:29 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
CommandMetaDataToCommandSpec(visit_handle h, avtSimulationCommandSpecification &scs)
{
    char *name = NULL;
    if(simv2_CommandMetaData_getName(h, &name) == VISIT_OKAY)
    {
        scs.SetName(name);
        scs.SetArgumentType(avtSimulationCommandSpecification::CmdArgNone);
        free(name);
    }
}
#endif

// ****************************************************************************
//  Method: avtSimV2FileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Fill the simulation metadata with the parameters from the file for
//      the mdserver.  Get the info from the simulation for the engine.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Mar  9 10:31:56 PST 2010
//
//  Modifications:
//
//    Mark C. Miller, Tue Oct 19 20:23:57 PDT 2010
//    Added test of debug level before calling MD's Print() method.
// ****************************************************************************

void
avtSimV2FileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    md->SetIsSimulation(true);
#ifdef MDSERVER
    md->SetSimInfo(simInfo);
#else
    const char *mName = "avtSimV2FileFormat::PopulateDatabaseMetaData: ";

    visit_handle h = simv2_invoke_GetMetaData();
    if(h == VISIT_INVALID_HANDLE)
        return;

    int simMode, currentCycle;
    double currentTime;
    if(simv2_SimulationMetaData_getData(h, simMode, currentCycle, 
        currentTime) == VISIT_ERROR)
    {
        debug4 << mName << "Can't get data from simulation metadata" << endl;
        simv2_FreeObject(h);
        return;
    }

    md->SetCycle(timestep, currentCycle);
    md->SetTime(timestep, currentTime);

    switch(simMode)
    {
    case VISIT_SIMMODE_UNKNOWN:
        md->GetSimInfo().SetMode(avtSimulationInformation::Unknown);
        break;
    case VISIT_SIMMODE_RUNNING:
        md->GetSimInfo().SetMode(avtSimulationInformation::Running);
        break;
    case VISIT_SIMMODE_STOPPED:
        md->GetSimInfo().SetMode(avtSimulationInformation::Stopped);
        break;
    }

    //
    // Add the meshes.
    //
    int numMeshes = 0;
    simv2_SimulationMetaData_getNumMeshes(h, numMeshes);
    for(int m = 0; m < numMeshes; m++)
    {
        visit_handle mHandle = VISIT_INVALID_HANDLE;
        if(simv2_SimulationMetaData_getMesh(h, m, mHandle) == VISIT_OKAY)
            AddMeshMetaData(md, mHandle);
    }

    //
    // Add the Curves
    // 
    int numCurves = 0;
    simv2_SimulationMetaData_getNumCurves(h, numCurves);
    for (int i=0; i < numCurves; i++)
    {
        visit_handle iHandle = VISIT_INVALID_HANDLE;
        if(simv2_SimulationMetaData_getCurve(h, i, iHandle) == VISIT_OKAY)
            AddCurveMetaData(md, iHandle);
    }
    for(int i = 0; i < md->GetNumCurves(); ++i)
        curveMeshes.insert(md->GetCurve(i)->name);

    //
    // Add the materials
    // 
    int numMaterials = 0;
    simv2_SimulationMetaData_getNumMaterials(h, numMaterials);
    for (int i=0; i < numMaterials; i++)
    {
        visit_handle iHandle = VISIT_INVALID_HANDLE;
        if(simv2_SimulationMetaData_getMaterial(h, i, iHandle) == VISIT_OKAY)
            AddMaterialMetaData(md, iHandle);
    }

    //
    // Add the Species
    // 
    int numSpecies = 0;
    simv2_SimulationMetaData_getNumSpecies(h, numSpecies);
    for (int i=0; i < numSpecies; i++)
    {
        visit_handle iHandle = VISIT_INVALID_HANDLE;
        if(simv2_SimulationMetaData_getSpecies(h, i, iHandle) == VISIT_OKAY)
            AddSpeciesMetaData(md, iHandle);
    }
     
    //
    // Add the variables
    //
    int numVariables = 0;
    simv2_SimulationMetaData_getNumVariables(h, numVariables);
    for (int s=0; s < numVariables; s++)
    {
        visit_handle vHandle = VISIT_INVALID_HANDLE;
        if(simv2_SimulationMetaData_getVariable(h, s, vHandle) == VISIT_OKAY)
            AddVariableMetaData(md, vHandle);
    }

    //
    // Add the Expressions
    // 
    int numExpressions = 0;
    simv2_SimulationMetaData_getNumExpressions(h, numExpressions);
    for (int i=0; i < numExpressions; i++)
    {
        visit_handle iHandle = VISIT_INVALID_HANDLE;
        if(simv2_SimulationMetaData_getExpression(h, i, iHandle) == VISIT_OKAY)
            AddExpressionMetaData(md, iHandle);
    }

    //
    // Add simulation commands.
    //
    int numCommands = 0;
    simv2_SimulationMetaData_getNumGenericCommands(h, numCommands);
    for (int i=0; i < numCommands; i++)
    {
        visit_handle cHandle = VISIT_INVALID_HANDLE;
        if(simv2_SimulationMetaData_getGenericCommand(h, i, cHandle) == VISIT_OKAY)
        {
            avtSimulationCommandSpecification spec;
            CommandMetaDataToCommandSpec(cHandle, spec);
            md->GetSimInfo().AddGenericCommands(spec);
        }
    }

    simv2_SimulationMetaData_getNumCustomCommands(h, numCommands);
    for (int i=0; i < numCommands; i++)
    {
        visit_handle cHandle = VISIT_INVALID_HANDLE;
        if(simv2_SimulationMetaData_getCustomCommand(h, i, cHandle) == VISIT_OKAY)
        {
            avtSimulationCommandSpecification spec;
            CommandMetaDataToCommandSpec(cHandle, spec);
            md->GetSimInfo().AddCustomCommands(spec);
        }
    }

    // Get domain boundary information
    int numMultiblock = 0;
    for (int m=0; m < md->GetNumMeshes(); m++)
    {
        if(md->GetMesh(m)->numBlocks > 1)
            numMultiblock++;
    }
    if(numMultiblock > 0)
    {
        for (int i=0; i < md->GetNumMeshes(); i++)
        {
            if(md->GetMesh(i)->numBlocks > 1)
            {
                debug4 << mName << "Getting domain boundaries for mesh: " << md->GetMesh(i)->name << endl;
                visit_handle boundaries = simv2_invoke_GetDomainBoundaries(md->GetMesh(i)->name.c_str());
                if(boundaries != VISIT_INVALID_HANDLE)
                {
                    // Cache the domain boundary information
                    avtStructuredDomainBoundaries *sdb = (avtStructuredDomainBoundaries *)
                        simv2_DomainBoundaries_avt(boundaries);
                    if(sdb != NULL)
                    {
                        sdb->CalculateBoundaries();
                        void_ref_ptr vr = void_ref_ptr(sdb,avtStructuredDomainBoundaries::Destruct);
                        const char *cache_meshname = (numMultiblock > 1) ? md->GetMesh(i)->name.c_str() : "any_mesh";
                        debug4 << mName << "Caching domain boundaries for mesh:" << cache_meshname << endl;
                        cache->CacheVoidRef(cache_meshname,
                                AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION,
                                timestep, -1, vr);
                    }
                    else
                    {
                        debug4 << mName << "Could not obtain domain boundaries from returned "
                                           "simulation object."
                               << endl;
                    }
                    simv2_DomainBoundaries_free(boundaries);
                }
                else
                {
                    debug4 << mName << "The simulation did not return a valid "
                                       "domain boundaries object for mesh "
                           << md->GetMesh(i)->name << endl;
                }
            }
        }
    }

    // Get domain nesting information
    int numAMR = 0;
    for (int m=0; m < md->GetNumMeshes(); m++)
    {
        if(md->GetMesh(m)->meshType == AVT_AMR_MESH)
            numAMR++;
    }
    if(numAMR > 0)
    {
        for (int i=0; i < md->GetNumMeshes(); i++)
        {
            if(md->GetMesh(i)->meshType == AVT_AMR_MESH)
            {
                debug4 << mName << "Getting domain nesting for mesh: " << md->GetMesh(i)->name << endl;
                visit_handle nesting = simv2_invoke_GetDomainNesting(md->GetMesh(i)->name.c_str());
                if(nesting != VISIT_INVALID_HANDLE)
                {
                    avtStructuredDomainNesting *sdn = (avtStructuredDomainNesting *)
                        simv2_DomainNesting_avt(nesting);
                    if(sdn != NULL)
                    {
                        void_ref_ptr vr = void_ref_ptr(sdn, avtStructuredDomainNesting::Destruct);
                        const char *cache_meshname = (numAMR > 1) ? md->GetMesh(i)->name.c_str() : "any_mesh";
                        debug4 << mName << "Caching domain nesting for mesh:" << cache_meshname << endl;
                        cache->CacheVoidRef(cache_meshname,
                                            AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION,
                                            timestep, -1, vr);
                    }
                    else
                    {
                        debug4 << mName << "Could not obtain domain nesting from returned "
                                           "simulation object."
                               << endl;
                    }
                    simv2_DomainNesting_free(nesting);
                }
                else
                {
                    debug4 << mName << "The simulation did not return a valid "
                                       "domain nesting object for mesh "
                           << md->GetMesh(i)->name << endl;
                }
            }
        }
    }

    if (DebugStream::Level4())
        md->Print(DebugStream::Stream4());

    simv2_SimulationMetaData_free(h);
#endif
}

// ****************************************************************************
//  Method: avtSimV2FileFormat::GetMesh
//
//  Purpose:
//      For the mdserver, do nothing.
//      For the engine, return a C-api mesh converted to a vtkDataSet.
//
//  Arguments:
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Feb  5 11:50:58 PST 2009
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtSimV2FileFormat::GetMesh(int domain, const char *meshname)
{
#ifdef MDSERVER
    return NULL;
#else

    if (curveMeshes.count(meshname))
    {
        return GetCurve(meshname);
    }

    visit_handle h = simv2_invoke_GetMesh(domain, meshname);

    // If the mesh could not be created then return.
    if(h == VISIT_INVALID_HANDLE)
        return NULL;

    vtkDataSet *ds = 0;
    PolyhedralSplit *phSplit = 0;
    TRY
    {
        int objType = simv2_ObjectType(h);
        switch (objType)
        {
        case VISIT_CURVILINEAR_MESH:
            ds = SimV2_GetMesh_Curvilinear(h);
            break;
        case VISIT_RECTILINEAR_MESH:
            ds = SimV2_GetMesh_Rectilinear(h);
            break;
        case VISIT_UNSTRUCTURED_MESH:
        {
            ds = SimV2_GetMesh_Unstructured(domain, h, &phSplit);

            // Cache the polyhedral split object in case we need it for
            // variables later.
            if(phSplit != 0)
            {
                void_ref_ptr vr = void_ref_ptr(phSplit, PolyhedralSplit::Destruct);
                debug4 << "Caching polyhedral split for mesh:" << meshname << endl;
                cache->CacheVoidRef(meshname,
                                    AUXILIARY_DATA_POLYHEDRAL_SPLIT,
                                    timestep, domain, vr);
            }
        }
            break;
        case VISIT_POINT_MESH:
            ds = SimV2_GetMesh_Point(h);
            break;
        case VISIT_CSG_MESH:
            ds = SimV2_GetMesh_CSG(h);
            break;
        default:       
            EXCEPTION1(ImproperUseException,
                "The simulation returned a handle that does not correspond "
                "to a mesh.\n");
            break;
        }
    }
    CATCH(VisItException)
    {
        delete phSplit;
        simv2_FreeObject(h);
        RETHROW;
    }
    ENDTRY

    simv2_FreeObject(h);

    return ds;
#endif
}

#ifndef MDSERVER
// ****************************************************************************
// Method: StoreVariableData
//
// Purpose: 
//   Store memory into the new vtkDataArray, taking 2-tuple to 3-tuple conversion
//   into account.
//
// Arguments:
//   array       : The destination vtkDataArray.
//   src         : The source data array.
//   nComponents : The number of components in the data.
//   nTuples     : The numebr of tuples in the data.
//   owner       : The owner of the array.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 27 11:12:21 PST 2009
//
// Modifications:
//   Brad Whitlock, Tue Jan 18 00:14:15 PST 2011
//   I added code to use the array from the simulation directly.
//
// ****************************************************************************

template <class ARR, class T>
void
StoreVariableData(ARR *array, T *src, int nComponents, int nTuples, int owner)
{
    if(nComponents == 2)
    {
        // Reorder 2-tuple vector data into 3-tuple vector data because VisIt
        // can't deal with 2-tuple vectors appropriately.
        array->SetNumberOfComponents(3);
        array->SetNumberOfTuples(nTuples);
        T *dest = (T *)array->GetVoidPointer(0);
        const T *s = src;
        const T *end = src + (nTuples * 2);
        while(s < end)
        {
            *dest++ = *s++;
            *dest++ = *s++;
            *dest++ = (T)0;
        }
    }
    else
    {
        array->SetNumberOfComponents(nComponents);
#define USE_SET_ARRAY
#ifdef USE_SET_ARRAY
        // Use the raw data provided by the sim.
        int saveArray = (owner == VISIT_OWNER_SIM) ? 1 : 0;
        array->SetArray(src, nComponents * nTuples, saveArray);
#else
        // Here we copy the data.
        array->SetNumberOfTuples(nTuples);
        memcpy(array->GetVoidPointer(0), src, 
               sizeof(T) * nTuples * nComponents);
#endif
    }
}
#endif

// ****************************************************************************
// Method: avtSimV2FileFormat::GetRestrictedMaterialIndices
//
// Purpose: 
//   Get the material indices for a variable.
//
// Arguments:
//   varname : The name of the variable for which we want material indices.
//
// Returns:    An intVector of the material indices.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 18 11:55:38 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

intVector
avtSimV2FileFormat::GetRestrictedMaterialIndices(const std::string &varname) const
{
    avtVarType varType = metadata->DetermineVarType(varname, false);
    if(varType == AVT_SCALAR_VAR)
        return metadata->GetScalar(varname)->matRestricted;
    else if(varType == AVT_VECTOR_VAR)
        return metadata->GetVector(varname)->matRestricted;
    else if(varType == AVT_TENSOR_VAR)
        return metadata->GetTensor(varname)->matRestricted;
    else if(varType == AVT_SYMMETRIC_TENSOR_VAR)
        return metadata->GetSymmTensor(varname)->matRestricted;
    else if(varType == AVT_ARRAY_VAR)
        return metadata->GetArray(varname)->matRestricted;
    else if(varType == AVT_LABEL_VAR)
        return metadata->GetLabel(varname)->matRestricted;
    return intVector();
}

// ****************************************************************************
// Method: avtSimV2FileFormat::GetCentering
//
// Purpose: 
//   Get the centering of the specified variable.
//
// Arguments:
//   varname : The name of the variable for which we want the centering.
//
// Returns:    The variable centering.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 18 11:56:34 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

avtCentering
avtSimV2FileFormat::GetCentering(const std::string &varname) const
{
    // Get the variable centering
    avtVarType varType = metadata->DetermineVarType(varname, false);
    avtCentering centering = AVT_ZONECENT;
    if(varType == AVT_SCALAR_VAR)
        centering = metadata->GetScalar(varname)->centering;
    else if(varType == AVT_VECTOR_VAR)
        centering = metadata->GetVector(varname)->centering;
    else if(varType == AVT_TENSOR_VAR)
        centering = metadata->GetTensor(varname)->centering;
    else if(varType == AVT_SYMMETRIC_TENSOR_VAR)
        centering = metadata->GetSymmTensor(varname)->centering;
    else if(varType == AVT_ARRAY_VAR)
        centering = metadata->GetArray(varname)->centering;
    else if(varType == AVT_LABEL_VAR)
        centering = metadata->GetLabel(varname)->centering;

    return centering;
}

// ****************************************************************************
// Method: avtSimV2FileFormat::ExpandVariable
//
// Purpose: 
//   Expand a material-restricted variable up to the whole mesh size.
//
// Arguments:
//   array          : The input variable.
//   mv             : The input mixed variable.
//   domain         : The domain.
//   varname        : The name of the variable.
//   restrictToMats : The vector of material indices.
//
// Returns:    
//
// Note:       This code is inspired by TraverseMaterialForSubsettedUcdvar in
//             avtSiloFileFormat.C
//
//             If we can't get the material or mesh we just return the input
//             array with an increased refcount.
//
//             Ideally, this kind of operation would be up in the database.
//
//             The reason that the restrictToMats values work with the matnos
//             in the avtMaterial is that the avtMaterial renumbers all materials
//             to start with 0.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 18 11:29:46 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

vtkDataArray *
avtSimV2FileFormat::ExpandVariable(vtkDataArray *array, avtMixedVariable **mv, 
    int nMixVarComponents, int domain, const std::string &varname, 
    const intVector &restrictToMats)
{
#ifdef MDSERVER
    return NULL;
#else
    const char *mName = "avtSimV2FileFormat::ExpandVariable: ";

    // Obtain the avtMaterial object associated with the mesh this
    // variable is defined on.
    string meshName = metadata->MeshForVar(varname);
    string matObjectName = metadata->MaterialOnMesh(meshName);
    avtMaterial *mat = NULL;

    // First, see if material object is already cached
    void_ref_ptr vr = cache->GetVoidRef(matObjectName.c_str(),
        AUXILIARY_DATA_MATERIAL, timestep, domain);
    mat = (avtMaterial*) *vr;

    // If we don't have the material, read it and cache it
    if (mat == NULL)
    {
        DestructorFunction df;
        void *p = GetAuxiliaryData(matObjectName.c_str(), domain,
            AUXILIARY_DATA_MATERIAL, (void*)0, df);
        mat = (avtMaterial*) p;
        void_ref_ptr vr = void_ref_ptr(p, df);
        cache->CacheVoidRef(matObjectName.c_str(), AUXILIARY_DATA_MATERIAL,
            this->timestep, domain, vr);
    }

    if(mat == NULL)
    {
        debug1 << mName << "Unable to obtain material object \"" << matObjectName
               << "\" required to compute subsets upon which the variable \""
               << varname << "\" is defined." << endl;
        array->Register(NULL);
        return array;
    }

    // If we have a node-centered variable then we need the dataset.
    avtCentering centering = GetCentering(varname);
    vtkDataSet *mesh = NULL;
    if(centering == AVT_NODECENT)
    {
        mesh = (vtkDataSet *) cache->GetVTKObject(meshName.c_str(),
                                                  avtVariableCache::DATASET_NAME,
                                                  this->timestep, domain, "_all");
        if (mesh == NULL)
        {
            debug1 << mName << "Cannot find cached mesh \"" << meshName
                   << "\" for domain %d to traverse subsetted node-centered variable, \""
                   << varname << "\"" << endl;
            array->Register(NULL);
            return array;
        }
    }

    // Create a new mesh-sized data array filled in with values from the 
    // first tuple. We initialize it thus because the code to copy the data over
    // from the array that we're expanding pokes values back into this array.
    // This initialization ensures that all values are initialized.
    int i, nZones = mat->GetNZones();
    vtkIdType nTuples = (centering == AVT_NODECENT) ? mesh->GetNumberOfPoints() : nZones;
    vtkDataArray *newvals = array->NewInstance();
    newvals->SetNumberOfComponents(array->GetNumberOfComponents());
    newvals->SetNumberOfTuples(nTuples);
    for(vtkIdType id = 0; id < nTuples; ++id)
        newvals->SetTuple(id, array->GetTuple(0));
    debug1 << mName << "New array has " << nTuples << " tuples and we've initialized them all to: " 
           << array->GetTuple1(0) << endl;

    // Create new mixvals array if we have zonal data and a mixed variable.
    const float *mixvals = NULL;
    int newmixlen = 0;
    float *newmixvals = NULL;
    if(*mv != NULL && centering == AVT_ZONECENT)
        mixvals = (*mv)->GetBuffer();
    if(mixvals != NULL)
    {
        newmixlen = array->GetNumberOfComponents() * mat->GetMixlen();
        newmixvals = new float[newmixlen];
        for(i = 0; i < newmixlen; ++i)
            newmixvals[i] = mixvals[0];
    }

    //
    // Map values back into the mesh sized data array. Copy over mixvals too.
    //
    debug1 << mName << "Iterating over " << nZones << " zones" << endl;
    std::map<vtkIdType,bool> haveVisitedPoint;
    int nvals = 0;
    int nmixvals = 0;
    for(i = 0; i < nZones; ++i)
    {
        int matno = mat->GetMatlist()[i];

        // Is this zone selected by restrictToMats?
        bool selected = false;
        if(matno >= 0) // clean case
        {
            // Is matno in our restrictToMats?
            for (size_t j = 0; j < restrictToMats.size() && !selected; j++)
                selected |= (matno == restrictToMats[j]);
        }
        else // mixed cell
        {
            // Are any matnos in our restrictToMats?
            int mix_idx = -(matno) - 1;
            while(mix_idx >= 0 && !selected)
            {
                for(size_t j = 0; j < restrictToMats.size() && !selected; j++)
                    selected |= (mat->GetMixMat()[mix_idx] == restrictToMats[j]);
                mix_idx = mat->GetMixNext()[mix_idx]-1;
            }
        }

        if(selected)
        {
            if(centering == AVT_NODECENT)
            {
                vtkCell *cell = mesh->GetCell(i);
                for (vtkIdType j = 0; j < cell->GetNumberOfPoints(); j++)
                {
                    vtkIdType ptId = cell->GetPointId(j);
                    if (haveVisitedPoint.find(ptId) == haveVisitedPoint.end())
                    {
                        haveVisitedPoint[ptId] = true;
                        // Copy current source value into the value for this node.
                        newvals->SetTuple(ptId, array->GetTuple(nvals));
                        nvals++;
                    }
                }
            }
            else
            {
                // Copy current source value into the value for this zone.
                newvals->SetTuple(i, array->GetTuple(nvals));
                nvals++;
            
                // If the cell is mixed then copy the current mixvar value 
                // into the mixvar for this zone.
                if(matno < 0 && mixvals != NULL)
                {
                    int mix_idx = -(matno) - 1;
                    while(mix_idx >= 0)
                    {      
                        int src = nMixVarComponents * nmixvals;
                        int dest = array->GetNumberOfComponents() * mix_idx;
                        for (int j = 0; j < array->GetNumberOfComponents(); j++)
                        {
                            if(j < nMixVarComponents)
                                newmixvals[dest + j] = mixvals[src + j];
                            else
                                newmixvals[dest + j] = 0.f;
                        }
                        nmixvals++;
                        mix_idx = mat->GetMixNext()[mix_idx]-1;
                    }
                }
            }
        }
    }

    // Package up a new mixed variable and pass it back. Note that we delete the old one.
    if(mixvals != NULL)
    {
        avtMixedVariable *newmv = new avtMixedVariable(newmixvals, newmixlen, (*mv)->GetVarname());
        delete *mv;
        delete [] newmixvals;
        *mv = newmv;
    }

    return newvals;
#endif
}

// ****************************************************************************
//  Method: avtSimV2FileFormat::GetVar
//
//  Purpose:
//      For the mdserver, do nothing.
//      For the engine, return a scalar converted to a VTK array.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Feb 19 15:09:06 PST 2010
//
//  Modifications:
//    Brad Whitlock, Tue Jan 18 00:16:58 PST 2011
//    I added support for sharing the simulation array directly.
//
//    Brad Whitlock, Thu Jul 19 12:19:54 PDT 2012
//    I added support for long. I also added support for various types of 
//    mixed variables beyond float/double. I also added support for expanding
//    material restricted variables back up to the whole mesh.
//
// ****************************************************************************

vtkDataArray *
avtSimV2FileFormat::GetVar(int domain, const char *varname)
{
#ifdef MDSERVER
    return NULL;
#else

    visit_handle h = simv2_invoke_GetVariable(domain, varname);
    if (h == VISIT_INVALID_HANDLE)
        return NULL;

    // Get the data from the opaque object.
    int owner, dataType, nComponents, nTuples;
    void *data = NULL;
    int err = simv2_VariableData_getData(h, owner, dataType, nComponents, 
                                         nTuples, data);
    if(err == VISIT_ERROR || nTuples < 1)
        return NULL;
    vtkDataArray *array = 0;
    if (dataType == VISIT_DATATYPE_FLOAT)
    {
        vtkFloatArray *farray = vtkFloatArray::New();
        StoreVariableData(farray, (float *)data, nComponents, nTuples, owner);
        array = farray;
    }
    else if (dataType == VISIT_DATATYPE_DOUBLE)
    {
        vtkDoubleArray *darray = vtkDoubleArray::New();
        StoreVariableData(darray, (double *)data, nComponents, nTuples, owner);
        array = darray;
    }
    else if (dataType == VISIT_DATATYPE_INT)
    {
        vtkIntArray *iarray = vtkIntArray::New();
        StoreVariableData(iarray, (int *)data, nComponents, nTuples, owner);
        array = iarray;
    }
    else if(dataType == VISIT_DATATYPE_CHAR)
    {
        vtkUnsignedCharArray *ucarray = vtkUnsignedCharArray::New();
        StoreVariableData(ucarray, (unsigned char *)data, nComponents, nTuples, owner);
        array = ucarray;
    }
    else if(dataType == VISIT_DATATYPE_LONG)
    {
        vtkLongArray *larray = vtkLongArray::New();
        StoreVariableData(larray, (long *)data, nComponents, nTuples, owner);
        array = larray;
    }
    else
    {
        EXCEPTION1(InvalidVariableException, varname);
    }
#ifdef USE_SET_ARRAY
    // We've given the pointer from the variable data object to the VTK data 
    // array, which will dispose of it. We must NULL out the object so we
    // don't delete the memory when we free the variable data object.
    simv2_VariableData_nullData(h);
#endif
    simv2_VariableData_free(h);

    // Get the mixed variable.
    avtMixedVariable *mv = NULL;
    h = simv2_invoke_GetMixedVariable(domain, varname);
    int nMixVarComponents = 1;
    if (h != VISIT_INVALID_HANDLE)
    {
        err = simv2_VariableData_getData(h, owner, dataType, nComponents, 
                                         nTuples, data);
        nMixVarComponents = nComponents;
        if(err != VISIT_ERROR &&
           nTuples > 0 &&
           (dataType == VISIT_DATATYPE_CHAR ||
            dataType == VISIT_DATATYPE_INT ||
            dataType == VISIT_DATATYPE_LONG ||
            dataType == VISIT_DATATYPE_FLOAT ||
            dataType == VISIT_DATATYPE_DOUBLE)
           )
        {
            int mixlen = nTuples * nComponents;
            float *mixvar = new float[mixlen];
            debug1 << "SimV2 copying mixvar data: " << mixlen
                   << " values" << endl;
            if(dataType == VISIT_DATATYPE_CHAR)
            {
                // Convert the int to floats.
                const char *src = (const char *)data;
                mixvar = new float[mixlen];
                for(int i = 0; i < mixlen; ++i)
                    mixvar[i] = (float)src[i];
            }
            else if(dataType == VISIT_DATATYPE_INT)
            {
                // Convert the int to floats.
                const int *src = (const int *)data;
                mixvar = new float[mixlen];
                for(int i = 0; i < mixlen; ++i)
                    mixvar[i] = (float)src[i];
            }
            else if(dataType == VISIT_DATATYPE_LONG)
            {
                // Convert the int to floats.
                const long *src = (const long *)data;
                mixvar = new float[mixlen];
                for(int i = 0; i < mixlen; ++i)
                    mixvar[i] = (float)src[i];
            }
            else if(dataType == VISIT_DATATYPE_DOUBLE)
            {
                // Convert the doubles to floats.
                const double *src = (const double *)data;
                mixvar = new float[mixlen];
                for(int i = 0; i < mixlen; ++i)
                    mixvar[i] = (float)src[i];
            }
            else
                memcpy(mixvar, data, sizeof(float)*mixlen);

            // Cache the mixed data.
            mv = new avtMixedVariable(mixvar, mixlen, varname);

            delete [] mixvar;
        }

        simv2_VariableData_free(h);
    }

    // See if the variable is restricted to certain materials. If so, we need to
    // inflate it back up to the whole mesh size.
    intVector matRestricted(GetRestrictedMaterialIndices(varname));
    if(!matRestricted.empty())
    {
        vtkDataArray *newarr = ExpandVariable(array, &mv, nMixVarComponents, 
            domain, varname, matRestricted);
        array->Delete();
        array = newarr;
    }

    // See if there is a polyhedral split for this variable's mesh.
    PolyhedralSplit *phSplit = 0;
    TRY
    {
        std::string meshName = metadata->MeshForVar(varname);
        void_ref_ptr vr = cache->GetVoidRef(meshName.c_str(), 
            AUXILIARY_DATA_POLYHEDRAL_SPLIT, this->timestep, domain);
        if(*vr != 0)
        {
            debug4 << "Found a cached polyhedral split for "
                   << meshName << " at: " << (*vr) << endl;
            phSplit = (PolyhedralSplit *)(*vr);

            // Get the variable centering
            avtCentering centering = GetCentering(varname);
            vtkDataArray *splitArray = phSplit->ExpandDataArray(array, 
                centering == AVT_ZONECENT);
            array->Delete();
            array = splitArray;
        }
    }
    CATCH(VisItException)
    {
        // ignore the exception
    }
    ENDTRY

    // Cache the mixed variable. Note that we only do it if we haven't further split
    // the mesh to handle polyhedra since the polyhedral splitting does not currently
    // take into account per-material values.
    if(mv != NULL)
    {
        if(phSplit == 0)
        {
            void_ref_ptr vr = void_ref_ptr(mv, avtMixedVariable::Destruct);
            cache->CacheVoidRef(varname, AUXILIARY_DATA_MIXED_VARIABLE, 
                                this->timestep, domain, vr);
            debug1 << "SimV2 cached mixvar data for " << varname
                   << " domain " << domain << endl;
        }
        else
        {
            debug1 << "SimV2 did not cache mixvar for " << varname
                   << " due to polyhedral splitting." << endl;
            delete mv;
        }
    }

    return array;
#endif
}


// ****************************************************************************
//  Method: avtSimV2FileFormat::GetVectorVar
//
//  Purpose:
//      For the mdserver, do nothing.
//      For the engine, also do nothing right now.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   March 17, 2005
//
//  Modifications:
//    Brad Whitlock, Sat Feb 14 15:49:02 PST 2009
//    I made it call GetVar.
//
// ****************************************************************************

vtkDataArray *
avtSimV2FileFormat::GetVectorVar(int domain, const char *varname)
{
    return GetVar(domain, varname);
}

// ****************************************************************************
//  Method:  avtSimV2FileFormat::GetAuxiliaryData
//
//  Purpose:
//    Get auxiliary data.  E.g. material, species.
//
//  Arguments:
//    var        variable name
//    domain     the domain
//    type       the type of auxiliary data
//    df         (out) the destructor
//
//  Programmer:  Brad Whitlock
//  Creation:    April 11, 2005
//
//  Modifications:
//    Brad Whitlock, Mon Feb  9 13:44:38 PST 2009
//    I added species.
//
// ****************************************************************************
void *
avtSimV2FileFormat::GetAuxiliaryData(const char *var, int domain,
                              const char *type, void *, DestructorFunction &df)
{
    void *rv = NULL;
#ifndef MDSERVER
    if (strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        rv = (void *) GetMaterial(domain, var);
        df = avtMaterial::Destruct;
    }
    else if (strcmp(type, AUXILIARY_DATA_SPECIES) == 0)
    {
        rv = (void *) GetSpecies(domain, var);
        df = avtSpecies::Destruct;
    }
#endif
    return rv;
}

// ****************************************************************************
//  Method:  avtSimV2FileFormat::GetMaterial
//
//  Purpose:
//    Return a material for a domain.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the material variable requested.
//
//  Programmer:  Brad Whitlock
//  Creation:    Tue Mar  2 11:49:42 PST 2010
//
//  Modifications:
//
// ****************************************************************************

avtMaterial *
avtSimV2FileFormat::GetMaterial(int domain, const char *varname)
{
#ifdef MDSERVER
    return NULL;
#else
    const char *mName = "avtSimV2FileFormat::GetMaterial: ";

    visit_handle h = simv2_invoke_GetMaterial(domain,varname);
    if (h == VISIT_INVALID_HANDLE)
    {
        debug1 << mName << "An invalid handle was given for the material" << endl;
        return NULL;
    }

    int nMaterials = 0;
    if(simv2_MaterialData_getNumMaterials(h, nMaterials) == VISIT_ERROR)
    {
        debug1 << mName << "Could not query number of materials" << endl;
        simv2_FreeObject(h);
        return NULL;
    }

    // Get the material names
    vector<string> matNames(nMaterials);
    int *materialNumbers = new int[nMaterials];
    char matName[100];
    for (int m=0; m < nMaterials; m++)
    {
        if(simv2_MaterialData_getMaterial(h, m, materialNumbers[m], matName, 
            100) == VISIT_ERROR)
        {
            debug1 << mName << "Could not get material " << m << endl;
            simv2_FreeObject(h);
            return NULL;
        }
        matNames[m] = string(matName);
    }

    // Get the materials
    visit_handle mHandles[5];
    if(simv2_MaterialData_getMaterials(h, mHandles[0]) == VISIT_ERROR)
    {
        debug1 << mName << "could not get material from MaterialData" << endl;
        simv2_FreeObject(h);
        return NULL;
    }

    // Get the mixed materials
    if(simv2_MaterialData_getMixedMaterials(h, mHandles[1], mHandles[2], 
        mHandles[3], mHandles[4]) == VISIT_ERROR)
    {
        debug1 << "Could not get mixed materials from MaterialData" << endl;
        simv2_FreeObject(h);
        return NULL;
    }

    // Get the actual data from the materials
    int owner, dataType[5]={0,0,0,0,0}, nComps[5]={0,0,0,0,0};
    int nTuples[5]={0,0,0,0,0};
    void *data[5] = {NULL,NULL,NULL,NULL,NULL};
    if(simv2_VariableData_getData(mHandles[0], owner, dataType[0], nComps[0], 
        nTuples[0], data[0]) == VISIT_ERROR)
    {
        debug1 << mName << "Could not access variable data for matlist" << endl;
        simv2_FreeObject(h);
        return NULL;
    }
    bool haveMixedMaterials = mHandles[1] != VISIT_INVALID_HANDLE;
    if(haveMixedMaterials)
    {
        for(int i = 1; i < 5; ++i)
        {
           if(simv2_VariableData_getData(mHandles[i], owner, dataType[i],
               nComps[i], nTuples[i], data[i]) == VISIT_ERROR)
           {
               debug1 << mName << "Could not access mixed material data" << endl;
               simv2_FreeObject(h);
               return NULL;
           }
        }
    }
    const int *matlist = (const int *)data[0];
    const int *mix_mat = (const int *)data[1];
    const int *mix_zone = (const int *)data[2];
    const int *mix_next = (const int *)data[3];
    const float *mix_vf = (const float *)data[4];

    // Scan the material numbers to see if they are 0..N-1. If not then use
    // the contructor that will perform re-ordering.
    bool *matUsed = new bool[nMaterials];
    for(int i = 0; i < nMaterials; ++i)
        matUsed[i] = false;

    bool reorderRequired = false;
    for(int i = 0; i < nTuples[0]; ++i)
    {
        if(matlist[i] < 0)
            continue;
        else if(matlist[i] >= 0 && matlist[i] < nMaterials)
            matUsed[matlist[i]] = true;
        else
        {
            reorderRequired = true;
            break;
        }
    }
    if(haveMixedMaterials && !reorderRequired)
    {        
        for(int i = 0; i < nTuples[1]; ++i)
        {
            if(mix_mat[i] >= 0 && mix_mat[i] < nMaterials)
                matUsed[mix_mat[i]] = true;
            else
            {
                reorderRequired = true;
                break;
            }
        }
    }
    if(!reorderRequired)
    {
        bool allUsed = true;
        for(int i = 0; i < nMaterials; ++i)
            allUsed &= matUsed[i];
        reorderRequired = !allUsed;
    }
    delete [] matUsed;

    avtMaterial *mat = 0;
    if(reorderRequired)
    {
        debug5 << mName << "Reordering of material numbers is needed." << endl;
        char **matnames = new char *[nMaterials];
        for(int i = 0; i < nMaterials; ++i)
            matnames[i] = (char *)matNames[i].c_str(); // for sake of avtMaterial
        mat = new avtMaterial(nMaterials,
                              materialNumbers,
                              matnames,
                              1,
                              &nTuples[0], // #zones
                              0,
                              matlist,
                              nTuples[1], // mixlen
                              mix_mat,
                              mix_next,
                              mix_zone,
                              mix_vf,
                              "domain", 1);
        delete [] matnames;
    }
    else
    {
        debug5 << mName << "No reordering of material numbers is needed." << endl;
        mat = new avtMaterial(nMaterials,
                              matNames,
                              nTuples[0], // #zones
                              matlist,
                              nTuples[1], // mixlen
                              mix_mat,
                              mix_next,
                              mix_zone,
                              mix_vf);
    }

    delete [] materialNumbers;
    simv2_FreeObject(h);

    return mat;
#endif
}

// ****************************************************************************
//  Method:  avtSimV2FileFormat::GetCurve
//
//  Purpose:
//    Return a curve by name.
//
//  Arguments:
//      varname    The name of the curve requested.
//
//  Programmer:  Brad Whitlock
//  Creation:    Mon Mar  1 15:03:37 PST 2010
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtSimV2FileFormat::GetCurve(const char *name)
{
#ifdef MDSERVER
    return NULL;
#else
    visit_handle h = simv2_invoke_GetCurve(name);
    if (h == VISIT_INVALID_HANDLE)
        return NULL;

    visit_handle cHandles[2];
    if(simv2_CurveData_getData(h, cHandles[0], cHandles[1]) == VISIT_ERROR)
    {
        simv2_FreeObject(h);
        EXCEPTION1(ImproperUseException,
                   "Could not obtain curve data using the provided handle.\n");
    }

    vtkRectilinearGrid *rg = 0;

    int owner[2], dataType[2], nComps[2], nTuples[2];
    void *data[2] = {0, 0};
    for(int i = 0; i < 2; ++i)
    {
        if(simv2_VariableData_getData(cHandles[i], owner[i], dataType[i],
            nComps[i], nTuples[i], data[i]) == VISIT_ERROR)
        {
            simv2_FreeObject(h);
            EXCEPTION1(ImproperUseException,
                "Could not obtain curve coordinate data using the provided handle.\n");
        }

        vtkFloatArray *arr = 0;
        if(i == 0)
        {
            rg = vtkVisItUtility::Create1DRGrid(nTuples[i], VTK_FLOAT);
            arr = vtkFloatArray::SafeDownCast(rg->GetXCoordinates());
        }
        else
        {
            arr = vtkFloatArray::New();
            arr->SetNumberOfTuples(nTuples[i]);
            arr->SetName(name);
            rg->GetPointData()->SetScalars(arr);
        }

        if(dataType[i] == VISIT_DATATYPE_DOUBLE)
        {
            double *ptr = (double *)data[i];
            for(int j = 0; j < nTuples[i]; ++j)
                arr->SetValue(j, (float)ptr[j]);
        }
        else if(dataType[i] == VISIT_DATATYPE_FLOAT)
        {
            float *ptr = (float *)data[i];
            for(int j = 0; j < nTuples[i]; ++j)
                arr->SetValue(j, ptr[j]);
        }
        else if(dataType[i] == VISIT_DATATYPE_INT)
        {
            int *ptr = (int *)data[i];
            for(int j = 0; j < nTuples[i]; ++j)
                arr->SetValue(j, (float)ptr[j]);
        }

        if(i > 0)
            arr->Delete();
    }

    simv2_FreeObject(h);

    return rg;
#endif
}

// ****************************************************************************
// Method: avtSimV2FileFormat::GetSpecies
//
// Purpose: 
//   Gets the species for the dataset.
//
// Arguments:
//   domain  : The domain for which we're returning species
//   varname : The name of the species variable.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb  6 16:47:44 PST 2009
//
// Modifications:
//   
// ****************************************************************************

avtSpecies *
avtSimV2FileFormat::GetSpecies(int domain, const char *varname)
{
#ifdef MDSERVER
    return NULL;
#else
    const char *mName = "avtSimV2FileFormat::GetSpecies: ";
    visit_handle h = simv2_invoke_GetSpecies(domain, varname);
    if (h == VISIT_INVALID_HANDLE)
        return NULL;

    // Get data out of the species object.
    std::vector<visit_handle> namelist;
    visit_handle species, speciesMF, mixedSpecies;
    if(simv2_SpeciesData_getData(h, namelist, species, speciesMF, 
        mixedSpecies) == VISIT_ERROR)
    {
        debug4 << mName << "Can't get data out of SpeciesData object" << endl;
        simv2_FreeObject(h);
        return NULL;
    }

    bool err = false;
    vector<int>             numSpecies;
    vector<vector<string> > speciesNames;
    for(size_t i = 0; i < namelist.size() && !err; ++i)
    {
        int nNames = 0;
        if(simv2_NameList_getNumName(namelist[i], &nNames) == VISIT_OKAY)
        {
            numSpecies.push_back(nNames);

            vector<string> onelist;
            for(int n = 0; n < nNames && !err; ++n)
            { 
                char *specName = NULL;
                if(simv2_NameList_getName(namelist[i], n, &specName) == VISIT_OKAY)
                {
                    onelist.push_back(specName);
                    free(specName);
                }
                else
                    err = true;
            }
            speciesNames.push_back(onelist);
        }
        else
            err = true;
    }
    if(err)
    {
        debug4 << mName << "Can't get name list" << endl;
        simv2_FreeObject(h);
        return NULL;
    }

    int species_owner, species_dataType, species_nComps, species_nTuples;
    void *species_data = NULL;
    if(simv2_VariableData_getData(species, species_owner, species_dataType,
        species_nComps, species_nTuples, species_data) == VISIT_ERROR)
    {
        debug4 << mName << "Can't get species" << endl;
        simv2_FreeObject(h);
        return NULL;
    }

    int speciesMF_owner, speciesMF_dataType, speciesMF_nComps, speciesMF_nTuples;
    void *speciesMF_data = NULL;
    if(simv2_VariableData_getData(speciesMF, speciesMF_owner, speciesMF_dataType,
        speciesMF_nComps, speciesMF_nTuples, speciesMF_data) == VISIT_ERROR)
    {
        debug4 << mName << "Can't get speciesMF" << endl;
        simv2_FreeObject(h);
        return NULL;
    }

    int mixedSpecies_owner, mixedSpecies_dataType, mixedSpecies_nComps, 
        mixedSpecies_nTuples;
    void *mixedSpecies_data = NULL;
    if(simv2_VariableData_getData(mixedSpecies, mixedSpecies_owner, 
        mixedSpecies_dataType, mixedSpecies_nComps, mixedSpecies_nTuples, 
        mixedSpecies_data) == VISIT_ERROR)
    {
        debug4 << mName << "Can't get mixedSpecies" << endl;
        simv2_FreeObject(h);
        return NULL;
    }

    avtSpecies *spec = new avtSpecies(numSpecies,
        speciesNames,
        species_nTuples, (int *)species_data,
        mixedSpecies_nTuples, (int *)mixedSpecies_data,
        speciesMF_nTuples, (float *)speciesMF_data);

    simv2_FreeObject(h);

    return spec;
#endif
}

// ****************************************************************************
//  Method:  avtSimV2FileFormat::PopulateIOInformation
//
//  Purpose:
//    Populate the list of acceptable domains for this processor.
//
//  Arguments:
//    ioinfo     the avtIOInformation containing the output domain list
//
//  Programmer:  Brad Whitlock
//  Creation:    Mon Mar  1 16:50:18 PST 2010
//
//  Modifications:
//
// ****************************************************************************

void
avtSimV2FileFormat::PopulateIOInformation(avtIOInformation& ioInfo)
{
#ifndef MDSERVER
    const char *mName = "avtSimV2FileFormat::PopulateIOInformation: ";

    // TODO: pass in a mesh name
    visit_handle h = simv2_invoke_GetDomainList("any");
    if (h == VISIT_INVALID_HANDLE)
        return;

    int rank = 0;
    int size = 1;
#ifdef PARALLEL
    rank = PAR_Rank();
    size = PAR_Size();
#endif

    int alldoms = 0;
    visit_handle mydoms;
    if(simv2_DomainList_getData(h, alldoms, mydoms) == VISIT_ERROR)
    {
        debug1 << mName << "Could not get domain list data" << endl;
        simv2_FreeObject(h);
        return;
    }

    int owner, dataType, nComps, nTuples;
    void *data = 0;
    if(simv2_VariableData_getData(mydoms, owner, dataType, nComps, nTuples, 
       data) == VISIT_ERROR)
    {
        debug1 << mName << "Could not get domain list data" << endl;
        simv2_FreeObject(h);
        return;
    }

    vector< vector<int> > hints;
    hints.resize(size);
    hints[rank].resize(nTuples);
    for (int i=0; i<nTuples; i++)
    {
        int dom = ((int *)data)[i];
        if(dom >= 0 && dom < alldoms)
            hints[rank][i] = dom; 
        else
        {
            debug1 << mName << "An out of range domain number " << dom
                   << " was given in the domain list. Valid numbers are in [0,"
                   << alldoms << "]" << endl;
            simv2_FreeObject(h);
            return;
        }
    }
    ioInfo.AddHints(hints);
    ioInfo.SetNDomains(alldoms);

    simv2_FreeObject(h);
#endif
}
