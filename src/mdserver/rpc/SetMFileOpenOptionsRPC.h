// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SET_M_FILE_OPEN_OPTIONS_RPC_H
#define SET_M_FILE_OPEN_OPTIONS_RPC_H
#include <mdsrpc_exports.h>

#include <VisItRPC.h>
#include <string>
#include <FileOpenOptions.h>

// ****************************************************************************
// Class: SetMFileOpenOptionsRPC
//
// Purpose:
//   This RPC sets the default file opening options.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   January 18, 2008
//
// Modifications:
//
// ****************************************************************************

class MDSERVER_RPC_API SetMFileOpenOptionsRPC : public BlockingRPC
{
public:
    SetMFileOpenOptionsRPC();
    virtual ~SetMFileOpenOptionsRPC();

    // Invokation method
    void operator()(const FileOpenOptions&);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetFileOpenOptions(const FileOpenOptions&);

    // Property getting methods
    const FileOpenOptions &GetFileOpenOptions() const;

    // Return name of object.
    virtual const std::string TypeName() const;
private:
    FileOpenOptions fileOpenOptions;
};


#endif
