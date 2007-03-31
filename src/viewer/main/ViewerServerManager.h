#ifndef VIEWER_SERVER_MANAGER_H
#define VIEWER_SERVER_MANAGER_H
#include <viewer_exports.h>
#include <vectortypes.h>
#include <map>

#include <HostProfile.h>

class HostProfileList;
class LauncherProxy;
class RemoteProxyBase;
class ViewerConnectionProgressDialog;

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
// ****************************************************************************

class VIEWER_API ViewerServerManager
{
    typedef std::map<std::string, LauncherProxy *> LauncherMap;
public:
    ViewerServerManager();
    virtual ~ViewerServerManager();

    static void CloseLaunchers();

    static void SetDebugLevel(int level);
    static void SetArguments(const stringVector &arg);
    static void SetLocalHost(const std::string &hostName);
    static bool HostIsLocalHost(const std::string &hostName);

    static HostProfileList *GetClientAtts();
protected:
    static bool ShouldShareBatchJob(const std::string &host);
    static void AddArguments(RemoteProxyBase *component,
                      const stringVector &args);
    static void AddProfileArguments(RemoteProxyBase *component,
                      const std::string &host);

    static void GetClientMachineNameOptions(const std::string &host,
                                     HostProfile::ClientHostDetermination &chd,
                                     std::string &clientHostName);
    static void GetSSHPortOptions(const std::string &host,
                                  bool &manualSSHPort, int &sshPort);

    static ViewerConnectionProgressDialog *
        SetupConnectionProgressWindow(RemoteProxyBase *component, 
                                      const std::string &host);

    static void OpenWithLauncher(const std::string &host,
                                 const stringVector &args, 
                                 void *data);

    const char *RealHostName(const char *hostName) const;

    static HostProfileList         *clientAtts;
private:
    static void StartLauncher(const std::string &host,
                              const std::string &visitPath,
                              ViewerConnectionProgressDialog *dialog);

    static int                      debugLevel;
    static std::string              localHost;
    static stringVector             arguments;
    static LauncherMap              launchers;
    static void                    *cbData[2];
};

#endif
