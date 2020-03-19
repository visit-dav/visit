// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef GET_FILELIST_EXCEPTION_H
#define GET_FILELIST_EXCEPTION_H
#include <mdsrpc_exports.h>
#include <string>
#include <VisItException.h>

// *******************************************************************
// Class: GetFileListException
//
// Purpose:
//   The exception that should be thrown when the file list cannot
//   be gotten.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 25 12:40:50 PDT 2000
//
// Modifications:
//   
// *******************************************************************

class MDSERVER_RPC_API2 GetFileListException : public VisItException
{
public:
    GetFileListException();
    virtual ~GetFileListException() VISIT_THROW_NOTHING {;};
};

#endif
