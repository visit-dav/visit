// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         OperatorPluginManager.h                           //
// ************************************************************************* //

#ifndef OPERATOR_PLUGIN_MANAGER_H
#define OPERATOR_PLUGIN_MANAGER_H
#include <plugin_exports.h>
#include <PluginManager.h>
#include <string>
#include <vector>

class GeneralOperatorPluginInfo;
class CommonOperatorPluginInfo;
class GUIOperatorPluginInfo;
class ViewerOperatorPluginInfo;
class EngineOperatorPluginInfo;
class ScriptingOperatorPluginInfo;

// ****************************************************************************
//  Class: OperatorPluginManager
//
//  Purpose:
//    The operator plugin manager.  It reads in the operator
//    plugins and provides information about the operator plugins.
//
//  Programmer: Eric Brugger
//  Creation:   March 7, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri May 11 14:09:33 PDT 2001
//    Added a PluginCategory which must be passed to Initialize.
//    Added an array of handles for the real shared libraries.
//
//    Jeremy Meredith, Thu Jul 26 09:32:55 PDT 2001
//    Renamed plugin info classes.
//
//    Jeremy Meredith, Thu Sep  6 16:23:17 PDT 2001
//    Reworked this to support more advanced plugin management, and 
//    support for runtime plugin loading/unloading.
//
//    Brad Whitlock, Thur Jul 12 17:44:54 PST 2001
//    Added support for scripting plugins.
//
//    Jeremy Meredith, Tue Jan 15 16:12:47 PST 2002
//    Added runtime parallel config.
//
//    Brad Whitlock, Fri Mar 29 09:21:27 PDT 2002
//    Ported to windows.
//
//    Jeremy Meredith, Fri Jul  5 17:36:23 PDT 2002
//    Made it work on plugins from multiple directories.
//
//    Jeremy Meredith, Tue Jul  9 17:42:07 PDT 2002
//    Made LoadCommonPluginInfo return a success flag.
//
//    Jeremy Meredith, Tue Aug 20 16:57:40 PDT 2002
//    Refactored most stuff into common PluginManager base class.
//
//    Jeremy Meredith, Fri Feb 28 12:26:45 PST 2003
//    Renamed LoadCommonPluginInfo to LoadGeneralPluginInfo to clarify its
//    purpose (which really is unrelated to FreeCommonPluginInfo and 
//    GetCommonPluginInfo).
//
//    Brad Whitlock, Tue Jun 24 10:48:11 PDT 2008
//    Removed singleton characteristics.
//
//    Brad Whitlock, Wed Jun 17 10:24:46 PDT 2009
//    I added arguments to Initialize.
//
//    Brad Whitlock, Fri Feb  5 14:44:32 PST 2010
//    I added stuff to enable categories to be accessed for any plugin whose
//    libI was loaded.
//
// ****************************************************************************

class PLUGIN_API OperatorPluginManager : public PluginManager
{
  public:
                                    OperatorPluginManager();
    virtual                        ~OperatorPluginManager();

    void                            Initialize(const PluginCategory pluginCategory,
                                               bool parallel=false,
                                               const char *pluginDir = 0,
                                               bool readInfo = true,
                                               PluginBroadcaster *broadcaster=0);

    virtual void                    ReloadPlugins();

    // Methods that return info about all plugins
    std::string                     GetOperatorCategoryName(const std::string &allid) const;

    // Methods that return info about loaded plugins
    virtual CommonOperatorPluginInfo    *GetCommonPluginInfo(const std::string&);
    virtual GUIOperatorPluginInfo       *GetGUIPluginInfo(const std::string&);
    virtual ViewerOperatorPluginInfo    *GetViewerPluginInfo(const std::string&);
    virtual EngineOperatorPluginInfo    *GetEnginePluginInfo(const std::string&);
    virtual ScriptingOperatorPluginInfo *GetScriptingPluginInfo(const std::string&);

  private:
    virtual void                    BroadcastGeneralInfo(PluginBroadcaster *);

    virtual bool                    LoadGeneralPluginInfo();
    virtual void                    LoadGUIPluginInfo();
    virtual void                    LoadViewerPluginInfo();
    virtual void                    LoadEnginePluginInfo();
    virtual void                    LoadScriptingPluginInfo();

    virtual void                    FreeCommonPluginInfo();
    virtual void                    FreeGUIPluginInfo();
    virtual void                    FreeViewerPluginInfo();
    virtual void                    FreeEnginePluginInfo();
    virtual void                    FreeScriptingPluginInfo();

    // arrays containing all plugins
    std::vector<std::string>                    operatorCategory;

    // arrays containing enabled plugins
    std::vector<CommonOperatorPluginInfo*>      commonPluginInfo;
    std::vector<GUIOperatorPluginInfo*>         guiPluginInfo;
    std::vector<ViewerOperatorPluginInfo*>      viewerPluginInfo;
    std::vector<EngineOperatorPluginInfo*>      enginePluginInfo;
    std::vector<ScriptingOperatorPluginInfo*>   scriptingPluginInfo;
};

#endif
