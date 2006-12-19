/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                          DatabasePluginManager.C                          //
// ************************************************************************* //

#include <DatabasePluginManager.h>
#include <DatabasePluginInfo.h>
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
DatabasePluginManager *DatabasePluginManager::instance=0;

// ****************************************************************************
//  Method: DatabasePluginManager constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 22, 2002
//
//  Modifications:
//    Brad Whitlock, Wed Nov 22 16:31:59 PST 2006
//    I made it call SetPluginDir since it's no longer done in the base
//    class's constructor.
//
// ****************************************************************************

DatabasePluginManager::DatabasePluginManager() : PluginManager("database")
{
    // Make it use VISITPLUGINDIR.
    SetPluginDir(0);
}

// ****************************************************************************
//  Method: DatabasePluginManager destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 22, 2002
//
//  Modifications:
//
// ****************************************************************************

DatabasePluginManager::~DatabasePluginManager()
{
}

// ****************************************************************************
//  Method:  DatabasePluginManager::Initialize
//
//  Purpose:
//    Initialize the manager and read the plugins.
//
//  Arguments:
//    pluginCategory:   either GUI, Viewer, or Engine
//    parallel      :   true if need parallel libraries
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 22, 2002
//
//  Modifications:
//
// ****************************************************************************

void
DatabasePluginManager::Initialize(const PluginCategory pluginCategory,
                                  bool                 parallel)
{
    Instance();
    instance->category = pluginCategory;
    instance->parallel = parallel;
    instance->ReadPluginInfo();
}

// ****************************************************************************
//  Method: DatabasePluginManager::Instance
//
//  Purpose:
//    Return a pointer to the sole instance of the DatabasePluginManager
//    class.
//
//  Returns:    A pointer to the sole instance of the DatabasePluginManager
//              class.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 22, 2002
//
// ****************************************************************************

DatabasePluginManager *
DatabasePluginManager::Instance()
{
    //
    // If the sole instance hasn't been instantiated, then instantiate it.
    //
    if (instance == 0)
    {
        instance = new DatabasePluginManager;
    }

    return instance;
}

// ****************************************************************************
//  Method: DatabasePluginManager::GetEnginePluginInfo
//
//  Purpose:
//    Return a pointer to the engine database plugin information for the
//    specified database type.
//
//  Arguments:
//    id        The id of the database type.
//
//  Returns:    The engine database plugin information for the database type.
//
//  Programmer: Jeremy Meredith
//  Creation:   February 22, 2005
//
//  Modifications:
//
// ****************************************************************************

EngineDatabasePluginInfo *
DatabasePluginManager::GetEnginePluginInfo(const string &id)
{
    return enginePluginInfo[loadedindexmap[id]];
}

// ****************************************************************************
//  Method: DatabasePluginManager::GetCommonPluginInfo
//
//  Purpose:
//    Return a pointer to the common database plugin information for the
//    specified database type.
//
//  Arguments:
//    id        The id of the database type.
//
//  Returns:    The common database plugin information for the database type.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 22, 2002
//
//  Modifications:
//
// ****************************************************************************

CommonDatabasePluginInfo *
DatabasePluginManager::GetCommonPluginInfo(const string &id)
{
    return commonPluginInfo[loadedindexmap[id]];
}

// ****************************************************************************
// Method: DatabasePluginManager::LoadGeneralPluginInfo
//
// Purpose: 
//   Loads general plugin info from the open plugin.
//
// Programmer: Jeremy Meredith
// Creation:   August 22, 2002
//
// Modifications:
//    Jeremy Meredith, Wed Nov  5 13:28:03 PST 2003
//    Use the default value for enabled status instead of always true.
//
//    Jeremy Meredith, Tue Feb 22 15:22:29 PST 2005
//    Added a way to determine directly if a plugin has a writer.
//
//    Hank Childs, Tue Mar 22 16:06:15 PST 2005
//    Fix memory leak.
//
// ****************************************************************************

bool
DatabasePluginManager::LoadGeneralPluginInfo()
{
    // Get the GeneralPluginInfo creator
    GeneralDatabasePluginInfo *(*getInfo)(void)    =
        (GeneralDatabasePluginInfo*(*)(void))PluginSymbol("GetGeneralInfo");
    if (!getInfo)
    {
        EXCEPTION3(InvalidPluginException, "Error retrieving info creator",
                   openPlugin.c_str(), PluginError());
    }

    // Get the general plugin info
    GeneralDatabasePluginInfo *info = (*getInfo)();
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
    ids      .push_back(info->GetID());
    names    .push_back(info->GetName());
    versions .push_back(info->GetVersion());
    enabled  .push_back(info->EnabledByDefault());
    haswriter.push_back(info->HasWriter());
    delete info;
    return true;
}

// ****************************************************************************
// Method: DatabasePluginManager::LoadEnginePluginInfo
//
// Purpose: 
//   Loads engine plugin info from the open plugin.
//
// Programmer: Jeremy Meredith
// Creation:   August 22, 2002
//
// Modifications:
//   
// ****************************************************************************

void
DatabasePluginManager::LoadEnginePluginInfo()
{
    EngineDatabasePluginInfo  *(*getEngineInfo)(void) =
        (EngineDatabasePluginInfo* (*)(void))PluginSymbol("GetEngineInfo");

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
// Method: DatabasePluginManager::LoadMDServerPluginInfo
//
// Purpose: 
//   Loads mdserver plugin info from the open plugin.
//
// Programmer: Jeremy Meredith
// Creation:   August 22, 2002
//
// Modifications:
//   
// ****************************************************************************

void
DatabasePluginManager::LoadMDServerPluginInfo()
{
    MDServerDatabasePluginInfo  *(*getMDServerInfo)(void) =
        (MDServerDatabasePluginInfo* (*)(void))PluginSymbol("GetMDServerInfo");

    if (!getMDServerInfo)
    {
        EXCEPTION2(InvalidPluginException,
                   "Error retrieving MDServer info",
                   openPlugin.c_str());
    }

    mdserverPluginInfo.push_back((*getMDServerInfo)());
    commonPluginInfo.push_back((*getMDServerInfo)());
}

// ****************************************************************************
// Method: DatabasePluginManager::FreeCommonPluginInfo
//
// Purpose: 
//   Frees common plugin info.
//
// Programmer: Jeremy Meredith
// Creation:   August 22, 2002
//
// Modifications:
//
// ****************************************************************************

void
DatabasePluginManager::FreeCommonPluginInfo()
{
    for (int i=0; i<commonPluginInfo.size(); i++)
        delete commonPluginInfo[i];
    commonPluginInfo.clear();
}

// ****************************************************************************
//  Method: DatabasePluginManager::ReloadPlugins
//
//  Purpose:
//    Free the loaded database plugins.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 22, 2002
//
//  Modifications:
//    Jeremy Meredith, Fri Feb 28 12:25:37 PST 2003
//    Made it use LoadPluginsNow or LoadPluginsOnDemand as appropriate.
//
// ****************************************************************************

void
DatabasePluginManager::ReloadPlugins()
{
    vector<void*>                        new_handles;
    vector<CommonDatabasePluginInfo*>    new_commonPluginInfo;

    loadedindexmap.clear();
    for (int i=0; i<loadedhandles.size(); i++)
    {
        if (enabled[allindexmap[commonPluginInfo[i]->GetID()]])
        {
            loadedindexmap[commonPluginInfo[i]->GetID()] = new_handles.size();
            new_commonPluginInfo.push_back(commonPluginInfo[i]);

            new_handles.push_back(loadedhandles[i]);
        }
        else
        {
            delete commonPluginInfo[i];

            handle = loadedhandles[i];
            PluginClose();
        }
    }
    commonPluginInfo    = new_commonPluginInfo;
    loadedhandles       = new_handles;

    if (loadOnDemand)
        LoadPluginsOnDemand();
    else
        LoadPluginsNow();
}

// ****************************************************************************
//  Method:  DatabasePluginManager::PluginHasWriter
//
//  Purpose:
//    Returns true if a database plugin has a writer method.
//
//  Arguments:
//    id         the plugin id
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 22, 2005
//
// ****************************************************************************
bool
DatabasePluginManager::PluginHasWriter(const string &id)
{
    bool retval = false;
    if(allindexmap.find(id) != allindexmap.end())
    {
        int index = allindexmap[id];
        if(index < names.size())
            retval = haswriter[index];
    }

    return retval;
}
