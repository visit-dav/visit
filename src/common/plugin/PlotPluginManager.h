// ************************************************************************* //
//                           PlotPluginManager.h                             //
// ************************************************************************* //

#ifndef PLOT_PLUGIN_MANAGER_H
#define PLOT_PLUGIN_MANAGER_H
#include <plugin_exports.h>
#include <PluginManager.h>
#include <string>
#include <vector>

class GeneralPlotPluginInfo;
class CommonPlotPluginInfo;
class GUIPlotPluginInfo;
class ViewerPlotPluginInfo;
class EnginePlotPluginInfo;
class ScriptingPlotPluginInfo;

// ****************************************************************************
//  Class: PlotPluginManager
//
//  Purpose:
//    The plot plugin manager.  It reads in the operator
//    plugins and provides information about the plot plugins.
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
// ****************************************************************************

class PLUGIN_API PlotPluginManager : public PluginManager
{
  public:
    virtual                        ~PlotPluginManager();

    static void                     Initialize(const PluginCategory, bool=false);
    static PlotPluginManager       *Instance();

    virtual void                    ReloadPlugins();

    CommonPlotPluginInfo           *GetCommonPluginInfo(const std::string&);
    GUIPlotPluginInfo              *GetGUIPluginInfo(const std::string&);
    ViewerPlotPluginInfo           *GetViewerPluginInfo(const std::string&);
    EnginePlotPluginInfo           *GetEnginePluginInfo(const std::string&);
    ScriptingPlotPluginInfo        *GetScriptingPluginInfo(const std::string&);

  private:
                                    PlotPluginManager();

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

    // the plugin manager instance
    static PlotPluginManager               *instance;

    // arrays containing enabled plugins
    std::vector<CommonPlotPluginInfo*>      commonPluginInfo;
    std::vector<GUIPlotPluginInfo*>         guiPluginInfo;
    std::vector<ViewerPlotPluginInfo*>      viewerPluginInfo;
    std::vector<EnginePlotPluginInfo*>      enginePluginInfo;
    std::vector<ScriptingPlotPluginInfo*>   scriptingPluginInfo;
};

#endif
