// ************************************************************************* //
//                            avtEnzoFileFormat.C                           //
// ************************************************************************* //

#include <avtEnzoFileFormat.h>

#include <string>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSetWriter.h>

#include <avtDatabaseMetaData.h>
#include <avtStructuredDomainBoundaries.h>
#include <avtStructuredDomainNesting.h>
#include <avtVariableCache.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <vtkCellType.h>
#include <Expression.h>

#include <hdf.h>
#include <mfhdf.h>

using std::string;

void avtEnzoFileFormat::Grid::PrintRecursive(vector<Grid> &grids, int level)
{
    string indent(level*3, ' ');
    cerr << indent << ID << endl;
    for (int i=0; i<childrenID.size(); i++)
    {
        grids[childrenID[i]].PrintRecursive(grids,level+1);
    }
}

void avtEnzoFileFormat::Grid::Print()
{
    cerr << "ID:        "<<ID<<endl;
    cerr << "zdims:     "<<zdims[0]<<","<<zdims[1]<<","<<zdims[2]<<endl;
    cerr << "minSpatialExtents:"<<minSpatialExtents[0]<<","<<minSpatialExtents[1]<<","<<minSpatialExtents[2]<<endl;
    cerr << "maxSpatialExtents:"<<maxSpatialExtents[0]<<","<<maxSpatialExtents[1]<<","<<maxSpatialExtents[2]<<endl;
    cerr << "parentID:  "<<parentID<<endl;
    cerr << "level:     "<<level<<endl;
    cerr << "parMinExt: "<<minLogicalExtentsInParent[0]<<","<<minLogicalExtentsInParent[1]<<","<<minLogicalExtentsInParent[2]<<endl;
    cerr << "parMaxExt: "<<maxLogicalExtentsInParent[0]<<","<<maxLogicalExtentsInParent[1]<<","<<maxLogicalExtentsInParent[2]<<endl;
    cerr << "globMinExt:"<<minLogicalExtentsGlobally[0]<<","<<minLogicalExtentsGlobally[1]<<","<<minLogicalExtentsGlobally[2]<<endl;
    cerr << "globMaxExt:"<<maxLogicalExtentsGlobally[0]<<","<<maxLogicalExtentsGlobally[1]<<","<<maxLogicalExtentsGlobally[2]<<endl;
    cerr << "parRatio:  "<<refinementRatio[0]<<","<<refinementRatio[1]<<","<<refinementRatio[2]<<endl;
    cerr << endl;
}

void avtEnzoFileFormat::Grid::DetermineExtentsInParent(vector<Grid> &grids)
{
    Grid &p = grids[parentID];
    if (ID != 1)
    {
        minLogicalExtentsInParent[0] = int(.5 + double(p.zdims[0]) * (minSpatialExtents[0] - p.minSpatialExtents[0])/(p.maxSpatialExtents[0] - p.minSpatialExtents[0]));
        minLogicalExtentsInParent[1] = int(.5 + double(p.zdims[1]) * (minSpatialExtents[1] - p.minSpatialExtents[1])/(p.maxSpatialExtents[1] - p.minSpatialExtents[1]));
        minLogicalExtentsInParent[2] = int(.5 + double(p.zdims[2]) * (minSpatialExtents[2] - p.minSpatialExtents[2])/(p.maxSpatialExtents[2] - p.minSpatialExtents[2]));
        maxLogicalExtentsInParent[0] = int(.5 + double(p.zdims[0]) * (maxSpatialExtents[0] - p.minSpatialExtents[0])/(p.maxSpatialExtents[0] - p.minSpatialExtents[0]));
        maxLogicalExtentsInParent[1] = int(.5 + double(p.zdims[1]) * (maxSpatialExtents[1] - p.minSpatialExtents[1])/(p.maxSpatialExtents[1] - p.minSpatialExtents[1]));
        maxLogicalExtentsInParent[2] = int(.5 + double(p.zdims[2]) * (maxSpatialExtents[2] - p.minSpatialExtents[2])/(p.maxSpatialExtents[2] - p.minSpatialExtents[2]));
        refinementRatio[0] = double(zdims[0]) / double(maxLogicalExtentsInParent[0]-minLogicalExtentsInParent[0]);
        refinementRatio[1] = double(zdims[1]) / double(maxLogicalExtentsInParent[1]-minLogicalExtentsInParent[1]);
        refinementRatio[2] = double(zdims[2]) / double(maxLogicalExtentsInParent[2]-minLogicalExtentsInParent[2]);
        if (refinementRatio[0] != 2 ||
            refinementRatio[1] != 2 ||
            refinementRatio[2] != 2)
        {
            EXCEPTION1(ImproperUseException,
                       "Found a refinement ratio that was not exactly 2.");
        }
    }
    else
    {
        minLogicalExtentsInParent[0] = 0;
        minLogicalExtentsInParent[1] = 0;
        minLogicalExtentsInParent[2] = 0;
        maxLogicalExtentsInParent[0] = ndims[0]-1;
        maxLogicalExtentsInParent[1] = ndims[1]-1;
        maxLogicalExtentsInParent[2] = ndims[2]-1;
        refinementRatio[0] = refinementRatio[1] = refinementRatio[2] = 1;
    }
}

void avtEnzoFileFormat::Grid::DetermineExtentsGlobally(int numLevels,
                                                       vector<Grid> &grids)
{
    Grid &p = grids[parentID];
    if (ID != 1)
    {
        minLogicalExtentsGlobally[0] = int((p.minLogicalExtentsGlobally[0] + minLogicalExtentsInParent[0])*refinementRatio[0]);
        minLogicalExtentsGlobally[1] = int((p.minLogicalExtentsGlobally[1] + minLogicalExtentsInParent[1])*refinementRatio[1]);
        minLogicalExtentsGlobally[2] = int((p.minLogicalExtentsGlobally[2] + minLogicalExtentsInParent[2])*refinementRatio[2]);
        maxLogicalExtentsGlobally[0] = int((p.minLogicalExtentsGlobally[0] + maxLogicalExtentsInParent[0])*refinementRatio[0]);
        maxLogicalExtentsGlobally[1] = int((p.minLogicalExtentsGlobally[1] + maxLogicalExtentsInParent[1])*refinementRatio[1]);
        maxLogicalExtentsGlobally[2] = int((p.minLogicalExtentsGlobally[2] + maxLogicalExtentsInParent[2])*refinementRatio[2]);
    }
    else
    {
        minLogicalExtentsGlobally[0] = 0;
        minLogicalExtentsGlobally[1] = 0;
        minLogicalExtentsGlobally[2] = 0;
        maxLogicalExtentsGlobally[0] = zdims[0];
        maxLogicalExtentsGlobally[1] = zdims[1];
        maxLogicalExtentsGlobally[2] = zdims[2];
    }
}

void
avtEnzoFileFormat::ReadHierachyFile()
{

    // PARSE HIERARCHY

    ifstream h(fnameH.c_str());
    if (!h)
        EXCEPTION1(InvalidFilesException, fnameH.c_str());

    // Set something reasonable for a grid with id==0
    Grid root;
    root.ID = 0;
    root.level = -1;
    root.parentID = -1;
    root.minSpatialExtents[0] = root.minSpatialExtents[1] = root.minSpatialExtents[2] = -1e37;
    root.maxSpatialExtents[0] = root.maxSpatialExtents[1] = root.maxSpatialExtents[2] = 1e37;
    root.minLogicalExtentsGlobally[0] = 0;
    root.minLogicalExtentsGlobally[1] = 0;
    root.minLogicalExtentsGlobally[2] = 0;
    grids.push_back(root);

    string buff = "";
    int level = 0;
    int parent = 0;
    while (h)
    {
        while (buff != "Grid" && buff != "Pointer:" && buff != "Time")
            h >> buff;
        if (buff == "Grid")
        {
            Grid g;
            h >> buff; // '='
            h >> g.ID;

            int min3[3];
            int max3[3];
            while (buff != "GridStartIndex")
            {
                h >> buff;
            }
            h >> buff; // '='
            h >> min3[0] >> min3[1] >> min3[2];
            while (buff != "GridEndIndex")
            {
                h >> buff;
            }
            h >> buff; // '='
            h >> max3[0] >> max3[1] >> max3[2];

            g.zdims[0] = max3[0]-min3[0] + 1;
            g.zdims[1] = max3[1]-min3[1] + 1;
            g.zdims[2] = max3[2]-min3[2] + 1;

            g.ndims[0] = g.zdims[0]+1;
            g.ndims[1] = g.zdims[1]+1;
            g.ndims[2] = g.zdims[2]+1;

            while (buff != "GridLeftEdge")
            {
                h >> buff;
            }
            h >> buff; // '='
            h >> g.minSpatialExtents[0] >> g.minSpatialExtents[1] >> g.minSpatialExtents[2];

            while (buff != "GridRightEdge")
            {
                h >> buff;
            }
            h >> buff; // '='
            h >> g.maxSpatialExtents[0] >> g.maxSpatialExtents[1] >> g.maxSpatialExtents[2];

            g.level = level;
            g.parentID = parent;
            g.DetermineExtentsInParent(grids);

            if (grids.size() != g.ID)
            {
                EXCEPTION2(InvalidFilesException, fnameH.c_str(),
                           "The grids in the hierarchy are currently "
                           "expected to be listed in order.");
            }

            grids.push_back(g);
            grids[parent].childrenID.push_back(g.ID);
            numGrids = grids.size()-1;
        }
        else if (buff == "Pointer:")
        {
            char c;
            buff = "";
            while ((c=h.get()) != '[');
            while ((c=h.get()) != ']') buff += c;
            int gridID = atoi(buff.c_str());
            h.get(); // -
            h.get(); // >
            h >> buff;
            if (buff == "NextGridNextLevel")
            {
                h >> buff; // '='
                int n;
                h >> n;
                if (n == 0)
                {
                }
                else
                {
                    level = grids[gridID].level + 1;
                    numLevels = level+1 > numLevels ? level+1 : numLevels;
                    parent = gridID;
                }
            }
            else // buff == "NextGridThisLevel"
            {
                h >> buff; // '='
                int n;
                h >> n;
                if (n == 0)
                {
                }
                else
                {
                }
            }
        }
        else if (buff == "Time")
        {
            h >> buff; // '='
            h >> curTime;
        }
        h >> buff;
    }
    h.close();
}

void
avtEnzoFileFormat::ReadParameterFile()
{
    // PARSE PARAMETER FILE

    ifstream r(fname_base.c_str());
    if (!r)
        EXCEPTION2(InvalidFilesException, fname_base.c_str(),
                   "This index file did not exist.");

    string buff("");
    while (r)
    {
        r >> buff;
        if (buff == "InitialCycleNumber")
        {
            r >> buff; // '='
            r >> curCycle;
        }
        else if (buff == "InitialTime")
        {
            r >> buff; // '='
            r >> curTime;
        }
    }
    r.close();
}

void
avtEnzoFileFormat::DetermineVariablesFromGridFile()
{
    int smallest_grid = 0;
    int smallest_grid_nzones = INT_MAX;
    for (int i=1; i<grids.size(); i++)
    {
        Grid &g = grids[i];
        int numZones = g.zdims[0]*g.zdims[1]*g.zdims[2];
        if (numZones < smallest_grid_nzones)
        {
            smallest_grid_nzones = numZones;
            smallest_grid = g.ID;
        }
    }

    char gridFileName[1000];
    sprintf(gridFileName, "%s.grid%04d", fname_base.c_str(), smallest_grid);

    int32 file_handle = SDstart(gridFileName, DFACC_READ);
    if (file_handle < 0)
    {
        EXCEPTION1(InvalidFilesException, gridFileName);
    }

    int32 n_datasets;
    int32 n_file_attrs;
    int32 status = SDfileinfo(file_handle, &n_datasets, &n_file_attrs);

    for (int var = 0 ; var < n_datasets ; var++)
    {
        int32 ndims;
        int32 dims[3];
        int32 data_type;
        char  name[65];
        int32 nattrs;
        int32 var_handle = SDselect(file_handle, var);
        SDgetinfo(var_handle, name, &ndims, dims, &data_type, &nattrs);
        SDendaccess(var_handle);
        if (ndims > 1)
        {
            // it's a normal mesh variable
            varNames.push_back(name);
        }
        else if (strlen(name) > 8 && strncmp(name,"particle",8)==0)
        {
            // it's a particle variable; skip over coordinate arrays
            if (strncmp(name,"particle_position_",18) != 0)
            {
                particleVarNames.push_back(name);
            }
        }
    }

    SDend(file_handle); 
}

// ****************************************************************************
//  Method: avtEnzo constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   January  3, 2005
//
// ****************************************************************************

avtEnzoFileFormat::avtEnzoFileFormat(const char *filename)
    : avtSTMDFileFormat(&filename, 1)
{
    numGrids = 0;
    numLevels = 0;
    curTime = 0;

    fname_base;
    string fname(filename);
    string extH(".hierarchy");
    string extB(".boundary");
    if (fname.length() > extH.length() &&
        fname.substr(fname.length()-extH.length()) == extH)
    {
        fname_base = fname.substr(0,fname.length()-extH.length());
        fnameH = fname;
        fnameB = fname_base + extB;
    }
    else if (fname.length() > extB.length() &&
        fname.substr(fname.length()-extB.length()) == extB)
    {
        fname_base = fname.substr(0,fname.length()-extB.length());
        fnameB = fname;
        fnameH = fname_base + extH;
    }
    else
    {
        EXCEPTION2(InvalidFilesException, fname.c_str(),
                   "It was not a .hierarchy or .boundary file.");
    }

}

avtEnzoFileFormat::~avtEnzoFileFormat()
{
}

void
avtEnzoFileFormat::ReadAllMetaData()
{
    // Check to see if we've read it yet
    if (numGrids > 0)
        return;

    // The parameter file 
    ReadParameterFile();

    // Read the hierarchy file, and simultaneously
    // convert spatial extents into logical extents
    ReadHierachyFile();

    // Convert the parent logical extents
    for (int i=1; i<grids.size(); i++)
    {
        grids[i].DetermineExtentsGlobally(numLevels, grids);
    }

    // Open the smallest grid file and determine what variables
    // are in it
    DetermineVariablesFromGridFile();
}

int
avtEnzoFileFormat::GetCyle()
{
    return curCycle;
}

// ****************************************************************************
//  Method: avtEnzoFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Jeremy Meredith
//  Creation:   January  4, 2005
//
// ****************************************************************************

void
avtEnzoFileFormat::FreeUpResources(void)
{
    /*
    grids.clear();
    varNames.clear();
    particleVarNames.clear();
    numGrids=0;
    numLevels=0;
    */
}


// ****************************************************************************
//  Method: avtEnzoFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: meredith -- generated by xml2avt
//  Creation:   Fri Dec 3 17:19:52 PST 2004
//
// ****************************************************************************

void
avtEnzoFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    ReadAllMetaData();

    float extents[6] = {grids[1].minSpatialExtents[0], grids[1].maxSpatialExtents[0],
                        grids[1].minSpatialExtents[1], grids[1].maxSpatialExtents[1],
                        grids[1].minSpatialExtents[2], grids[1].maxSpatialExtents[2]};

    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "mesh";

    mesh->meshType = AVT_RECTILINEAR_MESH;
    mesh->topologicalDimension = 3;
    mesh->spatialDimension = 3;
    mesh->hasSpatialExtents = false; // can we say yes?

    // spoof a group/domain mesh
    mesh->numBlocks = numGrids;
    mesh->blockTitle = "Grids";
    mesh->blockPieceName = "grid";
    mesh->numGroups = numLevels;
    mesh->groupTitle = "Levels";
    mesh->groupPieceName = "level";
    vector<int> groupIds(numGrids);
    vector<string> blockPieceNames(numGrids);
    for (int i = 1; i <= numGrids; i++)
    {
        char tmpName[64];
        sprintf(tmpName,"level%d,grid%d",grids[i].level, grids[i].ID);

        groupIds[i-1] = grids[i].level;
        blockPieceNames[i-1] = tmpName;
    }
    mesh->blockNames = blockPieceNames;
    mesh->numGroups = numLevels;
    mesh->groupIds = groupIds;
    md->Add(mesh);

    avtMeshMetaData *pmesh = new avtMeshMetaData;
    pmesh->name = "particles";
    pmesh->meshType = AVT_POINT_MESH;
    pmesh->topologicalDimension = 1;
    pmesh->spatialDimension = 3;
    pmesh->hasSpatialExtents = false;
    pmesh->numBlocks = numGrids;
    pmesh->blockTitle = "Grids";
    pmesh->blockPieceName = "grid";
    pmesh->numGroups = numLevels;
    pmesh->groupTitle = "Levels";
    pmesh->groupPieceName = "level";
    pmesh->blockNames = blockPieceNames;
    pmesh->numGroups = numLevels;
    pmesh->groupIds = groupIds;
    md->Add(pmesh);

    //md->AddGroupInformation(numLevels, numGrids, groupIds);

    for (int v = 0 ; v < varNames.size(); v++)
    {
        AddScalarVarToMetaData(md, varNames[v], "mesh", AVT_ZONECENT);
    }

    for (int p = 0 ; p < particleVarNames.size(); p++)
    {
        AddScalarVarToMetaData(md, particleVarNames[p], "particles",
                               AVT_NODECENT);
    }


    // Add Expressions
    Expression vel;
    vel.SetName("velocity");
    vel.SetDefinition("{<x-velocity>,<y-velocity>,<z-velocity>}");
    vel.SetType(Expression::VectorMeshVar);

    Expression pvel;
    pvel.SetName("particle_velocity");
    pvel.SetDefinition("{particle_velocity_x,particle_velocity_x,particle_velocity_x}");
    pvel.SetType(Expression::VectorMeshVar);

    md->AddExpression(&vel);
    md->AddExpression(&pvel);
}


// ****************************************************************************
//  Method: avtEnzoFileFormat::GetMesh
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
//  Programmer: meredith -- generated by xml2avt
//  Creation:   Fri Dec 3 17:19:52 PST 2004
//
// ****************************************************************************

vtkDataSet *
avtEnzoFileFormat::GetMesh(int domain, const char *meshname)
{
    ReadAllMetaData();

    if (string(meshname) == "mesh")
    {
        // rectilinear mesh
        BuildDomainNesting();  //  <<=== causing crashes!

        vtkFloatArray  *coords[3];
        int i;
        int d = domain+1;
        for (i = 0 ; i < 3 ; i++)
        {
            coords[i] = vtkFloatArray::New();
            coords[i]->SetNumberOfTuples(grids[d].ndims[i]);
            for (int j = 0 ; j < grids[d].ndims[i] ; j++)
            {
                double c = grids[d].minSpatialExtents[i] + double(j) * (grids[d].maxSpatialExtents[i]-grids[d].minSpatialExtents[i])/double(grids[d].ndims[i]-1);
                coords[i]->SetComponent(j, 0, c);
            }
        }
   
        vtkRectilinearGrid  *rGrid = vtkRectilinearGrid::New(); 
        rGrid->SetDimensions(grids[d].ndims);
        rGrid->SetXCoordinates(coords[0]);
        coords[0]->Delete();
        rGrid->SetYCoordinates(coords[1]);
        coords[1]->Delete();
        rGrid->SetZCoordinates(coords[2]);
        coords[2]->Delete();

        return rGrid;
    }
    else
    {
        // particle mesh
        char gridFileName[1000];
        sprintf(gridFileName, "%s.grid%04d", fname_base.c_str(), domain+1);

        int32 file_handle = SDstart(gridFileName, DFACC_READ);
        if (file_handle < 0)
        {
            EXCEPTION1(InvalidFilesException, gridFileName);
        }

        int32 var_index_x = SDnametoindex(file_handle, "particle_position_x");
        int32 var_index_y = SDnametoindex(file_handle, "particle_position_y");
        int32 var_index_z = SDnametoindex(file_handle, "particle_position_z");
        if (var_index_x < 0 || var_index_y < 0 || var_index_z < 0)
        {
            // This grid didn't have any particles.  No problem -- particles
            // won't exist in every grid.  Just close the file and return
            // NULL.
            SDend(file_handle);
            return NULL;
        }

        int32 var_handle_x = SDselect(file_handle, var_index_x);
        int32 var_handle_y = SDselect(file_handle, var_index_y);
        int32 var_handle_z = SDselect(file_handle, var_index_z);
        if (var_handle_x < 0 || var_handle_y < 0 || var_handle_z < 0)
        {
            // One of the particle position variables didn't exist.
            // This is strange, because we just converted the name
            // to an index.
            EXCEPTION1(InvalidVariableException, meshname);
        }

        // Get the number of particles
        int32 dims[3];
        {
            int32 ndims;
            int32 data_type;
            char name[65];
            int32 nattrs;
            SDgetinfo(var_handle_x, name, &ndims, dims, &data_type, &nattrs);
        }
        // It's one-dimensional
        int npart = dims[0];
        dims[1] = 1;
        dims[2] = 1;

        int32 start[3];
        start[0] = 0;
        start[1] = 0;
        start[2] = 0;

        vtkPoints *points  = vtkPoints::New();
        points->SetNumberOfPoints(npart);
        float *pts = (float *) points->GetVoidPointer(0);
        int i;

        double *ddata = new double[npart];
        SDreaddata(var_handle_x, start, NULL, dims, ddata);
        for (i=0; i<npart; i++)
            pts[i*3+0] = float(ddata[i]);

        SDreaddata(var_handle_y, start, NULL, dims, ddata);
        for (i=0; i<npart; i++)
            pts[i*3+1] = float(ddata[i]);

        SDreaddata(var_handle_z, start, NULL, dims, ddata);
        for (i=0; i<npart; i++)
            pts[i*3+2] = float(ddata[i]);

        SDendaccess(var_handle_x);
        SDendaccess(var_handle_y);
        SDendaccess(var_handle_z);

        vtkUnstructuredGrid  *ugrid = vtkUnstructuredGrid::New(); 
        ugrid->SetPoints(points);
        ugrid->Allocate(npart);
        vtkIdType onevertex;
        for(i = 0; i < npart; ++i)
        {
            onevertex = i;
            ugrid->InsertNextCell(VTK_VERTEX, 1, &onevertex);
        }

        points->Delete();

        // For now, always close the file
        SDend(file_handle);

        if (0)
        {
            char name[200];
            sprintf(name, "particlemesh_%d.vtk", domain);
            vtkDataSetWriter *wrtr = vtkDataSetWriter::New();
            wrtr->SetInput(ugrid);
            wrtr->SetFileName(name);
            wrtr->Write();
        }

        return ugrid;
    }
}


void
avtEnzoFileFormat::BuildDomainNesting()
{
    // first, look to see if we don't already have it cached
    void_ref_ptr vrTmp = cache->GetVoidRef("any_mesh",
                                   AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION,
                                   timestep, -1);

    if ((*vrTmp == NULL))
    {
        int i;

        avtRectilinearDomainBoundaries *rdb = new avtRectilinearDomainBoundaries(true);
        rdb->SetNumDomains(numGrids);
        for (i = 1; i <= numGrids; i++)
        {
            int logExts[6];
            logExts[0] = grids[i].minLogicalExtentsGlobally[0];
            logExts[1] = grids[i].maxLogicalExtentsGlobally[0];
            logExts[2] = grids[i].minLogicalExtentsGlobally[1];
            logExts[3] = grids[i].maxLogicalExtentsGlobally[1];
            logExts[4] = grids[i].minLogicalExtentsGlobally[2];
            logExts[5] = grids[i].maxLogicalExtentsGlobally[2];
            rdb->SetIndicesForAMRPatch(i-1, grids[i].level, logExts);
        }
        rdb->CalculateBoundaries();

        void_ref_ptr vrdb = void_ref_ptr(rdb,
                                         avtStructuredDomainBoundaries::Destruct);
        cache->CacheVoidRef("any_mesh", AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION,
                            timestep, -1, vrdb);

        //
        // build the avtDomainNesting object
        //
        avtStructuredDomainNesting *dn =
            new avtStructuredDomainNesting(numGrids, numLevels);

        dn->SetNumDimensions(3);

        //
        // Set refinement level ratio information
        //

        // NOTE: this appears to be on a per-level basis, not a
        //       per-grid basis.  We will just force them all to 
        //       a 2:1 ratio for now.  There is no reason internally
        //       that refinement ratios could not change on a
        //       per-grid basis, but it is only stored on a per-
        //       level basis
        int ratio = 1;
        vector<int> ratios(3);
        ratios[0] = ratio;
        ratios[1] = ratio;
        ratios[2] = ratio;
        dn->SetLevelRefinementRatios(0, ratios);
        for (i = 1; i < numLevels; i++)
        {
            ratio = 2;
            vector<int> ratios(3);
            ratios[0] = ratio;
            ratios[1] = ratio;
            ratios[2] = ratio;
            dn->SetLevelRefinementRatios(i, ratios);
        }

        //
        // set each domain's level, children and logical extents
        //
        for (i = 1; i <= numGrids; i++)
        {
            vector<int> childGrids;
            for (int j = 0; j < grids[i].childrenID.size(); j++)
            {
                // if this is allowed to be 1-origin, we will just pass
                // the "children" array up -- the "-1" here at least needs
                // to be removed
                childGrids.push_back(grids[i].childrenID[j] - 1);
            }

            vector<int> logExts(6);
            logExts[0] = grids[i].minLogicalExtentsGlobally[0];
            logExts[1] = grids[i].minLogicalExtentsGlobally[1];
            logExts[2] = grids[i].minLogicalExtentsGlobally[2];
            logExts[3] = grids[i].maxLogicalExtentsGlobally[0]-1;
            logExts[4] = grids[i].maxLogicalExtentsGlobally[1]-1;
            logExts[5] = grids[i].maxLogicalExtentsGlobally[2]-1;

            dn->SetNestingForDomain(i-1, grids[i].level,
                                    childGrids, logExts);
        }

        void_ref_ptr vr = void_ref_ptr(dn, avtStructuredDomainNesting::Destruct);

        cache->CacheVoidRef("any_mesh", AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION,
                            timestep, -1, vr);
    }
}


// ****************************************************************************
//  Method: avtEnzoFileFormat::GetVar
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
//  Programmer: meredith -- generated by xml2avt
//  Creation:   Fri Dec 3 17:19:52 PST 2004
//
// ****************************************************************************

vtkDataArray *
avtEnzoFileFormat::GetVar(int domain, const char *varname)
{
    ReadAllMetaData();

    // HDF4 STUFF

    char gridFileName[1000];
    sprintf(gridFileName, "%s.grid%04d", fname_base.c_str(), domain+1);

    int32 file_handle = SDstart(gridFileName, DFACC_READ);
    if (file_handle < 0)
    {
        EXCEPTION1(InvalidFilesException, gridFileName);
    }

    int32 var_index = SDnametoindex(file_handle, varname);
    if (var_index < 0)
    {
        EXCEPTION1(InvalidFilesException, gridFileName);
    }

    int32 var_handle = SDselect(file_handle, var_index);
    if (var_handle < 0)
    {
        EXCEPTION1(InvalidFilesException, gridFileName);
    }

    int32 ndims;
    int32 dims[3];
    int32 data_type;
    char  name[65];
    int32 nattrs;
    SDgetinfo(var_handle, name, &ndims, dims, &data_type, &nattrs);
    if (ndims == 1)
    {
        // force the other dimensions to length 1 for particle meshes
        dims[1]=1;
        dims[2]=1;
    }

    int ntuples = dims[0]*dims[1]*dims[2];

    vtkFloatArray * fa = vtkFloatArray::New();
    fa->SetNumberOfTuples(ntuples);
    float *data = fa->GetPointer(0);

    int32 start[3] = {0,0,0};

    double *f64_data;
    int32  *i32_data;
    uint32 *ui32_data;
    int i;
    switch (data_type)
    {
      case DFNT_FLOAT32:
        SDreaddata(var_handle, start, NULL, dims, data);
        break;
      case DFNT_FLOAT64:
        f64_data = new float64[ntuples];
        SDreaddata(var_handle, start, NULL, dims, f64_data);
        for (i = 0 ; i < ntuples ; i++)
            data[i] = f64_data[i];
        delete[] f64_data;
        break;
      case DFNT_INT32:
        i32_data = new int32[ntuples];
        SDreaddata(var_handle, start, NULL, dims, i32_data);
        for (i = 0 ; i < ntuples ; i++)
            data[i] = i32_data[i];
        delete[] i32_data;
        break;
      case DFNT_UINT32:
        ui32_data = new uint32[ntuples];
        SDreaddata(var_handle, start, NULL, dims, ui32_data);
        for (i = 0 ; i < ntuples ; i++)
            data[i] = ui32_data[i];
        delete[] ui32_data;
        break;
      default:
        // ERROR: UKNOWN TYPE
        break;
    }

    // Done with the variable; don't leak it
    SDendaccess(var_handle);

    // For now, always close the file
    SDend(file_handle);

    return fa;
}


// ****************************************************************************
//  Method: avtEnzoFileFormat::GetVectorVar
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
//  Creation:   January  4, 2005
//
// ****************************************************************************

vtkDataArray *
avtEnzoFileFormat::GetVectorVar(int domain, const char *varname)
{
    return NULL;
}
