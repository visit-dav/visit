// ************************************************************************* //
//                            avtZeusMPFileFormat.C                          //
// ************************************************************************* //

#include <avtZeusMPFileFormat.h>

#include <string>
#include <snprintf.h>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>

#include <avtDatabaseMetaData.h>

#include <Expression.h>
#include <DebugStream.h>

#include <InvalidFilesException.h>
#include <InvalidVariableException.h>

#include <hdf.h>
#include <mfhdf.h>

using     std::string;

#define MESH_COORDSYS_CARTESIAN        0
#define MESH_COORDSYS_SPHERICAL_POLAR  1

// Prototypes
static vtkDataArray *ReadVar(int32, int32, const char *);


// ****************************************************************************
//  Method: avtZeusMPFileFormat constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jan 13 10:36:24 PDT 2006
//
//  Modifications:
//
// ****************************************************************************

avtZeusMPFileFormat::avtZeusMPFileFormat(const char *filename)
    : avtSTSDFileFormat(filename), varsToDatasets()
{
    hdffile = -1;
    haveFileInformation = false;
    fileTime = 0.;
    haveFileTime = false;
    coords[0] = 0;
    coords[1] = 0;
    coords[2] = 0;
    meshCoordinateSystem = MESH_COORDSYS_CARTESIAN;
}

// ****************************************************************************
// Method: avtZeusMPFileFormat destructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 13 12:15:26 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

avtZeusMPFileFormat::~avtZeusMPFileFormat()
{
    FreeUpResources();
}

// ****************************************************************************
//  Method: avtZeusMPFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jan 13 10:36:24 PDT 2006
//
//  Modifications:
//
// ****************************************************************************

void
avtZeusMPFileFormat::FreeUpResources(void)
{
    debug4 << "avtZeusMPFileFormat::FreeUpResources" << endl;
    if(hdffile != -1)
    {
        SDend(hdffile);
        hdffile = -1;
    }

    for(int i = 0; i < 3; ++i)
    {
        if(coords[i] != 0)
        {
            coords[i]->Delete();
            coords[i] = 0;
        }
    }

    varsToDatasets.clear();
    haveFileTime = false;
    haveFileInformation = false;
}

// ****************************************************************************
// Method: avtZeusMPFileFormat::GetFileHandle
//
// Purpose: 
//   Opens the HDF file and returns a file handle to it.
//
// Returns:    A file handle.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 13 12:16:02 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

int32
avtZeusMPFileFormat::GetFileHandle()
{
    if(hdffile == -1)
    {
        hdffile = SDstart(GetFilename(), DFACC_READ);
        debug4 << "SDstart(" << GetFilename() << ") returned " << hdffile << endl;
        if(hdffile < 0)
        {
            EXCEPTION1(InvalidFilesException, GetFilename());
        }
    }

    return hdffile; 
}

// ****************************************************************************
// Method: avtZeusMPFileFormat::GetFileInformation
//
// Purpose: 
//   This method reads the file information and populates the coordinates and
//   variable to dataset map.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 13 12:16:28 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtZeusMPFileFormat::GetFileInformation()
{
    const char *mName = "avtZeusMPFileFormat::GetFileInformation: ";

    if(haveFileInformation)
        return;

    int32 file_handle = GetFileHandle();

    int32 n_datasets = 0;
    int32 n_file_attrs = 0;
    if(SDfileinfo(file_handle, &n_datasets, &n_file_attrs) == FAIL)
    {
        debug4 << mName << "SDfileinfo failed!" << endl;
        EXCEPTION1(InvalidFilesException, GetFilename());
    }

    int fakeCount = 0;

    int var;
    stringVector varNames;
    for (var = 0 ; var < n_datasets ; var++)
    {
        int32 data_type;
        char  dsname[65];
        int32 nattrs, tmp_ndims;
        DatasetInfo info;
        info.index = var;
        int32 var_handle = SDselect(file_handle, var);
        SDgetinfo(var_handle, dsname, &tmp_ndims, info.dims, &data_type, &nattrs);
        SDendaccess(var_handle);

        debug4 << "Var: " << dsname << " dims[] = {";
        info.ndims = 0;
        for(int d = 0; d < tmp_ndims; ++d)
        {
            debug4 << info.dims[d] << ", ";
            if(info.dims[d] > 1)
                ++info.ndims;
        }
        debug4 << "}" << endl;

        if (info.ndims >= 2)
        {
            int32 sds_id = SDselect(file_handle, var);
            if(sds_id == FAIL)
                debug4 << "SDselect failed for " << dsname << endl;
 
            // Read the long_name attribute to see what we should call the variable.
            int32 attr_index = SDfindattr(sds_id, "long_name");
            if (attr_index != FAIL)
            {
                // Read the attribute.
                char long_name[100];
                memset(long_name, 0, 100);
                int32 status = SDreadattr(sds_id, attr_index, long_name);
                if (status == FAIL)
                    varsToDatasets[dsname] = info;
                else
                {
                    char *s = strstr(long_name, " AT TIME=");
                    if(s != NULL)
                    {
                        if(haveFileTime)
                        {
                            double tmp;
                            if(sscanf(long_name+9, "%g", &tmp) == 1)
                            {
                                fileTime = tmp;
                                haveFileTime = true;
                            }
                        }

                        *s = '\0';
                    }
                    debug4 << "Adding dataset " << dsname << " as "
                           << long_name << endl;
                    varsToDatasets[long_name] = info;
                }
            }
            else
            {
                debug4 << "Could not get attr_index" << endl;
                varsToDatasets[dsname] = info;
            }

            // Read the coordsys attribute to see what we coordinate system we
            // should use.
            attr_index = SDfindattr(sds_id, "coordsys");
            meshCoordinateSystem = MESH_COORDSYS_CARTESIAN;
            if (attr_index != FAIL)
            {
                // Read the attribute.
                char coordsys[100];
                memset(coordsys, 0, 100);
                int32 status = SDreadattr(sds_id, attr_index, coordsys);
                if (status != FAIL)
                {
                    debug4 << "coordsys attribute = \"" << coordsys << "\"\n";
                    if(strcmp(coordsys, "spherical polar") == 0)
                        meshCoordinateSystem = MESH_COORDSYS_SPHERICAL_POLAR;
                }
                else 
                    debug4 << "coordsys attribute could not be read." << endl;
            }
            else 
                debug4 << "coordsys attribute not found on " << dsname << endl;
        }
        else if(fakeCount < 3)
        {
            // It's a fakeDim# dataset. Maybe it contains the coordinates.
            // print it out.
            TRY
            {
                vtkDataArray *arr = ReadVar(file_handle, var, dsname);
                coords[fakeCount++] = arr;
                debug4 << dsname << " = {";
                for(int i = 0; i < arr->GetNumberOfTuples(); ++i)
                {
                    float fval = arr->GetTuple1(i);
                    debug4 << fval << ", ";
                }
                debug4 << "}" << endl;
            }
            CATCH(VisItException)
            {
                debug4 << "Could not read " << dsname << endl;
            }
            ENDTRY
        }
    }

    haveFileInformation = true;
}

// ****************************************************************************
// Method: avtZeusMPFileFormat::GetTime
//
// Purpose: 
//   Returns the time for the dataset.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 13 12:17:07 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

double
avtZeusMPFileFormat::GetTime(void)
{
    debug4 << "avtZeusMPFileFormat::GetTime: "
           << (haveFileTime ? fileTime : INVALID_TIME) << endl;

    return haveFileTime ? fileTime : INVALID_TIME;
}

// ****************************************************************************
// Method: avtZeusMPFileFormat::ReturnsValidTime
//
// Purpose: 
//   Indicates whether the dataset returns a valid time.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 13 12:17:29 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
avtZeusMPFileFormat::ReturnsValidTime() const
{
    debug4 << "avtZeusMPFileFormat::ReturnsValidTime: " << (haveFileTime?"true":"false") << endl;
    return haveFileTime;
}

// ****************************************************************************
// Method: avtZeusMPFileFormat::ActivateTimestep
//
// Purpose: 
//   Activates the current time step.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 13 12:17:46 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtZeusMPFileFormat::ActivateTimestep(void)
{
    debug4 << "avtZeusMPFileFormat::ActivateTimestep: " << endl;
#ifndef MDSERVER
    GetFileInformation();
#endif
}

// ****************************************************************************
//  Method: avtZeusMPFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jan 13 10:36:24 PDT 2006
//
//  Modifications:
//
// ****************************************************************************

void
avtZeusMPFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    const char *mName = "avtZeusMPFileFormat::PopulateDatabaseMetaData: ";

#ifdef MDSERVER
    GetFileInformation();
#endif

    // Calculate the number of dimensions using the first variable that
    // we want to plot from the file.
    int ndims = 2;
    DatasetInfoMap::const_iterator pos = varsToDatasets.begin();
    if(pos != varsToDatasets.end())
         ndims = pos->second.ndims;

    debug1 << mName << "We think there are " << ndims << " dimensions" << endl;

    //
    // Add the mesh. Some of this information is subject to change.
    // For example:
    //   1. The mesh can be in cylindrical or spherical coordinates. If that's
    //      true then we'll have to create a curvilinear mesh and convert
    //      its coordinates to Cartesian.
    //
    avtMeshMetaData *mmd = new avtMeshMetaData;
    mmd->name = "Mesh";
    if(meshCoordinateSystem == MESH_COORDSYS_SPHERICAL_POLAR)
        mmd->meshType = AVT_CURVILINEAR_MESH;
    else
        mmd->meshType = AVT_RECTILINEAR_MESH;
    mmd->numBlocks = 1;
    mmd->blockOrigin = 1;
    mmd->cellOrigin = 1;
    mmd->spatialDimension = ndims;
    mmd->topologicalDimension = ndims;
    mmd->hasSpatialExtents = false;
    mmd->blockTitle = "tiles";
    mmd->blockPieceName = "tile";
    md->Add(mmd);

    //
    // Add scalars
    //
    for(pos = varsToDatasets.begin(); pos != varsToDatasets.end(); ++pos)
    {
        AddScalarVarToMetaData(md, pos->first, "Mesh", AVT_ZONECENT);
    }

    //
    // Try and create some vector expressions from certain variables.
    //
    std::map<std::string, stringVector> vectorCandidates;
    for(pos = varsToDatasets.begin(); pos != varsToDatasets.end(); ++pos)
    {
        if(pos->first[1] == '-')
        {
            std::string key(pos->first.substr(2, pos->first.size()-2));
            if(vectorCandidates.find(key) == vectorCandidates.end())
            {
                stringVector tmp;
                tmp.push_back(pos->first);
                vectorCandidates[key] = tmp;
            }
            else
                vectorCandidates[key].push_back(pos->first);
        }
    }
    for(std::map<std::string, stringVector>::const_iterator vpos = vectorCandidates.begin();
        vpos != vectorCandidates.end(); ++vpos)
    {
        if(vpos->second.size() >= ndims)
        {
            // Create the expression definition.
            char def[1024];
            Expression *e = new Expression;
            if(ndims == 2)
            {
                SNPRINTF(def, 1024, "{<%s>, <%s>}", 
                    vpos->second[0].c_str(),
                    vpos->second[1].c_str());
            }
            else if(ndims == 3)
            {
                SNPRINTF(def, 1024, "{<%s>, <%s>, <%s>}", 
                    vpos->second[0].c_str(),
                    vpos->second[1].c_str(),
                    vpos->second[2].c_str());
            }

            // Filter out invalid characters from the expression name.
            std::string vecName(vpos->first);
            std::string::size_type index;
            while((index = vecName.find(" ")) != std::string::npos)
                vecName[index] = '_';
 
            e->SetName(vecName);
            e->SetDefinition(def);
            e->SetType(Expression::VectorMeshVar);
            md->AddExpression(e);
        }
    }
}


// ****************************************************************************
//  Method: avtZeusMPFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jan 13 10:36:24 PDT 2006
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtZeusMPFileFormat::GetMesh(const char *meshname)
{
    vtkDataSet *retval = 0;
#ifndef MDSERVER
    // The coords[] array contains the coordinates in z,y,x order because
    // that's the order in which they appear in the file. The data arrays 
    // that we've read contain the same number of elements as the number of
    // cells in each direction. Let's assume that means the cell centers in 
    // each direction were stored. Let's compute node coordinates using those
    // values to give us arrays that are 1 element longer.
    int i;
    int dims[] = {1,1,1};
    int ndims = 0;
    vtkFloatArray *newcoords[3] = {0,0,0};
    for (i = 0 ; i < 3 ; i++)
    {
        vtkFloatArray *f = vtkFloatArray::New();
        newcoords[2-i] = f;

        if(coords[i]->GetNumberOfTuples() == 1)
        {
            f->SetNumberOfTuples(1);
            f->SetComponent(0, 0, 0.);
            dims[2-i] = 1;
        }
        else
        {
            ++ndims;
            dims[2-i] = coords[i]->GetNumberOfTuples()+1;
            int nnodes = coords[i]->GetNumberOfTuples() + 1;
            f->SetNumberOfTuples(nnodes);

            for (int j = 0 ; j < nnodes ; j++)
            {
                float v;
                if(j == 0)
                {
                    float V0 = coords[i]->GetTuple1(0);
                    float V1 = coords[i]->GetTuple1(1);
                    v = V0 - (V1 - V0) / 2.;
                }
                else if(j == nnodes-1)
                {
                    float Vjm1 = coords[i]->GetTuple1(j-1);
                    float Vjm2 = coords[i]->GetTuple1(j-2);
                    v = Vjm1 + (Vjm1 - Vjm2) / 2.;
                }
                else
                {
                    float Vjm1 = coords[i]->GetTuple1(j-1);
                    float Vj   = coords[i]->GetTuple1(j);
                    v = (Vjm1 + Vj) / 2.;
                }
                f->SetComponent(j, 0, v);
            }
        }
    }

    if(meshCoordinateSystem == MESH_COORDSYS_SPHERICAL_POLAR)
    {
        int nnodes = dims[0] * dims[1] * dims[2];

        vtkStructuredGrid    *sgrid   = vtkStructuredGrid::New(); 
        vtkPoints            *points  = vtkPoints::New();
        sgrid->SetPoints(points);
        points->Delete();
        sgrid->SetDimensions(dims);
        points->SetNumberOfPoints(nnodes);
        float *pts = (float *) points->GetVoidPointer(0);

        int nx = newcoords[0]->GetNumberOfTuples();
        int ny = newcoords[1]->GetNumberOfTuples();
        int nz = newcoords[2]->GetNumberOfTuples();
        int nyz = ny * nz;
        const float *coord0 = (const float *)newcoords[0]->GetVoidPointer(0);
        const float *coord1 = (const float *)newcoords[1]->GetVoidPointer(0);
        const float *coord2 = (const float *)newcoords[2]->GetVoidPointer(0);
    
        if(ndims == 2)
        {
            // polar
            for (int j = 0; j < ny; j++)
            {
                for (int i = 0; i < nx; i++)
                {
                    float rad = coord0[i];
                    float angle = coord1[j];

                    *pts++ = rad * cos(angle);
                    *pts++ = rad * sin(angle);
                    *pts++ = 0.;
                }
            }
        }
        else
        {
            // spherical coordinates
            for (int k = 0; k < nz; k++)
            {
                for (int j = 0; j < ny; j++)
                {
                    for (int i = 0; i < nx; i++)
                    {
                        // Kind of guessing until this is tested.
                        float rad   = coord0[i];
                        float theta = coord1[j];
                        float phi   = coord2[k];

                        *pts++ = rad * cos(theta) * sin(phi);
                        *pts++ = rad * sin(theta) * sin(phi);
                        *pts++ = rad * cos(phi);
                    }
                }
            }            
        }

        retval = sgrid;
    }
    else
    {
        vtkRectilinearGrid  *rGrid = vtkRectilinearGrid::New(); 
        rGrid->SetDimensions(dims);
        rGrid->SetXCoordinates(newcoords[0]);
        newcoords[0]->Delete();
        rGrid->SetYCoordinates(newcoords[1]);
        newcoords[1]->Delete();
        rGrid->SetZCoordinates(newcoords[2]);
        newcoords[2]->Delete();
        retval = rGrid;
    }
#endif

    return retval;
}

// ****************************************************************************
//  Method: avtZeusMPFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jan 13 10:36:24 PDT 2006
//
// ****************************************************************************

vtkDataArray *
avtZeusMPFileFormat::GetVar(const char *varname)
{
#ifndef MDSERVER
    int file_handle = GetFileHandle();

    DatasetInfoMap::const_iterator pos = varsToDatasets.find(varname);
    if(pos == varsToDatasets.end())
        EXCEPTION1(InvalidVariableException, varname);

    return ReadVar(file_handle, pos->second.index, varname);
#else
    return 0;
#endif
}


// ****************************************************************************
//  Method: avtZeusMPFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jan 13 10:36:24 PDT 2006
//
// ****************************************************************************

vtkDataArray *
avtZeusMPFileFormat::GetVectorVar(const char *varname)
{
    return 0;
}

// ****************************************************************************
// Method: ReadVar
//
// Purpose: 
//   This is a static function that reads an array from the HDF file and 
//   returns it in a vtkDataArray.
//
// Arguments:
//   file_handle : The handle to the open file.
//   dsindex     : The index of the dataset to read.
//   varname     : The name of the variable that we've associated with the
//                 dataset.
//
// Returns:    A data array containing the data.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 13 12:11:34 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

static vtkDataArray *
ReadVar(int32 file_handle, int32 dsindex, const char *varname)
{
    int32 var_handle = SDselect(file_handle, dsindex);
    if(var_handle == FAIL)
    {
        debug4 << "SDselect failed for " << varname << endl;
        EXCEPTION1(InvalidVariableException, varname);
    }

    // Get the associated data type.
    int32 ndims;
    int32 dims[3] = {1,1,1};
    int32 data_type;
    char  name[65];
    int32 nattrs;
    SDgetinfo(var_handle, name, &ndims, dims, &data_type, &nattrs);

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

    return fa;
}
