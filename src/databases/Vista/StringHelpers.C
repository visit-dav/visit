#include <StringHelpers.h>

#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

static string IGNORE_CHARS = StringHelpers::NON_RELEVANT_CHARS;

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

void
StringHelpers::GroupStrings(vector<string> stringList,
                            vector<vector<string> > &stringGroups,
                            vector<string> &groupNames,
                            int numLeadingVals,
                            string nonRelevantChars)
{

   int i;
   int nStrings = stringList.size();

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

   // now, scan the sorted list of strings for value transitions
   // in first N (default 3) chars. Each such transition indicates the end of
   // one group and the beginning of the next. The 'first 3' criterion
   // is arbitrary but seems to work well.
   if (numLeadingVals > stringList[0].size()-1)
       numLeadingVals  = stringList[0].size()-1;
   string lastVal;
   for (i = 0; i < numLeadingVals; i++)
       lastVal += stringPtrs[0][i];
   groupNames.push_back(RelevantString(stringPtrs[0]));
   vector<string> curGroup;
   curGroup.push_back(stringPtrs[0]);
   for (i = 1; i < nStrings; i++)
   {
       string thisVal;
       int j;
       for (j = 0; j < numLeadingVals; j++)
           thisVal += stringPtrs[i][j];

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

int
StringHelpers::FindRE(const char *strToSearch, const char *re)
{
    regex_t cre;
    regmatch_t pm;

    if (regcomp(&cre, re, REG_EXTENDED))
        return FindError;

    int rval = regexec(&cre, strToSearch, 1, &pm, 0);

    if (rval == REG_NOMATCH)
        return FindNone;

    if (pm.rm_so >= strlen(strToSearch))
        return FindError;

    if (pm.rm_so < 0)
        return FindError;

    return (int) pm.rm_so;
}
