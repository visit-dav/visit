#ifndef GET_FILE_LIST_RPC_EXECUTOR_H
#define GET_FILE_LIST_RPC_EXECUTOR_H
#include <Observer.h>

class MDServerConnection;

// ****************************************************************************
// Class: GetFileListRPCExecutor
//
// Purpose:
//   This class handles the RPC request for getting the file list
//   for the current working directory.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 16:01:58 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Nov 7 16:12:46 PST 2000
//   Added the parent argument to the constructor.
//
// ****************************************************************************

class GetFileListRPCExecutor : public Observer
{
public:
    GetFileListRPCExecutor(MDServerConnection *parent_, Subject *s);
    ~GetFileListRPCExecutor();

    virtual void Update(Subject *s);
private:
    MDServerConnection *parent;
};

#endif
