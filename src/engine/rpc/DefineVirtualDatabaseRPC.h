// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//   Hank Childs, Fri Mar  5 17:27:41 PST 2004
//   Added file format.
//
//   Kathleen Bonnell, Wed Oct 10 08:18:49 PDT 2007 
//   Added createMeshQualityExpressions and createTimeDerivativeExpressions.
//
// ****************************************************************************

class ENGINE_RPC_API DefineVirtualDatabaseRPC : public NonBlockingRPC
{
public:
    DefineVirtualDatabaseRPC();
    virtual ~DefineVirtualDatabaseRPC();

    virtual const std::string TypeName() const { return "DefineVirtualDatabaseRPC"; }

    void operator()(const std::string &fileFormat,
                    const std::string &wholeName,
                    const std::string &pathToTimesteps,
                    const stringVector &timeSteps, int,
                    bool, bool);

    virtual void SelectAll();

    const std::string  &GetFileFormat() const { return fileFormat; };
    const std::string  &GetDatabaseName() const { return databaseName; };
    const std::string  &GetDatabasePath() const { return databasePath; };
    const stringVector &GetDatabaseFiles() const { return databaseFiles; };
    int                GetTime() const { return time; };
    bool               GetCreateMeshQualityExpressions() const 
                           { return createMeshQualityExpressions; };
    bool               GetCreateTimeDerivativeExpressions() const 
                           { return createTimeDerivativeExpressions; };
private:
    std::string  fileFormat;
    std::string  databaseName;
    std::string  databasePath;
    stringVector databaseFiles;
    int          time;
    bool         createMeshQualityExpressions;
    bool         createTimeDerivativeExpressions;
};

#endif
