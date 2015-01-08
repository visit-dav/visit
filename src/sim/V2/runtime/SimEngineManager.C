/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/
#include <SimEngineManager.h>

#include <CompactSILRestrictionAttributes.h>
#include <EngineList.h>
#include <ExpressionList.h>
#include <GlobalAttributes.h>
#include <MaterialAttributes.h>
#include <MeshManagementAttributes.h>
#include <ParsingExprList.h>
#include <PickAttributes.h>
#include <ProcessAttributes.h>
#include <QueryAttributes.h>
#include <QueryOverTimeAttributes.h>
#include <SelectionList.h>
#include <SelectionProperties.h>
#include <WindowAttributes.h>

#include <ViewerState.h>

#include <PlotPluginManager.h>
#include <OperatorPluginManager.h>
#include <DatabasePluginManager.h>
#include <AbortException.h>
#include <TimingsManager.h>

#include <SimEngine.h>
#include <NetworkManager.h>

#include <avtCallback.h>
#include <avtDatabaseFactory.h>
#include <avtDataObjectString.h>
#include <avtParallel.h>
#include <avtNamedSelectionManager.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

// ****************************************************************************
// Method: SimEngineManager::SimEngineManager
//
// Purpose:
//   Constructor.
//
// Arguments:
//   e : The engine we'll use.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:06:28 PDT 2014
//
// Modifications:
//
// ****************************************************************************

SimEngineManager::SimEngineManager(SimEngine *e) : ViewerEngineManagerInterface(),
    engineKey(), engine(e)
{
}
// ****************************************************************************
// Method: SimEngineManager::SimEngineManager
//
// Purpose:
//   Constructor.
//
// Arguments:
//   e : The engine we'll use.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:06:28 PDT 2014
//
// Modifications:
//
// ****************************************************************************

SimEngineManager::~SimEngineManager()
{
}

// ****************************************************************************
// Method: SimEngineManager::CreateEngine
//
// Purpose:
//   Create an engine.
//
// Arguments:
//   
//
// Note: We're in situ so we don't need to create an engine.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:06:28 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::CreateEngine(const EngineKey &ek,
                      const stringVector &arguments,
                      bool  skipChooser,
                      int   numRestarts,
                      bool  reverseLaunch)
{
    // Save the engine key.
    engineKey = ek;

    // Does nothing
    return true;
}

// ****************************************************************************
// Method: SimEngineManager::CreateEngineEx
//
// Purpose:
//   Create an engine.
//
// Arguments:
//   
//
// Note: We're in situ so we don't need to create an engine.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:06:28 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::CreateEngineEx(const EngineKey &ek,
                        const stringVector &arguments,
                        bool  skipChooser,
                        int   numRestarts,
                        bool  reverseLaunch,
                        ViewerConnectionProgress *progress)
{
    // Save the engine key.
    engineKey = ek;

    // Does nothing
    return true;
}

void
SimEngineManager::CloseEngines()
{
    // Does nothing
}

void
SimEngineManager::CloseEngine(const EngineKey &/*ek*/)
{
    // Does nothing
}

void
SimEngineManager::InterruptEngine(const EngineKey &/*ek*/)
{
    // Does nothing.
}

void
SimEngineManager::SendKeepAlives()
{
    // Does nothing
}

bool
SimEngineManager::EngineExists(const EngineKey &/*ek*/)
{
    return true;
}
// ****************************************************************************
// Method: SimEngineManager::ClearCacheForAllEngines
//
// Purpose:
//   Clear the cache for all networks.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:25:46 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
SimEngineManager::ClearCacheForAllEngines()
{
    engine->GetNetMgr()->ClearAllNetworks();
}

// ****************************************************************************
// Method: SimEngineManager::UpdateEngineList
//
// Purpose:
//   Add the in situ engine to the engine list.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:26:08 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
SimEngineManager::UpdateEngineList()
{
    EngineList newEL;
    EngineProperties props(engine->GetEngineProperties());

    newEL.GetEngineName().push_back(engineKey.HostName());
    newEL.GetSimulationName().push_back(engineKey.SimName());
    newEL.AddProperties(props);

    // Send the engine list to the viewer's client.
    *(GetViewerState()->GetEngineList()) = newEL;
    GetViewerState()->GetEngineList()->Notify();
}

// ****************************************************************************
// Method: SimEngineManager::SendSimulationCommand
//
// Purpose:
//   Send a command that libsim wants to process to the engine object.
//
// Arguments:
//   ek       : The engine key.
//   command  : The command to execute.
//   argument : Encoded command arguments
//
// Returns:    
//
// Note:       The SimEngine does some "viewer-ey" things with the command.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:24:17 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
SimEngineManager::SendSimulationCommand(const EngineKey &/*ek*/,
                                        const std::string &command,
                                        const std::string &argument)
{
    engine->ExecuteSimulationCommand(command, argument);
}

//
// These methods are not used in batch mode since the "viewer" is integrated
// with libsim.
//

bool
SimEngineManager::ConnectSim(const EngineKey &ek,
                             const stringVector &arguments,
                             const std::string &simHost,
                             int simPort,
                             const std::string &simSecurityKey)
{
    // Save the engine key.
    engineKey = ek;

    return true;
}

int
SimEngineManager::GetWriteSocket(const EngineKey &/*ek*/)
{
    return -1;
}

void
SimEngineManager::ReadDataAndProcess(const EngineKey &/*ek*/)
{
    // Does nothing.
}

avtDatabaseMetaData *
SimEngineManager::GetSimulationMetaData(const EngineKey &/*ek*/)
{
    return NULL;
}

SILAttributes *
SimEngineManager::GetSimulationSILAtts(const EngineKey &/*ek*/)
{
    return NULL;
}

SimulationCommand *
SimEngineManager::GetCommandFromSimulation(const EngineKey &/*ek*/)
{
    return NULL;
}

// ****************************************************************************
// Method: SimEngineManager::OpenDatabase
//
// Purpose:
//   Called when the engine needs to open a database.
//
// Arguments:
//   
//
// Returns:    True on success; False on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:08:18 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::OpenDatabase(const EngineKey &/*ek*/, 
                               const std::string &format, 
                               const std::string &filename,
                               int timeState)
{
    return engine->OpenDatabase();
}

// ****************************************************************************
// Method: SimEngineManager::DefineVirtualDatabase
//
// Purpose:
//   Called when the engine needs to define a virtual database.
//
// Arguments:
//   
//
// Returns:    True on success; False on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:08:18 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::DefineVirtualDatabase(const EngineKey &/*ek*/,
                                        const std::string &format,
                                        const std::string &dbName,
                                        const std::string &path, 
                                        const stringVector &files,
                                        int time)
{
// NOTE: This is pretty much the same as Executors.h. We could make this a proper
//       method of NetworkManager to reduce code duplication.

    engine->GetNetMgr()->GetDatabasePluginManager()->PluginAvailable(format);

    avtDatabaseFactory::SetCreateMeshQualityExpressions(GetViewerState()->
        GetGlobalAttributes()->GetCreateMeshQualityExpressions());
    avtDatabaseFactory::SetCreateTimeDerivativeExpressions(GetViewerState()->
        GetGlobalAttributes()->GetCreateTimeDerivativeExpressions());

    engine->GetNetMgr()->DefineDB(dbName, path, files, time, format);
    return true;
}

// ****************************************************************************
// Method: SimEngineManager::ReadDataObject
//
// Purpose:
//   Called when the engine needs to read a data object to start a network.
//
// Arguments:
//   
//
// Returns:    True on success; False on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:08:18 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::ReadDataObject(const EngineKey &/*ek*/,
    const std::string &format,
    const std::string &filename,
    const std::string &var, const int ts,
    avtSILRestriction_p silr,
    const MaterialAttributes &matopts,
    const ExpressionList &expressions,
    const MeshManagementAttributes &meshopts,
    bool treatAllDbsAsTimeVarying,
    bool ignoreExtents,
    const std::string &selName,
    int windowID)
{
    engine->GetNetMgr()->GetDatabasePluginManager()->PluginAvailable(format);

    // Need this??
    *ParsingExprList::Instance()->GetList() = expressions;

    CompactSILRestrictionAttributes *cSilAtts = silr->MakeCompactAttributes();
    engine->GetNetMgr()->StartNetwork(format,
                                      filename, 
                                      var,
                                      ts,
                                      *cSilAtts,
                                      matopts,
                                      meshopts,
                                      treatAllDbsAsTimeVarying,
                                      ignoreExtents,
                                      selName,
                                      windowID);

    delete cSilAtts;
    return true;
}

// ****************************************************************************
// Method: SimEngineManager::ApplyOperator
//
// Purpose:
//   Called when the engine needs to add an operator to the pipeline.
//
// Arguments:
//   
//
// Returns:    True on success; False on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:08:18 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::ApplyOperator(const EngineKey &/*ek*/,
                                const std::string &name,
                                const AttributeSubject *atts)
{
    engine->GetNetMgr()->AddFilter(name, atts);
    return true;
}

// ****************************************************************************
// Method: SimEngineManager::MakePlot
//
// Purpose:
//   Called when the engine needs to add a plot to terminate the pipeline.
//
// Arguments:
//   
//
// Returns:    True on success; False on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:08:18 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::MakePlot(const EngineKey &/*ek*/,
                           const std::string &plotName,
                           const std::string &pluginID,
                           const AttributeSubject *atts,
                           const std::vector<double> &ext,
                           int winID, int *networkId)
{
    if (!engine->GetNetMgr()->GetPlotPluginManager()->PluginAvailable(pluginID))
    {
        engine->GetNetMgr()->CancelNetwork();
        return false;
    }

    engine->GetNetMgr()->MakePlot(plotName, pluginID, atts, ext);

    *networkId = engine->GetNetMgr()->EndNetwork(winID);

    return true;
}

// ****************************************************************************
// Method: SimEngineManager::UpdatePlotAttributes
//
// Purpose:
//   Called when the engine needs to set the plot attributes.
//
// Arguments:
//   
//
// Returns:    True on success; False on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:08:18 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::UpdatePlotAttributes(const EngineKey &/*ek*/,
                                       const std::string &pluginID,
                                       int id, const AttributeSubject *atts)
{
    // Prepare
    if (!engine->GetNetMgr()->GetPlotPluginManager()->PluginAvailable(pluginID))
    {
        return false;
    }

    // Update
    engine->GetNetMgr()->UpdatePlotAtts(id, atts);

    return true;
}

// ****************************************************************************
// Method: SimEngineManager::UpdatePlotAttributes
//
// Purpose:
//   Called when the engine needs to activate a particular network.
//
// Arguments:
//   
//
// Returns:    True on success; False on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:08:18 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::UseNetwork(const EngineKey &/*ek*/, int networkId)
{
    engine->GetNetMgr()->UseNetwork(networkId);
    return true;
}


// For now, we force the consolidated data from rank 0 to be sent to all ranks.
// This is bad but works for now. Maybe we can get away with a dummy dataset 
// instead down the road.
#define TEMPORARILY_FORCE_SAME_DATA_IN_VIEWER

// ****************************************************************************
// Method: SimEngineManager::WriteCallback
//
// Purpose:
//   Rank 0 write callback.
//
// Arguments:
//   do_str : The data object string we're writing.
//   cbdata : The callback data (in this case, a buffer to store the contents
//            of the data object string before it goes out of scope).
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 21:44:01 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
SimEngineManager::WriteCallback(avtDataObjectString &do_str, void *cbdata)
{
    DOString *d = (DOString *)cbdata;

    // Get the string as a whole from the do_str.
    d->buffer = do_str.GetWholeString(d->size);
}

// ****************************************************************************
// Method: SimEngineManager::ReadDataObjectString
//
// Purpose:
//   This method reads the DOString that we harvested from GatherData into the
//   data object reader.
//
// Arguments:
//   rdr : The data object reader.
//   dos : The data object string buffer.
//
// Returns:    
//
// Note:       We're temporarily broadcasting data object results from rank 0
//             to other ranks so they are happy. We can probably figure a way
//             around this.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 21:45:43 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
SimEngineManager::ReadDataObjectString(avtDataObjectReader_p rdr, 
    SimEngineManager::DOString &dos)
{
#if defined(PARALLEL) && defined(TEMPORARILY_FORCE_SAME_DATA_IN_VIEWER)
    BroadcastInt(dos.size);
    if(!PAR_UIProcess())
        dos.buffer = new char[dos.size];
    MPI_Bcast(dos.buffer, dos.size, MPI_CHAR, 0, VISIT_MPI_COMM);
#endif

    rdr->Read(dos.size, dos.buffer);
}

// ****************************************************************************
// Method: SimEngineManager::Execute
//
// Purpose:
//   Executes the network in the network manager, returns the dataset encoded
//   in a data object reader.
//
// Arguments:
//   ek : An engine key
//   rdr : A reference to the data object reader we're filling in based on
//         the results of the execution.
//   replyWithNullData : Whether to reply with null data.
//   waitCB            : A callback to execute while the engine executes.
//   waitCBData        : Data for the wait callback.
//
// Returns:    True on success, false on failure.
//
// Note:       The write/read using the data object reader is unnecessary in
//             the long run if we can streamline the Viewer/Engine interface
//             for exchanging data somewhat. This code is really similar to
//             the code in Executors.h, except we don't write to a socket.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 15 14:59:01 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::Execute(const EngineKey &/*ek*/, avtDataObjectReader_p &rdr,
                          bool replyWithNullData,
                          void (*waitCB)(void*), void *waitCBData)
{
    bool success = false;

    TRY
    {
        // save the current network id for later
        float cellCountMultiplier;
        int netId = engine->GetNetMgr()->GetCurrentNetworkId();
        int winId = engine->GetNetMgr()->GetCurrentWindowId();
        avtNullData abortDob(NULL);

        // Get the output of the network manager. This does the job of
        // executing the network.
        avtDataObjectWriter_p writer;
        int gettingData = visitTimer->StartTimer();
        TRY
        {
            writer = engine->GetNetMgr()->GetOutput(replyWithNullData, false,
                                                    &cellCountMultiplier);
        }
        CATCH(AbortException)
        {
            // make a dummy dataobject writer for the call to WriteData
            abortDob.SetWriterShouldMergeParallelStreams();
            writer = abortDob.InstantiateWriter();
        }
        ENDTRY
        visitTimer->StopTimer(gettingData, "Executing network");

        int writingData = visitTimer->StartTimer();
        // set params influencing scalable rendering 
        int scalableThreshold = engine->GetNetMgr()->GetScalableThreshold(winId);
        int currentTotalGlobalCellCount = engine->GetNetMgr()->GetTotalGlobalCellCounts(winId);
        int currentNetworkGlobalCellCount = 0;
        bool scalableThresholdExceeded = false;
        bool useCompression = engine->GetNetMgr()->GetShouldUseCompression(winId);

        // Gather the data for the viewer and call the write callback.
        rdr = new avtDataObjectReader;
        std::string errorMessage;
        TRY
        {
            DOString dos;
            success = engine->GatherData(writer,
                                     useCompression,
                                     replyWithNullData,
                                     scalableThreshold, 
                                     currentTotalGlobalCellCount, cellCountMultiplier,
                                     NULL, NULL,
                                     WriteCallback, (void*)&dos,
                                     errorMessage,
                                     &scalableThresholdExceeded,
                                     &currentNetworkGlobalCellCount);

            // Read the data object string that we harvested in WriteCallback.
            ReadDataObjectString(rdr, dos);
        }
        CATCH(VisItException)
        {
            visitTimer->StopTimer(writingData, "Writing data");
            RETHROW;
        }
        ENDTRY

        // re-set the network if we exceeded the scalable threshold
        if (scalableThresholdExceeded && !replyWithNullData)
            engine->GetNetMgr()->UseNetwork(netId);

        // only update cell count if we're not here asking for null data
        if (!replyWithNullData)
            engine->GetNetMgr()->SetGlobalCellCount(netId, currentNetworkGlobalCellCount);

        visitTimer->StopTimer(writingData, "Writing data");
    }
    CATCH(ImproperUseException)
    {
        engine->SetNoFatalExceptions(false);
    }
    ENDTRY

    return success;
}

// ****************************************************************************
// Method: SimEngineManager::Render
//
// Purpose:
//   Renders the window in the network manager, returns the image encoded
//   in a data object reader.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       The write/read using the data object reader is unnecessary in
//             the long run if we can streamline the Viewer/Engine interface
//             for exchanging data somewhat. This code is really similar to
//             the code in Executors.h, except we don't write to a socket.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 15 14:59:01 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool 
SimEngineManager::Render(const EngineKey &/*ek*/, avtDataObjectReader_p &rdr,
                         bool sendZBuffer, const intVector &networkIds, 
                         int annotMode, int windowID, bool leftEye,
                         void (*waitCB)(void *), void *waitCBData)
{
    // do the render
    avtDataObject_p image = engine->GetNetMgr()->Render(true,
        networkIds, sendZBuffer, annotMode, windowID, leftEye);
        
    avtDataObjectWriter_p writer;
    if(*image != NULL)
    {
        writer = image->InstantiateWriter();
        writer->SetInput(image);
    }
    else
    {
        writer = engine->GetNetMgr()->CreateNullDataWriter();
    }

    // Send the data back to the viewer.
    bool useCompression = engine->GetNetMgr()->GetShouldUseCompression(windowID);

    rdr = new avtDataObjectReader;
    std::string errorMessage;
    bool  respondWithNull = false;
    int   scalableThreshold = -1;
    bool  scalableThresholdExceeded = false;
    int   currentTotalGlobalCellCount = 0;
    float cellCountMultiplier = 1.f;
    int   currentNetworkGlobalCellCount = 0;

    DOString dos;

    bool success = engine->GatherData(writer,
                       useCompression,
                       respondWithNull,
                       scalableThreshold, 
                       currentTotalGlobalCellCount, cellCountMultiplier,
                       NULL, NULL,
                       WriteCallback, (void*)&dos,
                       errorMessage,
                       &scalableThresholdExceeded,
                       &currentNetworkGlobalCellCount);

    // Read the data object string that we harvested in WriteCallback.
    if (success)
        ReadDataObjectString(rdr, dos);

    return success;
}

// ****************************************************************************
// Method: SimEngineManager::Pick
//
// Purpose:
//   Called when we want to perform a pick.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 15 17:40:56 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::Pick(const EngineKey &/*ek*/, const int networkId, int windowId,
                       const PickAttributes *atts, PickAttributes &retAtts)
{
    retAtts = *atts;

    if (networkId >= 0)
        engine->GetNetMgr()->Pick(networkId, windowId, &retAtts);
    else
        engine->GetNetMgr()->PickForIntersection(windowId, &retAtts);
    return true;
}

// ****************************************************************************
// Method: SimEngineManager::StartPick
//
// Purpose:
//   Called when we want to perform a pick.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 15 17:40:56 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::StartPick(const EngineKey &/*ek*/, const bool forZones,
                            const bool flag, const int nid)
{
    if (flag)
        engine->GetNetMgr()->StartPickMode(forZones);
    else
        engine->GetNetMgr()->StopPickMode();
    return true;
}

// ****************************************************************************
// Method: SimEngineManager::StartQuery
//
// Purpose:
//   Called when we want to perform a query.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 15 17:40:56 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::StartQuery(const EngineKey &/*ek*/, const bool flag, const int nid)
{
    if (flag)
        engine->GetNetMgr()->StartQueryMode();
    else
        engine->GetNetMgr()->StopQueryMode();
    return true;
}

// ****************************************************************************
// Method: SimEngineManager::SetWinAnnotAtts
//
// Purpose:
//   Called when we want to set the engine's window properties.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 15 17:40:56 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::SetWinAnnotAtts(const EngineKey &/*ek*/,
                                 const WindowAttributes *wa,
                                 const AnnotationAttributes *aa,
                                 const AnnotationObjectList *ao,
                                 const std::string extStr,
                                 const VisualCueList *visCues,
                                 const int *frameAndState,
                                 const double *viewExtents,
                                 const std::string ctName,
                                 const int windowID)
{
    engine->GetNetMgr()->SetWindowAttributes(*wa,
                                extStr,
                                viewExtents,
                                ctName,
                                windowID);
    engine->GetNetMgr()->SetAnnotationAttributes(*aa,
                                    *ao,
                                    *visCues,
                                    frameAndState,
                                    windowID);

    return true;
}

// ****************************************************************************
// Method: SimEngineManager::Query
//
// Purpose:
//   Perform a query.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:12:03 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::Query(const EngineKey &/*ek*/,
                        const intVector &networkIds, 
                        const QueryAttributes *atts,
                        QueryAttributes &retAtts)
{
    retAtts = *atts;
    engine->GetNetMgr()->Query(networkIds, &retAtts);
    return true;
}

// ****************************************************************************
// Method: SimEngineManager::GetQueryParameters
//
// Purpose:
//   Get query parameters for the specified query.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:12:03 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::GetQueryParameters(const EngineKey &/*ek*/,
                                     const std::string &qname,
                                     std::string *params)
{
    *params = engine->GetNetMgr()->GetQueryParameters(qname);
    return true;
}

// ****************************************************************************
// Method: SimEngineManager::ClearCache
//
// Purpose:
//   Clear the engine cache.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:12:03 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::ClearCache(const EngineKey &/*ek*/, const std::string &dbName)
{
    if (dbName.empty())
        engine->GetNetMgr()->ClearAllNetworks();
    else
        engine->GetNetMgr()->ClearNetworksWithDatabase(dbName);
    return true;
}

// ****************************************************************************
// Method: SimEngineManager::GetProcInfo
//
// Purpose:
//   Get the engine's memory usage.
//
// Arguments:
//   ek      : Engine key
//   retAtts : The returned process atts.
//
// Returns:    True on success; False on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:12:03 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::GetProcInfo(const EngineKey &/*ek*/, ProcessAttributes &retAtts)
{
    retAtts = *engine->GetProcessAttributes();
    return true;
}

// ****************************************************************************
// Method: SimEngineManager::ReleaseData
//
// Purpose:
//   Release data for the specified network.
//
// Arguments:
//   ek        : Engine key
//   networkId : The network whose data we'll clear.
//
// Returns:    True on success; False on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:12:03 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::ReleaseData(const EngineKey &/*ek*/, int networkId)
{
    engine->GetNetMgr()->DoneWithNetwork(networkId);
    return true;
}

// ****************************************************************************
// Method: SimEngineManager::CloneNetwork
//
// Purpose:
//   Clone the specified network.
//
// Arguments:
//   ek        : Engine key
//   networkId : The network whose data we'll clear.
//
// Returns:    True on success; False on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:12:03 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::CloneNetwork(const EngineKey &/*ek*/, int networkId, 
                               const QueryOverTimeAttributes *qatts)
{
    engine->GetNetMgr()->CloneNetwork(networkId);
#if 0
// This is a sim. We don't want query over time atts do we?
    if (qatts != NULL)
    {
        QueryOverTimeAttributes atts(*qatts);
        engine->GetNetMgr()->AddQueryOverTimeFilter(atts, networkId);
    }
#endif
    return true;
}

// ****************************************************************************
// Method: SimEngineManager::CreateNamedSelection
//
// Purpose:
//   Create a named selection.
//
// Arguments:
//   ek        : Engine key
//   networkId : The network producing the named selection.
//   props     : The selection properties.
//   summary   : The returned selection summary.
//
// Returns:    True on success; False on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:12:03 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::CreateNamedSelection(const EngineKey &/*ek*/, 
                                       int networkId,
                                       const SelectionProperties &props,
                                       SelectionSummary &summary)
{
    summary.SetName(props.GetName());
    avtNamedSelectionManager::GetInstance()->ClearCache(props.GetName());
    summary = engine->GetNetMgr()->CreateNamedSelection(networkId, props);
    return true;
}

// ****************************************************************************
// Method: SimEngineManager::UpdateNamedSelection
//
// Purpose:
//   Update a named selection.
//
// Arguments:
//   ek        : Engine key
//   networkId : The network producing the named selection.
//   props     : The selection properties.
//   allowCache : Whether previously cached selection results can be used.
//   summary   : The returned selection summary.
//
// Returns:    True on success; False on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:12:03 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::UpdateNamedSelection(const EngineKey &/*ek*/, 
                                       int networkId,
                                       const SelectionProperties &props, 
                                       bool allowCache,
                                       SelectionSummary &summary)
{
    if(!allowCache)
        avtNamedSelectionManager::GetInstance()->ClearCache(props.GetName());
     engine->GetNetMgr()->DeleteNamedSelection(props.GetName());
     summary = engine->GetNetMgr()->CreateNamedSelection(networkId, props);
     return true;
}

// ****************************************************************************
// Method: SimEngineManager::DeleteNamedSelection
//
// Purpose:
//   Delete a named selection.
//
// Arguments:
//   ek      : Engine key
//   selName : The name of the selection.
//
// Returns:    True on success; False on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:12:03 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::DeleteNamedSelection(const EngineKey &/*ek*/,
                                       const std::string &selName)
{
    avtNamedSelectionManager::GetInstance()->ClearCache(selName);
    engine->GetNetMgr()->DeleteNamedSelection(selName);
    return true;
}

// ****************************************************************************
// Method: SimEngineManager::LoadNamedSelection
//
// Purpose:
//   Load a named selection.
//
// Arguments:
//   ek      : Engine key
//   selName : The name of the selection.
//
// Returns:    True on success; False on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:12:03 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::LoadNamedSelection(const EngineKey &/*ek*/,
                                     const std::string &selName)
{
    engine->GetNetMgr()->LoadNamedSelection(selName);
    return true;
}

// ****************************************************************************
// Method: SimEngineManager::SaveNamedSelection
//
// Purpose:
//   Save a named selection.
//
// Arguments:
//   ek      : Engine key
//   selName : The name of the selection.
//
// Returns:    True on success; False on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:12:03 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::SaveNamedSelection(const EngineKey &/*ek*/,
                                     const std::string &selName)
{
    engine->GetNetMgr()->SaveNamedSelection(selName);
    return true;
}

// ****************************************************************************
// Method: SimEngineManager::ExportDatabases
//
// Purpose:
//   Export the given networks to output files.
//
// Arguments:
//   ek   : Engine key
//   ids  : The network ids to export.
//   expAtts    : The export db attributes.
//   timeSuffix : A time suffix if we're exporting multiple files.
//
// Returns:    True on success; False on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:12:03 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::ExportDatabases(const EngineKey &/*ek*/,
                                  const intVector &ids,
                                  const ExportDBAttributes &expAtts, 
                                  const std::string &timeSuffix)
{
    engine->GetNetMgr()->ExportDatabases(ids, expAtts, timeSuffix);
    return true;
}

// ****************************************************************************
// Method: SimEngineManager::ConstructDataBinning
//
// Purpose:
//   Construct data binning.
//
// Arguments:
//   ek  : Engine key
//   id  : The network id
//
// Returns:    True on success; False on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:12:03 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::ConstructDataBinning(const EngineKey &/*ek*/, int id)
{
    engine->GetNetMgr()->ConstructDataBinning(id, GetViewerState()->GetConstructDataBinningAttributes());
    return true;
}

// ****************************************************************************
// Method: SimEngineManager::UpdateExpressions
//
// Purpose:
//   Send an updated expression list to the engine.
//
// Arguments:
//   ek  : Engine key
//   eL  : The expression list.
//
// Returns:    True on success; False on failure.
//
// Note:       Does nothing since we're already in the engine.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:12:03 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngineManager::UpdateExpressions(const EngineKey &/*ek*/, const ExpressionList &eL)
{
    // Does nothing.
    return true;
}

// ****************************************************************************
// Method: SimEngineManager::UpdateDefaultFileOpenOptions
//
// Purpose:
//   Update the file open options.
//
// Arguments:
//   opts : The file open options.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:12:03 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
SimEngineManager::UpdateDefaultFileOpenOptions(FileOpenOptions *opts)
{
    avtDatabaseFactory::SetDefaultFileOpenOptions(*opts);
}

// ****************************************************************************
// Method: SimEngineManager::UpdatePrecisionType
//
// Purpose:
//   Update the precision.
//
// Arguments:
//   type : The precision type.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:21:00 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
SimEngineManager::UpdatePrecisionType(const int type)
{
    avtDatabaseFactory::SetPrecisionType(type);
}

// ****************************************************************************
// Method: SimEngineManager::UpdateBackendType
//
// Purpose:
//   Update the back end type.
//
// Arguments:
//   type : The back end type.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:21:27 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void 
SimEngineManager::UpdateBackendType(const int type)
{
    avtDatabaseFactory::SetBackendType(type);
    avtCallback::SetBackendType((GlobalAttributes::BackendType)type);
}


// ****************************************************************************
// Method: SimEngineManager::UpdateRemoveDuplicateNodes
//
// Purpose:
//   Update the flag specifying duplicate node removal.
//
// Arguments:
//   val : the flag.
//
// Programmer: Kathleen Biagas
// Creation:   December 18, 2014
//
// Modifications:
//
// ****************************************************************************

void 
SimEngineManager::UpdateRemoveDuplicateNodes(const bool val)
{
    avtDatabaseFactory::SetRemoveDuplicateNodes(val);
}

// ****************************************************************************
// Method: SimEngineManager::LaunchProcess
//
// Purpose:
//   Launch a process.
//
// Arguments:
//   ek   : The engine key.
//   args : Command line arguments.
//
// Returns:    False
//
// Note:       Do nothing.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 22 22:21:55 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool 
SimEngineManager::LaunchProcess(const EngineKey &/*ek*/, const stringVector &args)
{
    // Does nothing.

    return false;
}

void
SimEngineManager::CreateNode(DataNode *parent, bool detailed)
{
    // Does nothing.
}

void
SimEngineManager::SetFromNode(DataNode *parent, const std::string &configVersion)
{
    // Does nothing.
}


