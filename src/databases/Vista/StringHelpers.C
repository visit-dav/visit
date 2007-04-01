#include <StringHelpers.h>

#include <stdlib.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

static string IGNORE_CHARS = "`~!@#$%^&*()-_=+{[}]|\\:;\"'<,>.?/0123456789";

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
                            vector<string> &groupNames)
{

   int i;
   int nStrings = stringList.size();

   // first, we need to sort the strings. Well, we don't really sort the
   // strings. Instead we sort an array of pointers to the strings. We
   // build that array here.
   const char **stringPtrs = new const char*[nStrings];
   for (i = 0; i < nStrings; i++)
       stringPtrs[i] = stringList[i].c_str();

   // now, call qsort for this array of string pointers
   qsort(stringPtrs, nStrings, sizeof(char *), CompareRelevantStrings);

   // now, scan the sorted list of strings for value transitions
   // in first three chars. Each such transition indicates the end of
   // one group and the beginning of the next. The 'first 3' criterion
   // is arbitrary but seems to work well.
   string lastVal;
   lastVal += stringPtrs[0][0];
   lastVal += stringPtrs[0][1];
   lastVal += stringPtrs[0][2]; 
   groupNames.push_back(RelevantString(stringPtrs[0]));
   vector<string> curGroup;
   curGroup.push_back(stringPtrs[0]);
   for (i = 1; i < nStrings; i++)
   {
       string thisVal;
       thisVal += stringPtrs[i][0];
       thisVal += stringPtrs[i][1];
       thisVal += stringPtrs[i][2]; 

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
