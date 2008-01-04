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

// ************************************************************************* //
//                            avtNASTRANFileFormat.C                         //
// ************************************************************************* //

#include <avtNASTRANFileFormat.h>

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
// NASTRAN models have a node id associated with each vertex and that nodeid
// is used in the GRID statement that defines the vertex. Sometimes the model
// may not define certain node numbers, leading to gaps in the node number
// sequence. If we define the USE_POINT_INDICES_TO_INSERT macro then we will use
// the node id's to insert new nodes into the vtkPoints so we'll not mess up
// the connectivity specified in the CHEXA, CQUAD,... cell declarations. The
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
//  Method: avtNASTRANFileFormat constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jul 5 17:24:35 PST 2005
//
//  Modifications:
//
// ****************************************************************************

avtNASTRANFileFormat::avtNASTRANFileFormat(const char *filename)
    : avtSTSDFileFormat(filename), title()
{
    meshDS = 0;

#ifdef MDSERVER
    if(!ReadFile(filename, 100))
    {
        EXCEPTION1(InvalidFilesException, filename);
    }
#endif
}

// ****************************************************************************
// Method: avtNASTRANFileFormat::~avtNASTRANFileFormat
//
// Purpose: 
//   Destructor for avtNASTRANFileFormat class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 6 15:59:07 PST 2005
//
// Modifications:
//   
// ****************************************************************************

avtNASTRANFileFormat::~avtNASTRANFileFormat()
{
    FreeUpResources();
}

// ****************************************************************************
//  Method: avtNASTRANFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jul 5 17:24:35 PST 2005
//
// ****************************************************************************

void
avtNASTRANFileFormat::FreeUpResources(void)
{
    debug4 << "avtNASTRANFileFormat::FreeUpResources" << endl;
    if(meshDS)
    {
        meshDS->Delete();
        meshDS = 0;
    }

    title = "";
}

// ****************************************************************************
// Method: avtNASTRANFileFormat::ActivateTimestep
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
avtNASTRANFileFormat::ActivateTimestep()
{
    debug4 << "avtNASTRANFileFormat::ActivateTimestep" << endl;
#ifndef MDSERVER
    if(meshDS == 0)
    {
        ReadFile(filename, ALL_LINES);
    }
#endif
}

// ****************************************************************************
// Method: avtNASTRANFileFormat::ReadFile
//
// Purpose: 
//   This method reads the NASTRAN file and constructs a dataset that gets
//   returned later in the GetMesh method.
//
// Arguments:
//   name   : The name of the file to read.
//   nLines : The max number of lines to read from the file.
//
// Returns:    True if the file looks like a NASTRAN bulk data file.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 6 16:00:08 PST 2005
//
// Modifications:
//   
//    Mark C. Miller, Thu Mar 30 16:45:35 PST 2006
//    Made it use VisItStat instead of stat
//
// ****************************************************************************

bool
avtNASTRANFileFormat::ReadFile(const char *name, int nLines)
{
    int total = visitTimer->StartTimer();
    debug4 << "avtNASTRANFileFormat::ReadFile" << endl;

    // Open the file.
    ifstream ifile(name);
    if (ifile.fail())
    {
        EXCEPTION1(InvalidFilesException, name);
    }

    // Determine the file size and come up with an estimate of the
    // number of vertices and cells so we can size the points and cells.
    int readingFile = visitTimer->StartTimer();
    int nPoints = 100;
    int nCells = 100;
    if(nLines == ALL_LINES)
    {
        VisItStat_t statbuf;
        VisItStat(name, &statbuf);
        VisItOff_t fileSize = statbuf.st_size;

        // Make a guess about the number of cells and points based on
        // the size of the file.
        nPoints = fileSize / (VisItOff_t) 130;
        nCells  = fileSize / (VisItOff_t) 150;
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

        // Determine what the line is for.
        if(line[0] == '$' && !recognized)
        {
            if(!titleRead)
                title += line;
        }
        else if(strncmp(line, "GRID*", 5) == 0)
        {
            recognized = true;

            // These GRID* lines need a second line of data. Read it into
            // the same buffer at the end.
            ifile.getline(line + 72, 1024-72);

#define LONG_FIELD_WIDTH 16
            char *valstart = line + 81;
            pt[2] = atof(valstart);

            valstart = line + 72 - LONG_FIELD_WIDTH+1;
            char *valend = line + 72;
            *valend = '\0';
            pt[1] = atof(valstart);

            valstart -= LONG_FIELD_WIDTH;
            valend -= LONG_FIELD_WIDTH;
            *valend = '\0';
            pt[0] = atof(valstart);

#ifdef USE_POINT_INDICES_TO_INSERT
            valstart -= (2 * LONG_FIELD_WIDTH);
            valend -= (2 * LONG_FIELD_WIDTH);
            *valend = '\0';
            int psi = atoi(valstart)-1;

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
        else if(strncmp(line, "GRID", 4) == 0)
        {
            recognized = true;

#define SHORT_FIELD_WIDTH 8
            char *valstart = line + 48 - SHORT_FIELD_WIDTH;
            char *valend = line + 48;

            *valend = '\0';
            pt[2] = atof(valstart);

            valstart -= SHORT_FIELD_WIDTH;
            valend -= SHORT_FIELD_WIDTH;
            *valend = '\0';
            pt[1] = atof(valstart);

            valstart -= SHORT_FIELD_WIDTH;
            valend -= SHORT_FIELD_WIDTH;
            *valend = '\0';
            pt[0] = atof(valstart);

#if 0
            debug4 << pt[0] << ", " << pt[1] << ", " << pt[2] << endl;
#endif
#ifdef USE_POINT_INDICES_TO_INSERT
            valstart -= (2 * SHORT_FIELD_WIDTH);
            valend -= (2 * SHORT_FIELD_WIDTH);
            *valend = '\0';
            int psi = atoi(valstart)-1;

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
        else if(strncmp(line, "CHEXA", 5) == 0)
        {
#define INDEX_FIELD_WIDTH 8
            // CHEXA requires more point indices so read another line.
            ifile.getline(line + 72, 1024-72);

            char *valstart = line + 88;
            char *valend = valstart;
            verts[7] = atoi(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[6] = atoi(valstart)-1;

            // Skip the blank
            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[5] = atoi(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[4] = atoi(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[3] = atoi(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[2] = atoi(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[1] = atoi(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[0] = atoi(valstart)-1;

            ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, verts);

#if 0
            debug4 << verts[0]
                   << ", " << verts[1]
                   << ", " << verts[2]
                   << ", " << verts[3]
                   << ", " << verts[4]
                   << ", " << verts[5]
                   << ", " << verts[6]
                   << ", " << verts[7]
                   << endl;
#endif
        }
        else if(strncmp(line, "CTETRA", 6) == 0)
        {
            char *valstart = line + 48;
            char *valend = valstart;
            verts[3] = atoi(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[2] = atoi(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[1] = atoi(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[0] = atoi(valstart)-1;

#if 0
            debug4 << verts[0]
                   << ", " << verts[1]
                   << ", " << verts[2]
                   << ", " << verts[3]
                   << endl;
#endif

            ugrid->InsertNextCell(VTK_TETRA, 4, verts);
        }
        else if(strncmp(line, "CPYRAM", 6) == 0)
        {
            char *valstart = line + 56;
            char *valend = valstart;
            verts[4] = atoi(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[3] = atoi(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[2] = atoi(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[1] = atoi(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[0] = atoi(valstart)-1;

#if 0
            debug4 << verts[0]
                   << ", " << verts[1]
                   << ", " << verts[2]
                   << ", " << verts[3]
                   << ", " << verts[4]
                   << endl;
#endif

            ugrid->InsertNextCell(VTK_PYRAMID, 5, verts);
        }
        else if(strncmp(line, "CPENTA", 6) == 0)
        {
            char *valstart = line + 64;
            char *valend = valstart;
            verts[5] = atoi(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[4] = atoi(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[3] = atoi(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[2] = atoi(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[1] = atoi(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[0] = atoi(valstart)-1;
#if 0
            debug4 << verts[0]
                   << ", " << verts[1]
                   << ", " << verts[2]
                   << ", " << verts[3]
                   << ", " << verts[4]
                   << ", " << verts[5]
                   << endl;
#endif
            ugrid->InsertNextCell(VTK_WEDGE, 6, verts);
        }
        else if(strncmp(line, "CQUAD4", 6) == 0)
        {
            char *valstart = line + 48;
            char *valend = valstart;
            verts[3] = atoi(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[2] = atoi(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[1] = atoi(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[0] = atoi(valstart)-1;

#if 0
            debug4 << verts[0]
                   << ", " << verts[1]
                   << ", " << verts[2]
                   << ", " << verts[3]
                   << endl;
#endif

            ugrid->InsertNextCell(VTK_QUAD, 4, verts);
        }
        else if(strncmp(line, "CTRIA2", 6) == 0 ||
                strncmp(line, "CTRIA3", 6) == 0)
        {
            char *valstart = line + 40;
            char *valend = valstart;
            verts[2] = atoi(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[1] = atoi(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[0] = atoi(valstart)-1;
#if 0
            debug4 << verts[0]
                   << ", " << verts[1]
                   << ", " << verts[2]
                   << endl;
#endif

            ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
        }
        else if(strncmp(line, "CBAR", 4) == 0)
        {
            char *valstart = line + 32;
            char *valend = valstart;
            verts[1] = atoi(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[0] = atoi(valstart)-1;

#if 0
            debug4 << verts[0]
                   << ", " << verts[1]
                   << endl;
#endif

            ugrid->InsertNextCell(VTK_LINE, 2, verts);
        }
        else if(strncmp(line, "TITLE", 5) == 0)
        {
            titleRead = true;
            char *ptr = 0;
            if((ptr = strstr(line, "=")) != 0)
                title = std::string(++ptr);
            else
                title = std::string(line+5);
        }
        else if(strncmp(line, "CEND", 4) == 0 ||
                strncmp(line, "BEGIN BULK", 10) == 0 ||
                strncmp(line, "NASTRAN", 7) == 0)
        {
            recognized = true;
        }
    }

    visitTimer->StopTimer(readingFile, "Interpreting NASTRAN file");

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

    visitTimer->StopTimer(total, "Loading NASTRAN file");

    return recognized;
}

// ****************************************************************************
//  Method: avtNASTRANFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jul 5 17:24:35 PST 2005
//
// ****************************************************************************

void
avtNASTRANFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    debug4 << "avtNASTRANFileFormat::PopulateDatabaseMetaData" << endl;
    md->SetDatabaseComment(title);
    int sdim = 3;
    int tdim = 3;
    AddMeshToMetaData(md, "mesh", AVT_UNSTRUCTURED_MESH, NULL, 1, 1,
                      sdim, tdim);
}


// ****************************************************************************
//  Method: avtNASTRANFileFormat::GetMesh
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
//  Creation:   Tue Jul 5 17:24:35 PST 2005
//
// ****************************************************************************

vtkDataSet *
avtNASTRANFileFormat::GetMesh(const char *meshname)
{
    debug4 << "avtNASTRANFileFormat::GetMesh" << endl;
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->ShallowCopy(meshDS);

    return ugrid;
}


// ****************************************************************************
//  Method: avtNASTRANFileFormat::GetVar
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
//  Creation:   Tue Jul 5 17:24:35 PST 2005
//
// ****************************************************************************

vtkDataArray *
avtNASTRANFileFormat::GetVar(const char *varname)
{
    debug4 << "avtNASTRANFileFormat::GetVar" << endl;

    // Can't read variables yet.
    EXCEPTION1(InvalidVariableException, varname);
}


// ****************************************************************************
//  Method: avtNASTRANFileFormat::GetVectorVar
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
//  Creation:   Tue Jul 5 17:24:35 PST 2005
//
// ****************************************************************************

vtkDataArray *
avtNASTRANFileFormat::GetVectorVar(const char *varname)
{
    debug4 << "avtNASTRANFileFormat::GetVectorVar" << endl;

    // Can't read variables yet.
    EXCEPTION1(InvalidVariableException, varname);
}

