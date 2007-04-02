// ************************************************************************* //
//                            avtFLASHFileFormat.C                           //
// ************************************************************************* //

#include <avtFLASHFileFormat.h>

#include <string>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCellType.h>

#include <float.h>

#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <avtVariableCache.h>
#include <avtStructuredDomainBoundaries.h>
#include <avtStructuredDomainNesting.h>
#include <avtIntervalTree.h>

#include <Expression.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>

using     std::vector;
using     std::string;


// ****************************************************************************
//  Function:  GetNiceParticleName
//
//  Purpose:
//    Nicely group particle names into a subdirectory.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 25, 2005
//
// ****************************************************************************
static string GetNiceParticleName(const string &varname)
{
    string nicename = varname;
    if (nicename.length() > 9 && nicename.substr(0,9) == "particle_")
    {
        nicename = string("Particles/") + nicename.substr(9);
    }
    else
    {
        nicename = string("Particles/") + nicename;
    }
    return nicename;
}

// ****************************************************************************
//  Method: avtFLASH constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 23, 2005
//
// ****************************************************************************

avtFLASHFileFormat::avtFLASHFileFormat(const char *cfilename)
    : avtSTMDFileFormat(&cfilename, 1)
{
    filename  = cfilename;
    fileId    = -1;
    dimension = 0;
    numBlocks = 0;
    numLevels = 0;
}


// ****************************************************************************
//  Method: avtFLASHFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 23, 2005
//
//  Modifications:
//    Jeremy Meredith, Thu Aug 25 15:07:36 PDT 2005
//    Added particle support.
//
// ****************************************************************************

void
avtFLASHFileFormat::FreeUpResources(void)
{
    if (fileId >= 0)
    {
        H5Fclose(fileId);
        fileId = -1;
    }
    blocks.clear();
    varNames.clear();
    particleVarNames.clear();
    particleVarTypes.clear();
    particleOriginalIndexMap.clear();
}


// ****************************************************************************
//  Method: avtFLASHFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 23, 2005
//
//  Modifications:
//    Jeremy Meredith, Thu Aug 25 15:07:36 PDT 2005
//    Added particle support.
//
// ****************************************************************************

void
avtFLASHFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    ReadAllMetaData();

    if (!avtDatabase::OnlyServeUpMetaData())
        BuildDomainNesting();

    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "mesh";
    mesh->originalName = "mesh";

    mesh->meshType = AVT_RECTILINEAR_MESH;
    mesh->topologicalDimension = dimension;
    mesh->spatialDimension = dimension;
    mesh->blockOrigin = 1;
    mesh->groupOrigin = 1;

    mesh->hasSpatialExtents = true;
    mesh->minSpatialExtents[0] = minSpatialExtents[0];
    mesh->maxSpatialExtents[0] = maxSpatialExtents[0];
    mesh->minSpatialExtents[1] = minSpatialExtents[1];
    mesh->maxSpatialExtents[1] = maxSpatialExtents[1];
    mesh->minSpatialExtents[2] = minSpatialExtents[2];
    mesh->maxSpatialExtents[2] = maxSpatialExtents[2];

    // spoof a group/domain mesh for the AMR hierarchy
    mesh->numBlocks = numBlocks;
    mesh->blockTitle = "Blocks";
    mesh->blockPieceName = "block";
    mesh->numGroups = numLevels;
    mesh->groupTitle = "Levels";
    mesh->groupPieceName = "level";
    mesh->numGroups = numLevels;
    vector<int> groupIds(numBlocks);
    vector<string> pieceNames(numBlocks);
    for (int i = 0; i < numBlocks; i++)
    {
        char tmpName[64];
        sprintf(tmpName,"level%d,block%d",blocks[i].level, blocks[i].ID);
        groupIds[i] = blocks[i].level-1;
        pieceNames[i] = tmpName;
    }
    mesh->blockNames = pieceNames;
    mesh->groupIds = groupIds;
    md->Add(mesh);


    // grid variables
    for (int v = 0 ; v < varNames.size(); v++)
    {
        AddScalarVarToMetaData(md, varNames[v], "mesh", AVT_ZONECENT);
    }

    // particles
    if (numParticles > 0)
    {
        // particle mesh
        avtMeshMetaData *pmesh = new avtMeshMetaData;
        pmesh->name = "particles";
        pmesh->originalName = "particles";

        pmesh->meshType = AVT_POINT_MESH;
        pmesh->topologicalDimension = 0;
        pmesh->spatialDimension = dimension;
        pmesh->cellOrigin = 1;
        pmesh->hasSpatialExtents = false;
        pmesh->numBlocks = 1;
        pmesh->numGroups = 0;
        md->Add(pmesh);

        // particle variables
        for (int pv = 0 ; pv < particleVarNames.size(); pv++)
        {
            AddScalarVarToMetaData(md,
                                   GetNiceParticleName(particleVarNames[pv]),
                                   "particles", AVT_NODECENT);
        }

    }

    // Populate cycle and time
    md->SetCycle(timestep, simParams.nsteps);
    md->SetTime(timestep, simParams.time);
    md->SetCycleIsAccurate(true, timestep);
    md->SetTimeIsAccurate(true, timestep);
}


// ****************************************************************************
//  Method: avtFLASHFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 23, 2005
//
//  Modifications:
//    Jeremy Meredith, Thu Aug 25 15:07:36 PDT 2005
//    Added particle support.
//
// ****************************************************************************

vtkDataSet *
avtFLASHFileFormat::GetMesh(int domain, const char *meshname)
{
    ReadAllMetaData();

    if (string(meshname) == "mesh")
    {
        // rectilinear mesh
        vtkFloatArray  *coords[3];
        int i;
        for (i = 0 ; i < 3 ; i++)
        {
            coords[i] = vtkFloatArray::New();
            coords[i]->SetNumberOfTuples(block_ndims[i]);
            for (int j = 0 ; j < block_ndims[i] ; j++)
            {
                if (i+1 > dimension)
                {
                    coords[i]->SetComponent(j, 0, 0);
                }
                else
                {
                    double minExt = blocks[domain].minSpatialExtents[i];
                    double maxExt = blocks[domain].maxSpatialExtents[i];
                    double c = minExt + double(j) *
                        (maxExt-minExt) / double(block_ndims[i]-1);
                    coords[i]->SetComponent(j, 0, c);
                }
            }
        }
   
        vtkRectilinearGrid  *rGrid = vtkRectilinearGrid::New(); 
        rGrid->SetDimensions(block_ndims);
        rGrid->SetXCoordinates(coords[0]);
        coords[0]->Delete();
        rGrid->SetYCoordinates(coords[1]);
        coords[1]->Delete();
        rGrid->SetZCoordinates(coords[2]);
        coords[2]->Delete();

        return rGrid;
    }
    else if (string(meshname) == "particles")
    {
        hid_t pointId = H5Dopen(fileId, "particle tracers");

        vtkPoints *points  = vtkPoints::New();
        points->SetNumberOfPoints(numParticles);
        float *pts = (float *) points->GetVoidPointer(0);
        int i;

        hid_t xtype = H5Tcreate(H5T_COMPOUND, sizeof(double));
        hid_t ytype = H5Tcreate(H5T_COMPOUND, sizeof(double));
        hid_t ztype = H5Tcreate(H5T_COMPOUND, sizeof(double));
        H5Tinsert(xtype, "particle_x", 0, H5T_NATIVE_DOUBLE);
        H5Tinsert(ytype, "particle_y", 0, H5T_NATIVE_DOUBLE);
        H5Tinsert(ztype, "particle_z", 0, H5T_NATIVE_DOUBLE);

        double *ddata = new double[numParticles];

        if (dimension >= 1)
        {
            H5Dread(pointId, xtype, H5S_ALL,H5S_ALL,H5P_DEFAULT, ddata);
            for (i=0; i<numParticles; i++)
                pts[i*3+0] = float(ddata[i]);
        }

        if (dimension >= 2)
        {
            H5Dread(pointId, ytype, H5S_ALL,H5S_ALL,H5P_DEFAULT, ddata);
            for (i=0; i<numParticles; i++)
                pts[i*3+1] = float(ddata[i]);
        }

        if (dimension >= 3)
        {
            H5Dread(pointId, ztype, H5S_ALL,H5S_ALL,H5P_DEFAULT, ddata);
            for (i=0; i<numParticles; i++)
                pts[i*3+2] = float(ddata[i]);
        }

        // fill in zeros
        for (int d=dimension; d<3; d++)
        {
            for (i=0; i<numParticles; i++)
                pts[i*3 + d] = 0;
        }

        H5Tclose(xtype);
        H5Tclose(ytype);
        H5Tclose(ztype);
        H5Dclose(pointId);

        vtkUnstructuredGrid  *ugrid = vtkUnstructuredGrid::New(); 
        ugrid->SetPoints(points);
        ugrid->Allocate(numParticles);
        vtkIdType onevertex;
        for(i = 0; i < numParticles; ++i)
        {
            onevertex = i;
            ugrid->InsertNextCell(VTK_VERTEX, 1, &onevertex);
        }

        points->Delete();

        return ugrid;
    }

    return NULL;
}


// ****************************************************************************
//  Method: avtFLASHFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Note: because the domains are small, we might get a speed boost by
//        leaving the varId and spaceId open for each variable, maybe even
//        caching things like the memspace
//
//  Programmer: Jeremy Meredith
//  Creation:   August 23, 2005
//
//  Modifications:
//    Jeremy Meredith, Thu Aug 25 15:07:36 PDT 2005
//    Added particle support.
//
// ****************************************************************************

vtkDataArray *
avtFLASHFileFormat::GetVar(int domain, const char *varname)
{
    ReadAllMetaData();

    if (particleOriginalIndexMap.count(varname))
    {
        //
        // It's a particle variable
        //
        int    index = particleOriginalIndexMap[varname];
        string varname = particleVarNames[index];
        hid_t  vartype = particleVarTypes[index];

        hid_t pointId = H5Dopen(fileId, "particle tracers");

        vtkFloatArray * fa = vtkFloatArray::New();
        fa->SetNumberOfTuples(numParticles);
        float *data = fa->GetPointer(0);

        if (vartype == H5T_NATIVE_DOUBLE)
        {
            hid_t h5type = H5Tcreate(H5T_COMPOUND, sizeof(double));
            H5Tinsert(h5type, varname.c_str(), 0, H5T_NATIVE_DOUBLE);

            double *ddata = new double[numParticles];
            H5Dread(pointId, h5type, H5S_ALL,H5S_ALL,H5P_DEFAULT, ddata);

            for (int i=0; i<numParticles; i++)
                data[i] = ddata[i];

            H5Tclose(h5type);
        }
        else if (vartype == H5T_NATIVE_INT)
        {
            hid_t h5type = H5Tcreate(H5T_COMPOUND, sizeof(int));
            H5Tinsert(h5type, varname.c_str(), 0, H5T_NATIVE_INT);

            int *idata = new int[numParticles];
            H5Dread(pointId, h5type, H5S_ALL,H5S_ALL,H5P_DEFAULT, idata);

            for (int i=0; i<numParticles; i++)
                data[i] = idata[i];

            H5Tclose(h5type);
        }
        else
        {
            EXCEPTION1(InvalidVariableException, varname);
        }

        H5Dclose(pointId);

        return fa;
    }
    else
    {
        //
        // It's a grid variable
        //

        hid_t varId = H5Dopen(fileId, varname);
        if (varId < 0)
        {
            EXCEPTION1(InvalidVariableException, varname);
        }

        hid_t spaceId = H5Dget_space(varId);

        hsize_t dims[4];
        hsize_t ndims = H5Sget_simple_extent_dims(spaceId, dims, NULL);

        if (ndims != 4)
        {
            EXCEPTION1(InvalidVariableException, varname);
        }

        int ntuples = dims[1]*dims[2]*dims[3];  //dims[0]==numBlocks

        //
        // Set up a data space to read the right domain
        //
        hssize_t start[5];
        hsize_t stride[5], count[5];
    
        start[0]  = domain;
        start[1]  = 0;
        start[2]  = 0;
        start[3]  = 0;
    
        stride[0] = 1;
        stride[1] = 1;
        stride[2] = 1;
        stride[3] = 1;
    
        count[0]  = 1;
        count[1]  = dims[1];
        count[2]  = dims[2];
        count[3]  = dims[3];
    
        hid_t dataspace = H5Screate_simple(4, dims, NULL);
        H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, start, stride, count, NULL);

        start[0]  = 0;
        start[1]  = 0;
        start[2]  = 0;
        start[3]  = 0;
    
        stride[0] = 1;
        stride[1] = 1;
        stride[2] = 1;
        stride[3] = 1;
    
        count[0]  = 1;
        count[1]  = dims[1];
        count[2]  = dims[2];
        count[3]  = dims[3];
    
        hid_t memspace = H5Screate_simple(4, dims, NULL);
        H5Sselect_hyperslab(memspace, H5S_SELECT_SET, start, stride, count, NULL);

        vtkFloatArray * fa = vtkFloatArray::New();
        fa->SetNumberOfTuples(ntuples);
        float *data = fa->GetPointer(0);

        double *d_data;
        int  *i_data;
        unsigned int *ui_data;
        int i;

        hid_t raw_data_type = H5Dget_type(varId);
        hid_t data_type = H5Tget_native_type(raw_data_type, H5T_DIR_ASCEND);
        if (H5Tequal(data_type, H5T_NATIVE_FLOAT)>0)
        {
            H5Dread(varId, data_type,memspace,dataspace,H5P_DEFAULT, data);
        }
        else if (H5Tequal(data_type, H5T_NATIVE_DOUBLE)>0)
        {
            d_data = new double[ntuples];
            H5Dread(varId, data_type,memspace,dataspace,H5P_DEFAULT, d_data);
            for (i = 0 ; i < ntuples ; i++)
                data[i] = d_data[i];
            delete[] d_data;
        }
        else if (H5Tequal(data_type, H5T_NATIVE_INT))
        {
            i_data = new int[ntuples];
            H5Dread(varId, data_type,memspace,dataspace,H5P_DEFAULT, i_data);
            for (i = 0 ; i < ntuples ; i++)
                data[i] = i_data[i];
            delete[] i_data;
        }
        else if (H5Tequal(data_type, H5T_NATIVE_UINT))
        {
            ui_data = new unsigned int[ntuples];
            H5Dread(varId, data_type,memspace,dataspace,H5P_DEFAULT, ui_data);
            for (i = 0 ; i < ntuples ; i++)
                data[i] = ui_data[i];
            delete[] ui_data;
        }
        else
        {
            // ERROR: UKNOWN TYPE
            EXCEPTION1(InvalidVariableException, varname);
        }

        // Done with hyperslab
        H5Sclose(dataspace);

        // Done with the space
        H5Sclose(spaceId);

        // Done with the type
        H5Tclose(data_type);
        H5Tclose(raw_data_type);

        // Done with the variable; don't leak it
        H5Dclose(varId);

        return fa;
    }
}


// ****************************************************************************
//  Method: avtFLASHFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 23, 2005
//
// ****************************************************************************

vtkDataArray *
avtFLASHFileFormat::GetVectorVar(int domain, const char *varname)
{
    return NULL;
}


// ****************************************************************************
//  Method:  avtFLASHFileFormat::ReadAllMetaData
//
//  Purpose:
//    Read the metadata if we have not done so yet.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2005
//
//  Modifications:
//    Jeremy Meredith, Thu Aug 25 15:07:36 PDT 2005
//    Added particle support.
//
// ****************************************************************************
void
avtFLASHFileFormat::ReadAllMetaData()
{
    if (fileId >= 0)
    {
        return;
    }
    
    fileId = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
    if (fileId < 0)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    ReadBlockStructure();
    ReadBlockExtents();
    ReadRefinementLevels();
    ReadSimulationParameters();
    ReadUnknownNames();
    DetermineGlobalLogicalExtentsForAllBlocks();
    ReadParticleAttributes();
}


// ****************************************************************************
//  Method:  avtFLASHFileFormat::ReadBlockStructure
//
//  Purpose:
//    Read the domain connectivity and hierarchy.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2005
//
// ****************************************************************************
void avtFLASHFileFormat::ReadBlockStructure()
{
    //
    // Read the "gid" block connectivity description
    //
    hid_t gidId = H5Dopen(fileId, "gid");
    if (gidId < 0)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    hid_t gidSpaceId = H5Dget_space(gidId);
    
    hsize_t gid_dims[2];
    hsize_t gid_ndims =  H5Sget_simple_extent_dims(gidSpaceId, gid_dims, NULL);
    if (gid_ndims != 2)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    numBlocks = gid_dims[0];
    switch (gid_dims[1])
    {
      case 5:
        dimension = 1;
        numChildrenPerBlock = 2;
        numNeighborsPerBlock = 2;
        break;
      case 9:
        dimension = 2;
        numChildrenPerBlock = 4;
        numNeighborsPerBlock = 4;
        break;
      case 15:
        dimension = 3;
        numChildrenPerBlock = 8;
        numNeighborsPerBlock = 6;
        break;
      default:
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    hid_t gid_raw_data_type = H5Dget_type(gidId);
    hid_t gid_data_type = H5Tget_native_type(gid_raw_data_type,H5T_DIR_ASCEND);
    
    int *gid_array = new int[numBlocks * gid_dims[1]];
    H5Dread(gidId, gid_data_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, gid_array);

    // convert to an easier-to-grok format
    blocks.resize(numBlocks);
    for (int b=0; b<numBlocks; b++)
    {
        int *gid_line = &gid_array[gid_dims[1]*b];
        int  pos=0;

        blocks[b].ID = b+1;  // 1-origin IDs
        for (int n=0; n<numNeighborsPerBlock; n++)
        {
            blocks[b].neighborIDs[n] = gid_line[pos++];
        }
        blocks[b].parentID = gid_line[pos++];
        for (int n=0; n<numChildrenPerBlock; n++)
        {
            blocks[b].childrenIDs[n] = gid_line[pos++];
        }
    }

    // Done with the type
    H5Tclose(gid_data_type);
    H5Tclose(gid_raw_data_type);

    // Done with the space
    H5Sclose(gidSpaceId);

    // Done with the variable; don't leak it
    H5Dclose(gidId);

    // Delete the raw array
    delete[] gid_array;
}

// ****************************************************************************
//  Method:  avtFLASHFileFormat::ReadBlockExtents
//
//  Purpose:
//    Read the spatial extents for the blocks.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2005
//
// ****************************************************************************
void avtFLASHFileFormat::ReadBlockExtents()
{
    //
    // Read the bounding box description for the blocks
    //
    hid_t bboxId = H5Dopen(fileId, "bounding box");
    if (bboxId < 0)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    hid_t bboxSpaceId = H5Dget_space(bboxId);
    
    hsize_t bbox_dims[3];
    hsize_t bbox_ndims = H5Sget_simple_extent_dims(bboxSpaceId,bbox_dims,NULL);

    if (bbox_ndims != 3 ||
        bbox_dims[0] != numBlocks ||
        bbox_dims[1] != dimension ||
        bbox_dims[2] != 2)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    hid_t bbox_raw_data_type = H5Dget_type(bboxId);
    hid_t bbox_data_type = H5Tget_native_type(bbox_raw_data_type,
                                              H5T_DIR_ASCEND);
    
    double *bbox_array = new double[numBlocks * dimension * 2];
    H5Dread(bboxId, bbox_data_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, bbox_array);

    minSpatialExtents[0] = FLT_MAX;
    minSpatialExtents[1] = FLT_MAX;
    minSpatialExtents[2] = FLT_MAX;
    maxSpatialExtents[0] = -FLT_MAX;
    maxSpatialExtents[1] = -FLT_MAX;
    maxSpatialExtents[2] = -FLT_MAX;

    for (int b=0; b<numBlocks; b++)
    {
        double *bbox_line = &bbox_array[dimension*2*b];
        for (int d=0; d<3; d++)
        {
            if (d+1 <= dimension)
            {
                blocks[b].minSpatialExtents[d] = bbox_line[d*2 + 0];
                blocks[b].maxSpatialExtents[d] = bbox_line[d*2 + 1];
            }
            else
            {
                blocks[b].minSpatialExtents[d] = 0;
                blocks[b].maxSpatialExtents[d] = 0;
            }

            if (blocks[b].minSpatialExtents[0] < minSpatialExtents[0])
                minSpatialExtents[0] = blocks[b].minSpatialExtents[0];
            if (blocks[b].minSpatialExtents[1] < minSpatialExtents[1])
                minSpatialExtents[1] = blocks[b].minSpatialExtents[1];
            if (blocks[b].minSpatialExtents[2] < minSpatialExtents[2])
                minSpatialExtents[2] = blocks[b].minSpatialExtents[2];

            if (blocks[b].maxSpatialExtents[0] > maxSpatialExtents[0])
                maxSpatialExtents[0] = blocks[b].maxSpatialExtents[0];
            if (blocks[b].maxSpatialExtents[1] > maxSpatialExtents[1])
                maxSpatialExtents[1] = blocks[b].maxSpatialExtents[1];
            if (blocks[b].maxSpatialExtents[2] > maxSpatialExtents[2])
                maxSpatialExtents[2] = blocks[b].maxSpatialExtents[2];
        }
    }

    // Done with the type
    H5Tclose(bbox_data_type);
    H5Tclose(bbox_raw_data_type);

    // Done with the space
    H5Sclose(bboxSpaceId);

    // Done with the variable; don't leak it
    H5Dclose(bboxId);

    // Delete the raw array
    delete[] bbox_array;
}


// ****************************************************************************
//  Method:  avtFLASHFileFormat::ReadRefinementLevels
//
//  Purpose:
//    Read the explicit refinement levels for the blocks.  We could
//    theoretically guess it based on size, but it's much cleaner and more
//    robust to simply read it from the file and use it as fact.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2005
//
// ****************************************************************************
void avtFLASHFileFormat::ReadRefinementLevels()
{
    //
    // Read the bounding box description for the blocks
    //
    hid_t refinementId = H5Dopen(fileId, "refine level");
    if (refinementId < 0)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    hid_t refinementSpaceId = H5Dget_space(refinementId);
    
    hsize_t refinement_dims[1];
    hsize_t refinement_ndims = H5Sget_simple_extent_dims(refinementSpaceId,
                                                         refinement_dims,NULL);

    if (refinement_ndims != 1 || refinement_dims[0] != numBlocks)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    hid_t refinement_raw_data_type = H5Dget_type(refinementId);
    hid_t refinement_data_type = H5Tget_native_type(refinement_raw_data_type,
                                                    H5T_DIR_ASCEND);
    
    int *refinement_array = new int[numBlocks];
    H5Dread(refinementId, refinement_data_type, H5S_ALL, H5S_ALL, H5P_DEFAULT,
            refinement_array);

    for (int b=0; b<numBlocks; b++)
    {
        int level = refinement_array[b];
        blocks[b].level = level;
        if (level > numLevels)
        {
            numLevels = level;
        }
    }

    // Done with the type
    H5Tclose(refinement_data_type);
    H5Tclose(refinement_raw_data_type);

    // Done with the space
    H5Sclose(refinementSpaceId);

    // Done with the variable; don't leak it
    H5Dclose(refinementId);

    // Delete the raw array
    delete[] refinement_array;
}

// ****************************************************************************
//  Method:  avtFLASHFileFormat::ReadSimulationParameters
//
//  Purpose:
//    Read the simulation parameters from the file.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2005
//
// ****************************************************************************
void avtFLASHFileFormat::ReadSimulationParameters()
{
    //
    // Read the simulation parameters
    //
    hid_t simparamsId = H5Dopen(fileId, "simulation parameters");
    if (simparamsId < 0)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    hid_t sp_raw_data_type = H5Dget_type(simparamsId);
    hid_t sp_data_type = H5Tget_native_type(sp_raw_data_type, H5T_DIR_ASCEND);

    H5Dread(simparamsId, sp_data_type, H5S_ALL, H5S_ALL, H5P_DEFAULT,
            &simParams);

    // Sanity check: size of the gid array better match number of blocks
    //               reported in the simulation parameters
    if (simParams.total_blocks != numBlocks)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    if (simParams.nxb == 1)
    {
        block_ndims[0] = 1;
        block_zdims[0] = 1;
    }
    else
    {
        block_ndims[0] = simParams.nxb+1;
        block_zdims[0] = simParams.nxb;
    }

    if (simParams.nyb == 1)
    {
        block_ndims[1] = 1;
        block_zdims[1] = 1;
    }
    else
    {
        block_ndims[1] = simParams.nyb+1;
        block_zdims[1] = simParams.nyb;
    }

    if (simParams.nzb == 1)
    {
        block_ndims[2] = 1;
        block_zdims[2] = 1;
    }
    else
    {
        block_ndims[2] = simParams.nzb+1;
        block_zdims[2] = simParams.nzb;
    }

    // Done with the type
    H5Tclose(sp_data_type);
    H5Tclose(sp_raw_data_type);

    // Done with the variable; don't leak it
    H5Dclose(simparamsId);
}

// ****************************************************************************
//  Method:  avtFLASHFileFormat::ReadUnknownNames
//
//  Purpose:
//    Read the variable name list.  They are called "unknowns" in
//    FLASH terminology.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2005
//
// ****************************************************************************
void avtFLASHFileFormat::ReadUnknownNames()
{
    //
    // Read the variable ("unknown") names
    //
    hid_t unknownsId = H5Dopen(fileId, "unknown names");
    if (unknownsId < 0)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    hid_t unkSpaceId = H5Dget_space(unknownsId);
    
    hsize_t unk_dims[2];
    hsize_t unk_ndims =  H5Sget_simple_extent_dims(unkSpaceId, unk_dims, NULL);
    if (unk_ndims != 2 || unk_dims[1] != 1)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    hid_t unk_raw_data_type = H5Dget_type(unknownsId);
    int length = H5Tget_size(unk_raw_data_type);

    int nvars = unk_dims[0];
    char *unk_array = new char[nvars * length];

    H5Dread(unknownsId, unk_raw_data_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, unk_array);

    varNames.resize(nvars);
    char *tmpstring = new char[length+1];
    for (int v=0; v<nvars; v++)
    {
        for (int c=0; c<length; c++)
        {
            tmpstring[c] = unk_array[v*length + c];
        }
        tmpstring[length] = '\0';

        varNames[v] = tmpstring;
    }


    // Done with the type
    H5Tclose(unk_raw_data_type);

    // Done with the space
    H5Sclose(unkSpaceId);

    // Done with the variable; don't leak it
    H5Dclose(unknownsId);
}

// ****************************************************************************
//  Method:  avtFLASHFileFormat::ReadParticleAttributes
//
//  Purpose:
//    Read the particle attributes, like number of particles and
//    variable names.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 25, 2005
//
// ****************************************************************************
void
avtFLASHFileFormat::ReadParticleAttributes()
{
    // temporarily disable error reporting
    H5E_auto_t  old_errorfunc;
    void       *old_clientdata;
    H5Eget_auto(&old_errorfunc, &old_clientdata);
    H5Eset_auto(NULL, NULL);

    // find the particle variable (if it exists)
    hid_t pointId = H5Dopen(fileId, "particle tracers");

    // turn back on error reporting
    H5Eset_auto(old_errorfunc, old_clientdata);

    // Doesn't exist?  No problem -- we just don't have any particles
    if (pointId < 0)
    {
        numParticles = 0;
        return;
    }

    hid_t pointSpaceId = H5Dget_space(pointId);

    hsize_t p_dims[0];
    hsize_t p_ndims =  H5Sget_simple_extent_dims(pointSpaceId, p_dims, NULL);
    if (p_ndims != 1)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }

    numParticles = p_dims[0];

    hid_t point_raw_type = H5Dget_type(pointId);
    int numMembers = H5Tget_nmembers(point_raw_type);
    for (int i=0; i<numMembers; i++)
    {
        char  *member_name = H5Tget_member_name(point_raw_type, i);
        string nice_name = GetNiceParticleName(member_name);
        hid_t  member_raw_type = H5Tget_member_type(point_raw_type, i);
        hid_t  member_type = H5Tget_native_type(member_raw_type, H5T_DIR_ASCEND);
        int    index = particleVarTypes.size();
        if (H5Tequal(member_type, H5T_NATIVE_DOUBLE)>0)
        {
            particleVarTypes.push_back(H5T_NATIVE_DOUBLE);
            particleVarNames.push_back(member_name);
            particleOriginalIndexMap[nice_name] = index;
        }
        else if (H5Tequal(member_type, H5T_NATIVE_INT)>0)
        {
            particleVarTypes.push_back(H5T_NATIVE_INT);
            particleVarNames.push_back(member_name);
            particleOriginalIndexMap[nice_name] = index;
        }
        else
        {
            // only support double and int for now
        }

        H5Tclose(member_type);
        H5Tclose(member_raw_type);
    }

    H5Tclose(point_raw_type);
    H5Sclose(pointSpaceId);
    H5Dclose(pointId);
}


// ****************************************************************************
//  Method:  avtFLASHFileFormat::DetermineGlobalLogicalExtentsForAllBlocks
//
//  Purpose:
//    Get the global logical extents for this block at this refinement level.
//    This is needed for domain nesting and relies on all the block
//    extents and simulation parameters having been read.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2005
//
// ****************************************************************************
void avtFLASHFileFormat::DetermineGlobalLogicalExtentsForAllBlocks()
{
    double problemsize[3] = {maxSpatialExtents[0] - minSpatialExtents[0],
                             maxSpatialExtents[1] - minSpatialExtents[1],
                             maxSpatialExtents[2] - minSpatialExtents[2]};
    for (int b=0; b<numBlocks; b++)
    {
        int multiplier = 1 << (blocks[b].level - 1);
        int ncells_x = multiplier * block_zdims[0];
        int ncells_y = multiplier * block_zdims[1];
        int ncells_z = multiplier * block_zdims[2];
        double startx=0, starty=0, startz=0;
        double endx=0,   endy=0,   endz=0;

        if (problemsize[0] > 0)
        {
            startx = (ncells_x * blocks[b].minSpatialExtents[0]) / problemsize[0];
            endx   = (ncells_x * blocks[b].maxSpatialExtents[0]) / problemsize[0];
        }

        if (problemsize[1] > 0)
        {
            starty = (ncells_y * blocks[b].minSpatialExtents[1]) / problemsize[1];
            endy   = (ncells_y * blocks[b].maxSpatialExtents[1]) / problemsize[1];
        }

        if (problemsize[2] > 0)
        {
            startz = (ncells_z * blocks[b].minSpatialExtents[2]) / problemsize[2];
            endz   = (ncells_z * blocks[b].maxSpatialExtents[2]) / problemsize[2];
        }

        blocks[b].minGlobalLogicalExtents[0] = int(startx + .5);
        blocks[b].minGlobalLogicalExtents[1] = int(starty + .5);
        blocks[b].minGlobalLogicalExtents[2] = int(startz + .5);
        blocks[b].maxGlobalLogicalExtents[0] = int(endx   + .5);
        blocks[b].maxGlobalLogicalExtents[1] = int(endy   + .5);
        blocks[b].maxGlobalLogicalExtents[2] = int(endz   + .5);
    }
}

// ****************************************************************************
//  Method:  avtFLASHFileFormat::BuildDomainNesting
//
//  Purpose:
//     Generates the domain nesting object needed to remove coarse zones in
//     an AMR mesh.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2005
//
// ****************************************************************************
void
avtFLASHFileFormat::BuildDomainNesting()
{
    if (numBlocks <= 1)
        return;

    // first, look to see if we don't already have it cached
    void_ref_ptr vrTmp = cache->GetVoidRef("mesh",
                                   AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION,
                                   timestep, -1);

    if ((*vrTmp == NULL))
    {
        int i;

        avtRectilinearDomainBoundaries *rdb = new avtRectilinearDomainBoundaries(true);
        rdb->SetNumDomains(numBlocks);
        for (i = 1; i <= numBlocks; i++)
        {
            int logExts[6];
            logExts[0] = blocks[i].minGlobalLogicalExtents[0];
            logExts[1] = blocks[i].maxGlobalLogicalExtents[0];
            logExts[2] = blocks[i].minGlobalLogicalExtents[1];
            logExts[3] = blocks[i].maxGlobalLogicalExtents[1];
            logExts[4] = blocks[i].minGlobalLogicalExtents[2];
            logExts[5] = blocks[i].maxGlobalLogicalExtents[2];
            rdb->SetIndicesForAMRPatch(i-1, blocks[i].level, logExts);
        }
        rdb->CalculateBoundaries();

        void_ref_ptr vrdb = void_ref_ptr(rdb,
                                         avtStructuredDomainBoundaries::Destruct);
        cache->CacheVoidRef("any_mesh", AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION,
                            timestep, -1, vrdb);

        //
        // build the avtDomainNesting object
        //

        if (numLevels > 0)
        {
            avtStructuredDomainNesting *dn =
                new avtStructuredDomainNesting(numBlocks, numLevels);

            dn->SetNumDimensions(dimension);

            //
            // Set refinement level ratio information
            //

            // NOTE: FLASH files always have a 2:1 ratio
            vector<int> ratios(3);
            ratios[0] = 1;
            ratios[1] = 1;
            ratios[2] = 1;
            dn->SetLevelRefinementRatios(0, ratios);
            for (i = 1; i < numLevels; i++)
            {
                vector<int> ratios(3);
                ratios[0] = 2;
                ratios[1] = 2;
                ratios[2] = 2;
                dn->SetLevelRefinementRatios(i, ratios);
            }

            //
            // set each domain's level, children and logical extents
            //
            for (i = 0; i < numBlocks; i++)
            {
                vector<int> childBlocks;
                for (int j = 0; j < numChildrenPerBlock; j++)
                {
                    // if this is allowed to be 1-origin, the "-1" here
                    // needs to be removed
                    if (blocks[i].childrenIDs[j] >= 0)
                        childBlocks.push_back(blocks[i].childrenIDs[j] - 1);
                }

                vector<int> logExts(6);

                logExts[0] = blocks[i].minGlobalLogicalExtents[0];
                logExts[1] = blocks[i].minGlobalLogicalExtents[1];
                logExts[2] = blocks[i].minGlobalLogicalExtents[2];

                logExts[3] = blocks[i].maxGlobalLogicalExtents[0]-1;
                if (dimension >= 2)
                    logExts[4] = blocks[i].maxGlobalLogicalExtents[1]-1;
                else
                    logExts[4] = blocks[i].maxGlobalLogicalExtents[1];
                if (dimension >= 3)
                    logExts[5] = blocks[i].maxGlobalLogicalExtents[2]-1;
                else
                    logExts[5] = blocks[i].minGlobalLogicalExtents[2];

                dn->SetNestingForDomain(i, blocks[i].level-1,
                                        childBlocks, logExts);
            }

            void_ref_ptr vr = void_ref_ptr(dn,
                                         avtStructuredDomainNesting::Destruct);

            cache->CacheVoidRef("mesh",
                                AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION,
                                timestep, -1, vr);
        }
    }
}

// ****************************************************************************
//  Method:  avtFLASHFileFormat::GetAuxiliaryData
//
//  Purpose:
//    Right now this only supports spatial interval trees.  There is no
//    other information like materials and species available.
//
//  Arguments:
//    type       the kind of auxiliary data to create
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 23, 2005
//
// ****************************************************************************
void *
avtFLASHFileFormat::GetAuxiliaryData(const char *var, int dom, 
                                     const char * type, void *,
                                     DestructorFunction &df)
{
    if (type != AUXILIARY_DATA_SPATIAL_EXTENTS)
        return NULL;

    avtIntervalTree *itree = new avtIntervalTree(numBlocks, 3);

    for (int b = 0 ; b < numBlocks ; b++)
    {
        float bounds[6];
        bounds[0] = blocks[b].minSpatialExtents[0];
        bounds[1] = blocks[b].maxSpatialExtents[0];
        bounds[2] = blocks[b].minSpatialExtents[1];
        bounds[3] = blocks[b].maxSpatialExtents[1];
        bounds[4] = blocks[b].minSpatialExtents[2];
        bounds[5] = blocks[b].maxSpatialExtents[2];
        itree->AddDomain(b, bounds);
    }
    itree->Calculate(true);

    df = avtIntervalTree::Destruct;

    return ((void *) itree);
}

