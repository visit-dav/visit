// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//   Hank Childs, Fri Mar  5 11:13:32 PST 2004
//   Added a format as an argument.
//
//   Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007 
//   Added args for createMeshQualityExpressions and 
//   createTimeDerivativeExpresisons. 
//
// ****************************************************************************

class ENGINE_RPC_API OpenDatabaseRPC : public NonBlockingRPC
{
public:
    OpenDatabaseRPC();
    virtual ~OpenDatabaseRPC();

    virtual const std::string TypeName() const { return "OpenDatabaseRPC"; }

    void operator()(const std::string &, const std::string &, int, 
                    bool, bool, bool);

    virtual void SelectAll();

    const std::string &GetFileFormat() const { return fileFormat; };
    const std::string &GetDatabaseName() const { return databaseName; };
    int                GetTime() const { return time; };
    bool               GetCreateMeshQualityExpressions() const 
                           { return createMeshQualityExpressions; };
    bool               GetCreateTimeDerivativeExpressions() const 
                           { return createTimeDerivativeExpressions; };
    bool               GetIgnoreExtents() const
                           { return ignoreExtents; };
private:
    std::string fileFormat;
    std::string databaseName;
    int         time;
    bool        createMeshQualityExpressions;
    bool        createTimeDerivativeExpressions;
    bool        ignoreExtents;
};

#endif
