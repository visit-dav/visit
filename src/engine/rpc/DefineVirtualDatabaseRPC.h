#ifndef DEFINE_VIRTUAL_DATABASE_RPC_H
#define DEFINE_VIRTUAL_DATABASE_RPC_H
#include <engine_rpc_exports.h>
#include <VisItRPC.h>
#include <string>
#include <vectortypes.h>

// ****************************************************************************
// Class: DefineVirtualDatabaseRPC
//
// Purpose:
//   Tells the engine to define a virtual database with the specified files.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 25 13:45:41 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class ENGINE_RPC_API DefineVirtualDatabaseRPC : public NonBlockingRPC
{
public:
    DefineVirtualDatabaseRPC();
    virtual ~DefineVirtualDatabaseRPC();

    void operator()(const std::string &wholeName,
                    const std::string &pathToTimesteps,
                    const stringVector &timeSteps, int);

    virtual void SelectAll();

    const std::string  &GetDatabaseName() const { return databaseName; };
    const std::string  &GetDatabasePath() const { return databasePath; };
    const stringVector &GetDatabaseFiles() const { return databaseFiles; };
    int                GetTime() const { return time; };
private:
    std::string  databaseName;
    std::string  databasePath;
    stringVector databaseFiles;
    int          time;
};

#endif
