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

    // first chars match ("?" matches any char), consume one char
    if (*p == '?' ||
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


