// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    David Camp, Thu Aug 27 09:40:00 PDT 2015
//    Added hostname to the SaveSession and RestoreSession functions, to add
//    the remote save and load functionality.
//
//    Brad Whitlock, Tue Jul 17 17:08:30 PDT 2018
//    Add override for RestoreSession.
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
    void SaveSession(const std::string &hostname, const std::string &filename);
    void RestoreSession(const std::string &hostname, const std::string &filename,
                        bool inVisItDir, const stringVector &sources);
    void RestoreSession(const std::string &filename,
                        const std::string &buffer, 
                        const stringVector &sources);

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
