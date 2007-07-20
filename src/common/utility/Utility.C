/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
//                               Utility.C                                   //
// ************************************************************************* //

#include <Utility.h>

#include <visitstream.h>
#include <visit-config.h>
#include <stdio.h>
#include <string.h>
#include <snprintf.h>

#include <string>
using std::string;

#include <vector>
using std::vector;

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#endif

//
// Static vars.
//
static bool isDevelopmentVersion = false;

// ****************************************************************************
//  Function: LongestCommonPrefixLength
//
//  Purpose:
//      Determines what the longest common prefix is to all strings.
//
//  Arguments:
//      list    A list of strings.
//      listN   The number of string in list.
//
//  Returns:   The length of the longest common prefix (possibly 0).
//
//  Programmer: Hank Childs
//  Creation:   June 15, 2000
//
// ****************************************************************************

int
LongestCommonPrefixLength(const char * const *list, int listN)
{
    int   i, j;

    //
    // Determine the minimum length over all strings.
    //
    int  min_length = 100000;
    for (i = 0 ; i < listN ; i++)
    {
        int  length = strlen(list[i]);
        min_length = (min_length < length ? min_length : length);
    }

    //
    // Determine the longest prefix common to all strings.
    //
    int  prefix_length = 0;
    for (i = 0 ; i < min_length ; i++)
    {
        bool  same_prefix = true;
        for (j = 1 ; j < listN ; j++)
        {
            if (list[0][i] != list[j][i])
            {
                same_prefix = false;
                break;
            }
        }
        if (same_prefix)
        {
            prefix_length++;
        }
        else
        {
            break;
        }
    }

    return prefix_length;
}


// ****************************************************************************
//  Function: LongestCommonSuffixLength
//
//  Purpose:
//      Determines what the longest common suffix is to all strings.
//
//  Arguments:
//      list    A list of strings.
//      listN   The number of string in list.
//
//  Returns:   The length of the longest common suffix (possibly 0).
//
//  Programmer: Hank Childs
//  Creation:   August 30, 2000
//
// ****************************************************************************

int
LongestCommonSuffixLength(const char * const *list, int listN)
{
    int   i, j;

    //
    // Take care of degenerative cases.
    //
    if (listN <= 0)
    {
        return -1;
    }

    //
    // Determine the minimum length over all strings.
    //
    int  *lengths    = new int[listN];
    int   min_length = 1000;
    for (i = 0 ; i < listN ; i++)
    {
        lengths[i] = strlen(list[i]);
        if (lengths[i] < min_length)
        {
            min_length = lengths[i];
        }
    }

    //
    // Determine the longest suffix common to all strings.
    //
    int    suffix_length = 0;
    for (i = 0 ; i < min_length ; i++)
    {
        char   c         = list[0][lengths[0]-i-1];
        bool   matches_c = true;
        for (j = 1 ; j < listN ; j++)
        {
            if (c != list[j][lengths[j]-i-1])
            {
                matches_c = false;
                break;
            }
        }
        if (matches_c)
        {
            suffix_length++;
        }
        else
        {
            break;
        }
    }

    delete [] lengths;

    return suffix_length;
}


// ****************************************************************************
//  Function: WaitUntilFile
//
//  Purpose:
//      Does not return until the file exists.  This is useful when debugging
//      and you need to be able to start the debugger with a running process
//      (dbx -p) before it crashes.
//
//  Arguments:
//      filename      The name of the file to wait until.
//
//  Programmer: Hank Childs
//  Creation:   September 25, 2000
//
//  Modifications:
//    Brad Whitlock, Fri Mar 29 08:25:35 PDT 2002
//    Added a windows version of the sleep function.
//
// ****************************************************************************

void
WaitUntilFile(const char *filename)
{
    for (;;)
    {
        ifstream ifile(filename);
        if (! ifile.fail())
        {
             break;
        }
#if defined(_WIN32)
        Sleep(5000);
#else
        sleep(5);
#endif
    }
}


// ****************************************************************************
//  Method: CreateMessageStrings
//
//  Purpose:
//      Makes messages strings of the correct size to go to each processor.
//
//  Arguments:
//      list    A list of strings to put pointers into a bigger string.
//      count   A list of the number of bytes to go to each processor.
//      nl      The number of lists in list.
//
//  Returns:    The char *'s in list will all be offset into a bigger array.
//              Return that bigger array here so it can be properly deleted.
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2001
//
// ****************************************************************************

char *
CreateMessageStrings(char **lists, int *count, int nl)
{
    int   i;

    //
    // Determine how big the big array should be.
    //
    int total = 0;
    for (i = 0 ; i < nl ; i++)
    {
        total += count[i];
    }

    //
    // Make the array of pointers just point into our one big array.
    //
    char *totallist = new char[total];
    char *totaltmp  = totallist;
    for (i = 0 ; i < nl ; i++)
    {
        lists[i] = totaltmp;
        totaltmp += count[i];
    }

    //
    // Return the big array so the top level routine can clean it up later.
    //
    return totallist;
}

// ****************************************************************************
//  Function:  WildcardStringMatch
//
//  Purpose:
//    Match a pattern to a string, using normal '*' and '?' wildcards.
//    Also match '#' to single numerical digits.
//
//  Notes:
//    Yes, recursive -- but it attempts to do a minimal number of recursion
//    levels, uses no local variables and no data copies, so total overhead
//    is minimal.
//
//  Arguments:
//    p          pattern string possibly including wildcards
//    s          string to compare with
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 12, 2002
//
//  Modifications:
//    Jeremy Meredith, Thu Jun 26 10:28:28 PDT 2003
//    Added the '#' wildcard.
//
//    Brad Whitlock, Fri Apr 30 16:13:50 PST 2004
//    I made it case insensitive on Windows.
//
// ****************************************************************************

bool
WildcardStringMatch(const string &p, const string &s)
{
    // wrap around the c-style function
    return WildcardStringMatch(p.c_str(), s.c_str());
}

#if defined(_WIN32)
inline bool
CaseInsensitiveCompare(char p, char s)
{
    bool p_uc = (p >= 'A' && p <= 'Z');
    bool p_lc = (p >= 'a' && p <= 'z');
    bool s_uc = (s >= 'A' && s <= 'Z');
    bool s_lc = (s >= 'a' && s <= 'z');
    bool retval;
    if((p_uc || p_lc) && (s_uc || s_lc))
    {
        char pi = p_uc ? (p - 'A') : (p - 'a');
        char si = s_uc ? (s - 'A') : (s - 'a');
        retval = (pi == si);
    }
    else
        retval = (p == s);

    return retval;
}
#endif

bool
WildcardStringMatch(const char *p, const char *s)
{
    // "*" matches any string
    if (p[0] == '*' && p[1] == 0)
        return true;

    // if either pattern or string is empty:
    //    if both are empty, match
    //    if not, no match
    if (*p == 0 || *s == 0)
    {
        if (*p == 0 && *s == 0)
            return true;
        else
            return false;
    }

    // first chars match ("?" matches any char, "#" matches a digit),
    // and consume one char
    if ( *p == '?' ||
        (*p == '#' && *s >= '0' && *s <= '9') ||
#if defined(_WIN32)
         CaseInsensitiveCompare(*p, *s)
#else
         *p == *s
#endif
       )
    {
        return WildcardStringMatch(&p[1], &s[1]);
    }

    // if first pattern char is "*", consume either the "*" or one string char
    if (*p == '*')
    {
        return
            WildcardStringMatch(&p[1], s) ||
            WildcardStringMatch(p, &s[1]);
    }

    // first characters don't match, so the whole string can't match
    return false;
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
                string fileName(directory);
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
            struct stat s;
            string fileName(directory);
            if(directory.substr(directory.size() - 1, 1) != "/")
                fileName += "/";
            fileName += ent->d_name;
            stat(fileName.c_str(), &s);

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
// Function: NumericStringCompare
//
// Purpose: 
//   Compares two strings but treats any numbers contained in the string as
//   numbers so they compare differently than a staight text comparison.
//
// Arguments:
//   str1 : The first string to compare.
//   str2 : The second string to compare.
//
// Returns:    true if str1<str2; false otherwise.
//
// Programmer: Sean Ahern
// Creation:   Tue Aug 26 11:41:54 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Aug 26 11:42:12 PDT 2003
//   I stole this function from MeshTV and adapted it to C++.
//
//   Brad Whitlock, Mon Sep 15 14:52:12 PST 2003
//   I fixed it so things sort correctly.
//
// ****************************************************************************

bool
NumericStringCompare(const std::string &str1, const std::string &str2)
{
    const char *p1 = str1.c_str();
    const char *p2 = str2.c_str();

    // Compare the two strings, character by character.
    while ((*p1 != '\0') && (*p2 != '\0'))
    {
        // If we're at some digits, we have to treat it differently.
        if (isdigit(*p1) && isdigit(*p2))
        {
            /* We're in the digits. */
            int num1 = (*p1) - '0';
            int num2 = (*p2) - '0';

            p1++;
            p2++;

            // Walk along until we're out of numbers in each string.
            while ((*p1 != '\0') && isdigit(*p1))
            {
                num1 *= 10;
                num1 += (*p1) - '0';
                p1++;
            }
            while ((*p2 != '\0') && isdigit(*p2))
            {
                num2 *= 10;
                num2 += (*p2) - '0';
                p2++;
            }

            // Compare the numbers. If they're the same, keep going. If 
            // they're different, return the difference.
            if (num1 != num2)
                return ((num1 - num2) < 0);
        }
        else
        {
            // We're in some normal characters. Just compare them normally.
            if (*p1 == *p2)
            {
                p1++;
                p2++;
            }
            else
            {
                return ((*p1 - *p2) < 0);
            }
        }
    }

    //
    // If we fall out here, the strings are the same up to the point. But 
    // one of the strings is shorter than the other.
    //
    if(*p1)
        return false;
    else
        return true;
}


// ****************************************************************************
//  Function:  SplitValues
//
//  Purpose:
//    Separate a string into a vector of strings using a single char delimiter.
//
//  Arguments:
//    buff       the string to split
//    delim      the single-character delimiter
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2004
//
// ****************************************************************************
vector<string>
SplitValues(const string &buff, char delim)
{
    vector<string> output;
    
    string tmp="";
    for (int i=0; i<buff.length(); i++)
    {
        if (buff[i] == delim)
        {
            if (!tmp.empty())
                output.push_back(tmp);
            tmp = "";
        }
        else
        {
            tmp += buff[i];
        }
    }
    if (!tmp.empty())
        output.push_back(tmp);

    return output;
}

// ****************************************************************************
// Method: GetDefaultConfigFile
//
// Purpose: 
//   Returns the name and path of the default configuration file.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 18:26:30 PST 2000
//
// Modifications:
//   Brad Whitlock, Wed Feb 16 09:29:44 PDT 2005
//   Moved from ConfigManager class, deleted old modification comments.
//
//   Hank Childs, Wed Aug 24 16:38:55 PDT 2005
//   Allow for files with absolute paths.
//
//   Kathleen Bonnell, Fri Jul 20 10:48:21 PDT 2007 
//   On Windows, 'realhome' is now VISITUSERHOME. 
//
// ****************************************************************************

char *
GetDefaultConfigFile(const char *filename, const char *home)
{
    char *retval;
    char *configFileName;
    int  filenameLength;

    // If the filename has an absolute path, do not prepend the home
    // directory.
    if (filename != NULL && filename[0] == SLASH_CHAR)
    {
        // Must make a copy because the caller will delete this.
        retval = new char[strlen(filename)+1];
        strcpy(retval, filename);
        return retval;
    }

    // Figure out the proper filename to use. If no filename was given, use
    // "config" as the default filename.
    if(filename == 0)
    {
        filenameLength = 7;
        configFileName = "config";
    }
    else
    {
        filenameLength = strlen(filename);
        configFileName = (char *)filename;
    }

#if defined(_WIN32)
    char *realhome = getenv("VISITUSERHOME");

    if(realhome != NULL)
    {
        if(home == NULL)
        {
            // User config. Get the username so we can append it to
            // the filename.
            DWORD namelen = 100;
            char username[100];
            GetUserName(username, &namelen);

            retval = new char[strlen(realhome) + namelen + 5 + filenameLength + 2 + 7];
            sprintf(retval, "%s\\%s for %s.ini", realhome, configFileName, username);
        }
        else
        {
            // System config.
            retval = new char[strlen(realhome) + filenameLength + 2 + 7];
            sprintf(retval, "%s\\%s.ini", realhome, configFileName);
        }
    }
    else
    {
        retval = new char[filenameLength + 1 + 4];
        sprintf(retval, "%s.ini", configFileName);
    }
#else
    // The file it is assumed to be in the home directory unless the home
    // directrory doesn't exist, in which case we will say it is
    // in the current directory.
    char *realhome = getenv((home == 0) ? "HOME" : home);
    if(realhome != NULL)
    {
        retval = new char[strlen(realhome) + filenameLength + 2 + 7];
        sprintf(retval, "%s/.visit/%s", realhome, configFileName);
    }
    else
    {
        retval = new char[filenameLength + 1];
        strcpy(retval, configFileName);
    }
#endif

    return retval;
}

// ****************************************************************************
// Method: GetSystemConfigFile
//
// Purpose: 
//   Returns the system config file name.
//
// Arguments:
//   filename : The base name of the system filename.
//
// Returns:    The system config file name.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 19 12:33:06 PDT 2002
//
// Modifications:
//   Brad Whitlock, Mon Feb 23 15:53:34 PST 2004
//   I added logic to try and determine the name of the appropriate config
//   file.
//
//   Brad Whitlock, Wed Feb 16 09:29:44 PDT 2005
//   Moved from ConfigManager class.
//
// ****************************************************************************

char *
GetSystemConfigFile(const char *filename)
{
    const char *sysConfigName = filename;

    //
    // If no system config file name was given, check the VISITSYSTEMCONFIG
    // environment variable if we're on Windows. Otherwise, just use the
    // name "config".
    //
    if(sysConfigName == 0)
    {
#if defined(_WIN32)
        // Try and get the system config filename from the environment settings.
        sysConfigName = getenv("VISITSYSTEMCONFIG");
#endif

        // If we still don't have the name of a system config file, use 
        // the name "config".
        if(sysConfigName == 0)
            sysConfigName = "config";
    }

    return GetDefaultConfigFile(sysConfigName, "VISITHOME");
}

// ****************************************************************************
// Method: GetUserVisItDirectory
//
// Purpose: 
//   Returns the user's .visit directory or equivalent.
//
// Returns:    The directory where VisIt likes to put stuff.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 3 17:44:59 PST 2003
//
// Modifications:
//   Brad Whitlock, Wed Feb 16 09:29:44 PDT 2005
//   Moved from ConfigManager class.
//
//   Kathleen Bonnell, Fri Jul 20 10:48:21 PDT 2007 
//   User directory on windows is now defined by VISITUSERHOME env var. 
//
// ****************************************************************************

std::string
GetUserVisItDirectory()
{
#if defined(_WIN32)
    const char *home = getenv("VISITUSERHOME");
#else
    const char *home = getenv("HOME");
#endif

    std::string homedir;

    if(home != 0)
    {
#if defined(_WIN32)
        homedir = std::string(home);
#else
        homedir = std::string(home) + "/.visit";
#endif

        if(homedir[homedir.size() - 1] != SLASH_CHAR)
            homedir += SLASH_STRING;
    }

    return homedir;
}

// ****************************************************************************
// Method: GetUserVisItRCFile
//
// Purpose: 
//   Returns the name of the VisIt RC file in the user's .visit directory.
//
// Returns:    The name of the RC file.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 17:44:04 PST 2007
//
// Modifications:
//   
// ****************************************************************************

std::string
GetUserVisItRCFile()
{
    return GetUserVisItDirectory() + "visitrc";
}

#if defined(_WIN32)
//
// Functions to get at VisIt data stored in the Windows registry.
//
int
ReadKeyFromRoot(HKEY which_root, const char *ver, const char *key,
    char **keyval)
{
    int  readSuccess = 0;
    char regkey[100];
    HKEY hkey;

    /* Try and read the key from the system registry. */
    sprintf(regkey, "VISIT%s", ver);
    *keyval = (char *)malloc(500);
    if(RegOpenKeyEx(which_root, regkey, 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
    {
        DWORD keyType, strSize = 500;
        if(RegQueryValueEx(hkey, key, NULL, &keyType,
           (unsigned char *)*keyval, &strSize) == ERROR_SUCCESS)
        {
            readSuccess = 1;
        }

        RegCloseKey(hkey);
    }

    return readSuccess;
}

int
ReadKey(const char *ver, const char *key, char **keyval)
{
    int retval = 0;

    if((retval = ReadKeyFromRoot(HKEY_CLASSES_ROOT, ver, key, keyval)) == 0)
        retval = ReadKeyFromRoot(HKEY_CURRENT_USER, ver, key, keyval);
    
    return retval;     
}

int
WriteKeyToRoot(HKEY which_root, const char *ver, const char *key,
    const char *keyval)
{
    int  writeSuccess = 0;
    char regkey[100];
    HKEY hkey;

    /* Try and read the key from the system registry. */
    sprintf(regkey, "VISIT%s", ver);
    if(RegOpenKeyEx(which_root, regkey, 0, KEY_SET_VALUE, &hkey) == ERROR_SUCCESS)
    {
        DWORD strSize = strlen(keyval);
        if(RegSetValueEx(hkey, key, NULL, REG_SZ,
           (const unsigned char *)keyval, strSize) == ERROR_SUCCESS)
        {
            writeSuccess = 1;
        }

        RegCloseKey(hkey);
    }

    return writeSuccess;
}

int
WriteKey(const char *ver, const char *key, const char *keyval)
{
    int retval = 0;

    if((retval = WriteKeyToRoot(HKEY_CLASSES_ROOT, ver, key, keyval)) == 0)
        retval = WriteKeyToRoot(HKEY_CURRENT_USER, ver, key, keyval);

    return retval;
}
#endif

// ****************************************************************************
// Function: ConfigStateGetRunCount
//
// Purpose: 
//   Returns the number of times the current version of VisIt has been run.
//
// Arguments:
//    code : Returns the success/error code for the operation.
//
// Note:       The number of times the current version of VisIt has been run.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 16 09:55:53 PDT 2005
//
// Modifications:
//   Brad Whitlock, Wed Mar 2 11:59:15 PDT 2005
//   Changed interface to ReadKey.
//
// ****************************************************************************

int
ConfigStateGetRunCount(ConfigStateEnum &code)
{
    int nStartups = 1;
#if defined(_WIN32)
    // Get the number of startups from the registry.
    char *rc = 0;
    if(ReadKey(VERSION, "VISITRC", &rc) == 1)
    {
        if(sscanf(rc, "%d", &nStartups) == 1)
        { 
            if(nStartups < 0)
                nStartups = 1;
        }
        free(rc);
        code = CONFIGSTATE_SUCCESS;
    }
    else
        code = CONFIGSTATE_IOERROR;
#else
    std::string rcFile(GetUserVisItDirectory());
    rcFile += "state";
    rcFile += VERSION;
    rcFile += ".txt";

    FILE *f = 0;
    if((f = fopen(rcFile.c_str(), "r")) != 0)
    {
        if(fscanf(f, "%d", &nStartups) == 1)
        { 
            if(nStartups < 0)
                nStartups = 1;
        }
        fclose(f);
        code = CONFIGSTATE_SUCCESS;
    }
    else
        code = CONFIGSTATE_IOERROR;
#endif

    return nStartups;
}

// ****************************************************************************
// Function: ConfigStateIncrementRunCount
//
// Purpose: 
//   Increments the number of times the current version of VisIt has been run.
//
// Arguments:
//    code : Returns the success/error code for the operation.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 16 09:56:54 PDT 2005
//
// Modifications:
//   Brad Whitlock, Wed Mar 2 11:58:53 PDT 2005
//   Changed interface to WriteKey.
//
//   Brad Whitlock, Mon Mar 7 11:31:35 PDT 2005
//   Changed the Win32 logic.
//
//   Brad Whitlock, Mon Jul 11 09:58:13 PDT 2005
//   I fixed a win32 coding problem.
//
// ****************************************************************************

void
ConfigStateIncrementRunCount(ConfigStateEnum &code)
{
#if defined(_WIN32)
    bool firstTime = false;
    ConfigStateEnum code2;
    int nStartups = ConfigStateGetRunCount(code2);
    if(code2 == CONFIGSTATE_IOERROR)
    {
        firstTime = true;
        nStartups = 0;
    }
    else if(code2 == CONFIGSTATE_SUCCESS)
    {
        firstTime = (nStartups == 0);
    }

    char keyval[100];
    SNPRINTF(keyval, 100, "%d", nStartups+1);
    if(WriteKey(VERSION, "VISITRC", keyval) == 1)
        code = firstTime ? CONFIGSTATE_FIRSTTIME : CONFIGSTATE_SUCCESS;
    else
        code = CONFIGSTATE_IOERROR;
#else
    std::string rcFile(GetUserVisItDirectory());
    rcFile += "state";
    rcFile += VERSION;
    rcFile += ".txt";

    // Does the file exist?
    bool firstTime = false;
    struct stat s;
    if(stat(rcFile.c_str(), &s) == -1)
        firstTime = true;

    ConfigStateEnum code2;
    int nStartups = firstTime ? 0 : ConfigStateGetRunCount(code2);
    if(code2 == CONFIGSTATE_IOERROR)
        nStartups = 0;
    FILE *f = 0;
    if((f = fopen(rcFile.c_str(), "w")) != 0)
    {
        fprintf(f, "%d\n", nStartups + 1);
        fclose(f);
        code = firstTime ? CONFIGSTATE_FIRSTTIME : CONFIGSTATE_SUCCESS;
    }
    else
        code = CONFIGSTATE_IOERROR;
#endif
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

        // Append the rest of the path to the home directory.
        std::string restOfPath(path.substr(i, path.length() - i + 1));
        std::string homeDir("C:\\Documents and Settings\\");
        newPath = homeDir + std::string(username) + restOfPath;
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

// ****************************************************************************
// Method: SetIsDevelopmentVersion
//
// Purpose: 
//   Sets whether the version of VisIt is a development version.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 14 10:51:30 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
SetIsDevelopmentVersion(bool val)
{
    isDevelopmentVersion = val;
}

// ****************************************************************************
// Method: GetIsDevelopmentVersion
//
// Purpose: 
//   Returns whether VisIt is a development version of VisIt.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 14 10:50:35 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

bool
GetIsDevelopmentVersion()
{
    return isDevelopmentVersion;
}

// ****************************************************************************
// Method: GetVisItInstallationDirectory
//
// Purpose: 
//   Returns the directory where VisIt was installed.
//
// Arguments:
//   version : The version for which we want information. Only used on Windows.
//
// Returns:    The directory where VisIt is installed.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 2 12:08:57 PDT 2005
//
// Modifications:
//   Brad Whitlock, Mon Mar 7 14:23:59 PST 2005
//   I fixed a bug that made it return the wrong part of the string.
//
//   Brad Whitlock, Mon May 9 16:26:43 PST 2005
//   I prevented it from stripping the last directory off if we're in a
//   development directory.
//
//   Brad Whitlock, Thu Jul 14 10:49:42 PDT 2005
//   I made it use the isDevelopmentDir library variable.
//
//   Brad Whitlock, Thu Dec 21 18:55:46 PST 2006
//   Added missing code for win32 development.
//
//   Kathleen Bonnell, Fri Jul 20 10:48:21 PDT 2007 
//   Removed MSVC6 specific code. 
//
// ****************************************************************************

std::string
GetVisItInstallationDirectory()
{
    return GetVisItInstallationDirectory(VERSION);
}

std::string
GetVisItInstallationDirectory(const char *version)
{
#if defined(_WIN32)
    // Get the installation dir for the specified from the registry.
    char *visitHome = 0;
    std::string installDir("C:\\");
    if(ReadKey(version, "VISITHOME", &visitHome) == 1)
    {
        installDir = visitHome;
        delete [] visitHome;
    }
    else
    {
        // Use the VISITDEVDIR environment var.
        std::string visitdev;
        char *devdir = getenv("VISITDEVDIR");
        if(devdir == 0)
            visitdev = std::string("C:\\VisItDev") + std::string(version);
        else
            visitdev = std::string(devdir);
        installDir = visitdev + "\\bin\\MSVC7.Net\\Release";
    }
    return installDir;
#else
    // Get the installation dir for the version that's running. They all use
    // the same "visit" script so it's okay to do this.
    std::string installDir("/usr/local/visit");
    const char *idir = getenv("VISITHOME");
    if(idir != 0)
    {
        // The directory often has a "/bin" on the end. Strip it off.
        std::string home(idir);
        if(isDevelopmentVersion)
            installDir = idir;
        else
        {
            int lastSlash = home.rfind("/");
            if(lastSlash != -1)
                installDir = home.substr(0, lastSlash);
            else
                installDir = idir;
        }
    }
    return installDir;
#endif
}

// ****************************************************************************
// Method: GetVisItArchitectureDirectory
//
// Purpose: 
//   Gets the name of the directory where VisIt's current binary is installed.
//   This directory typically contains the bin, lib, plugin, etc directories
//   on UNIX installations.
//
// Arguments:
//   version : The version number for which we want the archtecture dir.
//
// Returns:    The architecture dir.
//
// Note:       On Windows, this function returns the same as
//             GetVisItInstallationDirectory.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 24 11:33:07 PDT 2005
//
// Modifications:
//   Brad Whitlock, Mon Jul 11 10:08:30 PDT 2005
//   Made it work for the non-installed case on Windows.
//
//   Kathleen Bonnell, Fri Jul 20 10:48:21 PDT 2007 
//   Removed MSVC6 specific code. 
//
// ****************************************************************************

std::string
GetVisItArchitectureDirectory()
{
    return GetVisItArchitectureDirectory(VERSION);
}

std::string
GetVisItArchitectureDirectory(const char *version)
{
#if defined(_WIN32)
    // Get the installation dir for the specified from the registry.
    char *visitHome = 0;
    std::string archDir("C:\\");
    if(ReadKey(version, "VISITHOME", &visitHome) == 1)
    {
        archDir = visitHome;
        delete [] visitHome;
    }
    else
    {
        // Use the VISITDEVDIR environment var.
        std::string visitdev;
        char *devdir = getenv("VISITDEVDIR");
        if(devdir == 0)
            visitdev = std::string("C:\\VisItDev") + std::string(version);
        else
            visitdev = std::string(devdir);
        archDir = visitdev + "\\bin\\MSVC7.Net\\Release";
    }
    return archDir;
#else
    // Get the installation dir for the version that's running. They all use
    // the same "visit" script so it's okay to do this.
    std::string archDir(std::string("/usr/local/visit/") + std::string(VERSION));
    const char *adir = getenv("VISITARCHHOME");
    if(adir != 0)
        archDir = adir;
    return archDir;
#endif
}

// ****************************************************************************
// Method: GetVisItLauncher
//
// Purpose: 
//   Returns the name of the VisIt launch program, wherever it is found.
//
// Returns:    The full name of the VisIt launcher.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 14 10:54:33 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

std::string
GetVisItLauncher()
{
#if defined(_WIN32)
    return std::string(GetVisItInstallationDirectory() + "\\visit.exe");
#else
    return std::string(GetVisItInstallationDirectory() + "/bin/visit");
#endif
}

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
//  Method:  ConvertArgsToTunneledValues
//
//  Purpose:
//    When we're doing SSH tunneling, we  need to replace client:oldport
//    with remote:tunnelend port (called "localhost":tunneledport because
//    localhost is determined relative to the remote machine).
//
//  Arguments:
//    portTunnelMap        the map of local-to-remote port numbers
//    args                 the argv stringVector to modify in-place
//
//  Returns:  true on success, false on failure.
//
//  Programmer:  Jeremy Meredith
//  Creation:    May 24, 2007
//
//  Modifications:
//    Jeremy Meredith, Thu Jun 28 13:32:26 EDT 2007
//    If we're tunneling through SSH, this should override any selection of
//    how the client host name should otherwise be determined.  We had partly
//    solved this already by overriding the value of the -host argument,
//    and this adds support for the case where they specify -guesshost
//    instead of -host <hostname>.  I also added the -sshtunneling argument
//    just incase downstream calls need to know this.
//
// ****************************************************************************
bool
ConvertArgsToTunneledValues(const std::map<int,int> &portTunnelMap,
                            std::vector<std::string> &args)
{
    bool foundHost = false;
    // strip off -guesshost
    for (int i=0; i<args.size(); i++)
    {
        if (args[i] == "-guesshost")
        {
            for (int j=i+1; j<args.size(); j++)
            {
                args[j-1] = args[j];
            }
            args.resize(args.size()-1);
            break;
        }
    }
    // replace host and port
    for (int i=0; i<args.size(); i++)
    {
        if (i<args.size()-1 && args[i] == "-host")
        {
            args[i+1] = "localhost";
            foundHost = true;
        }
        if (i<args.size()-1 && args[i] == "-port")
        {
            int oldport = atoi(args[i+1].c_str());
            if (portTunnelMap.count(oldport) <= 0)
                return false;
            int newport = portTunnelMap.find(oldport)->second;
            char newportstr[10];
            sprintf(newportstr,"%d",newport);
            args[i+1] = newportstr;
        }
    }
    // add "localhost" if it there wasn't already a host specified
    if (!foundHost)
    {
        args.push_back("-host");
        args.push_back("localhost");
    }
    // add -sshtunneling for anyone that wants to know
    args.push_back("-sshtunneling");
    return true;
}
