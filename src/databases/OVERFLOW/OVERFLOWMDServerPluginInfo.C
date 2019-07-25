// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <OVERFLOWPluginInfo.h>

#ifdef OVERFLOW
#undef OVERFLOW
#endif
VISIT_DATABASE_PLUGIN_ENTRY(OVERFLOW,MDServer)

// this makes compilers happy... remove if we ever have functions here
void OVERFLOWMDServerPluginInfo::dummy()
{
}

