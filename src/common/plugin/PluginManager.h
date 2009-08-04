/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                             PluginManager.h                               //
// ************************************************************************* //

#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H
#include <plugin_exports.h>
#include <vectortypes.h>
#include <map>
#include <utility>

class PluginBroadcaster;

// ****************************************************************************
//  Class: PluginManager
//
//  Purpose:
//    The plugin manager.  It provides an abstraction for all plugin
//    managers.  The information is broken up into several classes since
//    portions of it are only relevant to particular components within visit.
//    There is the general information which all the components are interested
//    in, then portions for the gui, viewer, cli, engine, and mdserver.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 20, 2002
//
//  Modifications:
//    Jeremy Meredith, Fri Feb 28 12:28:50 PST 2003
//    Renamed some methods and data members to make their function and
//    usage more correct and obvious.  Added support for loading plugins
//    on demand.  Made PluginLoaded be private and added PluginAvailable,
//    which can attempt to load a plugin on demand before checking to see
//    if it is loaded.
//
//    Jeremy Meredith, Tue Jun 17 19:08:21 PDT 2003
//    Added GetEnabledIndex.
//
//    Jeremy Meredith, Wed Nov  5 13:48:58 PST 2003
//    Added ability to check if a plugin is enabled (by id).
//
//    Jeremy Meredith, Mon Feb  7 18:55:26 PST 2005
//    Added ability to check if any errors were generated when
//    the plugins were initialized.  Right now it is only used
//    for detecting incompatible versions.  Also, added ability
//    for PluginSymbol to not print an error if requested to be
//    silent about such things.
//
//    Brad Whitlock, Wed Nov 22 16:26:54 PST 2006
//    I added an argument to SetPluginDir.
//
//    Mark C. Miller, Mon Aug  6 13:36:16 PDT 2007
//    Added LoadSinglePluginNow. Changed LoadSinglePlugin to return bool
//
//    Hank Childs, Fri Oct  5 13:10:38 PDT 2007
//    Changed argument for ReadPluginDir.
//
//    Sean Ahern, Thu Jan 17 15:12:09 EST 2008
//    Made SetPluginDir be a public function so we can change the directory if
//    we need to.
//
//    Brad Whitlock, Thu Apr 23 11:36:20 PDT 2009
//    I added Simulation to the plugin categories, which will be a superset
//    of Engine.
//
//    Brad Whitlock, Wed Jun 17 10:10:20 PDT 2009
//    I added a callback for ReadPluginInfo so we can enable an optimization
//    that lets non-rank 0 processes not call it in parallel, saving a lot of
//    file system accesses.
//
// ****************************************************************************

class PLUGIN_API PluginManager
{
  public:
    enum PluginCategory
    {
        no_category,
        GUI,
        Viewer,
        Engine,
        MDServer,
        Scripting,
        Simulation
    };

  public:
    virtual ~PluginManager();

    void                            DisablePlugin(const std::string&);
    void                            EnablePlugin(const std::string&);
    bool                            PluginEnabled(const std::string&);

    virtual void                    LoadPluginsNow();
    virtual void                    LoadPluginsOnDemand();
    virtual bool                    LoadSinglePluginNow(const std::string&);
    virtual void                    ReloadPlugins();
    virtual void                    UnloadPlugins();

    bool                            PluginExists(const std::string&);
    bool                            PluginAvailable(const std::string&);

    std::string                     GetPluginName(const std::string&);
    std::string                     GetPluginVersion(const std::string&);

    int                             GetNAllPlugins() const;
    std::string                     GetAllID(const int) const;
    int                             GetAllIndex(const std::string &) const;
    int                             GetAllIndexFromName(const std::string &) const;

    int                             GetNEnabledPlugins() const;
    std::string                     GetEnabledID(const int) const;
    int                             GetEnabledIndex(const std::string &);

    std::string                     GetPluginInitializationErrors();
    void                            SetPluginDir(const char *dir = 0);

  protected:
                                    PluginManager(const std::string&);
    void                            ObtainPluginInfo(bool, PluginBroadcaster *);
    void                            ReadPluginInfo();
    virtual void                    BroadcastGeneralInfo(PluginBroadcaster *);

    void                            ReadPluginDir(std::vector<
                                                   std::vector<
                                                    std::pair<std::string,
                                                          std::string> > > &);
    void                            GetPluginList(std::vector<
                                                  std::pair<std::string,
                                                             std::string> >&);
    bool                            IsGeneralPlugin(const std::string &) const;

    bool                            PluginLoaded(const std::string&);
    void                            PluginOpen(const std::string &pluginFile);
    void                           *PluginSymbol(const std::string &symbol,bool ne=false);
    char                           *PluginError() const;
    void                            PluginClose();

    virtual bool                    LoadSinglePlugin(int i);

    virtual bool                    LoadGeneralPluginInfo()    = 0;
    virtual void                    LoadGUIPluginInfo()        { }
    virtual void                    LoadViewerPluginInfo()     { }
    virtual void                    LoadMDServerPluginInfo()   { }
    virtual void                    LoadEnginePluginInfo()     { }
    virtual void                    LoadScriptingPluginInfo()  { }

    virtual void                    FreeCommonPluginInfo()     = 0;
    virtual void                    FreeGUIPluginInfo()        { }
    virtual void                    FreeViewerPluginInfo()     { }
    virtual void                    FreeMDServerPluginInfo()   { }
    virtual void                    FreeEnginePluginInfo()     { }
    virtual void                    FreeScriptingPluginInfo()  { }

    std::vector<std::string>                pluginDirs;
    std::string                             openPlugin;
    void                                   *handle;
    char                                   *pluginError;
    int                                     category;
    bool                                    parallel;
    std::string                             managerName;
    bool                                    loadOnDemand;

    // arrays containing all plugins
    std::vector<std::string>                ids;
    std::vector<std::string>                names;
    std::vector<std::string>                versions;
    std::vector<std::string>                libfiles;
    std::vector<bool>                       enabled;

    // maps from id->allindex and id->loadedindex
    std::map<std::string, int>              allindexmap;
    std::map<std::string, int>              loadedindexmap;

    // arrays containing enabled plugins
    std::vector<void*>                      loadedhandles;
    std::vector<std::string>                loadedids;

    // accumlated plugin initialization errors
    std::string                             pluginInitErrors;
};

#endif
