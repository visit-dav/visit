#ifndef CREATEGROUPLIST_RPC_H
#define CREATEGROUPLIST_RPC_H
#include <mdsrpc_exports.h>

#include <VisItRPC.h>
#include <string>

using std::string;
using std::vector;

// ****************************************************************************
// Class: CreateGroupListRPC
//
// Purpose:
//   This class encodes an RPC that tells the MDserver to create a
//   grouplist with the given filename
//
// Notes:      I'm not sure I know what I'm doing.
//
// Programmer: Sean Ahern
// Creation:   Tue Feb 13 14:45:41 PST 2001
//
// Modifications:
//   
// ****************************************************************************

class MDSERVER_RPC_API CreateGroupListRPC : public NonBlockingRPC
{
public:
    CreateGroupListRPC();

    // Invokation method
    void operator()(const string, vector<string>);

    // Property selection methods
    virtual void SelectAll();

    // Access methods
    string filename;
    vector<string> groupList;
};


#endif
