#include <avtADAPTFileFormat.h>
#include <NETCDFFileObject.h>
#include <netcdf.h>
#include <map>

#include <avtDatabaseMetaData.h>
#include <avtSTSDFileFormatInterface.h>

#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkShortArray.h>
#include <vtkIntArray.h>
#include <vtkLongArray.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>

#include <InvalidVariableException.h>
#include <ImproperUseException.h>
#include <Expression.h>

#include <visit-config.h>
#include <DebugStream.h>

// ****************************************************************************
// Method: avtADAPTFileFormat::Identify
//
// Purpose: 
//   Identifies the file as an ADAPT file.
//
// Arguments:
//   fileObject : The file that we'll check.
//
// Returns:    True if the file is an ADAPT file; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 16 10:42:48 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

bool
avtADAPTFileFormat::Identify(NETCDFFileObject *fileObject)
{
    bool     isADAPT = false;

    std::string create_code;
    if(fileObject->ReadStringAttribute("create_code", create_code))
    {
        isADAPT = create_code == "ADAPT" ||
                  create_code == "CsGribSupport" ||
                  create_code == "getGrid" ||
                  create_code == "matExtractor";
    }

    return isADAPT;
}

// ****************************************************************************
// Method: avtADAPTFileFormat::CreateInterface
//
// Purpose: 
//   Creates an STSD file format interface for the specified files.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 16 10:43:37 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

avtFileFormatInterface *
avtADAPTFileFormat::CreateInterface(NETCDFFileObject *f, 
    const char *const *list, int nList, int nBlock)
{
    avtSTSDFileFormat ***ffl = new avtSTSDFileFormat**[nList];
    int nTimestep = nList / nBlock;

    for (int i = 0 ; i < nTimestep ; i++)
    {
        ffl[i] = new avtSTSDFileFormat*[nBlock];
        for (int j = 0 ; j < nBlock ; j++)
        {
            if(f != 0)
            {
                ffl[i][j] = new avtADAPTFileFormat(list[i*nBlock + j], f);
                f = 0;
            }
            else
                ffl[i][j] = new avtADAPTFileFormat(list[i*nBlock + j]);
        }
    }

    return new avtSTSDFileFormatInterface(ffl, nTimestep, nBlock);
}

// ****************************************************************************
// Method: avtADAPTFileFormat::avtADAPTFileFormat
//
// Purpose: 
//   Constructor for the avtADAPTFileFormat class.
//
// Arguments:
//   filename : The name of the file to read.
//   f        : A pointer to an object that is already reading the file.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 16 10:44:01 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

avtADAPTFileFormat::avtADAPTFileFormat(const char *filename) : 
    avtSTSDFileFormat(filename)
{
    fileObject = new NETCDFFileObject(filename);
    meshFile = 0;
}

avtADAPTFileFormat::avtADAPTFileFormat(const char *filename,
    NETCDFFileObject *f) : avtSTSDFileFormat(filename)
{
    fileObject = f;
    meshFile = 0;
}

// ****************************************************************************
// Method: avtADAPTFileFormat::~avtADAPTFileFormat
//
// Purpose: 
//   Destructor for the avtADAPTFileFormat class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 16 10:44:46 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

avtADAPTFileFormat::~avtADAPTFileFormat()
{
    FreeUpResources();

    delete fileObject;
    delete meshFile;
}

// ****************************************************************************
// Method: avtADAPTFileFormat::ActivateTimestep
//
// Purpose: 
//   Activates the time step.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 16 10:45:09 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtADAPTFileFormat::ActivateTimestep()
{
    debug4 << "avtADAPTFileFormat::ActiveTimestep" << endl;
}

// ****************************************************************************
// Method: avtADAPTFileFormat::FreeUpResources
//
// Purpose: 
//   Frees up resources like memory and file descriptors.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 16 10:45:27 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtADAPTFileFormat::FreeUpResources()
{
    debug4 << "avtADAPTFileFormat::FreeUpResources" << endl;
    fileObject->Close();
    if(meshFile != 0)
        meshFile->Close();
}

// ****************************************************************************
// Method: avtADAPTFileFormat::GetGridFileName
//
// Purpose: 
//   Gets the name of the grid file.
//
// Returns:    The name of the grid file to open.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 16 10:45:48 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

std::string
avtADAPTFileFormat::GetGridFileName()
{
    std::string grid_file;
    if(fileObject->ReadStringAttribute("grid_file", grid_file))
    {
        // Get the filename portion of the grid_file string.
        std::string::size_type pos = grid_file.rfind("/");
        if(pos != std::string::npos)
            grid_file = grid_file.substr(pos + 1, grid_file.size() - pos - 1);

        // Get the path to the filename that we're opening.
        std::string path(filename);
        pos = path.rfind(SLASH_STRING);
        if(pos != std::string::npos)
        {
            path = path.substr(0, pos);
            if(path.size() > 0 && path[path.size()-1] != SLASH_CHAR)
                path += SLASH_STRING;
        }
        else
            path = "";

        grid_file = path + grid_file;
    }

    return grid_file;
}

// ****************************************************************************
// Method: avtADAPTFileObject::GetMeshFile
//
// Purpose: 
//   Returns a pointer to the mesh file and first creates a handle to if,
//   if needed.
//
// Returns:    A pointer to the mesh file object.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 16 10:46:28 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

NETCDFFileObject *
avtADAPTFileFormat::GetMeshFile()
{
    if(meshFile == 0)
        meshFile = new NETCDFFileObject(GetGridFileName().c_str());

    return meshFile;
}

// ****************************************************************************
// Method: avtADAPTFileFormat::PopulateDatabaseMetaData
//
// Purpose: 
//   Populates the metadata object with the mesh and variables present in
//   the database
//
// Arguments:
//   md : The database object to populate.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 16 10:47:31 PDT 2005
//
// Modifications:
//    Jeremy Meredith, Thu Aug 25 12:55:29 PDT 2005
//    Added group origin to mesh metadata constructor.
//   
// ****************************************************************************

void
avtADAPTFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    const char *mName = "avtADAPTFileFormat::PopulateDatabaseMetaData: ";
    debug4 << mName << endl;
    if(debug4_real)
        fileObject->PrintFileContents(debug4_real);

    // Assemble a database title.
    std::string comment(GetType()), titleString, create_version, 
                create_date_time;
    if(fileObject->ReadStringAttribute("title", titleString))
    {
        comment += (std::string(" database: title=") + titleString);

        if(fileObject->ReadStringAttribute("create_version", create_version))
            comment += (std::string(", create_version=") + create_version);

        if(fileObject->ReadStringAttribute("create_date_time", create_date_time))
            comment += (std::string(", create_date_time=") + create_date_time);

        md->SetDatabaseComment(comment);
    }

    //
    // Add the main mesh.
    //
    std::string meshName("main");
    avtMeshMetaData *mmd = new avtMeshMetaData(meshName, 
                            1, 1, 1, 0, 3, 3, AVT_CURVILINEAR_MESH);
    // Get the units for the main mesh from the grid file.
    std::string xUnits, xLabel, yUnits, yLabel;
    GetMeshFile()->ReadStringAttribute("x", "units", xUnits);
    GetMeshFile()->ReadStringAttribute("x", "long_name", xLabel);
    GetMeshFile()->ReadStringAttribute("y", "units", yUnits);
    GetMeshFile()->ReadStringAttribute("y", "long_name", yLabel);
    GetMeshFile()->ReadStringAttribute("vert", "long_name", mmd->zLabel);
    mmd->xUnits = xUnits;
    mmd->xLabel = xLabel;
    mmd->yUnits = yUnits;
    mmd->yLabel = yLabel;
    md->Add(mmd);

    // Get the number of nodes in each dimension so we can determine which
    // variables belong to the main mesh. We can also use it to determine
    // node/zone centering.
    size_t sizes[3];
    bool haveAllSizes = true;
    int i;
    debug4 << mName << " meshSize={";
    for(i = 0; i < 3; ++i)
    {
        int status = nc_inq_dimlen(fileObject->GetFileHandle(), i, &sizes[i]);
        if(status != NC_NOERR)
        {
            haveAllSizes = false;
            fileObject->HandleError(status);
        }
        debug4 << ", " << sizes[i];
    }
    debug4 << "}" << endl;

    //
    // If we have the mesh sizes then iterate over the variables in the file
    // and add those that have the same dimensions as the "main" mesh.
    //
    if(haveAllSizes)
    {
        int status, nDims, nVars, nGlobalAtts, unlimitedDimension;
        status = nc_inq(fileObject->GetFileHandle(), &nDims, &nVars, &nGlobalAtts,
                        &unlimitedDimension);
        if(status != NC_NOERR)
        {
            fileObject->HandleError(status);
            return;
        }

        // Get the sizes of all dimensions.
        size_t *dimSizes = new size_t[nDims];
        for(i = 0; i < nDims; ++i)
        {
            int status = nc_inq_dimlen(fileObject->GetFileHandle(), i, &dimSizes[i]);
            if(status != NC_NOERR)
                fileObject->HandleError(status);
        }

        bool addedMain2d = false;
        std::map<std::string, bool> componentExists;
        for(i = 0; i < nVars; ++i)
        {
            char varname[NC_MAX_NAME+1];
            nc_type vartype;
            int  varndims;
            int  vardims[NC_MAX_VAR_DIMS];
            int  varnatts;
            if((status = nc_inq_var(fileObject->GetFileHandle(), i, varname,
                                    &vartype, &varndims, 
                                    vardims, &varnatts)) == NC_NOERR)
            {
                debug4 << mName << varname << " size={";
                for(int j = 0; j < varndims; ++j)
                {
                    if(j > 0)
                       debug4 << ", ";
                    debug4 << dimSizes[vardims[j]];
                }
                debug4 << "}\n";

                if(varndims == 2)
                {
                    avtCentering centering = AVT_ZONECENT;
                    if((sizes[0] - dimSizes[vardims[1]]) == 0 &&
                       (sizes[1] - dimSizes[vardims[0]]) == 0)
                    {
                        centering = AVT_NODECENT;
                    }

                    avtScalarMetaData *smd = new avtScalarMetaData(varname,
                        "main2d", centering);
                    smd->hasUnits = fileObject->ReadStringAttribute(
                        varname, "units", smd->units);
                    md->Add(smd);
                    addedMain2d = true;
                }
                else if(varndims == 3)
                {
                    componentExists[varname] = true;

                    avtCentering centering = AVT_ZONECENT;
                    if((sizes[0] - dimSizes[vardims[2]]) == 0 &&
                       (sizes[1] - dimSizes[vardims[1]]) == 0 &&
                       (sizes[2] - dimSizes[vardims[0]]) == 0)
                    {
                        centering = AVT_NODECENT;
                    }

                    avtScalarMetaData *smd = new avtScalarMetaData(varname,
                        meshName, centering);
                    smd->hasUnits = fileObject->ReadStringAttribute(
                        varname, "units", smd->units);
                    md->Add(smd);
                }
                else
                {
                    debug4 << mName << varname << " will not be plottable "
                           << "because it is not defined on the \"main\" mesh."
                           << endl;
                }
            }
            else
                fileObject->HandleError(status);
        }

        delete [] dimSizes;

        // If we referenced a 2D mesh then add it.
        if(addedMain2d)
        {
            mmd = new avtMeshMetaData("main2d",
                1, 1, 1, 0, 2, 2, AVT_RECTILINEAR_MESH);
            mmd->xUnits = xUnits;
            mmd->xLabel = xLabel;
            mmd->yUnits = yUnits;
            mmd->yLabel = yLabel;
            md->Add(mmd);
        }

        // Check to see if certain variables exist. If they do then we can
        // add expressions.
        if(componentExists.find("u") != componentExists.end() &&
           componentExists.find("v") != componentExists.end() &&
           componentExists.find("w") != componentExists.end())
        {
            Expression *e = new Expression;
            e->SetName("wind");
            e->SetDefinition("{u,v,w}");
            e->SetType(Expression::VectorMeshVar);
            md->AddExpression(e);
        }

        if(componentExists.find("u_int") != componentExists.end() &&
           componentExists.find("v_int") != componentExists.end() &&
           componentExists.find("w_int") != componentExists.end())
        {
            Expression *e = new Expression;
            e->SetName("interpolated_wind");
            e->SetDefinition("{u_int,v_int,w_int}");
            e->SetType(Expression::VectorMeshVar);
            md->AddExpression(e);
        }
    }
}

// ****************************************************************************
// Method: avtADAPTFileFormat::GetMesh
//
// Purpose: 
//   Returns a dataset containing the specified mesh.
//
// Arguments:
//   var : The name of the mesh to be returned.
//
// Returns:    A pointer to the mesh.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 16 10:48:25 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
avtADAPTFileFormat::GetMesh(const char *var)
{
    const char *mName = "avtADAPTFileFormat::GetMesh: ";
    debug4 << mName << "var=" << var << endl;
    vtkDataSet *retval = 0;

    bool isMain = strcmp(var, "main") == 0 ;
    bool isMain2d = strcmp(var, "main2d") == 0 ;
    if(isMain || isMain2d)
    {
        TypeEnum xt = NO_TYPE;
        int xndims = 0, *xdims = 0;
        float *xc = 0;
        if(GetMeshFile()->InqVariable("x", &xt, &xndims, &xdims))
        {
            xc = new float[xdims[0]];
            if(!GetMeshFile()->ReadVariableIntoAsFloat("x", xc))
            {
                debug4 << mName << "Could not read x coordinate from the "
                       << "mesh file.\n";
                delete [] xc;
                xc = 0;
            }
        }

        TypeEnum yt = NO_TYPE;
        int yndims = 0, *ydims = 0;
        float *yc = 0;
        if(GetMeshFile()->InqVariable("y", &yt, &yndims, &ydims))
        {
            yc = new float[ydims[0]];
            if(!GetMeshFile()->ReadVariableIntoAsFloat("y", yc))
            {
                debug4 << mName << "Could not read y coordinate from the "
                       << "mesh file.\n";
                delete [] yc;
                yc = 0;
            }
        }

        if(xc != 0 && yc != 0)
        {
            if(isMain2d)
                retval = CreateMain2D(xc, xdims[0], yc, ydims[0]);
            else
                retval = CreateMain(xc, xdims[0], yc, ydims[0]);
        }

        // Free the coordinates.
        delete [] xc;
        delete [] yc;

        // Free the dims
        delete [] xdims;
        delete [] ydims;
    }
    else
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    return retval;
}

// ****************************************************************************
// Method: avtADAPTFileFormat::CreateMain2D
//
// Purpose: 
//   Creates the main 2D mesh
//
// Arguments:
//   xc : The x coordinates to use.
//   nx : The number of x coordinates.
//   yc : The y coordinates to use.
//   ny : The number of y coordinates.
//
// Returns:    The main2d mesh or 0.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 17 17:59:18 PST 2005
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
avtADAPTFileFormat::CreateMain2D(const float *xc, int nx, const float *yc,
    int ny)
{
    const char *mName = "avtADAPTFileFormat::CreateMain2D";

    //
    // Populate the coordinates.
    //
    int    dims[3];
    dims[0] = nx;
    dims[1] = ny;
    dims[2] = 1;
    const float *coordinate[3];
    coordinate[0] = xc;
    coordinate[1] = yc;
    coordinate[2] = 0;
    const char *coordLabels[3] = {"X coords", "Y coords", "Z coords"};
    vtkFloatArray *coords[3];
    for (int i = 0 ; i < 3 ; i++)
    {
        // Default number of components for an array is 1.
        coords[i] = vtkFloatArray::New();
        coords[i]->SetNumberOfTuples(dims[i]);

        if(i < 2)
        {
            debug4 << mName << coordLabels[i] << " size=" << dims[i]
                   << ", values = {";
            for (int j = 0 ; j < dims[i] ; j++)
            {
                coords[i]->SetComponent(j, 0, coordinate[i][j]);
                debug4 << ", " << coordinate[i][j];
            }
            debug4 << "}" << endl;
        }
        else
            coords[i]->SetComponent(0, 0, 0.);
    }

    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New(); 
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
// Method: avtADAPTFileFormat::CreateMain
//
// Purpose: 
//   Creates the main mesh, which is a curvilinear mesh that is displaced in
//   Z by vert and zSfc.
//
// Arguments:
//   xc : The x coordinates to use.
//   nx : The number of x coordinates.
//   yc : The y coordinates to use.
//   ny : The number of y coordinates.
//
// Returns:    The main mesh or 0.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 17 17:59:18 PST 2005
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
avtADAPTFileFormat::CreateMain(const float *xc, int nx, const float *yc,
    int ny)
{
    const char *mName = "avtADAPTFileFormat::CreateMain: ";
    vtkDataSet *retval = 0;

    debug4 << mName << endl;

    // See if any transformations need to be done to the vert values before 
    // using them as Z values.
    std::string pvc;
    if(!GetMeshFile()->ReadStringAttribute(
       "principal_vertical_coordinate", pvc))
    {
        debug4 << mName << "Could not read principal_vertical_coordinate "
               << "attribute" << endl;
        return retval;
    }
    else
        debug4 << mName << "PVC=" << pvc.c_str() << endl;

    // Determine which arrays need to be read in for the height.
    bool needzSfc = false;
    bool needZ = false;
    if(pvc == "sigmaZ")
        needzSfc = true;
    else if(pvc == "zAGL")
    {
        needzSfc = true;
        needZ = true;
    }
    else if(pvc == "zMSL")
    {
        needzSfc = true;
        needZ = true;
    }

    // Read the arrays that we need to create the height.
    bool havezSfc = false;
    TypeEnum zg = NO_TYPE;
    int zSfcNdims, *zSfcDims = 0;
    void *zSfc = 0;
    if(needzSfc)
    {
        havezSfc = GetMeshFile()->ReadVariable("zSfc", &zg, &zSfcNdims, 
            &zSfcDims, &zSfc);

        // Make sure that zSfc has the expected characteristics.
        bool xDimsEqual = nx == zSfcDims[0];
        bool yDimsEqual = ny == zSfcDims[1];
        bool err = false;
        if(!xDimsEqual)
        {
            debug4 << mName << "nx=" << nx
                   << ", zSfcDims[0]="
                   << zSfcDims[0]
                   << " are not the same!" << endl;
            err = true;
        }
        if(!yDimsEqual)
        {
            debug4 << mName << "ny=" << ny
                   << ", zSfcDims[1]="
                   << zSfcDims[1]
                   << " are not the same!" << endl;
            err = true;
        }
        if(zSfcNdims != 2)
        {
            debug4 << mName << "zSfc did not have 2 dimensions." << endl;
            err = true;
        }
        if(havezSfc && err)
        {
            debug4 << mName << "zSfc did not have the expected "
                   << "characteristics." << endl;
            free_void_mem(zSfc, zg);
            zSfc = 0;
            havezSfc = false;
        }
    }

    bool haveZ = false;
    TypeEnum zt = NO_TYPE;
    int zNdims, *zDims = 0;
    void *z = 0;
    if(needZ)
    {
        haveZ = GetMeshFile()->ReadVariable("z", &zt, &zNdims, &zDims, &z);
        if(!haveZ)
        {
            debug4 << mName << "z could not be read." << endl;
        }
    }

    //
    // Create the mesh differently based on the type of 
    // principal_vertical_coordinate.
    //
    bool unsupportedPVC = false;
    if(pvc == "sigmaZ")
    {
        TypeEnum ztt = NO_TYPE;
        int zTopNdims, *zTopDims = 0;
        void *zTop = 0;
        bool haveZTop = GetMeshFile()->ReadVariable("zTop",
            &ztt, &zTopNdims, &zTopDims, &zTop);

        TypeEnum vertt = NO_TYPE;
        int vertndims = 0, *vertdims = 0;
        float *vertc = 0;
        if(GetMeshFile()->InqVariable("vert", &vertt, &vertndims, &vertdims))
        {
            vertc = new float[vertdims[0]];
            if(!GetMeshFile()->ReadVariableIntoAsFloat("vert", vertc))
            {
                debug4 << mName << "Could not read vert coordinate from the "
                       << "mesh file.\n";
            }
        }

        if(haveZTop && havezSfc && vertc != 0 &&
           ztt == DOUBLEARRAY_TYPE &&
           zg == DOUBLEARRAY_TYPE)
        {
            double ZT =  ((double*)zTop)[0];
            double *ZSFC = (double*)zSfc;

            debug4 << mName << "Applying sigmaZ" << endl;

            // Create the curvilinear mesh.
            vtkStructuredGrid *sgrid   = vtkStructuredGrid::New(); 
            vtkPoints         *points  = vtkPoints::New();
            sgrid->SetPoints(points);
            points->Delete();

            // Populate the points array
            int dims[3];
            dims[0] = nx;
            dims[1] = ny;
            dims[2] = vertdims[0];
            sgrid->SetDimensions(dims);
            points->SetNumberOfPoints(dims[0] * dims[1] * dims[2]);
            float *pts = (float *) points->GetVoidPointer(0);
            for(int k = 0; k < vertdims[0]; ++k)
            {
                for(int j = 0; j < ny; ++j)
                {
                    int index = nx * j;
                    for(int i = 0; i < nx; ++i, ++index)
                    {
                        *pts++ = xc[i];
                        *pts++ = yc[j];
                        *pts++ = ZSFC[index] + (ZT - ZSFC[index]) * vertc[k];
                    }
                }
            }

            retval = sgrid;
        }

        delete [] vertc;
        delete [] vertdims;
        if(zTop != 0)
            free_void_mem(zTop, ztt);
        delete [] zTopDims;
    }
    else if(pvc == "zMSL")
    {
        if(haveZ)
        {
            debug4 << mName << "Applying zMSL" << endl;

            // Create the curvilinear mesh.
            vtkStructuredGrid *sgrid  = vtkStructuredGrid::New(); 
            vtkPoints         *points = vtkPoints::New();
            sgrid->SetPoints(points);
            points->Delete();

            // Populate the points array
            int dims[3];
            dims[0] = nx;
            dims[1] = ny;
            dims[2] = zDims[0];
            sgrid->SetDimensions(dims);
            points->SetNumberOfPoints(dims[0] * dims[1] * dims[2]);
            float *pts = (float *) points->GetVoidPointer(0);
            double *Z = (double *)z;
            for(int k = 0; k < dims[2]; ++k)
            {
                for(int j = 0; j < ny; ++j)
                {
                    float y = yc[j];
                    for(int i = 0; i < nx; ++i)
                    {
                        *pts++ = xc[i];
                        *pts++ = yc[j];
                        *pts++ = float(*Z++);
                    }
                }
            }

            retval = sgrid;
        }
    }
    else if(pvc == "zAGL")
    {
        if(haveZ && havezSfc)
        {
            debug4 << mName << "Applying zAGL" << endl;

            // Create the curvilinear mesh.
            vtkStructuredGrid *sgrid  = vtkStructuredGrid::New(); 
            vtkPoints         *points = vtkPoints::New();
            sgrid->SetPoints(points);
            points->Delete();

            // Populate the points array
            int dims[3];
            dims[0] = nx;
            dims[1] = ny;
            dims[2] = zDims[0];
            sgrid->SetDimensions(dims);
            points->SetNumberOfPoints(dims[0] * dims[1] * dims[2]);
            float *pts = (float *) points->GetVoidPointer(0);
            double *Z = (double *)z;
            double *ZSFC = (double*)zSfc;
            for(int k = 0; k < dims[2]; ++k)
            {
                for(int j = 0; j < ny; ++j)
                {
                    float y = yc[j];
                    int index = nx * j;
                    for(int i = 0; i < nx; ++i, ++index)
                    {
                        *pts++ = xc[i];
                        *pts++ = yc[j];
                        float zval = float(*Z++);
                        *pts++ = zval - ZSFC[index];
                    }
                }
            }

            retval = sgrid;
        }
    }
    else
        unsupportedPVC = true; 
 
    //
    // Clean up.
    //
    if(havezSfc)
    {
        free_void_mem(zSfc, zg);
        delete [] zSfcDims;
    }
    if(haveZ)
    {
        free_void_mem(z, zt);
        delete [] zDims;
    }

    if(unsupportedPVC)
    {
        std::string msg("Unsupported principal vertical coordinate: ");
        msg += pvc;
        EXCEPTION1(ImproperUseException, msg);
    }

    return retval;
}

// ****************************************************************************
// Method: avtADAPTFileFormat::GetVar
//
// Purpose: 
//   Reads in the named variable and returns it as a VTK data array.
//
// Arguments:
//   var : The name of the variable to read.
//
// Returns:    A pointer to the data array.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 16 10:49:02 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

#define READVAR(VTKTYPE) \
        {\
            VTKTYPE *arr = VTKTYPE::New();\
            arr->SetNumberOfComponents(1);\
            arr->SetNumberOfTuples(nElems);\
            debug4 << "Allocated a " << \
                    #VTKTYPE \
                   << " of " << nElems << " elements" << endl; \
            if(fileObject->ReadVariableInto(var, t, arr->GetVoidPointer(0)))\
                retval = arr;\
            else\
                arr->Delete();\
        }

vtkDataArray *
avtADAPTFileFormat::GetVar(const char *var)
{
    debug4 << "avtADAPTFileFormat::GetVar: var=" << var << endl;

    vtkDataArray *retval = 0;
    TypeEnum t = NO_TYPE;
    int ndims = 0;
    int *dims = 0;
   
    if(fileObject->InqVariable(var, &t, &ndims, &dims))
    {
        debug4 << "avtADAPTFileFormat::GetVar: var=" << var << ", dims={";
        int nElems = 1;
        for(int i = 0; i < ndims; ++i)
        {
            debug4 << ", " << dims[i];
            nElems *= dims[i];
        }
        debug4 << "}" << endl;

        if(t == CHARARRAY_TYPE || t == UCHARARRAY_TYPE)
            READVAR(vtkUnsignedCharArray)
        else if(t == SHORTARRAY_TYPE)
            READVAR(vtkShortArray)
        else if(t == INTEGERARRAY_TYPE)
            READVAR(vtkIntArray)
        else if(t == LONGARRAY_TYPE)
            READVAR(vtkLongArray)
        else if(t == FLOATARRAY_TYPE)
            READVAR(vtkFloatArray)
        else if(t == DOUBLEARRAY_TYPE)
            READVAR(vtkDoubleArray)

        delete [] dims;
    }
    else
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    return retval;
}

