/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
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
// Static data members
//
char    *avtDatabaseFactory::defaultFormat = "Silo";


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
// ****************************************************************************

avtDatabase *
avtDatabaseFactory::FileList(const char * const * filelist, int filelistN,
                             int timestep, const char *format,
                             bool forceReadAllCyclesAndTimes)
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
            sprintf(msg,
                    "The DB factory was told to open a file of type %s, "
                    "but the engine had no plugin of that type.",
                    format);
            EXCEPTION1(ImproperUseException, msg);
        }

        string formatid = dbmgr->GetAllID(formatindex);
        if (!dbmgr->PluginAvailable(formatid))
        {
            char msg[1000];
            sprintf(msg,
                    "The DB factory was told to open a file of type %s, "
                    "but that format's plugin could not be loaded.",
                    format);
            EXCEPTION1(ImproperUseException, msg);
        }
        CommonDatabasePluginInfo *info = 
            dbmgr->GetCommonPluginInfo(formatid);
        rv = SetupDatabase(info, filelist, filelistN, timestep, fileIndex,
                           nBlocks, forceReadAllCyclesAndTimes);

        if (rv == NULL)
        {
            char msg[1000];
            sprintf(msg,
                    "The DB factory was told to open a file of type %s, but "
                    "that format is not a match for file %s",
                    format, filelist[0]);
            EXCEPTION1(ImproperUseException, msg);
        }
    }
 
    //
    // Try each database type looking for a match to the given extensions
    //
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
        for (j=0; j<nextensions && !foundMatch; j++)
        {
            string ext = extensions[j];
            if (ext[0] != '.')
            {
                ext = string(".") + extensions[j];
            }
#if defined(_WIN32)
            if (file.length() >= ext.length())
            {
                string fileExt(file.substr(file.length() - ext.length()));
                foundMatch = (_stricmp(fileExt.c_str(), ext.c_str()) == 0);
            }
#else
            if (file.length() >= ext.length() &&
                file.substr(file.length() - ext.length()) == ext)
            {
                foundMatch = true;
            }
#endif
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
                rv = SetupDatabase(info, filelist, filelistN, timestep,
                                   fileIndex, nBlocks, forceReadAllCyclesAndTimes);
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
    string defaultDatabaseType(defaultFormat);
    if (rv == NULL)
    {
        int defaultindex = dbmgr->GetAllIndexFromName(defaultDatabaseType);
        if (defaultindex != -1)
        {
            string defaultid = dbmgr->GetAllID(defaultindex);
            CommonDatabasePluginInfo *info = 
                                         dbmgr->GetCommonPluginInfo(defaultid);
            rv = SetupDatabase(info, filelist, filelistN, timestep, fileIndex,
                               nBlocks, forceReadAllCyclesAndTimes);
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
// ****************************************************************************

avtDatabase *
avtDatabaseFactory::SetupDatabase(CommonDatabasePluginInfo *info,
                                  const char * const *filelist, int filelistN, 
                                  int timestep, int fileIndex, int nBlocks,
                                  bool forceReadAllCyclesAndTimes)
{
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
        rv->ActivateTimestep(timestep);
        rv->SetFileFormat(info->GetID());
        rv->GetMetaData(timestep, forceReadAllCyclesAndTimes);
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
//      timestep    The timestep to open at.
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
// ****************************************************************************

avtDatabase *
avtDatabaseFactory::VisitFile(const char *visitFile, int timestep,
                              const char *format, bool forceReadAllCyclesAndTimes)
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
    avtDatabase *rv = FileList(reallist, listcount, timestep, format,
                               forceReadAllCyclesAndTimes);

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
