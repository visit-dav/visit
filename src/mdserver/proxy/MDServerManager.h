// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            MDServerManager.h                              //
// ************************************************************************* //

#ifndef _MDSERVERMANAGER_H_
#define _MDSERVERMANAGER_H_

#include <mdsproxy_exports.h>

#include <MDServerProxy.h>


// ****************************************************************************
//  Class: MDServerManager
//
//  Purpose:
//      MDServerManager maintains a list of all the MDServerProxy connections..
//
//  Note:
//
//  Programmer:
//  Creation:
//
//  Modifications:
//
// ****************************************************************************



class MDSERVER_PROXY_API MDServerManager
{
public:
    struct MDSERVER_PROXY_API ServerInfo
    {
        ServerInfo();
        ~ServerInfo();

        union {
        MDServerProxy *proxy;
        MDServerProxy *server;
        };
        stringVector  arguments;
        std::string   path;
    } ;

    typedef std::map<std::string, ServerInfo *> ServerMap;
private:
    ServerMap                 servers;

public:
    MDServerManager();
    virtual ~MDServerManager();
    ServerMap& GetServerMap() { return servers; }
    static MDServerManager* Instance();

};

#endif
