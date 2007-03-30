#ifndef OPEN_DATABASE_RPC_H
#define OPEN_DATABASE_RPC_H
#include <engine_rpc_exports.h>
#include <VisItRPC.h>
#include <string>

// ****************************************************************************
// Class: OpenDatabaseRPC
//
// Purpose:
//   Tells the engine to open a database.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 10 13:51:36 PST 2002
//
// Modifications:
//   
// ****************************************************************************

class ENGINE_RPC_API OpenDatabaseRPC : public NonBlockingRPC
{
public:
    OpenDatabaseRPC();
    virtual ~OpenDatabaseRPC();

    void operator()(const std::string &, int);

    virtual void SelectAll();

    const std::string &GetDatabaseName() const { return databaseName; };
    int                GetTime() const { return time; };
private:
    std::string databaseName;
    int         time;
};

#endif
