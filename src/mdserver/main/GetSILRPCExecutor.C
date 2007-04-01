#include <avtDatabase.h>
#include <SILAttributes.h>
#include <GetSILRPCExecutor.h>
#include <GetSILRPC.h>
#include <MDServerConnection.h>
#include <DatabaseException.h>

#include <DebugStream.h>

// ****************************************************************************
// Method: GetSILRPCExecutor::GetSILRPCExecutor
//
// Purpose: 
//   Constructor for the GetSILRPCExecutor class.
//
// Arguments:
//   parent_ : A pointer to the object that created this RPC executor.
//   s       : A pointer to the rpc that invokes this executor.
//
// Programmer: Hank Childs
// Creation:   March 29, 2001
//
// ****************************************************************************

GetSILRPCExecutor::GetSILRPCExecutor(MDServerConnection *parent_,
    Subject *s) : Observer(s)
{
    parent = parent_;
}

// ****************************************************************************
// Method: GetSILRPCExecutor::~GetSILRPCExecutor
//
// Purpose: 
//   Destructor for the GetSILRPCExecutor class.
//
// Programmer: Hank Childs
// Creation:   March 29, 2001
//
// ****************************************************************************

GetSILRPCExecutor::~GetSILRPCExecutor()
{
}

// ****************************************************************************
// Method: GetSILRPCExecutor::Update
//
// Purpose: 
//   Performs the work of the RPC executor which, in this case, is to change
//   the return metadata for the specified file.
//
// Arguments:
//   s : A pointer to the GetSILRPC that caused this method to
//       be called.
//
// Programmer: Hank Childs
// Creation:   March 29, 2001
//
// Modifications:
//   Brad Whitlock, Thu Feb 7 11:07:00 PDT 2002
//   Added code to handle exceptions.
//
//   Brad Whitlock, Tue May 13 15:40:51 PST 2003
//   I added timeState.
//
//   Jeremy Meredith, Wed Aug 25 11:40:22 PDT 2004
//   Handle errors through exceptions instead of error codes.   This allows
//   real error messages to make it to the user.
//
// ****************************************************************************

void
GetSILRPCExecutor::Update(Subject *s)
{
    GetSILRPC *rpc = (GetSILRPC *)s;

    debug2 << "GetSILRPCExecutor::Update - file="<<rpc->GetFile().c_str()<<"\n";
    debug2.flush();

    TRY
    {
        // Either send a successful reply or send an error.
        parent->ReadSIL(rpc->GetFile(), rpc->GetTimeState());
#ifdef DEBUG
        debug2 << "SIL=" << endl;
        parent->GetCurrentSIL()->Print(debug2);
#endif
        rpc->SendReply(parent->GetCurrentSIL());
    }
    CATCH2(DatabaseException, dbe)
    {
        rpc->SendError(dbe.GetMessage(), dbe.GetExceptionType());
    }
    CATCH2(VisItException, ve)
    {
        rpc->SendError("An unknown error has occurred", ve.GetExceptionType());
    }
    ENDTRY
}
