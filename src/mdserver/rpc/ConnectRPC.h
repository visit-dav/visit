#ifndef CONNECT_RPC_H
#define CONNECT_RPC_H
#include <mdsrpc_exports.h>
#include <VisItRPC.h>

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
//   Brad Whitlock, Mon Jun 16 13:39:48 PST 2003
//   I changed the definition so all fields are encapsulated in a string vector.
//
// ****************************************************************************

class MDSERVER_RPC_API ConnectRPC : public NonBlockingRPC
{
public:
    ConnectRPC();
    virtual ~ConnectRPC();

    // Invokation method
    void operator()(const stringVector &args);

    // Property selection methods
    virtual void SelectAll();

    // Access methods
    const stringVector &GetArguments() const;
private:
    stringVector arguments;
};


#endif
