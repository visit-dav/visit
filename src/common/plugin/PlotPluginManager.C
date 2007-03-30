// ************************************************************************* //
//                            PlotPluginManager.C                            //
// ************************************************************************* //

#include <PlotPluginManager.h>
#include <PlotPluginInfo.h>
#include <DebugStream.h>
#include <InvalidDirectoryException.h>
#include <InvalidPluginException.h>
#include <visit-config.h>
#include <iostream.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <algorithm>

#if defined(_WIN32)
#include <windows.h>
#include <direct.h>
#else
#include <dlfcn.h>
#include <dirent.h>
#endif

using std::string;
using std::vector;

//
// Storage for static data elements.
//
PlotPluginManager *PlotPluginManager::instance=0;

// ****************************************************************************
//  Method: PlotPluginManager constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 20, 2002
//
//  Modifications:
//
// ****************************************************************************

PlotPluginManager::PlotPluginManager() : PluginManager("plot")
{
}

// ****************************************************************************
//  Method: PlotPluginManager destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 20, 2002
//
//  Modifications:
//
// ****************************************************************************

PlotPluginManager::~PlotPluginManager()
{
}

// ****************************************************************************
//  Method:  PlotPluginManager::Initialize
//
//  Purpose:
//    Initialize the manager and read the plugins.
//
//  Arguments:
//    pluginCategory:   either GUI, Viewer, or Engine
//    parallel      :   true if need parallel libraries
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 26, 2001
//
//  Modifications:
//    Jeremy Meredith, Wed Jan 16 09:47:44 PST 2002
//    Added parallel flag.
//
// ****************************************************************************

void
PlotPluginManager::Initialize(const PluginCategory pluginCategory,
                              bool                 parallel)
{
    Instance();
    instance->category = pluginCategory;
    instance->parallel = parallel;
    instance->ReadPluginInfo();
}

// ****************************************************************************
//  Method: PlotPluginManager::Instance
//
//  Purpose:
//    Return a pointer to the sole instance of the PlotPluginManager
//    class.
//
//  Returns:    A pointer to the sole instance of the PlotPluginManager
//              class.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 26, 2001
//
// ****************************************************************************

PlotPluginManager *
PlotPluginManager::Instance()
{
    //
    // If the sole instance hasn't been instantiated, then instantiate it.
    //
    if (instance == 0)
    {
        instance = new PlotPluginManager;
    }

    return instance;
}

// ****************************************************************************
//  Method: PlotPluginManager::GetCommonPluginInfo
//
//  Purpose:
//    Return a pointer to the common plot plugin information for the
//    specified plot type.
//
//  Arguments:
//    id        The id of the plot type.
//
//  Returns:    The common plot plugin information for the plot type.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 26, 2001
//
//  Modifications:
//
// ****************************************************************************

CommonPlotPluginInfo *
PlotPluginManager::GetCommonPluginInfo(const string &id)
{
    return commonPluginInfo[loadedindexmap[id]];
}

// ****************************************************************************
//  Method: PlotPluginManager::GetGUIPluginInfo
//
//  Purpose:
//    Return a pointer to the GUI plot plugin information for the
//    specified plot type.
//
//  Arguments:
//    id        The id of the plot type.
//
//  Returns:    The GUI plot plugin information for the plot type.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 26, 2001
//
//  Modifications:
//
// ****************************************************************************

GUIPlotPluginInfo *
PlotPluginManager::GetGUIPluginInfo(const string &id)
{
    return guiPluginInfo[loadedindexmap[id]];
}

// ****************************************************************************
//  Method: PlotPluginManager::GetViewerPluginInfo
//
//  Purpose:
//    Return a pointer to the viewer plot plugin information for the
//    specified plot type.
//
//  Arguments:
//    id        The id of the plot type.
//
//  Returns:    The viewer plot plugin information for the plot type.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 26, 2001
//
//  Modifications:
//
// ****************************************************************************

ViewerPlotPluginInfo *
PlotPluginManager::GetViewerPluginInfo(const string &id)
{
    return viewerPluginInfo[loadedindexmap[id]];
}

// ****************************************************************************
//  Method: PlotPluginManager::GetEnginePluginInfo
//
//  Purpose:
//    Return a pointer to the engine plot plugin information for the
//    specified plot type.
//
//  Arguments:
//    id        The id of the plot type.
//
//  Returns:    The engine plot plugin information for the plot type.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 26, 2001
//
//  Modifications:
//
// ****************************************************************************

EnginePlotPluginInfo *
PlotPluginManager::GetEnginePluginInfo(const string &id)
{
    return enginePluginInfo[loadedindexmap[id]];
}

// ****************************************************************************
//  Method: PlotPluginManager::GetScriptingPluginInfo
//
//  Purpose:
//    Return a pointer to the scripting plot plugin information for the
//    specified plot type.
//
//  Arguments:
//    id        The type of the plot.
//
//  Returns:    The scripting plot plugin information for the plot type.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jul 26 12:59:36 PDT 2001
//
// ****************************************************************************

ScriptingPlotPluginInfo *
PlotPluginManager::GetScriptingPluginInfo(const string &id)
{
    return scriptingPluginInfo[loadedindexmap[id]];
}

// ****************************************************************************
// Method: PlotPluginManager::LoadGeneralPluginInfo
//
// Purpose: 
//   Loads general plugin info from the open plugin.
//
// Programmer: Jeremy Meredith, Brad Whitlock
// Creation:   Fri Mar 29 12:19:11 PDT 2002
//
// Modifications:
//    Jeremy Meredith, Tue Jul  9 17:42:50 PDT 2002
//    Made it refuse to load more than one plugin with the same id.
//    Made it return true on success, false on failure.
//
// ****************************************************************************

bool
PlotPluginManager::LoadGeneralPluginInfo()
{
    // Get the GeneralPluginInfo creator
    GeneralPlotPluginInfo *(*getInfo)(void)    =
        (GeneralPlotPluginInfo*(*)(void))PluginSymbol("GetGeneralInfo");
    if (!getInfo)
    {
        EXCEPTION3(InvalidPluginException, "Error retrieving info creator",
                   openPlugin.c_str(), PluginError());
    }

    // Get the general plugin info
    GeneralPlotPluginInfo *info = (*getInfo)();
    if (!info)
    {
        EXCEPTION2(InvalidPluginException, "Error creating general info",
                   openPlugin.c_str());
    }

    // We can't have two plugins with the same id.
    if (PluginExists(info->GetID()))
        return false;

    // Success!  Add it to the list.
    allindexmap[info->GetID()] = ids.size();
    ids     .push_back(info->GetID());
    names   .push_back(info->GetName());
    versions.push_back(info->GetVersion());
    enabled .push_back(true);
    return true;
}

// ****************************************************************************
// Method: PlotPluginManager::LoadGUIPluginInfo
//
// Purpose: 
//   Loads GUI plugin info from the open plugin.
//
// Programmer: Jeremy Meredith, Brad Whitlock
// Creation:   Fri Mar 29 12:19:11 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
PlotPluginManager::LoadGUIPluginInfo()
{
    GUIPlotPluginInfo  *(*getGUIInfo)(void) = 
        (GUIPlotPluginInfo* (*)(void))PluginSymbol("GetGUIInfo");

    if (!getGUIInfo)
    {
         EXCEPTION2(InvalidPluginException,
                    "Error retrieving GUI info",
                    openPlugin.c_str());
    }

    guiPluginInfo.push_back((*getGUIInfo)());
    commonPluginInfo.push_back((*getGUIInfo)());
}

// ****************************************************************************
// Method: PlotPluginManager::LoadViewerPluginInfo
//
// Purpose: 
//   Loads viewer plugin info from the open plugin.
//
// Programmer: Jeremy Meredith, Brad Whitlock
// Creation:   Fri Mar 29 12:19:11 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
PlotPluginManager::LoadViewerPluginInfo()
{
    ViewerPlotPluginInfo  *(*getViewerInfo)(void) =
        (ViewerPlotPluginInfo* (*)(void))PluginSymbol("GetViewerInfo");

    if (!getViewerInfo)
    {
        EXCEPTION2(InvalidPluginException,
        "Error retrieving Viewer info",
        openPlugin.c_str());
    }

    viewerPluginInfo.push_back((*getViewerInfo)());
    commonPluginInfo.push_back((*getViewerInfo)());
}

// ****************************************************************************
// Method: PlotPluginManager::LoadEnginePluginInfo
//
// Purpose: 
//   Loads engine plugin info from the open plugin.
//
// Programmer: Jeremy Meredith, Brad Whitlock
// Creation:   Fri Mar 29 12:19:11 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
PlotPluginManager::LoadEnginePluginInfo()
{
    EnginePlotPluginInfo  *(*getEngineInfo)(void) =
        (EnginePlotPluginInfo* (*)(void))PluginSymbol("GetEngineInfo");

    if (!getEngineInfo)
    {
        EXCEPTION2(InvalidPluginException,
                   "Error retrieving Engine info",
                   openPlugin.c_str());
    }

    enginePluginInfo.push_back((*getEngineInfo)());
    commonPluginInfo.push_back((*getEngineInfo)());
}

// ****************************************************************************
// Method: PlotPluginManager::LoadScriptingPluginInfo
//
// Purpose: 
//   Loads a scripting plugin from the open plugin.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 29 12:19:11 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
PlotPluginManager::LoadScriptingPluginInfo()
{
    ScriptingPlotPluginInfo  *(*getScriptingInfo)(void) =
        (ScriptingPlotPluginInfo* (*)(void))PluginSymbol("GetScriptingInfo");

    if (!getScriptingInfo)
    {
         EXCEPTION2(InvalidPluginException,
                    "Error retrieving Scripting info",
                    openPlugin.c_str());
    }

    scriptingPluginInfo.push_back((*getScriptingInfo)());
    commonPluginInfo.push_back((*getScriptingInfo)());
}

// ****************************************************************************
// Method: PlotPluginManager::FreeCommonPluginInfo
//
// Purpose: 
//   Frees common plugin info.
//
// Programmer: Jeremy Meredith
// Creation:   August 20, 2002
//
// Modifications:
//
// ****************************************************************************

void
PlotPluginManager::FreeCommonPluginInfo()
{
    for (int i=0; i<commonPluginInfo.size(); i++)
        delete commonPluginInfo[i];
    commonPluginInfo.clear();
}

// ****************************************************************************
// Method: PlotPluginManager::FreeGUIPluginInfo
//
// Purpose: 
//   Frees GUI plugin info.
//
// Programmer: Jeremy Meredith
// Creation:   August 20, 2002
//
// Modifications:
//   
// ****************************************************************************

void
PlotPluginManager::FreeGUIPluginInfo()
{
    for (int i=0; i<guiPluginInfo.size(); i++)
        delete guiPluginInfo[i];
    guiPluginInfo.clear();
}

// ****************************************************************************
// Method: PlotPluginManager::FreeViewerPluginInfo
//
// Purpose: 
//   Frees viewer plugin info.
//
// Programmer: Jeremy Meredith
// Creation:   August 20, 2002
//
// Modifications:
//   
// ****************************************************************************

void
PlotPluginManager::FreeViewerPluginInfo()
{
    for (int i=0; i<viewerPluginInfo.size(); i++)
        delete viewerPluginInfo[i];
    viewerPluginInfo.clear();
}

// ****************************************************************************
// Method: PlotPluginManager::FreeEnginePluginInfo
//
// Purpose: 
//   Frees engine plugin info.
//
// Programmer: Jeremy Meredith
// Creation:   August 20, 2002
//
// Modifications:
//   
// ****************************************************************************

void
PlotPluginManager::FreeEnginePluginInfo()
{
    for (int i=0; i<enginePluginInfo.size(); i++)
        delete enginePluginInfo[i];
    enginePluginInfo.clear();
}

// ****************************************************************************
// Method: PlotPluginManager::FreeScriptingPluginInfo
//
// Purpose: 
//   Frees scripting plugin info.
//
// Programmer: Jeremy Meredith
// Creation:   August 20, 2002
//
// Modifications:
//   
// ****************************************************************************

void
PlotPluginManager::FreeScriptingPluginInfo()
{
    for (int i=0; i<scriptingPluginInfo.size(); i++)
        delete scriptingPluginInfo[i];
    scriptingPluginInfo.clear();
}

// ****************************************************************************
//  Method: PlotPluginManager::ReloadPlugins
//
//  Purpose:
//    Free the loaded plot plugins.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 26, 2001
//
//  Modifications:
//    Brad Whitlock, Tue Nov 6 14:34:16 PST 2001
//    Added scripting plugins.
//
//    Brad Whitlock, Fri Mar 29 11:35:57 PDT 2002
//    Changed how plugins are closed.
//
//    Jeremy Meredith, Fri Feb 28 12:25:37 PST 2003
//    Made it use LoadPluginsNow or LoadPluginsOnDemand as appropriate.
//
// ****************************************************************************

void
PlotPluginManager::ReloadPlugins()
{
    vector<void*>                    new_handles;
    vector<CommonPlotPluginInfo*>    new_commonPluginInfo;
    vector<GUIPlotPluginInfo*>       new_guiPluginInfo;
    vector<ViewerPlotPluginInfo*>    new_viewerPluginInfo;
    vector<EnginePlotPluginInfo*>    new_enginePluginInfo;
    vector<ScriptingPlotPluginInfo*> new_scriptingPluginInfo;

    loadedindexmap.clear();
    for (int i=0; i<loadedhandles.size(); i++)
    {
        if (enabled[allindexmap[commonPluginInfo[i]->GetID()]])
        {
            loadedindexmap[commonPluginInfo[i]->GetID()] = new_handles.size();
            new_commonPluginInfo.push_back(commonPluginInfo[i]);
            if (category == GUI)
                new_guiPluginInfo.push_back(guiPluginInfo[i]);
            if (category == Viewer)
                new_viewerPluginInfo.push_back(viewerPluginInfo[i]);
            if (category == Engine)
                new_enginePluginInfo.push_back(enginePluginInfo[i]);
            if (category == Scripting)
                new_scriptingPluginInfo.push_back(scriptingPluginInfo[i]);

            new_handles.push_back(loadedhandles[i]);
        }
        else
        {
            delete commonPluginInfo[i];
            if (category == GUI)
                delete guiPluginInfo[i];
            if (category == Viewer)
                delete viewerPluginInfo[i];
            if (category == Engine)
                delete enginePluginInfo[i];
            if (category == Scripting)
                delete scriptingPluginInfo[i];

            handle = loadedhandles[i];
            PluginClose();
        }
    }
    commonPluginInfo    = new_commonPluginInfo;
    guiPluginInfo       = new_guiPluginInfo;
    viewerPluginInfo    = new_viewerPluginInfo;
    enginePluginInfo    = new_enginePluginInfo;
    scriptingPluginInfo = new_scriptingPluginInfo;
    loadedhandles       = new_handles;

    if (loadOnDemand)
        LoadPluginsOnDemand();
    else
        LoadPluginsNow();
}

