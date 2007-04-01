// ************************************************************************* //
//                         DatabasePluginManager.h                           //
// ************************************************************************* //

#ifndef DATABASE_PLUGIN_MANAGER_H
#define DATABASE_PLUGIN_MANAGER_H
#include <plugin_exports.h>
#include <PluginManager.h>
#include <string>
#include <vector>

class GeneralDatabasePluginInfo;
class CommonDatabasePluginInfo;
class MDServerDatabasePluginInfo;
class EngineDatabasePluginInfo;

// ****************************************************************************
//  Class: DatabasePluginManager
//
//  Purpose:
//    The database plugin manager.  It reads in the operator
//    plugins and provides information about the database plugins.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 21, 2002
//
//  Modifications:
//    Jeremy Meredith, Fri Feb 28 12:26:45 PST 2003
//    Renamed LoadCommonPluginInfo to LoadGeneralPluginInfo to clarify its
//    purpose (which really is unrelated to FreeCommonPluginInfo and 
//    GetCommonPluginInfo).
//
//    Jeremy Meredith, Tue Feb 22 15:20:03 PST 2005
//    Added way to determine directly if a plugin has a writer.
//
// ****************************************************************************

class PLUGIN_API DatabasePluginManager : public PluginManager
{
  public:
    virtual                        ~DatabasePluginManager();

    static void                     Initialize(const PluginCategory, bool=false);
    static DatabasePluginManager   *Instance();

    virtual void                    ReloadPlugins();

    CommonDatabasePluginInfo       *GetCommonPluginInfo(const std::string&);
    EngineDatabasePluginInfo       *GetEnginePluginInfo(const std::string&);
    bool                            PluginHasWriter(const std::string&);

  private:
                                    DatabasePluginManager();

    virtual bool                    LoadGeneralPluginInfo();
    virtual void                    LoadMDServerPluginInfo();
    virtual void                    LoadEnginePluginInfo();

    virtual void                    FreeCommonPluginInfo();

    // the plugin manager instance
    static DatabasePluginManager               *instance;

    // arrays containing all plugins (appends the ones in PluginManager.C)
    std::vector<bool>                       haswriter;

    // arrays containing enabled plugins
    std::vector<CommonDatabasePluginInfo*>      commonPluginInfo;
    std::vector<MDServerDatabasePluginInfo*>    mdserverPluginInfo;
    std::vector<EngineDatabasePluginInfo*>      enginePluginInfo;
};

#endif
