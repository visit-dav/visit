/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include <string>
#include <vector>

#include <avtDatabaseMetaData.h>
#include <avtGhostData.h>
#include <avtIOInformation.h>
#include <avtMixedVariable.h>
#include <avtSimulationInformation.h>
#include <avtSimulationCommandSpecification.h>
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
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVisItUtility.h>

#ifdef __APPLE__
#include <dlfcn.h>
#endif

using std::string;
using std::vector;

#ifdef PARALLEL
#include <mpi.h>
#include <avtParallel.h>
#endif

#ifndef __APPLE__
#ifndef MDSERVER
extern "C" VisIt_SimulationCallback visitCallbacks;
#endif
#endif

#ifndef MDSERVER
vtkDataSet *SimV2_GetMesh_Curvilinear(VisIt_CurvilinearMesh *);
vtkDataSet *SimV2_GetMesh_Rectilinear(VisIt_RectilinearMesh *);
vtkDataSet *SimV2_GetMesh_Unstructured(VisIt_UnstructuredMesh *);
vtkDataSet *SimV2_GetMesh_Point(VisIt_PointMesh *);
vtkDataSet *SimV2_GetMesh_CSG(VisIt_CSGMesh *csgm);
#endif

// For now, include the c file directly
#include "VisItDataInterface_V2.c"

// ****************************************************************************
//  Function:  FreeDataArray
//
//  Purpose:
//    Safely (i.e. only if we own it) frees a VisIt_DataArray.
//
//  Arguments:
//    da         the data array structure
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 28, 2005
//
// ****************************************************************************
void FreeDataArray(VisIt_DataArray &da)
{
    if (da.owner != VISIT_OWNER_VISIT)
        return;

    switch (da.dataType)
    {
      case VISIT_DATATYPE_CHAR:
        free(da.cArray);
        da.cArray = NULL;
        break;
      case VISIT_DATATYPE_INT:
        free(da.iArray);
        da.iArray = NULL;
        break;
      case VISIT_DATATYPE_FLOAT:
        free(da.fArray);
        da.fArray = NULL;
        break;
      case VISIT_DATATYPE_DOUBLE:
        free(da.dArray);
        da.dArray = NULL;
        break;
    }
}

// ****************************************************************************
//  Method: avtSimV1 constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   March 17, 2005
//
//  Modifications:
//
//    Jeremy Meredith, Fri Nov  2 18:00:15 EDT 2007
//    On OSX, use dlsym to retrieve the visitCallbacks.
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
        if (strcasecmp(buff, "host")==0)
        {
            in >> buff;
            simInfo.SetHost(buff);
        }
        else if (strcasecmp(buff, "port")==0)
        {
            int port;
            in >> port;
            simInfo.SetPort(port);
        }
        else if (strcasecmp(buff, "key")==0)
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
#else // ENGINE

    // On OSX, we want to check for the visitCallbacks via dlsym
    // because we're not currently allowing unresolved symbols in our
    // plugins, which the old method requires.  Theoretically, we could
    // use the dlsym method on other platforms, too, but it hasn't been
    // thoroughly tested.
#ifdef __APPLE__
    void *cbptr = dlsym(RTLD_DEFAULT, "visitCallbacks");
    if (!cbptr)
        EXCEPTION2(InvalidFilesException,filename,
                   "Could not find 'visitCallbacks' in the current exe.");
    cb = *((VisIt_SimulationCallback*)cbptr);
#else
    cb = visitCallbacks;
#endif

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
//  Programmer: Jeremy Meredith
//  Creation:   August 11, 2004
//
// ****************************************************************************

void
avtSimV2FileFormat::FreeUpResources(void)
{
}


// ****************************************************************************
//  Method: avtSimV2FileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Fill the simulation metadata with the parameters from the file for
//      the mdserver.  Get the info from the simulation for the engine.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 14, 2005
//
//  Modifications:
//   Brad Whitlock, Mon Feb  9 14:21:08 PST 2009
//   I added species and deletion of metadata objects.
//
// ****************************************************************************

void
avtSimV2FileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    md->SetIsSimulation(true);
#ifdef MDSERVER
    md->SetSimInfo(simInfo);
#else

    if (!cb.GetMetaData)
    {
        return;
    }

    VisIt_SimulationMetaData *vsmd = cb.GetMetaData();

    md->SetCycle(timestep, vsmd->currentCycle);
    md->SetTime(timestep, vsmd->currentTime);

    switch(vsmd->currentMode)
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

    for (int m=0; m<vsmd->numMeshes; m++)
    {
        VisIt_MeshMetaData *mmd = &vsmd->meshes[m];
        avtMeshMetaData *mesh = new avtMeshMetaData;
        mesh->name = mmd->name;

        switch (mmd->meshType)
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
          case VISIT_MESHTYPE_SURFACE:
            mesh->meshType = AVT_SURFACE_MESH;
            break;
          case VISIT_MESHTYPE_CSG:
            mesh->meshType = AVT_CSG_MESH;
            break;
          default:
            VisIt_SimulationMetaData_free(vsmd);
            EXCEPTION1(ImproperUseException,
                       "Invalid mesh type in VisIt_MeshMetaData.");
            break;
        }
        mesh->topologicalDimension = mmd->topologicalDimension;
        mesh->spatialDimension = mmd->spatialDimension;
        mesh->hasSpatialExtents = false;

        mesh->numBlocks = mmd->numBlocks;
        if (mmd->blockTitle)
            mesh->blockTitle = mmd->blockTitle;
        if (mmd->blockPieceName)
            mesh->blockPieceName = mmd->blockPieceName;

        mesh->numGroups = mmd->numGroups;
        if (mesh->numGroups > 0 && mmd->groupTitle)
            mesh->groupTitle = mmd->groupTitle;
        if (mesh->numGroups > 0 && mmd->groupPieceName)
            mesh->groupPieceName = mmd->groupPieceName;

        mesh->groupIds.resize(mesh->numGroups);
        for (int g = 0; g<mesh->numGroups; g++)
        {
            mesh->groupIds[g] = mmd->groupIds[g];
        }

        if (mmd->xLabel)
            mesh->xLabel = mmd->xLabel;
        if (mmd->yLabel)
            mesh->yLabel = mmd->yLabel;
        if (mmd->zLabel)
            mesh->zLabel = mmd->zLabel;

        if (mmd->units)
        {
            mesh->xUnits = mmd->units;
            mesh->yUnits = mmd->units;
            mesh->zUnits = mmd->units;
        }

        md->Add(mesh);
    }

    for (int s=0; s<vsmd->numScalars; s++)
    {
        VisIt_ScalarMetaData *smd = &vsmd->scalars[s];
        avtScalarMetaData *scalar = new avtScalarMetaData;
        scalar->name = smd->name;
        scalar->originalName = smd->name;
        scalar->meshName = smd->meshName;
        switch (smd->centering)
        {
          case VISIT_VARCENTERING_NODE:
            scalar->centering = AVT_NODECENT;
            break;
          case VISIT_VARCENTERING_ZONE:
            scalar->centering = AVT_ZONECENT;
            break;
          default:
            VisIt_SimulationMetaData_free(vsmd);
            EXCEPTION1(ImproperUseException,
                       "Invalid centering type in VisIt_ScalarMetaData.");
        }
        scalar->treatAsASCII = smd->treatAsASCII;
        scalar->hasDataExtents = false;
        scalar->hasUnits = false;

        md->Add(scalar);
    }

    for (int c=0; c<vsmd->numGenericCommands; c++)
    {
        VisIt_SimulationControlCommand *scc = &vsmd->genericCommands[c];
        avtSimulationCommandSpecification::CommandArgumentType t;
        switch (scc->argType)
        {
          case VISIT_CMDARG_NONE:
            t = avtSimulationCommandSpecification::CmdArgNone;
            break;
          case VISIT_CMDARG_INT:
            t = avtSimulationCommandSpecification::CmdArgInt;
            break;
          case VISIT_CMDARG_FLOAT:
            t = avtSimulationCommandSpecification::CmdArgFloat;
            break;
          case VISIT_CMDARG_STRING:
            t = avtSimulationCommandSpecification::CmdArgString;
            break;
          default:
            VisIt_SimulationMetaData_free(vsmd);
            EXCEPTION1(ImproperUseException,
                       "Invalid command argument type in "
                       "VisIt_SimulationControlCommand.");
        }
#define NOT_NULL(S) ((S) != 0 ? (S) : "")
        avtSimulationCommandSpecification scs;
        scs.SetName(NOT_NULL(scc->name));
        scs.SetText(NOT_NULL(scc->text));
        scs.SetValue(NOT_NULL(scc->value));
        scs.SetIsOn(scc->isOn);
        scs.SetUiType(NOT_NULL(scc->uiType));
        scs.SetClassName(NOT_NULL(scc->className));
        scs.SetSignal(NOT_NULL(scc->signal));
        scs.SetArgumentType(t);
        scs.SetEnabled(scc->enabled);
        md->GetSimInfo().AddGenericCommands(scs);
    }
 
    for (int c=0; c<vsmd->numCustomCommands; c++)
    {
        VisIt_SimulationControlCommand *scc = &vsmd->customCommands[c];
        avtSimulationCommandSpecification::CommandArgumentType t;
        t =  avtSimulationCommandSpecification::CmdArgString;
        avtSimulationCommandSpecification scs;
        scs.SetName(NOT_NULL(scc->name));
        scs.SetText(NOT_NULL(scc->text));
        scs.SetValue(NOT_NULL(scc->value));
        scs.SetIsOn(scc->isOn);
        scs.SetUiType(NOT_NULL(scc->uiType));
        scs.SetClassName(NOT_NULL(scc->className));
        scs.SetArgumentType(t);
        scs.SetEnabled(scc->enabled);
        md->GetSimInfo().AddCustomCommands(scs);
    }
 
    for (int mat=0; mat<vsmd->numMaterials; mat++)
    {
        VisIt_MaterialMetaData *mmd = &vsmd->materials[mat];
        avtMaterialMetaData *material = new avtMaterialMetaData;
        material->name = mmd->name;
        material->originalName = mmd->name;
        material->meshName = mmd->meshName;
        material->numMaterials = mmd->numMaterials;
        material->materialNames.clear();
        for (int m = 0; m < material->numMaterials; m++)
        {
            material->materialNames.push_back(mmd->materialNames[m]);
        }

        md->Add(material);
    }

    for(int spec = 0; spec < vsmd->numSpecies; ++spec)
    {
        VisIt_SpeciesMetaData *smd = &vsmd->species[spec];
        vector<int>   numSpecies;
        vector<vector<string> > speciesNames;
        for (int i = 0 ; i < smd->nmaterialSpecies ; ++i)
        {
            numSpecies.push_back(smd->materialSpeciesNames[i].numNames);

            vector<string> onelist;
            for(int j = 0; j < smd->materialSpeciesNames[i].numNames; ++j)
                onelist.push_back(smd->materialSpeciesNames[i].names[j]);
            speciesNames.push_back(onelist);
        }

        avtSpeciesMetaData *species = new avtSpeciesMetaData(smd->name,
            smd->meshName, smd->materialName, smd->nmaterialSpecies, numSpecies, 
            speciesNames);
        md->Add(species);
    }

    for (int cc=0; cc<vsmd->numCurves; cc++)
    {
        VisIt_CurveMetaData *cmd = &vsmd->curves[cc];
        avtCurveMetaData *curve = new avtCurveMetaData;
        curve->name = cmd->name;
        curve->originalName = cmd->name;
        if (cmd->xUnits)
            curve->xUnits = cmd->xUnits;
        if (cmd->yUnits)
            curve->yUnits = cmd->yUnits;
        if (cmd->xLabel)
            curve->xLabel = cmd->xLabel;
        if (cmd->yLabel)
            curve->yLabel = cmd->yLabel;

        curveMeshes.insert(curve->name);

        md->Add(curve);
    }

    for(int e = 0; e < vsmd->numExpressions; ++e)
    {
        Expression *newexp = new Expression;
        newexp->SetName(vsmd->expressions[e].name);
        newexp->SetDefinition(vsmd->expressions[e].definition);
        if(vsmd->expressions[e].vartype == VISIT_VARTYPE_MESH)
            newexp->SetType(Expression::Mesh);
        else if(vsmd->expressions[e].vartype == VISIT_VARTYPE_SCALAR)
            newexp->SetType(Expression::ScalarMeshVar);
        else if(vsmd->expressions[e].vartype == VISIT_VARTYPE_VECTOR)
            newexp->SetType(Expression::VectorMeshVar);
        else if(vsmd->expressions[e].vartype == VISIT_VARTYPE_TENSOR)
            newexp->SetType(Expression::TensorMeshVar);
        else if(vsmd->expressions[e].vartype == VISIT_VARTYPE_SYMMETRIC_TENSOR)
            newexp->SetType(Expression::SymmetricTensorMeshVar);
        else if(vsmd->expressions[e].vartype == VISIT_VARTYPE_MATERIAL)
            newexp->SetType(Expression::Material);
        else if(vsmd->expressions[e].vartype == VISIT_VARTYPE_MATSPECIES)
            newexp->SetType(Expression::Species);
        else
            newexp->SetType(Expression::Unknown);

        md->AddExpression(newexp);
    }
    //md->Print(cout);
    VisIt_SimulationMetaData_free(vsmd);
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

    if (!cb.GetMesh)
        return NULL;

    // Call into the simulation to get the mesh
    VisIt_MeshData *vmesh = cb.GetMesh(domain, meshname);

    // If the mesh could not be created then throw an exception.
    if(vmesh == NULL)
    {
        EXCEPTION1(InvalidVariableException, meshname);
    }

    vtkDataSet *ds = 0;
    TRY
    {
        switch (vmesh->meshType)
        {
        case VISIT_MESHTYPE_CURVILINEAR:
            ds = SimV2_GetMesh_Curvilinear(vmesh->cmesh);
            break;
        case VISIT_MESHTYPE_RECTILINEAR:
            ds = SimV2_GetMesh_Rectilinear(vmesh->rmesh);
            break;
        case VISIT_MESHTYPE_UNSTRUCTURED:
            ds = SimV2_GetMesh_Unstructured(vmesh->umesh);
            break;
        case VISIT_MESHTYPE_POINT:
            ds = SimV2_GetMesh_Point(vmesh->pmesh);
            break;
        case VISIT_MESHTYPE_CSG:
            ds = SimV2_GetMesh_CSG(vmesh->csgmesh);
            break;
        default:
            EXCEPTION1(ImproperUseException,
                       "You've tried to use an unsupported mesh type.\n");
            break;
        }
    }
    CATCH2(VisItException, e)
    {
        VisIt_MeshData_free(vmesh);
        RETHROW;
    }
    ENDTRY

    VisIt_MeshData_free(vmesh);

    return ds;
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
//  Programmer: Jeremy Meredith
//  Creation:   March 17, 2005
//
//  Modifications:
//    Jeremy Meredith, Thu Apr 28 18:00:32 PDT 2005
//    Added true data array structures in place of raw array pointers.
//
//    Jeremy Meredith, Wed May 11 10:56:21 PDT 2005
//    Allowed for NULL responses.  Added int/char support.
//
//    Brad Whitlock, Thu Apr 10 11:18:11 PDT 2008
//    Added mixvar support.
//
//    Brad Whitlock, Mon Jun  2 14:12:46 PDT 2008
//    Check that cb.GetMixedScalar != NULL.
//
//    Brad Whitlock, Fri Feb  6 16:39:09 PST 2009
//    Return native data types since the transform manager should handle it. I
//    also added code to delete the scalar objects.
//
// ****************************************************************************

vtkDataArray *
avtSimV2FileFormat::GetVar(int domain, const char *varname)
{
#ifdef MDSERVER
    return NULL;
#else
    if (!cb.GetScalar)
        return NULL;

    VisIt_ScalarData *sd = cb.GetScalar(domain,varname);
    if (!sd || sd->len<=0)
        return NULL;

    vtkDataArray *array = 0;
    if (sd->data.dataType == VISIT_DATATYPE_FLOAT)
    {
        array = vtkFloatArray::New();
        array->SetNumberOfTuples(sd->len);
        memcpy(array->GetVoidPointer(0), sd->data.fArray, sizeof(float) * sd->len);
    }
    else if (sd->data.dataType == VISIT_DATATYPE_DOUBLE)
    {
        array = vtkDoubleArray::New();
        array->SetNumberOfTuples(sd->len);
        memcpy(array->GetVoidPointer(0), sd->data.dArray, sizeof(double) * sd->len);
    }
    else if (sd->data.dataType == VISIT_DATATYPE_INT)
    {
        array = vtkIntArray::New();
        array->SetNumberOfTuples(sd->len);
        memcpy(array->GetVoidPointer(0), sd->data.iArray, sizeof(int) * sd->len);
    }
    else // (sd->data.dataType == VISIT_DATATYPE_CHAR)
    {
        array = vtkUnsignedCharArray::New();
        array->SetNumberOfTuples(sd->len);
        memcpy(array->GetVoidPointer(0), sd->data.cArray, sizeof(unsigned char) * sd->len);
    }

    VisIt_ScalarData_free(sd);

    // Try and read mixed scalar data.
    VisIt_MixedScalarData *mixed_sd = NULL;
    if(cb.GetMixedScalar != NULL)
         mixed_sd = cb.GetMixedScalar(domain,varname);
    if (mixed_sd != NULL)
    {
        if(mixed_sd->len > 0 &&
           (mixed_sd->data.dataType == VISIT_DATATYPE_FLOAT ||
            mixed_sd->data.dataType == VISIT_DATATYPE_DOUBLE)
           )
        {
            int mixlen = mixed_sd->len;
            float *mixvar = new float[mixlen];
            debug1 << "SimV1 copying mixvar data: " << mixlen
                   << " values" << endl;
            if(mixed_sd->data.dataType == VISIT_DATATYPE_DOUBLE)
            {
                // Convert the doubles to floats.
                const double *src = mixed_sd->data.dArray;
                mixvar = new float[mixlen];
                for(int i = 0; i < mixlen; ++i)
                    mixvar[i] = (float)src[i];
            }
            else
                memcpy(mixvar, mixed_sd->data.fArray, sizeof(float)*mixlen);

            // Cache the mixed data.
            avtMixedVariable *mv = new avtMixedVariable(mixvar,
                mixlen, varname);
            void_ref_ptr vr = void_ref_ptr(mv, avtMixedVariable::Destruct);
            cache->CacheVoidRef(varname, AUXILIARY_DATA_MIXED_VARIABLE, 
                                this->timestep, domain, vr);
            debug1 << "SimV1 cached mixvar data for " << varname
                   << " domain " << domain << endl;

            delete [] mixvar;
        }

        VisIt_MixedScalarData_free(mixed_sd);
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
//  Programmer: Jeremy Meredith
//  Creation:   March 17, 2005
//
// ****************************************************************************

vtkDataArray *
avtSimV2FileFormat::GetVectorVar(int domain, const char *varname)
{
    return NULL;
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
//  Programmer:  Jeremy Meredith
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
//  Programmer:  Jeremy Meredith
//  Creation:    April 11, 2005
//
//  Modifications:
//    Jeremy Meredith, Thu Apr 28 18:00:32 PDT 2005
//    Added true data array structures in place of raw array pointers.
//
//    Brad Whitlock, Fri Jan 18 10:22:59 PST 2008
//    Added code to detect when the material numbers are not 0..N so we can
//    use a constructor that will reorder them into that range so VisIt will
//    be happy.
//
//    Brad Whitlock, Tue Feb 10 11:34:10 PST 2009
//    Changed material structure and added code to free data.
//
// ****************************************************************************

avtMaterial *
avtSimV2FileFormat::GetMaterial(int domain, const char *varname)
{
#ifdef MDSERVER
    return NULL;
#else
    const char *mName = "avtSimV2FileFormat::GetMaterial: ";
    if (!cb.GetMaterial)
        return NULL;

    VisIt_MaterialData *md = cb.GetMaterial(domain,varname);
    if (!md)
        return NULL;

    if (md->matlist.dataType != VISIT_DATATYPE_INT)
    {
        VisIt_MaterialData_free(md);
        EXCEPTION1(ImproperUseException, "matlist array must be integers");
    }
    if (md->mix_mat.dataType != VISIT_DATATYPE_INT)
    {
        VisIt_MaterialData_free(md);
        EXCEPTION1(ImproperUseException, "mix_mat array must be integers");
    }
    if (md->mix_zone.dataType != VISIT_DATATYPE_INT)
    {
        VisIt_MaterialData_free(md);
        EXCEPTION1(ImproperUseException, "mix_zone array must be integers");
    }
    if (md->mix_next.dataType != VISIT_DATATYPE_INT)
    {
        VisIt_MaterialData_free(md);
        EXCEPTION1(ImproperUseException, "mix_next array must be integers");
    }
    if (md->mix_vf.dataType != VISIT_DATATYPE_FLOAT)
    {
        VisIt_MaterialData_free(md);
        EXCEPTION1(ImproperUseException, "mix_vf array must be floats");
    }

    vector<string> matNames(md->nMaterials);
    for (int m=0; m<md->nMaterials; m++)
        matNames[m] = md->materialNames[m];

    avtMaterial *mat = 0;
    // Scan the material numbers to see if they are 0..N-1. If not then use
    // the contructor that will perform re-ordering.
    bool *matUsed = new bool[md->nMaterials];
    bool reorderRequired = false;
    int i;
    for(int i = 0; i < md->nMaterials; ++i)
        matUsed[i] = false;
    for(int i = 0; i < md->nzones; ++i)
    {
        if(md->matlist.iArray[i] < 0)
            continue;
        else if(md->matlist.iArray[i] >= 0 && md->matlist.iArray[i] < md->nMaterials)
            matUsed[md->matlist.iArray[i]] = true;
        else
        {
            reorderRequired = true;
            break;
        }
    }
    if(!reorderRequired)
    {
        for(i = 0; i < md->mixlen; ++i)
        {
            if(md->mix_mat.iArray[i] >= 0 && md->mix_mat.iArray[i] < md->nMaterials)
                matUsed[md->mix_mat.iArray[i]] = true;
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
        for(i = 0; i < md->nMaterials; ++i)
            allUsed &= matUsed[i];
        reorderRequired = !allUsed;
    }
    delete [] matUsed;

    if(reorderRequired)
    {
        debug5 << mName << "Reordering of material numbers is needed." << endl;
        mat = new avtMaterial(md->nMaterials,
                              md->materialNumbers,
                              (char **)md->materialNames,
                              1,
                              &md->nzones,
                              0,
                              md->matlist.iArray,
                              md->mixlen,
                              md->mix_mat.iArray,
                              md->mix_next.iArray,
                              md->mix_zone.iArray,
                              md->mix_vf.fArray,
                              "domain", 1);
    }
    else
    {
        debug5 << mName << "No reordering of material numbers is needed." << endl;
        mat = new avtMaterial(md->nMaterials,
                              matNames,
                              md->nzones,
                              md->matlist.iArray,
                              md->mixlen,
                              md->mix_mat.iArray,
                              md->mix_next.iArray,
                              md->mix_zone.iArray,
                              md->mix_vf.fArray);
    }

    VisIt_MaterialData_free(md);

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
//  Programmer:  Jeremy Meredith
//  Creation:    April 14, 2005
//
//  Modifications:
//    Jeremy Meredith, Thu Apr 28 18:00:32 PDT 2005
//    Added true data array structures in place of raw array pointers.
//
//    Jeremy Meredith, Wed May 25 14:37:58 PDT 2005
//    Added ability to have separate types for x/y values.  Added support
//    for integer X's.
//
//    Kathleen Bonnell, Mon Jul 14 15:43:23 PDT 2008
//    Specify curves as 1D rectilinear grids with yvalues stored in point data.
//
//    Brad Whitlock, Tue Feb 10 11:16:31 PST 2009
//    I added code to delete curve data.
//
// ****************************************************************************

vtkDataSet *
avtSimV2FileFormat::GetCurve(const char *name)
{
#ifdef MDSERVER
    return NULL;
#else
    if (!cb.GetCurve)
        return NULL;

    VisIt_CurveData *cd = cb.GetCurve(name);
    if (!cd)
        return NULL;

    int npts = cd->len;

    vtkRectilinearGrid *rg = vtkVisItUtility::Create1DRGrid(npts, VTK_FLOAT);
    vtkFloatArray *xc = vtkFloatArray::SafeDownCast(rg->GetXCoordinates());
    if (cd->x.dataType == VISIT_DATATYPE_INT)
    {
        for (int j=0; j<npts; j++)
            xc->SetValue(j, cd->x.iArray[j]);
    }
    else if (cd->x.dataType == VISIT_DATATYPE_FLOAT)
    {
        for (int j=0; j<npts; j++)
            xc->SetValue(j, cd->x.fArray[j]);
    }
    else if (cd->x.dataType == VISIT_DATATYPE_DOUBLE)
    {
        for (int j=0; j<npts; j++)
            xc->SetValue(j, cd->x.dArray[j]);
    }
    else
    {
        VisIt_CurveData_free(cd);
        rg->Delete();
        EXCEPTION1(ImproperUseException, "Curve coordinate arrays "
                   "must be float, double, or int in X.\n");
    }

    vtkFloatArray *yv = vtkFloatArray::New();
    yv->SetNumberOfComponents(1);
    yv->SetNumberOfTuples(npts);
    yv->SetName(name);
    rg->GetPointData()->SetScalars(yv);
    yv->Delete();

    if (cd->y.dataType == VISIT_DATATYPE_FLOAT)
    {
        for (int j=0; j<npts; j++)
        {
            yv->SetValue(j, cd->y.fArray[j]);
        }
    }
    else if (cd->y.dataType == VISIT_DATATYPE_DOUBLE)
    {
        for (int j=0; j<npts; j++)
        {
            yv->SetValue(j, cd->y.dArray[j]);
        }
    }
    else
    {
        VisIt_CurveData_free(cd);
        rg->Delete();
        EXCEPTION1(ImproperUseException,
                   "Curve coordinate arrays must be float or double in Y.\n");
    }

    VisIt_CurveData_free(cd);

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
    if (!cb.GetSpecies)
        return NULL;

    VisIt_SpeciesData *spec = cb.GetSpecies(domain, varname);
    if (!spec)
        return NULL;

    if (spec->mixedSpecies.dataType != VISIT_DATATYPE_INT)
    {
        VisIt_SpeciesData_free(spec);
        EXCEPTION1(ImproperUseException,
                   "Species mixedSpecies array must contain int data.\n");
    }
    if (spec->species.dataType != VISIT_DATATYPE_INT)
    {
        VisIt_SpeciesData_free(spec);
        EXCEPTION1(ImproperUseException,
                   "Species species array must contain int data.\n");
    }
    if (spec->speciesMF.dataType != VISIT_DATATYPE_FLOAT)
    {
        VisIt_SpeciesData_free(spec);
        EXCEPTION1(ImproperUseException,
                   "Species speciesMF array must contain float data.\n");
    }
    if (spec->materialSpecies.dataType != VISIT_DATATYPE_INT)
    {
        VisIt_SpeciesData_free(spec);
        EXCEPTION1(ImproperUseException,
                   "Species materialSpecies array must contain int data.\n");
    }

    avtSpecies *species = 0;
    if(spec->materialSpeciesNames == 0)
    {
        species = new avtSpecies(spec->nmaterialSpecies,
                                 spec->materialSpecies.iArray,
                                 spec->ndims,
                                 spec->dims,
                                 spec->species.iArray,
                                 spec->nmixedSpecies,
                                 spec->mixedSpecies.iArray,
                                 spec->nspeciesMF,
                                 spec->speciesMF.fArray);
    }
    else
    {
        vector<int>   numSpecies;
        vector<vector<string> > speciesNames;
        for (int i = 0 ; i < spec->nmaterialSpecies ; ++i)
        {
            numSpecies.push_back(spec->materialSpecies.iArray[i]);

            vector<string> onelist;
            for(int j = 0; j < spec->materialSpeciesNames[i].numNames; ++j)
                onelist.push_back(spec->materialSpeciesNames[i].names[j]);
            speciesNames.push_back(onelist);
        }

        int  nz = 1;
        for (int i = 0 ; i < spec->ndims ; i++)
            nz *= spec->dims[i];

        species = new avtSpecies(numSpecies,
                                 speciesNames,
                                 nz,
                                 spec->species.iArray,
                                 spec->nmixedSpecies,
                                 spec->mixedSpecies.iArray,
                                 spec->nspeciesMF,
                                 spec->speciesMF.fArray);
    }

    VisIt_SpeciesData_free(spec);

    return species;
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
//  Programmer:  Jeremy Meredith
//  Creation:    May 9, 2005
//
//  Modifications:
//    Brad Whitlock, Tue Feb 10 11:24:20 PST 2009
//    Added code to free the domain list.
//
// ****************************************************************************

void
avtSimV2FileFormat::PopulateIOInformation(avtIOInformation& ioInfo)
{
#ifndef MDSERVER
    if (!cb.GetDomainList)
        return;

    VisIt_DomainList *dl = cb.GetDomainList();
    if (!dl)
        return;

    int rank = 0;
    int size = 1;
#ifdef PARALLEL
    rank = PAR_Rank();
    size = PAR_Size();
#endif

    vector< vector<int> > hints;
    hints.resize(size);
    hints[rank].resize(dl->nMyDomains);
    for (int i=0; i<dl->nMyDomains; i++)
    {
        hints[rank][i] = dl->myDomains.iArray[i];
    }
    ioInfo.AddHints(hints);

    ioInfo.SetNDomains(dl->nTotalDomains);

    VisIt_DomainList_free(dl);
#endif
}
