// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtPixieFileFormat.C                           //
// ************************************************************************* //

// define this to make sure the plugin always serves up float data to VisIt
// this was necessary prior to avtGenericDatabase having generic conversion
// capability
//#define FORCE_FLOATS


#ifdef __APPLE__
#include <malloc/malloc.h>
#include <cstdlib>
#include <cstdio>
#endif

static void
CheckHeap(const char *where)
{
return;
#ifdef __APPLE__
    if (!malloc_zone_check(NULL))
    {
        fprintf(stderr, "PIXIE HEAP CORRUPTION DETECTED: %s\n", where);
        fflush(stderr);
        abort();
    }
#endif
}

#include <avtPixieFileFormat.h>

#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkDoubleArray.h>
#include <vtkIntArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <avtParallel.h>
#include <DebugStream.h>
#include <DBOptionsAttributes.h>
#include <Expression.h>
#include <InvalidVariableException.h>
#include <InvalidDBTypeException.h>
#include <InvalidFilesException.h>
#include <InvalidTimeStepException.h>

// Define this symbol BEFORE including hdf5.h to indicate the HDF5 code
// in this file uses version 1.6 of the HDF5 API. This is harmless for
// versions of HDF5 before 1.8 and ensures correct compilation with
// version 1.8 and thereafter. When, and if, the HDF5 code in this file
// is explicitly upgraded to the 1.8 API, this symbol should be removed.
#define H5_USE_16_API
#include <hdf5.h>
#include <visit-hdf5.h>
#include <avtGhostData.h>
#include <algorithm>
#include <string>
#include <vector>

//
// These routines detect whether the file looks like a certain flavor of
// HDF5 file reader. If the flavor is detected, we throw an exception so
// the proper reader can be used instead of Pixie.
//
static void DetectSilo(int fileId)
{
    hid_t siloDir = H5Gopen(fileId, ".silo");
    if (siloDir >= 0)
    {
        H5Gclose(siloDir);
        H5Fclose(fileId);
        EXCEPTION1(InvalidDBTypeException,
                   "Cannot be a Pixie file because it looks like a Silo file.");
    }
}

static void DetectTetrad(int fileId)
{
    hid_t cell_array = H5Dopen(fileId, "CellArray");
    if (cell_array >= 0)
    {
        H5Dclose(cell_array);
        H5Fclose(fileId);
        EXCEPTION1(InvalidDBTypeException,
                   "Cannot be a Pixie file because it looks like a Tetrad file.");
    }
}

static void DetectPFLOTRAN(int fileId)
{
    hid_t coordsGID = H5Gopen(fileId, "Coordinates");
    if (coordsGID >= 0)
    {
        H5Gclose(coordsGID);
        H5Fclose(fileId);
        EXCEPTION1(InvalidDBTypeException,
                   "Cannot be a Pixie file because it looks like a PFLOTRAN file.");
    }
}

static void DetectUNIC(int fileId)
{
    hid_t control = H5Dopen(fileId, "CONTROL");
    if (control >= 0)
    {
        H5Dclose(control);
        H5Fclose(fileId);
        EXCEPTION1(InvalidDBTypeException,
                   "Cannot be a Pixie file because it looks like an UNIC file.");
    }
}

static void DetectVisSchema(int fileId)
{
    hid_t runInfo = H5Gopen(fileId, "runInfo");
    if (runInfo >= 0)
    {
        hid_t vsVersion = H5Aopen_name(runInfo, "vsVersion");
        if (vsVersion >= 0)
        {
            H5Aclose(vsVersion);
            H5Gclose(runInfo);
            H5Fclose(fileId);
            EXCEPTION1(InvalidDBTypeException,
                       "Cannot be a Pixie file because it looks like a VizSchema file.");
        }

        hid_t vsVsVersion = H5Aopen_name(runInfo, "vsVsVersion");
        if (vsVsVersion >= 0)
        {
            H5Aclose(vsVsVersion);
            H5Gclose(runInfo);
            H5Fclose(fileId);
            EXCEPTION1(InvalidDBTypeException,
                       "Cannot be a Pixie file because it looks like a VizSchema file.");
        }

        hid_t software = H5Aopen_name(runInfo, "software");
        hid_t version = H5Aopen_name(runInfo, "version");
        H5Gclose(runInfo);
        if (software >=0 && version >=0)
        {
            H5Aclose(software);
            H5Aclose(version);
            H5Fclose(fileId);
            EXCEPTION1(InvalidDBTypeException,
                       "Cannot be a Pixie file because it looks like a legacy VizSchema file.");
        }
        if (software >=0) H5Aclose(software);
        if (version >=0) H5Aclose(version);
    }
}
static void DetectTyphonIO(int fileId)
{
    //
    //See if file is TyphonIO[v0] format
    //
    hid_t tio_root = H5Gopen(fileId, "/");
    hid_t tio_version = H5Aopen_name(tio_root, "TIO_version_major");
    if (tio_version >= 0)
    {
        H5Aclose(tio_version);
        H5Gclose(tio_root);
        H5Fclose(fileId);
        EXCEPTION1(InvalidDBTypeException,
                   "Cannot be a Pixie file because it looks like a TyphonIO file.");
    }
    if (tio_root >= 0) H5Gclose(tio_root);
    hid_t tio_info = H5Gopen(fileId, "/TyphonIO_FileInfo");
    if (tio_info >= 0)
    {
        H5Gclose(tio_info);
        H5Fclose(fileId);
        EXCEPTION1(InvalidDBTypeException,
                   "Cannot be a Pixie file because it looks like a TyphonIOv0 file.");
    }    
}
// ****************************************************************************
//  Method: avtPixie constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Aug 13 14:31:43 PST 2004
//
//  Modifications:
//   Jeremy Meredith, Fri Jan 15 17:22:30 EST 2010
//   Don't need to call Initialize here -- it will be called when we need it,
//   even if we are being strict about ruling out non-Pixie files.
//
//   Eric Brugger, Tue Mar  9 15:20:50 PST 2010
//   Moved the code to turn off error message printing to the constructor
//   so that it is always called.
//
//   Jean Favre, Thu Jun 21 16:28:40 PDT 2012
//   Add read options.
//
// ****************************************************************************

avtPixieFileFormat::avtPixieFileFormat(const char *filename, const DBOptionsAttributes *readOpts)
    : avtMTSDFileFormat(&filename, 1), variables(), meshes(),
      timeStatePrefix("/Timestep ")
{
CheckHeap("Constructor");
    fileId = -1;
    nTimeStates = 0;
    metadataIsTimeInvariant = true;
    partitioning = PixieDBOptions::ZSLAB;
    duplicateData = false;

    if (readOpts != NULL)
    {
        for (int i = 0; i < readOpts->GetNumberOfOptions(); ++i)
        {
            if (readOpts->GetName(i) == PixieDBOptions::RDOPT_PARTITIONING)
                partitioning = (PixieDBOptions::PartitioningDirection) readOpts->GetEnum(PixieDBOptions::RDOPT_PARTITIONING);
            else if (readOpts->GetName(i) == PixieDBOptions::RDOPT_DUPLICATE)
                duplicateData = readOpts->GetBool(PixieDBOptions::RDOPT_DUPLICATE);
            else
                debug1 << "Ignoring unknown option " << readOpts->GetName(i) << endl;
        }
    }

    // Turn off error message printing.
    H5Eset_auto(NULL,NULL);
CheckHeap("End Constructor");
}

// ****************************************************************************
// Method: avtPixieFileFormat::~avtPixieFileFormat
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 13 14:37:34 PST 2004
//
// Modifications:
//
// ****************************************************************************

avtPixieFileFormat::~avtPixieFileFormat()
{
    if(fileId >= 0)
    {
        H5Fclose(fileId);
        fileId = -1;
    }
}

// ****************************************************************************
// Method: avtPixieFileFormat::GetCycles
//
// Purpose:
//   Gets the cycles.
//
// Arguments:
//   cycles : Return vector for the times.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 13 18:30:26 PST 2004
//
// Modifications:
//    Eric Brugger, Mon Nov 29 15:52:39 PST 2004
//    Modified the reader to handle gaps in the cycle numbering (e.g. allowing
//    0, 10, 20, 30 instead of requiring 0, 1, 2, 3).
//
//    Brad Whitlock, Thu Apr 27 11:49:07 PDT 2006
//    Fixed it so it works if cycles are never read.
//
// ****************************************************************************

void
avtPixieFileFormat::GetCycles(std::vector<int> &cycles)
{
    size_t nts = (nTimeStates < 1) ? 1 : nTimeStates;
    int lastCycle = 0;
    for(size_t i = 0; i < nts; ++i)
    {
        if(i < this->cycles.size())
        {
            cycles.push_back(this->cycles[i]);
            lastCycle = this->cycles[i];
        }
        else
        {
            cycles.push_back(lastCycle++);
        }
    }
}

// ****************************************************************************
// Method: avtPixieFileFormat::GetTimes
//
// Purpose:
//   Gets the times.
//
// Arguments:
//   times : Return vector for the times.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 13 18:30:05 PST 2004
//
// Modifications:
//    Eric Brugger, Mon Nov 29 15:52:39 PST 2004
//    Modified the reader to handle gaps in the cycle numbering (e.g. allowing
//    0, 10, 20, 30 instead of requiring 0, 1, 2, 3).
//
//    Brad Whitlock, Thu Apr 27 11:49:07 PDT 2006
//    Fixed it so it works if cycles are never read.
//
//    Luis Chacon, Tue Mar 2 10:02:00 EST 2010
//    Modified routine to read time values in double vector time_val (if available)
//
// ****************************************************************************

void
avtPixieFileFormat::GetTimes(std::vector<double> &times)
{
    size_t nts = (nTimeStates < 1) ? 1 : nTimeStates;
    double lastTime = 0.;
    for(size_t i = 0; i < nts; ++i)
    {
        if(i < cycles.size())
        {
            if (time_val.size() == (size_t)nTimeStates)
            {
                times.push_back(time_val[i]);
                lastTime = time_val[i];
            }
            else
            {
                times.push_back(double(cycles[i]));
                lastTime = double(cycles[i]);
            }
        }
        else
        {
            times.push_back(lastTime);
            lastTime = lastTime + 1.;
        }
    }
}

// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Aug 13 14:31:43 PST 2004
//
// ****************************************************************************

int
avtPixieFileFormat::GetNTimesteps(void)
{
    return (nTimeStates < 1) ? 1 : nTimeStates;
}


// ****************************************************************************
//  Method: avtPixieFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Aug 13 14:31:43 PST 2004
//
// ****************************************************************************

void
avtPixieFileFormat::FreeUpResources(void)
{
    if(fileId >= 0)
    {
        H5Fclose(fileId);
        fileId = -1;
    }
}

// ****************************************************************************
// Method: avtPixieFileFormat::Initialize
//
// Purpose:
//   Initializes the file format by reading the file and the contents, etc.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 13 15:21:08 PST 2004
//
// Modifications:
//   Brad Whitlock, Wed Sep 15 17:45:41 PST 2004
//   Added better support for determining whether arrays have coordinates.
//
//   Eric Brugger, Mon Nov 29 15:52:39 PST 2004
//   Modified the reader to handle gaps in the cycle numbering (e.g. allowing
//   0, 10, 20, 30 instead of requiring 0, 1, 2, 3).
//
//   Mark C. Miller, Mon Dec  6 14:13:11 PST 2004
//   Added std:: name resolution to call to sot
//
//   Mark C. Miller, Mon Apr  4 14:55:14 PDT 2005
//   Added expressions
//
//   Hank Childs, Wed Jul  9 06:02:00 PDT 2008
//   Added test for UNIC.
//
//   Gunther H. Weber, Wed Oct  8 16:50:31 PDT 2008
//   Added test for TechX VizSchema
//
//   Jeremy Meredith, Thu Jan  7 15:36:19 EST 2010
//   Close all open ids when returning an exception.  Added error detection.
//
//   Jeremy Meredith, Fri Jan 15 17:07:33 EST 2010
//   Added detection of and failure for Silo-looking HDF5 files.
//
//   Jeremy Meredith, Fri Jan 15 17:25:23 EST 2010
//   Use runtime strictness checking behavior.
//
//   Eric Brugger, Tue Mar  9 15:20:50 PST 2010
//   Moved the code to turn off error message printing to the constructor
//   so that it is always called.
//
//   Luis Chacon, Tue Mar 2 10:02:00 EST 2010
//   Added code to sort time values
//
//   Jeremy Meredith, Wed Mar 30 11:24:30 EDT 2011
//   Add a test to avoid opening PFLOTRAN files (though like the others tests,
//   it will be skipped if the user does Open As so we're not in strict mode).
//
//   Brad Whitlock, Thu Oct 25 10:18:53 PDT 2012
//   Move detection of other formats to helper functions.
//
//   Satheesh Maheswaran, Thu Oct 26 14:14:53 PDT 2012
//   Added static function for detecting TyphonIO function
//
//   Eric Brugger, Fri May 22 13:37:44 PDT 2020
//   Corrected a bug reading curvilinear meshes in parallel. I added
//   isCoord to TraversalInfo and VarInfo to track if a variable is a
//   coordinate array so that the decomposition can be done correctly.
//
// ****************************************************************************

void
avtPixieFileFormat::Initialize()
{
CheckHeap("entry Initialize");
    if(fileId == -1)
    {
        // Initialize some variables.
        meshes.clear();
        variables.clear();
        staticVariables.clear();
        stateVariables.clear();
        cycles.clear();
        time_val.clear();
        nTimeStates = 0;
        metadataIsTimeInvariant = true;
        hid_t fileAccessPropListID = H5Pcreate(H5P_FILE_ACCESS);
        if (fileAccessPropListID < 0)
        {
            EXCEPTION1(ImproperUseException, "Couldn't H5Pcreate");
        }
        herr_t err = H5Pset_fclose_degree(fileAccessPropListID, H5F_CLOSE_SEMI);
        if (err < 0)
        {
            EXCEPTION1(ImproperUseException, "Couldn't set file close access");
        }
        if((fileId = H5Fopen(filenames[0], H5F_ACC_RDONLY, fileAccessPropListID)) < 0)
        {
            char error[1024];
            snprintf(error, 1024, "Cannot be a pixie file (%s)",filenames[0]);

            EXCEPTION1(InvalidDBTypeException, error);
        }
        H5Pclose(fileAccessPropListID);
CheckHeap("entry strict mode");
        if (GetStrictMode())
        {
            //
            // See if the file format looks like a some other file format.
            // I know it's hackish to have to check like this but how else
            // should it be done when we don't want Pixie to read HDF5 files
            // that are really HDF5 files following a convention supported
            // by another format.
            //
            DetectSilo(fileId);
            DetectTetrad(fileId);
            DetectPFLOTRAN(fileId);
            DetectUNIC(fileId);
            DetectVisSchema(fileId);
            DetectTyphonIO(fileId);
        }

CheckHeap("entry populate scalar var list");
        // Populate the scalar variable list
        hid_t gid;
        if ((gid = H5Gopen(fileId, "/")) < 0)
        {
            H5Fclose(fileId);
            EXCEPTION1(InvalidFilesException, (const char *)filenames[0]);
        }
        TraversalInfo info;
        info.This = this;
        info.level = 0;
        info.path = "/";
        info.hasCoords = false;
        info.isCoord = false;
        info.coordX = "";
        info.coordY = "";
        info.coordZ = "";
        info.cycle = -1;

        // ARS - Note as of 1.8.0 H5Giterate has been deprecated and
        // H5Literate should be used. At the same time H5Literate will
        // traverse not only groups but all links inlcuding external
        // links. As such, code is in place to do this.

        // Iterate over the items in this group.
CheckHeap("entry H5Giterate");
        H5Giterate(fileId, "/", NULL, GetVariableList, (void*)&info);
//      H5Literate(fileId, H5_INDEX_NAME, H5_ITER_INC, 0, VisitLinks, (void*)&info);
        H5Gclose(gid);
CheckHeap("end H5Giterate begin expressions");

        // Per-timestep coordinate tagging and mesh construction are done
        // after traversal, once all variable shapes are known.

        //
        // Look for expressions dataset
        //
        hid_t expid;
        if ((expid = H5Dopen(fileId,"/visit_expressions")) >= 0)
        {
            // examine size, dimensionality and type of the dataspace
            hid_t spid    = H5Dget_space(expid);
            hid_t tyid    = H5Dget_type(expid);
            hsize_t hsize = H5Dget_storage_size(expid);
            int ndims     = H5Sget_simple_extent_ndims(spid);

            // should be a 1D, character data set
            if (ndims != 1 || H5Tget_class(tyid) != H5T_STRING)
            {
                EXCEPTION2(InvalidFilesException, (const char *)filenames[0],
                           "The dataset \"visit_expressions\" is not a 1D, character dataset");
            }

            // allocate and read
            char *expChars = new char[hsize+1];
            if (H5Dread(expid, tyid, H5S_ALL, H5S_ALL, H5P_DEFAULT, expChars) < 0)
            {
                EXCEPTION1(InvalidVariableException, "/visit_expressions");
            }
            expChars[hsize] = '\0';

            rawExpressionString = expChars;
            delete [] expChars;

            H5Tclose(tyid);
            H5Sclose(spid);
            H5Dclose(expid);
        }
CheckHeap("end expressions begin timestep stuff");

        // Sort the cycles and the times.
        std::sort(cycles.begin(), cycles.end());
        std::sort(time_val.begin(), time_val.end());

        // Build the first state's schema for compatibility with the existing
        // read/partition code, and determine whether metadata can be reused
        // across all states.
        PrepareTimestepInfo(0);
        if(cycles.size() > 1)
        {
            VarInfoMap firstVars = variables;
            VarInfoMap firstMeshes = meshes;
            for(size_t ts = 1; ts < cycles.size(); ++ts)
            {
                VarInfoMap stateVars = GetVariablesForTimestep((int)ts);
                VarInfoMap stateMeshes;
                BuildMeshesForTimestep(stateVars, stateMeshes);
                if(!SameSchema(firstVars, stateVars) ||
                   !SameSchema(firstMeshes, stateMeshes))
                {
                    metadataIsTimeInvariant = false;
                    break;
                }
            }
            PrepareTimestepInfo(0);
        }

#ifdef MDSERVER
        // We're on the mdserver so close the file now that we've determined
        // the variables in it.
        H5Fclose(fileId);
        fileId = -1;
#endif
    }
}

avtPixieFileFormat::VarInfoMap
avtPixieFileFormat::GetVariablesForTimestep(int timestate) const
{
    VarInfoMap retval = staticVariables;

    if(nTimeStates > 0 && timestate >= 0 && timestate < (int)cycles.size())
    {
        StateVarInfoMap::const_iterator sit = stateVariables.find(cycles[timestate]);
        if(sit != stateVariables.end())
        {
            for(VarInfoMap::const_iterator it = sit->second.begin();
                it != sit->second.end(); ++it)
                retval[it->first] = it->second;
        }
    }

    // Coordinate arrays named by a valid coords attribute are node-centered.
    for(VarInfoMap::const_iterator it = retval.begin(); it != retval.end(); ++it)
    {
        if(!it->second.hasCoords)
            continue;
        if(retval.count(it->second.coordX)) retval[it->second.coordX].isCoord = true;
        if(retval.count(it->second.coordY)) retval[it->second.coordY].isCoord = true;
        if(retval.count(it->second.coordZ)) retval[it->second.coordZ].isCoord = true;
    }

    return retval;
}

bool
avtPixieFileFormat::IsNodal(const VarInfo &info, const VarInfoMap &vars) const
{
    if(info.hasCoords || info.isCoord)
        return true;

    int nSpatialDims = 0;
    DetermineVarDimensions(info, 0, 0, nSpatialDims);
    if(nSpatialDims < 2)
        return false;

    bool active[3] = {info.dims[0] > 1, info.dims[1] > 1, info.dims[2] > 1};
    for(VarInfoMap::const_iterator it = vars.begin(); it != vars.end(); ++it)
    {
        if(&it->second == &info)
            continue;

        bool match = true;
        bool anyActive = false;
        for(int d = 0; d < 3; ++d)
        {
            bool otherActive = it->second.dims[d] > 1;
            if(active[d] != otherActive)
            {
                match = false;
                break;
            }
            if(active[d])
            {
                anyActive = true;
                if(info.dims[d] != it->second.dims[d] + 1)
                {
                    match = false;
                    break;
                }
            }
        }
        if(match && anyActive)
            return true;
    }

    return false;
}

avtPixieFileFormat::MeshKey
avtPixieFileFormat::GetMeshKey(const VarInfo &info, const VarInfoMap &vars) const
{
    MeshKey key;
    key.curvilinear = info.hasCoords;
    key.coordX = info.hasCoords ? info.coordX : "";
    key.coordY = info.hasCoords ? info.coordY : "";
    key.coordZ = info.hasCoords ? info.coordZ : "";

    bool nodal = IsNodal(info, vars);
    int size1D = info.hasCoords ? 2 : 1;
    for(int d = 0; d < 3; ++d)
    {
        key.dims[d] = info.dims[d];
        if(!nodal && info.dims[d] > (size_t)size1D)
            ++key.dims[d];
    }
    return key;
}

void
avtPixieFileFormat::BuildMeshesForTimestep(VarInfoMap &vars,
                                            VarInfoMap &stateMeshes) const
{
    stateMeshes.clear();
    typedef std::map<MeshKey, std::string> MeshNameMap;
    MeshNameMap meshNames;
    std::vector<MeshKey> curvilinearKeys;

    // First create explicit curvilinear mesh families. The coords attribute is
    // authoritative; raw coordinate datasets do not create meshes themselves.
    for(VarInfoMap::iterator it = vars.begin(); it != vars.end(); ++it)
    {
        if(it->second.isCoord || !it->second.hasCoords)
            continue;

        VarInfoMap::const_iterator x = vars.find(it->second.coordX);
        VarInfoMap::const_iterator y = vars.find(it->second.coordY);
        VarInfoMap::const_iterator z = vars.find(it->second.coordZ);
        if(x == vars.end() || y == vars.end() || z == vars.end())
            continue;

        bool same = true;
        for(int d = 0; d < 3; ++d)
            same &= x->second.dims[d] == it->second.dims[d] &&
                    y->second.dims[d] == it->second.dims[d] &&
                    z->second.dims[d] == it->second.dims[d];
        if(!same)
            continue;

        MeshKey key = GetMeshKey(it->second, vars);
        if(meshNames.find(key) == meshNames.end())
        {
            char tmp[128];
            snprintf(tmp, sizeof(tmp), "curvemesh_%dx%dx%d",
                     int(key.dims[2]), int(key.dims[1]), int(key.dims[0]));
            std::string name(tmp);
            if(stateMeshes.count(name))
            {
                int n = 2;
                std::string candidate;
                do
                {
                    char suffix[32];
                    snprintf(suffix, sizeof(suffix), "_%d", n++);
                    candidate = name + suffix;
                } while(stateMeshes.count(candidate));
                name = candidate;
            }
            meshNames[key] = name;
            stateMeshes[name] = it->second;
            curvilinearKeys.push_back(key);
        }
        it->second.meshName = meshNames[key];
    }

    // Assign every remaining spatial data variable to a mesh family. A
    // KxMxN zonal array and a (K+1)x(M+1)x(N+1) nodal array canonicalize to
    // the same node dimensions and therefore the same mesh.
    for(VarInfoMap::iterator it = vars.begin(); it != vars.end(); ++it)
    {
        if(it->second.isCoord || !it->second.meshName.empty())
            continue;

        int nSpatialDims = 0;
        DetermineVarDimensions(it->second, 0, 0, nSpatialDims);
        if(nSpatialDims < 2)
            continue;

        MeshKey rkey = GetMeshKey(it->second, vars);

        // If an explicit curvilinear mesh has these canonical node
        // dimensions, variables without their own coords attribute still live
        // on that mesh (e.g. 32^3 zonal data with 33^3 node coordinates).
        std::string matchingCurve;
        for(size_t k = 0; k < curvilinearKeys.size(); ++k)
        {
            bool sameDims = true;
            for(int d = 0; d < 3; ++d)
                sameDims &= curvilinearKeys[k].dims[d] == rkey.dims[d];
            if(sameDims)
            {
                matchingCurve = meshNames[curvilinearKeys[k]];
                break;
            }
        }
        if(!matchingCurve.empty())
        {
            it->second.meshName = matchingCurve;
            continue;
        }

        rkey.curvilinear = false;
        rkey.coordX.clear(); rkey.coordY.clear(); rkey.coordZ.clear();
        if(meshNames.find(rkey) == meshNames.end())
        {
            char tmp[128];
            snprintf(tmp, sizeof(tmp), "mesh_%dx%dx%d",
                     int(rkey.dims[2]), int(rkey.dims[1]), int(rkey.dims[0]));
            std::string name(tmp);
            meshNames[rkey] = name;
            stateMeshes[name] = it->second;
        }
        else
        {
            VarInfo &old = stateMeshes[meshNames[rkey]];
            if(!IsNodal(old, vars) && IsNodal(it->second, vars))
                old = it->second;
        }
        it->second.meshName = meshNames[rkey];
    }
}

bool
avtPixieFileFormat::SameSchema(const VarInfoMap &a, const VarInfoMap &b) const
{
    if(a.size() != b.size())
        return false;

    VarInfoMap::const_iterator ia = a.begin();
    VarInfoMap::const_iterator ib = b.begin();
    for(; ia != a.end(); ++ia, ++ib)
    {
        if(ia->first != ib->first)
            return false;
        for(int d = 0; d < 3; ++d)
            if(ia->second.dims[d] != ib->second.dims[d])
                return false;
        if(ia->second.hasCoords != ib->second.hasCoords ||
           ia->second.coordX != ib->second.coordX ||
           ia->second.coordY != ib->second.coordY ||
           ia->second.coordZ != ib->second.coordZ ||
           ia->second.meshName != ib->second.meshName)
            return false;
    }
    return true;
}

void
avtPixieFileFormat::PrepareTimestepInfo(int timestate)
{
    variables = GetVariablesForTimestep(timestate);
    BuildMeshesForTimestep(variables, meshes);
    PartitionDims();
}

void
avtPixieFileFormat::SetFullExtents(VarInfo &info) const
{
    for(int d = 0; d < 3; ++d)
    {
        info.start[d] = 0;
        info.count[d] = info.dims[d];
        info.start_no_ghost[d] = 0;
        info.count_no_ghost[d] = info.dims[d];
    }
}

bool
avtPixieFileFormat::IsNodal(const VarInfo &info) const
{
    return IsNodal(info, variables);
}

void
avtPixieFileFormat::PartitionVarInfo(VarInfo &info) const
{
    SetFullExtents(info);

    int nSpatialDims = 0;
    DetermineVarDimensions(info, 0, 0, nSpatialDims);
    if(nSpatialDims == 0)
        return;

    bool nodal = IsNodal(info);
    int size1D = info.hasCoords ? 2 : 1;
    int hdfZoneDims[3] = {1, 1, 1};
    bool active[3] = {false, false, false};
    for(int d = 0; d < 3; ++d)
    {
        active[d] = info.dims[d] > (size_t)size1D;
        if(active[d])
            hdfZoneDims[d] = int(info.dims[d]) - (nodal ? 1 : 0);
    }

    int domCount[3] = {1, 1, 1};
    int domLogicalCoords[3] = {0, 0, 0};
    switch(partitioning)
    {
    case PixieDBOptions::XSLAB:
        domCount[2] = PAR_Size();
        domLogicalCoords[2] = PAR_Rank();
        break;
    case PixieDBOptions::YSLAB:
        domCount[1] = PAR_Size();
        domLogicalCoords[1] = PAR_Rank();
        break;
    case PixieDBOptions::ZSLAB:
        domCount[0] = PAR_Size();
        domLogicalCoords[0] = PAR_Rank();
        break;
    case PixieDBOptions::KDTREE:
    {
        int xyzZoneDims[3] = {hdfZoneDims[2], hdfZoneDims[1], hdfZoneDims[0]};
        int xyzDomCount[3] = {1, 1, 1};
        int xyzDomLogicalCoords[3] = {0, 0, 0};
        avtDatabase::ComputeRectilinearDecomposition(nSpatialDims, PAR_Size(),
            xyzZoneDims[0], xyzZoneDims[1], xyzZoneDims[2],
            &xyzDomCount[0], &xyzDomCount[1], &xyzDomCount[2]);
        avtDatabase::ComputeDomainLogicalCoords(nSpatialDims, xyzDomCount,
            PAR_Rank(), xyzDomLogicalCoords);
        domCount[0] = xyzDomCount[2];
        domCount[1] = xyzDomCount[1];
        domCount[2] = xyzDomCount[0];
        domLogicalCoords[0] = xyzDomLogicalCoords[2];
        domLogicalCoords[1] = xyzDomLogicalCoords[1];
        domLogicalCoords[2] = xyzDomLogicalCoords[0];
        break;
    }
    }

    for(int d = 0; d < 3; ++d)
    {
        if(!active[d])
            continue;

        int baseZoneCount = hdfZoneDims[d] / domCount[d];
        int extraZones = hdfZoneDims[d] % domCount[d];
        int realZoneCount = baseZoneCount +
            (domLogicalCoords[d] < extraZones ? 1 : 0);
        int realZoneStart = domLogicalCoords[d] * baseZoneCount +
            std::min(domLogicalCoords[d], extraZones);
        int ghostZoneStart = std::max(0, realZoneStart - 1);
        int realZoneEnd = realZoneStart + realZoneCount - 1;
        int ghostZoneEnd = std::min(hdfZoneDims[d] - 1, realZoneEnd + 1);
        int ghostZoneCount = ghostZoneEnd - ghostZoneStart + 1;

        info.start[d] = ghostZoneStart;
        info.start_no_ghost[d] = realZoneStart;
        if(nodal)
        {
            info.count[d] = ghostZoneCount + 1;
            info.count_no_ghost[d] = realZoneCount + 1;
        }
        else
        {
            info.count[d] = ghostZoneCount;
            info.count_no_ghost[d] = realZoneCount;
        }
    }
}

// ****************************************************************************
// Method: avtPixieFileFormat::PartitionDims
//
// Purpose:
//   calculates the local count and start of a hyperslab before reading variable
//
// Programmer: Jean Favre
// Creation:   Thu Dec 22 10:46:13 CET 2011
//
// Modifications:
//    Jean Favre, Wed Sep  9 14:11:03 CEST 2015
//    Added a switch between PieceToExtent and PieceToExtentByPoints to
//    correctly handle cell-based data ghosting
//
//    Eric Brugger, Fri May 22 13:37:44 PDT 2020
//    Corrected a bug reading curvilinear meshes in parallel. I added
//    isCoord to TraversalInfo and VarInfo to track if a variable is a
//    coordinate array so that the decomposition can be done correctly.
//
//    Mark C. Miller, Fri Jun  5 15:38:06 PDT 2026
//    Applied same fix as Eric, above, for other mesh case.
//
//    Mark C. Miller, Thu Aug  6 11:45:00 PDT 2026
//    Use avtDatabase's rectilinear decomposition helpers for k-d tree
//    partitioning and derive HDF5 hyperslabs from zone extents according
//    to variable centering.
//
// ****************************************************************************

void
avtPixieFileFormat::PartitionDims()
{
    VarInfoMap::iterator it;
    for(it = variables.begin(); it != variables.end(); ++it)
    {
        if (resultMustBeProducedOnlyOnThisProcessor || duplicateData)
            SetFullExtents(it->second);
        else
            PartitionVarInfo(it->second);
        if(!PAR_Rank())
        {
            debug4 << PAR_Rank() << " : " << it->second.fileVarName
                   /*
                                      << " e=["<< extents[0]
                                      << ","<< extents[1]
                                      << ","<< extents[2]
                                      << " ,"<< extents[3]
                                      << ","<< extents[4]
                                      << ","<< extents[5] << "]"
                   */
                   << " d=["<< it->second.dims[0]
                   << "x"<< it->second.dims[1]
                   << "x"<< it->second.dims[2] <<"]"

                   << " c=["<< it->second.count[0]
                   << "x"<< it->second.count[1]
                   << "x"<< it->second.count[2] <<"]"

                   << " s=["<< it->second.start[0]
                   << "x"<< it->second.start[1]
                   << "x"<< it->second.start[2] << "]"

                   << " c0=["<< it->second.count_no_ghost[0]
                   << "x"<< it->second.count_no_ghost[1]
                   << "x"<< it->second.count_no_ghost[2] <<"]"

                   << " s0=["<< it->second.start_no_ghost[0]
                   << "x"<< it->second.start_no_ghost[1]
                   << "x"<< it->second.start_no_ghost[2] << "]"
                   <<"\n";
        }
    }

    for(it = meshes.begin(); it != meshes.end(); ++it)
    {
        if (resultMustBeProducedOnlyOnThisProcessor || duplicateData)
            SetFullExtents(it->second);
        else
            PartitionVarInfo(it->second);
        if(!PAR_Rank())
        {
            debug4 << PAR_Rank() << " : " << it->second.fileVarName
                   /*
                                      << " e=["<< extents[0]
                                      << ","<< extents[1]
                                      << ","<< extents[2]
                                      << " ,"<< extents[3]
                                      << ","<< extents[4]
                                      << ","<< extents[5] << "]"
                   */
                   << " d=["<< it->second.dims[0]
                   << "x"<< it->second.dims[1]
                   << "x"<< it->second.dims[2] <<"]"

                   << " c=["<< it->second.count[0]
                   << "x"<< it->second.count[1]
                   << "x"<< it->second.count[2] <<"]"

                   << " s=["<< it->second.start[0]
                   << "x"<< it->second.start[1]
                   << "x"<< it->second.start[2] << "]"

                   << " c0=["<< it->second.count_no_ghost[0]
                   << "x"<< it->second.count_no_ghost[1]
                   << "x"<< it->second.count_no_ghost[2] <<"]"

                   << " s0=["<< it->second.start_no_ghost[0]
                   << "x"<< it->second.start_no_ghost[1]
                   << "x"<< it->second.start_no_ghost[2] << "]"
                   <<"\n";
        }
    }
}

// ****************************************************************************
// Method: avtPixieFileFormat::ActivateTimestep
//
// Purpose:
//   This method is called each time we change to a new time state. Make
//   sure that the file has been initialized.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 14 12:53:08 PDT 2004
//
// Modifications:
//
// ****************************************************************************

void
avtPixieFileFormat::ActivateTimestep(int ts)
{
    //
    // Initialize the file if it has not been initialized.
    //
    debug4 << "avtPixieFileFormat::ActivateTimestep: ts=" << ts << endl;
    Initialize();
    PrepareTimestepInfo(ts);
}

// ****************************************************************************
// Method: avtPixieFileFormat::DetermineVarDimensions
//
// Purpose:
//   Gets the dimensions of a variable.
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 19 10:57:09 PDT 2004
//
// Modifications:
//   Eric Brugger, Tue Oct 26 08:36:53 PDT 2004
//   I modified the routine to return the hyperslab to read of an array.  I
//   also modified the routine to handle the fact that any arrays associated
//   with a curvilinear mesh that are 2*nx*ny should be treated as 2d.
//
//   Jean Favre, Fri Dec 23 16:17:18 CET 2011
//   returned the new count for hyperslabDims
// ****************************************************************************

void
avtPixieFileFormat::DetermineVarDimensions(const VarInfo &info,
        hsize_t *hyperslabDims, int *varDims, int &nVarDims) const
{
    //
    // If the mesh is rectilinear, then 1*nx*ny arrays should be treated
    // as 2d, if the mesh is curvilinear (hasCoords), then 2*nx*ny arrays
    // should be treated as 2d.
    //
    int size1D = 1;
    if (info.hasCoords)
    {
        size1D = 2;
    }

    //
    // Determine the hyperslab dimensions.
    //
    if (hyperslabDims != 0)
    {
        hyperslabDims[0] = (int(info.dims[0]) > size1D) ? info.count[0] : 1;
        hyperslabDims[1] = (int(info.dims[1]) > size1D) ? info.count[1] : 1;
        hyperslabDims[2] = (int(info.dims[2]) > size1D) ? info.count[2] : 1;
    }

    //
    // Determine the dimensions for the mesh.
    //
    if(varDims != 0)
    {
        int di = 0;
        varDims[0] = 1;
        varDims[1] = 1;
        varDims[2] = 1;

        if(info.dims[0] > (size_t)size1D)
            varDims[di++] = int(info.dims[0]);
        if(info.dims[1] > (size_t)size1D)
            varDims[di++] = int(info.dims[1]);
        if(info.dims[2] > (size_t)size1D)
            varDims[di++] = int(info.dims[2]);
    }

    //
    // Determine the number of spatial dimensions of the variable.
    //
    nVarDims = 0;
    if(info.dims[0] > (size_t)size1D) ++nVarDims;
    if(info.dims[1] > (size_t)size1D) ++nVarDims;
    if(info.dims[2] > (size_t)size1D) ++nVarDims;
}

// ****************************************************************************
// Method: avtPixieFileFormat::MeshIsCurvilinear
//
// Purpose:
//   Returns whether the named mesh is curvilinear.
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 15 08:45:29 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Sep 15 17:06:28 PST 2004
//   I changed how the support for mesh coordinates is handled.
//
//   Eric Brugger, Tue Oct 26 08:36:53 PDT 2004
//   I modified the routine to handle the fact that variables defined on a
//   curvilinear mesh are now nodal.  I also simplified the logic to compare
//   the raw array sizes instead of the reduced sizes since those should
//   also match.
//
// ****************************************************************************

bool
avtPixieFileFormat::MeshIsCurvilinear(const std::string &name) const
{
    VarInfoMap::const_iterator mesh = meshes.find(name);
    if(mesh == meshes.end() || !mesh->second.hasCoords)
        return false;

    VarInfoMap::const_iterator xvar = variables.find(mesh->second.coordX);
    VarInfoMap::const_iterator yvar = variables.find(mesh->second.coordY);
    VarInfoMap::const_iterator zvar = variables.find(mesh->second.coordZ);
    if(xvar == variables.end() || yvar == variables.end() ||
       zvar == variables.end())
        return false;

    for(int d = 0; d < 3; ++d)
    {
        if(xvar->second.dims[d] != mesh->second.dims[d] ||
           yvar->second.dims[d] != mesh->second.dims[d] ||
           zvar->second.dims[d] != mesh->second.dims[d])
            return false;
    }
    return true;
}

// ****************************************************************************
//  Method: avtPixieFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Aug 13 14:31:43 PST 2004
//
//  Modifications:
//    Brad Whitlock, Mon Aug 16 13:50:41 PST 2004
//    Added support for a curvilinear mesh and an optional point mesh for
//    debugging.
//
//    Eric Brugger, Tue Oct 26 08:36:53 PDT 2004
//    I modified the routine to handle the fact that variables defined on a
//    curvilinear mesh are now nodal.  I also modified the call to Determine
//    VarDimensions since an argument was added to it.
//
//    Mark C. Miller, Mon Apr  4 14:55:14 PDT 2005
//    Added expressions
//
//    Jeremy Meredith, Mon Apr  4 17:05:32 PDT 2005
//    Added "std::" prefix to string constructors.
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added timeState arg to satisfy new interface
//
//    Jean Favre, Fri Dec 23 16:17:18 CET 2011
//    Added SetFormatCanDoDomainDecomposition(true) and mmd->numBlocks = 1
//
//    Mark C. Miller, Thu Aug  6 12:10:00 PDT 2026
//    Mark arrays matching the larger-by-one mesh as node-centered.
//
// ****************************************************************************

void
avtPixieFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md,
        int timeState)
{
//#define ADD_POINT_MESH
    VarInfoMap::const_iterator it;
#ifdef ADD_POINT_MESH
    int pmnDims = -1;
    VarInfo pm;
#endif

    if (! duplicateData)
        md->SetFormatCanDoDomainDecomposition(true);

    PrepareTimestepInfo(timeState);
    if(!metadataIsTimeInvariant)
        md->SetMustRepopulateOnStateChange(true);

    for(it = meshes.begin();
        it != meshes.end(); ++it)
    {
        // Determine the number of spatial dimensions.
        int nSpatialDims = 0;
        DetermineVarDimensions(it->second, 0, 0, nSpatialDims);
        if(nSpatialDims == 0)
            continue;

        // Add the mesh.
        avtMeshMetaData *mmd = new avtMeshMetaData;
        mmd->name = it->first;
        mmd->spatialDimension = nSpatialDims;
        mmd->topologicalDimension = nSpatialDims;
        mmd->numBlocks = 1;  // must be 1 for automatic decomposition
        mmd->blockTitle = "blocks";
        mmd->blockPieceName = "block";
        // Determine the mesh type. Usually it will be rectilinear but
        // sometimes, if we have the right kind of coordinate arrays, it
        // could be curvilinear.
        mmd->meshType = MeshIsCurvilinear(it->first) ? AVT_CURVILINEAR_MESH :
            AVT_RECTILINEAR_MESH;

        mmd->cellOrigin = 1;
        md->Add(mmd);

#ifdef ADD_POINT_MESH
        // If we had a curvilinear mesh, add a point mesh to aid in debugging.
        if(mmd->meshType == AVT_CURVILINEAR_MESH)
        {
            pmnDims = nSpatialDims;
            pm.dims[0] = it->second.dims[0];
            pm.dims[1] = it->second.dims[1];
            pm.dims[2] = it->second.dims[2];
        }
#endif
    }

#ifdef ADD_POINT_MESH
    if(pmnDims != -1)
    {
        meshes["pointmesh"] = pm;

        avtMeshMetaData *mmd = new avtMeshMetaData;
        mmd->name = "pointmesh";
        mmd->spatialDimension = pmnDims;
        mmd->topologicalDimension = 0;
        mmd->meshType = AVT_POINT_MESH;
        mmd->cellOrigin = 1;
        md->Add(mmd);
    }
#endif

    // If we have more than 5 meshes, enable catchall mesh.
    if(meshes.size() > 5)
        md->SetUseCatchAllMesh(true);

    // Iterate through the variables and add them to the metadata. Coordinate
    // arrays support curvilinear mesh construction but are not separate data
    // variables.
    for(it = variables.begin();
        it != variables.end(); ++it)
    {
        if(it->second.isCoord || it->second.meshName.empty())
            continue;

        if(IsNodal(it->second))
            AddScalarVarToMetaData(md, it->first,
                                   it->second.meshName.c_str(), AVT_NODECENT);
        else
            AddScalarVarToMetaData(md, it->first,
                                   it->second.meshName.c_str(), AVT_ZONECENT);
    }

#ifdef ADD_POINT_MESH
    if(pmnDims != -1)
    {
        variables["pointvar"] = pm;
        AddScalarVarToMetaData(md, "pointvar", "pointmesh", AVT_NODECENT);
    }
#endif

    //
    // Add expressions
    //
    if (rawExpressionString.size())
    {
        std::string::size_type s = 0;
        while (s != std::string::npos)
        {
            std::string::size_type nexts = rawExpressionString.find_first_of(";", s);
            std::string exprStr;
            if (nexts != std::string::npos)
            {
                exprStr = std::string(rawExpressionString,s,nexts-s);
                nexts += 1;
            }
            else
            {
                exprStr = std::string(rawExpressionString,s,std::string::npos);
            }

            // remove offending chars from exprStr (spaces)
            std::string newExprStr;
            for (size_t i = 0; i < exprStr.size(); i++)
            {
                if (exprStr[i] != ' ')
                    newExprStr += exprStr[i];
            }

            std::string::size_type t = newExprStr.find_first_of(':');

            Expression vec;
            vec.SetName(std::string(newExprStr,0,t));
            vec.SetDefinition(std::string(newExprStr,t+1,std::string::npos));
            vec.SetType(Expression::VectorMeshVar);
            md->AddExpression(&vec);

            s = nexts;
        }
    }
}

// ****************************************************************************
//  Method: avtPixieFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      timestate   The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Aug 13 14:31:43 PST 2004
//
//  Modifications:
//    Brad Whitlock, Tue Aug 24 12:42:07 PDT 2004
//    Added support for a curvilinear mesh.
//
//    Brad Whitlock, Wed Sep 15 22:09:24 PST 2004
//    I reversed the dimensions for the 3D rectilinear mesh so the variables
//    would display correctly on it.
//
//    Eric Brugger, Tue Oct 26 08:36:53 PDT 2004
//    I modified the routine to handle the fact that variables defined on a
//    curvilinear mesh are now nodal.
//
//    Jean Favre, Fri Dec 23 16:17:18 CET 2011
//    Added stuff to read in parallel
// ****************************************************************************

vtkDataSet *
avtPixieFileFormat::GetMesh(int timestate, const char *meshname)
{
    debug4 << "avtPixieFileFormat::GetMesh: " << meshname << ":"
           << timestate << endl;

    // Check the time state.
    if(nTimeStates > 0 && timestate >= nTimeStates)
    {
        EXCEPTION2(InvalidTimeStepException, 0, nTimeStates);
    }

    PrepareTimestepInfo(timestate);

    // Check the mesh name.
    std::string meshNameString(meshname);
    VarInfoMap::iterator it = meshes.find(meshNameString);
    if(it == meshes.end())
    {
        EXCEPTION1(InvalidVariableException, meshNameString);
    }

    // The dims are being copied to a temporary array before being output
    // to get around a compiler (STL) deficiency with g++-3.3.3 on tru64.
    int originalDims[3];
    originalDims[0] = it->second.dims[0];
    originalDims[1] = it->second.dims[1];
    originalDims[2] = it->second.dims[2];
    debug4 << "avtPixieFileFormat::GetMesh: 0: " << it->second.fileVarName
           << " : originalDims={" << originalDims[0]
           << ", " << originalDims[1]
           << ", " << originalDims[2] << "} "<< endl;

    //
    // Determine the number of cells in each dimension. Note that
    // DetermineVarDimensions may throw out dimensions with 1 or 2
    // depending on wether the variable is nodal or zonal. So
    // 1x33x33 could become 33x33. When we use nVarDims hereafter,
    // it will contain the reduced number of dimensions if any
    // reduction has been done.
    //
    hsize_t hyperslabDims[3];
    int varDims[3];
    int nVarDims;
    DetermineVarDimensions(it->second, hyperslabDims, varDims, nVarDims);
    if(nVarDims < 2)
    {
        EXCEPTION1(InvalidVariableException, meshNameString);
    }

    debug4 << "avtPixieFileFormat::GetMesh: 1: nVarDims=" << nVarDims
           << ", varDims={" << varDims[0] << ", " << varDims[1] << ", "
           << varDims[2] << "} hyperslabDims={" << hyperslabDims[0] << ", " << hyperslabDims[1] << ", "
           << hyperslabDims[2] << "}" << endl;

    // Try to create a point or curvilinear mesh.
    vtkDataSet *retval = 0;
    if(meshNameString == "pointmesh")
        retval = CreatePointMesh(timestate, it->second, hyperslabDims,
                                 varDims, nVarDims);
    else if(MeshIsCurvilinear(meshname))
        retval = CreateCurvilinearMesh(timestate, it->second, hyperslabDims,
                                       varDims, nVarDims);

    // If the mesh isn't a point or curvilinear mesh, then create a
    // rectilinear mesh.
    // here below, used hyperslabDims instead of the original (serial) varDims, JF
    // and used offset of coordinate's value due to hyperslab's new position
    if(retval == 0)
    {
        //
        // Zonal arrays need one more node than cell in each active dimension.
        // Coordinate arrays and data with explicit coordinates are already
        // nodal, so adding one here creates overlapping zones between pieces.
        //
        if(!IsNodal(it->second))
        {
            ++hyperslabDims[0];
            ++hyperslabDims[1];
            if(nVarDims == 3)
                ++hyperslabDims[2];
        }

        // Reverse X,Z dimensions so the mesh is drawn properly.
        if(nVarDims == 3)
        {
            int tmp = hyperslabDims[0];
            hyperslabDims[0] = hyperslabDims[2];
            hyperslabDims[2] = tmp;
        }
        else if (nVarDims == 2)
        {
            int tmp = hyperslabDims[0];
            hyperslabDims[0] = hyperslabDims[1];
            hyperslabDims[1] = tmp;
        }

        vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
        vtkFloatArray *coords[3];
        int I;
        for (int i = 0 ; i < 3 ; i++)
        {
            // Default number of components for an array is 1.
            coords[i] = vtkFloatArray::New();
            if(i==0)
                I = 2;
            else if(i==2)
                I = 0;
            else
                I = 1;
            if (i < nVarDims)
            {
                coords[i]->SetNumberOfTuples(hyperslabDims[i]);
                for (size_t j = 0; j < hyperslabDims[i]; j++)
                    coords[i]->SetComponent(j, 0, it->second.start[I]+j);
            }
            else
            {
                hyperslabDims[i] = 1;
                coords[i]->SetNumberOfTuples(1);
                coords[i]->SetComponent(0, 0, 0.);
            }
        }

        rgrid->SetDimensions(hyperslabDims[0], hyperslabDims[1], hyperslabDims[2]);
        rgrid->SetXCoordinates(coords[0]);
        coords[0]->Delete();
        rgrid->SetYCoordinates(coords[1]);
        coords[1]->Delete();
        rgrid->SetZCoordinates(coords[2]);
        coords[2]->Delete();
        retval = rgrid;
    }
    AddGhostCellInfo(it->second, retval);
    return retval;
}

// ****************************************************************************
// Method: avtPixieFileFormat::CreatePointMesh
//
// Purpose:
//   Creates a point mesh.
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 15 08:43:28 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Sep 15 17:18:31 PST 2004
//   Added support for reading custom coordinate arrays.
//
//   Eric Brugger, Tue Oct 26 08:36:53 PDT 2004
//   I modified the routine to handle the fact that variables defined on a
//   curvilinear mesh are now nodal.
//
// ****************************************************************************

vtkDataSet *
avtPixieFileFormat::CreatePointMesh(int timestate, const VarInfo &info,
    const hsize_t *hyperslabDims, const int *varDims, int nVarDims) const
{
    vtkDataSet *ds = 0;
    float *coords[3] = {0,0,0};

    //
    // Try and read the coordinate fields.
    //
    if(ReadCoordinateFields(timestate, info, coords, hyperslabDims, nVarDims))
    {
        //
        // Populate the coordinates. Put in 3D points with z=0 if the mesh
        // is 2D.
        //
        int i, nPoints = varDims[0] * varDims[1] * ((nVarDims > 2) ? varDims[2] : 1);
        vtkPoints *points  = vtkPoints::New();
        points->SetNumberOfPoints(nPoints);
        float *pts = (float *) points->GetVoidPointer(0);
        for(i = 0; i < 3; ++i)
        {
            float *tmp = pts + i;
            if(nVarDims > 2)
            {
                float *coord = coords[i];
                for(int j = 0; j < nPoints; ++j)
                {
                    *tmp = *coord++;
                    tmp += 3;
                }
            }
            else
            {
                for (int j = 0; j < nPoints; ++j)
                {
                    *tmp = 0.f;
                    tmp += 3;
                }
            }
        }

        //
        // Create the VTK objects and connect them up.
        //
        vtkUnstructuredGrid  *ugrid = vtkUnstructuredGrid::New();
        ugrid->SetPoints(points);
        ugrid->Allocate(nPoints);
        vtkIdType onevertex;
        for(i = 0; i < nPoints; ++i)
        {
            onevertex = i;
            ugrid->InsertNextCell(VTK_VERTEX, 1, &onevertex);
        }

        points->Delete();
        ds = ugrid;
    }

    return ds;
}

// ****************************************************************************
// Method: avtPixieFileFormat::CreateCurvilinearMesh
//
// Purpose:
//   Returns a curvilinear mesh.
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 15 08:38:31 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Sep 15 17:19:05 PST 2004
//   Added support for custom coordinate arrays.
//
//   Eric Brugger, Tue Oct 26 08:36:53 PDT 2004
//   I modified the routine to handle the fact that variables defined on a
//   curvilinear mesh are now nodal.
//
//   Eric Brugger, Wed Dec 22 07:56:05 PST 2004
//   I added back some code that I inadvertently deleted that caused the
//   reading of 3d meshes to fail.
//
// ****************************************************************************

vtkDataSet *
avtPixieFileFormat::CreateCurvilinearMesh(int timestate, const VarInfo &info,
    const hsize_t *hyperslabDims, const int *varDims, int nVarDims)
{
    vtkDataSet *retval = 0;

    //
    // Try and read the coordinate fields.
    //
    float *coords[3] = {0,0,0};
    if(ReadCoordinateFields(timestate, info, coords, hyperslabDims, nVarDims))
    {
        //
        // Create the VTK objects and connect them up.
        //
        vtkStructuredGrid *sgrid  = vtkStructuredGrid::New();
        vtkPoints         *points = vtkPoints::New();
        sgrid->SetPoints(points);
        points->Delete();

        //
        // Tell the grid what its dimensions are and populate the points array.
        //
        if(nVarDims == 2)
        {
            int yxzNodes[] = {static_cast<int>(hyperslabDims[2]),
                              static_cast<int>(hyperslabDims[1]),
                              static_cast<int>(hyperslabDims[0])};
            sgrid->SetDimensions((int *)yxzNodes);
        }
        else
        {
            // In 3D, Pixie dimensions are stored ZYX. Reverse them so we
            // give the right order to VTK.
            int xyzNodes[] = {static_cast<int>(hyperslabDims[2]),
                              static_cast<int>(hyperslabDims[1]),
                              static_cast<int>(hyperslabDims[0])};
            sgrid->SetDimensions(xyzNodes);
        }
        int nMeshNodes = hyperslabDims[0] * hyperslabDims[1] * hyperslabDims[2];

        //
        // Populate the coordinates.  Put in 3D points with z=0 if the mesh is 2D.
        //
        points->SetNumberOfPoints(nMeshNodes);
        float *pts = (float *) points->GetVoidPointer(0);
        int       i, j, k;
        int       nx, ny, nz;
        float    *coord0, *coord1, *coord2;
        float    *tmp = pts;

        switch (nVarDims)
        {
        case 2:
            nx = hyperslabDims[1]; //varDims[0];
            ny = hyperslabDims[2]; //varDims[1];
            coord0 = coords[0];
            coord1 = coords[1];

            for (j = 0; j < ny; j++)
            {
                for (i = 0; i < nx; i++)
                {
                    *tmp++ = *coord0++;
                    *tmp++ = *coord1++;
                    *tmp++ = 0.;
                }
            }
            break;
        case 3:
            // If things are 3D then the varDims array did not get reduced
            // in the DetermineVarDimensions call in GetMesh so the numbers
            // of dimensions will be stored Z,Y,X.
            nx = hyperslabDims[2];
            ny = hyperslabDims[1];
            nz = hyperslabDims[0];
            coord0 = coords[0];
            coord1 = coords[1];
            coord2 = coords[2];

            for (k = 0; k < nz; k++)
            {
                for (j = 0; j < ny; j++)
                {
                    for (i = 0; i < nx; i++)
                    {
                        *tmp++ = *coord0++;
                        *tmp++ = *coord1++;
                        *tmp++ = *coord2++;
                    }
                }
            }
            break;
        }

        retval = sgrid;
    }

    // free up coord data
    if (coords[0] != 0) delete [] coords[0];
    if (coords[1] != 0) delete [] coords[1];
    if (coords[2] != 0) delete [] coords[2];

    return retval;
}

// ****************************************************************************
//  Method: avtPixieFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Aug 13 14:31:43 PST 2004
//
//  Modifications:
//    Eric Brugger, Tue Oct 26 08:36:53 PDT 2004
//    I modified the routine to handle the fact that variables defined on a
//    curvilinear mesh are now nodal.
//
// ****************************************************************************

vtkDataArray *
avtPixieFileFormat::GetVar(int timestate, const char *varname)
{
    const char *mName = "avtPixieFileFormat::GetVar: ";
    debug4 << mName << varname << " ts= "
           << timestate << endl;

    // Check the time state.
    if(nTimeStates > 0 && timestate >= nTimeStates)
    {
        EXCEPTION2(InvalidTimeStepException, 0, nTimeStates);
    }

    PrepareTimestepInfo(timestate);

    // Check the variable name.
    VarInfoMap::iterator it = variables.find(varname);
    if(it == variables.end())
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

#ifdef ADD_POINT_MESH
    if(it->first == "pointvar")
    {
        hsize_t nels = it->second.dims[0] *
                       it->second.dims[1] *
                       it->second.dims[2];
        vtkFloatArray *fscalars = vtkFloatArray::New();
        fscalars->SetNumberOfTuples((vtkIdType)nels);
        float *data = (float *)fscalars->GetVoidPointer(0);
        for(size_t p = 0; p < (size_t)nels; ++p)
            *data++ = float(p);
        return fscalars;
    }
#endif

    //
    // Try and read the data from the file.
    //
    int nVarDims;
    hsize_t hyperslabDims[3];
    DetermineVarDimensions(it->second, hyperslabDims, 0, nVarDims);

    vtkDataArray *scalars = 0;
    hsize_t nels = hyperslabDims[0] * hyperslabDims[1] * hyperslabDims[2];
    if(H5Tequal(it->second.nativeVarType, H5T_NATIVE_INT) > 0 ||
       H5Tequal(it->second.nativeVarType, H5T_NATIVE_UINT) > 0)
    {
        vtkIntArray *iscalars = vtkIntArray::New();
        iscalars->SetNumberOfTuples((vtkIdType)nels);
        scalars = iscalars;
        TRY
        {
            ReadVariableFromFile(timestate, it->first, it->second,
                hyperslabDims, iscalars->GetVoidPointer(0));
        }
        CATCH(VisItException)
        {
            iscalars->Delete();
            RETHROW;
        }
        ENDTRY
    }
    else if(H5Tequal(it->second.nativeVarType, H5T_NATIVE_FLOAT) > 0)
    {
        vtkFloatArray *fscalars = vtkFloatArray::New();
        fscalars->SetNumberOfTuples((vtkIdType)nels);
        scalars = fscalars;
        TRY
        {
            ReadVariableFromFile(timestate, it->first, it->second,
                hyperslabDims, fscalars->GetVoidPointer(0));
        }
        CATCH(VisItException)
        {
            fscalars->Delete();
            RETHROW;
        }
        ENDTRY
    }
    else if(H5Tequal(it->second.nativeVarType, H5T_NATIVE_DOUBLE) > 0)
    {
        vtkDoubleArray *dscalars = vtkDoubleArray::New();
        dscalars->SetNumberOfTuples((vtkIdType)nels);
        scalars = dscalars;
        TRY
        {
            ReadVariableFromFile(timestate, it->first, it->second,
                hyperslabDims, dscalars->GetVoidPointer(0));
        }
        CATCH(VisItException)
        {
            dscalars->Delete();
            RETHROW;
        }
        ENDTRY
    }
    else
    {
        debug4 << mName << "The variable " << varname << " was in a "
               <<"native format that we're not supporting." << endl;
    }

    return scalars;
}

// ****************************************************************************
// Method: avtPixieFileFormat::ReadVariableFromFile
//
// Purpose:
//   Reads a variable from the Pixie file into a buffer.
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 15 08:38:56 PDT 2004
//
// Modifications:
//   Eric Brugger, Tue Oct 26 08:36:53 PDT 2004
//   I modified the routine to read a hyperslab of the array.
//
//   Eric Brugger, Mon Nov 29 15:52:39 PST 2004
//   Modified the reader to handle gaps in the cycle numbering (e.g. allowing
//   0, 10, 20, 30 instead of requiring 0, 1, 2, 3).
//
//    Mark C. Miller, Thu Apr  6 17:06:33 PDT 2006
//    Added conditional compilation for hssize_t type
//
//   Jean Favre, Fri Dec 23 16:17:18 CET 2011
//   Added a simple memspace for partial read, and used it.start to offset the
//   start of the hyperslab read
//
// ****************************************************************************

bool
avtPixieFileFormat::ReadVariableFromFile(int timestate, const std::string &varname,
    const VarInfo &it, const hsize_t *dims, void *dest) const
{
    bool retval = false;
    VarInfo dup = it;
    // Add the time state prefix if necessary.
    std::string fileVar(it.fileVarName);
    if(nTimeStates > 0 && it.timeVarying)
    {
        char tsPrefix[40];
        snprintf(tsPrefix, 40, "%s%d/", timeStatePrefix.c_str(), cycles[timestate]);
        fileVar = std::string(tsPrefix) + fileVar;
    }

    //
    // Try and open the data.
    //
    debug4 << "avtPixieFileFormat::ReadVariableFromFile: Trying to open data: "
           << fileVar.c_str() << endl;
    hid_t dataId = H5Dopen(fileId, fileVar.c_str());
    if(dataId < 0) // try stripping leading slash
        dataId = H5Dopen(fileId, std::string(fileVar,1).c_str());
    if(dataId < 0)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    //
    // Get the data space.
    //
    hid_t spaceId = H5Dget_space(dataId);
    if(spaceId < 0)
    {
        H5Dclose(dataId);
        EXCEPTION1(InvalidVariableException, varname);
    }
#if HDF5_VERSION_GE(1,6,4)
    hsize_t start[3], count[3];
#else
    hssize_t start[3], count[3];
#endif
    start[0] = it.start[0];
    start[1] = it.start[1];
    start[2] = it.start[2];
    count[0] = dims[0];
    count[1] = dims[1];
    count[2] = dims[2];
    debug4 << "RVSTART: " << start[0] << " " <<  start[1] << " " << start[2] << endl;
    debug4 << "RVCOUNT: " << count[0] << " " <<  count[1] << " " << count[2] << endl;
    H5Sselect_hyperslab(spaceId, H5S_SELECT_SET, start, NULL, count, NULL);

    hid_t memspace  = H5Screate_simple(3, count, NULL);

    //
    // Try and read the data from the file.
    //
    if(H5Tequal(it.nativeVarType, H5T_NATIVE_INT) > 0 ||
       H5Tequal(it.nativeVarType, H5T_NATIVE_UINT) > 0 ||
       H5Tequal(it.nativeVarType, H5T_NATIVE_FLOAT) > 0 ||
       H5Tequal(it.nativeVarType, H5T_NATIVE_DOUBLE) > 0)
    {
        // Read the data into all_vars array.
        if(H5Dread(dataId, it.nativeVarType, memspace, spaceId,
                   H5P_DEFAULT, dest) < 0)
        {
            H5Sclose(spaceId);
            H5Dclose(dataId);
            EXCEPTION1(InvalidVariableException, varname);
        }
        retval = true;
    }
    else
    {
        debug4 << "avtPixieFileFormat::ReadVariableFromFile: The variable "
               << varname.c_str() << " was in a native format that we're not "
               "supporting." << endl;
    }

    // Close the data space so we don't leak resources.
    H5Sclose(memspace);
    H5Sclose(spaceId);
    H5Dclose(dataId);
    return retval;
}

// ****************************************************************************
// Method: ConvertToFloat
//
// Purpose:
//   Converts an array to a float array.
//
// Arguments:
//   data      : The data array to be converted.
//   nels      : The number of elements in the data array.
//   allocated : Whether a new data array had to be allocated.
//
// Returns:    Pointer to the converted data.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 15 08:39:27 PDT 2004
//
// Modifications:
//
// ****************************************************************************

template <class T>
float *ConvertToFloat(const T *data, size_t nels, bool &allocated)
{
    float *f;

    if(sizeof(float) == sizeof(T))
    {
        allocated = false;
        // Change to float in the same memory.
        f = (float *)data;
        for(size_t i = 0; i < nels; ++i)
            f[i] = (float)data[i];
    }
    else
    {
        allocated = true;
        f = new float[nels];
        for(size_t i = 0; i < nels; ++i)
            f[i] = (float)data[i];
    }

    return f;
}

// ****************************************************************************
// Method: avtPixieFileFormat::ReadCoordinateFields
//
// Purpose:
//   Reads the coordinate fields from the file.
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 15 08:41:58 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Sep 15 17:20:11 PST 2004
//   Added support for reading custom coordinate arrays.
//
//   Eric Brugger, Tue Oct 26 08:36:53 PDT 2004
//   I modified the routine to read a hyperslab of the array.
//
//   Brad Whitlock, Wed Apr 13 11:27:03 PDT 2005
//   I changed the calls to ConvertToFloat.
//
// ****************************************************************************

bool
avtPixieFileFormat::ReadCoordinateFields(int timestate, const VarInfo &info,
    float *coords[3], const hsize_t *dims, int nDims) const
{
    bool retval = false;

    // Make sure that all of the coordinate field variables are in the
    // variables map.
    VarInfoMap::const_iterator vars[3];
    vars[0] = variables.find(info.coordX);
    vars[1] = variables.find(info.coordY);
    if(nDims > 2)
    {
        vars[2] = variables.find(info.coordZ);
    }
    int i;
    for(i = 0; i < nDims; ++i)
        if(vars[i] == variables.end())
            return false;

    TRY
    {
        //
        // Read in the required variables from the file.
        //
        for(i = 0; i < nDims; ++i)
        {
            bool allocated;
            hsize_t nels = dims[0] * dims[1] * dims[2];
            if(H5Tequal(vars[i]->second.nativeVarType, H5T_NATIVE_INT) > 0 ||
            H5Tequal(vars[i]->second.nativeVarType, H5T_NATIVE_UINT) > 0)
            {
                int *data = new int[(size_t)nels];
                TRY
                {
                    ReadVariableFromFile(timestate, vars[i]->first,
                    vars[i]->second, dims, (void *)data);
                    coords[i] = ConvertToFloat(data, nels, allocated);
                    if(allocated)
                        delete [] data;
                }
                CATCH(VisItException)
                {
                    delete [] data;
                    RETHROW;
                }
                ENDTRY
            }
            else if(H5Tequal(vars[i]->second.nativeVarType, H5T_NATIVE_FLOAT) > 0)
            {
                coords[i] = new float[(size_t)nels];
                ReadVariableFromFile(timestate, vars[i]->first, vars[i]->second,
                                     dims, coords[i]);
            }
            else if(H5Tequal(vars[i]->second.nativeVarType, H5T_NATIVE_DOUBLE) > 0)
            {
                double *data = new double[(size_t)nels];
                TRY
                {
                    ReadVariableFromFile(timestate, vars[i]->first,
                    vars[i]->second, dims, (void *)data);
                    coords[i] = ConvertToFloat(data, nels, allocated);
                    if(allocated)
                        delete [] data;
                }
                CATCH(VisItException)
                {
                    delete [] data;
                    RETHROW;
                }
                ENDTRY
            }
            else
            {
                debug1 << "The " << vars[i]->first.c_str()
                       << " variable was not a type that the Pixie reader "
                       "supports yet." << endl;
                EXCEPTION1(InvalidVariableException, vars[i]->first);
            }
        }

        retval = true;
    }
    CATCH(VisItException)
    {
        // Delete any coordinate arrays that we may have read.
        for(i = 0; i < nDims; ++i)
            delete [] coords[i];
        coords[0] = coords[1] = coords[2] = 0;
    }
    ENDTRY

    return retval;
}

// ****************************************************************************
// Method: avtPixieFileFormat::VisitLinks
//
// Purpose: 
//   This is a callback function to H5Literate that allows us to iterate
//   over all of the links in the group.
//
// Arguments:
//   group : 
//   name   : The name of the current object.
//   linfo  : link info
//   opdata : Pointer to a TraversalInfo object that I pass in that helps
//            us create variable names without using global vars.
//
// Returns:    
//
// Note:       
//
// Programmer: Allen Sanderson
// Creation:   14 June 2012
//
herr_t
avtPixieFileFormat::VisitLinks(hid_t locId, const char* name,
                               const H5L_info_t *linfo, void* opdata) {
  
  switch (linfo->type) {
    case H5L_TYPE_HARD: {

      H5O_info_t objinfo;

      /* Stat the object */
      if(H5Oget_info_by_name(locId, name, &objinfo, H5O_INFO_ALL, H5P_DEFAULT) < 0) {
        debug5 << "visitLinks() - unable to open object with name " <<name <<std::endl;
        debug5 << "visitLinks() - this object and all children will be dropped." <<std::endl;
        return 0;
      }

      switch(objinfo.type)
      {
        case H5O_TYPE_GROUP:
        return GetVariableList( locId, name, opdata );
        break;
        case H5O_TYPE_DATASET:
        return GetVariableList( locId, name, opdata );
        break;

        default:
        debug5 << "visitLinks: node '" << name <<
        "' has an unknown type " << objinfo.type << std::endl;
        break;
      }
    }
    break;
    //end of case H5L_TYPE_HARD
    case H5L_TYPE_EXTERNAL: {

      char *targbuf = (char*) malloc( linfo->u.val_size );

      if (H5Lget_val(locId, name, targbuf, linfo->u.val_size, H5P_DEFAULT) < 0) {
        debug5 << "visitLinks() - unable to open external link with name " <<targbuf <<std::endl;
        debug5 << "visitLinks() - this object and all children will be dropped." <<std::endl;
        return 0;
      }
      
      const char *filename;
      const char *targname;

      if (H5Lunpack_elink_val(targbuf, linfo->u.val_size, 0, &filename, &targname) < 0) {
        debug5 << "visitLinks() - unable to open external file with name " <<filename <<std::endl;
        debug5 << "visitLinks() - this object and all children will be dropped." <<std::endl;
        return 0;
      }
      
      debug5 << "visitLinks(): node '" << name << "' is an external link." << std::endl;
      debug5 << "visitLinks(): node '" << targname << "' is an external target group." << std::endl;

      free(targbuf);
      targbuf = NULL;
      
      // Get info of the linked object.
      H5O_info_t objinfo;
      hid_t obj_id = H5Oopen(locId, name, H5P_DEFAULT);
      
      if (obj_id < 0) {
        debug5 << "visitLinks() - unable to get id for external object " <<name <<std::endl;
        debug5 << "visitLinks() - this object and all children will be dropped." <<std::endl;
        return 0;
      }

      //Test-open the linked object
      if (H5Oget_info (obj_id, &objinfo, H5O_INFO_ALL) < 0) {
        debug5 << "visitLinks() - unable to open external object " <<name <<std::endl;
        debug5 << "visitLinks() - this object and all children will be dropped." <<std::endl;
        return 0;
      }
      
      //Close the linked object to release hdf5 id
      H5Oclose( obj_id );

      //Finally, decide what to do depending on what type of object this is
      switch(objinfo.type)
      {
        case H5O_TYPE_GROUP:
        return GetVariableList( locId, name, opdata );
        break;
        case H5O_TYPE_DATASET:
        return GetVariableList( locId, name, opdata );
        break;

        default:
          debug5 << "visitLinks: node '" << name <<
        "' has an unknown type " << objinfo.type << std::endl;
        break;
      }
    }
    break;
      //END OF CASE H5L_TYPE_EXTERNAL
    
    default:
    debug5 << "visitLinks: node '" << name <<
    "' has an unknown object type " << linfo->type << std::endl;
    break;
  }

  return 0;
}


// ****************************************************************************
// Method: avtPixieFileFormat::GetVariableList
//
// Purpose:
//   This is a callback function to H5Giterate that allows us to iterate
//   over all of the objects in the file and pick out the ones that are
//   directories and variables.
//
// Arguments:
//   group :
//   name    : The name of the current object.
//   op_data : Pointer to a TraversalInfo object that I pass in that helps
//             us create variable names without using global vars.
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 13 18:24:27 PST 2004
//
// Modifications:
//   Brad Whitlock, Wed Sep 15 22:08:04 PST 2004
//   I added code to get the "coords" attribute on data groups that should
//   have a curvilinear mesh.
//
//   Eric Brugger, Mon Nov 29 15:52:39 PST 2004
//   Modified the reader to handle gaps in the cycle numbering (e.g. allowing
//   0, 10, 20, 30 instead of requiring 0, 1, 2, 3).
//
//   Mark C. Miller, Wed May 23 15:27:53 PDT 2007
//   Initialized varInfo.dims before populating with call to get_simple_extents
//
//   Luis Chacon, Wed Feb 25 15:40:06 EST 2009
//   Modified the reader to handle time-changing meshes.
//
//   Jeremy Meredith, Thu Jan  7 15:35:18 EST 2010
//   Skip ".." group names.
//
//   Luis Chacon, Tue Mar 2 10:02:00 EST 2010
//   Added code to read time value attributes
//
//   Eric Brugger, Fri May 22 13:37:44 PDT 2020
//   Corrected a bug reading curvilinear meshes in parallel. I added
//   isCoord to TraversalInfo and VarInfo to track if a variable is a
//   coordinate array so that the decomposition can be done correctly.
//
// ****************************************************************************

herr_t
avtPixieFileFormat::GetVariableList(hid_t group, const char *name,
    void *op_data)
{


CheckHeap("entry GetVariableList");


    // Silo files have a ".." group.  Don't process that....  Ideally we
    // might detect and skip hard links, but this doesn't come up often.
    if (std::string(name)=="..")
        return 0;

    hid_t      obj;
    H5G_stat_t statbuf;

    //
    // Create a variable name that includes the path and the current
    // variable name.
    //
    TraversalInfo *info = (TraversalInfo *)op_data;
    std::string    varName(info->path);
    if(info->path != "/")
        varName += "/";
    varName += name;

    //
    // Get information about the object so we know if it is a dataset,
    // group, type, etc.
    // changed 3rd argument to 1 to allow external links. Jean@cscs
    H5Gget_objinfo(group, name, 1, &statbuf);

    //
    // Do something with the object based on its type.
    //
    switch (statbuf.type)
    {
    case H5G_DATASET:
CheckHeap("handling dataset");
        if ((obj = H5Dopen(group, name)) >= 0)
        {
            VarInfo varInfo;
            varInfo.fileVarName = varName;
            varInfo.timeVarying = false;
            varInfo.hasCoords = info->hasCoords;
            varInfo.isCoord = info->isCoord;
            varInfo.coordX = info->coordX;
            varInfo.coordY = info->coordY;
            varInfo.coordZ = info->coordZ;
            varInfo.meshName = "";

            // Peel off the timestep prefix if there are multiple time states.
            if(info->This->nTimeStates > 0)
            {
                bool matchingTimePrefix = false;
                const std::string tsPrefix1("/Timestep_");
                const std::string tsPrefix2("/Timestep ");
                if(varName.substr(0, tsPrefix1.size()) == tsPrefix1)
                {
                    matchingTimePrefix = true;
                    info->This->timeStatePrefix = tsPrefix1;
                }
                else if(varName.substr(0, tsPrefix2.size()) == tsPrefix2)
                {
                    matchingTimePrefix = true;
                    info->This->timeStatePrefix = tsPrefix2;
                }

                if(matchingTimePrefix)
                {
                    varInfo.timeVarying = true;

                    // Strip off the "/Timestep #" prefix from the argument.
                    std::string::size_type index = varName.find("/", 1);
                    if(index != std::string::npos)
                        varName = varName.substr(index+1);

                    // Strip the timestep off of the file variable because
                    // we'll add that back later.
                    index = varInfo.fileVarName.find("/", 1);
                    if(index != std::string::npos)
                        varInfo.fileVarName = varInfo.fileVarName.substr(index+1);
                }
                else if(varName.size() > 0 && varName[0] == '/')
                {
                    // Trim off the leading slash.
                    varName = varName.substr(1);
                }
            }
            else if(varName.size() > 0 && varName[0] == '/')
                varName = varName.substr(1);

            // See if the variable's name contains any parenthesis. If so,
            // replace with square brackets.
            for(size_t i = 0; i < varName.size(); ++i)
            {
                if(varName[i] == '(')
                    varName[i]=  '[';
                else if(varName[i] == ')')
                    varName[i]=  ']';
            }

            // Get the variable's size.
            hid_t sid = H5Dget_space(obj);

            int rank = H5Sget_simple_extent_ndims(sid);

            debug1 << "Pixie dataset " << varName
                   << " has rank " << rank << endl;

            bool rank_supported = true;
            if (rank > 3)
            {
                debug1 << "Pixie cannot handle rank-" << rank
                       << " dataset " << varName << endl;
                rank_supported = false;
            }

            for (int dd = 0; dd < 3; varInfo.dims[dd] = 1, dd++);
            if (rank_supported)
                H5Sget_simple_extent_dims(sid, varInfo.dims, NULL);

            //
            // Determine the variable's type to see if we can support it.
            //
            hid_t t = H5Dget_type(obj);

            //
            // MCM - Added 16Mar05
            // VisIt can't deal well with a large variety of different data
            // types. So, we force everything to float with this line of
            // code. This tells the plugin that everything is float,
            // regardless of its real type on disk. Note that if we
            // every implement GetAuxiliaryData functions for global node/zone
            // ids, we'll have to be a little smarter.
            //
#ifdef FORCE_FLOATS
            varInfo.nativeVarType = H5T_NATIVE_FLOAT;
#else
            varInfo.nativeVarType = H5Tget_native_type(t, H5T_DIR_ASCEND);
#endif

            bool supported = false;
            if(H5Tequal(varInfo.nativeVarType, H5T_NATIVE_INT) > 0 ||
               H5Tequal(varInfo.nativeVarType, H5T_NATIVE_UINT) > 0)
            {
                supported = true;
            }
            else if(H5Tequal(varInfo.nativeVarType, H5T_NATIVE_FLOAT) > 0)
                supported = true;
            else if(H5Tequal(varInfo.nativeVarType, H5T_NATIVE_DOUBLE) > 0)
                supported = true;
            else
            {
                debug4 << "Variable " << varName.c_str()
                       << "'s type is: ";
                if(H5Tequal(varInfo.nativeVarType, H5T_NATIVE_CHAR) > 0)
                    debug4 << "CHAR";
                else if(H5Tequal(varInfo.nativeVarType, H5T_NATIVE_SHORT) > 0)
                    debug4 << "SHORT";
                else if(H5Tequal(varInfo.nativeVarType, H5T_NATIVE_INT) > 0)
                    debug4 << "INT";
                else if(H5Tequal(varInfo.nativeVarType, H5T_NATIVE_LONG) > 0)
                    debug4 << "LONG";
                else if(H5Tequal(varInfo.nativeVarType, H5T_NATIVE_LLONG) > 0)
                    debug4 << "LLONG";
                else if(H5Tequal(varInfo.nativeVarType, H5T_NATIVE_UCHAR) > 0)
                    debug4 << "UCHAR";
                else if(H5Tequal(varInfo.nativeVarType, H5T_NATIVE_USHORT) > 0)
                    debug4 << "USHORT";
                else if(H5Tequal(varInfo.nativeVarType, H5T_NATIVE_UINT) > 0)
                    debug4 << "UINT";
                else if(H5Tequal(varInfo.nativeVarType, H5T_NATIVE_ULONG) > 0)
                    debug4 << "ULONG";
                else if(H5Tequal(varInfo.nativeVarType, H5T_NATIVE_ULLONG) > 0)
                    debug4 << "ULLONG";
                else if(H5Tequal(varInfo.nativeVarType, H5T_NATIVE_LDOUBLE) > 0)
                    debug4 << "LDOUBLE";
                else
                    debug4 << "???";
                debug4 << ", which is not supported at this time." << endl;
            }

            // Store information about every timestep incarnation. The old
            // variables map retained only the first occurrence, which made
            // dimension changes in later timesteps invisible.
            if(rank_supported && supported)
            {
                if(info->cycle >= 0 && varInfo.timeVarying)
                    info->This->stateVariables[info->cycle][varName] = varInfo;
                else
                    info->This->staticVariables[varName] = varInfo;

                if(info->This->variables.find(varName) ==
                   info->This->variables.end())
                    info->This->variables[varName] = varInfo;

                debug4 << "Adding variable \"" << varName.c_str()
                       << "\" for file variable: \""
                       << varInfo.fileVarName.c_str() << "\"" << endl;
            }

            H5Sclose(sid);
            H5Dclose(obj);
        }
        else
        {
            debug4 << "unable to get dataset " << name << endl;
        }
        break;
    case H5G_GROUP:
CheckHeap("handling group");
        // We found a time state, increment the number of time states.
        if(info->level == 0 && varName.find("Timestep") != std::string::npos)
        {
            debug4 << "Added time state" << endl;
            ++info->This->nTimeStates;

            int cycle;
            if (varName[9] == '_')
                cycle = atoi(varName.substr(10).c_str());
            else
                cycle = atoi(varName.substr(9).c_str());
            info->This->cycles.push_back(cycle);
        }

        if ((obj = H5Gopen(group, name)) >= 0)
        {
            TraversalInfo info2;
            info2.This = info->This;
            info2.level = info->level + 1;
            info2.path = varName;
            info2.hasCoords = false;
            info2.isCoord = false;
            info2.coordX = "";
            info2.coordY = "";
            info2.coordZ = "";
            info2.cycle = info->cycle;

            if(info->level == 0 &&
               varName.find("Timestep") != std::string::npos)
            {
                if(varName[9] == '_')
                    info2.cycle = atoi(varName.substr(10).c_str());
                else
                    info2.cycle = atoi(varName.substr(9).c_str());
            }

// ************************* Begin Pixie-specific coding **********************
            //
            // See if the group has the "coords" attribute.
            //
            hid_t coordsAttribute = H5Aopen_name(obj, "coords");
            if(coordsAttribute >= 0)
            {
                hid_t attrType = H5Aget_type(coordsAttribute);
                if(attrType >= 0)
                {
                    char data[1000];
                    memset((void*)data, 0, sizeof(data));
                    if(H5Aread(coordsAttribute, attrType, (void *)data) >= 0)
                    {
                        for(int j = 0; j < 3; ++j)
                        {
                            size_t dsize = H5Tget_size(attrType);
                            char *ptr = data + dsize * j + 1;
                            char *tmp = new char[dsize+1];
                            size_t i;
                            for(i = 0; i < dsize && *ptr != ' '; ++i)
                                tmp[i] = *ptr++;
                            tmp[i] = '\0';

                            if(j == 0)
                                info2.coordX = std::string(tmp);
                            else if(j == 1)
                                info2.coordY = std::string(tmp);
                            else
                                info2.coordZ = std::string(tmp);
                            delete [] tmp;
                        }
                        info2.hasCoords = true;

                        debug4 << "Have mesh coordinates: "
                               << info2.coordX  << endl;
                        debug4 << "Have mesh coordinates: "
                               << info2.coordY  << endl;
                        debug4 << "Have mesh coordinates: "
                               << info2.coordZ  << endl;
                    }
                    else
                    {
                        debug4 << "No mesh coordinates found." << endl;
                    }
                    H5Tclose(attrType);
                }
                H5Aclose(coordsAttribute);
            }

            //
            // Read time level if available
            //
            hid_t timeAttribute = H5Aopen_name(obj, "Time");
            if(timeAttribute >= 0)
            {
                debug4 << "Found time attribute" << endl;

                hid_t attrType = H5Aget_type(timeAttribute);
                if(attrType >= 0)
                {
                    double time;
                    if(H5Aread(timeAttribute, H5T_NATIVE_DOUBLE, &time) >= 0)
                    {
                        info->This->time_val.push_back(time);
                        debug4 << "time value found="<< time << endl;
                    }
                    else
                    {
                        debug4 << "No time value found." << endl;
                    }
                    H5Tclose(attrType);
                }
                else
                {
                    debug4 << "Problems opening time attribute." << endl;
                }
                H5Aclose(timeAttribute);
            }

// ************************** End Pixie-specific coding ***********************
            
            // ARS - Note as of 1.8.0 H5Giterate has been deprecated
            // and H5Literate should be used. At the same time
            // H5Literate will traverse not only groups but all links
            // inlcuding external links. As such, code is in place to
            // do this.

            // Iterate over the items in this group.
            H5Giterate(obj, ".", NULL, GetVariableList, (void*)&info2);
//          H5Literate(obj, H5_INDEX_NAME, H5_ITER_INC, 0, VisitLinks, (void*)&info);
            H5Gclose(obj);
        }
        else
        {
            debug4 << "unable to dump group " << varName.c_str() << endl;
        }
        break;
#if 0
    case H5G_TYPE:
CheckHeap("handling type");
        if ((obj = H5Topen(group, name)) >= 0)
        {
            debug4 << "TYPE: " << varName.c_str() << endl;
            H5Tclose(obj);
        }
        else
        {
            debug4 << "unable to get dataset " << varName.c_str() << endl;
        }
        break;
#endif
    default:
        break;
    }

    return 0;
}

// ****************************************************************************
// Method: avtPixieFileFormat::AddGhostCellInfo
//
// Purpose: 
//   Add ghost cell arrays to the dataset.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Jean Favre
// Creation:   Thu Jun 21 14:53:29 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

void
avtPixieFileFormat::AddGhostCellInfo(const VarInfo &info, vtkDataSet *ds)
{
    if (resultMustBeProducedOnlyOnThisProcessor || duplicateData)
    {
      return;
    }
    else
    {
#define GHOST
#ifdef GHOST
      int nx, ny, nz, i, x, y, z, id, ncells;
      unsigned char realVal = 0, ghostVal=0;
      avtGhostData::AddGhostZoneType(ghostVal, DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
      vtkUnsignedCharArray *ghostCells = vtkUnsignedCharArray::New();
      ghostCells->SetName("avtGhostZones");
      if (IsNodal(info))
      {
        nx = info.count[2]-1;
        ny = info.count[1]-1;
        nz = info.count[0]-1;
      }
      else
        // the default is to always have zone-centered data and
        // grid dims were incremented by 1 in GetMesh
        // this fixes the size of the ghostCells array.
      {
        nx = info.count[2];
        ny = info.count[1];
        nz = info.count[0];
      }
      // here we swapped indices again, 2 <-> 0
      
      debug4 << "allocate GhostZone array of size " << nx << "x" << ny << "x" << nz << endl;

      ncells = nx * ny * nz;
      ghostCells->SetNumberOfTuples(ncells);
      unsigned char *gnp = ghostCells->GetPointer(0);
      for (i=0; i<ncells; i++)
      {
        gnp[i] = realVal;
      }
      if (info.start[2] < info.start_no_ghost[2])
      {
        debug4 << "Xmin: " << info.start[2]<< " < " << info.start_no_ghost[2] << endl;
        x = 0;
        for (y=0; y<ny; y++)
            for (z=0; z<nz; z++)
            {
                id = z*nx*ny + y*nx + x;
                gnp[id] = ghostVal;
            }
      }
      if (info.start[2]+info.count[2] > info.start_no_ghost[2]+info.count_no_ghost[2])
      {
        debug4 << "Xmax: " << info.start[2]+info.count[2] << " > " << info.start_no_ghost[2]+info.count_no_ghost[2] << endl;
        x = nx-1;
        for ( y=0; y<ny; y++)
            for ( z=0; z<nz; z++)
            {
                id = z*nx*ny + y*nx + x;
                gnp[id] = ghostVal;
            }
      }
      // Check planes cutting y
      if (info.start[1] < info.start_no_ghost[1])
      {
debug4 << "Ymin: " << info.start[1] << " < " << info.start_no_ghost[1] << endl;
        y = 0;
        for (x=0; x<nx; x++)
            for ( z=0; z<nz; z++)
            {
                id = z*nx*ny + y*nx + x;
                gnp[id] = ghostVal;
            }
      }
      if (info.start[1]+info.count[1] > info.start_no_ghost[1]+info.count_no_ghost[1])
      {
        debug4 << "Ymax: " << info.start[1]+info.count[1] << " > " << info.start_no_ghost[1]+info.count_no_ghost[1] << endl;
        y = ny-1;
        for (x=0; x<nx; x++)
            for (z=0; z<nz; z++)
            {
                id = z*nx*ny + y*nx + x;
                gnp[id] = ghostVal;
            }
      }
      // Check planes cutting z
      if (info.start[0] < info.start_no_ghost[0])
      {
        debug4 << "Zmin: " << info.start[0] << " < " << info.start_no_ghost[0] << endl;
        z = 0;
        for (x=0; x<nx; x++)
            for (y=0; y<ny; y++)
            {
                id = z*nx*ny + y*nx + x;
                gnp[id] = ghostVal;
            }
      }
      if (info.start[0]+info.count[0] > info.start_no_ghost[0]+info.count_no_ghost[0])
      {
        debug4 << "Zmax: " << info.start[0]+info.count[0] << " > " << info.start_no_ghost[0]+info.count_no_ghost[0] << endl;
        z = nz-1;
        for (x=0; x<nx; x++)
            for (y=0; y<ny; y++)
            {
                id = z*nx*ny + y*nx + x;
                gnp[id] = ghostVal;
            }
      }
//
      ds->GetCellData()->AddArray(ghostCells);
      ghostCells->Delete();
      
      ds->GetInformation()->Set(
                                vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(), 0);
#endif
      // Add the min/max local logical extents of this domain.  It's
      // an alternate way we label ghost zones for structured grids.
      vtkIntArray *realDims = vtkIntArray::New();
      realDims->SetName("avtRealDims");
      realDims->SetNumberOfValues(6);
      bool nodal = IsNodal(info);
      int size1D = info.hasCoords ? 2 : 1;
      bool active[3] = {info.dims[0] > (size_t)size1D,
                        info.dims[1] > (size_t)size1D,
                        info.dims[2] > (size_t)size1D};
      
      realDims->SetValue(0, info.start_no_ghost[2]-info.start[2]);
      realDims->SetValue(1, active[2] ?
                         info.start_no_ghost[2]-info.start[2] +
                         info.count_no_ghost[2] - (nodal ? 1 : 0) : 0);

      realDims->SetValue(2, info.start_no_ghost[1]-info.start[1]);
      realDims->SetValue(3, active[1] ?
                         info.start_no_ghost[1]-info.start[1] +
                         info.count_no_ghost[1] - (nodal ? 1 : 0) : 0);
      
      realDims->SetValue(4, info.start_no_ghost[0]-info.start[0]);
      realDims->SetValue(5, active[0] ?
                         info.start_no_ghost[0]-info.start[0] +
                         info.count_no_ghost[0] - (nodal ? 1 : 0) : 0);
      
      debug5 << "adding avtRealDims (" <<
        realDims->GetValue(0) << ", " <<
        realDims->GetValue(1) << ", " <<
        realDims->GetValue(2) << ", " <<
        realDims->GetValue(3) << ", " <<
        realDims->GetValue(4) << ", " <<
        realDims->GetValue(5) << ")\n";
      
      ds->GetFieldData()->AddArray(realDims);
      ds->GetFieldData()->CopyFieldOn("avtRealDims");
      realDims->Delete();
      
      vtkIntArray *arr = vtkIntArray::New();
      arr->SetNumberOfTuples(3);
      
      arr->SetValue(0, info.start[2]);
      arr->SetValue(1, info.start[1]);
      arr->SetValue(2, info.start[0]);
      
      debug1 << "adding base_index " <<
        arr->GetValue(0) << " " <<
        arr->GetValue(1) << " " <<
           arr->GetValue(2) << endl;

      arr->SetName("base_index");
      ds->GetFieldData()->AddArray(arr);
      arr->Delete();
    }
}
