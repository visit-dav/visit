#ifndef VISIT_MAP_TYPES_H
#define VISIT_MAP_TYPES_H
#include <string>
#include <vectortypes.h>

#ifdef WIN32
// Use VisIt's map class.
#include <VisItMap.h>
typedef VisItMap<std::string, int>          StringIntMap;
typedef StringIntMap                        LevelColorMap;
typedef VisItMap<std::string, stringVector> StringStringVectorMap;
#else
#include <map>
typedef std::map<std::string, int>          StringIntMap;
typedef StringIntMap                        LevelColorMap;
typedef std::map<std::string, stringVector> StringStringVectorMap;
#endif

#endif
