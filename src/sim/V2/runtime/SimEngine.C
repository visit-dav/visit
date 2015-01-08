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
#include <SimEngine.h>

#include <ObserverToCallback.h>

#include <DebugStream.h>
#include <DatabasePluginManager.h>
#include <SimPlotPluginManager.h>
#include <SimOperatorPluginManager.h>
#include <PlotPluginInfo.h>
#include <LoadBalancer.h>
#include <NetworkManager.h>
#include <Netnodes.h>
#include <avtDatabaseFactory.h>
#include <TimingsManager.h>
#include <FileFunctions.h>

#include <VisItInterfaceTypes_V2.h>


#ifdef SIMV2_VIEWER_INTEGRATION
#include <ViewerRPC.h>

#include <ExportDBAttributes.h>
#include <GlobalAttributes.h>
#include <SaveWindowAttributes.h>

#include <ViewerActionManager.h>
#include <ViewerFactory.h>
#include <ViewerMethods.h>
#include <ViewerOperatorFactory.h>
#include <ViewerPlotFactory.h>
#include <ViewerPlotList.h>
#include <ViewerState.h>
#include <ViewerStateManager.h>
#include <ViewerMessaging.h>
#include <ViewerProperties.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>
#include <ViewerQueryManager.h>

#include <SimEngineManager.h>
#include <SimFileServer.h>
#include <SimPlotPluginManager.h>
#include <SimOperatorPluginManager.h>

#include <avtParallel.h>

// ****************************************************************************
// Class: SimViewerFactory
//
// Purpose:
//   Custom factory for creating important viewer objects.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 18 13:35:37 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class SimViewerFactory : public ViewerFactory
{
public:
    SimViewerFactory(SimEngine *e) : ViewerFactory(), engine(e)
    {
    }

    virtual ViewerFileServerInterface *CreateFileServerInterface()
    {
        return new SimFileServer(engine);
    }

    virtual ViewerEngineManagerInterface *CreateEngineManagerInterface()
    {
        return new SimEngineManager(engine);
    }

    virtual PlotPluginManager *CreatePlotPluginManager()
    {
        // return an existing instance.
        return engine->GetNetMgr()->GetPlotPluginManager();
    }

    virtual OperatorPluginManager *CreateOperatorPluginManager()
    {
        // return an existing instance.
        return engine->GetNetMgr()->GetOperatorPluginManager();
    }

private:
    SimEngine *engine;
};

#endif

// ****************************************************************************
// Method: SimEngine::SimEngine
//
// Purpose:
//   Constructor.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 18:33:36 PDT 2014
//
// Modifications:
//
// ****************************************************************************

SimEngine::SimEngine() : Engine()
#ifdef SIMV2_VIEWER_INTEGRATION
                                 , ViewerBase()
#endif
    , viewerInitialized(false), simsource(), rpcNotifier(NULL)
{
}

// ****************************************************************************
// Method: SimEngine::~SimEngine
//
// Purpose:
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 18:33:36 PDT 2014
//
// Modifications:
//
// ****************************************************************************

SimEngine::~SimEngine()
{
}

// ****************************************************************************
// Method: SimEngine::InitializeViewer
//
// Purpose:
//   Initializes the viewer.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       This roughly follows the stuff that gets set up for a 
//             ViewerSubject but it is a lot more minimal.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 18 12:51:21 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
SimEngine::InitializeViewer()
{
#ifdef SIMV2_VIEWER_INTEGRATION
    if(!viewerInitialized)
    {
        // Non-existent filename that will be used for the simulation. It should not
        // really matter what the filename is because the file server and engine manager
        // replacements we've made for in situ viewer work don't care about the filename.
        simsource = FileFunctions::ExpandPath("batch.sim2", 
                    FileFunctions::GetCurrentWorkingDirectory());

        // Install a custom factory for viewer object creation.
        SetViewerFactory(new SimViewerFactory(this));

        GetViewerProperties()->SetNowin(true);
        GetViewerProperties()->SetMasterProcess(PAR_UIProcess());
        GetViewerStateManager()->CreateState();

        // Install a callback to schedule execution of internal commands.
        GetViewerMessaging()->SetCommandsNotifyCallback(CommandNotificationCallback, (void*)this);

        // Install a callback function to call when we do some viewer actions via
        // ViewerMethods.
        rpcNotifier = new ObserverToCallback(GetViewerState()->GetViewerRPC(), 
                                             HandleViewerRPCCallback, this);

        // Connect the the default state objectsto the config manager.
        GetViewerStateManager()->ConnectDefaultState();

        // Finish initialization.
        HeavyInitialization();

        // Force scalable rendering.
        GetViewerState()->GetRenderingAttributes()->
            SetDisplayListMode(RenderingAttributes::Never);
        GetViewerState()->GetRenderingAttributes()->
            SetScalableActivationMode(RenderingAttributes::Always);
        GetViewerState()->GetRenderingAttributes()->
            SetScalableAutoThreshold(1);
        GetViewerMethods()->SetRenderingAttributes();

        // Open the sim data.
        GetViewerMethods()->OpenDatabase(this->simsource);
    }

    viewerInitialized = true;
#endif
}

// ****************************************************************************
// Method: SimEngine::CreatePluginManagers
//
// Purpose:
//   Create the plugin managers for the network manager.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 18:33:36 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
SimEngine::CreatePluginManagers()
{
    // These 2 plugin managers can serve up viewer info from engine plugins.
    GetNetMgr()->SetPlotPluginManager(new SimPlotPluginManager());
    GetNetMgr()->SetOperatorPluginManager(new SimOperatorPluginManager());

    GetNetMgr()->SetDatabasePluginManager(new DatabasePluginManager());
}

// ****************************************************************************
// Method: SimEngine::SimulationInitiateCommand
//
// Purpose:
//   Handle commands initiated by the simulation.
//
// Arguments:
//   command : The command to execute.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 18 23:31:17 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
SimEngine::SimulationInitiateCommand(const std::string &command)
{
#ifdef SIMV2_VIEWER_INTEGRATION
    if(viewerInitialized)
    {
        if(command == "UpdatePlots")
        {
            EngineKey ek = ViewerWindowManager::Instance()->GetActiveWindow()->
                           GetPlotList()->GetEngineKey();

            // The simulation told us that it wants us to update all of the plots 
            // that use it as a source.
            ViewerWindowManager::Instance()->ReplaceDatabase(ek, this->simsource,
                0, false, true, false);
        }
        else if(command.substr(0,8) == "Message:")
        {
            GetViewerMessaging()->Message(command.substr(8,command.size()-8));
        }
        else if(command.substr(0,6) == "Error:")
        {
            GetViewerMessaging()->Error(command.substr(6,command.size()-6));
        }
#if 0
        else if(command.substr(0,10) == "Interpret:")
        {
            InterpretCommands(command.substr(10, command.size()-10));
        }
        else if(command.substr(0,12) == "INTERNALSYNC")
        {
            // Send the command back to the engine so it knows we're done syncing.
            std::string cmd("INTERNALSYNC");
            std::string args(command.substr(13, command.size()-1));
            GetViewerEngineManager()->SendSimulationCommand(key, cmd, args);
        }
#endif
    }
    else
#endif
    {
        Engine::SimulationInitiateCommand(command);
    }
}

// ****************************************************************************
// Method: SimEngine::OpenDatabase
//
// Purpose:
//   Opens the simulation's own data in the network manager.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 18 12:50:50 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngine::OpenDatabase()
{
    std::string format("SimV2_1.0");
    GetNetMgr()->GetDatabasePluginManager()->PluginAvailable(format);
   
    avtDatabaseFactory::SetCreateMeshQualityExpressions(GetViewerState()->
        GetGlobalAttributes()->GetCreateMeshQualityExpressions());
    avtDatabaseFactory::SetCreateTimeDerivativeExpressions(GetViewerState()->
        GetGlobalAttributes()->GetCreateTimeDerivativeExpressions()); 
    bool ignoreExtents = GetViewerState()->GetGlobalAttributes()->
                         GetIgnoreExtentsFromDbs();

    int  timeState = 0;
    bool treatAllDBsAsTimeVarying = true;
    bool fileMayHaveUnloadedPlugin = false;
    GetNetMgr()->GetDBFromCache(this->simsource, timeState,
                                format.c_str(),
                                treatAllDBsAsTimeVarying,
                                fileMayHaveUnloadedPlugin,
                                ignoreExtents);

    PopulateSimulationMetaData(this->simsource, format);

    return true;
}

// Note: The filename argument is not used at this time since we're only allowing
//       the simulation to process its own data.

const avtDatabaseMetaData *
SimEngine::GetMetaData(const std::string &/*filename*/)
{
    return GetMetaDataForState(this->simsource, 0);
}

const avtDatabaseMetaData *
SimEngine::GetMetaDataForState(const std::string &/*filename*/, int /*timeState*/)
{
    const char *mName = "SimEngine::GetMetaData: ";
    const avtDatabaseMetaData *md = NULL;

    OpenDatabase();

    int timeState = 0;
    NetnodeDB *dbNode = GetNetMgr()->GetDBFromCache(this->simsource, 
                                                    timeState,
                                                    "SimV2_1.0");
    if(dbNode != NULL)
        md = dbNode->GetMetaData(timeState);
    else
    {
        debug1 << mName << "Could not get metadata for " << this->simsource << endl;
    }

    return md;
}

const avtSIL *
SimEngine::GetSIL(const std::string &/*filename*/)
{
    return GetSILForState(this->simsource, 0);
}

const avtSIL *
SimEngine::GetSILForState(const std::string &/*filename*/, int /*timeState*/)
{
    const char *mName = "SimEngine::GetMetaData: ";
    const avtSIL *sil = NULL;

    OpenDatabase();

    int timeState = 0;
    NetnodeDB *dbNode = GetNetMgr()->GetDBFromCache(this->simsource, 
                                                    timeState,
                                                    "SimV2_1.0");
    if(dbNode != NULL)
        sil = dbNode->GetDB()->GetSIL(0, true);
    else
    {
        debug1 << mName << "Could not get SIL for " << this->simsource << endl;
    }

    return sil;
}

// ****************************************************************************
// Method: SimEngine::ExportDatabase
//
// Purpose:
//   Exports the active plots.
//
// Arguments:
//   filename : The name of the file to save.
//   format   : The format of the export file.
//   vars     : The variables to export.
//
// Returns:    True on success; False on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 18 11:29:09 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngine::ExportDatabase(const std::string &filename, const std::string &format,
                          const stringVector &vars)
{
    bool retval = false;
#ifdef SIMV2_VIEWER_INTEGRATION
    if(viewerInitialized)
    {
        // Get the plugin id from the input format, which could be an id or a name.
        std::string id, name;
        for(int i = 0; i < GetNetMgr()->GetDatabasePluginManager()->GetNAllPlugins(); ++i)
        {
            std::string thisID(GetNetMgr()->GetDatabasePluginManager()->GetAllID(i));
            if(thisID == format)
                id = thisID;
            if(GetNetMgr()->GetDatabasePluginManager()->GetPluginName(thisID) == format)
                id = thisID;
        }
        if(id.empty())
            return false;
        name = GetNetMgr()->GetDatabasePluginManager()->GetPluginName(id);

        std::string dName(FileFunctions::Dirname(filename));
        std::string fName(FileFunctions::Basename(filename));
        if(dName.empty() || dName == ".")
            dName = FileFunctions::GetCurrentWorkingDirectory();

        ExportDBAttributes *atts = GetViewerState()->GetExportDBAttributes();
        atts->SetAllTimes(false);
        atts->SetDb_type(name);
        atts->SetDb_type_fullname(id);
        atts->SetDirname(dName);
        atts->SetFilename(fName);
        atts->SetVariables(vars);
        atts->Notify();

        GetViewerMethods()->ExportDatabase();
        retval = true;
    }
#endif
    return retval;
}

// ****************************************************************************
// Method: SimEngine::RestoreSession
//
// Purpose:
//   Restores a session file to set up plots using the sim data.
//
// Arguments:
//   filename : The name of the session file to restore.
//
// Returns:    True on success; False on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 18 11:29:09 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngine::RestoreSession(const std::string &filename)
{
    bool retval = false;
#ifdef SIMV2_VIEWER_INTEGRATION
    if(viewerInitialized)
    {
        stringVector sources;
        for(int i = 0; i < 10; ++i)
            sources.push_back(this->simsource);

        TRY
        {
            GetViewerMethods()->
                ImportEntireStateWithDifferentSources(filename, false, sources);
            retval = true;
        }
        CATCHALL
        { 
            retval = false;
        }
        ENDTRY
    }
#endif
    return retval;
}

// ****************************************************************************
// Method: SimEngine::SaveWindow
//
// Purpose:
//   Saves the current vis window to an image file.
//
// Arguments:
//   filename : The name of the file to save.
//   w : The image width
//   h : The image height
//   format : The image format in libsim terms.
//
// Returns:    True on success; false on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 18 16:42:38 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngine::SaveWindow(const std::string &filename, int w, int h, int format)
{
    bool retval = false;

    TRY
    {
        SaveWindowAttributes::FileFormat fmt;
        if(format == VISIT_IMAGEFORMAT_BMP)
            fmt = SaveWindowAttributes::BMP;
        else if(format == VISIT_IMAGEFORMAT_JPEG)
            fmt = SaveWindowAttributes::JPEG;
        else if(format == VISIT_IMAGEFORMAT_PNG)
            fmt = SaveWindowAttributes::PNG;
        else if(format == VISIT_IMAGEFORMAT_POVRAY)
            fmt = SaveWindowAttributes::POVRAY;
        else if(format == VISIT_IMAGEFORMAT_PPM)
            fmt = SaveWindowAttributes::PPM;
        else if(format == VISIT_IMAGEFORMAT_RGB)
            fmt = SaveWindowAttributes::RGB;
        else
            fmt = SaveWindowAttributes::TIFF;

#ifdef SIMV2_VIEWER_INTEGRATION
        // Viewer based method.
        if(viewerInitialized)
        {
            std::string dName(FileFunctions::Dirname(filename));
            std::string fName(FileFunctions::Basename(filename));
            if(dName.empty() || dName == ".")
                dName = FileFunctions::GetCurrentWorkingDirectory();

            SaveWindowAttributes *swa = GetViewerState()->GetSaveWindowAttributes();
            swa->SetFileName(fName);
            swa->SetOutputToCurrentDirectory(false);
            swa->SetOutputDirectory(dName);
            swa->SetFamily(false);
            swa->SetFormat(fmt);
            swa->SetWidth(w);
            swa->SetHeight(h);
            swa->SetSaveTiled(false);
            swa->SetScreenCapture(false);
            swa->Notify();

            GetViewerMethods()->SaveWindow();

            retval = true;
        }
#endif
    }
    CATCHALL
    {
        retval = false;
    }
    ENDTRY

    return retval;
}

// ****************************************************************************
// Method: SimEngine::AddPlot
//
// Purpose:
//   Adds a plot to the active vis window.
//
// Arguments:
//   plotType : The plot type (its id or name)
//   var      : the plot variable.
//
// Returns:    true on success; false on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 18 17:26:58 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngine::AddPlot(const std::string &plotType, const std::string &var)
{
    // Get the plugin id from the input plotType, which could be an id or a name.
    std::string id;
    for(int i = 0; i < GetNetMgr()->GetPlotPluginManager()->GetNEnabledPlugins(); ++i)
    {
        std::string thisID(GetNetMgr()->GetPlotPluginManager()->GetEnabledID(i));
        if(thisID == plotType)
             id = thisID;
        if(GetNetMgr()->GetPlotPluginManager()->GetPluginName(thisID) == plotType)
             id = thisID;
    }
    if(id.empty())
        return false;

    bool retval = false;
    TRY
    {
#ifdef SIMV2_VIEWER_INTEGRATION
        // Viewer based method.
        if(viewerInitialized)
        {
            int plotIndex = GetNetMgr()->GetPlotPluginManager()->GetEnabledIndex(id);

// cout << "Viewer-based AddPlot(" << plotIndex << "=" << id << ", " << var << ")" << endl;
            GetViewerMethods()->AddPlot(plotIndex, var);
            retval = true;
        }
#endif
    }
    CATCHALL
    {
        retval = false;
    }
    ENDTRY

    return retval;
}

// ****************************************************************************
// Method: SimEngine::AddOperator
//
// Purpose:
//   Adds an operator to the active vis window.
//
// Arguments:
//   simfile : Name of the sim file.
//   plotType : 
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 18 17:26:58 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngine::AddOperator(const std::string &operatorType, bool applyToAll)
{
    // Get the plugin id from the input operatorType, which could be an id or a name.
    std::string id;
    for(int i = 0; i < GetNetMgr()->GetOperatorPluginManager()->GetNEnabledPlugins(); ++i)
    {
        std::string thisID(GetNetMgr()->GetOperatorPluginManager()->GetEnabledID(i));
        if(thisID == operatorType)
             id = thisID;
        if(GetNetMgr()->GetOperatorPluginManager()->GetPluginName(thisID) == operatorType)
             id = thisID;
    }
    if(id.empty())
        return false;

    bool retval = false;
    TRY
    {
#ifdef SIMV2_VIEWER_INTEGRATION
        // Viewer based method.
        if(viewerInitialized)
        {
            int operatorIndex = GetNetMgr()->GetPlotPluginManager()->GetEnabledIndex(id);

            bool applyOperatorSave = GetViewerState()->GetGlobalAttributes()->GetApplyOperator();
            GetViewerState()->GetGlobalAttributes()->SetApplyOperator(applyToAll != 0);

// cout << "Viewer-based AddOperator(" << operatorIndex << "=" << id << ", " << var << ")" << endl;
            GetViewerMethods()->AddOperator(operatorIndex);

            GetViewerState()->GetGlobalAttributes()->SetApplyOperator(applyOperatorSave);
            retval = true;
        }
#endif
    }
    CATCHALL
    {
        retval = false;
    }
    ENDTRY

    return retval;
}

// ****************************************************************************
// Method: SimEngine::DrawPlots
//
// Purpose:
//   Draw the plot(s).
//
// Arguments:
//
// Returns:    true on success; false on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 18 18:02:39 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngine::DrawPlots()
{
    bool retval = false;

    TRY
    {
#ifdef SIMV2_VIEWER_INTEGRATION
        // Viewer based method.
        if(viewerInitialized)
        {
//cout << "Viewer-based DrawPlots()" << endl;
            GetViewerMethods()->DrawPlots();
            retval = true;
        }
#endif
    }
    CATCHALL
    {
        retval = false;
    }
    ENDTRY

    return retval;
}

// ****************************************************************************
// Method: SimEngine::DeleteActivePlots
//
// Purpose:
//   Delete the active plots.
//
// Arguments:
//
// Returns:    true on success; false on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 18 18:02:39 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
SimEngine::DeleteActivePlots()
{
    bool retval = false;

    TRY
    {
#ifdef SIMV2_VIEWER_INTEGRATION
        // Viewer based method.
        if(viewerInitialized)
        {
            GetViewerMethods()->DeleteActivePlots();
            retval = true;
        }
#endif
    }
    CATCHALL
    {
        retval = false;
    }
    ENDTRY

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
//                            VIEWER-RELATED CODE
///////////////////////////////////////////////////////////////////////////////

#ifdef SIMV2_VIEWER_INTEGRATION
// ****************************************************************************
// Method: SimEngine::HeavyInitialization
//
// Purpose:
//   Sets up several of the viewer systems.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       This method mimics a subset of what's done in ViewerSubject.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 18 13:30:40 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
SimEngine::HeavyInitialization()
{
    // The viewer likes to have its plugins loaded... I'd like to not have to
    // do this...
    LoadPlotPlugins();
    LoadOperatorPlugins();

    ViewerQueryManager::Instance()->InitializeQueryList();

    GetViewerStateManager()->ConnectPluginDefaultState();

    AddInitialWindows();

    // Process settings. This also does stuff like read in external color tables.
    GetViewerStateManager()->ProcessSettings();
}

// ****************************************************************************
// Method: SimEngine::LoadPlotPlugins
//
// Purpose: 
//   Loads the plot plugins and creates the plot factory object.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 17 15:16:13 PST 2003
//
// Modifications:
//
// ****************************************************************************

void
SimEngine::LoadPlotPlugins()
{
    int total  = visitTimer->StartTimer();
    int timeid = visitTimer->StartTimer();

    //
    // It's safe to load the plugins now
    //
    TRY
    {
        GetPlotPluginManager()->LoadPluginsNow();
    }
    CATCH2(VisItException, e)
    {
        // Just print out an error message to the console because we cannot
        // abort without hanging the viewer's client.
        cerr << "VisIt could not read all of the plot plugins. "
             << "The error message is: \"" << e.Message().c_str() << "\"" << endl;
    }
    ENDTRY
    visitTimer->StopTimer(timeid, "Loading plot plugins.");

    //
    // Create the Plot factory.
    //
    for (int i = 0; i < GetPlotFactory()->GetNPlotTypes(); ++i)
    {
        AttributeSubject *attr = GetPlotFactory()->GetClientAtts(i);
        if (attr != 0)
            GetViewerState()->RegisterPlotAttributes(attr);
    }

    visitTimer->StopTimer(total, "Loading plot plugins and instantiating objects.");
}

// ****************************************************************************
// Method: SimEngine::LoadOperatorPlugins
//
// Purpose: 
//   Loads the operator plugins and creates the operator factory object.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 17 15:16:57 PST 2003
//
// Modifications:
//
// ****************************************************************************

void
SimEngine::LoadOperatorPlugins()
{
    int total = visitTimer->StartTimer();
    int timeid = visitTimer->StartTimer();

    //
    // It's safe to load the plugins now
    //
    TRY
    {
        GetOperatorPluginManager()->LoadPluginsNow();
    }
    CATCH2(VisItException, e)
    {
        // Just print out an error message to the console because we cannot
        // abort without hanging the viewer's client.
        cerr << "VisIt could not read all of the operator plugins. "
             << "The error message is: \"" << e.Message().c_str() << "\"" << endl;
    }
    ENDTRY
    visitTimer->StopTimer(timeid, "Loading operator plugins.");

    //
    // Create the Operator factory.
    //
    for (int i = 0; i < GetOperatorFactory()->GetNOperatorTypes(); ++i)
    {
        AttributeSubject *attr = GetOperatorFactory()->GetClientAtts(i);
        if (attr != 0)
            GetViewerState()->RegisterOperatorAttributes(attr);
    }

    visitTimer->StopTimer(total, "Loading operator plugins and instantiating objects.");
}

// ****************************************************************************
// Method: SimEngine::AddInitialWindows
//
// Purpose:
//   Adds the initial viewer window.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 18 13:31:23 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
SimEngine::AddInitialWindows()
{
    // Make sure the viewer properties have good values for windows and decorations.
    if (GetViewerProperties()->GetWindowBorders().empty())
    {
        GetViewerProperties()->SetWindowBorders("0,0,0,0");
    }
    if (GetViewerProperties()->GetWindowShift().empty())
    {
        GetViewerProperties()->SetWindowShift("0,0");
    }
    if (GetViewerProperties()->GetWindowPreShift().empty())
    {
        GetViewerProperties()->SetWindowPreShift("0,0");
    }
    if (GetViewerProperties()->GetWindowGeometry().empty())
    {
        if (GetViewerProperties()->GetWindowSmall())
            GetViewerProperties()->SetWindowGeometry("512x512");
        else
            GetViewerProperties()->SetWindowGeometry("1024x1024");
    }

    //
    // Set the options in the viewer window manager.
    //
    ViewerWindowManager *windowManager=ViewerWindowManager::Instance();
    windowManager->SetBorders(GetViewerProperties()->GetWindowBorders().c_str());
    windowManager->SetShift(GetViewerProperties()->GetWindowShift().c_str());
    windowManager->SetPreshift(GetViewerProperties()->GetWindowPreShift().c_str());
    windowManager->SetGeometry(GetViewerProperties()->GetWindowGeometry().c_str());

    // Add the first window.
    windowManager->AddWindow();
    windowManager->ShowAllWindows(); // offscreen, of course
}

// ****************************************************************************
// Method: SimEngine::CommandNotificationCallback
//
// Purpose:
//   This callback function is used when we've added some commands to process
//   to the work queue in ViewerMessaging. We just process the work immediately.
//
// Arguments:
//   cbdata : The callback data
//   int    : timeout
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 18 12:56:44 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
SimEngine::CommandNotificationCallback(void *cbdata, int)
{
    SimEngine::GetViewerMessaging()->ProcessCommands();
}

// ****************************************************************************
// Method: SimEngine::HandleViewerRPCCallback
//
// Purpose:
//   This callback function is called when we have modified ViewerRPC via
//   operations to ViewerMethods.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 18 13:11:54 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
SimEngine::HandleViewerRPCCallback(Subject *, void *)
{
    if(GetViewerState()->GetViewerRPC()->GetRPCType() != ViewerRPC::CloseRPC)
    {
        ViewerWindowManager::Instance()->GetActiveWindow()->GetActionManager()->
            HandleAction(*GetViewerState()->GetViewerRPC());
    }
}
#endif
