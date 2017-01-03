/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
#include <FileFunctions.h>
#include <Utility.h>

#include <visitstream.h>
#include <vectortypes.h>

#include <cerrno>

#include <stdio.h>
#include <string.h>
#include <snprintf.h>

#if defined(_WIN32)
#include <windows.h>
#include <userenv.h> // for GetProfilesDirectory
#include <direct.h>
#include <Shlwapi.h> // PathIsRelative
#else
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#endif


const int STATIC_BUF_SIZE = 4096;
static char StaticStringBuf[STATIC_BUF_SIZE];

// ****************************************************************************
// Method: FileFunctions::VisItStat
//
// Purpose: platform independent stat function that supports large files
// when possible
//
// Programmer: Mark C. Miller
// Creation:   March 23, 2006
//
// Modifications:
//   Kathleen Biagas, Wed Nov 24 16:25:13 MST 2015
//   Use _stat64 if running 64 bit windows version.
//
// ****************************************************************************

int
FileFunctions::VisItStat(const std::string &filename, VisItStat_t *buf)
{
#if defined(_WIN32)
  #if defined(_WIN64)
    return _stat64(filename.c_str(), buf);
  #else
   return _stat(filename.c_str(), buf);
  #endif
#else

#if SIZEOF_OFF64_T > 4
    return stat64(filename.c_str(), buf);
#else
    return stat(filename.c_str(), buf);
#endif

#endif
}

// ****************************************************************************
// Method: FileFunctions::VisItFStat
//
// Purpose: platform independent fstat function that supports large files
// when possible
//
// Programmer: Mark C. Miller 
// Creation:   March 23, 2006 
//
// Modifications:
//   Kathleen Biagas, Wed Nov 24 16:25:13 MST 2015
//   Use _fstat64 if running 64 bit windows version.
//
// ****************************************************************************

int
FileFunctions::VisItFstat(int fd, VisItStat_t *buf)
{
#if defined(_WIN32)
  #if defined(_WIN64)
    return _fstat64(fd, buf);
  #else
    return _fstat(fd, buf);
  #endif
#else

#if SIZEOF_OFF64_T > 4
    return fstat64(fd, buf);
#else
    return fstat(fd, buf);
#endif

#endif
}

// ****************************************************************************
// Function: FileFunctions::ReadAndProcessDirectory
//
// Purpose: 
//   Reads the list of files in the specified directory and calls a callback
//   function on each file.
//
// Arguments:
//   directory     : The directory to read.
//   processOneFile : Callback function to process one file.
//   data          : Callback data.
//   checkAccess   : Whether or not to check the file permissions.
//
// Returns:    True if successful; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 7 15:08:02 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Jul 11 14:18:21 PST 2003
//   Made it work on Windows.
//
//   Hank Childs, Thu Jun  8 16:13:20 PDT 2006
//   Fix warning regarding uninitialized variable.
//
//   Kathleen Bonnell, Thu Apr 22 17:23:43 MST 2010 
//   Add '.' to test for isDir on Windows.
//
// ****************************************************************************

bool
FileFunctions::ReadAndProcessDirectory(const std::string &directory,
    FileFunctions::ProcessDirectoryCallback *processOneFile, void *data,
    bool checkAccess)
{
    bool retval = false;

#if defined(_WIN32)
    if(directory == "My Computer")
    {
        // Add the drives to the list.
        char buf[200];
        DWORD bufLen = 200;
        DWORD slen = GetLogicalDriveStrings(200, buf);

        if(slen > 0)
        {
            char *ptr = buf;
            while(*ptr != 0)
            {
                std::string drive(ptr);
                (*processOneFile)(data, drive, true, true, 0);
                ptr += (drive.size() + 1);
                retval = true;
            }
        }
    }
    else if (directory.size() > 0)
    {
        // Try and read the files in fullPath.
        std::string searchPath(directory + std::string("\\*"));
        WIN32_FIND_DATA fd;
        HANDLE dirHandle = FindFirstFile(searchPath.c_str(), &fd);
        if(dirHandle != INVALID_HANDLE_VALUE)
        {
            do
            {
                bool isDir =
                    ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) ||
                     (strcmp(fd.cFileName, "..") == 0) ||
                     (strcmp(fd.cFileName, ".") == 0) ;
                long sz = ((fd.nFileSizeHigh * MAXDWORD) + fd.nFileSizeLow);
                std::string fileName(directory);
                if(directory.substr(directory.size() - 1) != "\\")
                    fileName += "\\";
                fileName += fd.cFileName;
                (*processOneFile)(data, fileName, isDir, true, sz);
                retval = true;

            } while(FindNextFile(dirHandle, &fd));
            FindClose(dirHandle);
        }
    } else {
        //Directory string was empty, nothing to do
        retval = false;
    }
#else
    DIR     *dir;
    dirent  *ent;

    // If the directory cannot be opened, return an error code.
    dir = opendir(directory.c_str());
    if (dir)
    {
        // Get the userId and the groups for that user so we can check the
        // file permissions.
        gid_t gids[100];
        int ngids = 0;
        uid_t uid = 0;
        if(checkAccess)
        {
            uid = getuid();
            ngids = getgroups(100, gids);
        }

        // Process each directory entry.
        while ((ent = readdir(dir)) != NULL)
        {
            // Get information about the file.
            VisItStat_t s;
            std::string fileName(directory);
            if(directory.substr(directory.size() - 1, 1) != "/")
                fileName += "/";
            fileName += ent->d_name;
            VisItStat(fileName.c_str(), &s);

            mode_t mode = s.st_mode;
            bool isdir = S_ISDIR(mode);
   
            bool canaccess = checkAccess ? false : true;
            if(checkAccess)
            {
                bool isuser  = (s.st_uid == uid);
                bool isgroup = false;
                for (int i=0; i<ngids && !isgroup; i++)
                    if (s.st_gid == gids[i])
                        isgroup=true;
    
                if (isdir)
                {
                    if ((mode & S_IROTH) &&
                        (mode & S_IXOTH))
                        canaccess=true;
                    else if (isuser &&
                             (mode & S_IRUSR) &&
                             (mode & S_IXUSR))
                        canaccess=true;
                    else if (isgroup &&
                             (mode & S_IRGRP) &&
                             (mode & S_IXGRP))
                        canaccess=true;
                }
                else
                {
                    if (mode & S_IROTH)
                        canaccess=true;
                    else if (isuser &&
                             (mode & S_IRUSR))
                        canaccess=true;
                    else if (isgroup &&
                             (mode & S_IRGRP))
                        canaccess=true;
                }
            }

            (*processOneFile)(data, fileName, isdir, canaccess, (long)s.st_size);
            retval = true;
        }
 
        closedir(dir);
    }
#endif

    return retval;
}

// ****************************************************************************
// Method: MDServerConnection::ReadCWD
//
// Purpose:
//   Gets the current directory.
//
// Programmer: Jeremy Meredith
// Creation:   Wed Aug 23 12:09:39 PDT 2000
//
// Modifications:
//
// ****************************************************************************

std::string
FileFunctions::GetCurrentWorkingDirectory()
{
    char tmpcwd[1024];
#if defined(_WIN32)
    _getcwd(tmpcwd,1023);
#else
    if(getcwd(tmpcwd,1023) == NULL)
        tmpcwd[0] = '\0';
#endif
    tmpcwd[1023]='\0';
    return std::string(tmpcwd);
}

// ****************************************************************************
// Function: FileFunctions::ExpandPath
//
// Purpose: 
//   Expands a path that contains ~ as its first character so it contains the
//   absolute path to the specified user's home directory.
//
// Arguments:
//   path : The path to expand.
//
// Returns:    If the first character is "~" then the path gets expanded,
//             otherwise the path is returned unmodified.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 17 14:57:57 PST 2005
//
// Modifications:
//   Kathleen Bonnell, Thu Nov 6 11:04:15 PST 2008
//   Change how homeDir is determined on Windows, to allow for roaming profiles
//   (where it may not be on C:) and for Vista, where the default profiles
//   directory is C:\Users not C:\Documents and Settings.
//   
//   Kathleen Bonnell, Fri Nov 7 15:46:33 PST 2008
//   Forgot path separator between homeDir and username. 
//
// ****************************************************************************

std::string
FileFunctions::ExpandPath(const std::string &path, 
    const std::string &cwd)
{
    std::string newPath(path);
    std::string workingDir(cwd);

    if(workingDir.empty())
        workingDir = GetCurrentWorkingDirectory();

    if(path.empty())
        return workingDir;

    if(path[0] == '~')
    {
        char username[256];
        int  i;

        // Find the user name portion of the path, ie ~user
        for (i = 1; isalnum(path[i]); i++)
        {
            username[i - 1] = path[i];
        }
        username[i - 1] = '\0';

#if defined(_WIN32)
        if(i == 1)
        {
            // User just specified '~', get the current user name.
            DWORD s = 256;
            GetUserName(username, &s);
        }
        // Get 'home' directory
        char *profDir = new char[MAX_PATH];
        DWORD size = 256;
        GetProfilesDirectory(profDir, &size);
        std::string homeDir(profDir);
        delete [] profDir;    

        // Append the rest of the path to the home directory.
        std::string restOfPath(path.substr(i, path.length() - i + 1));
        newPath = homeDir + "\\" + std::string(username) + restOfPath;
#else
        // Check if the user specified '~' or '~name'.
        struct passwd *users_passwd_entry = NULL;
        if (i == 1)
        {
            // User just specified '~', get /etc/passwd entry
            users_passwd_entry = getpwuid(getuid());
        }
        else
        {
            // User specified '~name', get /etc/passwd entry
            users_passwd_entry = getpwnam(username);
        }

        // Now that we have a passwd entry, validate it.
        if (users_passwd_entry == NULL)
        {
            // Did not specify a valid user name.  Do nothing. 
            return newPath;
        }
        if (users_passwd_entry->pw_dir == NULL)
        {
            // Passwd entry is invalid.  Do nothing.
            return newPath;
        }

        // Append the rest of the path to the home directory.
        std::string restOfPath(path.substr(i, path.length() - i + 1));
        newPath = std::string(users_passwd_entry->pw_dir) + restOfPath;
#endif
    }
#if defined(_WIN32)
    else if(path.substr(0, 12) == "My Computer\\" && path[13] == ':')
    {
        // Filter out the "My Computer" part of the path.
        newPath = path.substr(12);
    }
    else if(path.size() > 1 && path[1] == ':')
    {
        // absolute path. do nothing
        newPath = path;
    }
    else if(path == "My Computer")
    {
        // special path. do nothing
        newPath = path;
    }
    else
    {
        // relative path:
        newPath = workingDir + "\\" + path;
    }
#else
    else if(path[0] != '/')
    {
        // relative path:
        newPath = workingDir + "/" + path;
    }
    else
    {
        // absolute path: do nothing
        newPath = path;
    }
#endif

    return FilteredPath(newPath);
}

// ****************************************************************************
// Method: FileFunctions::FilteredPath
//
// Purpose: 
//   Filters extra junk out of a path.
//
// Arguments:
//   path : The path string that we're filtering.
//
// Returns:    A filtered path string.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 13 14:08:01 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Mar 18 11:40:40 PDT 2004
//   I fixed the code so it filters out .. properly.
//
//   Brad Whitlock, Wed Apr 14 17:22:53 PST 2004
//   I fixed the .. filtering for Windows.
//
//   Brad Whitlock, Mon Sep 28 16:36:18 PDT 2009
//   Don't pop_back unless there's something to remove.
//
// ****************************************************************************

std::string
FileFunctions::FilteredPath(const std::string &path)
{
    // Remove multiple slashes in a row.
    size_t i = 0;
    size_t state = 0;
    std::string filteredPath;
    for(i = 0; i < path.length(); ++i)
    {
        if(state == 0)
        {
            filteredPath += path[i];
            if(path[i] == VISIT_SLASH_CHAR)
                state = 1;
        }
        else if(path[i] != VISIT_SLASH_CHAR)
        {
            filteredPath += path[i];
            state = 0;
        }
    }

    std::string path2(filteredPath);
    if(path2.length() > 0 && path2[path2.length() - 1] == VISIT_SLASH_CHAR)
    {
        filteredPath = path2.substr(0, path2.length() - 1);
    }

    if(filteredPath.size() == 0)
        filteredPath = VISIT_SLASH_STRING;
    else
    {
        // Filter out .. so we get the right path.
        stringVector tmpNames;
        std::string  tmp;
        state = 0;
        const char *str = filteredPath.c_str();
        for(i = 0; i < filteredPath.length() + 1; ++i)
        {
            if(str[i] == VISIT_SLASH_CHAR || str[i] == '\0')
            {
                if(tmp.size() > 0)
                {
                    if(tmp == "..")
                    {
                        if(tmpNames.size() > 0)
                            tmpNames.pop_back();
                    }
                    else
                        tmpNames.push_back(tmp);
                }
                tmp = "";
            }
            else
                tmp += str[i];
        }

        // Reassemble the path fragments.
        if(tmpNames.size() > 0)
        {
            filteredPath = "";
            for(i = 0; i < tmpNames.size(); ++i)
            { 
#if defined(_WIN32)
                if(i > 0)
                    filteredPath += VISIT_SLASH_STRING;
#else
                filteredPath += VISIT_SLASH_STRING;
#endif
                filteredPath += tmpNames[i];
            }
        }
    }

    return filteredPath;
}

// ****************************************************************************
//  Function: FileFunctions::Basename
//
//  Purpose: Find the basename of a file path string
//
//  Programmer: Mark C. Miller
//  Creation:   Unknown
//
//  Modifications:
//    Jeremy Meredith, Wed May 20 13:46:39 EDT 2009
//    Should default to "0" for start, and only use "-1" for
//    the "all-slash-string" case.
//
//    Kathleen Biagas, Thu Jul 28 09:41:27 PDT 2011
//    When searching the string, look for either type of slash char, but still
//    use the sys-dependent VISIT_SLASH_STRING when setting in the empty buf.
//
//    Mark C. Miller, Tue Sep 15 20:18:22 PDT 2015
//    Added arg and logic to support an optional suffix string just like
//    Unix basename command.
// ****************************************************************************
static char const *
basename(char const *path, int& start, char const *suffix=0)
{
   start = 0;

   if (path == 0)
   {
       strcpy(StaticStringBuf, ".");
       return StaticStringBuf;
   }
   else if (*path == '\0')
   {
       strcpy(StaticStringBuf, ".");
       return StaticStringBuf;
   }
   else
   {
       // find end of path string
       int n = 0;
       while ((path[n] != '\0') && (n < STATIC_BUF_SIZE))
           n++;

       // deal with string too large
       if (n == STATIC_BUF_SIZE)
       {
           strcpy(StaticStringBuf, ".");
           return StaticStringBuf;
       }

       // backup, skipping over all trailing slash chars
       int j = n-1;
       while ((j >= 0) && (path[j] == '/' || path[j] == '\\'))
           j--;

       // deal with string consisting of all slash chars
       if (j == -1)
       {
           start = -1;
           strcpy(StaticStringBuf, VISIT_SLASH_STRING);
           return StaticStringBuf;
       }

       // backup to just after next slash char
       int i = j-1;
       while ((i >= 0) && (path[i] != '/' && path[i] != '\\'))
           i--;
       i++;
       start = i;

       // build the candidate return string
       int k;
       for (k = 0; k < j - i + 1; k++)
           StaticStringBuf[k] = path[i+k];
       StaticStringBuf[k] = '\0';

       // Handle optional suffix but only if its not equal to
       // remaining string (as per man pages for dirname)
       if (suffix)
       {
           int n = strlen(suffix);
           if (n < k && !strncmp(&StaticStringBuf[k-n],suffix,n))
               StaticStringBuf[k-n] = '\0';
       }

       return StaticStringBuf;
   }
}

char const *
FileFunctions::Basename(char const *path, char const *suffix)
{
   int dummy1;
   return basename(path, dummy1, suffix);
}

std::string
FileFunctions::Basename(const std::string &path, const std::string &suffix)
{
    return Basename(path.c_str(), suffix.c_str());
}

// ****************************************************************************
//  Function: Dirname
//
//  Purpose: Find the dirname of a file path string
//
//  Programmer: Mark C. Miller
//  Creation:   Unknown
//
//  Modifications:
//    Jeremy Meredith, Wed May 20 13:46:39 EDT 2009
//    Special cases were unnecessary; they fall out of the start position
//    returned from the above implementation of basename naturally.  Fixed
//    a couple of the special cases as well.
//
//    Kathleen Biagas, Thu Jul 28 09:41:27 PDT 2011
//    When searching the string, look for either type of slash char, but still
//    use the sys-dependent VISIT_SLASH_STRING when setting in the empty buf.
//
//    Mark C. Miller, Wed Jul 11 20:03:16 PDT 2012
//    Fixed the special case where the only part of the string left after
//    eliminating the basename part is a single slash char at index zero.
// ****************************************************************************
const char *
FileFunctions::Dirname(const char *path)
{
    int start;

    // ok, figure out the basename
    basename(path, start);

    if (start == -1)
    {
        strcpy(StaticStringBuf, VISIT_SLASH_STRING);
        return StaticStringBuf;
    }
    else if (start == 0)
    {
        strcpy(StaticStringBuf, ".");
        return StaticStringBuf;
    }
    else
    {
        int i;
        for (i = 0; i < start; i++)
            StaticStringBuf[i] = path[i];
        if (i > 1 && (StaticStringBuf[i-1] == '/' ||
                      StaticStringBuf[i-1] == '\\'))
            StaticStringBuf[i-1] = '\0';
        else
            StaticStringBuf[i] = '\0';
        return StaticStringBuf;
    }
}

std::string
FileFunctions::Dirname(const std::string &path)
{
    return Dirname(path.c_str());
}

// ****************************************************************************
//  Function: Normalize
//
//  Purpose: Normalize a pathname; removing all embedded './' and/or '../' or
//  '//', and any trailing '/'. Note, however, that he code is written to use
//  whatever the VISIT_SLASH_STRING is so it should work on Windows as well
//  with the exception of a leading drive letter and colon.
//
//  Programmer: Mark C. Miller, Mon Jul 16 21:56:03 PDT 2012
//
//  Modifications:
//    Kathleen Biagas, Thu June 6 09:39:25 PDT 2013
//    Added pathSep argument that defaults to platform-specific 
//    VISIT_SLASH_STRING.  Use of non-platform specific case my be needed if
//    parsing internal database path-names.
//
// ****************************************************************************

const char *
FileFunctions::Normalize(const char *path, const char *pathSep)
{
    std::string retval(path);

    // First, remove any double slashes
    std::string dbl_slash = std::string(pathSep) + std::string(pathSep);
    size_t dbl_slash_idx = retval.rfind(dbl_slash);
    while (dbl_slash_idx != std::string::npos)
    {
        retval.erase(dbl_slash_idx, 1);
        dbl_slash_idx = retval.rfind(dbl_slash);
    }

    // Remove any terms of the form "./". These have no effect
    std::string dot_slash = std::string(".") + std::string(pathSep);
    size_t dot_slash_idx = retval.rfind(dot_slash);
    while (dot_slash_idx != std::string::npos)
    {
        if ((dot_slash_idx > 0 && retval[dot_slash_idx-1] != '.') ||
             dot_slash_idx == 0)
        {
            retval.erase(dot_slash_idx, 2);
            dot_slash_idx = retval.rfind(dot_slash,dot_slash_idx-1);
        }
        else
        {
            if (dot_slash_idx > 0)
                dot_slash_idx = retval.rfind(dot_slash,dot_slash_idx-1);
            else
                dot_slash_idx = std::string::npos;
        }
    }

    // Remove any trailing slash if one exists
    if (retval[retval.size()-1] == pathSep[0])
        retval.erase(retval.size()-1);

    // At this point we have a string that begins with a slash
    // and has only <path> terms or "../" terms. We need to
    // resolve any "../" terms by backing up through the <path>
    // terms that precede them.
    std::string slash_dot_dot = std::string(pathSep) + std::string("..");
    size_t slash_dot_dot_idx = retval.find(slash_dot_dot);
    bool noCharsRemainingToBackup = false;
    while (slash_dot_dot_idx != std::string::npos)
    {
        size_t preceding_slash_idx = retval.rfind(pathSep, slash_dot_dot_idx-1);
        if (preceding_slash_idx == std::string::npos)
        {
            size_t nchars = slash_dot_dot_idx + 3;
            retval.erase(0, nchars);
            slash_dot_dot_idx = retval.find(slash_dot_dot);
            if (slash_dot_dot_idx == 0)
            {
                retval = "";
                noCharsRemainingToBackup = true;
                break;
            }
        }
        else
        {
            size_t nchars = slash_dot_dot_idx - preceding_slash_idx + 3;
            retval.erase(preceding_slash_idx+1, nchars);
            slash_dot_dot_idx = retval.find(slash_dot_dot);
        }
    }

    // Remove any trailing slash if one exists
    if (retval.size() && retval[retval.size()-1] == pathSep[0])
        retval.erase(retval.size()-1);

    if (retval == "" && !noCharsRemainingToBackup) retval = ".";

    StaticStringBuf[0] = '\0';
    strcat(StaticStringBuf, retval.c_str());
    return StaticStringBuf;
}

std::string
FileFunctions::Normalize(const std::string &path, const std::string &pathSep)
{
    return Normalize(path.c_str(), pathSep.c_str());
}

// ****************************************************************************
//  Function: Absname
//
//  Purpose: Compute absolute path name based on cwd and a path relative to
//  the cwd.
//
//  Programmer: Mark C. Miller, Mon Jul 16 21:56:03 PDT 2012
//
//  Modifications:
//    Kathleen Biagas, Thu June 6 09:39:25 PDT 2013
//    Added pathSep argument that defaults to platform-specific 
//    VISIT_SLASH_STRING.  Use of non-platform specific case my be needed if
//    parsing internal database path-names.
//
//    Kathleen Biagas, Thu Jan 29 15:53:12 MST 2015
//    Some tweaks on windows to hanlde cwd_context of '.'.
//
// ****************************************************************************

const char *
FileFunctions::Absname(const char *cwd_context, const char *path, 
    const char *pathSep)
{
    // Clear our temporary array for handling char * return values.
    StaticStringBuf[0] = '\0';

    // cwd_context is null or empty string
    if (!cwd_context || cwd_context[0] == '\0')
    {
        if (!path) return StaticStringBuf;
        if (path[0] != pathSep[0]) return StaticStringBuf;

        std::string npath(Normalize(path, pathSep));
        strcpy(StaticStringBuf, npath.c_str());
        return StaticStringBuf;
    }

    // path is null or empty string
    if (!path || path[0] == '\0')
    {
        if (!cwd_context) return StaticStringBuf;
        if (cwd_context[0] != pathSep[0]) return StaticStringBuf;

        std::string ncwd(Normalize(cwd_context, pathSep));
        strcpy(StaticStringBuf, ncwd.c_str());
        return StaticStringBuf;
    }

    if (path[0] == pathSep[0])
    {
        std::string npath(Normalize(path, pathSep));
        strcpy(StaticStringBuf, npath.c_str());
        return StaticStringBuf;
    }

#ifndef _WIN32
    if (cwd_context[0] != pathSep[0])
    {
        return StaticStringBuf;
    }
#else
    if(cwd_context[0] == '.')
    {
        if (PathIsRelative(path))
        {
            if(_fullpath(StaticStringBuf, ".\\", _MAX_PATH) != NULL)
                return StaticStringBuf;
            else
                return path;
        }
        return path;
    }
#endif

    // Catenate path to cwd_context and then Normalize the result
    std::string path2 = std::string(cwd_context) + std::string(pathSep) + std::string(path);
    std::string npath = Normalize(path2.c_str(), pathSep);
    strcpy(StaticStringBuf, npath.c_str());
    return StaticStringBuf;
}

std::string
FileFunctions::Absname(const std::string &cwd_context, 
                       const std::string &path, 
                       const std::string &pathSep)
{
    return Absname(cwd_context.c_str(), path.c_str(), pathSep.c_str());
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
//    Tom Fogal, Sun May  3 15:33:55 MDT 2009
//    Marked the functions static.
//
//    Kathleen Biagas, Thu Feb 6 13:24:01 PST 2014
//    Test for file's existence on Windows. Throwing the exception here can
//    prevent engine crashing elsewhere. (conn_cmfe test with bad_file.silo).
//
// ****************************************************************************

#if defined(_WIN32)
FileFunctions::PermissionsResult
FileFunctions::CheckPermissions(const std::string &filename)
{
    FileFunctions::PermissionsResult retval = FileFunctions::PERMISSION_RESULT_READABLE;
    FileFunctions::VisItStat_t s;
    int result = FileFunctions::VisItStat(filename, &s);
    if (result < 0)
    {
        retval = FileFunctions::PERMISSION_RESULT_NOFILE;
    }
    return retval;
}

#else
static bool    setUpUserInfo = false;
static uid_t   uid;
static gid_t   gids[100];
static int     ngids;

static void
SetUpUserInfo(void)
{
    setUpUserInfo = true;
    uid = getuid();
    ngids = getgroups(100, gids);
}

FileFunctions::PermissionsResult
FileFunctions::CheckPermissions(const std::string &filename)
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
            return PERMISSION_RESULT_NOFILE;
        }
        else
        {
            return PERMISSION_RESULT_NONREADABLE;
        }
    }
    mode_t mode = s.st_mode;

    //
    // If other has permission, then we are set.
    //
    if (mode & S_IROTH)
    {
        return PERMISSION_RESULT_READABLE;
    }

    //
    // If we are the user and the user has permission, then we are set.
    //
    bool isuser =  (s.st_uid == uid);
    if (isuser && (mode & S_IRUSR))
    {
        return PERMISSION_RESULT_READABLE;
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
        return PERMISSION_RESULT_READABLE;
    }

    return PERMISSION_RESULT_NONREADABLE;
}
#endif

// ****************************************************************************
// Method: FileFunctions::SplitHostDatabase
//
// Purpose: 
//   Splits a hostDB into host and database names.
//
// Arguments:
//   hostDB : The entire host and db name.
//   host   : The host that was split out of the hostDB.
//   db     : The db that was split out of the hostDB.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 25 15:40:48 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
FileFunctions::SplitHostDatabase(const std::string &hostDB, 
    std::string &host, std::string &db)
{
    std::string hdb(hostDB);
    //
    // If we found what looks like drive punctuation (on Windows), replace
    // the : in the drive with something else so we can still check
    // for a host without getting it wrong.
    //
    bool foundDrive = false;
    std::string::size_type driveColon = hdb.find(":\\");
    if(driveColon != std::string::npos)
    {
        hdb.replace(driveColon, 1, "]");
        foundDrive = true;
    }

    // Look for the host colon.
    std::string::size_type hostColon = hdb.find(':');

    //
    // Now that the host colon was looked for, replace the drive if
    // we found one previously.
    //
    if(foundDrive)
        hdb.replace(driveColon, 1, ":");

    //
    // If the database string doesn't have a ':' in it then assume that
    // the host name is "localhost" and the database name is the entire
    // string.
    //
    if (hostColon == std::string::npos)
    {
        host = "localhost";
        db = hdb;
    }
    else
    {
        //
        // If the database string does have a ':' in it then the part before
        // it is the host name and the part after it is the database name.
        //
        host = hdb.substr(0, hostColon);
        db = hdb.substr(hostColon + 1);
    }
}

// ****************************************************************************
// Method: FileFunctions::ComposeDatabaseName
//
// Purpose: 
//   Composes a database name from host and file names.
//
// Arguments:
//   host : The name of the host where the database is stored.
//   db   : The name of the database.
//
// Returns:    The host + database name.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 25 15:05:40 PST 2004
//
// Modifications:
//   
// ****************************************************************************

std::string
FileFunctions::ComposeDatabaseName(const std::string &host,
    const std::string &db)
{
    std::string h(host);

    if(h == "")
        h = "localhost";

    return h + ":" + db;
}

// ****************************************************************************
//  Method: FileMatchesPatternCB
//
//  Purpose:
//    This function is a callback to the method ReadAndProcessDirectory,
//    located in Utility.h.  It is called for each file in a given directory.
//    Once it receives a file, it feeds that file to caller which then
//    determines if the filename matches the requested pattern.
//
//  Programmer: Kathleen Biagas
//  Creation:   Jun 26, 2013
//
//  Modifications:
//    Kathleen Biagas, Fri Jun 26 12:13:39 PDT 2015
//    Moved from NetworkManager, and added the 'returnFullpath' callback data
//    item.
//
// ****************************************************************************


void
FileFunctions::FileMatchesPatternCB(void *cbdata, const std::string &filename, bool isDir, bool canAccess, long size)
{
    if (!isDir)
    {
        void **arr = (void **)cbdata;
        std::vector< std::string > *fl = (std::vector< std::string > *)arr[0];
        std::string *pattern = (std::string*)arr[1];
        int *returnFullPath = (int*)arr[2];
        std::string name(filename);
        size_t index  = filename.rfind(VISIT_SLASH_CHAR);
        if(index != std::string::npos)
            name = name.substr(index+1);
        if (WildcardStringMatch(*pattern, name))
        {
           if (*returnFullPath)
               fl->push_back(filename);
           else 
               fl->push_back(name);
        }
    }
}
