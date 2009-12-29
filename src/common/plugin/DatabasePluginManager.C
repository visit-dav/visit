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
//                          DatabasePluginManager.C                          //
// ************************************************************************* //

#include <DatabasePluginManager.h>
#include <DatabasePluginInfo.h>
#include <PluginBroadcaster.h>
#include <DebugStream.h>
#include <InvalidPluginException.h>
#include <Utility.h>
#include <visitstream.h>
#include <visit-config.h>
#include <string>
#include <vector>

#include <cstring>

using std::string;
using std::vector;

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
//    Sean Ahern, Thu Jan 17 16:06:20 EST 2008
//    Moved the call to SetPluginDir to Initialize to make it conform
//    with the rest of the plugin managers.
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
//    Brad Whitlock, Wed Jun 25 10:27:17 PDT 2008
//    Call UnloadPlugins here since it calls virtual methods for this class.
//
// ****************************************************************************

DatabasePluginManager::~DatabasePluginManager()
{
    UnloadPlugins();
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
//    pluginDir     :   Allows us to pass in the plugin dir that we want to use.
//    readInfo      :   Whether the plugin info should be read directly.
//    broadcaster   :   An object that can be used to broadcast plugin info.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 22, 2002
//
//  Modifications:
//    Sean Ahern, Thu Jan 17 16:10:00 EST 2008
//    Added SetPluginDir so that we can find plugins correctly.  Arguably,
//    this is better here than the constructor.
//
//    Brad Whitlock, Tue Jun 24 11:13:19 PDT 2008
//    Removed plugin characteristics.
//
//    Brad Whitlock, Wed Jun 17 13:05:54 PDT 2009
//    I added readInfo, broadcaster arguments to allow parallel optimizations.
//
// ****************************************************************************

void
DatabasePluginManager::Initialize(const PluginCategory pluginCategory,
    bool par, const char *pluginDir, bool readInfo, PluginBroadcaster *broadcaster)
{
    category = pluginCategory;
    parallel = par;
    SetPluginDir(pluginDir);
    ObtainPluginInfo(readInfo, broadcaster);
}

// ****************************************************************************
// Method: DatabasePluginManager::BroadcastGeneralInfo
//
// Purpose: 
//   Broadcasts the general libI plugin information to other processors.
//
// Arguments:
//   broadcaster : The broadcaster object to use.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 18 11:30:15 PDT 2009
//
// Modifications:
//    Jeremy Meredith, Tue Dec 29 11:40:42 EST 2009
//    Replaced "Extensions" and "Filenames" with "FilePatterns".  Added
//    filePatternsStrict and opensWholeDirectory.
//   
// ****************************************************************************

void
DatabasePluginManager::BroadcastGeneralInfo(PluginBroadcaster *broadcaster)
{
    PluginManager::BroadcastGeneralInfo(broadcaster);

    broadcaster->BroadcastBoolVector(haswriter);
    broadcaster->BroadcastStringVectorVector(filePatterns);
    broadcaster->BroadcastBoolVector(filePatternsAreStrict);
    broadcaster->BroadcastBoolVector(opensWholeDirectory);
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
//    Mark C. Miller, Mon Aug  6 13:36:16 PDT 2007
//    Fixed problem where calling this method in a situation where the given
//    plugin had not already been loaded would result in defining it in
//    loadedindexmap with an index of 0.
//
//    Brad Whitlock, Tue Jun 24 16:24:25 PDT 2008
//    Added a pointer to this in the info.
//
// ****************************************************************************

EngineDatabasePluginInfo *
DatabasePluginManager::GetEnginePluginInfo(const string &id)
{
    if (!PluginLoaded(id))
        return 0;
    EngineDatabasePluginInfo *info = enginePluginInfo[loadedindexmap[id]];
    info->SetPluginManager(this);
    return info;
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
//    Mark C. Miller, Mon Aug  6 13:36:16 PDT 2007
//    Fixed problem where calling this method in a situation where the given
//    plugin had not already been loaded would result in defining it in
//    loadedindexmap with an index of 0.
//
//    Brad Whitlock, Tue Jun 24 16:24:25 PDT 2008
//    Added a pointer to this in the info.
//
// ****************************************************************************

CommonDatabasePluginInfo *
DatabasePluginManager::GetCommonPluginInfo(const string &id)
{
    if (!PluginLoaded(id))
        return 0;
    CommonDatabasePluginInfo *info = commonPluginInfo[loadedindexmap[id]];
    info->SetPluginManager(this);
    return info;
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
//    Mark C. Miller, Mon Aug  6 13:36:16 PDT 2007
//    Added code to update extensions and filenames.
//
//    Jeremy Meredith, Tue Dec 29 11:40:42 EST 2009
//    Replaced "Extensions" and "Filenames" with "FilePatterns".  Added
//    filePatternsStrict and opensWholeDirectory.
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
    ids       .push_back(info->GetID());
    names     .push_back(info->GetName());
    versions  .push_back(info->GetVersion());
    enabled   .push_back(info->EnabledByDefault());
    haswriter .push_back(info->HasWriter());
    filePatterns.push_back(info->GetDefaultFilePatterns());
    filePatternsAreStrict.push_back(info->AreDefaultFilePatternsStrict());
    opensWholeDirectory.push_back(info->OpensWholeDirectory());
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
    for (size_t i=0; i<commonPluginInfo.size(); i++)
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
    for (size_t i=0; i<loadedhandles.size(); i++)
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

// ****************************************************************************
//  Method:  DatabasePluginManager::PluginFilePatterns
//
//  Purpose:
//    Returns file name patterns for a plugin 
//
//  Arguments:
//    id         the plugin id
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 29, 2009
//
// ****************************************************************************
vector<string>
DatabasePluginManager::PluginFilePatterns(const string &id)
{
    vector<string> retval;
    if(allindexmap.find(id) != allindexmap.end())
    {
        int index = allindexmap[id];
        if(index < names.size())
            retval = filePatterns[index];
    }

    return retval;
}

// ****************************************************************************
//  Method:  DatabasePluginManager::PluginFilePatternsAreStrict
//
//  Purpose:
//    Returns true for a plugin filename patterns are intended
//    to be interpreted strictly.
//
//  Arguments:
//    id         the plugin id
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 29, 2009
//
// ****************************************************************************
bool
DatabasePluginManager::PluginFilePatternsAreStrict(const string &id)
{
    bool retval = false;
    if(allindexmap.find(id) != allindexmap.end())
    {
        int index = allindexmap[id];
        if(index < names.size())
            retval = filePatternsAreStrict[index];
    }

    return retval;
}

// ****************************************************************************
//  Method:  DatabasePluginManager::PluginOpensWholeDirectory
//
//  Purpose:
//    Returns true if a plugin is intended to be handed a whole
//    directory to open, not just a single file.
//
//  Arguments:
//    id         the plugin id
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 29, 2009
//
// ****************************************************************************
bool
DatabasePluginManager::PluginOpensWholeDirectory(const string &id)
{
    bool retval = false;
    if(allindexmap.find(id) != allindexmap.end())
    {
        int index = allindexmap[id];
        if(index < names.size())
            retval = opensWholeDirectory[index];
    }

    return retval;
}

// ****************************************************************************
//  Method:  DatabasePluginManager::GetMatchingPluginId
//
//  Purpose: Returns database plugin id matching extensions with given filename
//
//  Arguments:
//    id         the filename to match with 
//
//  Programmer:  Mark C. Miller 
//  Creation:    July 31, 2007 
//
//  Modifications:
//    Mark C. Miller, Thu Aug  9 09:16:01 PDT 2007
//    Made it return a vector of strings
//
//    Kathleen Bonnell, Tue Sep 11 08:56:42 PDT 2007 
//    Replace 'and' with '&&' for compilation on windows. 
// 
//    Jeremy Meredith, Tue Dec 29 11:42:47 EST 2009
//    Replaced simple extension and full-filename matching with single
//    comprehensive file pattern match.  Broke compile compatibility with old
//    plugins as part of this change, so removed code to check for deprecated
//    *info generated code.
//
// ****************************************************************************
vector<string>
DatabasePluginManager::GetMatchingPluginIds(const char *filename, bool searchAll)
{
    int i;
    vector<string> rv;

    //
    // Parse out the path and get just the filename.
    //
    string file_and_path = filename; 
    const char *fap = file_and_path.c_str();
    int len = strlen(fap);
    int lastSlash = -1;
    for (i = len-1 ; i >= 0 ; i--)
    {
        if (fap[i] == VISIT_SLASH_CHAR)
        {
            lastSlash = i;
            break;
        }
    }
    int start = lastSlash+1;
    string file(fap + start);

    //
    // Try each database type looking for a match to the given extensions
    //
    int iMax = searchAll ? GetNAllPlugins() : GetNEnabledPlugins();
    for (i=0; i<iMax; i++)
    {
        string id = searchAll ? GetAllID(i) : GetEnabledID(i);
        vector<string> patterns = PluginFilePatterns(id);
        int nPatterns = patterns.size();
        bool foundMatch = false;
        for (int j=0; j<nPatterns && !foundMatch; j++)
        {
            foundMatch |= WildcardStringMatch(patterns[j], file);
        }

        if (foundMatch)
            rv.push_back(id);
    }

    return rv;
}

// ****************************************************************************
// Method:  DatabasePluginManager::ReportWarning
//
// Purpose:
//   Mechanism by which we can report warnings about non-fatal incidents
//   that occurred while opening a file.
//
// Arguments:
//   s          the warning to report
//
// Programmer:  Jeremy Meredith
// Creation:    December 29, 2009
//
// ****************************************************************************
void
DatabasePluginManager::ReportWarning(const std::string &w)
{
    pluginInitErrors += w;
}
