#ifndef EXPORT_DATABASE_RPC_H
#define EXPORT_DATABASE_RPC_H

#include <engine_rpc_exports.h>

#include <VisItRPC.h>
#include <ExportDBAttributes.h>

// ****************************************************************************
//  Class:  ExportDatabaseRPC
//
//  Purpose:
//    Implements an RPC to export a database.
//
//  Programmer:  Hank Childs 
//  Creation:    May 26, 2005
//
// ****************************************************************************
class ENGINE_RPC_API ExportDatabaseRPC : public BlockingRPC
{
  public:
    ExportDatabaseRPC();
    virtual ~ExportDatabaseRPC();

    // Invocation method
    void operator()(const int, const ExportDBAttributes *);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetID(const int);
    void SetExportDBAtts(const ExportDBAttributes *);

    // Property getting methods
    int                 GetID() const;
    ExportDBAttributes *GetExportDBAtts();


  private:
    int                  id;
    ExportDBAttributes   exportDBAtts; 
};

#endif
