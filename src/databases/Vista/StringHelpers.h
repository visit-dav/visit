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
   void GroupStrings(vector<string> stringList,
                     vector<vector<string> > &stringGroups,
                     vector<string> &groupNames);
}

#endif
