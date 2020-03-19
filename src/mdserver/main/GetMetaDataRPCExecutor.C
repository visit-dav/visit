// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <GetMetaDataRPCExecutor.h>
#include <GetMetaDataRPC.h>
#include <MDServerConnection.h>
#include <DatabaseException.h>

#include <DebugStream.h>
#include <TimingsManager.h>


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
//   Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//   Added call to GetForceReadAllCyclesAndTimes
//
//   Jeremy Meredith, Mon Aug 28 16:48:30 EDT 2006
//   Added ability to force using a specific plugin when reading
//   the metadata from a file (if it causes the file to be opened).
//
//   Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//   Added support to treat all databases as time varying
//
//   Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007
//   Send flags from rpc for controlling creation of MeshQuality and 
//   TimeDerivative expressions to the ReadMetaData call.
//
//   Cyrus Harrison, Wed Nov 28 11:17:23 PST 2007
//   Added flag for auto vector magnitude expression creation 
//
//   Hank Childs, Wed Dec 19 08:39:46 PST 2007
//   Add timing information.
//
//   Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//   Changed interface to DebugStream to obtain current debug level.
// ****************************************************************************

void
GetMetaDataRPCExecutor::Update(Subject *s)
{
    GetMetaDataRPC *rpc = (GetMetaDataRPC *)s;

    debug2 << "GetMetaDataRPCExecutor::Update - file="
           << rpc->GetFile().c_str() << " type="
           << rpc->GetForcedFileType() << "\n" << std::flush;

    TRY
    {
        // Either send a successful reply or send an error.
        parent->ReadMetaData(rpc->GetFile(), rpc->GetTimeState(),
                             rpc->GetForceReadAllCyclesAndTimes(),
                             rpc->GetForcedFileType(),
                             rpc->GetTreatAllDBsAsTimeVarying(),
                             rpc->GetCreateMeshQualityExpressions(),
                             rpc->GetCreateTimeDerivativeExpressions(),
                             rpc->GetCreateVectorMagnitudeExpressions());

        debug5 << "MetaData=" << endl;
        if(DebugStream::Level5())
            parent->GetCurrentMetaData()->Print(DebugStream::Stream5());

        int t0 = visitTimer->StartTimer();
        rpc->SendReply(parent->GetCurrentMetaData());
        visitTimer->StopTimer(t0, "Sending MD");
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
