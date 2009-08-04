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
//                            avtPATRANFileFormat.C                          //
// ************************************************************************* //

#include <avtPATRANFileFormat.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <map>

#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkFloatArray.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>
#include <avtMaterial.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>

#include <TimingsManager.h>
#include <DebugStream.h>
#include <FileFunctions.h>
#include <snprintf.h>

#define PATRAN_PACKET_NODE_DATA        1
#define PATRAN_PACKET_ELEMENT_DATA     2
#define PATRAN_PACKET_CELL_DATA        4
#define PATRAN_PACKET_NAMED_COMPONENTS 21
#define PATRAN_PACKET_TITLE            25
#define PATRAN_PACKET_SUMMARY          26

#define SHORT_FIELD_WIDTH              8
#define LONG_FIELD_WIDTH               16

#define MAX_CELL_PROPERTIES            5

//#define DEBUG_PRINT_HEADER
//#define DEBUG_PRINT_CELL_VERTS
//#define DEBUG_PRINT_NODES
//#define DEBUG_PRINT_MATERIAL_NAMES

#define VARNAME_ELEMENTIDS  "elementIds"
#define VARNAME_ELEMENTMATS "elementMats"


//
// PATRAN models have a node id associated with each vertex and that nodeid
// is used in the GRID statement that defines the vertex. Sometimes the model
// may not define certain node numbers, leading to gaps in the node number
// sequence. If we define the USE_POINT_INDICES_TO_INSERT macro then we will use
// the node id's to insert new nodes into the vtkPoints so we'll not mess up
// the connectivity specified in the CHEXA, CQUAD,... cell declarations. The
// penalty is that we have to run the data through the relevant points filter
// so the gaps are removed from the points array.
//
//#define USE_POINT_INDICES_TO_INSERT
#if !defined(MDSERVER) && defined(USE_POINT_INDICES_TO_INSERT)
#include <vtkUnstructuredGridRelevantPointsFilter.h>
#endif

using     std::string;

#define ALL_LINES -1

// ****************************************************************************
//  Method: avtPATRANFileFormat constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jul 28 13:53:21 PST 2005
//
//  Modifications:
//    Brad Whitlock, Mon Jun  8 10:01:53 PDT 2009
//    I added properties.
//
// ****************************************************************************

avtPATRANFileFormat::avtPATRANFileFormat(const char *filename)
    : avtSTSDFileFormat(filename), title(), componentNames()
{
    meshDS = 0;
    elementMats = 0;
    elementIds = 0;
    componentList = 0;
    componentListSize = -1;
    for(int i = 0; i < MAX_CELL_PROPERTIES; ++i)
        properties[i] = 0;

#ifdef MDSERVER
    debug4 << "avtPATRANFileFormat::avtPATRANFileFormat\n";
    if(!ReadFile(filename, 100))
    {
        EXCEPTION1(InvalidFilesException, filename);
    }
    componentNames.clear();

    debug4 << "avtPATRANFileFormat::avtPATRANFileFormat: meshDS="
           << (void*)meshDS << endl;
#endif
}

// ****************************************************************************
// Method: avtPATRANFileFormat::~avtPATRANFileFormat
//
// Purpose: 
//   Destructor for avtPATRANFileFormat class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 28 13:53:21 PST 2005
//
// Modifications:
//   
// ****************************************************************************

avtPATRANFileFormat::~avtPATRANFileFormat()
{
    FreeUpResources();
}

// ****************************************************************************
//  Method: avtPATRANFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jul 28 13:53:21 PST 2005
//
//  Modifications:
//    Brad Whitlock, Mon Jun  8 10:02:42 PDT 2009
//    I added properties.
//
// ****************************************************************************

void
avtPATRANFileFormat::FreeUpResources(void)
{
    debug4 << "avtPATRANFileFormat::FreeUpResources" << endl;
    if(meshDS != 0)
    {
        meshDS->Delete();
        meshDS = 0;
    }

    if(elementMats != 0)
    {
        elementMats->Delete();
        elementMats = 0;
    }

    if(elementIds != 0)
    {
        elementIds->Delete();
        elementIds = 0;
    }

    if(componentList != 0)
    {
        delete [] componentList;
        componentList = 0;
        componentListSize = 0;
    }

    for(int i = 0; i < MAX_CELL_PROPERTIES; ++i)
    {
        if(properties[i] != 0)
        {
            properties[i]->Delete();
            properties[i] = 0;
        }
    }

    title = "";
    componentNames.clear();
}

// ****************************************************************************
// Method: avtPATRANFileFormat::ActivateTimestep
//
// Purpose: 
//   Called when we're activating the current time step.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 28 13:53:21 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtPATRANFileFormat::ActivateTimestep()
{
    debug4 << "avtPATRANFileFormat::ActivateTimestep: meshDS="
           << (void*)meshDS << endl;

    if(meshDS == 0)
    {
        ReadFile(filename, ALL_LINES);
    }
}

#ifndef MDSERVER
// ****************************************************************************
// Function: AddPoint
//
// Purpose: 
//   Reads a point from the line of text and inserts it into the pts object.
//
// Arguments:
//   line    : The text from which to read the points.
//   ptid    : The id of the point.
//   nPoints : The new number of points in the pts object.
//   pts     : The points object used to store the points.
//
// Note:       Not compiled on MDSERVER.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 29 16:28:55 PST 2005
//
// Modifications:
//   
// ****************************************************************************

inline void
AddPoint(char *line, const int ptid, int &nPoints, vtkPoints *pts)
{
    // Read the coordinate.
    char *valstart = line + 2 * LONG_FIELD_WIDTH;
    char *valend = valstart + LONG_FIELD_WIDTH;
    float pt[3];
    pt[2] = atof(valstart);

    valstart -= LONG_FIELD_WIDTH;
    valend -= LONG_FIELD_WIDTH;
    *valend = '\0';
    pt[1] = atof(valstart);

    valstart -= LONG_FIELD_WIDTH;
    valend -= LONG_FIELD_WIDTH;
    *valend = '\0';
    pt[0] = atof(valstart);

#ifdef DEBUG_PRINT_NODES
    debug4 << "Node: " << pt[0]
           << ", " << pt[1]
           << ", " << pt[0] << endl;
#endif

#ifdef USE_POINT_INDICES_TO_INSERT
    if(ptid < nPoints)
        pts->SetPoint(ptid, pt);
    else
    {
        int newSize = int(float(nPoints) * 1.1f);
        if(newSize < ptid)
            newSize = int(float(ptid) * 1.1f);

        debug4 << "Resizing point array from " << nPoints
               << " points to " << newSize
               << " points because we need to insert point index "
               << ptid << endl;
        nPoints = newSize;
        pts->GetData()->Resize(nPoints);
        pts->SetNumberOfPoints(nPoints);
        pts->SetPoint(ptid, pt);
    }
#else
    pts->InsertNextPoint(pt);
#endif
}

// ****************************************************************************
// Method: AddCell
//
// Purpose: 
//   Reads the line of text for the cell connectivity and inserts a cell into
//   the ugrid.
//
// Arguments:
//   line         : The line of text from which to read the point ids.
//   cellNumNodes : The expected number of nodes in the cell.
//   IV           : The value for IV.
//   ugrid        : The dataset in which to insert the cell.
//
// Returns:    True if the cell was inserted; false otherwise.
//
// Note:       Not compiled on the MDSERVER.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 29 16:30:36 PST 2005
//
// Modifications:
//   
// ****************************************************************************

inline bool
AddCell(char *line, const int cellNumNodes, const int IV, 
        vtkUnstructuredGrid *ugrid)
{
    static const int IVToVTKCell[] = {
        -1, -1, VTK_LINE, VTK_TRIANGLE, VTK_QUAD,
        VTK_TETRA, -1, VTK_WEDGE, VTK_HEXAHEDRON
    };
    bool retval = true;

    if(IV >= 2 && IV <= 8 && IVToVTKCell[IV] != -1)
    {
        char *valstart = line + (cellNumNodes-1) * SHORT_FIELD_WIDTH;
        char *valend = valstart + SHORT_FIELD_WIDTH;
        int   verts[8];

        for(int v = 0; v < cellNumNodes; ++v)
        {
            *valend = '\0';
            verts[v] = atoi(valstart)-1;
            valstart -= SHORT_FIELD_WIDTH;
            valend -= SHORT_FIELD_WIDTH;
        }
#ifdef DEBUG_PRINT_CELL_VERTS
        debug4 << "Cell verts: ";
        for(int i = 0; i < cellNumNodes; ++i)
            debug4 << ", " << verts[i];
        debug4 << endl;
#endif
        ugrid->InsertNextCell(IVToVTKCell[IV], cellNumNodes,
                              verts);
    }
    else
    {
        debug5 << "Invalid cell type: " << IV << endl;
        retval = false;
    }

    return retval;
}

// ****************************************************************************
// Method: ProcessComponent
//
// Purpose: 
//   Processes the line for cell ids to add to the specified component.
//
// Arguments:
//   line               : The line from which to read the cell ids.
//   compno             : The component to which the cells will belong.
//   ML                 : The component list array, as a ** so we can resize
//                        if required.
//  componentListSize   : The size of the array pointed to by ML.
//  cellid_to_cellindex : The cellid to cellindex map.
//
// Note:       This is not compiled on the MDSERVER.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 29 16:32:31 PST 2005
//
// Modifications:
//   
// ****************************************************************************

inline void
ProcessComponent(char *line, const int compno, const int ncomps, 
    int **ML, int &componentListSize,
    const std::map<int,int> &cellid_to_cellindex)
{
    char *valstart = line + ((ncomps << 1) - 1) * SHORT_FIELD_WIDTH;
    char *valend = valstart + SHORT_FIELD_WIDTH;
    int cellIndices[5];
    int comp, compsFound = 0;
    for(comp = 0; comp < ncomps; ++comp)
    {
        // Get the cellid.
        *valend = '\0';
        int cellid = atoi(valstart)-1;

        // Try and map that cellid back to a cell index. If we can do the
        // mapping then save the cell index.
        std::map<int,int>::const_iterator cellindex =
            cellid_to_cellindex.find(cellid);
        if(cellindex != cellid_to_cellindex.end())
            cellIndices[compsFound++] = cellindex->second;

        valstart -= 2*SHORT_FIELD_WIDTH;
        valend -= 2*SHORT_FIELD_WIDTH;
    }

    if(compsFound < ncomps)
    {
        debug4 << "Expected " << ncomps << " but found only " << compsFound
               << ". This means that some cellid's in the named component "
               << "section of the file are invalid!";
    }

    // Find the maximum cellid
    int *componentList = *ML;
    int maxid = cellIndices[0];
    for(comp = 1; comp < compsFound; ++comp)
        maxid = (maxid > cellIndices[comp]) ? maxid : cellIndices[comp];

    // Resize if we can't fit the maxid.
    if(maxid >= componentListSize)
    {
        int newSize = int(float(maxid) * 1.25);
        int *newcomponentList = new int[newSize];
        int *dest = newcomponentList;
        int S = newSize;
        if(componentList != 0)
        {
            dest = newcomponentList + componentListSize;
            S = (maxid - componentListSize);
        }
        for(int i = 0; i < S; ++i)
            componentList[i] = 1;

        if(componentList != 0)
        {
            memcpy(newcomponentList, componentList, sizeof(int)*componentListSize);
            delete [] componentList;
        }
        componentList = newcomponentList;
        *ML = newcomponentList;

        debug4 << "Resizing componentList from " << componentListSize;
        componentListSize = newSize;
        debug4 << " to " << componentListSize << endl;
    }

    for(comp = 0; comp < compsFound; ++comp)
        componentList[cellIndices[comp]] = compno;
}
#endif

// ****************************************************************************
// Method: avtPATRANFileFormat::ReadFile
//
// Purpose: 
//   This method reads the PATRAN file and constructs a dataset that gets
//   returned later in the GetMesh method.
//
// Arguments:
//   name   : The name of the file to read.
//   nLines : The max number of lines to read from the file.
//
// Returns:    True if the file looks like a PATRAN bulk data file.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 28 13:53:21 PST 2005
//
// Modifications:
//   
//    Mark C. Miller, Thu Mar 30 16:45:35 PST 2006
//    Made it use VisItStat instead of stat
//
// ****************************************************************************

bool
avtPATRANFileFormat::ReadFile(const char *name, int nLines)
{
    int total = visitTimer->StartTimer();
    debug4 << "avtPATRANFileFormat::ReadFile(" << name << ", "
           << nLines << ")" << endl;

    // Open the file.
    ifstream ifile(name);
    if (ifile.fail())
    {
        visitTimer->StopTimer(total, "Loading PATRAN file");
        EXCEPTION1(InvalidFilesException, name);
    }

    // Determine the file size and come up with an estimate of the
    // number of vertices and cells so we can size the points and cells.
    int guessingSize = visitTimer->StartTimer();
    int nPoints = 100;
    int nCells = 100;
    if(nLines == ALL_LINES)
    {
        VisItStat_t statbuf;
        VisItStat(name, &statbuf);
        VisItOff_t fileSize = statbuf.st_size;

        // Make a guess about the number of cells and points based on
        // the size of the file.
        nPoints = fileSize / (VisItOff_t) 220;
        nCells  = fileSize / (VisItOff_t) 320;
    }
    visitTimer->StopTimer(guessingSize, "Determining file size");
    int initArrays = visitTimer->StartTimer();
    vtkPoints *pts = vtkPoints::New();
    pts->Allocate(nPoints);
#ifdef USE_POINT_INDICES_TO_INSERT
    pts->SetNumberOfPoints(nPoints);
#endif

    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->SetPoints(pts);
    ugrid->Allocate(nCells);
    pts->Delete();

    vtkFloatArray *emats = vtkFloatArray::New();
    vtkFloatArray *cids = vtkFloatArray::New();

    bool have_cell_data = false;
    for(int i = 0; i < MAX_CELL_PROPERTIES; ++i)
    {
        if(properties[i] != 0)
            properties[i]->Delete();

        char tmpname[100];
        SNPRINTF(tmpname, 100, "property%d", i+1);
        properties[i] = vtkFloatArray::New();
        properties[i]->SetName(tmpname);
#ifndef MDSERVER
        properties[i]->Allocate(nCells);
        memset(properties[i]->GetVoidPointer(0), 0, sizeof(float)*nCells);
#endif
    }

#ifndef MDSERVER
    // Allocate memory for the componentList, elementMats, elementIds.
    emats->SetNumberOfComponents(1);
    emats->Allocate(nCells);
    cids->SetNumberOfComponents(1);
    cids->Allocate(nCells);

    debug4 << "Starting with " << nCells << " cells as a guess" << endl;
    // Initialize the componentsList array, to all material 1, which is
    // "default" We use componentList to cells that are members of the
    // same named component.
    componentList = new int[nCells];
    for(int i = 0; i < nCells; ++i)
        componentList[i] = 1;
    componentListSize = nCells;

    std::map<int, int> cellid_to_cellindex;
#endif

    // We will always have the "default" component name to absorb cells
    // that don't ever get put in a component in the named component
    // section of the file.
    componentNames.push_back("default");
    visitTimer->StopTimer(initArrays, "Initializing arrays.");

    char  line[1024];
    bool  recognized = false;
    int   cellNumNodes = 0;
    float cellMatno = 1.f;
    int cellCount = 0;
    int   card = 0;
    int   IT, // Packet type
          ID, // Identification number. "0" means not applicable.
          IV, // Additional ID. "0" means not applicable.
          KC, // Number of data cards after the header.
          N1; // Supplemental integer values.

    int readingFile = visitTimer->StartTimer();
    for(int lineIndex = 0; !ifile.eof(); ++lineIndex)
    {
        if(nLines != ALL_LINES && lineIndex >= nLines)
            break;

        // Get the line
        ifile.getline(line, 1024);

        // Determine what the line is for.
        if(card == 0)
        {
            char *valstart = line + 2 + 3 * SHORT_FIELD_WIDTH;
            char *valend = valstart + SHORT_FIELD_WIDTH;

            // Ignore N2,N3,N4,N5 for now.

            *valend = '\0';
            N1 = atoi(valstart);

            valstart -= SHORT_FIELD_WIDTH;
            valend -= SHORT_FIELD_WIDTH;
            *valend = '\0';
            KC = atoi(valstart);

            valstart -= SHORT_FIELD_WIDTH;
            valend -= SHORT_FIELD_WIDTH;
            *valend = '\0';
            IV = atoi(valstart);

            valstart -= SHORT_FIELD_WIDTH;
            valend -= SHORT_FIELD_WIDTH;
            *valend = '\0';
            ID = atoi(valstart);

            line[2] = '\0';
            IT = atoi(line);

#ifdef DEBUG_PRINT_HEADER
            debug4 << "IT=" << IT
                   << ", ID=" << ID
                   << ", IV=" << IV
                   << ", KC=" << KC
                   << ", N1=" << N1
                   << endl;
#endif

            // Read the values for IT, ID, IV, KC out of the line.
            ++card;
        }
        else
        {
            if(IT == PATRAN_PACKET_NODE_DATA)
            {
                if(card == 1)
                {
                    recognized = true;
#ifndef MDSERVER
                    AddPoint(line, ID-1, nPoints, pts);
#endif
                }
                // Don't bother interpreting card 2.
            }
#ifndef MDSERVER
            else if(IT == PATRAN_PACKET_ELEMENT_DATA)
            {
                if(card == 1)
                {
                    // Get this cell's material number.
                    char *valstart = line + 2 * SHORT_FIELD_WIDTH;
                    char *valend = valstart + SHORT_FIELD_WIDTH;
                    *valend = '\0';
                    cellMatno = atof(valstart);

                    // Get the number of nodes in this cell.
                    line[SHORT_FIELD_WIDTH] = '\0';
                    cellNumNodes = atoi(line);
                }
                else if(card == 2)
                {
                    // Store the cell index associated with this cell's
                    // id because we need it later to correctly create
                    // the fragments.
                    if(AddCell(line, cellNumNodes, IV, ugrid))
                    {
                        emats->InsertNextTuple(&cellMatno);
                        float fid = (float)ID;
                        cids->InsertNextTuple(&fid);

                        cellid_to_cellindex[ID-1] = cellCount++;
                    }

                    if(N1 == 0)
                        card = KC;
                }
                // Don't bother interpreting card 3 if it exists.
            }
#endif
            else if(IT == PATRAN_PACKET_NAMED_COMPONENTS)
            {
                if(card == 1)
                {
                    componentNames.push_back(line);
#ifdef DEBUG_PRINT_MATERIAL_NAMES
                    debug4 << "partName = " << line << endl;
#endif
                }
#ifndef MDSERVER
                // Don't process the material numbers in the mdserver.
                else
                {
                    int partno = componentNames.size();
                    int nfull_lines = IV / 10;
                    int nparts_this_line;
                    if(card-2 < nfull_lines)
                        nparts_this_line = 5;
                    else
                        nparts_this_line = (IV % 10) / 2;
                    ProcessComponent(line, partno, nparts_this_line,
                                     &componentList, componentListSize,
                                     cellid_to_cellindex);
                }
#endif
            }
            else if(IT == PATRAN_PACKET_CELL_DATA)
            {
                have_cell_data = true;
#ifndef MDSERVER
                char *valstart = line + (MAX_CELL_PROPERTIES-1) * LONG_FIELD_WIDTH;
                char *valend = valstart + LONG_FIELD_WIDTH;
                for(int col = MAX_CELL_PROPERTIES-1; col >= 0; col--)
                {
                    *valend = '\0';
                    double tmp = atof(valstart);
                    valstart -= LONG_FIELD_WIDTH;
                    valend -= LONG_FIELD_WIDTH;
                    properties[col]->InsertNextTuple(&tmp);
                }
#endif
            }
            else if(IT == PATRAN_PACKET_TITLE)
            {
                title = line;
                recognized = true;
            }
            else if(IT == PATRAN_PACKET_SUMMARY)
            {
                title += "\n";
                title += line;
                recognized = true;
            }

            if(++card > KC)
                card = 0;
        }        
    }

    visitTimer->StopTimer(readingFile, "Interpreting PATRAN file");

    //
    // If we recognized the file format and we're reading all of the file
    // then save the mesh and arrays that we created. Otherwise, delete them.
    //
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
        elementMats = emats;
        elementIds = cids;

        // Log the cell count.
        debug4 << "cellCount = " << cellCount << endl;
    }
    else
    {
        ugrid->Delete();
        emats->Delete();
        cids->Delete();
    }

    // If we did not find any cell data, delete them.
    if(!have_cell_data)
    {
        for(int i = 0; i < MAX_CELL_PROPERTIES; ++i)
        {
            if(properties[i] != 0)
            {
                properties[i]->Delete();
                properties[i] = 0;
            }
        }
    }

    visitTimer->StopTimer(total, "Loading PATRAN file");

    return recognized;
}

// ****************************************************************************
//  Method: avtPATRANFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jul 28 13:53:21 PST 2005
//
//  Modifications:
//    Brad Whitlock, Mon Jun  8 10:09:05 PDT 2009
//    I added properties, which are cell data.
//
// ****************************************************************************

void
avtPATRANFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    debug4 << "avtPATRANFileFormat::PopulateDatabaseMetaData" << endl;
    md->SetDatabaseComment(title);

    avtMeshMetaData *mmd = new avtMeshMetaData;
    mmd->name = "mesh";
    mmd->cellOrigin = 1;
    mmd->meshType = AVT_UNSTRUCTURED_MESH;
    mmd->spatialDimension = 3;
    mmd->topologicalDimension = 3;
    md->Add(mmd);

    // Add the material numbers as a zonal array.
    md->Add(new avtScalarMetaData(VARNAME_ELEMENTMATS, "mesh", AVT_ZONECENT));
    // Add the cellid's as a zonal array.
    md->Add(new avtScalarMetaData(VARNAME_ELEMENTIDS, "mesh", AVT_ZONECENT));

    // Only advertise a material if we have fragment names.
    if(componentNames.size() > 0)
    {
        avtMaterialMetaData *mmd = new avtMaterialMetaData("named_components",
            "mesh", componentNames.size(), componentNames);
        md->Add(mmd);
    }

    // Add cell data.
    for(int i = 0; i < MAX_CELL_PROPERTIES; ++i)
    {
        if(properties[i] != 0)
        {
            md->Add(new avtScalarMetaData(properties[i]->GetName(), "mesh",
                                          AVT_ZONECENT));
        }
    }
}


// ****************************************************************************
//  Method: avtPATRANFileFormat::GetMesh
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
//  Creation:   Thu Jul 28 13:53:21 PST 2005
//
// ****************************************************************************

vtkDataSet *
avtPATRANFileFormat::GetMesh(const char *meshname)
{
    debug4 << "avtPATRANFileFormat::GetMesh" << endl;
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->ShallowCopy(meshDS);

#if 0
    // If we have elementIds then copy them to the ugrid as 
    // avtOriginalCellNumbers because some datasets have weird cell ids that
    // start at really high numbers. Normally the cellids start at 1.
    if(elementIds != 0)
    {
        vtkFloatArray *f = vtkFloatArray::New();
        f->DeepCopy(elementIds);
        f->SetName("avtOriginalCellNumbers");
        ugrid->GetCellData()->AddArray(f);
        f->Delete();
    }
#endif

    return ugrid;
}


// ****************************************************************************
//  Method: avtPATRANFileFormat::GetVar
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
//  Creation:   Thu Jul 28 13:53:21 PST 2005
//
//  Modifications:
//    Brad Whitlock, Mon Jun  8 10:12:06 PDT 2009
//    Return the properties arrays if they are requested.
//
// ****************************************************************************

vtkDataArray *
avtPATRANFileFormat::GetVar(const char *varname)
{
    debug4 << "avtPATRANFileFormat::GetVar: varname=" << varname << endl;

    vtkDataArray *arr = 0;

    if(strcmp(varname, VARNAME_ELEMENTMATS) == 0 && elementMats != 0)
    {
        vtkFloatArray *f = vtkFloatArray::New();
        f->DeepCopy(elementMats);
        arr = f;
    }
    else if(strcmp(varname, VARNAME_ELEMENTIDS) == 0 && elementIds != 0)
    {
        vtkFloatArray *f = vtkFloatArray::New();
        f->DeepCopy(elementIds);
        arr = f;
    }
    else
    {
        for(int i = 0; i < MAX_CELL_PROPERTIES; ++i)
        {
            if(properties[i] != 0 && 
               strcmp(properties[i]->GetName(), varname) == 0)
            {
                properties[i]->Register(NULL);
                arr = properties[i];
                break;
            }
        }
    }

    return arr;
}


// ****************************************************************************
//  Method: avtPATRANFileFormat::GetVectorVar
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
//  Creation:   Thu Jul 28 13:53:21 PST 2005
//
// ****************************************************************************

vtkDataArray *
avtPATRANFileFormat::GetVectorVar(const char *varname)
{
    debug4 << "avtPATRANFileFormat::GetVectorVar" << endl;

    // Can't read variables yet.
    EXCEPTION1(InvalidVariableException, varname);
}

// ****************************************************************************
// Method: avtPATRANFileFormat::GetAuxiliaryData
//
// Purpose: 
//   Gets the material.
//
// Arguments:
//   var  : The name of the variable to get.
//   type : The type of variable to get.
//   args : not used.
//   df   : The destructor function to use when deleting the returned data.
//
// Returns:    avtMaterial or 0.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 29 16:24:43 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void  *
avtPATRANFileFormat::GetAuxiliaryData(const char *var, const char *type,
    void *args, DestructorFunction &df)
{
    debug4 << "avtPATRANFileFormat::GetAuxiliaryData: "
           << "var=" << var
           << ", type=" << type
           << endl;

    avtMaterial *retval = 0;
    if(strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        df = avtMaterial::Destruct;

        int *matnos = new int[componentNames.size()];
        char **names = new char *[componentNames.size()];
        for(int i = 0; i < componentNames.size(); ++i)
        {
            matnos[i] = i + 1;
            names[i] = (char *)componentNames[i].c_str();
        }

        int dims[3]; 
        dims[0] = meshDS->GetNumberOfCells();
        dims[1] = 1;
        dims[2] = 1;
        retval = new avtMaterial(
            componentNames.size(),
            matnos,
            names,
            3,
            dims,
            0,
            componentList,
            0,
            0,
            0,
            0,
            0
            );
    }

    debug4 << "avtPATRANFileFormat::GetAuxiliaryData: end" << endl;

    return retval;
}
