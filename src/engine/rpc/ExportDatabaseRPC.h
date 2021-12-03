// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//  Modifications:
//    Brad Whitlock, Fri Jan 24 16:37:14 PST 2014
//    Allow more than one network.
//    Work partially supported by DOE Grant SC0007548.
//
//    Kathleen Biagas, Fri Apr 23 2021
//    Added return atts, Pass pointer to atts instead of reference.
//
// ****************************************************************************

class ENGINE_RPC_API ExportDatabaseRPC : public BlockingRPC
{
  public:
    ExportDatabaseRPC();
    virtual ~ExportDatabaseRPC();

    virtual const std::string TypeName() const { return "ExportDatabaseRPC"; }

    // Invocation method
    void operator()(const intVector &, const ExportDBAttributes *, const std::string &);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetIDs(const intVector &ids);
    void SetExportDBAtts(const ExportDBAttributes *);
    void SetTimeSuffix(const std::string &);

    // Property getting methods
    const intVector          &GetIDs() const;
    ExportDBAttributes       *GetExportDBAtts();
    const std::string        &GetTimeSuffix() const;

    ExportDBAttributes GetReturnAtts() const { return returnAtts; }


  private:
    intVector            ids;
    ExportDBAttributes   exportDBAtts; 
    std::string          timeSuffix;
    ExportDBAttributes   returnAtts;
};

#endif
