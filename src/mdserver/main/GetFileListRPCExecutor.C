#include <DebugStream.h>
#include <GetFileListRPCExecutor.h>
#include <GetFileListRPC.h>
#include <MDServerConnection.h>

// ****************************************************************************
// Method: GetFileListRPCExecutor::GetFileListRPCExecutor
//
// Purpose: 
//   Constructor for the GetFileListRPCExecutor class.
//
// Arguments:
//   parent : A pointer to the object that created this executor.
//   s      : A pointer to the RPC that will call this RPC executor.
//
// Returns:    
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2000
//
// Modifications:
//   
// ****************************************************************************

GetFileListRPCExecutor::GetFileListRPCExecutor(MDServerConnection *parent_,
    Subject *s) : Observer(s)
{
    parent = parent_;
}

// ****************************************************************************
// Method: GetFileListRPCExecutor::~GetFileListRPCExecutor
//
// Purpose: 
//   Destructor for the GetFileListRPCExecutor class.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2000
//
// Modifications:
//   
// ****************************************************************************

GetFileListRPCExecutor::~GetFileListRPCExecutor()
{
}

// ****************************************************************************
// Method: GetFileListRPCExecutor::Update
//
// Purpose: 
//   Gets the current file list.
//
// Arguments:
//   s : A pointer to the GetFileListRPC that called this method.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2000
//
// Modifications:
//   Brad Whitlock, Tue Mar 25 11:00:46 PDT 2003
//   I added support for automatic file grouping.
//
// ****************************************************************************

void
GetFileListRPCExecutor::Update(Subject *s)
{
    GetFileListRPC *rpc = (GetFileListRPC *)s;
#ifdef DEBUG
    debug2 << "GetFileListRPCExecutor::Update\n"; debug2.flush();
#endif

    // Either send a successful reply or send an error.
    if(parent->GetReadFileListReturnValue() == 0)
    {
        if(rpc->GetAutomaticFileGrouping())
        {
            // Get the filtered file list.
            GetFileListRPC::FileList files;
            parent->GetFilteredFileList(files, rpc->GetFilter());

#ifdef DEBUG
            debug2 << "FILELIST=" << files << endl;
            debug2.flush();
#endif
            rpc->SendReply(&files);
        }
        else
        {
            GetFileListRPC::FileList *files = parent->GetCurrentFileList();
#ifdef DEBUG
            debug2 << "FILELIST=" << files << endl;
            debug2.flush();
#endif
            rpc->SendReply(files);
        }
    }
    else
        rpc->SendError();
}

