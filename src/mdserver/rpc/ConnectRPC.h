#ifndef CONNECT_RPC_H
#define CONNECT_RPC_H
#include <mdsrpc_exports.h>
#include <VisItRPC.h>
#include <string>

// ****************************************************************************
// Class: ConnectRPC
//
// Purpose:
//   This class encodes an RPC that tells the MDserver to connect to another
//   process.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:49:14 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Dec 26 15:58:30 PST 2002
//   I added a security key feature.
//
// ****************************************************************************

class MDSERVER_RPC_API ConnectRPC : public NonBlockingRPC
{
public:
    ConnectRPC();
    virtual ~ConnectRPC();

    // Invokation method
    void operator()(const std::string &host, const std::string &key,
                    int port, int nread, int nwrite);

    // Property selection methods
    virtual void SelectAll();

    // Access methods
    const std::string &GetHost() const;
    const std::string &GetKey() const;
    int GetPort() const;
    int GetNRead() const;
    int GetNWrite() const;
private:
    std::string connectHost;
    std::string connectKey;
    int         connectPort;
    int         connectNRead;
    int         connectNWrite;
};


#endif
