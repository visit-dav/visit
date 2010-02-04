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
//                            avtShapefileFileFormat.C                       //
// ************************************************************************* //

#include <avtShapefileFileFormat.h>

#include <string>

#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>

#include <DBOptionsAttributes.h>
#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <Expression.h>
#include <ExpressionList.h>
#include <DebugStream.h>
#include <stdlib.h>


// ****************************************************************************
//  Method: avtShapefileFileFormat constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Mar 24 12:18:02 PDT 2005
//
//  Modifications:
//    Brad Whitlock, Wed Feb 28 16:02:49 PST 2007
//    Added VISIT_SHAPEFILE_POLYGONS_AS_LINES environment variable.
//
//    Brad Whitlock, Tue Mar 6 18:02:28 PST 2007
//    Added VISIT_SHAPEFILE_NO_TESSELLATION.
//
// ****************************************************************************

avtShapefileFileFormat::avtShapefileFileFormat(const char *filename,
    const DBOptionsAttributes *rdopts)
    : avtSTSDFileFormat(filename), shapes()
{
    initialized = false;
    numShapeTypes = 0;
    dbfFile = 0;
    polygonsAsLines = false;
    tessellatePolygons = true;
    esriLogging = false;
    dbfLogging = false;

    //
    // Process any read options
    //
    for (int i = 0; rdopts && i < rdopts->GetNumberOfOptions(); i++)
    {
        if (rdopts->GetName(i) == "Polygons as lines")
            polygonsAsLines = rdopts->GetBool("Polygons as lines");
        else if (rdopts->GetName(i) == "Tessellate polygons")
            tessellatePolygons = rdopts->GetBool("Tessellate polygons");
        else if (rdopts->GetName(i) == "ESRI Logging")
            esriLogging = rdopts->GetBool("ESRI Logging");
        else if (rdopts->GetName(i) == "DBF Logging")
            dbfLogging = rdopts->GetBool("DBF Logging");
        else
            debug1 << "Ignoring unknown option \"" << rdopts->GetName(i) << "\"" << endl;
    }
}

// ****************************************************************************
// Method: avtShapefileFileFormat::~avtShapefileFileFormat
//
// Purpose: 
//   Destructor for the avtShapefileFileFormat class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 25 13:52:45 PST 2005
//
// Modifications:
//   
// ****************************************************************************

avtShapefileFileFormat::~avtShapefileFileFormat()
{
    FreeUpResources();
}

// ****************************************************************************
//  Method: avtShapefileFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Mar 24 12:18:02 PDT 2005
//
// ****************************************************************************

void
avtShapefileFileFormat::FreeUpResources(void)
{
    debug4 << "avtShapefileFileFormat::FreeUpResources: this=" << (void*)this
           << endl;

    // Free all of the shapes objects.
    for(int i = 0; i < shapes.size(); ++i)
        esriFreeShape(shapes[i].shapeType, shapes[i].shape);
    shapes.clear();

    // If we have a DBF file open, close it now.
    if(dbfFile != 0)
    {
        dbfFileClose(dbfFile);
        dbfFile = 0;
        dbfFinalize();
    }

    initialized = false;
}

// ****************************************************************************
// Method: avtShapefileFileFormat::ActivateTimestep
//
// Purpose: 
//   Initializes the file format reader.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 25 13:53:23 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtShapefileFileFormat::ActivateTimestep(void)
{
    debug4 << "avtShapefileFileFormat::ActivateTimestep: this=" << (void*)this
           << endl;
    Initialize();
}

// ****************************************************************************
// Method: avtShapefileFileFormat::Initialize
//
// Purpose: 
//   Reads the shape file and populates the shapes vector with the records
//   read from the file.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 25 13:53:50 PST 2005
//
// Modifications:
//   Brad Whitlock, Tue Mar 29 16:08:26 PST 2005
//   Added an error function for ESRI so it can write messages to the
//   VisIt debug logs.
//
//   Brad Whitlock, Wed Apr 6 10:48:52 PDT 2005
//   Changed dbfInitialize interface.
//
//   Brad Whitlock, Tue Feb 27 11:46:22 PDT 2007
//   Added call to CountShapeTypes to set the numShapeTypes member.
//
//   Brad Whitlock, Tue Mar 6 18:03:15 PST 2007
//   Added nRepeats.
//
// ****************************************************************************

static void
add_esri_errors_to_log(const char *msg)
{
    debug4 << "esriError: " << msg << endl;
}

void
avtShapefileFileFormat::Initialize()
{
    static const char *mName = "avtShapefileFileFormat::Initialize: ";
    if(!initialized)
    {
        esriShapefile_t *f = 0;
        esriFileError_t code;

        debug4 << mName << "this=" << (void*)this << endl;

        // Look at the filename string and chop off the extension.
        std::string SHPfile, DBFfile, fName(filename);
        int extPos = fName.rfind(".");
        if(extPos != -1)
        {
            std::string fNameNoExt(fName.substr(0, extPos));
            SHPfile = fNameNoExt + ".shp";
            DBFfile = fNameNoExt + ".dbf";
        }
        else
        {
            SHPfile = fName;
            // There were no extensions. Assume no DBF file.
        }
        debug4 << mName << "SHPfile = " << SHPfile.c_str() << endl;
        debug4 << mName << "DBFfile = " << DBFfile.c_str() << endl;

        // Initialize the ESRI library.
        debug4 << mName << "Reading SHP file using esri API." << endl;
        esriInitialize(1, add_esri_errors_to_log, esriLogging);

        // Try and open the file.
        f = esriShapefileOpen(SHPfile.c_str(), &code);
        if(code == esriFileErrorInvalidFile)
        {
            EXCEPTION1(InvalidFilesException, filename);
        }
        else if(code == esriFileErrorNotFound)
        {
            EXCEPTION1(InvalidFilesException, filename);
        }

        // Process the file.
        bool keepReading = true;
        do
        {
            esriShape newShape;
            void *s = 0;

            // Read the record header.
            esriShapefileRecordHeader_t header;
            esriReadError_t rcErr;
            esriShapefileReadRecordHeader(f, &header, &rcErr);

            if(rcErr == esriReadErrorSuccess)
            {
                /*fprintf(stderr, "Record %d: ", header.recordNumber);*/
                switch(header.shapeType)
                {
                case esriNullShape:
                    break;
                case esriPoint:
                    s = esriMalloc(sizeof(esriPoint_t));
                    esriShapefileReadPoint(&header, (esriPoint_t *)s, &rcErr);
                    break;
                case esriPolyLine:
                    s = esriMalloc(sizeof(esriPolyLine_t));
                    esriShapefileReadPolyLine(&header, (esriPolyLine_t *)s, &rcErr);
                    break;
                case esriPolygon:
                    s = esriMalloc(sizeof(esriPolygon_t));
                    esriShapefileReadPolygon(&header, (esriPolygon_t *)s, &rcErr);
                    break;
                case esriMultiPoint:
                    s = esriMalloc(sizeof(esriMultiPoint_t));
                    esriShapefileReadMultiPoint(&header, (esriMultiPoint_t *)s, &rcErr);
                    break;
                case esriPointZ:
                    s = esriMalloc(sizeof(esriPointZ_t));
                    esriShapefileReadPointZ(&header, (esriPointZ_t *)s, &rcErr);
                    break;
                case esriPolyLineZ:
                    s = esriMalloc(sizeof(esriPolyLineZ_t));
                    esriShapefileReadPolyLineZ(&header, (esriPolyLineZ_t *)s, &rcErr);
                    break;
                case esriPolygonZ:
                    s = esriMalloc(sizeof(esriPolygonZ_t));
                    esriShapefileReadPolygonZ(&header, (esriPolygonZ_t *)s, &rcErr);
                    break;
                case esriMultiPointZ:
                    s = esriMalloc(sizeof(esriMultiPointZ_t));
                    esriShapefileReadMultiPointZ(&header, (esriMultiPointZ_t *)s, &rcErr);
                    break;
                case esriPointM:
                    s = esriMalloc(sizeof(esriPointM_t));
                    esriShapefileReadPointM(&header, (esriPointM_t *)s, &rcErr);
                    break;
                case esriPolyLineM:
                    s = esriMalloc(sizeof(esriPolyLineM_t));
                    esriShapefileReadPolyLineM(&header, (esriPolyLineM_t *)s, &rcErr);
                    break;
                case esriPolygonM:
                    s = esriMalloc(sizeof(esriPolygonM_t));
                    esriShapefileReadPolygonM(&header, (esriPolygonM_t *)s, &rcErr);
                    break;
                case esriMultiPointM:
                    s = esriMalloc(sizeof(esriMultiPointM_t));
                    esriShapefileReadMultiPointM(&header, (esriMultiPointM_t *)s, &rcErr);
                    break;
                case esriMultiPatch:
                    s = esriMalloc(sizeof(esriMultiPatch_t));
                    esriShapefileReadMultiPatch(&header, (esriMultiPatch_t *)s, &rcErr);
                    break;
                default:
                    debug4 << "Skipping shapeType "
                           << (int)header.shapeType
                           << ". Size="
                           << header.contentLength
                           << " bytes.\n",
                    /* Skip the record. */
                    fseek(f->fp, (long)header.contentLength, SEEK_CUR);
                }

                keepReading = rcErr == esriReadErrorSuccess;
                if(s != 0)
                {
                    if(keepReading)
                    {
                        newShape.shapeType = header.shapeType;
                        newShape.shape = s;
                        newShape.nRepeats = 1;
                        shapes.push_back(newShape);
                    }
                    else
                        esriFreeShape(header.shapeType, s);
                }
            }
            else
                keepReading = false;
        } while(keepReading);


        esriShapefileClose(f);
        esriFinalize();

        debug4 << mName << "VisIt read in " << shapes.size() << " shape records." << endl;

        /* Try and read in the header from the DBF file. */
        if(DBFfile.size() > 0)
        {
            dbfFileError_t code2;

            dbfInitialize(0, 0, dbfLogging);

            // Try and open the file.
            dbfFile = dbfFileOpen(DBFfile.c_str(), &code2);
            if(code2 != dbfFileErrorSuccess)
            {
                debug4 << mName << "Could not open DBF file: "
                       << DBFfile.c_str() << endl;
            }
            else
            {
                debug4 << mName << "Opened the DBF file. Fields = {";
                for(int i = 0; i < dbfFile->header.numFieldDescriptors; ++i)
                    debug4 << ", " << dbfFile->header.fieldDescriptors[i].fieldName;
                debug4 << "}" << endl;
            }
        }

        // Now that the file has been read, count the number of shapes.
        numShapeTypes = CountShapeTypes();

        initialized = true;
    }
}

// ****************************************************************************
// Method: avtShapefileFileFormat::CountMemberPoints
//
// Purpose: 
//   Counts the number of points for a specified shape type.
//
// Arguments:
//   shapeType : The type of shape for which we want the number of points.
//
// Returns:    The number of points.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 25 13:48:18 PST 2005
//
// Modifications:
//   
// ****************************************************************************

int
avtShapefileFileFormat::CountMemberPoints(esriShapeType_t shapeType) const
{
    int npts = 0;
    for(int i = 0; i < shapes.size(); ++i)
    {
        if(shapes[i].shapeType == shapeType)
        {
            switch(shapes[i].shapeType)
            {
            case esriNullShape:
                break;
            case esriPoint:
                ++npts;
                break;
            case esriPolyLine:
                npts += ((esriPolyLine_t *)shapes[i].shape)->numPoints;
                break;
            case esriPolygon:
                npts += (((esriPolygon_t *)shapes[i].shape)->numPoints -
                         ((esriPolygon_t *)shapes[i].shape)->numParts);
                break;
            case esriMultiPoint:
                npts += ((esriMultiPoint_t *)shapes[i].shape)->numPoints;
                break;
            case esriPointZ:
                ++npts;
                break;
            case esriPolyLineZ:
                npts += ((esriPolyLineZ_t *)shapes[i].shape)->numPoints;
                break;
            case esriPolygonZ:
                npts += (((esriPolygonZ_t *)shapes[i].shape)->numPoints -
                         ((esriPolygonZ_t *)shapes[i].shape)->numParts);
                break;
            case esriMultiPointZ:
                npts += ((esriMultiPointZ_t *)shapes[i].shape)->numPoints;
                break;
            case esriPointM:
                ++npts;
                break;
            case esriPolyLineM:
                npts += ((esriPolyLineM_t *)shapes[i].shape)->numPoints;
                break;
            case esriPolygonM:
                npts += (((esriPolygonM_t *)shapes[i].shape)->numPoints -
                         ((esriPolygonM_t *)shapes[i].shape)->numParts);
                break;
            case esriMultiPointM:
                npts += ((esriMultiPointM_t *)shapes[i].shape)->numPoints;
                break;
            case esriMultiPatch:
                npts += ((esriMultiPatch_t *)shapes[i].shape)->numPoints;
                break;
            default:
                debug4 << "Unknown type!" << endl;
            }
        }
    }

    return npts;
}

// ****************************************************************************
// Method: avtShapefileFileFormat::CountShapeTypes
//
// Purpose: 
//   Counts the number of shape types that were read from the file.
//
// Returns:    Returns the number of shape types in the file.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 27 11:11:07 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

int
avtShapefileFileFormat::CountShapeTypes() const
{

    int i, ret = 0, shapeCounts[15];
    for(i = 0; i < 15; ++i)
        shapeCounts[i] = 0;

    // Count the number of instances of each shape.
    for(i = 0; i < shapes.size(); ++i)
    {
        switch(shapes[i].shapeType)
        {
        case esriNullShape:
            shapeCounts[0]++;
            break;
        case esriPoint:
            shapeCounts[1]++;
            break;
        case esriPolyLine:
            shapeCounts[2]++;
            break;
        case esriPolygon:
            shapeCounts[3]++;
            break;
        case esriMultiPoint:
            shapeCounts[4]++;
            break;
        case esriPointZ:
            shapeCounts[5]++;
            break;
        case esriPolyLineZ:
            shapeCounts[6]++;
            break;
        case esriPolygonZ:
            shapeCounts[7]++;
            break;
        case esriMultiPointZ:
            shapeCounts[9]++;
            break;
        case esriPointM:
            shapeCounts[10]++;
            break;
        case esriPolyLineM:
            shapeCounts[11]++;
            break;
        case esriPolygonM:
            shapeCounts[12]++;
            break;
        case esriMultiPointM:
            shapeCounts[13]++;
            break;
        case esriMultiPatch:
            shapeCounts[14]++;
            break;
        default:
            debug4 << "Unknown type!" << endl;
        }
    }

    // Sum up the number of shape types.
    for(i = 0; i < 15; ++i)
        ret += ((shapeCounts[i] > 0) ? 1 : 0);

    return ret;
}

// ****************************************************************************
// Method: avtShapefileFileFormat::CountShapes
//
// Purpose: 
//   Returns the number of shapes of a given type.
//
// Arguments:
//   shapeType : The shape type that we're counting.
//
// Returns:    The number of shapes of the specified type.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 28 01:38:16 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

int
avtShapefileFileFormat::CountShapes(esriShapeType_t shapeType) const
{
    int nShapes = 0;
    for(int i = 0; i < shapes.size(); ++i)
    {
        if(shapes[i].shapeType == shapeType)
        {
            ++nShapes;
        }
    }

    return nShapes;
}

// ****************************************************************************
// Method: avtShapefileFileFormat::CountCellsForShape
//
// Purpose: 
//   Returns the number of cells for a specified shape type. This can differ
//   from the number of shape records because shapes with multiple parts are
//   broken up into multiple cells in the VTK dataset that we create later.
//
// Arguments:
//   shapeType : The shape type that we're counting.
//
// Returns:    The number of cells for the specified shape type.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 28 02:59:42 PDT 2005
//
// Modifications:
//   Brad Whitlock, Tue Mar 6 18:04:52 PST 2007
//   Added polygon tessellation.
//
// ****************************************************************************

int
avtShapefileFileFormat::CountCellsForShape(esriShapeType_t shapeType) const
{
    int nCells = 0;
    for(int i = 0; i < shapes.size(); ++i)
    {
        if(shapes[i].shapeType == shapeType)
        {
            switch(shapeType)
            {
            case esriPolyLine:
                nCells += ((esriPolyLine_t *)shapes[i].shape)->numParts;
                break;
            case esriPolygon:
                if(tessellatePolygons)
                    // We treat polygons specially since they could have been
                    // split up due to tesselation.
                    nCells += shapes[i].nRepeats;
                else
                    nCells += ((esriPolygon_t *)shapes[i].shape)->numParts;
                break;
            case esriMultiPoint:
                nCells += ((esriMultiPoint_t *)shapes[i].shape)->numPoints;
                break;
            case esriPolyLineZ:
                nCells += ((esriPolyLineZ_t *)shapes[i].shape)->numParts;
                break;
            case esriPolygonZ:
                nCells += ((esriPolygonZ_t *)shapes[i].shape)->numParts;
                break;
            case esriMultiPointZ:
                nCells += ((esriMultiPointZ_t *)shapes[i].shape)->numPoints;
                break;
            case esriPolyLineM:
                nCells += ((esriPolyLineM_t *)shapes[i].shape)->numParts;
                break;
            case esriPolygonM:
                nCells += ((esriPolygonM_t *)shapes[i].shape)->numParts;
                break;
            case esriMultiPointM:
                nCells += ((esriMultiPointM_t *)shapes[i].shape)->numPoints;
                break;
            case esriMultiPatch:
                nCells += ((esriMultiPatch_t *)shapes[i].shape)->numParts;
                break;
            default:
                ++nCells;
                break;
            }
        }
    }

    return nCells;
}

// ****************************************************************************
//  Method: avtShapefileFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Mar 24 12:18:02 PDT 2005
//
//  Modifications:
//    Brad Whitlock, Wed Mar 30 15:43:26 PST 2005
//    Added code to serve up x,y,z expressions.
//
//    Brad Whitlock, Tue Feb 27 11:13:08 PDT 2007
//    Count the shape types elsewhere.
//
//    Brad Whitlock, Tue Mar 6 18:06:02 PST 2007
//    Added tessellated mesh that the variables will use.
//
//    Brad Whitlock, Wed Mar 14 20:24:22 PST 2007
//    Made sure that we're initialized in case the order of the calls is
//    not what we expect.
//
// ****************************************************************************

void
avtShapefileFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    const char *mName = "avtShapefileFileFormat::PopulateDatabaseMetaData: ";

    // In case FreeUpResources has been called, make sure that we're initialized.
    Initialize();

    //
    // 2D, no data
    //
    std::string  meshname;
    stringVector meshes, tessmeshes;
    intVector    is3D;
    int polygonTopDim = polygonsAsLines ? 1 : 2;

    int npts = 0;
    if((npts = CountMemberPoints(esriPoint)) > 0)
    {
        meshname = "point";
        debug4 << npts << " points for esriPoint." << endl;
        AddMeshToMetaData(md, meshname, AVT_POINT_MESH, NULL, 1, 0, 2, 0);
        meshes.push_back(meshname);
        tessmeshes.push_back(meshname);
        is3D.push_back(0);
    }

    if((npts = CountMemberPoints(esriPolyLine)) > 0)
    {
        meshname = "polyline";
        debug4 << npts << " points for esriPolyLine." << endl;
        AddMeshToMetaData(md, meshname, AVT_UNSTRUCTURED_MESH, NULL, 1, 0, 2, 1);
        meshes.push_back(meshname);
        tessmeshes.push_back(meshname);
        is3D.push_back(0);
    }

    if((npts = CountMemberPoints(esriPolygon)) > 0)
    {
        meshname = "polygon";
        debug4 << npts << " points for esriPolygon." << endl;
        AddMeshToMetaData(md, meshname, AVT_UNSTRUCTURED_MESH, NULL, 1, 0, 2, polygonTopDim);
        meshes.push_back(meshname);

        if(tessellatePolygons)
        {
            std::string tessname("tessellated_polygon");
            AddMeshToMetaData(md, tessname, AVT_UNSTRUCTURED_MESH, NULL, 1, 0, 2, polygonTopDim);
            tessmeshes.push_back(tessname);
        }
        else
            tessmeshes.push_back(meshname);
        is3D.push_back(0);
    }

    if((npts = CountMemberPoints(esriMultiPoint)) > 0)
    {
        meshname = "multipoint";
        debug4 << npts << " points for esriMultiPoint." << endl;
        AddMeshToMetaData(md, meshname, AVT_POINT_MESH, NULL, 1, 0, 2, 0);
        meshes.push_back(meshname);
        tessmeshes.push_back(meshname);
        is3D.push_back(0);
    }

    //
    // 2D with data
    //
    if((npts = CountMemberPoints(esriPointM)) > 0)
    {
        meshname = "pointM";
        debug4 << npts << " points for esriPointM." << endl;
        AddMeshToMetaData(md, meshname, AVT_POINT_MESH, NULL, 1, 0, 2, 0);
        AddScalarVarToMetaData(md, "pointM_measure", meshname, AVT_NODECENT);
        meshes.push_back(meshname);
        tessmeshes.push_back(meshname);
        is3D.push_back(0);
    }

    if((npts = CountMemberPoints(esriPolyLineM)) > 0)
    {
        meshname = "polylineM";
        debug4 << npts << " points for esriPolyLineM." << endl;
        AddMeshToMetaData(md, meshname, AVT_UNSTRUCTURED_MESH, NULL, 1, 0, 2, 1);
        AddScalarVarToMetaData(md, "polylineM_measure", meshname, AVT_NODECENT);
        meshes.push_back(meshname);
        tessmeshes.push_back(meshname);
        is3D.push_back(0);
    }

    if((npts = CountMemberPoints(esriPolygonM)) > 0)
    {
        meshname = "polygonM";
        debug4 << npts << " points for esriPolygonM." << endl;
        AddMeshToMetaData(md, meshname, AVT_UNSTRUCTURED_MESH, NULL, 1, 0, 2, polygonTopDim);
        AddScalarVarToMetaData(md, "polygonM_measure", meshname, AVT_NODECENT);
        meshes.push_back(meshname);
        tessmeshes.push_back(meshname);
        is3D.push_back(0);
    }

    if((npts = CountMemberPoints(esriMultiPointM)) > 0)
    {
        meshname = "multipointM";
        debug4 << npts << " points for esriMultiPointM." << endl;
        AddMeshToMetaData(md, meshname, AVT_POINT_MESH, NULL, 1, 0, 2, 0);
        AddScalarVarToMetaData(md, "multipointM_measure", meshname, AVT_NODECENT);
        meshes.push_back(meshname);
        tessmeshes.push_back(meshname);
        is3D.push_back(0);
    }

    //
    // 3D with data
    //
    if((npts = CountMemberPoints(esriPointZ)) > 0)
    {
        meshname = "pointZ";
        debug4 << npts << " points for esriPointZ." << endl;
        AddMeshToMetaData(md, meshname, AVT_POINT_MESH, NULL, 1, 0, 3, 0);
        AddScalarVarToMetaData(md, "pointZ_measure", meshname, AVT_NODECENT);
        meshes.push_back(meshname);
        tessmeshes.push_back(meshname);
        is3D.push_back(1);
    }

    if((npts = CountMemberPoints(esriPolyLineZ)) > 0)
    {
        meshname = "polylineZ";
        debug4 << npts << " points for esriPolyLineZ." << endl;
        AddMeshToMetaData(md, meshname, AVT_UNSTRUCTURED_MESH, NULL, 1, 0, 3, 1);
        AddScalarVarToMetaData(md, "polylineZ_measure", meshname, AVT_NODECENT);
        meshes.push_back(meshname);
        tessmeshes.push_back(meshname);
        is3D.push_back(1);
    }

    if((npts = CountMemberPoints(esriPolygonZ)) > 0)
    {
        meshname = "polygonZ";
        debug4 << npts << " points for esriPolygonZ." << endl;
        AddMeshToMetaData(md, meshname, AVT_UNSTRUCTURED_MESH, NULL, 1, 0, 3, polygonTopDim);
        AddScalarVarToMetaData(md, "polygonZ_measure", meshname, AVT_NODECENT);
        meshes.push_back(meshname);
        tessmeshes.push_back(meshname);
        is3D.push_back(1);
    }

    if((npts = CountMemberPoints(esriMultiPointZ)) > 0)
    {
        meshname = "multipointZ";
        debug4 << npts << " points for esriMultiPointZ." << endl;
        AddMeshToMetaData(md, meshname, AVT_POINT_MESH, NULL, 1, 0, 3, 0);
        AddScalarVarToMetaData(md, "multipointZ_measure", meshname, AVT_NODECENT);
        meshes.push_back(meshname);
        tessmeshes.push_back(meshname);
        is3D.push_back(1);
    }

    if((npts = CountMemberPoints(esriMultiPatch)) > 0)
    {
        meshname = "multipatch";
        debug4 << npts << " points for esriMultiPatch." << endl;
        AddMeshToMetaData(md, meshname, AVT_UNSTRUCTURED_MESH, NULL, 1, 0, 3, polygonTopDim);
        AddScalarVarToMetaData(md, "multipatch_measure", meshname, AVT_NODECENT);
        meshes.push_back(meshname);
        tessmeshes.push_back(meshname);
        is3D.push_back(1);
    }

    //
    // Add the DBF file's records to the metadata. Note that we define labels
    // on the original mesh but we define scalars on the tessellated mesh. This
    // is because VisIt does not need to draw the labels as filled polygons.
    //
    if(dbfFile != 0)
    {
        debug4 << mName << "nMeshes = " << meshes.size() << endl;
        for(int m = 0; m < meshes.size(); ++m)
        {
            dbfFieldDescriptor_t *fieldDescriptor = 
                dbfFile->header.fieldDescriptors;
            for(int i = 0; i < dbfFile->header.numFieldDescriptors; ++i)
            {
                std::string varName;
                if(meshes.size() > 1)
                    varName = meshes[i] + "/";
                varName += fieldDescriptor->fieldName;

                if(fieldDescriptor->fieldType == dbfFieldChar ||
                   fieldDescriptor->fieldType == dbfFieldDate)
                {
                    avtLabelMetaData *lmd = new avtLabelMetaData;
                    lmd->name = varName;
                    lmd->originalName = varName;
                    lmd->meshName = meshes[m];
                    lmd->centering = AVT_ZONECENT;
                    lmd->validVariable = true;
                    md->Add(lmd);

                    debug4 << mName << "Added label metadata for " << varName << endl;
                }
                else
                {
                    avtScalarMetaData *smd = new avtScalarMetaData;
                    smd->name = varName;
                    smd->originalName = varName;
                    smd->meshName = tessmeshes[m];
                    smd->centering = AVT_ZONECENT;
                    smd->hasDataExtents = false;
                    smd->minDataExtents = 0.f;
                    smd->maxDataExtents = 0.f;
                    smd->validVariable = 
                        fieldDescriptor->fieldType == dbfFieldFloatingPointNumber ||
                        fieldDescriptor->fieldType == dbfFieldFixedPointNumber ||
                        fieldDescriptor->fieldType == dbfFieldShortInt ||
                        fieldDescriptor->fieldType == dbfFieldInt ||
                        fieldDescriptor->fieldType == dbfFieldDouble;
         
                    md->Add(smd);
                    debug4 << mName << "Added scalar metadata for " << varName << endl;
                }

                ++fieldDescriptor;
            }
        }
#ifdef MDSERVER
        // If we're on the mdserver, close the file since we won't need it
        // anymore.
        FreeUpResources();
#endif
    }
    else
    {
        debug4 << mName << "Could not open the dbfFile!" << endl;
    }

    // Add expressions to get the x,y,z coordinates of the mesh, which can
    // be useful for coloring elevations.
    if(meshes.size() == 1)
    {
        Expression expr;
        expr.SetName("x");
        expr.SetDefinition(std::string("coord(") + tessmeshes[0] + ")[0]");
        expr.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&expr);

        expr.SetName("y");
        expr.SetDefinition(std::string("coord(") + tessmeshes[0] + ")[1]");
        md->AddExpression(&expr);

        if(is3D[0] == 1)
        {
            expr.SetName("z");
            expr.SetDefinition(std::string("coord(") + tessmeshes[0] + ")[2]");
            md->AddExpression(&expr);
        }
    }
    else
    {
        for(int m = 0; m < meshes.size(); ++m)
        {
            Expression expr;
            expr.SetName(meshes[m] + "_x");
            expr.SetDefinition(std::string("coord(") + tessmeshes[m] + ")[0]");
            expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&expr);

            expr.SetName(meshes[m] + "_y");
            expr.SetDefinition(std::string("coord(") + tessmeshes[m] + ")[1]");
            md->AddExpression(&expr);

            if(is3D[m] == 1)
            {
                expr.SetName(meshes[m] + "_z");
                expr.SetDefinition(std::string("coord(") + tessmeshes[m] + ")[2]");
                md->AddExpression(&expr);
            }
        }
    }
}

// ****************************************************************************
// Method: avtShapefileFileFormat::GetMesh_TessellatedPolygon
//
// Purpose: 
//   Takes the input mesh and tessellates the polygon cells and returns a polydata.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 6 18:08:41 PST 2007
//
// Modifications:
//   
// ****************************************************************************

#ifndef MDSERVER
// We're not in the mdserver so include some helper classes.
#include <PolygonToTriangles.C>
#endif

vtkDataSet *
avtShapefileFileFormat::GetMesh_TessellatedPolygon()
{
#ifdef MDSERVER
    return 0;
#else
    const char *mName = "avtShapefileFileFormat::GetMesh_TessellatedPolygon: ";
    int npts = CountMemberPoints(esriPolygon);

    // Allocate some points.
    vtkPoints *pts = vtkPoints::New();
    pts->Allocate(npts);

    VertexManager      uniqueVerts(pts);
    PolygonToTriangles tess(&uniqueVerts);

    // Go through each cell and tesselate into triangles.
    debug5 << mName << "Start tessellation." << endl;
    for(int i = 0; i < shapes.size(); ++i)
    {
        if(shapes[i].shapeType == esriPolygon)
        {
            esriPolygon_t *pg = (esriPolygon_t *)shapes[i].shape;

            debug5 << "\tsplitting polygon " << i << "...";
            tess.BeginPolygon();
            for(int part = 0; part < pg->numParts; ++part)
            {
                int start = pg->parts[part];
                int end = (part < pg->numParts-1) ? 
                   pg->parts[part+1] : pg->numPoints;
                int index = 0;

                tess.BeginContour();
                for(int j = start; j < end-1; ++j, ++index)
                {
                    // Stash the point
                    double v[3];
                    v[0] = pg->points[j].x;
                    v[1] = pg->points[j].y;
                    v[2] = 0.f;

                    tess.AddVertex(v);
                }
                tess.EndContour();
            }
            tess.EndPolygon();

            // Store off the number of times this shape's data will need to
            // be repeated in the data.
            shapes[i].nRepeats = tess.GetNumTrianglesInLastPolygon();

            debug5 << " into " << shapes[i].nRepeats << " triangles." << endl;
        }
    }

    pts->Squeeze();

    vtkDataSet *ret = NULL;
    if(tess.GetNumTriangles() > 0)
    {
        // Allocate polydata. The number of cells probably will be larger than
        // we allocate so it will grow as we insert.
        vtkPolyData *pd = vtkPolyData::New();
        pd->SetPoints(pts);
        pd->Allocate(shapes.size() * 3);

        // Now that we've tesselated everything, let's create the cells for it.
        for(int i = 0; i < tess.GetNumTriangles(); ++i)
        {
            int a,b,c;
            vtkIdType verts[3];
            tess.GetTriangle(i, a, b, c);
            verts[0] = a;
            verts[1] = b;
            verts[2] = c;
            pd->InsertNextCell(VTK_TRIANGLE, 3, verts);
        }
        pd->Squeeze();
        ret = pd;
    }
    else
    {
        debug5 << mName << "No cells made from triangulated polygons" << endl;
    }
    pts->Delete();

    return ret;
#endif
}

// ****************************************************************************
//  Method: avtShapefileFileFormat::GetMesh
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
//  Creation:   Thu Mar 24 12:18:02 PDT 2005
//
//  Modifications:
//    Brad Whitlock, Tue Mar 6 18:07:53 PST 2007
//    Added tessellated mesh.
//
// ****************************************************************************

vtkDataSet *
avtShapefileFileFormat::GetMesh(const char *meshname)
{
#ifdef MDSERVER
    return 0;
#else
    //
    // Determine the shape type from the mesh name.
    //
    esriShapeType_t shapeType;
    if(strcmp(meshname, "point") == 0)
       shapeType = esriPoint;
    else if(strcmp(meshname, "polyline") == 0)
       shapeType = esriPolyLine;
    else if(strcmp(meshname, "polygon") == 0)
       shapeType = esriPolygon;
    else if(strcmp(meshname, "tessellated_polygon") == 0)
       return GetMesh_TessellatedPolygon();
    else if(strcmp(meshname, "multipoint") == 0)
       shapeType = esriMultiPoint;
    else if(strcmp(meshname, "pointM") == 0)
       shapeType = esriPointM;
    else if(strcmp(meshname, "polylineM") == 0)
       shapeType = esriPolyLineM;
    else if(strcmp(meshname, "polygonM") == 0)
       shapeType = esriPolygonM;
    else if(strcmp(meshname, "multipointM") == 0)
       shapeType = esriMultiPointM;
    else if(strcmp(meshname, "pointZ") == 0)
       shapeType = esriPointZ;
    else if(strcmp(meshname, "polylineZ") == 0)
       shapeType = esriPolyLineZ;
    else if(strcmp(meshname, "polygonZ") == 0)
       shapeType = esriPolygonZ;
    else if(strcmp(meshname, "multipointZ") == 0)
       shapeType = esriMultiPointZ;
//    else if(strcmp(meshname, "multipatch") == 0)
//       shapeType = esriMultiPatch;
    else
    {
        EXCEPTION1(InvalidVariableException, meshname);
    }

    //
    // Assemble the mesh for the specified shape type.
    //
    vtkPoints  *points = 0;
    vtkDataSet *ds = 0;
    int npts = CountMemberPoints(shapeType);
    if(npts > 0)
    {
        points  = vtkPoints::New();
        points->SetNumberOfPoints(npts);
        float *pts = (float *) points->GetVoidPointer(0);

#define VTKIDTYPE_BUFFSIZE 200
        vtkIdType vertices[VTKIDTYPE_BUFFSIZE], *verts = vertices;
        vtkPolyData *pd = 0;

        //
        // Create the grid.
        //
        switch(shapeType)
        {
        case esriPoint:
        case esriMultiPoint:
        case esriPointM:
        case esriMultiPointM:
        case esriPointZ:
        case esriMultiPointZ:
            pd = vtkPolyData::New();
            pd->SetPoints(points);
            pd->Allocate(npts);
            ds = pd;
            break;
        case esriPolyLine:
        case esriPolyLineM:
        case esriPolyLineZ:
            debug4 << "Creating polydata for " << meshname << endl;
            pd = vtkPolyData::New();
            pd->SetPoints(points);
            pd->Allocate(npts);
            ds = pd;
            break;
        case esriPolygon:
        case esriPolygonM:
        case esriPolygonZ:
            debug4 << "Creating ugrid for " << meshname << endl;
            pd = vtkPolyData::New(); 
            pd->SetPoints(points);
            pd->Allocate(npts);
            ds = pd;
            break;
        default:
            ;
        };


        //
        // Add the points and the cells.
        //
        int pointIndex = 0;
        int verts_size = VTKIDTYPE_BUFFSIZE;

#define CHECK_VERTS_SIZE(S) if((S) >= verts_size) { \
                                if(verts != vertices) \
                                    delete [] verts; \
                                verts = new vtkIdType[S]; \
                                verts_size = nverts; \
                            }

        for(int i = 0; i < shapes.size(); ++i)
        {
            int part, j;

            if(shapeType == shapes[i].shapeType)
            {
                switch(shapes[i].shapeType)
                {
                case esriNullShape:
                    break;
                case esriPoint:
                    pts[0] = ((esriPoint_t *)shapes[i].shape)->x;
                    pts[1] = ((esriPoint_t *)shapes[i].shape)->y;
                    pts[2] = 0.f;
                    pts += 3;
                    vertices[0] = pointIndex++;
                    pd->InsertNextCell(VTK_VERTEX, 1, vertices);
                    break;
                case esriPointM:
                    pts[0] = ((esriPointM_t *)shapes[i].shape)->x;
                    pts[1] = ((esriPointM_t *)shapes[i].shape)->y;
                    pts[2] = 0.f;
                    pts += 3;
                    vertices[0] = pointIndex++;
                    pd->InsertNextCell(VTK_VERTEX, 1, vertices);
                    break;
                case esriPointZ:
                    pts[0] = ((esriPointZ_t *)shapes[i].shape)->x;
                    pts[1] = ((esriPointZ_t *)shapes[i].shape)->y;
                    pts[2] = ((esriPointZ_t *)shapes[i].shape)->z;
                    pts += 3;
                    vertices[0] = pointIndex++;
                    pd->InsertNextCell(VTK_VERTEX, 1, vertices);
                    break;
                case esriPolyLine:
                    { // new scope
                    esriPolyLine_t *pl = (esriPolyLine_t *)shapes[i].shape;
                    for(part = 0; part < pl->numParts; ++part)
                    {
                        int start = pl->parts[part];
                        int end = (part < pl->numParts-1) ? 
                            pl->parts[part+1] : pl->numPoints;
                        int nverts = end - start;
                        int index = 0;
                        CHECK_VERTS_SIZE(nverts);
                        for(j = start; j < end; ++j, ++index)
                        {
                            // Stash the point
                            pts[0] = pl->points[j].x;
                            pts[1] = pl->points[j].y;
                            pts[2] = 0.f;
                            pts += 3;
                            verts[index] = pointIndex++;
                        }

                        pd->InsertNextCell(VTK_POLY_LINE, nverts, verts);
                    }
                    } // end new scope
                    break;
                case esriPolyLineM:
                    { // new scope
                    esriPolyLineM_t *pl = (esriPolyLineM_t *)shapes[i].shape;
                    for(part = 0; part < pl->numParts; ++part)
                    {
                        int start = pl->parts[part];
                        int end = (part < pl->numParts-1) ? 
                            pl->parts[part+1] : pl->numPoints;
                        int nverts = end - start;
                        int index = 0;
                        CHECK_VERTS_SIZE(nverts);
                        for(j = start; j < end; ++j, ++index)
                        {
                            // Stash the point
                            pts[0] = pl->points[j].x;
                            pts[1] = pl->points[j].y;
                            pts[2] = 0.f;
                            pts += 3;
                            verts[index] = pointIndex++;
                        }

                        pd->InsertNextCell(VTK_POLY_LINE, nverts, verts);
                    }

                    } // end new scope
                    break;
                case esriPolyLineZ:
                    { // new scope
                    esriPolyLineZ_t *pl = (esriPolyLineZ_t *)shapes[i].shape;
                    for(part = 0; part < pl->numParts; ++part)
                    {
                        int start = pl->parts[part];
                        int end = (part < pl->numParts-1) ? 
                            pl->parts[part+1] : pl->numPoints;
                        int nverts = end - start;
                        int index = 0;
                        CHECK_VERTS_SIZE(nverts);
                        for(j = start; j < end; ++j, ++index)
                        {
                            // Stash the point
                            pts[0] = pl->points[j].x;
                            pts[1] = pl->points[j].y;
                            pts[2] = pl->z[j];
                            pts += 3;
                            verts[index] = pointIndex++;
                        }

                        pd->InsertNextCell(VTK_POLY_LINE, nverts, verts);
                    }

                    } // end new scope
                    break;

//
// Temporarily serve up polygons as polylines. We have to devise some
// way of transforming these polygons, which can have thousands of vertices,
// and holes, into triangles that VTK will like.
//

                case esriPolygon:
                    { // new scope
                    esriPolygon_t *pg = (esriPolygon_t *)shapes[i].shape;
                    for(part = 0; part < pg->numParts; ++part)
                    {
                        int start = pg->parts[part];
                        int end = (part < pg->numParts-1) ? 
                            pg->parts[part+1] : pg->numPoints;
                        int nverts = end - start;
                        int firstPoint = pointIndex;
                        int index = 0;
                        CHECK_VERTS_SIZE(nverts);
                        if(polygonsAsLines)
                        {
                            for(j = start; j < end; ++j, ++index)
                            {
                                if(j < end - 1)
                                {
                                    // Stash the point
                                    pts[0] = pg->points[j].x;
                                    pts[1] = pg->points[j].y;
                                    pts[2] = 0.f;
                                    pts += 3;
                                    verts[index] = pointIndex++;
                                }
                                else
                                    verts[index] = firstPoint;
                            }

                            pd->InsertNextCell(VTK_POLY_LINE, nverts, verts);
                        }
                        else
                        {
                            for(j = start; j < end-1; ++j, ++index)
                            {
                                // Stash the point
                                pts[0] = pg->points[j].x;
                                pts[1] = pg->points[j].y;
                                pts[2] = 0.f;
                                pts += 3;
                                verts[index] = pointIndex++;
                            }

                            pd->InsertNextCell(VTK_POLYGON, nverts-1, verts);
                        }
                    }

                    } // end new scope
                    break;
                case esriPolygonM:
                    { // new scope
                    esriPolygonM_t *pg = (esriPolygonM_t *)shapes[i].shape;
                    for(part = 0; part < pg->numParts; ++part)
                    {
                        int start = pg->parts[part];
                        int end = (part < pg->numParts-1) ? 
                            pg->parts[part+1] : pg->numPoints;
                        int nverts = end - start;
                        int firstPoint = pointIndex;
                        int index = 0;
                        CHECK_VERTS_SIZE(nverts);
                        if(polygonsAsLines)
                        {
                            for(j = start; j < end; ++j, ++index)
                            {
                                if(j < end - 1)
                                {
                                    // Stash the point
                                    pts[0] = pg->points[j].x;
                                    pts[1] = pg->points[j].y;
                                    pts[2] = 0.f;
                                    pts += 3;
                                    verts[index] = pointIndex++;
                                }
                                else
                                    verts[index] = firstPoint;
                            }

                            pd->InsertNextCell(VTK_POLY_LINE, nverts, verts);
                        }
                        else
                        {
                            for(j = start; j < end-1; ++j, ++index)
                            {
                                // Stash the point
                                pts[0] = pg->points[j].x;
                                pts[1] = pg->points[j].y;
                                pts[2] = 0.f;
                                pts += 3;
                                verts[index] = pointIndex++;
                            }

                            pd->InsertNextCell(VTK_POLYGON, nverts-1, verts);
                        }
                    }
                    } // end new scope
                    break;
                case esriPolygonZ:
                    { // new scope
                    esriPolygonZ_t *pg = (esriPolygonZ_t *)shapes[i].shape;
                    for(part = 0; part < pg->numParts; ++part)
                    {
                        int start = pg->parts[part];
                        int end = (part < pg->numParts-1) ? 
                            pg->parts[part+1] : pg->numPoints;
                        int nverts = end - start;
                        int firstPoint = pointIndex;
                        int index = 0;
                        CHECK_VERTS_SIZE(nverts);
                        if(polygonsAsLines)
                        {
                            for(j = start; j < end; ++j, ++index)
                            {
                                if(j < end - 1)
                                {
                                    // Stash the point
                                    pts[0] = pg->points[j].x;
                                    pts[1] = pg->points[j].y;
                                    pts[2] = pg->z[j];
                                    pts += 3;
                                    verts[index] = pointIndex++;
                                }
                                else
                                    verts[index] = firstPoint;
                            }

                            pd->InsertNextCell(VTK_POLY_LINE, nverts, verts);
                        }
                        else
                        {
                            for(j = start; j < end-1; ++j, ++index)
                            {
                                // Stash the point
                                pts[0] = pg->points[j].x;
                                pts[1] = pg->points[j].y;
                                pts[2] = pg->z[j];
                                pts += 3;
                                verts[index] = pointIndex++;
                            }

                            pd->InsertNextCell(VTK_POLYGON, nverts-1, verts);
                        }
                    }
                    } // end new scope
                    break;

                case esriMultiPoint:
                    { // new scope
                    esriMultiPoint_t *mp = (esriMultiPoint_t *)shapes[i].shape;
                    for(j = 0; j < mp->numPoints; ++j)
                    {
                        pts[0] = mp->points[j].x;
                        pts[1] = mp->points[j].y;
                        pts[2] = 0.f;
                        pts += 3;

                        vertices[0] = pointIndex++;
                        pd->InsertNextCell(VTK_VERTEX, 1, vertices);
                    }
                    } // end new scope
                    break;
                case esriMultiPointM:
                    { // new scope
                    esriMultiPointM_t *mp = (esriMultiPointM_t *)shapes[i].shape;
                    for(j = 0; j < mp->numPoints; ++j)
                    {
                        pts[0] = mp->points[j].x;
                        pts[1] = mp->points[j].y;
                        pts[2] = 0.f;
                        pts += 3;

                        vertices[0] = pointIndex++;
                        pd->InsertNextCell(VTK_VERTEX, 1, vertices);
                    }
                    } // end new scope
                    break;
                case esriMultiPointZ:
                    { // new scope
                    esriMultiPointZ_t *mp = (esriMultiPointZ_t *)shapes[i].shape;
                    for(j = 0; j < mp->numPoints; ++j)
                    {
                        pts[0] = mp->points[j].x;
                        pts[1] = mp->points[j].y;
                        pts[2] = mp->z[j];
                        pts += 3;

                        vertices[0] = pointIndex++;
                        pd->InsertNextCell(VTK_VERTEX, 1, vertices);
                    }
                    } // end new scope
                    break;
                case esriMultiPatch:
                    break;
                default:
                    debug4 << "Unknown type!" << endl;
                }
            }
        }

        if(verts != vertices)
            delete [] verts;

        points->Delete();
    }

    return ds;
#endif
}

// ****************************************************************************
// Method: avtShapefileFileFormat::GetNumRepeats
//
// Purpose: 
//   Determines the number of times a data value should be repeated in the
//   data array.
//
// Arguments:
//   TheShape  : A pointer to an esriShape shape.
//   tessMesh  : True if we're asking for the number of repeats needed for 
//               the tessellated mesh; False otherwise.
//
// Returns:    The number of times data should be repeated for the shape.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 1 23:38:01 PST 2005
//
// Modifications:
//   Brad Whitlock, Wed Mar 7 08:49:19 PDT 2007
//   Pass in the shape container instead and added support for tessellated 
//   polygons.
//
// ****************************************************************************

int
avtShapefileFileFormat::GetNumRepeats(const esriShape &TheShape, bool tessMesh) const
{
    int nr = 1;
    void *shape = TheShape.shape;

    switch(TheShape.shapeType)
    {
    case esriPolyLine:
        nr = ((esriPolyLine_t *)shape)->numParts;
        break;
    case esriPolygon:
        if(tessMesh && tessellatePolygons)
            nr = TheShape.nRepeats;
        else
            nr = ((esriPolygon_t *)shape)->numParts;
        break;
    case esriMultiPoint:
        nr = ((esriMultiPoint_t *)shape)->numPoints;
        break;
    case esriPolyLineZ:
        nr = ((esriPolyLineZ_t *)shape)->numParts;
        break;
    case esriPolygonZ:
        nr = ((esriPolygonZ_t *)shape)->numParts;
        break;
    case esriMultiPointZ:
        nr = ((esriMultiPointZ_t *)shape)->numPoints;
        break;
    case esriPolyLineM:
        nr = ((esriPolyLineM_t *)shape)->numParts;
        break;
    case esriPolygonM:
        nr = ((esriPolygonM_t *)shape)->numParts;
        break;
    case esriMultiPointM:
        nr = ((esriMultiPointM_t *)shape)->numPoints;
        break;
    case esriMultiPatch:
        nr = ((esriMultiPatch_t *)shape)->numParts;
        break;
    default:
        break;
    }

    return nr;
}

// ****************************************************************************
//  Method: avtShapefileFileFormat::GetVar
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
//  Creation:   Thu Mar 24 12:18:02 PDT 2005
//
//  Modifications:
//    Brad Whitlock, Tue Mar 29 16:07:59 PST 2005
//    Use pointer arithmetic for sticking the metrics into the array. I added
//    support for reading label variables.
//
// ****************************************************************************

vtkDataArray *
avtShapefileFileFormat::GetVar(const char *varname)
{
#ifdef MDSERVER
    return 0;
#else
    static const char *mName = "avtShapefileFileFormat::GetVar: ";

    //
    // Determine the shape type from the mesh name.
    //
    dbfFieldDescriptor_t *field = 0;
    esriShapeType_t shapeType;
    if(strcmp(varname, "pointM_measure") == 0)
        shapeType = esriPointM;
    else if(strcmp(varname, "polylineM_measure") == 0)
        shapeType = esriPolyLineM;
    else if(strcmp(varname, "polygonM_measure") == 0)
        shapeType = esriPolygonM;
    else if(strcmp(varname, "multipointM_measure") == 0)
        shapeType = esriMultiPointM;
    else if(strcmp(varname, "pointZ_measure") == 0)
        shapeType = esriPointZ;
    else if(strcmp(varname, "polylineZ_measure") == 0)
        shapeType = esriPolyLineZ;
    else if(strcmp(varname, "polygonZ_measure") == 0)
        shapeType = esriPolygonZ;
    else if(strcmp(varname, "multipointZ_measure") == 0)
        shapeType = esriMultiPointZ;
    else if(strcmp(varname, "multipatch_measure") == 0)
        shapeType = esriMultiPatch;
    else if(dbfFile != 0)
    {
        unsigned int offset = 0;
        if(numShapeTypes == 1)
        {
            shapeType = shapes[0].shapeType;
            field = dbfGetFieldDescriptor(dbfFile, varname, &offset);
        }
        else
        {
            std::string vn(varname);
            int slashPos = vn.rfind("/");
            if(slashPos != -1)
            {
                std::string tn(vn.substr(slashPos));
                std::string realVar(vn.substr(slashPos+1,vn.size()-1));
                if(strcmp(tn.c_str(), "esriPoint") == 0)
                    shapeType = esriPoint;
                else if(strcmp(tn.c_str(), "esriPolyLine") == 0)
                    shapeType = esriPolyLine;
                else if(strcmp(tn.c_str(), "esriPolygon") == 0)
                    shapeType = esriPolygon;
                else if(strcmp(tn.c_str(), "esriMultiPoint") == 0)
                    shapeType = esriMultiPoint;
                else if(strcmp(tn.c_str(), "esriPointZ") == 0)
                    shapeType = esriPointZ;
                else if(strcmp(tn.c_str(), "esriPolyLineZ") == 0)
                    shapeType = esriPolyLineZ;
                else if(strcmp(tn.c_str(), "esriPolygonZ") == 0)
                    shapeType = esriPolygonZ;
                else if(strcmp(tn.c_str(), "esriMultiPointZ") == 0)
                    shapeType = esriMultiPointZ;
                else if(strcmp(tn.c_str(), "esriPointM") == 0)
                    shapeType = esriPointM;
                else if(strcmp(tn.c_str(), "esriPolyLineM") == 0)
                    shapeType = esriPolyLineM;
                else if(strcmp(tn.c_str(), "esriPolygonM") == 0)
                    shapeType = esriPolygonM;
                else if(strcmp(tn.c_str(), "esriMultiPointM") == 0)
                    shapeType = esriMultiPointM;
                else if(strcmp(tn.c_str(), "esriMultiPatch") == 0)
                    shapeType = esriMultiPatch;

                field = dbfGetFieldDescriptor(dbfFile, realVar.c_str(), &offset);
            }
        }
    }
    else
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    vtkDataArray *ds = 0;
    if(field != 0)
    {
        dbfReadError_t rcError;
        int nShapes = CountShapes(shapeType);
        int nCells  = CountCellsForShape(shapeType);

        debug4 << mName << "var="<<varname<<", nShapes=" << nShapes
               << ", nCells=" << nCells
               << ", dbfFile.numRecords=" << dbfFile->header.numRecords << endl;

        if(nCells > nShapes)
        {
            // We're going to have nCells rather than nShapes because some of
            // the shapes had multiple parts.
            if(field->fieldType == dbfFieldChar ||
               field->fieldType == dbfFieldDate)
            {
                // char data
                vtkUnsignedCharArray *rv = vtkUnsignedCharArray::New();
                rv->SetNumberOfComponents(field->fieldLength + 1);
                rv->SetNumberOfTuples(nCells);
                unsigned char *ucdest = (unsigned char *)rv->GetVoidPointer(0);
                unsigned char *ucsrc = (unsigned char *)dbfFileReadField(dbfFile, field->fieldName,
                    &rcError);
                if(rcError != dbfReadErrorSuccess)
                {
                    rv->Delete();
                    rv = 0;
                }
                else
                {
                    unsigned char *ucptr = ucsrc;
                    for(int i = 0; i < shapes.size(); ++i)
                    {
                        int nr = 1;
                        if(shapes[i].shapeType == shapeType)
                            nr = GetNumRepeats(shapes[i], false);

                        // Repeat the string field value for all cells in the shape.
                        for(int j = 0; j < nr; ++j)
                        {
                            memcpy((void *)ucdest, (const void *)ucptr, field->fieldLength);
                            ucdest += field->fieldLength + 1;
                        }

                        // Move to the next shape's field data value.
                        ucptr += field->fieldLength + 1;
                    }

                    dbfFree((void *)ucsrc);
                }
                ds = rv;
            }
            else
            {
                // Float data.
                vtkFloatArray *rv = vtkFloatArray::New();
                rv->SetNumberOfTuples(nCells);

                float *fdest = (float *)rv->GetVoidPointer(0);
                dbfSetForceFloat(1);
                float *fsrc = (float *)dbfFileReadField(dbfFile, field->fieldName,
                    &rcError);
                if(rcError != dbfReadErrorSuccess)
                {
                    rv->Delete();
                    rv = 0;
                }
                else
                {
                    float *fptr = fsrc;
                    for(int i = 0; i < shapes.size(); ++i)
                    {
                        int nr = 1;
                        if(shapes[i].shapeType == shapeType)
                            nr = GetNumRepeats(shapes[i], true);

                        // Repeat the field value for all cells in the shape.
                        for(int j = 0; j < nr; ++j)
                            *fdest++ = *fptr;

                        // Move to the next shape's field data value.
                        ++fptr;
                    }

                    dbfFree((void *)fsrc);
                }
                ds = rv;
            }
        }
        else
        {
           /* Try and read the data into the preallocated VTK memory. Note that
            * this code is not correct if a shape file ever had more than one
            * shape type. It would not be hard to make it right but shape files
            * never have more than 1 type of shape at this point.
            */
            if(field->fieldType == dbfFieldChar ||
               field->fieldType == dbfFieldDate)
            {
                vtkUnsignedCharArray *rv = vtkUnsignedCharArray::New();
                rv->SetNumberOfComponents(field->fieldLength + 1);
                rv->SetNumberOfTuples(nShapes);

                dbfFileReadField2(dbfFile, field->fieldName, rv->GetVoidPointer(0),
                    &rcError);
                if(rcError != dbfReadErrorSuccess)
                {
                    rv->Delete();
                    rv = 0;
                }
                ds = rv;
            }
            else
            {
                vtkFloatArray *rv = vtkFloatArray::New();
                rv->SetNumberOfTuples(nShapes);
                dbfSetForceFloat(1);
                dbfFileReadField2(dbfFile, field->fieldName, rv->GetVoidPointer(0),
                    &rcError);
                if(rcError != dbfReadErrorSuccess)
                {
                    rv->Delete();
                    rv = 0;
                }
                ds = rv;
            }
        }
    }
    else
    {
        // The number of entries in the variable.
        int ntuples = CountMemberPoints(shapeType);
        vtkFloatArray *rv = vtkFloatArray::New();
        rv->SetNumberOfTuples(ntuples);
        float *data = (float *)rv->GetVoidPointer(0);

        int j;
        for(int i = 0; i < shapes.size(); ++i)
        {
            if(shapes[i].shapeType == shapeType)
            {
                switch(shapeType)
                {
                case esriPointM:
                    *data++ = float(((esriPointM_t *)shapes[i].shape)->m);
                    break;
                case esriPolyLineM:
                    { // new scope
                        esriPolyLineM_t *obj = (esriPolyLineM_t *)shapes[i].shape;
                        for(j = 0; j < obj->numPoints; ++j)
                            *data++ = float(obj->measures[j]);
                    }
                    break;
                case esriPolygonM:
                    { // new scope
                        esriPolygonM_t *obj = (esriPolygonM_t *)shapes[i].shape;
                        for(j = 0; j < obj->numPoints; ++j)
                            *data++ = float(obj->measures[j]);
                    }
                    break;
                case esriMultiPointM:
                    { // new scope
                        esriMultiPointM_t *obj = (esriMultiPointM_t *)shapes[i].shape;
                        for(j = 0; j < obj->numPoints; ++j)
                            *data++ = float(obj->measures[j]);
                    }
                    break;
                case esriPointZ:
                    *data++ = float(((esriPointM_t *)shapes[i].shape)->m);
                    break;
                case esriPolyLineZ:
                    { // new scope
                        esriPolyLineZ_t *obj = (esriPolyLineZ_t *)shapes[i].shape;
                        for(j = 0; j < obj->numPoints; ++j)
                            *data++ = float(obj->measures[j]);
                    }
                    break;
                case esriPolygonZ:
                    { // new scope
                        esriPolygonZ_t *obj = (esriPolygonZ_t *)shapes[i].shape;
                        for(j = 0; j < obj->numPoints; ++j)
                            *data++ = float(obj->measures[j]);
                    }
                    break;
                case esriMultiPointZ:
                    { // new scope
                        esriMultiPointZ_t *obj = (esriMultiPointZ_t *)shapes[i].shape;
                        for(j = 0; j < obj->numPoints; ++j)
                            *data++ = float(obj->measures[j]);
                    }
                    break;
                case esriMultiPatch:
                    { // new scope
                        esriMultiPatch_t *obj = (esriMultiPatch_t *)shapes[i].shape;
                        for(j = 0; j < obj->numPoints; ++j)
                            *data++ = float(obj->measures[j]);
                    }
                    break;
                default:
                    //nothing
                    break;
                }
            }
        }

        ds = rv;
    }

    return ds;
#endif
}


// ****************************************************************************
//  Method: avtShapefileFileFormat::GetVectorVar
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
//  Creation:   Thu Mar 24 12:18:02 PDT 2005
//
// ****************************************************************************

vtkDataArray *
avtShapefileFileFormat::GetVectorVar(const char *varname)
{
    EXCEPTION1(InvalidVariableException, varname);
}
