// ************************************************************************* //
//                            avtVistaFileFormat.C                           //
// ************************************************************************* //
// this include directive refernces code provided by Jeff Keasler. I am
// including it using an include directive so that as Jeff updates this
// code, it can be easily incorporated into the plugin 
#include <VisitALE.c>

#include <visit-config.h>

#define HAVE_HDF5 ((HAVE_HDF5_H==1) && (HAVE_LIBHDF5==1))

#if HAVE_HDF5
#include <hdf5.h>
#endif
#include <silo.h>

#include <stdarg.h>
#include <stdio.h>

#include <snprintf.h>

#include <string>
#include <map>

#include <StringHelpers.h>
#include <avtCallback.h>
#include <avtVistaFileFormat.h>
#include <avtVistaAle3dFileFormat.h>
#include <avtVistaDiabloFileFormat.h>

#include <DebugStream.h>

#include <BadIndexException.h>
#include <InvalidFilesException.h>
#include <UnexpectedValueException.h>
#include <ImproperUseException.h>
#include <Utility.h>


using std::string;
using std::map;

avtVistaFileFormat::
VistaTree::VistaTree(const char *buf, size_t size)
{
    theVistaString = new char[size];
    memcpy(theVistaString, buf, size);
    VisitParseInternal(theVistaString, &top);
    ExtractAttr(top);
}

avtVistaFileFormat::
VistaTree::~VistaTree()
{
    VisitFreeVistaInfo(top);
    delete [] theVistaString;
}

const void
avtVistaFileFormat::
VistaTree::DumpTree() const
{
    VisitDumpTree(top);
}

const Node *
avtVistaFileFormat::
VistaTree::GetNodeFromPath(const Node *root, const char *path) const
{
    return VisitGetNodeFromPath(root, path);
}

char *
avtVistaFileFormat::
VistaTree::GetPathFromNode(const Node *root, const Node *node) const
{
    return VisitGetPathFromNode(root, node);
}

void
avtVistaFileFormat::
VistaTree::FindNodes(const Node *root, const char *re, Node ***results,
          int *nmatches, RecurseMode rmode) const
{
    VisitFindNodes(root, re, results, nmatches, rmode);
}

const Node *
avtVistaFileFormat::
VistaTree::GetTop() const
{
    return top;
}

const int avtVistaFileFormat::MASTER_FILE_INDEX = 0;
int avtVistaFileFormat::objcnt = 0;

#if HAVE_HDF5
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
#endif

// ****************************************************************************
//  Function:  avtVistaFileFormat::CreateFileFormatInterface
//
//  Programmer:  Mark C. Miller 
//  Creation:    July 14, 2004 
//
// ****************************************************************************
avtFileFormatInterface *
avtVistaFileFormat::CreateFileFormatInterface(const char *const *list, int nList)
{
    avtFileFormatInterface *ffi = 0;
    avtVistaFileFormat *vff = 0;

    if (list != 0 || nList > 0)
    {
        TRY
        {
            // Try and open the Vista file 
            vff = new avtVistaFileFormat(list[0]);

            if (vff == 0)
                return 0;

            switch (vff->GetFormatType())
            {
                case FTYPE_ALE3D:
                    ffi = avtVistaAle3dFileFormat::
                              CreateInterface(vff, list, nList);
                    break;
                case FTYPE_DIABLO:
                    ffi = avtVistaDiabloFileFormat::
                              CreateInterface(vff, list, nList);
                    break;
#if 0
                case FTYPE_UNKNOWN:
                    ffi = avtVistaUnknownFileFormat::
                        CreateInterface(vff, list, nList);
                    break;
#endif
                // case FTYPE_???: put new file formats here 
                default:
                    break;
            }

#if 0
            //
            // If we didn't succeed above, try the unknown format
            //
            if (ffi == 0)
            {
                ffi = avtVistaUnknownFileFormat::
                    CreateInterface(vff, list, nList);
            }
#endif

        }
        CATCH(VisItException)
        {
            if (vff)
                delete vff;
            RETHROW;
        }
        ENDTRY

    }

    // since the derived class' formats take ownership of the
    // vista file format, we can safely delete it
    if (vff)
        delete vff;

    return ffi;

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
    int filePart = chunkToFileMap[dom];
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
    wasMorphed = false;

    masterFileName = StringHelpers::Basename(filename);
    masterDirName = StringHelpers::Dirname(filename);

    fileHandles = new void*[MAX_FILES];
    for (int i = 0 ; i < MAX_FILES ; i++)
    {
        fileHandles[i] = 0;
    }

    //
    // Determine file format type and open it 
    //
    DBShowErrors(DB_NONE, 0);
    DBfile *dbfile = DBOpen((char *) filename, DB_UNKNOWN, DB_READ);
    if (dbfile != NULL)
    {
        isSilo = true;

        //
        // keep this file now and set error and precision behavior of silo 
        //
        DBForceSingle(1);
        DBShowErrors(DB_ALL, 0);
        fileHandles[MASTER_FILE_INDEX] = dbfile;
    }
#if HAVE_HDF5
    else
    {
        isSilo = false;

        //
        // Initialize HDF5 library
        //
        if (avtVistaFileFormat::objcnt == 0)
            InitializeHDF5();

        //
        // Open the HDF5 file
        //
        fileHandles[MASTER_FILE_INDEX] = (void *) new hid_t;
        *((hid_t*) fileHandles[MASTER_FILE_INDEX]) = 
            H5Fopen(filenames[MASTER_FILE_INDEX], H5F_ACC_RDONLY, H5P_DEFAULT);
    }
#endif
    RegisterFile(MASTER_FILE_INDEX);

    //
    // Read the name of the code that generated this vista file 
    //
    writerName = 0;
    ReadDataset(0, "dbtype", 0, 0, (void**) &writerName);
    if (writerName == 0)
    {
        const char *msg = "Unable to determine name of code that generated "
                          "this Vista file. Therefore, VisIt cannot decide "
                          "which Vista file format to use.";
        if (!avtCallback::IssueWarning(msg))
            cerr << msg << endl;
    }


    //
    // Determine format type
    //
    if (strcmp(writerName, "ALE3D") == 0)
        formatType = FTYPE_ALE3D;
    else if (strcmp(writerName, "DIABLO") == 0)
        formatType = FTYPE_DIABLO;
    else
        formatType = FTYPE_UNKNOWN;

    //
    // Read and construct the vista tree
    //
    size_t size = 0;
    char *buf = 0;
    ReadDataset(0, "VisIt", 0, &size, (void**) &buf);
    vTree = new VistaTree(buf, size);
    delete [] buf;

    //
    // Read the domain to file "part" map
    //
    chunkToFileMap = 0;
    size = 0;
    ReadDataset(0, "domtofilepart", 0, &size, (void**) &chunkToFileMap);
    numChunks = size;

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
    if (wasMorphed)
        return;

    FreeUpResources();

    if (writerName)
        delete [] writerName;

    if (fileHandles)
        delete [] fileHandles;

    if (chunkToFileMap)
        delete [] chunkToFileMap;

    avtVistaFileFormat::objcnt--;

    // handle HDF5 library termination on descrution of last instance
#if HAVE_HDF5
    if (!isSilo && (avtVistaFileFormat::objcnt == 0))
        FinalizeHDF5();
#endif

}

// ****************************************************************************
//  Method:  avtVistaFileFormat constructor
//
//  Purpose: Shallow copy various dynamically allocated objects. After calling
//  this method, the derived object that calls it owns all the dynamically
//  allocated stuff. The this object that this method was called on can be
//  destructed without it also destroying the dynamically allocated stuff it
//  once owned.
//
//  The 'this' object is expected to be one of the derived classes of 
//  avtVistaFileFormat.  This method is used in part of the process of
//  constructing a derived vista file format object. The first such derived
//  object constructed is basically morphed from a avtVistaFileFormat object. 
//
//  Programmer:  Mark C. Miller 
//  Creation:    July 15, 2004 
//
// ****************************************************************************
avtVistaFileFormat::avtVistaFileFormat(const char *filename,
    avtVistaFileFormat *fromObj) : avtSTMDFileFormat(&filename, 1)
{
    if (fromObj->wasMorphed)
    {
        EXCEPTION1(ImproperUseException, "file format object is already morphed");
    }

    wasMorphed     = false;

    writerName     = fromObj->writerName;
    formatType     = fromObj->formatType;
    masterFileName = fromObj->masterFileName;
    masterDirName  = fromObj->masterDirName;
    numChunks      = fromObj->numChunks;
    chunkToFileMap = fromObj->chunkToFileMap;
    vTree          = fromObj->vTree;
    fileHandles    = fromObj->fileHandles;
    isSilo         = fromObj->isSilo;

    RegisterFile(MASTER_FILE_INDEX);

    fromObj->wasMorphed = true;

    avtVistaFileFormat::objcnt++;
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
#if HAVE_HDF5
    else
    {
        fileHandles[f] = (void *) new hid_t;
        *((hid_t*) fileHandles[f]) = H5Fopen(filenames[f],
                                         H5F_ACC_RDONLY, H5P_DEFAULT);
    }
#endif

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
#if HAVE_HDF5
        else
        {
            H5Fclose(*((hid_t*) fileHandles[f]));
            delete (hid_t *) fileHandles[f];
        }
#endif
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
    VistaDataType *dataType, size_t *size, void **buf)
{
    VistaDataType retType;
    size_t retSize;
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
            retType = DTYPE_UNKNOWN;
            retSize = 0;
        }
        else
        {
            // examine the variable size
            retSize = DBGetVarLength(dbfile, (char *) dsName);

            // examine the variable type
            int type = DBGetVarType(dbfile, (char *) dsName);
            switch (type)
            {
                case DB_CHAR:   retType = DTYPE_CHAR;    break;
                case DB_INT:    retType = DTYPE_INT;     break;
                case DB_FLOAT:  retType = DTYPE_FLOAT;   break;
                case DB_DOUBLE: retType = DTYPE_DOUBLE;  break;
                default:        retType = DTYPE_UNKNOWN; break;
            }

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
#if HAVE_HDF5
    else
    {
        // open the dataset
        hid_t hdfFile= *((hid_t *) f);
        hid_t ds = H5Dopen(hdfFile, dsName);

        if (ds < 0)
        {
            retType = DTYPE_UNKNOWN;
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
            hid_t memType;
            H5T_class_t typeClass = H5Tget_class(type);
            size_t typeSize = H5Tget_size(type);
            if (typeClass == H5T_FLOAT)
            {
                if (typeSize == sizeof(float))
                {
                    retType = DTYPE_FLOAT;
                    memType = H5T_NATIVE_FLOAT;
                }
                else if (typeSize == sizeof(double))
                {
                    retType = DTYPE_DOUBLE;
                    memType = H5T_NATIVE_DOUBLE;
                }
                else
                {
                    retType = DTYPE_UNKNOWN;
                    memType = type;
                }
            }
            else if (typeClass == H5T_INTEGER)
            {
                if (typeSize == sizeof(char))
                {
                    retType = DTYPE_CHAR;
                    memType = H5T_NATIVE_CHAR;
                }
                else if (typeSize == sizeof(int))
                {
                    retType = DTYPE_INT;
                    memType = H5T_NATIVE_INT;
                }
                else
                {
                    retType = DTYPE_UNKNOWN;
                    memType = type;
                }
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
                H5Dread(ds, memType, H5S_ALL, H5S_ALL, H5P_DEFAULT, *buf);
            }

            H5Sclose(space);
            H5Tclose(type);
            H5Dclose(ds);
            retVal = true;
        }
    }
#endif

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

#warning should we delete other stuff during FreeUpResources

    // we don't finalize HDF5 here because its not clear if/when we might
    // need to re-initialize it, again
}
