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
#include <algorithm>
#include <map>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>

#include <visit-config.h>
#include <FileFunctions.h>

using std::map;
using std::string;
using std::vector;

static string IGNORE_CHARS = StringHelpers::NON_RELEVANT_CHARS;

bool has_nonspace_chars(const std::string &s);

#if defined(_WIN32) || defined(__APPLE__)

#define MACCESS(...)

#else

#include <stdint.h>    // uintptr_t
#include <stdio.h>
#include <unistd.h>     // getpid...

extern int etext;

#define MAXRNGS 100

static struct rng { uintptr_t alpha, omega; } rngv[MAXRNGS], *rend = NULL;

void maccess_init()
{
    uintptr_t       brk = (uintptr_t)sbrk(0);
    char            buf[99];
    sprintf(buf, "/proc/%d/maps", getpid());
    FILE            *fp = fopen(buf, "re");
    rend = rngv;
    while (0 < fscanf(fp, "%lx-%lx %4s %*[^\n]",
                          &rend->alpha, &rend->omega, buf)) {
        if (buf[1] == '-' || rend->alpha < brk)
            continue;
        else if (rend > rngv && rend->alpha == rend[-1].omega)
            rend[-1].omega = rend->omega;
        else if (++rend == rngv+MAXRNGS)
            break;
    }
    fclose(fp);
}

// On my home system, "sbrk(0)" takes about .015 usec.
int maccess(void const *mem, int len)
{
    if ((intptr_t)mem < 0 && (intptr_t)mem + len >= 0)
        return 0;
    if ((char const*)mem + len < (char const*)sbrk(0))
        return mem > (void*)&etext;
    if (!rend)
        maccess_init();
    struct rng *p;
    for (p = rngv; p != rend; ++p)
        if ((uintptr_t)mem + len <= p->omega)
            return (uintptr_t)mem >= p->alpha;
    return 0;
}

#define MACCESS(curArgTypeName, num) if( maccess(curArgTypeName, num) == 0 ) break;

#endif

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
   string lastVal = FileFunctions::Dirname(stringPtrs[0]);
   groupNames.push_back(lastVal);
   vector<string> curGroup;
   curGroup.push_back(stringPtrs[0]);
   for (i = 1; i < nStrings; i++)
   {
       string thisVal = FileFunctions::Dirname(stringPtrs[i]);

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
//  Modifications:
//
//    Mark C. Miller, Wed Aug 26 11:16:31 PDT 2009
//    Made version with string args.
//
//    Mark C. Miller, Mon Aug 31 14:37:23 PDT 2009
//    Made string version use const references.
// ****************************************************************************

int
StringHelpers::FindRE(const string &s, const string &re)
{
    return StringHelpers::FindRE(s.c_str(), re.c_str());
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

    if (pm.rm_so >= (int)strlen(strToSearch))
        return FindError;

    if (pm.rm_so < 0)
        return FindError;

    return (int) pm.rm_so;
}

// ****************************************************************************
//  Function: ReplaceRE
//
//  Purpose: Replace portion of string matching RE with replacement string
//
//  Programmer: Mark C. Miller
//  Creation:   August 17, 2009
//
//  Modifications:
//    Mark C. Miller, Mon Aug 31 14:37:45 PDT 2009
//    Made it use const (where possible) references to strings.
// ****************************************************************************
bool
StringHelpers::ReplaceRE(string &s, const string &re, const string &repl)
{
    int n = FindRE(s, re);
    if (n < 0)
        return false;

    string news = string(s,0,n) + repl;
    s = news;
    return true;
}


// ****************************************************************************
//  Function: Replace
//
//  Purpose: Simple string replace helper.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Mar 17 13:00:43 PDT 2010
//
//
//  Modifications:
//
// ****************************************************************************

string
StringHelpers::Replace(const string &source,
                       const string &before,
                       const string &after)
{
    string res = source;
    string::size_type before_len = before.size();
    string::size_type after_len  = after.size();
    if(before_len == 0)
        return res;
    string::size_type pos = 0; // Must initialize
    while ( ( pos = res.find (before,pos) ) != string::npos )
    {
        res.replace(pos,before_len,after);
        pos += after_len;
    }

    return res;
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
//                      "<.*_([0-9]{4})_.*\\..*> \0"
//          opening char-^                     ^--closing char
//                        ^------RE part------^
//
//  Do a 'man 7 regex' to get more information on regular expression syntax
//
//  Note: The substring reference implemented here is a zero-origin index.
//  That is NOT consistent with 'man 7 regex' which uses a one-origin index.
//  We should probably update this logic to use one-origin index.
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

    typeNameToFmtREMap["float"]                   = "[^%]*%#?0?-? ?\\+?'?(([1-9][0-9]*)?(\\.[0-9]*)?)?[eEfFgGaA]{1}";
    typeNameToFmtREMap["double"]                  = "[^%]*%#?0?-? ?\\+?'?(([1-9][0-9]*)?(\\.[0-9]*)?)?[eEfFgGaA]{1}";
    typeNameToFmtREMap["long double"]             = "[^%]*%#?0?-? ?\\+?'?(([1-9][0-9]*)?(\\.[0-9]*)?)?L[eEfFgGaA]{1}";
    typeNameToFmtREMap["int"]                     = "[^%]*%#?0?-? ?\\+?'?I?(([1-9][0-9]*)?(\\.[0-9]*)?)?[di]{1}";
    typeNameToFmtREMap["long int"]                = "[^%]*%#?0?-? ?\\+?'?I?(([1-9][0-9]*)?(\\.[0-9]*)?)?l[di]{1}";
    typeNameToFmtREMap["long long int"]           = "[^%]*%#?0?-? ?\\+?'?I?(([1-9][0-9]*)?(\\.[0-9]*)?)?ll[di]{1}";
    typeNameToFmtREMap["unsigned int"]            = "[^%]*%#?0?-? ?\\+?'?I?(([1-9][0-9]*)?(\\.[0-9]*)?)?[ouxX]{1}";
    typeNameToFmtREMap["unsigned long int"]       = "[^%]*%#?0?-? ?\\+?'?I?(([1-9][0-9]*)?(\\.[0-9]*)?)?l[ouxX]{1}";
    typeNameToFmtREMap["unsigned long long int"]  = "[^%]*%#?0?-? ?\\+?'?I?(([1-9][0-9]*)?(\\.[0-9]*)?)?ll[ouxX]{1}";
    typeNameToFmtREMap["short int"]               = "[^%]*%#?0?-? ?\\+?'?I?(([1-9][0-9]*)?(\\.[0-9]*)?)?h[di]{1}";
    typeNameToFmtREMap["unsigned short int"]      = "[^%]*%#?0?-? ?\\+?'?I?(([1-9][0-9]*)?(\\.[0-9]*)?)?h[ouxX]{1}";
    typeNameToFmtREMap["char"]                    = "[^%]*%c{1}";
    typeNameToFmtREMap["unsigned char"]           = "[^%]*%#?0?-? ?\\+?'?I?(([1-9][0-9]*)?(\\.[0-9]*)?)?hh[ouxX]{1}";
    typeNameToFmtREMap["char*"]                   = "[^%]*%#?0?-? ?\\+?'?I?(([1-9][0-9]*)?(\\.[0-9]*)?)?s{1}";
    typeNameToFmtREMap["void*"]                   = "[^%]*%p{1}";
    typeNameToFmtREMap["size_t"]                  = "[^%]*%#?0?-? ?\\+?'?I?(([1-9][0-9]*)?(\\.[0-9]*)?)?z[ouxX]{1}";

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
        //MACCESS(currentArgTypeName, 1);
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

// ****************************************************************************
//  Function: car
//
//  Purpose: Pulls the first word out of a delimited string.
//
//  Programmer: Tom Fogal
//  Creation:   August 5, 2008
//
//  Modifications:
//
//    Tom Fogal, Wed Nov  4 17:14:29 MST 2009
//    Take an argument to use as the separating character.
//
// ****************************************************************************
std::string
StringHelpers::car(const std::string s, const char separator)
{
    if(s.find(separator) != std::string::npos) {
        return s.substr(0, s.find(separator));
    }
    return s;
}

// ****************************************************************************
//  Function: cdr
//
//  Purpose: Removes the first word from a delimited string.
//
//  Programmer: Tom Fogal
//  Creation:   August 5, 2008
//
//  Modifications:
//
//    Tom Fogal, Wed Nov  4 17:14:29 MST 2009
//    Take an argument to use as the separating character.
//
// ****************************************************************************
std::string
StringHelpers::cdr(const std::string s, const char separator)
{
    std::string::size_type sep;
    if((sep = s.find(separator)) != std::string::npos) {
        return s.substr(sep+1);
    }
    return s;
}


//****************************************************************************
//  Function: has_nonspace_chars
//
//  Purpose: Predicate to determine if a string has any useful content.
//
//  Programmer: Tom Fogal
//  Creation:   August 20, 2008
//
//  Modifications:
//
//****************************************************************************
bool
has_nonspace_chars(const std::string &s)
{
    std::string::const_iterator iter = s.begin();

    while(iter != s.end())
    {
        if(!isspace(*iter))
        {
            return true;
        }
    }
    return false;
}

// ****************************************************************************
//  Function: append
//
//  Purpose: Append all elements from one vector to another.
//
//  Programmer: Tom Fogal
//  Creation:   August 5, 2008
//
//  Modifications:
//
//    Tom Fogal, Thu Aug  7 16:39:39 EDT 2008
//    Remove/ignore empty strings.
//
// ****************************************************************************
void
StringHelpers::append(std::vector<std::string> &argv,
                      std::vector<std::string> lst)
{
    if(lst.empty()) { return; }

    if(has_nonspace_chars(lst.front()))
    {
        argv.push_back(lst.front());
    }
    lst.erase(lst.begin());
    append(argv, lst);
}

// ****************************************************************************
//  Function: split
//
//  Purpose: Splits a string based on a delimiter
//
//  Programmer: Tom Fogal
//  Creation:   December 7, 2009
//
//  Modifications:
//
// ****************************************************************************
std::vector<std::string>
StringHelpers::split(const std::string input, const char separator)
{
    std::istringstream iss(input);
    std::string cur;
    std::vector<std::string> retval;
    while(std::getline(iss, cur, separator))
    {
        if(iss)
        {
            retval.push_back(cur);
        }
    }
    return retval;
}

// ****************************************************************************
//  Function: Plural (and support structures)
//
//  Purpose: Given singular english noun, compute plural form
//
//  Programmer: Mark C. Miller
//  Creation:   August 17, 2009
//
//  Notes: The regular expression table below is by no means complete in that
//  it covers all nouns in the english language. However, it does cover most
//  of those likely to be used in VisIt (for names of set for example). It was
//  compiled and re-coded for C++ after reading several sources on the web
//  including http://www.csse.monash.edu.au/~damian/papers/HTML/Plurals.html
//  and http://www.emacswiki.org/emacs/plural.el
//
//  Modifications:
//
//    Mark C. Miller, Mon Aug 31 14:38:18 PDT 2009
//    Made function arg a reference.
//
//    Jeremy Meredith, Tue Mar 29 14:38:02 EDT 2011
//    Added a new flavor of this function which only pluralizes if
//    a passed integer is greater than one.
//
// ****************************************************************************
typedef struct _plural_rule_t {
    const char *re;
    const char *repl;
} plural_rule_t;

static const plural_rule_t plural_rules_table[] = {
    {"ss$", "sses"},
    {"zz$", "zzes"},
    {"sh$", "shes"},
    {"tch$", "tches"},
    {"eaf$", "eaves"},
    {"ief$", "ieves"},
    {"roof$", "roofs"},
    {"ife$", "ives"},
    {"lf$", "lves"},
    {"ay$", "ays"},
    {"ey$", "eys"},
    {"iy$", "iys"},
    {"oy$", "oys"},
    {"uy$", "uys"},
    {"ndum$", "nda"},
    {"um$", "a"},
    {"schema$", "schemas"},
    {"ia$", "ium"},
    {"ma$", "mata"},
    {"na$", "nae"},
    {"ta$", "tum"},
    {"atlas$", "atlases"},
    {"aircraft$", "aircraft"},
    {"alga$", "algae"},
    {"alumna$", "alumnae"},
    {"automaton$", "automata"},
    {"corpus$", "corpora"},
    {"cs$", "csen"},
    {"foot$", "feet"},
    {"formula$", "formulae"},
    {"rion$", "ria"},
    {"focus$", "foci"},
    {"genus$", "genera"},
    {"hedron$", "hedra"},
    {"index$", "indices"},
    {"ouse$", "ice"},
    {"man$", "men"},
    {"matrix$", "matrices"},
    {"nucleus$", "nuclei"},
    {"offspring", "offspring"},
    {"phenomenon$", "phenomena"},
    {"people$", "people"},
    {"perch$", "perch"},
    {"piano$", "pianos"},
    {"police$", "police"},
    {"portico$", "porticos"},
    {"quarto$", "quartos"},
    {"radius$", "radii"},
    {"solo$", "solos"},
    {"syllabus$", "syllabi"},
    {"terminus$", "termini"},
    {"ulus$", "uli"},
    {"tooth$", "teeth"},
    {"uterus$", "uteri"},
    {"virtuoso", "virtuosi"},
    {"viscus$", "viscera"},
    {"is$", "es"},
    {"us$", "uses"},
    {"io$", "ios"},
    {"oo$", "oos"},
    {"o$", "oes"},
    {"y$", "ies"},
    {"ex$", "ices"},
    {"ix$", "ices"},
    {"x$", "xes"}
};
static const int NPluralRules = sizeof(plural_rules_table) / sizeof(plural_rules_table[0]);

string
StringHelpers::Plural(const string &s)
{
    string ps = s;
    for (int n = 0; n < NPluralRules; n++)
    {
        if (ReplaceRE(ps, plural_rules_table[n].re, plural_rules_table[n].repl))
            return ps;
    }
    // We have no other choice but to simply try adding 's'
    return ps + "s";
}

string
StringHelpers::Plural(int n, const std::string &s)
{
    if (n == 1)
        return s;
    else
        return Plural(s);
}

// ****************************************************************************
// Method:  StringHelpers::HumanReadableList
//
// Purpose:
//   Turns a vector of strings into a comma-delimited single string
//   appropriate for insertion into a paragraph of text.
//
// Arguments:
//   sv         the string vector to turn into a single comma-delimited string
//
// Programmer:  Jeremy Meredith
// Creation:    March 29, 2011
//
// ****************************************************************************
string
StringHelpers::HumanReadableList(const std::vector<std::string> &sv)
{
    int n = sv.size();
    string s;
    for (int i=0; i<n; i++)
    {
        if (i > 0 && i == n-1)
            s += ", and "; // to serial comma, or not to serial comma?
        else if (i > 0)
            s += ", ";
        s += sv[i];
    }
    return s;
}


// ****************************************************************************
// Method:  StringHelpers::IsPureASCII
//
// Purpose:
//   Check to see if a string is purely ASCII printable characters.
//
// Arguments:
//   txt        the string data
//   length     the maximum number of bytes to check
//
// Programmer:  Jeremy Meredith
// Creation:    January  7, 2010
//
// ****************************************************************************
bool
StringHelpers::IsPureASCII(const std::string &txt)
{
    return IsPureASCII(txt.c_str(), txt.length());
}

bool
StringHelpers::IsPureASCII(const char *const txt, size_t length)
{
    for (size_t i=0; i<length; i++)
    {
        const unsigned char c = txt[i];

        if ((c>1 && c<7) || (c>13 && c<32) || c>127)
            return false;

        // if it is an ascii string, don't check past its terminator
        if (c==0)
            break;
    }
    return true;
}

// ****************************************************************************
// Method:  StringHelpers::CaseInsenstiveEqual
//
// Purpose:
//   Check to see two strings compare as equal, after result of ::tolower.
//
// Arguments:
//   str_a, str_b   Input strings.
//
// Programmer:  Cyrus Harrison
// Creation:    Mon Sep 19 16:23:05 PDT 2011
//
// ****************************************************************************
bool
StringHelpers::CaseInsenstiveEqual(const std::string &str_a,
                                   const std::string &str_b)
{
    std::string sa_l = str_a;
    std::string sb_l = str_b;
    std::transform(sa_l.begin(),sa_l.end(),sa_l.begin(),::tolower);
    std::transform(sb_l.begin(),sb_l.end(),sb_l.begin(),::tolower);
    return sa_l == sb_l;
}

// ****************************************************************************
// Method:  StringHelpers::rtrim
//
// Purpose:
//  Trim whitespace off the right of a string.
//
//
// Programmer:  Cyrus Harrison
// Creation:   Tue Mar 12 12:07:21 PDT 2013
//
// ****************************************************************************
void
StringHelpers::rtrim(string &val)
{
    // trim trailing spaces & tabs
    size_t pos = val.find_last_not_of(" \t");
    if( string::npos != pos)
    {
        val = val.substr( 0, pos+1 );
    }
}


// ****************************************************************************
// Method:  StringHelpers::ltrim
//
// Purpose:
//  Trim whitespace off the left of a string.
//
//
// Programmer:  Cyrus Harrison
// Creation:   Tue Mar 12 12:07:21 PDT 2013
//
// ****************************************************************************
void
StringHelpers::ltrim(string &val)
{
    // trim leading spaces & tabs
    size_t pos = val.find_first_not_of(" \t");
    if( string::npos != pos )
    {
        val = val.substr( pos );
    }
}


// ****************************************************************************
// Method:  StringHelpers::trim
//
// Purpose:
//  Trim whitespace off the left and right of a string.
//
// Programmer:  Cyrus Harrison
// Creation:   Tue Mar 12 12:07:21 PDT 2013
//
// ****************************************************************************

void
StringHelpers::trim(string &val)
{
    rtrim(val);
    ltrim(val);
}

// ****************************************************************************
// Method: StringHelpers::UpperCase
//
// Purpose:
//   Make the string all upper case.
//
// Arguments:
//   src : The source string.
//
// Returns:    An upper case string.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep  8 14:42:48 PDT 2014
//
// Modifications:
//
// ****************************************************************************

std::string
StringHelpers::UpperCase(const std::string &src)
{
    std::string tmp(src);
    for(size_t i = 0; i < tmp.size(); ++i)
    {
        if(tmp[i] >= 'a' && tmp[i] <= 'z')
            tmp[i] -= 'a' - 'A';
    }
    return tmp;
}

// ****************************************************************************
//  Method:  StringHelpers::StringToInt
//
//  Purpose:
//     Utility method to convert a string to an int. returns true if the 
//     conversion was successful.
//  Arguments 
//    input: the string to convert to an int
//    output: an integer representation of the string
//
//  Returns:
//    boolean indicating if the conversion was successful.
//
//  Progammer:  Matt Larsen
//  Creation:   Dec 12, 2016
//
//  Modifications:
//
// ****************************************************************************

bool 
StringHelpers::StringToInt(const string &input, int &output)
{
    bool valid 
        = input.find_first_not_of(" 0123456789") == std::string::npos;
    if(!valid) return false;
    
    output = atoi(input.c_str());
    //
    //  Check for conversion errors
    //
    if(output == 0 && input != "0") 
        return false;

    return true;
}
//
// ****************************************************************************
//  Method:  StringHelpers::ParseRange
//
//  Purpose:
//      To parse a string that constains a range of positive integers
//      in a comma separated list. For example, "1,5-6,10" becomes "1,5,6,10".
//  
//
//  Progammer:  Matt Larsen
//  Creation:   Dec 12, 2016
//
//  Modifications:
//
// ****************************************************************************
bool
StringHelpers::ParseRange(const string range, std::vector<int> &list)
{
    std::vector<std::string> rangeTokens = StringHelpers::split(range, ',');

    bool parseError = false;
    for(int i =0; i < rangeTokens.size(); ++i)
    {
        int first = 0;
        int last = 0;
        std::vector<std::string> currentRange 
            = StringHelpers::split(rangeTokens.at(i),'-');
        const int size = static_cast<int>(currentRange.size());
        
        //
        // Check to see that we have exactly one or two items 
        //

        if(size < 1 || size > 2) 
        {
            parseError = true;
            continue;
        }
       
        size_t dashPos = rangeTokens[i].find("-");
        bool hasDash = dashPos != std::string::npos;

        if(hasDash && size == 1)
        {
            parseError = true;
            continue;
        }
         
        bool valid = StringHelpers::StringToInt(currentRange[0], first);

        if(!valid)
        {
            parseError = true;
            continue;
        }
        
        if(size == 1) 
        {
            list.push_back(first);
            continue;
        }

        valid = StringHelpers::StringToInt(currentRange[1], last);

        if(!valid)
        {
            parseError = true;
            continue;
        } 
        
        //
        // Allow for backward range and don't go into inf loop
        //

        if(first > last)
        {
            int tmp = first;
            first = last;
            last = tmp;
        }

        for(int n = first; n <= last; ++n)
        {
            list.push_back(n);
        }
    }
  
    return parseError;
}

