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
#include <avtNASTRANOptions.h>

#include <errno.h>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>

#include <vtkCellType.h>
#include <vtkFloatArray.h>
#include <vtkUnstructuredGrid.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>
#include <avtMaterial.h>
#include <avtMaterialMetaData.h>

#include <ImproperUseException.h>
#include <InvalidVariableException.h>
#include <InvalidFilesException.h>

#include <DBOptionsAttributes.h>
#include <DebugStream.h>
#include <FileFunctions.h>
#include <TimingsManager.h>
#include <snprintf.h>

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
using     std::map;

#define ALL_LINES -1

// ****************************************************************************
//  Method: avtNASTRANFileFormat constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jul 5 17:24:35 PST 2005
//
//  Modifications:
//
//    Mark C. Miller, Wed May 13 23:11:59 PDT 2009
//    Added option to indicate number of materials.
// ****************************************************************************

avtNASTRANFileFormat::avtNASTRANFileFormat(const char *filename,
    DBOptionsAttributes *rdatts) : avtSTSDFileFormat(filename), title()
{
    meshDS = 0;
    matCountOpt = 0;

    for (int i = 0; rdatts != 0 && i < rdatts->GetNumberOfOptions(); ++i)
    {
        if (rdatts->GetName(i) == NASTRAN_RDOPT_MAT_COUNT)
            matCountOpt = rdatts->GetInt(NASTRAN_RDOPT_MAT_COUNT);
        else
            debug1 << "Ignoring unknown option \"" << rdatts->GetName(i) << "\"" << endl;
    }

#ifdef MDSERVER
    if(!ReadFile(filename, matCountOpt == -1 ? ALL_LINES : 100))
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
        matList.clear();
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
// Function: Getf 
//
// Purpose: Robust way of reading string for float value
// 
// Programmer: Mark C. Miller, Thu Apr  3 16:27:01 PDT 2008
//
// Modifications:
//    Jeremy Meredith, Thu Aug  7 13:43:03 EDT 2008
//    Format %s doesn't use space modifier.
//
//    Mark C. Miller, Wed May  6 11:43:37 PDT 2009
//    Added logic to deal with funky NASTRAN format where the 'e' character
//    may be missing from exponentiated numbers.
//
//    Mark C. Miller, Thu May  7 10:30:49 PDT 2009
//    Fixed bug of triggering funky case in presence of leading spaces. 
//
//    Mark C. Miller, Mon May 11 14:21:22 PDT 2009
//    Ok, I 'fixed' this funky logic again. The above 'fix' caused the alg.
//    to basically completely fail.
// ****************************************************************************
static float Getf(const char *s)
{
    char *ends;
    double val = 0.0;

    // Check for one of these funky 'NASTRAN exponential format' strings.
    // This is where a value like '1.2345e-5' is actually represented in the 
    // file as '1.2345-5' with the 'e' character missing. It is awkward but 
    // apparently a NASTRAN standard. I guess the rationale is that given
    // an 8 character field width limit, removing the 'e' character gives them
    // one additional digit of precision. This logic is basically looking for
    // the condition of encountering a sign character, '-' or '+', AFTER having
    // seen characters that could represent part of a number. In such a case,
    // it MUST be the sign of the exponent.
    const char *p = s;
    char tmps[32];
    char *q = tmps;
    bool haveSeenNumChars = false;
    while (!haveSeenNumChars || (*p != '-' && *p != '+' && *p != '\0'))
    {
        if ('0' <= *p && *p <= '9' || *p == '.' || *p == '+' || *p == '-')
            haveSeenNumChars = true;
        *q++ = *p++;
    }
    if (haveSeenNumChars && (*p == '-' || *p == '+'))
    {
        *q++ = 'e';
        while (*p != '\0')
            *q++ = *p++;
        *q++ = '\0';
        errno = 0;
        val = strtod(tmps, &ends);
    }
    else
    {
        errno = 0;
        val = strtod(s, &ends);
    }

    if (errno != 0)
    {
        char msg[512];
        SNPRINTF(msg, sizeof(msg),
            "Error \"%s\" at word \"%32s\"\n", strerror(errno), s);
        if (!avtCallback::IssueWarning(msg))
            cerr << msg << endl;
        return 0.0;
    }

    return (float) val;
}

// ****************************************************************************
// Function: Geti 
//
// Purpose: Robust way of reading string for integer value
// 
// Programmer: Mark C. Miller, Thu Apr  3 16:27:01 PDT 2008
//
// Modifications:
//    Jeremy Meredith, Thu Aug  7 13:43:03 EDT 2008
//    Format %s doesn't use space modifier.
//
// ****************************************************************************
static int Geti(const char *s)
{
    char *ends;

    errno = 0;
    long val = strtol(s, &ends, 10);

    if (errno != 0)
    {
        char msg[512];
        SNPRINTF(msg, sizeof(msg),
            "Error \"%s\" at word \"%32s\"\n", strerror(errno), s);
        if (!avtCallback::IssueWarning(msg))
            cerr << msg << endl;
        return 0;
    }

    return (int) val;
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
//    Mark C. Miller, Tue May  5 11:26:50 PDT 2009
//    Fixed bug handling mesh with very small number of points (<10). The
//    logic to increase vtkPoints object size resulted in having no effect
//    because 1.1* current size was resulting in same size. Also, added
//    logic to deal with CPENTA element types that are really pyramids.
//
//    Mark C. Miller, Wed May 13 23:11:27 PDT 2009
//    Added support for materials.
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

#if !defined(MDSERVER)

    vtkPoints *pts = vtkPoints::New();
    pts->Allocate(nPoints);
#ifdef USE_POINT_INDICES_TO_INSERT
    pts->SetNumberOfPoints(nPoints);
#endif

    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->SetPoints(pts);
    ugrid->Allocate(nCells);
    pts->Delete();
    pts = ugrid->GetPoints();

#endif // if !defined(MDSERVER)

    char  line[1024];
    float pt[3];
    int verts[8];
    int matid = 0;
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
            char *valstart = line + 81 - 1;
            pt[2] = Getf(valstart);

            valstart = line + 72 - LONG_FIELD_WIDTH+1 - 1;
            char *valend = line + 72;
            *valend = '\0';
            pt[1] = Getf(valstart);

            valstart -= LONG_FIELD_WIDTH;
            valend -= LONG_FIELD_WIDTH;
            *valend = '\0';
            pt[0] = Getf(valstart);

#ifdef USE_POINT_INDICES_TO_INSERT
            valstart -= (2 * LONG_FIELD_WIDTH);
            valend -= (2 * LONG_FIELD_WIDTH);
            *valend = '\0';
            int psi = Geti(valstart)-1;

#if !defined(MDSERVER)

            if(psi < nPoints)
                pts->SetPoint(psi, pt);
            else
            {
                int newSize = int(float(nPoints) * 1.1f);
                if(newSize < psi)
                    newSize = int(float(psi) * 1.1f);
                if(newSize <= nPoints)
                    newSize = nPoints + 1;

                debug4 << "Resizing point array from " << nPoints
                       << " points to " << newSize
                       << " points because we need to insert point index "
                       << psi << endl;
                nPoints = newSize;
                pts->GetData()->Resize(nPoints);
                pts->SetNumberOfPoints(nPoints);
                pts->SetPoint(psi, pt);
            }

#endif // if !defined(MDSERVER)

#else
#if !defined(MDSERVER)
            pts->InsertNextPoint(pt);
#endif
#endif

        }
        else if(strncmp(line, "GRID", 4) == 0)
        {
            recognized = true;

#define SHORT_FIELD_WIDTH 8
            char *valstart = line + 48 - SHORT_FIELD_WIDTH;
            char *valend = line + 48;

            *valend = '\0';
            pt[2] = Getf(valstart);

            valstart -= SHORT_FIELD_WIDTH;
            valend -= SHORT_FIELD_WIDTH;
            *valend = '\0';
            pt[1] = Getf(valstart);

            valstart -= SHORT_FIELD_WIDTH;
            valend -= SHORT_FIELD_WIDTH;
            *valend = '\0';
            pt[0] = Getf(valstart);

#if 0
            debug4 << pt[0] << ", " << pt[1] << ", " << pt[2] << endl;
#endif
#ifdef USE_POINT_INDICES_TO_INSERT
            valstart -= (2 * SHORT_FIELD_WIDTH);
            valend -= (2 * SHORT_FIELD_WIDTH);
            *valend = '\0';
            int psi = Geti(valstart)-1;

#if !defined(MDSERVER)

            if(psi < nPoints)
                pts->SetPoint(psi, pt);
            else
            {
                int newSize = int(float(nPoints) * 1.1f);
                if(newSize < psi)
                    newSize = int(float(psi) * 1.1f);
                if(newSize <= nPoints)
                    newSize = nPoints + 1;

                debug4 << "Resizing point array from " << nPoints
                       << " points to " << newSize
                       << " points because we need to insert point index "
                       << psi << endl;
                nPoints = newSize;
                pts->GetData()->Resize(nPoints);
                pts->SetNumberOfPoints(nPoints);
                pts->SetPoint(psi, pt);
            }

#endif // if !defined(MDSERVER)

#else

#if !defined(MDSERVER)
            pts->InsertNextPoint(pt);
#endif // if !defined(MDSERVER)

#endif


        }
        else if(strncmp(line, "CHEXA", 5) == 0)
        {
#define INDEX_FIELD_WIDTH 8
            // CHEXA requires more point indices so read another line.
            ifile.getline(line + 72, 1024-72);

            char *valstart = line + 88;
            char *valend = valstart;
            verts[7] = Geti(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[6] = Geti(valstart)-1;

            // Skip the blank
            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[5] = Geti(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[4] = Geti(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[3] = Geti(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[2] = Geti(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[1] = Geti(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[0] = Geti(valstart)-1;

            if (matCountOpt)
            {
                valstart -= INDEX_FIELD_WIDTH;
                valend -= INDEX_FIELD_WIDTH;
                *valend = '\0';
                matid = Geti(valstart);
            }

#if !defined(MDSERVER)
            ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, verts);
            if (matCountOpt) matList.push_back(matid);
#endif

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
            verts[3] = Geti(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[2] = Geti(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[1] = Geti(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[0] = Geti(valstart)-1;

            if (matCountOpt)
            {
                valstart -= INDEX_FIELD_WIDTH;
                valend -= INDEX_FIELD_WIDTH;
                *valend = '\0';
                matid = Geti(valstart);
            }

#if 0
            debug4 << verts[0]
                   << ", " << verts[1]
                   << ", " << verts[2]
                   << ", " << verts[3]
                   << endl;
#endif

#if !defined(MDSERVER)
            ugrid->InsertNextCell(VTK_TETRA, 4, verts);
            if (matCountOpt) matList.push_back(matid);
#endif
        }
        else if(strncmp(line, "CPYRAM", 6) == 0)
        {
            char *valstart = line + 56;
            char *valend = valstart;
            verts[4] = Geti(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[3] = Geti(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[2] = Geti(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[1] = Geti(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[0] = Geti(valstart)-1;

            if (matCountOpt)
            {
                valstart -= INDEX_FIELD_WIDTH;
                valend -= INDEX_FIELD_WIDTH;
                *valend = '\0';
                matid = Geti(valstart);
            }

#if 0
            debug4 << verts[0]
                   << ", " << verts[1]
                   << ", " << verts[2]
                   << ", " << verts[3]
                   << ", " << verts[4]
                   << endl;
#endif

#if !defined(MDSERVER)
            ugrid->InsertNextCell(VTK_PYRAMID, 5, verts);
            if (matCountOpt) matList.push_back(matid);
#endif
        }
        else if(strncmp(line, "CPENTA", 6) == 0)
        {
            char *valstart = line + 64;
            char *valend = valstart;
            verts[5] = Geti(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[4] = Geti(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[3] = Geti(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[2] = Geti(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[1] = Geti(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[0] = Geti(valstart)-1;

            if (matCountOpt)
            {
                valstart -= INDEX_FIELD_WIDTH;
                valend -= INDEX_FIELD_WIDTH;
                *valend = '\0';
                matid = Geti(valstart);
            }

#if 0
            debug4 << verts[0]
                   << ", " << verts[1]
                   << ", " << verts[2]
                   << ", " << verts[3]
                   << ", " << verts[4]
                   << ", " << verts[5]
                   << endl;
#endif
            //
            // http://www.simcenter.msstate.edu/docs/ug_io/3d_grid_file_type_nastran.html
            // says that if 5th and 6th nodes are identical, then its really a 5 noded
            // pyramid.
            //
#if !defined(MDSERVER)
            if (verts[4] == verts[5])
                ugrid->InsertNextCell(VTK_PYRAMID, 5, verts);
            else
                ugrid->InsertNextCell(VTK_WEDGE, 6, verts);
            if (matCountOpt) matList.push_back(matid);
#endif
        }
        else if(strncmp(line, "CQUAD4", 6) == 0)
        {
            char *valstart = line + 48;
            char *valend = valstart;
            verts[3] = Geti(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[2] = Geti(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[1] = Geti(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[0] = Geti(valstart)-1;

            if (matCountOpt)
            {
                valstart -= INDEX_FIELD_WIDTH;
                valend -= INDEX_FIELD_WIDTH;
                *valend = '\0';
                matid = Geti(valstart);
            }

#if 0
            debug4 << verts[0]
                   << ", " << verts[1]
                   << ", " << verts[2]
                   << ", " << verts[3]
                   << endl;
#endif

#if !defined(MDSERVER)
            ugrid->InsertNextCell(VTK_QUAD, 4, verts);
            if (matCountOpt) matList.push_back(matid);
#endif
        }
        else if(strncmp(line, "CTRIA2", 6) == 0 ||
                strncmp(line, "CTRIA3", 6) == 0)
        {
            char *valstart = line + 40;
            char *valend = valstart;
            verts[2] = Geti(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[1] = Geti(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            valend -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[0] = Geti(valstart)-1;

            if (matCountOpt)
            {
                valstart -= INDEX_FIELD_WIDTH;
                valend -= INDEX_FIELD_WIDTH;
                *valend = '\0';
                matid = Geti(valstart);
            }

#if 0
            debug4 << verts[0]
                   << ", " << verts[1]
                   << ", " << verts[2]
                   << endl;
#endif

#if !defined(MDSERVER)
            ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
            if (matCountOpt) matList.push_back(matid);
#endif
        }
        else if(strncmp(line, "CBAR", 4) == 0)
        {
            char *valstart = line + 32;
            char *valend = valstart;
            verts[1] = Geti(valstart)-1;

            valstart -= INDEX_FIELD_WIDTH;
            *valend = '\0';
            verts[0] = Geti(valstart)-1;

            if (matCountOpt)
            {
                valstart -= INDEX_FIELD_WIDTH;
                valend -= INDEX_FIELD_WIDTH;
                *valend = '\0';
                matid = Geti(valstart);
            }

#if 0
            debug4 << verts[0]
                   << ", " << verts[1]
                   << endl;
#endif

#if !defined(MDSERVER)
            ugrid->InsertNextCell(VTK_LINE, 2, verts);
            if (matCountOpt) matList.push_back(matid);
#endif
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

        uniqMatIds[matid] = 1;
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
#if !defined(MDSERVER)
        meshDS = ugrid;
#endif
#endif
    }
#if !defined(MDSERVER)
    else
        ugrid->Delete();

    if (matCountOpt == -1)
        matCountOpt = uniqMatIds.size();
#endif

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
//    Mark C. Miller, Wed May 13 23:12:41 PDT 2009
//    Added materials using new MaterialMetaData constructor that takes only
//    number of materials.
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

    if (matCountOpt > 0)
    {
        avtMaterialMetaData *mmd = new avtMaterialMetaData("materials", "mesh",
                                           matCountOpt);
        md->Add(mmd);
    }

}

// ****************************************************************************
//  Method: avtNASTRANFileFormat::GetAuxiliaryData
//
//  Purpose:
//      Gets the auxiliary data from a Silo file.
//
//  Arguments:
//      var        The variable of interest.
//      domain     The domain of interest.
//      type       The type of auxiliary data.
//      <unnamed>  The arguments for that -- not used for any Silo types.
//
//  Programmer: Mark C. Miller
//  Creation:   Wed May 13 17:14:48 PDT 2009
// ****************************************************************************

void *
avtNASTRANFileFormat::GetAuxiliaryData(const char *var, const char *type,
    void *, DestructorFunction &df)
{
    void *rv = NULL;

    if (strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        rv = (void *) GetMaterial(var);
        df = avtMaterial::Destruct;
    }

    return rv;
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
//  Method: avtNASTRANFileFormat::GetMaterial
//
//  Purpose:
//      Gets the material of given name.
//
//  Programmer: Mark C. Miller 
//  Creation:   Wed May 13 17:51:17 PDT 2009
//
// ****************************************************************************

avtMaterial *
avtNASTRANFileFormat::GetMaterial(const char *mat)
{
    //
    // This condition can happen if user specified mat count via read options
    // and ReadFile found a different number of them.
    //
    if (matCountOpt != uniqMatIds.size())
    {
        char msg[256];
        SNPRINTF(msg, sizeof(msg), "Material count specified in read options, %d, "
            "does not match what is actually found in the file, %d",
            matCountOpt, uniqMatIds.size());
        EXCEPTION1(ImproperUseException, msg);
    }

    vector<string> names;
    for (int i = 0; i < matCountOpt; i++)
    {
        char tmpn[32];
        SNPRINTF(tmpn,sizeof(tmpn),"mat_%d",i);
        names.push_back(tmpn);
    }

    //
    // We have to do this copy because VisIt is expecting to be able to delete
    // the avtMaterial object and we can't override it with our own variant
    // that DOES NOT delete the matlist part of it because that member is
    // private.
    //
    int *matlist = new int[matList.size()];
    memcpy(matlist, &matList[0], matList.size() * sizeof(int));
    return new avtMaterial(matCountOpt, names, matList.size(),
                             &matList[0], 0, 0, 0, 0, 0);
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

