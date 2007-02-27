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

#include <StringHelpers.h>

#include <sys/types.h>
#if defined(_WIN32)
#include <win32-regex.h>
#else
#include <regex.h>
#endif
#include <stdlib.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

static string IGNORE_CHARS = StringHelpers::NON_RELEVANT_CHARS;

const int STATIC_BUF_SIZE = 4096;
static char StaticStringBuf[STATIC_BUF_SIZE];

string RelevantString(string inStr)
{
   string outStr;
   string::size_type n;

   n = inStr.find_first_not_of(IGNORE_CHARS);
   while (n != string::npos)
   {
       outStr += inStr[n]; 
       n = inStr.find_first_not_of(IGNORE_CHARS, n+1);
   }

   return outStr;
}

static int CompareRelevantStrings(const void *arg1, const void *arg2)
{
    string str1 = RelevantString(*((char **) arg1));
    string str2 = RelevantString(*((char **) arg2));
    return strcmp(str1.c_str(),str2.c_str());
}

//
// Groups a list of strings by finding identical leading substrings
// of length numLeadingVals.
//
void
StringHelpers::GroupStrings(vector<string> stringList,
                            vector<vector<string> > &stringGroups,
                            vector<string> &groupNames,
                            int numLeadingVals,
                            string nonRelevantChars)
{

   int i;
   int nStrings = stringList.size();

   if (nStrings == 0)
       return;

   // prime the input to the compare functions
   IGNORE_CHARS = nonRelevantChars;

   // first, we need to sort the strings. Well, we don't really sort the
   // strings. Instead we sort an array of pointers to the strings. We
   // build that array here.
   const char **stringPtrs = new const char*[nStrings];
   for (i = 0; i < nStrings; i++)
       stringPtrs[i] = stringList[i].c_str();

   // now, call qsort for this array of string pointers
   qsort(stringPtrs, nStrings, sizeof(char *), CompareRelevantStrings);

   // adjust numLeadingVals if its too big 
   int len = strlen(stringPtrs[0]);
   if (numLeadingVals < 0)
   {
       if (-numLeadingVals > len)
           numLeadingVals  = -len;
   }
   else
   {
       if (numLeadingVals > len)
           numLeadingVals  = len;
   }


   // initialize the 'lastVal' for the loop below
   string lastVal;
   if (numLeadingVals < 0)
   {
       for (i = len+numLeadingVals; i < len; i++)
           lastVal += stringPtrs[0][i];
   }
   else
   {
       if (numLeadingVals == 0)
           lastVal = stringPtrs[0];
       else
       {
           for (i = 0; i < numLeadingVals; i++)
               lastVal += stringPtrs[0][i];
       }
   }

   //
   // now, scan the sorted list of strings for value transitions
   // in first N (default 3) chars. Each such transition indicates the end of
   // one group and the beginning of the next.
   //
   groupNames.push_back(RelevantString(stringPtrs[0]));
   vector<string> curGroup;
   curGroup.push_back(stringPtrs[0]);
   for (i = 1; i < nStrings; i++)
   {
       string thisVal;
       int j;

       if (numLeadingVals < 0)
       {
           int len = stringList[i].size()-1;
           for (j = len+numLeadingVals; j < len; j++)
               lastVal += stringPtrs[i][j];
       }
       else
       {
           if (numLeadingVals == 0)
               thisVal = stringPtrs[i];
           else
           {
               for (j = 0; j < numLeadingVals; j++)
                   thisVal += stringPtrs[i][j];
           }
       }

       if (thisVal != lastVal)
       {
           lastVal = thisVal;
           groupNames.push_back(RelevantString(stringPtrs[i]));
           stringGroups.push_back(curGroup);
           curGroup.clear();
           curGroup.push_back(stringPtrs[i]);
       }
       else
       {
           curGroup.push_back(stringPtrs[i]);
       }
   }
   stringGroups.push_back(curGroup);

   delete [] stringPtrs;
}

//
// Groups a list of strings that look like file paths into groups
// that have same dirname
//
void
StringHelpers::GroupStringsAsPaths(vector<string> stringList,
                            vector<vector<string> > &stringGroups,
                            vector<string> &groupNames)
{

   int i;
   int nStrings = stringList.size();

   if (nStrings == 0)
       return;

   // prime the input to the compare functions
   IGNORE_CHARS = "`~!@#$%^&*()|\\\"'?";

   // first, we need to sort the strings. Well, we don't really sort the
   // strings. Instead we sort an array of pointers to the strings. We
   // build that array here.
   const char **stringPtrs = new const char*[nStrings];
   for (i = 0; i < nStrings; i++)
       stringPtrs[i] = stringList[i].c_str();

   // now, call qsort for this array of string pointers
   qsort(stringPtrs, nStrings, sizeof(char *), CompareRelevantStrings);

   // now, scan the sorted list of strings for value transitions
   // in the Dirname of each member
   string lastVal = Dirname(stringPtrs[0]);
   groupNames.push_back(lastVal);
   vector<string> curGroup;
   curGroup.push_back(stringPtrs[0]);
   for (i = 1; i < nStrings; i++)
   {
       string thisVal = Dirname(stringPtrs[i]);

       if (thisVal != lastVal)
       {
           lastVal = thisVal;
           groupNames.push_back(RelevantString(thisVal));
           stringGroups.push_back(curGroup);
           curGroup.clear();
           curGroup.push_back(stringPtrs[i]);
       }
       else
       {
           curGroup.push_back(stringPtrs[i]);
       }
   }
   stringGroups.push_back(curGroup);

   delete [] stringPtrs;
}


//
// Groups a list of strings into a fixed number of groups
// by alphabetizing and then dividing the alphabetized list into pieces
//
void
StringHelpers::GroupStringsFixedAlpha(vector<string> stringList,
                            int numGroups,
                            vector<vector<string> > &stringGroups)
{

   int i;
   int nStrings = stringList.size();

   if (nStrings == 0)
       return;

   // prime the input to the compare functions
   IGNORE_CHARS = "";

   // first, we need to sort the strings. Well, we don't really sort the
   // strings. Instead we sort an array of pointers to the strings. We
   // build that array here.
   const char **stringPtrs = new const char*[nStrings];
   for (i = 0; i < nStrings; i++)
       stringPtrs[i] = stringList[i].c_str();

   // now, call qsort for this array of string pointers
   qsort(stringPtrs, nStrings, sizeof(char *), CompareRelevantStrings);

   int groupSize = nStrings / numGroups;
   if (nStrings % numGroups)
       groupSize++;
   for (i = 0; i < nStrings; i++)
   {
       int groupNum = i / groupSize;
       int groupIdx = i % groupSize;

       if (groupIdx == 0)
       {
           vector<string> newGroup;
           stringGroups.push_back(newGroup);
       }
       stringGroups[groupNum].push_back(stringPtrs[i]);
    }
}

//
// This version does not need to sort the strings since they are already
// sorted because they're in a set. The sort rule for the set is the same
// as that for the other GroupStringsFixedAlpha because IGNORE_CHARS gets
// set to "", which means use the entire string in comparisons.
//
void
StringHelpers::GroupStringsFixedAlpha(
    const std::set<std::string> &stringList,
    int groupSize, std::vector<std::set<std::string> > &stringGroups)
{
    int nStrings = stringList.size();

    if (nStrings == 0)
        return;

    int i = 0;
    stringGroups.reserve(stringList.size() / groupSize);
    for(std::set<std::string>::const_iterator it = stringList.begin();
        it != stringList.end(); ++it, ++i)
    {
        int groupNum = i / groupSize;
        int groupIdx = i % groupSize;

        if (groupIdx == 0)
        {
            std::set<std::string> newGroup;
            stringGroups.push_back(newGroup);
        }
        stringGroups[groupNum].insert(*it);
    }
}

int
StringHelpers::FindRE(const char *strToSearch, const char *re)
{
    regex_t cre;
    regmatch_t pm;

    if (regcomp(&cre, re, REG_EXTENDED))
        return FindError;

    int rval = regexec(&cre, strToSearch, 1, &pm, 0);

    regfree(&cre);

    if (rval == REG_NOMATCH)
        return FindNone;

    if (pm.rm_so >= strlen(strToSearch))
        return FindError;

    if (pm.rm_so < 0)
        return FindError;

    return (int) pm.rm_so;
}

static const char *
basename(const char *path, int& start)
{
   start = -1;

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

       // backup, skipping over all trailing '/' chars
       int j = n-1;
       while ((j >= 0) && (path[j] == '/'))
           j--;

       // deal with string consisting of all '/' chars
       if (j == -1)
       {
           strcpy(StaticStringBuf, "/");
           return StaticStringBuf;
       }

       // backup to just after next '/' char
       int i = j-1;
       while ((i >= 0) && (path[i] != '/'))
           i--;
       i++;
       start = i;

       // build the return string
       int k;
       for (k = 0; k < j - i + 1; k++)
           StaticStringBuf[k] = path[i+k];
       StaticStringBuf[k] = '\0';
       return StaticStringBuf;
   }
}

const char *
StringHelpers::Basename(const char *path)
{
   int dummy1;
   return basename(path, dummy1);
}

const char *
StringHelpers::Dirname(const char *path)
{
    int start;

   // deal with special cases first
   if ((path == 0) ||                             // null path
       (path[0] == '\0') ||                       // ""
       ((path[0] == '.') && (path[1] == '\0')) || // "."
       ((path[0] == '.') && (path[1] == '.') &&   // ".."
        (path[2] == '\0')))
   {
       strcpy(StaticStringBuf, ".");
       return StaticStringBuf;
   }
   else if ((path[0] == '/') && (path[1] == '\0'))
   {
       strcpy(StaticStringBuf, "/");
       return StaticStringBuf;
   }

    // ok, figure out the basename
    basename(path, start);

    if (start == -1)
    {
        strcpy(StaticStringBuf, "/");
        return StaticStringBuf;
    }
    else
    {
        int i;
        for (i = 0; i < start; i++)
            StaticStringBuf[i] = path[i];
        if (StaticStringBuf[i-1] == '/')
            StaticStringBuf[i-1] = '\0';
       else
            StaticStringBuf[i] = '\0';
        return StaticStringBuf;
    }
}
