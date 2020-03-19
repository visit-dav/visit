// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef INSTALLATION_FUNCTIONS_H
#define INSTALLATION_FUNCTIONS_H
#include <string>
#include <misc_exports.h>

typedef enum {CONFIGSTATE_IOERROR,
              CONFIGSTATE_FIRSTTIME,
              CONFIGSTATE_SUCCESS} ConfigStateEnum;

// Installation directory functions
std::string MISC_API GetVisItInstallationDirectory();
std::string MISC_API GetVisItInstallationDirectory(const char *version);
bool        MISC_API ReadInstallationInfo(std::string &distName, 
                                          std::string &configName, 
                                          std::string &bankName);

std::string MISC_API GetVisItArchitectureDirectory();
std::string MISC_API GetVisItArchitectureDirectory(const char *version);
std::string MISC_API GetVisItLauncher();
std::string MISC_API GetVisItLibraryDirectory();
std::string MISC_API GetVisItLibraryDirectory(const char *version);

// User installation / data functions
std::string MISC_API GetUserVisItDirectory();
MISC_API char *      GetDefaultConfigFile(const char *filename = 0, const char *home = 0);
MISC_API char *      GetSystemConfigFile(const char *filename = 0);

std::string MISC_API GetUserVisItRCFile();
std::string MISC_API GetSystemVisItRCFile();

std::string MISC_API GetAndMakeUserVisItHostsDirectory();
std::string MISC_API GetVisItHostsDirectory();
std::string MISC_API GetSystemVisItHostsDirectory();

typedef enum {
    VISIT_RESOURCES,
    VISIT_RESOURCES_COLORTABLES,
    VISIT_RESOURCES_HELP,
    VISIT_RESOURCES_HOSTS,
    VISIT_RESOURCES_TRANSLATIONS,
    VISIT_RESOURCES_MOVIETEMPLATES,
    VISIT_RESOURCES_IMAGES
} VisItResourceDirectoryType;

std::string MISC_API GetVisItResourcesDirectory(VisItResourceDirectoryType t);
std::string MISC_API GetVisItResourcesFile(VisItResourceDirectoryType t, 
                                           const std::string &filename);

int         MISC_API ConfigStateGetRunCount(ConfigStateEnum &code);
void        MISC_API ConfigStateIncrementRunCount(ConfigStateEnum &code);

// Version functions
int         MISC_API GetVisItVersionFromString(const char*, int&, int&, int&);
bool        MISC_API VisItVersionsCompatible(const char*, const char*);
bool        MISC_API VersionGreaterThan(const std::string &v1, const std::string &v2);
void        MISC_API SetIsDevelopmentVersion(bool val);
bool        MISC_API GetIsDevelopmentVersion();

// Process IDs
std::string MISC_API GetVisItPIDString();


#endif
