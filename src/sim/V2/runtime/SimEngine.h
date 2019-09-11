// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SIM_ENGINE_H
#define SIM_ENGINE_H
#include <Engine.h>
#include <vectortypes.h>

#ifdef SIMV2_VIEWER_INTEGRATION
#include <ViewerBase.h>
#endif

class Subject;
class ObserverToCallback;
class DBOptionsAttributes;
class PluginManager;

class VisWindow;
class View2DAttributes;
class View3DAttributes;

// ****************************************************************************
// Class: SimEngine
//
// Purpose:
//   A simulation-based engine class.
//
// Notes:    This class can optionally use the viewercore library to do a 
//           better job of state management for batch in situ.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 18:29:22 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class SimEngine : public Engine
#ifdef SIMV2_VIEWER_INTEGRATION
                               , public ViewerBase
#endif
{
public:
    SimEngine();
    virtual ~SimEngine();

    virtual void SimulationTimeStepChanged();
    virtual void SimulationInitiateCommand(const std::string &command);

    void InitializeViewer(const std::vector<std::string> &plotPlugins,
                          const std::vector<std::string> &operatorPlugins,
                          bool noconfig);

    bool  OpenDatabase();
    const avtDatabaseMetaData *GetMetaData(const std::string &filename);
    const avtDatabaseMetaData *GetMetaDataForState(const std::string &filename, int timeState);
    const avtSIL *GetSIL(const std::string &filename);
    const avtSIL *GetSILForState(const std::string &filename, int timeState);

    // Viewer-ey methods.
    bool ExportDatabase(const std::string &filename, const std::string &format,
                        const stringVector &vars,
                        const DBOptionsAttributes &);

    bool RestoreSession(const std::string &filename);
    bool SaveWindow(const std::string &filename, int w, int h, int format);
    bool AddPlot(const std::string &plotType, const std::string &var);
    bool AddOperator(const std::string &operatorType, bool applyToAll);
    bool DrawPlots();
    bool DeleteActivePlots();
    bool SetActivePlots(const int *ids, int nids);
    bool ChangePlotVar(const char *var, int all);
    bool SetPlotOptions(const std::string &fieldName, 
                        int fieldType, void *fieldVal, int fieldLen);
    bool SetOperatorOptions(const std::string &fieldName, 
                            int fieldType, void *fieldVal, int fieldLen);
    bool SetView2D(const View2DAttributes &);
    bool GetView2D(View2DAttributes &);
    bool SetView3D(const View3DAttributes &);
    bool GetView3D(View3DAttributes &);
private:
    virtual void CreatePluginManagers();

#ifdef SIMV2_VIEWER_INTEGRATION
    static void CommandNotificationCallback(void *cbdata, int timeout);
    static void HandleViewerRPCCallback(Subject *, void *cbdata);

    void HeavyInitialization(const std::vector<std::string> &plotPlugins,
                             const std::vector<std::string> &operatorPlugins);
    void LoadPlotPlugins(const std::vector<std::string> &idsOrNames);
    void LoadOperatorPlugins(const std::vector<std::string> &idsOrNames);
    void AddInitialWindows();
    void RestrictPlugins(PluginManager *mgr,
                         const std::vector<std::string> &idsOrNames);

    void CreateVisWindow(int winID, VisWindow *&viswindow, bool &owns);
    static void CreateVisWindowCB(int winID, VisWindow *&viswindow, bool &owns, void*);

    bool allowCommandExecution;
#endif

private:
    bool                          viewerInitialized;
    std::string                   simsource;
    ObserverToCallback           *rpcNotifier;
};

#endif
