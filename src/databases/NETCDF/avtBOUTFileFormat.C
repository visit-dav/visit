/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#include <avtBOUTFileFormat.h>
#include <NETCDFFileObject.h>
#include <netcdf.h>

#include <algorithm>
#include <cmath>

#include <avtDatabaseMetaData.h>
#include <avtMTMDFileFormatInterface.h>
#include <avtGhostData.h>

#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkLongArray.h>
#include <vtkShortArray.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>

#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <NonCompliantFileException.h>

#include <visit-config.h>

#include <string.h>

static int oneXGridDomainToSubgrid[4] = {0, 1, 0, 1};
static int twoXGridDomainToSubgrid[6] = {0, 1, 0, 2, 1, 2};

#define READSCALARINTO(VARNAME, VAR) \
    {\
        TypeEnum t = NO_TYPE;\
        int ndims = 0, *dims = 0;\
        void *vals = 0;\
        if (meshFile->ReadVariable(VARNAME, &t, &ndims, &dims, &vals))\
        {\
            if (ndims != 0) \
            {\
                std::string msg;\
                msg = std::string("Error reading ") + std::string(VARNAME) + std::string(": Not a scalar");\
                EXCEPTION2(NonCompliantFileException, "BOUT", msg);\
            }\
            delete [] dims;\
            if (t != INTEGERARRAY_TYPE && t != SHORTARRAY_TYPE) \
            {\
                std::string msg;\
                msg = std::string("Error reading ") + std::string(VARNAME) + std::string(": Not an int or short");\
                EXCEPTION2(NonCompliantFileException, "BOUT", msg);\
            }\
            if (t == INTEGERARRAY_TYPE) \
            {\
                VAR = ((int*)vals)[0];\
                delete [] (int*) vals;\
            }\
            else if (t == SHORTARRAY_TYPE) \
            {\
                VAR = ((short*)vals)[0];\
                delete [] (short*) vals; \
            }\
        }\
        else\
        {\
            std::string msg;\
            msg = std::string("Error reading ") + std::string(VARNAME);\
            EXCEPTION2(NonCompliantFileException, "BOUT", msg);\
        }\
    }
// ****************************************************************************
// Method: avtBOUTFileFormat::Identify
//
// Purpose: 
//   This method checks to see if the file is a BOUT file.
//
// Arguments:
//   fileObject : The file to check.
//
// Returns:    True if the file is a BOUT file; False otherwise.
//
// Note:       
//
// Programmer: Eric Brugger
// Creation:   Thu Aug  1 16:42:56 PDT 2013
//
// Modifications:
//   
// ****************************************************************************

bool
avtBOUTFileFormat::Identify(NETCDFFileObject *fileObject)
{
    bool     isBOUT = false;

    TypeEnum t = NO_TYPE;
    int ndims = 0, *dims = 0;
    if (fileObject->InqVariable("gridname", &t, &ndims, &dims))
    {
        delete [] dims;
        t = NO_TYPE;
        ndims = 0; dims = 0;
        if (fileObject->InqVariable("zperiod", &t, &ndims, &dims))
        {
            delete [] dims;
            isBOUT = true;
        }
    }

    return isBOUT;
}

// ****************************************************************************
// Method: avtBOUTFileFormat::CreateInterface
//
// Purpose: 
//   This method creates a MTSD file format interface containing BOUT 
//   file format readers.
//
// Programmer: Eric Brugger
// Creation:   Thu Aug  1 16:42:56 PDT 2013
//
// Modifications:
//   
// ****************************************************************************

avtFileFormatInterface *
avtBOUTFileFormat::CreateInterface(NETCDFFileObject *f,
    const char *const *list, int nList, int nBlock)
{
    int nTimestepGroups = nList / nBlock;
    avtMTMDFileFormat **ffl = new avtMTMDFileFormat*[nTimestepGroups];
    for (int i = 0 ; i < nTimestepGroups ; ++i)
        ffl[i] = 0;
    TRY
    {
        for (int i = 0 ; i < nTimestepGroups ; ++i)
        {
            ffl[i] = new avtBOUTFileFormat(list[i*nBlock], (i==0) ? f : NULL);
        }
    }
    CATCH(VisItException)
    {
        for (int i = 0 ; i < nTimestepGroups ; ++i)
        {
            if (ffl[i] != 0)
                delete ffl[i];
        }
        delete [] ffl;
        RETHROW;
    }
    ENDTRY

    return new avtMTMDFileFormatInterface(ffl, nTimestepGroups);
}

// ****************************************************************************
// Method: avtBOUTFileFormat::avtBOUTFileFormat
//
// Purpose: 
//   Constructor for the avtBOUTFileFormat file format.
//
// Arguments:
//   filename : The name of the file to read.
//   f        : The file object to use when reading the netcdf file.
//
// Programmer: Eric Brugger
// Creation:   Thu Aug  1 16:42:56 PDT 2013
//
// Modifications:
//   Eric Brugger, Mon Dec  2 15:44:28 PST 2013
//   I added the ability to handle circular grids.
//   
// ****************************************************************************

avtBOUTFileFormat::avtBOUTFileFormat(const char *filename) :
    avtMTMDFileFormat(filename)
{
    const char *lastSlash = strrchr(filename, VISIT_SLASH_CHAR);
    filePath = new char[lastSlash - filename + 1];
    memcpy (filePath, filename, lastSlash - filename);
    filePath[lastSlash - filename] = '\0';

    fileObject = new NETCDFFileObject(filename);
    meshFile = 0;
    timesRead = false;
    meshRead = false;

    Rxy = 0;
    Zxy = 0;
    zShift = 0;
    zShiftZero = 0;

    nxRaw = 0;
    nyRaw = 0;
    nz = 0;
    nzOut = 0;

    for (int i = 0; i < MAX_SUB_MESHES; ++i)
    {
        subgrid[i].ijindex = 0;
        subgrid[i].jindex  = 0;
        subgrid[i].inrep   = 0;
        subgrid[i].jnrep   = 0;
    }

    cacheDataRaw = 0;
    for (int i = 0; i < MAX_SUB_MESHES; ++i)
        cacheData[i] = 0;
    cacheTime = -1;
}

avtBOUTFileFormat::avtBOUTFileFormat(const char *filename,
    NETCDFFileObject *f) : avtMTMDFileFormat(filename)
{
    const char *lastSlash = strrchr(filename, VISIT_SLASH_CHAR);
    filePath = new char[lastSlash - filename + 1];
    memcpy (filePath, filename, lastSlash - filename);
    filePath[lastSlash - filename] = '\0';

    fileObject = f;
    meshFile = 0;
    timesRead = false;
    meshRead = false;

    Rxy = 0;
    Zxy = 0;
    zShift = 0;
    zShiftZero = 0;

    nxRaw = 0;
    nyRaw = 0;
    nz = 0;
    nzOut = 0;

    for (int i = 0; i < MAX_SUB_MESHES; ++i)
    {
        subgrid[i].ijindex = 0;
        subgrid[i].jindex  = 0;
        subgrid[i].inrep   = 0;
        subgrid[i].jnrep   = 0;
    }

    cacheDataRaw = 0;
    for (int i = 0; i < MAX_SUB_MESHES; ++i)
        cacheData[i] = 0;
    cacheTime = -1;
}

// ****************************************************************************
// Method: avtBOUTFileFormat::~avtBOUTFileFormat
//
// Purpose: 
//   Destructor for the avtBOUTFileFormat class.
//
// Programmer: Eric Brugger
// Creation:   Thu Aug  1 16:42:56 PDT 2013
//
// Modifications:
//   Eric Brugger, Mon Dec  2 15:44:28 PST 2013
//   I added the ability to handle circular grids.
//   
// ****************************************************************************

avtBOUTFileFormat::~avtBOUTFileFormat()
{
    FreeUpResources();

    delete [] filePath;

    delete fileObject;
    if (meshFile != 0) delete meshFile;

    if (Rxy != 0) delete [] Rxy;
    if (Zxy != 0) delete [] Zxy;
    if (zShift != 0) delete [] zShift;
    if (zShiftZero != 0) delete [] zShiftZero;

    for (int i = 0; i < MAX_SUB_MESHES; ++i)
    {
        if (subgrid[i].ijindex != 0) delete [] subgrid[i].ijindex;
        if (subgrid[i].jindex  != 0) delete [] subgrid[i].jindex;
        if (subgrid[i].inrep   != 0) delete [] subgrid[i].inrep;
        if (subgrid[i].jnrep   != 0) delete [] subgrid[i].jnrep;
    }

    if (cacheDataRaw != 0) delete [] cacheDataRaw;
    for (int i = 0; i < MAX_SUB_MESHES; ++i)
        if (cacheData[i] != 0) cacheData[i]->Delete();
}

// ****************************************************************************
// Method: avtBOUTFileFormat::FreeUpResources
//
// Purpose: 
//   Called when the file format needs to free up resources.
//
// Programmer: Eric Brugger
// Creation:   Thu Aug  1 16:42:56 PDT 2013
//
// Modifications:
//   
// ****************************************************************************

void
avtBOUTFileFormat::FreeUpResources()
{
    debug4 << "avtBOUTFileFormat::FreeUpResources" << endl;

    fileObject->Close();
    if (meshFile != 0) meshFile->Close();
}

// ****************************************************************************
// Method: avtBOUTFileFormat::ActivateTimestep
//
// Purpose: 
//   Called to activate the specified time step.
//
// Arguments:
//   ts : The new active time step.
//
// Programmer: Eric Brugger
// Creation:   Thu Aug  1 16:42:56 PDT 2013
//
// Modifications:
//   
// ****************************************************************************

void
avtBOUTFileFormat::ActivateTimestep(int ts)
{
    debug4 << "avtBOUTFileFormat::ActivateTimestep: ts=" << ts << endl;

    ReadTimes();
}

// ****************************************************************************
// Method: avtBOUTFileFormat::ReadTimes
//
// Purpose: 
//   Reads in the times to be returned by this reader.
//
// Returns:    True if the times were read; False otherwise.
//
// Programmer: Eric Brugger
// Creation:   Thu Aug  1 16:42:56 PDT 2013
//
// Modifications:
//   
// ****************************************************************************

bool
avtBOUTFileFormat::ReadTimes()
{
    const char *mName = "avtBOUTFileFormat::ReadTimes: ";
    debug4 << mName << endl;

    if (!timesRead)
    {
        double timescale = 1.;
        fileObject->ReadVariableInto("timescale", DOUBLEARRAY_TYPE, &timescale);

        TypeEnum t = NO_TYPE;
        int ndims = 0, *dims = 0;
        if (fileObject->InqVariable("timestep", &t, &ndims, &dims))
        {
            if (t == FLOATARRAY_TYPE && ndims == 1)
            {
                float *timestep = new float[dims[0]];
                if (fileObject->ReadVariableInto("timestep", t, timestep))
                {
                    for (int i = 0; i < dims[0]; ++i)
                        times.push_back(((float *)timestep)[i]*timescale);
                }
                delete [] timestep;
            }
            else
            {
                debug4 << mName << "Error reading timestep." << endl;
            }
            delete [] dims;
        }
        else
        {
            debug4 << mName << "Error reading timestep." << endl;
        }
        timesRead = true;
    }

    return timesRead;
}

// ****************************************************************************
// Method: avtBOUTFileFormat::GetNTimesteps
//
// Purpose: 
//   Returns the number of time states in this database.
//
// Returns:    The number of time states.
//
// Programmer: Eric Brugger
// Creation:   Thu Aug  1 16:42:56 PDT 2013
//
// Modifications:
//   
// ****************************************************************************

int
avtBOUTFileFormat::GetNTimesteps()
{
    const char *mName = "avtBOUTFileFormat::GetNTimesteps: ";
    debug4 << mName << endl;

    ReadTimes();

    debug4 << mName << "returning " << times.size() << endl;
    return times.size();
}

// ****************************************************************************
// Method: avtBOUTFileFormat::GetTimes
//
// Purpose: 
//   Returns the times in the file.
//
// Arguments:
//   t : The times to be returned.
//
// Programmer: Eric Brugger
// Creation:   Thu Aug  1 16:42:56 PDT 2013
//
// Modifications:
//   
// ****************************************************************************

void
avtBOUTFileFormat::GetTimes(doubleVector &t)
{
    debug4 << "avtBOUTFileFormat::GetTimes" << endl;

    ReadTimes();

    t = times;
}

// ****************************************************************************
// Method: avtBOUTFileFormat::PopulateDatabaseMetaData
//
// Purpose: 
//   Returns information about the variables in the file.
//
// Arguments:
//   md : The metadata object to populate with variable names.
//
// Programmer: Eric Brugger
// Creation:   Thu Aug  1 16:42:56 PDT 2013
//
// Modifications:
//   Eric Brugger, Mon Dec  2 15:44:28 PST 2013
//   I added the ability to handle circular grids.
//
//   Eric Brugger, Tue Dec  3 10:23:53 PST 2013
//   I added the ability to handle grids with two X points.
//
//   Eric Brugger, Fri Apr 11 10:33:08 PDT 2014
//   I modified the creation of the diverter to include all of the lower
//   diverter as well as create the upper diverter with a two X point grid.
//
//   Eric Brugger, Mon Sep 22 16:45:10 PDT 2014
//   I modified the routine to handle zperiod being a char, short, int, long,
//   float or double. I also added a check to make sure zperiod was valid.
//
// ****************************************************************************

void
avtBOUTFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    const char *mName = "avtBOUTFileFormat::PopulateDatabaseMetaData: ";

    debug4 << mName << endl;
    if (DebugStream::Level4())
        fileObject->PrintFileContents(DebugStream::Stream4());

    int status, nDims, nVars, nGlobalAtts, unlimitedDimension;
    status = nc_inq(fileObject->GetFileHandle(), &nDims, &nVars, &nGlobalAtts,
                    &unlimitedDimension);
    debug4 << "nDims=" << nDims
           << ", nVars=" << nVars
           << ", nGlobalAtts=" << nGlobalAtts
           << ", unlimitedDimension=" << unlimitedDimension
           << endl;
    if (status != NC_NOERR)
    {
        fileObject->HandleError(status);
        return;
    }

    //
    // Read the zperiod.
    //
    zperiod = -1;
    TypeEnum t = NO_TYPE;
    int ndims = 0, *dims = 0;
    void *vals = 0;
    if (fileObject->ReadVariable("zperiod", &t, &ndims, &dims, &vals))
    {
        if (t == CHARARRAY_TYPE || t == UCHARARRAY_TYPE)
        {
            if (ndims == 0)
                zperiod = ((unsigned char*)vals)[0];
            delete [] (unsigned char*) vals;
        }
        else if (t == SHORTARRAY_TYPE)
        {
            if (ndims == 0)
                zperiod = ((short*)vals)[0];
            delete [] (short*) vals;
        }
        else if (t == INTEGERARRAY_TYPE)
        {
            if (ndims == 0)
                zperiod = ((int*)vals)[0];
            delete [] (int*) vals;
        }
        else if (t == LONGARRAY_TYPE)
        {
            if (ndims == 0)
                zperiod = ((long*)vals)[0];
            delete [] (long*) vals;
        }
        else if (t == FLOATARRAY_TYPE)
        {
            if (ndims == 0)
                zperiod = ((float*)vals)[0];
            delete [] (float*) vals;
        }
        else if (t == DOUBLEARRAY_TYPE)
        {
            if (ndims == 0)
                zperiod = ((double*)vals)[0];
            delete [] (double*) vals;
        }
        delete [] dims;
    }
    else
    {
        debug4 << mName << "Error reading zperiod." << endl;
    }

    //
    // Check that zperiod is valid.
    //
    if (zperiod <= 0)
    {
        zperiod = 1;
        debug4 << mName << "Warning: zperiod is <= zero, setting zperiod = 1." << endl;
    } 
    else if (zperiod > 50)
    {
        debug4 << mName << "Warning: zperiod is greater than 50." << endl;
    } 

    //
    // Read the mesh meta data so that we can determine the type of
    // mesh.
    //
    ReadMeshMetaData();
    if (jyseps1_1 == -1)
    {
        gridType = circularGrid;
        nSubMeshes = 1;
    }
    else if (jyseps2_1 == jyseps1_2)
    {
        gridType = oneXGrid;
        nSubMeshes = 4;
    }
    else
    {
        gridType = twoXGrid;
        nSubMeshes = 7;
    }

    //
    // Create some standard mesh and variable names.
    //
    std::string zShiftName("zShift");
    std::string meshName("mesh");
    std::string meshNameZShift("mesh_zshift");
    std::string meshName2D("mesh_2d");
    std::string meshNameDiverter("mesh_diverter");

    //
    // Loop over all the variables and if the variable has 4 dimensions
    // and is of type float, then add it to the list of variables.
    //
    for (int i = 0; i < nVars; ++i)
    {
        char varname[NC_MAX_NAME+1];
        nc_type vartype;
        int  varndims;
        int  vardims[NC_MAX_VAR_DIMS];
        int  varnatts;
        if ((status = nc_inq_var(fileObject->GetFileHandle(), i, varname,
                                 &vartype, &varndims,
                                 vardims, &varnatts)) == NC_NOERR)
        {
            if (varndims == 4 && vartype == NC_FLOAT)
            {
                std::string varName(varname);
                avtScalarMetaData *smd = new avtScalarMetaData(varName,
                    meshName, AVT_NODECENT);
                md->Add(smd);

                std::string varNameZShift = varName + "_zshift";
                smd = new avtScalarMetaData(varNameZShift,
                    meshNameZShift, AVT_NODECENT);
                md->Add(smd);

                if (gridType != circularGrid)
                {
                    std::string varNameDiverter= varName + "_diverter";
                    smd = new avtScalarMetaData(varNameDiverter,
                        meshNameDiverter, AVT_NODECENT);
                    md->Add(smd);
                }

                size_t mz;
                if ((status = nc_inq_dimlen(fileObject->GetFileHandle(),
                                            vardims[3], &mz)) == NC_NOERR)
                {
                    nz = mz;
                    nzOut = mz + 1;
                }
            }
        }
    }

    //
    // Add the standard meshes and variables present in all BOUT files.
    //
    avtScalarMetaData *smd = new avtScalarMetaData(zShiftName,
                meshName2D, AVT_NODECENT);
    md->Add(smd);

    avtMeshMetaData *mmd = new avtMeshMetaData(meshName, nSubMeshes * zperiod,
        1, 1, 0, 3, 3, AVT_CURVILINEAR_MESH);
    md->Add(mmd);

    mmd = new avtMeshMetaData(meshNameZShift, nSubMeshes * zperiod,
        1, 1, 0, 3, 3, AVT_CURVILINEAR_MESH);
    md->Add(mmd);

    mmd = new avtMeshMetaData(meshName2D, nSubMeshes,
        1, 1, 0, 2, 2, AVT_CURVILINEAR_MESH);
    md->Add(mmd);

    if (gridType == oneXGrid)
    {
        mmd = new avtMeshMetaData(meshNameDiverter,
            N_DIVERTER_ONEX_SUB_MESHES * zperiod, 1, 1, 0, 3, 3,
            AVT_CURVILINEAR_MESH);
        md->Add(mmd);
    }
    else if (gridType == twoXGrid)
    {
        mmd = new avtMeshMetaData(meshNameDiverter,
            N_DIVERTER_TWOX_SUB_MESHES * zperiod, 1, 1, 0, 3, 3,
            AVT_CURVILINEAR_MESH);
        md->Add(mmd);
    }
}

// ****************************************************************************
// Method: avtBOUTFileFormat::DetermineMeshReplication
//
// Purpose: 
//   This method determines the amount to replicate each row of zones so that
//   the grid is smooth when zshift is applied.
//
// Arguments:
//
// Programmer: Eric Brugger
// Creation:   Thu Aug  1 16:42:56 PDT 2013
//
// Modifications:
//   
// ****************************************************************************

void
avtBOUTFileFormat::DetermineMeshReplication(Subgrid &grid)
{
    //
    // Determine the size of the input grid.
    //
    grid.nxIn = grid.iend - grid.istart;
    grid.nyIn = 0;
    for (int ib = 0; ib < grid.nb; ++ib)
    {
        grid.nyIn += abs(grid.jend[ib] - grid.jstart[ib]);
    }

    //
    // Special case for the third sub grid.
    //
    if (grid.nb == 2 && grid.istart + 1 == grid.iend)
    {
        grid.nxIn = 2;
        grid.nyIn = 2;
    }

    int nxIn = grid.nxIn;
    int nyIn = grid.nyIn;

    grid.ijindex = new int[nxIn*nyIn]; 
    grid.jindex  = new int[nyIn]; 
    grid.inrep   = new int[nxIn-1]; 
    grid.jnrep   = new int[nyIn-1]; 
    int *ijindex = grid.ijindex;
    int *jindex  = grid.jindex;
    int *inrep   = grid.inrep;
    int *jnrep   = grid.jnrep;

    int nb;
    int istart, iend;
    int jstart[2], jend[2];
    nb        = grid.nb;
    istart    = grid.istart;
    iend      = grid.iend;
    jstart[0] = grid.jstart[0];
    jstart[1] = grid.jstart[1];
    jend[0]   = grid.jend[0];
    jend[1]   = grid.jend[1];

    int jsum = 0;
    for (int ib = 0; ib < nb; ++ib)
    {
        for (int j = jstart[ib]; j < jend[ib]; ++j)
        {
            jindex[jsum] = j;
            jsum++;
        }
    }

    jsum = 0;
    for (int i = istart; i < iend; ++i)
    {
        for (int ib = 0; ib < nb; ++ib)
        {
            if (jstart[ib] < jend[ib])
            {
                for (int j = jstart[ib]; j < jend[ib]; ++j)
                {
                    ijindex[jsum] = i * ny2d + j;
                    jsum++;
                }
            }
            else
            {
                for (int j = jstart[ib]; j > jend[ib]; --j)
                {
                    ijindex[jsum] = i * ny2d + j;
                    jsum++;
                }
            }
        }
    }

    for (int j = 0; j < nyIn - 1; ++j)
    {
        double maxDeltaAngle = 0.;
        for (int i = istart; i < iend; ++i)
        {
            int ipt1 = i * ny2d + jindex[j];
            int ipt2 = i * ny2d + jindex[j+1];
            double theta  = zShift[ipt1];
            double theta2 = zShift[ipt2];
            double delta  = fabs(theta2 - theta);
            maxDeltaAngle = std::max(maxDeltaAngle, delta);
        }
        jnrep[j] = std::max(std::min(ceil(maxDeltaAngle / (3.141592653589793 / 24.)), 240.),6.);
    }

    for (int i = istart; i < iend - 1; ++i)
    {
        double maxDeltaAngle = 0.;
        for (int j = 0; j < nyIn; ++j)
        {
            int ipt1 = i * ny2d + jindex[j];
            int ipt2 = (i+1) * ny2d + jindex[j];
            double theta  = zShift[ipt1];
            double theta2 = zShift[ipt2];
            double delta  = fabs(theta2 - theta);
            maxDeltaAngle = std::max(maxDeltaAngle, delta);
        }
        inrep[i-istart] = std::min(ceil(maxDeltaAngle / (3.141592653589793 / 24.)), 240.);
    }

#if 0
    //
    // Turn off refining the mesh.
    //
    for (int i = 0; i < nxIn - 1; ++i)
    {
        inrep[i] = 1;
    }
    for (int j = 0; j < nyIn - 1; ++j)
    {
        jnrep[j] = 1;
    }
#endif

    //
    // Calculate the size of the output mesh.
    //
    int nxOut, nyOut;
    if (nxIn == 2 && nyIn == 2)
    {
        nxOut = 2;  // This is a bogus value, it will be set properly later.
        nyOut = 2;  // This is a bogus value, it will be set properly later.
    }
    else
    {
        nxOut = 0;
        for (int i = 0; i < nxIn - 1; ++i)
        {
            nxOut += inrep[i];
        }
        nxOut++;
        nyOut = 0;
        for (int j = 0; j < nyIn - 1; ++j)
        {
            nyOut += jnrep[j];
        }
        nyOut++;
    }

    grid.nxOut = nxOut; 
    grid.nyOut = nyOut; 
}

// ****************************************************************************
// Method: avtBOUTFileFormat::ReadMeshMetaData
//
// Purpose:
//   This method reads the mesh meta data from the grid file.
//
// Arguments:
//
// Programmer: Eric Brugger
// Creation:   Mon Dec  2 15:44:28 PST 2013
//
// Modifications:
//   Eric Brugger, Tue Dec  3 08:59:26 PST 2013
//   I corrected an uninitialized memory error in the code that forms the
//   full path of the grid file.
//
//   Eric Brugger, Mon Sep 22 16:45:10 PDT 2014
//   I moved READSCALARINTO to the top of the file and removed VARTYPE from
//   READSCALARINTO since it wasn't used.
//
// ****************************************************************************

void
avtBOUTFileFormat::ReadMeshMetaData()
{
    const char *mName = "avtBOUTFileFormat::ReadMeshMetaData: ";
    debug4 << mName << endl;

    //
    // Read the gridname string and open the mesh file.
    //
    TypeEnum t = NO_TYPE;
    int ndims = 0, *dims = 0;
    void *vals = 0;
    if (fileObject->ReadVariable("gridname", &t, &ndims, &dims, &vals))
    {
        if (t == CHARARRAY_TYPE && ndims == 1)
        {
            //
            // Prepend the path of the directory containing the file
            // to the gridname file.
            //
            char *gridname = new char[strlen(filePath)+1+dims[0]+1];
            memcpy(gridname, filePath, strlen(filePath));
            gridname[strlen(filePath)] = VISIT_SLASH_CHAR;
            memcpy(&gridname[strlen(filePath)+1], vals, dims[0]);
            gridname[strlen(filePath)+1+dims[0]] = '\0';

            //
            // The constructor just stores the filename. This means that
            // any errors in opening the file will not happen until the
            // first attempt to read a variable from it.
            //
            meshFile = new NETCDFFileObject((char*)gridname);
            delete [] gridname;
        }
        delete [] dims;
        delete [] (char*) vals;
    }
    else
    {
        std::string msg("The gridname could not be read");
        EXCEPTION2(NonCompliantFileException, "BOUT", msg);
    }

    //
    // Read some key scalars that indicate how the grid is connected.
    //
    READSCALARINTO("ixseps1", ixseps1);
    READSCALARINTO("ixseps2", ixseps2);
    READSCALARINTO("jyseps1_1", jyseps1_1);
    READSCALARINTO("jyseps1_2", jyseps1_2);
    READSCALARINTO("jyseps2_1", jyseps2_1);
    READSCALARINTO("jyseps2_2", jyseps2_2);

    debug4 << "   ixseps1   = " << ixseps1 << endl;
    debug4 << "   ixseps2   = " << ixseps2 << endl;
    debug4 << "   jyseps1_1 = " << jyseps1_1 << endl;
    debug4 << "   jyseps1_2 = " << jyseps1_2 << endl;
    debug4 << "   jyseps2_1 = " << jyseps2_1 << endl;
    debug4 << "   jyseps2_2 = " << jyseps2_2 << endl;
}

// ****************************************************************************
// Method: avtBOUTFileFormat::ReadMesh
//
// Purpose: 
//   This method reads the mesh information from the grid file.
//
// Arguments:
//
// Returns:    True.
//
// Programmer: Eric Brugger
// Creation:   Thu Aug  1 16:42:56 PDT 2013
//
// Modifications:
//   Eric Brugger, Mon Dec  2 15:44:28 PST 2013
//   I added the ability to handle circular grids.
//   
//   Eric Brugger, Tue Dec  3 10:23:53 PST 2013
//   I added the ability to handle grids with two X points.
//
//   Eric Brugger, Fri Apr 11 12:51:32 PDT 2014
//   I modified the reader to eliminate the grid above the upper X point
//   when working with a grid with two X points.
//
//   Eric Brugger, Mon Sep 22 16:45:10 PDT 2014
//   I added more error checks on the data.
//
// ****************************************************************************

bool
avtBOUTFileFormat::ReadMesh()
{
    const char *mName = "avtBOUTFileFormat::ReadMesh: ";
    debug4 << mName << endl;

    //
    // Return if we have already read the mesh.
    //
    if (meshRead)
        return true;

    //
    // Read the coordinate information.
    //
    TypeEnum rt = NO_TYPE, zt = NO_TYPE, zshiftt = NO_TYPE;
    int rndims = 0, zndims = 0, zshiftndims = 0;
    int *rdims = 0, *zdims = 0, *zshiftdims = 0;
    meshFile->InqVariable("Rxy", &rt, &rndims, &rdims);
    meshFile->InqVariable("Zxy", &zt, &zndims, &zdims);
    meshFile->InqVariable("zShift", &zshiftt, &zshiftndims, &zshiftdims);

    if (rndims != 2 || rdims == 0 ||
        zndims != 2 || zdims == 0 || 
        zshiftndims != 2 || zshiftdims == 0)
    {
        if (rndims != 2 || rdims == 0)
        {
            debug4 << mName << "Error reading Rxy." << endl;
        }
        if (zndims != 2 || zdims == 0)
        {
            debug4 << mName << "Error reading Zxy." << endl;
        }
        if (zshiftndims != 2 || zshiftdims == 0)
        {
            debug4 << mName << "Error reading zShift." << endl;
        }

        if (rdims != 0) delete [] rdims;
        if (zdims != 0) delete [] zdims;
        if (zshiftdims != 0) delete [] zshiftdims;

        std::string msg("\"The grid could not be read.\"");
        EXCEPTION1(InvalidVariableException, msg);
    }

    if (rdims[0] != zdims[0] || rdims[1] != zdims[1] ||
        rdims[0] != zshiftdims[0] || rdims[1] != zshiftdims[1])
    {
        debug4 << mName << "Error: The Rxy, Zxy and zShift dimensions were "
               << "inconsistent." << endl; 

        delete [] rdims;
        delete [] zdims;
        delete [] zshiftdims;

        std::string msg("\"The grid could not be read.\"");
        EXCEPTION1(InvalidVariableException, msg);
    }

    Rxy = new float[rdims[0]*rdims[1]];
    Zxy = new float[zdims[0]*zdims[1]];
    zShift = new float[zshiftdims[0]*zshiftdims[1]];

    meshFile->ReadVariableInto("Rxy", FLOATARRAY_TYPE, Rxy);
    meshFile->ReadVariableInto("Zxy", FLOATARRAY_TYPE, Zxy);
    meshFile->ReadVariableInto("zShift", FLOATARRAY_TYPE, zShift);

    zShiftZero = new float[zshiftdims[0]*zshiftdims[1]];
    memset(zShiftZero, 0, zshiftdims[0]*zshiftdims[1]*sizeof(float));

    nx2d = rdims[0];
    ny2d = rdims[1];

    nxRaw = rdims[0];
    nyRaw = rdims[1];

    delete [] rdims;
    delete [] zdims;
    delete [] zshiftdims;

    //
    // Create the definitions of how to create the various subgrids.
    //
    if (gridType == circularGrid)
    {
        subgrid[0].nb = 2;
        subgrid[0].istart = 0;
        subgrid[0].iend = ixseps1;
        subgrid[0].jstart[0] = jyseps1_1 + 1;
        subgrid[0].jend[0] = jyseps2_2 + 1;
        subgrid[0].jstart[1] = jyseps1_1 + 1;
        subgrid[0].jend[1] = jyseps1_1 + 2;

        //
        // Create the subgrid.
        //
        DetermineMeshReplication(subgrid[0]);
    }
    else if (gridType == oneXGrid)
    {
        subgrid[0].nb = 2;
        subgrid[0].istart = 0;
        subgrid[0].iend = ixseps1 + 1;
        subgrid[0].jstart[0] = 0;
        subgrid[0].jend[0] = jyseps1_1 + 1;
        subgrid[0].jstart[1] = jyseps2_2 + 1;
        subgrid[0].jend[1] = nyRaw;

        subgrid[1].nb = 1;
        subgrid[1].istart = ixseps1;
        subgrid[1].iend = nxRaw;
        subgrid[1].jstart[0] = 0;
        subgrid[1].jend[0] = nyRaw;

        subgrid[2].nb = 2;
        subgrid[2].istart = 0;
        subgrid[2].iend = ixseps1 + 1;
        subgrid[2].jstart[0] = jyseps1_1 + 1;
        subgrid[2].jend[0] = jyseps2_2 + 1;
        subgrid[2].jstart[1] = jyseps1_1 + 1;
        subgrid[2].jend[1] = jyseps1_1 + 2;

        subgrid[3].nb = 2;
        subgrid[3].istart = ixseps1;
        subgrid[3].iend = ixseps1 + 1;
        subgrid[3].jstart[0] = jyseps1_1;
        subgrid[3].jend[0] = jyseps1_1 + 2;
        subgrid[3].jstart[1] = jyseps2_2 + 1;
        subgrid[3].jend[1] = jyseps2_2 - 1;
 
        //
        // Create the subgrids.
        //
        for (int isubgrid = 0; isubgrid < nSubMeshes; isubgrid++)
        {
            DetermineMeshReplication(subgrid[isubgrid]);
        }

        //
        // Adjust the replication factors of the different grids where
        // they meet at the center so that they all match.
        //
        int nxMax = int(std::max(subgrid[1].jnrep[jyseps1_1],
                                 subgrid[1].jnrep[jyseps2_2]));
        int nyMax = int(std::max(subgrid[0].jnrep[jyseps1_1],
                                 subgrid[2].jnrep[subgrid[2].nyIn-2]));

        subgrid[1].nyOut += (nxMax - subgrid[1].jnrep[jyseps1_1]) +
                            (nxMax - subgrid[1].jnrep[jyseps2_2]);
        subgrid[1].jnrep[jyseps1_1] = nxMax;
        subgrid[1].jnrep[jyseps2_2] = nxMax;

        subgrid[0].nyOut += nyMax - subgrid[0].jnrep[jyseps1_1];
        subgrid[0].jnrep[jyseps1_1] = nyMax;
        subgrid[2].nyOut += nyMax - subgrid[2].jnrep[subgrid[2].nyIn-2];
        subgrid[2].jnrep[subgrid[2].nyIn-2] = nyMax;

        subgrid[3].nxOut = nxMax + 1;
        subgrid[3].inrep[0] = nxMax;
        subgrid[3].nyOut = nyMax + 1;
        subgrid[3].jnrep[0] = nyMax;
    }
    else
    {
        subgrid[0].nb = 2;
        subgrid[0].istart = 0;
        subgrid[0].iend = ixseps1 + 1;
        subgrid[0].jstart[0] = 0;
        subgrid[0].jend[0] = jyseps1_1 + 1;
        subgrid[0].jstart[1] = jyseps2_2 + 1;
        subgrid[0].jend[1] = nyRaw;

        subgrid[1].nb = 1;
        subgrid[1].istart = ixseps1;
        subgrid[1].iend = nxRaw;
        subgrid[1].jstart[0] = 0;
        subgrid[1].jend[0] = jyseps2_1 + (jyseps1_2 - jyseps2_1) / 2 + 1;

        subgrid[2].nb = 1;
        subgrid[2].istart = ixseps1;
        subgrid[2].iend = nxRaw;
        subgrid[2].jstart[0] = jyseps2_1 + (jyseps1_2 - jyseps2_1) / 2 + 1;
        subgrid[2].jend[0] = nyRaw;

        subgrid[3].nb = 2;
        subgrid[3].istart = 0;
        subgrid[3].iend = ixseps1 + 1;
        subgrid[3].jstart[0] = jyseps1_1 + 1;
        subgrid[3].jend[0] = jyseps2_1 + 1;
        subgrid[3].jstart[1] = jyseps1_2 + 1;
        subgrid[3].jend[1] = jyseps1_2 + 2;

        subgrid[4].nb = 2;
        subgrid[4].istart = 0;
        subgrid[4].iend = ixseps1 + 1;
        subgrid[4].jstart[0] = jyseps1_2 + 1;
        subgrid[4].jend[0] = jyseps2_2 + 1;
        subgrid[4].jstart[1] = jyseps1_1 + 1;
        subgrid[4].jend[1] = jyseps1_1 + 2;

        subgrid[5].nb = 2;
        subgrid[5].istart = ixseps1;
        subgrid[5].iend = ixseps1 + 1;
        subgrid[5].jstart[0] = jyseps1_1;
        subgrid[5].jend[0] = jyseps1_1 + 2;
        subgrid[5].jstart[1] = jyseps2_2 + 1;
        subgrid[5].jend[1] = jyseps2_2 - 1;
 
        subgrid[6].nb = 2;
        subgrid[6].istart = ixseps1;
        subgrid[6].iend = ixseps1 + 1;
        subgrid[6].jstart[0] = jyseps2_1;
        subgrid[6].jend[0] = jyseps2_1 + 2;
        subgrid[6].jstart[1] = jyseps1_2 + 1;
        subgrid[6].jend[1] = jyseps1_2 - 1;
 
        //
        // Create the subgrids.
        //
        for (int isubgrid = 0; isubgrid < nSubMeshes; isubgrid++)
        {
            DetermineMeshReplication(subgrid[isubgrid]);
        }

        //
        // Adjust the replication factors of the different grids where
        // they meet at the center so that they all match.
        //
        int nxMax = int(std::max(subgrid[1].jnrep[jyseps1_1],
                                 subgrid[2].jnrep[jyseps2_2-jyseps2_1-6]));
        int nyMax = int(std::max(subgrid[0].jnrep[jyseps1_1],
                                 subgrid[4].jnrep[subgrid[4].nyIn-2]));

        subgrid[1].nyOut += (nxMax - subgrid[1].jnrep[jyseps1_1]);
        subgrid[1].jnrep[jyseps1_1] = nxMax;
        subgrid[2].nyOut += (nxMax - subgrid[2].jnrep[jyseps2_2-jyseps2_1-5]);
        subgrid[2].jnrep[jyseps2_2-jyseps2_1-5] = nxMax;

        subgrid[0].nyOut += nyMax - subgrid[0].jnrep[jyseps1_1];
        subgrid[0].jnrep[jyseps1_1] = nyMax;
        subgrid[4].nyOut += nyMax - subgrid[4].jnrep[subgrid[4].nyIn-2];
        subgrid[4].jnrep[subgrid[4].nyIn-2] = nyMax;

        subgrid[5].nxOut = nxMax + 1;
        subgrid[5].inrep[0] = nxMax;
        subgrid[5].nyOut = nyMax + 1;
        subgrid[5].jnrep[0] = nyMax;

        nxMax = int(std::max(subgrid[1].jnrep[jyseps2_1],
                             subgrid[2].jnrep[(jyseps1_2-jyseps2_1)/2-1]));
        nyMax = subgrid[3].jnrep[subgrid[3].nyIn-2];

        subgrid[1].nyOut += (nxMax - subgrid[1].jnrep[jyseps2_1]);
        subgrid[1].jnrep[jyseps2_1] = nxMax;
        subgrid[2].nyOut += (nxMax - subgrid[2].jnrep[(jyseps1_2-jyseps2_1)/2-1]);
        subgrid[2].jnrep[(jyseps1_2-jyseps2_1)/2-1] = nxMax;

        subgrid[6].nxOut = nxMax + 1;
        subgrid[6].inrep[0] = nxMax;
        subgrid[6].nyOut = nyMax + 1;
        subgrid[6].jnrep[0] = nyMax;
    }

    meshRead = true;

    return meshRead;
}

// ****************************************************************************
// Method: avtBOUTFileFormat::CreateDiverterMesh
//
// Purpose: 
//   This method creates the mesh points for the diverter mesh for the
//   specified domain.
//
// Arguments:
//   grid   : The grid description.
//   domain : The domain number.
//   zShift : The z shift to apply to the points.
//   pts    : The points of the mesh.
//
// Programmer: Eric Brugger
// Creation:   Thu Aug  1 16:42:56 PDT 2013
//
// Modifications:
//   Eric Brugger, Fri Apr 11 10:33:08 PDT 2014
//   I modified the creation of the diverter to include all of the lower
//   diverter as well as create the upper diverter with a two X point grid.
//   
// ****************************************************************************

void
avtBOUTFileFormat::CreateDiverterMesh(Subgrid &grid, int domain,
    float *zShift, float *pts)
{
    //
    // Calculate the block and subgrid indexes.
    //
    int iblock, isubgrid;
    if (gridType == oneXGrid)
    {
        iblock = domain / N_DIVERTER_ONEX_SUB_MESHES;
        isubgrid = domain % N_DIVERTER_ONEX_SUB_MESHES;
    }
    else
    {
        iblock = domain / N_DIVERTER_TWOX_SUB_MESHES;
        isubgrid = domain % N_DIVERTER_TWOX_SUB_MESHES;
    }

    //
    // Set the dimensions
    //
    //int istart, iend;
    int *ijindex;
    int *inrep;
    int nxIn, nyIn;
    int nxOut;
    //istart  = grid.istart;
    //iend    = grid.iend;
    ijindex = grid.ijindex;
    inrep   = grid.inrep;
    nxIn    = grid.nxIn;
    nyIn    = grid.nyIn;
    nxOut   = grid.nxOut;

    int jj;
    if (isubgrid == 0 || isubgrid == 1 || isubgrid == 5)
        jj = 0;
    else
        jj = nyIn - 1;

    for (int k = 0; k < nzOut; ++k)
    {
        int isum = 0;
        for (int i = 0; i < nxIn - 1; ++i)
        {
            int ipt1 = ijindex[i*nyIn+jj];
            int ipt2 = ijindex[(i+1)*nyIn+jj];
            double r = Rxy[ipt1];
            double r2 = Rxy[ipt2];
            double z = Zxy[ipt1];
            double z2 = Zxy[ipt2];
            double theta = zShift[ipt1] + float(k + nz * iblock) *
                6.283185307179586 / (nz * zperiod);
            double theta2 = zShift[ipt2] + float(k + nz * iblock) *
                6.283185307179586 / (nz * zperiod);

            double dr = (r2 - r) / double(inrep[i]);
            double dz = (z2 - z) / double(inrep[i]);
            double dtheta = (theta2 - theta) / double(inrep[i]);
            for (int ii = 0; ii < inrep[i] + 1; ++ii)
            {
                double r2 = r + double(ii) * dr;
                double z2 = z + double(ii) * dz;
                double theta2 = theta + double(ii) * dtheta;
            
                int ipts = (k * nxOut + ii + isum) * 3;
                pts[ipts+0] = r2 * cos(theta2);
                pts[ipts+1] = z2;
                pts[ipts+2] = r2 * sin(theta2);
            }
            isum += inrep[i];
        }
    }
}

// ****************************************************************************
// Method: avtBOUTFileFormat::CreateMesh
//
// Purpose: 
//   This method creates the mesh points for the specified domain.
//
// Arguments:
//   grid   : The grid description.
//   iblock : The block number.
//   ndims  : The number of dimensions to create the mesh.
//   zShift : The z shift to apply to the points.
//   pts    : The points of the mesh.
//
// Programmer: Eric Brugger
// Creation:   Thu Aug  1 16:42:56 PDT 2013
//
// Modifications:
//   
// ****************************************************************************

void
avtBOUTFileFormat::CreateMesh(Subgrid &grid, int iblock, int ndims,
    float *zShift, float *pts)
{
    //
    // Set the dimensions
    //
    //int *jindex;
    int *ijindex;
    int *inrep;
    int *jnrep;
    int nxIn, nyIn;
    int nxOut, nyOut;
    //jindex    = grid.jindex;
    ijindex   = grid.ijindex;
    inrep     = grid.inrep;
    jnrep     = grid.jnrep;
    nxIn      = grid.nxIn;
    nyIn      = grid.nyIn;
    nxOut     = grid.nxOut;
    nyOut     = grid.nyOut;

    int kend = nzOut;
    if (ndims == 2)
        kend = 1;

    if (nxIn == 2 && nyIn == 2)
    {
        for (int k = 0; k < kend; ++k)
        {
            int ipt11 = ijindex[0];
            int ipt12 = ijindex[1];
            int ipt21 = ijindex[nxIn];
            int ipt22 = ijindex[nxIn+1];
            double r11 = Rxy[ipt11];
            double r12 = Rxy[ipt12];
            double r21 = Rxy[ipt21];
            double r22 = Rxy[ipt22];
            double z11 = Zxy[ipt11];
            double z12 = Zxy[ipt12];
            double z21 = Zxy[ipt21];
            double z22 = Zxy[ipt22];
            double theta11 = zShift[ipt11] + float(k + nz * iblock) *
                6.283185307179586 / (nz * zperiod);
            double theta12 = zShift[ipt12] + float(k + nz * iblock) *
                6.283185307179586 / (nz * zperiod);
            double theta21 = zShift[ipt21] + float(k + nz * iblock) *
                6.283185307179586 / (nz * zperiod);
            double theta22 = zShift[ipt22] + float(k + nz * iblock) *
                6.283185307179586 / (nz * zperiod);
            for (int ii = 0; ii < nxOut; ++ii)
            {
                for (int jj = 0; jj < nyOut; ++jj)
                {
                    double dr1 = r11 + (r21 - r11) * (double(jj) / (nyOut-1));
                    double dz1 = z11 + (z21 - z11) * (double(jj) / (nyOut-1));
                    double dtheta1 = theta11 + (theta21 - theta11) * (double(jj) / (nyOut-1));
                    double dr2 = r12 + (r22 - r12) * (double(jj) / (nyOut-1));
                    double dz2 = z12 + (z22 - z12) * (double(jj) / (nyOut-1));
                    double dtheta2 = theta12 + (theta22 - theta12) * (double(jj) / (nyOut-1));
                    double dr = dr1 + (dr2 - dr1) * (double(ii) / (nxOut-1));
                    double dz = dz1 + (dz2 - dz1) * (double(ii) / (nxOut-1));
                    double dtheta = dtheta1 + (dtheta2 - dtheta1) * (double(ii) / (nxOut-1));
                    int ipts = (k * nxOut * nyOut + ii * nyOut + jj) * 3;
                    pts[ipts+0] = dr * cos(dtheta);
                    pts[ipts+1] = dz;
                    pts[ipts+2] = dr * sin(dtheta);
                }
            }
        }
    }
    else
    {
        for (int k = 0; k < kend; ++k)
        {
            int isum = 0;
            for (int i = 0; i < nxIn - 1; ++i)
            {
                int jsum = 0;
                for (int j = 0; j < nyIn - 1; ++j)
                {
                    int ipt11 = ijindex[i*nyIn+j];
                    int ipt12 = ijindex[i*nyIn+j+1];
                    int ipt21 = ijindex[(i+1)*nyIn+j];
                    int ipt22 = ijindex[(i+1)*nyIn+j+1];
                    double r11 = Rxy[ipt11];
                    double r12 = Rxy[ipt12];
                    double r21 = Rxy[ipt21];
                    double r22 = Rxy[ipt22];
                    double z11 = Zxy[ipt11];
                    double z12 = Zxy[ipt12];
                    double z21 = Zxy[ipt21];
                    double z22 = Zxy[ipt22];
                    double theta11 = zShift[ipt11] + float(k + nz * iblock) *
                        6.283185307179586 / (nz * zperiod);
                    double theta12 = zShift[ipt12] + float(k + nz * iblock) *
                        6.283185307179586 / (nz * zperiod);
                    double theta21 = zShift[ipt21] + float(k + nz * iblock) *
                        6.283185307179586 / (nz * zperiod);
                    double theta22 = zShift[ipt22] + float(k + nz * iblock) *
                        6.283185307179586 / (nz * zperiod);
                    for (int ii = 0; ii < inrep[i] + 1; ++ii)
                    {
                        double dr1 = r11 + (r21 - r11) * (double(ii) / (inrep[i]));
                        double dz1 = z11 + (z21 - z11) * (double(ii) / (inrep[i]));
                        double dtheta1 = theta11 + (theta21 - theta11) * (double(ii) / (inrep[i]));
                        double dr2 = r12 + (r22 - r12) * (double(ii) / (inrep[i]));
                        double dz2 = z12 + (z22 - z12) * (double(ii) / (inrep[i]));
                        double dtheta2 = theta12 + (theta22 - theta12) * (double(ii) / (inrep[i]));
                        for (int jj = 0; jj < jnrep[j] + 1; ++jj)
                        {
                            double dr = dr1 + (dr2 - dr1) * (double(jj) / (jnrep[j]));
                            double dz = dz1 + (dz2 - dz1) * (double(jj) / (jnrep[j]));
                            double dtheta = dtheta1 + (dtheta2 - dtheta1) * (double(jj) / (jnrep[j]));
                            int ipts = (k * nxOut * nyOut + (ii + isum) * nyOut + jj + jsum) * 3;
                            pts[ipts+0] = dr * cos(dtheta);
                            pts[ipts+1] = dz;
                            pts[ipts+2] = dr * sin(dtheta);
                        }
                    }
                    jsum += jnrep[j];
                }
                isum += inrep[i];
            }
        }
    }
}

// ****************************************************************************
// Method: avtBOUTFileFormat::CreateDiverterVar
//
// Purpose: 
//   This method creates the data values for the diverter for the
//   specified domain.
//
// Arguments:
//   grid   : The grid description.
//   domain : The domain number.
//   ndims  : The number of dimensions to create the mesh.
//   data   : The input data.
//   vals   : The output data.
//
// Programmer: Eric Brugger
// Creation:   Thu Aug  1 16:42:56 PDT 2013
//
// Modifications:
//   Eric Brugger, Thu Apr 10 10:39:53 PDT 2014
//   I modified the reader to set the variable to the absolute value of
//   the variable on the diverter.
//   
//   Eric Brugger, Fri Apr 11 10:33:08 PDT 2014
//   I modified the creation of the diverter to include all of the lower
//   diverter as well as create the upper diverter with a two X point grid.
//   
// ****************************************************************************

void
avtBOUTFileFormat::CreateDiverterVar(Subgrid &grid, int domain, float *data,
    float *vals)
{
    //
    // Calculate the block and subgrid indexes.
    //
    int /*iblock,*/ isubgrid;
    if (gridType == oneXGrid)
    {
        //iblock = domain / N_DIVERTER_ONEX_SUB_MESHES;
        isubgrid = domain % N_DIVERTER_ONEX_SUB_MESHES;
    }
    else
    {
        //iblock = domain / N_DIVERTER_TWOX_SUB_MESHES;
        isubgrid = domain % N_DIVERTER_TWOX_SUB_MESHES;
    }

    //
    // Set the dimensions
    //
    int istart;
    int *jindex;
    int *inrep;
    int nxIn, nyIn;
    int nxOut;
    istart  = grid.istart;
    jindex  = grid.jindex;
    inrep   = grid.inrep;
    nxIn    = grid.nxIn;
    nyIn    = grid.nyIn;
    nxOut   = grid.nxOut;

    int nZ = nz;
    int nyz = nyIn * nz;

    int jj;
    if (isubgrid == 0 || isubgrid == 1 || isubgrid == 5)
        jj = 0;
    else
        jj = nyIn - 1;

    for (int k = 0; k < nzOut; ++k)
    {
        int k2 = k % nz;
        int isum = 0;
        for (int i = 0; i < nxIn - 1; ++i)
        {
            int ipt1 = (i+istart)   * nyz + jindex[jj] * nZ + k2;
            int ipt2 = (i+istart+1) * nyz + jindex[jj] * nZ + k2;
            double v = data[ipt1];
            double v2 = data[ipt2];
            double dv = (v2 - v) / double(inrep[i]);
            for (int ii = 0; ii < inrep[i] + 1; ++ii)
            {
                int ivals = k * nxOut + ii + isum;
                vals[ivals] = std::abs(v + double(ii) * dv);
            }
            isum += inrep[i];
        }
    }
}

// ****************************************************************************
// Method: avtBOUTFileFormat::CreateVar
//
// Purpose: 
//   This method creates the data values for the specified domain.
//
// Arguments:
//   grid   : The grid description.
//   iblock : The block number.
//   ndims  : The number of dimensions to create the mesh.
//   data   : The input data.
//   vals   : The output data.
//
// Programmer: Eric Brugger
// Creation:   Thu Aug  1 16:42:56 PDT 2013
//
// Modifications:
//   
// ****************************************************************************

void
avtBOUTFileFormat::CreateVar(Subgrid &grid, int iblock, int ndims,
    float *data, float *vals)
{
    //
    // Set the dimensions
    //
    //int nb;
    int istart/*, iend*/;
    int jstart[2], jend[2];
    int *jindex;
    //int *ijindex;
    int *inrep;
    int *jnrep;
    int nxIn, nyIn;
    int nxOut, nyOut;
    //nb        = grid.nb;
    istart    = grid.istart;
    jstart[0] = grid.jstart[0];
    jstart[1] = grid.jstart[1];
    jend[0]   = grid.jend[0];
    jend[1]   = grid.jend[1];
    jindex    = grid.jindex;
    //ijindex   = grid.ijindex;
    inrep     = grid.inrep;
    jnrep     = grid.jnrep;
    nxIn      = grid.nxIn;
    nyIn      = grid.nyIn;
    nxOut     = grid.nxOut;
    nyOut     = grid.nyOut;

    int kend = nzOut;
    int nZ = nz;
    int nyz = nyRaw * nz;
    if (ndims == 2)
    {
        kend = 1;
        nZ = 1;
        nyz = ny2d;
    }

    if (nxIn == 2 && nyIn == 2)
    {
        for (int k = 0; k < kend; ++k)
        {
            int k2 = k % nZ;
            int ipt11 = istart * nyz + jstart[0] * nZ + k2;
            int ipt12 = istart * nyz + (jend[0] - 1) * nZ + k2;
            int ipt21 = istart * nyz + jstart[1] * nZ + k2;
            int ipt22 = istart * nyz + (jend[1] + 1) * nZ + k2;
            double v11 = data[ipt11];
            double v12 = data[ipt12];
            double v21 = data[ipt21];
            double v22 = data[ipt22];
            int jsum = 0;
            for (int i = 0; i < nxOut; ++i)
            {
                for (int j = 0; j < nyOut; ++j)
                {
                    double dv1 = v11 + (v21 - v11) * (double(j) / (nyOut-1));
                    double dv2 = v12 + (v22 - v12) * (double(j) / (nyOut-1));
                    double dv = dv1 + (dv2 - dv1) * (double(i) / (nxOut-1));
                    int ivals = k * nxOut * nyOut + i * nyOut + j;
                    vals[ivals] = dv;
                    jsum++;
                }
            }
        }
    }
    else
    {
        for (int k = 0; k < kend; ++k)
        {
            int k2 = k % nZ;
            int isum = 0;
            for (int i = 0; i < nxIn - 1; ++i)
            {
                int jsum = 0;
                for (int j = 0; j < nyIn - 1; ++j)
                {
                    int ipt11 = (i+istart) * nyz + jindex[j] * nZ + k2;
                    int ipt12 = (i+istart) * nyz + jindex[j+1] * nZ + k2;
                    int ipt21 = (i+istart+1) * nyz + jindex[j] * nZ + k2;
                    int ipt22 = (i+istart+1) * nyz + jindex[j+1] * nZ + k2;
                    double v11 = data[ipt11];
                    double v12 = data[ipt12];
                    double v21 = data[ipt21];
                    double v22 = data[ipt22];
                    for (int ii = 0; ii < inrep[i] + 1; ++ii)
                    {
                        double dv1 = v11 + (v21 - v11) * (double(ii) / (inrep[i]));
                        double dv2 = v12 + (v22 - v12) * (double(ii) / (inrep[i]));
                        for (int jj = 0; jj < jnrep[j] + 1; ++jj)
                        {
                            double dv = dv1 + (dv2 - dv1) * (double(jj) / (jnrep[j]));

                            int ivals = k * nxOut * nyOut + (ii + isum) * nyOut + jj + jsum;
                            vals[ivals] = dv;
                        }
                    }
                    jsum += jnrep[j];
                }
                isum += inrep[i];
            }
        }
    }
}

// ****************************************************************************
// Method: avtBOUTFileFormat::GetMesh
//
// Purpose: 
//   This method returns the mesh for the specified time state.
//
// Arguments:
//   ts     : The time state for which to return the mesh.
//   domain : The index of the domain.
//   var    : The mesh to fetch.
//
// Returns:    A vtkDataSet or 0.
//
// Programmer: Eric Brugger
// Creation:   Thu Aug  1 16:42:56 PDT 2013
//
// Modifications:
//   Eric Brugger, Mon Dec  2 15:44:28 PST 2013
//   I added the ability to handle circular grids.
//   
//   Eric Brugger, Tue Dec  3 10:23:53 PST 2013
//   I added the ability to handle grids with two X points.
//
//   Eric Brugger, Fri Apr 11 10:33:08 PDT 2014
//   I modified the creation of the diverter to include all of the lower
//   diverter as well as create the upper diverter with a two X point grid.
//   
// ****************************************************************************

#if 0
    //
    // THIS CODE IS COMPLETELY WRONG IN TERMS OF INDEXING BUT IT SHOWS
    // HOW TO CREATE GHOST DATA.
    //
    // Set the ghost data
    int ncells = (dims[0] - 1) * (dims[1] - 1);
    vtkUnsignedCharArray *gz = vtkUnsignedCharArray::New();
    gz->SetName("avtGhostZones");
    gz->SetNumberOfTuples(ncells);
    sgrid->GetCellData()->AddArray(gz);
    gz->Delete();
    unsigned char *gz_raw = gz->GetPointer(0);

    pts = (float *) points->GetVoidPointer(0);
    for (int i = 0; i < nxIn - 1; ++i)
    {
        for (int j = 0; j < nyIn - 1; ++j)
        {
            if (j == 3 || j == 59)
            {
                avtGhostData::AddGhostNodeType(*gz_raw,
                    NODE_NOT_APPLICABLE_TO_PROBLEM);
            }
            gz_raw++;
        }
    }
#endif

vtkDataSet *
avtBOUTFileFormat::GetMesh(int ts, int domain, const char *var)
{
    const char *mName = "avtBOUTFileFormat::GetMesh: ";
    debug4 << mName << "ts=" << ts
           << ", var=" << var << endl;
    vtkDataSet *retval = 0;

    //
    // Read the mesh.
    //
    ReadMesh();

    //
    // Check that we successfully read the mesh.
    //
    if (Rxy == 0 || Zxy == 0 || zShift == 0)
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    //
    // Create the curvilinear mesh.
    //
    vtkStructuredGrid *sgrid   = vtkStructuredGrid::New();
    vtkPoints         *points  = vtkPoints::New();
    sgrid->SetPoints(points);
    points->Delete();

    //
    // Calculate the dimensions of the mesh.
    //
    int dims[3];
    if (strcmp(var, "mesh") == 0 || strcmp(var, "mesh_zshift") == 0)
    {
        int isubgrid = domain % nSubMeshes;
        dims[0] = subgrid[isubgrid].nyOut;
        dims[1] = subgrid[isubgrid].nxOut;
        dims[2] = nzOut;
    }
    else if (strcmp(var, "mesh_2d") == 0)
    {
        int isubgrid = domain % nSubMeshes;
        dims[0] = subgrid[isubgrid].nyOut;
        dims[1] = subgrid[isubgrid].nxOut;
        dims[2] = 1;
    }
    else
    {
        int isubgrid;
        if (gridType == oneXGrid)
            isubgrid = oneXGridDomainToSubgrid[domain % N_DIVERTER_ONEX_SUB_MESHES];
        else
            isubgrid = twoXGridDomainToSubgrid[domain % N_DIVERTER_TWOX_SUB_MESHES];
        dims[0] = subgrid[isubgrid].nxOut;
        dims[1] = nzOut;
        dims[2] = 1;
    }
    sgrid->SetDimensions(dims);

    //
    // Set the points
    //
    int npts = dims[0] * dims[1] * dims[2];
    points->SetNumberOfPoints(npts);
    float *pts = (float *) points->GetVoidPointer(0);

    if (strcmp(var, "mesh") == 0)
    {
        int iblock = domain / nSubMeshes;
        int isubgrid = domain % nSubMeshes;
        CreateMesh(subgrid[isubgrid], iblock, 3, zShiftZero, pts);
    }
    else if (strcmp(var, "mesh_zshift") == 0)
    {
        int iblock = domain / nSubMeshes;
        int isubgrid = domain % nSubMeshes;
        CreateMesh(subgrid[isubgrid], iblock, 3, zShift, pts);
    }
    else if (strcmp(var, "mesh_2d") == 0)
    {
        int iblock = domain / nSubMeshes;
        int isubgrid = domain % nSubMeshes;
        CreateMesh(subgrid[isubgrid], iblock, 2, zShiftZero, pts);
    }
    else
    {
        int isubgrid;
        if (gridType == oneXGrid)
            isubgrid = oneXGridDomainToSubgrid[domain % N_DIVERTER_ONEX_SUB_MESHES];
        else
            isubgrid = twoXGridDomainToSubgrid[domain % N_DIVERTER_TWOX_SUB_MESHES];
        CreateDiverterMesh(subgrid[isubgrid],
                           domain, zShift, pts);
    }

    retval = sgrid;

    return retval;
}

// ****************************************************************************
// Method: avtBOUTFileFormat::GetVar
//
// Purpose: 
//   Reads a variable from the file and returns the values in a data array.
//
// Arguments:
//   ts     : The time state for which we want data.
//   domain : The index of the domain.
//   var    : The name of the variable to read.
//
// Returns:    A data array or 0.
//
// Note:       
//
// Programmer: Eric Brugger
// Creation:   Thu Aug  1 16:42:56 PDT 2013
//
// Modifications:
//   Eric Brugger, Mon Dec  2 15:44:28 PST 2013
//   I added the ability to handle circular grids.
//   
//   Eric Brugger, Tue Dec  3 10:23:53 PST 2013
//   I added the ability to handle grids with two X points.
//
//   Eric Brugger, Fri Apr 11 10:33:08 PDT 2014
//   I modified the creation of the diverter to include all of the lower
//   diverter as well as create the upper diverter with a two X point grid.
//   
//   Eric Brugger, Mon Sep 22 16:45:10 PDT 2014
//   I modified the routine to handle ghost zones in the y direction for
//   3d variables. I also added more error checks on the data.
//
// ****************************************************************************

vtkDataArray *
avtBOUTFileFormat::GetVar(int ts, int domain, const char *var)
{
    const char *mName = "avtBOUTFileFormat::GetVar: ";
    debug4 << mName << "ts=" << ts
           << ", var=" << var << endl;

    vtkDataArray *retval = 0;
    bool variesOverTime = true;
    int varDim = 0;

    //
    // Calculate the variable to read from the user name. This means removing
    // either "_zshift" or "_diverter" if present.
    //
    bool diverter_var = false;
    char *var2 = 0;
    int lvar = strlen(var);
    int lzshift = strlen("_zshift");
    int ldiverter = strlen("_diverter");
    if (lvar > lzshift && strcmp(&(var[lvar-lzshift]), "_zshift") == 0)
    {
        var2 = new char[lvar-lzshift+1];
        strncpy(var2, var, lvar-lzshift);
        var2[lvar-lzshift] = '\0';
    }
    else if (lvar > ldiverter && strcmp(&(var[lvar-ldiverter]), "_diverter") == 0)
    {
        var2 = new char[lvar-ldiverter+1];
        strncpy(var2, var, lvar-ldiverter);
        var2[lvar-ldiverter] = '\0';
        diverter_var = true;
    }
    else
    {
        var2 = new char[lvar+1];
        strcpy(var2, var);
    }

    //
    // Determine which file the variable is in.
    //
    NETCDFFileObject *varFileObject;
    if (strcmp(var, "zShift") == 0)
    {
        variesOverTime = false;
        varFileObject = meshFile;
        varDim = 2;
    }
    else
    {
        variesOverTime = true;
        varFileObject = fileObject;
        varDim = 3;
    }

    //
    // Calculate the block and subgrid indexes.
    //
    int isubgrid;
    if (diverter_var)
    {
        if (gridType == oneXGrid)
            isubgrid = domain % N_DIVERTER_ONEX_SUB_MESHES;
        else
            isubgrid = domain % N_DIVERTER_TWOX_SUB_MESHES;
    }
    else
    {
        isubgrid = domain % nSubMeshes;
    }

    //
    // Check the cache and if we have a match return it.
    //
    float *data = NULL;
    if (cacheTime == ts)
    {
        if (cacheVarRaw == var2)
        {
            data = cacheDataRaw;
        }
        if (cacheVar == var && cacheData[isubgrid] != NULL)
        {
            debug4 << "Returning variable from the cache." << endl;
            delete [] var2;
            cacheData[isubgrid]->Register(NULL);
            return cacheData[isubgrid];
        }
    }

    //
    // Read the variable.
    //
    if (data == NULL)
    {
        TypeEnum t = NO_TYPE;
        int vndims = 0, *vdims = 0;
        if (varFileObject->InqVariable(var2, &t, &vndims, &vdims))
        {
            if (varDim == 2 && vndims != 2)
            {
                debug4 << mName << "Error: The number of dimensions for " << var
                       << " was " << vndims << " and it should have been 2."
                       << endl;
                delete [] vdims;
                EXCEPTION1(InvalidVariableException, var);
            }
            else if (varDim == 3 && vndims != 4)
            {
                debug4 << mName << "Error: The number of dimensions for "
                       << var << " was " << vndims << " and it should have "
                       << "been 4." << endl;
                delete [] vdims;
                EXCEPTION1(InvalidVariableException, var);
            }

            // Figure out the size of the chunk that we want to read.
            size_t *starts = new size_t[4];
            size_t *counts = new size_t[4];
            debug4 << mName << "var=" << var << " dims={";
            for (int i = 0; i < vndims; ++i)
            {
                starts[i] = 0;
                counts[i] = vdims[i];
                if (i > 0) {
                    debug4 << ", ";
                }
                debug4 << vdims[i];
            }
            debug4 << "}" << endl;

            delete [] vdims;

            if (variesOverTime)
            {
                starts[0] = ts;
                counts[0] = 1;

                if (nxRaw != counts[1] || nyRaw > counts[2] ||
                    ((counts[2] - nyRaw) % 2) != 0)
                {
                    debug4 << mName << "Error: The size of the 2d grid and "
                           << "variable were inconsitent." << endl;
                    EXCEPTION1(InvalidVariableException, var);
                }

                starts[2] = (counts[2] - nyRaw) / 2;
                counts[2] = nyRaw;
            }

            int varId;
            varFileObject->GetVarId(var2, &varId);

            if (varDim == 2) 
            {
                int nValues = nxRaw * nyRaw;
                data = new float[nValues];
                int status = nc_get_vara_float(varFileObject->GetFileHandle(),
                                  varId, starts, counts, data); (void) status;
            }
            else
            {
                int nValues = nxRaw * nyRaw * nz;
                data = new float[nValues];
                int status = nc_get_vara_float(varFileObject->GetFileHandle(),
                                  varId, starts, counts, data); (void) status;
            }

            delete [] starts;
            delete [] counts;
        }
        else
        {
            delete [] var2;
            EXCEPTION1(InvalidVariableException, var);
        }
    }

    //
    // Create the vtk object from the data.
    //
    int nValues2;
    if (diverter_var)
    {
        int isubgrid;
        if (gridType == oneXGrid)
            isubgrid = oneXGridDomainToSubgrid[domain % N_DIVERTER_ONEX_SUB_MESHES];
        else
            isubgrid = twoXGridDomainToSubgrid[domain % N_DIVERTER_TWOX_SUB_MESHES];
        nValues2 = subgrid[isubgrid].nxOut * nzOut;
    }
    else
    {
        if (varDim == 2) 
        {
            nValues2 = subgrid[isubgrid].nxOut * subgrid[isubgrid].nyOut;
        }
        else
        {
            nValues2 = subgrid[isubgrid].nxOut * subgrid[isubgrid].nyOut * nzOut;
        }
    }

    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetNumberOfTuples(nValues2);
    float *vals = (float *)arr->GetVoidPointer(0);

    if (diverter_var)
    {
        int isubgrid;
        if (gridType == oneXGrid)
            isubgrid = oneXGridDomainToSubgrid[domain % N_DIVERTER_ONEX_SUB_MESHES];
        else
            isubgrid = twoXGridDomainToSubgrid[domain % N_DIVERTER_TWOX_SUB_MESHES];
        CreateDiverterVar(subgrid[isubgrid],
                          domain, data, vals);
    }
    else
    {
        int iblock = domain / nSubMeshes;
        CreateVar(subgrid[isubgrid], iblock, varDim, data, vals);
    }

    retval = arr;

    //
    // Add the variable to the cache.
    //
    if (cacheVarRaw != var2 || cacheTime != ts)
    {
        if (cacheDataRaw != NULL) delete [] cacheDataRaw;
        cacheDataRaw = NULL;
    }
    if (cacheVar != var || cacheTime != ts)
    {
        for (int i = 0; i < MAX_SUB_MESHES; ++i)
        {
            if (cacheData[i] != NULL) cacheData[i]->Delete();
            cacheData[i] = NULL;
        }
    }
    cacheTime = ts;
    cacheVar = var;
    cacheVarRaw = var2;
    if (cacheData[isubgrid] != NULL) cacheData[isubgrid]->Delete();
    cacheData[isubgrid] = retval;
    cacheData[isubgrid]->Register(NULL);
    cacheDataRaw = data;

    delete [] var2;

    return retval;
}

int
ReorderFile()
{
    int retval;

    //
    // Open the input file.
    //
    int ncid;
    if ((retval = nc_open("te_DIIID144382_nosheath.nc", NC_NOWRITE, &ncid)))
        return retval;

    //
    // Read the data.
    //
    int varid;
    if ((retval = nc_inq_varid(ncid, "TE", &varid)))
        return retval;

    char tmp[NC_MAX_NAME+1];
    nc_type vartype;
    int varndims;
    int *vardims = new int[NC_MAX_VAR_DIMS];
    int varnatts;
    if ((retval = nc_inq_var(ncid, varid, tmp, &vartype, &varndims,
                                vardims, &varnatts)))
        return retval;

    for (int i = 0; i < varndims; ++i)
    {
        size_t realSize;
        if ((nc_inq_dimlen(ncid, vardims[i], &realSize)))
            return retval;
        vardims[i] = realSize;
    }
    int nx =vardims[0];
    int ny =vardims[1];
    int nz =vardims[2];
    int nt =vardims[3];

    int nValues = vardims[0] * vardims[1] * vardims[2] * vardims[3];
    size_t start[4], count[4];
    for (int i = 0; i < varndims; ++i)
    {
        start[i] = 0;
        count[i] = vardims[i];
    }

    float *data = new float[nValues];
    if ((nc_get_vara_float(ncid, varid, start, count, data)))
        return retval;

    //
    // Close the file.
    //
    if ((retval = nc_close(ncid)))
        return retval;

    //
    // Create the output file.
    //
    if ((retval = nc_create("TE.nc", NC_CLOBBER, &ncid)))
        return retval;

    //
    // Define the dimensions. The record dimension is defined to have
    // unlimited length - it can grow as needed. In this example it is
    // the time dimension.
    //
    int xdimid, ydimid, zdimid, tdimid;
    if ((retval = nc_def_dim(ncid, "x", nx, &xdimid)))
        return retval;
    if ((retval = nc_def_dim(ncid, "y", ny, &ydimid)))
        return retval;
    if ((retval = nc_def_dim(ncid, "z", nz, &zdimid)))
        return retval;
    if ((retval = nc_def_dim(ncid, "t", NC_UNLIMITED, &tdimid)))
        return retval;

    //
    // Define the netCDF variables for the pressure and temperature
    // data.
    //
    int dimids[4];
    dimids[0] = tdimid;
    dimids[1] = xdimid;
    dimids[2] = ydimid;
    dimids[3] = zdimid;
    int tevarid;

    if ((retval = nc_def_var(ncid, "TE", NC_FLOAT, 4, dimids, &tevarid)))
        return retval;

    //
    // End define mode.
    //
    if ((retval = nc_enddef(ncid)))
        return retval;

    //
    // Write the data, one time step at a time.
    //
    count[0] = 1;
    count[1] = nx;
    count[2] = ny;
    count[3] = nz;
    start[1] = 0;
    start[2] = 0;
    start[3] = 0;
    float *buf = new float[nx*ny*nz];
    for (int it = 0; it < nt; ++it)
    {
        //
        // Extract the current time step from the TE array.
        //
        float *bptr = buf;
        for (int ix = 0; ix < nx; ++ix)
            for (int iy = 0; iy < ny; ++iy)
                for (int iz = 0; iz < nz; ++iz)
                    *bptr++ = data[ix*ny*nz*nt + iy*nz*nt + iz*nt + it];
  
        //
        // Write the current time step.
        //
        start[0] = it;
        if ((retval = nc_put_vara_float(ncid, tevarid, start, count, buf)))
            return retval;
    }
    delete [] buf;

    //
    // Close the file.
    //
    if ((retval = nc_close(ncid)))
        return retval;

    delete [] data;

    return 0;
}

