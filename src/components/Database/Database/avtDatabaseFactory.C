/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
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
//                          avtDatabaseFactory.C                             //
// ************************************************************************* //

#include <avtDatabaseFactory.h>

#include <errno.h> // for errno
#include <visitstream.h>
#include <sys/stat.h>

#include <vector>
#include <string>

#include <visit-config.h>

#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <avtGenericDatabase.h>

#include <DatabasePluginManager.h>
#include <DatabasePluginInfo.h>
#include <Utility.h>
#include <DBOptionsAttributes.h>

#include <BadPermissionException.h>
#include <FileDoesNotExistException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidFilesException.h>
#include <InvalidDBTypeException.h>

#if !defined(_WIN32)
#include <unistd.h>
#endif
#include <snprintf.h>

using std::string;
using std::vector;

//
// Static data members
//
char    *avtDatabaseFactory::defaultFormat = "Silo";
char    *avtDatabaseFactory::formatToTryFirst = NULL;
bool    avtDatabaseFactory::createMeshQualityExpressions = true;
bool    avtDatabaseFactory::createTimeDerivativeExpressions = true;
bool    avtDatabaseFactory::createVectorMagnitudeExpressions = true;
FileOpenOptions avtDatabaseFactory::defaultFileOpenOptions;

//
// Function Prototypes
//
void CheckPermissions(const char *);


// ****************************************************************************
//  Method: avtDatabaseFactory::SetDefaultFormat
//
//  Purpose:
//      Sets the default format to use if the file type cannot be determined
//      by looking at extensions.
//
//  Arguments:
//      format  The name of the format to use (example: "Silo")
//
//  Programmer: Hank Childs
//  Creation:   May 9, 2004
//
// ****************************************************************************

void
avtDatabaseFactory::SetDefaultFormat(const char *f)
{
    //
    // It's quite possible that the current format string is pointing to 
    // something on the heap (in which case we should free it).  However, it
    // might be pointing to something in the program portion of memory, which
    // would be a bad thing to delete.  So just leak it.  It's only a few
    // bytes.
    defaultFormat = new char[strlen(f)+1];
    strcpy(defaultFormat, f);
}


// ****************************************************************************
//  Method: avtDatabaseFactory::SetFormatToTryFirst
//
//  Purpose:
//      Sets the format to use before trying various ones from a file
//      extension pattern.
//
//  Arguments:
//      format  The name of the format to use (example: "Silo")
//
//  Programmer: Jeremy Meredith
//  Creation:   January 24, 2008
//
// ****************************************************************************

void
avtDatabaseFactory::SetFormatToTryFirst(const char *f)
{
    // Safer to allocate and leak a small string.
    formatToTryFirst = new char[strlen(f)+1];
    strcpy(formatToTryFirst, f);
}


// ****************************************************************************
//  Method:  avtDatabaseFactory::SetDefaultFileOpenOptions
//
//  Purpose:
//    Store off the default file opening options.  We use this when we
//    initialize database plugin readers.
//
//  Arguments:
//    opts       the new default options
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2008
//
// ****************************************************************************

void
avtDatabaseFactory::SetDefaultFileOpenOptions(const FileOpenOptions &opts)
{
    defaultFileOpenOptions = opts;
}

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
//      timestep     The timestep to open to.
//      plugins      The plugins types that were tried (output argument)
//      format       The format to try first.
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
//    Hank Childs, Fri Mar  5 11:25:47 PST 2004
//    Set the file format type with the database meta data.
//
//    Mark C. Miller, Tue Mar 16 09:38:19 PST 2004
//    Just prior to calls to GetMetaData, added calls to ActivateTimestep.
//
//    Hank Childs, Mon Mar 22 09:43:33 PST 2004
//    Added the ability for a format to be specified.
//
//    Brad Whitlock, Fri Apr 30 16:05:42 PST 2004
//    I made extension comparison be case insensitive on Windows.
//
//    Hank Childs, Sun May  9 11:41:45 PDT 2004
//    Use the user-settable default format, rather than a hard-coded "Silo".
//
//    Brad Whitlock, Thu May 27 18:12:45 PST 2004
//    I fixed a bug that I introduced into the Windows extension comparison.
//
//    Jeremy Meredith, Tue Aug 24 17:59:24 PDT 2004
//    Vastly improved the error messages when failing to open a file.
//    The extra error detection prevented crashes in some common situations.
//
//    Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//    Added bool arg to forceReadAllCyclesTimes
//
//    Hank Childs, Thu Jan 11 15:56:53 PST 2007
//    Added argument for which plugin types were tried.
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added support to treat all databases as time varying
//
//    Mark C. Miller, Mon Aug  6 13:36:16 PDT 2007
//    Replaced sprintfs with SNPRINTFs. Adjusted to accomodate possible
//    null return from GetCommonPluginInfo. Moved bulk of code to match
//    on file extensions and filenames to DatabasePluginManager
//
//    Mark C. Miller, Thu Aug  9 09:16:01 PDT 2007
//    Made GetMatchingPluginIds return a vector of possible ids
//
//    Hank Childs, Fri Oct  5 16:28:42 PDT 2007
//    Catch any type of exception that a file format may throw.
//
//    Jeremy Meredith, Wed Jan 23 16:15:41 EST 2008
//    Set the options from the current set of default options before
//    calling SetupDatabase, no matter how we determined which plugin
//    we're using.
//
// ****************************************************************************

avtDatabase *
avtDatabaseFactory::FileList(const char * const * filelist, int filelistN,
                             int timestep, vector<string> &plugins,
                             const char *format,
                             bool forceReadAllCyclesAndTimes,
                             bool treatAllDBsAsTimeVarying)
{
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

    DatabasePluginManager *dbmgr = DatabasePluginManager::Instance();

    //
    // If we were specifically told which format to use, then try that now.
    //
    if (format != NULL)
    {
        int formatindex = dbmgr->GetAllIndex(format);
        if (formatindex < 0)
        {
            char msg[1000];
            SNPRINTF(msg, sizeof(msg),
                    "The DB factory was told to open a file of type %s, "
                    "but the engine had no plugin of that type.",
                    format);
            EXCEPTION1(ImproperUseException, msg);
        }

        string formatid = dbmgr->GetAllID(formatindex);
        if (!dbmgr->PluginAvailable(formatid))
        {
            char msg[1000];
            SNPRINTF(msg, sizeof(msg),
                    "The DB factory was told to open a file of type %s, "
                    "but that format's plugin could not be loaded.",
                    format);
            EXCEPTION1(ImproperUseException, msg);
        }
        CommonDatabasePluginInfo *info = 
            dbmgr->GetCommonPluginInfo(formatid);
        // Set the opening options
        const DBOptionsAttributes *opts = 
            defaultFileOpenOptions.GetOpenOptionsForID(formatid);
        if (opts && info)
            info->SetReadOptions(new DBOptionsAttributes(*opts));
        plugins.push_back(info ? info->GetName(): "");
        rv = SetupDatabase(info, filelist, filelistN, timestep, fileIndex,
                           nBlocks, forceReadAllCyclesAndTimes,
                           treatAllDBsAsTimeVarying);

        if (rv == NULL)
        {
            char msg[1000];
            SNPRINTF(msg, sizeof(msg),
                    "The DB factory was told to open a file of type %s, but "
                    "that format is not a match for file %s",
                    format, filelist[0]);
            EXCEPTION1(ImproperUseException, msg);
        }
    }
 
    //
    // If we have a format we're supposed to try first, do that now.
    //
    if (formatToTryFirst != NULL)
    {
        int formatindex = dbmgr->GetAllIndexFromName(formatToTryFirst);
        if (formatindex >= 0)
        {
            string formatid = dbmgr->GetAllID(formatindex);
            if (dbmgr->PluginAvailable(formatid))
            {
                CommonDatabasePluginInfo *info = 
                    dbmgr->GetCommonPluginInfo(formatid);
                // Set the opening options
                const DBOptionsAttributes *opts = 
                    defaultFileOpenOptions.GetOpenOptionsForID(formatid);
                if (opts && info)
                    info->SetReadOptions(new DBOptionsAttributes(*opts));
                plugins.push_back(info ? info->GetName(): "");
                rv = SetupDatabase(info, filelist, filelistN,
                                   timestep, fileIndex,
                                   nBlocks, forceReadAllCyclesAndTimes,
                                   treatAllDBsAsTimeVarying);
            }
        }
    }
 
    //
    // Check to see if there is an extension that matches.
    //
    vector<string> ids = dbmgr->GetMatchingPluginIds(filelist[fileIndex]);
    for (int i = 0; i < ids.size() && rv == 0; i++)
    {
        CommonDatabasePluginInfo *info = dbmgr->GetCommonPluginInfo(ids[i]);
        // Set the opening options
        const DBOptionsAttributes *opts = 
            defaultFileOpenOptions.GetOpenOptionsForID(ids[i]);
        if (opts && info)
            info->SetReadOptions(new DBOptionsAttributes(*opts));
        TRY
        {
            plugins.push_back(info ? info->GetName() : "");
            rv = SetupDatabase(info, filelist, filelistN, timestep,
                               fileIndex, nBlocks, forceReadAllCyclesAndTimes,
                               treatAllDBsAsTimeVarying);
        }
        CATCHALL(...)
        {
            rv = NULL;
        }
        ENDTRY
    }

    //
    // If no file extension match, then we default to the given database type
    //
    string defaultDatabaseType(defaultFormat);
    if (rv == NULL)
    {
        int defaultindex = dbmgr->GetAllIndexFromName(defaultDatabaseType);
        if (defaultindex != -1)
        {
            string defaultid = dbmgr->GetAllID(defaultindex);
            CommonDatabasePluginInfo *info = 
                                         dbmgr->GetCommonPluginInfo(defaultid);
            // Set the opening options
            const DBOptionsAttributes *opts = 
                defaultFileOpenOptions.GetOpenOptionsForID(defaultid);
            if (opts && info)
                info->SetReadOptions(new DBOptionsAttributes(*opts));
            plugins.push_back(info ? info->GetName() : "");
            rv = SetupDatabase(info, filelist, filelistN, timestep, fileIndex,
                               nBlocks, forceReadAllCyclesAndTimes,
                               treatAllDBsAsTimeVarying);
        }
        else
        {
            char msg[1000];
            SNPRINTF(msg, sizeof(msg),
                    "The default file format plugin '%s' was not available.",
                    defaultDatabaseType.c_str());
            EXCEPTION1(ImproperUseException, msg);
        }
    }

    return rv;
}


// ****************************************************************************
//  Method: avtDatabaseFactory::SetupDatabase
//
//  Purpose:
//      Setups the avtDatabase.  There are several spots where we want to do
//      this, so this provides a "single point of source" that all routines
//      can call.
//
//  Arguments:
//      info         The CommonDatabasePluginInfo that can instantiate a DB.
//      filelist     A list of files.
//      filelistN    The number of files in filelist.
//      timestep     The current timestep.
//
//  Returns:         A database.  This may very well be NULL.
//
//  Notes:           If the format type is not correct, there is a strong
//                   possibility that the exception InvalidDBTypeException
//                   will be thrown.
//
//  Programmer: Hank Childs
//  Creation:   March 22, 2004
//
//  Modifications:
//    Jeremy Meredith/Hank Childs, Tue Mar 23 12:26:55 PST 2004
//    Set the file format with the database, not the meta-data.
//
//    Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//    Added bool arg to forceReadAllCyclesTimes
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added support to treat all databases as time varying
//
//    Mark C. Miller, Mon Aug  6 13:36:16 PDT 2007
//    Added logic to accomodate possible null info.
//
//    Mark C. Miller, Mon Aug 20 18:20:37 PDT 2007
//    Added logic to support a timestep specification of -2 indicating that
//    methods to get information from the format (e.g. ActivateTimestep and
//    GetMetaData) should NOT be called. The rationale for '-2' instead of
//    '-1' is that a timestep of '-1' has typically been used throughout
//    VisIt to mean 'any' timestep. This is primarily to support ZipWrapper.
//
//    Hank Childs, Fri Oct  5 16:35:54 PDT 2007
//    Added debug statements.
//
//    Hank Childs, Mon Oct  8 11:17:56 PDT 2007
//    Correct default argument usage.  A missing argument caused 
//    treatAllDBsAsTimeVarying to be interpreted as forceReadThisStateCycleTime
//
// ****************************************************************************

avtDatabase *
avtDatabaseFactory::SetupDatabase(CommonDatabasePluginInfo *info,
                                  const char * const *filelist, int filelistN, 
                                  int timestep, int fileIndex, int nBlocks,
                                  bool forceReadAllCyclesAndTimes,
                                  bool treatAllDBsAsTimeVarying)
{
    if (info == 0)
    {
        char msg[1024];
        SNPRINTF(msg, 1024, "Attempted to setup a database with null info object");
        EXCEPTION1(ImproperUseException, msg);
    }

    debug4 << "Trying to open the file with the "
           << info->GetName() << " file format." << endl;

    avtDatabase *rv = info->SetupDatabase(filelist+fileIndex,
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
    {
        if (timestep != -2)
            rv->ActivateTimestep(timestep);
        rv->SetFileFormat(info->GetID());
        bool forceReadThisStateCycleTime = false;
        if (timestep != -2)
            rv->GetMetaData(timestep, forceReadAllCyclesAndTimes, 
                            forceReadThisStateCycleTime,
                            treatAllDBsAsTimeVarying);
        debug4 << "File open appears to be successful." << endl;
    }
    else
        debug4 << "File open resulted in NULL database" << endl;

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
//      timestep    The timestep to open at.
//      plugins     The plugins types that were tried (output argument)
//      format      The file format type (optional).
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
//    Hank Childs, Mon Mar 22 11:01:05 PST 2004
//    Added format argument.
//
//    Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//    Added bool arg to forceReadAllCyclesTimes
//
//    Brad Whitlock, Tue Jan 31 14:55:35 PST 2006
//    Fixed a problem that lead to !NBLOCKS failing on win32.
//
//    Hank Childs, Thu Jan 11 15:56:53 PST 2007
//    Added an argument for which plugins were tried.
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added support to treat all databases as time varying
// ****************************************************************************

avtDatabase *
avtDatabaseFactory::VisitFile(const char *visitFile, int timestep,
                              vector<string> &plugins,
                              const char *format, 
                              bool forceReadAllCyclesAndTimes,
                              bool treatAllDBsAsTimeVarying)
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
            if((fileName.find(":") != std::string::npos) ||
               (fileName.size() > 0 && fileName[0] == '!'))
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
    avtDatabase *rv = FileList(reallist, listcount, timestep, plugins, format,
                               forceReadAllCyclesAndTimes, treatAllDBsAsTimeVarying);

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
//    Mark C. Miller, Thu Mar 30 16:45:35 PST 2006
//    Made it use VisItStat instead of stat
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

    VisItStat_t s;
    int rv = VisItStat(filename, &s);
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
