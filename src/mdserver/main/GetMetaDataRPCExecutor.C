#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <GetMetaDataRPCExecutor.h>
#include <GetMetaDataRPC.h>
#include <MDServerConnection.h>
#include <DatabaseException.h>

#include <DebugStream.h>

// ****************************************************************************
// Method: GetMetaDataRPCExecutor::GetMetaDataRPCExecutor
//
// Purpose: 
//   Constructor for the GetMetaDataRPCExecutor class.
//
// Arguments:
//   parent_ : A pointer to the object that created this RPC executor.
//   s       : A pointer to the rpc that invokes this executor.
//
// Programmer: Jeremy Meredith
// Creation:   September 1, 2000
//
// Modifications:
//   
// ****************************************************************************

GetMetaDataRPCExecutor::GetMetaDataRPCExecutor(MDServerConnection *parent_,
    Subject *s) : Observer(s)
{
    parent = parent_;
}

// ****************************************************************************
// Method: GetMetaDataRPCExecutor::~GetMetaDataRPCExecutor
//
// Purpose: 
//   Destructor for the GetMetaDataRPCExecutor class.
//
// Programmer: Jeremy Meredith
// Creation:   September 1, 2000
//
// Modifications:
//   
// ****************************************************************************

GetMetaDataRPCExecutor::~GetMetaDataRPCExecutor()
{
}

// ****************************************************************************
// Method: GetMetaDataRPCExecutor::Update
//
// Purpose: 
//   Performs the work of the RPC executor which, in this case, is to change
//   the return metadata for the specified file.
//
// Arguments:
//   s : A pointer to the GetMetaDataRPC that caused this method to
//       be called.
//
// Programmer: Jeremy Meredith
// Creation:   September 1, 2000
//
// Modifications:
//   Brad Whitlock, Fri Nov 17 17:05:31 PST 2000
//   Modified the routine to use methods in MDServerConnection.
//
//   Brad Whitlock, Thu Feb 7 11:08:07 PDT 2002
//   Modified the routine so it handles exceptions.
//
//   Brad Whitlock, Tue May 13 15:40:03 PST 2003
//   I added timeState.
//
//   Jeremy Meredith, Wed Aug 25 11:40:22 PDT 2004
//   Handle errors through exceptions instead of error codes.   This allows
//   real error messages to make it to the user.
//
//   Brad Whitlock, Wed Feb 9 11:36:17 PDT 2005
//   I made it print the metadata to debug5 all the time.
//
// ****************************************************************************

void
GetMetaDataRPCExecutor::Update(Subject *s)
{
    GetMetaDataRPC *rpc = (GetMetaDataRPC *)s;

    debug2 << "GetMetaDataRPCExecutor::Update - file="<<rpc->GetFile().c_str()<<"\n";
    debug2.flush();

    TRY
    {
        // Either send a successful reply or send an error.
        parent->ReadMetaData(rpc->GetFile(), rpc->GetTimeState());

        debug5 << "MetaData=" << endl;
        if(debug5_real)
            parent->GetCurrentMetaData()->Print(debug5_real);

        rpc->SendReply(parent->GetCurrentMetaData());
    }
    CATCH2(DatabaseException, dbe)
    {
        rpc->SendError(dbe.Message(), dbe.GetExceptionType());
    }
    CATCH2(VisItException, ve)
    {
        rpc->SendError("An unknown error has occurred", ve.GetExceptionType());
    }
    ENDTRY
}
