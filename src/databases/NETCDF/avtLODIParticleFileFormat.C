#include <avtLODIParticleFileFormat.h>
#include <NETCDFFileObject.h>
#include <netcdf.h>

#include <avtDatabaseMetaData.h>
#include <avtMaterial.h>
#include <avtMTSDFileFormatInterface.h>
#include <avtTypes.h>
#include <avtVariableCache.h>

#include <vtkCellType.h>
#include <vtkUnsignedCharArray.h>
#include <vtkShortArray.h>
#include <vtkIntArray.h>
#include <vtkLongArray.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>

#include <InvalidVariableException.h>
#include <DebugStream.h>
#include <snprintf.h>

// ****************************************************************************
// Method: avtLODIParticleFileFormat::Identify
//
// Purpose: 
//   This method checks to see if the file is a LODI Particle file.
//
// Arguments:
//   fileObject : The file to check.
//
// Returns:    True if the file is a particle file; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 15 18:04:15 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool
avtLODIParticleFileFormat::Identify(NETCDFFileObject *fileObject)
{
    bool isLODIParticle = false;

    std::string create_code;
    if(fileObject->ReadStringAttribute("create_code", create_code))
    {
        bool isLODI = create_code == "LODI";
        std::string create_type;
        if(isLODI && fileObject->ReadStringAttribute("create_type", create_type))
        {
            isLODIParticle = create_type == "particle data";
        }
    }

    return isLODIParticle;
}

// ****************************************************************************
// Method: avtLODIParticleFileFormat::CreateInterface
//
// Purpose: 
//   This method creates a MTSD file format interface containing LODI Particle
//   file format readers.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 15 18:05:07 PST 2005
//
// Modifications:
//   
// ****************************************************************************

avtFileFormatInterface *
avtLODIParticleFileFormat::CreateInterface(NETCDFFileObject *f,
    const char *const *list, int nList, int)
{
    avtMTSDFileFormat **ffl = new avtMTSDFileFormat*[nList];
    for (int i = 0 ; i < nList ; i++)
        ffl[i] = 0;

    TRY
    {
        ffl[0] = new avtLODIParticleFileFormat(list[0], f);
        for (int i = 1 ; i < nList ; i++)   
            ffl[i] = new avtLODIParticleFileFormat(list[i]);
    }
    CATCH(VisItException)
    {
        for (int i = 0 ; i < nList ; i++)
            delete ffl[i];
        delete [] ffl;

        RETHROW;
    }
    ENDTRY

    return new avtMTSDFileFormatInterface(ffl, nList);
}

// ****************************************************************************
// Method: avtLODIParticleFileFormat::avtLODIParticleFileFormat
//
// Purpose: 
//   Constructor for the avtLODIParticleFileFormat class.
//
// Arguments:
//   filename : The name of the file being read.
//   f        : The file object to use.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 15 18:05:46 PST 2005
//
// Modifications:
//   
// ****************************************************************************

avtLODIParticleFileFormat::avtLODIParticleFileFormat(const char *filename) :
    avtMTSDFileFormat(&filename, 1), times(), sourceids()
{
    fileObject = new NETCDFFileObject(filename);
    timesRead = false;
}

avtLODIParticleFileFormat::avtLODIParticleFileFormat(const char *filename,
    NETCDFFileObject *f) : avtMTSDFileFormat(&filename, 1), times(), sourceids()
{
    fileObject = f;
    timesRead = false;
}

// ****************************************************************************
// Method: avtLODIParticleFileFormat::~avtLODIParticleFileFormat
//
// Purpose: 
//   Destructor for the avtLODIParticleFileFormat class.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 15 18:06:30 PST 2005
//
// Modifications:
//   
// ****************************************************************************

avtLODIParticleFileFormat::~avtLODIParticleFileFormat()
{
    FreeUpResources();

    delete fileObject;
}

// ****************************************************************************
// Method: avtLODIParticleFileFormat::ActivateTimestep
//
// Purpose: 
//   Activates the specified time step.
//
// Arguments:
//   ts : The time step to activate.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 15 18:06:45 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtLODIParticleFileFormat::ActivateTimestep(int ts)
{
    debug4 << "avtLODIParticleFileFormat::ActivateTimestep: ts=" << ts << endl;
    ReadTimes();
}

// ****************************************************************************
// Method: avtLODIParticleFileFormat::FreeUpResources
//
// Purpose: 
//   Frees up the resources used by this object.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 15 18:07:08 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtLODIParticleFileFormat::FreeUpResources()
{
    debug4 << "avtLODIParticleFileFormat::FreeUpResources" << endl;
    fileObject->Close();
}

// ****************************************************************************
// Method: avtLODIParticleFileFormat::ReadTimes
//
// Purpose: 
//   Reads in the times to be returned by this reader.
//
// Returns:    True if the times were read; False otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 15 18:07:27 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool
avtLODIParticleFileFormat::ReadTimes()
{
    const char *mName = "avtLODIParticleFileFormat::ReadTimes: ";
    debug4 << mName << endl;
    if(!timesRead)
    {
        // Set the times
        TypeEnum t = NO_TYPE;
        int ndims = 0, *dims = 0;
        void *values = 0;
        if(fileObject->ReadVariable("elapsed_time", &t, &ndims, &dims, &values))
        {
            if(ndims == 1 && t == DOUBLEARRAY_TYPE)
            {
                debug4 << mName << "times={";
                double *dptr = (double *)values;
                intVector cycles;
                for(int i = 0; i < dims[0]; ++i)
                {
                    debug4 << ", " << *dptr;
                    cycles.push_back(i);
                    times.push_back(*dptr++);
                }
                debug4 << "}" << endl;
                timesRead = true;
            }
            else
            {
                debug4 << mName << "elapsed_time was read but it was the "
                       << "wrong type." << endl;
            }

            delete [] dims;
            free_void_mem(values, t);
        }
        else
        {
            debug4 << mName << "Could not read elapsed_time array!" << endl;
        }
    }

    return timesRead;
}

// ****************************************************************************
// Method: avtLODIParticleFileFormat::GetNTimesteps
//
// Purpose: 
//   Returns the number of time states in this database.
//
// Returns:    The number of time states.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 15 18:08:06 PST 2005
//
// Modifications:
//   
// ****************************************************************************

int
avtLODIParticleFileFormat::GetNTimesteps()
{
    const char *mName = "avtLODIParticleFileFormat::GetNTimesteps: ";
    debug4 << mName << endl;
    ReadTimes();
    debug4 << mName << "returning " << times.size() << endl;
    return times.size();
}

// ****************************************************************************
// Method: avtLODIParticleFileFormat::GetTimes
//
// Purpose: 
//   Returns the times in the file.
//
// Arguments:
//   t : The times to be returned.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 15 18:08:32 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtLODIParticleFileFormat::GetTimes(doubleVector &t)
{
    debug4 << "avtLODIParticleFileFormat::GetTimes" << endl;
    ReadTimes();
    t = times;
}

// ****************************************************************************
// Method: avtLODIParticleFileFormat::PopulateDatabaseMetaData
//
// Purpose: 
//   Populates the list of variables for this file.
//
// Arguments:
//   md : The metadata object to populate.
//
// Note:       Creates a title and a mesh name.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 15 18:09:03 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtLODIParticleFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    const char *mName = "avtLODIParticleFileFormat::PopulateDatabaseMetaData: ";
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
    // Add a point mesh for the particles
    //
    avtMeshMetaData *mmd = new avtMeshMetaData("particles", 1, 0, 1, 3, 0,
        AVT_POINT_MESH);
    // Read the mesh units
    std::string meshUnits;
    if(fileObject->ReadStringAttribute("part_posn", "units", meshUnits))
    {
        mmd->xUnits = meshUnits;
        mmd->yUnits = meshUnits;
        mmd->zUnits = meshUnits;
    }
    // Read the mesh labels
    TypeEnum t = NO_TYPE;
    int ndims = 0, *dims = 0;
    void *values = 0;
    debug4 << mName << "Trying to read dimnum_labels" << endl;
    if(fileObject->ReadVariable("dimnum_labels", &t, &ndims, &dims, &values))
    {
        if(ndims == 2 && t == CHARARRAY_TYPE)
        {
            int len = dims[1];
            char *cptr = (char *)values;

            char *labels = new char[3 * (len+1)];
            char *xL = labels;
            char *yL = labels + len + 1;
            char *zL = yL + len + 1;
            memset(labels, 0, 3 * (len+1));
            memcpy(xL, cptr,       len);
            memcpy(yL, cptr+len,   len);
            memcpy(zL, cptr+len*2, len);

            mmd->xLabel = xL;
            debug4 << mName << "xLabel = " << xL << endl;

            mmd->yLabel = yL;
            debug4 << mName << "yLabel = " << yL << endl;

            mmd->zLabel = zL;
            debug4 << mName << "zLabel = " << zL << endl;
        }

        delete [] dims;
        free_void_mem(values, t);
    }
    md->Add(mmd);

    //
    // Read the sourceid variable and create material names from it.
    //
    sourceids.clear();
    if(fileObject->ReadVariable("sourceid", &t, &ndims, &dims, &values))
    {
        if(t == CHARARRAY_TYPE && ndims == 2)
        {
            int nsrcs = dims[0];
            int len = dims[1];
            char *name = new char[len+1];
            char *start = (char *)values;

            debug4 << mName << "sourceid={";
            for(int i = 0; i < nsrcs; ++i)
            {
                char *namestart = start + len * i;
                memcpy((void*)name, (void*)namestart, len);
                name[len] = '\0';

                char *end = name + len - 1;
                while(end >= name && *end == ' ')
                    *end-- = '\0';

                sourceids.push_back(name);

                if(i > 0)
                    debug4 << ", ";
                debug4 << name;
            }
            debug4 << "}" << endl;
            delete [] name;

            // Add the material to the metadata.
            avtMaterialMetaData *matmd = new avtMaterialMetaData("sourceid",
                "particles", sourceids.size(), sourceids);
            md->Add(matmd);
        }

        delete [] dims;
        free_void_mem(values, t);
    }

    //
    // Look for variables defined on the particles mesh.
    //
    if(fileObject->InqVariable("part_posn", &t, &ndims, &dims))
    {
        // Iterate over all of the variables and pick those that have
        // the same number of elements as nPts.
        int status, nDims, nVars, nGlobalAtts, unlimitedDimension;
        status = nc_inq(fileObject->GetFileHandle(), &nDims, &nVars, &nGlobalAtts,
                        &unlimitedDimension);
        if(status != NC_NOERR)
        {
            fileObject->HandleError(status);
            return;
        }

        // Get the sizes of all dimensions.
        int i;
        size_t *dimSizes = new size_t[nDims];
        for(i = 0; i < nDims; ++i)
        {
            int status = nc_inq_dimlen(fileObject->GetFileHandle(), i, &dimSizes[i]);
            if(status != NC_NOERR)
                fileObject->HandleError(status);
        }

        // Determine the maximum number of points in the particle mesh.
        int nElems = 1;
        for(i = 0; i < ndims; ++i)
            nElems *= dims[i];
        int nPts = nElems / 3;
        delete [] dims;

        // Look for variables with the same number of values as nPts.
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
                nElems = 1;
                for(int j = 0; j < varndims; ++j)
                    nElems *= dimSizes[vardims[j]];
                if(nElems == nPts)
                {
                    avtScalarMetaData *smd = new avtScalarMetaData(varname,
                        "particles", AVT_NODECENT);
                    smd->hasUnits = fileObject->ReadStringAttribute(
                        varname, "units", smd->units);
                    md->Add(smd);
                }
            }
        }

        delete [] dimSizes;
    }
}

// ****************************************************************************
// Method: avtLODIParticleFileFormat::GetMesh
//
// Purpose: 
//   Retrieves the mesh for the specified time state.
//
// Arguments:
//   ts  : The time state.
//   var : The name of the mesh to get.
//
// Returns:    A VTK dataset.
//
// Note:       We use the fill_value attribute of the part_posn field to
//             restrict points that should not be added to the final point mesh.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 15 18:09:47 PST 2005
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
avtLODIParticleFileFormat::GetMesh(int ts, const char *var)
{
    const char *mName = "avtLODIParticleFileFormat::GetMesh: ";
    debug4 << mName << "ts=" << ts
           << ", var=" << var << endl;

    vtkDataSet *retval = 0;
    bool err = true;
    if(strcmp(var, "particles") == 0)
    {
        TypeEnum t = NO_TYPE;
        int ndims = 0, *dims = 0;
        if(fileObject->InqVariable("part_posn", &t, &ndims, &dims))
        {
            size_t *starts = new size_t[ndims];
            size_t *counts = new size_t[ndims];
            int nElems = 1;
            for(int i = 1; i < ndims; ++i)
            {
                starts[i] = 0;
                counts[i] = dims[i];
                nElems *= dims[i];
            }
            starts[0] = ts;
            counts[0] = 1;

            int nPts = nElems / 3, varId;
            vtkPoints *pts = vtkPoints::New();
            fileObject->GetVarId("part_posn", &varId);

            //
            // Get the particle mask for the current time step.
            //
            bool *particleMask = GetParticleMask(ts);
            if(particleMask != 0)
            {
                debug4 << mName << "Adding particles using the "
                       << "particle mask." << endl;
                pts->Allocate(nPts);
                float *points = new float[nElems];
                int status = nc_get_vara_float(fileObject->GetFileHandle(),
                                               varId, starts, counts, points);
                if(status == NC_NOERR)
                {
                    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New(); 
                    ugrid->SetPoints(pts);
                    ugrid->Allocate(nPts);
                    vtkIdType onevertex[1];
                    float *fptr = points;
                    vtkIdType ptIndex = 0;
                    for (int i = 0 ; i < nPts ; i++)
                    {
                        if(particleMask[i])
                        {
                            pts->InsertNextPoint(fptr);
                            onevertex[0] = ptIndex++;
                            ugrid->InsertNextCell(VTK_VERTEX, 1, onevertex);
                        }
                        fptr += 3;
                    }
                    debug4 << mName << "Added " << ptIndex
                           << " points to the mesh." << endl;
                    err = false;
                    retval = ugrid;
                }
                else
                    fileObject->HandleError(status);

                delete [] points;
            }
            else
            {
                debug4 << mName << "Adding all particles." << endl;

                //
                // Read the coordinates into the VTK points array.
                //
                int nPts = nElems / 3, varId;
                vtkPoints *pts = vtkPoints::New();
                pts->SetNumberOfPoints(nPts);
                float *fptr = (float *)pts->GetVoidPointer(0);
                int status = nc_get_vara_float(fileObject->GetFileHandle(),
                                               varId, starts, counts, fptr);
                if(status == NC_NOERR)
                {
                    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New(); 
                    ugrid->SetPoints(pts);
                    ugrid->Allocate(nPts);
                    vtkIdType onevertex[1];
                    for (int i = 0 ; i < nPts ; i++)
                    {
                        onevertex[0] = i;
                        ugrid->InsertNextCell(VTK_VERTEX, 1, onevertex);
                    }

                    err = false;
                    retval = ugrid;
                }
                else
                    fileObject->HandleError(status);
            }

            pts->Delete();
            delete [] starts;
            delete [] counts;
            delete [] dims;
        }
    }

    if(err)
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    return retval;
}

// ****************************************************************************
// Method: avtLODIParticleFileFormat::GetVar
//
// Purpose: 
//   Returns the a variable
//
// Arguments:
//   ts  : The time state.
//   var : The name of the variable to get.
//
// Returns:    Nothing yet.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 15 18:11:21 PST 2005
//
// Modifications:
//   
// ****************************************************************************

vtkDataArray *
avtLODIParticleFileFormat::GetVar(int ts, const char *var)
{
    debug4 << "avtLODIParticleFileFormat::GetVar: ts=" << ts
           << ", var=" << var << endl;

    vtkDataArray *retval = 0;

    // Try and get the particle mask.
    bool *mask = GetParticleMask(ts);

    TypeEnum t = NO_TYPE;
    int ndims = 0, *dims = 0;
    if(mask != 0 && fileObject->InqVariable(var, &t, &ndims, &dims))
    {
        // Figure out the size of the chunk that we want to read.
        size_t *starts = new size_t[ndims];
        size_t *counts = new size_t[ndims];
        int nPts = 1;
        for(int i = 1; i < ndims; ++i)
        {
            starts[i] = 0;
            counts[i] = dims[i];
            nPts *= dims[i];
        }
        delete [] dims;
        starts[0] = ts;
        counts[0] = 1;

        int varId;
        fileObject->GetVarId(var, &varId);

#define READVAR(VTK, T, FUNC) \
        {\
            T *values = new T[nPts];\
            int status = FUNC(fileObject->GetFileHandle(),\
                              varId, starts, counts, values);\
            VTK *arr = VTK::New();\
            arr->Allocate(nPts);\
            vtkIdType index = 0;\
            for(int j = 0; j < nPts; ++j)\
            {\
                if(mask[j])\
                    arr->SetValue(index++, values[j]);\
            }\
            delete [] values;\
            arr->SetNumberOfTuples(index); \
            arr->Squeeze();\
            retval = arr;\
        }

        if(t == CHARARRAY_TYPE)
            READVAR(vtkUnsignedCharArray, char, nc_get_vara_text)
        else if(t == UCHARARRAY_TYPE)
            READVAR(vtkUnsignedCharArray, unsigned char, nc_get_vara_uchar)
        else if(t == SHORTARRAY_TYPE)
            READVAR(vtkShortArray, short, nc_get_vara_short)
        else if(t == INTEGERARRAY_TYPE)
            READVAR(vtkIntArray, int, nc_get_vara_int)
        else if(t == LONGARRAY_TYPE)
            READVAR(vtkLongArray, long, nc_get_vara_long)
        else if(t == FLOATARRAY_TYPE)
            READVAR(vtkFloatArray, float, nc_get_vara_float)
        else if(t == DOUBLEARRAY_TYPE)
            READVAR(vtkDoubleArray, double, nc_get_vara_double)

        delete [] starts;
        delete [] counts;
    }
    else
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    return retval;
}

// ****************************************************************************
// Method: avtLODIParticleFileFormat::GetAuxiliaryData
//
// Purpose: 
//   Gets the material object for the particles.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 16 18:05:10 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void *
avtLODIParticleFileFormat::GetAuxiliaryData(const char *var, int ts,
    const char *type, void *args, DestructorFunction &df)
{
    avtMaterial *retval = 0;

    if(strcmp(type, AUXILIARY_DATA_MATERIAL) == 0 &&
       strcmp(var, "sourceid") == 0)
    {
        df = avtMaterial::Destruct;

        // Figure out the max number of particles per time state.
        int nPts = 1;
        TypeEnum t = NO_TYPE;
        int ndims = 0, *dims = 0;
        int maxPointsPerSource = -1;
        if(fileObject->InqVariable("in_grid", &t, &ndims, &dims))
        {
            for(int i = 1; i < ndims; ++i)
                nPts *= dims[i];
            maxPointsPerSource = dims[ndims-1];
            delete [] dims;
        }
        if(maxPointsPerSource == -1)
            maxPointsPerSource = nPts;

        // Get the particle mask, which tells us which particles to keep
        // and which to throw out.
        bool *mask = GetParticleMask(ts);
        if(mask != 0)
        {
            // Use the particle mask to build a list of materials.
            intVector matlist;
            matlist.reserve(nPts);
            for(int i = 0; i < nPts; ++i)
            {
                if(mask[i])
                    matlist.push_back(i / maxPointsPerSource + 1);
            }

            // Create matnos and names arrays so we can create an avtMaterial.
            int *matnos = new int[sourceids.size()];
            char **names = new char *[sourceids.size()];
            for(int i = 0; i < sourceids.size(); ++i)
            {
                matnos[i] = i + 1;
                names[i] = (char *)sourceids[i].c_str();
            }

            // Create the avtMaterial.
            int mdims[1];
            mdims[0] = matlist.size();
            retval = new avtMaterial(
                sourceids.size(),
                matnos,
                names,
                1,
                mdims,
                0,
                &matlist[0],
                0,
                0,
                0,
                0,
                0
                );

            delete [] matnos;
            delete [] names;
        }
    }

    return retval;
}

// ****************************************************************************
// Method: avtLODIParticleFileFormat::GetFillValue
//
// Purpose: 
//   Returns the fill value that is used to blank out points.
//
// Arguments:
//   fill_value : The return fill value.
//
// Returns:    True if the fill value was read; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 16 18:19:19 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool
avtLODIParticleFileFormat::GetFillValue(float &fill_value)
{
    const char *mName = "avtLODIFileFormat::GetFillValue: ";
    bool haveFillValue = false;
    TypeEnum t = NO_TYPE;
    int ndims = 0, *dims = 0;
    void *values = 0;
    if(fileObject->ReadAttribute("part_posn", "fill_value", &t,
       &ndims, &dims, &values))
    {
        if(ndims == 1 && t == DOUBLEARRAY_TYPE)
        {
            double *d = (double *)values;
            fill_value = float(*d);
            haveFillValue = true;
            debug4 << mName << "part_posn fill value is: "
                   << fill_value << endl;
        }
        delete [] dims;
        free_void_mem(values, t);
    }

    return haveFillValue;
}

// ****************************************************************************
// Method: avtLODIParticleFileFormat::CreateParticleMask
//
// Purpose: 
//   Returns an array of bools (1 per point) indicating which points are good
//   and should remain in the visualization.
//
// Arguments:
//   ts : The time state for which we want the particle mask.
//
// Returns:    An array of bools or 0 if we could not create it.
//
// Note:       We check the part_posn array for values that have been blanked
//             using the fill_value and we check the in_grid array to make sure
//             that a point's value is 1.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 16 18:20:06 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool *
avtLODIParticleFileFormat::CreateParticleMask(int ts)
{
    const char *mName = "avtLODIParticleFileFormat::CreateParticleMask: ";
    bool *mask = 0;

    TypeEnum t = NO_TYPE;
    int ndims = 0, *dims = 0;
    if(fileObject->InqVariable("part_posn", &t, &ndims, &dims))
    {
        // Figure out the size of the chunk that we want to read.
        size_t *starts = new size_t[ndims];
        size_t *counts = new size_t[ndims];
        int nElems = 1;
        for(int i = 1; i < ndims; ++i)
        {
            starts[i] = 0;
            counts[i] = dims[i];
            nElems *= dims[i];
        }
        delete [] dims;
        starts[0] = ts;
        counts[0] = 1;
        int nPts = nElems / 3;

        //
        // Read the fill_value attribute if we can and use it to
        // eliminate points that should not be added to the mesh.
        //
        float fill_value;
        bool haveFillValue = GetFillValue(fill_value);

        if(haveFillValue)
        {
            int varId;
            fileObject->GetVarId("part_posn", &varId);
            float *points = new float[nElems];
            int status = nc_get_vara_float(fileObject->GetFileHandle(),
                                           varId, starts, counts, points);
            if(status == NC_NOERR)
            {
                debug4 << mName << "Using part_posn and fill_value to fill "
                       << "in particle mask for ts=" << ts << endl;
                mask = new bool[nPts];

                float *fptr = points;
                for (int i = 0 ; i < nPts ; i++)
                {
                    mask[i] = (fptr[0] != fill_value &&
                               fptr[1] != fill_value &&
                               fptr[2] != fill_value);
                    fptr += 3;
                }
            }
            else
                fileObject->HandleError(status);

            delete [] points;
        }

        if(mask == 0)
        {
            mask = new bool[nPts];
            for(int i = 0; i < nPts; ++i)
                mask[i] = true;
        }

        delete [] starts;
        delete [] counts;
    }

#ifdef USE_IN_GRID_IN_PARTICLE_MASK
    if(fileObject->InqVariable("in_grid", &t, &ndims, &dims))
    {
        // Figure out the size of the chunk that we want to read.
        size_t *starts = new size_t[ndims];
        size_t *counts = new size_t[ndims];
        int nElems = 1;
        for(int i = 1; i < ndims; ++i)
        {
            starts[i] = 0;
            counts[i] = dims[i];
            nElems *= dims[i];
        }
        delete [] dims;
        starts[0] = ts;
        counts[0] = 1;
        int nPts = nElems;

        int varId;
        fileObject->GetVarId("in_grid", &varId);
        int *in_grid = new int[nElems];
        int status = nc_get_vara_int(fileObject->GetFileHandle(),
                                     varId, starts, counts, in_grid);
        if(status != NC_NOERR)
            fileObject->HandleError(status);
        
        if(mask == 0)
        {
            mask = new bool[nPts];
            for(int i = 0; i < nPts; ++i)
                mask[i] = true;
        }

        // If we were able to read in_grid, only admit those that are
        // in the grid and are not set to the fill_value.
        if(in_grid != 0)
        {
            debug4 << mName << "Using in_grid to further fill "
                   << "in particle mask for ts=" << ts << endl;

            for(int i = 0; i < nPts; ++i)
                mask[i] &= (in_grid[i] == 1);
        }

        delete [] in_grid;
        delete [] starts;
        delete [] counts;
    }
#endif
    return mask;
}

// ****************************************************************************
// Method: avtLODIParticleFileFormat::GetParticleMask
//
// Purpose: 
//   Gets the particle mask for the specified time state, creating it and
//   caching it as necessary.
//
// Arguments:
//   ts : The time state for which to return the particle mask.
//
// Returns:    The particle mask array or 0.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 16 18:21:52 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool *
avtLODIParticleFileFormat::GetParticleMask(int ts)
{
    const char *mName = "avtLODIParticleFileFormat::GetParticleMask: ";
    const char *MASK_KEY = "mask key";

    char key[1000];
    SNPRINTF(key, 1000, "%s_particle_mask_ts=%d",
        fileObject->GetName().c_str(), ts);
    bool *retval = 0;

    void_ref_ptr vr = cache->GetVoidRef(key, MASK_KEY, ts, -1);
    if(*vr != 0)
    {
        debug4 << mName << "Found a cached particle mask for "
               << key << " at: " << (*vr) << endl;
        retval = (bool *)(*vr);
    }
    else
    {
        retval = CreateParticleMask(ts);
        if(retval != 0)
        {
            debug4 << mName << "Created a new particle mask for "
                   << key << endl;

            // Store it in the cache.
            void_ref_ptr vr2 = void_ref_ptr(retval, ParticleMaskDestruct);
            cache->CacheVoidRef(key, MASK_KEY, ts, -1, vr2);
        }
        else
             debug4 << mName << "Could not create particle mask" << endl;
    }

    return retval;
}

// ****************************************************************************
// Method: avtLODIParticleFileFormat::ParticleMaskDestruct
//
// Purpose: 
//   This method is called when the cache wants to delete its cached particle
//   masks.
//
// Arguments:
//   ptr : The particle mask array to delete.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 16 18:22:44 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtLODIParticleFileFormat::ParticleMaskDestruct(void *ptr)
{
    debug4 << "avtLODIParticleFileFormat::ParticleMaskDestruct: " << ptr << endl;
    bool *bptr = (bool *)ptr;
    delete [] bptr;
}
