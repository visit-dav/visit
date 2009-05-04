/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

// ************************************************************************* //
//                            avtS3DFileFormat.C                           //
// ************************************************************************* //

#include <avtS3DFileFormat.h>

#include <string>
#include <visit-config.h>

#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkPointData.h>

#include <avtDatabaseMetaData.h>
#include <avtGhostData.h>

#include <Expression.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <netcdfcpp.h>
#define INVALID_FILE_HANDLE -1

#include <DebugStream.h>

#include <snprintf.h>
#define S3D_PLUGIN_VERSION 1.1

using     std::string;
using     std::vector;

#ifdef WIN32
#include <stdio.h>
#define F_SLASH_STRING "/"
#define B_SLASH_STRING "\\"
#define DEV_CHAR ":"
#else
#include <libgen.h>
#endif

// ****************************************************************************
// parse_dirname
//
//  Purpose: to parse a directory name from the given path.  
//           Added because windows does not have 'libgen.h' (dirname).
//
//
// ****************************************************************************
string
parse_dirname(char *wholePath)
{
#ifndef WIN32
    return string(dirname(wholePath));
#else
    string wholePathString(wholePath);
    int len = wholePathString.size();
    if (wholePathString[len-1] == DEV_CHAR[0])
        return wholePathString;
    int pos = len;
    string es(".");
    string searchFor = (string)F_SLASH_STRING + (string)B_SLASH_STRING + (string)DEV_CHAR;
    while (pos == len)
    {
        pos = wholePathString.find_last_of(searchFor, --len);
    }
    if (pos == -1)
        return es;
    else if (pos == 0)
        pos++;
    if (wholePathString[pos] == DEV_CHAR[0])
    {
       pos++;
    }
    string rets = wholePathString.substr(0, pos);
    return rets;
#endif
}

// ****************************************************************************
// CreateStringFromDouble
//
// Purpose: 
//   snprintf on Windows uses 3 digits for exponents, but unix uses 2, so
//   this method uses snprintf to create the string, but then removes the 
//   leading 0 in the exponent part if on a windows system.  When we start 
//   using MSVC8, can switch to _set_output_format(_TWO_DIGIT_EXPONENT)
//   before the call to SNPRINTF instead of using find / replace.
//
//  Creation:  June 5, 2007
//  Programmer:  Kathleen Bonnell
//
// ****************************************************************************

string
CreateStringFromDouble(double ts)
{
    char temp[256];
    SNPRINTF(temp,256,"%1.3E",ts);
    string tempStr(temp);
#ifdef WIN32
    int off = tempStr.find("E+0");
    if (off < tempStr.size())
    {
        tempStr.replace(off, 3, "E+");
    }
    else
    {
        off = tempStr.find("E-0");
        if (off < tempStr.size())
            tempStr.replace(off, 3, "E-");
    }
#endif
    return tempStr;
}


// ****************************************************************************
//  Method: avtS3D constructor
//
//  Programmer: ahern -- generated by xml2avt
//  Creation:   Thu Jan 11 10:02:22 PDT 2007
//
// ****************************************************************************

avtS3DFileFormat::avtS3DFileFormat(const char *filename)
    : avtMTMDFileFormat(filename)
{
    mainFilename = strdup(filename);

    opened = false;
}

// ****************************************************************************
//  Method: avtS3DFileFormat::OpenLogFile
//
//  Programmer: ahern
//  Creation:   Thu Jan 11 10:02:22 PDT 2007
//
//  Modifications:
//    Jeremy Meredith, Thu Aug  7 16:13:20 EDT 2008
//    Assume code reading "line=line++" was supposed to just be "line++".
//
// ****************************************************************************
void
avtS3DFileFormat::OpenLogFile(void)
{
    if (opened)
        return;

    debug4 << "avtS3DFileFormat::OpenLogFile" << endl;

    // Read the metadata

    ifstream ifile(mainFilename);
    if (ifile.fail())
    {
        EXCEPTION1(InvalidFilesException, mainFilename);
    }

    // Find the dimensions of the mesh and the layout of domains.
    char buf[128];
    while (!ifile.eof())
    {
        char *line = buf;
        buf[0] = '\0';
        ifile.getline(line, 128);
        if (strncmp(line, "NX", 2) == 0)
            sscanf(line, "NX: %d", &globalDims[0]);
        else if (strncmp(line, "NY", 2) == 0)
            sscanf(line, "NY: %d", &globalDims[1]);
        else if (strncmp(line, "NZ", 2) == 0)
            sscanf(line, "NZ: %d", &globalDims[2]);
        else if (strncmp(line, "PX", 2) == 0)
            sscanf(line, "PX: %d", &procs[0]);
        else if (strncmp(line, "PY", 2) == 0)
            sscanf(line, "PY: %d", &procs[1]);
        else if (strncmp(line, "PZ", 2) == 0)
            sscanf(line, "PZ: %d", &procs[2]);
    }
    debug4 << "avtS3DFileFormat::OpenLogFile: dimensions [" << globalDims[0] << ", " << globalDims[1] << ", " << globalDims[2] << "]" << endl;
    debug4 << "avtS3DFileFormat::OpenLogFile: procs [" << procs[0] << ", " << procs[1] << ", " << procs[2] << "]" << endl;

    // Find the names/count of the timesteps.
    // 
    // Cut the ".s3d" from the filename and append ".savefile.log"
    char *copy = strdup(mainFilename);
    int len = strlen(copy);
    char *logFilename = new char[len+32];
    copy[len-4] = '\0';
    SNPRINTF(logFilename, len+32, "%s.savefile.log", copy);

    debug4 << "avtS3DFileFormat::OpenLogFile: logFilename " << logFilename << endl;
    ifstream timefile(logFilename);
    delete[] logFilename;
    if (timefile.fail())
    {
        debug1 << "avtS3DFileFormat::OpenLogFile: Unable to open log file " << logFilename << endl;
        EXCEPTION1(InvalidFilesException, logFilename);
    }
    timefile.ignore(1000,'\n');    // Ignore the first two lines
    timefile.ignore(1000,'\n');    // Ignore the first two lines
    fileNCycles = 0;
    while(!timefile.eof())
    {
        char *line = buf;
        buf[0] = '\0';
        timefile.getline(line, 128);
        if (line[0] == '!')
        {
            line++;
        }

        float time;
        int conv = sscanf(line,"%E",&time);
        if (conv == 1)
        {
            fileTimes.push_back(time);
            fileNCycles++;
        }
    }
    debug4 << "avtS3DFileFormat::OpenLogFile: ncycles " << fileNCycles << endl;

    opened = true;
}

// ****************************************************************************
//  Method: avtS3D destructor
//
//  Programmer: ahern
//  Creation:   Thu Jan 11 10:02:22 PDT 2007
//
// ****************************************************************************

avtS3DFileFormat::~avtS3DFileFormat()
{
}


// ****************************************************************************
//  Method: avtS3DFileFormat::GetTimes
//
//  Programmer: Sean Ahern
//  Creation:   Fri Feb 16 13:29:23 EST 2007
//
// ****************************************************************************

void
avtS3DFileFormat::GetTimes(std::vector<double> &times)
{
    times = fileTimes;
}

// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: ahern -- generated by xml2avt
//  Creation:   Thu Jan 11 10:02:22 PDT 2007
//
// ****************************************************************************

int
avtS3DFileFormat::GetNTimesteps(void)
{
    OpenLogFile();
    return fileNCycles;
}


// ****************************************************************************
//  Method: avtS3DFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: ahern -- generated by xml2avt
//  Creation:   Thu Jan 11 10:02:22 PDT 2007
//
// ****************************************************************************

void
avtS3DFileFormat::FreeUpResources(void)
{
}


// ****************************************************************************
//  Method: avtS3DFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: ahern -- generated by xml2avt
//  Creation:   Thu Jan 11 10:02:22 PDT 2007
//
//  Modifications:
//    Kathleen Bonnell, Tue Jun  5 13:53:29 PDT 2007
//    To get around WIN32 issues, call parse_dirname instead of dirname, and
//    CreateStringFromDouble instead of SNPRINTF.  
//
// ****************************************************************************

void
avtS3DFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md,
                                           int timeState)
{
    debug5 << "avtS3DFileFormat::PopulateDatabaseMetaData" << endl;
    // Get the metadata from the log file first.
    OpenLogFile();

    // Mesh
    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "mesh";
    mesh->meshType = AVT_RECTILINEAR_MESH;
    mesh->numBlocks = procs[0] * procs[1] * procs[2];
    mesh->blockOrigin = 1;
    mesh->cellOrigin = 0;
    mesh->spatialDimension = 3;
    mesh->topologicalDimension = 3;
    mesh->blockTitle = "blocks";
    mesh->blockPieceName = "block";
    mesh->hasSpatialExtents = false;
    mesh->xUnits = "mm";
    mesh->yUnits = "mm";
    mesh->zUnits = "mm";
    md->Add(mesh);

    //
    // Look in the NetCDF file for the first block for the list of variables.
    //

    // Calculate the timestep directory that the data lives in.
    char *pathcopy = strdup(mainFilename);
    string dir = parse_dirname(pathcopy);
    string timestepDir = CreateStringFromDouble(fileTimes[timeState]);

    char path[256];
    SNPRINTF(path,256,"%s%s%s%sfield.00000",dir.c_str(),VISIT_SLASH_STRING, timestepDir.c_str(), VISIT_SLASH_STRING);

    NcError err(NcError::verbose_nonfatal);
 
    NcFile nf(path);
    if (!nf.is_valid())
    {
        EXCEPTION1(InvalidFilesException, path);
    }
    debug5 << "avtS3DFileFormat::PopulateDatabaseMetaData: Got valid file" << endl;

    int nvars = nf.num_vars();
    debug5 << "avtS3DFileFormat::PopulateDatabaseMetaData: Found " << nvars << " variables" << endl;
    for (int i=0 ; i<nvars; i++)
    {
        NcVar *v = nf.get_var(i);
        if (!v)
            continue;
        debug4 << "Found variable " << v->name() << endl;

        // Check dimensionality
        int nvals = v->num_vals();
        if (nvals != 1) // Single scalars are useless.
        {
            avtScalarMetaData *scalar = new avtScalarMetaData();
            scalar->name = v->name();
            scalar->meshName = "mesh";
            scalar->centering = AVT_NODECENT;
            scalar->hasDataExtents = false;
            scalar->treatAsASCII = false;

            NcAtt *units = v->get_att("units");
            if (units)
            {
                long nv = units->num_vals();
                if (nv == 0)
                {
                    scalar->hasUnits = false;
                } else {
                    char *unitString = units->as_string(0);
                    scalar->units = unitString;
                    scalar->hasUnits = true;
                }
            } else
                scalar->hasUnits = false;

            md->Add(scalar);
        } else {
            debug4 << "Unable to process variable " << v->name() <<
                      " since it is a single scalar" << endl;

        }
    }

#if 0
    // Expressions
    Expression tempGradient_expr;
    tempGradient_expr.SetName("Temperature_gradient");
    tempGradient_expr.SetDefinition("gradient(Temperature)");
    tempGradient_expr.SetType(Expression::VectorMeshVar);
    tempGradient_expr.SetHidden(true);
    md->AddExpression(&tempGradient_expr);

    Expression tempUnit_expr;
    tempUnit_expr.SetName("Temperature_grad_unit");
    //tempUnit_expr.SetDefinition("(Temperature_gradient + {1e-6,0,0})/(magnitude(Temperature_gradient) + 1e-6)");
    //tempUnit_expr.SetDefinition("Temperature_gradient/(magnitude(Temperature_gradient) + 1e-6)");
    tempUnit_expr.SetDefinition("normalize(Temperature_gradient)");
    tempUnit_expr.SetType(Expression::VectorMeshVar);
    tempUnit_expr.SetHidden(true);
    md->AddExpression(&tempUnit_expr);

    Expression tempCurv_expr;
    tempCurv_expr.SetName("Temperature_curvature");
    tempCurv_expr.SetDefinition("divergence(Temperature_grad_unit)");
    tempCurv_expr.SetType(Expression::ScalarMeshVar);
    tempUnit_expr.SetHidden(false);
    md->AddExpression(&tempCurv_expr);
#endif
}


// ****************************************************************************
//  Method: avtS3DFileFormat::CalculateSubpiece
//
//  Purpose:
//      Determines the position in the grid of a given domain.  Calculates the
//      grid dimensions of that domain.
//
//  Arguments:
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//
//  Programmer: ahern -- generated by xml2avt
//  Creation:   Tue Feb  6 14:49:04 EST 2007
//
// ****************************************************************************
void
avtS3DFileFormat::CalculateSubpiece(int domain)
{
    // Calculate where on the processor grid this domain lies.
    int d = domain;
    pos[2] = d / (procs[0]*procs[1]);
    d = d - pos[2]*procs[0]*procs[1];
    pos[1] = d / procs[0];
    d = d - pos[1]*procs[0];
    pos[0] = d;

    debug4 << "Domain position " << pos[0] << ", " << pos[1] << ", " << pos[2] << endl;

    // Pull out the piece
    for (int dim=0;dim<2;dim++)  // Only x and y
    {
        localDims[dim] = offsetBy[dim] = globalDims[dim]/procs[dim];
        if (pos[dim] < procs[dim]-1)
            localDims[dim]++;
    }
    // Special case z
    localDims[2] = (globalDims[2]-1)/procs[2] + 1;
    offsetBy[2]  = (globalDims[2]-1)/procs[2];
}


// ****************************************************************************
//  Method: avtS3DFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      timeState   The index of the timeState.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: ahern -- generated by xml2avt
//  Creation:   Thu Jan 11 10:02:22 PDT 2007
//
//  Modifications:
//    Kathleen Bonnell, Tue Jun  5 13:53:29 PDT 2007
//    To get around WIN32 issues, call parse_dirname instead of dirname, and
//    CreateStringFromDouble instead of SNPRINTF.  
//
// ****************************************************************************

vtkDataSet *
avtS3DFileFormat::GetMesh(int timeState, int domain, const char *meshname)
{
    debug4 << "avtS3DFileFormat::GetMesh( timeState =" << timeState << ", domain="
        << domain << ", meshname=" << meshname << ")" << endl;

    //
    // Read the coordinates arrays
    //
    char *pathcopy = strdup(mainFilename);
    string dir = parse_dirname(pathcopy);
    string gridFilename = dir + "/grid.out";
    ifstream gridFile(gridFilename.c_str());
    if (gridFile.fail())
    {
        EXCEPTION1(InvalidFilesException, gridFilename.c_str());
    }

    vtkDoubleArray *globalCoords[3];
    char buf[128];

    for (int dim=0;dim<3;dim++)
    {
        globalCoords[dim] = vtkDoubleArray::New();
        globalCoords[dim]->SetNumberOfTuples(globalDims[dim]);
        double *axis = (double*)globalCoords[dim]->GetVoidPointer(0);

        gridFile.ignore(1000,'\n');    // Ignore the first line
        for (int i=0;i<globalDims[dim];i++)
        {
            char *line = buf;
            buf[0] = '\0';
            gridFile.getline(line, 128);
            sscanf(line,"%lE",&axis[i]);
        }
    }
    debug4 << "Read in " << globalDims[0] << ", " << globalDims[1] << ", " << globalDims[2] << endl;

    //
    // Take out the subpiece for this domain.
    //
    CalculateSubpiece(domain);

    int nx = localDims[0];
    int ny = localDims[1];
    int nz = localDims[2];

    debug4 << "offsetBy " << offsetBy[0] << ", " << offsetBy[1] << ", " << offsetBy[2] << endl;
    debug4 << "Local dims " << nx << ", " << ny << ", " << nz << endl;

    vtkDoubleArray *coords[3];
    for (int dim=0;dim<3;dim++)
    {
        coords[dim] = vtkDoubleArray::New();
        coords[dim]->SetNumberOfTuples(localDims[dim]);
        double *axis = (double*)coords[dim]->GetVoidPointer(0);
        long offset = offsetBy[dim]*pos[dim];
        debug4 << "Offset for dim " << dim << " is " << offset << endl;
        char *ptr = (char*)(globalCoords[dim]->GetVoidPointer(0));
        memcpy(axis, ptr+sizeof(double)*offset, localDims[dim]*sizeof(double));
    }
    globalCoords[0]->Delete();
    globalCoords[1]->Delete();
    globalCoords[2]->Delete();
    for (int dim=0;dim<3;dim++)
    {
        double *p = (double*)(coords[dim]->GetVoidPointer(0));
        debug5 << "Dimension " << dim << ": first is " << p[0] <<
                  "   last is " << p[localDims[dim]-1] << endl;
    }

    // Create the vtkRectilinearGrid object.
    vtkRectilinearGrid *grid = vtkRectilinearGrid::New();
    grid->SetDimensions(localDims);
    grid->SetXCoordinates(coords[0]);
    grid->SetYCoordinates(coords[1]);
    grid->SetZCoordinates(coords[2]);
    coords[0]->Delete();
    coords[1]->Delete();
    coords[2]->Delete();

    // Do the ghost nodes
    vtkUnsignedCharArray *ghostNodes = vtkUnsignedCharArray::New();
    ghostNodes->SetName("avtGhostNodes");
    int nvals = nx * ny * nz;
    ghostNodes->SetNumberOfTuples(nvals);
    unsigned char *gnp = ghostNodes->GetPointer(0);
    for (int i=0;i<nvals;i++)
        gnp[i] = 0;
    // Check planes cutting x
    if (pos[0] > 0)
    {
        int x = 0;
        for (int y=0;y<ny;y++)
            for (int z=0;z<nz;z++)
                avtGhostData::AddGhostNodeType(gnp[z*nx*ny + y*nx + x],
                                               DUPLICATED_NODE);
    }
    if (pos[0] < procs[0]-1)
    {
        int x = localDims[0]-1;
        for (int y=0;y<ny;y++)
            for (int z=0;z<nz;z++)
                avtGhostData::AddGhostNodeType(gnp[z*nx*ny + y*nx + x],
                                               DUPLICATED_NODE);
    }
    // Check planes cutting y
    if (pos[1] > 0)
    {
        int y = 0;
        for (int x=0;x<nx;x++)
            for (int z=0;z<nz;z++)
                avtGhostData::AddGhostNodeType(gnp[z*nx*ny + y*nx + x],
                                               DUPLICATED_NODE);
    }
    if (pos[1] < procs[1]-1)
    {
        int y = localDims[1]-1;
        for (int x=0;x<nx;x++)
            for (int z=0;z<nz;z++)
                avtGhostData::AddGhostNodeType(gnp[z*nx*ny + y*nx + x],
                                               DUPLICATED_NODE);
    }
    // Check planes cutting z
    if (pos[2] > 0)
    {
        int z = 0;
        for (int x=0;x<nx;x++)
            for (int y=0;y<ny;y++)
                avtGhostData::AddGhostNodeType(gnp[z*nx*ny + y*nx + x],
                                               DUPLICATED_NODE);
    }
    if (pos[2] < procs[2]-1)
    {
        int z = localDims[2]-1;
        for (int x=0;x<nx;x++)
            for (int y=0;y<ny;y++)
                avtGhostData::AddGhostNodeType(gnp[z*nx*ny + y*nx + x],
                                               DUPLICATED_NODE);
    }
    grid->GetPointData()->AddArray(ghostNodes);
    ghostNodes->Delete();

    return grid;
}


// ****************************************************************************
//  Method: avtS3DFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timeState  The index of the timeState.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: ahern -- generated by xml2avt
//  Creation:   Thu Jan 11 10:02:22 PDT 2007
//
//  Modifications:
//    Kathleen Bonnell, Tue Jun  5 13:53:29 PDT 2007
//    To get around WIN32 issues, call parse_dirname instead of dirname, and
//    CreateStringFromDouble instead of SNPRINTF.  
//
//    Kathleen Bonnell, Wed Jul 2 14:44:11 PDT 2008
//    Removed unreferenced variables.
//
// ****************************************************************************

vtkDataArray *
avtS3DFileFormat::GetVar(int timeState, int domain, const char *varname)
{
    debug5 << "avtS3DFileFormat::GetVar( timeState=" << timeState << ", domain="
        << domain << ", varname=" << varname << ")" << endl;

    // Calculate the timestep directory that the data lives in.
    char *pathcopy = strdup(mainFilename);
    string dir = parse_dirname(pathcopy);
    string timestepDir = CreateStringFromDouble(fileTimes[timeState]);
    debug4 << "Timestep directory is <" << timestepDir <<  ">" << endl;
    
    // Figure out how big this piece is.
    CalculateSubpiece(domain);

    // Open up the NetCDF file.
    char path[256];
    SNPRINTF(path,256,"%s%s%s%sfield.%05d",dir.c_str(),VISIT_SLASH_STRING, timestepDir.c_str(), VISIT_SLASH_STRING, domain);
    debug5 << "avtS3DFileFormat::GetVar: Full path to data file is " << path << endl;

    NcFile nf(path);
    if (!nf.is_valid())
    {
        debug1 << nc_strerror(NcError().get_err()) << endl;
        EXCEPTION1(InvalidFilesException, path);
    }
    debug5 << "avtS3DFileFormat::GetVar: Got valid file." << endl;

    // Pull out the appropriate variable.
    NcVar *v = nf.get_var(varname);
    if (!v)
    {
        debug1 << nc_strerror(NcError().get_err()) << endl;
        EXCEPTION1(InvalidVariableException, varname);
    }

    // Check if it fits the size of the mesh.  Always node-centered, remember.
    int ntuples = localDims[0] * localDims[1] * localDims[2];
    debug5 << "ntuples:" << ntuples << endl;
    int nvals = v->num_vals();
    if (ntuples != nvals)
    {
        debug1 << "The variable " << v->name() <<
                  " does not conform to its mesh (" << nvals << " != " << 
                  ntuples << ")" << endl;
        EXCEPTION1(InvalidVariableException, v->name());
    }

    // Set up the VTK dataset.
    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfTuples(ntuples);
    float *p = (float*)rv->GetVoidPointer(0);
    NcValues *input = v->values();
    if (!input)
    {
        debug1 << nc_strerror(NcError().get_err()) << endl;
        EXCEPTION1(InvalidVariableException, v->name());
    }

    // Get the scaling factor.
    NcAtt *scaling = v->get_att("scale_factor");
    float scaling_factor = 1;
    if (scaling)
    {
        scaling_factor = scaling->as_float(0);
        debug5 << "avtS3DFileFormat::GetVar: Set the scaling factor as " << scaling_factor << endl;
    }

    // Process the variable into the returned data.
    float *base = (float*)input->base();
    for(int i=0;i<ntuples;i++)
    {
        p[i] = *(base + i) * scaling_factor;
    }

    return rv;
}


// ****************************************************************************
//  Method: avtS3DFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timeState.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: ahern -- generated by xml2avt
//  Creation:   Thu Jan 11 10:02:22 PDT 2007
//
// ****************************************************************************

vtkDataArray *
avtS3DFileFormat::GetVectorVar(int timeState, int domain,const char *varname)
{
    debug5 << "avtS3DFileFormat::GetVectorVar( timeState=" << timeState << ", domain="
        << domain << ", varname=" << varname << ")" << endl;
    //YOU MUST IMPLEMENT THIS
    //
    // If you have a file format where variables don't apply (for example a
    // strictly polygonal format like the STL (Stereo Lithography) format,
    // then uncomment the code below.
    //
    // EXCEPTION1(InvalidVariableException, varname);
    //

    //
    // If you do have a vector variable, here is some code that may be helpful.
    //
    // int ncomps = YYY;  // This is the rank of the vector - typically 2 or 3.
    // int ntuples = XXX; // this is the number of entries in the variable.
    // vtkFloatArray *rv = vtkFloatArray::New();
    // int ucomps = (ncomps == 2 ? 3 : ncomps);
    // rv->SetNumberOfComponents(ucomps);
    // rv->SetNumberOfTuples(ntuples);
    // float *one_entry = new float[ucomps];
    // for (int i = 0 ; i < ntuples ; i++)
    // {
    //      int j;
    //      for (j = 0 ; j < ncomps ; j++)
    //           one_entry[j] = ...
    //      for (j = ncomps ; j < ucomps ; j++)
    //           one_entry[j] = 0.;
    //      rv->SetTuple(i, one_entry); 
    // }
    //
    // delete [] one_entry;
    // return rv;
    //
    return NULL;
}
