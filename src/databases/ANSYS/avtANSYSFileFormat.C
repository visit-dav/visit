/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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
//                            avtANSYSFileFormat.C                           //
// ************************************************************************* //

#include <avtANSYSFileFormat.h>

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>

#if defined(_WIN32)
// for _strnicmp
#include <string.h>
#define STRNCASECMP _strnicmp
#else
// for strcasecmp
#include <strings.h>
#define STRNCASECMP strncasecmp
#endif

#include <vtkCellType.h>
#include <vtkFloatArray.h>
#include <vtkUnstructuredGrid.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>

#include <TimingsManager.h>
#include <DebugStream.h>
#include <FileFunctions.h>

using     std::string;

#define ALL_LINES -1

// ****************************************************************************
//  Method: avtANSYSFileFormat constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jul 5 17:24:35 PST 2005
//
//  Modifications:
//
// ****************************************************************************

avtANSYSFileFormat::avtANSYSFileFormat(const char *filename)
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
// Method: avtANSYSFileFormat::~avtANSYSFileFormat
//
// Purpose: 
//   Destructor for avtANSYSFileFormat class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 6 15:59:07 PST 2005
//
// Modifications:
//   
// ****************************************************************************

avtANSYSFileFormat::~avtANSYSFileFormat()
{
    FreeUpResources();
}

// ****************************************************************************
//  Method: avtANSYSFileFormat::FreeUpResources
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
avtANSYSFileFormat::FreeUpResources(void)
{
    debug4 << "avtANSYSFileFormat::FreeUpResources" << endl;
    if(meshDS)
    {
        meshDS->Delete();
        meshDS = 0;
    }

    title = "";
}

// ****************************************************************************
// Method: avtANSYSFileFormat::ActivateTimestep
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
avtANSYSFileFormat::ActivateTimestep()
{
    debug4 << "avtANSYSFileFormat::ActivateTimestep" << endl;
#ifndef MDSERVER
    if(meshDS == 0)
    {
        ReadFile(filename, ALL_LINES);
    }
#endif
}

// ****************************************************************************
// Method: avtANSYSFileFormat::ReadFile
//
// Purpose: 
//   This method reads the ANSYS file and constructs a dataset that gets
//   returned later in the GetMesh method.
//
// Arguments:
//   name   : The name of the file to read.
//   nLines : The max number of lines to read from the file.
//
// Returns:    True if the file looks like a ANSYS bulk data file.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 6 16:00:08 PST 2005
//
// Modifications:
//   Brad Whitlock, Wed Jul 27 10:55:58 PDT 2005
//   Fixed for win32.
//
//    Mark C. Miller, Thu Mar 30 16:45:35 PST 2006
//    Made it use VisItStat instead of stat
//
//    Brad Whitlock, Wed May 16 12:02:53 PDT 2012
//    Change how we read the lines so it is more robust.
//
//    Mark C. Miller, Thu Apr 13 16:28:01 PDT 2017
//    Change atof() to strtod() and added some basic error checking of
//    read floating point values. Corrected logic for NBLOCK and EBLOCK
//    parsing to interpret field count after *first* comma and not second.
//    Changed interface to InterpretFormatString to accept field count arg.
// ****************************************************************************

int
get_errno()
{
   int eno = 0;
#ifdef WIN32
    _get_errno(&eno);
#else
    eno = errno;
#endif
    return eno;
}



#define CHECK_COORD_COMPONENT(Coord)                                \
do {                                                                \
    int _errno = get_errno();                                       \
    char msg[512] = "Further warnings will be supressed";           \
    if (_errno != 0 && invalidCoordCompWarning++ < 5)               \
    {                                                               \
        if (invalidCoordCompWarning < 5)                            \
            SNPRINTF(msg, sizeof(msg),"Encountered invalid value "  \
                "\"%s\" (%s) at or near line %d", strerror(_errno), \
                valstart, lineIndex);                               \
        debug1 << msg;                                              \
        TRY                                                         \
        {                                                           \
            if (!avtCallback::IssueWarning(msg))                    \
                cerr << msg << endl;                                \
        }                                                           \
        CATCH(VisItException)                                       \
        {                                                           \
            cerr << msg << endl;                                    \
        }                                                           \
        ENDTRY                                                      \
    }                                                               \
    endptr = 0;                                                     \
} while (0)

bool
avtANSYSFileFormat::ReadFile(const char *name, int nLines)
{
    const char *mName = "avtANSYSFileFormat::ReadFile: ";
    const char *timerName = "Reading ANSYS file";
    int total = visitTimer->StartTimer();
    int invalidCoordCompWarning = 0;
    
    debug4 << mName << endl;

    // Open the file.
    ifstream ifile(name);
    if (ifile.fail())
    {
        visitTimer->StopTimer(total, timerName);
        EXCEPTION1(InvalidFilesException, name);
    }

    // Determine the file size and come up with an estimate of the
    // number of vertices and cells so we can size the points and cells.
    int nPoints = 100;
    int nCells = 100;
    if(nLines == ALL_LINES)
    {
        FileFunctions::VisItStat_t statbuf;
        FileFunctions::VisItStat(name, &statbuf);
        FileFunctions::VisItOff_t fileSize = statbuf.st_size;

        // Make a guess about the number of cells and points based on
        // the size of the file.
        nPoints = fileSize / (FileFunctions::VisItOff_t) 190;
        nCells  = fileSize / (FileFunctions::VisItOff_t) 210;
    }
    vtkPoints *pts = vtkPoints::New();
    pts->Allocate(nPoints);

    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->SetPoints(pts);
    ugrid->Allocate(nCells);
    pts->Delete();

#define MAX_ANSYS_LINE 200
    char  line[MAX_ANSYS_LINE];
    float pt[3];
    vtkIdType   verts[8];
    bool  recognized = false;
    bool  fatalError = false;
    bool  readingCoordinates = false;
    bool  readingConnectivity = false;
    int   expectedLineLength = 0;

    int   firstFieldWidth = 8;
    int   fieldWidth = 16;
    int   fieldStart = 56;

    for(int lineIndex = 0; !ifile.eof(); ++lineIndex)
    {
        if(nLines != ALL_LINES && lineIndex >= nLines)
            break;

        // Get the line
        ifile.getline(line, MAX_ANSYS_LINE);
#if defined(_WIN32)
        int linelen = strlen(line);
#else
        int linelen = strlen(line)-1; // account for the end of line char.
#endif
        // If the line length is less than expected then pad with NULLs.
        if(expectedLineLength > 0 && linelen < expectedLineLength)
        {
             memset(line + linelen + 1, 0, (MAX_ANSYS_LINE - linelen - 1) * sizeof(char));
#if 0
             debug5 << "Padding line with NULLs" << endl;
             debug5 << line << endl;
#endif
        }

        // Give it a chance to break out of coordinate reading.
        if(readingCoordinates)
        {
            bool valid = true;
            for(int i = 0; i < firstFieldWidth && valid; ++i)
                valid &= (line[i] == ' ' || (line[i] >= '0' && line[i] <= '9'));

            if(!valid)
            {                
                expectedLineLength = 0;
                readingCoordinates = false;
                continue;
            }
        }

        if(readingCoordinates)
        {
            char *valstart = line + fieldStart;
            char *valend = valstart + fieldWidth;
            char *endptr = 0;
            pt[2] = strtod(valstart, &endptr);
            CHECK_COORD_COMPONENT(pt[2]);

            valstart -= fieldWidth;
            valend -= fieldWidth;
            *valend = '\0';
            pt[1] = strtod(valstart, &endptr);
            CHECK_COORD_COMPONENT(pt[1]);

            valstart -= fieldWidth;
            valend -= fieldWidth;
            *valend = '\0';
            pt[0] = strtod(valstart, &endptr);
            CHECK_COORD_COMPONENT(pt[0]);
#if 0
            debug4 << pt[0] << ", " << pt[1] << ", " << pt[2] << endl;
#endif
            pts->InsertNextPoint(pt);
        }
        else if(readingConnectivity)
        {
            // Get whether this cell is real from column 0
            line[fieldWidth] = '\0';
            bool realCell = atoi(line) > 0;
            if(!realCell)
            {
                expectedLineLength = 0;
                readingConnectivity = false;
                continue;
            }

            // Get the number of vertices in this cell from column 9.
            static const int ncellsColumn = 9;
            line[ncellsColumn * fieldWidth] = '\0';
            int nverts = atoi(line + (ncellsColumn-1) * fieldWidth);

            if(nverts == 8)
            {
                char *valstart = line + fieldStart;
                char *valend   = valstart + fieldWidth;
                for(int i = 0; i < 8; ++i)
                {
                    int ivalue = atoi(valstart);
                    verts[7-i] = (ivalue > 0) ? (ivalue - 1) : ivalue;
                    valstart -= fieldWidth;
                    valend   -= fieldWidth;
                    *valend = '\0';
                }

#if 0
                for(int j = 0; j < 8; ++j)
                    debug4 << ", " << verts[j];
                debug4 << endl;
#endif
                ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, verts);
            }
            else
            {
                debug1 << mName << "The file " << name << " contained cells "
                       "that are not hexes."<< endl;

                fatalError = true;
                break;
            }
        }
        else if(STRNCASECMP(line, "NBLOCK", 6) == 0)
        {
            int numFields = 6;
            char *comma = strstr(line, ",");
            if(comma != 0)
            {
                char *cols = comma + 1;
                numFields = atoi(cols);
                debug4 << mName << "Coordinate data stored in "
                       << numFields << " columns." << endl;
                char *comma2 = strstr(comma+1, ",");
                if(comma2 != 0)
                {
                    *comma2 = '\0';
                    recognized = true;
                }
                recognized = true;
            }

            // Get the field format string. Use it to set expectedLineLength,
            // fieldWidth, and fieldStart.
            ifile.getline(line, 1024);
            if(line[0] == '(')
            {
                InterpretFormatString(line, numFields, firstFieldWidth, fieldStart, fieldWidth,
                                      expectedLineLength);
                debug4 << mName << "firstFieldWidth=" << firstFieldWidth
                       << ", fieldStart=" << fieldStart
                       << ", fieldWidth=" << fieldWidth 
                       << ", expectedLineLength=" << expectedLineLength
                       << endl;
                readingCoordinates = true;
            }
            else
            {
                debug1 << mName << "Malformed format string: " << line << endl;
                fatalError = true;
            }
        }
        else if(STRNCASECMP(line, "EBLOCK", 6) == 0)
        {
            int numFields;
            char *comma = strstr(line, ",");
            if(comma != 0)
            {
                char *cols = comma + 1;
                numFields = atoi(cols);
                debug4 << mName << "Connectivity data stored in "
                       << numFields << " columns." << endl;
                char *comma2 = strstr(comma+1, ",");
                if(comma2 != 0)
                {
                    *comma2 = '\0';
                    recognized = true;
                }
                recognized = true;
            }

            // Get the field format string. Use it to set expectedLineLength,
            // fieldWidth, and fieldStart.
            ifile.getline(line, 1024);
            if(line[0] == '(')
            {
                InterpretFormatString(line, numFields, firstFieldWidth, fieldStart, fieldWidth,
                                      expectedLineLength);
                debug4 << mName << "firstFieldWidth=" << firstFieldWidth
                       << ", fieldStart=" << fieldStart
                       << ", fieldWidth=" << fieldWidth
                       << ", expectedLineLength=" << expectedLineLength 
                       << endl; 
                readingConnectivity = true;
            }
            else
            {
                debug1 << mName << "Malformed format string: " << line << endl;
                fatalError = true;
            }
        }
        else if(STRNCASECMP(line, "/COM", 4) == 0)
        {
            if(title == "")
                title = std::string(line+6);
            recognized = true;
        }
        else if(STRNCASECMP(line, "/TITLE", 6) == 0)
        {
            title = std::string(line+8);
            recognized = true;
        }
        else if(STRNCASECMP(line, "/BATCH",  6) == 0 ||
                STRNCASECMP(line, "/NOPR",   5) == 0 ||
                STRNCASECMP(line, "/CONFIG", 7) == 0 ||
                STRNCASECMP(line, "/NOLIST", 7) == 0)
        {
            recognized = true;
        }
        else
        {
            expectedLineLength = 0;
            readingCoordinates = false;
            readingConnectivity = false;
        }
    }

    if(recognized && nLines == ALL_LINES && !fatalError)
        meshDS = ugrid;
    else
        ugrid->Delete();

    visitTimer->StopTimer(total, timerName);

    return recognized;
}

// ****************************************************************************
// Method: avtANSYSFileFormat::Interpret
//
// Purpose: 
//   Interprets the format string to deduce the field width and the line length
//   prescribed by the format string.
//
// Arguments:
//   fmt        : The format string.
//   fieldWidth : The width of an individual field.
//   linelen    : The length of the fields in the format string.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 7 09:55:59 PDT 2005
//
// Modifications:
//   
//    Mark C. Miller, Thu Apr 13 16:29:45 PDT 2017
//
//    Used this reference...
//        http://www.ansys.stuba.sk/html/prog_55/g-int/INS3.htm
//
//    Added logic to accept and track total field count and counts of integer
//    'i' and floating point, 'e' fields. ANSYS files and ANSYS format
//    documentation has some conflicting information regarding these format
//    strings. Lets try to clarify that information here. Here is an example
//    of an NBLOCK segment defining a block of nodes of an ANSYS file...
//
//        NBLOCK ,6,SOLID
//        (3i8,6e16.9)
//        1       0       0-8.499200642E-02-8.218053728E-02-5.578922108E-02
//        2       0       0-8.494276553E-02-7.795915008E-02-5.406552181E-02
//        3       0       0-1.196498796E-01-7.090078294E-02-2.062848397E-02
//        ...
//        
//    The integer after the *first* comma on the NBLOCK line is a count of the
//    number of fields (columns) in the data. The format line, (3i8,6e16.9)
//    defines the data type and field width of those fields. Here, the NBLOCK
//    line tells us there are 6 fields. The parentha-format-line tells us that
//    there are '3' columns of integer ('i') values each '8' digits in length
//    followed by '6' columns of floating point ('e') values, '16' characters
//    in length with '9' digits after the decimal point. Typically, the first
//    3 integer fields define the node #, solid #, line # of the node and next
//    3 fields define the floating point coordinates of the node. But wait,
//    there are clearly only 3 columns of floating point data here. Why is there
//    a '6' before the 'e'? Shouldn't that be a '3' there? IMHO, yes. However,
//    I believe this is a proliferant bug in many ANSYS file instances. There
//    *are* cases where there may be 6 floating point fields, 3 for
//    coordinates as we have here plus 3 for a "rotational vector". In that
//    case, however, I believe the proper specification should look something
//    like...
//
//       NBLOCK,9
//       (1i8,6e10.3)
//             1-8.499E-02-8.218E-02-5.5788E-02 1.000E+00 0.000E+00 0.000E+00
//             2-8.942E-02-7.795E-02-5.4061E-02 8.931E-01 2.011E-02 0.000E+00
//
//    where I have artificially shrunk the floating point field width to a
//    number small enough, 10, that we can fit 6 fields in the example here.
//    However, the ANSYS file format documentation for NBLOCK directive is
//    erroneous here too because it says that fields 7-9 will exist *only* if 
//    NBLOCK specifies a number of fields > 3. Thats incorrect. fields 7-9
//    should exist, IMHO, only if NBLOCK specifies > 6 fields (e.g. 9).
//    However, the ANSYS documentation goes on to say that the format string
//    will *always* be 3i8,6e16.9 and that is what has lead to a situation
//    where most ANSYS files have an NBLOCK directive that disagrees with
//    the format string in field counts.
//
//    Summary:

//    1) Fields are back-to-back with no guarantee of spaces separating fields.
//    2) The field count specified in NBLOCK doesn't agree with the sum of 'i'
//       and 'e' field counts from the format string.
//    3) Instead of *always* 3 integer (e.g. '3i8') fields with whitespace
//       being used for optional solid # and line # fields, a field count of
//       '1' is specified (e.g '1i8')
//    
//    I adjusted logic here to use the "old" way if numFields read in BLOCK
//    directive is indeed 6 but otherwise treat the file as though the
//    field count in NBLOCK directive *should*be* the sum of 'i' ane 'e'
//    fields in the format string and, if it is, assume the data producer
//    knew what it as doing and treat the data as it specifies.
//        
// ****************************************************************************

void
avtANSYSFileFormat::Interpret(const char *fmt, bool isstd, int &numFields,
    int &fieldWidth, int &linelen) const
{
    int i0, i1, i2;
    bool goodFormat = true;

    debug4 << "avtANSYSFileFormat::Interpret: " << fmt << endl;

    // Example: 6e16.9
    if(sscanf(fmt, "%de%d.%d", &i0, &i1, &i2) == 3)
    {
        if (isstd) // std (buggy) ANSYS format string
        {
            //linelen = i0 * i1 / 2;
            linelen = 3 * i1;
            fieldWidth = i1;
        }
        else if (numFields == i0)
        {
            linelen = i0 * (i1+1);
            fieldWidth = i1;
        }
        else
        {
            goodFormat = false;
        }
    }
    // Example: 19i7
    else if(sscanf(fmt, "%di%d", &i0, &i1) == 2)
    {
        linelen = i0 * i1;
        fieldWidth = i1;
        if (!isstd) // std (buggy) ANSYS format string
            numFields -= i0;
    }
    else
    {
        goodFormat = false;
    }

    if (!goodFormat)
    {
        char msg[128];
        SNPRINTF(msg, sizeof(msg), "Invalid field formatting string: "
            "numFields=%d, fmt=\"%s\"", numFields, fmt);
        EXCEPTION1(InvalidFilesException, msg);
    }
}

// ****************************************************************************
// Method: avtANSYSFileFormat::InterpretFormatString
//
// Purpose: 
//   Interprets a format string to get fieldStart, fieldWidth, and the expected
//   line length that the format string wants.
//
// Arguments:
//   line               : The line containing the format string.
//   firstFieldWidth    : The width of the first field.
//   fieldStart         : The starting location of the first field that we
//                        want to read.
//   fieldWidth         : The length of the field that we want to read.
//   expectedLineLength : The length of the lines that should be considered
//                        for parsing as coords or connectivity.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 7 09:57:23 PDT 2005
//
// Modifications:
//   Brad Whitlock, Wed May 16 13:36:03 PDT 2012
//   Pass out the width of the first field.
//
// ****************************************************************************

void
avtANSYSFileFormat::InterpretFormatString(char *line, int numFields,
    int &firstFieldWidth, int &fieldStart, int &fieldWidth,
    int &expectedLineLength) const
{
    char *fmt = line + 1;
    char *ptr = 0;

    expectedLineLength = 0;
    bool keepGoing = true;
    bool first = true;
    bool isStdNBLOCKFmt = numFields==6 && STRNCASECMP(line, "(3i8,6e16.9)", 12)==0;
    while(keepGoing)
    {
        int linelen = 0;

        if((ptr = strstr(fmt, ",")) != 0)
        {
            *ptr = '\0';
            Interpret(fmt, isStdNBLOCKFmt, numFields, fieldWidth, linelen);
            if(first)
            {
                first = false;
                firstFieldWidth = fieldWidth;
            }
            expectedLineLength += linelen;
            fmt = ptr + 1;
        }
        else if((ptr = strstr(fmt, ")")) != 0)
        {
            *ptr = '\0';
            Interpret(fmt, isStdNBLOCKFmt, numFields, fieldWidth, linelen);
            if(first)
            {
                first = false;
                firstFieldWidth = fieldWidth;
            }
            expectedLineLength += linelen;
            keepGoing = false;
        }
        else
            keepGoing = false;
    }

    fieldStart = expectedLineLength - fieldWidth;
}

// ****************************************************************************
//  Method: avtANSYSFileFormat::PopulateDatabaseMetaData
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
avtANSYSFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    debug4 << "avtANSYSFileFormat::PopulateDatabaseMetaData" << endl;
    md->SetDatabaseComment(title);
    AddMeshToMetaData(md, "mesh", AVT_UNSTRUCTURED_MESH, NULL, 1, 1,
                      3, 3);
}


// ****************************************************************************
//  Method: avtANSYSFileFormat::GetMesh
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
avtANSYSFileFormat::GetMesh(const char *meshname)
{
    debug4 << "avtANSYSFileFormat::GetMesh" << endl;
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->ShallowCopy(meshDS);

    return ugrid;
}


// ****************************************************************************
//  Method: avtANSYSFileFormat::GetVar
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
avtANSYSFileFormat::GetVar(const char *varname)
{
    debug4 << "avtANSYSFileFormat::GetVar" << endl;

    // Can't read variables yet.
    EXCEPTION1(InvalidVariableException, varname);
}


// ****************************************************************************
//  Method: avtANSYSFileFormat::GetVectorVar
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
avtANSYSFileFormat::GetVectorVar(const char *varname)
{
    debug4 << "avtANSYSFileFormat::GetVectorVar" << endl;

    // Can't read variables yet.
    EXCEPTION1(InvalidVariableException, varname);
}


