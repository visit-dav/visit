// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef GET_METADATA_RPC_EXECUTOR_H
#define GET_METADATA_RPC_EXECUTOR_H
#include <Observer.h>

class MDServerConnection;

// *******************************************************************
// Class: GetMetaDataRPCExecutor
//
// Purpose:
//   This class handles the RPC request for getting the metadata
//   for a file (which may in the CWD if no path is specified).
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   Brad Whitlock, Fri Nov 17 14:30:36 PST 2000
//   I added the parent_ argument.
//
// *******************************************************************

class GetMetaDataRPCExecutor : public Observer
{
public:
    GetMetaDataRPCExecutor(MDServerConnection *parent_, Subject *s);
    virtual ~GetMetaDataRPCExecutor();

    virtual void Update(Subject *s);
private:
    MDServerConnection *parent;
};

#endif
