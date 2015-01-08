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
#include <ViewerStateManager.h>

#include <ViewerConfigManager.h>
#include <ViewerDatabaseCorrelationMethods.h>
#include <ViewerEngineManagerInterface.h>
#include <ViewerFileServerInterface.h>
#include <ViewerMessaging.h>
#include <ViewerOperatorFactory.h>
#include <ViewerPlotFactory.h>
#include <ViewerPlotList.h>
#include <ViewerProperties.h>
#include <ViewerQueryManager.h>
#include <ViewerState.h>
#include <ViewerText.h>
#include <ViewerVariableMethods.h>
#include <ViewerWindowManager.h>

// NOTE: a bunch of these includes are needed because of the CreatState method.
#include <AnnotationAttributes.h>
#include <AnnotationObjectList.h>
#include <AppearanceAttributes.h>
#include <ColorTableAttributes.h>
#include <ExportDBAttributes.h>
#include <FileOpenOptions.h>
#include <GlobalAttributes.h>
#include <InteractorAttributes.h>
#include <LightList.h>
#include <MaterialAttributes.h>
#include <MeshManagementAttributes.h>
#include <MovieAttributes.h>
#include <ParsingExprList.h>
#include <PickAttributes.h>
#include <PluginManagerAttributes.h>
#include <PrinterAttributes.h>
#include <QueryList.h>
#include <QueryOverTimeAttributes.h>
#include <RenderingAttributes.h>
#include <SaveWindowAttributes.h>
#include <ViewCurveAttributes.h>
#include <View2DAttributes.h>
#include <View3DAttributes.h>
#include <ViewerWindowManagerAttributes.h>
#include <WindowInformation.h>

#include <DataNode.h>
#include <DebugStream.h>
#include <FileFunctions.h>
#include <HostProfileList.h>
#include <MachineProfile.h>
#include <InstallationFunctions.h>
#include <SingleAttributeConfigManager.h>
#include <TimingsManager.h>


#include <avtColorTables.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include <snprintf.h>

static void ReadHostProfileCallback(void *, const std::string&,bool,bool,long);
static void CleanHostProfileCallback(void *, const std::string&,bool,bool,long);

// ****************************************************************************
// Method: ViewerStateManager::ViewerStateManager()
//
// Purpose:
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 23:12:37 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerStateManager::ViewerStateManager() : ViewerBase()
{
    configMgr = new ViewerConfigManager;
    configMgr->SetWriteConfigFileCallback(WriteCallback, (void*)this);

    systemSettings = NULL;
    localSettings = NULL;

    originalSystemHostProfileList = NULL;

    correlationMethods = new ViewerDatabaseCorrelationMethods;
    variableMethods = new ViewerVariableMethods;
}

// ****************************************************************************
// Method: ViewerStateManager::~ViewerStateManager()
//
// Purpose:
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 23:12:37 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerStateManager::~ViewerStateManager()
{
    delete configMgr;
    delete systemSettings;
    delete localSettings;
    delete originalSystemHostProfileList;
    delete correlationMethods;
    delete variableMethods;
}

// ****************************************************************************
// Method: ViewerStateManager::CreateState
//
// Purpose: 
//   Creates the viewer's state objects and adds them to the viewerState
//   object, which lets us more easily create copies of the viewer's state.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 23:12:37 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerStateManager::CreateState()
{
    ViewerState *s = GetViewerState();

    // The ViewerState object automatically creates its own state objects in
    // the right order. However, certain objects in the viewer use their own
    // copies and it is those objects that we need to use in the ViewerState
    // object, etc. Let's override the values of some of the objects in the
    // ViewerState object with those of the other viewer objects.
    //
    // Since the important viewer objects now inherit from ViewerBase, they should
    // use the object from ViewerState *ViewerBase::GetViewerState() when possible 
    // instead of maintaining their own objects. If we eventually switch to that
    // paradigm then we can delete this code!

    s->SetColorTableAttributes(avtColorTables::Instance()->GetColorTables(), false);
    s->SetExpressionList(ParsingExprList::Instance()->GetList(), false);
}

// ****************************************************************************
// Method: ViewerStateManager::ConnectDefaultState
//
// Purpose: 
//   Connects initial ViewerState objects to the config manager.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 17 14:57:28 PST 2003
//
// Modifications:
//
// ****************************************************************************

void
ViewerStateManager::ConnectDefaultState()
{
    //
    // Connect objects that can be written to the config file. Note that
    // we don't hook up some fancy mechanism for viewer state because
    // viewer state contains all of the client attributes and some objects
    // have client attributes and default attributes. We want to hook up
    // default attributes if they are available.
    //
    configMgr->Add(GetViewerState()->GetGlobalAttributes());
    configMgr->Add(GetViewerState()->GetSaveWindowAttributes());
    configMgr->Add(GetViewerState()->GetColorTableAttributes());
    configMgr->Add(GetViewerState()->GetExpressionList());
    configMgr->Add(GetViewerState()->GetAnimationAttributes());
    configMgr->Add(ViewerWindowManager::GetAnnotationDefaultAtts());
    configMgr->Add(GetViewerState()->GetViewCurveAttributes());
    configMgr->Add(GetViewerState()->GetView2DAttributes());
    configMgr->Add(GetViewerState()->GetView3DAttributes());
    configMgr->Add(ViewerWindowManager::GetLightListDefaultAtts());
    configMgr->Add(ViewerWindowManager::GetWindowAtts());
    configMgr->Add(GetViewerState()->GetWindowInformation());
    configMgr->Add(GetViewerState()->GetPrinterAttributes());
    configMgr->Add(GetViewerState()->GetRenderingAttributes());
    configMgr->Add(GetMaterialDefaultAtts());
    configMgr->Add(GetMeshManagementDefaultAtts());
    configMgr->Add(ViewerWindowManager::GetDefaultAnnotationObjectList());
    configMgr->Add(ViewerQueryManager::Instance()->GetPickDefaultAtts());
    configMgr->Add(ViewerQueryManager::Instance()->GetQueryOverTimeDefaultAtts());
    configMgr->Add(ViewerWindowManager::Instance()->GetInteractorDefaultAtts());
    configMgr->Add(GetViewerState()->GetMovieAttributes());
    configMgr->Add(GetViewerState()->GetFileOpenOptions());
}

// ****************************************************************************
// Method: ViewerStateManager::ConnectPluginDefaultState
//
// Purpose: 
//   Connects the plugin state objects from ViewerState to the config manager.
//
// Notes: This method must be called after plugin loading.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 17 14:57:28 PST 2003
//
// Modifications:
//
// ****************************************************************************

void
ViewerStateManager::ConnectPluginDefaultState()
{
    for (int i = 0; i < GetPlotFactory()->GetNPlotTypes(); ++i)
    {
        AttributeSubject *defaultAttr = GetPlotFactory()->GetDefaultAtts(i);
        if (defaultAttr != 0)
            configMgr->Add(defaultAttr);
    }
    for (int i = 0; i < GetOperatorFactory()->GetNOperatorTypes(); ++i)
    {
        AttributeSubject *defaultAttr = GetOperatorFactory()->GetDefaultAtts(i);
        if (defaultAttr != 0)
            configMgr->Add(defaultAttr);
    }
}

// ****************************************************************************
// Method: ViewerStateManager::ProcessSettings
//
// Purpose: 
//   Processes the config file settings that were read in.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 17 15:19:27 PST 2003
//
// Modifications:
//   Kathleen Bonnell, Wed Dec 17 14:44:26 PST 2003
//   Added PickAtts.
//
//   Kathleen Bonnell, Wed Mar 31 11:08:05 PST 2004 
//   Added QueryOverTimeAtts.
//
//   Kathleen Bonnell, Wed Aug 18 09:25:33 PDT 2004 
//   Added InteractorAtts.
//
//   Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//   Added mesh management attributes
//
//   Jeremy Meredith, Wed Jan 23 16:31:06 EST 2008
//   We might start an mdserver before reading the config files, so
//   make sure we send default file opening options from the config file
//   to all existing mdservers.
//
//   Brad Whitlock, Tue Apr 14 12:02:10 PDT 2009
//   Use ViewerProperties.
//
//   Jeremy Meredith, Thu Feb 18 15:39:42 EST 2010
//   Host profiles are now handles outside the config manager.
//
// ****************************************************************************

bool
ViewerStateManager::ProcessSettings()
{
    //
    // Make the hooked up state objects set their properties from
    // both the system and local settings.
    //
    configMgr->ProcessConfigSettings(systemSettings);
    configMgr->ProcessConfigSettings(localSettings);

    // Import external color tables.
    if(!GetViewerProperties()->GetNoConfig())
        avtColorTables::Instance()->ImportColorTables();

    // Send the user's config settings to the client.
    configMgr->Notify();

    // Notify the host profile list manually, since it's
    // populated outside the config manager now
    GetViewerState()->GetHostProfileList()->Notify();

    delete systemSettings; systemSettings = 0;

    // Add the appearanceAtts *after* the config settings have been read. This
    // prevents overwriting the attributes and sending them to the client.
    configMgr->Add(GetViewerState()->GetAppearanceAttributes());

    // Add the pluginAtts *after* the config settings have been read.
    // First, tell the client which plugins we've really loaded.
    configMgr->Add(GetViewerState()->GetPluginManagerAttributes());

    // Copy the default annotation attributes into the client annotation
    // attributes.
    ViewerWindowManager::SetClientAnnotationAttsFromDefault();

    // Copy the default material atts to the client material atts
    SetClientMaterialAttsFromDefault();

    // Copy the default pick atts to the client pick atts
    ViewerQueryManager::Instance()->SetClientPickAttsFromDefault();

    // Copy the default time query atts to the client time query atts
    ViewerQueryManager::Instance()->SetClientQueryOverTimeAttsFromDefault();

    // Copy the default time query atts to the client time query atts
    ViewerWindowManager::Instance()->SetClientInteractorAttsFromDefault();

    // Send the queries to the client.
    GetViewerState()->GetQueryList()->Notify();

    // Copy the default mesh management atts to the client material atts
    SetClientMeshManagementAttsFromDefault();

    // If we started an mdserver before the default file open options were
    // obtained from the config file, we need to re-notify mdservers.
    GetViewerFileServer()->BroadcastUpdatedFileOpenOptions();
    GetViewerEngineManager()->UpdateDefaultFileOpenOptions(GetViewerState()->GetFileOpenOptions());

    return localSettings != NULL;
}

// ****************************************************************************
// Method: ViewerStateManager::ProcessLocalSettings
//
// Purpose:
//   Processes any local settings that have been read.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 17:43:54 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerStateManager::ProcessLocalSettings()
{
    const char *mName = "ViewerStateManager::ProcessLocalSettings: ";
    int timeid = visitTimer->StartTimer();

    TRY
    {
        if(localSettings != 0)
        {
            // Get the VisIt node.
            DataNode *visitRoot = localSettings->GetNode("VisIt");
            if(visitRoot == 0)
            {
                debug1 << mName << "Can't read VisIt node." << endl;
                return;
            }

            // Get the viewer node.
            DataNode *viewerNode = visitRoot->GetNode("VIEWER");
            if(viewerNode == 0)
            {
                debug1 << mName << "Can't read VisIt node." << endl;
                return;
            }

            // Get the ViewerSubject node
            DataNode *vsNode = viewerNode->GetNode("ViewerSubject");
            if(vsNode == 0)
            {
                debug1 << mName << "Can't read ViewerSubject node." << endl;
                return;
            }

            // Get the version
            std::string configVersion(VISIT_VERSION);
            DataNode *version = visitRoot->GetNode("Version");
            if(version != 0)
                configVersion = version->AsString();

            // Let the important objects read their settings.
            std::map<std::string, std::string> empty;
            GetViewerFileServer()->SetFromNode(vsNode);
            correlationMethods->SetFromNode(vsNode, empty, configVersion);
            ViewerWindowManager::Instance()->SetFromNode(vsNode, empty, configVersion);
            ViewerQueryManager::Instance()->SetFromNode(vsNode, configVersion);
            ReadEngineSettingsFromNode(vsNode, configVersion);

            delete localSettings;  localSettings = 0;
        }
    }
    CATCHALL
    {
        ; // nothing
    }
    ENDTRY
    visitTimer->StopTimer(timeid, "Processing config file data.");
}

// ****************************************************************************
// Method: ViewerStateManager::ReadConfigFile
//
// Purpose:
//   Reads
//
// Arguments:
//   specifiedConfig : True if the user specified a config file name.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 21:50:36 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerStateManager::ReadConfigFile(bool specifiedConfig)
{
    int timeid = visitTimer->StartTimer();

    //
    // Read the config file and setup the appearance attributes. Note that
    // we call the routine to process the config file settings here because
    // it only has to update the appearance attributes.
    //
    char *configFile = GetSystemConfigFile();
    if (GetViewerProperties()->GetNoConfig())
        systemSettings = NULL;
    else
        systemSettings = configMgr->ReadConfigFile(configFile);
    delete [] configFile;
    std::string configFileName(GetViewerProperties()->GetConfigurationFileName());
    const char *cfn = (configFileName != "") ? configFileName.c_str() : NULL;
    configFile = GetDefaultConfigFile(cfn);
    if (specifiedConfig && strcmp(configFile, cfn) != 0)
    {
        cerr << "\n\nYou specified a config file with the \"-config\" option,"
                " but the config file could not be located.  Note that this "
                "may be because you must fully qualify the directory of the "
                "config file.\n\n\n";
    }
    if (GetViewerProperties()->GetNoConfig())
        localSettings = NULL;
    else
        localSettings = configMgr->ReadConfigFile(configFile);
    delete [] configFile;
    configMgr->Add(GetViewerState()->GetAppearanceAttributes());
    configMgr->Add(GetViewerState()->GetPluginManagerAttributes());
    configMgr->ProcessConfigSettings(systemSettings);
    configMgr->ProcessConfigSettings(localSettings);
    configMgr->ClearSubjects();
    visitTimer->StopTimer(timeid, "Reading config files.");
}

// ****************************************************************************
//  Method: ViewerStateManager::WriteConfigFile
//
//  Purpose:
//    Execute the WriteConfigFile RPC.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Sep 28 11:07:44 PDT 2000
//
//  Modifications:
//    Brad Whitlock, Fri Nov 2 11:26:32 PDT 2001
//    Added code to tell the ViewerWindowManager to gather the size, location
//    of its windows so it can be saved.
//
//    Brad Whitlock, Fri May 16 15:47:35 PST 2003
//    I made it capable of saving to the config file that was specified
//    with the -config option when VisIt started. If no config file was ever
//    specified, we save to the default config file name.
//
//    Brad Whitlock, Wed Feb 16 11:51:49 PDT 2005
//    Made it call utility function GetDefaultConfigFile.
//
//    Brad Whitlock, Thu Feb 17 16:11:21 PST 2005
//    I made it issue an error message if the settings can't be saved.
//
//    Brad Whitlock, Wed Apr 30 09:26:08 PDT 2008
//    Support for internationalization.
//
//    Brad Whitlock, Tue Apr 14 13:42:31 PDT 2009
//    Use ViewerProperties.
//
//    Jeremy Meredith, Thu Feb 18 15:39:42 EST 2010
//    Host profiles are now handled outside the config manager.
//
//    Jeremy Meredith, Fri Feb 19 13:29:24 EST 2010
//    Make sure host profile filenames are unique.
//
//    Jeremy Meredith, Wed Apr 21 13:15:09 EDT 2010
//    Only write out host profiles users have changed.
//
//    Jeremy Meredith, Thu Apr 22 13:21:27 EDT 2010
//    Also make sure to write out new host profiles that don't exist
//    in the system host profiles.
//
//    Jeremy Meredith, Thu Apr 29 14:48:03 EDT 2010
//    If the user modified a profile from the original system-global one,
//    only write out the fields they actually changed, not the whole thing.
//
// ****************************************************************************

void
ViewerStateManager::WriteConfigFile()
{
    std::string configFileName(GetViewerProperties()->GetConfigurationFileName());
    const char *cfn = (configFileName != "") ? configFileName.c_str() : 0;
    char *defaultConfigFile = GetDefaultConfigFile(cfn);

    //
    // Tell the ViewerWindowManager to get the current location, size of the
    // viewer windows so that information can be saved.
    //
    ViewerWindowManager::Instance()->UpdateWindowAtts();

    //
    // Tell the configuration manager to write the file.
    //
    if(!configMgr->WriteConfigFile(defaultConfigFile))
    {
        ViewerText err(TR("VisIt could not save your settings to: %1.").
                          arg(defaultConfigFile));
        GetViewerMessaging()->Error(err);
    }

    //
    // Delete the memory for the config file name.
    //
    delete [] defaultConfigFile;
}

// ****************************************************************************
// Method: ViewerStateManager::SaveSession
//
// Purpose:
//   Save all of VisIt's state to a session file.
//
// Arguments:
//   filename : The name of the session file to save.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 22:43:13 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerStateManager::SaveSession(const std::string &filename)
{
    configMgr->SetWriteDetail(true);
    bool wroteSession = configMgr->WriteConfigFile(filename.c_str());
    configMgr->SetWriteDetail(false);

    if(wroteSession)
    {
        GetViewerMessaging()->Message(
            TR("VisIt exported the current session to: %1.").arg(filename));
    }
    else
    {
        GetViewerMessaging()->Error(
            TR("VisIt could not save your session to: %1.").arg(filename));
    }
}

// ****************************************************************************
// Method: ViewerStateManager::WriteCallback
//
// Purpose:
//   This callback is called when the config manager writes a config file. The
//   callback lets us inject additional information into the config file.
//
// Arguments:
//   viewerNode  : The viewer data node.
//   writeDetail : If true then write more detail.
//   cbdata      : Callback data (ViewerStateManager instance).
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 22:47:51 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerStateManager::WriteCallback(DataNode *viewerNode, bool writeDetail, void *cbdata)
{
    ViewerStateManager *This = (ViewerStateManager *)cbdata;
    if(This != NULL)
        This->CreateNode(viewerNode, writeDetail);
}

// ****************************************************************************
// Method: ViewerStateManager::CreateNode
//
// Purpose: 
//   Saves the viewer's state to a DataNode object.
//
// Arguments:
//   parentNode : The node to which the state is added.
//   detailed   : Tells whether lots of details should be added to the nodes.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 30 12:32:00 PDT 2003
//
// Modifications:
//   Brad Whitlock, Fri Jul 18 10:47:20 PDT 2003
//   Added detailed argument. Made query manager add its data.
//
//   Brad Whitlock, Thu Mar 18 08:47:17 PDT 2004
//   Made the file server save its settings.
//
//   Brad Whitlock, Tue Aug 3 15:35:07 PST 2004
//   Made the engine manager save its settings so they are available in
//   session files so visit -movie can use them.
//
//   Brad Whitlock, Thu Nov 9 16:13:05 PST 2006
//   I added code to create a SourceMap node in the saved data so we
//   can reference it from other parts of the session to make it easier to
//   change databases.
//
//   Brad Whitlock, Tue Mar 27 11:18:36 PDT 2007
//   Made GetDatabasesForWindows get all databases not just MT ones.
//
// ****************************************************************************

void
ViewerStateManager::CreateNode(DataNode *parentNode, bool detailed)
{
    if(parentNode == 0)
        return;

    DataNode *vsNode = new DataNode("ViewerSubject");
    parentNode->AddNode(vsNode);

    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    stringVector databases;
    intVector    wIds;
    // Get the ids of the windows that currently exist.
    int nWin, *windowIndices;
    windowIndices = wM->GetWindowIndices(&nWin);
    for(int i = 0; i < nWin; ++i)
        wIds.push_back(windowIndices[i]);
    delete [] windowIndices;

    // Get all of the databases that are open in the specified windows
    // (even the ST databases).
    wM->GetDatabasesForWindows(wIds, databases, true);

    // Create a map of source ids to source names and also store
    // that information into the session.
    char keyName[100];
    std::map<std::string, std::string> dbToSource;
    DataNode *sourceMapNode = new DataNode("SourceMap");
    for(int i = 0; i < (int)databases.size(); ++i)
    {
        SNPRINTF(keyName, 100, "SOURCE%02d", i);
        std::string key(keyName);
        dbToSource[databases[i]] = key;
        sourceMapNode->AddNode(new DataNode(key, databases[i]));
    }
    vsNode->AddNode(sourceMapNode);

    GetViewerFileServer()->CreateNode(vsNode);
    correlationMethods->CreateNode(vsNode, dbToSource, detailed);
    wM->CreateNode(vsNode, dbToSource, detailed);
    if(detailed)
        ViewerQueryManager::Instance()->CreateNode(vsNode);

    // Handle ViewerEngineManager.
    {
        DataNode *vemNode = new DataNode("ViewerEngineManager");
        parentNode->AddNode(vemNode);
        GetViewerEngineManager()->CreateNode(vemNode, detailed);
        // save material and mesh management attributes
        GetViewerState()->GetMaterialAttributes()->CreateNode(vemNode,detailed,true);
        GetViewerState()->GetMeshManagementAttributes()->CreateNode(vemNode,detailed,true);
        GetViewerState()->GetExportDBAttributes()->CreateNode(vemNode, detailed, true);
    }
}

// ****************************************************************************
// Method: ViewerStateManager::RestoreSession
//
// Purpose:
//   Restore a session file.
//
// Arguments:
//   filename   : The name of the session file.
//   inVisItDir : Whether the file is in the .visit directory.
//   sources    : A list of sources to use to override the sources in the file.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 22:49:28 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerStateManager::RestoreSession(const std::string &filename, 
    bool inVisItDir, const stringVector &sources)
{
    // If we're importing a session, delete the localSettings in case the
    // DelayedProcessSettings method has not fired yet. This affects session
    // loading via the -sessionfile command line argument.
    if(localSettings != 0)
    {
        delete localSettings;
        localSettings = 0;
    }

    // Set up an alternate filename if we're writing files to the user directory.
    std::string file2(filename);
    if(inVisItDir)
    {
        file2 = GetUserVisItDirectory() + std::string(VISIT_SLASH_STRING) +
                filename;
    }

    // Read the config file.
    DataNode *node = configMgr->ReadConfigFile(file2.c_str());
    if(node != NULL)
    {
        // Make the hooked up objects get their settings.
        bool processPluginAtts = false;
        configMgr->ProcessConfigSettings(node, processPluginAtts);

        // What follows here is extra session processing code to let the other
        // objects set themselves up from the data node.

        // Get the VisIt node.
        DataNode *visitRoot = node->GetNode("VisIt");
        if(visitRoot != 0)
        {
            ViewerText differentVersionMessage;
            DataNode *versionNode = visitRoot->GetNode("Version");
            std::string configVersion(VISIT_VERSION);
            if(versionNode != 0)
            {
                configVersion = versionNode->AsString();
                if(versionNode->AsString() != VISIT_VERSION)
                {
                    differentVersionMessage = TR(
                        " Note that the session file was saved using VisIt %1 "
                        " and it may not be 100% compatible with VisIt %2.").
                        arg(versionNode->AsString()).arg(VISIT_VERSION);
                }
            }

            // Get the viewer node.
            DataNode *viewerNode = visitRoot->GetNode("VIEWER");
            if(viewerNode != 0)
            {
                // Build the source map.
                std::map<std::string,std::string> sourceToDB;
                sourceToDB = BuildSourceMap(viewerNode, sources);

                // Let the other viewer objects read their settings.
                bool fatalError = SetFromNode(viewerNode, sourceToDB, 
                                              configVersion.c_str());

                if(fatalError)
                {
                    ViewerText str(TR("VisIt detected serious errors in the "
                        "session file from: %1 so the session was not restored.").
                        arg(filename));
                    str += differentVersionMessage;
                    GetViewerMessaging()->Error(str);
                }
                else
                {
                    ViewerText str(TR("VisIt imported a session from: %1.").
                        arg(filename));
                    str += differentVersionMessage;
                    GetViewerMessaging()->Message(str);

                    // For state objects that changed, notify any observers.
                    configMgr->NotifyIfSelected();
                }

                return;
            }
        }
    }

    ViewerText str;
    if(inVisItDir)
    {
        str = TR("VisIt could not locate the session file: %1.").arg(filename);
        str += TR(" VisIt looks for session files in %1 by default.").
            arg(GetUserVisItDirectory());
    }
    else
        str = TR("VisIt could not locate the session file: %1.").arg(file2);

    str += TR(" Check that you provided the correct session "
           "file name or try including the entire path to the "
           "session file.");
    GetViewerMessaging()->Error(str);
}

// ****************************************************************************
// Method: ViewerStateManager::BuildSourceMap
//
// Purpose:
//   Use the sources and the viewerNode to build a source map to use when
//   restoring the session.
//
// Arguments:
//   viewerNode : The viewer node.
//   sources    : A vector of database names to use in the source map. If it
//                is empty then the names from the session file will be used.
//
// Returns:    A source map.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 22:20:20 PDT 2014
//
// Modifications:
//
// ****************************************************************************

std::map<std::string,std::string>
ViewerStateManager::BuildSourceMap(DataNode *viewerNode, const stringVector &sources)
{
    std::map<std::string,std::string> sourceToDB;

    if(!sources.empty())
    {
        int nSourceIds = (int)sources.size();
        DataNode *vsNode = viewerNode->GetNode("ViewerSubject");
        DataNode *sourceMapNode = 0;
        if(vsNode != 0 && 
           (sourceMapNode = vsNode->GetNode("SourceMap")) != 0)
        {
            if(sourceMapNode->GetNumChildren() > nSourceIds)
                nSourceIds = sourceMapNode->GetNumChildren();
        }

        std::string tmpHost, tmpDB, source;
        // Use the list of sources that the user provided
        // so we can override what's in the sesssion file.
        // This lets restore the session with new databases.
        for(int i = 0; i < nSourceIds; ++i)
        {
            char tmp[100];
            SNPRINTF(tmp, 100, "SOURCE%02d", i);
            if(i < (int)sources.size())
            {
                source = sources[i];
            }
            else
            {
                // pad the list of sources
                source = sources[sources.size()-1];
            }
            // Ensure we save a fully host-qualified name.
            FileFunctions::SplitHostDatabase(source, tmpHost, tmpDB);
            sourceToDB[std::string(tmp)] = tmpHost + std::string(":") + tmpDB;
        }
    }
    else
    {
        // Get the SourceMap node and use it to construct a map
        // that lets the rest of the session reading routines
        // pick out the right database name when they see a 
        // given source id.
        DataNode *vsNode = viewerNode->GetNode("ViewerSubject");
        DataNode *sourceMapNode = 0;
        if(vsNode != 0 && 
           (sourceMapNode = vsNode->GetNode("SourceMap")) != 0)
        {
            DataNode **srcFields = sourceMapNode->GetChildren();
            for(int i = 0; i < sourceMapNode->GetNumChildren(); ++i)
            {
                if(srcFields[i]->GetNodeType() == STRING_NODE)
                {
                    std::string key(srcFields[i]->GetKey());
                    std::string db(srcFields[i]->AsString());
                    sourceToDB[key] = db;
                }
            }
        }
    }

    return sourceToDB;
}

// ****************************************************************************
// Method: ViewerStateManager::SetFromNode
//
// Purpose: 
//   Sets the viewer's state from a DataNode object.
//
// Arguments:
//   parentNode : The DataNode object to use to set the state.
//
// Notes:
//   This method is only called when VisIt is reading session files.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 30 12:36:00 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Jul 22 10:12:59 PDT 2003
//   Added code to let the query manager initialize itself.
//
//   Brad Whitlock, Thu Mar 18 08:48:39 PDT 2004
//   Added code to initialize the file server.
//
//   Brad Whitlock, Wed Jan 11 14:44:13 PST 2006
//   I added some error checking to the session file processing.
//
//   Brad Whitlock, Thu Nov 9 17:14:12 PST 2006
//   I added support for SourceMap, which other objects use to get the
//   names of the databases that are used in the visualization.
//
//   Cyrus Harrison, Fri Mar 16 09:02:17 PDT 2007
//   Added support for ViewerEngineManager to restore its settings
//   from a node. 
//
//   Brad Whitlock, Wed Feb 13 14:08:18 PST 2008
//   Added configVersion argument.
//
//   Brad Whitlock, Mon Oct 26 15:45:27 PDT 2009
//   I added code to clear the caches of all engines since they could contain
//   metadata that we want to reread as part of restoring the session file.
//
// ****************************************************************************

bool
ViewerStateManager::SetFromNode(DataNode *parentNode, 
    const std::map<std::string,std::string> &sourceToDB, 
    const std::string &configVersion)
{
    bool fatalError = true;

    if(parentNode == 0)
        return fatalError;

    DataNode *vsNode = parentNode->GetNode("ViewerSubject");
    if(vsNode == 0)
        return fatalError;

    // See if there are any obvious errors in the session file.
    fatalError = ViewerWindowManager::Instance()->SessionContainsErrors(vsNode);
    if(!fatalError)
    {
        GetViewerEngineManager()->ClearCacheForAllEngines();
        GetViewerFileServer()->SetFromNode(vsNode);
        correlationMethods->SetFromNode(vsNode, sourceToDB, configVersion);
        ViewerWindowManager::Instance()->SetFromNode(vsNode, sourceToDB, configVersion);
        ViewerQueryManager::Instance()->SetFromNode(vsNode, configVersion);
        ReadEngineSettingsFromNode(vsNode, configVersion);
    }

    return fatalError;
}

// ****************************************************************************
// Method: ViewerStateManager::ReadEngineSettingsFromNode
//
// Purpose:
//   Read some engine settings.
//
// Arguments:
//   vsNode : The VS node.
//   configVersion : The config version.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep  9 10:54:52 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerStateManager::ReadEngineSettingsFromNode(DataNode *vsNode, 
    const std::string &configVersion)
{
    GetViewerEngineManager()->SetFromNode(vsNode, configVersion);

    // get the ViewerEngineManager node
    DataNode *vem_node = vsNode->GetNode("ViewerEngineManager");
    if(vem_node == 0)
        return;

    // Allow changes to be made to the contents of the vem_node before
    // we call SetFromNode.
    GetViewerState()->GetMaterialAttributes()->ProcessOldVersions(vem_node, configVersion.c_str());
    GetViewerState()->GetMeshManagementAttributes()->ProcessOldVersions(vem_node, configVersion.c_str());
    GetViewerState()->GetExportDBAttributes()->ProcessOldVersions(vem_node, configVersion.c_str());

    // restore material, mesh management, and export attributes
    GetViewerState()->GetMaterialAttributes()->SetFromNode(vem_node);
    GetViewerState()->GetMeshManagementAttributes()->SetFromNode(vem_node);
    GetViewerState()->GetExportDBAttributes()->SetFromNode(vem_node);
    GetViewerState()->GetMaterialAttributes()->Notify();
    GetViewerState()->GetMeshManagementAttributes()->Notify();
    GetViewerState()->GetExportDBAttributes()->Notify();
}

// ****************************************************************************
// Method: ViewerStateManager::ReadHostProfiles
//
// Purpose:
//   Read new user-defined host profiles from the user directory.
//
// Programmer: Jeremy Meredith
// Creation:   Thu Aug 28 22:36:03 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerStateManager::ReadHostProfiles()
{
    // And do the host profiles.  Keep the system one around
    // so we can see what the user changed and that we have to keep.
    if (!GetViewerProperties()->GetNoConfig())
    {
        if (originalSystemHostProfileList != NULL)
            delete originalSystemHostProfileList;
        originalSystemHostProfileList = new HostProfileList;
        FileFunctions::ReadAndProcessDirectory(GetSystemVisItHostsDirectory(),
                                &ReadHostProfileCallback,
                                originalSystemHostProfileList);
        *(GetViewerState()->GetHostProfileList()) =
                                                *originalSystemHostProfileList;
        FileFunctions::ReadAndProcessDirectory(GetAndMakeUserVisItHostsDirectory(),
                                &ReadHostProfileCallback,
                                GetViewerState()->GetHostProfileList());
    }
}

// ****************************************************************************
// Method: ViewerStateManager::ReadHostProfilesFromDirectory
//
// Purpose:
//   Read host profiles from the specified directory.
//
// Arguments:
//   dir   : The directory from which we'll read host profiles.
//   clear : Whether to clear the list first.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 15 14:56:40 PST 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerStateManager::ReadHostProfilesFromDirectory(const std::string &dir, bool clear)
{
    if(clear)
        GetViewerState()->GetHostProfileList()->ClearMachines();

    // Read host profiles from the specified directory.
    FileFunctions::ReadAndProcessDirectory(dir,
                                           &ReadHostProfileCallback,
                                           GetViewerState()->GetHostProfileList());
}

// ****************************************************************************
// Method: ViewerStateManager::WriteHostProfiles
//
// Purpose:
//   Write new user-defined host profiles to the user directory.
//
// Programmer: Jeremy Meredith
// Creation:   Thu Aug 28 22:36:03 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerStateManager::WriteHostProfiles()
{
    //
    // Clean old user host profiles from the file system
    // and write the new ones out.
    //
    std::string userdir = GetAndMakeUserVisItHostsDirectory();
    HostProfileList *hpl = GetViewerState()->GetHostProfileList();
    FileFunctions::ReadAndProcessDirectory(userdir, &CleanHostProfileCallback);
    // Make a filename-safe version of the nicknames
    stringVector basenames;
    int n = hpl->GetNumMachines();
    for (int i=0; i<n; i++)
    {
        MachineProfile &pl = hpl->GetMachines(i);
        std::string s = pl.GetHostNickname();
        for (size_t j=0; j<s.length(); j++)
        {
            if (s[j]>='A' && s[j]<='Z')
                s[j] += int('a')-int('A');
            if ((s[j]<'a'||s[j]>'z') && (s[j]<'0'||s[j]>'9'))
                s[j] = '_';
        }
        basenames.push_back(s);
    }
    // Make sure the filenames are unique
    for (int i=n-1; i>0; i--)
    {
        int count = 0;
        for (int j=0; j<i; j++)
        {
            if (basenames[j] == basenames[i])
                count++;
        }
        if (count > 0)
        {
            char tmp[100];
            sprintf(tmp, "_%d", count+1);
            basenames[i] += tmp;
        }
    }
    // Write them out if they are different
    for (int i=0; i<hpl->GetNumMachines(); i++)
    {
        MachineProfile &pl = hpl->GetMachines(i);
        // try to find an original system-global profile which our
        // user one was modified from
        MachineProfile *orig = NULL;
        for (int j=0; j<originalSystemHostProfileList->GetNumMachines(); j++)
        {
            MachineProfile &origpl(originalSystemHostProfileList->GetMachines(j));
            if (origpl.GetHostNickname() == pl.GetHostNickname())
            {
                orig = &origpl;
                break;
            }
        }
        std::string filename = userdir + VISIT_SLASH_STRING +
                          "host_" + basenames[i] + ".xml";
        if (orig)
        {
            // if we found a match, compare the original with the new
            if (*orig != pl)
            {
                // if there are differences, we should only write
                // out the things the user actually changed, so that
                // they pick up any updates to the system one
                pl.SelectOnlyDifferingFields(*orig);
                SingleAttributeConfigManager mgr(&pl);
                mgr.Export(filename, false);
            }
            // else skip it entirely - it's identical
        }
        else
        {
            // there's no original version - user-created, write it out
            // in its entirety
            SingleAttributeConfigManager mgr(&pl);
            mgr.Export(filename);
        }
    }
}

// ****************************************************************************
// Method:  CleanHostProfileCallback
//
// Purpose:
//   Callback for directory processing.  Unlinks old host profiles.
//
// Arguments:
//   file       the current filename
//   isdir      true if it's a directory
//
// Programmer:  Jeremy Meredith
// Creation:    February 18, 2010
//
// ****************************************************************************
static void
CleanHostProfileCallback(void *,
                         const std::string &file,
                         bool isdir,
                         bool canaccess,
                         long size)
{
    if (isdir)
        return;
    std::string base = FileFunctions::Basename(file);
    if (base.length()<=5 ||
        (base.substr(0,5) != "host_" &&
         base.substr(0,5) != "HOST_"))
        return;
    if (base.length()<=4 ||
        (base.substr(base.length()-4) != ".xml" &&
         base.substr(base.length()-4) != ".XML"))
        return;
#ifdef WIN32
    _unlink(file.c_str());
#else
    unlink(file.c_str());
#endif
}

// ****************************************************************************
// Method:  ReadostProfileCallback
//
// Purpose:
//   Callback for directory processing.  Reads old host profiles.
//
// Arguments:
//   hpl        the host profile list to load into
//   file       the current filename
//   isdir      true if it's a directory
//
// Programmer:  Jeremy Meredith
// Creation:    February 18, 2010
//
// Modifications:
//   Jeremy Meredith, Wed Apr 21 11:29:10 EDT 2010
//   If we're reading something with the same nickname, clobber the old one.
//   This allows user-saved profiles to override system ones.
//
//   Jeremy Meredith, Thu Apr 29 15:05:52 EDT 2010
//   Don't completely override the old one -- instead, import it over top of
//   the original one.  We no longer assume that host profiles saved to disk
//   are complete -- they now only contain values users changed from the 
//   system-global host profiles.
//
// ****************************************************************************
static void
ReadHostProfileCallback(void *hpl,
                        const std::string &file,
                        bool isdir,
                        bool canaccess,
                        long size)
{
    HostProfileList *profileList = (HostProfileList*)hpl;
    if (isdir)
        return;
    std::string base = FileFunctions::Basename(file);
    if (base.length()<=5 ||
        (base.substr(0,5) != "host_" &&
         base.substr(0,5) != "HOST_"))
        return;
    if (base.length()<=4 ||
        (base.substr(base.length()-4) != ".xml" &&
         base.substr(base.length()-4) != ".XML"))
        return;

    // Import the machine profile
    MachineProfile mp;
    SingleAttributeConfigManager mgr(&mp);
    mgr.Import(file);
    mp.SelectAll();

    // If it matches one of the existing ones, import it
    // over top of the old one so the old settings remain
    bool found = false;
    for (int i=0; i<profileList->GetNumMachines(); i++)
    {
        MachineProfile &mpold(profileList->GetMachines(i));
        if (mpold.GetHostNickname() == mp.GetHostNickname())
        {
            // note: yes, it's less inefficient to import it
            // twice, but the only easy way to override only
            // some of the old settings instead of all of them
            SingleAttributeConfigManager mgr2(&mpold);
            mgr2.Import(file);
            found = true;
            break;
        }
    }
    // and if it doesn't match, just add it to the list
    if (!found)
    {
        profileList->AddMachines(mp);
    }
    mp.SelectAll();
    profileList->SelectMachines();
}

// ****************************************************************************
// Method: ViewerStateManager::GetDatabaseCorrelationMethods
//
// Purpose:
//   Returns a pointer to the database correlation methods.
//
// Returns:    A pointer to the database correlation methods.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep  8 09:50:04 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerDatabaseCorrelationMethods *
ViewerStateManager::GetDatabaseCorrelationMethods()
{
    return correlationMethods;
}

// ****************************************************************************
// Method: ViewerStateManager::GetVariableMethods
//
// Purpose:
//   Returns  a pointer to some variable helper methods.
//
// Returns:    A pointer to the variable helper methods.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep  8 16:28:18 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerVariableMethods *
ViewerStateManager::GetVariableMethods()
{
    return variableMethods;
}

///////////////////////////////////////////////////////////////////////////////

MaterialAttributes *ViewerStateManager::materialDefaultAtts = NULL;

// ****************************************************************************
//  Method: ViewerWindowManager::GetMaterialDefaultAtts
//
//  Purpose: 
//    Returns a pointer to the default material attributes.
//
//  Returns:    A pointer to the default material attributes.
//
//  Programmer: Jeremy Meredith
//  Creation:   October 24, 2002
//
// ****************************************************************************

MaterialAttributes *
ViewerStateManager::GetMaterialDefaultAtts()
{
    if(materialDefaultAtts == NULL)
        materialDefaultAtts = new MaterialAttributes;

    return materialDefaultAtts;
}

// ****************************************************************************
//  Method:  ViewerStateManager::SetClientMaterialAttsFromDefault
//
//  Purpose:
//    Copy the current client atts to be the default.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 24, 2002
//
// ****************************************************************************

void
ViewerStateManager::SetClientMaterialAttsFromDefault()
{
    *(GetViewerState()->GetMaterialAttributes()) = *GetMaterialDefaultAtts();
    GetViewerState()->GetMaterialAttributes()->Notify();
}

// ****************************************************************************
//  Method:  ViewerStateManager::SetDefaultMaterialAttsFromClient
//
//  Purpose:
//    Copy the default atts back to the client.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 24, 2002
//
// ****************************************************************************

void
ViewerStateManager::SetDefaultMaterialAttsFromClient()
{
    *GetMaterialDefaultAtts() = *(GetViewerState()->GetMaterialAttributes());
}

///////////////////////////////////////////////////////////////////////////////
#include <MeshManagementAttributes.h>

MeshManagementAttributes *ViewerStateManager::meshManagementDefaultAtts = NULL;

// ****************************************************************************
//  Method: ViewerWindowManager::GetMeshManagementDefaultAtts
//
//  Purpose: Returns a pointer to the default meshManagement attributes.
//
//  Programmer: Mark C. Miller 
//  Creation:   November 5, 2005
//
// ****************************************************************************

MeshManagementAttributes *
ViewerStateManager::GetMeshManagementDefaultAtts()
{
    if(meshManagementDefaultAtts == NULL)
        meshManagementDefaultAtts = new MeshManagementAttributes;

    return meshManagementDefaultAtts;
}

// ****************************************************************************
//  Method:  ViewerStateManager::SetClientMeshManagementAttsFromDefault
//
//  Purpose: Copy the current client atts to be the default.
//
//  Programmer: Mark C. Miller 
//  Creation:   November 5, 2005
// ****************************************************************************

void
ViewerStateManager::SetClientMeshManagementAttsFromDefault()
{
    *(GetViewerState()->GetMeshManagementAttributes()) = *GetMeshManagementDefaultAtts();
    GetViewerState()->GetMeshManagementAttributes()->Notify();
    
}

// ****************************************************************************
//  Method:  ViewerStateManager::SetDefaultMeshManagementAttsFromClient
//
//  Purpose: Copy the default atts back to the client.
//
//  Programmer: Mark C. Miller 
//  Creation:   November 5, 2005
//
// ****************************************************************************

void
ViewerStateManager::SetDefaultMeshManagementAttsFromClient()
{
    *GetMeshManagementDefaultAtts() = *(GetViewerState()->GetMeshManagementAttributes());
}
