// ************************************************************************* //
//                            avtSimV1FileFormat.C                           //
// ************************************************************************* //

#include <avtSimV1FileFormat.h>

#include <string>
#include <vector>

#include <avtDatabaseMetaData.h>
#include <avtSimulationInformation.h>
#include <avtSimulationCommandSpecification.h>

#include <InvalidFilesException.h>

#include <visitstream.h>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

using std::string;
using std::vector;


#ifndef MDSERVER
extern "C" VisIt_SimulationCallback visitCallbacks;
#endif

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

        // FILL IN GROUP NAME STUFF
        vector<int> groupIds(mesh->numGroups);

        // OH, and units too; in fact, double check everything

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

    md->Print(cout);

    for (int c=0; c<vsmd->numCommands; c++)
    {
        VisIt_SimulationControlCommand *scc = &vsmd->commands[c];
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
        avtSimulationInformation simInfo = md->GetSimInfo();
        avtSimulationCommandSpecification *scs = new avtSimulationCommandSpecification;
        scs->SetName(scc->name);
        scs->SetArgumentType(t);
        simInfo.AddAvtSimulationCommandSpecification(*scs);
        md->SetSimInfo(simInfo);
    }

    // DO THE MATERIALS, EXPRESSIONS, CURVES, ETC.
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
// ****************************************************************************
vtkDataSet *
avtSimV1FileFormat::GetMesh(int domain, const char *meshname)
{
#ifdef MDSERVER
    return NULL;
#else
    if (!cb.GetMesh)
        return NULL;

    VisIt_MeshData *vmesh = cb.GetMesh(domain, meshname);
    switch (vmesh->meshType)
    {
      case VISIT_MESHTYPE_CURVILINEAR:
        {
            VisIt_CurvilinearMesh *cmesh = vmesh->cmesh;
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

            // USE THE BASEINDEX AND REAL/GHOST ZONE STUFF!

            int npts = 0;
            for (int i=0; i<ni; i++)
            {
                for (int j=0; j<nj; j++)
                {
                    for (int k=0; k<nk; k++)
                    {
                        pts[npts*3 + 0] = cmesh->xcoords[i];
                        pts[npts*3 + 1] = cmesh->ycoords[j];
                        pts[npts*3 + 2] = cmesh->zcoords[k];
                        npts++;
                    }
                }
            }

            return sgrid;
        }
        break;
      default:
        EXCEPTION1(ImproperUseException,
                   "Only curvilinear meshes are currently supported.\n");
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
    if (sd->len<=0 || !sd->data)
        return NULL;

     vtkFloatArray *array = vtkFloatArray::New();
     array->SetNumberOfTuples(sd->len);
     for (int i=0; i<sd->len; i++)
     {
         array->SetTuple1(i, sd->data[i]);
     }
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
