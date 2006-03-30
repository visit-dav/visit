// ************************************************************************* //
//                            avtANSYSFileFormat.C                           //
// ************************************************************************* //

#include <avtANSYSFileFormat.h>

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

#include <avtDatabaseMetaData.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>

#include <TimingsManager.h>
#include <DebugStream.h>
#include <Utility.h>

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
// ****************************************************************************

bool
avtANSYSFileFormat::ReadFile(const char *name, int nLines)
{
    const char *mName = "avtANSYSFileFormat::ReadFile: ";
    const char *timerName = "Reading ANSYS file";
    int total = visitTimer->StartTimer();
    
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
        VisItStat_t statbuf;
        VisItStat(name, &statbuf);
        VisItOff_t fileSize = statbuf.st_size;

        // Make a guess about the number of cells and points based on
        // the size of the file.
        nPoints = fileSize / (VisItOff_t) 190;
        nCells  = fileSize / (VisItOff_t) 210;
    }
    vtkPoints *pts = vtkPoints::New();
    pts->Allocate(nPoints);

    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->SetPoints(pts);
    ugrid->Allocate(nCells);
    pts->Delete();

    char  line[1024];
    float pt[3];
    int   verts[8];
    bool  recognized = false;
    bool  fatalError = false;
    bool  readingCoordinates = false;
    bool  readingConnectivity = false;
    int   expectedLineLength = 0;

    int   fieldWidth = 16;
    int   fieldStart = 56;

    for(int lineIndex = 0; !ifile.eof(); ++lineIndex)
    {
        if(nLines != ALL_LINES && lineIndex >= nLines)
            break;

        // Get the line
        ifile.getline(line, 1024);
#if defined(_WIN32)
        int linelen = strlen(line);
#else
        int linelen = strlen(line)-1; // account for the end of line char.
#endif

        // Determine what the line is for.
        bool rightLength = (linelen == expectedLineLength);
        readingCoordinates  = readingCoordinates && rightLength;
        readingConnectivity = readingConnectivity && rightLength;

        if(readingCoordinates)
        {
            char *valstart = line + fieldStart;
            char *valend = valstart + fieldWidth;
            pt[2] = atof(valstart);

            valstart -= fieldWidth;
            valend -= fieldWidth;
            *valend = '\0';
            pt[1] = atof(valstart);

            valstart -= fieldWidth;
            valend -= fieldWidth;
            *valend = '\0';
            pt[0] = atof(valstart);

#if 0
            debug4 << pt[0] << ", " << pt[1] << ", " << pt[2] << endl;
#endif
            pts->InsertNextPoint(pt);
        }
        else if(readingConnectivity)
        {
            char tmp = line[fieldWidth];
            line[fieldWidth] = '\0';
            int nverts = atoi(line);
            line[fieldWidth] = tmp;
            if(nverts == 8)
            {
                char *valstart = line + fieldStart;
                char *valend   = valstart + fieldWidth;
                for(int i = 0; i < 8; ++i)
                {
                    verts[7-i] = atoi(valstart)-1;
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
            char *comma = strstr(line, ",");
            if(comma != 0)
            {
                char *comma2 = strstr(comma+1, ",");
                if(comma2 != 0)
                {
                    *comma2 = '\0';
                    char *cols = comma + 1;
                    debug4 << mName << "Coordinate data stored in "
                           << atoi(cols) << " columns." << endl;
                    recognized = true;
                }
            }

            // Get the field format string. Use it to set expectedLineLength,
            // fieldWidth, and fieldStart.
            ifile.getline(line, 1024);
            if(line[0] == '(')
            {
                InterpretFormatString(line, fieldStart, fieldWidth,
                                      expectedLineLength);
                debug4 << mName << "fieldStart=" << fieldStart
                       << ", fieldWidth=" << fieldWidth 
                       << ",expectedLineLength=" << expectedLineLength
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
            char *comma = strstr(line, ",");
            if(comma != 0)
            {
                char *comma2 = strstr(comma+1, ",");
                if(comma2 != 0)
                {
                    *comma2 = '\0';
                    char *cols = comma + 1;
                    debug4 << mName << "Connectivity data stored in "
                           << atoi(cols) << " columns." << endl;
                    recognized = true;
                }
            }

            // Get the field format string. Use it to set expectedLineLength,
            // fieldWidth, and fieldStart.
            ifile.getline(line, 1024);
            if(line[0] == '(')
            {
                InterpretFormatString(line, fieldStart, fieldWidth,
                                      expectedLineLength);
                debug4 << mName << "fieldStart=" << fieldStart
                       << ", fieldWidth=" << fieldWidth
                       << ",expectedLineLength=" << expectedLineLength 
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
// ****************************************************************************

void
avtANSYSFileFormat::Interpret(const char *fmt, int &fieldWidth,
    int &linelen) const
{
    int i0, i1, i2;

    debug4 << "avtANSYSFileFormat::Interpret: " << fmt << endl;

    // Example: 6e16.9
    if(sscanf(fmt, "%de%d.%d", &i0, &i1, &i2) == 3)
    {
        linelen = i0 * i1 / 2;
        fieldWidth = i1;
    }
    // Example: 19i7
    else if(sscanf(fmt, "%di%d", &i0, &i1) == 2)
    {
        linelen = i0 * i1;
        fieldWidth = i1;
    }
    else
    {
        debug1 << "Invalid format string: " << fmt << endl;
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
//   
// ****************************************************************************

void
avtANSYSFileFormat::InterpretFormatString(char *line, int &fieldStart,
    int &fieldWidth, int &expectedLineLength) const
{
    char *fmt = line + 1;
    char *ptr = 0;

    expectedLineLength = 0;
    bool keepGoing = true;
    while(keepGoing)
    {
        int linelen = 0;

        if((ptr = strstr(fmt, ",")) != 0)
        {
            *ptr = '\0';
            Interpret(fmt, fieldWidth, linelen);
            expectedLineLength += linelen;
            fmt = ptr + 1;
        }
        else if((ptr = strstr(fmt, ")")) != 0)
        {
            *ptr = '\0';
            Interpret(fmt, fieldWidth, linelen);
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


