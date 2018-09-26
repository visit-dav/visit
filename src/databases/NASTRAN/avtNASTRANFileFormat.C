/*****************************************************************************
*
* Copyright (c) 2000 - 2018, Lawrence Livermore National Security, LLC
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

// ************************************************************************* //
//                            avtNASTRANFileFormat.C                         //
// ************************************************************************* //

#include <avtNASTRANFileFormat.h>
#include <avtNASTRANOptions.h>

using namespace NASTRANDBOptions;

#include <vtkCellType.h>
#include <vtkFloatArray.h>

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

#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <map>
#include <string>
#include <vector>
#include <iostream>

using std::map;
using std::string;
using std::vector;

#define ALL_LINES -1
#define INVALID_MAT_ID -INT_MAX

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
//
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
        if (('0' <= *p && *p <= '9') || *p == '.' || *p == '+' || *p == '-')
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
//
//    Mark C. Miller, Mon Jul 27 20:52:42 PDT 2009
//    Moved re-setting of matCountOpt from -1 (which means to search) to
//    OUTSIDE of conditional compilation for !mdserver.
//
//    Mark C. Miller, Tue Aug  4 11:30:07 PDT 2009
//    Added logic to loop over lines of input to handle iterations in which
//    a material id is NOT actually defined. Previously, at the end of
//    every iteration through the loop REGARDLESS of which line of input
//    was observed, it would take the resulting material id -- which defaulted
//    to zero -- and put it into the list of unique material ids. Now, it
//    keeps track of whether a 'valid' material id has been seen before
//    updating the list of unique material ids.
//
//    Edward Rusu, Tue Aug 14 09:48:24 PDT 2018
//    Updated the file reader to handle the new NASTRAN capabilities.
//    Specifically, many of the of the NASTRAN types now support more nodes to
//    make quadratic elements, so I've updated the reader to handle this.
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
    int nPoints = 100; (void) nPoints;
    int nCells = 100; (void) nCells;
    if(nLines == ALL_LINES)
    {
        FileFunctions::VisItStat_t statbuf;
        FileFunctions::VisItStat(name, &statbuf);
        FileFunctions::VisItOff_t fileSize = statbuf.st_size;

        // Make a guess about the number of cells and points based on
        // the size of the file.
        nPoints = fileSize / (FileFunctions::VisItOff_t) 130;
        nCells  = fileSize / (FileFunctions::VisItOff_t) 150;
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
    float pt[3]; (void) pt;
    vtkIdType verts[20]; (void) verts; // QuadraticHexahedron supports up to 20 vertices
    bool recognized = false;
    bool titleRead = false;

#define INDEX_FIELD_WIDTH 8

    for(int lineIndex = 0; ReadLine(ifile, line); ++lineIndex)
    {
        int matid = INVALID_MAT_ID;

        if(nLines != ALL_LINES && lineIndex >= nLines)
            break;

        // Determine what the line is for.
        if(line[0] == '$' && !recognized)
        {
            if(!titleRead)
                title += line;
        }
        else if(strncmp(line, "GRID*", 5) == 0)
        {
            recognized = true;

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
            //int psi = Geti(valstart)-1;

#if !defined(MDSERVER)
            int psi = Geti(valstart)-1;

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
            //int psi = Geti(valstart)-1;

#if !defined(MDSERVER)
            int psi = Geti(valstart)-1;

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
        else if(strncmp(line, "CBAR", 4) == 0 ||
                strncmp(line, "CBEAM", 5) == 0 ||
                strncmp(line, "CBUSH", 5) == 0 ||
                strncmp(line, "CBUSH1D", 7) == 0 ||
                strncmp(line, "CCABLE", 6) == 0 ||
                strncmp(line, "CDAMP3", 6) == 0 ||
                strncmp(line, "CDAMP4", 6) == 0 || // rusu1 - uses a scalar value B for the damper instead of a property identification number
                strncmp(line, "CELAS3", 6) == 0 ||
                strncmp(line, "CELAS4", 6) == 0 || // rusu1 - uses a scalar value K for the stiffness instead of a property identification number
                strncmp(line, "CGAP", 4) == 0 ||
                strncmp(line, "CMASS3", 6) == 0 ||
                strncmp(line, "CMASS4", 6) == 0 || // rusu1 - uses a scalar value M for the stiffness instead of a property identification number
                strncmp(line, "CPIPE", 5) == 0 ||
                strncmp(line, "CROD", 4) == 0 ||
                strncmp(line, "CTUBE", 5) == 0 ||
                strncmp(line, "CVISC", 5) == 0)
        {
            // These cell types are like:
            // CELL_TYPE | EID | PID (or scalar) | GA | GB |
            // The differences in cell types are in the information stored after GB,
            // but we don't care about that for our purposes.
            // We want PID, GA, and GB.

            // Parse the line into verts
            ParseLine(verts, line, 3, 2);

            // Check if element is buildable
            int buildable = CheckBuildable(verts, 2);

            // Set the material property id
            if (matCountOpt) {
                matid = ParseField(line, 2);
            }


#if 0
            debug4 << verts[0]
                   << ", " << verts[1]
                   << endl;
#endif

#if !defined(MDSERVER)
            if (buildable == 1) {
                ugrid->InsertNextCell(VTK_LINE, 2, verts);
            }
            else {
                debug4 << "ERROR: Could not build element EID: " << ParseField(line, 1) << endl;
            }
            if (matCountOpt) matList.push_back(matid);
#endif
        }
        else if(strncmp(line, "CDAMP1", 6) == 0 ||
                strncmp(line, "CDAMP2", 6) == 0 ||
                strncmp(line, "CELAS1", 6) == 0 ||
                strncmp(line, "CELAS2", 6) == 0 ||
                strncmp(line, "CMASS1", 6) == 0 ||
                strncmp(line, "CMASS2", 6) == 0)
        {
            // These cell types are like:
            // CELL_TYPE | EID | PID (or scalar) | G1 | C1 | G2 | C2 |
            // We want PID, G1, and G2

            // Parse the line into verts array
            ParseLine(verts, line, 3, 3);

            // Swap C1 and G2
            int swap_t = verts[1];
            verts[1] = verts[2];
            verts[2] = swap_t;

            // Check if element is buildable
            int buildable = CheckBuildable(verts, 2);

            // Set material property id
            if (matCountOpt) {
                matid = ParseField(line, 2);
            }
#if !defined(MDSERVER)
            if (buildable == 1) {
                ugrid->InsertNextCell(VTK_LINE, 2, verts);
            }
            else {
                debug4 << "ERROR: Could not build element EID: " << ParseField(line, 1) << endl;
            }
            if (matCountOpt) matList.push_back(matid);
#endif

        }
        else if(strncmp(line, "CONROD", 6) == 0)
        {
            // These cell types are like:
            // CELL_TYPE | EID | GA | GB
            // We want GA and GB

            // Parse the line into verts array
            ParseLine(verts, line, 2, 2);

            // Check if element is buildable
            int buildable = CheckBuildable(verts, 2);

            // Set material property id
            if (matCountOpt) {
                matid = ParseField(line, 2);
            }
#if !defined(MDSERVER)
            if (buildable == 1) {
                ugrid->InsertNextCell(VTK_LINE, 2, verts);
            }
            else {
                debug4 << "ERROR: Could not build CONROD EID: " << ParseField(line, 1) << endl;
            }
            if (matCountOpt) matList.push_back(matid);
#endif
        }
        else if(strncmp(line, "CTRIA2", 6) == 0 ||
                strncmp(line, "CTRIA3", 6) == 0 ||
                strncmp(line, "CTRIAR", 6) == 0)
        {
            // These cell types are like:
            // CELL_TYPE | EID | PID | G1 | G2 | G3 |
            // Looks like NASTRAN no longer supports CTRIA2, but we have them in our tests
            // suite, so I've included them here in case a user wants to run with older
            // files.

            // Parse line into verts array
            ParseLine(verts, line, 3, 3);

            // Check if the element is buildable
            int buildable = CheckBuildable(verts, 3);

            // Set the material property id
            if (matCountOpt) {
                matid = ParseField(line, 2);
            }
#if !defined(MDSERVER)
            if (buildable == 1) {
                ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
            }
            else {
                debug4 << "ERROR: Could not build element EID: " << ParseField(line, 1) << endl;
            }
            if (matCountOpt) matList.push_back(matid);
#endif
        }
        else if(strncmp(line, "CTRIA6", 6) == 0)
        {
            // These cell types are like:
            // CELL_TYPE | EID | PID | G1 | G2 | G3 | G4 | G5 | G6

            // Parse line into verts array
            ParseLine(verts, line, 3, 6);

            // Check if element is buildable
            int buildable = CheckBuildable(verts, 6, 3);

            // Set the material property id
            if (matCountOpt) {
                matid = ParseField(line, 2);
            }
#if !defined(MDSERVER)
            if(buildable == 2) { // High order build passed
                ugrid->InsertNextCell(VTK_QUADRATIC_TRIANGLE, 6, verts);
            }
            else if(buildable == 1) { // First order build passed
                debug4 << "WARNING: Could not build QUADRATIC_TRIANGLE "
                       << "at CTRIA6 EID: " << ParseField(line, 1) << ". "
                       << "Building TRIANGLE instead." << endl;
                ugrid->InsertNextCell(VTK_QUAD, 4, verts);
            } else {
                debug4 << "ERROR: Could not build element CTRIA6 EID: " << ParseField(line, 1)
                       << endl;
            }
            if (matCountOpt) matList.push_back(matid);
#endif
        }
        else if(strncmp(line, "CQUAD4", 6) == 0 ||
                strncmp(line, "CSHEAR", 6) == 0 ||
                strncmp(line, "CQUADR", 6) == 0)
        {
            // These cell types are like:
            // CELL_TYPE | EID | PID | G1 | G2 | G3 | G4 |

            // Parse line into verts array
            ParseLine(verts, line, 3, 4);

            // Check if the element is buildable
            int buildable = CheckBuildable(verts, 4);

            // Set the material property id
            if (matCountOpt)
            {
                matid = ParseField(line, 2);
            }
#if !defined(MDSERVER)
            if (buildable == 1) {
                ugrid->InsertNextCell(VTK_QUAD, 4, verts);
            }
            else {
                debug4 << "ERROR: Could not build element EID: " << ParseField(line, 1) << endl;
            }
            if (matCountOpt) matList.push_back(matid);
#endif
        }
        else if(strncmp(line, "CQUAD8", 6) == 0)
        {
            // These cell types are like:
            // CELL_TYPE | EID | PID | G1 | G2 | G3 | G4 | G5 | G6 |
            //           | G7  | G8  |

            // Parse line into verts array
            ParseLine(verts, line, 3, 8);

            // Check if the element is buildable
            int buildable = CheckBuildable(verts, 8, 4);

            // Set the material property id
            if (matCountOpt)
            {
                matid = ParseField(line, 2);
            }
#if !defined(MDSERVER)
            if(buildable == 2) { // High order build passed
                ugrid->InsertNextCell(VTK_QUADRATIC_QUAD, 8, verts);
            }
            else if(buildable == 1) { // First order build passed
                debug4 << "WARNING: Could not build QUADRATIC_QUAD "
                       << "at CQUAD8 EID: " << ParseField(line, 1) << ". "
                       << "Building QUAD instead." << endl;
                ugrid->InsertNextCell(VTK_QUAD, 4, verts);
            } else {
                debug4 << "ERROR: Could not build element CQUAD8 EID: " << ParseField(line, 1)
                       << endl;
            }
            if (matCountOpt) matList.push_back(matid);
#endif
        }
        else if(strncmp(line, "CTETRA", 6) == 0)
        {
            // These cell types are like:
            // CELL_TYPE | EID | PID | G1 | G2  | G3 | G4 | G5 | G6 |
            //           | G7  | G8  | G9 | G10 |
            // where G5-10 are only needed for QUADRATIC.

            // Parse line into verts array
            ParseLine(verts, line, 3, 10);

            // Check if the element is buildable
            int buildable = CheckBuildable(verts, 10, 4);

            // Set the material property id
            if (matCountOpt)
            {
                matid = ParseField(line, 2);
            }
#if !defined(MDSERVER)
            if(buildable == 2) {
                ugrid->InsertNextCell(VTK_QUADRATIC_TETRA, 10, verts);
            }
            else if(buildable == 1) {
                debug5 << "Could not build QUADRATIC_TETRA "
                       << "at CTETRA EID: " << ParseField(line, 1) << ". "
                       << "Building TETRA element." << endl;
                ugrid->InsertNextCell(VTK_TETRA, 4, verts);
            }
            else {
                debug4 << "ERROR: Could not build element CTETRA EID: " << ParseField(line, 1)
                       << endl;
            }
            if (matCountOpt) matList.push_back(matid);
#endif
        }
        else if(strncmp(line, "CPYRAM", 6) == 0 ||
                strncmp(line, "CPYRA", 5) == 0)
        {
            // These cell types are like:
            // CELL_TYPE | EID | PID | G1 | G2  | G3  | G4  | G5  | G6 |
            //           | G7  | G8  | G9 | G10 | G11 | G12 | G13 |
            // where G6-13 are only needed for QUADRATIC.
            // Looks like NASTRAN no longer supporst CPYRAM, but we have that in our test
            // suite so I've included it here in case the user wants to run an older file.

            // Parse line into verts array
            ParseLine(verts, line, 3, 13);

            // Check if the element is buildable
            int buildable = CheckBuildable(verts, 13, 5);

            // Set the material property id
            if (matCountOpt)
            {
                matid = ParseField(line, 2);
            }
#if !defined(MDSERVER)
            if(buildable == 2) { // High order build passed
                ugrid->InsertNextCell(VTK_QUADRATIC_PYRAMID, 13, verts);
            }
            else if(buildable == 1) { // First order build passed
                debug5 << "Could not build QUADRATIC_PYRAMID "
                       << "at CPYRA EID: " << ParseField(line, 1) << ". "
                       << "Building PYRAMID element." << endl;
                ugrid->InsertNextCell(VTK_PYRAMID, 5, verts);
            } else {
                debug4 << "ERROR: Could not build element CPYRA EID: " << ParseField(line, 1)
                       << std::endl;
            }
            if (matCountOpt) matList.push_back(matid);
#endif
        }
        else if(strncmp(line, "CPENTA", 6) == 0)
        {
            // These cell types are like:
            // CELL_TYPE | EID | PID | G1  | G2  | G3  | G4  | G5  | G6  |
            //           | G7  | G8  | G9  | G10 | G11 | G12 | G13 | G14 |
            //           | G15 |
            // where G7-15. are only needed for QUADRATIC.

            // Parse line into verts array
            ParseLine(verts, line, 3, 15);

            // Check if the element is buildable
            int buildable = CheckBuildable(verts, 15, 6);


            // PENTA's (Wedges) have mismatched indices between NATRAN and VTK,
            // so we fix that here.
            if (buildable > 0) { // First order element fix
                int swap_t = verts[1];
                verts[1] = verts[2];
                verts[2] = swap_t;

                swap_t = verts[4];
                verts[4] = verts[5];
                verts[5] = swap_t;
            }
            if (buildable > 1) { // Quadratic element fix
                int swap_t = verts[6];
                verts[6] = verts[8];
                verts[8] = swap_t;

                swap_t = verts[9];
                verts[9] = verts[14];
                verts[14] = verts[10];
                verts[10] = verts[13];
                verts[13] = verts[11];
                verts[11] = verts[12];
                verts[12] = swap_t;
            }

            // Set the material property id
            if (matCountOpt)
            {
                matid = ParseField(line, 2);
            }
#if !defined(MDSERVER)
            if(buildable == 2) { // High order build passed
                ugrid->InsertNextCell(VTK_QUADRATIC_WEDGE, 15, verts);
            }
            else if(buildable == 1) { // First order build passed
                debug5 << "Could not build QUADRATIC_WEDGE "
                       << "at CPENTA EID: " << ParseField(line, 1) << ". " << endl;
                if (verts[4] == verts[5])
                {
                    debug5 << "Could not build WEDGE at CPENTA EID: "
                           << ParseField(line, 1) << ". "
                           << "Building PYRAMID element." << endl;
                    ugrid->InsertNextCell(VTK_PYRAMID, 5, verts);
                }
                else {
                    debug5 << "Building WEDGE element." << endl;
                    ugrid->InsertNextCell(VTK_WEDGE, 6, verts);
                }
            } else {
                debug4 << "ERROR: Could not build element CPENTA EID: " << ParseField(line, 1)
                       << std::endl;
            }
            if (matCountOpt) matList.push_back(matid);
#endif
        }
        else if(strncmp(line, "CHEXA", 5) == 0)
        {
            // These cell types are like:
            // CELL_TYPE | EID | PID | G1  | G2  | G3  | G4  | G5  | G6  |
            //           | G7  | G8  | G9  | G10 | G11 | G12 | G13 | G14 |
            //           | G15 | G16 | G17 | G18 | G19 | G20 |
            // where G9-20. are only needed for QUADRATIC.

            // Parse line into verts array
            ParseLine(verts, line, 3, 20);

            // Check if the element is buildable
            int buildable = CheckBuildable(verts, 20, 8);

            // Quadratic Hexahedrons are mismatched between NASTRAN and VTK.
            if(buildable == 2)
            {
                int swap_t = -1;
                for (int i = 12, j = 16; i < 16; i++, j++)
                {
                    swap_t = verts[i];
                    verts[i] = verts[j];
                    verts[j] = swap_t;
                }
            }

            // Set the material property id
            if (matCountOpt)
            {
                matid = ParseField(line, 2);
            }
#if !defined(MDSERVER)
            if(buildable == 2) { // High order build passed
                ugrid->InsertNextCell(VTK_QUADRATIC_HEXAHEDRON, 20, verts);
            }
            else if(buildable == 1) { // First order build passed
                debug5 << "Could not build QUADRATIC_HEXAHEDRON "
                       << "at CHEXA EID: " << ParseField(line, 1) << ". "
                       << "Building HEXAHEDRON." << endl;
                ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, verts);
            } else {
                debug4 << "ERROR: Could not build element CHEXA EID: " << ParseField(line, 1)
                       << endl;
            }
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
        else if(strncmp(line, "ENDDATA", 7) == 0)
        {
            break;
        } // end line if

        if (matid != INVALID_MAT_ID)
            uniqMatIds[matid] = 1;
    } // end ReadLine loop


    visitTimer->StopTimer(readingFile, "Interpreting NASTRAN file");

    if(recognized && nLines == ALL_LINES)
    {
#if !defined(MDSERVER) && defined(USE_POINT_INDICES_TO_INSERT)
        int rpfTime = visitTimer->StartTimer();
        vtkUnstructuredGridRelevantPointsFilter *rpf =
            vtkUnstructuredGridRelevantPointsFilter::New();
        rpf->SetInputData(ugrid);
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
#endif

    if (matCountOpt == -1)
        matCountOpt = (int)uniqMatIds.size();

    visitTimer->StopTimer(total, "Loading NASTRAN file");

    return recognized;
}

// ****************************************************************************
//  Method: avtNASTRANFileFormat::ReadLine
//
//  Purpose:
//      This is a helper function for ReadFile that builds the line variable.
//      If the line ends in "+CONT", "*CONT", "+", or "*", then read the next
//      line as well. Return true if the file reader is still good and false if
//      the file reader has failed.
//
//  Programmer: Edward Rusu
//  Creation:   Tue Jul 31 9:26:18 PST 2018
// ****************************************************************************
bool
avtNASTRANFileFormat::ReadLine(ifstream& ifile, char *line)
{
    // Wipe out the line to ensure clear read
    for (int i = 0; i < 1024; i++) {
        line[i] = ' ';
    }

    // Initial line read
    ifile.getline(line, 1024);

    // Check for continuation line
    char* contCheck;
    for (int i = 0; i < 1024/72; i++)
    {
        int ndx = 72*(i+1);
        contCheck = line + ndx;

        if(strncmp(contCheck, "+CONT", 5) == 0 ||
           strncmp(contCheck, "*CONT", 5) == 0 ||
           *contCheck == '+' ||
           *contCheck == '*')
        {
            // If continuation line, add in the next line
            ifile.getline(line + ndx, 1024-ndx);
        }
        else break;
    }

    // Determine if the reader has reached the end of the file
    // or if there is some kind of error.
    if(ifile.fail())
    {
        if (ifile.eof())
        {
            std::cout << "File successfuly read!" << std::endl;
            return false;
        }
        else {
            std::cout << "ERROR: File reader has failed!" << std::endl;
            return false;
        }
    }
    else if(!ifile.good())
    {
        std::cout << "ERROR: File reader is not good!" << std::endl;
        return false;
    }
    else
    {
        return true;
    }
}

// ****************************************************************************
//  Method: avtNASTRANFileFormat::ParseLine
//
//  Purpose:
//      This is a helper function for ReadFile that parses the line into
//      the vertices array based on the spacing defined in INDEX_FIELD_WIDTH.
//      Takes as input the verts array as input, the line to parse, an index
//      for where to start parsing, and the number of elements to parse.
//
//  Programmer: Edward Rusu
//  Creation:   Mon Aug 13 14:31:24 PST 2018
//
//  Modifications:
//    Kathleen Biagas, Tue Sep 25 16:00:00 MST 2018
//    Ensure verts up to index 'count' have been initialized. Prevents crash
//    later on when it is expected that 'count' values are either valid or
//    are set to -1.
//
// ****************************************************************************

void
avtNASTRANFileFormat::ParseLine(vtkIdType *verts, char *line, int start,
                                int count)
{
    char *valstart = line + INDEX_FIELD_WIDTH * start;
    char *valend = valstart + INDEX_FIELD_WIDTH;
    char val_t;
    int i;
    for (i = 0; (i < count) && (*valstart != '\0'); i++) {
        if(*valstart == '+' || *valstart == '*')
        { // At blank continuation element
            valstart += INDEX_FIELD_WIDTH;
            valend += INDEX_FIELD_WIDTH;
        }

        val_t = *valend;
        *valend = '\0';

        verts[i] = Geti(valstart)-1;

        *valend = val_t;
        valstart += INDEX_FIELD_WIDTH;
        valend += INDEX_FIELD_WIDTH;
    }
    for (int j = i; j < count; ++j)
        verts[j] = -1;
}

// ****************************************************************************
//  Method: avtNASTRANFileFormat::CheckBuildable
//
//  Purpose:
//      This is a helper function for ReadFile that checks the verts array
//      to ensure that the cell is buildable to specification. Takes in the
//      verts array and the number of expected elements. Returns 1 if base
//      element can be built, and 0 if the element cannot be built.
//      The overload also takes in the number of needed elements. It returns
//      2 if a quadratic element can be built.
//
//  Programmer: Edward Rusu
//  Creation:   Mon Aug 13 10:42:24 PST 2018
// ****************************************************************************
int
avtNASTRANFileFormat::CheckBuildable(const vtkIdType *verts, int numExpected)
{
    int buildable = 1;
    for (int i = 0; i < numExpected; i++) {
        if (verts[i] == -1)
        {
            buildable = 0;
            break;
        }
    }
    return buildable;
}

int
avtNASTRANFileFormat::CheckBuildable(const vtkIdType *verts, int numExpected,
                                     int numNeeded)
{
    int buildable = 2;
    for (int i = numNeeded; i < numExpected; i++) {
        if (verts[i] == -1)
        {
            buildable = 1;
            break;
        }
    }
    for (int i = 0; i < numNeeded; i++) {
        if (verts[i] == -1)
        {
            buildable = 0;
            break;
        }
    }
    return buildable;
}

// ****************************************************************************
//  Method: avtNASTRANFileFormat::ParseField
//
//  Purpose:
//      This is a helper function for ReadFile that parses a single field out
//      of line. Takes the line as input as well as starting point for parsing.
//      Assumes spacing defined in INDEX_FIELD_WIDTH. Returns the parsed
//      integer.
//
//  Programmer: Edward Rusu
//  Creation:   Mon Aug 14 13:06:24 PST 2018
// ****************************************************************************
int
avtNASTRANFileFormat::ParseField(char *line, int start)
{
    char *valstart = line + INDEX_FIELD_WIDTH * start;
    char *valend = valstart + INDEX_FIELD_WIDTH;
    char val_t = *valend;

    *valend = '\0';

    int out = Geti(valstart);

    *valend = val_t;

    return out;
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
//
//    Mark C. Miller, Tue Aug  4 11:32:31 PDT 2009
//    Modified generated names to use actual material ids found in the data.
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

    if (uniqMatIds.size() > 0)
    {
        vector<string> names;
        for (map<int,int>::iterator mit = uniqMatIds.begin();
             mit != uniqMatIds.end(); mit++)
        {
            char tmpn[32];
            SNPRINTF(tmpn,sizeof(tmpn),"mat_%d",mit->first);
            names.push_back(tmpn);
        }

        avtMaterialMetaData *mmd = new avtMaterialMetaData("materials", "mesh",
                                           matCountOpt, names);
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
//  Modifications:
//    Mark C. Miller, Tue Aug  4 11:33:58 PDT 2009
//    Fixed some leaks and unnecessary copies of matlist array. Changed
//    constructor to one that supports specification of the list of material
//    numbers. Changed name generation to use actual material numbers found
//    in the input data.
// ****************************************************************************

avtMaterial *
avtNASTRANFileFormat::GetMaterial(const char *mat)
{
    //
    // This condition can happen if user specified mat count via read options
    // and ReadFile found a different number of them.
    //
    if ((size_t)matCountOpt != uniqMatIds.size())
    {
        char msg[256];
        SNPRINTF(msg, sizeof(msg), "Material count specified in read options, %d, "
            "does not match what is actually found in the file, %d",
            matCountOpt, (int)uniqMatIds.size());
        EXCEPTION1(ImproperUseException, msg);
    }

    char **names = new char*[uniqMatIds.size()];
    int *matnos = new int[uniqMatIds.size()];
    size_t mno = 0;
    for (map<int,int>::iterator mit = uniqMatIds.begin();
         mit != uniqMatIds.end(); mno++, mit++)
    {
        char tmpn[32];
        SNPRINTF(tmpn,sizeof(tmpn),"mat_%d",mit->first);
        names[mno] = strdup(tmpn);
        matnos[mno] = mit->first;
    }

    int dims = (int)matList.size();
    avtMaterial *retval = new avtMaterial((int)uniqMatIds.size(), matnos,
        names, 1, &dims, 0, &matList[0], 0, 0, 0, 0, 0, 0, 0);

    delete [] matnos;
    for (mno = 0; mno < uniqMatIds.size(); mno++)
        delete [] names[mno];
    delete [] names;

    return retval;
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

