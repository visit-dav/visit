#ifndef REMOTE_PROCESS_H
#define REMOTE_PROCESS_H
#include <comm_exports.h>
#if defined(_WIN32)
#include <winsock2.h>
#else
#include <netdb.h>
#include <netinet/in.h>
#endif
#include <vectortypes.h>
#include <HostProfile.h>

class Connection;

// ****************************************************************************
// Class: RemoteProcess
//
// Purpose:
//   Starts a process on a remote machine using ssh and connects some
//   sockets to it.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 13 15:47:04 PST 2000
//
// Modifications:
//    Jeremy Meredith, Tue Aug  8 13:49:42 PDT 2000
//    Changed it to allow more than one read/write socket.   
//
//    Brad Whitlock, Fri Aug 25 10:49:10 PDT 2000
//    Changed it so it can launch local processes instead of always
//    having to go through ssh.
//
//    Brad Whitlock, Mon Nov 20 16:12:38 PST 2000
//    Changed private permissions to protected.
//
//    Jeremy Meredith, Fri Apr 27 11:41:05 PDT 2001
//    Added authentication callback function.
//
//    Brad Whitlock, Mon Sep 24 11:26:13 PDT 2001
//    Added GetLocalHostName() method.
//
//    Brad Whitlock, Thu Feb 21 10:02:10 PDT 2002
//    Added GetLocalUserName() method.
//
//    Brad Whitlock, Wed Apr 3 12:01:38 PDT 2002
//    Added WaitForTermination() method.
//
//    Brad Whitlock, Mon Aug 26 12:47:38 PDT 2002
//    I added methods to return the SSH program being used as well as any
//    arguments that it might use.
//
//    Brad Whitlock, Thu Aug 29 17:46:48 PST 2002
//    I added another argument to the authentication callback so we can
//    pass the remote login name.
//
//    Brad Whitlock, Thu Sep 26 16:48:55 PST 2002
//    I added a progress callback and I split AcceptSocket into two methods.
//
//    Brad Whitlock, Mon Dec 16 14:28:02 PST 2002
//    I added securityKey and a method to close the listen socket.
//
//    Brad Whitlock, Mon May 5 13:24:47 PST 2003
//    I refactored the code a little to simplify handling of command lines.
//
//    Jeremy Meredith, Thu Jul  3 14:49:23 PDT 2003
//    Added ability to disable ptys.
//
//    Jeremy Meredith, Thu Oct  9 13:56:56 PDT 2003
//    Added ability to manually specify a client host name or to have it
//    parsed from the SSH_CLIENT (or related) environment variables.  Added
//    ability to specify an SSH port.
//
// ****************************************************************************

class COMM_API RemoteProcess
{
public:
    RemoteProcess(const std::string &rProgram);
    virtual ~RemoteProcess();
    virtual bool Open(const std::string &rHost,
                      HostProfile::ClientHostDetermination chd,
                      const std::string &clientHostName,
                      bool manualSSHPort,
                      int sshPort,
                      int numRead, int numWrite,
                      bool createAsThoughLocal = false);
    void WaitForTermination();
    void AddArgument(const std::string &arg);
    void SetRemoteUserName(const std::string &rUserName);
    const std::string &GetLocalHostName() const;
    const std::string &GetLocalUserName() const;
    Connection *GetReadConnection(int i=0) const;
    Connection *GetWriteConnection(int i=0) const;
    int  GetProcessId() const;
    void SetProgressCallback(bool (*)(void *, int), void *);

    static void SetAuthenticationCallback(void (*)(const char *, const char *, int));    
    static void DisablePTY();
protected:
    bool StartMakingConnection(const std::string &rHost, int numRead,
                               int numWrite);
    void FinishMakingConnection(int numRead, int numWrite);
    const char *SecureShell() const;
    const char *SecureShellArgs() const;
    bool CallProgressCallback(int stage);
    bool HostIsLocal(const std::string &rHost) const;
    void CreateCommandLine(stringVector &args, const std::string &rHost,
                           HostProfile::ClientHostDetermination chd,
                           const std::string &clientHostName,
                           bool manualSSHPort,
                           int sshPort,
                           int numRead, int numWrite, bool local) const;
protected:
    int                      listenPortNum;
    std::string              localHost, localUserName;
    std::string              securityKey;
private:
    bool GetSocketAndPort();
    int  AcceptSocket();
    int  SingleThreadedAcceptSocket();
    int  MultiThreadedAcceptSocket();
    void CloseListenSocket();
    void ExchangeTypeRepresentations();
    void LaunchRemote(const stringVector &args);
    void LaunchLocal(const stringVector &args);
    char **CreateSplitCommandLine(const stringVector &args, int &argc) const;
    void DestroySplitCommandLine(char **args, int argc) const;
    char *StrDup(const std::string &) const;
private:
    DESCRIPTOR               listenSocketNum;
    struct sockaddr_in       sin;
    std::string              remoteHost, remoteProgram, remoteUserName;
    std::vector<std::string> argList;
    int                      remoteProgramPid;
    Connection             **readConnections, **writeConnections;
    int                      nReadConnections, nWriteConnections;
    bool                   (*progressCallback)(void *, int);
    void                    *progressCallbackData;

    static void            (*getAuthentication)(const char *, const char *, int);
    static bool              disablePTY;
};

#endif
