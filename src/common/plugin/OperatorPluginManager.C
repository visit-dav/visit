// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          OperatorPluginManager.C                          //
// ************************************************************************* //

#include <OperatorPluginManager.h>
#include <OperatorPluginInfo.h>
#include <PluginBroadcaster.h>
#include <AttributeSubject.h>
#include <DebugStream.h>
#include <InvalidPluginException.h>
#include <visitstream.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

// ****************************************************************************
//  Method: OperatorPluginManager constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 20, 2002
//
//  Modifications:
//    Brad Whitlock, Fri Feb  5 14:46:00 PST 2010
//    I added operatorCategory.
//
// ****************************************************************************

OperatorPluginManager::OperatorPluginManager() : PluginManager("operator"), 
    operatorCategory()
{
}

// ****************************************************************************
//  Method: OperatorPluginManager destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 20, 2002
//
//  Modifications:
//    Brad Whitlock, Wed Jun 25 10:27:17 PDT 2008
//    Call UnloadPlugins here since it calls virtual methods for this class.
//
// ****************************************************************************

OperatorPluginManager::~OperatorPluginManager()
{
    UnloadPlugins();
}

// ****************************************************************************
//  Method:  OperatorPluginManager::Initialize
//
//  Purpose:
//    Initialize the manager and read the plugins.
//
//  Arguments:
//    pluginCategory:   either GUI, Viewer, or Engine
//    parallel      :   true if need parallel libraries
//    pluginDir     :   Allows us to pass in the plugin dir that we want to use.
//    readInfo      :   Whether the plugin info should be read directly.
//    cb            :   A callback from which we can obtain plugin info.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 26, 2001
//
//  Modifications:
//    Jeremy Meredith, Wed Jan 16 09:47:44 PST 2002
//    Added parallel flag.
//
//    Brad Whitlock, Wed Nov 22 16:23:24 PST 2006
//    I added the pluginDir flag so applications can tell the plugin 
//    manager where to look directly without environment vars.
//
//    Brad Whitlock, Tue Jun 24 11:08:52 PDT 2008
//    Removed singleton characteristics.
//
//    Brad Whitlock, Wed Jun 17 13:05:54 PDT 2009
//    I added readInfo, broadcaster arguments to allow parallel optimizations.
//
// ****************************************************************************

void
OperatorPluginManager::Initialize(const PluginCategory pluginCategory,
    bool par, const char *pluginDir, bool readInfo, PluginBroadcaster *broadcaster)
{
    category = pluginCategory;
    parallel = par;
    operatorCategory.clear();
    SetPluginDir(pluginDir);
    ObtainPluginInfo(readInfo, broadcaster);
}

// ****************************************************************************
// Method: OperatorPluginManager::GetOperatorCategoryName
//
// Purpose: 
//   Return the category name of any operator plugin. This is the name of the
//   group in which it wants to be grouped.
//
// Arguments:
//   id : The all index id. This method provides data for plugins that were
//        loaded as libI.
//
// Returns:    The category.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb  5 14:43:23 PST 2010
//
// Modifications:
//   
// ****************************************************************************

std::string
OperatorPluginManager::GetOperatorCategoryName(const std::string &id) const
{
    std::map<std::string, int>::const_iterator it = allindexmap.find(id);
    return (it != allindexmap.end()) ? operatorCategory[it->second] : std::string("");
}

// ****************************************************************************
//  Method: OperatorPluginManager::GetCommonPluginInfo
//
//  Purpose:
//    Return a pointer to the common operator plugin information for the
//    specified operator type.
//
//  Arguments:
//    id        The id of the operator type.
//
//  Returns:    The common operator plugin information for the operator type.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 26, 2001
//
//  Modifications:
//
// ****************************************************************************

CommonOperatorPluginInfo *
OperatorPluginManager::GetCommonPluginInfo(const string &id)
{
    return commonPluginInfo[loadedindexmap[id]];
}

// ****************************************************************************
//  Method: OperatorPluginManager::GetGUIPluginInfo
//
//  Purpose:
//    Return a pointer to the GUI operator plugin information for the
//    specified operator type.
//
//  Arguments:
//    id        The id of the operator type.
//
//  Returns:    The GUI operator plugin information for the operator type.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 26, 2001
//
//  Modifications:
//
// ****************************************************************************

GUIOperatorPluginInfo *
OperatorPluginManager::GetGUIPluginInfo(const string &id)
{
    return guiPluginInfo[loadedindexmap[id]];
}

// ****************************************************************************
//  Method: OperatorPluginManager::GetViewerPluginInfo
//
//  Purpose:
//    Return a pointer to the viewer operator plugin information for the
//    specified operator type.
//
//  Arguments:
//    id        The id of the operator type.
//
//  Returns:    The viewer operator plugin information for the operator type.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 26, 2001
//
//  Modifications:
//
// ****************************************************************************

ViewerOperatorPluginInfo *
OperatorPluginManager::GetViewerPluginInfo(const string &id)
{
    return viewerPluginInfo[loadedindexmap[id]];
}

// ****************************************************************************
//  Method: OperatorPluginManager::GetEnginePluginInfo
//
//  Purpose:
//    Return a pointer to the engine operator plugin information for the
//    specified operator type.
//
//  Arguments:
//    id        The id of the operator type.
//
//  Returns:    The engine operator plugin information for the operator type.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 26, 2001
//
//  Modifications:
//
// ****************************************************************************

EngineOperatorPluginInfo *
OperatorPluginManager::GetEnginePluginInfo(const string &id)
{
    return enginePluginInfo[loadedindexmap[id]];
}

// ****************************************************************************
//  Method: OperatorPluginManager::GetScriptingPluginInfo
//
//  Purpose:
//    Return a pointer to the scripting operator plugin information for the
//    specified operator type.
//
//  Arguments:
//    id        The type of the operator.
//
//  Returns:    The scripting operator plugin information for the operator type.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jul 26 12:59:36 PDT 2001
//
// ****************************************************************************

ScriptingOperatorPluginInfo *
OperatorPluginManager::GetScriptingPluginInfo(const string &id)
{
    return scriptingPluginInfo[loadedindexmap[id]];
}

// ****************************************************************************
// Method: OperatorPluginManager::BroadcastGeneralInfo
//
// Purpose: 
//   This method broadcasts the general libI information to other processors
//   using a PluginBroadcaster object.
//
// Arguments:
//   broadcaster : The broadcaster to use.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb  5 14:40:39 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
OperatorPluginManager::BroadcastGeneralInfo(PluginBroadcaster *broadcaster)
{
    PluginManager::BroadcastGeneralInfo(broadcaster);
    broadcaster->BroadcastStringVector(operatorCategory);
}

// ****************************************************************************
// Method: OperatorPluginManager::LoadGeneralPluginInfo
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
//    Jeremy Meredith, Wed Nov  5 13:28:03 PST 2003
//    Use the default value for enabled status instead of always true.
//
//    Hank Childs, Tue Mar 22 16:06:15 PST 2005
//    Fix memory leak.
//
//    Brad Whitlock, Fri Feb  5 14:41:14 PST 2010
//    Added category.
//
// ****************************************************************************

bool
OperatorPluginManager::LoadGeneralPluginInfo()
{
    VISIT_PLUGIN_ENTRY_ARGS_DECLARE

    // Get the GeneralPluginInfo creator
    GeneralOperatorPluginInfo *(*getInfo)(VISIT_PLUGIN_ENTRY_ARGS)    =
        (GeneralOperatorPluginInfo* (*)(VISIT_PLUGIN_ENTRY_ARGS))PluginSymbol("GetGeneralInfo");
    if (!getInfo)
    {
        EXCEPTION3(InvalidPluginException, "Error retrieving info creator",
                   openPlugin.c_str(), PluginError());
    }

    // Get the general plugin info
    GeneralOperatorPluginInfo *info = (*getInfo)(VISIT_PLUGIN_ENTRY_ARGS_CALL);
    if (!info)
    {
        EXCEPTION2(InvalidPluginException, "Error creating general info",
                   openPlugin.c_str());
    }

    // We can't have two plugins with the same id.
    if (PluginExists(info->GetID()))
    {
        delete info;
        return false;
    }

    // Success!  Add it to the list.
    allindexmap[info->GetID()] = ids.size();
    ids     .push_back(info->GetID());
    names   .push_back(info->GetName());
    versions.push_back(info->GetVersion());
    enabled .push_back(info->EnabledByDefault());
    operatorCategory.push_back(info->GetCategoryName());
    delete info;
    return true;
}

// ****************************************************************************
// Method: OperatorPluginManager::LoadGUIPluginInfo
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
OperatorPluginManager::LoadGUIPluginInfo()
{
    VISIT_PLUGIN_ENTRY_ARGS_DECLARE

    GUIOperatorPluginInfo  *(*getGUIInfo)(VISIT_PLUGIN_ENTRY_ARGS) = 
        (GUIOperatorPluginInfo* (*)(VISIT_PLUGIN_ENTRY_ARGS))PluginSymbol("GetGUIInfo");

    if (!getGUIInfo)
    {
         EXCEPTION2(InvalidPluginException,
                    "Error retrieving GUI info",
                    openPlugin.c_str());
    }

    guiPluginInfo.push_back((*getGUIInfo)(VISIT_PLUGIN_ENTRY_ARGS_CALL));
    commonPluginInfo.push_back((*getGUIInfo)(VISIT_PLUGIN_ENTRY_ARGS_CALL));
}

// ****************************************************************************
// Method: OperatorPluginManager::LoadViewerPluginInfo
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
OperatorPluginManager::LoadViewerPluginInfo()
{
    VISIT_PLUGIN_ENTRY_ARGS_DECLARE

    ViewerOperatorPluginInfo  *(*getViewerInfo)(VISIT_PLUGIN_ENTRY_ARGS) =
        (ViewerOperatorPluginInfo* (*)(VISIT_PLUGIN_ENTRY_ARGS))PluginSymbol("GetViewerInfo");

    if (!getViewerInfo)
    {
        EXCEPTION2(InvalidPluginException,
        "Error retrieving Viewer info",
        openPlugin.c_str());
    }

    viewerPluginInfo.push_back((*getViewerInfo)(VISIT_PLUGIN_ENTRY_ARGS_CALL));
    commonPluginInfo.push_back((*getViewerInfo)(VISIT_PLUGIN_ENTRY_ARGS_CALL));
}

// ****************************************************************************
// Method: OperatorPluginManager::LoadEnginePluginInfo
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
OperatorPluginManager::LoadEnginePluginInfo()
{
    VISIT_PLUGIN_ENTRY_ARGS_DECLARE

    EngineOperatorPluginInfo  *(*getEngineInfo)(VISIT_PLUGIN_ENTRY_ARGS) =
        (EngineOperatorPluginInfo* (*)(VISIT_PLUGIN_ENTRY_ARGS))PluginSymbol("GetEngineInfo");

    if (!getEngineInfo)
    {
        EXCEPTION2(InvalidPluginException,
                   "Error retrieving Engine info",
                   openPlugin.c_str());
    }

    enginePluginInfo.push_back((*getEngineInfo)(VISIT_PLUGIN_ENTRY_ARGS_CALL));
    commonPluginInfo.push_back((*getEngineInfo)(VISIT_PLUGIN_ENTRY_ARGS_CALL));
}

// ****************************************************************************
// Method: OperatorPluginManager::LoadScriptingPluginInfo
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
OperatorPluginManager::LoadScriptingPluginInfo()
{
    VISIT_PLUGIN_ENTRY_ARGS_DECLARE

    ScriptingOperatorPluginInfo  *(*getScriptingInfo)(VISIT_PLUGIN_ENTRY_ARGS) =
        (ScriptingOperatorPluginInfo* (*)(VISIT_PLUGIN_ENTRY_ARGS))PluginSymbol("GetScriptingInfo");

    if (!getScriptingInfo)
    {
         EXCEPTION2(InvalidPluginException,
                    "Error retrieving Scripting info",
                    openPlugin.c_str());
    }

    scriptingPluginInfo.push_back((*getScriptingInfo)(VISIT_PLUGIN_ENTRY_ARGS_CALL));
    commonPluginInfo.push_back((*getScriptingInfo)(VISIT_PLUGIN_ENTRY_ARGS_CALL));
}

// ****************************************************************************
// Method: OperatorPluginManager::FreeCommonPluginInfo
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
OperatorPluginManager::FreeCommonPluginInfo()
{
    for (size_t i=0; i<commonPluginInfo.size(); i++)
        delete commonPluginInfo[i];
    commonPluginInfo.clear();
}

// ****************************************************************************
// Method: OperatorPluginManager::FreeGUIPluginInfo
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
OperatorPluginManager::FreeGUIPluginInfo()
{
    for (size_t i=0; i<guiPluginInfo.size(); i++)
        delete guiPluginInfo[i];
    guiPluginInfo.clear();
}

// ****************************************************************************
// Method: OperatorPluginManager::FreeViewerPluginInfo
//
// Purpose: 
//   Frees viewer plugin info.
//
// Programmer: Jeremy Meredith
// Creation:   August 20, 2002
//
// Modifications:
//   Eric Brugger, Wed Dec  4 11:10:35 PST 2019
//   Eliminate a memory leak by deleting the default attributes for all the
//   plugins.
//   
// ****************************************************************************

void
OperatorPluginManager::FreeViewerPluginInfo()
{
    for (size_t i=0; i<viewerPluginInfo.size(); i++)
    {
        // The client atts get deleted elsewhere.
        delete viewerPluginInfo[i]->GetDefaultAtts();
        delete viewerPluginInfo[i];
    }
    viewerPluginInfo.clear();
}

// ****************************************************************************
// Method: OperatorPluginManager::FreeEnginePluginInfo
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
OperatorPluginManager::FreeEnginePluginInfo()
{
    for (size_t i=0; i<enginePluginInfo.size(); i++)
        delete enginePluginInfo[i];
    enginePluginInfo.clear();
}

// ****************************************************************************
// Method: OperatorPluginManager::FreeScriptingPluginInfo
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
OperatorPluginManager::FreeScriptingPluginInfo()
{
    for (size_t i=0; i<scriptingPluginInfo.size(); i++)
        delete scriptingPluginInfo[i];
    scriptingPluginInfo.clear();
}

// ****************************************************************************
//  Method: OperatorPluginManager::ReloadPlugins
//
//  Purpose:
//    Free the loaded operator plugins.
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
//    Brad Whitlock, Fri Feb  5 15:04:46 PST 2010
//    I added operatorCategories.
//
// ****************************************************************************

void
OperatorPluginManager::ReloadPlugins()
{
    vector<void*>                        new_handles;
    vector<string>                       new_operatorCategory;
    vector<CommonOperatorPluginInfo*>    new_commonPluginInfo;
    vector<GUIOperatorPluginInfo*>       new_guiPluginInfo;
    vector<ViewerOperatorPluginInfo*>    new_viewerPluginInfo;
    vector<EngineOperatorPluginInfo*>    new_enginePluginInfo;
    vector<ScriptingOperatorPluginInfo*> new_scriptingPluginInfo;

    loadedindexmap.clear();
    for (size_t i=0; i<loadedhandles.size(); i++)
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

            new_operatorCategory.push_back(operatorCategory[i]);
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
    operatorCategory    = new_operatorCategory;
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

