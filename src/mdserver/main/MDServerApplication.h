#ifndef MDSERVER_APPLICATION_H
#define MDSERVER_APPLICATION_H
#include <vector>
#include <string>

class MDServerConnection;

// ****************************************************************************
// Class: MDServerApplication
//
// Purpose:
//   This class manages several MDServerConnections which are connections to
//   programs that drive the MDServer.
//
// Notes:     This class follows the singleton design pattern. 
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 16:44:32 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Mar 15 12:47:22 PDT 2001
//   Added a method to remove dead client connections.
//
//   Brad Whitlock, Fri Apr 18 14:57:17 PST 2003
//   I added a debug method that reads the file list and quits.
//
//   Jeremy Meredith, Wed Dec 31 14:12:12 PST 2003
//   Added code to keep track of the client host.  This lets us guess
//   host names better.
//
// ****************************************************************************

class MDServerApplication
{
    typedef std::vector<MDServerConnection *> MDServerConnectionVector;
public:
    ~MDServerApplication();
    void AddConnection(int *argc, char **argv[]);
    void Execute();
    void ExecuteDebug();
    void SetTimeout(long t) {timeout = t;}

    static MDServerApplication *Instance();
    static void AlarmHandler(int signal);
private:
    MDServerApplication();
    void DisconnectDeadConnections(const std::vector<int> &deadList);
private:
    static MDServerApplication *instance;
    std::string                 clientHost;
    MDServerConnectionVector    clients;
    bool                        keepGoing;
    long                        timeout;
};

#endif
