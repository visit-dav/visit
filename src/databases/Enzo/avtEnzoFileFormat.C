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
#include <vtkCellType.h>

#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <avtStructuredDomainBoundaries.h>
#include <avtStructuredDomainNesting.h>
#include <avtVariableCache.h>
#include <avtIntervalTree.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <Expression.h>
#include <DebugStream.h>

#include <hdf.h>
#include <mfhdf.h>

#include <hdf5.h>

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

// ****************************************************************************
//  Method:  avtEnzoFileFormat::Grid::DetermineExtentsInParent
//
//  Purpose:
//    Use our spatial extents and the spatial extents of our parent, as well
//    as our dimensions, to determine our logical extents within our parent.
//
//  Programmer:  Jeremy Meredith
//  Creation:    January  3, 2005
//
//  Modifications:
//    Jeremy Meredith, Wed Feb 23 15:18:48 PST 2005
//    May have multiple root grids; identify by parentID==0, not by ID==1.
//
//    Jeremy Meredith, Wed Aug  3 10:22:36 PDT 2005
//    Added support for 2D files.
//
// ****************************************************************************
void avtEnzoFileFormat::Grid::DetermineExtentsInParent(vector<Grid> &grids)
{
    if (parentID != 0)
    {
        Grid &p = grids[parentID];
        minLogicalExtentsInParent[0] = int(.5 + double(p.zdims[0]) * (minSpatialExtents[0] - p.minSpatialExtents[0])/(p.maxSpatialExtents[0] - p.minSpatialExtents[0]));
        maxLogicalExtentsInParent[0] = int(.5 + double(p.zdims[0]) * (maxSpatialExtents[0] - p.minSpatialExtents[0])/(p.maxSpatialExtents[0] - p.minSpatialExtents[0]));
        minLogicalExtentsInParent[1] = int(.5 + double(p.zdims[1]) * (minSpatialExtents[1] - p.minSpatialExtents[1])/(p.maxSpatialExtents[1] - p.minSpatialExtents[1]));
        maxLogicalExtentsInParent[1] = int(.5 + double(p.zdims[1]) * (maxSpatialExtents[1] - p.minSpatialExtents[1])/(p.maxSpatialExtents[1] - p.minSpatialExtents[1]));
        if (dimension == 3)
        {
            minLogicalExtentsInParent[2] = int(.5 + double(p.zdims[2]) * (minSpatialExtents[2] - p.minSpatialExtents[2])/(p.maxSpatialExtents[2] - p.minSpatialExtents[2]));
            maxLogicalExtentsInParent[2] = int(.5 + double(p.zdims[2]) * (maxSpatialExtents[2] - p.minSpatialExtents[2])/(p.maxSpatialExtents[2] - p.minSpatialExtents[2]));
        }
        else
        {
            minLogicalExtentsInParent[2] = 0;
            maxLogicalExtentsInParent[2] = 0;
        }

        refinementRatio[0] = double(zdims[0]) / double(maxLogicalExtentsInParent[0]-minLogicalExtentsInParent[0]);
        refinementRatio[1] = double(zdims[1]) / double(maxLogicalExtentsInParent[1]-minLogicalExtentsInParent[1]);
        if (dimension == 3)
            refinementRatio[2] = double(zdims[2]) / double(maxLogicalExtentsInParent[2]-minLogicalExtentsInParent[2]);
        else
            refinementRatio[2] = 1;

        if (refinementRatio[0] != 2 ||
            refinementRatio[1] != 2 ||
            (dimension==3 && refinementRatio[2] != 2))
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
        if (dimension == 3)
            maxLogicalExtentsInParent[2] = ndims[2]-1;
        else
            maxLogicalExtentsInParent[2] = 0;
        refinementRatio[0] = refinementRatio[1] = refinementRatio[2] = 1;
    }
}

// ****************************************************************************
//  Method:  avtEnzoFileFormat::Grid::DetermineExtentsGlobally
//
//  Purpose:
//    Use the global extents from the parent, and our local extens within
//    our parent, to determine our global extents.
//
//  Programmer:  Jeremy Meredith
//  Creation:    January  3, 2005
//
//  Modifications:
//    Jeremy Meredith, Wed Feb 23 15:18:48 PST 2005
//    May have multiple root grids; identify by parentID==0, not by ID==1.
//
// ****************************************************************************
void
avtEnzoFileFormat::Grid::DetermineExtentsGlobally(int numLevels,
                                                  vector<Grid> &grids)
{
    if (parentID != 0)
    {
        Grid &p = grids[parentID];
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

// ****************************************************************************
//  Method:  avtEnzoFileFormat::ReadHierachyFile
//
//  Purpose:
//    Read the .hierarchy file and get the grid/level hierarchy.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January  6, 2005
//
//  Modifications:
//    Jeremy Meredith, Wed Aug  3 10:22:36 PDT 2005
//    Added support for 2D files.
//
// ****************************************************************************
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
    root.dimension = dimension;
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
            g.dimension = dimension;

            h >> buff; // '='
            h >> g.ID;

            int min3[3];
            int max3[3];
            while (buff != "GridStartIndex")
            {
                h >> buff;
            }
            h >> buff; // '='

            if (dimension == 3)
                h >> min3[0] >> min3[1] >> min3[2];
            else
                h >> min3[0] >> min3[1];

            while (buff != "GridEndIndex")
            {
                h >> buff;
            }
            h >> buff; // '='

            if (dimension == 3)
                h >> max3[0] >> max3[1] >> max3[2];
            else
                h >> max3[0] >> max3[1];

            g.zdims[0] = max3[0]-min3[0] + 1;
            g.zdims[1] = max3[1]-min3[1] + 1;
            if (dimension == 3)
                g.zdims[2] = max3[2]-min3[2] + 1;
            else
                g.zdims[2] = 1;

            g.ndims[0] = g.zdims[0]+1;
            g.ndims[1] = g.zdims[1]+1;
            if (dimension == 3)
                g.ndims[2] = g.zdims[2]+1;
            else
                g.ndims[2] = 1;

            while (buff != "GridLeftEdge")
            {
                h >> buff;
            }
            h >> buff; // '='

            if (dimension == 3)
                h >> g.minSpatialExtents[0] >> g.minSpatialExtents[1] >> g.minSpatialExtents[2];
            else
            {
                h >> g.minSpatialExtents[0] >> g.minSpatialExtents[1];
                g.minSpatialExtents[2] = 0;
            }

            while (buff != "GridRightEdge")
            {
                h >> buff;
            }
            h >> buff; // '='

            if (dimension == 3)
                h >> g.maxSpatialExtents[0] >> g.maxSpatialExtents[1] >> g.maxSpatialExtents[2];
            else
            {
                h >> g.maxSpatialExtents[0] >> g.maxSpatialExtents[1];
                g.maxSpatialExtents[2] = 0;
            }

            while (buff != "NumberOfParticles")
            {
                h >> buff;
            }
            h >> buff; // '='
            h >> g.numberOfParticles;

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

// ****************************************************************************
//  Method:  avtEnzoFileFormat::ReadParameterFile
//
//  Purpose:
//    The parameter file is the one without an extension, and it
//    has some problem setup stuff in it.  We are currently only
//    using it to get the cycle and time.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January  6, 2005
//
//  Modifications:
//    Jeremy Meredith, Wed Aug  3 10:22:36 PDT 2005
//    Added support for 2D files.
//
// ****************************************************************************
void
avtEnzoFileFormat::ReadParameterFile()
{
    // PARSE PARAMETER FILE

    ifstream r(fname_base.c_str());
    if (!r)
        EXCEPTION2(InvalidFilesException, fname_base.c_str(),
                   "This parameter file did not exist.");

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
        else if (buff == "TopGridRank")
        {
            r >> buff; // '='
            r >> dimension;
        }
    }
    r.close();
}

// ****************************************************************************
//  Method:  
//
//  Purpose:
//    Find the smallest grid that contains some particles, or just the smallest
//    grid if none have any particles.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January  6, 2005
//
//  Modifications:
//    Jeremy Meredith, Fri Feb 11 18:15:49 PST 2005
//    Added HDF5 support.
//
// ****************************************************************************
void
avtEnzoFileFormat::DetermineVariablesFromGridFile()
{
    int smallest_grid = 0;
    int smallest_grid_nzones = INT_MAX;
    bool found_grid_with_particles = false;
    for (int i=1; i<grids.size(); i++)
    {
        Grid &g = grids[i];
        if (found_grid_with_particles && g.numberOfParticles <= 0)
            continue;

        int numZones = g.zdims[0]*g.zdims[1]*g.zdims[2];
        if ((!found_grid_with_particles && g.numberOfParticles > 0) ||
            (numZones < smallest_grid_nzones))
        {
            if (!found_grid_with_particles ||
                (found_grid_with_particles && g.numberOfParticles > 0))
            {
                smallest_grid_nzones = numZones;
                smallest_grid = g.ID;
                found_grid_with_particles = (g.numberOfParticles > 0);
            }
        }
    }

    char gridFileName[1000];
    sprintf(gridFileName, "%s.grid%04d", fname_base.c_str(), smallest_grid);
    debug3 << "Smallest Enzo grid with particles was # "<<smallest_grid<<endl;

    int32 file_handle = SDstart(gridFileName, DFACC_READ);
    if (file_handle >= 0)
    {
        fileType = ENZO_FT_HDF4;

        int32 n_datasets;
        int32 n_file_attrs;
        SDfileinfo(file_handle, &n_datasets, &n_file_attrs);

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
    else
    {
        hid_t fileId = H5Fopen(gridFileName, H5F_ACC_RDONLY, H5P_DEFAULT);
        if (fileId < 0)
        {
            EXCEPTION1(InvalidFilesException, gridFileName);
        }

        fileType = ENZO_FT_HDF5;

        // NOTE: H5Gget_num_objs fails using a file id in HDF51.6.0, but
        //       works correctly (as the documentation says it should) in
        //       1.6.3.  Since we're going for portability, just open the
        //       darn root group and use that instead.
        hid_t rootId = H5Gopen(fileId, "/");

        hsize_t n_objs;
        H5Gget_num_objs(rootId, &n_objs);

        for (int var = 0 ; var < n_objs ; var++)
        {
            if (H5Gget_objtype_by_idx(rootId, var) == H5G_DATASET)
            {
                char  name[65];
                H5Gget_objname_by_idx(rootId, var, name, 64);

                // NOTE: to do the same diligence as HDF4 here, we should
                // really H5Dopen, H5Dget_space, H5Sget_simple_extent_ndims
                // and make sure it is a 3D (or 2D?) object before assuming
                // it is a mesh variable.  For now, assume away!
                if (strlen(name) > 8 && strncmp(name,"particle",8)==0)
                {
                    // it's a particle variable; skip over coordinate arrays
                    if (strncmp(name,"particle_position_",18) != 0)
                    {
                        particleVarNames.push_back(name);
                    }
                }
                else
                {
                    varNames.push_back(name);
                }
            }
        }

        H5Gclose(rootId);
        H5Fclose(fileId);
    }
}

// ****************************************************************************
//  Method: avtEnzoFileFormat constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   January  3, 2005
//
//  Modifications:
//    Jeremy Meredith, Fri Feb 11 18:15:49 PST 2005
//    Added HDF5 support.
//
//    Jeremy Meredith, Wed Aug  3 10:22:36 PDT 2005
//    Added support for 2D files.
//
// ****************************************************************************

avtEnzoFileFormat::avtEnzoFileFormat(const char *filename)
    : avtSTMDFileFormat(&filename, 1)
{
    fileType = ENZO_FT_UNKNOWN;
    dimension = 0;
    numGrids = 0;
    numLevels = 0;
    curTime = 0;

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

// ****************************************************************************
//  Method:  avtEnzoFileFormat destructure
//
//  Programmer:  Jeremy Meredith
//  Creation:    January  6, 2005
//
// ****************************************************************************
avtEnzoFileFormat::~avtEnzoFileFormat()
{
}

// ****************************************************************************
//  Method:  avtEnzoFileFormat::ReadAllMetaData
//
//  Purpose:
//    Read the metadata if we have not done so yet.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January  6, 2005
//
// ****************************************************************************
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

// ****************************************************************************
//  Method:  avtEnzoFileFormat::GetCycle
//
//  Purpose:
//    Return the cycle.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January  6, 2005
//
// ****************************************************************************
int
avtEnzoFileFormat::GetCycle()
{
    return curCycle;
}


// ****************************************************************************
//  Method:  avtEnzoFileFormat::GetCycleFromFilename
//
//  Purpose:
//    Do a better job at guessing the cycle number for an Enzo file.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 15, 2005
//
// ****************************************************************************
int avtEnzoFileFormat::GetCycleFromFilename(const char *cfilename) const
{
    string f = cfilename;

    string::size_type last_slash = f.rfind('/');
    if (last_slash != string::npos)
        f = f.substr(last_slash+1);

    string::size_type last_dot = f.rfind('.');
    if (last_dot != string::npos)
        f = f.substr(0, last_dot);

    string::size_type last_nondigit = f.find_last_not_of("0123456789");
    if (last_nondigit != string::npos)
        f = f.substr(last_nondigit+1);

    if (f.empty())
        return 0;

    if (f.length() > 4)
        f = f.substr(f.length() - 4);

    return atoi(f.c_str());
}

// ****************************************************************************
//  Method: avtEnzoFileFormat::ActivateTimestep
//
//  Purpose:
//      This is called by the generic database to signal that this file format
//      is going to be used soon.  It is important that each file format defer
//      as much work as possible, since there can be many (one per timestep),
//      and we won't need all their work at once, if at all.
//
//  Programmer: Jeremy Meredith
//  Creation:   July 15, 2005
//
// ****************************************************************************

void
avtEnzoFileFormat::ActivateTimestep(void)
{
    // Nothing to do
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
    grids.clear();
    varNames.clear();
    particleVarNames.clear();
    numGrids=0;
    numLevels=0;
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
//  Modifications:
//
//    Hank Childs, Thu Jan 13 15:31:08 PST 2005
//    Generate domain nesting object.
//
//    Jeremy Meredith, February 23, 2005
//    May have more than one root-level grid.  Unify extents over all of them.
//
//    Jeremy Meredith, Fri Jul 15 15:29:14 PDT 2005
//    Added cycle and time to the metadata.
//
//    Jeremy Meredith, Wed Aug  3 10:22:36 PDT 2005
//    Added support for 2D files.  Fixed problem with single-grid files.
//    Fixed dimensionality of point meshes.
//
// ****************************************************************************

void
avtEnzoFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    // read in the metadata if we have not done so yet
    ReadAllMetaData();

    if (!avtDatabase::OnlyServeUpMetaData())
        BuildDomainNesting();

    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "mesh";
    mesh->originalName = "mesh";

    mesh->meshType = AVT_RECTILINEAR_MESH;
    mesh->topologicalDimension = dimension;
    mesh->spatialDimension = dimension;
    mesh->hasSpatialExtents = true;

    mesh->minSpatialExtents[0] = grids[1].minSpatialExtents[0];
    mesh->maxSpatialExtents[0] = grids[1].maxSpatialExtents[0];
    mesh->minSpatialExtents[1] = grids[1].minSpatialExtents[1];
    mesh->maxSpatialExtents[1] = grids[1].maxSpatialExtents[1];
    if (dimension == 3)
    {
        mesh->minSpatialExtents[2] = grids[1].minSpatialExtents[2];
        mesh->maxSpatialExtents[2] = grids[1].maxSpatialExtents[2];
    }
    else
    {
        mesh->minSpatialExtents[2] = 0;
        mesh->maxSpatialExtents[2] = 0;
    }
    // now loop over all level zero grids
    for (int g = 2 ; g <= numGrids && grids[g].parentID == 0 ; g++)
    {
        for (int j = 0 ; j < dimension ; j++)
        {
            if (grids[g].minSpatialExtents[j] < mesh->minSpatialExtents[j])
                mesh->minSpatialExtents[j] = grids[g].minSpatialExtents[j];
            if (grids[g].maxSpatialExtents[j] > mesh->maxSpatialExtents[j])
                mesh->maxSpatialExtents[j] = grids[g].maxSpatialExtents[j];
        }
    }

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
    pmesh->originalName = "particles";
    pmesh->meshType = AVT_POINT_MESH;
    pmesh->topologicalDimension = 0;
    pmesh->spatialDimension = dimension;
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

    // grid variables
    for (int v = 0 ; v < varNames.size(); v++)
    {
        AddScalarVarToMetaData(md, varNames[v], "mesh", AVT_ZONECENT);
    }

    // particle variables
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

    // Populate cycle and time
    md->SetCycle(timestep, curCycle);
    md->SetTime(timestep, curTime);
    md->SetCycleIsAccurate(true, timestep);
    md->SetTimeIsAccurate(true, timestep);
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
//  Programmer: Jeremy Meredith
//  Creation:   January  6, 2005
//
//  Modifications:
//    Hank Childs, Thu Jan 13 15:31:08 PST 2005
//    Remove hack where domain boundary nesting structure is removed from
//    cache to accomodate incorrect applications in generic database.
//
//    Jeremy Meredith, Fri Feb 11 18:15:49 PST 2005
//    Added HDF5 support.
//
//    Jeremy Meredith, Wed Aug  3 10:22:36 PDT 2005
//    Added support for 2D files.  Added support for single-level files.
//
// ****************************************************************************

vtkDataSet *
avtEnzoFileFormat::GetMesh(int domain, const char *meshname)
{
    ReadAllMetaData();

    if (string(meshname) == "mesh")
    {
        // rectilinear mesh
        vtkFloatArray  *coords[3];
        int i;
        int d = domain+1;
        for (i = 0 ; i < 3 ; i++)
        {
            coords[i] = vtkFloatArray::New();
            coords[i]->SetNumberOfTuples(grids[d].ndims[i]);
            for (int j = 0 ; j < grids[d].ndims[i] ; j++)
            {
                if (i+1 > dimension)
                {
                    coords[i]->SetComponent(j, 0, 0);
                }
                else
                {
                    double c = grids[d].minSpatialExtents[i] + double(j) *
                        (grids[d].maxSpatialExtents[i]-grids[d].minSpatialExtents[i]) / 
                        double(grids[d].ndims[i]-1);
                    coords[i]->SetComponent(j, 0, c);
                }
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
    else if (fileType == ENZO_FT_HDF4)
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
        int32 var_index_z = dimension==3 ? SDnametoindex(file_handle, "particle_position_z") : -1;
        if (var_index_x < 0 || var_index_y < 0 ||
            (dimension==3 && var_index_z < 0))
        {
            // This grid didn't have any particles.  No problem -- particles
            // won't exist in every grid.  Just close the file and return
            // NULL.
            SDend(file_handle);
            return NULL;
        }

        int32 var_handle_x = SDselect(file_handle, var_index_x);
        int32 var_handle_y = SDselect(file_handle, var_index_y);
        int32 var_handle_z = dimension==3 ? SDselect(file_handle, var_index_z) : -1;
        if (var_handle_x < 0 || var_handle_y < 0 ||
            (dimension==3 && var_handle_z < 0))
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

        if (dimension == 3)
        {
            SDreaddata(var_handle_z, start, NULL, dims, ddata);
            for (i=0; i<npart; i++)
                pts[i*3+2] = float(ddata[i]);
        }
        else
        {
            for (i=0; i<npart; i++)
                pts[i*3+2] = 0;
        }

        SDendaccess(var_handle_x);
        SDendaccess(var_handle_y);
        if (dimension == 3)
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
    else if (fileType == ENZO_FT_HDF5)
    {
        // particle mesh
        char gridFileName[1000];
        sprintf(gridFileName, "%s.grid%04d", fname_base.c_str(), domain+1);

        hid_t fileId = H5Fopen(gridFileName, H5F_ACC_RDONLY, H5P_DEFAULT);
        if (fileId < 0)
        {
            EXCEPTION1(InvalidFilesException, gridFileName);
        }

        // temporarily disable error reporting
        H5E_auto_t old_errorfunc;
        void *old_clientdata;
        H5Eget_auto(&old_errorfunc, &old_clientdata);
        H5Eset_auto(NULL, NULL);

        // find the coordinate variables (if they exist)
        hid_t var_id_x = H5Dopen(fileId, "particle_position_x");
        hid_t var_id_y = H5Dopen(fileId, "particle_position_y");
        hid_t var_id_z = dimension==3 ? H5Dopen(fileId, "particle_position_z") : -1;

        // turn back on error reporting
        H5Eset_auto(old_errorfunc, old_clientdata);

        // check if the variables exist
        if (var_id_x < 0 || var_id_y < 0 ||
            (dimension==3 && var_id_z < 0))
        {
            // This grid didn't have any particles.  No problem -- particles
            // won't exist in every grid.  Just close the file and return
            // NULL.
            H5Fclose(fileId);
            return NULL;
        }

        // Get the number of particles
        hid_t spaceId = H5Dget_space(var_id_x);

        hsize_t dims[3];
        H5Sget_simple_extent_dims(spaceId, dims, NULL);

        // It's one-dimensional
        int npart = dims[0];

        vtkPoints *points  = vtkPoints::New();
        points->SetNumberOfPoints(npart);
        float *pts = (float *) points->GetVoidPointer(0);
        int i;

        double *ddata = new double[npart];
        H5Dread(var_id_x, H5T_NATIVE_DOUBLE,H5S_ALL,H5S_ALL,H5P_DEFAULT, ddata);
        for (i=0; i<npart; i++)
            pts[i*3+0] = float(ddata[i]);

        H5Dread(var_id_y, H5T_NATIVE_DOUBLE,H5S_ALL,H5S_ALL,H5P_DEFAULT, ddata);
        for (i=0; i<npart; i++)
            pts[i*3+1] = float(ddata[i]);

        if (dimension == 3)
        {
            H5Dread(var_id_z, H5T_NATIVE_DOUBLE,H5S_ALL,H5S_ALL,H5P_DEFAULT, ddata);
            for (i=0; i<npart; i++)
                pts[i*3+2] = float(ddata[i]);
        }
        else
        {
            for (i=0; i<npart; i++)
                pts[i*3+2] = 0;
        }

        H5Dclose(var_id_x);
        H5Dclose(var_id_y);
        if (dimension == 3)
            H5Dclose(var_id_z);

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
        H5Fclose(fileId);
        return ugrid;
    }

    return NULL;
}


// ****************************************************************************
//  Method: avtEnzoFileFormat::BuildDomainNesting
//
//  Purpose:
//      Generates the domain nesting object needed to remove coarse zones in
//      an AMR mesh.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 6, 2005
//
//  Modifications:
//    Hank Childs, Thu Jan 13 15:31:08 PST 2005
//    Renamed "any_mesh" to "mesh", since the generic database can now use this
//    information.
//
//    Jeremy Meredith, Thu May  5 10:28:28 PDT 2005
//    Skip the whole thing if there's only one grid.
//
//    Jeremy Meredith, Wed Aug  3 10:22:36 PDT 2005
//    Added support for 2D files.
//
// ****************************************************************************

void
avtEnzoFileFormat::BuildDomainNesting()
{
    if (numGrids <= 1)
        return;

    // first, look to see if we don't already have it cached
    void_ref_ptr vrTmp = cache->GetVoidRef("mesh",
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

        if (numLevels > 0)
        {
            avtStructuredDomainNesting *dn =
                new avtStructuredDomainNesting(numGrids, numLevels);

            dn->SetNumDimensions(dimension);

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
                if (dimension == 3)
                    logExts[5] = grids[i].maxLogicalExtentsGlobally[2]-1;
                else
                    logExts[5] = grids[i].minLogicalExtentsGlobally[2];

                dn->SetNestingForDomain(i-1, grids[i].level,
                                        childGrids, logExts);
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
//  Programmer: Jeremy Meredith
//  Creation:   January 6, 2005
//
//  Modifications:
//    Jeremy Meredith, Fri Feb 11 18:15:49 PST 2005
//    Added HDF5 support.
//
//    Jeremy Meredith, Wed Aug  3 10:22:36 PDT 2005
//    Added support for 2D files.
//
// ****************************************************************************

vtkDataArray *
avtEnzoFileFormat::GetVar(int domain, const char *varname)
{
    ReadAllMetaData();

    if (fileType == ENZO_FT_HDF4)
    {
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
        if (ndims == 2)
        {
            // force the third dimension to 1
            dims[2]=1;
        }
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
    else
    {
        // HDF5 STUFF
        char gridFileName[1000];
        sprintf(gridFileName, "%s.grid%04d", fname_base.c_str(), domain+1);

        hid_t fileId = H5Fopen(gridFileName, H5F_ACC_RDONLY, H5P_DEFAULT);
        if (fileId < 0)
        {
            EXCEPTION1(InvalidFilesException, gridFileName);
        }

        // temporarily disable error reporting
        H5E_auto_t old_errorfunc;
        void *old_clientdata;
        H5Eget_auto(&old_errorfunc, &old_clientdata);
        H5Eset_auto(NULL, NULL);

        // find the variable (if it exists)
        hid_t varId = H5Dopen(fileId, varname);

        // turn back on error reporting
        H5Eset_auto(old_errorfunc, old_clientdata);

        // check if the variable exists
        if (varId < 0)
        {
            EXCEPTION1(InvalidFilesException, gridFileName);
        }

        hid_t spaceId = H5Dget_space(varId);

        hsize_t dims[3];
        H5Sget_simple_extent_dims(spaceId, dims, NULL);

        hsize_t ndims = H5Sget_simple_extent_ndims(spaceId);

        int ntuples;
        if (ndims == 1)
            ntuples = dims[0];
        else if (ndims == 2)
            ntuples = dims[0]*dims[1];
        else if (ndims == 3)
            ntuples = dims[0]*dims[1]*dims[2];
        else
            EXCEPTION1(InvalidVariableException, varname);

        vtkFloatArray * fa = vtkFloatArray::New();
        fa->SetNumberOfTuples(ntuples);
        float *data = fa->GetPointer(0);

        double *d_data;
        int  *i_data;
        unsigned int *ui_data;
        int i;

        for (i = 0 ; i < ntuples ; i++)
            data[i] = 1;

        hid_t raw_data_type = H5Dget_type(varId);
        hid_t data_type = H5Tget_native_type(raw_data_type, H5T_DIR_ASCEND);
        if (H5Tequal(data_type, H5T_NATIVE_FLOAT)>0)
        {
            H5Dread(varId, data_type,H5S_ALL,H5S_ALL,H5P_DEFAULT, data);
        }
        else if (H5Tequal(data_type, H5T_NATIVE_DOUBLE)>0)
        {
            d_data = new double[ntuples];
            H5Dread(varId, data_type,H5S_ALL,H5S_ALL,H5P_DEFAULT, d_data);
            for (i = 0 ; i < ntuples ; i++)
                data[i] = d_data[i];
            delete[] d_data;
        }
        else if (H5Tequal(data_type, H5T_NATIVE_INT))
        {
            i_data = new int[ntuples];
            H5Dread(varId, data_type,H5S_ALL,H5S_ALL,H5P_DEFAULT, i_data);
            for (i = 0 ; i < ntuples ; i++)
                data[i] = i_data[i];
            delete[] i_data;
        }
        else if (H5Tequal(data_type, H5T_NATIVE_UINT))
        {
            ui_data = new unsigned int[ntuples];
            H5Dread(varId, data_type,H5S_ALL,H5S_ALL,H5P_DEFAULT, ui_data);
            for (i = 0 ; i < ntuples ; i++)
                data[i] = ui_data[i];
            delete[] ui_data;
        }
        else
        {
            // ERROR: UKNOWN TYPE
        }

        // Done with the type
        H5Tclose(data_type);
        H5Tclose(raw_data_type);

        // Done with the variable; don't leak it
        H5Dclose(varId);

        // For now, always close the file
        H5Fclose(fileId);

        return fa;
    }
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

// ****************************************************************************
//  Method:  avtEnzoFileFormat::GetAuxiliaryData
//
//  Purpose:
//    Right now this only supports spatial interval trees.  There is no
//    other information like materials and species available.
//
//  Arguments:
//    type       the kind of auxiliary data to create
//
//  Programmer:  Jeremy Meredith
//  Creation:    January  6, 2005
//
// ****************************************************************************
void *
avtEnzoFileFormat::GetAuxiliaryData(const char *var, int dom, 
                                    const char * type, void *,
                                    DestructorFunction &df)
{
    if (type != AUXILIARY_DATA_SPATIAL_EXTENTS)
        return NULL;

    avtIntervalTree *itree = new avtIntervalTree(numGrids, 3);

    for (int grid = 1 ; grid <= numGrids ; grid++)
    {
        float bounds[6];
        bounds[0] = grids[grid].minSpatialExtents[0];
        bounds[1] = grids[grid].maxSpatialExtents[0];
        bounds[2] = grids[grid].minSpatialExtents[1];
        bounds[3] = grids[grid].maxSpatialExtents[1];
        bounds[4] = grids[grid].minSpatialExtents[2];
        bounds[5] = grids[grid].maxSpatialExtents[2];
        itree->AddDomain(grid-1, bounds);
    }
    itree->Calculate(true);

    df = avtIntervalTree::Destruct;

    return ((void *) itree);
}

