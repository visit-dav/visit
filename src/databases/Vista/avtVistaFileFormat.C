// ************************************************************************* //
//                            avtVistaFileFormat.C                           //
// ************************************************************************* //

// this include directive refernces code provided by Jeff Keasler. I am
// including it using an include directive so that as Jeff updates this
// code, it can be easily incorporated into the plugin 
#include <VisitALE.c>

#include <stdarg.h>
#include <stdio.h>

#include <snprintf.h>

#include <string>
#include <map>

#include <StringHelpers.h>
#include <avtVistaFileFormat.h>

#include <vtkCellType.h>
#include <vtkFloatArray.h>
#include <vtkIdType.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>

#include <DebugStream.h>

#include <BadIndexException.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>
#include <UnexpectedValueException.h>
#include <Utility.h>

#include <visit-config.h>

using std::string;
using std::map;

const int avtVistaFileFormat::MASTER_FILE_INDEX = 0;
static char tempStr[1024];

#define MATCH(A,B)        strncmp(A,B,sizeof(B))==0

// ****************************************************************************
//  Function:  VisitNameToVistaName 
//
//  Purpose:   Convert a visit variable name to Vista list of vista scalar
//             variables 
//
//  Programmer:  Mark C. Miller 
//  Creation:    March 15, 2004 
//
//  Modifications:
//
//    Mark C. Miller, Wed May 19 10:56:11 PDT 2004
//    Added support for 2D meshes
//
// ****************************************************************************
static void
VisitNameToVistaName(const char *visitName, vector<string> &vistaNames, int dims)
{
    if (MATCH(visitName, "coords"))
    {
        vistaNames.push_back("x");
        vistaNames.push_back("y");
        if (dims == 3)
            vistaNames.push_back("z");
    }
    else if (MATCH(visitName, "coords0"))
    {
        vistaNames.push_back("x0");
        vistaNames.push_back("y0");
        if (dims == 3)
            vistaNames.push_back("z0");
    }
    else if (MATCH(visitName, "velocity"))
    {
        vistaNames.push_back("xd");
        vistaNames.push_back("yd");
        if (dims == 3)
            vistaNames.push_back("zd");
    }
    else if (MATCH(visitName, "stress"))
    {
        vistaNames.push_back("sx");
        vistaNames.push_back("sy");
        vistaNames.push_back("p");
        vistaNames.push_back("txy");
        vistaNames.push_back("txz");
        vistaNames.push_back("tyz");
    }
    else
    {
        vistaNames.push_back(visitName);
    }
}

// ****************************************************************************
//  Function:  HasAllComponents 
//
//  Purpose:   Check that all components names passed in have count equal
//             to pieceCount in fieldMap
//
//  Programmer:  Mark C. Miller 
//  Creation:    February 17, 2004 
//
// ****************************************************************************
static bool
HasAllComponents(map<string, IMVal<int,0> >& fieldMap, int pieceCount,
    const char *compName, ...)
{
    bool retval = true;
    const char *name;
    va_list ap;

    va_start(ap, compName);
    name = compName;
    while (name != 0 && retval)
    {
        if (fieldMap[name].val != pieceCount)
            retval = false;
        name = va_arg(ap, const char *);
    }
    va_end(ap);

    return retval;
}

int avtVistaFileFormat::objcnt = 0;

// ****************************************************************************
//  Function:  avtVistaFileFormat::InitializeHDF5
//
//  Purpose:   Initialize interaction with the HDF5 library
//
//  Programmer:  Mark C. Miller 
//  Creation:    February 17, 2004 
//
// ****************************************************************************
static void
InitializeHDF5(void)
{
    debug5 << "Initializing HDF5 Library" << endl;
    H5open();
    H5Eset_auto(NULL, NULL);
}

// ****************************************************************************
//  Function:  avtVistaFileFormat::FinalizeHDF5
//
//  Purpose:   End interaction with the HDF5 library
//
//  Programmer:  Mark C. Miller 
//  Creation:    February 17, 2004 
//
// ****************************************************************************
static void
FinalizeHDF5(void)
{
    debug5 << "Finalizing HDF5 Library" << endl;
    H5close();
}

// ****************************************************************************
//  Method:  GetFileNameForRead 
//
//  Purpose:   Determines file name for a read based on domain number 
//             variables 
//
//  Programmer:  Mark C. Miller 
//  Creation:    April 28, 2004 
//
// ****************************************************************************
void
avtVistaFileFormat::GetFileNameForRead(int dom, char *fileName, int size)
{
    int filePart = domToFileMap[dom];
    if (filePart == MASTER_FILE_INDEX)
        strncpy(fileName, masterFileName.c_str(), size);
    else
        SNPRINTF(fileName, size, "%s.%d", masterFileName.c_str(),
                    filePart);
}

// ****************************************************************************
//  Method: avtVistaFileFormat constructor
//
//  Programmer: Mark C. Miller
//  Creation:   Tue Feb 17 19:19:07 PST 2004
//
//  Modifications:
//
//    Mark C. Miller, Wed May 19 10:56:11 PDT 2004
//    Replaced use of basename/dirname with StringHelper functions
//
// ****************************************************************************

avtVistaFileFormat::avtVistaFileFormat(const char *filename)
    : avtSTMDFileFormat(&filename, 1)
{
    int i;

    vTree = 0;
    domToFileMap = 0;

    spatialDim = 0;

    masterFileName = StringHelpers::Basename(filename);
    masterDirName = StringHelpers::Dirname(filename);

    fileHandles = new void*[MAX_FILES];
    for (i = 0 ; i < MAX_FILES ; i++)
    {
        fileHandles[i] = 0;
    }

    // lets determine, up front, if its a Silo or HDF5 vista file
    DBShowErrors(DB_NONE, 0);
    DBfile *dbfile = DBOpen((char *) filename, DB_UNKNOWN, DB_READ);
    if (dbfile != NULL)
    {
        isSilo = true;
        DBForceSingle(1);
        DBShowErrors(DB_ALL, 0);
        DBClose(dbfile);
    }
    else
    {
        isSilo = false;
    }

    materialNumbersArray = 0;
    materialNamesArray = 0;

    // if its Vista-HDF5, do HDF5 library initialization on consturction of
    // first instance 
    if (!isSilo && (avtVistaFileFormat::objcnt == 0))
        InitializeHDF5();
    avtVistaFileFormat::objcnt++;
}

// ****************************************************************************
//  Destructor:  avtVistaFileFormat::~avtVistaFileFormat
//
//  Programmer:  Mark C. Miller 
//  Creation:   Tue Feb 17 19:19:07 PST 2004
//
// ****************************************************************************
avtVistaFileFormat::~avtVistaFileFormat()
{
    FreeUpResources();

    if (fileHandles)
        delete [] fileHandles;

    if (domToFileMap)
        delete [] domToFileMap;

    if (materialNumbersArray)
        delete [] materialNumbersArray;

    if (materialNamesArray)
        delete [] materialNamesArray;

    // handle HDF5 library termination on descrution of last instance
    avtVistaFileFormat::objcnt--;
    if (!isSilo && (avtVistaFileFormat::objcnt == 0))
        FinalizeHDF5();
}

// ****************************************************************************
//  Method: avtVistaFileFormat::OpenFile
//
//  Purpose:
//      Opens a Vista file.
//
//  Arguments:
//      f       The index of the file in our array.
//
//  Returns:    A handle to the open file.
//
//  Programmer: Mark C. Miller
//  Creation:   February 17, 2004
//
// ****************************************************************************

void *
avtVistaFileFormat::OpenFile(int f)
{
    //
    // Make sure this is in range.
    //
    if (f < 0 || f >= nFiles)
    {
        EXCEPTION2(BadIndexException, f, nFiles);
    }

    //
    // Check to see if the file is already open.
    //
    if (fileHandles[f] != 0)
    {
        UsedFile(f);
        return fileHandles[f];
    }

    //
    // Open the Vista file.
    //
    if (isSilo)
    {
        fileHandles[f] = (void *) DBOpen(filenames[f], DB_UNKNOWN, DB_READ);
    }
    else
    {
        fileHandles[f] = (void *) new hid_t;
        *((hid_t*) fileHandles[f]) = H5Fopen(filenames[f],
                                         H5F_ACC_RDONLY, H5P_DEFAULT);
    }

    //
    // Check to see if we got a valid handle.
    //
    if (fileHandles[f] == 0)
    {
        EXCEPTION1(InvalidFilesException, filenames[f]);
    }

    RegisterFile(f);

    return fileHandles[f];
}


// ****************************************************************************
//  Method: avtVistaFileFormat::OpenFile
//
//  Purpose:
//      Opens a Vista file by name; adds the file name to a list
//      if we haven't heard of it before.
//
//  Programmer: Mark C. Miller 
//  Creation:   February 17, 2004
//
// *****************************************************************************

void *
avtVistaFileFormat::OpenFile(const char *fileName)
{
    //
    // The directory of this file is all relative to the directory of the
    // table of contents.  Reflect that here.
    //
    char name[1024];
    char *tocFile = filenames[0];
    char *thisSlash = tocFile, *lastSlash = tocFile;
    while (thisSlash != NULL)
    {
        lastSlash = thisSlash;
        thisSlash = strstr(lastSlash+1, SLASH_STRING);
    }
    if (lastSlash == tocFile)
    {
        strcpy(name, fileName);
    }
    else
    {
        int amount = lastSlash-tocFile+1;
        strncpy(name, tocFile, amount);
        strcpy(name+amount, fileName);
    }

    int fileIndex = -1;
    for (int i = 0 ; i < nFiles ; i++)
    {
        if (strcmp(filenames[i], name) == 0)
        {
            fileIndex = i;
            break;
        }
    }

    if (fileIndex == -1)
    {
        //
        // We have asked for a previously unseen file.  Add it to the list and
        // continue.  AddFile will automatically take care of overflow issues.
        //
        fileIndex = AddFile(name);
    }

    return OpenFile(fileIndex);
}

// ****************************************************************************
//  Method: avtVistaFileFormat::CloseFile
//
//  Purpose:
//      Closes the Vista file.
//
//  Programmer: Mark C. Miller 
//  Creation:   February 17, 2004
//
// ****************************************************************************

void
avtVistaFileFormat::CloseFile(int f)
{
    if (fileHandles[f] != 0)
    {
        debug4 << "Closing Vista file " << filenames[f] << endl;
        if (isSilo)
        {
            DBClose((DBfile*) fileHandles[f]);
        }
        else
        {
            H5Fclose(*((hid_t*) fileHandles[f]));
            delete (hid_t *) fileHandles[f];
        }
        UnregisterFile(f);
        fileHandles[f] = 0;
    }
}

// ****************************************************************************
//  Method: avtVistaFileFormat::ReadDataset
//
//  Purpose:
//      Reads a named datasets from the file.
//
//  Programmer: Mark C. Miller 
//  Creation:   February 17, 2004
//
// ****************************************************************************

bool
avtVistaFileFormat::ReadDataset(const char *fileName, const char *dsName,
    hid_t *dataType, hsize_t *size, void **buf)
{
    hid_t retType;
    hsize_t retSize;
    bool retVal = false;
    void *f;
    
    if (fileName == 0)
        f = OpenFile(MASTER_FILE_INDEX);
    else
        f = OpenFile(fileName);

    if (isSilo)
    {
        DBfile *dbfile = (DBfile*) f;

        if (DBInqVarExists(dbfile, (char *) dsName) == 0)
        {
            retType = -1;
            retSize = 0;
        }
        else
        {
            // examine the variable size
            retSize = DBGetVarLength(dbfile, (char *) dsName);

            // examine the variable type
            int type = DBGetVarType(dbfile, (char *) dsName);
            if (type == DB_INT)
                retType = H5T_NATIVE_INT;
            else if (type == DB_DOUBLE)
                retType = H5T_NATIVE_DOUBLE;
            else if (type == DB_CHAR)
                retType = H5T_NATIVE_CHAR;

            // read the variable
            if (buf != 0)
            {
                if (*buf == 0)
                    *buf = new char [DBGetVarByteLength(dbfile, (char *) dsName)];
                else
                {
                    if (*size < retSize)
                    {
                        EXCEPTION2(UnexpectedValueException, (int) retSize, (int) *size);
                    }
                }
                DBReadVar(dbfile, (char *) dsName, *buf);
            }

            retVal = true;
        }

    }
    else
    {
        // open the dataset
        hid_t hdfFile= *((hid_t *) f);
        hid_t ds = H5Dopen(hdfFile, dsName);

        if (ds < 0)
        {
            retType = -1;
            retSize = 0;
        }
        else
        {
            // examine the dataspace
            hid_t space = H5Dget_space(ds);
            int hndims = H5Sget_simple_extent_ndims(space);
            hsize_t *hdims = new hsize_t[hndims];
            hsize_t *max_hdims = new hsize_t[hndims];
            H5Sget_simple_extent_dims(space, hdims, max_hdims);
            retSize = 1;
            int j;
            for (j = 0; j < hndims; j++)
                retSize *= hdims[j];
            delete [] hdims;
            delete [] max_hdims;

            // examine the datatype
            hid_t type = H5Dget_type(ds);
            H5T_class_t typeClass = H5Tget_class(type);
            size_t typeSize = H5Tget_size(type);
            if (typeClass == H5T_FLOAT)
               retType = H5T_NATIVE_DOUBLE;
            else if (typeClass == H5T_INTEGER)
            {
                if (typeSize == 1)
                    retType = H5T_NATIVE_CHAR;
                else if (typeSize == 4)
                    retType = H5T_NATIVE_INT;
                else if (typeSize == 8)
                    retType = H5T_NATIVE_INT;
            }

            // do the read
            if (buf != 0)
            {
                if (*buf == 0)
                    *buf = new char [retSize * H5Tget_size(type)];
                else
                {
                    if (*size < retSize)
                    {
                        EXCEPTION2(UnexpectedValueException, (int) retSize, (int) *size);
                    }
                }
                H5Dread(ds, retType, H5S_ALL, H5S_ALL, H5P_DEFAULT, *buf);
            }

            H5Sclose(space);
            H5Tclose(type);
            H5Dclose(ds);
            retVal = true;
        }
    }

    // set up the return values
    if (dataType != 0)
        *dataType = retType;
    if (size != 0)
        *size = retSize;
    return retVal;
}


// ****************************************************************************
//  Method: avtVistaFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Mark C. Miller
//  Creation:   Tue Feb 17 19:19:07 PST 2004
//
// ****************************************************************************

void
avtVistaFileFormat::FreeUpResources(void)
{
    int i;

    for (i = 0 ; i < nFiles; i++)
    {
        CloseFile(i);
    }

    if (vTree != 0)
    {
        delete vTree;
        vTree = 0;
    }

    // we don't finalize HDF5 here because its not clear if/when we might
    // need to re-initialize it, again
}


// ****************************************************************************
//  Method: avtVistaFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Notes on Vista File Format:
//     pre-defined named entities that can be in a view
//        + Fields (group)
//        + Indexset (integer dataset)
//        + Relations (group)
//            - elemToNode (zonelist)
//        + Attr (group)
//        + Params (group)
//
//  Programmer: Mark C. Miller
//  Creation:   Tue Feb 17 19:19:07 PST 2004
//
//  Modifications:
//
//    Mark C. Miller, Wed May 19 10:56:11 PDT 2004
//    Added support for 2D meshes
//
// ****************************************************************************

void
avtVistaFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    int i, j;
    hsize_t hsize;

    // get a vista tree if we haven't already 
    if (vTree == 0)
    {
        char *buf = 0;
        hsize = 0;
        ReadDataset(0, "VisIt", 0, &hsize, (void**) &buf);
        vTree = new VistaTreeParser(buf, hsize);
        //vTree->DumpTree();
        delete [] buf;
    }

    const Node *top = vTree->GetTop();

    // Find top-level pieces in the vista tree
    VisitFindNodes(top, "/.*/[a-zA-Z]{1,}[-_.:;=#+@ ]{0,1}[0-9]{1,}", &pieceNodes, &numPieces);

    if (numPieces == 0)
    {
        cerr << "WARNING!!! Didn't find any domains, nothing to visualize" << endl;
        return;
    }

    // use the string grouping utility to see how many "groups" we got
    vector<string> pieceNames;
    vector<vector<string> > pieceGroups;
    vector<string> groupNames;
    for (i = 0; i < numPieces; i++)
        pieceNames.push_back(pieceNodes[i]->text);
    StringHelpers::GroupStrings(pieceNames, pieceGroups, groupNames);

    // if we get more than one candidate group that look like domains, pick one
    if (pieceGroups.size() > 1)
    {
        cerr << "WARNING!!! Found more than one candidate group of domains" << endl;
        cerr << "           Using group named \"" << groupNames[0] << "\"" << endl;

        // rebuild the list of pieceNodes using only the group we've chosen
        delete [] pieceNodes;
        numPieces = 0;
        sprintf(tempStr,"/./%s[0-9]{1,}", groupNames[0].c_str());
        VisitFindNodes(top, tempStr, &pieceNodes, &numPieces);
        if (numPieces != pieceGroups[0].size())
        {
            cerr << "WARNING!!! Unable to find domains" << endl;
            return;
        }
    }
    pieceNames.clear();
    pieceGroups.clear();
    groupNames.clear();
    
    // determinie block name stuff 
    string blockPieceName;
    string blockTitle;
    vector<string> blockNames;
    string aPieceName = pieceNodes[0]->text;
    string::size_type firstDigit = aPieceName.find_first_of("0123456789");
    if (firstDigit != string::npos)
    {
        string sepChars = "_-.:;=#+@";
        while ((sepChars.find(aPieceName[firstDigit-1]) != string::npos) &&
               (firstDigit-1 != 0))
            firstDigit--;
        blockTitle = string(aPieceName, 0, firstDigit) + "s";
        blockPieceName = string(aPieceName, 0, firstDigit);
        for (i = 0; i < numPieces; i++)
            blockNames.push_back(pieceNodes[i]->text);
    }
    else
    {
       blockTitle = "blocks";
       blockPieceName = "block";
    }

    //
    // Read the domain to file "part" map
    //
    hsize = 0;
    domToFileMap = 0;
    ReadDataset(0, "domtofilepart", 0, &hsize, (void**) &domToFileMap);
    if (hsize != numPieces)
    {
        EXCEPTION2(UnexpectedValueException, hsize, numPieces);
    }

    //
    // See of domain0 block has a nodes field of name 'z' to
    // decide if we've got a 2D or 3D mesh
    //
    spatialDim = 2;
    sprintf(tempStr, "/%s/%s/node/Fields/z", top->child[0]->text,
        pieceNodes[0]->text);
    char fileName[1024];
    GetFileNameForRead(0, fileName, sizeof(fileName));
    if (ReadDataset(fileName, tempStr, 0, 0, 0))
        spatialDim = 3;

    //
    // Add the GLOBAL mesh object
    //
    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = CXX_strdup(top->child[0]->text);
    mesh->meshType = AVT_UNSTRUCTURED_MESH;
    mesh->topologicalDimension = spatialDim;
    mesh->spatialDimension = spatialDim;
    cerr << "WARNING!!! Assuming both topological and spatial dimension is " << spatialDim << endl;
    mesh->numBlocks = numPieces;
    mesh->blockTitle = blockTitle;
    mesh->blockPieceName = blockPieceName;
    mesh->blockNames = blockNames;
    md->Add(mesh);

    //
    // Loop over two types of global fields; node-centered and zone-centered
    // Any fields we encounter that are NOT global field have their Vista Tree
    // nodes pushed onto nonGlobalDomainLevelFieldNames 
    //
    vector<string> nonGlobalDomainLevelFieldNames;
    for (j = 0; j < 2; j++)
    {
        int numFieldNodes;
        Node **fieldNodes;
        avtCentering centering = (j==0 ? AVT_NODECENT : AVT_ZONECENT);

        // Find all domain-level fields
        if (centering == AVT_NODECENT)
            VisitFindNodes(top, "/.*/domain[0-9]{1,}/node/%F.*", &fieldNodes, &numFieldNodes);
        else
            VisitFindNodes(top, "/.*/domain[0-9]{1,}/elem/%F.*", &fieldNodes, &numFieldNodes);

        // Every domain-level, field that appears numPieces times is a global field.
        // So, build a map that counts number of occurances of each field's name
        map<string, IMVal<int,0> > fieldMap;
        for (i = 0; i < numFieldNodes; i++)
           fieldMap[fieldNodes[i]->text].val++;

        //
        // Add high-rank fields (vector/tensor) using apriori knowledge of Ale3d's
        // field names.
        //
        if (centering == AVT_NODECENT)
        {
            const char  *zstr = spatialDim == 3 ?  "z" : 0;
            const char *zdstr = spatialDim == 3 ? "zd" : 0;
            const char *z0str = spatialDim == 3 ? "z0" : 0;

            if (HasAllComponents(fieldMap, numPieces, "x", "y", zstr, 0))
            {
                // set counts to zero these entries won't get processed in loop below 
                fieldMap["x"].val = fieldMap["y"].val = 0; if (zstr) fieldMap["z"].val = 0;
                AddVectorVarToMetaData(md, "coords", mesh->name, centering, spatialDim);
            }

            if (HasAllComponents(fieldMap, numPieces, "xd", "yd", zdstr, 0))
            {
                // set counts to zero these entries won't get processed in loop below 
                fieldMap["xd"].val = fieldMap["yd"].val = 0; if (zdstr) fieldMap["zd"].val = 0;
                AddVectorVarToMetaData(md, "velocity", mesh->name, centering, spatialDim);
            }

            if (HasAllComponents(fieldMap, numPieces, "x0", "y0", z0str, 0))
            {
                // set counts to zero these entries won't get processed in loop below 
                fieldMap["x0"].val = fieldMap["y0"].val = 0; if (z0str) fieldMap["z0"].val = 0;
                AddVectorVarToMetaData(md, "coords0", mesh->name, centering, spatialDim);
            }
        }
        else
        {
            // This tensor isn't quite right. Need help from Al or Jeff
            if (HasAllComponents(fieldMap, numPieces, "sx", "sy", "p", "txy", "txz", "tyz", 0))
            {
                // set counts to zero these entries won't get processed in loop below 
                // don't set "p" to zero because we want that as a scalar field, too
                fieldMap["sx"].val = fieldMap["sy"].val = fieldMap["txy"].val = 
                    fieldMap["txz"].val = fieldMap["tyz"].val = 0;
                AddSymmetricTensorVarToMetaData(md, "stress", mesh->name, centering, 6);
            }
        }

        // Ok, now run through the map and find which fields were counted exactly
        // numPieces time and which were less (there should not be any that occured
        // more than numPieces times)
        map<string, IMVal<int,0> >::const_iterator mi;
        for (mi = fieldMap.begin(); mi != fieldMap.end(); mi++)
        {
            if (mi->second.val == numPieces)
                AddScalarVarToMetaData(md, mi->first, mesh->name, centering);
            else if (mi->second.val > 0)
                nonGlobalDomainLevelFieldNames.push_back(mi->first);
        }

        delete [] fieldNodes;
    }

    //
    // Materials
    //
    int numMatNodes;
    Node **matNodes;
    VisitFindNodes(top, "/.*/domain[0-9]{1,}/elem"
                        "/[0-9]{1,}[-_.:;=#+@ ]{0,1}[0-9]{1,}", &matNodes, &numMatNodes);

    //
    // Go through and find unique material names
    //
    groupNames.clear();
    vector<string> matNames;
    vector<vector<string> > matGroups;
    for (i = 0; i < numMatNodes; i++)
        matNames.push_back(matNodes[i]->text);
    StringHelpers::GroupStrings(matNames, matGroups, groupNames, 3, "");

    //
    // Process the group names to extract either names or numbers
    // and populate this object's knowledge of materials
    //
    for (i = 0; i < groupNames.size(); i++)
    {
        char matName[256]; 
        int matNum1, matNum2;
        const char *groupName = groupNames[i].c_str();

        if ((sscanf(groupName, "%d_%d", &matNum1, &matNum2) == 2) ||
            (sscanf(groupName, "%d_%d_mix", &matNum1, &matNum2) == 2))
        {
            sprintf(tempStr, "%d", matNum1);
            materialNames.push_back(tempStr);
            materialNumbers.push_back(matNum2);
        }
        else if ((sscanf(groupName, "%s_%d", matName, &matNum2) == 2) ||
                 (sscanf(groupName, "%s_%d_mix", matName, &matNum2) == 2))
        {
            materialNames.push_back(matName);
            materialNumbers.push_back(matNum2);
        }
        else
        {
            sprintf(tempStr, "%d", i);
            materialNames.push_back(tempStr);
            materialNumbers.push_back(i);
        }
    }
    numMaterials = materialNames.size();

    //
    // Populate the equivalent 'Array' forms of material names/numbers
    //
    materialNumbersArray = new int[numMaterials];
    materialNamesArray = new const char*[numMaterials];
    for (i = 0; i < numMaterials; i++)
    {
        materialNumbersArray[i] = materialNumbers[i];
        materialNamesArray[i] = materialNames[i].c_str();
    }

    //
    // Add the material object to the metadata
    //
    avtMaterialMetaData *mmd = new avtMaterialMetaData("materials", mesh->name,
        materialNames.size(), materialNames);
    md->Add(mmd);
}

// ****************************************************************************
//  Method: avtVistaFileFormat::GetAuxiliaryData
//
//  Purpose:
//      Gets the auxiliary data from a Vista file.
//
//  Arguments:
//      var        The variable of interest.
//      domain     The domain of interest.
//      type       The type of auxiliary data.
//      <unnamed>  The arguments for that -- not used for any Vista types.
//
//  Returns:    The auxiliary data.  Throws an exception if this is not a
//              supported data type.
//
//  Programmer: Mark C. Miller
//  Creation:   April 27, 2004
//
// ****************************************************************************

void *
avtVistaFileFormat::GetAuxiliaryData(const char *var, int domain,
                              const char *type, void *, DestructorFunction &df)
{
    void *rv = NULL;
    if (strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        rv = (void *) GetMaterial(domain, var);
        df = avtMaterial::Destruct;
    }

    return rv;
}


// ****************************************************************************
//  Method: avtVistaFileFormat::GetMaterial
//
//  Purpose:
//      Gets the specified material.
//
//  Arguments:
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      var         The name of the material object of interest.
//
//  Programmer: Mark C. Miller
//  Creation:   April 27, 2004 
//
// ****************************************************************************
avtMaterial *
avtVistaFileFormat::GetMaterial(int domain, const char *var)
{
    int i;
    const Node *top = vTree->GetTop();

    //
    // Locate all the Vista material views underneath this domain view
    //
    int numMatNodes;
    Node **matNodes;
    SNPRINTF(tempStr, sizeof(tempStr),
             "/.*/domain%d{1,}/elem/[0-9]{1,}[-_.:;=#+@ ]{0,1}[0-9]{1,}", domain);
    VisitFindNodes(top, tempStr, &matNodes, &numMatNodes);

    //
    // Loop and populate matMap with lists of clean and mixed zones
    //
    vector<MatZoneMap> matMap;
    for (i = 0; i < numMatNodes; i++)
    {
        MatZoneMap mapEntry;

        //
        // Skip mixed entries as we deal with those in the attempt to get 
        // mix information below 
        //
        if (StringHelpers::FindRE(matNodes[i]->text,"_mix$") >= 0)
            continue;

        //
        // Get this material's number
        //
        string matName;
        char *p = matNodes[i]->text;
        while (!isdigit(*p))
            matName += *p++;
        int matNum = *p - '0';
        while (isdigit(*(++p)))
        {
            matNum *= 10;
            matNum += (*p - '0');
        }
        if (matName == "")
        {
            SNPRINTF(tempStr, sizeof(tempStr), "%d", matNum);
            matName = tempStr;
        }

        mapEntry.matno = matNum;
        mapEntry.name = matName;

        //
        // read the clean index set
        //
        char fileName[1024];
        hsize_t dSize = 0;
        int *cleanIndexSet= 0;
        GetFileNameForRead(domain, fileName, sizeof(fileName));
        SNPRINTF(tempStr, sizeof(tempStr), "%s/Indexset", vTree->GetPathFromNode(matNodes[i])); 
        ReadDataset(fileName, tempStr, 0, &dSize, (void**) &cleanIndexSet);
        if (dSize != matNodes[i]->len)
        {
            EXCEPTION2(UnexpectedValueException, matNodes[i]->len, dSize);
        }

        mapEntry.numClean = matNodes[i]->len;
        mapEntry.cleanZones = cleanIndexSet;

        //
        // Look for the mixed Vista node
        //
        SNPRINTF(tempStr, sizeof(tempStr), "%s_mix", vTree->GetPathFromNode(matNodes[i])); 
        const Node *mixedNode = vTree->GetNodeFromPath(top, tempStr);
        if (mixedNode)
        {

            //
            // read the mixed index set
            //
            dSize = 0;
            int *mixedIndexSet= 0;
            SNPRINTF(tempStr, sizeof(tempStr), "%s/Indexset", vTree->GetPathFromNode(mixedNode)); 
            ReadDataset(fileName, tempStr, 0, &dSize, (void**) &mixedIndexSet);
            if (dSize != mixedNode->len)
            {
                EXCEPTION2(UnexpectedValueException, mixedNode->len, dSize);
            }

            //
            // read the volume fractions
            //
            dSize = 0;
            double *dvf = 0;
            SNPRINTF(tempStr, sizeof(tempStr), "%s/Fields/vf", vTree->GetPathFromNode(mixedNode)); 
            ReadDataset(fileName, tempStr, 0, &dSize, (void**) &dvf);
            if (dSize != mixedNode->len)
            {
                EXCEPTION2(UnexpectedValueException, mixedNode->len, dSize);
            }

            // convert double to float
            float *vf = new float[mixedNode->len];
            int j;
            for (j = 0; j < mixedNode->len; j++)
                vf[j] = dvf[j];
            delete [] dvf;

            mapEntry.numMixed = mixedNode->len;
            mapEntry.mixedZones = mixedIndexSet;
            mapEntry.volFracs = vf;
        }
        else
        {
            mapEntry.numMixed = 0;
            mapEntry.mixedZones = 0; 
            mapEntry.volFracs = 0;
        }

        matMap.push_back(mapEntry);
    }

    //
    // Find 'elem' Vista nodes beneath this domain's Vista node
    //
    int numElemViews = 0;
    Node **elemViews = 0;
    VisitFindNodes(pieceNodes[domain], "/%Velem", &elemViews, &numElemViews);
    if ((numElemViews == 0) || (numElemViews > 1))
    {
        EXCEPTION2(UnexpectedValueException, 1, numElemViews);
    }

    //
    // Construct the object we can here for, avtMaterial
    //
    int numElems = elemViews[0]->len;
    int dims[1] = {numElems};
    int major_order = 0;
    SNPRINTF(tempStr, sizeof(tempStr), "%d", domain);
    avtMaterial *mat = new avtMaterial(numMaterials, materialNumbersArray,
                              materialNamesArray, matMap, 1, dims, major_order,
                              tempStr);

    //
    // clean up 
    //
    delete [] matNodes;
    delete [] elemViews;
    for (i = 0; i < matMap.size(); i++)
    {
        if (matMap[i].cleanZones != 0)
            delete [] matMap[i].cleanZones;
        if (matMap[i].mixedZones != 0)
            delete [] matMap[i].mixedZones;
        if (matMap[i].volFracs != 0)
            delete [] matMap[i].volFracs;
    }

    return mat;
}

// ****************************************************************************
//  Method: avtVistaFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Mark C. Miller
//  Creation:   Tue Feb 17 19:19:07 PST 2004
//
//  Modifications:
//
//    Mark C. Miller, Wed May 19 10:56:11 PDT 2004
//    Added support for 2D meshes
//
// ****************************************************************************

vtkDataSet *
avtVistaFileFormat::GetMesh(int domain, const char *meshname)
{
    int i;
    const Node *top = vTree->GetTop();

    //
    // Find 'elem' and 'node' Vista nodes beneath this domain's Vista node
    //
    int numElemViews = 0;
    Node **elemViews = 0;
    VisitFindNodes(pieceNodes[domain], "/%Velem", &elemViews, &numElemViews);
    if ((numElemViews == 0) || (numElemViews > 1))
    {
        EXCEPTION2(UnexpectedValueException, 1, numElemViews);
    }
    int numNodeViews = 0;
    Node **nodeViews = 0;
    VisitFindNodes(pieceNodes[domain], "/%Vnode", &nodeViews, &numNodeViews);
    if ((numNodeViews == 0) || (numNodeViews > 1))
    {
        EXCEPTION2(UnexpectedValueException, 1, numNodeViews);
    }

    int numElems = elemViews[0]->len;
    int numNodes = nodeViews[0]->len;

    delete [] elemViews;
    delete [] nodeViews;

    //
    // Figure out which file to read from
    //
    char fileName[1024];
    GetFileNameForRead(domain, fileName, sizeof(fileName));

    //
    // Read coordinate arrays
    //
    double *coords[3] = {0, 0, 0};
    for (i = 0; i < spatialDim; i++)
    {
        sprintf(tempStr, "/%s/%s/node/Fields/%c", top->child[0]->text,
            pieceNodes[domain]->text, (char) ('x'+i));

        hsize_t dSize = 0;
        ReadDataset(fileName, tempStr, 0, &dSize, (void**) &coords[i]);

        if (dSize != numNodes)
        {
            EXCEPTION2(UnexpectedValueException, numNodes, dSize);
        }
    }
    for (i = spatialDim; i < 3; i++)
    {
        int j;
        coords[i] = new double[numNodes];
        for (j = 0; j < numNodes; j++)
            coords[i][j] = 0.0;
    }

    //
    // Read elem to node relation (connectivity)
    //
    int numNodesPerElem = 0;
    int *elemToNode = 0;
    {
        sprintf(tempStr, "/%s/%s/elem/Relations/elemToNode", top->child[0]->text,
            pieceNodes[domain]->text);

        hsize_t dSize = 0;
        ReadDataset(fileName, tempStr, 0, &dSize, (void**) &elemToNode);

        numNodesPerElem = dSize / numElems;

        if ((numNodesPerElem != 4) && (numNodesPerElem != 8))
        {
            EXCEPTION2(UnexpectedValueException, numNodesPerElem, "4 or 8");
        }
    }

    //
    // Populate the coordinates.  Put in 3D points with z=0 if the mesh is 2D.
    //
    vtkPoints            *points  = vtkPoints::New();
    points->SetNumberOfPoints(numNodes);
    float *pts = (float *) points->GetVoidPointer(0);
    float *tmp = pts;
    const double *coords0 = coords[0];
    const double *coords1 = coords[1];
    const double *coords2 = coords[2];
    for (int i = 0 ; i < numNodes; i++)
    {
        *tmp++ = *coords0++;
        *tmp++ = *coords1++;
        *tmp++ = *coords2++;
    }
    delete [] coords[0];
    delete [] coords[1];
    delete [] coords[2];

    //
    // Ok, build the VTK unstructured grid object
    // It turns out that Ale3d's internal node order is identical to VTK's
    //
    vtkUnstructuredGrid    *ugrid   = vtkUnstructuredGrid::New();
    ugrid->SetPoints(points);
    ugrid->Allocate(numElems * numNodesPerElem);
    for (i = 0; i < numElems; i++)
    {
        int vtkCellType = VTK_HEXAHEDRON;
        if (numNodesPerElem == 4)
           vtkCellType = VTK_QUAD;
        ugrid->InsertNextCell(vtkCellType, numNodesPerElem, &elemToNode[i*numNodesPerElem]);
    }
    points->Delete();
    return ugrid;
}


// ****************************************************************************
//  Method: avtVistaFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Mark C. Miller
//  Creation:   Tue Feb 17 19:19:07 PST 2004
//
// ****************************************************************************

vtkDataArray *
avtVistaFileFormat::GetVar(int domain, const char *varname)
{
    return ReadVar(domain, varname);
}

// ****************************************************************************
//  Method: avtVistaFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Mark C. Miller
//  Creation:   Tue Feb 17 19:19:07 PST 2004
//
// ****************************************************************************

vtkDataArray *
avtVistaFileFormat::GetVectorVar(int domain, const char *varname)
{
    return ReadVar(domain, varname);
}

// ****************************************************************************
//  Method: avtVistaFileFormat::ReadVar
//
//  Purpose:
//      Reads scalar, vector or tensor data into a vtkFloatArray. 
//
//  Programmer: Mark C. Miller
//  Creation:   March 15, 2004 
//
//  Modifications:
//
//    Mark C. Miller, Wed May 19 10:56:11 PDT 2004
//    Added support for 2D meshes
//
// ****************************************************************************

vtkFloatArray *
avtVistaFileFormat::ReadVar(int domain, const char *visitName)
{
    int i, j;
    const Node *top = vTree->GetTop();

    vector<string> vistaNames;
    VisitNameToVistaName(visitName, vistaNames, spatialDim);
    int numComponents = vistaNames.size();

    //
    // Find 'elem' and 'node' Vista nodes beneath this domain's Vista node
    //
    int numElemViews = 0;
    Node **elemViews = 0;
    VisitFindNodes(pieceNodes[domain], "/%Velem", &elemViews, &numElemViews);
    if ((numElemViews == 0) || (numElemViews > 1))
    {
        EXCEPTION2(UnexpectedValueException, 1, numElemViews);
    }
    int numNodeViews = 0;
    Node **nodeViews = 0;
    VisitFindNodes(pieceNodes[domain], "/%Vnode", &nodeViews, &numNodeViews);
    if ((numNodeViews == 0) || (numNodeViews > 1))
    {
        EXCEPTION2(UnexpectedValueException, 1, numNodeViews);
    }

    int numElems = elemViews[0]->len;
    int numNodes = nodeViews[0]->len;

    delete [] elemViews;
    delete [] nodeViews;

    //
    // Figure out which file to read from
    //
    char fileName[1024];
    GetFileNameForRead(domain, fileName, sizeof(fileName));

    //
    // Read all the component's data
    //
    int numVals;
    double **compData = new double*[numComponents];
    for (i = 0; i < numComponents; i++)
    {
        hsize_t dSize;

        // try to read from 'elem' fields
        sprintf(tempStr, "/%s/%s/elem/Fields/%s", top->child[0]->text,
            pieceNodes[domain]->text, vistaNames[i].c_str());

        compData[i] = 0;
        if (ReadDataset(fileName, tempStr, 0, &dSize, (void**) &compData[i]))
        {
            if (dSize != numElems)
            {
                EXCEPTION2(UnexpectedValueException, numElems, dSize);
            }
            numVals = numElems;
            continue;
        }

        // try to read from 'node' fields
        sprintf(tempStr, "/%s/%s/node/Fields/%s", top->child[0]->text,
            pieceNodes[domain]->text, vistaNames[i].c_str());

        compData[i] = 0;
        if (ReadDataset(fileName, tempStr, 0, &dSize, (void**) &compData[i]))
        {
            if (dSize != numNodes)
            {
                EXCEPTION2(UnexpectedValueException, numElems, dSize);
            }
            numVals = numNodes;
        }
        else
        {
            // if we get here, we must not have been able to read
            // the variable as either zone- or node-centered
            EXCEPTION1(InvalidVariableException, visitName);
        }
    }

    //
    // Regardless of what kind of variable we have in Vista,
    // VTK supports only scalar (1 component), vector (3 component)
    // of tensor (9 compoenent) variables. So, do the mapping here.
    //
    avtVarType varType = GuessVarTypeFromNumDimsAndComps(spatialDim, numComponents);
    if (varType == AVT_UNKNOWN_TYPE)
    {
        EXCEPTION1(InvalidVariableException, visitName);
    }

    int numAllocComponents = 0;
    switch (varType)
    {
        case AVT_SCALAR_VAR: numAllocComponents = 1; break;
        case AVT_VECTOR_VAR: numAllocComponents = 3; break;
        case AVT_SYMMETRIC_TENSOR_VAR:
        case AVT_TENSOR_VAR: numAllocComponents = 9; break;
        default: break;
    }
    if (numAllocComponents == 0)
    {
        EXCEPTION2(UnexpectedValueException, numAllocComponents, ">0");
    }

    vtkFloatArray *var_data = vtkFloatArray::New();
    var_data->SetNumberOfComponents(numAllocComponents);
    var_data->SetNumberOfTuples(numVals);
    float *fbuf = (float*) var_data->GetVoidPointer(0);
    for (i = 0; i < numVals; i++)
    {
        for (j = 0; j < numComponents; j++)
            *fbuf++ = compData[j][i];
        for (j = numComponents; j < numAllocComponents; j++)
            *fbuf++ = 0.0;
    }

    // clean-up
    for (i = 0; i < numComponents; i++)
        delete [] compData[i];
    delete [] compData;

    return var_data;
}
