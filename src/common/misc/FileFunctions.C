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
#include <FileFunctions.h>

#include <visitstream.h>
#include <visit-config.h>
#include <stdio.h>
#include <string.h>
#include <snprintf.h>

#if defined(_WIN32)
#include <windows.h>
#include <userenv.h> // for GetProfilesDirectory
#else
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#endif

// ****************************************************************************
// Method: VisItStat 
//
// Purpose: platform independent stat function that supports large files
// when possible
//
// Programmer: Mark C. Miller 
// Creation:   March 23, 2006 
//
// ****************************************************************************

int
VisItStat(const char *file_name, VisItStat_t *buf)
{
#if defined(_WIN32)
   return _stat(file_name, buf);
#else

#if SIZEOF_OFF64_T > 4
    return stat64(file_name, buf);
#else
    return stat(file_name, buf);
#endif

#endif
}

// ****************************************************************************
// Method: VisItFStat 
//
// Purpose: platform independent fstat function that supports large files
// when possible
//
// Programmer: Mark C. Miller 
// Creation:   March 23, 2006 
//
// ****************************************************************************

int
VisItFstat(int fd, VisItStat_t *buf)
{
#if defined(_WIN32)
   return _fstat(fd, buf);
#else

#if SIZEOF_OFF64_T > 4
    return fstat64(fd, buf);
#else
    return fstat(fd, buf);
#endif

#endif
}

// ****************************************************************************
// Function: ReadAndProcessDirectory
//
// Purpose: 
//   Reads the list of files in the specified directory and calls a callback
//   function on each file.
//
// Arguments:
//   directory     : The directory to read.
//   procesOneFile : Callback function to process one file.
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
// ****************************************************************************

bool
ReadAndProcessDirectory(const std::string &directory,
    ProcessDirectoryCallback *processOneFile, void *data,
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
    else
    {
        // Try and read the files in fullPath.
        std::string searchPath(directory + std::string("\\*"));
        WIN32_FIND_DATA fd;
        HANDLE dirHandle = FindFirstFile(searchPath.c_str(), &fd);
        if(dirHandle != INVALID_HANDLE_VALUE)
        {
            do
            {
                bool isDir = ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) ||
                             (strcmp(fd.cFileName, "..") == 0);
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
// Function: ExpandUserPath
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
ExpandUserPath(const std::string &path)
{
    std::string newPath(path);

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

    return newPath;
}
