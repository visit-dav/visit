// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_SERVER_MANAGER_H
#define VIEWER_SERVER_MANAGER_H
#include <visit-config.h>
#include <viewer_exports.h>
#include <ViewerBase.h>
#include <vectortypes.h>
#include <map>

#include <MachineProfile.h>

class Connection;
class LauncherProxy;
class RemoteProxyBase;
class ViewerConnectionPrinter;
class ViewerConnectionProgress;

// ****************************************************************************
// Class: ViewerServerManager
//
// Purpose:
//   This is a base class for the mdserver and engine managers.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri May 3 16:04:21 PST 2002
//
// Modifications:
//    Jeremy Meredith, Thu Jun 26 10:50:36 PDT 2003
//    Added ShouldShareBatchJob function.
//
//    Jeremy Meredith, Thu Oct  9 13:41:32 PDT 2003
//    Added client host name determination options.  Added ssh port options.
//
//    Brad Whitlock, Fri Mar 12 11:55:32 PDT 2004
//    I added SendKeepAlivesToLaunchers.
//
//    Jeremy Meredith, Tue Mar 30 10:51:33 PST 2004
//    Added support for simulations.
//
//    Brad Whitlock, Mon Feb 12 17:46:53 PST 2007
//    Added ViewerBase base class.
//
//    Jeremy Meredith, Tue May 22 13:00:38 EDT 2007
//    Added SSH tunneling option.
//
//    Thomas R. Treadway, Mon Oct  8 13:27:42 PDT 2007
//    Backing out SSH tunneling on Panther (MacOS X 10.3)
//
//    Brad Whitlock, Wed Nov 21 14:32:31 PST 2007
//    Added support for printing out remote process console output.
//
//    Jeremy Meredith, Wed Dec  3 16:48:35 EST 2008
//    Allowed commandline override forcing-on of SSH tunneling.
//
//    Mark C. Miller, Tue Apr 21 14:24:18 PDT 2009
//    Added bool to SetDebugLevel to control buffering of debug logs. 
//
//    Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//    Split HostProfile int MachineProfile and LaunchProfile.
//
//    Eric Brugger, Mon May  2 17:10:45 PDT 2011
//    I added the ability to use a gateway machine when connecting to a
//    remote host.
//
// ****************************************************************************

class VIEWER_API ViewerServerManager : public ViewerBase
{
    struct LauncherData
    {
        LauncherProxy           *launcher;
        ViewerConnectionPrinter *notifier;
    };

    typedef std::map<std::string, LauncherData> LauncherMap;
public:
    ViewerServerManager();
    virtual ~ViewerServerManager();

    static void CloseLaunchers();
    static void SendKeepAlivesToLaunchers();

    static void SetArguments(const stringVector &arg);
    static void SetLocalHost(const std::string &hostName);
    static bool HostIsLocalHost(const std::string &hostName);

    static void SetLaunchProgressCallback(bool (*cb)(void *, int),
                                          void *cbdata);
    static void SetOpenWithEngineCallback(void (*cb)(const std::string &, 
                                                     const stringVector &,
                                                     void *),
                                          void *cbdata);
protected:
    ViewerConnectionProgress *CreateConnectionProgress(const std::string &host);
    void SetupConnectionProgress(RemoteProxyBase *component,
                                 ViewerConnectionProgress *progress);

    MachineProfile GetMachineProfile(const std::string &host) const;

    static void AddArguments(RemoteProxyBase *component,
                      const stringVector &args);

    static void OpenWithLauncher(const std::string &host,
                                 const stringVector &args, 
                                 void *data);
    static void SimConnectThroughLauncher(const std::string &host,
                                          const stringVector &args, 
                                          void *data);

    const char *RealHostName(const char *hostName) const;

    static std::map<int,int> GetPortTunnelMap(const std::string &host);

    // Callback function when we need to launch a component using engine.
    static void (*OpenWithEngineCB)(const std::string &, 
                                    const stringVector &args,
                                    void *);
    static void *OpenWithEngineCBData;
    // Callback function for launch progress.
    static bool (*LaunchProgressCB)(void *, int);
    static void *LaunchProgressCBData;

    static void                    *cbData[2];
private:
    static void StartLauncher(const std::string &host,
                              const std::string &visitPath,
                              ViewerConnectionProgress *progress);

    static std::string              localHost;
    static stringVector             arguments;
    static LauncherMap              launchers;
};

#endif
