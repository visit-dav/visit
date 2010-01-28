/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <avtLODIFileFormat.h>
#include <NETCDFFileObject.h>
#include <netcdf.h>

#include <avtDatabaseMetaData.h>
#include <avtMTSDFileFormatInterface.h>

#include <vtkRectilinearGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkShortArray.h>
#include <vtkIntArray.h>
#include <vtkLongArray.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>

#include <InvalidVariableException.h>
#include <DebugStream.h>

// ****************************************************************************
// Method: avtLODIFileFormat::Identify
//
// Purpose: 
//   This method checks to see if the file is a LODI file.
//
// Arguments:
//   fileObject : The file to check.
//
// Returns:    True if the file is a LODI file; False otherwise.
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
avtLODIFileFormat::Identify(NETCDFFileObject *fileObject)
{
    bool     isLODI = false;

    std::string create_code;
    if(fileObject->ReadStringAttribute("create_code", create_code))
    {
        isLODI = create_code == "LODI";
    }

    return isLODI;
}

// ****************************************************************************
// Method: avtLODIFileFormat::CreateInterface
//
// Purpose: 
//   This method creates a MTSD file format interface containing LODI 
//   file format readers.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 15 18:05:07 PST 2005
//
// Modifications:
//    Jeremy Meredith, Thu Jan 28 12:28:07 EST 2010
//    MTSD now accepts grouping multiple files into longer sequences, so
//    its interface has changed to accept both a number of timestep groups
//    and a number of blocks.
//   
// ****************************************************************************

avtFileFormatInterface *
avtLODIFileFormat::CreateInterface(NETCDFFileObject *f,
    const char *const *list, int nList, int nBlock)
{
    int nTimestepGroups = nList / nBlock;
    avtMTSDFileFormat ***ffl = new avtMTSDFileFormat**[nTimestepGroups];
    for (int i = 0 ; i < nTimestepGroups ; i++)
    {
        ffl[i] = new avtMTSDFileFormat*[nBlock];
        for (int j = 0 ; j < nBlock ; j++)
        {
            ffl[i][j] = NULL;
        }
    }

    TRY
    {
        for (int i = 0 ; i < nTimestepGroups ; i++)
        {
            for (int j = 0 ; j < nBlock ; j++)
            {
                ffl[i][j] = new avtLODIFileFormat(list[i*nBlock+j],
                                                  (i==0) ? f : NULL);
            }
        }
    }
    CATCH(VisItException)
    {
        for (int i = 0 ; i < nTimestepGroups ; i++)
        {
            for (int j = 0 ; j < nBlock ; j++)
            {
                if(ffl[i][j] != 0)
                    delete ffl[i][j];
            }
            delete [] ffl[i];
        }
        delete [] ffl;
        RETHROW;
    }
    ENDTRY

    return new avtMTSDFileFormatInterface(ffl, nTimestepGroups, nBlock);
}

// ****************************************************************************
// Method: avtLODIFileFormat::avtLODIFileFormat
//
// Purpose: 
//   Constructor for the avtLODIFileFormat file format.
//
// Arguments:
//   filename : The name of the file to read.
//   f        : The file object to use when reading the netcdf file.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 22 16:09:02 PST 2005
//
// Modifications:
//   
// ****************************************************************************

avtLODIFileFormat::avtLODIFileFormat(const char *filename) :
    avtMTSDFileFormat(&filename, 1), times()
{
    fileObject = new NETCDFFileObject(filename);
    timesRead = false;
}

avtLODIFileFormat::avtLODIFileFormat(const char *filename,
    NETCDFFileObject *f) : avtMTSDFileFormat(&filename, 1), times()
{
    fileObject = f;
    timesRead = false;
}

// ****************************************************************************
// Method: avtLODIFileFormat::~avtLODIFileFormat
//
// Purpose: 
//   Destructor for the avtLODIFileFormat class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 22 16:09:44 PST 2005
//
// Modifications:
//   
// ****************************************************************************

avtLODIFileFormat::~avtLODIFileFormat()
{
    FreeUpResources();

    delete fileObject;
}

// ****************************************************************************
// Method: avtLODIFileFormat::FreeUpResources
//
// Purpose: 
//   Called when the file format needs to free up resources.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 22 16:41:08 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtLODIFileFormat::FreeUpResources()
{
    debug4 << "avtLODIFileFormat::FreeUpResources" << endl;

    fileObject->Close();
}

// ****************************************************************************
// Method: avtLODIFileFormat::ActivateTimestep
//
// Purpose: 
//   Called to activate the specified time step.
//
// Arguments:
//   ts : The new active time step.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 22 16:41:29 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtLODIFileFormat::ActivateTimestep(int ts)
{
    debug4 << "avtLODIFileFormat::ActivateTimestep: ts=" << ts << endl;
    ReadTimes();
}

// ****************************************************************************
// Method: avtLODIFileFormat::ReadTimes
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
avtLODIFileFormat::ReadTimes()
{
    const char *mName = "avtLODIFileFormat::ReadTimes: ";
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
// Method: avtLODIFileFormat::GetNTimesteps
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
avtLODIFileFormat::GetNTimesteps()
{
    const char *mName = "avtLODIFileFormat::GetNTimesteps: ";
    debug4 << mName << endl;
    ReadTimes();
    debug4 << mName << "returning " << times.size() << endl;
    return times.size();
}

// ****************************************************************************
// Method: avtLODIFileFormat::GetTimes
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
avtLODIFileFormat::GetTimes(doubleVector &t)
{
    debug4 << "avtLODIFileFormat::GetTimes" << endl;
    ReadTimes();
    t = times;
}

// ****************************************************************************
// Method: avtLODIFileFormat::PopulateDatabaseMetaData
//
// Purpose: 
//   Returns information about the variables in the file.
//
// Arguments:
//   md : The metadata object to populate with variable names.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 22 16:15:51 PST 2005
//
// Modifications:
//    Jeremy Meredith, Thu Aug 25 12:55:29 PDT 2005
//    Added group origin to mesh metadata constructor.
//
//    Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//    Changed interface to DebugStream to obtain current debug level.
// ****************************************************************************

void
avtLODIFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    const char *mName = "avtLODIFileFormat::PopulateDatabaseMetaData: ";

    debug4 << mName << endl;
    if(DebugStream::Level4())
        fileObject->PrintFileContents(DebugStream::Stream4());

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

    TypeEnum t = NO_TYPE;
    int ndims = 0, *dims = 0;
    if(fileObject->InqVariable("concen", &t, &ndims, &dims))
    {
        std::string meshName("mesh");
        avtMeshMetaData *mmd = new avtMeshMetaData(meshName, 
                              1, 1, 1, 0, ndims-1, ndims-1, AVT_RECTILINEAR_MESH);
        fileObject->ReadStringAttribute("cgridx", "units", mmd->xUnits);
        fileObject->ReadStringAttribute("cgridy", "units", mmd->yUnits);
        md->Add(mmd);
        delete [] dims;

        int concen_ndims = 0, *concen_dims = 0;
        if(fileObject->InqVariable("concen", &t, &concen_ndims, &concen_dims))
        {
            avtScalarMetaData *smd = new avtScalarMetaData("concen",
                        meshName, AVT_ZONECENT);
            smd->hasUnits = fileObject->ReadStringAttribute("concen", "units", smd->units);
            md->Add(smd);
            delete [] concen_dims;
        }

        if(ndims-1 == 2)
        {
            int ac_ndims = 0, *ac_dims = 0;
            if(fileObject->InqVariable("area_cell", &t, &ac_ndims, &ac_dims))
            {
                avtScalarMetaData *smd = new avtScalarMetaData("area_cell",
                            meshName, AVT_ZONECENT);
                smd->hasUnits = fileObject->ReadStringAttribute("area_cell", "units", smd->units);
                md->Add(smd);
                delete [] ac_dims;
            }

            int zgij_ndims = 0, *zgij_dims = 0;
            if(fileObject->InqVariable("zgij", &t, &zgij_ndims, &zgij_dims))
            {
                avtScalarMetaData *smd = new avtScalarMetaData("zgij",
                            meshName, AVT_ZONECENT);
                smd->hasUnits = fileObject->ReadStringAttribute("zgij", "units", smd->units);
                md->Add(smd);
                delete [] zgij_dims;
            }
        }
    }
}

// ****************************************************************************
// Method: avtLODIFileFormat::GetMesh
//
// Purpose: 
//   This method returns the mesh for the specified time state.
//
// Arguments:
//   ts  : The time state for which to return the mesh.
//   var : The mesh to fetch.
//
// Returns:    A vtkDataSet or 0.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 22 17:21:35 PST 2005
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
avtLODIFileFormat::GetMesh(int ts, const char *var)
{
    const char *mName = "avtLODIFileFormat::GetMesh: ";
    debug4 << mName << "ts=" << ts
           << ", var=" << var << endl;
    vtkDataSet *retval = 0;

    if(strcmp(var, "mesh") == 0)
    {
        TypeEnum t = NO_TYPE;
        int concen_ndims = 0, *concen_dims = 0;
        if(fileObject->InqVariable("concen", &t, &concen_ndims, &concen_dims))
        {
            delete [] concen_dims;
            if(concen_ndims-1 == 2)
            {
                // Read the X and Y coordinates.
                bool err = false;
                TypeEnum xt = NO_TYPE, yt = NO_TYPE;
                int xndims = 0, yndims = 0, *xdims = 0, *ydims = 0;
                void *xvals = 0, *yvals = 0;
                fileObject->ReadVariable("cgridx", &xt, &xndims, &xdims, &xvals);
                fileObject->ReadVariable("cgridy", &yt, &yndims, &ydims, &yvals);
                if(xvals != 0 && xt == DOUBLEARRAY_TYPE &&
                   yvals != 0 && yt == DOUBLEARRAY_TYPE)
                {
                    //
                    // Populate the coordinates.
                    //
                    int    dims[3];
                    dims[0] = xdims[0];
                    dims[1] = ydims[0];
                    dims[2] = 1;
                    const double *coordinate[3];
                    coordinate[0] = (const double *)xvals;
                    coordinate[1] = (const double *)yvals;
                    coordinate[2] = 0;
                    const char *coordLabels[3] = {"X coords", "Y coords", "Z coords"};
                    vtkDoubleArray *coords[3];
                    for (int i = 0 ; i < 3 ; i++)
                    {
                        // Default number of components for an array is 1.
                        coords[i] = vtkDoubleArray::New();
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

                    retval = rgrid;
                }
                else
                    err = true;

                // Clean up
                delete [] xdims;
                if(xvals != 0)
                   free_void_mem(xvals, xt);
                delete [] ydims;
                if(yvals != 0)
                   free_void_mem(yvals, yt);

                if(err)
                {
                    if(xvals == 0)
                        debug4 << mName << "error reading cgridx" << endl;
                    if(yvals == 0)
                        debug4 << mName << "error reading cgridy" << endl;
                    if(xt != DOUBLEARRAY_TYPE)
                        debug4 << mName << "cgridx was not the right type" << endl;
                    if(yt != DOUBLEARRAY_TYPE)
                        debug4 << mName << "cgridy was not the right type" << endl;

                    std::string msg("\"The variable ");
                    msg += var;
                    msg += " could not be created. Contact a VisIt developer.\"";
                    EXCEPTION1(InvalidVariableException, msg);
                }
            }
            else
            {
                std::string msg("\"The variable ");
                msg += var;
                msg += " is not 2D. Contact a VisIt developer.\"";
                EXCEPTION1(InvalidVariableException, msg);
            }
        }
        else
        {
            EXCEPTION1(InvalidVariableException, var);
        }
    }
    else
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    return retval;
}

// ****************************************************************************
// Method: avtLODIFileFormat::GetVar
//
// Purpose: 
//   Reads a variable from the file and returns the values in a data array.
//
// Arguments:
//   ts  : The time state for which we want data.
//   var : The name of the variable to read.
//
// Returns:    A data array or 0.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 22 17:30:09 PST 2005
//
// Modifications:
//   
// ****************************************************************************

vtkDataArray *
avtLODIFileFormat::GetVar(int ts, const char *var)
{
    const char *mName = "avtLODIFileFormat::GetVar: ";
    debug4 << mName << "ts=" << ts
           << ", var=" << var << endl;

    vtkDataArray *retval = 0;
    bool variesOverTime = false;
    if(strcmp(var, "concen") == 0)
    {
        variesOverTime = true;
    }
    else if(strcmp(var, "area_cell") == 0 ||
            strcmp(var, "zgij") == 0)
    {
        variesOverTime = false;
    }
    else
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    TypeEnum t = NO_TYPE;
    int ndims = 0, *dims = 0;
    if(fileObject->InqVariable(var, &t, &ndims, &dims))
    {
        // Figure out the size of the chunk that we want to read.
        size_t *starts = new size_t[ndims];
        size_t *counts = new size_t[ndims];
        int nValues = 1;
        int xdimi = 0;
        int ydimi = 1;
        debug4 << mName << "var=" << var << " dims={";
        for(int i = 0; i < ndims; ++i)
        {
            starts[i] = 0;
            counts[i] = dims[i];
            nValues *= dims[i];
            if(i > 0)
                debug4 << ", ";
            debug4 << dims[i];
        }
        debug4 << "}" << endl;

        if(variesOverTime)
        {
            starts[0] = ts;
            counts[0] = 1;
            nValues /= dims[0];
            ++xdimi;
            ++ydimi;
        }

        int varId;
        fileObject->GetVarId(var, &varId);

#if 0
#define READVAR(VTK, T, FUNC) \
        {\
            VTK *arr = VTK::New();\
            arr->SetNumberOfTuples(nValues);\
            int status = FUNC(fileObject->GetFileHandle(),\
                              varId, starts, counts, (T *)arr->GetVoidPointer(0));\
            retval = arr;\
        }
#else
// Transpose the data in x,y so it looks like other related datasets
#define READVAR(VTK, T, FUNC) \
        {\
            T *data = new T[nValues];\
            VTK *arr = VTK::New();\
            int status = FUNC(fileObject->GetFileHandle(),\
                              varId, starts, counts, data);\
            arr->SetNumberOfTuples(nValues);\
            T *ptr = (T *)arr->GetVoidPointer(0);\
            int nx = dims[xdimi];\
            int ny = dims[ydimi];\
            for(int j = 0; j < ny; ++j)\
                for(int i = 0; i < nx; ++i)\
                {\
                    *ptr++ = data[i * ny + j];\
                }\
            delete [] data; \
            retval = arr;\
        }
#endif
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

        delete [] dims;
        delete [] starts;
        delete [] counts;
    }
    else
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    return retval;
}

