// ************************************************************************* //
//                               Utility.C                                   //
// ************************************************************************* //

#include <Utility.h>

#include <fstream.h>
#include <stdio.h>
#include <string.h>

#include <string>
using std::string;

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#endif

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
// ****************************************************************************
bool
WildcardStringMatch(const string &p, const string &s)
{
    // wrap around the c-style function
    return WildcardStringMatch(p.c_str(), s.c_str());
}

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
         *p == *s)
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
        uid_t uid;
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

            (*processOneFile)(data, fileName, isdir, (long)s.st_size, canaccess);
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
