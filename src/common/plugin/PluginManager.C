// ************************************************************************* //
//                              PluginManager.C                              //
// ************************************************************************* //

#include <PluginManager.h>
#include <DebugStream.h>
#include <InvalidDirectoryException.h>
#include <InvalidPluginException.h>
#include <visit-config.h>
#include <visitstream.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#if defined(_WIN32)
#include <windows.h>
#include <direct.h>
#elif defined(__APPLE__)
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
// ****************************************************************************

PluginManager::PluginManager(const string &mgr)
{
    pluginInitErrors = "";
    loadOnDemand = false;
    managerName = mgr;
    category = no_category;
    handle = 0;
    pluginError = new char[MAX_PLUGINERROR];
    SetPluginDir();
}

// ****************************************************************************
//  Method: PluginManager destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   September 26, 2001
//
//  Modifications:
//
// ****************************************************************************

PluginManager::~PluginManager()
{
    delete [] pluginError;
    UnloadPlugins();
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
// ****************************************************************************

void
PluginManager::GetPluginList(vector<pair<string,string> > &libs)
{
    // Read the files in the plugin directory.
    vector<pair<string,string> > files;
    ReadPluginDir(files);

    // Add each file that is a library to the list.
    string ext(PLUGIN_EXTENSION);
    int extLen = ext.size();

    for(int i = 0; i < files.size(); ++i)
    {
        const string &filename = files[i].second;

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
        bool found = false;
        for(int j = 0; j < libs.size() && !found; ++j)
            found = (libs[j].second == files[i].second);
    
        if(!found)
            libs.push_back(files[i]);
    }

    // Sort the library filename list.
    sort(libs.begin(), libs.end());
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
    string ext(PLUGIN_EXTENSION);
    for (int i=0; i<libs.size(); i++)
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
          case Engine:    str = string("libE") +
                              filename.substr(4, filename.length() - 4 - ext.size())
                              + (parallel ? string("_par") : string("_ser"))
                              + ext;
                          break;
        }
        bool match = false;
        for (int j=0; j<libs.size() && !match; j++)
        {
            if (libs[i].first  == dirname &&
                libs[j].second == str)
                match = true;
        }
        if (!match)
            continue;

        // We're okay, now try to open the plugin info.
        string pluginFile(dirname + SLASH_STRING + filename);
        PluginOpen(pluginFile);
        const char **VisItPluginVersion =
                              (const char**)PluginSymbol("VisItPluginVersion");
        bool success;
        if (!VisItPluginVersion)
        {
            pluginsWithNoVersion.push_back(pluginFile);
            success = false; 
        }
        else if (strcmp(*VisItPluginVersion, VERSION) != 0)
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
            libfiles.push_back(dirname + SLASH_STRING + str);
        }
    }

    debug1 << "Succesfully loaded info about " 
           << ids.size() << " " << managerName.c_str() << " plugins.\n";

    if (pluginsWithWrongVersion.size() != 0)
    {
        pluginInitErrors += "The following plugins were built with an old "
            "version of VisIt.  Please either rebuild the plugins associated "
            "with these files or delete them:\n";
        for (int i=0; i<pluginsWithWrongVersion.size(); i++)
        {
            string pluginFile(pluginsWithWrongVersion[i]);
            string ext(PLUGIN_EXTENSION);
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

    if (pluginsWithNoVersion.size() != 0)
    {
        pluginInitErrors += "The following plugins are missing version "
            "information;  this means that their source code was generated "
            "with an old version of VisIt.  Please either regenerate the "
            "plugin info files by running xml2info and rebuild them, or else "
            "simply delete them:\n";
        for (int i=0; i<pluginsWithNoVersion.size(); i++)
        {
            string pluginFile(pluginsWithNoVersion[i]);
            string ext(PLUGIN_EXTENSION);
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
    for (int i=0; i<libfiles.size(); i++)
    {
        LoadSinglePlugin(i);
    }
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
// ****************************************************************************

void
PluginManager::LoadSinglePlugin(int index)
{
    if (!enabled[index])
    {
        debug1 << "Skipping disabled "<<managerName.c_str()<<" plugin "
               << names[index].c_str() << " version " << versions[index].c_str()
               << endl;
        return;
    }

    if (PluginLoaded(ids[index]))
    {
        debug1 << "Skipping already loaded "<<managerName.c_str()<<" plugin "
               << names[index].c_str() << " version " << versions[index].c_str()
               << endl;
        return;
    }

    // Open the plugin
    TRY
    {
        PluginOpen(libfiles[index]);
    }
    CATCHALL(...)
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
            CATCH_RETURN(1);
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
      case Scripting:
        LoadScriptingPluginInfo();
        break;
    }

    debug1 << "Loaded full "<<managerName.c_str()<<" plugin "
           << names[index].c_str() << " version " << versions[index].c_str()
           << endl;
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
    int i;

    FreeCommonPluginInfo();
    FreeGUIPluginInfo();
    FreeViewerPluginInfo();
    FreeMDServerPluginInfo();
    FreeEnginePluginInfo();
    FreeScriptingPluginInfo();

    // Close the plugins.
    for (i=0; i < loadedhandles.size(); i++)
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
    for(int i = 0; i < ids.size(); ++i)
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
    for(int i = 0; i < names.size(); ++i)
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

#if defined(__APPLE__)
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///
/// Methods that simulate dlopen, dlsym, dlclose, and dlerror on MacOS X.
///
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <stdarg.h>

#define ERR_STR_LEN 1000
#define RTLD_LAZY   1
#define RTLD_GLOBAL 2

/* Set and get the error string for use by dlerror */
static const char *dlerror_helper(int setget, const char *str, ...)
{
    static char errstr[ERR_STR_LEN];
    static int err_filled = 0;
    const char *retval;
    NSLinkEditErrors ler;
    int lerno;
    const char *dylderrstr;
    const char *file;
    va_list arg;
    if (setget <= 0)
    {
        va_start(arg, str);
        strncpy(errstr, "dlsimple: ", ERR_STR_LEN);
        vsnprintf(errstr + 10, ERR_STR_LEN - 10, str, arg);
        va_end(arg);
        /* We prefer to use the dyld error string if getset is 1*/
        if (setget == 0) {
            NSLinkEditError(&ler, &lerno, &file, &dylderrstr);
            //fprintf(stderr,"dyld: %s\n",dylderrstr);
            if (dylderrstr && strlen(dylderrstr))
                strncpy(errstr,dylderrstr,ERR_STR_LEN);
        }       
        err_filled = 1;
        retval = NULL;
    }
    else
    {
        if (!err_filled)
            retval = NULL;
        else
            retval = errstr;
        err_filled = 0;
    }
    return retval;
}

const char *dlerror(void)
{
    return dlerror_helper(1, (char *)NULL);
}


// ****************************************************************************
// Modifications:
//
//   Hank Childs, Fri Jan 28 13:19:33 PST 2005
//   Change comment style so escan will not pick up false positives.
//
// ****************************************************************************

void *dlopen(const char *path, int mode)
{
    void *module = 0;
    NSObjectFileImage ofi = 0;
    NSObjectFileImageReturnCode ofirc;
    static int (*make_private_module_public) (NSModule module) = 0;
    unsigned int flags =  NSLINKMODULE_OPTION_RETURN_ON_ERROR |
                          NSLINKMODULE_OPTION_PRIVATE;

    /* If we got no path, the app wants the global namespace,
       use -1 as the marker in this case */
    if (!path)
        return (void *)-1;

    /* Create the object file image, works for things linked
       with the -bundle arg to ld */
    ofirc = NSCreateObjectFileImageFromFile(path, &ofi);
    switch (ofirc)
    {
        case NSObjectFileImageSuccess:
            /* It was okay, so use NSLinkModule to link in the image */
            if (!(mode & RTLD_LAZY)) flags += NSLINKMODULE_OPTION_BINDNOW;
            module = NSLinkModule(ofi, path,flags);
            /* Don't forget to destroy the object file
               image, unless you like leaks */
            NSDestroyObjectFileImage(ofi);
            /* If the mode was global, then change the module, this avoids
               multiply defined symbol errors to first load private then
               make global. Silly, isn't it. */
            if ((mode & RTLD_GLOBAL))
            {
              if (!make_private_module_public)
              {
                _dyld_func_lookup("__dyld_NSMakePrivateModulePublic", 
                (unsigned long *)&make_private_module_public);
              }
              make_private_module_public(module);
            }
            break;
        case NSObjectFileImageInappropriateFile:
            // It may have been a dynamic library rather
            // than a bundle, try to load it
            module = (void *)NSAddImage(path, 
                        NSADDIMAGE_OPTION_RETURN_ON_ERROR);
            break;
        case NSObjectFileImageFailure:
            dlerror_helper(0,"Object file setup failure :  \"%s\"", path);
            return 0;
        case NSObjectFileImageArch:
            dlerror_helper(0,"No object for this architecture :  \"%s\"", path);
            return 0;
        case NSObjectFileImageFormat:
            dlerror_helper(0,"Bad object file format :  \"%s\"", path);
            return 0;
        case NSObjectFileImageAccess:
            dlerror_helper(0,"Can't read object file :  \"%s\"", path);
            return 0;       
    }
    if (!module)
        dlerror_helper(0, "Can not open \"%s\"", path);
    return module;
}

int dlclose(void *handle)
{
    if ((((struct mach_header *)handle)->magic == MH_MAGIC) ||
        (((struct mach_header *)handle)->magic == MH_CIGAM))
    {
        dlerror_helper(-1, "Can't remove dynamic libraries on darwin");
        return 0;
    }
    if (!NSUnLinkModule(handle, 0))
    {
        dlerror_helper(0, "unable to unlink module %s", NSNameOfModule(handle));
        return 1;
    }
    return 0;
}

/* dlsymIntern is used by dlsym to find the symbol */
void *dlsymIntern(void *handle, const char *symbol)
{
    NSSymbol *nssym = 0;
    /* If the handle is -1, if is the app global context */
    if (handle == (void *)-1)
    {
        /* Global context, use NSLookupAndBindSymbol */
        if (NSIsSymbolNameDefined(symbol))
        {
            nssym = NSLookupAndBindSymbol(symbol);
        }

    }
    /* Now see if the handle is a struch mach_header* or not,
       use NSLookupSymbol in image for libraries, and
       NSLookupSymbolInModule for bundles */
    else
    {
        /* Check for both possible magic numbers depending
           on x86/ppc byte order */
        if ((((struct mach_header *)handle)->magic == MH_MAGIC) ||
            (((struct mach_header *)handle)->magic == MH_CIGAM))
        {
            if (NSIsSymbolNameDefinedInImage((struct mach_header *)handle,
                symbol))
            {
                nssym = NSLookupSymbolInImage((struct mach_header *)handle,
                            symbol, NSLOOKUPSYMBOLINIMAGE_OPTION_BIND
                          | NSLOOKUPSYMBOLINIMAGE_OPTION_RETURN_ON_ERROR);
            }

        }
        else
        {
            nssym = NSLookupSymbolInModule(handle, symbol);
        }
    }
    if (!nssym)
    {
        dlerror_helper(0, "Symbol \"%s\" Not found", symbol);
        return NULL;
    }
    return NSAddressOfSymbol(nssym);
}

/* dlsym, prepend the underscore and call dlsymIntern */
void *dlsym(void *handle, const char *symbol)
{
    static char undersym[257];  /* Saves calls to malloc(3) */
    int sym_len = strlen(symbol);
    void *value = NULL;
    char *malloc_sym = NULL;

    if (sym_len < 256)
    {
        snprintf(undersym, 256, "_%s", symbol);
        value = dlsymIntern(handle, undersym);
    }
    else
    {
        malloc_sym = malloc(sym_len + 2);
        if (malloc_sym)
        {
            sprintf(malloc_sym, "_%s", symbol);
            value = dlsymIntern(handle, malloc_sym);
            free(malloc_sym);
        }
        else
        {
            dlerror_helper(-1, "Unable to allocate memory");
        }
    }
    return value;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///
/// Methods that have conditionally compiled code for multiple platforms.
///
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

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
// ****************************************************************************

void
PluginManager::SetPluginDir()
{
    // Get the plugin directory from the environment.
    char *plugindir = getenv("VISITPLUGINDIR");
    if (!plugindir)
    {
#if defined(_WIN32)
        plugindir = "C:\\VisItWindows\\bin";
#else
        EXCEPTION1(VisItException,
                   "The environment variable VISITPLUGINDIR must be defined.");
#endif
    }
    
#if defined(_WIN32)
    pluginDirs.push_back(string(plugindir) + SLASH_STRING + managerName + "s");
#else
    char *c = plugindir;
    while (*c)
    {
        string dir;
        while (*c && *c!=':' && *c!=';')
        {
            dir += *c;
            c++;
        }
        if (!dir.empty())
        {
            pluginDirs.push_back(string(dir) + SLASH_STRING + managerName + "s");
        }
        dir = "";
        if (*c)
            c++;
    }
#endif
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
// ****************************************************************************

void
PluginManager::ReadPluginDir(vector<pair<string,string> > &files)
{
    for (int i=0; i<pluginDirs.size(); i++)
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
                files.push_back(pair<string,string>(pluginDir,fd.cFileName));
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
                files.push_back(pair<string,string>(pluginDir,ent->d_name));
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
// ****************************************************************************

void
PluginManager::PluginOpen(const string &pluginFile)
{
#if defined(_WIN32)
    HINSTANCE lib = LoadLibrary(pluginFile.c_str());
    if(!lib)
    {
        EXCEPTION3(InvalidPluginException, "Error opening plugin file",
                   pluginFile.c_str(), PluginError());
    }

    handle = (void *)lib;
#else
    // dlopen the plugin
    handle = dlopen(pluginFile.c_str(), RTLD_LAZY);
    if (!handle)
    {
        EXCEPTION3(InvalidPluginException, "Error opening plugin file",
                   pluginFile.c_str(), PluginError());
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
        string ext(PLUGIN_EXTENSION);
        int slashPos = openPlugin.rfind("/");
        int suffixLen = (openPlugin.find("_ser") != -1 ||
                         openPlugin.find("_par") != -1) ? 4 : 0;
        int len = openPlugin.size() - slashPos - suffixLen - 5 -
                  managerName.size() - ext.size();
        string pluginPrefix(openPlugin.substr(slashPos + 5, len));
        debug4 << "PluginSymbol: prefix: " << pluginPrefix << endl;
        if(pluginVersion)
            symbolName = string(pluginPrefix + symbol);
        else
            symbolName = string(pluginPrefix + "_" + symbol);
        debug4 << "PluginSymbol: sym: " << symbolName << endl;
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
    openPlugin = "";

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
        dlclose(handle);
        handle = 0;
    }
#endif
}
