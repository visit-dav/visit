// ************************************************************************* //
//                            avtPixieFileFormat.C                           //
// ************************************************************************* //

#include <avtPixieFileFormat.h>

#include <string>

#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkIntArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>
#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <InvalidDBTypeException.h>
#include <InvalidFilesException.h>
#include <InvalidTimeStepException.h>
#include <snprintf.h>

using     std::string;

#define MAKE_SURE_THE_FILE_IS_NOT_TETRAD

// ****************************************************************************
//  Method: avtPixie constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Aug 13 14:31:43 PST 2004
//
// ****************************************************************************

avtPixieFileFormat::avtPixieFileFormat(const char *filename)
    : avtMTSDFileFormat(&filename, 1), variables(), meshes()
{
     fileId = -1;
     nTimeStates = 0;

#ifdef MAKE_SURE_THE_FILE_IS_NOT_TETRAD
     // This sucks to have to call this here but it's the only way to
     // make sure that this file format does not suck up other file formats'
     // data. This is primarily a check to make sure that the Tetrad
     // file format, which also has a .h5 extension continues to work.
     Initialize();
#endif
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
        H5Fclose(fileId);
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
//   
// ****************************************************************************

void
avtPixieFileFormat::GetCycles(std::vector<int> &cycles)
{
    int nts = (nTimeStates < 1) ? 1 : nTimeStates;
    for(int i = 0; i < nts; ++i)
        cycles.push_back(i);
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
//   
// ****************************************************************************

void
avtPixieFileFormat::GetTimes(std::vector<double> &times)
{
    int nts = (nTimeStates < 1) ? 1 : nTimeStates;
    for(int i = 0; i < nts; ++i)
        times.push_back(double(i));
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
//   
// ****************************************************************************
    
void
avtPixieFileFormat::Initialize()
{
    if(fileId == -1)
    {
        // Initialize some variables.
        meshes.clear();
        variables.clear();
        nTimeStates = 0;

        if((fileId = H5Fopen(filenames[0], H5F_ACC_RDONLY, H5P_DEFAULT)) < 0)
        {
            EXCEPTION1(InvalidFilesException, (const char *)filenames[0]);
        }

#ifdef MAKE_SURE_THE_FILE_IS_NOT_TETRAD
        // Turn off error message printing.
        H5Eset_auto(0,0);

        //
        // See if the file format looks like a Tetrad file. I know it's
        // hackish to have to check like this but how else should it be
        // done when we don't want Pixie to read HDF5 files that happen
        // to have Tetrad stuff in them.
        //
        hid_t cell_array = H5Dopen(fileId, "CellArray");
        if (cell_array >= 0)
        {
            H5Dclose(cell_array);
            EXCEPTION1(InvalidDBTypeException,
               "Cannot be a Pixie file because it looks like a Tetrad file.");
        }
#endif
        // Populate the scalar variable list
        int gid;
        if ((gid = H5Gopen(fileId, "/")) < 0)
        {
            EXCEPTION1(InvalidFilesException, (const char *)filenames[0]);
        }
        TraversalInfo info;
        info.This = this; 
        info.level = 0;
        info.path = "/";
        H5Giterate(fileId, "/", NULL, GetVariableList, (void*)&info);
        H5Gclose(gid);

        // Determine the names of the meshes that we'll need.
        for(VarInfoMap::const_iterator it = variables.begin();
            it != variables.end(); ++it)
        {
            char tmp[100];
            SNPRINTF(tmp, 100, "mesh_%dx%dx%d", int(it->second.dims[0]),
                     int(it->second.dims[1]),
                     int(it->second.dims[2]));
            meshes[std::string(tmp)] = it->second;
        }

#ifdef MDSERVER
        // We're on the mdserver so close the file now that we've determined
        // the variables in it.
        H5Fclose(fileId);
        fileId = -1;
#endif
    }
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
// ****************************************************************************

void
avtPixieFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
#ifndef MAKE_SURE_THE_FILE_IS_NOT_TETRAD
    //
    // Initialize the file if it has not been initialized.
    //
    Initialize();
#endif

    VarInfoMap::const_iterator it;
    for(it = meshes.begin();
        it != meshes.end(); ++it)
    {
        // Determine the number of spatial dimensions.
        int nSpatialDims = 0;
        if(it->second.dims[0] > 1) ++nSpatialDims;
        if(it->second.dims[1] > 1) ++nSpatialDims;
        if(it->second.dims[2] > 1) ++nSpatialDims;
        if(nSpatialDims == 0)
            continue;

        // Add the mesh.
        avtMeshMetaData *mmd = new avtMeshMetaData;
        mmd->name = it->first;
        mmd->spatialDimension = nSpatialDims;
        mmd->topologicalDimension = nSpatialDims;
        mmd->meshType = AVT_RECTILINEAR_MESH;
        mmd->cellOrigin = 1;
        md->Add(mmd);
    }

    // If we have more than 5 meshes, enable catchall mesh.
    if(meshes.size() > 5)
        md->SetUseCatchAllMesh(true);

    // Iterate through the variables and add them to the metadata.
    for(it = variables.begin();
        it != variables.end(); ++it)
    {
        // Determine the mesh name based on the variable mesh size.
        char tmp[100];
        SNPRINTF(tmp, 100, "mesh_%dx%dx%d", int(it->second.dims[0]),
                 int(it->second.dims[1]),
                 int(it->second.dims[2]));

        // Add a zonal scalar to the metadata.
        AddScalarVarToMetaData(md, it->first, tmp, AVT_ZONECENT);        
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

    // Check the mesh name.
    string meshNameString(meshname);
    VarInfoMap::const_iterator it = meshes.find(meshNameString);
    if(it == meshes.end())
    {
        EXCEPTION1(InvalidVariableException, meshNameString);
    }

    // Determine the number of spatial dimensions.
    int nSpatialDims = 0;
    if(it->second.dims[0] > 1) ++nSpatialDims;
    if(it->second.dims[1] > 1) ++nSpatialDims;
    if(it->second.dims[2] > 1) ++nSpatialDims;
    if(nSpatialDims < 2)
    {
        EXCEPTION1(InvalidVariableException, meshNameString);
    }

    //
    // Create a rectilinear grid for now. Curvlinear can come later
    // once the convention is determined...
    //

    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
    int XDIMS = 1, YDIMS = 1, ZDIMS = 1;
    if(nSpatialDims == 2)
    {
        int d[3], di = 0;
        if(it->second.dims[0] > 1)
            d[di++] = int(it->second.dims[0]);
        if(it->second.dims[1] > 1)
            d[di++] = int(it->second.dims[1]);
        if(it->second.dims[2] > 1)
            d[di++] = int(it->second.dims[2]);
        XDIMS = d[0]+1;
        YDIMS = d[1]+1;
    }
    else if(nSpatialDims == 3)
    {
        XDIMS = int(it->second.dims[0])+1;
        YDIMS = int(it->second.dims[1])+1;
        ZDIMS = int(it->second.dims[2])+1;
    }

    int dims[3] = {XDIMS, YDIMS, ZDIMS};
    int ndims = nSpatialDims;
    vtkFloatArray *coords[3];
    for (int i = 0 ; i < 3 ; i++)
    {
        // Default number of components for an array is 1.
        coords[i] = vtkFloatArray::New();

        if (i < ndims)
        {
            coords[i]->SetNumberOfTuples(dims[i]);
            for (int j = 0; j < dims[i]; j++)
                coords[i]->SetComponent(j, 0, j);
        }
        else
        {
            dims[i] = 1;
            coords[i]->SetNumberOfTuples(1);
            coords[i]->SetComponent(0, 0, 0.);
        }
    }

    rgrid->SetDimensions(dims);
    rgrid->SetXCoordinates(coords[0]);
    coords[0]->Delete();
    rgrid->SetYCoordinates(coords[1]);
    coords[1]->Delete();
    rgrid->SetZCoordinates(coords[2]);
    coords[2]->Delete();

    return rgrid;
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

    // Check the variable name.
    VarInfoMap::const_iterator it = variables.find(varname);
    if(it == variables.end())
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    //
    // Try and open the data.
    //
    hid_t dataId = H5Dopen(fileId, it->second.fileVarName.c_str());
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

    //
    // Try and read the data from the file.
    //
    vtkDataArray *scalars = 0;
    int nels = it->second.dims[0] * it->second.dims[1] * it->second.dims[2];
    if(H5Tequal(it->second.nativeVarType, H5T_NATIVE_INT) > 0 ||
       H5Tequal(it->second.nativeVarType, H5T_NATIVE_UINT) > 0)
    {
        vtkIntArray *iscalars = vtkIntArray::New();
        iscalars->SetNumberOfTuples(nels);
        scalars = iscalars;

        // Read the data into all_vars array.
        if(H5Dread(dataId, it->second.nativeVarType, H5S_ALL, spaceId,
                   H5P_DEFAULT, iscalars->GetVoidPointer(0)) < 0)
        {
            H5Sclose(spaceId);
            H5Dclose(dataId);
            EXCEPTION1(InvalidVariableException, varname);
        }
    }
    else if(H5Tequal(it->second.nativeVarType, H5T_NATIVE_FLOAT) > 0)
    {
        vtkFloatArray *fscalars = vtkFloatArray::New();
        fscalars->SetNumberOfTuples(nels);
        scalars = fscalars;

        // Read the data into all_vars array.
        if(H5Dread(dataId, it->second.nativeVarType, H5S_ALL, spaceId,
                   H5P_DEFAULT, fscalars->GetVoidPointer(0)) < 0)
        {
            H5Sclose(spaceId);
            H5Dclose(dataId);
            EXCEPTION1(InvalidVariableException, varname);
        }   
    }
    else if(H5Tequal(it->second.nativeVarType, H5T_NATIVE_DOUBLE) > 0)
    {
        vtkDoubleArray *dscalars = vtkDoubleArray::New();
        dscalars->SetNumberOfTuples(nels);
        scalars = dscalars;

        // Read the data into all_vars array.
        if(H5Dread(dataId, it->second.nativeVarType, H5S_ALL, spaceId,
                   H5P_DEFAULT, dscalars->GetVoidPointer(0)) < 0)
        {
            H5Sclose(spaceId);
            H5Dclose(dataId);
            EXCEPTION1(InvalidVariableException, varname);
        }
    }
    else
    {
        debug4 << mName << "The variable " << varname << " was in a "
               <<"native format that we're not supporting." << endl;
    }

    // Close the data space so we don't leak resources.
    H5Sclose(spaceId);
    H5Dclose(dataId);

    return scalars;
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
//   
// ****************************************************************************

herr_t
avtPixieFileFormat::GetVariableList(hid_t group, const char *name,
    void *op_data)
{
    hid_t                   obj;
    H5G_stat_t              statbuf;

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
    //
    H5Gget_objinfo(group, name, 0, &statbuf);

    //
    // Do something with the object based on its type.
    //
    switch (statbuf.type)
    {
    case H5G_DATASET:
        if ((obj = H5Dopen(group, name)) >= 0)
        {
            VarInfo varInfo;
            varInfo.fileVarName = varName;

            // Peel off the timestep prefix if there are multiple time states.
            if(info->This->nTimeStates > 0)
            {
                std::string::size_type index = varName.find("/", 1);
                if(index != -1)
                    varName = varName.substr(index+1);
            }

            // See if the variable's name contains any parenthesis. If so,
            // replace with square brackets.
            for(int i = 0; i < varName.size(); ++i)
            {
                if(varName[i] == '(')
                    varName[i]=  '[';
                else if(varName[i] == ')')
                    varName[i]=  ']';
            }

            // Get the variable's size.
            hid_t sid = H5Dget_space(obj);
            H5Sget_simple_extent_dims(sid, varInfo.dims, NULL);

            //
            // Determine the variable's type to see if we can support it.
            //
            hid_t t = H5Dget_type(obj);
            varInfo.nativeVarType = H5Tget_native_type(t, H5T_DIR_ASCEND);
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
                 debug4 << ", which it not supported at this time." << endl;
            }

            // Store information about the variable.
            if(supported)
                info->This->variables[varName] = varInfo;

            H5Sclose(sid);
            H5Dclose(obj);
        }
        else
        {
            debug4 << "unable to get dataset " << name << endl;
        }
        break;
    case H5G_GROUP:
        // We found a time state, increment the number of time states.
        if(info->level == 0 && varName.find("Timestep") != -1)
        {
            debug4 << "Added time state" << endl;
            ++info->This->nTimeStates;
        }

        if ((obj = H5Gopen(group, name)) >= 0)
        {
            // Iterate over the items in this group.
            TraversalInfo info2;
            info2.This = info->This;
            info2.level = info->level + 1;
            info2.path = varName;
            H5Giterate(obj, ".", NULL, GetVariableList, (void*)&info2);
            H5Gclose(obj);
        }
        else
        {
            debug4 << "unable to dump group " << varName.c_str() << endl;
        }
        break;
#if 0
    case H5G_TYPE:
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
