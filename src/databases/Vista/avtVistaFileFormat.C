// ************************************************************************* //
//                            avtVistaFileFormat.C                           //
// ************************************************************************* //

// this include directive refernces code provided by Jeff Keasler. I am
// including it using an include directive so that as Jeff updates this
// code, it can be easily incorporated into the plugin 
#include <VisitALE.c>

#include <stdarg.h>
#include <libgen.h>

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


#define MATCH(A,B)        strncmp(A,B,sizeof(#B))==0

// ****************************************************************************
//  Function:  VisitNameToVistaName 
//
//  Purpose:   Convert a visit variable name to Vista list of vista scalar
//             variables 
//
//  Programmer:  Mark C. Miller 
//  Creation:    March 15, 2004 
//
// ****************************************************************************
static void
VisitNameToVistaName(const char *visitName, vector<string> &vistaNames)
{
    if (MATCH(visitName, "coords"))
    {
        vistaNames.push_back("x");
        vistaNames.push_back("y");
        vistaNames.push_back("z");
    }
    else if (MATCH(visitName, "coords0"))
    {
        vistaNames.push_back("x0");
        vistaNames.push_back("y0");
        vistaNames.push_back("z0");
    }
    else if (MATCH(visitName, "velocity"))
    {
        vistaNames.push_back("xd");
        vistaNames.push_back("yd");
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
//  Method: avtVistaFileFormat constructor
//
//  Programmer: Mark C. Miller
//  Creation:   Tue Feb 17 19:19:07 PST 2004
//
// ****************************************************************************

avtVistaFileFormat::avtVistaFileFormat(const char *filename)
    : avtSTMDFileFormat(&filename, 1)
{
    int i;

    vTree = 0;
    domToFileMap = 0;

    char *fname = CXX_strdup(filename);
    masterFileName = basename(fname);
    masterDirName = dirname(fname);
    delete [] fname;

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
        char reStr[256];
        sprintf(reStr,"/./%s[0-9]{1,}", groupNames[0].c_str());
        VisitFindNodes(top, reStr, &pieceNodes, &numPieces);
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
    // Add the GLOBAL mesh object
    //
    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = CXX_strdup(top->child[0]->text);
    mesh->meshType = AVT_UNSTRUCTURED_MESH;
    mesh->topologicalDimension = 3;
    mesh->spatialDimension = 3;
    cerr << "WARNING!!! Assuming topological and spatial dimension is 3" << endl;
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
            if (HasAllComponents(fieldMap, numPieces, "x", "y", "z", 0))
            {
                // set counts to zero these entries won't get processed in loop below 
                fieldMap["x"].val = fieldMap["y"].val = fieldMap["z"].val = 0;
                AddVectorVarToMetaData(md, "coords", mesh->name, centering, 3);
            }

            if (HasAllComponents(fieldMap, numPieces, "xd", "yd", "zd", 0))
            {
                // set counts to zero these entries won't get processed in loop below 
                fieldMap["xd"].val = fieldMap["yd"].val = fieldMap["zd"].val = 0;
                AddVectorVarToMetaData(md, "velocity", mesh->name, centering, 3);
            }

            if (HasAllComponents(fieldMap, numPieces, "x0", "y0", "z0", 0))
            {
                // set counts to zero these entries won't get processed in loop below 
                fieldMap["x0"].val = fieldMap["y0"].val = fieldMap["z0"].val = 0;
                AddVectorVarToMetaData(md, "coords0", mesh->name, centering, 3);
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
    VisitFindNodes(top, "/.*/[a-zA-Z]{1,}[-_.:;=#+@ ]{0,1}[0-9]{1,}/elem"
                        "/[a-zA-Z]{1,}[-_.:;=#+@ ]{0,1}[0-9]{1,}/%RIndexset", &matNodes, &numMatNodes);

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

    //
    // Figure out which file to read from
    //
    char fileName[1024];
    int filePart = domToFileMap[domain];
    if (filePart == MASTER_FILE_INDEX)
        strncpy(fileName, masterFileName.c_str(), sizeof(fileName));
    else
        sprintf(fileName, "%s.%d", masterFileName.c_str(), filePart);

    //
    // Read coordinate arrays
    //
    double *coords[3] = {0, 0, 0};
    for (i = 0; i < 3; i++)
    {
        char tmpName[256];
        sprintf(tmpName, "/%s/%s/node/Fields/%c", top->child[0]->text,
            pieceNodes[domain]->text, (char) ('x'+i));

        hsize_t dSize = 0;
        ReadDataset(fileName, tmpName, 0, &dSize, (void**) &coords[i]);

        if (dSize != numNodes)
        {
            EXCEPTION2(UnexpectedValueException, numNodes, dSize);
        }
    }

    //
    // Read elem to node relation (connectivity)
    //
    int *elemToNode = 0;
    {
        char tmpName[256];
        sprintf(tmpName, "/%s/%s/elem/Relations/elemToNode", top->child[0]->text,
            pieceNodes[domain]->text);

        hsize_t dSize = 0;
        ReadDataset(fileName, tmpName, 0, &dSize, (void**) &elemToNode);

        if (dSize != 8*numElems)
        {
            EXCEPTION2(UnexpectedValueException, 8*numElems, dSize);
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
    ugrid->Allocate(numElems * 8);
    for (i = 0; i < numElems; i++)
        ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, &elemToNode[i*8]);
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
// ****************************************************************************

vtkFloatArray *
avtVistaFileFormat::ReadVar(int domain, const char *visitName)
{
    int i, j;
    const Node *top = vTree->GetTop();

    vector<string> vistaNames;
    VisitNameToVistaName(visitName, vistaNames);
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

    //
    // Figure out which file to read from
    //
    char fileName[1024];
    int filePart = domToFileMap[domain];
    if (filePart == MASTER_FILE_INDEX)
        strncpy(fileName, masterFileName.c_str(), sizeof(fileName));
    else
        sprintf(fileName, "%s.%d", masterFileName.c_str(), filePart);

    //
    // Read all the component's data
    //
    int numVals;
    double **compData = new double*[numComponents];
    for (i = 0; i < numComponents; i++)
    {
        hsize_t dSize;
        char tmpName[256];

        // try to read from 'elem' fields
        sprintf(tmpName, "/%s/%s/elem/Fields/%s", top->child[0]->text,
            pieceNodes[domain]->text, vistaNames[i].c_str());

        compData[i] = 0;
        if (ReadDataset(fileName, tmpName, 0, &dSize, (void**) &compData[i]))
        {
            if (dSize != numElems)
            {
                EXCEPTION2(UnexpectedValueException, numElems, dSize);
            }
            numVals = numElems;
            continue;
        }

        // try to read from 'node' fields
        sprintf(tmpName, "/%s/%s/node/Fields/%s", top->child[0]->text,
            pieceNodes[domain]->text, vistaNames[i].c_str());

        compData[i] = 0;
        if (ReadDataset(fileName, tmpName, 0, &dSize, (void**) &compData[i]))
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

    vtkFloatArray *var_data = vtkFloatArray::New();
    var_data->SetNumberOfComponents(numComponents);
    var_data->SetNumberOfTuples(numVals);
    float *fbuf = (float*) var_data->GetVoidPointer(0);
    for (i = 0; i < numVals; i++)
    {
        for (j = 0; j < numComponents; j++)
            *fbuf++ = compData[j][i];
    }

    // clean-up
    for (i = 0; i < numComponents; i++)
        delete [] compData[i];
    delete [] compData;

    return var_data;
}
