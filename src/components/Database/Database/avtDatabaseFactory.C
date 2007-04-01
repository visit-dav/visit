// ************************************************************************* //
//                          avtDatabaseFactory.C                             //
// ************************************************************************* //
#include <errno.h> // for errno
#include <fstream.h>
#include <sys/stat.h>

#include <vector>
#include <string>

#include <visit-config.h>

#include <avtDatabase.h>
#include <avtDatabaseFactory.h>
#include <avtGenericDatabase.h>

#include <DatabasePluginManager.h>
#include <DatabasePluginInfo.h>
#include <Utility.h>

#include <BadPermissionException.h>
#include <FileDoesNotExistException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidFilesException.h>
#include <InvalidDBTypeException.h>

#if !defined(_WIN32)
#include <unistd.h>
#else
#include <snprintf.h>
#endif


using std::string;
using std::vector;

//
// Function Prototypes
//

void CheckPermissions(const char *);

// ****************************************************************************
//  Method: avtDatabaseFactory::FileList
//
//  Purpose:
//      Looks through the file list and tries to guess what the appropriate
//      derived type of database the files correspond to.
//
//  Arguments:
//      filelist     A list of files.
//      filelistN    The number of files in `filelist'.
//
//  Returns:    An avtDatabase object that is correctly typed for the filelist.
//
//  Note:       It is assumed that the file list is homogeneous.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Tue Aug 27 14:30:22 PDT 2002
//    Made databases be plugins.
//
//    Hank Childs, Mon Oct 14 08:48:40 PDT 2002
//    Allow for multiple formats to support the same extension.  Blew away
//    outdated comments.
//
//    Hank Childs, Thu Feb 19 08:00:35 PST 2004
//    Allow for exact filenames in addition to default extensions.
//
//    Hank Childs, Mon Mar  1 08:56:52 PST 2004
//    Allow for the time to be specified.
//
// ****************************************************************************

avtDatabase *
avtDatabaseFactory::FileList(const char * const * filelist, int filelistN,
                             int timestep)
{
    int   i, j;

    if (filelistN <= 0)
    {
        EXCEPTION1(InvalidFilesException, filelistN);
    }

    avtDatabase *rv = NULL;
    int fileIndex = 0;

    int nBlocks = 1;
    if (strstr(filelist[fileIndex], "!NBLOCKS") != NULL)
    {
        nBlocks = atoi(filelist[fileIndex] + strlen("!NBLOCKS "));
        debug1 << "Found a multi-block file with " << nBlocks << " blocks."
               << endl;
        fileIndex++;
    }
    if (nBlocks <= 0)
    {
        debug1 << "BAD SYNTAX FOR N BLOCKS, RESETTING TO 1"  << endl;
        nBlocks = 1;
    }

    //
    // Make sure we can read the file before we proceed.
    //
    CheckPermissions(filelist[fileIndex]);

    //
    // Parse out the path and get just the filename.
    //
    string file_and_path = filelist[fileIndex];
    const char *fap = file_and_path.c_str();
    int len = strlen(fap);
    int lastSlash = -1;
    for (i = len-1 ; i >= 0 ; i--)
    {
        if (fap[i] == SLASH_CHAR)
        {
            lastSlash = i;
            break;
        }
    }
    int start = lastSlash+1;
    string file(fap + start);

    string defaultDatabaseType("Silo");

    //
    // Try each database type looking for a match to the given extensions
    //
    DatabasePluginManager *dbmgr = DatabasePluginManager::Instance();
    for (i=0; i<dbmgr->GetNEnabledPlugins() && rv == NULL; i++)
    {
        string id = dbmgr->GetEnabledID(i);
        CommonDatabasePluginInfo *info = dbmgr->GetCommonPluginInfo(id);
        bool foundMatch = false;

        //
        // Check to see if there is an extension that matches.
        //
        vector<string> extensions = info->GetDefaultExtensions();
        int nextensions = extensions.size();
        for (j=0; j<nextensions; j++)
        {
            string ext = extensions[j];
            if (ext[0] != '.')
            {
                ext = string(".") + extensions[j];
            }
            if (file.length() >= ext.length() &&
                file.substr(file.length() - ext.length()) == ext)
            {
                foundMatch = true;
            }
        }

        //
        // Check to see if there is an exact name that matches.
        //
        vector<string> filenames = info->GetFilenames();
        int nfiles = filenames.size();
        for (j=0; j<nfiles; j++)
        {
            if (filenames[j] == file)
            {
                foundMatch = true;
            }
        }

        if (foundMatch)
        {
            TRY
            {
                rv = info->SetupDatabase(filelist+fileIndex,
                                         filelistN-fileIndex, nBlocks);

                //
                // By policy, the plugin doesn't do much work to set up the
                // database.  So there is a chance that a format did not
                // throw an exception, but is the wrong type.  To make
                // sure we have the correct plugin, force it to read
                // in its metadata.  This does not cause extra work,
                // because the metadata is cached and we are going to
                // ask for it in a bit anyway.
                //
                if (rv != NULL)
                    rv->GetMetaData(timestep);
            }
            CATCH2(InvalidDBTypeException, e)
            {
                rv = NULL;
            }
            ENDTRY
        }
    }

    //
    // If no file extension match, then we default to the given database type
    //
    if (rv == NULL)
    {
        int defaultindex = dbmgr->GetAllIndexFromName(defaultDatabaseType);
        if (defaultindex != -1)
        {
            string defaultid = dbmgr->GetAllID(defaultindex);
            CommonDatabasePluginInfo *info = 
                                         dbmgr->GetCommonPluginInfo(defaultid);
            rv = info->SetupDatabase(filelist+fileIndex, filelistN-fileIndex,
                                     nBlocks);

            //
            // If we match an extension or an exact filename, we call get the
            // database metadata to make sure we have the right plugin.  We 
            // make the same call here to be consistent.
            //
            if (rv != NULL)
                rv->GetMetaData(timestep);
        }
        else
        {
            char msg[1000];
            sprintf(msg,
                    "The default file format plugin '%s' was not available.",
                    defaultDatabaseType.c_str());
            EXCEPTION1(ImproperUseException, msg);
        }
    }

    return rv;
}


// ****************************************************************************
//  Method: avtDatabaseFactory::VisitFile
//
//  Purpose:
//      Gets an avtDatabase from just the .visit file.
//
//  Arguments:
//      visitFile   The name of the .visit file.
//
//  Returns:    An avtDatabase object that is correctly typed for the .visit
//              file.
//
//  Programmer: Hank Childs
//  Creation:   September 18, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Mar  2 11:22:58 PST 2001
//    Modified routine to just read in the file list and pass it
//    to the file list routine.
//
//    Hank Childs, Wed Nov  7 15:09:08 PST 2001
//    Check permissions of the first file before starting.
//
//    Jeremy Meredith, Tue Aug 27 15:17:57 PDT 2002
//    Moved GetFileListFromTextFile to be a static avtDatabase function.
//
//    Brad Whitlock, Wed Mar 26 14:24:11 PST 2003
//    I added code to prepend the path to the VisIt file onto all of the
//    timestep files if they are relative. The engine was crashing on Windows
//    because it did not know where to get the timestep files.
//
//    Hank Childs, Mon Mar  1 08:56:52 PST 2004
//    Allow for the time to be specified.
//
// ****************************************************************************

avtDatabase *
avtDatabaseFactory::VisitFile(const char *visitFile, int timestep)
{
    //
    // Make sure we can read the file before we proceed.
    //
    CheckPermissions(visitFile);

    //
    // Get the list of files from the .visit file.
    //
    char  **reallist  = NULL;
    int     listcount = 0;
    avtDatabase::GetFileListFromTextFile(visitFile, reallist, listcount);

#if defined(_WIN32)
    //
    // Get the path out of the VisIt file and prepend it to all of the
    // timestep filenames.
    //
    string visitPath(visitFile);
    std::string::size_type sepIndex = visitPath.find_last_of("\\/");
    if(sepIndex != std::string::npos)
    {
        visitPath = visitPath.substr(0, sepIndex + 1);

        int s = visitPath.size();
        string curDir("./");
        for(int j = 0; j < listcount; ++j)
        {
            string fileName(reallist[j]);

            // If the filename contains a colon, assume that it contains
            // the whole path to the file. We don't need to prepend a 
            // path if it already contains one.
            if(fileName.find(":") != std::string::npos)
                continue;

            // If the filename begins with "./", remove it.
            if(fileName.substr(0,2) == curDir)
                fileName = fileName.substr(2, fileName.size() - 2);

            // Create a new filename that has the path prepended to it.
            int len = fileName.size() + s + 2;
            char *name = new char[len];
            SNPRINTF(name, len, "%s%s", visitPath.c_str(), fileName.c_str());
            delete [] reallist[j];
            reallist[j] = name;
        }
    }
#endif

    //
    // Create a database using the list of files.
    //
    avtDatabase *rv = FileList(reallist, listcount, timestep);

    //
    // Clean up memory
    //
    for (int i = 0 ; i < listcount ; i++)
    {
        delete [] reallist[i];
    }
    delete [] reallist;

    return rv;
}


// ****************************************************************************
//  Function: CheckPermissions
//
//  Purpose:
//      Makes sure that we have read permission to the file.
//
//  Arguments:
//      filename   The fully qualified (wrt to path) name of the file.
//
//  Programmer: Hank Childs
//  Creation:   November 7, 2001
//
//  Modifications:
//    Brad Whitlock, Tue Apr 2 14:21:07 PST 2002
//    Added a Windows implementation.
//
//    Hank Childs, Mon May  6 16:03:04 PDT 2002
//    Don't read uninitialized memory if the stat failed.
//
//    Brad Whitlock, Fri Jul 26 11:33:07 PDT 2002
//    I changed the code so it gives a different kind of exception if the
//    desired file does not exist.
//
// ****************************************************************************

#if defined(_WIN32)
void
CheckPermissions(const char *filename)
{
   // nothing
}

#else
static bool    setUpUserInfo = false;
static uid_t   uid;
static gid_t   gids[100];
static int     ngids;

void
SetUpUserInfo(void)
{
    setUpUserInfo = true;
    uid = getuid();
    ngids = getgroups(100, gids);
}

void
CheckPermissions(const char *filename)
{
    if (!setUpUserInfo)
    {
        SetUpUserInfo();
    }

    struct stat s;
    int rv = stat(filename, &s);
    if (rv < 0)
    {
        if(errno == ENOENT || errno == ENOTDIR)
        {
            EXCEPTION1(FileDoesNotExistException, filename);
        }
        else
        {
            EXCEPTION1(BadPermissionException, filename);
        }
    }
    mode_t mode = s.st_mode;

    //
    // If other has permission, then we are set.
    //
    if (mode & S_IROTH)
    {
        return;
    }

    //
    // If we are the user and the user has permission, then we are set.
    //
    bool isuser =  (s.st_uid == uid);
    if (isuser && (mode & S_IRUSR))
    {
        return;
    }

    //
    // If we are in the group and the group has permission, then we are set.
    //
    bool isgroup = false;
    for (int i = 0 ; i < ngids ; i++)
    {
        if (gids[i] == s.st_gid)
        {
            isgroup = true;
        }
    }
    if (isgroup && (mode & S_IRGRP))
    {
        return;
    }

    EXCEPTION1(BadPermissionException, filename);
}

#endif
