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
#include <vtkCellArray.h>
#include <vtkPolyData.h>

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
//  Modifications:
//    Jeremy Meredith, Thu Apr 14 16:48:04 PDT 2005
//    Fixed groups.  Added curves and materials.
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

        mesh->groupIds.resize(mesh->numGroups);
        for (int g = 0; g<mesh->numGroups; g++)
        {
            mesh->groupIds[g] = mmd->groupIds[g];
        }

        mesh->xLabel = mmd->xLabel ? mmd->xLabel : "";
        mesh->yLabel = mmd->yLabel ? mmd->yLabel : "";
        mesh->zLabel = mmd->zLabel ? mmd->zLabel : "";

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

    md->Print(cout);

    // DO THE EXPRESSIONS, ETC.
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
                        pts[npts*3 + 0] = cmesh->xcoords[npts];
                        pts[npts*3 + 1] = cmesh->ycoords[npts];
                        pts[npts*3 + 2] = cmesh->zcoords[npts];
                        npts++;
                    }
                }
            }

            return sgrid;
        }
        break;
      case VISIT_MESHTYPE_RECTILINEAR:
        {
            VisIt_RectilinearMesh *rmesh = vmesh->rmesh;
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

            // USE THE BASEINDEX AND REAL/GHOST ZONE STUFF!

            vtkFloatArray *xcoords;
            vtkFloatArray *ycoords;
            vtkFloatArray *zcoords;

            xcoords = vtkFloatArray::New();
            xcoords->SetNumberOfTuples(ni);
            for (int i=0; i<ni; i++)
            {
                xcoords->SetComponent(i, 0, rmesh->xcoords[i]);
            }

            ycoords = vtkFloatArray::New();
            ycoords->SetNumberOfTuples(nj);
            for (int j=0; j<nj; j++)
            {
                ycoords->SetComponent(j, 0, rmesh->ycoords[j]);
            }

            zcoords = vtkFloatArray::New();
            zcoords->SetNumberOfTuples(nk);
            for (int k=0; k<nk; k++)
            {
                zcoords->SetComponent(k, 0, rmesh->zcoords[k]);
            }

            rgrid->SetXCoordinates(xcoords);
            xcoords->Delete();
            rgrid->SetYCoordinates(ycoords);
            ycoords->Delete();
            rgrid->SetZCoordinates(zcoords);
            zcoords->Delete();

            return rgrid;
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
// ****************************************************************************
avtMaterial *
avtSimV1FileFormat::GetMaterial(int domain, const char *varname)
{
    if (!cb.GetMaterial)
        return NULL;

    VisIt_MaterialData *md = cb.GetMaterial(domain,varname);
    if (!md)
        return NULL;

    vector<string> matNames(md->nMaterials);
    for (int m=0; m<md->nMaterials; m++)
        matNames[m] = md->materialNames[m];

    avtMaterial *mat = new avtMaterial(md->nMaterials,
                                       matNames,
                                       md->nzones,
                                       md->matlist,
                                       md->mixlen,
                                       md->mix_mat,
                                       md->mix_next,
                                       md->mix_zone,
                                       md->mix_vf);

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
    for (int j=0; j<npts; j++)
    {
        pts->SetPoint(j, cd->x[j], cd->y[j], 0);
    }

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
