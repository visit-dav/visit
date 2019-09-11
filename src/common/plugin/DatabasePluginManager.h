// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    Mark C. Miller, Mon Aug  6 13:36:16 PDT 2007
//    Added PluginFileExtensions, PluginFilenames and GetMatchingPluginId
//    add supporting data members, extensions, filenames.
//
//    Mark C. Miller, Thu Aug  9 09:16:01 PDT 2007
//    Made GetMatchingPluginIds return a vector of strings
//    
//    Sean Ahern, Thu Jan 17 16:40:08 EST 2008
//    Added a pluginDir argument to Initialize to make it conform to the other
//    plugin managers.
//
//    Brad Whitlock, Tue Jun 24 11:11:24 PDT 2008
//    Removed singleton characteristics.
//
//    Brad Whitlock, Wed Jun 17 10:24:46 PDT 2009
//    I added arguments to Initialize and I overrode the new
//    BroadcastGeneralInfo method.
//
//    Jeremy Meredith, Tue Dec 29 11:40:42 EST 2009
//    Replaced "Extensions" and "Filenames" with "FilePatterns".  Added
//    filePatternsStrict and opensWholeDirectory.
//
//    Jeremy Meredith, Tue Dec 29 15:52:06 EST 2009
//    Added ability to report warnings occurring during file opening.
//
// ****************************************************************************

class PLUGIN_API DatabasePluginManager : public PluginManager
{
  public:
                                    DatabasePluginManager();
    virtual                        ~DatabasePluginManager();

    void                            Initialize(const PluginCategory,
                                               bool parallel=false,
                                               const char *pluginDir=0,
                                               bool readInfo = true,
                                               PluginBroadcaster *broadcaster=0);

    virtual void                    ReloadPlugins();

    CommonDatabasePluginInfo       *GetCommonPluginInfo(const std::string&);
    EngineDatabasePluginInfo       *GetEnginePluginInfo(const std::string&);
    bool                            PluginHasWriter(const std::string&);
    std::vector<std::string>        PluginFilePatterns(const std::string&);
    bool                            PluginFilePatternsAreStrict(const std::string&);
    bool                            PluginOpensWholeDirectory(const std::string&);

    std::vector<std::string>        GetMatchingPluginIds(const char *fname,
                                        bool searchAll = false);

    void                            ReportWarning(const std::string &w);

  private:
    virtual bool                    LoadGeneralPluginInfo();
    virtual void                    LoadMDServerPluginInfo();
    virtual void                    LoadEnginePluginInfo();

    virtual void                    FreeCommonPluginInfo();

    virtual void                    BroadcastGeneralInfo(PluginBroadcaster *);

    // arrays containing all plugins (appends the ones in PluginManager.C)
    std::vector<bool>                       haswriter;
    std::vector<std::vector<std::string> >  filePatterns;
    std::vector<bool>                       filePatternsAreStrict;
    std::vector<bool>                       opensWholeDirectory;

    // arrays containing enabled plugins
    std::vector<CommonDatabasePluginInfo*>      commonPluginInfo;
    std::vector<MDServerDatabasePluginInfo*>    mdserverPluginInfo;
    std::vector<EngineDatabasePluginInfo*>      enginePluginInfo;
};

#endif
