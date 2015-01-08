/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#ifndef VIEWER_STATE_MANAGER_H
#define VIEWER_STATE_MANAGER_H
#include <viewercore_exports.h>
#include <ViewerBase.h>

#include <string>
#include <map>
#include <vectortypes.h>

class DataNode;
class HostProfileList;
class ViewerConfigManager;
class ViewerDatabaseCorrelationMethods;
class ViewerVariableMethods;

#if 1
class MaterialAttributes;
class MeshManagementAttributes;
#endif

// ****************************************************************************
// Class: ViewerStateManager
//
// Purpose:
//   This class manages config file state for the viewer.
//
// Notes:    config files, sessions, host profiles.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 22:59:16 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API ViewerStateManager : public ViewerBase
{
public:
    ViewerStateManager();
    virtual ~ViewerStateManager();

    // Making sure that the central ViewerState object is set up right.
    void CreateState();

    // Connecting state objects to the config manager.
    void ConnectDefaultState();
    void ConnectPluginDefaultState();

    // For processing config file settings.
    bool ProcessSettings();
    void ProcessLocalSettings();

    // For default settings.
    void ReadConfigFile(bool specifiedConfig);
    void WriteConfigFile();

    // For sessions.
    void SaveSession(const std::string &filename);
    void RestoreSession(const std::string &filename, bool inVisItDir, const stringVector &sources);

    // For user-defined host profiles
    void ReadHostProfiles();
    void WriteHostProfiles();
    void ReadHostProfilesFromDirectory(const std::string &, bool);

    // Helper methods
    ViewerDatabaseCorrelationMethods *GetDatabaseCorrelationMethods();
    ViewerVariableMethods *GetVariableMethods();

#if 1
    // Materials
    static void SetClientMaterialAttsFromDefault();
    static void SetDefaultMaterialAttsFromClient();
    static MaterialAttributes *GetMaterialDefaultAtts();

    // Mesh management
    static MeshManagementAttributes *GetMeshManagementDefaultAtts();
    static void SetClientMeshManagementAttsFromDefault();
    static void SetDefaultMeshManagementAttsFromClient();
#endif
private:
    static void WriteCallback(DataNode *, bool, void *);

    std::map<std::string,std::string> BuildSourceMap(DataNode *viewerNode, const stringVector &sources);
    void        CreateNode(DataNode *parentNode, bool detailed);
    bool        SetFromNode(DataNode *parentNode, 
                            const std::map<std::string,std::string> &sourceToDB, 
                            const std::string &configVersion);
    void        ReadEngineSettingsFromNode(DataNode *vsNode, 
                                           const std::string &configVersion);
private:
    ViewerConfigManager   *configMgr;
    DataNode              *systemSettings;
    DataNode              *localSettings;
    HostProfileList       *originalSystemHostProfileList;

    ViewerDatabaseCorrelationMethods *correlationMethods;
    ViewerVariableMethods            *variableMethods;

#if 1
    static MaterialAttributes       *materialDefaultAtts;
    static MeshManagementAttributes *meshManagementDefaultAtts;
#endif
};

#endif
