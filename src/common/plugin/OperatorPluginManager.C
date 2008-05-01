/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

// ************************************************************************* //
//                          OperatorPluginManager.C                          //
// ************************************************************************* //

#include <OperatorPluginManager.h>
#include <OperatorPluginInfo.h>
#include <DebugStream.h>
#include <InvalidPluginException.h>
#include <visitstream.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

//
// Storage for static data elements.
//
OperatorPluginManager *OperatorPluginManager::instance=0;

// ****************************************************************************
//  Method: OperatorPluginManager constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 20, 2002
//
//  Modifications:
//
// ****************************************************************************

OperatorPluginManager::OperatorPluginManager() : PluginManager("operator")
{
}

// ****************************************************************************
//  Method: OperatorPluginManager destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 20, 2002
//
//  Modifications:
//
// ****************************************************************************

OperatorPluginManager::~OperatorPluginManager()
{
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
// ****************************************************************************

void
OperatorPluginManager::Initialize(const PluginCategory pluginCategory,
                                  bool parallel, const char *pluginDir)
{
    Instance();
    instance->category = pluginCategory;
    instance->parallel = parallel;
    instance->SetPluginDir(pluginDir);
    instance->ReadPluginInfo();
}

// ****************************************************************************
//  Method: OperatorPluginManager::Instance
//
//  Purpose:
//    Return a pointer to the sole instance of the OperatorPluginManager
//    class.
//
//  Returns:    A pointer to the sole instance of the OperatorPluginManager
//              class.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 26, 2001
//
// ****************************************************************************

OperatorPluginManager *
OperatorPluginManager::Instance()
{
    //
    // If the sole instance hasn't been instantiated, then instantiate it.
    //
    if (instance == 0)
    {
        instance = new OperatorPluginManager;
    }

    return instance;
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
// ****************************************************************************

bool
OperatorPluginManager::LoadGeneralPluginInfo()
{
    // Get the GeneralPluginInfo creator
    GeneralOperatorPluginInfo *(*getInfo)(void)    =
        (GeneralOperatorPluginInfo*(*)(void))PluginSymbol("GetGeneralInfo");
    if (!getInfo)
    {
        EXCEPTION3(InvalidPluginException, "Error retrieving info creator",
                   openPlugin.c_str(), PluginError());
    }

    // Get the general plugin info
    GeneralOperatorPluginInfo *info = (*getInfo)();
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
    GUIOperatorPluginInfo  *(*getGUIInfo)(void) = 
        (GUIOperatorPluginInfo* (*)(void))PluginSymbol("GetGUIInfo");

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
    ViewerOperatorPluginInfo  *(*getViewerInfo)(void) =
        (ViewerOperatorPluginInfo* (*)(void))PluginSymbol("GetViewerInfo");

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
    EngineOperatorPluginInfo  *(*getEngineInfo)(void) =
        (EngineOperatorPluginInfo* (*)(void))PluginSymbol("GetEngineInfo");

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
    ScriptingOperatorPluginInfo  *(*getScriptingInfo)(void) =
        (ScriptingOperatorPluginInfo* (*)(void))PluginSymbol("GetScriptingInfo");

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
//   
// ****************************************************************************

void
OperatorPluginManager::FreeViewerPluginInfo()
{
    for (size_t i=0; i<viewerPluginInfo.size(); i++)
        delete viewerPluginInfo[i];
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
// ****************************************************************************

void
OperatorPluginManager::ReloadPlugins()
{
    vector<void*>                        new_handles;
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

