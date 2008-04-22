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

#include <StringHelpers.h>

#include <errno.h>
#include <sys/types.h>
#include <stdarg.h>
#if defined(_WIN32)
#include <win32-regex.h>
#else
#include <regex.h>
#endif
#include <stdlib.h>
#include <map>
#include <string>
#include <vector>

#include <visit-config.h>

using std::map;
using std::string;
using std::vector;

static string IGNORE_CHARS = StringHelpers::NON_RELEVANT_CHARS;

const int STATIC_BUF_SIZE = 4096;
static char StaticStringBuf[STATIC_BUF_SIZE];

// ****************************************************************************
//  Function: RelevantString 
//
//  Purpose: Return a string containing only the relevant characters of the
//  input string. Relevant characters are those NOT in IGNORE_CHARS
//
//  Programmer: Mark C. Miller 
//  Creation:   Unknown
//
// ****************************************************************************
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

// ****************************************************************************
//  Function: CompareRelevantStrings
//
//  Purpose: Compare two strings using only their relevant characters 
//
//  Programmer: Mark C. Miller 
//  Creation:   Unknown
//
// ****************************************************************************

static int CompareRelevantStrings(const void *arg1, const void *arg2)
{
    string str1 = RelevantString(*((char **) arg1));
    string str2 = RelevantString(*((char **) arg2));
    return strcmp(str1.c_str(),str2.c_str());
}

// ****************************************************************************
//  Function: GroupStrings 
//
//  Purpose: Groups a list of strings by finding identical leading substrings
//  of length numLeadingVals.
//
//  Programmer: Mark C. Miller 
//  Creation:   Unknown
//
// ****************************************************************************
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

// ****************************************************************************
//  Function: GroupStringsAsPaths
//
//  Purpose: Groups a list of strings that look like file paths into groups
//  that have same dirname
//
//  Programmer: Mark C. Miller 
//  Creation:   Unknown
//
// ****************************************************************************
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


// ****************************************************************************
//  Function: GroupStringsAsFixedAlpha
//
//  Purpose: Groups a list of strings into a fixed number of groups
//  by alphabetizing and then dividing the alphabetized list into pieces
//
//  Programmer: Brad Whitlock 
//  Creation:   Unknown
//
// ****************************************************************************
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

// ****************************************************************************
//  Function: GroupStringsAsFixedAlpha
//
//  Purpose: Groups a list of strings into a fixed number of groups
//  by alphabetizing and then dividing the alphabetized list into pieces
//
//  This version does not need to sort the strings since they are already
//  sorted because they're in a set. The sort rule for the set is the same
//  as that for the other GroupStringsFixedAlpha because IGNORE_CHARS gets
//  set to "", which means use the entire string in comparisons.
//
//  Programmer: Brad Whitlock 
//  Creation:   Unknown
//
// ****************************************************************************
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

// ****************************************************************************
//  Function: FindRE 
//
//  Purpose: Find match of a regular expression in a given string. Return the
//  starting offset into the string where the match occured.
//
//  Programmer: Mark C. Miller 
//  Creation:   Unknown
//
// ****************************************************************************
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


// ****************************************************************************
//  Function: ExtractRESubstr 
//
//  Purpose: Extract the (sub)string matched by the regular expression. 
//
//  The format of RE string passed here is an opening '<' followed by the
//  actual regular expression string followed by a closing '>', optionally
//  followed by a ' ' (space) and a backslashed substring reference. For
//  example, to extract the cycle digits from strings looking like
//  'run_23_0010_yana.silo' where '0010' is the cycle digits, the string to
//   pass here would look like...
//
//                                               V--substring reference
//                      "<.*_([0-9]{4})_.*\\..*> \1"
//          opening char-^                     ^--closing char
//                        ^------RE part------^
//
//  Do a 'man 7 regex' to get more information on regular expression syntax
//
//  Programmer: Mark C. Miller 
//  Creation:   June 12, 2007 
//
// ****************************************************************************
std::string
StringHelpers::ExtractRESubstr(const char *strToSearch, const char *re)
{
    regex_t cre;
    regmatch_t pm[255];
    string reToUse;
    string retval = "";

    //
    // extract actual RE from VisIt's required format ('<RE> \i')
    //
    int len = strlen(re);
    int matchToExtract;
    if (re[0] != '<')
        return retval;
    const char *last = strrchr(re, '>'); 
    if (last == 0)
        return retval;
    if (*(last+1) == '\0')
    {
        reToUse = string(re, 1, len-2);
        matchToExtract = 0;
    }
    else if (*(last+1) == ' ' && *(last+2) == '\\')
    {
        reToUse = string(re, 1, (last-re+1)-2); // -2 for '<' and '>' chars
        errno = 0;
        matchToExtract = strtol(last+3, 0, 10);
	if (errno != 0)
	    return retval;
    }
    else
    {
        return retval;
    }

    if (regcomp(&cre, reToUse.c_str(), REG_EXTENDED))
        return retval;

    int rval = regexec(&cre, strToSearch, 255, pm, 0);

    regfree(&cre);

    if (rval == REG_NOMATCH)
        return retval;

    for (int i = 0; i < 255; i++)
    {
        if (pm[i].rm_so == -1)
	    continue;
	if (i == matchToExtract)
	{
	    retval = std::string(strToSearch, pm[i].rm_so,
	                                      pm[i].rm_eo - pm[i].rm_so);
	    break;
        }
    }
    return retval;
}

// ****************************************************************************
//  Function: Basename 
//
//  Purpose: Find the basename of a file path string
//
//  Programmer: Mark C. Miller 
//  Creation:   Unknown
//
// ****************************************************************************
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

       // backup, skipping over all trailing SLASH_CHAR chars
       int j = n-1;
       while ((j >= 0) && (path[j] == SLASH_CHAR))
           j--;

       // deal with string consisting of all SLASH_CHAR chars
       if (j == -1)
       {
           strcpy(StaticStringBuf, SLASH_STRING);
           return StaticStringBuf;
       }

       // backup to just after next SLASH_CHAR char
       int i = j-1;
       while ((i >= 0) && (path[i] != SLASH_CHAR))
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

// ****************************************************************************
//  Function: Dirname 
//
//  Purpose: Find the dirname of a file path string
//
//  Programmer: Mark C. Miller 
//  Creation:   Unknown
//
// ****************************************************************************
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
   else if ((path[0] == SLASH_CHAR) && (path[1] == '\0'))
   {
       strcpy(StaticStringBuf, SLASH_STRING);
       return StaticStringBuf;
   }

    // ok, figure out the basename
    basename(path, start);

    if (start == -1)
    {
        strcpy(StaticStringBuf, SLASH_STRING);
        return StaticStringBuf;
    }
    else
    {
        int i;
        for (i = 0; i < start; i++)
            StaticStringBuf[i] = path[i];
        if (StaticStringBuf[i-1] == SLASH_CHAR)
            StaticStringBuf[i-1] = '\0';
       else
            StaticStringBuf[i] = '\0';
        return StaticStringBuf;
    }
}

// ****************************************************************************
//  Function: InitTypeNameToFmtREMap
//
//  Purpose: Support routine to build map of regular expressions for different
//  data type names.
//
//  Do a 'man 7 regex' for information on format of the regular expression.
//
//  Programmer: Mark C. Miller 
//  Creation:   September 20, 2007 
//
//  Modifications:
//    Mark C. Miller, Fri Sep 21 20:26:18 PDT 2007
//    Fixed bug(s) in RE's allowing bad width/precision flags
//
// ****************************************************************************
static map<string,string> typeNameToFmtREMap;
static void InitTypeNameToFmtREMap()
{
    if (typeNameToFmtREMap.size())
        return;

    typeNameToFmtREMap["float"]                   = "[^%]*%#?0?-? ?+?'?(([1-9][0-9]*)?(\\.[0-9]*)?)?[eEfFgGaA]{1}";
    typeNameToFmtREMap["double"]                  = "[^%]*%#?0?-? ?+?'?(([1-9][0-9]*)?(\\.[0-9]*)?)?[eEfFgGaA]{1}";
    typeNameToFmtREMap["long double"]             = "[^%]*%#?0?-? ?+?'?(([1-9][0-9]*)?(\\.[0-9]*)?)?L[eEfFgGaA]{1}";
    typeNameToFmtREMap["int"]                     = "[^%]*%#?0?-? ?+?'?I?(([1-9][0-9]*)?(\\.[0-9]*)?)?[di]{1}";
    typeNameToFmtREMap["long int"]                = "[^%]*%#?0?-? ?+?'?I?(([1-9][0-9]*)?(\\.[0-9]*)?)?l[di]{1}";
    typeNameToFmtREMap["long long int"]           = "[^%]*%#?0?-? ?+?'?I?(([1-9][0-9]*)?(\\.[0-9]*)?)?ll[di]{1}";
    typeNameToFmtREMap["unsigned int"]            = "[^%]*%#?0?-? ?+?'?I?(([1-9][0-9]*)?(\\.[0-9]*)?)?[ouxX]{1}";
    typeNameToFmtREMap["unsigned long int"]       = "[^%]*%#?0?-? ?+?'?I?(([1-9][0-9]*)?(\\.[0-9]*)?)?l[ouxX]{1}";
    typeNameToFmtREMap["unsigned long long int"]  = "[^%]*%#?0?-? ?+?'?I?(([1-9][0-9]*)?(\\.[0-9]*)?)?ll[ouxX]{1}";
    typeNameToFmtREMap["short int"]               = "[^%]*%#?0?-? ?+?'?I?(([1-9][0-9]*)?(\\.[0-9]*)?)?h[di]{1}";
    typeNameToFmtREMap["unsigned short int"]      = "[^%]*%#?0?-? ?+?'?I?(([1-9][0-9]*)?(\\.[0-9]*)?)?h[ouxX]{1}";
    typeNameToFmtREMap["char"]                    = "[^%]*%c{1}";
    typeNameToFmtREMap["unsigned char"]           = "[^%]*%#?0?-? ?+?'?I?(([1-9][0-9]*)?(\\.[0-9]*)?)?hh[ouxX]{1}";
    typeNameToFmtREMap["char*"]                   = "[^%]*%#?0?-? ?+?'?I?(([1-9][0-9]*)?(\\.[0-9]*)?)?s{1}";
    typeNameToFmtREMap["void*"]                   = "[^%]*%p{1}";
    typeNameToFmtREMap["size_t"]                  = "[^%]*%#?0?-? ?+?'?I?(([1-9][0-9]*)?(\\.[0-9]*)?)?z[ouxX]{1}";

    // aliases
    typeNameToFmtREMap["long"]                    = typeNameToFmtREMap["long int"]; 
    typeNameToFmtREMap["long long"]               = typeNameToFmtREMap["long long int"]; 
    typeNameToFmtREMap["unsigned"]                = typeNameToFmtREMap["unsigned int"]; 
    typeNameToFmtREMap["unsigned long"]           = typeNameToFmtREMap["unsigned long int"]; 
    typeNameToFmtREMap["unsigned long long"]      = typeNameToFmtREMap["unsigned long long int"]; 
    typeNameToFmtREMap["short"]                   = typeNameToFmtREMap["short int"]; 
    typeNameToFmtREMap["unsigned short"]          = typeNameToFmtREMap["unsigned short int"];
}

// ****************************************************************************
//  Function: ValidatePrintfFormatString
//
//  Purpose: Validates a printf style format string against a variable length
//  list of argument type names. 
//
//  Programmer: Mark C. Miller 
//  Creation:   September 20, 2007 
//
//  Modifications:
//    Mark C. Miller, Fri Sep 21 07:31:02 PDT 2007
//    Fixed end anchor to be any chars not a '%'
//
//    Mark C. Miller, Wed Dec  5 17:53:06 PST 2007
//    Fixed issue with missing REG_EXTENDED on Mac by adding check for simple
//    case and falling back to REs for more complicated cases.
// ****************************************************************************
bool
StringHelpers::ValidatePrintfFormatString(const char *fmtStr, const char *arg1Type, ... )
{
    int n;
    int i;

    // this block should be removed with regcomp on Mac works with REG_EXTENDED
#if defined(__APPLE__)
    // first char must be a conversion specifier ('%')
    n = 0;
    if (fmtStr[n] == '%')
    {
        n++;

        // optional sign designation
        if (strchr(" +-", fmtStr[n]) != 0)
            n++;

        // walk over field width digits
        while (fmtStr[n] >= '0' && fmtStr[n] <= '9')
            n++;
   
        // optional dot
        if (fmtStr[n] == '.')
        {
	    n++;
	    // walk over precision digits
            while (fmtStr[n] >= '0' && fmtStr[n] <= '9')
                n++;
        }

        if (strchr("eEfFgGaAouxXdi", fmtStr[n]) != 0)
        {
            if (fmtStr[n+1] == '\0')
                return true;
        }
    }
#endif

    //
    // fall back to RE based validation
    //
    string re = "^"; // anchor first char to beginning of line

    // compute length up to max of 4096
    n = 0;
    while (n < 4096 && fmtStr[n] != '\0')
        n++;
    if (n == 4096)
        return false;

    // count conversion specs.
    int ncspecs = 0;
    for (i = 0; i < n-1; i++)
    {
        if (fmtStr[i] == '%' && fmtStr[i+1] != '%')
	    ncspecs++;
    }
    if (ncspecs == 0)
        return true;

    InitTypeNameToFmtREMap();

    // start processing the varargs list
    va_list ap;
    va_start(ap, arg1Type);
    const char *currentArgTypeName = arg1Type; 
    // loop adding RE terms for each argument type
    for (i = 0; i < ncspecs; i++)
    {
	if (typeNameToFmtREMap.find(string(currentArgTypeName)) == typeNameToFmtREMap.end())
	    break;
        re += typeNameToFmtREMap[string(currentArgTypeName)];
	currentArgTypeName = va_arg(ap, const char *);
    }
    va_end(ap);

    // if we broke out of loop early, a bad type name was encountered
    if (i < ncspecs)
        return false;

    re += "[^%]*$"; // anchor last char to end of line

    return StringHelpers::FindRE(fmtStr, re.c_str()) >= 0;
}
