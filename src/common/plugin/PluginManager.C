// ************************************************************************* //
//                              PluginManager.C                              //
// ************************************************************************* //

#include <PluginManager.h>
#include <DebugStream.h>
#include <InvalidDirectoryException.h>
#include <InvalidPluginException.h>
#include <visit-config.h>
#include <iostream.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#if defined(_WIN32)
#include <windows.h>
#include <direct.h>
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
// ****************************************************************************

PluginManager::PluginManager(const string &mgr)
{
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
// ****************************************************************************

string
PluginManager::GetPluginName(const string &id)
{
    return names[allindexmap[id]];
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
// ****************************************************************************

string
PluginManager::GetPluginVersion(const string &id)
{
    return versions[allindexmap[id]];
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
// ****************************************************************************

int
PluginManager::GetEnabledIndex(const std::string &id)
{
    return loadedindexmap[id];
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
// ****************************************************************************

void
PluginManager::DisablePlugin(const string &id)
{
    enabled[allindexmap[id]] = false;
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
// ****************************************************************************

void
PluginManager::EnablePlugin(const string &id)
{
    enabled[allindexmap[id]] = true;
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
// ****************************************************************************

void
PluginManager::ReadPluginInfo()
{
    // Get the list of plugins to load.
    vector<pair<string,string> > libs;
    GetPluginList(libs);

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
        bool success = LoadGeneralPluginInfo();
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
    PluginOpen(libfiles[index]);

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

    debug1 << "Loaded "<<managerName.c_str()<<" plugin info for "
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
// ****************************************************************************

void
PluginManager::LoadPluginsOnDemand()
{
    loadOnDemand = true;
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
    void (*init)(void) = (void(*)(void))PluginSymbol("_GLOBAL__DI");
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
//   
// ****************************************************************************

void *
PluginManager::PluginSymbol(const string &symbol)
{
    void *retval;
#if defined(_WIN32)
    retval = (void *)GetProcAddress((HMODULE)handle, symbol.c_str());
#else
    retval = dlsym(handle, symbol.c_str());
#endif
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
// ****************************************************************************

void
PluginManager::PluginClose()
{
    // Try to destruct static objects.  This is a g++ism.
    void (*fini)(void) = (void(*)(void))PluginSymbol("_GLOBAL__DD");
    if (fini)
        fini();

    openPlugin = "";

#if defined(_WIN32)
    if(handle)
    {
        FreeLibrary((HMODULE)handle);
        handle = 0;
    }
#else
    if(handle)
    {
        dlclose(handle);
        handle = 0;
    }
#endif
}
