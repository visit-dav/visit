// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <SimEngine.h>

#include <ObserverToCallback.h>
#include <DBOptionsAttributes.h>
#include <DebugStream.h>
#include <DatabasePluginManager.h>
#include <DatabasePluginInfo.h>
#include <ExpressionList.h>
#include <Expression.h>
#include <ParsingExprList.h>
#include <SimPlotPluginManager.h>
#include <SimOperatorPluginManager.h>
#include <OperatorPluginInfo.h>
#include <PlotPluginInfo.h>
#include <LoadBalancer.h>
#include <NetworkManager.h>
#include <Netnodes.h>
#include <avtDatabaseFactory.h>
#include <SingleAttributeConfigManager.h>
#include <StackTimer.h>
#include <TimingsManager.h>
#include <FileFunctions.h>

#include <VisItInterfaceTypes_V2.h>
#include <VisItInterfaceTypes_V2P.h>

#ifdef SIMV2_VIEWER_INTEGRATION
#include <ViewerRPC.h>

#include <ExportDBAttributes.h>
#include <GlobalAttributes.h>
#include <Plot.h>
#include <PlotList.h>
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

#include <VisWindow.h>

#include <SimEngineManager.h>
#include <SimFileServer.h>
#include <SimPlotPluginManager.h>
#include <SimOperatorPluginManager.h>

#include <VisItParser.h>
#include <avtExprNodeFactory.h>
#include <ExprParser.h>

#include <avtDatabaseMetaData.h>

#include <avtParallel.h>
#include <cstring>
#include <sstream>

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
                                 , ViewerBase(), allowCommandExecution(true)
#endif
    , viewerInitialized(false), simsource(), rpcNotifier(NULL)
{
    // Install an avtExprNodeFactory object in the parser. This is critical for
    // making the parser create avt versions of the parse objects, which is needed
    // for the various dynamic_cast calls in the expression library to convert
    // the avt*Expr objects.
    Parser *p = new ExprParser(new avtExprNodeFactory());
    // This saves the instance in ParsingExprList::Instance(). Don't free.
    ParsingExprList *l = new ParsingExprList(p);
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
#ifdef SIMV2_VIEWER_INTEGRATION
    if(viewerInitialized)
    {
        delete ViewerBase::GetPlotFactory();
        delete ViewerBase::GetOperatorFactory();

        delete ViewerBase::GetViewerProperties();
        delete ViewerBase::GetViewerState();
        delete ViewerBase::GetViewerMethods();
        delete ViewerBase::GetViewerStateManager();
        delete ViewerBase::GetViewerMessaging();
        delete ViewerBase::GetViewerFileServer();
        delete ViewerBase::GetViewerEngineManager();
    }
#endif
}

// ****************************************************************************
// Method: SimEngine::InitializeViewer
//
// Purpose:
//   Initializes the viewer.
//
// Arguments:
//   plotPlugins     : The list of plot plugins we wanted loaded.
//   operatorPlugins : The list of operator plugins we want loaded.
//   noconfig        : Whether we're running in -noconfig mode.
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
SimEngine::InitializeViewer(const std::vector<std::string> &plotPlugins,
                            const std::vector<std::string> &operatorPlugins,
                            bool noconfig)
{
    StackTimer t0("SimEngine::InitializeViewer");
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

        // Since we're initializing via the viewer, force the network manager
        // to obtain its vis window pointer from the ViewerWindow objects that 
        // will be created from the viewer side. This way, the viewer and engine
        // pieces of VisIt will share the same vis window object, which prevents
        // some weirdness like having to set the vis window size each time we render.
        GetNetMgr()->SetCreateVisWindow(CreateVisWindowCB, this);

        GetViewerProperties()->SetNowin(true);
        GetViewerProperties()->SetNoConfig(noconfig);
        GetViewerProperties()->SetMasterProcess(PAR_UIProcess());
        GetViewerProperties()->SetInSitu(true);
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
        HeavyInitialization(plotPlugins, operatorPlugins);

        // Force scalable rendering.
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
// Method: SimEngine::SimulationTimeStepChanged
//
// Purpose:
//   This method is called when we change timesteps.
//
// Note:       We need this for in situ.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 14 15:02:36 PDT 2016
//
// Modifications:
//
// ****************************************************************************

void
SimEngine::SimulationTimeStepChanged()
{
    // Get the new metadata from the simulation, save it.
    Engine::SimulationTimeStepChanged();

    // Let's the use new metadata to make sure that expressions are up to date.
    ExpressionList newList;

    //
    // Create a new expression list that contains all of the expressions
    // from the main expression list that are not expressions that come
    // from databases.
    //
    ExpressionList *exprList = ParsingExprList::Instance()->GetList();
    for(int i = 0; i < exprList->GetNumExpressions(); ++i)
    {
        const Expression &expr = exprList->GetExpressions(i);
        if(!expr.GetFromDB() && !expr.GetFromOperator())
            newList.AddExpressions(expr);
    }
    // Add the expressions for the database.
    for (int i = 0 ; i < metaData->GetNumberOfExpressions(); ++i)
        newList.AddExpressions(*(metaData->GetExpression(i)));

    // NOTE: adapted from VariableMenuPopulator::GetOperatorCreatedExpressions

    // Iterate over the meshes in the metadata and add operator-created expressions
    // for each relevant mesh.
    avtDatabaseMetaData md2 = *metaData;
    md2.GetExprList() = newList;
    for(int j = 0; j < GetOperatorPluginManager()->GetNEnabledPlugins(); j++)
    {
        std::string id(GetOperatorPluginManager()->GetEnabledID(j));
        CommonOperatorPluginInfo *ComInfo = GetOperatorPluginManager()->GetCommonPluginInfo(id);
        ExpressionList *fromOperators = ComInfo->GetCreatedExpressions(&md2);
        if(fromOperators != NULL)
        {
            for(int k = 0; k < fromOperators->GetNumExpressions(); k++)
                newList.AddExpressions(fromOperators->GetExpressions(k));
            delete fromOperators;
        }
    }

    // Stash the expressions.
    if(newList != *exprList)
        *exprList = newList;
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
    StackTimer t0("SimEngine::OpenDatabase");

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

    if(md == NULL)
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
    const char *mName = "SimEngine::GetSILForState: ";
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
//    Brad Whitlock, Fri Aug 14 11:51:02 PDT 2015
//    Added support for an option list and for setting the write group size.
//    Make sure the requested plugin is loaded.
//
// ****************************************************************************

bool
SimEngine::ExportDatabase(const std::string &filename, const std::string &format,
                          const stringVector &vars, const DBOptionsAttributes &opt)
{
    bool retval = false;

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

    // Make sure the plugin is loaded.
    GetNetMgr()->GetDatabasePluginManager()->LoadSinglePluginNow(id);

    std::string dName(FileFunctions::Dirname(filename));
    std::string fName(FileFunctions::Basename(filename));
    if(dName.empty() || dName == ".")
        dName = FileFunctions::GetCurrentWorkingDirectory();

    // Get some values for the export from the option list.
    int writeUsingGroups = 0;
    int groupSize = PAR_Size();
    TRY
    {
        if(opt.FindIndex("EXPORT_WRITE_USING_GROUPS") != -1)
            writeUsingGroups = opt.GetInt("EXPORT_WRITE_USING_GROUPS");
        if(opt.FindIndex("EXPORT_GROUP_SIZE") != -1)
            groupSize = opt.GetInt("EXPORT_GROUP_SIZE");
    }
    CATCHALL
    {
    }
    ENDTRY

    // The database options we'll put into the export.
    DBOptionsAttributes exportOptions;

    // Get the plugin's default write attributes and store them in exportOptions.
    EngineDatabasePluginInfo *info = GetNetMgr()->GetDatabasePluginManager()->
        GetEnginePluginInfo(id);
    if (info != NULL)
    {
        DBOptionsAttributes *writeOptions = info->GetWriteOptions();
        if(writeOptions != NULL)
        {
            exportOptions = *writeOptions;
            delete writeOptions;
        }
    }
    else
    {
        debug5 << "Could not get write options for " << id << endl;
    }

    // Merge the user-specified options into the exportOptions. Values that
    // have the same keys as the options already there will override the
    // default options.
    exportOptions.Merge(opt);
    debug5 << "exportOptions = " << exportOptions << endl;

    // Fill in the export db attributes.
    ExportDBAttributes atts;
    atts.SetAllTimes(false);
    atts.SetDb_type(name);
    atts.SetDb_type_fullname(id);
    atts.SetDirname(dName);
    atts.SetFilename(fName);
    atts.SetVariables(vars);
    atts.SetWriteUsingGroups(writeUsingGroups > 0);
    atts.SetGroupSize(groupSize);
    atts.SetOpts(exportOptions);

#ifdef SIMV2_VIEWER_INTEGRATION
    if(viewerInitialized)
    {
        // Set the export db attributes into the state and export the database.
        ExportDBAttributes *eAtts = GetViewerState()->GetExportDBAttributes();
        *eAtts = atts;
        eAtts->Notify();
        GetViewerMethods()->ExportDatabase();
        retval = true;
    }
    else
    {
#endif
        // Serialize the export db attributes using the XML form so we can 
        // send them to the viewer as a string command.
        std::stringstream cmd;
        cmd << "ExportDatabase:";
        SingleAttributeConfigManager mgr(&atts);
        mgr.Export(cmd);
        SimulationInitiateCommand(cmd.str());
        retval = true;
#ifdef SIMV2_VIEWER_INTEGRATION
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
//   Brad Whitlock, Tue Jul 17 16:37:46 PDT 2018
//   Broadcast the session file contents if we can for batch.
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
            std::string sessionContents, hostname;
            if(PAR_Rank() == 0)
                FileFunctions::ReadTextFile(filename, sessionContents);
            int s = static_cast<int>(sessionContents.size());
            BroadcastInt(s);
            if(s > 0)
                BroadcastString(sessionContents, PAR_Rank());

            if(sessionContents.empty())
            {
                GetViewerMethods()->
                    ImportEntireStateWithDifferentSources(filename, false, sources, hostname);
            }
            else
            {
                // Load the session from the buffer.
                GetViewerMethods()->
                    ImportEntireStateWithDifferentSourcesFromString(sessionContents,
                        sources);
            }
            retval = true;
        }
        CATCHALL
        { 
            retval = false;
        }
        ENDTRY
    }
    else
    {
#endif
        std::string cmd("RestoreSession:");
        cmd.append(filename);
        SimulationInitiateCommand(cmd);
        retval = true;
#ifdef SIMV2_VIEWER_INTEGRATION
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
//   Brad Whitlock, Wed Sep 30 12:12:48 PDT 2015
//   Don't get the current working directory. Instead, pass a basically empty
//   working directory and set the "output to current directory" flag accordingly.
//   This lets simulations save client-side for interactive connections when
//   they do not specify an output path.
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
        else if(format == VISIT_IMAGEFORMAT_EXR)
            fmt = SaveWindowAttributes::EXR;
        else
            fmt = SaveWindowAttributes::TIFF;

        std::string dName(FileFunctions::Dirname(filename));
        std::string fName(FileFunctions::Basename(filename));
        bool outputCurrentDirectory = (dName.empty() || dName == ".");

#ifdef SIMV2_VIEWER_INTEGRATION
        // Viewer based method.
        if(viewerInitialized)
        {
            SaveWindowAttributes *swa = GetViewerState()->GetSaveWindowAttributes();
            swa->SetFileName(fName);
            swa->SetOutputToCurrentDirectory(outputCurrentDirectory);
            swa->SetOutputDirectory(dName);
            swa->SetFamily(false);
            swa->SetFormat(fmt);
            swa->SetWidth(w);
            swa->SetHeight(h);
            swa->SetSaveTiled(false);
            swa->SetScreenCapture(false);
            swa->SetResConstraint(SaveWindowAttributes::NoConstraint);
            swa->Notify();

            GetViewerMethods()->SaveWindow();

            retval = true;
        }
        else
        {
#endif
            // Send a message to the viewer indicating we want it to save an image.
            std::string f(SaveWindowAttributes::FileFormat_ToString(fmt));
            char cmd[2048];
            snprintf(cmd, 2048, "SaveWindow:%s:%s:%d:%d:%s",
                dName.c_str(), fName.c_str(), w, h, f.c_str());

            debug5 << "SaveWindow" << endl;
            debug5 << "\toutputDirectory = " << dName << endl;
            debug5 << "\tfilename = " << fName << endl;
            debug5 << "\twidth = " << w << endl;
            debug5 << "\theight = " << h << endl;
            debug5 << "\tformat = " << f << endl;

            SimulationInitiateCommand(cmd);
            retval = true;
#ifdef SIMV2_VIEWER_INTEGRATION
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
//    Brad Whitlock, Thu Jul 30 13:33:55 PDT 2015
//    Execute AddPlot through the viewer plot list directly so we can better
//    know whether the plot was successfully added.
//
// ****************************************************************************

bool
SimEngine::AddPlot(const std::string &plotType, const std::string &var)
{
    StackTimer t0("SimEngine::AddPlot");

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

            // Go directly through the plot list so we can know if there was an error.
            bool replacePlots = GetViewerState()->GetGlobalAttributes()->GetReplacePlots();
            bool applyOperator = false;
            bool applySelection = GetViewerState()->GetGlobalAttributes()->GetApplySelection();
            bool inheritSILRestriction = GetViewerState()->GetGlobalAttributes()->
                                         GetNewPlotsInheritSILRestriction();

            ViewerPlotList *pL = ViewerWindowManager::Instance()->GetActiveWindow()->GetPlotList();
            retval = pL->AddPlot(plotIndex, var.c_str(), replacePlots, applyOperator,
                        inheritSILRestriction, applySelection) >= 0;
        }
        else
        {
#endif
            // Send the viewer a message to add  plot.
            char cmd[200];
            snprintf(cmd, 200, "AddPlot:%s:%s", plotType.c_str(), var.c_str());
            SimulationInitiateCommand(cmd);
            retval = true;
#ifdef SIMV2_VIEWER_INTEGRATION
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
        int operatorIndex = GetNetMgr()->GetOperatorPluginManager()->GetEnabledIndex(id);

#ifdef SIMV2_VIEWER_INTEGRATION
        // Viewer based method.
        if(viewerInitialized)
        {
            bool applyOperatorSave = GetViewerState()->GetGlobalAttributes()->GetApplyOperator();
            GetViewerState()->GetGlobalAttributes()->SetApplyOperator(applyToAll);

// cout << "Viewer-based AddOperator(" << operatorIndex << "=" << id << ", " << var << ")" << endl;
            GetViewerMethods()->AddOperator(operatorIndex);

            GetViewerState()->GetGlobalAttributes()->SetApplyOperator(applyOperatorSave);
            retval = true;
        }
        else
        {
#endif
            // Send the viewer a message to add an operator.
            char cmd[200];
            snprintf(cmd, 200, "AddOperator:%s:%d", operatorType.c_str(), applyToAll?1:0);
            SimulationInitiateCommand(cmd);
            retval = true;
#ifdef SIMV2_VIEWER_INTEGRATION
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
//   Brad Whitlock, Wed Mar 14 17:45:48 PDT 2018
//   Disable command execution during DrawPlots. We can end up in situations
//   where an UpdateFrame indirectly recurses into parts of the viewer that
//   try to change the SR mode. This can make it try and use network id -1
//   to get data back from the "engine", which throws an exception.
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
            // Do not allow command execution during the DrawPlots.
            allowCommandExecution = false;

//cout << "Viewer-based DrawPlots()" << endl;
            GetViewerMethods()->DrawPlots();

            // DrawPlots may have resulted in some internal commands
            // to process. Do them now.
            allowCommandExecution = true;
            SimEngine::GetViewerMessaging()->ProcessCommands();

            retval = true;
        }
        else
        {
#endif
            SimulationInitiateCommand("DrawPlots");
            retval = true;
#ifdef SIMV2_VIEWER_INTEGRATION
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
        else
        {
#endif
            SimulationInitiateCommand("DeleteActivePlots");
            retval = true;
#ifdef SIMV2_VIEWER_INTEGRATION
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
// Method: SimEngine::SetActivePlots
//
// Purpose:
//   Set the active plots.
//
// Arguments:
//   ids : The plot ids.
//   nids : The number of ids.
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
SimEngine::SetActivePlots(const int *ids, int nids)
{
    bool retval = false;

    TRY
    {
#ifdef SIMV2_VIEWER_INTEGRATION
        // Viewer based method.
        if(viewerInitialized)
        {
            if(ids != NULL && nids > 0)
            {
                intVector activePlotIds;
                for(int i = 0; i < nids; ++i)
                    activePlotIds.push_back(ids[i]);
                GetViewerMethods()->SetActivePlots(activePlotIds);
            }
            retval = true;
        }
        else
        {
#endif
            std::string cmd("SetActivePlots");
            char tmp[10];
            for(int i = 0; i < nids; ++i)
            {
                snprintf(tmp, 10, ":%d", ids[i]);
                cmd.append(tmp);
            }
            SimulationInitiateCommand(cmd);
            retval = true;
#ifdef SIMV2_VIEWER_INTEGRATION
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
// Method: SimEngine::ChangePlotVar
//
// Purpose:
//   Changes the plot variable.
//
// Arguments:
//   var : The new plot variable.
//   all : Whether to change the variable on all plots.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 14 14:55:33 PST 2017
//
// Modifications:
//
// ****************************************************************************

bool
SimEngine::ChangePlotVar(const char *var, int all)
{
    bool retval = false;

    TRY
    {
#ifdef SIMV2_VIEWER_INTEGRATION
        // Viewer based method.
        if(viewerInitialized)
        {
            if(var != NULL)
            {
                if(all)
                {
                    int np = GetViewerState()->GetPlotList()->GetNumPlots();
                    intVector activePlotIds;
                    for(int i = 0; i < np; ++i)
                        activePlotIds.push_back(i);
                    GetViewerMethods()->SetActivePlots(activePlotIds);
                }

                GetViewerMethods()->ChangeActivePlotsVar(var);
            }
            retval = true;
        }
        else
        {
#endif
            char cmd[1000];
            snprintf(cmd, 1000, "ChangePlotVar:%s:%d", var, all);
            SimulationInitiateCommand(cmd);
            retval = true;
#ifdef SIMV2_VIEWER_INTEGRATION
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
// Method: SetAttributeSubjectValues
//
// Purpose:
//   Set some field data into the attribute subject.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb  2 14:57:36 PST 2015
//
// Modifications:
//
// ****************************************************************************

template <class T>
static std::vector<T> makevector(const T *val, int nval)
{
    std::vector<T> vec;
    vec.reserve(nval);
    for(int i = 0; i < nval; ++i)
        vec.push_back(val[i]);
    return vec;
}

static bool 
SetAttributeSubjectValues(AttributeSubject *atts, 
    const std::string &name, int fieldType, void *fieldVal, int fieldLen)
{
    const char *mName = "SetAttributeSubjectValues: ";
    bool retval = true;
    int fIndex = atts->FieldNameToIndex(name);
    if(fIndex < 0 || fieldVal == NULL)
    {
        debug5 << mName << "Failed to look up field name: " << name << endl;
        return false;
    }
    AttributeGroup::FieldType ft = atts->GetFieldType(fIndex);

    if(fieldType == VISIT_FIELDTYPE_CHAR)
    {
        const char *val = (const char *)fieldVal;
        if(ft == AttributeGroup::FieldType_char)
            retval = atts->SetValue(name, *val);
        // "casts"
        else if(ft == AttributeGroup::FieldType_uchar)
            retval = atts->SetValue(name, (unsigned char)*val);
        else if(ft == AttributeGroup::FieldType_bool)
            retval = atts->SetValue(name, *val > 0);
        else if(ft == AttributeGroup::FieldType_int)
            retval = atts->SetValue(name, (int)*val);
        else if(ft == AttributeGroup::FieldType_long)
            retval = atts->SetValue(name, (long)*val);
        else if(ft == AttributeGroup::FieldType_float)
            retval = atts->SetValue(name, (float)*val);
        else if(ft == AttributeGroup::FieldType_double)
            retval = atts->SetValue(name, (double)*val);
        else
        {
            debug5 << mName << "Could not set array or vector types from char: " << name << "=" << *val << endl;
            retval = false;
        }
    }
    else if(fieldType == VISIT_FIELDTYPE_UNSIGNED_CHAR)
    {
        const unsigned char *val = (const unsigned char *)fieldVal;
        if(ft == AttributeGroup::FieldType_uchar)
            retval = atts->SetValue(name, *val);
        // "casts"
        else if(ft == AttributeGroup::FieldType_char)
            retval = atts->SetValue(name, (char)*val);
        else if(ft == AttributeGroup::FieldType_bool)
            retval = atts->SetValue(name, *val > 0);
        else if(ft == AttributeGroup::FieldType_int)
            retval = atts->SetValue(name, (int)*val);
        else if(ft == AttributeGroup::FieldType_long)
            retval = atts->SetValue(name, (long)*val);
        else if(ft == AttributeGroup::FieldType_float)
            retval = atts->SetValue(name, (float)*val);
        else if(ft == AttributeGroup::FieldType_double)
            retval = atts->SetValue(name, (double)*val);
        else
        {
            debug5 << mName << "Could not set array or vector types from unsigned char: " << name << "=" << *val << endl;
            retval = false;
        }
    }
    else if(fieldType == VISIT_FIELDTYPE_INT)
    {
        const int *val = (const int *)fieldVal;
        if(ft == AttributeGroup::FieldType_int)
           retval = atts->SetValue(name, *val);
        // "casts"
        else if(ft == AttributeGroup::FieldType_enum)
            retval = atts->SetValue(name, *val);
        else if(ft == AttributeGroup::FieldType_char)
            retval = atts->SetValue(name, (char)*val);
        else if(ft == AttributeGroup::FieldType_uchar)
            retval = atts->SetValue(name, (unsigned char)*val);
        else if(ft == AttributeGroup::FieldType_bool)
            retval = atts->SetValue(name, *val > 0);
        else if(ft == AttributeGroup::FieldType_long)
            retval = atts->SetValue(name, (long)*val);
        else if(ft == AttributeGroup::FieldType_float)
            retval = atts->SetValue(name, (float)*val);
        else if(ft == AttributeGroup::FieldType_double)
            retval = atts->SetValue(name, (double)*val);
        else
        {
            debug5 << mName << "Could not set array or vector types from int: " << name << "=" << *val << endl;
            retval = false;
        }
    }
    else if(fieldType == VISIT_FIELDTYPE_LONG)
    {
        const long *val = (const long *)fieldVal;
        if(ft == AttributeGroup::FieldType_long)
            retval = atts->SetValue(name, *val);
        // "casts"
        else if(ft == AttributeGroup::FieldType_char)
            retval = atts->SetValue(name, (char)*val);
        else if(ft == AttributeGroup::FieldType_uchar)
            retval = atts->SetValue(name, (unsigned char)*val);
        else if(ft == AttributeGroup::FieldType_bool)
            retval = atts->SetValue(name, *val > 0);
        else if(ft == AttributeGroup::FieldType_int)
            retval = atts->SetValue(name, (int)*val);
        else if(ft == AttributeGroup::FieldType_float)
            retval = atts->SetValue(name, (float)*val);
        else if(ft == AttributeGroup::FieldType_double)
            retval = atts->SetValue(name, (double)*val);
        else
        {
            debug5 << mName << "Could not set array or vector types from long: " << name << "=" << *val << endl;
            retval = false;
        }
    }
    else if(fieldType == VISIT_FIELDTYPE_FLOAT)
    {
        const float *val = (const float *)fieldVal;
        if(ft == AttributeGroup::FieldType_float)
            retval = atts->SetValue(name, *val);
        // "casts"
        else if(ft == AttributeGroup::FieldType_char)
            retval = atts->SetValue(name, (char)*val);
        else if(ft == AttributeGroup::FieldType_uchar)
            retval = atts->SetValue(name, (unsigned char)*val);
        else if(ft == AttributeGroup::FieldType_bool)
            retval = atts->SetValue(name, *val > 0);
        else if(ft == AttributeGroup::FieldType_int)
            retval = atts->SetValue(name, (int)*val);
        else if(ft == AttributeGroup::FieldType_long)
            retval = atts->SetValue(name, (long)*val);
        else if(ft == AttributeGroup::FieldType_double)
            retval = atts->SetValue(name, (double)*val);
        else
        {
            debug5 << mName << "Could not set array or vector types from float: " << name << "=" << *val << endl;
            retval = false;
        }
    }
    else if(fieldType == VISIT_FIELDTYPE_DOUBLE)
    {
        const double *val = (const double *)fieldVal;

        if(ft == AttributeGroup::FieldType_double)
            retval = atts->SetValue(name, *val);
        // "casts"
        else if(ft == AttributeGroup::FieldType_char)
            retval = atts->SetValue(name, (char)*val);
        else if(ft == AttributeGroup::FieldType_uchar)
            retval = atts->SetValue(name, (unsigned char)*val);
        else if(ft == AttributeGroup::FieldType_bool)
            retval = atts->SetValue(name, *val > 0);
        else if(ft == AttributeGroup::FieldType_int)
            retval = atts->SetValue(name, (int)*val);
        else if(ft == AttributeGroup::FieldType_long)
            retval = atts->SetValue(name, (long)*val);
        else if(ft == AttributeGroup::FieldType_float)
            retval = atts->SetValue(name, (float)*val);
        else
        {
            debug5 << mName << "Could not set array or vector types from double: " << name << "=" << *val << endl;
            retval = false;
        }
    }
    else if(fieldType == VISIT_FIELDTYPE_STRING)
    {
        std::string val((const char *)fieldVal);
        retval = atts->SetValue(name, val);
    }
    else if(fieldLen <= 0)
    {
        debug5 << mName << "Array or vector with fieldLen=" << fieldLen << endl;
        retval = false;
    }
    // Array and vector
    else if(fieldType == VISIT_FIELDTYPE_CHAR_ARRAY)
    {
        const char *val = (const char *)fieldVal;
        if(ft == AttributeGroup::FieldType_charArray)
            retval = atts->SetValue(name, val, fieldLen);
        else if(ft == AttributeGroup::FieldType_charVector)
            retval = atts->SetValue(name, makevector(val, fieldLen));
        else
        {
            debug5 << mName << "Could not set " << name << " using char array or vector." << endl;
            retval = false;
        }
    }
    else if(fieldType == VISIT_FIELDTYPE_UNSIGNED_CHAR_ARRAY)
    {
        const unsigned char *val = (const unsigned char *)fieldVal;
        if(ft == AttributeGroup::FieldType_ucharArray)
            retval = atts->SetValue(name, val, fieldLen);
        else if(ft == AttributeGroup::FieldType_ucharVector)
            retval = atts->SetValue(name, makevector(val, fieldLen));
        else
        {
            debug5 << mName << "Could not set " << name << " using unsigned char array or vector." << endl;
            retval = false;
        }
    }
    else if(fieldType == VISIT_FIELDTYPE_INT_ARRAY)
    {
        const int *val = (const int *)fieldVal;
        if(ft == AttributeGroup::FieldType_intArray)
            retval = atts->SetValue(name, val, fieldLen);
        else if(ft == AttributeGroup::FieldType_intVector)
            retval = atts->SetValue(name, makevector(val, fieldLen));
        else
        {
            debug5 << mName << "Could not set " << name << " using int array or vector." << endl;
            retval = false;
        }
    }
    else if(fieldType == VISIT_FIELDTYPE_LONG_ARRAY)
    {
        const long *val = (const long *)fieldVal;
        if(ft == AttributeGroup::FieldType_longArray)
            retval = atts->SetValue(name, val, fieldLen);
        else if(ft == AttributeGroup::FieldType_longVector)
            retval = atts->SetValue(name, makevector(val, fieldLen));
        else
        {
            debug5 << mName << "Could not set " << name << " using long array or vector." << endl;
            retval = false;
        }
    }
    else if(fieldType == VISIT_FIELDTYPE_FLOAT_ARRAY)
    {
        const float *val = (const float *)fieldVal;
        if(ft == AttributeGroup::FieldType_floatArray)
            retval = atts->SetValue(name, val, fieldLen);
        else if(ft == AttributeGroup::FieldType_floatVector)
            retval = atts->SetValue(name, makevector(val, fieldLen));
        else
        {
            debug5 << mName << "Could not set " << name << " using float array or vector." << endl;
            retval = false;
        }
    }
    else if(fieldType == VISIT_FIELDTYPE_DOUBLE_ARRAY)
    {
        const double *val = (const double *)fieldVal;
        if(ft == AttributeGroup::FieldType_doubleArray)
            retval = atts->SetValue(name, val, fieldLen);
        else if(ft == AttributeGroup::FieldType_doubleVector)
            retval = atts->SetValue(name, makevector(val, fieldLen));
        else
        {
            debug5 << mName << "Could not set " << name << " using double array or vector." << endl;
            retval = false;
        }
    }
    else if(fieldType == VISIT_FIELDTYPE_STRING_ARRAY)
    {
        const char **val = (const char **)fieldVal;
        stringVector s;
        s.resize(fieldLen);
        for(int i = 0; i < fieldLen; ++i)
            s[i] = std::string(val[i]);
        if(ft == AttributeGroup::FieldType_stringArray)
            retval = atts->SetValue(name, &s[0], fieldLen);
        else if(ft == AttributeGroup::FieldType_stringVector)
            retval = atts->SetValue(name, s);
        else
        {
            debug5 << mName << "Could not set " << name << " using string array or vector." << endl;
            retval = false;
        }
    }
    else
    {
        debug5 << mName << "Unsuppored field type." << endl;
        retval = false;
    }

    return retval;
}

// ****************************************************************************
// Method: SimEngine::SetPlotOptions
//
// Purpose:
//   Set the plot attributes for the active plots.
//
// Arguments:
//   fieldName : The name of the field to set.
//   fieldType : The type of the data we're passing in.
//   fieldVal  : A pointer to the field data we're passing in.
//   fieldLen  : The length of the field data (if it is an array).   
//
// Returns:    true on success; false on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb  2 14:14:37 PST 2015
//
// Modifications:
//
// ****************************************************************************

bool SimEngine::SetPlotOptions(const std::string &fieldName, 
     int fieldType, void *fieldVal, int fieldLen)
{
    bool retval = false;

    TRY
    {
#ifdef SIMV2_VIEWER_INTEGRATION
        // Viewer based method.
        if(viewerInitialized)
        {
            if(fieldVal != NULL)
            {
                // Get the plot type of the plot from the first active plot
                // in the plot list.
                int activePlotType = -1;
                for(int i = 0; i < GetViewerState()->GetPlotList()->GetNumPlots(); ++i)
                {
                    const Plot &p = GetViewerState()->GetPlotList()->GetPlots(i);
                    if(i == 0)
                        activePlotType = p.GetPlotType();
                    if(p.GetActiveFlag())
                    {
                        activePlotType = p.GetPlotType();
                        break;
                    }
                }
                if(activePlotType != -1)
                {
                    AttributeSubject *atts = GetViewerState()->GetPlotAttributes(activePlotType);
                    if(atts != NULL)
                    {
                        if(SetAttributeSubjectValues(atts, fieldName, fieldType, fieldVal, fieldLen))
                        {
#if 0
                            cout << *atts << endl;
#endif
                            GetViewerMethods()->SetPlotOptions(activePlotType);
                            retval = true;
                        }
                    }
                }
            }
        }
        else
        {
#endif
            debug5 << "SimEngine::SetPlotOptions is just implemented for batch mode." << endl;
#ifdef SIMV2_VIEWER_INTEGRATION
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
// Method: SimEngine::SetOperatorOptions
//
// Purpose:
//   Set the plot attributes for the active plots.
//
// Arguments:
//   fieldName : The name of the field to set.
//   fieldType : The type of the data we're passing in.
//   fieldVal  : A pointer to the field data we're passing in.
//   fieldLen  : The length of the field data (if it is an array).   
//
// Returns:    true on success; false on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb  2 14:14:37 PST 2015
//
// Modifications:
//
// ****************************************************************************

bool SimEngine::SetOperatorOptions(const std::string &fieldName, 
     int fieldType, void *fieldVal, int fieldLen)
{
    bool retval = false;

    TRY
    {
#ifdef SIMV2_VIEWER_INTEGRATION
        // Viewer based method.
        if(viewerInitialized)
        {
            if(fieldVal != NULL)
            {
                // Get the plot type of the plot from the first active plot
                // in the plot list.
                int activePlotType = -1, plotIndex = -1;
                for(int i = 0; i < GetViewerState()->GetPlotList()->GetNumPlots(); ++i)
                {
                    const Plot &p = GetViewerState()->GetPlotList()->GetPlots(i);
                    if(i == 0)
                    {
                        activePlotType = p.GetPlotType();
                        plotIndex = i;
                    }
                    if(p.GetActiveFlag())
                    {
                        activePlotType = p.GetPlotType();
                        plotIndex = i;
                        break;
                    }
                }
                // Get the active operator on the active plot.
                int activeOperatorType = -1;
                if(activePlotType != -1)
                {
                    const Plot &p = GetViewerState()->GetPlotList()->GetPlots(plotIndex);
                    if(!p.GetOperators().empty())
                    {
                        activeOperatorType = p.GetOperators()[p.GetActiveOperator()];
                    }
                    else
                    {
                        // The active plot had no operators, get the first plot that has some operators.
                        for(int i = 0; i < GetViewerState()->GetPlotList()->GetNumPlots(); ++i)
                        {
                            const Plot &p2 = GetViewerState()->GetPlotList()->GetPlots(i);
                            if(!p2.GetOperators().empty())
                            {
                                activeOperatorType = p2.GetOperators()[p2.GetActiveOperator()];
                                break;
                            }
                        }
                    }
                }

                if(activeOperatorType != -1)
                {
                    AttributeSubject *atts = GetViewerState()->GetOperatorAttributes(activeOperatorType);
                    if(atts != NULL)
                    {
                        if(SetAttributeSubjectValues(atts, fieldName, fieldType, fieldVal, fieldLen))
                        {
#if 0
                            cout << *atts << endl;
#endif
                            GetViewerMethods()->SetOperatorOptions(activeOperatorType);
                            retval = true;
                        }
                    }
                }
            }
        }
        else
        {
#endif
            debug5 << "SimEngine::SetOperatorOptions is just implemented for batch mode." << endl;
#ifdef SIMV2_VIEWER_INTEGRATION
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
// Method: SimEngine::SetView2D
//
// Purpose:
//   Sets the 2D view.
//
// Arguments:
//   view : The view object we're using to set the view.
//
// Returns:    true on success; false on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  1 16:25:34 PDT 2017
//
// Modifications:
//
// ****************************************************************************

bool
SimEngine::SetView2D(const View2DAttributes &view)
{
    bool retval = false;

    TRY
    {
#ifdef SIMV2_VIEWER_INTEGRATION
        // Viewer based method.
        if(viewerInitialized)
        {
            View2DAttributes *v = GetViewerState()->GetView2DAttributes();
            *v = view;
            v->Notify();

            GetViewerMethods()->SetView2D();

            retval = true;
        }
        else
        {
#endif
            // Send a message to the viewer indicating we want it to set the view.
            std::stringstream cmd;
            cmd << "SetView2D:";
            SingleAttributeConfigManager mgr(GetViewerState()->GetView2DAttributes());
            mgr.Export(cmd);
            SimulationInitiateCommand(cmd.str());
            retval = true;
#ifdef SIMV2_VIEWER_INTEGRATION
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

bool
SimEngine::GetView2D(View2DAttributes &view)
{
    bool retval = false;

    TRY
    {
#ifdef SIMV2_VIEWER_INTEGRATION
        // Viewer based method.
        if(viewerInitialized)
        {
            View2DAttributes *v = GetViewerState()->GetView2DAttributes();
            view = *v;
            debug5 << view << endl;

            retval = true;
        }
        else
        {
#endif
            // TODO: get the network manager's vis window and get the view from it.

            retval = false;
#ifdef SIMV2_VIEWER_INTEGRATION
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
// Method: SimEngine::SetView3D
//
// Purpose:
//   Sets the 3D view.
//
// Arguments:
//   view : The view object we're using to set the view.
//
// Returns:    true on success; false on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  1 16:25:34 PDT 2017
//
// Modifications:
//
// ****************************************************************************

bool
SimEngine::SetView3D(const View3DAttributes &view)
{
    bool retval = false;

    TRY
    {
#ifdef SIMV2_VIEWER_INTEGRATION
        // Viewer based method.
        if(viewerInitialized)
        {
            View3DAttributes *v = GetViewerState()->GetView3DAttributes();
            *v = view;
            v->Notify();

            GetViewerMethods()->SetView3D();

            retval = true;
        }
        else
        {
#endif
            // Send a message to the viewer indicating we want it to set the view.
            std::stringstream cmd;
            cmd << "SetView3D:";
            SingleAttributeConfigManager mgr(GetViewerState()->GetView3DAttributes());
            mgr.Export(cmd);
            SimulationInitiateCommand(cmd.str());
            retval = true;
#ifdef SIMV2_VIEWER_INTEGRATION
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

bool
SimEngine::GetView3D(View3DAttributes &view)
{
    bool retval = false;

    TRY
    {
#ifdef SIMV2_VIEWER_INTEGRATION
        // Viewer based method.
        if(viewerInitialized)
        {
            View3DAttributes *v = GetViewerState()->GetView3DAttributes();
            view = *v;
            debug5 << view << endl;

            retval = true;
        }
        else
        {
#endif
            // TODO: get the network manager's vis window and get the view from it.

            retval = false;
#ifdef SIMV2_VIEWER_INTEGRATION
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
//   plotPlugins     : The list of plot plugins that we want to load.
//   operatorPlugins : The list of operator plugins that we want to load.
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
SimEngine::HeavyInitialization(const std::vector<std::string> &plotPlugins,
    const std::vector<std::string> &operatorPlugins)
{
    // The viewer likes to have its plugins loaded... I'd like to not have to
    // do this...
    LoadPlotPlugins(plotPlugins);
    LoadOperatorPlugins(operatorPlugins);

    ViewerQueryManager::Instance()->InitializeQueryList();

    GetViewerStateManager()->ConnectPluginDefaultState();

    AddInitialWindows();

    // Process settings. This also does stuff like read in external color tables.
    GetViewerStateManager()->ProcessSettings();
}

// ****************************************************************************
// Method: SimEngine::RestrictPlugins
//
// Purpose:
//   Restrict the plugins that we'll load to a specific list to speed startup.
//
//
// Arguments:
//   mgr : A plugin manager.
//   ids : The list of ids or names of the plugins that we want to keep on.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 17 17:04:50 PDT 2015
//
// Modifications:
//
// ****************************************************************************

void
SimEngine::RestrictPlugins(PluginManager *mgr,
    const std::vector<std::string> &idsOrNames)
{
    // If we passed a 1-element vector with "None" then let's disable all plugins.
    bool none = idsOrNames.size() == 1 && 
#if defined(_WIN32)
                (stricmp(idsOrNames[0].c_str(), "None") == 0);
#else
                (strcasecmp(idsOrNames[0].c_str(), "None") == 0);
#endif

    if(none)
    {
        // Disable all plugins.
        for(int i =0; i < mgr->GetNAllPlugins(); ++i) 
        {
            std::string thisId = mgr->GetAllID(i);
            mgr->DisablePlugin(thisId);
        }
    }
    else if(!idsOrNames.empty())
    {
        // Make the list of plugins we want to enable.
        std::vector<std::string> ids;
        for(size_t i = 0; i < idsOrNames.size(); ++i)
        {
            std::string id;
            for(int j =0; j < mgr->GetNAllPlugins(); ++j) 
            {
                std::string thisId = mgr->GetAllID(j);
                if(idsOrNames[i] == thisId)
                    ids.push_back(thisId);
                // or, do we have a matching name?
                else if(idsOrNames[i] == mgr->GetPluginName(thisId))
                    ids.push_back(thisId);
            }
        }

        // Disable all plugins.
        for(int i =0; i < mgr->GetNAllPlugins(); ++i) 
        {
            std::string thisId = mgr->GetAllID(i);
            mgr->DisablePlugin(thisId);
        }

        // Re-enable the plugins that we do want to load.
        for(size_t i = 0; i < ids.size(); ++i)
            mgr->EnablePlugin(ids[i]);
    }
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
SimEngine::LoadPlotPlugins(const std::vector<std::string> &plotPlugins)
{
    int total  = visitTimer->StartTimer();
    int timeid = visitTimer->StartTimer();

    //
    // It's safe to load the plugins now
    //
    TRY
    {
        RestrictPlugins(GetPlotPluginManager(), plotPlugins);

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
SimEngine::LoadOperatorPlugins(const std::vector<std::string> &operatorPlugins)
{
    int total = visitTimer->StartTimer();
    int timeid = visitTimer->StartTimer();

    //
    // It's safe to load the plugins now
    //
    TRY
    {
        RestrictPlugins(GetOperatorPluginManager(), operatorPlugins);

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
    StackTimer t0("SimEngine::AddInitialWindows");

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
//   Brad Whitlock, Wed Mar 14 17:45:48 PDT 2018
//   Check to see if we're allowing command execution.
//
// ****************************************************************************

void
SimEngine::CommandNotificationCallback(void *cbdata, int)
{
    SimEngine *This = (SimEngine *)cbdata;
    if(This->allowCommandExecution)
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
        std::string name = ViewerRPC::ViewerRPCType_ToString(
            GetViewerState()->GetViewerRPC()->GetRPCType());
        StackTimer t0(name);

        ViewerWindowManager::Instance()->GetActiveWindow()->GetActionManager()->
            HandleAction(*GetViewerState()->GetViewerRPC());
    }
}

// ****************************************************************************
// Method: SimEngineManager::CreateVisWindow
//
// Purpose:
//   This method is called when the sim engine's network manager wants to create
//   a vis window. From the viewer side of the sim runtime, we 
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 16 13:11:55 PDT 2017
//
// Modifications:
//
// ****************************************************************************
void
SimEngine::CreateVisWindowCB(int winID, VisWindow *&viswindow, bool &owns, void *cbdata)
{
    SimEngine *s = (SimEngine *)cbdata;
    s->CreateVisWindow(winID, viswindow, owns);
}

void
SimEngine::CreateVisWindow(int winID, VisWindow *&viswindow, bool &owns)
{
    debug5 << "SimEngine::CreateVisWindow: winID = " << winID << endl;
    ViewerWindow *w = ViewerWindowManager::Instance()->GetWindow(winID);
    if(w != NULL)
        viswindow = w->GetVisWindow();
    else
        viswindow = NULL;
    owns = false;
}

#endif
