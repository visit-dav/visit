// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_MAP_TYPES_H
#define VISIT_MAP_TYPES_H
#include <string>
#include <vectortypes.h>
#include <map>

typedef std::map<std::string, int>          StringIntMap;
typedef std::map<std::string, std::string>  StringStringMap;
typedef StringIntMap                        LevelColorMap;
typedef std::map<std::string, stringVector> StringStringVectorMap;
typedef std::map<double, int>               DoubleIntMap;

#endif
