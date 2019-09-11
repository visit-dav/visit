// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers. See the top-level LICENSE file for dates and other
// details. No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtXolotlFileFormat.C                           //
// ************************************************************************* //

#include <avtXolotlFileFormat.h>

#include <string>
#include <iostream>
#include <sstream>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPointData.h>

#include <avtDatabaseMetaData.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>

#include <InvalidVariableException.h>
#include <InvalidDBTypeException.h>
#include <InvalidFilesException.h>
#include <InvalidTimeStepException.h>

#include <hdf5.h>
#include <visit-hdf5.h>

using namespace std;
static const int debug = 0;

//
// struct for reading the HDF compound type 'concentration'
//
typedef struct {
    int     clusterNumber;
    float  concentration;
} concentrationTypeStruct;


// ****************************************************************************
//  Method: avtXolotlFileFormat constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   March 22, 2016
//
// ****************************************************************************
avtXolotlFileFormat::avtXolotlFileFormat(const char *fn, DBOptionsAttributes *readOpts)
    : avtMTSDFileFormat(&fn, 1)
{
    fileId = -1;
    filename = fn;
    nTimeStates = 0;
    nx = ny = nz = hz = hy = hz = 0;

    //
    // Initialize the file if it has not been initialized.
    //
    Initialize();

    H5Eset_auto(H5E_DEFAULT,0,0);
}


// ****************************************************************************
//  Method: avtXolotlFileFormat destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   March 22, 2016
//
// ****************************************************************************
avtXolotlFileFormat::~avtXolotlFileFormat()
{
    FreeUpResources();
}


// ****************************************************************************
//  Method: avtXolotlFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: James Kress
//  Creation:   July 15, 2019
//
// ****************************************************************************
int
avtXolotlFileFormat::GetNTimesteps(void)
{
    return nTimeStates;
}


// ****************************************************************************
//  Method: avtXolotlFileFormat::GetCycles
//
//  Purpose:
//      Tells the rest of the code the actual cycle number in the hdf file
//      so that it can be retrieved and read.
//
//  Programmer: James Kress
//  Creation:   July 15, 2019
//
// ****************************************************************************
void
avtXolotlFileFormat::GetCycles(std::vector<int> &in)
{
    in = cycleNumbers;
}


// ****************************************************************************
//  Method: avtXolotlFileFormat::GetTimes
//
//  Purpose:
//      Tells the rest of the code the actual simulation cycle time at each
//      cycle.
//
//  Programmer: James Kress
//  Creation:   July 15, 2019
//
// ****************************************************************************
void
avtXolotlFileFormat::GetTimes(std::vector<double> &in)
{
    in = times;
}


// ****************************************************************************
//  Method: avtXolotlFileFormat::GroupInfo
//
//  Purpose:
//      Operator function to iterate over groups in an hdf5 file.
//
//  Programmer: James Kress
//  Creation:   July 15, 2019
//
// ****************************************************************************
int
avtXolotlFileFormat::GroupInfo(hid_t loc_id, const char *name, const H5L_info_t *linfo, void *opdata)
{
    auto cycleNum = reinterpret_cast<std::vector<int>*>(opdata);

    std::stringstream ss(name);
    std::string item;
    while(std::getline(ss, item, '_'))
    {
        if (!item.empty() && std::all_of(item.begin(), item.end(), ::isdigit))
        {
            cycleNum->push_back(stoi(item));
        }
    }
    return 0;
}


// ****************************************************************************
//  Method: avtXolotlFileFormat::PopulateConcentrationGroupMetadata
//
//  Purpose:
//      Gets the cycle and time meta data from the hdf file
//
//  Programmer: James Kress
//  Creation:   July 15, 2019
//
// ****************************************************************************
void
avtXolotlFileFormat::PopulateConcentrationGroupMetaData()
{
    //
    // Get the true cycle numbers from each saved time step in the file
    //
    cycleNumbers.clear();
    H5Literate(concentrationsGroup,
               H5_INDEX_NAME,
               H5_ITER_INC,
               NULL,
               GroupInfo,
               &cycleNumbers);
    nTimeStates = cycleNumbers.size();
    if (debug) cerr << "Xolotl:: nTimeStates = " << nTimeStates << endl;
    if (debug)
    {
	      for (auto it = cycleNumbers.cbegin(); it != cycleNumbers.cend(); it++)
	      {
		      cerr << *it << ' ';
	      }
	      cerr << endl;
    }

    //
    // Get the 'absoulteTime' and 'iSurface' value for each cycle if they exist
    //
    times.clear();
    isurface.clear();
    for (int i = 0; i < cycleNumbers.size(); i++)
    {
        int realTime = cycleNumbers[i];
        // Open the concentration group for this particular timestate
        char varname[100];
        snprintf(varname, 100, "concentration_%d", realTime);
        hid_t currentConcentration = H5Gopen(concentrationsGroup, varname, H5P_DEFAULT);
        if (currentConcentration < 0)
        {
            FreeUpResources();
            snprintf(varname, 100, "No 'concentration_%d' group found", realTime);
            EXCEPTION1(InvalidDBTypeException, varname);
        }

        // Open and read the absolute time
        hid_t timeAttr = H5Aopen(currentConcentration,
                       "absoluteTime", H5P_DEFAULT);
        if (timeAttr < 0)
        {
            H5Gclose(currentConcentration);
            FreeUpResources();
            EXCEPTION1(InvalidDBTypeException, "No 'absoluteTime' attribute.");
        }

        double currentTime;
        int err2 = H5Aread(timeAttr, H5T_NATIVE_DOUBLE, &currentTime);
        if (err2 < 0)
        {
            H5Gclose(currentConcentration);
            FreeUpResources();
            EXCEPTION1(InvalidDBTypeException, "cannot read 'absoluteTime' var.");
        }

        times.push_back(currentTime);

        // Open and read the iSurface number if not a 0D file
        if (dimension > 0)
        {
            hid_t isurfaceAttr = H5Aopen(currentConcentration,
                       "iSurface", H5P_DEFAULT);
            int surface;
            int err3 = H5Aread(isurfaceAttr, H5T_NATIVE_INT, &surface);
            if (err3 < 0)
            {
                H5Gclose(currentConcentration);
                FreeUpResources();
                EXCEPTION1(InvalidDBTypeException, "cannot read 'iSurface' var.");
            }

            isurface.push_back(surface);
            H5Aclose(isurfaceAttr);
        }

        H5Aclose(timeAttr);
        H5Gclose(currentConcentration);
    }
}


// ****************************************************************************
//  Method: avtXolotlFileFormat::PopulateHeaderGroupMetadata
//
//  Purpose:
//      Gets the meta data and range info from the headerGroup
//
//  Programmer: James Kress
//  Creation:   July 15, 2019
//
//  Modifications:
//   Kathleen Biagas, Tue Sep 10 17:30:33 PDT 2019
//   Create 'data' array on the heap. Visual Studio won't compile stack created
//   arrays unless their size can be determined at compile time.
//
// ****************************************************************************
void
avtXolotlFileFormat::PopulateHeaderGroupMetaData()
{
    hid_t headerGroup = H5Gopen(fileId, "headerGroup", H5P_DEFAULT);
    if (headerGroup < 0)
    {
        FreeUpResources();
        EXCEPTION1(InvalidDBTypeException, "No 'headerGroup'.");
    }

    hid_t hxAttr = H5Aopen(headerGroup, "hx", H5P_DEFAULT);
    if (hxAttr < 0)
    {
        H5Gclose(headerGroup);
        FreeUpResources();
        EXCEPTION1(InvalidDBTypeException, "No 'hx' attribute.");
    }

    hid_t hyAttr = H5Aopen(headerGroup, "hy", H5P_DEFAULT);
    if (hyAttr < 0)
    {
        H5Gclose(headerGroup);
        FreeUpResources();
        EXCEPTION1(InvalidDBTypeException, "No 'hy' attribute.");
    }

    hid_t hzAttr = H5Aopen(headerGroup, "hz", H5P_DEFAULT);
    if (hzAttr < 0)
    {
        H5Gclose(headerGroup);
        FreeUpResources();
        EXCEPTION1(InvalidDBTypeException, "No 'hz' attribute.");
    }

    hid_t nxAttr = H5Aopen(headerGroup, "nx", H5P_DEFAULT);
    if (nxAttr < 0)
    {
        H5Gclose(headerGroup);
        FreeUpResources();
        EXCEPTION1(InvalidDBTypeException, "No 'nx' attribute.");
    }

    hid_t nyAttr = H5Aopen(headerGroup, "ny", H5P_DEFAULT);
    if (nyAttr < 0)
    {
        H5Gclose(headerGroup);
        FreeUpResources();
        EXCEPTION1(InvalidDBTypeException, "No 'ny' attribute.");
    }

    hid_t nzAttr = H5Aopen(headerGroup, "nz", H5P_DEFAULT);
    if (nzAttr < 0)
    {
        H5Gclose(headerGroup);
        FreeUpResources();
        EXCEPTION1(InvalidDBTypeException, "No 'nz' attribute.");
    }

    // read the variables
    int err2 = -1;
    err2 = H5Aread(hxAttr, H5T_NATIVE_DOUBLE, &hx);
    if (err2 < 0)
    {
        H5Gclose(headerGroup);
        FreeUpResources();
        EXCEPTION1(InvalidDBTypeException, "cannot read 'hx' var.");
    }

    err2 = H5Aread(hyAttr, H5T_NATIVE_DOUBLE, &hy);
    if (err2 < 0)
    {
        H5Gclose(headerGroup);
        FreeUpResources();
        EXCEPTION1(InvalidDBTypeException, "cannot read 'hy' var.");
    }

    err2 = H5Aread(hzAttr, H5T_NATIVE_DOUBLE, &hz);
    if (err2 < 0)
    {
        H5Gclose(headerGroup);
        FreeUpResources();
        EXCEPTION1(InvalidDBTypeException, "cannot read 'hz' var.");
    }

    err2 = H5Aread(nxAttr, H5T_NATIVE_INT, &nx);
    if (err2 < 0)
    {
        H5Gclose(headerGroup);
        FreeUpResources();
        EXCEPTION1(InvalidDBTypeException, "cannot read 'nx' var.");
    }

    err2 = H5Aread(nyAttr, H5T_NATIVE_INT, &ny);
    if (err2 < 0)
    {
        H5Gclose(headerGroup);
        FreeUpResources();
        EXCEPTION1(InvalidDBTypeException, "cannot read 'ny' var.");
    }

    err2 = H5Aread(nzAttr, H5T_NATIVE_INT, &nz);
    if (err2 < 0)
    {
        H5Gclose(headerGroup);
        FreeUpResources();
        EXCEPTION1(InvalidDBTypeException, "cannot read 'nz' var.");
    }

    // Figure out file dimensionality
    if (nx > 1)
        dimension = 1;
    else
        dimension = 0;

    if (debug) cerr << "hx="<<hx<<endl;
    if (debug) cerr << "hy="<<hy<<endl;
    if (debug) cerr << "hz="<<hz<<endl;
    if (debug) cerr << "nx="<<nx<<endl;
    if (debug) cerr << "ny="<<ny<<endl;
    if (debug) cerr << "nz="<<nz<<endl;
    if (debug) cerr << "dimension="<<dimension<<endl;


    // close the attributes and group
    H5Aclose(hxAttr);
    H5Aclose(hyAttr);
    H5Aclose(hzAttr);
    H5Aclose(nxAttr);
    H5Aclose(nyAttr);
    H5Aclose(nzAttr);
    H5Gclose(headerGroup);

    //
    // Now we need to get the data ranges from the composition table
    //
    hid_t compositionDataSet = H5Dopen(fileId, "headerGroup/composition", H5P_DEFAULT);
    if (compositionDataSet < 0)
    {
        FreeUpResources();
        EXCEPTION1(InvalidDBTypeException, "No 'headerGroup/composition'.");
    }

    hid_t sid = H5Dget_space(compositionDataSet);
    hsize_t composition_dims[2];
    hid_t ndims = H5Sget_simple_extent_dims(sid, composition_dims, NULL);
    if (debug) cout << "dimensions " <<
      (unsigned long)(composition_dims[0]) << " x " <<
      (unsigned long)(composition_dims[1]) <<
      " ndims " << ndims << endl;

    int *data = new int[composition_dims[0]*composition_dims[1]];
    H5Dread(compositionDataSet, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, (void*)data);

    int maximumHeliumDimension =       data[0];
    int maximumDeuteriumDimension =    data[1];
    int maximumTritiumDimension =      data[2];
    int maximumVacancyDimension =      data[3];
    int maximumInterstitialDimension = data[4];
    for (int i = 1; i < composition_dims[0]; i++)
    {
        int baseIndex = i * composition_dims[1];
        if (data[baseIndex] > maximumHeliumDimension)
            maximumHeliumDimension = data[baseIndex];
        if (data[baseIndex + 1] > maximumDeuteriumDimension)
            maximumDeuteriumDimension = data[baseIndex + 1];
        if (data[baseIndex + 2] > maximumTritiumDimension)
            maximumTritiumDimension = data[baseIndex + 2];
        if (data[baseIndex + 3] > maximumVacancyDimension)
            maximumVacancyDimension = data[baseIndex + 3];
        if (data[baseIndex + 4] > maximumInterstitialDimension)
            maximumInterstitialDimension = data[baseIndex + 4];
    }
    delete [] data;
    varMaxes[0] = maximumHeliumDimension;
    varMaxes[1] = maximumDeuteriumDimension;
    varMaxes[2] = maximumTritiumDimension;
    varMaxes[3] = maximumVacancyDimension;
    varMaxes[4] = maximumInterstitialDimension;

    // close HDF resources
    H5Sclose(sid);
    H5Dclose(compositionDataSet);

    if (dimension == 1)
    {
        //
        // Now we need to get grid since this is a 1D file
        //
        hid_t gridDataSet = H5Dopen(fileId, "headerGroup/grid", H5P_DEFAULT);
        if (gridDataSet < 0)
        {
            FreeUpResources();
            EXCEPTION1(InvalidDBTypeException, "No 'headerGroup/grid'.");
        }

        hid_t sid = H5Dget_space(gridDataSet);
        hsize_t headerGrid_dims[1];
        hid_t ndims = H5Sget_simple_extent_dims(sid, headerGrid_dims, NULL);
        if (debug) cout << "dimensions " <<
          (unsigned long)(headerGrid_dims[0]) <<
          " ndims " << ndims << endl;

        double *data  = new double[headerGrid_dims[0]];
        int err2 = -1;
        err2 = H5Dread(gridDataSet, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, (void*)data);
        if (err2 < 0)
        {
            H5Gclose(headerGroup);
            FreeUpResources();
            EXCEPTION1(InvalidDBTypeException, "cannot read 'grid' var.");
        }

        // Save the grid for later use
        for (int i = 0; i < headerGrid_dims[0]; i++)
        {
            oneDGrid.push_back(data[i]);
        }
        delete [] data;
        H5Sclose(sid);
        H5Dclose(gridDataSet);
    }
}


// ****************************************************************************
//  Method: avtXolotlFileFormat::Initialize
//
//  Purpose:
//      Opens the Xolotl HDF file and checks if it is valid, then populates
//      metadata arrays.
//
//  Programmer: James Kress
//  Creation:   July 15, 2019
//
// ****************************************************************************
void
avtXolotlFileFormat::Initialize()
{
    if (fileId != -1)
        return;

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
        snprintf(error, 1024, "Cannot be a Xolotl file (%s)",filenames[0]);

        EXCEPTION1(InvalidDBTypeException, error);
    }
    H5Pclose(fileAccessPropListID);

    //
    // Open the header group and read mesh attributes
    //
    PopulateHeaderGroupMetaData();

    //
    // Open the concentrations group and read the number of time steps
    //
    concentrationsGroup = H5Gopen(fileId, "concentrationsGroup", H5P_DEFAULT);
    if (concentrationsGroup < 0)
    {
        FreeUpResources();
        EXCEPTION1(InvalidDBTypeException, "No 'concentrationsGroup'.");
    }
    PopulateConcentrationGroupMetaData();


    if (dimension > 1)
    {
        //
        // Read the network
        //
        hid_t networkGroup = H5Gopen(fileId, "networkGroup", H5P_DEFAULT);
        if (networkGroup<0)
        {
            FreeUpResources();
            EXCEPTION1(InvalidDBTypeException, "No 'networkGroup'.");
        }

        hid_t networkDS = H5Dopen(networkGroup, "network", H5P_DEFAULT);
        if (networkDS<0)
        {
            H5Gclose(networkGroup);
            FreeUpResources();
            EXCEPTION1(InvalidDBTypeException, "No 'network' dataset.");
        }

        hid_t networkSpace = H5Dget_space(networkDS);
        hsize_t dims[2], maxdims[2];
        int ndims = H5Sget_simple_extent_dims(networkSpace, dims, maxdims);
        if (debug) cerr << "NDIMS="<<ndims<<" dims="<<dims[0]<<","<<dims[1]<<endl;

        networkSize = dims[0];
        networkParams = dims[1];
        network.resize(networkSize * networkParams);
        int err1 = H5Dread(networkDS, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
                          H5P_DEFAULT, (void*)(&(network[0])));
        ///\todo: check for err1<0
        if (debug) cerr << "network[0]="<<network[0]<<endl;
        if (debug) cerr << "network[1]="<<network[1]<<endl;
        if (debug) cerr << "network[2]="<<network[2]<<endl;
        if (debug) cerr << "network[3]="<<network[3]<<endl;
        if (debug) cerr << "network[4]="<<network[4]<<endl;
        if (debug) cerr << "network[5]="<<network[5]<<endl;
        if (debug) cerr << "network[6]="<<network[6]<<endl;

        H5Sclose(networkSpace);
        H5Dclose(networkDS);
        H5Gclose(networkGroup);
    }
}


// ****************************************************************************
//  Method: avtXolotlFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: js9 -- generated by xml2avt
//  Creation:   Tue Mar 22 15:09:52 PST 2016
//
// ****************************************************************************
void
avtXolotlFileFormat::FreeUpResources(void)
{
    if (concentrationsGroup >=0)
        H5Gclose(concentrationsGroup);

    if (fileId >= 0)
    {
        H5Fclose(fileId);
        fileId = -1;
    }
}


// ****************************************************************************
//  Method: avtXolotlFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: js9 -- generated by xml2avt
//  Creation:   Tue Mar 22 15:09:52 PST 2016
//
// ****************************************************************************
void
avtXolotlFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    Initialize();

    if (dimension == 0)
    {
        avtMeshType mt = AVT_RECTILINEAR_MESH;
        int nblocks = 1;
        int block_origin = 0;
        int spatial_dimension = 2;
        int topological_dimension = 2;
        double *extents = NULL;

        // 0D Helium Meshes/vars
        string meshname = "0D/Helium/Deuterium-mesh";
        AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin,
                    spatial_dimension, topological_dimension);
        AddScalarVarToMetaData(md, "Helium/Deuterium", meshname, AVT_ZONECENT);
        meshname = "0D/Helium/Tritium-mesh";
        AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin,
                    spatial_dimension, topological_dimension);
        AddScalarVarToMetaData(md, "Helium/Tritium", meshname, AVT_ZONECENT);
        meshname = "0D/Helium/Vacancies-mesh";
        AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin,
                    spatial_dimension, topological_dimension);
        AddScalarVarToMetaData(md, "Helium/Vacancies", meshname, AVT_ZONECENT);
        meshname = "0D/Helium/Interstitial-mesh";
        AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin,
                    spatial_dimension, topological_dimension);
        AddScalarVarToMetaData(md, "Helium/Interstitial", meshname, AVT_ZONECENT);

        // 0D Deuterium Meshes/vars
        meshname = "0D/Deuterium/Tritium-mesh";
        AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin,
                    spatial_dimension, topological_dimension);
        AddScalarVarToMetaData(md, "Deuterium/Tritium", meshname, AVT_ZONECENT);
        meshname = "0D/Deuterium/Vacancies-mesh";
        AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin,
                    spatial_dimension, topological_dimension);
        AddScalarVarToMetaData(md, "Deuterium/Vacancies", meshname, AVT_ZONECENT);
        meshname = "0D/Deuterium/Interstitial-mesh";
        AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin,
                    spatial_dimension, topological_dimension);
        AddScalarVarToMetaData(md, "Deuterium/Interstitial", meshname, AVT_ZONECENT);

        // 0D Tritium Meshes/vars
        meshname = "0D/Tritium/Vacancies-mesh";
        AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin,
                    spatial_dimension, topological_dimension);
        AddScalarVarToMetaData(md, "Tritium/Vacancies", meshname, AVT_ZONECENT);
        meshname = "0D/Tritium/Interstitial-mesh";
        AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin,
                    spatial_dimension, topological_dimension);
        AddScalarVarToMetaData(md, "Tritium/Interstitial", meshname, AVT_ZONECENT);

        // 0D Vacancy Meshes/vars
        meshname = "0D/Vacancies/Interstitial-mesh";
        AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin,
                    spatial_dimension, topological_dimension);
        AddScalarVarToMetaData(md, "Vacancies/Interstitial", meshname, AVT_ZONECENT);
    }
    else if (dimension == 1)
    {
        string meshname = "1D/mesh";
        avtMeshType mt = AVT_RECTILINEAR_MESH;
        int nblocks = 1;
        int block_origin = 0;
        int spatial_dimension = 2;
        int topological_dimension = 2;
        double *extents = NULL;
        AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin,
                          spatial_dimension, topological_dimension);


        AddScalarVarToMetaData(md, "Helium Concentration", meshname, AVT_ZONECENT);
        AddScalarVarToMetaData(md, "Deuterium Concentration", meshname, AVT_ZONECENT);
        AddScalarVarToMetaData(md, "Tritium Concentration", meshname, AVT_ZONECENT);
        AddScalarVarToMetaData(md, "Vacancies Concentration", meshname, AVT_ZONECENT);
        AddScalarVarToMetaData(md, "Interstitial Concentration", meshname, AVT_ZONECENT);
    }
    else
    {
        //TODO
    }
}


// ****************************************************************************
//  Method: avtXolotlFileFormat::GetMesh
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
//  Programmer: js9 -- generated by xml2avt
//  Creation:   Tue Mar 22 15:09:52 PST 2016
//
// ****************************************************************************
vtkDataSet *
avtXolotlFileFormat::GetMesh(int timestate, const char *meshname)
{
    if (dimension == 0)
    {

        // What mesh do we need?
        int meshSizeIndex[2];
        if (debug) cerr << "Getting mesh: " << meshname << endl;
        if (strncmp(meshname, "0D/Helium", strlen("0D/Helium")) == 0)
        {
            meshSizeIndex[0] = 0;
        }
        else if (strncmp(meshname, "0D/Deuterium", strlen("0D/Deuterium")) == 0)
        {
            meshSizeIndex[0] = 1;
        }
        else if (strncmp(meshname, "0D/Tritium", strlen("0D/Tritium")) == 0)
        {
            meshSizeIndex[0] = 2;
        }
        else if (strncmp(meshname, "0D/Vacancies", strlen("0D/Vacancies")) == 0)
        {
            meshSizeIndex[0] = 3;
        }
        else if (strncmp(meshname, "0D/Interstitial", strlen("0D/Interstitial")) == 0)
        {
            meshSizeIndex[0] = 4;
        }
        else
        {
            EXCEPTION1(InvalidVariableException, meshname);
        }

        // Get position of second variable
        const char *slash = strrchr(meshname, '/');
        if (slash && !strcmp(slash, "/Helium-mesh"))
        {
            meshSizeIndex[1] = 0;
        }
        else if (slash && !strcmp(slash, "/Deuterium-mesh"))
        {
            meshSizeIndex[1] = 1;
        }
        else if (slash && !strcmp(slash, "/Tritium-mesh"))
        {
            meshSizeIndex[1] = 2;
        }
        else if (slash && !strcmp(slash, "/Vacancies-mesh"))
        {
            meshSizeIndex[1] = 3;
        }
        else if (slash && !strcmp(slash, "/Interstitial-mesh"))
        {
            meshSizeIndex[1] = 4;
        }
        else
        {
            EXCEPTION1(InvalidVariableException, meshname);
        }
        nx = varMaxes[meshSizeIndex[0]] + 2;
        ny = varMaxes[meshSizeIndex[1]] + 2;
        nz = 1;

        // Create the mesh we need
        vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
        vtkFloatArray *xc = vtkFloatArray::New();
        vtkFloatArray *yc = vtkFloatArray::New();
        vtkFloatArray *zc = vtkFloatArray::New();

        xc->SetNumberOfTuples(nx);
        for (int i=0; i<nx; ++i)
            xc->SetComponent(i, 0, 1.0*i);

        yc->SetNumberOfTuples(ny);
        for (int i=0; i<ny; ++i)
            yc->SetComponent(i, 0, 1.0*i);

        zc->SetNumberOfTuples(nz);
        for (int i=0; i<nz; ++i)
            zc->SetComponent(i, 0, 1.0*i);

        rgrid->SetDimensions(nx, ny, nz);
        rgrid->SetXCoordinates(xc);
        rgrid->SetYCoordinates(yc);
        rgrid->SetZCoordinates(zc);
        xc->Delete();
        yc->Delete();
        zc->Delete();
        return rgrid;
    }
    else if (dimension == 1)
    {
        vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
        vtkFloatArray *xc = vtkFloatArray::New();
        vtkFloatArray *yc = vtkFloatArray::New();
        vtkFloatArray *zc = vtkFloatArray::New();

        nx = oneDGrid.size();
        ny = varMaxes[0] + 2;
        nz = 1;

        xc->SetNumberOfTuples(nx);
        for (int i = 0; i < nx; i++)
            xc->SetComponent(i, 0, (oneDGrid[i] - oneDGrid[isurface[timestate]]));

        yc->SetNumberOfTuples(ny);
        for (int i = 0; i < ny; i++)
            yc->SetComponent(i, 0, 1.0*i);

        zc->SetNumberOfTuples(nz);
        for (int i = 0; i < nz; i++)
            zc->SetComponent(i, 0, 1.0*i);

        rgrid->SetDimensions(nx, ny, nz);
        rgrid->SetXCoordinates(xc);
        rgrid->SetYCoordinates(yc);
        rgrid->SetZCoordinates(zc);
        xc->Delete();
        yc->Delete();
        zc->Delete();
        return rgrid;
    }
}

// ****************************************************************************
//  Method: avtXolotlFileFormat::GetPositionsOfVariableFromCompositionTable
//
//  Purpose:
//      Helper function that turns the selected variable name into a set of
//      indexes to retrieve the data from an HDF5 dataset.
//
//  Arguments:
//      variableIndes   Pointer to the 2 element array where we are setting
//                      our indexes
//      vn              The name of the requested variable
//
//  Programmer: James Kress
//
// ****************************************************************************
void
avtXolotlFileFormat::GetPositionsOfVariableFromCompositionTable(int *variableIndexes, const char *vn)
{
    if (strncmp(vn, "Helium", strlen("Helium")) == 0)
    {
        variableIndexes[0] = 0;
    }
    else if (strncmp(vn, "Deuterium", strlen("Deuterium")) == 0)
    {
        variableIndexes[0] = 1;
    }
    else if (strncmp(vn, "Tritium", strlen("Tritium")) == 0)
    {
        variableIndexes[0] = 2;
    }
    else if (strncmp(vn, "Vacancies", strlen("Vacancies")) == 0)
    {
        variableIndexes[0] = 3;
    }
    else if (strncmp(vn, "Interstitial", strlen("Interstitial")) == 0)
    {
        variableIndexes[0] = 4;
    }
    else
    {
        EXCEPTION1(InvalidVariableException, vn);
    }

    // If this is a 0D file, we need a second dimension
    if (dimension == 0)
    {
        // Get position of second variable
        const char *slash = strrchr(vn, '/');
        if (slash && !strcmp(slash, "/Helium"))
        {
            variableIndexes[1] = 0;
        }
        else if (slash && !strcmp(slash, "/Deuterium"))
        {
            variableIndexes[1] = 1;
        }
        else if (slash && !strcmp(slash, "/Tritium"))
        {
            variableIndexes[1] = 2;
        }
        else if (slash && !strcmp(slash, "/Vacancies"))
        {
            variableIndexes[1] = 3;
        }
        else if (slash && !strcmp(slash, "/Interstitial"))
        {
            variableIndexes[1] = 4;
        }
        else
        {
            EXCEPTION1(InvalidVariableException, vn);
        }
    }
}

// ****************************************************************************
//  Method: avtXolotlFileFormat::GetVar
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
//  Programmer: js9 -- generated by xml2avt
//  Creation:   Tue Mar 22 15:09:52 PST 2016
//
//   Kathleen Biagas, Tue Sep 10 17:30:33 PDT 2019
//   Create 'indicies' and 'data' arrays on the heap. Visual Studio won't
//   compile stack created arrays unless their size can be determined at
//   compile time.
//
// ****************************************************************************

vtkDataArray *
avtXolotlFileFormat::GetVar(int timestate, const char *vn)
{
    // Create output array
    vtkFloatArray *rv = vtkFloatArray::New();

    // Translate the timestate into a real cycle number
    int realTime = cycleNumbers[timestate];

    // Open the concentration group for this particular timestate
    char varname[100];
    snprintf(varname, 100, "concentration_%d", realTime);
    hid_t currentConcentration = H5Gopen(concentrationsGroup, varname, H5P_DEFAULT);
    if (currentConcentration < 0)
    {
        FreeUpResources();
        snprintf(varname, 100, "No 'concentration_%d' group found", realTime);
        EXCEPTION1(InvalidDBTypeException, varname);
    }

    // Read the concentration datasets
    hid_t concDset = H5Dopen(currentConcentration, "concs", H5P_DEFAULT);
    if (concDset < 0)
    {
        FreeUpResources();
        EXCEPTION1(InvalidDBTypeException, "No 'concs' group.");
    }

    hsize_t dims[1];
    hid_t  memtype = H5Tcreate(H5T_COMPOUND, sizeof (concentrationTypeStruct));
    H5Tinsert (memtype, "ConcType.first",
             HOFFSET(concentrationTypeStruct,clusterNumber), H5T_NATIVE_INT);
    H5Tinsert (memtype, "ConcType.second",
             HOFFSET(concentrationTypeStruct,concentration), H5T_NATIVE_FLOAT);
    hid_t space   = H5Dget_space(concDset);
    hid_t tyid    = H5Dget_type(concDset);
    hsize_t hsize = H5Dget_storage_size(concDset);
    hid_t conc_ndims = H5Sget_simple_extent_dims(space, dims, NULL);

    int rDataSize = dims[0];
    concentrationTypeStruct *rdata;
    rdata = (concentrationTypeStruct *) malloc (rDataSize * sizeof (concentrationTypeStruct));
    hid_t status = H5Dread (concDset, memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, rdata);

    // Read the indices dataset
    hid_t indexDset = H5Dopen(currentConcentration, "concs_startingIndices", H5P_DEFAULT);
    if (indexDset < 0)
    {
        FreeUpResources();
        EXCEPTION1(InvalidDBTypeException, "No 'concs_startingIndices' group.");
    }

    hid_t indexSpace = H5Dget_space(indexDset);
    hsize_t index_dims[1];
    hid_t index_ndims = H5Sget_simple_extent_dims(indexSpace, index_dims, NULL);

    int *indicies = new int[index_dims[0]];
    H5Dread(indexDset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, (void*)indicies);

    // Read the composition index to know which cluster is what
    hid_t compGetExtents = H5Dopen(fileId, "headerGroup/composition", H5P_DEFAULT);
    if (compGetExtents < 0)
    {
        FreeUpResources();
        EXCEPTION1(InvalidDBTypeException, "No 'headerGroup/composition'.");
    }

    hid_t sid = H5Dget_space(compGetExtents);
    hsize_t composition_dims[2];
    hid_t ndims = H5Sget_simple_extent_dims(sid, composition_dims, NULL);

    int *data = new int[composition_dims[0]*composition_dims[1]];
    H5Dread(compGetExtents, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, (void*)data);

    // Take the requested variable and turn that into an index into our composition table
    int variableIndexes[2] = {0,0};
    GetPositionsOfVariableFromCompositionTable(variableIndexes, vn);

    if (dimension == 0)
    {
        // Set nx and ny since they may not yet be set correctly
        nx = varMaxes[variableIndexes[0]] + 2;
        ny = varMaxes[variableIndexes[1]] + 2;

        // Since data is saved sparsely, all unsaved points are this special "ZERO"
        int nvals = (nx-1) * (ny-1);
        float zero = 1.0e-20;
        rv->SetNumberOfTuples(nvals);
        for (int i = 0; i < nvals; i++)
        {
            rv->SetTuple1(i, zero);
        }

        // Loop on the concentrations
        for (int i = indicies[0]; i < indicies[1]; i++)
        {
            // Skip the temperature which is appened at end of concs table
            if (rdata[i].clusterNumber > composition_dims[0] - 1)
            {
                continue;
            }
            // Get the x and y sizes of this cluster
            int base = int(rdata[i].clusterNumber) * composition_dims[1];
            float xSize = data[base + variableIndexes[0]];
            float ySize = data[base + variableIndexes[1]];
            int pos = (ySize * (nx - 1)) + xSize; // The coordinates had to be flipped here
            rv->SetTuple1(pos, (rv->GetTuple1(pos) + rdata[i].concentration));
        }
    }
    else if (dimension == 1)
    {
        // Set nx and ny since they may not yet be set correctly
        nx = oneDGrid.size();
        ny = varMaxes[0] + 2;
        nz = 1;

        // Since data is saved sparsely, all unsaved points are this special "ZERO"
        int nvals = (nx-1) * (ny-1);
        float zero = 1.0e-20;
        rv->SetNumberOfTuples(nvals);
        for (int i = 0; i < nvals; i++)
        {
            rv->SetTuple1(i, zero);
        }

        //Loop over the grid
        for (int j = 0; j < oneDGrid.size(); j++)
        {
            // Loop on the concentrations
            for (int i = indicies[j]; i < indicies[j + 1]; i++)
            {
                // Skip the temperature value(s)
                if (rdata[i].clusterNumber > composition_dims[0] - 1)
                {
                    continue;
                }
                // Get the x sizes of this cluster
                float xSize = data[int(rdata[i].clusterNumber)*composition_dims[1] + variableIndexes[0]];
                int pos = (xSize * (nx - 1)) + j;
                rv->SetTuple1(pos, (rv->GetTuple1(pos) + rdata[i].concentration));
            }
        }
    }
    delete [] indicies;
    delete [] data;
    // Cleanup everything that was opened
    H5Sclose(sid);
    H5Sclose(indexSpace);
    H5Sclose(space);
    H5Sclose(memtype);
    H5Dclose(concDset);
    H5Dclose(indexDset);
    H5Dclose(compGetExtents);
    H5Gclose(currentConcentration);
    free(rdata);
    rdata = NULL;
    return rv;
}


// ****************************************************************************
//  Method: avtXolotlFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      varname    The name of the variable requested.
//
//  Programmer: js9 -- generated by xml2avt
//  Creation:   Tue Mar 22 15:09:52 PST 2016
//
// ****************************************************************************
vtkDataArray *
avtXolotlFileFormat::GetVectorVar(int timestate, const char *varname)
{
    return NULL;
}
