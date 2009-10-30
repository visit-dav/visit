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

#include <vector>
#include <snprintf.h>
#include <netcdf.h>

#include <avtBasicNETCDFReader.h>
#include <NETCDFFileObject.h>
#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <avtSTSDFileFormatInterface.h>
#include <DebugStream.h>

#include <vtkCellArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>

#include <vtkUnsignedCharArray.h>
#include <vtkShortArray.h>
#include <vtkIntArray.h>
#include <vtkLongArray.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkVisItUtility.h>

#include <InvalidVariableException.h>

#define TIME_DIMENSION      -1

#ifdef PARALLEL
#include <avtParallel.h>
#endif

// ****************************************************************************
// Method: avtBasicNETCDFReader::avtBasicNETCDFReader
//
// Purpose: 
//   Constructor for the avtBasicNETCDFReader class.
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
//   Mark C. Miller, Tue Aug 15 15:28:11 PDT 2006
//   Added procNum, procCount to support on-the-fly parallel decomposition
//
// ****************************************************************************

avtBasicNETCDFReader::avtBasicNETCDFReader(const char *filename) :
    avtNETCDFReaderBase(filename), meshNameToDimensionsSizes(), varToDimensionsSizes(),
    meshNameToNCDimensions()
{
    meshNamesCreated = false;
    procNum = 0;
    procCount = 1;
#ifdef PARALLEL
    procNum = PAR_Rank();
    procCount = PAR_Size();
#endif
}

avtBasicNETCDFReader::avtBasicNETCDFReader(const char *filename, NETCDFFileObject *f) :
    avtNETCDFReaderBase(filename, f), meshNameToDimensionsSizes(), varToDimensionsSizes(),
    meshNameToNCDimensions()
{
    meshNamesCreated = false;
    procNum = 0;
    procCount = 1;
#ifdef PARALLEL
    procNum = PAR_Rank();
    procCount = PAR_Size();
#endif
}

// ****************************************************************************
// Method: avtBasicNETCDFReader::~avtBasicNETCDFReader
//
// Purpose: 
//   Destructor for the avtBasicNETCDFReader class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 18 18:04:36 PST 2005
//
// Modifications:
//   
// ****************************************************************************

avtBasicNETCDFReader::~avtBasicNETCDFReader()
{
}

// ****************************************************************************
// Method: avtBasicNETCDFReader::CreateGlobalAttributesString
//
// Purpose: 
//   Create a string for the global attributes.
//
// Arguments:
//   nGlobalAtts : The number of global attributes
//   gaString    : global attributes string.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 30 11:01:08 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
avtBasicNETCDFReader::CreateGlobalAttributesString(int nGlobalAtts, std::string &gaString)
{
    for(int i = 0; i < nGlobalAtts; ++i)
    {
        int     status;
        char    attname[NC_MAX_NAME+1];
        nc_type atttype;
        size_t  attsize;
        if((status = nc_inq_attname(fileObject->GetFileHandle(), NC_GLOBAL, i, attname))
            == NC_NOERR)
        {
            if((status = nc_inq_att(fileObject->GetFileHandle(), NC_GLOBAL, attname, &atttype,
                                    &attsize)) == NC_NOERR)
            {
                std::string tmpStr("\t");
                tmpStr += attname;

                if(atttype == NC_CHAR)
                {
                    char *value = new char[attsize+1];
                    nc_get_att_text(fileObject->GetFileHandle(), NC_GLOBAL, attname, value);
                    value[attsize] = '\0';
                    char *c2 = value + attsize - 1;
                    while(c2 >= value && *c2 == ' ')
                        *c2-- = '\0';
                    tmpStr += " = \"";
                    tmpStr += value;
                    tmpStr += "\"";

                    delete [] value;
                }

                tmpStr += "\n";
                gaString += tmpStr;
            }
        }
    }
}

// ****************************************************************************
// Method: avtBasicNETCDFReader::PopulateDatabaseMetaData
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
//    this method on ActivateTimestep to ensure that the meshNameToDimensionsSizes map
//    is populated. I also added a check to set the validVariable flag for
//    meshes and variables so that we don't have problems later. By adding
//    the valid variable check, I was able to make 4+ dimensional arrays
//    safely appear in the variable lists.
//
//    Mark C. Miller, Tue Aug 15 15:28:11 PDT 2006
//    Added call to SetFormatCanDoDomainDecomposition
//
//    Jeremy Meredith, Mon Aug 28 17:39:33 EDT 2006
//    Added test to make sure a variable had >0 dimensions before examining
//    them.
//
//    Hank Childs, Fri Feb 23 09:20:38 PST 2007
//    Fix memory leak.
//
// ****************************************************************************

void
avtBasicNETCDFReader::PopulateDatabaseMetaData(int timeState, avtDatabaseMetaData *md)
{
    debug4 << "avtBasicNETCDFReader::PopulateDatabaseMetaData" << endl;
    if(DebugStream::Level4())
        fileObject->PrintFileContents(DebugStream::Stream4());

    int status, nDims, nVars, nGlobalAtts, unlimitedDimension;
    status = nc_inq(fileObject->GetFileHandle(), &nDims, &nVars, &nGlobalAtts,
                    &unlimitedDimension);
    debug4 << "nDims=" << nDims
           << ", nVars=" << nVars
           << ", nGlobalAtts=" << nGlobalAtts
           << ", unlimitedDimension=" << unlimitedDimension
           << endl;
    if(status != NC_NOERR)
    {
        fileObject->HandleError(status);
        return;
    }

    if(md != 0)
    {
        std::string gaString;
        CreateGlobalAttributesString(nGlobalAtts, gaString);
        md->SetDatabaseComment(gaString);
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

    // Get the time dimension.
    int timedim = -1, time_nts = 0;
    std::string timedimname;
    GetTimeDimension(fileObject, timedim, time_nts, timedimname);

    meshNameToDimensionsSizes.clear();
    meshNameToNCDimensions.clear();
    varToDimensionsSizes.clear();

    // Iterate over the variables and create a list of meshes names and add
    // the variable to the metadata.
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
            if (varndims == 0)
                continue;

            int nGt1Dims = 0;
            int maxDim = dimSizes[vardims[0]];
            int maxDimIndex = 0;
            bool hasTimeDimension = false;
            for(int dim = 0; dim < varndims; ++dim)
            {
                int d = dimSizes[vardims[dim]];
                if(d > 1)
                    ++nGt1Dims;
                if(d > maxDim)
                {
                    maxDim = d;
                    maxDimIndex = dim;
                }

                if(vardims[dim] == timedim)
                    hasTimeDimension = true;
            }

            if(nGt1Dims == 1)
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
                                                vardims[maxDimIndex], dimName);
                        if(status == NC_NOERR)
                        {
                            cmd->xLabel = dimName;
                            fileObject->ReadStringAttribute(dimName, "units",
                                                            cmd->xUnits);
                        }
                        md->Add(cmd);
                    }

                    intVector meshDims; meshDims.push_back(maxDim);
                    meshNameToDimensionsSizes[varname] = meshDims;
                }
            }
            else
            {
                char tmp[100];
                std::string meshName("mesh");
                intVector meshDims, meshDimSizes, vDims;
                int elems = 1;
                for(int dim = varndims-1; dim >= 0; --dim)
                {
                    int d;
                    if(vardims[dim] != timedim)
                    {
                        vDims.push_back(vardims[dim]);
                        d = dimSizes[vardims[dim]];
                        elems *= d;
                    }
                    else
                        d = TIME_DIMENSION;

                    meshDims.push_back(vardims[dim]);
                    meshDimSizes.push_back(d);
                }

                int maxDimNotOne = meshDimSizes.size();
                while(meshDimSizes[maxDimNotOne-1] == 1 && maxDimNotOne >= 1)
                     maxDimNotOne--;

                // Make sure it's not 1 or 0 in some of its dimensions
                if(meshDimSizes.size() >= 2 && elems != maxDim && elems > 0)
                {
                    // Come up with the mesh name that we'll use for this
                    // variable. Note that we don't consider the time dimension
                    // so time-varying and non-time-varying variables can share
                    // the same dimension if possible.
                    int nSpatialDims = 0;
                    for(int j = 0; j < maxDimNotOne; ++j)
                    {
                        if(meshDimSizes[j] != TIME_DIMENSION)
                        {
                            if(j != 0)
                                SNPRINTF(tmp, 100, "x%d", meshDimSizes[j]);
                            else
                                SNPRINTF(tmp, 100, "%d", meshDimSizes[j]);
                            ++nSpatialDims;
                            meshName += tmp;
                        }
                    }

                    // Add the name of the mesh to the list of meshes.
                    if(meshNameToDimensionsSizes.find(meshName) == meshNameToDimensionsSizes.end())
                    { 
                        // Filter out time from the dimensions so time varying and static
                        // variables can share the same mesh.
                        intVector meshDimsWithoutTime, meshDimSizesWithoutTime;
                        for(int dim = 0; dim < meshDimSizes.size(); ++dim)
                            if(meshDimSizes[dim] != TIME_DIMENSION)
                            {
                                meshDimSizesWithoutTime.push_back(meshDimSizes[dim]);
                                meshDimsWithoutTime.push_back(meshDims[dim]);
                            }
                        meshNameToDimensionsSizes[meshName] = meshDimSizesWithoutTime;
                        meshNameToNCDimensions[meshName] = meshDimsWithoutTime;

                        if(md != 0)
                        {
                            md->SetFormatCanDoDomainDecomposition(true);
                            avtMeshMetaData *mmd = new avtMeshMetaData(meshName, 
                                1, 1, 1, 0, nSpatialDims, nSpatialDims,
                                AVT_RECTILINEAR_MESH);
                            mmd->validVariable = nSpatialDims <= 3;
                            // Provide the dimension names as the axis labels.
                            char dimName[NC_MAX_NAME+1];
                            size_t sz;
                            if((status = nc_inq_dim(fileObject->GetFileHandle(), vDims[0], dimName,
                                &sz)) == NC_NOERR)
                            {
                                mmd->xLabel = dimName;
                                fileObject->ReadStringAttribute(dimName, "units", mmd->xUnits);
                            }
                            if((status = nc_inq_dim(fileObject->GetFileHandle(), vDims[1], dimName,
                                &sz)) == NC_NOERR)
                            {
                                mmd->yLabel = dimName;
                                fileObject->ReadStringAttribute(dimName, "units", mmd->yUnits);
                            }
                            if(nSpatialDims == 3)
                            {
                                if((status = nc_inq_dim(fileObject->GetFileHandle(), vDims[2], dimName,
                                    &sz)) == NC_NOERR)
                                {
                                    mmd->zLabel = dimName;
                                    fileObject->ReadStringAttribute(dimName, "units", mmd->zUnits);
                                }
                            }
                            md->Add(mmd);
                        }
                    }

                    // Try and get the variable units.
                    varToDimensionsSizes[varname] = meshDimSizes;
                    if(md != 0)
                    {
                        avtScalarMetaData *smd = new avtScalarMetaData(varname, meshName,
                            AVT_NODECENT);
                        smd->hasUnits = fileObject->ReadStringAttribute(
                            varname, "units", smd->units);
                        smd->validVariable = nSpatialDims <= 3;
                        md->Add(smd);
                    }
                } 
            }
        }
    }

    delete [] dimSizes;
    meshNamesCreated = true;
}

// ****************************************************************************
// Method: avtBasicNETCDFReader::ReturnSpatialDimensionIndices
//
// Purpose: 
//   Returns the indices of the spatial dimensions.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 29 15:41:42 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

bool
avtBasicNETCDFReader::ReturnSpatialDimensionIndices(const intVector &dims, int sDims[3], int &nSDims) const
{
    int i;
    const char *mName = "avtBasicNETCDFReader::ReturnValidDimensions: ";

    // Look for up to 3 valid spatial dimensions.
    nSDims = 0;
    for(i = 0; i < dims.size() && nSDims < 3; ++i)
    {
        if(dims[i] > 1 && dims[i] != TIME_DIMENSION)
        {
            sDims[nSDims++] = i;
        }
    }

    // Count the number of cells that comprise the spatial dimensions
    int nCells = 1;
    debug5 << mName << "validDims=(";
    for(i = 0; i < nSDims; ++i)
    {
        nCells *= dims[sDims[i]];
        debug5 << dims[sDims[i]] << ", ";
    }
    debug5 << ")" << endl;

    // Count the number of cells that comprise all dimensions (except time).
    int nValues = 1;
    debug5 << mName << "actualDims=(";
    for(i = 0; i < dims.size(); ++i)
    {
        if(dims[i] != TIME_DIMENSION)
            nValues *= dims[i];
        debug5 << dims[i] << ", ";
    }
    debug5 << ")" << endl;

    return nCells == nValues;
}

// ****************************************************************************
// Method: avtBasicNETCDFReader::ReturnDimStartsAndCounts
//
// Purpose: 
//   Returns the dimStarts and dimCounts for all dimensions.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 29 15:41:42 PDT 2009
//
// Modifications:
//   Mark C. Miller, Tue Aug 15 15:28:11 PDT 2006
//   Added code to support on-the-fly domain decomposition 
//
//   Mark C. Miller, Wed Aug 16 14:45:22 PDT 2006
//   Fixed possible exit without initializing all contents of starts/counts
//
//   Mark C. Miller, Tue Dec  5 18:14:58 PST 2006
//   Fixed UMRs
//
// ****************************************************************************

void
avtBasicNETCDFReader::ReturnDimStartsAndCounts(int timeState, const intVector &dims, 
    intVector &dimStarts, intVector &dimCounts) const
{
    //
    // Initialize starts/counts, slicing in time.
    //
    dimStarts.clear();
    dimCounts.clear();
    for (int i = 0; i < dims.size(); i++)
    {
        if(dims[i] == TIME_DIMENSION)
        {
            dimStarts.push_back(timeState);
            dimCounts.push_back(1);
        }
        else
        {
            dimStarts.push_back(0);
            dimCounts.push_back(dims[i]);
        }
    }

    // Get the indices of the spatial dimensions
    int spatialDimIndices[3] = {0,0,0}, nSpatialDims = 0;
    ReturnSpatialDimensionIndices(dims, spatialDimIndices, nSpatialDims);

    // Compute how many values that makes.
    int spatialSizes[3] = {0,0,0};
    int nValues = 1;
    for(int i = 0; i < nSpatialDims; ++i)
    {
        spatialSizes[i] = dims[spatialDimIndices[i]];
        nValues *= spatialSizes[i];
    }

    //
    // We won't decompose something that is smaller than some threshold
    //
    if (nValues < 100000)
        return;

    //
    // Here's where we let VisIt decide how to divide up the spatial dimensions.
    //

    //
    // Above, we're counting nodes (e.g. mesh lines). The decomposition
    // stuff operates on zones.
    //
    int validZDims[3] = {0,0,0};
    for (int i = 0; i < nSpatialDims; i++)
        validZDims[i] = spatialSizes[i]-1;

    //
    // Ok, now compute the zone-oriented domain decomposition
    //
    int domCount[3] = {0, 0, 0};
    avtDatabase::ComputeRectilinearDecomposition(
        nSpatialDims, procCount,
        validZDims[0], validZDims[1], validZDims[2],
        &domCount[0], &domCount[1], &domCount[2]);

    debug5 << "Decomposition: " << domCount[0] << ", "
           << domCount[1] << ", " << domCount[2] << endl;

    //
    // Determine this processor's logical domain (e.g. domain ijk) indices
    //
    int domLogicalCoords[3] = {0, 0, 0};
    avtDatabase::ComputeDomainLogicalCoords(nSpatialDims, domCount, procNum,
        domLogicalCoords);

    debug5 << "Processor " << procNum << " domain logical coords: "
           << domLogicalCoords[0] << ", " << domLogicalCoords[1] << ", "
           << domLogicalCoords[2] << endl;

    //
    // compute the bounds, in terms of output zone numbers,
    // of this processor's domain. Store the divided spatial domain's starts
    // and counts, overwriting the previous values.
    //
    debug5 << "Processor " << procNum << " zone-centered bounds..." << endl;
    for (int i = 0; i < nSpatialDims; i++)
    {
        avtDatabase::ComputeDomainBounds(validZDims[i], domCount[i], domLogicalCoords[i],
            &dimStarts[spatialDimIndices[i]], &dimCounts[spatialDimIndices[i]]);
        dimCounts[spatialDimIndices[i]]++; // convert to # of zones to # of nodes  
        debug5 << "   start[" << i << "] = " << dimStarts[spatialDimIndices[i]]
               << ",  count[" << i << "] = " << dimCounts[spatialDimIndices[i]]-1 << endl;
    }
}

// ****************************************************************************
// Method: avtBasicNETCDFReader::GetMesh
//
// Purpose: 
//   Returns the specified mesh.
//
// Arguments:
//   timeState : The time state.
//   var       : The name of the mesh to create.
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
//   Mark C. Miller, Tue Aug 15 15:28:11 PDT 2006
//   Added code to support on-the-fly domain decomp
//
//   Kathleen Bonnell, Mon Jul 14 14:07:39 PDT 2008
//   Specify curves as 1D rectilinear grids with yvalues stored in point data.
//
//   Brad Whitlock, Thu Oct 29 15:38:29 PDT 2009
//   I rewrote it with support for time.
//
// ****************************************************************************

vtkDataSet *
avtBasicNETCDFReader::GetMesh(int timeState, const char *var)
{
    const char *mName = "avtBasicNETCDFReader::GetMesh: ";
    debug4 << mName << "var=" << var << endl;
    vtkDataSet *retval = 0;
 
    // Populate the mesh names if we've not done so yet.
    if(!meshNamesCreated)
        PopulateDatabaseMetaData(timeState, 0);

    StringIntVectorMap::const_iterator mesh = meshNameToDimensionsSizes.find(var);
    if(mesh != meshNameToDimensionsSizes.end())
    {
        if(mesh->second.size() == 1)
        {
            // Try and read the variable making up the curve.
            int nPts = mesh->second[0];
            vtkFloatArray *yv = vtkFloatArray::New();
            yv->SetNumberOfTuples(nPts); 
            if(fileObject->ReadVariableIntoAsFloat(var, (float*)yv->GetVoidPointer(0)))
            {
                vtkRectilinearGrid *rg = vtkVisItUtility::Create1DRGrid(
                                         nPts, VTK_FLOAT);
                vtkFloatArray *xc = vtkFloatArray::SafeDownCast(
                                        rg->GetXCoordinates());
                yv->SetName(var);
                for (int j = 0 ; j < nPts ; j++)
                    xc->SetValue(j, (float)j); 

                rg->GetPointData()->SetScalars(yv);
                retval = rg;
            }

            yv->Delete();
        }
        else
        {
            vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New(); 

            // Get the dim starts and counts.
            intVector dimStarts, dimCounts;
            ReturnDimStartsAndCounts(timeState, mesh->second, dimStarts, dimCounts);

            // Get the indices of the spatial dimensions
            int spatialDimIndices[3] = {0,0,0}, nSpatialDims = 0;
            ReturnSpatialDimensionIndices(mesh->second, spatialDimIndices, nSpatialDims);
             
            //
            // Populate the coordinates.  Put in 3D points with z=0 if
            // the mesh is 2D.
            //
            const char *xyzname[] = {"X", "Y", "Z"};
            int            dims[3] = {0,0,0};
            vtkFloatArray *coords[3] = {0,0,0};
            for (int i = 0 ; i < 3 ; i++)
            {
                // Default number of components for an array is 1.
                coords[i] = vtkFloatArray::New();

                if (i < nSpatialDims)
                {
                    // Get the name of this dimension and try and read an array by the same 
                    // name, if one exists and is the same size as the dimension. We'll use 
                    // it for the mesh's coordinate values.
                    float *coordvals = 0;
                    char dimName[NC_MAX_NAME+1];
                    StringIntVectorMap::const_iterator meshDims = meshNameToNCDimensions.find(var);
                    if(meshDims != meshNameToNCDimensions.end())
                    {
                        int status;
                        size_t sz;
                        debug4 << mName << "Looking for " << xyzname[i]
                               << " coordinate dimension name" << endl;
                        if((status = nc_inq_dim(fileObject->GetFileHandle(), 
                            meshDims->second[spatialDimIndices[i]], dimName, &sz)) == NC_NOERR)
                        {
                            TypeEnum dvart = NO_TYPE;
                            int dvarndims = 0;
                            int *dvardims = 0;
                            debug4 << mName << "Looking for " << dimName << " array as "
                                   << xyzname[i] << " coordinate" << endl;
                            if(fileObject->InqVariable(dimName, &dvart, &dvarndims, &dvardims))
                            {
                                if(dvarndims == 1 && dvardims[0] == sz)
                                    coordvals = ReadArray(dimName);
                                delete [] dvardims;
                            }
                        }
                    }

                    // Populate the coordinates
                    dims[i] = dimCounts[spatialDimIndices[i]];
                    coords[i]->SetNumberOfTuples(dims[i]);
                    if(coordvals != 0)
                    {
                        debug4 << mName << "Using " << dimName << " array as "
                                   << xyzname[i] << " coordinate" << endl;
                        int start = dimStarts[spatialDimIndices[i]];
                        for (int j = 0 ; j < dims[i] ; j++)
                            coords[i]->SetComponent(j, 0, coordvals[start + j]);
                    }
                    else
                    {
                        debug4 << mName << "Using indices for " << xyzname[i] << " coordinate" << endl;
                        for (int j = 0 ; j < dims[i] ; j++)
                            coords[i]->SetComponent(j, 0, dimStarts[spatialDimIndices[i]] + j);
                    }

                    if(coordvals != 0)
                        delete [] coordvals;
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
// Method: avtBasicNETCDFReader::GetVar
//
// Purpose: 
//   Returns the data for the specified variable.
//
// Arguments:
//   timeState : The time state to read.
//   var       : The name of the variable to read.
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
//   Mark C. Miller, Tue Aug 15 15:28:11 PDT 2006
//   Added logic to support on-the-fly domain decomposition. Added macro to
//   do a partial read
//
//   Mark C. Miller, Wed Aug 16 14:45:22 PDT 2006
//   Fixed reversal of coords between VisIt and netcdf
//   
//   Jeremy Meredith, Thu Mar 22 15:05:02 EDT 2007
//   Since we're calling InqVariable here, we need to take the same steps
//   as we do when reading the mesh dims to remove trailing 1's.  Also,
//   print to debug4 the valid dimensions array instead of the raw one.
//
//   Jeremy Meredith, Wed Apr  4 14:01:40 EDT 2007
//   Adjusted to account for an occasional "4-dimensional" mesh.
//
//   Brad Whitlock, Thu Oct 29 15:23:32 PDT 2009
//   I rewrote the code so it can read variables that have a time dimension.
//
// ****************************************************************************

#define READVAR(VTKTYPE) \
        {\
            VTKTYPE *arr = VTKTYPE::New();\
            arr->SetNumberOfComponents(1);\
            arr->SetNumberOfTuples(nValues);\
            debug4 << "Allocated a " << \
                    #VTKTYPE \
                   << " of " << nValues << " elements" << endl; \
            int *rdimStarts = new int[dimStarts.size()]; \
            int *rdimCounts = new int[dimCounts.size()]; \
            for (int kk = 0; kk < dimStarts.size(); kk++)\
            {\
                rdimStarts[ndims-kk-1] = dimStarts[kk];\
                rdimCounts[ndims-kk-1] = dimCounts[kk];\
            }\
            if(fileObject->ReadVariableInto(var, t, rdimStarts, rdimCounts,\
                                            arr->GetVoidPointer(0)))\
                retval = arr;\
            else\
                arr->Delete();\
            delete [] rdimStarts;\
            delete [] rdimCounts;\
        }


vtkDataArray *
avtBasicNETCDFReader::GetVar(int timeState, const char *var)
{
    const char *mName = "avtBasicNETCDFReader::GetVar: ";
    debug4 << mName << "var=" << var << endl;

    vtkDataArray *retval = 0;

    // Determine the storage type for the variable.
    TypeEnum t = NO_TYPE;
    int ndims = 0;
    int *dims = 0;
    if(!fileObject->InqVariable(var, &t, &ndims, &dims))
    {
        EXCEPTION1(InvalidVariableException, var);
    }
    delete [] dims;

    // Look up the mesh dimensions for the mesh.
    StringIntVectorMap::const_iterator minfo = varToDimensionsSizes.find(var);
    if(minfo != meshNameToDimensionsSizes.end())
    {
        // Get the dim starts and counts.
        intVector dimStarts, dimCounts;
        ReturnDimStartsAndCounts(timeState, minfo->second, dimStarts, dimCounts);

        // Compute how many values that makes.
        unsigned long nValues = 1;
        for(int i = 0; i < dimCounts.size(); ++i)
            nValues *= (unsigned long)dimCounts[i];

        debug4 << mName << "dimStarts = {";
        for(int i = 0; i < dimStarts.size(); ++i)
            debug4 << dimStarts[i] << ", ";
        debug4 << "}\n";
        debug4 << mName << "dimCounts = {";
        for(int i = 0; i < dimCounts.size(); ++i)
            debug4 << dimCounts[i] << ", ";
        debug4 << "}\n";

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
    }
    else
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    return retval;
}

