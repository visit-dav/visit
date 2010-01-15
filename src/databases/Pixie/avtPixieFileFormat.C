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

// ************************************************************************* //
//                            avtPixieFileFormat.C                           //
// ************************************************************************* //

// define this to make sure the plugin always serves up float data to VisIt
// this was necessary prior to avtGenericDatabase having generic conversion
// capability
//#define FORCE_FLOATS

#include <avtPixieFileFormat.h>

#include <algorithm>
#include <string>

#include <vtkCellType.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkIntArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>
#include <DebugStream.h>
#include <Expression.h>
#include <InvalidVariableException.h>
#include <InvalidDBTypeException.h>
#include <InvalidFilesException.h>
#include <InvalidTimeStepException.h>
#include <snprintf.h>

// Define this symbol BEFORE including hdf5.h to indicate the HDF5 code
// in this file uses version 1.6 of the HDF5 API. This is harmless for
// versions of HDF5 before 1.8 and ensures correct compilation with
// version 1.8 and thereafter. When, and if, the HDF5 code in this file
// is explicitly upgraded to the 1.8 API, this symbol should be removed.
#define H5_USE_16_API
#include <hdf5.h>
#include <visit-hdf5.h>


#define MAKE_SURE_THE_FILE_IS_NOT_OTHER_FORMAT

// ****************************************************************************
//  Method: avtPixie constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Aug 13 14:31:43 PST 2004
//
// ****************************************************************************

avtPixieFileFormat::avtPixieFileFormat(const char *filename)
    : avtMTSDFileFormat(&filename, 1), variables(), meshes(),
    timeStatePrefix("/Timestep ")
{
     fileId = -1;
     nTimeStates = 0;
     haveMeshCoords = false;

#ifdef MAKE_SURE_THE_FILE_IS_NOT_OTHER_FORMAT
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
//    Eric Brugger, Mon Nov 29 15:52:39 PST 2004
//    Modified the reader to handle gaps in the cycle numbering (e.g. allowing
//    0, 10, 20, 30 instead of requiring 0, 1, 2, 3).
//
//    Brad Whitlock, Thu Apr 27 11:49:07 PDT 2006
//    Fixed it so it works if cycles are never read.
//
// ****************************************************************************

void
avtPixieFileFormat::GetCycles(std::vector<int> &cycles)
{
    int nts = (nTimeStates < 1) ? 1 : nTimeStates;
    int lastCycle = 0;
    for(int i = 0; i < nts; ++i)
    {
        if(i < this->cycles.size())
        {
            cycles.push_back(this->cycles[i]);
            lastCycle = this->cycles[i];
        }
        else
        {
            cycles.push_back(lastCycle++);
        }
    }
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
//    Eric Brugger, Mon Nov 29 15:52:39 PST 2004
//    Modified the reader to handle gaps in the cycle numbering (e.g. allowing
//    0, 10, 20, 30 instead of requiring 0, 1, 2, 3).
//
//    Brad Whitlock, Thu Apr 27 11:49:07 PDT 2006
//    Fixed it so it works if cycles are never read.
//
// ****************************************************************************

void
avtPixieFileFormat::GetTimes(std::vector<double> &times)
{
    int nts = (nTimeStates < 1) ? 1 : nTimeStates;
    double lastTime = 0.;
    for(int i = 0; i < nts; ++i)
    {
        if(i < cycles.size())
        {
            times.push_back(double(cycles[i]));
            lastTime = double(cycles[i]);
        }
        else
        {
            times.push_back(lastTime);
            lastTime = lastTime + 1.;
        }
    }
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
//   Brad Whitlock, Wed Sep 15 17:45:41 PST 2004
//   Added better support for determining whether arrays have coordinates.
//
//   Eric Brugger, Mon Nov 29 15:52:39 PST 2004
//   Modified the reader to handle gaps in the cycle numbering (e.g. allowing
//   0, 10, 20, 30 instead of requiring 0, 1, 2, 3).
//
//   Mark C. Miller, Mon Dec  6 14:13:11 PST 2004
//   Added std:: name resolution to call to sot
//
//   Mark C. Miller, Mon Apr  4 14:55:14 PDT 2005
//   Added expressions
//
//   Hank Childs, Wed Jul  9 06:02:00 PDT 2008
//   Added test for UNIC.
//
//   Gunther H. Weber, Wed Oct  8 16:50:31 PDT 2008
//   Added test for TechX VizSchema
//
//   Jeremy Meredith, Thu Jan  7 15:36:19 EST 2010
//   Close all open ids when returning an exception.  Added error detection.
//
//   Jeremy Meredith, Fri Jan 15 17:07:33 EST 2010
//   Added detection of and failure for Silo-looking HDF5 files.
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

#ifdef MAKE_SURE_THE_FILE_IS_NOT_OTHER_FORMAT
        // Turn off error message printing.
        H5Eset_auto(0,0);

        //
        // See if the file format looks like a Tetrad file. I know it's
        // hackish to have to check like this but how else should it be
        // done when we don't want Pixie to read HDF5 files that happen
        // to have Tetrad stuff in them.
        //
        hid_t siloDir = H5Gopen(fileId, ".silo");
        if (siloDir >= 0)
        {
            H5Gclose(siloDir);
            H5Fclose(fileId);
            EXCEPTION1(InvalidDBTypeException,
                       "Cannot be a Pixie file because it looks like a Silo file.");
        }
        hid_t cell_array = H5Dopen(fileId, "CellArray");
        if (cell_array >= 0)
        {
            H5Dclose(cell_array);
            H5Fclose(fileId);
            EXCEPTION1(InvalidDBTypeException,
               "Cannot be a Pixie file because it looks like a Tetrad file.");
        }
        hid_t control = H5Dopen(fileId, "CONTROL");
        if (control >= 0)
        {
            H5Dclose(control);
            H5Fclose(fileId);
            EXCEPTION1(InvalidDBTypeException,
               "Cannot be a Pixie file because it looks like an UNIC file.");
        }
        hid_t runInfo = H5Gopen(fileId, "runInfo");
        if (runInfo >= 0)
        {
            hid_t vsVersion = H5Aopen_name(runInfo, "vsVersion");
            if (vsVersion >= 0)
            {
                H5Aclose(vsVersion);
                H5Gclose(runInfo);
                H5Fclose(fileId);
                EXCEPTION1(InvalidDBTypeException,
                        "Cannot be a Pixie file because it looks like a VizSchema file.");
            }

            hid_t vsVsVersion = H5Aopen_name(runInfo, "vsVsVersion");
            if (vsVsVersion >= 0)
            {
                H5Aclose(vsVsVersion);
                H5Gclose(runInfo);
                H5Fclose(fileId);
                EXCEPTION1(InvalidDBTypeException,
                        "Cannot be a Pixie file because it looks like a VizSchema file.");
            }

            hid_t software = H5Aopen_name(runInfo, "software");
            hid_t version = H5Aopen_name(runInfo, "version");
            H5Gclose(runInfo);
            if (software >=0 && version >=0)
            {
                H5Aclose(software);
                H5Aclose(version);
                H5Fclose(fileId);
                EXCEPTION1(InvalidDBTypeException,
                        "Cannot be a Pixie file because it looks like a legacy VizSchema file.");
            }
            if (software >=0) H5Aclose(software);
            if (version >=0) H5Aclose(version);
        }
#endif
        // Populate the scalar variable list
        int gid;
        if ((gid = H5Gopen(fileId, "/")) < 0)
        {
            H5Fclose(fileId);
            EXCEPTION1(InvalidFilesException, (const char *)filenames[0]);
        }
        TraversalInfo info;
        info.This = this; 
        info.level = 0;
        info.path = "/";
        info.hasCoords = false;
        info.coordX = "";
        info.coordY = "";
        info.coordZ = "";
        H5Giterate(fileId, "/", NULL, GetVariableList, (void*)&info);
        H5Gclose(gid);

        // Determine the names of the meshes that we'll need.
        for(VarInfoMap::const_iterator it = variables.begin();
            it != variables.end(); ++it)
        {
            const char *mNames[] = {"mesh", "curvemesh"};
            char tmp[100];
            SNPRINTF(tmp, 100, "%s_%dx%dx%d", mNames[it->second.hasCoords?1:0],
                     int(it->second.dims[2]),
                     int(it->second.dims[1]),
                     int(it->second.dims[0]));
            meshes[std::string(tmp)] = it->second;
        }

        //
        // Look for expressions dataset
        //
        int expid;
        if ((expid = H5Dopen(fileId,"/visit_expressions")) >= 0)
        {
            // examine size, dimensionality and type of the dataspace
            hid_t spid    = H5Dget_space(expid);
            hid_t tyid    = H5Dget_type(expid); 
            hsize_t hsize = H5Dget_storage_size(expid);
            int ndims     = H5Sget_simple_extent_ndims(spid);

            // should be a 1D, character data set
            if (ndims != 1 || H5Tget_class(tyid) != H5T_STRING)
            {
                EXCEPTION2(InvalidFilesException, (const char *)filenames[0],
                    "The dataset \"visit_expressions\" is not a 1D, character dataset");
            }

            // allocate and read
            char *expChars = new char[hsize+1];
            if (H5Dread(expid, tyid, H5S_ALL, H5S_ALL, H5P_DEFAULT, expChars) < 0)
            {
                EXCEPTION1(InvalidVariableException, "/visit_expressions");
            }
            expChars[hsize] = '\0';

            rawExpressionString = expChars;
            delete [] expChars;

            H5Tclose(tyid);
            H5Sclose(spid);
            H5Dclose(expid);
        }

        // Sort the cycles.
        std::sort(cycles.begin(), cycles.end());

#ifdef MDSERVER
        // We're on the mdserver so close the file now that we've determined
        // the variables in it.
        H5Fclose(fileId);
        fileId = -1;
#endif
    }
}

// ****************************************************************************
// Method: avtPixieFileFormat::ActivateTimestep
//
// Purpose: 
//   This method is called each time we change to a new time state. Make
//   sure that the file has been initialized.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 14 12:53:08 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
avtPixieFileFormat::ActivateTimestep(int ts)
{
    //
    // Initialize the file if it has not been initialized.
    //
    debug4 << "avtPixieFileFormat::ActivateTimestep: ts=" << ts << endl;
    Initialize();
}

// ****************************************************************************
// Method: avtPixieFileFormat::DetermineVarDimensions
//
// Purpose: 
//   Gets the dimensions of a variable.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 19 10:57:09 PDT 2004
//
// Modifications:
//   Eric Brugger, Tue Oct 26 08:36:53 PDT 2004
//   I modified the routine to return the hyperslab to read of an array.  I
//   also modified the routine to handle the fact that any arrays associated
//   with a curvilinear mesh that are 2*nx*ny should be treated as 2d.
//
// ****************************************************************************

void
avtPixieFileFormat::DetermineVarDimensions(const VarInfo &info,
    hsize_t *hyperslabDims, int *varDims, int &nVarDims) const
{
    //
    // If the mesh is rectilinear, then 1*nx*ny arrays should be treated
    // as 2d, if the mesh is curvilinear (hasCoords), then 2*nx*ny arrays
    // should be treated as 2d.
    //
    int size1D = 1;
    if (info.hasCoords)
    {
        size1D = 2;
    }

    //
    // Determine the hyperslab dimensions.
    //
    if (hyperslabDims != 0)
    {
        hyperslabDims[0] = (info.dims[0] > size1D) ? info.dims[0] : 1;
        hyperslabDims[1] = (info.dims[1] > size1D) ? info.dims[1] : 1;
        hyperslabDims[2] = (info.dims[2] > size1D) ? info.dims[2] : 1;
    }

    //
    // Determine the dimensions for the mesh.
    //
    if(varDims != 0)
    {
        int di = 0;
        varDims[0] = 1; varDims[1] = 1; varDims[2] = 1;

        if(info.dims[0] > size1D)
            varDims[di++] = int(info.dims[0]);
        if(info.dims[1] > size1D)
            varDims[di++] = int(info.dims[1]);
        if(info.dims[2] > size1D)
            varDims[di++] = int(info.dims[2]);
    }

    //
    // Determine the number of spatial dimensions of the variable.
    //
    nVarDims = 0;
    if(info.dims[0] > size1D) ++nVarDims;
    if(info.dims[1] > size1D) ++nVarDims;
    if(info.dims[2] > size1D) ++nVarDims;
}

// ****************************************************************************
// Method: avtPixieFileFormat::MeshIsCurvilinear
//
// Purpose: 
//   Returns whether the named mesh is curvilinear.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 15 08:45:29 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Sep 15 17:06:28 PST 2004
//   I changed how the support for mesh coordinates is handled.
//
//   Eric Brugger, Tue Oct 26 08:36:53 PDT 2004
//   I modified the routine to handle the fact that variables defined on a
//   curvilinear mesh are now nodal.  I also simplified the logic to compare
//   the raw array sizes instead of the reduced sizes since those should
//   also match.
//
// ****************************************************************************

bool
avtPixieFileFormat::MeshIsCurvilinear(const std::string &name) const
{
    bool retval = false;

    if (haveMeshCoords)
    {
        VarInfoMap::const_iterator mesh = meshes.find(name);

        if (mesh != meshes.end() && mesh->second.hasCoords)
        {
            VarInfoMap::const_iterator xvar = variables.find(
                mesh->second.coordX);
            VarInfoMap::const_iterator yvar = variables.find(
                mesh->second.coordY);
            VarInfoMap::const_iterator zvar = variables.find(
                mesh->second.coordZ);

            int dims[3], nSpatialDims = 3;
            dims[0] = int(mesh->second.dims[0]);
            dims[1] = int(mesh->second.dims[1]);
            dims[2] = int(mesh->second.dims[2]);
            int xDims[3], nXDims = 3;
            xDims[0] = int(xvar->second.dims[0]);
            xDims[1] = int(xvar->second.dims[1]);
            xDims[2] = int(xvar->second.dims[2]);
            int yDims[3], nYDims = 3;
            yDims[0] = int(yvar->second.dims[0]);
            yDims[1] = int(yvar->second.dims[1]);
            yDims[2] = int(yvar->second.dims[2]);
            int zDims[3], nZDims = 3;
            zDims[0] = int(zvar->second.dims[0]);
            zDims[1] = int(zvar->second.dims[1]);
            zDims[2] = int(zvar->second.dims[2]);

            bool same = true;
            for(int i = 0; i < nSpatialDims && same; ++i)
            {
                same &= (xDims[i] == yDims[i]);
                same &= (yDims[i] == zDims[i]);
                same &= (zDims[i] == dims[i]);
            }

            retval = same;
        }
    }

    return retval;
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
//  Modifications:
//    Brad Whitlock, Mon Aug 16 13:50:41 PST 2004
//    Added support for a curvilinear mesh and an optional point mesh for
//    debugging.
//
//    Eric Brugger, Tue Oct 26 08:36:53 PDT 2004
//    I modified the routine to handle the fact that variables defined on a
//    curvilinear mesh are now nodal.  I also modified the call to Determine
//    VarDimensions since an argument was added to it.
//
//    Mark C. Miller, Mon Apr  4 14:55:14 PDT 2005
//    Added expressions
//
//    Jeremy Meredith, Mon Apr  4 17:05:32 PDT 2005
//    Added "std::" prefix to string constructors.
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added timeState arg to satisfy new interface
// ****************************************************************************

void
avtPixieFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md,
    int timeState)
{
//#define ADD_POINT_MESH
    VarInfoMap::const_iterator it;
#ifdef ADD_POINT_MESH
    int pmnDims = -1;
    VarInfo pm;
#endif
    for(it = meshes.begin();
        it != meshes.end(); ++it)
    {
        // Determine the number of spatial dimensions.
        int nSpatialDims = 0;
        DetermineVarDimensions(it->second, 0, 0, nSpatialDims);
        if(nSpatialDims == 0)
            continue;

        // Add the mesh.
        avtMeshMetaData *mmd = new avtMeshMetaData;
        mmd->name = it->first;
        mmd->spatialDimension = nSpatialDims;
        mmd->topologicalDimension = nSpatialDims;

        // Determine the mesh type. Usually it will be rectilinear but
        // sometimes, if we have the right kind of coordinate arrays, it
        // could be curvilinear.
        mmd->meshType = MeshIsCurvilinear(it->first) ? AVT_CURVILINEAR_MESH : 
            AVT_RECTILINEAR_MESH;

        mmd->cellOrigin = 1;
        md->Add(mmd);

#ifdef ADD_POINT_MESH
        // If we had a curvilinear mesh, add a point mesh to aid in debugging.
        if(mmd->meshType == AVT_CURVILINEAR_MESH)
        {
            pmnDims = nSpatialDims;
            pm.dims[0] = it->second.dims[0];
            pm.dims[1] = it->second.dims[1];
            pm.dims[2] = it->second.dims[2];
        }
#endif
    }

#ifdef ADD_POINT_MESH
    if(pmnDims != -1)
    {
        meshes["pointmesh"] = pm;

        avtMeshMetaData *mmd = new avtMeshMetaData;
        mmd->name = "pointmesh";
        mmd->spatialDimension = pmnDims;
        mmd->topologicalDimension = 0;
        mmd->meshType = AVT_POINT_MESH;
        mmd->cellOrigin = 1;
        md->Add(mmd);
    }
#endif

    // If we have more than 5 meshes, enable catchall mesh.
    if(meshes.size() > 5)
        md->SetUseCatchAllMesh(true);

    // Iterate through the variables and add them to the metadata.
    for(it = variables.begin();
        it != variables.end(); ++it)
    {
        // Determine the mesh name based on the variable mesh size.
        const char *mNames[] = {"mesh", "curvemesh"};
        char tmp[100];
        SNPRINTF(tmp, 100, "%s_%dx%dx%d", mNames[it->second.hasCoords?1:0],
                 int(it->second.dims[2]),
                 int(it->second.dims[1]),
                 int(it->second.dims[0]));

        // Add a zonal scalar to the metadata.
        if (it->second.hasCoords)
            AddScalarVarToMetaData(md, it->first, tmp, AVT_NODECENT);
        else
            AddScalarVarToMetaData(md, it->first, tmp, AVT_ZONECENT);
    }

#ifdef ADD_POINT_MESH
    if(pmnDims != -1)
    {
        variables["pointvar"] = pm;
        AddScalarVarToMetaData(md, "pointvar", "pointmesh", AVT_NODECENT);
    }
#endif

    //
    // Add expressions
    //
    if (rawExpressionString.size())
    {
        std::string::size_type s = 0;
        while (s != std::string::npos)
        {
            std::string::size_type nexts = rawExpressionString.find_first_of(";", s);
            std::string exprStr;
            if (nexts != std::string::npos)
            {
                exprStr = std::string(rawExpressionString,s,nexts-s);
                nexts += 1;
            }
            else
            {
                exprStr = std::string(rawExpressionString,s,std::string::npos);
            }

            // remove offending chars from exprStr (spaces)
            std::string newExprStr;
            for (int i = 0; i < exprStr.size(); i++)
            {
                if (exprStr[i] != ' ')
                    newExprStr += exprStr[i];
            }

            std::string::size_type t = newExprStr.find_first_of(':');

            Expression vec;
            vec.SetName(std::string(newExprStr,0,t));
            vec.SetDefinition(std::string(newExprStr,t+1,std::string::npos));
            vec.SetType(Expression::VectorMeshVar);
            md->AddExpression(&vec);

            s = nexts;
        }
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
//  Modifications:
//    Brad Whitlock, Tue Aug 24 12:42:07 PDT 2004
//    Added support for a curvilinear mesh.
//
//    Brad Whitlock, Wed Sep 15 22:09:24 PST 2004
//    I reversed the dimensions for the 3D rectilinear mesh so the variables
//    would display correctly on it.
//
//    Eric Brugger, Tue Oct 26 08:36:53 PDT 2004
//    I modified the routine to handle the fact that variables defined on a
//    curvilinear mesh are now nodal.
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
    std::string meshNameString(meshname);
    VarInfoMap::const_iterator it = meshes.find(meshNameString);
    if(it == meshes.end())
    {
        EXCEPTION1(InvalidVariableException, meshNameString);
    }

    // The dims are being copied to a temporary array before being output
    // to get around a compiler (STL) deficiency with g++-3.3.3 on tru64.
    int originalDims[3];
    originalDims[0] = it->second.dims[0];
    originalDims[1] = it->second.dims[1];
    originalDims[2] = it->second.dims[2];
    debug4 << "avtPixieFileFormat::GetMesh: 0: "
           << "originalDims={" << originalDims[0]
           << ", " << originalDims[1]
           << ", " << originalDims[2] << "}" << endl;

    //
    // Determine the number of cells in each dimension. Note that 
    // DetermineVarDimensions may throw out dimensions with 1 or 2
    // depending on wether the variable is nodal or zonal. So
    // 1x33x33 could become 33x33. When we use nVarDims hereafter,
    // it will contain the reduced number of dimensions if any
    // reduction has been done.
    //
    hsize_t hyperslabDims[3];
    int varDims[3];
    int nVarDims;
    DetermineVarDimensions(it->second, hyperslabDims, varDims, nVarDims);
    if(nVarDims < 2)
    {
        EXCEPTION1(InvalidVariableException, meshNameString);
    }

    debug4 << "avtPixieFileFormat::GetMesh: 1: nVarDims=" << nVarDims 
           << ", varDims={" << varDims[0] << ", " << varDims[1] << ", "
           << varDims[2] << "}" << endl;

    // Try to create a point or curvilinear mesh.
    vtkDataSet *retval = 0;
    if(meshNameString == "pointmesh")
        retval = CreatePointMesh(timestate, it->second, hyperslabDims,
                                 varDims, nVarDims);
    else if(MeshIsCurvilinear(meshname))
        retval = CreateCurvilinearMesh(timestate, it->second, hyperslabDims,
                                       varDims, nVarDims);

    // If the mesh isn't a point or curvilinear mesh, then create a
    // rectilinear mesh.
    if(retval == 0)
    {
        //
        // Add 1 so we have the number of nodes instead of #cells.
        //
        ++varDims[0];
        ++varDims[1];
        if(nVarDims == 3)
            ++varDims[2];

        // Reverse X,Z dimensions so the mesh is drawn properly.
        if(nVarDims == 3)
        {
            int tmp = varDims[0];
            varDims[0] = varDims[2];
            varDims[2] = tmp;
        }
        else if (nVarDims == 2)
        {
            int tmp = varDims[0];
            varDims[0] = varDims[1];
            varDims[1] = tmp;
        }

        vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
        vtkFloatArray *coords[3];
        for (int i = 0 ; i < 3 ; i++)
        {
            // Default number of components for an array is 1.
            coords[i] = vtkFloatArray::New();

            if (i < nVarDims)
            {
                coords[i]->SetNumberOfTuples(varDims[i]);
                for (int j = 0; j < varDims[i]; j++)
                    coords[i]->SetComponent(j, 0, j);
            }
            else
            {
                varDims[i] = 1;
                coords[i]->SetNumberOfTuples(1);
                coords[i]->SetComponent(0, 0, 0.);
            }
        }

        rgrid->SetDimensions(varDims);
        rgrid->SetXCoordinates(coords[0]);
        coords[0]->Delete();
        rgrid->SetYCoordinates(coords[1]);
        coords[1]->Delete();
        rgrid->SetZCoordinates(coords[2]);
        coords[2]->Delete();
        retval = rgrid;
    }

    return retval;
}

// ****************************************************************************
// Method: avtPixieFileFormat::CreatePointMesh
//
// Purpose: 
//   Creates a point mesh.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 15 08:43:28 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Sep 15 17:18:31 PST 2004
//   Added support for reading custom coordinate arrays.
//
//   Eric Brugger, Tue Oct 26 08:36:53 PDT 2004
//   I modified the routine to handle the fact that variables defined on a
//   curvilinear mesh are now nodal.
//
// ****************************************************************************

vtkDataSet *
avtPixieFileFormat::CreatePointMesh(int timestate, const VarInfo &info,
    const hsize_t *hyperslabDims, const int *varDims, int nVarDims) const
{
    vtkDataSet *ds = 0;
    float *coords[3] = {0,0,0};

    //
    // Try and read the coordinate fields.
    //
    if(ReadCoordinateFields(timestate, info, coords, hyperslabDims, nVarDims))
    {
        //
        // Populate the coordinates. Put in 3D points with z=0 if the mesh
        // is 2D.
        //
        int i, nPoints = varDims[0] * varDims[1] * ((nVarDims > 2) ? varDims[2] : 1);        
        vtkPoints *points  = vtkPoints::New();
        points->SetNumberOfPoints(nPoints);
        float *pts = (float *) points->GetVoidPointer(0);
        for(i = 0; i < 3; ++i)
        {
            float *tmp = pts + i;
            if(nVarDims > 2)
            {
                float *coord = coords[i];
                for(int j = 0; j < nPoints; ++j)
                {
                    *tmp = *coord++;
                    tmp += 3;
                }
            }
            else
            {
                for (int j = 0; j < nPoints; ++j)
                {
                    *tmp = 0.f;
                    tmp += 3;
                }
            }
        }

        //
        // Create the VTK objects and connect them up.
        //
        vtkUnstructuredGrid  *ugrid = vtkUnstructuredGrid::New(); 
        ugrid->SetPoints(points);
        ugrid->Allocate(nPoints);
        vtkIdType onevertex;
        for(i = 0; i < nPoints; ++i)
        {
            onevertex = i;
            ugrid->InsertNextCell(VTK_VERTEX, 1, &onevertex);
        }

        points->Delete();
        ds = ugrid;
    }

    return ds;
}

// ****************************************************************************
// Method: avtPixieFileFormat::CreateCurvilinearMesh
//
// Purpose: 
//   Returns a curvilinear mesh.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 15 08:38:31 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Sep 15 17:19:05 PST 2004
//   Added support for custom coordinate arrays.
//
//   Eric Brugger, Tue Oct 26 08:36:53 PDT 2004
//   I modified the routine to handle the fact that variables defined on a
//   curvilinear mesh are now nodal.
//
//   Eric Brugger, Wed Dec 22 07:56:05 PST 2004
//   I added back some code that I inadvertently deleted that caused the
//   reading of 3d meshes to fail.
//
// ****************************************************************************

vtkDataSet *
avtPixieFileFormat::CreateCurvilinearMesh(int timestate, const VarInfo &info,
    const hsize_t *hyperslabDims, const int *varDims, int nVarDims)
{
    vtkDataSet *retval = 0;

    //
    // Try and read the coordinate fields.
    //
    float *coords[3] = {0,0,0};
    if(ReadCoordinateFields(timestate, info, coords, hyperslabDims, nVarDims))
    {
        //
        // Create the VTK objects and connect them up.
        //
        vtkStructuredGrid *sgrid  = vtkStructuredGrid::New(); 
        vtkPoints         *points = vtkPoints::New();
        sgrid->SetPoints(points);
        points->Delete();

        //
        // Tell the grid what its dimensions are and populate the points array.
        //
        if(nVarDims == 2)
        {
            int yxzNodes[] = {varDims[1], varDims[0], varDims[2]};
            sgrid->SetDimensions((int *)yxzNodes);
        }
        else
        {
            // In 3D, Pixie dimensions are stored ZYX. Reverse them so we
            // give the right order to VTK.
            int xyzNodes[] = {varDims[2], varDims[1], varDims[0]};
            sgrid->SetDimensions(xyzNodes);
        }
        int nMeshNodes = varDims[0] * varDims[1] * varDims[2];

        //
        // Populate the coordinates.  Put in 3D points with z=0 if the mesh is 2D.
        //
        points->SetNumberOfPoints(nMeshNodes);
        float *pts = (float *) points->GetVoidPointer(0);
        int       i, j, k;
        int       nx, ny, nz;
        float    *coord0, *coord1, *coord2;
        float    *tmp = pts;

        switch (nVarDims)
        {
        case 2:
            nx = varDims[0];
            ny = varDims[1];
            coord0 = coords[0];
            coord1 = coords[1];

            for (j = 0; j < ny; j++)
            {
                for (i = 0; i < nx; i++)
                {
                    *tmp++ = *coord0++;
                    *tmp++ = *coord1++;
                    *tmp++ = 0.;
                }
            }
            break;
        case 3:
            // If things are 3D then the varDims array did not get reduced
            // in the DetermineVarDimensions call in GetMesh so the numbers
            // of dimensions will be stored Z,Y,X.
            nx = varDims[2];
            ny = varDims[1];
            nz = varDims[0];
            coord0 = coords[0];
            coord1 = coords[1];
            coord2 = coords[2];

            for (k = 0; k < nz; k++)
            {
                for (j = 0; j < ny; j++)
                {
                    for (i = 0; i < nx; i++)
                    {
                        *tmp++ = *coord0++;
                        *tmp++ = *coord1++;
                        *tmp++ = *coord2++;
                    }
                }
            }
            break;
        }

        retval = sgrid;
    }

    // free up coord data
    if (coords[0] != 0) delete [] coords[0];
    if (coords[1] != 0) delete [] coords[1];
    if (coords[2] != 0) delete [] coords[2];

    return retval;
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
//  Modifications:
//    Eric Brugger, Tue Oct 26 08:36:53 PDT 2004
//    I modified the routine to handle the fact that variables defined on a
//    curvilinear mesh are now nodal.
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

#ifdef ADD_POINT_MESH
    if(it->first == "pointvar")
    {
        int nels = it->second.dims[0] * 
                      it->second.dims[1] * 
                      it->second.dims[2];
        vtkFloatArray *fscalars = vtkFloatArray::New();
        fscalars->SetNumberOfTuples(nels);
        float *data = (float *)fscalars->GetVoidPointer(0);
        for(int p = 0; p < nels; ++p)
            *data++ = float(p);
        return fscalars;
    }
#endif

    //
    // Try and read the data from the file.
    //
    int nVarDims;
    hsize_t hyperslabDims[3];
    DetermineVarDimensions(it->second, hyperslabDims, 0, nVarDims);

    vtkDataArray *scalars = 0;
    int nels = hyperslabDims[0] * hyperslabDims[1] * hyperslabDims[2];
    if(H5Tequal(it->second.nativeVarType, H5T_NATIVE_INT) > 0 ||
       H5Tequal(it->second.nativeVarType, H5T_NATIVE_UINT) > 0)
    {
        vtkIntArray *iscalars = vtkIntArray::New();
        iscalars->SetNumberOfTuples(nels);
        scalars = iscalars;
        TRY
        {
            ReadVariableFromFile(timestate, it->first, it->second,
                hyperslabDims, iscalars->GetVoidPointer(0));
        }
        CATCH(VisItException)
        {
            iscalars->Delete();
            RETHROW;
        }
        ENDTRY
    }
    else if(H5Tequal(it->second.nativeVarType, H5T_NATIVE_FLOAT) > 0)
    {
        vtkFloatArray *fscalars = vtkFloatArray::New();
        fscalars->SetNumberOfTuples(nels);
        scalars = fscalars;
        TRY
        {
            ReadVariableFromFile(timestate, it->first, it->second,
                hyperslabDims, fscalars->GetVoidPointer(0));
        }
        CATCH(VisItException)
        {
            fscalars->Delete();
            RETHROW;
        }
        ENDTRY
    }
    else if(H5Tequal(it->second.nativeVarType, H5T_NATIVE_DOUBLE) > 0)
    {
        vtkDoubleArray *dscalars = vtkDoubleArray::New();
        dscalars->SetNumberOfTuples(nels);
        scalars = dscalars;
        TRY
        {
            ReadVariableFromFile(timestate, it->first, it->second,
                hyperslabDims, dscalars->GetVoidPointer(0));
        }
        CATCH(VisItException)
        {
            dscalars->Delete();
            RETHROW;
        }
        ENDTRY
    }
    else
    {
        debug4 << mName << "The variable " << varname << " was in a "
               <<"native format that we're not supporting." << endl;
    }

    return scalars;
}

// ****************************************************************************
// Method: avtPixieFileFormat::ReadVariableFromFile
//
// Purpose: 
//   Reads a variable from the Pixie file into a buffer.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 15 08:38:56 PDT 2004
//
// Modifications:
//   Eric Brugger, Tue Oct 26 08:36:53 PDT 2004
//   I modified the routine to read a hyperslab of the array.
//   
//   Eric Brugger, Mon Nov 29 15:52:39 PST 2004
//   Modified the reader to handle gaps in the cycle numbering (e.g. allowing
//   0, 10, 20, 30 instead of requiring 0, 1, 2, 3).
//   
//    Mark C. Miller, Thu Apr  6 17:06:33 PDT 2006
//    Added conditional compilation for hssize_t type
//
// ****************************************************************************

bool
avtPixieFileFormat::ReadVariableFromFile(int timestate, const std::string &varname,
    const VarInfo &it, const hsize_t *dims, void *dest) const
{
    bool retval = false;

    // Add the time state prefix if necessary.
    std::string fileVar(it.fileVarName);
    if(nTimeStates > 0 && it.timeVarying)
    {
        char tsPrefix[40];
        SNPRINTF(tsPrefix, 40, "%s%d/", timeStatePrefix.c_str(), cycles[timestate]);
        fileVar = std::string(tsPrefix) + fileVar;
    }

    //
    // Try and open the data.
    //
    debug4 << "avtPixieFileFormat::ReadVariableFromFile: Trying to open data: "
           << fileVar.c_str() << endl;
    hid_t dataId = H5Dopen(fileId, fileVar.c_str());
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
#if HDF5_VERSION_GE(1,6,4)
    hsize_t start[3];
#else
    hssize_t start[3];
#endif
    start[0] = 0; start[1] = 0; start[2] = 0;
    H5Sselect_hyperslab(spaceId, H5S_SELECT_SET, start, NULL, dims, NULL);

    //
    // Try and read the data from the file.
    //
    if(H5Tequal(it.nativeVarType, H5T_NATIVE_INT) > 0 ||
       H5Tequal(it.nativeVarType, H5T_NATIVE_UINT) > 0 ||
       H5Tequal(it.nativeVarType, H5T_NATIVE_FLOAT) > 0 ||
       H5Tequal(it.nativeVarType, H5T_NATIVE_DOUBLE) > 0)
    {
        // Read the data into all_vars array.
        if(H5Dread(dataId, it.nativeVarType, H5S_ALL, spaceId,
                   H5P_DEFAULT, dest) < 0)
        {
            H5Sclose(spaceId);
            H5Dclose(dataId);
            EXCEPTION1(InvalidVariableException, varname);
        }
        retval = true;
    }
    else
    {
        debug4 << "avtPixieFileFormat::ReadVariableFromFile: The variable "
               << varname.c_str() << " was in a native format that we're not "
               "supporting." << endl;
    }

    // Close the data space so we don't leak resources.
    H5Sclose(spaceId);
    H5Dclose(dataId);
    return retval;
}

// ****************************************************************************
// Method: ConvertToFloat
//
// Purpose: 
//   Converts an array to a float array.
//
// Arguments:
//   data      : The data array to be converted.
//   nels      : The number of elements in the data array.
//   allocated : Whether a new data array had to be allocated.
//
// Returns:    Pointer to the converted data.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 15 08:39:27 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

template <class T>
float *ConvertToFloat(const T *data, int nels, bool &allocated)
{
    float *f;

    if(sizeof(float) == sizeof(T))
    {
        allocated = false;
        // Change to float in the same memory.
        f = (float *)data;
        for(int i = 0; i < nels; ++i)
            f[i] = (float)data[i];        
    }
    else
    {
        allocated = true;
        f = new float[nels]; 
        for(int i = 0; i < nels; ++i)
            f[i] = (float)data[i];
    }

    return f;
}

// ****************************************************************************
// Method: avtPixieFileFormat::ReadCoordinateFields
//
// Purpose: 
//   Reads the coordinate fields from the file.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 15 08:41:58 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Sep 15 17:20:11 PST 2004
//   Added support for reading custom coordinate arrays.
//
//   Eric Brugger, Tue Oct 26 08:36:53 PDT 2004
//   I modified the routine to read a hyperslab of the array.
//
//   Brad Whitlock, Wed Apr 13 11:27:03 PDT 2005
//   I changed the calls to ConvertToFloat.
//
// ****************************************************************************

bool
avtPixieFileFormat::ReadCoordinateFields(int timestate, const VarInfo &info,
    float *coords[3], const hsize_t *dims, int nDims) const
{
    bool retval = false;

    // Make sure that all of the coordinate field variables are in the
    // variables map.
    VarInfoMap::const_iterator vars[3];
    vars[0] = variables.find(info.coordX);
    vars[1] = variables.find(info.coordY);
    if(nDims > 2)
        vars[2] = variables.find(info.coordZ);

    int i;
    for(i = 0; i < nDims; ++i)
        if(vars[i] == variables.end())
             return false;

    TRY
    {
        //
        // Read in the required variables from the file.
        //
        for(i = 0; i < nDims; ++i)
        {
            bool allocated;
            int  nels = dims[0] * dims[1] * dims[2];
            if(H5Tequal(vars[i]->second.nativeVarType, H5T_NATIVE_INT) > 0 ||
               H5Tequal(vars[i]->second.nativeVarType, H5T_NATIVE_UINT) > 0)
            {
                int *data = new int[nels];
                TRY
                {
                    ReadVariableFromFile(timestate, vars[i]->first,
                        vars[i]->second, dims, (void *)data);
                    coords[i] = ConvertToFloat(data, nels, allocated);
                    if(allocated)
                        delete [] data;
                }
                CATCH(VisItException)
                {
                    delete [] data;
                    RETHROW;
                }
                ENDTRY
            }
            else if(H5Tequal(vars[i]->second.nativeVarType, H5T_NATIVE_FLOAT) > 0)
            {
                coords[i] = new float[nels];
                ReadVariableFromFile(timestate, vars[i]->first, vars[i]->second,
                    dims, coords[i]);
            }
            else if(H5Tequal(vars[i]->second.nativeVarType, H5T_NATIVE_DOUBLE) > 0)
            {
                double *data = new double[nels];
                TRY
                {
                    ReadVariableFromFile(timestate, vars[i]->first,
                        vars[i]->second, dims, (void *)data);
                    coords[i] = ConvertToFloat(data, nels, allocated);
                    if(allocated)
                        delete [] data;
                }
                CATCH(VisItException)
                {
                    delete [] data;
                    RETHROW;
                }
                ENDTRY
            }
            else
            {
                debug1 << "The " << vars[i]->first.c_str()
                       << " variable was not a type that the Pixie reader "
                          "supports yet." << endl;
                EXCEPTION1(InvalidVariableException, vars[i]->first);
            }
        }

        retval = true;
    }
    CATCH(VisItException)
    {
        // Delete any coordinate arrays that we may have read.
        for(i = 0; i < nDims; ++i)
            delete [] coords[i];
        coords[0] = coords[1] = coords[2] = 0;
    }
    ENDTRY

    return retval;
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
//   Brad Whitlock, Wed Sep 15 22:08:04 PST 2004
//   I added code to get the "coords" attribute on data groups that should
//   have a curvilinear mesh.
//
//   Eric Brugger, Mon Nov 29 15:52:39 PST 2004
//   Modified the reader to handle gaps in the cycle numbering (e.g. allowing
//   0, 10, 20, 30 instead of requiring 0, 1, 2, 3).
//
//   Mark C. Miller, Wed May 23 15:27:53 PDT 2007
//   Initialized varInfo.dims before populating with call to get_simple_extents
//   
//   Luis Chacon, Wed Feb 25 15:40:06 EST 2009
//   Modified the reader to handle time-changing meshes.
//
//   Jeremy Meredith, Thu Jan  7 15:35:18 EST 2010
//   Skip ".." group names.
//
// ****************************************************************************

herr_t
avtPixieFileFormat::GetVariableList(hid_t group, const char *name,
    void *op_data)
{
    // Silo files have a ".." group.  Don't process that....  Ideally we
    // might detect and skip hard links, but this doesn't come up often.
    if (string(name)=="..")
        return 0;

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
            varInfo.timeVarying = false;
            varInfo.hasCoords = info->hasCoords;
            varInfo.coordX = info->coordX;
            varInfo.coordY = info->coordY;
            varInfo.coordZ = info->coordZ;
            
            // Peel off the timestep prefix if there are multiple time states.
            if(info->This->nTimeStates > 0)
            {
                bool matchingTimePrefix = false;
                const std::string tsPrefix1("/Timestep_");
                const std::string tsPrefix2("/Timestep ");
                if(varName.substr(0, tsPrefix1.size()) == tsPrefix1)
                {
                    matchingTimePrefix = true;
                    info->This->timeStatePrefix = tsPrefix1;
                }
                else if(varName.substr(0, tsPrefix2.size()) == tsPrefix2)
                {
                    matchingTimePrefix = true;
                    info->This->timeStatePrefix = tsPrefix2;
                }

                if(matchingTimePrefix)
                {
                    varInfo.timeVarying = true;

                    // Strip off the "/Timestep #" prefix from the argument.
                    std::string::size_type index = varName.find("/", 1);
                    if(index != -1)
                        varName = varName.substr(index+1);

                    // Strip the timestep off of the file variable because
                    // we'll add that back later.
                    index = varInfo.fileVarName.find("/", 1);
                    if(index != -1)
                        varInfo.fileVarName = varInfo.fileVarName.substr(index+1);
                }
                else if(varName.size() > 0 && varName[0] == '/')
                {
                    // Trim off the leading slash.
                    varName = varName.substr(1);
                }
            }
            else if(varName.size() > 0 && varName[0] == '/')
                varName = varName.substr(1);
            
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
            for (int dd = 0; dd < 3; varInfo.dims[dd] = 1, dd++);
            H5Sget_simple_extent_dims(sid, varInfo.dims, NULL);

            //
            // Determine the variable's type to see if we can support it.
            //
            hid_t t = H5Dget_type(obj);

            //
            // MCM - Added 16Mar05
            // VisIt can't deal well with a large variety of different data
            // types. So, we force everything to float with this line of
            // code. This tells the plugin that everything is float,
            // regardless of its real type on disk. Note that if we
            // every implement GetAuxiliaryData functions for global node/zone
            // ids, we'll have to be a little smarter.
            //
#ifdef FORCE_FLOATS
            varInfo.nativeVarType = H5T_NATIVE_FLOAT; 
#else
            varInfo.nativeVarType = H5Tget_native_type(t, H5T_DIR_ASCEND);
#endif

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
                 debug4 << ", which is not supported at this time." << endl;
            }

            // Store information about the variable.
            if(supported &&
               info->This->variables.find(varName) == info->This->variables.end())
            {
                info->This->variables[varName] = varInfo;
                debug4 << "Adding variable \"" << varName.c_str()
                       << "\" for file variable: \""
                       << varInfo.fileVarName.c_str() << "\"" << endl;
            } 

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

            int cycle;
            if (varName[9] == '_')
               cycle = atoi(varName.substr(10).c_str());
            else
               cycle = atoi(varName.substr(9).c_str());
            info->This->cycles.push_back(cycle);
        }

        // Indicate that we have the mesh coordinates.
        if(varName.find("nodes"))
        {
            debug4 << "Have mesh coordinates." << endl;
            info->This->haveMeshCoords = true;
        }

        if ((obj = H5Gopen(group, name)) >= 0)
        {
            TraversalInfo info2;
            info2.This = info->This;
            info2.level = info->level + 1;
            info2.path = varName;
            info2.hasCoords = false;
            info2.coordX = "";
            info2.coordY = "";
            info2.coordZ = "";

// ************************* Begin Pixie-specific coding **********************
            //
            // See if the group has the "coords" attribute.
            //
            hid_t coordsAttribute = H5Aopen_name(obj, "coords");
            if(coordsAttribute >= 0)
            {
                hid_t attrType = H5Aget_type(coordsAttribute);
                if(attrType >= 0)
                {
                    char data[1000];
                    memset((void*)data, 0, sizeof(data));
                    if(H5Aread(coordsAttribute, attrType, (void *)data) >= 0)
                    {
                        for(int j = 0; j < 3; ++j)
                        {
                            int dsize = H5Tget_size(attrType);
                            char *ptr = data + dsize * j + 1;
                            char *tmp = new char[dsize+1];
                            int i;
                            for(i = 0; i < dsize && *ptr != ' '; ++i)
                                tmp[i] = *ptr++;
                            tmp[i] = '\0';
                        
                            if(j == 0)
                                info2.coordX = std::string(tmp);
                            else if(j == 1)
                                info2.coordY = std::string(tmp);
                            else
                                info2.coordZ = std::string(tmp);
                            delete [] tmp;
                        }
                        info2.hasCoords = true;

                        debug4 << "Have mesh coordinates: " 
                               << info2.coordX  << endl;
                        debug4 << "Have mesh coordinates: " 
                               << info2.coordY  << endl;
                        debug4 << "Have mesh coordinates: " 
                               << info2.coordZ  << endl;
                    }
                    else
                    {
                        debug4 << "No mesh coordinates found." << endl;
                    }
                    H5Tclose(attrType);
                }
                H5Aclose(coordsAttribute);
            }
// ************************** End Pixie-specific coding ***********************

            // Iterate over the items in this group.           
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
