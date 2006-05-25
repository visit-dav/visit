#include <vector>
#include <snprintf.h>
#include <netcdf.h>

#include <avtBasicNETCDFFileFormat.h>
#include <NETCDFFileObject.h>
#include <avtDatabaseMetaData.h>
#include <avtSTSDFileFormatInterface.h>
#include <DebugStream.h>

#include <vtkCellArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>

#include <vtkUnsignedCharArray.h>
#include <vtkShortArray.h>
#include <vtkIntArray.h>
#include <vtkLongArray.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>

#include <InvalidVariableException.h>

// ****************************************************************************
// Method: avtBasicNETCDFFileFormat::CreateInterface
//
// Purpose: 
//   Creates the file format interface for this reader.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 18 18:03:39 PST 2005
//
// Modifications:
//   
// ****************************************************************************

avtFileFormatInterface *
avtBasicNETCDFFileFormat::CreateInterface(NETCDFFileObject *f, 
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
                ffl[i][j] = new avtBasicNETCDFFileFormat(list[i*nBlock + j], f);
                f = 0;
            }
            else
                ffl[i][j] = new avtBasicNETCDFFileFormat(list[i*nBlock + j]);
        }
    }

    return new avtSTSDFileFormatInterface(ffl, nTimestep, nBlock);
}

// ****************************************************************************
// Method: avtBasicNETCDFFileFormat::avtBasicNETCDFFileFormat
//
// Purpose: 
//   Constructor for the avtBasicNETCDFFileFormat class.
//
// Arguments:
//   filename : The name of the file being read.
//   f        : The file object associated with the file being read.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 18 18:03:59 PST 2005
//
// Modifications:
//   Brad Whitlock, Wed Apr 26 17:40:20 PST 2006
//   Initialized meshNamesCreated.
//
// ****************************************************************************

avtBasicNETCDFFileFormat::avtBasicNETCDFFileFormat(const char *filename) :
    avtSTSDFileFormat(filename),  meshNames()
{
    fileObject = new NETCDFFileObject(filename);
    meshNamesCreated = false;
}

avtBasicNETCDFFileFormat::avtBasicNETCDFFileFormat(const char *filename,
    NETCDFFileObject *f) : avtSTSDFileFormat(filename),  meshNames()
{
    fileObject = f;
    meshNamesCreated = false;
}

// ****************************************************************************
// Method: avtBasicNETCDFFileFormat::~avtBasicNETCDFFileFormat
//
// Purpose: 
//   Destructor for the avtBasicNETCDFFileFormat class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 18 18:04:36 PST 2005
//
// Modifications:
//   
// ****************************************************************************

avtBasicNETCDFFileFormat::~avtBasicNETCDFFileFormat()
{
    FreeUpResources();

    delete fileObject;
}

// ****************************************************************************
// Method: avtBasicNETCDFFileFormat::FreeUpResources
//
// Purpose: 
//   Frees up the resources.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 18 18:04:58 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtBasicNETCDFFileFormat::FreeUpResources()
{
    debug4 << "avtBasicNETCDFFileFormat::FreeUpResources" << endl;
    fileObject->Close();
}

// ****************************************************************************
// Method: avtBasicNETCDFFileFormat::GetCycleFromFilename
//
// Purpose: 
//   Make it guess the cycle from the filename.
//
// Returns:    The cycle.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 16 14:00:37 PST 2006
//
// Modifications:
//   
// ****************************************************************************

int
avtBasicNETCDFFileFormat::GetCycleFromFilename(const char *f) const
{
    return GuessCycle(f);
}

// ****************************************************************************
// Method: avtBasicNETCDFFileFormat::ActivateTimestep
//
// Purpose: 
//   Activates the time step.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 18 18:05:14 PST 2005
//
// Modifications:
//
// ****************************************************************************

void
avtBasicNETCDFFileFormat::ActivateTimestep()
{
    debug4 << "avtBasicNETCDFFileFormat::ActivateTimestep" << endl;
}

// ****************************************************************************
// Method: avtBasicNETCDFFileFormat::PopulateDatabaseMetaData
//
// Purpose: 
//   Populates the metadata from information in the file.
//
// Arguments:
//   md : The metadata object to populate.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 18 18:05:29 PST 2005
//
// Modifications:
//    Jeremy Meredith, Thu Aug 25 12:55:29 PDT 2005
//    Added group origin to mesh metadata constructor.
//
//    Brad Whitlock, Wed Apr 26 17:53:24 PST 2006
//    I made it possible to call with a NULL metadata pointer so we can call
//    this method on ActivateTimestep to ensure that the meshNames map
//    is populated. I also added a check to set the validVariable flag for
//    meshes and variables so that we don't have problems later. By adding
//    the valid variable check, I was able to make 4+ dimensional arrays
//    safely appear in the variable lists.
//
// ****************************************************************************

void
avtBasicNETCDFFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    debug4 << "avtBasicNETCDFFileFormat::PopulateDatabaseMetaData" << endl;
    if(debug4_real)
        fileObject->PrintFileContents(debug4_real);

    int status, nDims, nVars, nGlobalAtts, unlimitedDimension;
    status = nc_inq(fileObject->GetFileHandle(), &nDims, &nVars, &nGlobalAtts,
                    &unlimitedDimension);
    if(status != NC_NOERR)
    {
        fileObject->HandleError(status);
        return;
    }

    // Get the size of all of the dimensions in the file.
    size_t *dimSizes = new size_t[nDims];
    int i;
    for(i = 0; i < nDims; ++i)
    {
        char   dimName[NC_MAX_NAME+1];
        size_t dimSize;
        if((status = nc_inq_dim(fileObject->GetFileHandle(), i, dimName,
           &dimSize)) == NC_NOERR)
        {
            dimSizes[i] = dimSize;
        }
        else
        {
            dimSizes[i] = 1;
            fileObject->HandleError(status);
        }
    }

    // Iterate over the variables and create a list of meshes names and add
    // the variable to the metadata.
    meshNames.clear();
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
            // Count the number of variable dimensions that are greater
            // than 1.
            int nDims = 0;
            int maxDim = dimSizes[vardims[0]];
            int maxDimIndex = 0;
            for(int dim = 0; dim < varndims; ++dim)
            {
                int d = dimSizes[vardims[dim]];
                if(d > 1)
                    ++nDims;
                if(d > maxDim)
                {
                    maxDim = d;
                    maxDimIndex = dim;
                }
            }

            if(nDims == 1)
            {
                if(maxDim > 1 && 
                   (vartype == NC_INT ||
                    vartype == NC_SHORT||
                    vartype == NC_FLOAT ||
                    vartype == NC_DOUBLE)
                  )
                {
                    if(md != 0)
                    {
                        avtCurveMetaData *cmd = new avtCurveMetaData;
                        cmd->name = varname;
                        cmd->yLabel = varname;
                        fileObject->ReadStringAttribute(varname, "units",
                            cmd->yUnits);

                        char dimName[NC_MAX_NAME+1];
                        status = nc_inq_dimname(fileObject->GetFileHandle(),
                                                maxDimIndex, dimName);
                        if(status == NC_NOERR)
                        {
                            cmd->xLabel = dimName;
                            fileObject->ReadStringAttribute(dimName, "units",
                                                            cmd->xUnits);
                        }
                        md->Add(cmd);
                    }

                    intVector meshDims; meshDims.push_back(maxDim);
                    meshNames[varname] = meshDims;
                }
            }
            else
            {
                char tmp[100];
                std::string meshName("mesh");
                intVector meshDims;
                int j, elems = 1;
                for(int j = varndims-1; j >= 0; --j)
                {
                    int d = dimSizes[vardims[j]];
                    elems *= d;
                    meshDims.push_back(d);
                }

                // Remove all of the trailing 1's in the meshDims array.
                while(meshDims.size() > 0 &&
                      meshDims[meshDims.size()-1] == 1)
                {
                    meshDims.pop_back();
                }

                // Make sure it's not 1 or 0 in some of its dimensions
                if(meshDims.size() >= 2 && elems != maxDim && elems > 0)
                {
                    // Come up with the mesh name that we'll use for this
                    // variable.
                    for(int j = 0; j < meshDims.size(); ++j)
                    {
                        if(j != 0)
                            SNPRINTF(tmp, 100, "x%d", meshDims[j]);
                        else
                            SNPRINTF(tmp, 100, "%d", meshDims[j]);
                        meshName += tmp;
                    }

                    // Return the dimensions that we should care about.
                    int validDims[3];
                    int nValidDims;
                    bool valid = ReturnValidDimensions(meshDims, 
                         validDims, nValidDims);

                    // Add the name of the mesh to the list of meshes.
                    if(meshNames.find(meshName) == meshNames.end())
                    {
                        meshNames[meshName] = meshDims;

                        if(md != 0)
                        {
                            avtMeshMetaData *mmd = new avtMeshMetaData(meshName, 
                                1, 1, 1, 0, nValidDims, nValidDims,
                                AVT_RECTILINEAR_MESH);
                            mmd->validVariable = valid;
                            md->Add(mmd);
                        }
                    }

                    // Try and get the variable units.
                    if(md != 0)
                    {
                        avtScalarMetaData *smd = new avtScalarMetaData(varname, meshName,
                            AVT_NODECENT);
                        smd->hasUnits = fileObject->ReadStringAttribute(
                            varname, "units", smd->units);
                        smd->validVariable = valid;
                        md->Add(smd);
                    }
                } 
            }
        }
    }

    meshNamesCreated = true;
}

// ****************************************************************************
// Method: avtBasicNETCDFFileFormat::ReturnValidDimensions
//
// Purpose: 
//   Returns the valid dimensions.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 27 10:36:12 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
avtBasicNETCDFFileFormat::ReturnValidDimensions(const intVector &dims, int validDims[3], int &nValidDims)
{
    int i;
    const char *mName = "avtBasicNETCDFFileFormat::ReturnValidDimensions: ";

    // Look for a valid 3rd dimension.
    nValidDims = 2;
    validDims[0] = dims[0];
    validDims[1] = dims[1];
    for(i = 2; i < dims.size() && nValidDims == 2; ++i)
    {
        if(dims[i] > 1)
        {
            validDims[nValidDims++] = dims[i];
        }
    }

    int nCells = 1;
    debug4 << mName << "validDims=(";
    for(i = 0; i < nValidDims; ++i)
    {
        nCells *= validDims[i];
        debug4 << validDims[i] << ", ";
    }
    debug4 << ")" << endl;

    int nValues = 1;
    debug4 << mName << "actualDims=(";
    for(i = 0; i < dims.size(); ++i)
    {
        nValues *= dims[i];
        debug4 << dims[i] << ", ";
    }
    debug4 << ")" << endl;

    return nCells == nValues;
}

// ****************************************************************************
// Method: avtBasicNETCDFFileFormat::GetMesh
//
// Purpose: 
//   Returns the specified mesh.
//
// Arguments:
//   var : The name of the mesh to create.
//
// Returns:    A vtkDataSet containing the mesh or 0.
// 
// Programmer: Brad Whitlock
// Creation:   Thu Aug 18 18:05:59 PST 2005
//
// Modifications:
//   Brad Whitlock, Wed Apr 26 17:56:15 PST 2006
//   I made it call PopulateDatabaseMetaData.
//
// ****************************************************************************

vtkDataSet *
avtBasicNETCDFFileFormat::GetMesh(const char *var)
{
    debug4 << "avtBasicNETCDFFileFormat::GetMesh: var=" << var << endl;
    vtkDataSet *retval = 0;
 
    // Populate the mesh names if we've not done so yet.
    if(!meshNamesCreated)
        PopulateDatabaseMetaData(0);

    MeshNameMap::const_iterator mesh = meshNames.find(var);
    if(mesh != meshNames.end())
    {
        if(mesh->second.size() == 1)
        {
            // Try and read the variable making up the curve.
            int nPts = mesh->second[0];
            float *yValues = new float[nPts];
            if(fileObject->ReadVariableIntoAsFloat(var, yValues))
            {
                vtkPolyData *pd = vtkPolyData::New();
                vtkPoints   *pts = vtkPoints::New();
                pd->SetPoints(pts);
                pts->SetNumberOfPoints(nPts);
                for (int j = 0 ; j < nPts ; j++)
                {
                    pts->SetPoint(j, float(j), yValues[j], 0.f);
                }
 
                //
                // Connect the points up with line segments.
                //
                vtkCellArray *line = vtkCellArray::New();
                pd->SetLines(line);
                for (int k = 1 ; k < nPts ; k++)
                {
                    line->InsertNextCell(2);
                    line->InsertCellPoint(k-1);
                    line->InsertCellPoint(k);
                }

                pts->Delete();
                line->Delete();
                retval = pd;
            }

            delete [] yValues;
        }
        else
        {
            int   i, j;
            vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New(); 

            // Return the dimensions that we should care about.
            int validDims[3];
            int nValidDims;
            ReturnValidDimensions(mesh->second, validDims, nValidDims);

            //
            // Populate the coordinates.  Put in 3D points with z=0 if
            // the mesh is 2D.
            //
            int            dims[3];
            vtkFloatArray *coords[3];
            for (i = 0 ; i < 3 ; i++)
            {
                // Default number of components for an array is 1.
                coords[i] = vtkFloatArray::New();

                if (i < nValidDims)
                {
                    dims[i] = validDims[i];
                    coords[i]->SetNumberOfTuples(dims[i]);
                    for (j = 0 ; j < dims[i] ; j++)
                    {
                        coords[i]->SetComponent(j, 0, j);
                    }
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

            retval = rgrid;
        }
    }
    else
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    return retval;
}

// ****************************************************************************
// Method: avtBasicNETCDFFileFormat::GetVar
//
// Purpose: 
//   Returns the data for the specified variable.
//
// Arguments:
//   var : The name of the variable to read.
//
// Returns:    The data or 0.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 18 18:06:49 PST 2005
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
avtBasicNETCDFFileFormat::GetVar(const char *var)
{
    debug4 << "avtBasicNETCDFFileFormat::GetVar: var=" << var << endl;

    vtkDataArray *retval = 0;
    TypeEnum t = NO_TYPE;
    int ndims = 0;
    int *dims = 0;
   
    if(fileObject->InqVariable(var, &t, &ndims, &dims))
    {
        debug4 << "avtBasicNETCDFFileFormat::GetVar: var=" << var << ", dims={";
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

