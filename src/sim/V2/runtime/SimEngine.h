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
#ifndef SIM_ENGINE_H
#define SIM_ENGINE_H
#include <Engine.h>
#include <vectortypes.h>

#ifdef SIMV2_VIEWER_INTEGRATION
#include <ViewerBase.h>
#endif

class Subject;
class ObserverToCallback;

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

    virtual void SimulationInitiateCommand(const std::string &command);

    void InitializeViewer();

    bool  OpenDatabase();
    const avtDatabaseMetaData *GetMetaData(const std::string &filename);
    const avtDatabaseMetaData *GetMetaDataForState(const std::string &filename, int timeState);
    const avtSIL *GetSIL(const std::string &filename);
    const avtSIL *GetSILForState(const std::string &filename, int timeState);

    // Viewer-ey methods.
    bool ExportDatabase(const std::string &filename, const std::string &format,
                        const stringVector &vars);

    bool RestoreSession(const std::string &filename);
    bool SaveWindow(const std::string &filename, int w, int h, int format);
    bool AddPlot(const std::string &plotType, const std::string &var);
    bool AddOperator(const std::string &operatorType, bool applyToAll);
    bool DrawPlots();
    bool DeleteActivePlots();
    bool SetActivePlots(const int *ids, int nids);
    bool SetPlotOptions(const std::string &fieldName, 
                        int fieldType, void *fieldVal, int fieldLen);
    bool SetOperatorOptions(const std::string &fieldName, 
                            int fieldType, void *fieldVal, int fieldLen);
private:
    virtual void CreatePluginManagers();

#ifdef SIMV2_VIEWER_INTEGRATION
    static void CommandNotificationCallback(void *cbdata, int timeout);
    static void HandleViewerRPCCallback(Subject *, void *cbdata);

    void HeavyInitialization();
    void LoadPlotPlugins();
    void LoadOperatorPlugins();
    void AddInitialWindows();
#endif

private:
    bool                          viewerInitialized;
    std::string                   simsource;
    ObserverToCallback           *rpcNotifier;
};

#endif
