/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            avtTSurfFileFormat.C                           //
// ************************************************************************* //

#include <avtTSurfFileFormat.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <string>

#include <vtkCellType.h>
#include <vtkFloatArray.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>

#include <TimingsManager.h>
#include <DebugStream.h>
#include <Utility.h>

//
// TSurf models have a node id associated with each vertex and that nodeid
// is used in the VRTX statement that defines the vertex. Sometimes the model
// may not define certain node numbers, leading to gaps in the node number
// sequence. If we define the USE_POINT_INDICES_TO_INSERT macro then we will use
// the node id's to insert new nodes into the vtkPoints so we'll not mess up
// the connectivity specified in the TRGL cell declarations. The
// penalty is that we have to run the data through the relevant points filter
// so the gaps are removed from the points array.
//
#define USE_POINT_INDICES_TO_INSERT
#if !defined(MDSERVER) && defined(USE_POINT_INDICES_TO_INSERT)
#include <vtkUnstructuredGridRelevantPointsFilter.h>
#endif

using     std::string;

#define ALL_LINES -1

// ****************************************************************************
//  Method: avtTSurfFileFormat constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Apr 11 10:28:43 PDT 2006
//
//  Modifications:
//
// ****************************************************************************

avtTSurfFileFormat::avtTSurfFileFormat(const char *filename)
    : avtSTSDFileFormat(filename), title()
{
    // Init frame of reference.
    frameOfReferenceSet = false;
    ref.gridLat = 37.;
    ref.gridLon = -118.;
    ref.gridAZ = 135.;
    ref.zScale = 1000.;

    meshDS = 0;

#ifdef MDSERVER
    if(!ReadFile(filename, 10))
    {
        EXCEPTION1(InvalidFilesException, filename);
    }
#endif
}

// ****************************************************************************
// Method: avtTSurfFileFormat::~avtTSurfFileFormat
//
// Purpose: 
//   Destructor for avtTSurfFileFormat class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 6 15:59:07 PST 2005
//
// Modifications:
//   
// ****************************************************************************

avtTSurfFileFormat::~avtTSurfFileFormat()
{
    FreeUpResources();
}

// ****************************************************************************
//  Method: avtTSurfFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Apr 11 10:28:43 PDT 2006
//
// ****************************************************************************

void
avtTSurfFileFormat::FreeUpResources(void)
{
    debug4 << "avtTSurfFileFormat::FreeUpResources" << endl;
    if(meshDS)
    {
        meshDS->Delete();
        meshDS = 0;
    }

    title = "";
}

// ****************************************************************************
// Method: avtTSurfFileFormat::ActivateTimestep
//
// Purpose: 
//   Called when we're activating the current time step.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 6 15:59:34 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtTSurfFileFormat::ActivateTimestep()
{
    debug4 << "avtTSurfFileFormat::ActivateTimestep" << endl;
#ifndef MDSERVER
    if(meshDS == 0)
    {
        ReadFile(filename, ALL_LINES);
    }
#endif
}

// ****************************************************************************
// Method: avtTSurfFileFormat::ReadConfigFile
//
// Purpose: 
//   Reads a file called TSurf.ini from the .visit directory and uses the
//   values 
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 11 11:36:35 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
avtTSurfFileFormat::ReadConfigFile()
{
    const char *mName = "avtTSurfFileFormat::ReadConfigFile: ";
    bool retval = frameOfReferenceSet;
    
    if(!frameOfReferenceSet)
    {
        std::string configFile(GetUserVisItDirectory());
        configFile += "TSurf.ini";

        // Open the file.
        ifstream ifile(configFile.c_str());
        if (ifile.fail())
        {
            debug4 << mName << "Could not open config file: "
                   << configFile.c_str() << endl;
        }
        else
        {
            debug4 << mName << "Opened config file: "
                   << configFile.c_str() << endl;

            bool err = false;
            char line[1024];
            for(int lineIndex = 0; !ifile.eof(); ++lineIndex)
            {
                // Get the line
                ifile.getline(line, 1024);

                if(strncmp(line, "gridLon", 7) == 0)
                {
                    if(sscanf(line+7, "%f", &ref.gridLon) != 1)
                    {
                        err = true;
                        break;
                    }
                }
                else if(strncmp(line, "gridLat", 7) == 0)
                {
                    if(sscanf(line+7, "%f", &ref.gridLat) != 1)
                    {
                        err = true;
                        break;
                    }
                }
                else if(strncmp(line, "gridAZ", 6) == 0)
                {
                    if(sscanf(line+6, "%f", &ref.gridAZ) != 1)
                    {
                        err = true;
                        break;
                    }
                }
                else if(strncmp(line, "zScale", 6) == 0)
                {
                    if(sscanf(line+6, "%f", &ref.zScale) != 1)
                    {
                        err = true;
                        break;
                    }
                }
            }
      
            frameOfReferenceSet = retval = !err;
        }
    }

    debug4 << mName << "gridLon=" << ref.gridLon << endl;
    debug4 << mName << "gridLat=" << ref.gridLat << endl;
    debug4 << mName << "gridAZ=" << ref.gridAZ << endl;
    debug4 << mName << "zScale=" << ref.zScale << endl;

    return retval;
}

// ****************************************************************************
// Method: avtTSurfFileFormat::ComputeCartesianCoord
//
// Purpose: 
//   Convert long,lat coordinates into x,y coordinates.
//
// Arguments:
//   lon : Longitude
//   lat : Latitude
//   x   : The resulting x coordinate.
//   y   : The resulting y coordinate.
//
// Programmer: Kathleen McCandless
// Creation:   Tue Apr 11 11:35:13 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtTSurfFileFormat::ComputeCartesianCoord(float lon, float lat, float &x, float &y)
{
    // -----------------------------------------------------------------
    // Compute the cartesian coordinate give the geographic coodinate
    // -----------------------------------------------------------------
    float deg2rad = M_PI/180.0;
    float mMetersPerDegree = 111319.5;
    float phi = ref.gridAZ * deg2rad;

    // compute x and y
    x = mMetersPerDegree*(cos(phi)*(lat-ref.gridLat) + cos(lat*deg2rad)*(lon-ref.gridLon)*sin(phi));
    y = mMetersPerDegree*(-sin(phi)*(lat-ref.gridLat) + cos(lat*deg2rad)*(lon-ref.gridLon)*cos(phi));
}


// ****************************************************************************
// Method: avtTSurfFileFormat::ReadFile
//
// Purpose: 
//   This method reads the TSurf file and constructs a dataset that gets
//   returned later in the GetMesh method.
//
// Arguments:
//   name   : The name of the file to read.
//   nLines : The max number of lines to read from the file.
//
// Returns:    True if the file looks like a TSurf bulk data file.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 6 16:00:08 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool
avtTSurfFileFormat::ReadFile(const char *name, int nLines)
{
    int total = visitTimer->StartTimer();
    debug4 << "avtTSurfFileFormat::ReadFile" << endl;

    // Open the file.
    ifstream ifile(name);
    if (ifile.fail())
    {
        EXCEPTION1(InvalidFilesException, name);
    }

    // Read the frame of reference config file if we've not yet done so.
    ReadConfigFile();

    // Determine the file size and come up with an estimate of the
    // number of vertices and cells so we can size the points and cells.
    int readingFile = visitTimer->StartTimer();
    int nPoints = 100;
    int nCells = 100;
    if(nLines == ALL_LINES)
    {
#if 0
        // Do it this way for VisIt 1.5.3
        VisItStat_t statbuf;
        VisItStat(name, &statbuf);
        VisItOff_t fileSize = statbuf.st_size;

        // Make a guess about the number of cells and points based on
        // the size of the file.
        nPoints = fileSize / (VisItOff_t) 60;
        nCells  = fileSize / (VisItOff_t) 45;
#else
        // For now
        long fileSize = 0;
        FILE *fp = fopen(name, "rb");
        if(fp == 0)
        {
            EXCEPTION1(InvalidFilesException, name);
        }
        fseek(fp, 0, SEEK_END);
        fileSize = ftell(fp);
        debug4 << "File is " << fileSize << " bytes long." << endl;

        // Make a guess about the number of cells and points based on
        // the size of the file.
        nPoints = fileSize / 60;
        nCells  = fileSize / 45;
#endif
    }
    vtkPoints *pts = vtkPoints::New();
    pts->Allocate(nPoints);
#ifdef USE_POINT_INDICES_TO_INSERT
    pts->SetNumberOfPoints(nPoints);
#endif

    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->SetPoints(pts);
    ugrid->Allocate(nCells);
    pts->Delete();

    char  line[1024];
    float pt[3];
    int verts[8];
    bool recognized = false;
    bool titleRead = false;
    for(int lineIndex = 0; !ifile.eof(); ++lineIndex)
    {
        if(nLines != ALL_LINES && lineIndex >= nLines)
            break;

        // Get the line
        ifile.getline(line, 1024);

        if(strncmp(line, "VRTX", 4) == 0)
        {
            recognized = true;

            int psi = 1;
            if(sscanf(line, "VRTX %d %f %f %f", &psi,
                   &pt[0], &pt[1], &pt[2]) != 4)
            {
                debug4 << "Error at line: " << line << endl;
            }

#if 0
            debug4 << pt[0] << ", " << pt[1] << ", " << pt[2] << endl;
#endif
#ifdef USE_POINT_INDICES_TO_INSERT
            // Decrement the value by 1 to make it zero offset.
            --psi;

            // Transform the x,y values into cartesian coords
            float x = pt[0];
            float y = pt[1];
            ComputeCartesianCoord(x, y, pt[0], pt[1]);
            pt[2] *= ref.zScale;

            if(psi < nPoints)
                pts->SetPoint(psi, pt);
            else
            {
                int newSize = int(float(nPoints) * 1.1f);
                if(newSize < psi)
                    newSize = int(float(psi) * 1.1f);

                debug4 << "Resizing point array from " << nPoints
                       << " points to " << newSize
                       << " points because we need to insert point index "
                       << psi << endl;
                nPoints = newSize;
                pts->GetData()->Resize(nPoints);
                pts->SetNumberOfPoints(nPoints);
                pts->SetPoint(psi, pt);
            }
#else
            pts->InsertNextPoint(pt);
#endif
        }
        else if(strncmp(line, "TRGL", 4) == 0)
        {
            recognized = true;

            if(sscanf(line, "TRGL %d %d %d",
               &verts[0], &verts[1], &verts[2]) != 3)
            {
                debug4 << "Error at line: " << line << endl;
            }

            // Make them be zero offset.
            --verts[0];
            --verts[1];
            --verts[2];
#if 0
            debug4 << verts[0]
                   << ", " << verts[1]
                   << ", " << verts[2]
                   << endl;
#endif

            ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
        }
        else if(strncmp(line, "*name:", 6) == 0)
        {
            titleRead = true;
            title = std::string(line + 6);
            recognized = true;
        }
        else if(strncmp(line, "END", 3) == 0)
        {
            recognized = true;
            break;
        }
        else if(strncmp(line, "GOCAD TSurf", 11) == 0)
        {
            recognized = true;
        }
    }

    visitTimer->StopTimer(readingFile, "Interpreting TSurf file");

    if(recognized && nLines == ALL_LINES)
    {
#if !defined(MDSERVER) && defined(USE_POINT_INDICES_TO_INSERT)
        int rpfTime = visitTimer->StartTimer();
        vtkUnstructuredGridRelevantPointsFilter *rpf = 
            vtkUnstructuredGridRelevantPointsFilter::New();
        rpf->SetInput(ugrid);
        rpf->Update();

        meshDS = vtkUnstructuredGrid::New();
        meshDS->ShallowCopy(rpf->GetOutput());
        ugrid->Delete();
        rpf->Delete();
        visitTimer->StopTimer(rpfTime, "Relevant points filter");
#else
        meshDS = ugrid;
#endif
    }
    else
        ugrid->Delete();

    visitTimer->StopTimer(total, "Loading TSurf file");

    return recognized;
}

// ****************************************************************************
//  Method: avtTSurfFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Apr 11 10:28:43 PDT 2006
//
// ****************************************************************************

void
avtTSurfFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    debug4 << "avtTSurfFileFormat::PopulateDatabaseMetaData" << endl;
    md->SetDatabaseComment(title);
    int sdim = 3;
    int tdim = 2;
    AddMeshToMetaData(md, "mesh", AVT_UNSTRUCTURED_MESH, NULL, 1, 1,
                      sdim, tdim);
}


// ****************************************************************************
//  Method: avtTSurfFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Apr 11 10:28:43 PDT 2006
//
// ****************************************************************************

vtkDataSet *
avtTSurfFileFormat::GetMesh(const char *meshname)
{
    debug4 << "avtTSurfFileFormat::GetMesh" << endl;
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->ShallowCopy(meshDS);

    return ugrid;
}


// ****************************************************************************
//  Method: avtTSurfFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Apr 11 10:28:43 PDT 2006
//
// ****************************************************************************

vtkDataArray *
avtTSurfFileFormat::GetVar(const char *varname)
{
    debug4 << "avtTSurfFileFormat::GetVar" << endl;

    // Can't read variables yet.
    EXCEPTION1(InvalidVariableException, varname);
}


// ****************************************************************************
//  Method: avtTSurfFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Apr 11 10:28:43 PDT 2006
//
// ****************************************************************************

vtkDataArray *
avtTSurfFileFormat::GetVectorVar(const char *varname)
{
    debug4 << "avtTSurfFileFormat::GetVectorVar" << endl;

    // Can't read variables yet.
    EXCEPTION1(InvalidVariableException, varname);
}

