/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

// ***************************************************************************
//                              avtRectFileFormat.C                           
//
//  Purpose:  Sample database which reads multi-domain multi-timestep files
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  4, 2003
//
// ****************************************************************************


#include <avtRectFileFormat.h>

#include <vector>
#include <string>

#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkFloatArray.h>

#include <avtDatabaseMetaData.h>
#include <avtStructuredDomainBoundaries.h>
#include <avtTypes.h>
#include <avtVariableCache.h>

#include <BadIndexException.h>
#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <InvalidFilesException.h>


using std::vector;
using std::string;

static string GetDirName(const char *path);

#if defined(_WIN32)
// Define >> for strings since Windows does not seem to have it.
ifstream &operator >> (ifstream &s, string &str)
{
    char tmp[1000];
    s >> tmp;
    str = string(tmp);
    return s;
}
#endif

// ****************************************************************************
//  Constructor:  avtRectFileFormat::avtRectFileFormat
//
//  Arguments:
//    fname      the file name of the root .viz file
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  4, 2003
//
//  Modifications:
//    Mark C. Miller, Thu Feb 12 01:18:59 PST 2009
//    Removed setting of gridType here. It is set in ReadVizFile
// ****************************************************************************
avtRectFileFormat::avtRectFileFormat(const char *fname)
    : avtMTMDFileFormat(fname)
{
    dirname = GetDirName(fname);
    filename = fname;

    ifstream ifile(filename.c_str());

    if (ifile.fail())
    {
        debug1 << "Unable to open viz file " << fname << endl;
    }
    else
    {
        ReadVizFile(ifile);
    }
    ifile.close();

    cachedMeshes = new vtkDataSet*[ndomains];
    for (int d=0; d<ndomains; d++)
        cachedMeshes[d] = NULL;
}

// ****************************************************************************
//  Destructor:  avtRectFileFormat::~avtRectFileFormat
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  4, 2003
//
// ****************************************************************************
avtRectFileFormat::~avtRectFileFormat()
{
}

// ****************************************************************************
//  Method:  avtRectFileFormat::GetMesh
//
//  Purpose:
//    Returns the mesh with the given name for the given time step and
//    domain.  This function will return a cached reference to the mesh
//    for this domain, since the mesh does not change over time.
//
//  Note:  The Generic Database (which calls this function) already handles
//         caching for a single time step.
//
//  Arguments:
//    ts         the time step
//    dom        the domain number
//    mesh       the name of the mesh to read
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  7, 2003
//
// ****************************************************************************
vtkDataSet *
avtRectFileFormat::GetMesh(int ts, int dom, const char *mesh)
{
    if (cachedMeshes[dom] != NULL)
    {
        // The reference count will be decremented by the generic database,
        // because it will assume it owns it.
        cachedMeshes[dom]->Register(NULL);
        return cachedMeshes[dom];
    }

    vtkDataSet *ds = ReadMesh(ts, dom, mesh);
    cachedMeshes[dom] = ds;
    ds->Register(NULL);

    return ds;
}

// ****************************************************************************
//  Method:  avtRectFileFormat::ReadMesh
//
//  Purpose:
//    Reads the mesh with the given name for the given time step and
//    domain from the file.  NOTE: time step is ignored in this case
//    because the mesh does not change over time.
//
//  Arguments:
//    ts         the time step
//    dom        the domain number
//    mesh       the name of the mesh to read
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  7, 2003
//
//  Modifications:
//
//    Mark C. Miller, Wed Jan 16 11:29:24 PST 2008
//    Added support for rectilinear grids
//
//    Mark C. Miller, Wed Jan 16 17:25:43 PST 2008
//    Added support for multiple rect blocks
//
//    Mark C. Miller, Thu Feb 12 01:19:27 PST 2009
//    Moved code to open grid file to AFTER block that handles rectlinear grid
//    as a grid file is needed only for curvilinear grids.
//
//    Mark C. Miller, Thu Feb 12 11:09:38 PST 2009
//    Handle 1D (and 2D though it was not tested) case of grid
// ****************************************************************************
vtkDataSet *
avtRectFileFormat::ReadMesh(int ts, int dom, const char *name)
{
    // timestep (ts) is unused because the mesh is constant over time
    // should really interact with variable cache (see Exodus format)
    // so the mesh is only read once for all time steps

    if (strcmp(name, "Mesh") != 0)
    {
        EXCEPTION1(InvalidVariableException, name);
    }

    int dims[3];
    dims[0] = dxsize[dom];
    dims[1] = dysize[dom];
    dims[2] = dzsize[dom];

    if (gridType == AVT_RECTILINEAR_MESH)
    {
	int i,j;
        vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
        rgrid->SetDimensions(dims);

        //
        // Populate the coordinates.  Put in 3D points with z=0 if the mesh is 2D.
        //
        vtkFloatArray   *coords[3];
        for (i = 0 ; i < 3 ; i++)
        {
	    int origin = 0;
            switch (i) {
            case 0: origin = origins[dom].x0; break;
            case 1: origin = origins[dom].y0; break;
            case 2: origin = origins[dom].z0; break;
            }

            // Default number of components for an array is 1.
            coords[i] = vtkFloatArray::New();

            if (dims[i] == 0 || dims[i] == 1)
            {
                coords[i]->SetNumberOfTuples(1);
                coords[i]->SetComponent(0, 0, 0.);
            }
	    else
	    {
                coords[i]->SetNumberOfTuples(dims[i]);
                for (j = 0 ; j < dims[i] ; j++)
                {
                    coords[i]->SetComponent(j, 0, (float) (origin + j));
                }
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

    //
    // Open the file
    //
    char fname[256];
    sprintf(fname, "%sgrid/domain%04d", dirname.c_str(), dom);
    ifstream in(fname, ios::in);
    if (in.fail())
    {
        EXCEPTION1(InvalidFilesException, fname);
    }

    //
    // Create the VTK objects and connect them up.
    //
    vtkStructuredGrid    *sgrid   = vtkStructuredGrid::New(); 
    vtkPoints            *points  = vtkPoints::New();
    sgrid->SetPoints(points);
    points->Delete();

    //
    // Tell the grid what its dimensions are and populate the points array.
    //
    sgrid->SetDimensions(dims);
    int ndims = 0;
    if (dims[0] > 1) ndims++;
    if (dims[1] > 1) ndims++;
    if (dims[2] > 1) ndims++;

    //
    // Populate the coordinates.
    //
    points->SetNumberOfPoints(numpts[dom]);
    float *pts = (float *) points->GetVoidPointer(0);

    int i,d;
    for (d=0; d<ndims; d++)
    {
        for (i=0; i<numpts[dom]; i++)
            in >> pts[i*3 + d];
    }
    for (d=ndims; d < 3; d++)
    {
        for (i=0; i<numpts[dom]; i++)
            pts[i*3 + d] = 0.0;
    }

    in.close();

    return sgrid;
}

// ****************************************************************************
//  Method:  avtRectFileFormat::GetVar
//
//  Purpose:
//    Reads the variable with the given name for the given time step and
//    domain.  NOTE: a binary implementation should use fseek() or similar
//    to skip to the correct part of the data file.
//
//  Note:  The Generic Database (which calls this function) already handles
//         caching for a single time step.
//
//  Arguments:
//    ts         the time step
//    dom        the domain number
//    name       the name of the variable to read
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  7, 2003
//
// ****************************************************************************
vtkDataArray *
avtRectFileFormat::GetVar(int ts, int dom, const char *name)
{
    for (int i = 0; i < nvars; i++)
    {
        if (varnames[i] == name)
        {
            //
            // Open the file
            //
            char fname[256];
            sprintf(fname, "%s%s%04d/domain%04d", dirname.c_str(),
                    basename.c_str(), ts, dom);

            ifstream in(fname, ios::in);
            if (!in)
            {
                cerr << "Unable to open data file " << fname << endl;
                return NULL;
            }

            //
            // Skip to the right part of the file
            // (this is horribly inefficient as it
            //  is a text file)
            //
            char buff[4096];
            for (int j=0; j<i*numpts[dom]; j++)
                in >> buff;

            //
            // Create the array and read from the file
            //
            vtkFloatArray *scalars = vtkFloatArray::New();
            scalars->SetNumberOfTuples(numpts[dom]);
            float *ptr = (float*)scalars->GetVoidPointer(0);
            for (int k=0; k<numpts[dom]; k++)
            {
                in >> ptr[k];
            }

            in.close();
            return scalars;
        }
    }

    EXCEPTION1(InvalidVariableException, name);
}

// ****************************************************************************
//  Method:  avtRectFileFormat::GetCycles
//
//  Purpose:
//    Returns the actual cycle numbers for each time step.
//
//  Arguments:
//   cycles      the output vector of cycle numbers 
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  7, 2003
//
// ****************************************************************************
void
avtRectFileFormat::GetCycles(vector<int> &cycles)
{
    int nTimesteps = GetNTimesteps();

    cycles.resize(nTimesteps);
    for (int i = 0 ; i < nTimesteps ; i++)
    {
        cycles[i] = i;
    }
}

// ****************************************************************************
//  Method:  avtRectFileFormat::GetNTimesteps
//
//  Purpose:
//    Returns the number of timesteps
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  7, 2003
//
// ****************************************************************************
int
avtRectFileFormat::GetNTimesteps()
{
    return ntimesteps;
}

// ****************************************************************************
//  Method:  avtRectFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//    Returns meta-data about the database.
//
//  Arguments:
//    md         The meta-data structure to populate
//    timeState  The time index to use (if metadata varies with time)
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  4, 2003
//
//  Modifications:
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added timeState arg
//
//    Mark C. Miller, Thu Feb 12 11:10:21 PST 2009
//    Set dimensions according to xyz extents of mesh
// ****************************************************************************
void
avtRectFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md,
    int timeState)
{
    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "Mesh";
    mesh->meshType = AVT_CURVILINEAR_MESH;
    mesh->numBlocks = ndomains;
    mesh->blockOrigin = 0;
    int ndims = 0;
    if (xsize > 1) ndims++;
    if (ysize > 1) ndims++;
    if (zsize > 1) ndims++;
    mesh->spatialDimension = ndims;
    mesh->topologicalDimension = ndims;
    mesh->hasSpatialExtents = false;
    md->Add(mesh);
    for (int i=0; i < nvars; i++)
    {
        md->Add(new avtScalarMetaData(varnames[i], "Mesh", AVT_NODECENT));
    }

    SetUpDomainConnectivity();
}

// ****************************************************************************
//  Method:  avtRectFileFormat::SetUpDomainConnectivity
//
//  Purpose:
//    Sets up the connectivity between domains so that ghost zones
//    are automatically created.  NOTE: assumes the domains are slabs
//    in the Z direction listed in increasing order.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  7, 2003
//
//  Modifications:
//
//    Hank Childs, Wed Nov 12 16:47:46 PST 2003
//    Use curvilinear domain boundaries, since structured domain boundaries is
//    now an abstract type.
//
// ****************************************************************************
void
avtRectFileFormat::SetUpDomainConnectivity()
{
    if (ndomains < 3)
        return;

    avtCurvilinearDomainBoundaries *dbi=new avtCurvilinearDomainBoundaries();
    dbi->SetNumDomains(ndomains);

    int zstart = 1;
    int extents[6];
    int orientation[3] = {1,2,3};
    int boundary[6];

    for (int i=0; i<ndomains; i++)
    {
        extents[0] = 1;
        extents[1] = dxsize[i];
        extents[2] = 1;
        extents[3] = dysize[i];
        extents[4] = zstart;
        extents[5] = zstart + dzsize[i]-1;

        boundary[0] = 1;
        boundary[1] = dxsize[i];
        boundary[2] = 1;
        boundary[3] = dysize[i];

        dbi->SetExtents(i, extents);

        if (i > 0)
        {
            boundary[4] = zstart;
            boundary[5] = zstart;
            dbi->AddNeighbor(i, i-1, (i>1)?1:0, orientation, boundary);
        }

        if (i < ndomains-1)
        {
            boundary[4] = zstart + dzsize[i]-1;
            boundary[5] = zstart + dzsize[i]-1;
            dbi->AddNeighbor(i, i+1, 0, orientation, boundary);
        }

        dbi->Finish(i);

        zstart += dzsize[i]-1;
    }

    void_ref_ptr vr = void_ref_ptr(dbi,
                                   avtStructuredDomainBoundaries::Destruct);
    cache->CacheVoidRef("any_mesh",
                       AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION, -1, -1, vr);
}

// ****************************************************************************
//  Method:  avtRectFileFormat::ReadVizFile
//
//  Purpose:
//    Read the .viz (database meta-data) file
//
//  Arguments:
//    in         the open ifstream to read from
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  4, 2003
//
//  Modifications:
//    Mark C. Miller, Wed Jan 16 17:25:43 PST 2008
//    Added support for multiple rect blocks
//
//    Mark C. Miller, Thu Feb 12 01:20:13 PST 2009
//    Fixed parsing of size info from 'domainIJK' lines
// ****************************************************************************
void
avtRectFileFormat::ReadVizFile(ifstream &in)
{
    int i;
    string buff;

    in >> buff;
    if (buff != "name")
        EXCEPTION1(InvalidFilesException, filename.c_str());

    in >> basename;
    debug1 << "basename = " << basename << endl;

    in >> buff;
    if (buff != "domains")
        EXCEPTION1(InvalidFilesException, filename.c_str());

    in >> ndomains;
    debug1 << "ndomains = " << ndomains << endl;

    in >> buff;
    if (buff != "timesteps")
        EXCEPTION1(InvalidFilesException, filename.c_str());

    in >> ntimesteps;
    debug1 << "ntimesteps = " << ntimesteps << endl;

    in >> buff;
    if (buff != "variables")
        EXCEPTION1(InvalidFilesException, filename.c_str());

    in >> nvars;
    debug1 << "nvars = " << nvars << endl;

    for (i=0; i<nvars;i++)
    {
        in >> buff;
        varnames.push_back(buff);
        debug1 << "    varname = " << buff << endl;
    }

    in >> buff;
    if (buff != "size")
        EXCEPTION1(InvalidFilesException, filename.c_str());

    in >> xsize >> ysize >> zsize;
    debug1 << "xsize = " << xsize << endl;
    debug1 << "ysize = " << ysize << endl;
    debug1 << "zsize = " << zsize << endl;

    gridType = AVT_CURVILINEAR_MESH;
    in >> buff;
    if (buff == "gridtype")
    {
        in >> buff;
	if (buff == "rect")
	    gridType = AVT_RECTILINEAR_MESH;
        else if (buff == "curv")
	    gridType = AVT_CURVILINEAR_MESH;
    }

    for (i=0; i<ndomains; i++)
    {
        if (i == 0 && (buff == "curv" || buff == "rect") || i > 0)
            in >> buff; // for "domainIJK" token
        int x,y,z;
        in >> x >> y >> z;
        dxsize.push_back(x);
        dysize.push_back(y);
        dzsize.push_back(z);
        numpts.push_back(x*y*z);
        debug4 << "domain " << i << ": xsize = "
	       << x << ", ysize = " << y << ", zsize = " << z;
	if (gridType == AVT_RECTILINEAR_MESH)
	{
	    origin_t o;
            in >> o.x0 >> o.y0 >> o.z0;
	    origins.push_back(o);
            debug4 << " (xo = " << o.x0 << ", yo = " << o.y0 << ", zo = " << o.z0 << ")";
	}
	debug4 << endl;
    }
}

// ****************************************************************************
//  Method:  GetDirName
//
//  Purpose:
//    Returns the directory from a full path name
//
//  Arguments:
//    path       the full path name
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  7, 2003
//
// ****************************************************************************
string 
GetDirName(const char *path)
{
    string dir = "";

    int len = strlen(path);
    const char *last = path + (len-1);
    while (*last != '/' && last > path)
    {
        last--;
    }

    if (*last != '/')
    {
        return "";
    }

    char str[1024];
    strcpy(str, path);
    str[last-path+1] = '\0';

    return str;
}
