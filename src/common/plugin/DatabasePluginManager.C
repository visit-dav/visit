// ************************************************************************* //
//                          DatabasePluginManager.C                          //
// ************************************************************************* //

#include <DatabasePluginManager.h>
#include <DatabasePluginInfo.h>
#include <DebugStream.h>
#include <InvalidPluginException.h>
#include <iostream.h>
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
//
// ****************************************************************************

DatabasePluginManager::DatabasePluginManager() : PluginManager("database")
{
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

