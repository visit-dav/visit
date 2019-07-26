// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SET_E_FILE_OPEN_OPTIONS_RPC_H
#define SET_E_FILE_OPEN_OPTIONS_RPC_H
#include <engine_rpc_exports.h>

#include <VisItRPC.h>
#include <string>
#include <FileOpenOptions.h>

// ****************************************************************************
// Class: SetEFileOpenOptionsRPC
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

class ENGINE_RPC_API SetEFileOpenOptionsRPC : public BlockingRPC
{
public:
    SetEFileOpenOptionsRPC();
    virtual ~SetEFileOpenOptionsRPC();

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
