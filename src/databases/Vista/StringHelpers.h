// ************************************************************************* //
//                              StringHelpers.h                              //
// ************************************************************************* //

#ifndef STRINGHELPERS_H
#define STRINGHELPERS_H

#include <string>
#include <vector>

using std::string;
using std::vector;


namespace StringHelpers
{
   const string NON_RELEVANT_CHARS = "`~!@#$%^&*()-_=+{[}]|\\:;\"'<,>.?/0123456789";

   enum FindResult {FindNone = -1, FindError = -2};

   void GroupStrings(vector<string> stringList,
                     vector<vector<string> > &stringGroups,
                     vector<string> &groupNames,
                     int numLeadingVals = 3,
                     string nonRelevantChars = NON_RELEVANT_CHARS);
   int FindRE(const char *stringToSearch, const char *re);

   const char * Basename(const char *path);
   const char * Dirname(const char *path);
}

#endif
