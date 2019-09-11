// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              MDServerProxy.C                              //
// ************************************************************************* //
#include <MDServerManager.h>


//MDServerManager* MDServerManager::manager = new MDServerManager();
MDServerManager manager;

MDServerManager::ServerInfo::ServerInfo()
{
    proxy = 0;
}

MDServerManager::ServerInfo::~ServerInfo()
{}

MDServerManager::MDServerManager()
{}

MDServerManager::~MDServerManager()
{}

MDServerManager*
MDServerManager::Instance()
{
    return &manager;
}

