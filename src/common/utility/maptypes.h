// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_MAP_TYPES_H
#define VISIT_MAP_TYPES_H
#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <string>
#include <vectortypes.h>

typedef std::map<std::string, int>          StringIntMap;
typedef std::map<std::string, std::string>  StringStringMap;
typedef StringIntMap                        LevelColorMap;
typedef std::map<std::string, stringVector> StringStringVectorMap;
typedef std::map<double, int>               DoubleIntMap;

// CI short for Case-Insenstive
// The two transform lines here previously used C lib `tolower` as the 4th argument
// That is incorrect as it works on ints, not chars. C++ transform over strings is
// expecting a method that operates on chars. Using C++ tolower works but there are
// multiple std::twolower methods and so we use a lambda notation to force it to
// find the correct one.
struct CIComparator {
  bool operator() (const std::string& s1, const std::string& s2) const
  {
      std::string str1(s1.length(),' ');
      std::string str2(s2.length(),' ');
      std::transform(s1.begin(), s1.end(), str1.begin(), [](unsigned char c)->char{return std::tolower(c);});
      std::transform(s2.begin(), s2.end(), str2.begin(), [](unsigned char c)->char{return std::tolower(c);});
      return  str1 < str2;
  }
};

typedef std::multimap<std::string, bool, CIComparator> CIStringBoolMap;
typedef std::multimap<std::string, std::string, CIComparator> CIStringStringMap;

typedef std::set<std::string, CIComparator> CIStringSet;
typedef std::map<std::string, CIStringSet> StringCIStringSetMap;
// there is a separate include for vector defines, but need the
// Comparator.
typedef std::vector<CIStringSet> CIStringSetVector;


#endif
