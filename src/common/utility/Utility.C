/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                               Utility.C                                   //
// ************************************************************************* //

#include <Utility.h>

#include <visitstream.h>
#include <visit-config.h>
#include <stdio.h>
#include <string.h>
#include <snprintf.h>

using std::vector;

#if defined(_WIN32)
#include <windows.h>
#else
#if defined(__APPLE__)
#include <malloc/malloc.h> // for mstat
#endif
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#endif

// ****************************************************************************
//  Function: LongestCommonPrefixLength
//
//  Purpose:
//      Determines what the longest common prefix is to all strings.
//
//  Arguments:
//      list    A list of strings.
//      listN   The number of std::string in list.
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
//      listN   The number of strings in list.
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
//  Function: GetMemorySize
//
//  Purpose:
//      Gets the amount of memory in use and the RSS (resident set size).
//
//  Programmer: Hank Childs (from code from Peter Lindstrom)
//  Creation:   February 28, 2008
//
//  Modifications:
//    Cyrus Harrison, Wed Apr  1 12:16:19 PDT 2009
//    Modified to use unsigned ints so we can go above 2 gigabytes (up to
//    4 gigabytes).
//
//    Brad Whitlock, Tue Jun 23 17:07:57 PDT 2009
//    I added a Mac implementation.
//
// ****************************************************************************

void
GetMemorySize(unsigned int &size, unsigned int &rss)
{
    size = 0;
    rss  = 0;
#if defined(__APPLE__)
    struct mstats m = mstats();
    size = (unsigned int)m.bytes_used; // The bytes used out of the bytes_total.
    rss = (unsigned int)m.bytes_total; // not quite accurate but this should be the total
                                       // amount allocated by malloc.
#elif !defined(_WIN32)
    FILE *file = fopen("/proc/self/statm", "r");
    if (file == NULL)
    {
        return;
    }

    int count = fscanf(file, "%u%u", &size, &rss);
    if (count != 2)
    {
        return;
    }
    size *= getpagesize();
    rss  *= getpagesize();
    fclose(file);
#endif
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
//    Jeremy Meredith, Mon Dec 28 15:40:12 EST 2009
//    I made the case-insensitivity apply to all platforms.
//
// ****************************************************************************

bool
WildcardStringMatch(const std::string &p, const std::string &s)
{
    // wrap around the c-style function
    return WildcardStringMatch(p.c_str(), s.c_str());
}

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
         CaseInsensitiveCompare(*p, *s))
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
//   Brad Whitlock, Fri Nov 21 16:11:43 PST 2008
//   Take the number of digits into account when the numbers are the same.
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
            int ndigs1 = 1;
            while ((*p1 != '\0') && isdigit(*p1))
            {
                num1 *= 10;
                num1 += (*p1) - '0';
                p1++;
                ndigs1++;
            }
            int ndigs2 = 1;
            while ((*p2 != '\0') && isdigit(*p2))
            {
                num2 *= 10;
                num2 += (*p2) - '0';
                p2++;
                ndigs2++;
            }

            // Compare the numbers. If they're the same, keep going. If 
            // they're different, return the difference. If the numbers were
            // the same but used a different number of characters then
            // return the lesser number of digits.
            if (num1 != num2)
                return ((num1 - num2) < 0);
            else if(ndigs1 != ndigs2)
                return ndigs1 < ndigs2;
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
vector<std::string>
SplitValues(const std::string &buff, char delim)
{
    vector<std::string> output;
    
    std::string tmp="";
    for (size_t i=0; i<buff.length(); i++)
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
//    Thomas R. Treadway, Mon Oct  8 13:27:42 PDT 2007
//    Backing out SSH tunneling on Panther (MacOS X 10.3)
//
// ****************************************************************************
#if defined(PANTHERHACK)
// Broken on Panther
#else
bool
ConvertArgsToTunneledValues(const std::map<int,int> &portTunnelMap,
                            std::vector<std::string> &args)
{
    bool foundHost = false;
    // strip off -guesshost
    for (size_t i=0; i<args.size(); i++)
    {
        if (args[i] == "-guesshost")
        {
            for (size_t j=i+1; j<args.size(); j++)
            {
                args[j-1] = args[j];
            }
            args.resize(args.size()-1);
            break;
        }
    }
    // replace host and port
    for (size_t i=0; i<args.size(); i++)
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
#endif
