#include <QString>
#include <QTextStream>
QTextStream cOut(stdout);
QTextStream cErr(stderr);
QString     Endl("\n");

#include "Field.h"
#include "Attribute.h"
#include "Enum.h"
#include "Plugin.h"

#include <BJHash.h>
#include <stdio.h>
#ifndef WIN32
#include <unistd.h>
#endif

std::vector<EnumType*> EnumType::enums;

bool print   = true;
bool clobber = false;
bool using_dev = false;
bool installpublic  = false;
bool installprivate = false;
bool outputtoinputdir = false;
QString currentInputDir = "";
QString preHeaderLeader = "pre_";

const char *copyright_str =
"// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt\n"
"// Project developers.  See the top-level LICENSE file for dates and other\n"
"// details.  No copyright assignment is required to contribute to VisIt.\n";
