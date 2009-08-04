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
//                              PluginManager.C                              //
// ************************************************************************* //
#include <PluginManager.h>
#include <visit-config.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <string>
#include <map>
#include <vector>

#include <DebugStream.h>
#include <InvalidDirectoryException.h>
#include <InvalidPluginException.h>
#include <InstallationFunctions.h>
#include <PluginBroadcaster.h>
#include <visitstream.h>

#if __APPLE__
#include <AvailabilityMacros.h>
#endif
#if defined(_WIN32)
#include <windows.h>
#include <direct.h>
#include <shlobj.h>
#include <shlwapi.h>
#elif defined(__APPLE__) && ( MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_2 )
#include <mach-o/dyld.h>
#include <dirent.h>
#else
#include <dlfcn.h>
#include <dirent.h>
#endif

using std::string;
using std::vector;
using std::map;
using std::pair;
using std::sort;

#define MAX_PLUGINERROR 500

// ****************************************************************************
//  Method: PluginManager constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   September 26, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Jul 26 12:57:46 PDT 2001
//    Added scripting plugins.
//
//    Brad Whitlock, Fri Mar 29 09:29:50 PDT 2002
//    Moved the plugin dir code to here.
//
//    Jeremy Meredith, Fri Feb 28 12:33:52 PST 2003
//    Added initialization of loadOnDemand.
//
//    Jeremy Meredith, Tue Feb  8 08:40:57 PST 2005
//    Added initialization of pluginInitErrors.
//
//    Brad Whitlock, Wed Nov 22 16:26:20 PST 2006
//    I removed the call to SetPluginDir.
//
// ****************************************************************************

PluginManager::PluginManager(const string &mgr) : pluginDirs(), openPlugin(),
    handle(0), pluginError(0), category(no_category), parallel(false),
    managerName(mgr), loadOnDemand(false), 
    ids(), names(), versions(), libfiles(), enabled(),
    allindexmap(), loadedindexmap(), loadedhandles(), loadedids(),
    pluginInitErrors()
{
    pluginError = new char[MAX_PLUGINERROR];
}

// ****************************************************************************
//  Method: PluginManager destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   September 26, 2001
//
//  Modifications:
//    Brad Whitlock, Wed Jun 25 10:28:07 PDT 2008
//    Removed call to UnloadPlugins because it can't call back up to derived
//    classes since they've been destructed. That means that some of the 
//    methods called were back to pure virtual by the time we got here.
//
// ****************************************************************************

PluginManager::~PluginManager()
{
    delete [] pluginError;
}

// ****************************************************************************
//  Method:  PluginManager::PluginExists
//
//  Purpose:
//    Return true if the plugin exists -- this means it can be enabled and
//    it can attempt to be loaded
//
//  Arguments:
//    id         the id to check
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 26, 2001
//
// ****************************************************************************

bool
PluginManager::PluginExists(const string &id)
{
    return (allindexmap.count(id) > 0);
}

// ****************************************************************************
//  Method:  PluginManager::PluginEnabled
//
//  Purpose:
//    Returns true if the plugin indexed by the given ID is currently enabled.
//
//  Arguments:
//    id         the id to check
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  5, 2003
//
// ****************************************************************************
bool
PluginManager::PluginEnabled(const string &id)
{
    return PluginExists(id) && enabled[allindexmap[id]];
}


// ****************************************************************************
//  Method:  PluginManager::PluginAvailable
//
//  Purpose:
//    Return true if the plugin is available -- this means it exists, it
//    is enabled, and it is successfully loaded
//
//  Arguments:
//    id         the id to check
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 27, 2003
//
// ****************************************************************************

bool
PluginManager::PluginAvailable(const string &id)
{
    if (!PluginExists(id))
        return false;

    if (loadOnDemand)
    {
        int index = GetAllIndex(id);
        LoadSinglePlugin(index);
    }

    return PluginLoaded(id);
}

// ****************************************************************************
//  Method:  PluginManager::PluginLoaded
//
//  Purpose:
//    Return true if the plugin was succesfully loaded
//
//  Arguments:
//    id         the id to check
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 26, 2001
//
// ****************************************************************************

bool
PluginManager::PluginLoaded(const string &id)
{
    return (loadedindexmap.count(id) > 0);
}

// ****************************************************************************
//  Method:  PluginManager::GetPluginName
//
//  Purpose:
//    Get the name of a plugin by its id
//
//  Arguments:
//    id         the plugin id
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 26, 2001
//
//  Modfications:
//    Brad Whitlock, Thu Jul 17 09:56:19 PDT 2003
//    Prevented case where a bad id could add itself to the allindexmap.
//
// ****************************************************************************

string
PluginManager::GetPluginName(const string &id)
{
    string retval;
    if(allindexmap.find(id) != allindexmap.end())
    {
        int index = allindexmap[id];
        if(index < names.size())
            retval = names[index];
    }

    return retval;
}

// ****************************************************************************
//  Method:  PluginManager::GetPluginVersion
//
//  Purpose:
//    Get the version of a plugin by its id
//
//  Arguments:
//    id         the plugin id
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 26, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Jul 17 09:56:19 PDT 2003
//    Prevented case where a bad id could add itself to the allindexmap.
//
// ****************************************************************************

string
PluginManager::GetPluginVersion(const string &id)
{
    string retval;
    if(allindexmap.find(id) != allindexmap.end())
    {
        int index = allindexmap[id];
        if(index < versions.size())
            retval = versions[index];
    }

    return retval;
}

// ****************************************************************************
//  Method: PluginManager::GetNAllPlugins
//
//  Purpose:
//    Return the number of all plugins.
//
//  Returns:    The number of all plugins.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 26, 2001
//
// ****************************************************************************

int
PluginManager::GetNAllPlugins() const
{
    return ids.size();
}

// ****************************************************************************
//  Method: PluginManager::GetAllID
//
//  Purpose:
//    Return the name id of a plugin by its overall index.
//
//  Arguments:
//    index     The index of a plugin by its overall index
//
//  Returns:    The id of a plugin.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 26, 2001
//
// ****************************************************************************

string
PluginManager::GetAllID(int index) const
{
    return ids[index];
}

// ****************************************************************************
//  Method: PluginManager::GetNEnabledPlugins
//
//  Purpose:
//    Return the number of loaded plugins.
//
//  Returns:    The number of loaded plugins.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 26, 2001
//
// ****************************************************************************

int
PluginManager::GetNEnabledPlugins() const
{
    return loadedhandles.size();
}

// ****************************************************************************
//  Method: PluginManager::GetEnabledID
//
//  Purpose:
//    Return the name id of a loaded plugin.
//
//  Arguments:
//    index     The index of a loaded plugin id
//
//  Returns:    The id of a plugin.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 26, 2001
//
// ****************************************************************************

string
PluginManager::GetEnabledID(int index) const
{
    return loadedids[index];
}

// ****************************************************************************
//  Method: PluginManager::GetEnabledIndex
//
//  Purpose:
//    Return the enabled-index of a loaded plugin.
//
//  Arguments:
//    id     The id of a plugin.
//
//  Returns:    The index of a loaded plugin id.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 17, 2003
//
//  Modifications:
//    Brad Whitlock, Thu Jul 17 09:51:19 PDT 2003
//    Added a check to prevent the case where a bad id could add itself to
//    the map.
//
// ****************************************************************************

int
PluginManager::GetEnabledIndex(const std::string &id)
{
    int retval = -1;
    if(loadedindexmap.find(id) != loadedindexmap.end())
        retval = loadedindexmap[id];

    return retval;
}

// ****************************************************************************
//  Method: PluginManager::DisablePlugin
//
//  Purpose:
//    Disable a plugin if it is enabled
//
//  Arguments:
//    id        The id of a plugin
//
//  Programmer: Jeremy Meredith
//  Creation:   September 26, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Jul 17 09:51:19 PDT 2003
//    Added a check to prevent the case where a bad id could add itself to
//    the map.
//
// ****************************************************************************

void
PluginManager::DisablePlugin(const string &id)
{
    if(allindexmap.find(id) != allindexmap.end())
    {
        int index = allindexmap[id];
        if(index < enabled.size())
            enabled[index] = false;
    }
}

// ****************************************************************************
//  Method: PluginManager::EnablePlugin
//
//  Purpose:
//    Enable a plugin if it is disabled
//
//  Arguments:
//    id        The id of a plugin
//
//  Programmer: Jeremy Meredith
//  Creation:   September 26, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Jul 17 09:51:19 PDT 2003
//    Added a check to prevent the case where a bad id could add itself to
//    the map.
//
// ****************************************************************************

void
PluginManager::EnablePlugin(const string &id)
{
    if(allindexmap.find(id) != allindexmap.end())
    {
        int index = allindexmap[id];
        if(index < enabled.size())
            enabled[index] = true;
    }
}

// ****************************************************************************
// Method: PluginManager::GetPluginList
//
// Purpose: 
//   Reads the list of files that should be checked to see if they are 
//   plugins.
//
// Arguments:
//   libs : The list of filenames that we're returning.
//
// Programmer: Jeremy Meredith, Brad Whitlock
// Creation:   Fri Mar 29 09:09:16 PDT 2002
//
// Modifications:
//    Jeremy Meredith, Fri Jul  5 17:36:23 PDT 2002
//    Made it work on plugins from multiple directories.
//
//    Brad Whitlock, Thu Aug 21 14:10:51 PST 2003
//    Added code to prevent the same plugin from being in the list multiple
//    times.
//
//    Jeremy Meredith, Thu Mar 25 14:05:21 PST 2004
//    Use only the first plugin with the same name, independent of directory.
//
//    Hank Childs, Fri Jul  6 15:04:56 PDT 2007
//    Allow for duplicates.  Throwing out duplicates means that a bad private
//    plugin disallows the public one from getting loaded.
//
//    Hank Childs, Fri Oct  5 15:42:58 PDT 2007
//    Honor the precedence of the plugin directories, rather than just throwing
//    them in a giant list.
//
//    Hank Childs, Fri Nov 16 15:24:16 PST 2007
//    Fix bug pointed out by Paul Selby of AWE.
//
// ****************************************************************************

void
PluginManager::GetPluginList(vector<pair<string,string> > &libs)
{
    // Read the files in the plugin directory.
    vector< vector<pair<string,string> > > files;
    ReadPluginDir(files);

    // Add each file that is a library to the list.
    string ext(VISIT_PLUGIN_EXTENSION);
    int extLen = ext.size();

    vector< vector<pair<string,string> > > tmp;
    tmp.resize(files.size());
    for (size_t dir = 0 ; dir < files.size() ; dir++)
    {
        for (size_t f = 0; f < files[dir].size(); f++)
        {
            const string &filename = files[dir][f].second;

            // Ignore dot directories
            if (filename == "." || filename == "..")
                continue;

#define PLUGIN_MAX(A,B) (((A) < (B)) ? (B) : (A))

            // Ignore it if it does not end in the correct extension
            if (filename.length() < PLUGIN_MAX(5,extLen) ||
                !(filename.substr(filename.length()-extLen,extLen) == ext))
            {
                continue;
            }

#undef PLUGIN_MAX
            // It is a valid library name so add it to the list.
            tmp[dir].push_back(files[dir][f]);
        }
    }

    // Sort the file names, but keep the precedence of the directory
    // they came from preserved.
    for (size_t dir = 0 ; dir < tmp.size() ; dir++)
    {
        sort(tmp[dir].begin(), tmp[dir].end());
        for (size_t f = 0 ; f < tmp[dir].size() ; f++)
            libs.push_back(tmp[dir][f]);
    }
}


// ****************************************************************************
//  Method: PluginManager::ReadPluginInfo
//
//  Purpose:
//    Read general info about plugins from the shared libraries.
//    Skip plugins which do not have libraries for the current VisIt component.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 26, 2001
//
//  Modifications:
//    Brad Whitlock, Tue Nov 6 14:31:11 PST 2001
//    Added scripting plugins.
//
//    Jeremy Meredith, Thu Dec 13 11:40:28 PST 2001
//    Added error check for length of possible library name.
//    Closed the unused libI* libraries to free the file descriptors.
//
//    Jeremy Meredith, Wed Jan 16 09:48:16 PST 2002
//    Made the plugin extension not include _ser or _par.  Made the
//    engine add the _ser/_par itself based on the new "parallel" flag.
//
//    Jeremy Meredith, Fri Jul  5 17:36:23 PDT 2002
//    Made it work on plugins from multiple directories.
//
//    Jeremy Meredith, Tue Jul  9 17:43:45 PDT 2002
//    Made it only match I and GSVE plugins from the same directory.
//    Made it refuse to load more than one plugin with the same id.
//
//    Jeremy Meredith, Tue Feb  8 09:02:32 PST 2005
//    Added logic to confirm that the version the plugin was generated
//    with matches the current version of VisIt.  Added code to store
//    the appropriate list of errors.
//
//    Jeremy Meredith, Thu Mar  3 11:46:05 PST 2005
//    Track the missing vs old plugin version separately, because solving
//    them correctly requires a different action in each case.
//
//    Hank Childs, Fri Jul  6 14:44:55 PDT 2007
//    Look through the libraries for duplicates.  Previously, duplicates were
//    sorted out by GetPluginList.  But this made it difficult to load a 
//    public one if the private one was bad.  Now the duplicates get sent to
//    this function and we can load public plugins if the private ones fail to
//    load.
//    Also, dump warning messages into the debug logs as well.
//
//    Hank Childs, Thu Aug 30 08:45:02 PDT 2007
//    Clean up debug statements.
//
//    Jeremy Meredith, Wed Dec 12 16:09:38 EST 2007
//    Allow plugins to be compatible across point releases.
//
//    Kathleen Bonnell, Wed May 21 08:12:16 PDT 2008 
//    Fix libs indexing when searching for match.
//
//    Brad Whitlock, Thu Apr 23 11:55:41 PDT 2009
//    Ignore any SimV plugin if we're not in the engine and not a simulation.
//
// ****************************************************************************

void
PluginManager::ReadPluginInfo()
{
    // Get the list of plugins to load.
    vector<pair<string,string> > libs;
    GetPluginList(libs);

    // Keep track of plugin version errors
    vector<string> pluginsWithWrongVersion;
    vector<string> pluginsWithNoVersion;

    // Read the plugin info for each plugin in the libs list.
    string ext(VISIT_PLUGIN_EXTENSION);
    vector<string> alreadyLoaded;
    vector<string> alreadyLoadedDir;
    for (size_t i=0; i<libs.size(); i++)
    {
        const string &dirname  = libs[i].first;
        const string &filename = libs[i].second;
        // only try for general info libraries
        if (!IsGeneralPlugin(filename))
            continue;

        // make sure there's a match for our library
        string str;
        switch (category)
        {
          case GUI:       str = string("libG") + filename.substr(4); break;
          case Scripting: str = string("libS") + filename.substr(4); break;
          case Viewer:    str = string("libV") + filename.substr(4); break;
          case MDServer:  str = string("libM") + filename.substr(4); break;
          case Engine:    if(filename.substr(0,8) == "libISimV")
                          {
                              debug1 << "Skipping plugin " << filename
                                     << " because it is a simulation plugin."
                                     << endl;
                              continue;
                          }
                          // Fall through to Simulation
          case Simulation:str = string("libE") +
                          filename.substr(4, filename.length() - 4 - ext.size())
                          + (parallel ? string("_par") : string("_ser"))
                          + ext;
                          break;
        }
        bool match = false;
        for (size_t j=0; j<libs.size() && !match; j++)
        {
            if (libs[j].first  == dirname &&
                libs[j].second == str)
                match = true;
        }
        if (!match)
            continue;

        // see if this plugin has already been loaded, presumably because
        // we loaded a private one and now we're considering the installed
        // version.
        for (size_t j = 0 ; j < alreadyLoaded.size() ; j++)
        {
            if (filename == alreadyLoaded[j])
            {
                debug1 << "Skipping plugin " << filename << " in " << dirname 
                       << ", since a plugin by that name was already loaded "
                       << "from " << alreadyLoadedDir[j] << endl;
                continue;
            }
        }

        // We're okay, now try to open the plugin info.
        string pluginFile(dirname + VISIT_SLASH_STRING + filename);
        PluginOpen(pluginFile);
        const char **VisItPluginVersion =
                              (const char**)PluginSymbol("VisItPluginVersion");
        bool success;
        if (!VisItPluginVersion)
        {
            pluginsWithNoVersion.push_back(pluginFile);
            success = false; 
        }
        else if (!VisItVersionsCompatible(*VisItPluginVersion, VISIT_VERSION))
        {
            pluginsWithWrongVersion.push_back(pluginFile);
            success = false; 
        }
        else
        {
            success = LoadGeneralPluginInfo();
        }
        PluginClose();

        if (success)
        {
            // Add the name of the category plugin to the list of plugins
            // that will be loaded later.
            alreadyLoaded.push_back(filename);
            alreadyLoadedDir.push_back(dirname);
            libfiles.push_back(dirname + VISIT_SLASH_STRING + str);
        }
    }

    debug1 << "Succesfully loaded info about " 
           << ids.size() << " " << managerName.c_str() << " plugins.\n";

    if (pluginsWithWrongVersion.size() != 0)
    {
        pluginInitErrors += "The following plugins were built with an old "
            "version of VisIt.  Please either rebuild the plugins associated "
            "with these files or delete them:\n";
        for (size_t i=0; i<pluginsWithWrongVersion.size(); i++)
        {
            string pluginFile(pluginsWithWrongVersion[i]);
            string ext(VISIT_PLUGIN_EXTENSION);
            int slashPos = pluginFile.rfind("/");
            string dirname = pluginFile.substr(0, slashPos);
            int suffixLen = (pluginFile.find("_ser") != -1 ||
                             pluginFile.find("_par") != -1) ? 4 : 0;
            int len = pluginFile.size() - slashPos - suffixLen - 5 -
                managerName.size() - ext.size();
            string pluginPrefix(pluginFile.substr(slashPos + 5, len));
            string pluginlib(pluginFile.substr(slashPos + 1, 
                                             pluginFile.size() - (slashPos+1)));

            pluginInitErrors += string("   the ")+pluginPrefix+
                                " plugin in the directory "+dirname+"\n";
            for (size_t j = 0 ; j < alreadyLoaded.size() ; j++)
            {
                if (alreadyLoaded[j] == pluginlib)
                    pluginInitErrors += string("\t(Note that the plugin from ")
                                     + alreadyLoadedDir[j] 
                                     + " is being used in its place.)\n";
            }
        }
        pluginInitErrors += "\n";
    }

    if (pluginsWithNoVersion.size() != 0)
    {
        pluginInitErrors += "The following plugins are missing version "
            "information;  this means that their source code was generated "
            "with an old version of VisIt.  Please either regenerate the "
            "plugin info files by running xml2info and rebuild them, or else "
            "simply delete them:\n";
        for (size_t i=0; i<pluginsWithNoVersion.size(); i++)
        {
            string pluginFile(pluginsWithNoVersion[i]);
            string ext(VISIT_PLUGIN_EXTENSION);
            int slashPos = pluginFile.rfind("/");
            string dirname = pluginFile.substr(0, slashPos);
            int suffixLen = (pluginFile.find("_ser") != -1 ||
                             pluginFile.find("_par") != -1) ? 4 : 0;
            int len = pluginFile.size() - slashPos - suffixLen - 5 -
                managerName.size() - ext.size();
            string pluginPrefix(pluginFile.substr(slashPos + 5, len));

            pluginInitErrors += string("   the ")+pluginPrefix+
                                " plugin in the directory "+dirname+"\n";
        }
        pluginInitErrors += "\n";
    }

    if (pluginInitErrors != "")
    {
         debug1 << "Going to print the following message to the user: " << endl;
         debug1 << pluginInitErrors;
    }
}

// ****************************************************************************
// Method: PluginManager::BroadcastGeneralInfo
//
// Purpose: 
//   This method broadcasts the general libI information to other processors
//   using a PluginBroadcaster object.
//
// Arguments:
//   broadcaster : The broadcaster to use.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 18 11:22:52 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
PluginManager::BroadcastGeneralInfo(PluginBroadcaster *broadcaster)
{
    broadcaster->BroadcastStringVector(ids);
    broadcaster->BroadcastStringVector(names);
    broadcaster->BroadcastStringVector(versions);
    broadcaster->BroadcastStringVector(libfiles);
    broadcaster->BroadcastBoolVector(enabled);
}

// ****************************************************************************
// Method: PluginManager::ObtainPluginInfo
//
// Purpose: 
//   This method gets the plugin info by calling ReadPluginInfo or from the
//   rpiCallback function.
//
// Arguments:
//   readInfo    : Whether to read the plugin info directly.
//   broadcaster : The broadcaster to use for sending plugin info to other procs.
//
// Returns:    
//
// Note:       In most cases, ReadPluginInfo will be called. However, in parallel
//             we can install a callback function lets us share the plugin info
//             read by processor 0 with the other processors via broadcasts.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 17 10:15:11 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
PluginManager::ObtainPluginInfo(bool readInfo, PluginBroadcaster *broadcaster)
{
    if(readInfo || broadcaster == 0)
        ReadPluginInfo();

    if(broadcaster != 0)
    {
        BroadcastGeneralInfo(broadcaster);

        // If we used the broadcaster to populate ids, names, etc. then that would
        // not have set up the appropriate items in allindexmap. Do that now.
        if(allindexmap.size() == 0)
        {
            for(size_t i = 0; i < ids.size(); ++i)
                allindexmap[ids[i]] = i;
        }

        debug5 << "Shared information about " << ids.size() << " " << managerName
               << " plugins." << endl;
    }

#if 0
    // Keep this for debugging
    for(size_t i = 0; i < ids.size(); ++i)
    {
        debug1 << "plugin " << i << ":\n";
        debug1 << "\tid      = " << ids[i] << endl;
        debug1 << "\tname    = " << names[i] << endl;
        debug1 << "\tversion = " << versions[i] << endl;
        debug1 << "\tlibfile = " << libfiles[i] << endl;
        debug1 << "\tenabled = " << (enabled[i] ? "true" : "false") << endl;
        debug1 << "\tallindexmap[id] = " << allindexmap[ids[i]] << endl;
    }
#endif
}

// ****************************************************************************
// Method: PluginManager::IsGeneralPlugin
//
// Purpose: 
//   Returns whether or not the file is a common plugin.
//
// Arguments:
//   pluginFile : The name of the plugin.
//
// Returns:    Whether or not the file is a common plugin.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 29 13:29:13 PST 2002
//
// Modifications:
//   
// ****************************************************************************

bool
PluginManager::IsGeneralPlugin(const string &pluginFile) const
{
     return (pluginFile.substr(0,4) == "libI");
}

// ****************************************************************************
//  Method: PluginManager::LoadPluginsNow
//
//  Purpose:
//    Read the plugins from the shared library.
//
//  Programmer: Jeremy Meredith
//  Creation:   February 27, 2003
//
//  Modifications:
//
// ****************************************************************************

void
PluginManager::LoadPluginsNow()
{
    for (size_t i=0; i<libfiles.size(); i++)
    {
        LoadSinglePlugin(i);
    }
}

// ****************************************************************************
//  Method: PluginManager::LoadSinglePluginNow
//
//  Purpose: Load a specific plugin 
//
//  Programmer: Mark C. Miller
//  Creation:   August 5, 2007 
//
// ****************************************************************************

bool
PluginManager::LoadSinglePluginNow(const std::string& id)
{
    int index = GetAllIndex(id);
    return LoadSinglePlugin(index);
}

// ****************************************************************************
//  Method: PluginManager::LoadSinglePlugin
//
//  Purpose:
//    Read a single plugin from the shared library.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 26, 2001
//
//  Modifications:
//    Brad Whitlock, Tue Nov 6 14:29:42 PST 2001
//    Added scripting plugins.
//
//    Brad Whitlock, Fri Mar 29 12:24:28 PDT 2002
//    Moved most of the code to other methods so it is easier to port.
//
//    Jeremy Meredith, Wed Oct 29 13:16:48 PST 2003
//    Added code to catch the PluginOpen call and simply print an error
//    and fail to load the plugin, as long as we're not the viewer or
//    the gui.
//
//    Hank Childs, Fri Jan 28 13:19:33 PST 2005
//    Use catch return.
//
//    Mark C. Miller, Mon Aug  6 13:36:16 PDT 2007
//    Changed return value to bool to indicate if it actually loaded
//    the plugin.
//
//    Brad Whitlock, Thu Apr 23 11:56:46 PDT 2009
//    Added Simulation case.
//
//    Mark C. Miller, Wed Jun 17 14:27:08 PDT 2009
//    Replaced CATCHALL(...) with CATCHALL.
// ****************************************************************************

bool
PluginManager::LoadSinglePlugin(int index)
{
    if (!enabled[index])
    {
        debug1 << "Skipping disabled "<<managerName.c_str()<<" plugin "
               << names[index].c_str() << " version " << versions[index].c_str()
               << endl;
        return false;
    }

    if (PluginLoaded(ids[index]))
    {
        debug1 << "Skipping already loaded "<<managerName.c_str()<<" plugin "
               << names[index].c_str() << " version " << versions[index].c_str()
               << endl;
        return false;
    }

    // Open the plugin
    TRY
    {
        PluginOpen(libfiles[index]);
    }
    CATCHALL
    {
        if (category==GUI || category==Viewer)
        {
            // These guys can't handle failing to load a plugin
            // once they decided to load it
            RETHROW;
        }
        else
        {
            // Everyone else can!
            debug1 << "Skipping " << managerName.c_str() << " plugin "
                   << names[index].c_str()
                   << " version " << versions[index].c_str()
                   << " because it failed to open." << endl;
            CATCH_RETURN2(1, false);
        }
    }
    ENDTRY

    // Success so far -- add the handle and the info to the list
    int loadedindex = loadedhandles.size();
    loadedindexmap[ids[index]] = loadedindex;
    loadedhandles.push_back(handle);
    loadedids.push_back(ids[index]);
    switch (category)
    {
      case GUI:
        LoadGUIPluginInfo();
        break;
      case Viewer:
        LoadViewerPluginInfo();
        break;
      case MDServer:
        LoadMDServerPluginInfo();
        break;
      case Engine:
        LoadEnginePluginInfo();
        break;
      case Simulation:
        LoadEnginePluginInfo();
        break;
      case Scripting:
        LoadScriptingPluginInfo();
        break;
    }

    debug1 << "Loaded full "<<managerName.c_str()<<" plugin "
           << names[index].c_str() << " version " << versions[index].c_str()
           << endl;

    return true;
}

// ****************************************************************************
//  Method: PluginManager::LoadPluginsOnDemand
//
//  Purpose:
//    Tell the plugin manager that actual plugin data should be loaded on
//    demand, not right now.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  6, 2003
//
//  Modifications:
//    Jeremy Meredith, Tue Aug 16 11:25:07 PDT 2005
//    Enable all plugins when loading on demand.  There is currently no path
//    to inform the Engine which plugins are actually enabled (see '6489), but
//    since we are loading plugins on demand, it is simpler and perfectly
//    correct to blindly enable all plugins in this case.
//
// ****************************************************************************

void
PluginManager::LoadPluginsOnDemand()
{
    loadOnDemand = true;

    // If we are loading plugins on demand, there is no
    // reason to leave any plugin disabled.
    int nAllPlugins = ids.size();
    for (int i=0; i<nAllPlugins; i++)
    {
        EnablePlugin(ids[i]);
    }
}

// ****************************************************************************
//  Method: PluginManager::UnloadPlugins
//
//  Purpose:
//    Free the loaded plugins.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 26, 2001
//
//  Modifications:
//    Brad Whitlock, Tue Nov 6 14:32:29 PST 2001
//    Added scripting plugins.
//
//    Brad Whitlock, Fri Mar 29 09:04:03 PDT 2002
//    Changed how plugins are closed.
//
// ****************************************************************************

void
PluginManager::UnloadPlugins()
{
    FreeCommonPluginInfo();
    FreeGUIPluginInfo();
    FreeViewerPluginInfo();
    FreeMDServerPluginInfo();
    FreeEnginePluginInfo();
    FreeScriptingPluginInfo();

    // Close the plugins.
    for (size_t i=0; i < loadedhandles.size(); i++)
    {
        handle = loadedhandles[i];
        PluginClose();
    }
    loadedhandles.clear();

    loadedindexmap.clear();
}

// ****************************************************************************
//  Method: PluginManager::ReloadPlugins
//
//  Purpose:
//    Free the loaded plugins.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 20, 2002
//
//  Modifications:
//    Jeremy Meredith, Fri Feb 28 12:25:37 PST 2003
//    Made it use LoadPluginsNow or LoadPluginsOnDemand as appropriate.
//
// ****************************************************************************

void
PluginManager::ReloadPlugins()
{
    UnloadPlugins();

    if (loadOnDemand)
        LoadPluginsOnDemand();
    else
        LoadPluginsNow();
}

// ****************************************************************************
// Method: PluginManager::GetAllIndex
//
// Purpose: 
//   Returns the index of the specified plugin within the plugin manager.
//
// Arguments:
//   id : The id of the plugin that we're looking for.
//
// Returns:    The index of the specified plugin or -1 if the lookup fails.
//
// Note:       This is useful for determining if a plugin id exists in the
//             plugin list.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 24 11:30:49 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

int
PluginManager::GetAllIndex(const string &id) const
{
    //
    // Look through the entire list of plugins for the specified name.
    //
    int pluginTypeIndex = -1;
    for(size_t i = 0; i < ids.size(); ++i)
    {
        if(id == ids[i])
        {
            pluginTypeIndex = i;
            break;
        }
    }

    return pluginTypeIndex;
}

// ****************************************************************************
// Method: PluginManager::GetAllIndexFromName
//
// Purpose: 
//   Returns the index of the specified plugin within the plugin manager.
//
// Arguments:
//   name : The name of the plugin that we're looking for.
//
// Returns:    The index of the specified plugin or -1 if the lookup fails.
//
// Note:       This is useful for determining if a plugin id exists in the
//             plugin list.
//
// Programmer: Kathleen Bonnell 
// Creation:   April 25, 2002 
//
// Modifications:
//   
// ****************************************************************************

int
PluginManager::GetAllIndexFromName(const string &name) const
{
    //
    // Look through the entire list of plugins for the specified name.
    //
    int pluginTypeIndex = -1;
    for(size_t i = 0; i < names.size(); ++i)
    {
        if(name == names[i])
        {
            pluginTypeIndex = i;
            break;
        }
    }

    return pluginTypeIndex;
}

// ****************************************************************************
//  Method:  PluginManager::GetPluginInitializationErrors
//
//  Purpose:
//    Return errors generated by plugin initialization, then clear
//    them out so we don't wind up reporting errors twice.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  7, 2005
//
// ****************************************************************************

std::string
PluginManager::GetPluginInitializationErrors()
{
    std::string ret = pluginInitErrors;
    pluginInitErrors = "";
    return ret;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///
/// Methods that have conditionally compiled code for multiple platforms.
///
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: AddUniquePluginDir
//
// Purpose: 
//   Adds a plugin directory to the pluginDirs vector if it has not been added
//   yet. This prevents a plugin directory from being added more than once
//   as can happen when the user provides a redundant -plugindir command line
//   option.
//
// Arguments:
//   path : The path to add.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 17 16:26:31 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
AddUniquePluginDir(stringVector &pluginDirs, const std::string &path)
{
    if(std::find(pluginDirs.begin(), pluginDirs.end(), path) == pluginDirs.end())
        pluginDirs.push_back(path);
}

// ****************************************************************************
// Method: PluginManager::SetPluginDir
//
// Purpose: 
//   Sets the plugin directory.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 29 13:50:50 PST 2002
//
// Modifications:
//    Jeremy Meredith, Fri Jul  5 17:36:23 PDT 2002
//    Made it work on plugins from multiple directories.
//   
//    Brad Whitlock, Wed Jul 24 12:27:04 PDT 2002
//    I made it only use a single plugin directory on Windows since the
//    parsing algorithm for UNIX directories is inadequate for Windows
//    directory names.
//
//    Brad Whitlock, Wed Nov 22 16:25:19 PST 2006
//    I made it possible to pass in the plugin directory. If it does not
//    get passed in (the default) then we use VISITPLUGINDIR.
//    
//    Sean Ahern, Thu Jan 17 15:52:42 EST 2008
//    Only fail the environment exception if the plugin directory hasn't been
//    set in a prior call to SetPluginDir.
//
//    Kathleen Bonnell, Wed May 21 08:12:16 PDT 2008 
//    Modified path-parsing for Windows.  ';' is the only valid separator
//    between paths since ':' could indicate a drive.
//
//    Brad Whitlock, Wed Jun 17 16:28:14 PDT 2009
//    I made it use AddUniquePluginDir so we don't add a path multiple times.
//
// ****************************************************************************

void
PluginManager::SetPluginDir(const char *PluginDir)
{
    const char *plugindir = 0;
    if (PluginDir == 0)
    {
        // Get the plugin directory from the environment.
        plugindir = getenv("VISITPLUGINDIR");
        if (!plugindir)
        {
            // No environment variable found.  If we have directories in the
            // list already, this isn't a problem.
            if (pluginDirs.empty())
            {
                debug4 << "No environment variable!" << endl;
#if defined(_WIN32)
                char *tmp = new char[MAX_PATH];
                if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL,
                                  SHGFP_TYPE_CURRENT, tmp)))
                {
                    PathAppend(tmp, "LLNL");
                    PathAppend(tmp, "VisIt");
                    AddUniquePluginDir(pluginDirs, string(tmp) + VISIT_SLASH_STRING + 
                                       managerName + "s");
                    delete [] tmp;
                    return;
                }
                else
                {
                    delete [] tmp;
                    EXCEPTION1(VisItException,
                        "The path to AppData variable could not be found.");
                }
 #else
                EXCEPTION1(VisItException,
                    "The environment variable VISITPLUGINDIR must be defined.");
#endif
            }
            else
            {
                // The pluginDirs list is already populated.  So not having an
                // environment variable is okay.  We got the list from somewhere
                // else (-plugindir option?).
                return;
            }
        }
    }
    else
    {
        // Manually set the plugin directory.
        plugindir = PluginDir;
    }

    const char *c = plugindir;
    while (*c)
    {
        string dir;
#ifndef _WIN32
        while (*c && *c!=':' && *c!=';')
#else
        while (*c && *c!=';')
#endif
        {
            dir += *c;
            c++;
        }
        if (!dir.empty())
        {
            AddUniquePluginDir(pluginDirs, string(dir) + VISIT_SLASH_STRING + managerName + "s");
        }
        dir = "";
        if (*c)
            c++;
    }
}

// ****************************************************************************
// Method: PluginManager::ReadPluginDir
//
// Purpose: 
//   Reads the files in the plugin directory.
//
// Arguments:
//   files : The return list of files.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 29 09:32:57 PDT 2002
//
// Modifications:
//    Jeremy Meredith, Fri Jul  5 17:36:23 PDT 2002
//    Made it work on plugins from multiple directories.
//
//    Brad Whitlock, Mon Jul 15 16:41:59 PST 2002
//    I fixed the win32 implementation.
//
//    Hank Childs, Fri Oct  5 13:08:55 PDT 2007
//    Changed argument to make separation of directories easier.
//
// ****************************************************************************

void
PluginManager::ReadPluginDir(vector< vector<pair<string,string> > > &files)
{
    files.resize(pluginDirs.size());
    for (size_t i=0; i<pluginDirs.size(); i++)
    {
        string pluginDir(pluginDirs[i]);

#if defined(_WIN32)
        string searchPath(pluginDir + string("\\*"));
        WIN32_FIND_DATA fd;
        HANDLE dirHandle = FindFirstFile(searchPath.c_str(), &fd);
        if(dirHandle != INVALID_HANDLE_VALUE)
        {
            while(FindNextFile(dirHandle, &fd))
            {
                files[i].push_back(pair<string,string>(pluginDir,fd.cFileName));
            }
            FindClose(dirHandle);
        }
#else
        // Open the directory
        DIR *dir = opendir(pluginDir.c_str());
        if (dir)
        {
            // Add each file in the directory to the list.
            dirent  *ent;
            while ((ent = readdir(dir)) != NULL)
                files[i].push_back(pair<string,string>(pluginDir,ent->d_name));
            closedir(dir);
        }
        //else
        //    EXCEPTION1(InvalidDirectoryException, pluginDir.c_str());
#endif
    }
}

// ****************************************************************************
// Method: PluginManager::PluginOpen
//
// Purpose: 
//   Opens a plugin file and keeps a handle to it.
//
// Arguments:
//   pluginFile : The plugin filename.
//
// Programmer: Jeremy Meredith, Brad Whitlock
// Creation:   Fri Mar 29 11:58:42 PDT 2002
//
// Modifications:
//    Jeremy Meredith, Tue Feb  8 09:06:53 PST 2005
//    Told PluginSymbol not to complain if it could not find
//    the g++ initialization code.
//
//    Hank Childs, Thu Jan 19 17:05:49 PST 2006
//    Print out plugin errors to the screen.  ['6629]
//
//    Mark C. Miller, Thu Aug 14 01:22:59 PDT 2008
//    Made messages reported to screen include plugin name and error msg.
// ****************************************************************************

void
PluginManager::PluginOpen(const string &pluginFile)
{
#if defined(_WIN32)
    HINSTANCE lib = LoadLibrary(pluginFile.c_str());
    if(!lib)
    {
        const char *pluginError = PluginError();
        cerr << "Error opening plugin file: " << pluginFile
             << " (" << pluginError << ")" << endl;
        EXCEPTION3(InvalidPluginException, "Error opening plugin file",
                   pluginFile.c_str(), pluginError);
    }

    handle = (void *)lib;
#else
    // dlopen the plugin
    handle = dlopen(pluginFile.c_str(), RTLD_LAZY);
    if (!handle)
    {
        const char *pluginError = PluginError();
        cerr << "Error opening plugin file: " << pluginFile
             << " (" << pluginError << ")" << endl;
        EXCEPTION3(InvalidPluginException, "Error opening plugin file",
                   pluginFile.c_str(), pluginError);
    }

    // Try to initialize static constructors.  This is a g++ism.
    void (*init)(void) = (void(*)(void))PluginSymbol("_GLOBAL__DI", true);
    if (init)
        init();
#endif

    openPlugin = pluginFile;
}

// ****************************************************************************
// Method: PluginManager::PluginSymbol
//
// Purpose: 
//   Reads a symbol from a shared library and returns a pointer to it.
//
// Arguments:
//   symbol : The symbol to find.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 29 12:03:46 PDT 2002
//
// Modifications:
//   Brad Whitlock, Thu Aug 21 14:14:40 PST 2003
//   Added a special implementation for MacOS X.
//
//   Brad Whitlock, Wed Mar 10 09:36:16 PDT 2004
//   Added code to print the error message if the symbol could not be found.
//   I did this here because most functions that call PluginSymbol use the
//   return value to throw an exception without calling PluginError themselves.
//   That prevents useful information from being printed to the debug logs.
//
//   Brad Whitlock, Tue Sep 14 13:27:35 PST 2004
//   I made the debug output go to debug5.
//
//   Brad Whitlock, Mon Mar 7 12:23:06 PDT 2005
//   I made the plugin version be handled a little differently on the Mac
//   because the names that were given to the plugin versions for the Mac
//   in all of the plugins don't have underscores in them and I don't want
//   to change them in all of the plugins. Someday we should though.
//
//   Cyrus Harrison, Wed Jun 20 14:05:58 PDT 2007
//   Changed __DARWIN__ back to __APPLE__
//
// ****************************************************************************

void *
PluginManager::PluginSymbol(const string &symbol, bool noError)
{
    void *retval;
#if defined(_WIN32)
    retval = (void *)GetProcAddress((HMODULE)handle, symbol.c_str());
#elif defined(__APPLE__)
    string symbolName(symbol);

    //
    // If the symbol that we want begins with "Get" then we're most likely
    // trying to access a plugin and in order to get plugins to work on MacOS X
    // in a flat namespace, I changed the name of the access function so that
    // it has the name of the plugin prepended to it in order to create a unique
    // function name. Here, we use the name of the plugin file to determine the
    // name of the plugin so we can create the symbol that we're really after.
    //
    bool pluginVersion = (symbol == "VisItPluginVersion");
    if(pluginVersion || symbol.substr(0,3) == "Get")
    {
        string ext(VISIT_PLUGIN_EXTENSION);
        int slashPos = openPlugin.rfind("/");
        int suffixLen = (openPlugin.find("_ser") != -1 ||
                         openPlugin.find("_par") != -1) ? 4 : 0;
        int len = openPlugin.size() - slashPos - suffixLen - 5 -
                  managerName.size() - ext.size();
        string pluginPrefix(openPlugin.substr(slashPos + 5, len));
//        debug4 << "PluginSymbol: prefix: " << pluginPrefix << endl;
        if(pluginVersion)
            symbolName = string(pluginPrefix + symbol);
        else
            symbolName = string(pluginPrefix + "_" + symbol);
//        debug4 << "PluginSymbol: sym: " << symbolName << endl;
    }
    
    retval = dlsym(handle, symbolName.c_str());
#else
    retval = dlsym(handle, symbol.c_str());
#endif

    // If the symbol was not found, print the error message if appropriate.
    if (retval == 0 && !noError)
        debug5 << PluginError() << endl;

    return retval;
}

// ****************************************************************************
// Method: PluginManager::PluginError
//
// Purpose: 
//   Returns the last plugin error message.
//
// Returns:    The last plugin error message.
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 2 09:56:41 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

char *
PluginManager::PluginError() const
{
#if defined(_WIN32)
    va_list *va = 0;
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0,
                  pluginError, MAX_PLUGINERROR, va);
#else
    strncpy(pluginError, dlerror(), MAX_PLUGINERROR);
#endif
    return pluginError;
}

// ****************************************************************************
// Method: PluginManager::PluginClose
//
// Purpose: 
//   Closes the open plugin.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 29 10:00:07 PDT 2002
//
// Modifications:
//    Jeremy Meredith, Tue Mar 18 13:24:05 PST 2003
//    Added destruction of static objects.
//
//    Jeremy Meredith, Tue Feb  8 09:06:53 PST 2005
//    Told PluginSymbol not to complain if it could not find
//    the g++ destruction code.
//
// ****************************************************************************

void
PluginManager::PluginClose()
{
#if defined(_WIN32)
    if(handle)
    {
        FreeLibrary((HMODULE)handle);
        handle = 0;
    }
#else
    // Try to destruct static objects.  This is a g++ism.
    void (*fini)(void) = (void(*)(void))PluginSymbol("_GLOBAL__DD", true);
    if (fini)
        fini();
        
    if(handle)
    {
        if (dlclose(handle) != 0)
        {
            const char *pluginError = PluginError();
            cerr << "Error closing plugin file: " << openPlugin 
                 << " (" << pluginError << ")" << endl;
        }
        handle = 0;
    }
#endif

    openPlugin = "";
}
