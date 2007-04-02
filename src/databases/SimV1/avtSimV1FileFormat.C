/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

// ************************************************************************* //
//                            avtSimV1FileFormat.C                           //
// ************************************************************************* //

#include <avtSimV1FileFormat.h>

#include <string>
#include <vector>

#include <avtDatabaseMetaData.h>
#include <avtGhostData.h>
#include <avtIOInformation.h>
#include <avtSimulationInformation.h>
#include <avtSimulationCommandSpecification.h>

#include <Expression.h>
#include <DebugStream.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>

#include <visitstream.h>
                                                    
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkIdTypeArray.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

using std::string;
using std::vector;

#ifdef PARALLEL
#include <mpi.h>
#include <avtParallel.h>
#endif

#ifndef MDSERVER
extern "C" VisIt_SimulationCallback visitCallbacks;
#endif


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
static void FreeDataArray(VisIt_DataArray &da)
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
//  Function:  GetQuadGhostZones
//
//  Purpose:  Add ghost zone information to a dataset.
//    
//  Note:  stolen from the Silo file format method of the same name
//
// ****************************************************************************
static void 
GetQuadGhostZones(int nnodes, int ndims,
                  int *dims, int *min_index, int *max_index,
                  vtkDataSet *ds)
{
    //
    //  Determine if we have ghost points
    //
    int first[3];
    int last[3];
    bool ghostPresent = false;
    bool badIndex = false;
    for (int i = 0; i < 3; i++)
    {
        first[i] = (i < ndims ? min_index[i] : 0);
        last[i]  = (i < ndims ? max_index[i] : 0);

        if (first[i] < 0 || first[i] >= dims[i])
        {
            debug1 << "bad Index on first[" << i << "] dims is: " 
                   << dims[i] << endl;
            badIndex = true;
        }

        if (last[i] < 0 || last[i] >= dims[i])
        {
            debug1 << "bad Index on last[" << i << "] dims is: " 
                   << dims[i] << endl;
            badIndex = true;
        }

        if (first[i] != 0 || last[i] != dims[i] -1)
        {
            ghostPresent = true;
        }
    }

    //
    //  Create the ghost zones array if necessary
    //
    if (ghostPresent && !badIndex)
    {
        bool *ghostPoints = new bool[nnodes];
        //
        // Initialize as all ghost levels
        //
        for (int ii = 0; ii < nnodes; ii++)
            ghostPoints[ii] = true; 

        //
        // Set real values
        //
        for (int k = first[2]; k <= last[2]; k++)
            for (int j = first[1]; j <= last[1]; j++)
                for (int i = first[0]; i <= last[0]; i++)
                {
                    int index = k*dims[1]*dims[0] + j*dims[0] + i;
                    ghostPoints[index] = false; 
                }

        //
        //  okay, now we have ghost points, but what we really want
        //  are ghost cells ... convert:  if all points associated with
        //  cell are 'real' then so is the cell.
        //
        unsigned char realVal = 0;
        unsigned char ghostVal = 0;
        avtGhostData::AddGhostZoneType(ghostVal, 
                                       DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
        int ncells = ds->GetNumberOfCells();
        vtkIdList *ptIds = vtkIdList::New();
        vtkUnsignedCharArray *ghostCells = vtkUnsignedCharArray::New();
        ghostCells->SetName("avtGhostZones");
        ghostCells->Allocate(ncells);
 
        for (int i = 0; i < ncells; i++)
        {
            ds->GetCellPoints(i, ptIds);
            bool ghost = false;
            for (int idx = 0; idx < ptIds->GetNumberOfIds(); idx++)
                ghost |= ghostPoints[ptIds->GetId(idx)];

            if (ghost)
                ghostCells->InsertNextValue(ghostVal);
            else
                ghostCells->InsertNextValue(realVal);
 
        } 
        ds->GetCellData()->AddArray(ghostCells);
        delete [] ghostPoints;
        ghostCells->Delete();
        ptIds->Delete();

        vtkIntArray *realDims = vtkIntArray::New();
        realDims->SetName("avtRealDims");
        realDims->SetNumberOfValues(6);
        realDims->SetValue(0, first[0]);
        realDims->SetValue(1, last[0]);
        realDims->SetValue(2, first[1]);
        realDims->SetValue(3, last[1]);
        realDims->SetValue(4, first[2]);
        realDims->SetValue(5, last[2]);
        ds->GetFieldData()->AddArray(realDims);
        ds->GetFieldData()->CopyFieldOn("avtRealDims");
        realDims->Delete();

        ds->SetUpdateGhostLevel(0);
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
// ****************************************************************************

avtSimV1FileFormat::avtSimV1FileFormat(const char *filename)
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
    cb = visitCallbacks;
#endif
}

// ****************************************************************************
//  Method: avtSimV1FileFormat::FreeUpResources
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
avtSimV1FileFormat::FreeUpResources(void)
{
}


// ****************************************************************************
//  Method: avtSimV1FileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Fill the simulation metadata with the parameters from the file for
//      the mdserver.  Get the info from the simulation for the engine.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 14, 2005
//
//  Modifications:
//    Jeremy Meredith, Thu Apr 14 16:48:04 PDT 2005
//    Fixed groups.  Added curves and materials.
//
//    Jeremy Meredith, Thu Apr 28 17:59:48 PDT 2005
//    Added cycle and time.  Fixed labels.  Added control command enabling.
//
//    Shelly Prevost, Tue Jan 24 17:49:11 PST 2006
//    Added the ability to send more general commands to the simulation.
//
//    Brad Whitlock, Wed Mar 1 15:46:40 PST 2006
//    I added expressions to the metadata.
//
//    Shelly Prevost, Tue Sep 12 15:42:17 PDT 2006
//    I added code to copy all the fields in the
//    generic command array. This is now needed because
//    simulation will be control the main simulation window
//    in a very simular way as the custom simulation window
//    works.
//
//    Brad Whitlock, Fri Nov 3 10:29:21 PDT 2006
//    Fixed memory leak.
//
// ****************************************************************************

void
avtSimV1FileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
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
          default:
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
            EXCEPTION1(ImproperUseException,
                       "Invalid centering type in VisIt_ScalarMetaData.");
        }
        scalar->treatAsASCII = smd->treatAsASCII;
        scalar->hasDataExtents = false;
        scalar->hasUnits = false;

        AddScalarVarToMetaData(md, smd->name, smd->meshName, (avtCentering)(smd->centering));
        //md->Add(scalar);
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
            EXCEPTION1(ImproperUseException,
                       "Invalid command argument type in "
                       "VisIt_SimulationControlCommand.");
        }
        avtSimulationCommandSpecification scs;
        scs.SetName(scc->name);
        scs.SetText(scc->text);
        scs.SetValue(scc->value);
        scs.SetIsOn(scc->isOn);
        scs.SetUiType(scc->uiType);
        scs.SetClassName(scc->className);
        scs.SetSignal(scc->signal);
        scs.SetArgumentType(t);
        scs.SetEnabled(scc->enabled);
        md->GetSimInfo().AddAvtSimulationCommandSpecification(scs);
    }
 
    for (int c=0; c<vsmd->numCustomCommands; c++)
    {
        VisIt_SimulationControlCommand *scc = &vsmd->customCommands[c];
        avtSimulationCommandSpecification::CommandArgumentType t;
        avtSimulationCommandSpecification scs;
        scs.SetName(scc->name);
        scs.SetText(scc->text);
        scs.SetValue(scc->value);
        scs.SetIsOn(scc->isOn);
        scs.SetUiType(scc->uiType);
        scs.SetClassName(scc->className);
        scs.SetArgumentType(t);
        scs.SetEnabled(scc->enabled);
        md->GetSimInfo().AddAvtSimulationCustCommandSpecification(scs);
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

#endif
}


// ****************************************************************************
//  Method: avtSimV1FileFormat::GetMesh
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
//  Programmer: Jeremy Meredith
//  Creation:   March 14, 2005
//
//  Modifications:
//    Jeremy Meredith, Thu Apr 21 09:26:12 PDT 2005
//    Added rectilinear and fixed curvilinear support.
//
//    Jeremy Meredith, Thu Apr 28 18:00:32 PDT 2005
//    Added true data array structures in place of raw array pointers.
//
//    Jeremy Meredith, Wed May 11 10:56:01 PDT 2005
//    Added ghost zone support.
//
//    Jeremy Meredith, Wed May 25 14:38:35 PDT 2005
//    Added unstructured mesh support.
//
//    Brad Whitlock, Fri Mar 3 09:36:52 PDT 2006
//    Added point mesh support and made it throw an exception if the 
//    simulation's mesh callback could not find the named mesh.
//
// ****************************************************************************

vtkDataSet *
avtSimV1FileFormat::GetMesh(int domain, const char *meshname)
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

    switch (vmesh->meshType)
    {
      case VISIT_MESHTYPE_CURVILINEAR:
        {
            VisIt_CurvilinearMesh *cmesh = vmesh->cmesh;

            if (!cmesh)
                return NULL;

            //
            // Create the VTK objects and connect them up.
            //
            vtkStructuredGrid    *sgrid   = vtkStructuredGrid::New(); 
            vtkPoints            *points  = vtkPoints::New();
            sgrid->SetPoints(points);
            points->Delete();

            //
            // Tell the grid what its dimensions are and populate the points array.
            //
            sgrid->SetDimensions(cmesh->dims);

            //
            // Populate the coordinates.
            //
            int ni = cmesh->dims[0];
            int nj = cmesh->dims[1];
            int nk = cmesh->dims[2];
            points->SetNumberOfPoints(ni * nj * nk);
            float *pts = (float *) points->GetVoidPointer(0);

            if (cmesh->xcoords.dataType == VISIT_DATATYPE_FLOAT)
            {
                int npts = 0;
                for (int i=0; i<ni; i++)
                {
                    for (int j=0; j<nj; j++)
                    {
                        for (int k=0; k<nk; k++)
                        {
                            pts[npts*3 + 0] = cmesh->xcoords.fArray[npts];
                            pts[npts*3 + 1] = cmesh->ycoords.fArray[npts];
                            if (cmesh->ndims==3)
                                pts[npts*3 + 2] = cmesh->zcoords.fArray[npts];
                            else
                                pts[npts*3 + 2] = 0;
                            npts++;
                        }
                    }
                }
            }
            else if (cmesh->xcoords.dataType == VISIT_DATATYPE_DOUBLE)
            {
                int npts = 0;
                for (int i=0; i<ni; i++)
                {
                    for (int j=0; j<nj; j++)
                    {
                        for (int k=0; k<nk; k++)
                        {
                            pts[npts*3 + 0] = cmesh->xcoords.dArray[npts];
                            pts[npts*3 + 1] = cmesh->ycoords.dArray[npts];
                            if (cmesh->ndims==3)
                                pts[npts*3 + 2] = cmesh->zcoords.dArray[npts];
                            else
                                pts[npts*3 + 2] = 0;
                            npts++;
                        }
                    }
                }
            }
            else
            {
                EXCEPTION1(ImproperUseException,
                           "Coordinate arrays must be float or double.\n");
            }

            FreeDataArray(cmesh->xcoords);
            FreeDataArray(cmesh->ycoords);
            FreeDataArray(cmesh->zcoords);

            GetQuadGhostZones(ni*nj*nk,
                              cmesh->ndims,
                              cmesh->dims,
                              cmesh->minRealIndex,
                              cmesh->maxRealIndex,
                              sgrid);

            vtkIntArray *arr = vtkIntArray::New();
            arr->SetNumberOfTuples(3);
            arr->SetValue(0, cmesh->baseIndex[0]);
            arr->SetValue(1, cmesh->baseIndex[1]);
            arr->SetValue(2, cmesh->baseIndex[2]);
            arr->SetName("base_index");
            sgrid->GetFieldData()->AddArray(arr);
            arr->Delete();

            return sgrid;
        }
        break;
      case VISIT_MESHTYPE_RECTILINEAR:
        {
            VisIt_RectilinearMesh *rmesh = vmesh->rmesh;

            if (!rmesh)
                return NULL;

            //
            // Create the VTK objects and connect them up.
            //
            vtkRectilinearGrid   *rgrid   = vtkRectilinearGrid::New(); 

            //
            // Tell the grid what its dimensions are and populate the points array.
            //
            rgrid->SetDimensions(rmesh->dims);

            //
            // Populate the coordinates.
            //
            int ni = rmesh->dims[0];
            int nj = rmesh->dims[1];
            int nk = rmesh->dims[2];

            vtkFloatArray *xcoords;
            vtkFloatArray *ycoords;
            vtkFloatArray *zcoords;

            xcoords = vtkFloatArray::New();
            xcoords->SetNumberOfTuples(ni);
            ycoords = vtkFloatArray::New();
            ycoords->SetNumberOfTuples(nj);
            zcoords = vtkFloatArray::New();
            zcoords->SetNumberOfTuples(nk);

            if (rmesh->xcoords.dataType == VISIT_DATATYPE_FLOAT)
            {
                for (int i=0; i<ni; i++)
                    xcoords->SetComponent(i, 0, rmesh->xcoords.fArray[i]);

                for (int j=0; j<nj; j++)
                    ycoords->SetComponent(j, 0, rmesh->ycoords.fArray[j]);

                if (rmesh->ndims==3)
                {
                    for (int k=0; k<nk; k++)
                        zcoords->SetComponent(k, 0, rmesh->zcoords.fArray[k]);
                }
                else
                {
                    for (int k=0; k<nk; k++)
                        zcoords->SetComponent(k, 0, 0);
                }
            }
            else if (rmesh->xcoords.dataType == VISIT_DATATYPE_DOUBLE)
            {
                for (int i=0; i<ni; i++)
                    xcoords->SetComponent(i, 0, rmesh->xcoords.dArray[i]);

                for (int j=0; j<nj; j++)
                    ycoords->SetComponent(j, 0, rmesh->ycoords.dArray[j]);

                if (rmesh->ndims==3)
                {
                    for (int k=0; k<nk; k++)
                        zcoords->SetComponent(k, 0, rmesh->zcoords.dArray[k]);
                }
                else
                {
                    for (int k=0; k<nk; k++)
                        zcoords->SetComponent(k, 0, 0);
                }
            }
            else
            {
                EXCEPTION1(ImproperUseException,
                           "Coordinate arrays must be float or double.\n");
            }

            FreeDataArray(rmesh->xcoords);
            FreeDataArray(rmesh->ycoords);
            FreeDataArray(rmesh->zcoords);

            rgrid->SetXCoordinates(xcoords);
            xcoords->Delete();
            rgrid->SetYCoordinates(ycoords);
            ycoords->Delete();
            rgrid->SetZCoordinates(zcoords);
            zcoords->Delete();

            GetQuadGhostZones(ni*nj*nk,
                              rmesh->ndims,
                              rmesh->dims,
                              rmesh->minRealIndex,
                              rmesh->maxRealIndex,
                              rgrid);

            vtkIntArray *arr = vtkIntArray::New();
            arr->SetNumberOfTuples(3);
            arr->SetValue(0, rmesh->baseIndex[0]);
            arr->SetValue(1, rmesh->baseIndex[1]);
            arr->SetValue(2, rmesh->baseIndex[2]);
            arr->SetName("base_index");
            rgrid->GetFieldData()->AddArray(arr);
            arr->Delete();

            return rgrid;
        }
        break;
      case VISIT_MESHTYPE_UNSTRUCTURED:
        {
            VisIt_UnstructuredMesh *umesh = vmesh->umesh;

            if (!umesh)
                return NULL;


            if (umesh->connectivity.dataType != VISIT_DATATYPE_INT)
            {
                EXCEPTION1(ImproperUseException,
                           "Connectivity array must be ints.");
            }

            vtkUnstructuredGrid  *ugrid = vtkUnstructuredGrid::New();
            vtkPoints            *points  = vtkPoints::New();
            ugrid->SetPoints(points);
            points->Delete();

            //
            // Populate the coordinates.
            //
            int npts = umesh->nnodes;
            points->SetNumberOfPoints(npts);
            float *pts = (float *) points->GetVoidPointer(0);

            if (umesh->xcoords.dataType == VISIT_DATATYPE_FLOAT)
            {
                for (int i=0; i<npts; i++)
                {
                    pts[i*3 + 0] = umesh->xcoords.fArray[i];
                    pts[i*3 + 1] = umesh->ycoords.fArray[i];
                    if (umesh->ndims==3)
                        pts[i*3 + 2] = umesh->zcoords.fArray[i];
                    else
                        pts[i*3 + 2] = 0;
                }
            }
            else if (umesh->xcoords.dataType == VISIT_DATATYPE_DOUBLE)
            {
                for (int i=0; i<npts; i++)
                {
                    pts[i*3 + 0] = umesh->xcoords.dArray[i];
                    pts[i*3 + 1] = umesh->ycoords.dArray[i];
                    if (umesh->ndims==3)
                        pts[i*3 + 2] = umesh->zcoords.dArray[i];
                    else
                        pts[i*3 + 2] = 0;
                }
            }
            else
            {
                EXCEPTION1(ImproperUseException,
                           "Coordinate arrays must be float or double.\n");
            }

            int celltype_npts[10];
            celltype_npts[VISIT_CELL_BEAM]  = 2;
            celltype_npts[VISIT_CELL_TRI]   = 3;
            celltype_npts[VISIT_CELL_QUAD]  = 4;
            celltype_npts[VISIT_CELL_TET]   = 4;
            celltype_npts[VISIT_CELL_PYR]   = 5;
            celltype_npts[VISIT_CELL_WEDGE] = 6;
            celltype_npts[VISIT_CELL_HEX]   = 8;

            int celltype_idtype[10];
            celltype_idtype[VISIT_CELL_BEAM]  = VTK_LINE;
            celltype_idtype[VISIT_CELL_TRI]   = VTK_TRIANGLE;
            celltype_idtype[VISIT_CELL_QUAD]  = VTK_QUAD;
            celltype_idtype[VISIT_CELL_TET]   = VTK_TETRA;
            celltype_idtype[VISIT_CELL_PYR]   = VTK_PYRAMID;
            celltype_idtype[VISIT_CELL_WEDGE] = VTK_WEDGE;
            celltype_idtype[VISIT_CELL_HEX]   = VTK_HEXAHEDRON;

            vtkIdTypeArray *nlist = vtkIdTypeArray::New();
            nlist->SetNumberOfValues(umesh->connectivityLen);
            vtkIdType *nl = nlist->GetPointer(0);

            vtkUnsignedCharArray *cellTypes = vtkUnsignedCharArray::New();
            cellTypes->SetNumberOfValues(umesh->nzones);
            unsigned char *ct = cellTypes->GetPointer(0);

            vtkIdTypeArray *cellLocations = vtkIdTypeArray::New();
            cellLocations->SetNumberOfValues(umesh->nzones);
            int *cl = cellLocations->GetPointer(0);

            int numCells = 0;
            int offset = 0;
            while (offset < umesh->connectivityLen)
            {
                int celltype = umesh->connectivity.iArray[offset];

                int vtktype = celltype_idtype[celltype];
                int nelempts = celltype_npts[celltype];
                *ct++ = vtktype;
                *nl++ = nelempts;

                for (int j=0; j<nelempts; j++)
                {
                    *nl++ = umesh->connectivity.iArray[offset+1+j];
                }

                numCells++;
                *cl++ = offset;
                offset += nelempts+1;
            }

            if (numCells != umesh->nzones)
            {
                EXCEPTION1(ImproperUseException,
                           "Number of zones and length of connectivity "
                           "array did not match!");
            }

            vtkCellArray *cells = vtkCellArray::New();
            cells->SetCells(umesh->nzones, nlist);
            nlist->Delete();

            ugrid->SetCells(cellTypes, cellLocations, cells);
            cellTypes->Delete();
            cellLocations->Delete();
            cells->Delete();

            int firstCell = umesh->firstRealZone;
            int lastCell  = umesh->lastRealZone;
            if (firstCell == 0 && lastCell == 0 )
            {
                debug5 << "Cannot tell if ghost zones are present because "
                       << "min_index & max_index are both zero!" << endl;
            }
            else if (firstCell < 0 || firstCell >= numCells ||
                     lastCell  < 0 || lastCell  >= numCells ||
                     firstCell > lastCell)
            {
                // bad min or max index
                debug5 << "Invalid min/max index for determining ghost zones:  "
                       << "\n\tnumCells: " << numCells
                       << "\n\tfirstRealZone: " << firstCell
                       << "\n\tlastRealZone: " << lastCell << endl;
            }
            else if (firstCell != 0 || lastCell != numCells -1)
            {
                int i;
                vtkUnsignedCharArray *ghostZones = vtkUnsignedCharArray::New();
                ghostZones->SetName("avtGhostZones");
                ghostZones->SetNumberOfTuples(numCells);
                unsigned char *tmp = ghostZones->GetPointer(0);
                for (i = 0; i < firstCell; i++)
                {
                    //
                    //  ghostZones at the begining of the zone list
                    //
                    unsigned char val = 0;
                    avtGhostData::AddGhostZoneType(val, 
                                           DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
                    *tmp++ = val;
                }
                for (i = firstCell; i <= lastCell; i++)
                {
                    //
                    // real zones
                    //
                    *tmp++ = 0;
                }
                for (i = lastCell+1; i < numCells; i++)
                {
                    //
                    //  ghostZones at the end of the zone list
                    //
                    unsigned char val = 0;
                    avtGhostData::AddGhostZoneType(val, 
                                           DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
                    *tmp++ = val;
                }
                ugrid->GetCellData()->AddArray(ghostZones);
                ghostZones->Delete();
                ugrid->SetUpdateGhostLevel(0);
            }

            return ugrid;
        }
        break;
      case VISIT_MESHTYPE_POINT:
        {
            VisIt_PointMesh *pmesh = vmesh->pmesh;

            if (!pmesh)
                return NULL;


            vtkPolyData  *pd = vtkPolyData::New();
            vtkPoints    *points  = vtkPoints::New();
            pd->SetPoints(points);
            points->Delete();

            //
            // Populate the coordinates.
            //
            int npts = pmesh->nnodes;
            points->SetNumberOfPoints(npts);
            float *pts = (float *) points->GetVoidPointer(0);
            pd->Allocate(npts);
            vtkIdType onevertex[1];
            if (pmesh->xcoords.dataType == VISIT_DATATYPE_FLOAT)
            {
                for (int i=0; i<npts; i++)
                {
                    pts[i*3 + 0] = pmesh->xcoords.fArray[i];
                    pts[i*3 + 1] = pmesh->ycoords.fArray[i];
                    if (pmesh->ndims==3)
                        pts[i*3 + 2] = pmesh->zcoords.fArray[i];
                    else
                        pts[i*3 + 2] = 0.f;

                    onevertex[0] = i;
                    pd->InsertNextCell(VTK_VERTEX, 1, onevertex);
                }
            }
            else if (pmesh->xcoords.dataType == VISIT_DATATYPE_DOUBLE)
            {
                for (int i=0; i<npts; i++)
                {
                    pts[i*3 + 0] = pmesh->xcoords.dArray[i];
                    pts[i*3 + 1] = pmesh->ycoords.dArray[i];
                    if (pmesh->ndims==3)
                        pts[i*3 + 2] = pmesh->zcoords.dArray[i];
                    else
                        pts[i*3 + 2] = 0.f;

                    onevertex[0] = i;
                    pd->InsertNextCell(VTK_VERTEX, 1, onevertex);
                }
            }
            else
            {
                EXCEPTION1(ImproperUseException,
                           "Coordinate arrays must be float or double.\n");
            }

            FreeDataArray(pmesh->xcoords);
            FreeDataArray(pmesh->ycoords);
            FreeDataArray(pmesh->zcoords);

            return pd;
        }
        break;
      default:
        EXCEPTION1(ImproperUseException,
                   "You've tried to use an unsupported mesh type.\n");
        break;
    }

    return NULL;
   
#endif
}


// ****************************************************************************
//  Method: avtSimV1FileFormat::GetVar
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
// ****************************************************************************

vtkDataArray *
avtSimV1FileFormat::GetVar(int domain, const char *varname)
{
#ifdef MDSERVER
    return NULL;
#else
    if (!cb.GetScalar)
        return NULL;

    VisIt_ScalarData *sd = cb.GetScalar(domain,varname);
    if (!sd || sd->len<=0)
        return NULL;

     vtkFloatArray *array = vtkFloatArray::New();
     array->SetNumberOfTuples(sd->len);
     if (sd->data.dataType == VISIT_DATATYPE_FLOAT)
     {
         for (int i=0; i<sd->len; i++)
         {
             array->SetTuple1(i, sd->data.fArray[i]);
         }
     }
     else if (sd->data.dataType == VISIT_DATATYPE_DOUBLE)
     {
         for (int i=0; i<sd->len; i++)
         {
             array->SetTuple1(i, sd->data.dArray[i]);
         }
     }
     else if (sd->data.dataType == VISIT_DATATYPE_INT)
     {
         for (int i=0; i<sd->len; i++)
         {
             array->SetTuple1(i, sd->data.iArray[i]);
         }
     }
     else // (sd->data.dataType == VISIT_DATATYPE_CHAR)
     {
         for (int i=0; i<sd->len; i++)
         {
             array->SetTuple1(i, sd->data.cArray[i]);
         }
     }

     FreeDataArray(sd->data);

     return array;
#endif
}


// ****************************************************************************
//  Method: avtSimV1FileFormat::GetVectorVar
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
avtSimV1FileFormat::GetVectorVar(int domain, const char *varname)
{
    return NULL;
}

// ****************************************************************************
//  Method:  avtSimV1FileFormat::GetAuxiliaryData
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
//    Jeremy Meredith, Thu Apr 28 18:00:32 PDT 2005
//    Added true data array structures in place of raw array pointers.
//
// ****************************************************************************
void *
avtSimV1FileFormat::GetAuxiliaryData(const char *var, int domain,
                              const char *type, void *, DestructorFunction &df)
{
    void *rv = NULL;
    if (strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        rv = (void *) GetMaterial(domain, var);
        df = avtMaterial::Destruct;
    }
    /*
    else if (strcmp(type, AUXILIARY_DATA_SPECIES) == 0)
    {
        rv = (void *) GetSpecies(domain, var);
        df = avtSpecies::Destruct;
    }
    */

    //
    // Note -- may want to do mixed variables here
    //

    return rv;
}

// ****************************************************************************
//  Method:  avtSimV1FileFormat::GetMaterial
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
// ****************************************************************************
avtMaterial *
avtSimV1FileFormat::GetMaterial(int domain, const char *varname)
{
    if (!cb.GetMaterial)
        return NULL;

    VisIt_MaterialData *md = cb.GetMaterial(domain,varname);
    if (!md)
        return NULL;

    if (md->matlist.dataType != VISIT_DATATYPE_INT)
    {
        EXCEPTION1(ImproperUseException,
                   "matlist array must be integers");
    }

    vector<string> matNames(md->nMaterials);
    for (int m=0; m<md->nMaterials; m++)
        matNames[m] = md->materialNames[m];

    avtMaterial *mat = new avtMaterial(md->nMaterials,
                                       matNames,
                                       md->nzones,
                                       md->matlist.iArray,
                                       md->mixlen,
                                       md->mix_mat,
                                       md->mix_next,
                                       md->mix_zone,
                                       md->mix_vf);

    FreeDataArray(md->matlist);

    return mat;
}

// ****************************************************************************
//  Method:  avtSimV1FileFormat::GetMaterial
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
// ****************************************************************************

vtkDataSet *
avtSimV1FileFormat::GetCurve(const char *name)
{
    if (!cb.GetCurve)
        return NULL;

    VisIt_CurveData *cd = cb.GetCurve(name);
    if (!cd)
        return NULL;

    vtkPolyData *pd  = vtkPolyData::New();
    vtkPoints   *pts = vtkPoints::New();
    pd->SetPoints(pts);
    int npts = cd->len;
    pts->SetNumberOfPoints(npts);

    float *xpts = new float[npts];
    if (cd->x.dataType == VISIT_DATATYPE_INT)
    {
        for (int j=0; j<npts; j++)
            xpts[j] = cd->x.iArray[j];
    }
    else if (cd->x.dataType == VISIT_DATATYPE_FLOAT)
    {
        for (int j=0; j<npts; j++)
            xpts[j] = cd->x.fArray[j];
    }
    else if (cd->x.dataType == VISIT_DATATYPE_DOUBLE)
    {
        for (int j=0; j<npts; j++)
            xpts[j] = cd->x.dArray[j];
    }
    else
    {
        EXCEPTION1(ImproperUseException, "Curve coordinate arrays "
                   "must be float, double, or int in X.\n");
    }

    if (cd->y.dataType == VISIT_DATATYPE_FLOAT)
    {
        for (int j=0; j<npts; j++)
        {
            pts->SetPoint(j, xpts[j], cd->y.fArray[j], 0);
        }
    }
    else if (cd->y.dataType == VISIT_DATATYPE_DOUBLE)
    {
        for (int j=0; j<npts; j++)
        {
            pts->SetPoint(j, xpts[j], cd->y.dArray[j], 0);
        }
    }
    else
    {
        EXCEPTION1(ImproperUseException,
                   "Curve coordinate arrays must be float or double in Y.\n");
    }

    delete[] xpts;

    FreeDataArray(cd->x);
    FreeDataArray(cd->y);

    vtkCellArray *line = vtkCellArray::New();
    pd->SetLines(line);
    for (int k = 1 ; k < npts ; k++)
    {
        line->InsertNextCell(2);
        line->InsertCellPoint(k-1);
        line->InsertCellPoint(k);
    }

    pts->Delete();
    line->Delete();

    return pd;
}

// ****************************************************************************
//  Method:  avtSimV1FileFormat::PopulateIOInformation
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
// ****************************************************************************
void
avtSimV1FileFormat::PopulateIOInformation(avtIOInformation& ioInfo)
{
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
}
