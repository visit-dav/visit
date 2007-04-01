#ifndef CLOSE_DATABASE_RPC_H
#define CLOSE_DATABASE_RPC_H
#include <mdsrpc_exports.h>
#include <VisItRPC.h>

// ****************************************************************************
// Class: CloseDatabaseRPC
//
// Purpose:
//   This RPC closes the active database.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 10:38:56 PDT 2002
//
// Modifications:
//   Brad Whitlock, Fri Feb 4 08:24:38 PDT 2005
//   Added a database argument.
//
// ****************************************************************************

class MDSERVER_RPC_API CloseDatabaseRPC : public BlockingRPC
{
public:
    CloseDatabaseRPC();
    virtual ~CloseDatabaseRPC();

    // Invocation method
    void operator()(const std::string &db);

    virtual void SelectAll();

    void SetDatabase(const std::string &db);
    const std::string &GetDatabase() { return database; };
private:
    std::string database;
};

#endif
