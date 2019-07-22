// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CONFIGURE_INFORMATION_H
#define CONFIGURE_INFORMATION_H
#include <misc_exports.h>
#include <string>

namespace visitcommon {

std::string MISC_API Version();

std::string MISC_API GITVersion();

std::string MISC_API VersionControlVersionString();

std::string MISC_API SlashString();

char MISC_API SlashChar();

std::string MISC_API PluginExtension();

int MISC_API RenderingSizeLimit();

};

#endif
