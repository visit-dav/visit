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
//                            avtDyna3DFileFormat.C                          //
// ************************************************************************* //

#include <avtDyna3DFileFormat.h>

#include <set>
#include <map>
#include <snprintf.h>

#include <vtkCellType.h>
#include <vtkFloatArray.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>
#include <avtMaterial.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>

#include <Expression.h>

#include <TimingsManager.h>
#include <DebugStream.h>
#include <Utility.h>


using     std::string;

#define ALL_LINES -1

// ****************************************************************************
//  Method: avtDyna3DFileFormat constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Nov 27 16:25:13 PST 2006
//
//  Modifications:
//
// ****************************************************************************

avtDyna3DFileFormat::avtDyna3DFileFormat(const char *filename)
    : avtSTSDFileFormat(filename), materialCards()
{
    meshDS = 0;
    memset(&cards, 0, sizeof(ControlCards_t));
    matNumbers = 0;
    line = 0;
    velocity = 0;

#ifdef MDSERVER
    if(!ReadFile(filename, 100))
    {
        EXCEPTION1(InvalidFilesException, filename);
    }
#endif
}

// ****************************************************************************
// Method: avtDyna3DFileFormat::~avtDyna3DFileFormat
//
// Purpose: 
//   Destructor for avtDyna3DFileFormat class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 27 16:25:13 PST 2006
//
// Modifications:
//   
// ****************************************************************************

avtDyna3DFileFormat::~avtDyna3DFileFormat()
{
    FreeUpResources();
}

// ****************************************************************************
//  Method: avtDyna3DFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Nov 27 16:25:13 PST 2006
//
// ****************************************************************************

void
avtDyna3DFileFormat::FreeUpResources(void)
{
    debug4 << "avtDyna3DFileFormat::FreeUpResources" << endl;
    if(meshDS)
    {
        meshDS->Delete();
        meshDS = 0;
    }

    if(matNumbers != 0)
    {
        delete [] matNumbers;
        matNumbers = 0;
    }

    if(line != 0)
    {
        delete [] line;
        line = 0;
    }

    if(velocity != 0)
    {
        velocity->Delete();
        velocity = 0;
    }
}

// ****************************************************************************
// Method: avtDyna3DFileFormat::ActivateTimestep
//
// Purpose: 
//   Called when we're activating the current time step.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 27 16:25:13 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtDyna3DFileFormat::ActivateTimestep()
{
    debug4 << "avtDyna3DFileFormat::ActivateTimestep" << endl;
#ifndef MDSERVER
    if(meshDS == 0)
    {
        ReadFile(filename, ALL_LINES);
    }
#endif
}

// ****************************************************************************
// Method: avtDyna3DFileFormat::ReadControlCards
//
// Purpose: 
//   Reads all of the control cards.
//
// Arguments:
//   ifile : The file stream from which to read.
//
// Returns:    True if the file looks like DYNA3D; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 27 16:23:44 PST 2006
//
// Modifications:
//   
// ****************************************************************************

bool
avtDyna3DFileFormat::ReadControlCards(ifstream &ifile)
{
    bool ret = ReadControlCard2(ifile);
    ReadControlCard3(ifile);
    ReadControlCard4(ifile);
    ReadControlCard5(ifile);
    ReadControlCard6(ifile);
    ReadControlCard7(ifile);
    ReadControlCard8(ifile);
    ReadControlCard9(ifile);
    return ret;
}

// ****************************************************************************
// Method: avtDyna3DFileFormat::GetLine
//
// Purpose: 
//   Reads a line from the file and puts it in the line member array.
//
// Arguments:
//   ifile : The stream from which to read lines.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 5 11:53:23 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtDyna3DFileFormat::GetLine(ifstream &ifile)
{
    ifile.getline(line, 1024);
}

// ****************************************************************************
// Method: avtDyna3DFileFormat::SkipComments
//
// Purpose: 
//   Skips comments.
//
// Arguments:
//   ifile       : The stream from which to read lines.
//   sectionName : The name of the last section to be read.
//
// Returns:    True if the name of the last section that was encountered
//             is sectionName; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 17:09:36 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtDyna3DFileFormat::SkipComments(ifstream &ifile, const char *sectionName,
    bool &sectionNameFound, bool &sectionNameIsCurrent)
{
    sectionNameFound = false;
    sectionNameIsCurrent = false;
    debug5 << "**************** SECTION begin *****************" << endl;
    do
    {
        GetLine(ifile);
        if(strstr(line, "------") != NULL)
        {
            sectionNameIsCurrent = (strstr(line, sectionName) != NULL);
            sectionNameFound |= sectionNameIsCurrent;
        }
    } while(line[0] == '*');
    debug5 << "**************** SECTION end *****************" << endl;
}

// ****************************************************************************
// Method: avtDyna3DFileFormat::SkipToSection
//
// Purpose: 
//   Skip to the named section and return true if it was found.
//
// Arguments:
//   ifile   : The stream from which to read lines.
//   section : The name of the section to look for.
//
// Returns:    True if the section was found.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 5 11:54:12 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
avtDyna3DFileFormat::SkipToSection(ifstream &ifile, const char *section)
{
    bool sectionFound = false;
    do
    {
        GetLine(ifile);
        sectionFound = (strstr(line, "------") != NULL &&
                        strstr(line, section) != NULL);
    } while(!ifile.eof() && !sectionFound);
    if(sectionFound)
    {
        do
        {
            GetLine(ifile);
        } while(line[0] == '*');
    }
    return sectionFound;
}

// ****************************************************************************
// Method: avtDyna3DFileFormat::ReadControlCard2
//
// Purpose: 
//   Read control card 2 since it contains information about the number
//   of points, cells, etc.
//
// Arguments:
//   ifile : The file stream from which to read.
//
// Returns:    True if the file looks like a DYNA3D file.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 27 16:23:01 PST 2006
//
// Modifications:
//   
// ****************************************************************************

bool
avtDyna3DFileFormat::ReadControlCard2(ifstream &ifile)
{
    // Skip the comments but search them for "DYNA3D".
    bool isDyna, tmp;
    SkipComments(ifile, "DYNA3D", isDyna, tmp);

    // Process the card line.
    bool err = false;
    if(sscanf(line, "%5d%10d%10d%10d%10d%10d%10d+", 
        &cards.card2.nMaterials,
        &cards.card2.nPoints,
        &cards.card2.nSolidHexes,
        &cards.card2.nBeamElements,
        &cards.card2.nShellElements4,
        &cards.card2.nShellElements8,
        &cards.card2.nInterfaceSegments) != 7)
    {
        err = true;
    }

    if(sscanf(line+65, "%g %g", 
        &cards.card2.interfaceInterval,
        &cards.card2.shellTimestep) != 2)
    {
        err = true;
    }
    
    if(err)
    {
        debug1 << "Error reading card 2:" << line << endl;
    }

    debug4 << "\tcards.card2.nMaterials=" << cards.card2.nMaterials << ",\n";
    debug4 << "\tcards.card2.nPoints=" << cards.card2.nPoints << ",\n";
    debug4 << "\tcards.card2.nSolidHexes=" << cards.card2.nSolidHexes << ",\n";
    debug4 << "\tcards.card2.nBeamElements=" << cards.card2.nBeamElements << ",\n";
    debug4 << "\tcards.card2.nShellElements4=" << cards.card2.nShellElements4 << ",\n";
    debug4 << "\tcards.card2.nShellElements8=" << cards.card2.nShellElements8 << ",\n";
    debug4 << "\tcards.card2.nInterfaceSegments=" << cards.card2.nInterfaceSegments << "\n";
    debug4 << "\tcards.card2.interfaceInterval=" << cards.card2.interfaceInterval << ",\n";
    debug4 << "\tcards.card2.shellTimestep=" << cards.card2.shellTimestep << "\n";

    return isDyna;
}

//
// Fill these in later if we have to understand more control data.
//

void
avtDyna3DFileFormat::ReadControlCard3(ifstream &ifile)
{
    // Skip the comments
    bool b0,b1; SkipComments(ifile, "CONTROL CARD #3", b0, b1);

    // Process the card line.
}

void
avtDyna3DFileFormat::ReadControlCard4(ifstream &ifile)
{
    // Skip the comments
    bool b0,b1; SkipComments(ifile, "CONTROL CARD #4", b0, b1);

    // Process the card line.
}

void
avtDyna3DFileFormat::ReadControlCard5(ifstream &ifile)
{
    // Skip the comments
    bool b0,b1; SkipComments(ifile, "CONTROL CARD #5", b0, b1);

    // Process the card line.
}

void
avtDyna3DFileFormat::ReadControlCard6(ifstream &ifile)
{
    // Skip the comments
    bool b0,b1; SkipComments(ifile, "CONTROL CARD #6", b0, b1);

    // Process the card line.
}

void
avtDyna3DFileFormat::ReadControlCard7(ifstream &ifile)
{
    // Skip the comments
    bool b0,b1; SkipComments(ifile, "CONTROL CARD #7", b0, b1);

    // Process the card line.
}

void
avtDyna3DFileFormat::ReadControlCard8(ifstream &ifile)
{
    // Skip the comments
    bool b0,b1; SkipComments(ifile, "CONTROL CARD #8", b0, b1);

    // Process the card line.
}

void
avtDyna3DFileFormat::ReadControlCard9(ifstream &ifile)
{
    bool b0,b1; SkipComments(ifile, "CONTROL CARD #9", b0, b1);

    // If the next line is not a comment then assume that we have
    // some commands that will terminate with "endfree".
    GetLine(ifile);
    if(line[0] != '*')
    {
        do 
        {
            if(strncmp(line, "endfree", 7) == 0)
                break;
            GetLine(ifile);
        } while(!ifile.eof());
    }
}

// ****************************************************************************
// Method: avtDyna3DFileFormat::ReadMaterialCards
//
// Purpose: 
//   Read the material cards.
//
// Arguments:
//   ifile : The file stream to read.
//
// Returns:    
//
// Note:       Only the names and material numbers are currently used.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 27 16:22:08 PST 2006
//
// Modifications:
//   Brad Whitlock, Fri Aug  1 11:49:57 PDT 2008
//   Added code to read material strength.
//
// ****************************************************************************

void
avtDyna3DFileFormat::ReadMaterialCards(ifstream &ifile)
{
    // Skip the comments at the start of the section
    bool matCards, currentlyInMatCards;
    SkipComments(ifile, "MATERIAL CARDS", matCards, currentlyInMatCards);
    if(!currentlyInMatCards)
    {
        debug5 << "Returning without reading any materials." << endl;
        return;
    }
    debug5 << "Reading materials..." << endl;

    std::set<std::string> uniqueNames;
    for(int i = 0; i < cards.card2.nMaterials; ++i)
    {
        MaterialCard_t mat;

        // Read the first line of a material card.
        if(i > 0)
        {
            // Take care of comments.
            do
            {
                GetLine(ifile);
            } while(line[0] == '*');
        }
        
        sscanf(line, "%d", &mat.materialNumber);

        // Get the material density out of cols 11-20.
        line[20] = '\0';
        mat.density = atof(line + 10);

        // Read the name line of the material card and strip out
        // extra spaces from the name.
        GetLine(ifile);
        char matNameBuf[1024];
        memset(matNameBuf, 0, 1024);
        char *s = matNameBuf, *ptr = line;
        while(*ptr != '\0' && ((s - matNameBuf) < 1024))
        {
            if(*ptr == ' ')
            {
               *s++ = *ptr;
               while(*ptr == ' ')
                   ptr++;
            }
            else if(*ptr != '\n')
            {
               *s++ = *ptr++;
            }
        }

        // Make sure that the material name is unique.
        std::string matName(matNameBuf);
        if(uniqueNames.find(matName) != uniqueNames.end())
        {
            // Find a unique name.
            int index = 2;
            std::string tmpName(matName);
            do
            {
                SNPRINTF(matNameBuf, 1024, "%s %d", matName.c_str(), index++);
                debug4 << "Searching for " << matNameBuf << endl;
                tmpName = std::string(matNameBuf);
            } while(uniqueNames.find(tmpName) != uniqueNames.end());
            matName = tmpName;
        }

        uniqueNames.insert(matName);
        mat.materialName = matName;

        // Get the next line since it contains the strength
        GetLine(ifile);
        double tmp;
        sscanf(line, "%lg %lg", &tmp, &mat.strength);

        // Read past the other material information.
        for(int j = 0; j < 9; ++j)
            GetLine(ifile);

        // Add to the list of materials.
        materialCards.push_back(mat);

        debug4 << "Added material (" << mat.materialNumber << "): "
               << mat.materialName.c_str() << ", density="
               << mat.density << ", strength=" << mat.strength << endl;
    }    
}


// ****************************************************************************
// Method: avtDyna3DFileFormat::ReadFile
//
// Purpose: 
//   This method reads the Dyna3D file and constructs a dataset that gets
//   returned later in the GetMesh method.
//
// Arguments:
//   name   : The name of the file to read.
//   nLines : The max number of lines to read from the file.
//
// Returns:    True if the file looks like a Dyna3D bulk data file.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 27 16:25:13 PST 2006
//
// Modifications:
//
// ****************************************************************************

bool
avtDyna3DFileFormat::ReadFile(const char *name, int nLines)
{
    const char *mName = "avtDyna3DFileFormat::ReadFile: ";
    int total = visitTimer->StartTimer();
    debug4 << mName << endl;

    // Open the file.
    ifstream ifile(name);
    if (ifile.fail())
    {
        EXCEPTION1(InvalidFilesException, name);
    }

    // Allocate a buffer that we'll use for reading lines.
    if(line == 0)
    {
        line = new char[1024];
        memset(line, 0, 1024 * sizeof(char));
    }

    // Skip line 1
    GetLine(ifile);

    int readCards = visitTimer->StartTimer();
    bool recognized = ReadControlCards(ifile);
    ReadMaterialCards(ifile);
    visitTimer->StopTimer(readCards, "Reading control and material cards.");
    
    if(recognized)
    {
        if(nLines == ALL_LINES)
        {
            int readingFile = visitTimer->StartTimer();
            int nPoints = cards.card2.nPoints;
            int nCells = cards.card2.nSolidHexes;
            debug4 << mName << "File is DYNA3D and we're reading the mesh. "
                   << "nPoints=" << nPoints << ", nCells=" << nCells << endl;

            vtkPoints *pts = vtkPoints::New();
            pts->Allocate(nPoints);

            vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
            ugrid->SetPoints(pts);
            ugrid->Allocate(nCells);
            pts->Delete();

            // Read nodes
            double pt[3];
            for(int node = 0; node < nPoints;)
            {
                // In the event that we skipped reading materials then
                // we can re-use the line that we read already.
                if(node > 0)
                    GetLine(ifile);
                else if(materialCards.size() > 0)
                    GetLine(ifile);

                if(line[0] != '*')
                {
                    // It's a valid point line.
                    char *valstart = line + 53;
                    char *valend = valstart + 73;
                    *valend = '\0';
                    pt[2] = atof(valstart);
    
                    valstart -= 20;
                    valend   -= 20;
                    *valend = '\0';
                    pt[1] = atof(valstart);

                    valstart -= 20;
                    valend   -= 20;
                    *valend = '\0';
                    pt[0] = atof(valstart);

                    // Insert the point.
                    pts->InsertNextPoint(pt);
                    ++node;
                }
            }

            // Allocate material numbers.
            if(materialCards.size() > 0)
            {
                matNumbers = new int[nCells];
                memset(matNumbers, 0, sizeof(int) * nCells);
            }

            //
            // Read connectivity.
            //
            int verts[8];
            for(int cellid = 0; cellid < nCells;)
            {
                GetLine(ifile);
                if(line[0] != '*')
                {
#define INDEX_FIELD_WIDTH 8
                    char *valstart = line + 69;
                    char *valend = valstart + INDEX_FIELD_WIDTH;
                    *valend = '\0';
                    verts[7] = atoi(valstart)-1;
    
                    valstart -= INDEX_FIELD_WIDTH;
                    valend -= INDEX_FIELD_WIDTH;
                    *valend = '\0';
                    verts[6] = atoi(valstart)-1;
    
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
    
                    // Insert the hex cell.
                    ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, verts);
    
                    // Save the material number
                    if(matNumbers != 0)
                    {
                        line[13] = '\0';
                        matNumbers[cellid] = atoi(line+11);
                    }
                    ++cellid;
                }
            }

            // Skip ahead to the initial conditions section.
            bool inIC = SkipToSection(ifile, "INITIAL CONDITIONS");
            if(inIC)
            {
                velocity = vtkFloatArray::New();
                velocity->SetNumberOfComponents(3);
                velocity->SetNumberOfTuples(nPoints);
                double vel[3];
                for(int node = 0; node < nPoints; ++node)
                {
                    if(node > 0)
                        GetLine(ifile);

#define VEL_FIELD_WIDTH 10
                    char *valstart = line + 28;
                    char *valend = valstart + VEL_FIELD_WIDTH;
                    *valend = '\0';
                    vel[2] = atof(valstart);
    
                    valstart -= VEL_FIELD_WIDTH;
                    valend -= VEL_FIELD_WIDTH;
                    *valend = '\0';
                    vel[1] = atof(valstart);
    
                    valstart -= VEL_FIELD_WIDTH;
                    valend -= VEL_FIELD_WIDTH;
                    *valend = '\0';
                    vel[0] = atof(valstart);

                    velocity->SetTuple(node, vel);
                }
            }

            meshDS = ugrid;
        }
        else
        {
            debug4 << mName << "File is DYNA3D." << endl;

            // Skip ahead to the initial conditions section. If we can
            // then create a velocity object so we can add a velocity 
            // variable to the metadata.
            if(SkipToSection(ifile, "INITIAL CONDITIONS"))
            {
                velocity = vtkFloatArray::New();
                velocity->SetNumberOfTuples(1);
            }
        }
    }
    else
    {
        debug4 << mName << "The file is not DYNA3D" << endl;
    }

    visitTimer->StopTimer(total, "Loading Dyna3D file");

    return recognized;
}

// ****************************************************************************
//  Method: avtDyna3DFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Nov 27 16:25:13 PST 2006
//
//  Modifications:
//    Brad Whitlock, Fri Aug  1 11:44:37 PDT 2008
//    Added strength.
//
// ****************************************************************************

void
avtDyna3DFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    debug4 << "avtDyna3DFileFormat::PopulateDatabaseMetaData" << endl;
    int sdim = 3;
    int tdim = 3;
    AddMeshToMetaData(md, "mesh", AVT_UNSTRUCTURED_MESH, NULL, 1, 1,
                      sdim, tdim);

    // Only advertise a material if we have material names.
    if(materialCards.size() > 0)
    {
        stringVector materialNames;
        for(int i = 0; i < materialCards.size(); ++i)
            materialNames.push_back(materialCards[i].materialName);

        avtMaterialMetaData *mmd = new avtMaterialMetaData("material",
            "mesh", materialNames.size(), materialNames);
        md->Add(mmd);

        // We had material names so advertise a density field.
        avtScalarMetaData *smd = new avtScalarMetaData(
                "density", "mesh", AVT_ZONECENT);
        smd->hasUnits = true;
        smd->units = "g/cc";
        md->Add(smd);

        // We had material names so advertise a strength field.
        smd = new avtScalarMetaData(
                "strength", "mesh", AVT_ZONECENT);
        md->Add(smd);

        // We had material names so advertise the material number.
        smd = new avtScalarMetaData(
                "matno", "mesh", AVT_ZONECENT);
        smd->hasUnits = true;
        smd->units = "material number";
        md->Add(smd);
    }

    // Add a velocity vector if we were able to find one.
    if(velocity != 0)
    {
        avtVectorMetaData *vmd = new avtVectorMetaData(
                "velocity", "mesh", AVT_NODECENT, 3);
        md->Add(vmd);

        Expression vx, vy, vz;
        vx.SetName("vx");
        vx.SetDefinition("velocity[0]");
        vx.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&vx);
        vy.SetName("vy");
        vy.SetDefinition("velocity[1]");
        vy.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&vy);
        vz.SetName("vz");
        vz.SetDefinition("velocity[2]");
        vz.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&vz);
    }
}


// ****************************************************************************
//  Method: avtDyna3DFileFormat::GetMesh
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
//  Creation:   Mon Nov 27 16:25:13 PST 2006
//
// ****************************************************************************

vtkDataSet *
avtDyna3DFileFormat::GetMesh(const char *meshname)
{
    debug4 << "avtDyna3DFileFormat::GetMesh" << endl;
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    ugrid->ShallowCopy(meshDS);

    return ugrid;
}


// ****************************************************************************
//  Method: avtDyna3DFileFormat::GetVar
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
//  Creation:   Mon Nov 27 16:25:13 PST 2006
//
//  Modifications:
//    Brad Whitlock, Fri Aug  1 14:10:33 PDT 2008
//    Added support for strength.
//
// ****************************************************************************

vtkDataArray *
avtDyna3DFileFormat::GetVar(const char *varname)
{
    debug4 << "avtDyna3DFileFormat::GetVar" << endl;
    vtkDataArray *ret = 0;

    if((strcmp(varname, "density") == 0 || strcmp(varname, "strength") == 0) &&
       matNumbers != 0 && 
       materialCards.size() > 0)
    {
        std::map<int, double> matToDensity, matToStrength;
        for(int i = 0; i < materialCards.size(); ++i)
        {
            matToDensity[materialCards[i].materialNumber] = materialCards[i].density;
            matToStrength[materialCards[i].materialNumber] = materialCards[i].strength;
        }

        vtkFloatArray *arr = vtkFloatArray::New();
        arr->SetNumberOfTuples(meshDS->GetNumberOfCells());
        float *fptr = (float *)arr->GetVoidPointer(0); 
        if(strcmp(varname, "density") == 0)
        {
            for(vtkIdType id = 0; id < meshDS->GetNumberOfCells(); ++id)
               *fptr++ = (float)matToDensity[matNumbers[id]];
        }
        else
        {
            for(vtkIdType id = 0; id < meshDS->GetNumberOfCells(); ++id)
               *fptr++ = (float)matToStrength[matNumbers[id]];
        }

        ret = arr;
    }
    else if(strcmp(varname, "matno") == 0 &&
            matNumbers != 0)
    {
        vtkFloatArray *arr = vtkFloatArray::New();
        arr->SetNumberOfTuples(meshDS->GetNumberOfCells());
        float *fptr = (float *)arr->GetVoidPointer(0);
        for(vtkIdType id = 0; id < meshDS->GetNumberOfCells(); ++id)
        {
            *fptr++ = (float)matNumbers[id];
        }

        ret = arr;
    }
    else
    {
        // Bad variable name.
        EXCEPTION1(InvalidVariableException, varname);
    }

    return ret;
}


// ****************************************************************************
//  Method: avtDyna3DFileFormat::GetVectorVar
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
//  Creation:   Mon Nov 27 16:25:13 PST 2006
//
// ****************************************************************************

vtkDataArray *
avtDyna3DFileFormat::GetVectorVar(const char *varname)
{
    debug4 << "avtDyna3DFileFormat::GetVectorVar" << endl;
    vtkDataArray *ret = 0;

    if(strcmp(varname, "velocity") == 0 &&
       velocity != 0)
    {
        //
        // The routine that calls this method is going to assume that it 
        // can call Delete on what is returned.  That means we better add 
        // an extra reference.
        //
        velocity->Register(NULL);
        ret = velocity;
    }
    else
    {
        // Can't read variables yet.
        EXCEPTION1(InvalidVariableException, varname);
    }
    return ret;
}

// ****************************************************************************
// Method: avtDyna3DFileFormat::GetAuxiliaryData
//
// Purpose: 
//   Read the material from the database.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 27 16:21:34 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void  *
avtDyna3DFileFormat::GetAuxiliaryData(const char *var, const char *type,
    void *args, DestructorFunction &df)
{
    debug4 << "avtDyna3DFileFormat::GetAuxiliaryData: "
           << "var=" << var
           << ", type=" << type
           << endl;

    avtMaterial *retval = 0;
    if(strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        df = avtMaterial::Destruct;

        int *matnos = new int[materialCards.size()];
        char **names = new char *[materialCards.size()];
        for(int i = 0; i < materialCards.size(); ++i)
        {
            matnos[i] = materialCards[i].materialNumber;
            names[i] = (char *)materialCards[i].materialName.c_str();
        }

        int dims[3]; 
        dims[0] = meshDS->GetNumberOfCells();
        dims[1] = 1;
        dims[2] = 1;
        retval = new avtMaterial(
            materialCards.size(),
            matnos,
            names,
            3,
            dims,
            0,
            matNumbers,
            0,
            0,
            0,
            0,
            0
            );
    }

    debug4 << "avtDyna3DFileFormat::GetAuxiliaryData: end" << endl;

    return retval;
}
