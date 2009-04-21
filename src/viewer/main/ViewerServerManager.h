/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef VIEWER_SERVER_MANAGER_H
#define VIEWER_SERVER_MANAGER_H
#include <visit-config.h>
#include <viewer_exports.h>
#include <ViewerBase.h>
#include <vectortypes.h>
#include <map>
#include <QSocketNotifier>

#include <HostProfile.h>

class Connection;
class HostProfileList;
class LauncherProxy;
class RemoteProxyBase;
class ViewerConnectionPrinter;
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

    static void SetDebugLevel(int level, bool useBuf);
    static void SetArguments(const stringVector &arg);
    static void SetLocalHost(const std::string &hostName);
    static bool HostIsLocalHost(const std::string &hostName);
    static void ForceSSHTunnelingForAllConnections();

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
    static void GetSSHTunnelOptions(const std::string &host, bool &tunnelSSH);

    static ViewerConnectionProgressDialog *
        SetupConnectionProgressWindow(RemoteProxyBase *component, 
                                      const std::string &host);

    static void OpenWithLauncher(const std::string &host,
                                 const stringVector &args, 
                                 void *data);
    static void SimConnectThroughLauncher(const std::string &host,
                                          const stringVector &args, 
                                          void *data);

    const char *RealHostName(const char *hostName) const;

#if defined(PANTHERHACK)
// Broken on Panther
#else
    static std::map<int,int> GetPortTunnelMap(const std::string &host);
#endif

    static HostProfileList         *clientAtts;

private:
    static void StartLauncher(const std::string &host,
                              const std::string &visitPath,
                              ViewerConnectionProgressDialog *dialog);

    static int                      debugLevel;
    static bool                     bufferDebug;
    static std::string              localHost;
    static stringVector             arguments;
    static LauncherMap              launchers;
    static bool                     sshTunnelingForcedOn;
    static void                    *cbData[2];
};


// ****************************************************************************
// Class: ViewerConnectionPrinter
//
// Purpose:
//   Subclass of QSocketNotifier that we use for printing VCL console output.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 21 15:14:14 PST 2007
//
// Modifications:
//   Brad Whitlock, Tue May 27 14:17:46 PDT 2008
//   Removed name.
//
// ****************************************************************************

class ViewerConnectionPrinter : public QSocketNotifier
{
    Q_OBJECT
public:
    ViewerConnectionPrinter(Connection *);
    virtual ~ViewerConnectionPrinter();
private slots:
    void HandleRead(int);
private:
    Connection *conn;
};

#endif
