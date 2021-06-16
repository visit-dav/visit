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

#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <InvalidDBTypeException.h>
#include <InvalidFilesException.h>
#include <InvalidTimeStepException.h>

#include <hdf5.h>
#include <visit-hdf5.h>

using namespace std;

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
avtXolotlFileFormat::avtXolotlFileFormat(const char *fn, const DBOptionsAttributes *readOpts)
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
    while (std::getline(ss, item, '_'))
    {
        if (!item.empty() && std::all_of(item.begin(), item.end(), ::isdigit))
        {
            cycleNum->push_back(stoi(item));
        }
    }
    return 0;
}


// ****************************************************************************
//  Method: avtXolotlFileFormat::PopulateNetworkGroupMetadata
//
//  Purpose:
//      Gets the network group meta data
//
//  Programmer: James Kress
//  Creation:   February 19, 2021
//
//
//  James Kress, Friday Apr 9 11:30:30 PDT 2021
//  Added the ability to visualize phase-space Xolot files.
//
// ****************************************************************************
void
avtXolotlFileFormat::PopulateNetworkGroupMetaData()
{
    // We need to stash the networkGroup attributes
    herr_t networkArrributeStatus = H5Aexists(networkGroup, "normalSize");
    if (networkArrributeStatus > 0)
    {
        hid_t normalSizeAttr = H5Aopen(networkGroup, "normalSize", H5P_DEFAULT);
        H5Aread(normalSizeAttr, H5T_NATIVE_INT, &normalSize);
        H5Aclose(normalSizeAttr);

    }

    networkArrributeStatus = H5Aexists(networkGroup, "superSize");
    if (networkArrributeStatus > 0)
    {
        hid_t superSizeAttr = H5Aopen(networkGroup, "superSize", H5P_DEFAULT);
        H5Aread(superSizeAttr, H5T_NATIVE_INT, &superSize);
        H5Aclose(superSizeAttr);
    }

    networkArrributeStatus = H5Aexists(networkGroup, "totalSize");
    if (networkArrributeStatus > 0)
    {
        hid_t totalSizeAttr = H5Aopen(networkGroup, "totalSize", H5P_DEFAULT);
        H5Aread(totalSizeAttr, H5T_NATIVE_INT, &totalSize);
        H5Aclose(totalSizeAttr);
    }

    networkArrributeStatus = H5Aexists(networkGroup, "phaseSpace");
    if (networkArrributeStatus > 0)
    {
        hid_t phaseSpaceAttr = H5Aopen(networkGroup, "phaseSpace", H5P_DEFAULT);

        // Get the datatype.
        hid_t  filetype = H5Aget_type(phaseSpaceAttr);

        // Check if the data type is an int, if not we continue reading 'phaseSpace'
        if(H5Tequal(filetype, H5T_NATIVE_INT) <= 0)
        {
            // Get dataspace and allocate memory for read buffer.
            hsize_t dims[1] = {5};
            hid_t  space = H5Aget_space (phaseSpaceAttr);
            int ndims = H5Sget_simple_extent_dims (space, dims, NULL);
            char **rdata = (char **) malloc (dims[0] * sizeof (char *));

            // Create the memory datatype.
            hid_t  memtype = H5Tcopy (H5T_C_S1);
            herr_t status = H5Tset_size (memtype, H5T_VARIABLE);

            // Read the data
            status = H5Aread (phaseSpaceAttr, memtype, rdata);

            // Output the data to the screen.
            for (int i=0; i<dims[0]; i++)
            {
                if(strcmp(rdata[i],"He") == 0)
                    variablesInPhaseSpace.push_back("Helium");
                else if(strcmp(rdata[i],"D") == 0)
                    variablesInPhaseSpace.push_back("Deuterium");
                else if(strcmp(rdata[i],"T") == 0)
                    variablesInPhaseSpace.push_back("Tritium");
                else if(strcmp(rdata[i],"V") == 0)
                    variablesInPhaseSpace.push_back("Vacancies");
                else if(strcmp(rdata[i],"I") == 0)
                    variablesInPhaseSpace.push_back("Interstitial");
                debug1 << "variablesInPhaseSpace[" << i << "]: " << variablesInPhaseSpace[i] << endl;
            }

            free(rdata);
            rdata = NULL;
        }

        H5Aclose(phaseSpaceAttr);
    }

    // Loop over the clusters to get max value in order to set grid dimensions
    // when visualing phase-space
    for (int j = 0; j < totalSize; j++)
    {
        // Open the network group for this itteration
        char clusterName[100];
        snprintf(clusterName, 100, "%d", j);
        hid_t currentCluster = H5Gopen(networkGroup, clusterName, H5P_DEFAULT);
        if (currentCluster < 0)
        {
            FreeUpResources();
            snprintf(clusterName, 100, "No '%d' network found", j);
            EXCEPTION1(InvalidDBTypeException, clusterName);
        }

        // Read the cluster bounds
        hid_t boundsAttr = H5Aopen(currentCluster, "bounds", H5P_DEFAULT);
        hsize_t dims[1] = {0};
        hid_t  space = H5Aget_space (boundsAttr);
        int ndims = H5Sget_simple_extent_dims (space, dims, NULL);
        int *currentBoundsArray = new int[dims[0]-1];
        H5Aread(boundsAttr, H5T_NATIVE_INT, (void*)currentBoundsArray);

        // Get max value from bounds array
        for (int m = 0; m < dims[0]; m++)
        {
            if(currentBoundsArray[m] > phaseSpaceMaxDims)
                phaseSpaceMaxDims = currentBoundsArray[m];
        }

        H5Aclose(boundsAttr);
        H5Gclose(currentCluster);
        delete [] currentBoundsArray;
    }

    debug1 << "normalSize=" << normalSize << endl;
    debug1 << "superSize=" << superSize << endl;
    debug1 << "totalSize=" << totalSize << endl;
    debug1 << "phase-space max grid dimensions=" << phaseSpaceMaxDims << endl;
    debug1 << "Number of phase space vars=" << variablesInPhaseSpace.size() << endl;

    // close and cleanup
    H5Gclose(networkGroup);
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
    debug1 << "Xolotl:: nTimeStates = " << nTimeStates << endl;


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
//  James Kress, Friday Apr 9 11:30:30 PDT 2021
//  Added the ability to visualize phase-space Xolot files.
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

    // Read the variables
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

    debug1 << "hx=" <<hx << endl;
    debug1 << "hy=" <<hy << endl;
    debug1 << "hz=" <<hz << endl;
    debug1 << "nx=" <<nx << endl;
    debug1 << "ny=" <<ny << endl;
    debug1 << "nz=" <<nz << endl;
    debug1 << "dimension=" << dimension << endl;

    // Close the attributes and group
    H5Aclose(hxAttr);
    H5Aclose(hyAttr);
    H5Aclose(hzAttr);
    H5Aclose(nxAttr);
    H5Aclose(nyAttr);
    H5Aclose(nzAttr);
    H5Gclose(headerGroup);

    // Figure out file dimensionality
    if (nx > 1)
        dimension = 1;
    else
        dimension = 0;

    // Determine if we are visualizing concentrations or phase-space by
    // checking for the composition table
    herr_t compositionDataSetStatus = H5Lexists(fileId, "headerGroup/composition", H5P_DEFAULT);
    if (compositionDataSetStatus <= 0)
    {
        // We are visualizing phasespace
        dimension = 3;

        // Create a grid double the dimensions since this is phase-space
        for (int i = 0; i <= phaseSpaceMaxDims; i++)
        {
            oneDGrid.push_back(0);
        }
    }
    else
    {
        // We are visualizing concentrations
        // Now we need to get the data ranges from the composition data set
        hid_t compositionDataSet = H5Dopen(fileId, "headerGroup/composition", H5P_DEFAULT);
        hid_t sid = H5Dget_space(compositionDataSet);
        hsize_t composition_dims[2];
        hid_t ndims = H5Sget_simple_extent_dims(sid, composition_dims, NULL);
        debug1 << "dimensions " <<
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

        // Close HDF resources
        H5Sclose(sid);
        H5Dclose(compositionDataSet);
    }

    if (dimension == 1)
    {
        // Now we need to get grid since this is a 1D file
        hid_t gridDataSet = H5Dopen(fileId, "headerGroup/grid", H5P_DEFAULT);
        if (gridDataSet < 0)
        {
            FreeUpResources();
            EXCEPTION1(InvalidDBTypeException, "No 'headerGroup/grid'.");
        }

        hid_t sid = H5Dget_space(gridDataSet);
        hsize_t headerGrid_dims[1];
        hid_t ndims = H5Sget_simple_extent_dims(sid, headerGrid_dims, NULL);
        debug1 << "dimensions " <<
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

        // Cleanup and close
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
//   Modifications:
//   James Kress, Thur Feb 25 17:30:33 PDT 2021
//   Added a PopulateNetworkGroupMetaData() function to simplify Initialize().
//
//
//  James Kress, Friday Apr 9 11:30:30 PDT 2021
//  Moved the PopulateNetworkGroupMetaData() call to gather info necessary
//  for the other method calls.
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
    if ((fileId = H5Fopen(filenames[0], H5F_ACC_RDONLY, fileAccessPropListID)) < 0)
    {
        char error[1024];
        snprintf(error, 1024, "Cannot be a Xolotl file (%s)",filenames[0]);

        EXCEPTION1(InvalidDBTypeException, error);
    }
    H5Pclose(fileAccessPropListID);

    // Read the network
    networkGroup = H5Gopen(fileId, "networkGroup", H5P_DEFAULT);
    if (networkGroup<0)
    {
        FreeUpResources();
        EXCEPTION1(InvalidDBTypeException, "No 'networkGroup'.");
    }
    PopulateNetworkGroupMetaData();

    // Open the header group and read mesh attributes
    PopulateHeaderGroupMetaData();

    // Open the concentrations group and read the number of time steps
    concentrationsGroup = H5Gopen(fileId, "concentrationsGroup", H5P_DEFAULT);
    if (concentrationsGroup < 0)
    {
        FreeUpResources();
        EXCEPTION1(InvalidDBTypeException, "No 'concentrationsGroup'.");
    }
    PopulateConcentrationGroupMetaData();
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
    if (concentrationsGroup >= 0)
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
        avtMeshType mt = AVT_RECTILINEAR_MESH;
        int nblocks = 1;
        int block_origin = 0;
        int spatial_dimension = 3;
        int topological_dimension = 3;
        double *extents = NULL;

        avtMeshMetaData *mmd;
        string currentVar0, currentVar1 , currentVar2;
        char meshname[512], variablename[256];
        string ps0, ps1, ps2, ps3, ps4;
        ps0 = variablesInPhaseSpace[0];
        ps1 = variablesInPhaseSpace[1];
        ps2 = variablesInPhaseSpace[2];

        // Create the variables based on the number of phase-space variables
        if (variablesInPhaseSpace.size() == 5)
        {
            ps3 = variablesInPhaseSpace[3];
            ps4 = variablesInPhaseSpace[4];

            // Phase-Space ps0 vars
            currentVar0 = ps0;
            currentVar1 = ps1;
            currentVar2 = ps2;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);

            currentVar0 = ps0;
            currentVar1 = ps1;
            currentVar2 = ps3;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);

            currentVar0 = ps0;
            currentVar1 = ps1;
            currentVar2 = ps4;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);

            currentVar0 = ps0;
            currentVar1 = ps2;
            currentVar2 = ps1;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);

            currentVar0 = ps0;
            currentVar1 = ps2;
            currentVar2 = ps3;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);

            currentVar0 = ps0;
            currentVar1 = ps2;
            currentVar2 = ps4;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);

            currentVar0 = ps0;
            currentVar1 = ps3;
            currentVar2 = ps1;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);

            currentVar0 = ps0;
            currentVar1 = ps3;
            currentVar2 = ps2;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);

            currentVar0 = ps0;
            currentVar1 = ps3;
            currentVar2 = ps4;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);

            currentVar0 = ps0;
            currentVar1 = ps4;
            currentVar2 = ps1;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);

            currentVar0 = ps0;
            currentVar1 = ps4;
            currentVar2 = ps2;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);

            currentVar0 = ps0;
            currentVar1 = ps4;
            currentVar2 = ps3;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);


            // Phase-Space ps1 vars
            currentVar0 = ps1;
            currentVar1 = ps2;
            currentVar2 = ps3;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);

            currentVar0 = ps1;
            currentVar1 = ps2;
            currentVar2 = ps4;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);

            currentVar0 = ps1;
            currentVar1 = ps3;
            currentVar2 = ps2;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);

            currentVar0 = ps1;
            currentVar1 = ps3;
            currentVar2 = ps4;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);

            currentVar0 = ps1;
            currentVar1 = ps4;
            currentVar2 = ps2;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);

            currentVar0 = ps1;
            currentVar1 = ps4;
            currentVar2 = ps3;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);


            // Phase-Space ps2 vars
            currentVar0 = ps2;
            currentVar1 = ps3;
            currentVar2 = ps4;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);

            currentVar0 = ps2;
            currentVar1 = ps4;
            currentVar2 = ps3;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);
        }
        else if (variablesInPhaseSpace.size() == 4)
        {
            ps3 = variablesInPhaseSpace[3];

            // Phase-Space ps0 vars
            currentVar0 = ps0;
            currentVar1 = ps1;
            currentVar2 = ps2;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);

            currentVar0 = ps0;
            currentVar1 = ps1;
            currentVar2 = ps3;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);

            currentVar0 = ps0;
            currentVar1 = ps2;
            currentVar2 = ps1;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);

            currentVar0 = ps0;
            currentVar1 = ps2;
            currentVar2 = ps3;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);

            currentVar0 = ps0;
            currentVar1 = ps3;
            currentVar2 = ps1;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);

            currentVar0 = ps0;
            currentVar1 = ps3;
            currentVar2 = ps2;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);

            // Phase-Space ps1 vars
            currentVar0 = ps1;
            currentVar1 = ps2;
            currentVar2 = ps3;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);

            currentVar0 = ps1;
            currentVar1 = ps3;
            currentVar2 = ps2;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);
        }
        else
        {
            // Phase-Space ps0 vars
            currentVar0 = ps0;
            currentVar1 = ps1;
            currentVar2 = ps2;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);

            currentVar0 = ps0;
            currentVar1 = ps2;
            currentVar2 = ps1;
            sprintf(variablename, "%s/%s/%s",currentVar0.c_str(), currentVar1.c_str() , currentVar2.c_str());
            sprintf(meshname, "Phase-Space %s", variablename);
            AddScalarVarToMetaData(md, variablename, meshname, AVT_ZONECENT);
            mmd = new avtMeshMetaData(meshname, 0, 0, 0, 0, 3, 3, mt);
            mmd->zLabel = currentVar0;
            mmd->yLabel = currentVar1 ;
            mmd->xLabel = currentVar2;
            md->Add(mmd);
        }
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
        debug1 << "Getting mesh: " << meshname << endl;
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
    else if (dimension == 3)
    {
        vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
        vtkFloatArray *xc = vtkFloatArray::New();
        vtkFloatArray *yc = vtkFloatArray::New();
        vtkFloatArray *zc = vtkFloatArray::New();

        nx = oneDGrid.size() + 1;
        ny = oneDGrid.size() + 1;
        nz = oneDGrid.size() + 1;

        xc->SetNumberOfTuples(nx);
        for (int i = 0; i < nx; i++)
            xc->SetComponent(i, 0, 1.0*i);

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

    return nullptr;
}

// ****************************************************************************
//  Method: avtXolotlFileFormat::First
//
//  Purpose:
//      if x is present in arr[] then returns the count
//      of occurrences of x, otherwise returns 0.
//
//  Arguments:
//      arr   Pointer to the 2 element array
//      x     The size of the array
//      n     The number of interest
//
//  Programmer: James Kress
//
// ****************************************************************************
int
avtXolotlFileFormat::First(int arr[], int n, int x)
{
    int first = -1;
    for (int i = 0; i < n; i++) {
        if (x != arr[i])
            continue;
        if (first == -1)
            return i;
    }

    return -1;
}

// ****************************************************************************
//  Method: avtXolotlFileFormat::findInVector
//
//  Purpose:
//      Generic function to find an element in vector and also its position.
//      It returns a pair of bool & int
//
//      bool : Represents if element is present in vector or not.
//      int : Represents the index of element in vector if its found else -1
//
//  Arguments:
//      vecOfElements   Vector to search
//      element         The element of interest
//
//  Programmer: James Kress
//
// ****************************************************************************
template < typename T> std::pair<bool, int >
avtXolotlFileFormat::findInVector(const std::vector<T>  & vecOfElements, const T  & element)
{
    std::pair<bool, int > result;
    // Find given element in vector
    auto it = std::find(vecOfElements.begin(), vecOfElements.end(), element);
    if (it != vecOfElements.end())
    {
        result.second = distance(vecOfElements.begin(), it);
        result.first = true;
    }
    else
    {
        result.first = false;
        result.second = -1;
    }
    return result;
}

// ****************************************************************************
//  Method: avtXolotlFileFormat::GetPositionsOfVariableFromCompositionTable
//
//  Purpose:
//      Helper function that turns the selected variable name into a set of
//      indexes to retrieve the data from an HDF5 dataset.
//
//  Arguments:
//      variableIndexes   Pointer to the 2 element array where we are setting
//                      our indexes
//      vn              The name of the requested variable
//
//  Programmer: James Kress
//
//  James Kress, Friday Apr 9 11:30:30 PDT 2021
//  updated the indexing scheme to support phase-space, which may have
//  variable numbers of phase-space vars present
//
//  Kathleen Biagas, Fri Jun 4, 2021
//  Switch from strsep to strtok_s/_r since strsep isn't available on Windows.
//
// ****************************************************************************

#ifdef _WIN32
#define STRTOK strtok_s
#else
#define STRTOK strtok_r
#endif

void
avtXolotlFileFormat::GetPositionsOfVariableFromCompositionTable(int *variableIndexes, const char *vn)
{
    char *token, *nextToken=NULL, *str, *tofree;
    tofree = str = strdup(vn);

    int currentVariableNumber = 0;
    token = STRTOK(str, "/", &nextToken);
    while (token)
    {
        if (strncmp(token, "Helium", strlen("Helium")) == 0)
        {
            if (variablesInPhaseSpace.size() > 0)
            {
                std::pair<bool, int> res = findInVector(variablesInPhaseSpace, std::string("Helium"));
                variableIndexes[currentVariableNumber] = res.second;
            }
            else
                variableIndexes[currentVariableNumber] = 0;
        }
        else if (strncmp(token, "Deuterium", strlen("Deuterium")) == 0)
        {
            if (variablesInPhaseSpace.size() > 0)
            {
                std::pair<bool, int> res = findInVector(variablesInPhaseSpace, std::string("Deuterium"));
                variableIndexes[currentVariableNumber] = res.second;
            }
            else
                variableIndexes[currentVariableNumber] = 1;
        }
        else if (strncmp(token, "Tritium", strlen("Tritium")) == 0)
        {
            if (variablesInPhaseSpace.size() > 0)
            {
                std::pair<bool, int> res = findInVector(variablesInPhaseSpace, std::string("Tritium"));
                variableIndexes[currentVariableNumber] = res.second;
            }
            else
                variableIndexes[currentVariableNumber] = 2;
        }
        else if (strncmp(token, "Vacancies", strlen("Vacancies")) == 0)
        {
            if (variablesInPhaseSpace.size() > 0)
            {
                std::pair<bool, int> res = findInVector(variablesInPhaseSpace, std::string("Vacancies"));
                variableIndexes[currentVariableNumber] = res.second;
            }
            else
                variableIndexes[currentVariableNumber] = 3;
        }
        else if (strncmp(token, "Interstitial", strlen("Interstitial")) == 0)
        {
            if (variablesInPhaseSpace.size() > 0)
            {
                std::pair<bool, int> res = findInVector(variablesInPhaseSpace, std::string("Interstitial"));
                variableIndexes[currentVariableNumber] = res.second;
            }
            else
                variableIndexes[currentVariableNumber] = 4;
        }
        else
        {
            EXCEPTION1(InvalidVariableException, vn);
        }

        currentVariableNumber++;

        token=STRTOK(NULL, "/", &nextToken);
    }

    // If we are in 3 dimensions we need the array indexs for our unused variables
    if (dimension == 3)
    {
        for (int z = 0; z < 5; z++)
        {
            //Check if value for 'z' has been set
            int result = First(variableIndexes, 5, z);
            if (result == -1)
            {
                variableIndexes[currentVariableNumber] = z;
                currentVariableNumber++;
            }
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
//  James Kress, Thur Feb 25 17:30:33 PDT 2021
//  Added the ability to visualize normal and super clusters for
//  1D Xolotl files.
//
//  James Kress, Friday Apr 9 11:30:30 PDT 2021
//  Added the ability to visualize phase-space Xolot files.
//
// ****************************************************************************

vtkDataArray *
avtXolotlFileFormat::GetVar(int timestate, const char *vn)
{
    // Create output array
    vtkFloatArray *rv = vtkFloatArray::New();

    // Translate the timestate into a real cycle number
    int realTime = cycleNumbers[timestate];

    // Check if we are doing Phase-Space or cluster vis
    if (dimension != 3)
    {
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
        int variableIndexes[5] = {0,0,0,0,0};
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

            // Open the network for reading
            networkGroup = H5Gopen(fileId, "networkGroup", H5P_DEFAULT);
            if (networkGroup < 0)
            {
                FreeUpResources();
                EXCEPTION1(InvalidDBTypeException, "No 'networkGroup'.");
            }

            // Loop over the grid
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

                    // Take care of the normal clustetrs
                    if (rdata[i].clusterNumber < normalSize)
                    {
                        // Get the x sizes of this cluster
                        // variableIndexes tells us either Helium, Deuterium, Tritium, Vacancies
                        float xSize = data[int(rdata[i].clusterNumber)*composition_dims[1] + variableIndexes[0]];
                        int pos = (xSize * (nx - 1)) + j;
                        rv->SetTuple1(pos, (rv->GetTuple1(pos) + rdata[i].concentration));
                    }
                    else // Take care of the super clusters
                    {
                        // Loop on the number of clusters it contains
                        // Open the network group for this particular cluster
                        char clusterName[100];
                        snprintf(clusterName, 100, "%d", rdata[i].clusterNumber);
                        hid_t currentCluster = H5Gopen(networkGroup, clusterName, H5P_DEFAULT);
                        if (currentCluster < 0)
                        {
                            FreeUpResources();
                            snprintf(varname, 100, "No '%d' network found", rdata[i].clusterNumber);
                            EXCEPTION1(InvalidDBTypeException, clusterName);
                        }

                        // Read the heVList
                        hid_t heVList = H5Dopen(currentCluster, "heVList", H5P_DEFAULT);
                        if (heVList < 0)
                        {
                            FreeUpResources();
                            EXCEPTION1(InvalidDBTypeException, "No 'heVList' group.");
                        }

                        hsize_t vListDims[1];
                        hid_t heVListSpace   = H5Dget_space(heVList);
                        hid_t heVList_ndims = H5Sget_simple_extent_dims(heVListSpace, vListDims, NULL);

                        // Read the values in the heVList table
                        int *heVListTableData = new int[vListDims[0] * 4];
                        H5Dread(heVList, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, (void*)heVListTableData);

                        for ( int k = 0; k < vListDims[0]; k++)
                        {
                            float xSize = heVListTableData[k * 4 + variableIndexes[0]];
                            int pos = (xSize * (nx - 1)) + j;
                            rv->SetTuple1(pos, (rv->GetTuple1(pos) + rdata[i].concentration));
                        }

                        H5Sclose(heVListSpace);
                        H5Dclose(heVList);
                        H5Gclose(currentCluster);
                        delete [] heVListTableData;
                    }
                }
            }
        }

        // Cleanup everything that was opened
        delete [] data;
        delete [] indicies;
        H5Sclose(indexSpace);
        H5Sclose(space);
        H5Sclose(memtype);
        H5Dclose(concDset);
        H5Dclose(indexDset);
        H5Gclose(currentConcentration);
        H5Sclose(sid);
        H5Dclose(compGetExtents);
        free(rdata);
        rdata = NULL;
    }
    else //dimension == 3
    {
        // Set nx and ny since they may not yet be set correctly
        nx = oneDGrid.size() + 1;
        ny = oneDGrid.size() + 1;
        nz = oneDGrid.size() + 1;

        // Since data is saved sparsely, zero out the array
        int nvals = (nx -1) * (ny -1) * (nz -1);
        rv->SetNumberOfTuples(nvals);
        for (int i = 0; i < nvals; i++)
        {
            rv->SetTuple1(i, 0);
        }

        // Open the network for reading
        networkGroup = H5Gopen(fileId, "networkGroup", H5P_DEFAULT);
        if (networkGroup < 0)
        {
            FreeUpResources();
            EXCEPTION1(InvalidDBTypeException, "No 'networkGroup'.");
        }

        // Take the requested variable and turn that into an index into our currentBoundsArray
        int variableIndexes[5] = {-1,-1,-1,-1,-1};
        GetPositionsOfVariableFromCompositionTable(variableIndexes, vn);

        // Loop over the grid
        for (int j = 0; j < totalSize; j++)
        {
            // Open the network group for this itteration
            char clusterName[100];
            snprintf(clusterName, 100, "%d", j);
            hid_t currentCluster = H5Gopen(networkGroup, clusterName, H5P_DEFAULT);
            if (currentCluster < 0)
            {
                FreeUpResources();
                snprintf(clusterName, 100, "No '%d' network found", j);
                EXCEPTION1(InvalidDBTypeException, clusterName);
            }

            // Read the cluster bounds
            hid_t boundsAttr = H5Aopen(currentCluster, "bounds", H5P_DEFAULT);
            hsize_t dims[1] = {0};
            hid_t  space = H5Aget_space (boundsAttr);
            int ndims = H5Sget_simple_extent_dims (space, dims, NULL);
            int *currentBoundsArray = new int[dims[0]-1];
            H5Aread(boundsAttr, H5T_NATIVE_INT, (void*)currentBoundsArray);

            int firstIndex, secondIndex, thirdIndex, fourthIndex, fifthIndex;
            firstIndex = variableIndexes[0] + variableIndexes[0];
            secondIndex = variableIndexes[1] + variableIndexes[1];
            thirdIndex = variableIndexes[2] + variableIndexes[2];

            // Setup mesh based on number of phase-space vars
            if (variablesInPhaseSpace.size() == 3)
            {
                for (int m = currentBoundsArray[thirdIndex]; m <= currentBoundsArray[thirdIndex + 1]; m++)
                {
                    for (int k = currentBoundsArray[secondIndex]; k <= currentBoundsArray[secondIndex + 1]; k++)
                    {
                        for (int l = currentBoundsArray[firstIndex]; l <= currentBoundsArray[firstIndex + 1]; l++)
                        {
                            int pos = (l * (ny - 1) * (nx - 1)) + ((nx - 1) * k) + m;
                            rv->SetTuple1(pos, 100);
                        }
                    }
              }
            }
            else if (variablesInPhaseSpace.size() == 4)
            {
                fourthIndex = variableIndexes[3] + variableIndexes[3];
                if (currentBoundsArray[fourthIndex] == 0)
                {
                    for (int m = currentBoundsArray[thirdIndex]; m <= currentBoundsArray[thirdIndex + 1]; m++)
                    {
                        for (int k = currentBoundsArray[secondIndex]; k <= currentBoundsArray[secondIndex + 1]; k++)
                        {
                            for (int l = currentBoundsArray[firstIndex]; l <= currentBoundsArray[firstIndex + 1]; l++)
                            {
                                int pos = (l * (ny - 1) * (nx - 1)) + ((nx - 1) * k) + m;
                                rv->SetTuple1(pos, 100);
                            }
                        }
                    }
                }
            }
            else if (variablesInPhaseSpace.size() == 5)
            {
                fourthIndex = variableIndexes[3] + variableIndexes[3];
                fifthIndex = variableIndexes[4] + variableIndexes[4];
                if (currentBoundsArray[fourthIndex] == 0 && currentBoundsArray[fifthIndex] == 0)
                {
                    for (int m = currentBoundsArray[thirdIndex]; m <= currentBoundsArray[thirdIndex + 1]; m++)
                    {
                        for (int k = currentBoundsArray[secondIndex]; k <= currentBoundsArray[secondIndex + 1]; k++)
                        {
                            for (int l = currentBoundsArray[firstIndex]; l <= currentBoundsArray[firstIndex + 1]; l++)
                            {
                                int pos = (l * (ny - 1) * (nx - 1)) + ((nx - 1) * k) + m;
                                rv->SetTuple1(pos, 100);
                            }
                        }
                    }
                }
            }

            H5Aclose(boundsAttr);
            H5Gclose(currentCluster);
            delete [] currentBoundsArray;
        }
    }

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
