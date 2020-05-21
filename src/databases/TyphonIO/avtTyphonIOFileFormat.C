// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//                            avtTyphonIOFileFormat.C
// ****************************************************************************

#include <avtTyphonIOFileFormat.h>

#include <string>

#include <vtkCellTypes.h>
#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>
#include <avtMaterial.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>

#include <DebugStream.h>

#include <InvalidDBTypeException.h>
#include <InvalidVariableException.h>
#include <TyphonIOException.h>


using     std::string;

// ****************************************************************************
//  Method: avtTyphonIOFileFormat constructor
//
//  Arguments:
//    filename  The file name
//
//  Programmer: Paul Selby
//  Creation:   February 11, 2015
//
//  Modifications:
//    Paul Selby, Tue 10 Mar 15:16:25 GMT 2015
//    Moved file type check to Initialize method
//
//    Paul Selby, Wed 18 Mar 15:36:05 GMT 2015
//    Added fileId, stateId & currentTimestep
//
// ****************************************************************************

avtTyphonIOFileFormat::avtTyphonIOFileFormat(const char *filename)
    : avtMTMDFileFormat(filename)
{
    // INITIALIZE DATA MEMBERS
    initialized = false;
    fileId = -1;
    stateId = -1;
    currentTimestep=-1;
}


// ****************************************************************************
//  Method: avtTyphonIOFileFormat destructor
//
//  Programmer: Paul Selby
//  Creation:   March 18, 2015
//
// ****************************************************************************

avtTyphonIOFileFormat::~avtTyphonIOFileFormat()
{
    //
    // Close file and active state
    // - all other objects closed immediately when finished with
    //
    if (fileId != -1)
    {
        if (stateId != -1)
        {
            if (TIO_Close_State(fileId, stateId) != TIO_SUCCESS)
            {
                debug1 << "Failed to close state in TyphonIO file "
                       << filename << endl;
            }
        }
        if (TIO_Close(fileId) != TIO_SUCCESS)
        {
            debug1 << "Failed to close TyphonIO file " << filename << endl;
        }
    }
}


// ****************************************************************************
//  Method: avtTyphonIOFileFormat::GetCycles
//
//  Purpose:
//    Gets the cycles.
//
//  Arguments:
//    cycles  Return vector for the cycles.
//
//  Programmer: Paul Selby
//  Creation:   March 24, 2015
//
// ****************************************************************************

void
avtTyphonIOFileFormat::GetCycles(std::vector<int> &cycles)
{
    PopulateStateInfo();

    for (size_t s = 0; s < stateInfo.size(); ++s)
    {
       cycles.push_back(stateInfo[s].step);
    }
}


// ****************************************************************************
//  Method: avtTyphonIOFileFormat::GetTimes
//
//  Purpose:
//    Gets the times.
//
//  Arguments:
//    times  Return vector for the times.
//
//  Programmer: Paul Selby
//  Creation:   March 24, 2015
//
// ****************************************************************************

void
avtTyphonIOFileFormat::GetTimes(std::vector<double> &times)
{
    PopulateStateInfo();

    for (size_t s = 0; s < stateInfo.size(); ++s)
    {
       times.push_back(stateInfo[s].time);
    }
}


// ****************************************************************************
//  Method: avtTyphonIOFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//      Note: Do not do anything expensive here as called often
//
//  Programmer: Paul Selby
//  Creation:   February 11, 2015
//
//  Modifications:
//    Paul Selby, Wed 18 Mar 15:36:05 GMT 2015
//    Get nstates from Initialize
//
// ****************************************************************************

int
avtTyphonIOFileFormat::GetNTimesteps(void)
{
    //
    // Initialize the file if it has not been initialized.
    //
    Initialize();

    return nstates;
}


// ****************************************************************************
//  Method: avtTyphonIOFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Paul Selby
//  Creation:   February 11, 2015
//
// ****************************************************************************

void
avtTyphonIOFileFormat::FreeUpResources(void)
{
}


// ****************************************************************************
//  Method: avtTyphonIOFileFormat::Initialize
//
//  Purpose:
//    Initializes the file format by reading the file and the contents, etc.
//
//  Programmer: Paul Selby
//  Creation:   March 10, 2015
//
//  Modifications:
//    Paul Selby, Wed 18 Mar 15:36:05 GMT 2015
//    Opens file, populates filecomment and gets number of states
//
//    Paul Selby, Mon 30 Mar 12:00:59 BST 2015
//    Calls TIO_Disable_Fatality to avoid abort on error if TIO_ERRORS_FATAL
//    environment variable set
//
// ****************************************************************************

void
avtTyphonIOFileFormat::Initialize(void)
{
    if (!initialized)
    {
        //
        // TyphonIO aborts on error if TIO_ERRORS_FATAL environment variable
        // set => would prevent *.h5 files being opened by any reader
        //
        TIO_Disable_Fatality();

        if (TIO_CheckFile(filename) != TIO_SUCCESS)
        {
            EXCEPTION1(InvalidDBTypeException,
                       "TIO_Checkfile failed => not a TyphonIO file");
        }

        char codename[TIO_STRLEN] = "";
        char version[TIO_STRLEN]  = "";
        char date[TIO_STRLEN]     = "";
        char title[TIO_STRLEN]    = "";

        if (TIO_Open(filename, &fileId, TIO_ACC_READONLY, codename, version,
                     date, title, MPI_COMM_NULL, MPI_INFO_NULL, MPI_PROC_NULL)
            != TIO_SUCCESS)
        {
            EXCEPTION1(InvalidDBTypeException, "TIO_Open failed");
        }

        debug4 << "TyphonIO file opened: " << filename << endl;
        debug5 << "  codename : " << codename << endl;
        debug5 << "  version  : " << version  << endl;
        debug5 << "  date     : " << date     << endl;
        debug5 << "  title    : " << title    << endl;

        //
        // Construct comment string for use in MetaData
        //
        std::ostringstream comment;
        comment << "codename: " << codename << ", version: " << version
                << ", date: " << date << ", title: " << title;
        filecomment = comment.str();

        //
        // Get nstates here as GetNTimesteps called frequently
        // - returns nstates = 0 if no states in file
        //
        if (TIO_List_States(fileId, &nstates, NULL) != TIO_SUCCESS)
        {
            EXCEPTION1(InvalidDBTypeException, "TIO_List_States failed");
        }
    }
    initialized = true;
}


// ****************************************************************************
//  Method: avtTyphonIOFileFormat::PopulateStateInfo
//
//  Purpose:
//    Gets the cycles and times for each state and stores for later use
//    Avoids opening states in both GetCycles and GetTimes
//
//  Programmer: Paul Selby
//  Creation:   March 24, 2015
//
// ****************************************************************************

void
avtTyphonIOFileFormat::PopulateStateInfo()
{
    if (stateInfo.size() == 0)
    {
        //
        // Initialize the file if it has not been initialized.
        //
        Initialize();

        //
        // Get state metadata and cache for use in GetCycles/Times
        // Note: VisIt doesn't seem to have a use for the time units
        //
        for (TIO_Size_t s = 0; s < nstates; ++s)
        {
            TIO_Object_t tempId;
            TyphonIO_stateInfo sinfo;
            if (s == (TIO_Size_t) currentTimestep && stateId != -1)
            {
                //
                // State already open
                //
                if (TIO_Get_State_Info(fileId, stateId, NULL, &sinfo.step,
                                       &sinfo.time, NULL) != TIO_SUCCESS)
                {
                    EXCEPTION1(InvalidDBTypeException,
                               "TIO_Get_State_Info failed");
                }
            }
            else
            {
                //
                // Open state temporarily
                //
                if (TIO_Open_State_idx(fileId, s, NULL, &tempId, &sinfo.step,
                                       &sinfo.time, NULL) != TIO_SUCCESS)
                {
                    EXCEPTION1(InvalidDBTypeException,
                               "TIO_Open_State_idx failed");
                }
                if (TIO_Close_State(fileId, tempId) != TIO_SUCCESS)
                {
                    debug1 << "TIO_Close_State failed " << endl;
                }
            }
            stateInfo.push_back(sinfo);
        }
    }
}


// ****************************************************************************
//  Method: avtTyphonIOFileFormat::ActivateTimestep
//
//  Purpose:
//    This method is called each time we change to a new time state. Make
//    sure that the file has been initialized.
//
//  Arguments:
//    ts  timestate index
//
//  Programmer: Paul Selby
//  Creation:   March 10, 2015
//
//  Modifications:
//    Paul Selby, Wed 18 Mar 15:36:05 GMT 2015
//    Opens state and closes old one if necessary
//
// ****************************************************************************

void
avtTyphonIOFileFormat::ActivateTimestep(int ts)
{
    if (currentTimestep == ts) return;

    //
    // Initialize the file if it has not been initialized.
    //
    Initialize();

    //
    // Close existing state if open
    //
    if (stateId != -1)
    {
        if (TIO_Close_State(fileId, stateId) != TIO_SUCCESS)
        {
            debug1 << "Failed to close state in TyphonIO file " << filename
                   << endl;
        }
    }

    TIO_t success;
    success = TIO_Open_State_idx(fileId, ts, NULL, &stateId, NULL, NULL,
                                 NULL);
    if (success != TIO_SUCCESS)
    {
        EXCEPTION1(TyphonIOException, success);
    }
    currentTimestep = ts;
}


// ****************************************************************************
//  Method: avtTyphonIOFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Arguments:
//    md         metadata object to populate
//    timeState  timestate index
//
//  Programmer: Paul Selby
//  Creation:   February 11, 2015
//
//  Modifications:
//    Paul Selby, Wed 18 Mar 15:36:05 GMT 2015
//    Adds filecomment and all meshes in state to metadata
//
//    Paul Selby, Wed 25 Mar 18:03:06 GMT 2015
//    Added labels & units for axes. Changed blocknames to "chunk"
//
//    Paul Selby, Thu 26 Mar 10:27:08 GMT 2015
//    Minor cosmetic changes to coding style
//
//    Paul Selby, Thu 26 Mar 12:20:08 GMT 2015
//    Adds quants on meshes (i.e. ScalarVars) to metadata
//
//    Paul Selby, Thu 26 Mar 15:01:38 GMT 2015
//    Fixed bug where Quant exceptions would have incorect success value
//
//    Paul Selby, Mon 13 Apr 16:55:23 BST 2015
//    Added units for scalar variables
//
//    Paul Selby, Wed 17 Jun 17:33:23 BST 2015
//    Added material to metadata
//
//    Paul Selby, Tue 23 Jun 16:32:20 BST 2015
//    Added extents to mesh metadata
//
//    Paul Selby, Tue 23 Jun 17:31:40 BST 2015
//    Added extents to scalar variable metadata
//
// ****************************************************************************

void
avtTyphonIOFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md,
                                                int timeState)
{
    md->SetDatabaseComment(filecomment);

    //
    // Add all meshes on state
    //
    TIO_Size_t nmeshes;
    TIO_t success;

    ActivateTimestep(timeState);
    success = TIO_List_Meshes(fileId, stateId, &nmeshes, NULL);
    if (success != TIO_SUCCESS)
    {
        EXCEPTION1(TyphonIOException, success);
    }

    for (TIO_Size_t m = 0; m < nmeshes; ++m)
    {
        TIO_Object_t meshId;
        char meshName[TIO_STRLEN];
        TIO_Mesh_t meshType;
        TIO_Dims_t ndims;
        TIO_Size_t nchunks;
        char iUnits[TIO_STRLEN], jUnits[TIO_STRLEN], kUnits[TIO_STRLEN];
        char iLabel[TIO_STRLEN], jLabel[TIO_STRLEN], kLabel[TIO_STRLEN];

        success = TIO_Open_Mesh_idx(fileId, stateId, m, meshName, &meshId,
                                    &meshType, NULL, NULL, NULL, NULL, NULL,
                                    NULL, &ndims, NULL, NULL, NULL, NULL,
                                    &nchunks, iUnits, jUnits, kUnits, iLabel,
                                    jLabel, kLabel);
        if (success != TIO_SUCCESS)
        {
            EXCEPTION1(TyphonIOException, success);
        }

        avtMeshType mt;
        int topological_dimension = ndims;
        switch(meshType)
        {
          case TIO_MESH_QUAD_COLINEAR:
            mt = AVT_RECTILINEAR_MESH;
            break;
          case TIO_MESH_QUAD_NONCOLINEAR:
            mt = AVT_CURVILINEAR_MESH;
            break;
          case TIO_MESH_UNSTRUCT:
            mt = AVT_UNSTRUCTURED_MESH;
            break;
          case TIO_MESH_POINT:
            mt = AVT_POINT_MESH;
            topological_dimension = 0;
            break;
          default:
            mt = AVT_UNKNOWN_MESH;
        }
        //
        // Populate mesh metadata - use TyphonIO naming convention for blocks
        // Could use dimensions as bounds for structured meshes but IndexSelect
        // seems to assume it is bounds of chunk so no point if nchunks > 1
        //
        avtMeshMetaData *mesh = new avtMeshMetaData();
        mesh->name = meshName;
        mesh->meshType = mt;
        mesh->numBlocks = nchunks;
        mesh->spatialDimension = ndims;
        mesh->topologicalDimension = topological_dimension;
        mesh->blockTitle = "chunks";
        mesh->blockPieceName = "chunk";
        mesh->xLabel = "";
        mesh->yLabel = "";
        mesh->zLabel = "";
        switch(ndims)
        {
          case TIO_3D:
            mesh->zUnits = kUnits;
            mesh->zLabel = kLabel;
            // FALLTHRU
          case TIO_2D:
            mesh->yUnits = jUnits;
            mesh->yLabel = jLabel;
            // FALLTHRU
          case TIO_1D:
            mesh->xUnits = iUnits;
            mesh->xLabel = iLabel;
            // FALLTHRU
          default:
            break;
        }
        //
        // Look for mesh range (optional)
        // TyphonIO will convert to double on the fly
        //
        double extents[6];
        success = TIO_Get_Mesh_Range(fileId, meshId, TIO_DOUBLE, NULL, 
                                     &extents[0], &extents[1], &extents[2],
                                     &extents[3], &extents[4], &extents[5]);
        if (success == TIO_SUCCESS)
        {
            mesh->SetExtents(extents);
        }
        else if (success != TIO_ERR_INVALID_RANGE)
        {
            TIO_Close_Mesh(fileId, meshId);
            EXCEPTION1(TyphonIOException, success);
        }
        md->Add(mesh);

        //
        // Add all quants on mesh
        // Note: TyphonIO Variables are non-mesh based arrays
        //
        TIO_Size_t nquants;
        success = TIO_List_Quants(fileId, meshId, &nquants, NULL);
        if (success != TIO_SUCCESS)
        {
            TIO_Close_Mesh(fileId, meshId);
            EXCEPTION1(TyphonIOException, success);
        }

        for (TIO_Size_t q = 0; q < nquants; ++q)
        {
            TIO_Object_t quantId;
            char quantName[TIO_STRLEN];
            TIO_Centre_t centreType;
            char units[TIO_STRLEN];
            success = TIO_Open_Quant_idx(fileId, meshId, q, quantName,
                                         &quantId, NULL, &centreType, NULL,
                                         NULL, units);
            if (success != TIO_SUCCESS)
            {
                TIO_Close_Mesh(fileId, meshId);
                EXCEPTION1(TyphonIOException, success);
            }

            avtCentering cent;
            switch(centreType)
            {
              case TIO_CENTRE_CELL:
                cent = AVT_ZONECENT;
                break;
              case TIO_CENTRE_NODE:
                cent = AVT_NODECENT;
                break;
              default:
                cent = AVT_UNKNOWN_CENT;
            }
            //
            // Populate ScalarVar metadata
            //
            avtScalarMetaData *scalar = new avtScalarMetaData();
            scalar->name = quantName;
            scalar->meshName = meshName;
            scalar->centering = cent;
            scalar->units = units;
            //
            // Look for quant range (optional)
            // TyphonIO will convert to double on the fly
            //
            double extents[2];
            success = TIO_Get_Quant_Range(fileId, quantId, TIO_DOUBLE, 
                                          &extents[0], &extents[1]);
            if (success == TIO_SUCCESS)
            {
                scalar->SetExtents(extents);
            }
            else if (success != TIO_ERR_INVALID_RANGE)
            {
                TIO_Close_Quant(fileId, quantId);
                TIO_Close_Mesh(fileId, meshId);
                EXCEPTION1(TyphonIOException, success);
            }
            md->Add(scalar);

            success = TIO_Close_Quant(fileId, quantId);
            if (success != TIO_SUCCESS)
            {
                TIO_Close_Mesh(fileId, meshId);
                EXCEPTION1(TyphonIOException, success);
            }
        }

        //
        // Add Material - TyphonIO allows one per mesh
        //
        char matName[TIO_STRLEN];
        success = TIO_List_Material(fileId, meshId, matName);
        if (success != TIO_SUCCESS)
        {
            //
            // TIO_FAIL => no material present, which is not an error
            //
            if (success != TIO_FAIL)
            {
                TIO_Close_Mesh(fileId, meshId);
                EXCEPTION1(TyphonIOException, success);
            }
        }
        else
        {
            TIO_Object_t matId;
            TIO_Size_t nmat;
            success = TIO_Open_Material(fileId, meshId, matName, &matId, NULL,
                                        &nmat, NULL, NULL, NULL, NULL, NULL);
            if (success != TIO_SUCCESS)
            {
                TIO_Close_Mesh(fileId, meshId);
                EXCEPTION1(TyphonIOException, success);
            }

            //
            // Get material names
            //
            char (*matNames)[TIO_STRLEN] = new char[nmat][TIO_STRLEN];
            success = TIO_Get_Material_Range(fileId, matId, TIO_INT, NULL,
                                             NULL, matNames);
            if (success != TIO_SUCCESS)
            {
                TIO_Close_Material(fileId, matId);
                TIO_Close_Mesh(fileId, meshId);
                EXCEPTION1(TyphonIOException, success);
            }

            //
            // Populate material metadata
            // - call directly to simplify setting materialNames
            //
            avtMaterialMetaData *mat = new avtMaterialMetaData();
            mat->name = matName;
            mat->meshName = meshName;
            mat->numMaterials = nmat;
            for (TIO_Size_t i = 0; i < nmat; i++)
            {
                mat->materialNames.push_back(matNames[i]);
            }
            delete[] matNames;
            md->Add(mat);

            success = TIO_Close_Material(fileId, matId);
            if (success != TIO_SUCCESS)
            {
                TIO_Close_Mesh(fileId, meshId);
                EXCEPTION1(TyphonIOException, success);
            }
        }

        success = TIO_Close_Mesh(fileId, meshId);
        if (success != TIO_SUCCESS)
        {
            EXCEPTION1(TyphonIOException, success);
        }
    }

    //
    // Here's the way to add expressions:
    //Expression momentum_expr;
    //momentum_expr.SetName("momentum");
    //momentum_expr.SetDefinition("{u, v}");
    //momentum_expr.SetType(Expression::VectorMeshVar);
    //md->AddExpression(&momentum_expr);
    //Expression KineticEnergy_expr;
    //KineticEnergy_expr.SetName("KineticEnergy");
    //KineticEnergy_expr.SetDefinition("0.5*(momentum*momentum)/(rho*rho)");
    //KineticEnergy_expr.SetType(Expression::ScalarMeshVar);
    //md->AddExpression(&KineticEnergy_expr);
    //
}


// ****************************************************************************
//  Method: avtTyphonIOFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      timestate   The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Paul Selby
//  Creation:   February 11, 2015
//
//  Modifications:
//    Paul Selby, Wed 18 Mar 15:36:05 GMT 2015
//    Added support for Point & Rectilinear meshes
//
//    Paul Selby, Thu 26 Mar 10:27:08 GMT 2015
//    Minor cosmetic changes to coding style
//
//    Paul Selby, Tue 23 Jun 15:50:34 BST 2015
//    Added support for Curvilinear meshes
//
// ****************************************************************************

vtkDataSet *
avtTyphonIOFileFormat::GetMesh(int timestate, int domain, const char *meshname)
{
    //
    // Open mesh and pass to helper function to read
    //
    TIO_Object_t meshId;
    TIO_Mesh_t meshType;
    TIO_t success;

    ActivateTimestep(timestate);
    success = TIO_Open_Mesh(fileId, stateId, meshname, &meshId, &meshType,
                            NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                            NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                            NULL, NULL);
    if (success != TIO_SUCCESS)
    {
        EXCEPTION1(TyphonIOException, success);
    }

    vtkDataSet *rv = 0;
    TRY
    {
        switch(meshType)
        {
          case TIO_MESH_QUAD_COLINEAR:
            rv = GetQuadColinearMesh(meshId, domain);
            break;
          case TIO_MESH_QUAD_NONCOLINEAR:
            rv = GetQuadNonColinearMesh(meshId, domain);
            break;
          case TIO_MESH_POINT:
            rv = GetPointMesh(meshId, domain);
            break;
          default:
            EXCEPTION1(VisItException, "Unsupported TyphonIO meshtype");
        }
    }
    CATCHALL
    {
        if (TIO_Close_Mesh(fileId, meshId) != TIO_SUCCESS)
        {
            debug1 << "Failed to close mesh in TyphonIO file " << filename
                   << endl;
        }
        RETHROW;
    }
    ENDTRY

    success = TIO_Close_Mesh(fileId, meshId);
    if (success != TIO_SUCCESS)
    {
        EXCEPTION1(TyphonIOException, success);
    }

    return rv;
}


// ****************************************************************************
//  Method: avtTyphonIOFileFormat::GetQuadColinearMesh
//
//  Purpose:
//      Gets the quad mesh chunk as vtkRectilinearGrid.
//
//  Arguments:
//      meshId  Previously opened TyphonIO meshId
//      chunk   TyphonIO chunk index
//
//  Programmer: Paul Selby
//  Creation:   March 16, 2015
//
//  Modifications:
//    Paul Selby, Thu 26 Mar 10:27:08 GMT 2015
//    Minor cosmetic changes to coding style
//
// ****************************************************************************

vtkRectilinearGrid *
avtTyphonIOFileFormat::GetQuadColinearMesh(TIO_Object_t meshId,
                                           TIO_Size_t chunk)
{
    TIO_Dims_t ndims;
    TIO_Size_t nx, ny, nz;
    TIO_t success;

    //
    // Note: Currently discarding ghost cells
    //
    success = TIO_Read_QuadMesh_Chunk(fileId, meshId, chunk,
                                      TIO_XFER_INDEPENDENT, TIO_FLOAT,
                                      TIO_GHOSTS_NONE, &ndims, &nx, &ny, &nz,
                                      NULL, NULL, NULL);
    if (success != TIO_SUCCESS)
    {
        EXCEPTION1(TyphonIOException, success);
    }

    //
    // Can read co-ordinates directly into memory
    // - will fake y/z value if 1D or 2D
    // Note: TyphonIO will convert to float on the fly if needed
    //
    int dims[3] = {1, 1, 1};
    switch (ndims)
    {
      case TIO_3D: dims[2] = nz; // FALLTHRU
      case TIO_2D: dims[1] = ny; // FALLTHRU
      case TIO_1D: dims[0] = nx; // FALLTHRU
      default:
        break;
    }

    vtkFloatArray *coords[3] = {NULL, NULL, NULL};
    coords[0] = vtkFloatArray::New();
    coords[0]->SetNumberOfTuples(dims[0]);
    float *x = (float *) coords[0]->GetVoidPointer(0);
    coords[1] = vtkFloatArray::New();
    coords[1]->SetNumberOfTuples(dims[1]);
    float *y = (float *) coords[1]->GetVoidPointer(0);
    coords[2] = vtkFloatArray::New();
    coords[2]->SetNumberOfTuples(dims[2]);
    float *z = (float *) coords[2]->GetVoidPointer(0);

    if (dims[0] > 0) x[0] = 0.0;
    if (dims[1] > 0) y[0] = 0.0;
    if (dims[2] > 0) z[0] = 0.0;

    success = TIO_Read_QuadMesh_Chunk(fileId, meshId, chunk,
                                      TIO_XFER_INDEPENDENT, TIO_FLOAT,
                                      TIO_GHOSTS_NONE, NULL, NULL, NULL, NULL,
                                      x, y, z);
    if (success != TIO_SUCCESS)
    {
        coords[0]->Delete();
        coords[1]->Delete();
        coords[2]->Delete();
        EXCEPTION1(TyphonIOException, success);
    }

    //
    // Create vtkRectilinearGrid to contain co-ordinates
    //
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
//  Method: avtTyphonIOFileFormat::GetQuadNonColinearMesh
//
//  Purpose:
//      Gets the non-colinear quad mesh chunk as vtkStructuredGrid
//
//  Arguments:
//      meshId  Previously opened TyphonIO meshId
//      chunk   TyphonIO chunk index
//
//  Programmer: Paul Selby
//  Creation:   June 23, 2015
//
// ****************************************************************************

vtkStructuredGrid *
avtTyphonIOFileFormat::GetQuadNonColinearMesh(TIO_Object_t meshId,
                                              TIO_Size_t chunk)
{
    TIO_Dims_t ndims;
    TIO_Size_t nx, ny, nz;
    TIO_t success;

    //
    // Note: Currently discarding ghost cells
    //
    success = TIO_Read_QuadMesh_Chunk(fileId, meshId, chunk,
                                      TIO_XFER_INDEPENDENT, TIO_FLOAT,
                                      TIO_GHOSTS_NONE, &ndims, &nx, &ny, &nz,
                                      NULL, NULL, NULL);
    if (success != TIO_SUCCESS)
    {
        EXCEPTION1(TyphonIOException, success);
    }

    //
    // Cannot read co-ordinates directly into memory as not points[nn][3]
    // Note: TyphonIO will convert to float on the fly if needed
    //
    int dims[3] = {1, 1, 1};
    switch (ndims)
    {
      case TIO_3D: dims[2] = nz; // FALLTHRU
      case TIO_2D: dims[1] = ny; // FALLTHRU
      case TIO_1D: dims[0] = nx; // FALLTHRU
      default:
        break;
    }
    size_t nnodes = dims[0] * dims[1] * dims[2];

    float *x = 0;
    float *y = 0;
    float *z = 0;
    switch (ndims)
    {
      case TIO_3D: z = new float[nnodes]; // FALLTHRU
      case TIO_2D: y = new float[nnodes]; // FALLTHRU
      case TIO_1D: x = new float[nnodes]; // FALLTHRU
      default:
        break;
    }

    success = TIO_Read_QuadMesh_Chunk(fileId, meshId, chunk,
                                      TIO_XFER_INDEPENDENT, TIO_FLOAT,
                                      TIO_GHOSTS_NONE, NULL, NULL, NULL, NULL,
                                      x, y, z);
    if (success != TIO_SUCCESS)
    {
        delete[] x;
        delete[] y;
        delete[] z;
        EXCEPTION1(TyphonIOException, success);
    }

    //
    // Create vtkPoints object and copy points into it
    //
    vtkPoints *points = vtkPoints::New();
    points->SetNumberOfPoints(nnodes);
    float *pts = (float *) points->GetVoidPointer(0);
    if (ndims == TIO_3D)
    {
        for (TIO_Size_t i = 0; i < nnodes; ++i)
        {
            *pts++ = x[i];
            *pts++ = y[i];
            *pts++ = z[i];
        }
    }
    else if (ndims == TIO_2D)
    {
        for (TIO_Size_t i = 0; i < nnodes; ++i)
        {
            *pts++ = x[i];
            *pts++ = y[i];
            *pts++ = 0.0;
        }
    }
    else if (ndims == TIO_1D)
    {
        for (TIO_Size_t i = 0; i < nnodes; ++i)
        {
            *pts++ = x[i];
            *pts++ = 0.0;
            *pts++ = 0.0;
        }
    }

    //
    // Create vtkStructuredGrid to contain point cells
    //
    vtkStructuredGrid *sgrid = vtkStructuredGrid::New();
    sgrid->SetPoints(points);
    sgrid->SetDimensions(dims);
    points->Delete();

    //
    // Delete temporary arrays
    //
    delete[] x;
    delete[] y;
    delete[] z;

    return sgrid;
}


// ****************************************************************************
//  Method: avtTyphonIOFileFormat::GetPointMesh
//
//  Purpose:
//      Gets the point mesh chunk as vtkUnstructuredGrid.
//
//  Arguments:
//      meshId  Previously opened TyphonIO meshId
//      chunk   TyphonIO chunk index
//
//  Programmer: Paul Selby
//  Creation:   March 12, 2015
//
//  Modifications:
//    Paul Selby, Thu 26 Mar 10:27:08 GMT 2015
//    Minor cosmetic changes to coding style
//
// ****************************************************************************

vtkUnstructuredGrid *
avtTyphonIOFileFormat::GetPointMesh(TIO_Object_t meshId, TIO_Size_t chunk)
{
    TIO_Dims_t ndims;
    TIO_Size_t nnodes;
    TIO_t success;

    success = TIO_Read_PointMesh_Chunk(fileId, meshId, chunk,
                                       TIO_XFER_INDEPENDENT, TIO_FLOAT,
                                       TIO_GHOSTS_NONE, &ndims, &nnodes,
                                       NULL, NULL, NULL);
    if (success != TIO_SUCCESS)
    {
        EXCEPTION1(TyphonIOException, success);
    }

    //
    // Cannot read co-ordinates directly into memory as not points[nn][3]
    // Note: TyphonIO will convert to float on the fly if needed
    //
    float *x = 0;
    float *y = 0;
    float *z = 0;
    switch (ndims)
    {
      case TIO_3D: z = new float[nnodes]; // FALLTHRU
      case TIO_2D: y = new float[nnodes]; // FALLTHRU
      case TIO_1D: x = new float[nnodes]; // FALLTHRU
      default:
        break;
    }

    success = TIO_Read_PointMesh_Chunk(fileId, meshId, chunk,
                                       TIO_XFER_INDEPENDENT, TIO_FLOAT,
                                       TIO_GHOSTS_NONE, NULL, NULL, x, y, z);
    if (success != TIO_SUCCESS)
    {
        delete[] x;
        delete[] y;
        delete[] z;
        EXCEPTION1(TyphonIOException, success);
    }

    //
    // Create vtkPoints object and copy points into it
    //
    vtkPoints *points = vtkPoints::New();
    points->SetNumberOfPoints(nnodes);
    float *pts = (float *) points->GetVoidPointer(0);
    if (ndims == TIO_3D)
    {
        for (TIO_Size_t i = 0; i < nnodes; ++i)
        {
            *pts++ = x[i];
            *pts++ = y[i];
            *pts++ = z[i];
        }
    }
    else if (ndims == TIO_2D)
    {
        for (TIO_Size_t i = 0; i < nnodes; ++i)
        {
            *pts++ = x[i];
            *pts++ = y[i];
            *pts++ = 0.0;
        }
    }
    else if (ndims == TIO_1D)
    {
        for (TIO_Size_t i = 0; i < nnodes; ++i)
        {
            *pts++ = x[i];
            *pts++ = 0.0;
            *pts++ = 0.0;
        }
    }

    //
    // Create vtkUnstructuredGrid to contain point cells
    //
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->SetPoints(points);
    points->Delete();
    ugrid->Allocate(nnodes);
    vtkIdType onevertex;
    for (TIO_Size_t i = 0; i < nnodes ; ++i)
    {
        onevertex = i;
        ugrid->InsertNextCell(VTK_VERTEX, 1, &onevertex);
    }

    //
    // Delete temporary arrays
    //
    delete[] x;
    delete[] y;
    delete[] z;

    return ugrid;
}


// ****************************************************************************
//  Method: avtTyphonIOFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Paul Selby
//  Creation:   February 11, 2015
//
//  Modifications:
//    Paul Selby, Thu 26 Mar 13:13:59 GMT 2015
//    Added support for Point meshes
//
//    Paul Selby, Thu 18 Jun 17:02:18 BST 2015
//    Added support for Quad meshes
//
// ****************************************************************************

vtkDataArray *
avtTyphonIOFileFormat::GetVar(int timestate, int domain, const char *varname)
{
    //
    // Open mesh & quant and pass to helper function to read
    //
    TIO_Object_t meshId, quantId;
    TIO_Mesh_t meshType;
    TIO_Centre_t centreType;
    TIO_t success;

    ActivateTimestep(timestate);

    string meshName = metadata->MeshForVar(varname);

    success = TIO_Open_Mesh(fileId, stateId, meshName.c_str(), &meshId,
                            &meshType, NULL, NULL, NULL, NULL, NULL, NULL,
                            NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                            NULL, NULL, NULL, NULL);
    if (success != TIO_SUCCESS)
    {
        EXCEPTION1(TyphonIOException, success);
    }

    success = TIO_Open_Quant(fileId, meshId, varname, &quantId, NULL,
                             &centreType, NULL, NULL, NULL);
    if (success != TIO_SUCCESS)
    {
        TIO_Close_Mesh(fileId, meshId);
        EXCEPTION1(TyphonIOException, success);
    }

    vtkDataArray *rv = 0;
    TRY
    {
        switch(meshType)
        {
          case TIO_MESH_QUAD_COLINEAR: // FALLTHRU
          case TIO_MESH_QUAD_NONCOLINEAR:
            rv = GetQuadVar(quantId, domain, centreType);
            break;
          case TIO_MESH_POINT:
            rv = GetPointVar(quantId, domain, centreType);
            break;
          default:
            EXCEPTION1(VisItException, "Unsupported TyphonIO meshtype");
        }
    }
    CATCHALL
    {
        if (TIO_Close_Quant(fileId, quantId) != TIO_SUCCESS)
        {
            debug1 << "Failed to close quant in TyphonIO file " << filename
                   << endl;
        }
        if (TIO_Close_Mesh(fileId, meshId) != TIO_SUCCESS)
        {
            debug1 << "Failed to close mesh in TyphonIO file " << filename
                   << endl;
        }
        RETHROW;
    }
    ENDTRY

    success = TIO_Close_Quant(fileId, quantId);
    if (success != TIO_SUCCESS)
    {
        TIO_Close_Mesh(fileId, meshId);
        EXCEPTION1(TyphonIOException, success);
    }
    success = TIO_Close_Mesh(fileId, meshId);
    if (success != TIO_SUCCESS)
    {
        EXCEPTION1(TyphonIOException, success);
    }

    return rv;
}


// ****************************************************************************
//  Method: avtTyphonIOFileFormat::GetQuadVar
//
//  Purpose:
//      Gets the point mesh variable
//
//  Arguments:
//      quantId    Previously opened TyphonIO quantId
//      chunk      TyphonIO chunk index
//      centreType TyphonIO centre type
//
//  Programmer: Paul Selby
//  Creation:   June 18, 2015
//
// ****************************************************************************

vtkDataArray *
avtTyphonIOFileFormat::GetQuadVar(TIO_Object_t quantId, TIO_Size_t chunk,
                                  TIO_Centre_t centreType)
{
    TIO_Dims_t ndims;
    TIO_Size_t ni, nj, nk;
    TIO_t success;

    success = TIO_Read_QuadQuant_Chunk(fileId, quantId, chunk,
                                       TIO_XFER_INDEPENDENT, TIO_FLOAT,
                                       TIO_GHOSTS_NONE, &ndims, &ni, &nj, &nk,
                                       NULL, NULL, NULL);
    if (success != TIO_SUCCESS)
    {
        EXCEPTION1(TyphonIOException, success);
    }

    switch(centreType)
    {
      case TIO_CENTRE_CELL: // FALLTHRU
      case TIO_CENTRE_NODE:
        break;
      default:
        EXCEPTION1(VisItException, "Unsupported centring for Quad Mesh");
    }

    //
    // Can read values directly into memory
    // Note: TyphonIO will convert to float on the fly if needed
    //
    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfTuples(ni * (ndims > 1 ? nj : 1) * (ndims > 2 ? nk : 1));
    float *vals = (float *) rv->GetVoidPointer(0);

    success = TIO_Read_QuadQuant_Chunk(fileId, quantId, chunk,
                                       TIO_XFER_INDEPENDENT, TIO_FLOAT,
                                       TIO_GHOSTS_NONE, NULL, NULL, NULL, NULL,
                                       vals, NULL, NULL);
    if (success != TIO_SUCCESS)
    {
        rv->Delete();
        EXCEPTION1(TyphonIOException, success);
    }

    return rv;
}


// ****************************************************************************
//  Method: avtTyphonIOFileFormat::GetPointVar
//
//  Purpose:
//      Gets the point mesh variable
//
//  Arguments:
//      quantId    Previously opened TyphonIO quantId
//      chunk      TyphonIO chunk index
//      centreType TyphonIO centre type
//
//  Programmer: Paul Selby
//  Creation:   March 26, 2015
//
// ****************************************************************************

vtkDataArray *
avtTyphonIOFileFormat::GetPointVar(TIO_Object_t quantId, TIO_Size_t chunk,
                                   TIO_Centre_t centreType)
{
    TIO_Size_t nnodes;
    TIO_t success;

    success = TIO_Read_PointQuant_Chunk(fileId, quantId, chunk,
                                        TIO_XFER_INDEPENDENT, TIO_FLOAT,
                                        TIO_GHOSTS_NONE, &nnodes, NULL);
    if (success != TIO_SUCCESS)
    {
        EXCEPTION1(TyphonIOException, success);
    }

    switch(centreType)
    {
      case TIO_CENTRE_CELL:
        break;
      case TIO_CENTRE_NODE:
        break;
      default:
        EXCEPTION1(VisItException, "Unsupported centring for Point Mesh");
    }

    //
    // Can read values directly into memory
    // Note: TyphonIO will convert to float on the fly if needed
    //
    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfTuples(nnodes);
    float *vals = (float *) rv->GetVoidPointer(0);

    success = TIO_Read_PointQuant_Chunk(fileId, quantId, chunk,
                                       TIO_XFER_INDEPENDENT, TIO_FLOAT,
                                       TIO_GHOSTS_NONE, NULL, vals);
    if (success != TIO_SUCCESS)
    {
        rv->Delete();
        EXCEPTION1(TyphonIOException, success);
    }

    return rv;
}


// ****************************************************************************
//  Method: avtTyphonIOFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Paul Selby
//  Creation:   February 11, 2015
//
// ****************************************************************************

vtkDataArray *
avtTyphonIOFileFormat::GetVectorVar(int timestate, int domain,
                                    const char *varname)
{
    //
    // TyphonIO only contains scalar variables
    // - vectors or tensors will be constructed using Expressions
    //
    EXCEPTION1(InvalidVariableException, varname);
}


// ****************************************************************************
//  Method: avtTyphonIOFileFormat::GetAuxiliaryData
//
//  Purpose:
//      Gets the auxiliary data specified.
//
//  Arguments:
//      varname    The variable.
//      timestate  The index of the timestate.
//      domain     The index of the domain.
//      type       The type of auxiliary data.
//      args       Any additional arguments.
//      df         Destructor function.
//
//  Returns:    A void * of the auxiliary data.
//
//  Programmer: Paul Selby
//  Creation:   June 18, 2015
//
// ****************************************************************************

void *
avtTyphonIOFileFormat::GetAuxiliaryData(const char *varname, int timestate,
                                        int domain, const char *type,
                                        void *args, DestructorFunction &df)
{
    void *retval = 0;

    //
    // Need to find mesh for var
    //

    if(strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        //
        // Open mesh & material and pass to helper function to read
        //
        TIO_Object_t meshId, matId;
        TIO_Mesh_t meshType;
        TIO_t success;

        ActivateTimestep(timestate);

        string meshName = metadata->MeshForVar(varname);

        success = TIO_Open_Mesh(fileId, stateId, meshName.c_str(), &meshId,
                                &meshType, NULL, NULL, NULL, NULL, NULL, NULL,
                                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                                NULL, NULL, NULL, NULL);
        if (success != TIO_SUCCESS)
        {
            EXCEPTION1(TyphonIOException, success);
        }

        success = TIO_Open_Material(fileId, meshId, varname, &matId, NULL,
                                    NULL, NULL, NULL, NULL, NULL, NULL);
        if (success != TIO_SUCCESS)
        {
            TIO_Close_Mesh(fileId, meshId);
            EXCEPTION1(TyphonIOException, success);
        }

        TRY
        {
            switch(meshType)
            {
              case TIO_MESH_POINT:
                retval = GetPointMat(matId, domain);
                break;
              default:
                EXCEPTION1(VisItException, "Unsupported TyphonIO meshtype");
            }
        }
        CATCHALL
        {
            if (TIO_Close_Material(fileId, matId) != TIO_SUCCESS)
            {
                debug1 << "Failed to close material in TyphonIO file "
                       << filename << endl;
            }
            if (TIO_Close_Mesh(fileId, meshId) != TIO_SUCCESS)
            {
                debug1 << "Failed to close mesh in TyphonIO file " << filename
                       << endl;
            }
            RETHROW;
        }
        ENDTRY

        success = TIO_Close_Material(fileId, matId);
        if (success != TIO_SUCCESS)
        {
            TIO_Close_Mesh(fileId, meshId);
            EXCEPTION1(TyphonIOException, success);
        }
        success = TIO_Close_Mesh(fileId, meshId);
        if (success != TIO_SUCCESS)
        {
            EXCEPTION1(TyphonIOException, success);
        }

        df = avtMaterial::Destruct;
    }

    return retval;
}


// ****************************************************************************
//  Method: avtTyphonIOFileFormat::GetPointMat
//
//  Purpose:
//      Gets the point mesh variable
//
//  Arguments:
//      matId    Previously opened TyphonIO matId
//      chunk      TyphonIO chunk index
//
//  Programmer: Paul Selby
//  Creation:   June 18, 2015
//
// ****************************************************************************

avtMaterial *
avtTyphonIOFileFormat::GetPointMat(TIO_Object_t matId, TIO_Size_t chunk)
{
    TIO_Size_t nnodes;
    TIO_t success;

    success = TIO_Read_PointMaterial_Chunk(fileId, matId, chunk,
                                           TIO_XFER_INDEPENDENT, TIO_INT,
                                           TIO_GHOSTS_NONE, &nnodes, NULL);
    if (success != TIO_SUCCESS)
    {
        EXCEPTION1(TyphonIOException, success);
    }

    //
    // Read material values
    // Note: TyphonIO will convert to int on the fly if needed
    //
    int *matlist = new int[nnodes];

    success = TIO_Read_PointMaterial_Chunk(fileId, matId, chunk,
                                           TIO_XFER_INDEPENDENT, TIO_INT,
                                           TIO_GHOSTS_NONE, NULL, matlist);
    if (success != TIO_SUCCESS)
    {
        delete[] matlist;
        EXCEPTION1(TyphonIOException, success);
    }

    //
    // Get material names and numbers
    //
    TIO_Size_t nmat;
    success = TIO_Get_Material_Range(fileId, matId, TIO_INT, &nmat, NULL,
                                     NULL);
    if (success != TIO_SUCCESS)
    {
        delete[] matlist;
        EXCEPTION1(TyphonIOException, success);
    }
    int *matNums = new int[nmat];
    char (*matNames)[TIO_STRLEN] = new char[nmat][TIO_STRLEN];
    success = TIO_Get_Material_Range(fileId, matId, TIO_INT, NULL, matNums,
                                     matNames);
    if (success != TIO_SUCCESS)
    {
        delete[] matlist;
        delete[] matNums;
        delete[] matNames;
        EXCEPTION1(TyphonIOException, success);
    }

    char **names = new char*[nmat];
    for (TIO_Size_t i = 0; i < nmat; ++i)
    {
        names[i] = matNames[i];
    }

    //
    // Construct domain message for use in errors
    //
    std::ostringstream domain;
    domain << "chunk " << chunk;

    //
    // Point mesh material never has mix
    //
    int dims = nnodes;
    avtMaterial *mat = new avtMaterial(nmat, matNums, names, 1, &dims, 0,
                                       matlist, 0, NULL, NULL, NULL, NULL,
                                       domain.str().c_str());
    delete[] matlist;
    delete[] matNums;
    delete[] matNames;
    delete[] names;

    return mat;
}