// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              MDServerProxy.h                              //
// ************************************************************************* //

#ifndef _MDSERVERPROXY_H_
#define _MDSERVERPROXY_H_
#include <mdsproxy_exports.h>

#include <RemoteProxyBase.h>

#include <string>
#include <vector>
#include <maptypes.h>
#include <vectortypes.h>
#include <MDServerState.h>
#include <MDServerMethods.h>
class     DBPluginInfoAttributes;


// ****************************************************************************
//  Class: MDServerProxy
//
//  Purpose:
//      MDServerProxy is a proxy class for creating and controlling a viewer.
//
//  Note:
//
//  Programmer: Jeremy Meredith
//  Creation:   August 18, 2000
//
//  Modifications:
//    Brad Whitlock, Mon Aug 21 14:51:36 PST 2000
//    I added a cc, assignment operator to FileList.
//
//    Jeremy Meredith, Fri Sep  1 17:17:12 PDT 2000
//    Added GetMetaData() and its associated objects (RPC, data).
//
//    Eric Brugger, Wed Oct 25 15:30:02 PDT 2000
//    I removed the argument "prog" from the Create method.
//
//    Brad Whitlock, Thu Nov 16 16:21:43 PST 2000
//    I added code that allows it to connect to a process that did not
//    launch it.
//
//    Sean Ahern, Wed Feb 28 14:34:26 PST 2001
//    Added the CreateGroupListRPC.
//
//    Hank Childs, Thu Mar 29 16:49:59 PST 2001
//    Added GetSIL.
//
//    Brad Whitlock, Tue Feb 12 13:56:43 PST 2002
//    Added ExpandPath.
//
//    Brad Whitlock, Mon Mar 25 16:17:24 PST 2002
//    Took away some methods.
//
//    Brad Whitlock, Fri May 3 17:22:54 PST 2002
//    Added a method for setting the remote user name.
//
//    Brad Whitlock, Tue Jul 30 10:52:33 PDT 2002
//    I added a method to close the active database.
//
//    Brad Whitlock, Mon Aug 26 15:53:27 PST 2002
//    I added methods to get the file separator.
//
//    Brad Whitlock, Fri Sep 27 15:34:11 PST 2002
//    I added methods to support a launch progress window.
//
//    Brad Whitlock, Thu Dec 26 16:17:27 PST 2002
//    I added support for security keys.
//
//    Brad Whitlock, Mon Mar 24 11:55:35 PDT 2003
//    I made VisIt capable of automatically detecting time-varying databases.
//
//    Brad Whitlock, Tue Apr 22 13:50:40 PST 2003
//    I fixed a crash on Windows.
//
//    Brad Whitlock, Fri May 2 12:14:28 PDT 2003
//    I made it inherit from RemoteProxyBase.
//
//    Brad Whitlock, Tue May 13 15:29:50 PST 2003
//    I made it possible to get metadata for a certain timestate.
//
//    Brad Whitlock, Mon Jun 16 13:45:31 PST 2003
//    I changed the Connect method so it only has a single argument.
//
//    Hank Childs, Thu Jan 22 21:02:56 PST 2004
//    Added LoadPluginsRPC.
//
//    Brad Whitlock, Thu Jul 29 12:26:38 PDT 2004
//    Added another argument to GetFileList.
//
//    Jeremy Meredith, Tue Feb  8 08:52:45 PST 2005
//    Added the ability to query for errors detected during plugin
//    initialization.
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added bool forceReadAllCyclesAndTimes to GetMetaData
//
//    Hank Childs, Mon May 23 13:35:56 PDT 2005
//    Added ability to get DB plugin info.
//
//    Jeremy Meredith, Mon Aug 28 16:48:30 EDT 2006
//    Added ability to force using a specific plugin when reading
//    the metadata from a file (if it causes the file to be opened).
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added support to treat all databases as time varying
//
//    Mark C. Miller, Wed Aug 22 20:16:59 PDT 2007
//    Added treatAllDBsAsTimeVarying to GetSIL
//
//    Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007
//    Added flags to GetMetaData for controlling creation of MeshQuality 
//    and TimeDerivative expressions.
//
//    Cyrus Harrison, Wed Nov 28 14:33:34 PST 2007
//    Added flag to GetMetaData for controlling auto creation of vector
//    magnitude expressions.
//
//    Jeremy Meredith, Wed Jan 23 16:10:37 EST 2008
//    Added ability to notify mdservers of the file opening options for the
//    database reader plugins.
//
// ****************************************************************************

class MDSERVER_PROXY_API MDServerProxy : public RemoteProxyBase
{
public:
    MDServerProxy();
    virtual ~MDServerProxy();

    virtual std::string GetComponentName() const;

    MDServerMethods    *GetMDServerMethods() { return methods; }
    MDServerState      *GetMDServerState() { return state; }

protected:
    virtual void        SetupComponentRPCs();

private:

    MDServerState   *state;
    MDServerMethods *methods;
};

#endif
