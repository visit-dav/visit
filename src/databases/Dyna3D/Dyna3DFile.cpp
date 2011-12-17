// ************************************************************************* //
//                            Dyna3DFile.C                                   //
// ************************************************************************* //

#include <Dyna3DFile.h>

#include <string.h>

#include <set>
#include <map>

#include <vtkAppendFilter.h>
#include <vtkCell.h>
#include <vtkCellType.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkUnstructuredGrid.h>

#include <avtVector.h>


#define ALL_LINES -1

#ifdef VISIT_DYNA
// We're building this code inside of VisIt.
#include <snprintf.h>
#include <DebugStream.h>
#define DEBUG_READER(A) A
#else
#define SNPRINTF snprintf
#define DEBUG_READER(A)
#endif

// ****************************************************************************
//  Method: Dyna3DFile constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Nov 27 16:25:13 PST 2006
//
//  Modifications:
//
// ****************************************************************************

Dyna3DFile::Dyna3DFile()
{
    ugrid = 0;
    memset(&cards, 0, sizeof(ControlCards_t));
    matNumbers = 0;
    line = 0;
    velocity = 0;
}

// ****************************************************************************
//  Method: Dyna3DFile copy constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jun 10 14:11:46 PST 2010
//
//  Modifications:
//
// ****************************************************************************

Dyna3DFile::Dyna3DFile(const Dyna3DFile &obj) : ugrid(0), cards(obj.cards),
    materialCards(obj.materialCards), matNumbers(0), line(0), velocity(0)
{
    if(obj.ugrid != 0)
    {
        ugrid = vtkUnstructuredGrid::New();
        ugrid->DeepCopy(obj.ugrid);
    
        if(obj.matNumbers != 0)
        {
            matNumbers = new int[ugrid->GetNumberOfCells()];
            memcpy(matNumbers, obj.matNumbers, sizeof(int) * ugrid->GetNumberOfCells());
        }
        else
            matNumbers = 0;
 
        if(obj.velocity != 0)
        {
            velocity = vtkFloatArray::New();
            velocity->DeepCopy(obj.velocity);
        }
    }
}

// ****************************************************************************
// Method: Dyna3DFile::~Dyna3DFile
//
// Purpose: 
//   Destructor for Dyna3DFile class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 27 16:25:13 PST 2006
//
// Modifications:
//   
// ****************************************************************************

Dyna3DFile::~Dyna3DFile()
{
    FreeUpResources();
}

bool
Dyna3DFile::HasBeenRead() const
{
    return ugrid != 0;
}

bool
Dyna3DFile::Read(const std::string &filename)
{
    return ReadFile(filename, ALL_LINES);
}

bool
Dyna3DFile::Identify(const std::string &filename)
{
    return ReadFile(filename, 100);
}

// ****************************************************************************
//  Method: Dyna3DFile::FreeUpResources
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
Dyna3DFile::FreeUpResources(void)
{
    if(ugrid)
    {
        ugrid->Delete();
        ugrid = 0;
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
// Method: Dyna3DFile::GetMaterials
//
// Purpose: 
//   Get the material numbers and names.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 19 10:36:42 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

void
Dyna3DFile::GetMaterials(intVector &matnos, stringVector &matnames, doubleVector &matdens)
{
    for(int i = 0; i < materialCards.size(); ++i)
    {
        matnos.push_back(materialCards[i].materialNumber);
        matnames.push_back(materialCards[i].materialName);
        matdens.push_back(materialCards[i].density);
    }
}

const MaterialPropertiesVector &
Dyna3DFile::GetMaterials() const
{
    return materialCards;
}

int
Dyna3DFile::GetNumMaterials() const
{
    return materialCards.size();
}

MaterialProperties
Dyna3DFile::GetMaterial(int i) const
{
    MaterialProperties mat;
    if(i >= 0 && i < materialCards.size())
    {
        mat = materialCards[i];
    }
    return mat;  
}

void
Dyna3DFile::SetMaterial(int i, const MaterialProperties &mat)
{
    if(i >= 0 && i < materialCards.size())
    {
        materialCards[i] = mat;
    }
}

// ****************************************************************************
// Method: Dyna3DFile::ReadControlCards
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
Dyna3DFile::ReadControlCards(ifstream &ifile)
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
// Method: Dyna3DFile::ReadControlCardsNewFormat
//
// Purpose: 
//   Read the new format of control cards.
//
// Arguments:
//   ifile : The file stream from which to read.
//
// Returns:    True if the file looks like DYNA3D; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 16 16:37:07 PST 2011
//
// Modifications:
//   
// ****************************************************************************

bool
Dyna3DFile::ReadControlCardsNewFormat(ifstream &ifile)
{
    DEBUG_READER(debug5 << "Dyna3DFile::ReadControlCardsNewFormat" << endl;);
    bool nmmat = false, numnp = false, numelh = false;
    int i = 2;
    do 
    {
        GetLine(ifile);
        DEBUG_READER(debug5 << "line " << (i++) << ": " << line << endl;)

        if(line[0] == '*')
            continue;

        char *ns = line;
        while(ns[0] == ' ')
            ns++;

        if(strstr(line, "nmmat") != NULL)
            nmmat |= (sscanf(ns, "nmmat %d", &cards.card2.nMaterials) == 1);
        else if(strstr(line, "numnp") != NULL)
            numnp |= (sscanf(ns, "numnp %d", &cards.card2.nPoints) == 1);
        else if(strstr(line, "numelh") != NULL)
            numelh |= (sscanf(ns, "numelh %d", &cards.card2.nSolidHexes) == 1);
        else if(strncmp(line, "endfree", 7) == 0)
            break;
    } while(ifile.good());

    return nmmat && numnp && numelh;
}

// ****************************************************************************
// Method: Dyna3DFile::GetLine
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
Dyna3DFile::GetLine(ifstream &ifile)
{
    ifile.getline(line, 1024);
}

// ****************************************************************************
// Method: Dyna3DFile::SkipComments
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
Dyna3DFile::SkipComments(ifstream &ifile, const char *sectionName,
    bool &sectionNameFound, bool &sectionNameIsCurrent)
{
    sectionNameFound = false;
    sectionNameIsCurrent = false;
    DEBUG_READER(debug5 << "**************** SECTION begin *****************" << endl;)
    do
    {
        GetLine(ifile);
        if(strstr(line, "------") != NULL)
        {
            sectionNameIsCurrent = (strstr(line, sectionName) != NULL);
            sectionNameFound |= sectionNameIsCurrent;
        }
    } while(line[0] == '*');
    DEBUG_READER(debug5 << "**************** SECTION end *****************" << endl;)
}

// ****************************************************************************
// Method: Dyna3DFile::SkipToSection
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
Dyna3DFile::SkipToSection(ifstream &ifile, const char *section)
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
// Method: Dyna3DFile::ReadControlCard2
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
Dyna3DFile::ReadControlCard2(ifstream &ifile)
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
        DEBUG_READER(debug1 << "Error reading card 2:" << line << endl;)
    }

    DEBUG_READER(
    debug4 << "\tcards.card2.nMaterials=" << cards.card2.nMaterials << ",\n";
    debug4 << "\tcards.card2.nPoints=" << cards.card2.nPoints << ",\n";
    debug4 << "\tcards.card2.nSolidHexes=" << cards.card2.nSolidHexes << ",\n";
    debug4 << "\tcards.card2.nBeamElements=" << cards.card2.nBeamElements << ",\n";
    debug4 << "\tcards.card2.nShellElements4=" << cards.card2.nShellElements4 << ",\n";
    debug4 << "\tcards.card2.nShellElements8=" << cards.card2.nShellElements8 << ",\n";
    debug4 << "\tcards.card2.nInterfaceSegments=" << cards.card2.nInterfaceSegments << "\n";
    debug4 << "\tcards.card2.interfaceInterval=" << cards.card2.interfaceInterval << ",\n";
    debug4 << "\tcards.card2.shellTimestep=" << cards.card2.shellTimestep << "\n";
    )

    return isDyna;
}

//
// Fill these in later if we have to understand more control data.
//

void
Dyna3DFile::ReadControlCard3(ifstream &ifile)
{
    // Skip the comments
    bool b0,b1; SkipComments(ifile, "CONTROL CARD #3", b0, b1);

    // Process the card line.
}

void
Dyna3DFile::ReadControlCard4(ifstream &ifile)
{
    // Skip the comments
    bool b0,b1; SkipComments(ifile, "CONTROL CARD #4", b0, b1);

    // Process the card line.
}

void
Dyna3DFile::ReadControlCard5(ifstream &ifile)
{
    // Skip the comments
    bool b0,b1; SkipComments(ifile, "CONTROL CARD #5", b0, b1);

    // Process the card line.
}

void
Dyna3DFile::ReadControlCard6(ifstream &ifile)
{
    // Skip the comments
    bool b0,b1; SkipComments(ifile, "CONTROL CARD #6", b0, b1);

    // Process the card line.
}

void
Dyna3DFile::ReadControlCard7(ifstream &ifile)
{
    // Skip the comments
    bool b0,b1; SkipComments(ifile, "CONTROL CARD #7", b0, b1);

    // Process the card line.
}

void
Dyna3DFile::ReadControlCard8(ifstream &ifile)
{
    // Skip the comments
    bool b0,b1; SkipComments(ifile, "CONTROL CARD #8", b0, b1);

    // Process the card line.
}

void
Dyna3DFile::ReadControlCard9(ifstream &ifile)
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
        } while(!ifile.good());
    }
}

// ****************************************************************************
// Method: Dyna3DFile::ReadOneMaterialCard
//
// Purpose: 
//   Read one material card
//
// Arguments:
//   ifile : The input file.
//   mat   : The material card to fill in.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar  9 16:12:45 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
Dyna3DFile::ReadOneMaterialCard(ifstream &ifile, MaterialProperties &mat)
{
    const char *mName = "avtDyna3DFileFormat::ReadOneMaterialCard: ";
    DEBUG_READER(debug5 << mName << "0: " << line << endl;)
    sscanf(line, "%d", &mat.materialNumber);

    // Get the material density out of cols 11-20.
    line[20] = '\0';
    mat.density = atof(line + 10);

    // Read the name line of the material card and strip out
    // extra spaces from the name.
    GetLine(ifile);
    DEBUG_READER(debug5 << mName << "1: " << line << endl;)
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
    mat.materialName = matNameBuf;

    // If there's no material name then use the number.
    if(mat.materialName.size() == 0 || mat.materialName == " ")
    {
        SNPRINTF(matNameBuf, 1024, "%d", mat.materialNumber);
        mat.materialName = matNameBuf;
    }

    // Get the next line since it contains the strength
    GetLine(ifile);
    DEBUG_READER(debug5 << mName << "2: " << line << endl;)
    double tmp;
    sscanf(line, "%lg %lg", &tmp, &mat.strength);

    // Get the next line since it contains the eps.
    GetLine(ifile);
    sscanf(line, "%lg", &mat.equivalentPlasticStrain);
}

// ****************************************************************************
// Method: Dyna3DFile::ReadMaterialCards
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
//   Brad Whitlock, Mon Mar  9 16:31:41 PDT 2009
//   I rewrote the routine so it can handle variable line material cards.
//
//   Brad Whitlock, Mon Aug  9 14:22:41 PDT 2010
//   I added a little code to try and get us to the material cards section
//   if we're not already there by the time this routine gets called. This
//   makes the code a little more tolerant of funky control cards.
//
// ****************************************************************************

void
Dyna3DFile::ReadMaterialCards(ifstream &ifile)
{
    // Skip the comments at the start of the section
    bool matCards, currentlyInMatCards;
    const char *matCardHeader = "MATERIAL CARDS";
    SkipComments(ifile, matCardHeader, matCards, currentlyInMatCards);
    if(!currentlyInMatCards)
    {
        DEBUG_READER(debug5 << "Trying to skip ahead to " << matCardHeader << endl;)
        if(!SkipToSection(ifile, matCardHeader))
        {
            DEBUG_READER(debug5 << "We must have been past " << matCardHeader
                   << ". Reset and try again" << endl;)
            ifile.seekg(0, ios::beg);
            if(!SkipToSection(ifile, matCardHeader))
            {
                DEBUG_READER(debug5 << "Returning without reading any materials." << endl;)
                return;
            }
        }
    }
    DEBUG_READER(debug5 << "Reading materials..." << endl;)

    std::set<std::string> uniqueNames;
    bool notFirstLine = false;
    bool keepReading = true;
    do
    {
        if(notFirstLine)
            GetLine(ifile);
        notFirstLine = true;

        // See if the line ends like a material
        int len = strlen(line);
        const char *end_of_matline = "0    0    0";
        bool endsLikeMat = (strcmp(line + 69, end_of_matline) == 0);
        bool startsLikeMat = false;
        if(!endsLikeMat)
        {
            // See if the line starts like a material.
            if(line[0] == ' ' && line[1] == ' ' && line[6] == ' ')
            {
                line[6] = '\0';
                int matno; 
                if(sscanf(line, "%d", &matno) == 1)
                {
                    startsLikeMat = (matno == materialCards.size()+1);
                }
                line[6] = ' ';
            }
        }
        if(startsLikeMat || endsLikeMat)
        {
            DEBUG_READER(debug4 << "Reading material " << (materialCards.size()+1) << endl;)
            MaterialProperties mat;
            ReadOneMaterialCard(ifile, mat);

            // Make sure that the material name is unique.
            std::string matName(mat.materialName);
            if(uniqueNames.find(matName) != uniqueNames.end())
            {
                // Find a unique name.
                int index = 2;
                std::string tmpName(matName);
                char *matNameBuf = new char[1024];
                do
                {
                    SNPRINTF(matNameBuf, 1024, "%s %d", matName.c_str(), index++);
                    tmpName = std::string(matNameBuf);
                } while(uniqueNames.find(tmpName) != uniqueNames.end());
                matName = tmpName;
                delete [] matNameBuf;
            }
            uniqueNames.insert(matName);
            mat.materialName = matName;

            // Add to the list of materials.
            materialCards.push_back(mat);
        }

        // Read until we have the proper number of materials or we
        // run inth the "NODE DEFINITIONS" section.
        keepReading = (materialCards.size() < cards.card2.nMaterials) &&
                      (strstr(line, "NODE DEFINITIONS") == NULL);
    } while(keepReading);

    // Fill out the material list in case we didn't get to read them all
    int n = 1;
    while(materialCards.size() < cards.card2.nMaterials)
    {
        char badname[100];
        SNPRINTF(badname, 100, "invalid %d", n++);
        MaterialProperties mat;
        mat.materialName = badname;
        mat.materialNumber = (int)(materialCards.size() + 1);
        materialCards.push_back(mat);
    }

    // Check the material numbers. If the stored material number does not
    // match the material index then assume that the material numbers are
    // wrong and renumber them.
    size_t i, diff = 0;
    for(i = 0; i < materialCards.size(); ++i)
    {
        if(materialCards[i].materialNumber != i+1)
            diff++;
    }
    if(diff > materialCards.size()/2)
    {
        DEBUG_READER(debug5 << "Renumbering materials" << endl;)
        for(i = 0; i < materialCards.size(); ++i)
            materialCards[i].materialNumber = i+1;
    }

    DEBUG_READER(
    debug5 << "********************** MATERIALS *************************" << endl;
    for(i = 0; i < materialCards.size(); ++i)
    {
        debug5 << "Added material (" << materialCards[i].materialNumber << "): "
               << materialCards[i].materialName.c_str() << ", density="
               << materialCards[i].density << ", strength=" << materialCards[i].strength << endl;
    }
    debug5 << "**********************************************************" << endl;
    )
}

// ****************************************************************************
// Method: Dyna3DFile::ReadFile
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
//   Brad Whitlock, Mon Mar  9 16:32:16 PDT 2009
//   I changed how we do materials. I also put in some diagnostic code so we
//   can see the start and end of node and cell sequences.
//
//   Brad Whitlock, Thu Apr  2 16:26:10 PDT 2009
//   I changed the reader so it reopens the file after reading materials
//   just in case there was an error.
//
//   Brad Whitlock, Mon Aug  9 14:27:22 PDT 2010
//   I changed the code so it seeks to the start of the file instead of
//   reopening the file.
//
//   Brad Whitlock, Fri Dec 16 16:38:03 PST 2011
//   Read a new format of control cards.
//
// ****************************************************************************

bool
Dyna3DFile::ReadFile(const std::string &name, int nLines)
{
    const char *mName = "avtDyna3DFileFormat::ReadFile: ";
//    int total = visitTimer->StartTimer();
    DEBUG_READER(debug4 << mName << endl;)

    // Open the file.
    ifstream ifile(name.c_str());
    if (ifile.fail())
    {
        return false;
    }

    // Allocate a buffer that we'll use for reading lines.
    if(line == 0)
    {
        line = new char[1024];
        memset(line, 0, 1024 * sizeof(char));
    }

    // Skip line 1
    GetLine(ifile);
    DEBUG_READER(debug5 << "line1: " << line << endl;)

//    int readCards = visitTimer->StartTimer();
    bool recognized = false;
    if(strstr(line, "10 large") != NULL)
        recognized = ReadControlCardsNewFormat(ifile);
    else
        recognized = ReadControlCards(ifile);

//    visitTimer->StopTimer(readCards, "Reading control cards.");

    if(recognized)
    {
//        readCards = visitTimer->StartTimer();
        ReadMaterialCards(ifile);
//        visitTimer->StopTimer(readCards, "Reading material cards.");

        // Close and reopen the file and skip to the NODE DEFINITIONS section. This
        // prevents us from messing up the nodes if we happened to mess up the 
        // materials.
        ifile.seekg(0, ios::beg);
        SkipToSection(ifile, "NODE DEFINITIONS");

        if(nLines == ALL_LINES)
        {
//            int readingFile = visitTimer->StartTimer();
            int nPoints = cards.card2.nPoints;
            int nCells = cards.card2.nSolidHexes;
            DEBUG_READER(debug4 << mName << "File is DYNA3D and we're reading the mesh. "
                                << "nPoints=" << nPoints << ", nCells=" << nCells << endl;
            )

            vtkPoints *pts = vtkPoints::New();
            pts->Allocate(nPoints);

            ugrid = vtkUnstructuredGrid::New();
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

                if(line[0] != '*')
                {
                    DEBUG_READER(
                    if(node < 10 || node >= nPoints-10)
                        debug5 << line << endl;
                    )

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
            vtkIdType verts[8];
            for(int cellid = 0; cellid < nCells;)
            {
                GetLine(ifile);
                if(line[0] != '*')
                {
                    DEBUG_READER(
                    if(cellid < 10 || cellid >= nCells-10)
                        debug5 << line << endl;
                    )

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
                DEBUG_READER(
                debug4 << mName << "Found INITIAL CONDITIONS section. "
                       "Creating velocities." << endl;
                )
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
            else
            {
                DEBUG_READER(
                debug4 << mName << "Could not find INITIAL CONDITIONS section."
                       << endl;
                )
            }
        }
        else
        {
            DEBUG_READER(
            debug4 << mName << "File is DYNA3D." << endl;
            )
            // Skip ahead to the initial conditions section. If we can
            // then create a velocity object so we can add a velocity 
            // variable to the metadata.
            if(SkipToSection(ifile, "INITIAL CONDITIONS"))
            {
                DEBUG_READER(
                debug4 << mName << "Found INITIAL CONDITIONS section. "
                       "Creating velocities." << endl;
                )
                velocity = vtkFloatArray::New();
                velocity->SetNumberOfTuples(1);
            }
            else
            {
                DEBUG_READER(
                debug4 << mName << "Could not find INITIAL CONDITIONS section."
                       << endl;
                )
            }
        }
    }
    else
    {
        DEBUG_READER(
        debug4 << mName << "The file is not DYNA3D" << endl;
        )
    }

//    visitTimer->StopTimer(total, "Loading Dyna3D file");

    return recognized;
}

// ****************************************************************************
//  Method: Dyna3DFile::GetMesh
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
Dyna3DFile::GetMesh(const char *meshname)
{
    DEBUG_READER(debug4 << "Dyna3DFile::GetMesh" << endl;)
    vtkUnstructuredGrid *ugrid2 = vtkUnstructuredGrid::New();
    ugrid2->ShallowCopy(ugrid);

    return ugrid2;
}


// ****************************************************************************
//  Method: Dyna3DFile::GetVar
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
//    Brad Whitlock, Tue Sep 21 14:14:28 PST 2010
//    I added equivalentPlasticStrain.
//
// ****************************************************************************

vtkDataArray *
Dyna3DFile::GetVar(const char *varname)
{
    DEBUG_READER(debug4 << "Dyna3DFile::GetVar" << endl;)
    vtkDataArray *ret = 0;

    if((strcmp(varname, "density") == 0 ||
        strcmp(varname, "strength") == 0 ||
        strcmp(varname, "equivalentPlasticStrain") == 0) &&
       matNumbers != 0 && 
       materialCards.size() > 0)
    {
        std::map<int, int> mat2card;
        for(int i = 0; i < materialCards.size(); ++i)
            mat2card[materialCards[i].materialNumber] = i;

        vtkFloatArray *arr = vtkFloatArray::New();
        arr->SetNumberOfTuples(ugrid->GetNumberOfCells());
        float *fptr = (float *)arr->GetVoidPointer(0); 
        if(strcmp(varname, "density") == 0)
        {
            for(vtkIdType id = 0; id < ugrid->GetNumberOfCells(); ++id)
            {
               int cellMat = matNumbers[id];
               int card = mat2card[cellMat];
               *fptr++ = (float)materialCards[card].density;
            }
        }
        else if(strcmp(varname, "strength") == 0)
        {
            for(vtkIdType id = 0; id < ugrid->GetNumberOfCells(); ++id)
            {
               int cellMat = matNumbers[id];
               int card = mat2card[cellMat];
               *fptr++ = (float)materialCards[card].strength;
            }
        }
        else if(strcmp(varname, "equivalentPlasticStrain") == 0)
        {
            for(vtkIdType id = 0; id < ugrid->GetNumberOfCells(); ++id)
            {
               int cellMat = matNumbers[id];
               int card = mat2card[cellMat];
               *fptr++ = (float)materialCards[card].equivalentPlasticStrain;
            }
        }

        ret = arr;
    }
    else if((strcmp(varname, "material") == 0  || strcmp(varname, "matno") == 0) &&
            matNumbers != 0)
    {
        vtkIntArray *arr = vtkIntArray::New();
        arr->SetNumberOfTuples(ugrid->GetNumberOfCells());
        int *iptr = (int *)arr->GetVoidPointer(0);
        for(vtkIdType id = 0; id < ugrid->GetNumberOfCells(); ++id)
            *iptr++ = matNumbers[id];

        ret = arr;
    }

    if(ret != NULL)
        ret->SetName(varname);

    return ret;
}


// ****************************************************************************
//  Method: Dyna3DFile::GetVectorVar
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
Dyna3DFile::GetVectorVar(const char *varname)
{
    DEBUG_READER(debug4 << "Dyna3DFile::GetVectorVar" << endl;)
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

    return ret;
}

bool
Dyna3DFile::Write(const std::string &filename, const avtMatrix &M) const
{
    if(ugrid == 0)
        return false;

    FILE *f = fopen(filename.c_str(), "wt");
    if(f != NULL)
    {
        fprintf(f, "                                                                        88 large\n");
        fprintf(f, "*\n");
        fprintf(f, "* This file was created using SDM's Dyna3DFile class.\n");
        fprintf(f, "*\n");
        fprintf(f, "*------------------- ANALYSIS INPUT DATA FOR DYNA3D -------------------*\n");
        fprintf(f, "*\n");
        fprintf(f, "*\n");
        fprintf(f, "*-------------------------- CONTROL CARD #2 ---------------------------*\n");
        fprintf(f, "*\n");
        fprintf(f, "* number of materials[1] nodal points[2] solid hexahedron elements[3] beam\n");
        fprintf(f, "* elements[4] 4-node shell elements[5] 8-node solid shell elements[6]\n");
        fprintf(f, "* interface segments[7] interface interval[8] min. shell time step[9]\n");
        fprintf(f, "%5d%10d%10d         0         0         0         0 0.000E+00  0.0\n",
                (int)materialCards.size(), (int)ugrid->GetPoints()->GetNumberOfPoints(), 
                (int)ugrid->GetNumberOfCells());

        fprintf(f, "*\n");
        fprintf(f, "*-------------------------- CONTROL CARD #3 ---------------------------*\n");
        fprintf(f, "*\n");
        fprintf(f, "* number of time history blocks for nodes[1] hexahedron elements[2] beam\n");
        fprintf(f, "* elements[3] shell elements[4] thick shell elements[5] and report interval[6]\n");
        fprintf(f, "* reaction forces print flag[7] discrete element forces print flag[8]\n");
        fprintf(f, "* element deletion/SAND database flag[9]\n");
        fprintf(f, "    0    0    0    0    0    0    0    0    1\n");
        fprintf(f, "*\n");
        fprintf(f, "*-------------------------- CONTROL CARD #4 ---------------------------*\n");
        fprintf(f, "*\n");
        fprintf(f, "* number of sliding boundary planes[1]\n");
        fprintf(f, "* sliding boundary planes w/ failure[2] points in density vs depth\n");
        fprintf(f, "* curve[3] brode function flag[4] number of rigid body merge cards[5] \n");
        fprintf(f, "* nodal coordinate format[6] force cross sections[7] \n");
        fprintf(f, "* cross section forces interval[8]\n");
        fprintf(f, "         0    0    0    0    0e20.0    0 0.000E+00\n");
        fprintf(f, "*\n");
        fprintf(f, "*-------------------------- CONTROL CARD #5 ---------------------------*\n");
        fprintf(f, "*\n");
        fprintf(f, "* number of load curves[1] concentrated nodal loads[2] element sides having\n");
        fprintf(f, "* pressure loads applied[3] velocity/acceleration boundary condition cards[4]\n");
        fprintf(f, "* rigid walls (stonewalls)[5] nodal constraint cards[6] initial condition\n");
        fprintf(f, "* parameter[7] sliding interfaces[8] base acceleration in x[9] y[10] and\n");
        fprintf(f, "* z-direction[11] angular velocity about x[12] y[13] and z-axis[14] number of \n");
        fprintf(f, "* solid hexahedron elements for momentum deposit[15] detonation points[16]\n");
        fprintf(f, "    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0\n");
        fprintf(f, "*\n");
        fprintf(f, "*-------------------------- CONTROL CARD #6 ---------------------------*\n");
        fprintf(f, "*\n");
        fprintf(f, "* termination time[1] time history dump interval[2] complete dump interval[3]\n");
        fprintf(f, "* time steps between restart dumps[4] time steps between running restart\n");
        fprintf(f, "* dumps[5] initial time step[6] sliding interface penalty factor[7] thermal\n");
        fprintf(f, "* effects option[8] default viscosity flag[9] computed time step factor[10]\n");
        fprintf(f, " 1.000E+05 1.000E+03 1.000E+03    0    0 0.000E+00 0.000E+00    0    0 0.000E+00\n");
        fprintf(f, "*\n");
        fprintf(f, "*-------------------------- CONTROL CARD #7 ---------------------------*\n");
        fprintf(f, "*\n");
        fprintf(f, "* number of joint definitions[1] rigid bodies with extra nodes[2] shell-\n");
        fprintf(f, "* solid interfaces[3] tie-breaking shell slidelines[4] tied node sets with\n");
        fprintf(f, "* failure[5] limiting time step load curve number[6] springs-dampers-masses\n");
        fprintf(f, "* flag[7] rigid bodies with inertial properties[8] dump shell strain flag[9]\n");
        fprintf(f, "* number of material groups for deformable-rigid switching[10] number of\n");
        fprintf(f, "* mass proportional damping sets[11] Hughes-Liu shell update[12] shell\n");
        fprintf(f, "* thickness change[13] shell formulation[14] number of nonreflecting boundary\n");
        fprintf(f, "* segments[15]\n");
        fprintf(f, "    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0\n");
        fprintf(f, "*\n");
        fprintf(f, "*-------------------------- CONTROL CARD #8 ---------------------------*\n");
        fprintf(f, "*\n");
        fprintf(f, "* number of point constraint nodes[1] coordinate systems for constraint\n");
        fprintf(f, "* nodes[2] minimum step factor[3] number of beam integration rules[4]\n");
        fprintf(f, "* maximum integration points for beams[5] number of shell integration rules[6]\n");
        fprintf(f, "* maximum integration points for shells[7] relaxation iterations between\n");
        fprintf(f, "* checks[8] relaxation tolerance[9] dynamic relaxation factor[10] dynamic\n");
        fprintf(f, "* relaxation time step factor[11] 4-node shell time step option[12]\n");
        fprintf(f, "    0    0 0.000E+00    0    0    0    0  250 1.000E-03 9.950E-01 0.000E+00    0\n");
        fprintf(f, "*\n");
        fprintf(f, "*-------------------------- CONTROL CARD #9 ---------------------------*\n");
        fprintf(f, "*\n");
        fprintf(f, "* plane stress plasticity[1] printout flag[2] number of 1D slidelines[3]\n");
        fprintf(f, "* relaxation database[4] Rayleigh coefficient[5]\n");
        fprintf(f, "* materials w/Rayleigh damping[6] materials for initial rotation[7]\n");
        fprintf(f, "* materials w/ body force loads[8]\n");
        fprintf(f, "    0    0    0    0 0.000E+00    0    0    0\n");

        fprintf(f, "*\n");
        fprintf(f, "*--------------------------- MATERIAL CARDS ---------------------------*\n");
        fprintf(f, "*\n");

        if(materialCards.size() > 0)
        {
            for(int i = 0; i < materialCards.size(); ++i)
            {
                char matname[81];
                memset(matname, ' ', 80);
                const char *src = materialCards[i].materialName.c_str();
                char *dest = matname;
                while((*src != '\0') && (dest < (matname+80)))
                    *dest++ = *src++;
                matname[80] = '\0';

                fprintf(f, "%5d    0%1.4E    0    00.0000E+00    00.0000E+000.0000E+00    0    0    0\n",
                        materialCards[i].materialNumber, materialCards[i].density);
                fprintf(f, "%s\n", matname);
                fprintf(f, " 0.000E+00% 1.3E 0.000E+00 0.000E+00 0.000E+00 0.000E+00 0.000E+00 0.000E+00\n",
                        materialCards[i].strength);
                fprintf(f, " 0.000E+00 0.000E+00 0.000E+00 0.000E+00 0.000E+00 0.000E+00 0.000E+00 0.000E+00\n");
                fprintf(f, " 0.000E+00 0.000E+00 0.000E+00 0.000E+00 0.000E+00 0.000E+00 0.000E+00 0.000E+00\n");
                fprintf(f, " 0.000E+00 0.000E+00 0.000E+00 0.000E+00 0.000E+00 0.000E+00 0.000E+00 0.000E+00\n");
                fprintf(f, " 0.000000000E+00 0.000000000E+00 0.000000000E+00 0.000000000E+00 0.000000000E+00\n");
                fprintf(f, " 0.000000000E+00 0.000000000E+00 0.000000000E+00 0.000000000E+00 0.000000000E+00\n");
            }
        }

        fprintf(f, "*\n");
        fprintf(f, "*-------------------------- NODE DEFINITIONS --------------------------*\n");
        fprintf(f, "*\n");
        int npts = ugrid->GetPoints()->GetNumberOfPoints();
        const float *p = (const float *)ugrid->GetPoints()->GetVoidPointer(0);
        for(int i = 0; i < npts; ++i)
        {
            avtVector v(M * avtVector(p[0],p[1],p[2]));
            fprintf(f, "%8d    0%+1.13E%+1.13E%+1.13E    0\n", i+1, v.x, v.y, v.z);
            p += 3;
        }

        fprintf(f, "*\n");
        fprintf(f, "*------------------ ELEMENT CARDS FOR SOLID ELEMENTS ------------------*\n");
        fprintf(f, "*\n");
        vtkIdType ncells = ugrid->GetNumberOfCells();
        for(vtkIdType cellid = 0; cellid < ncells; ++cellid)
        {
            int mat = (matNumbers != NULL) ? matNumbers[cellid] : 1;

            vtkCell *cell = ugrid->GetCell(cellid);
            fprintf(f, "%8d%5d%8d%8d%8d%8d%8d%8d%8d%8d\n",
                    (int)cellid+1,
                    mat, 
                    (int)cell->GetPointId(0)+1,
                    (int)cell->GetPointId(1)+1,
                    (int)cell->GetPointId(2)+1,
                    (int)cell->GetPointId(3)+1,
                    (int)cell->GetPointId(4)+1,
                    (int)cell->GetPointId(5)+1,
                    (int)cell->GetPointId(6)+1,
                    (int)cell->GetPointId(7)+1
                   );
        }

        if(velocity != NULL)
        {
            fprintf(f, "*\n");
            fprintf(f, "*---------------------- INITIAL CONDITIONS ----------------------------*\n");
            fprintf(f, "* \n");
            int ntuples = velocity->GetNumberOfTuples();
            float *fptr = (float *)velocity->GetVoidPointer(0);
            char tmp[80];
            for(int i = 0; i < ntuples; ++i)
            {
                fprintf(f, "%8d%+2.3E%+2.3E%+2.3E\n", i+1, fptr[0], fptr[1], fptr[2]);

                fptr += 3;
            }
        }

        fclose(f);
    }
    return f != NULL;
}

void
Dyna3DFile::Combine(const Dyna3DFile &obj)
{
    // Combine the materials
    std::map<std::string,int> matNames;
    for(int i = 0; i < materialCards.size(); ++i)
        matNames[materialCards[i].materialName] = 1;
    std::map<int,int> oldMat2NewMat;
    for(int i = 0; i < obj.materialCards.size(); ++i)
    {
        MaterialProperties mc;
        mc = obj.materialCards[i];
        std::map<std::string,int>::iterator it = matNames.find(mc.materialName);
        if(it != matNames.end())
        {
            it->second = it->second + 1;

            char tmp[10];
            sprintf(tmp, " (copy %d)", it->second);
            mc.materialName += std::string(tmp);
        }

        int nextMat = materialCards.size() + 1;
        oldMat2NewMat[obj.materialCards[i].materialNumber] = nextMat;
        mc.materialNumber = nextMat;

        materialCards.push_back(mc);
    }
    if(matNumbers != NULL || obj.matNumbers != NULL)
    {
        int ncells = cards.card2.nSolidHexes + obj.cards.card2.nSolidHexes;
        int *newMats = new int[ncells];
        for(int i = 0; i < ncells; ++i)
            newMats[i] = 1;

        if(matNumbers != NULL)
            memcpy(newMats, matNumbers, sizeof(int)*cards.card2.nSolidHexes);

        if(obj.matNumbers != NULL)
        {
            int *iptr = newMats + cards.card2.nSolidHexes;
            for(int i = 0; i < obj.cards.card2.nSolidHexes; ++i)
            {
                iptr[i] = oldMat2NewMat[obj.matNumbers[i]];
            }
        }

        delete [] matNumbers;
        matNumbers = newMats;
    }

    // Combine the datasets.
    if(ugrid == 0 && obj.ugrid == 0)
    {
        // no op 
    }
    else if(ugrid != 0 && obj.ugrid != 0)
    {
        vtkAppendFilter *append = vtkAppendFilter::New();
        append->AddInput(ugrid);
        append->AddInput(obj.ugrid);
        append->Update();
        append->GetOutput()->Register(NULL);
        ugrid->Delete();
        ugrid = append->GetOutput();
        append->Delete();
    }
    else if(obj.ugrid == 0)
    {
        // no op
    }
    else if(ugrid == 0)
    {
        ugrid = vtkUnstructuredGrid::New();
        ugrid->DeepCopy(obj.ugrid);
    }

    // Combine the velocity data
    if(velocity != NULL || obj.velocity != NULL)
    {
        int n1 = cards.card2.nPoints;
        int n2 = obj.cards.card2.nPoints;

        vtkFloatArray *v2 = vtkFloatArray::New();
        v2->SetNumberOfComponents(3);
        v2->SetNumberOfTuples(n1+n2);
        float *f = (float *)v2->GetVoidPointer(0);
        memset(f, 0, sizeof(float)*(n1+n2));
        if(obj.velocity != NULL)
            memcpy(f+n1, obj.velocity->GetVoidPointer(0), sizeof(float)*n2);
        if(velocity != NULL)
        {
            memcpy(f, velocity->GetVoidPointer(0), sizeof(float)*n1);
            velocity->Delete();
        }

        velocity = v2;        
    }

    // Combine the cards
    cards.card2.nMaterials += obj.cards.card2.nMaterials;
    cards.card2.nPoints += obj.cards.card2.nPoints;
    cards.card2.nSolidHexes += obj.cards.card2.nSolidHexes;
    cards.card2.nBeamElements += obj.cards.card2.nBeamElements;
    cards.card2.nShellElements4 += obj.cards.card2.nShellElements4;
    cards.card2.nShellElements8 += obj.cards.card2.nShellElements8;
    cards.card2.nInterfaceSegments += obj.cards.card2.nInterfaceSegments;
}

void
Dyna3DFile::TransformPoints(const avtMatrix &M)
{
    if(ugrid != 0)
    {
        int npts = ugrid->GetPoints()->GetNumberOfPoints();
        float *p = (float *)ugrid->GetPoints()->GetVoidPointer(0);
        for(int i = 0; i < npts; ++i)
        {
            avtVector v(M * avtVector(p[0],p[1],p[2]));
            p[0] = v.x;
            p[1] = v.y;
            p[2] = v.z;
            p += 3;
        }
    }
}

bool
Dyna3DFile::CreateTransformedFile(const char *infile, const char *outfile,
    const avtMatrix &M)
{
    // Let's open the file and scan until we find the nodes. Once we find the
    // nodes, we'll read them in, transform them and write them back out.

    // Open the input file.
    ifstream ifile(infile);
    if (ifile.fail())
    {
        DEBUG_READER(cerr << "Can't open input file: " << infile << endl;)
        return false;
    }

    // Get the number of nodes from the input file.
    Dyna3DFile f;
    f.ReadFile(infile, 10);
    int originalNumNodes = f.cards.card2.nPoints;

    // Open the output file
    ofstream ofile(outfile);
    if (ofile.fail())
    {
        DEBUG_READER(cerr << "Can't open output file: " << outfile << endl;)
        ifile.close();
        return false;
    }

    // Read in the header stuff, copying it over into the new file
    bool not_nodes = true;
    bool comment = false;
    char *line = new char[2048];
    memset(line, 0, 2048 * sizeof(char));
    do
    {
        ifile.getline(line, 2048);
        comment = (line[0] == '*');
        if(comment && strstr(line, "NODE DEF") != NULL)
        {
            not_nodes = false;
        }
        ofile << line << endl;
    } while(!ifile.eof() && not_nodes);

    // Now, let's replace all of the nodes.
    avtVector vec;
    for(int node = 0; node < originalNumNodes && !ifile.eof(); )
    {
        ifile.getline(line, 2048);

        if(line[0] != '*')
        {
//                    if(node < 10 || node >= nPoints-10)
//                        debug5 << line << endl;
            char tmp = line[73];

            // Read the point from line
            char *valstart = line + 53;
            char *valend = valstart + 73;
            *valend = '\0';
            vec.z = atof(valstart);
    
            valstart -= 20;
            valend   -= 20;
            *valend = '\0';
            vec.y = atof(valstart);

            valstart -= 20;
            valend   -= 20;
            *valend = '\0';
            vec.x = atof(valstart);

            // Transform the point
            avtVector vec2(M * vec);

            // Write the point back out into line
            if(vec2.x < 0)
                sprintf(line+13,"%1.13E", vec2.x);
            else
                sprintf(line+13," %1.13E", vec2.x);

            if(vec2.y < 0)
                sprintf(line+33,"%1.13E", vec2.y);
            else
                sprintf(line+33," %1.13E", vec2.y);

            if(vec2.z < 0)
                sprintf(line+53,"%1.13E", vec2.z);
            else
                sprintf(line+53," %1.13E", vec2.z);

            line[73] = tmp;

            ++node;
        }

        ofile << line << endl;
    }

    // Now, read until the end of the file
    while(!ifile.eof())
    {
        ifile.getline(line, 2048);
        ofile << line;
        if(!ifile.eof())
            ofile << endl;
    }
    ifile.close();
    ofile.close();

    delete [] line;

    return true;
}
