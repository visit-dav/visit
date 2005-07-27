// ************************************************************************* //
//                              StringHelpers.h                              //
// ************************************************************************* //

#ifndef STRINGHELPERS_H
#define STRINGHELPERS_H
#include <utility_exports.h>

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace StringHelpers
{
   const string NON_RELEVANT_CHARS = "`~!@#$%^&*()-_=+{[}]|\\:;\"'<,>.?/0123456789";

   enum FindResult {FindNone = -1, FindError = -2};

   void UTILITY_API GroupStrings(vector<string> stringList,
                     vector<vector<string> > &stringGroups,
                     vector<string> &groupNames,
                     int numLeadingVals = 3,
                     string nonRelevantChars = NON_RELEVANT_CHARS);
   void UTILITY_API GroupStringsAsPaths(vector<string> stringList,
                     vector<vector<string> > &stringGroups,
                     vector<string> &groupNames);
   void UTILITY_API GroupStringsFixedAlpha(vector<string> stringList,
                     int numGroups,
                     vector<vector<string> > &stringGroups);
   int UTILITY_API FindRE(const char *stringToSearch, const char *re);

   const char UTILITY_API *Basename(const char *path);
   const char UTILITY_API *Dirname(const char *path);
}

#endif
