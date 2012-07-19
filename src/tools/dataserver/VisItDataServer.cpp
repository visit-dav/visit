/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
#include <VisItDataServer.h>

#include <EngineProxy.h>
#include <MDServerProxy.h>
#include <VisItException.h>

#include <Expression.h>
#include <ExpressionList.h>

#include <avtColorTables.h>
#include <avtDataObjectReader.h>
#include <avtDataset.h>
#include <avtSILRestriction.h>

// Includes for plot and operator plugins
#include <VisItInit.h>
#include <PlotPluginManager.h>
#include <OperatorPluginManager.h>
#include <PlotPluginInfo.h>
#include <OperatorPluginInfo.h>

#include <vtkDataSet.h>

#include <snprintf.h>

// ****************************************************************************
// Class: avtDataset2VTK
//
// Purpose:
//   Provides a quick way to convert an avtDataset to an array of vtkDataSet
//   objects.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed May 23 15:41:53 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

class avtDataset2VTK : public avtDataset
{
public:
    avtDataset2VTK(avtDataset_p ds) : avtDataset(ds)
    {
    }

    vtkDataSet **GetAllDatasets(int &n)
    {
        return dataTree->GetAllLeaves(n);
    }
};

// ****************************************************************************
// Class: VisItDataServerPrivate
//
// Purpose:
//   This is a private class that bundles the mdserver and engine into a set
//   of simple services that we can easily use.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed May 23 17:01:42 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

class VisItDataServerPrivate
{
public:
    VisItDataServerPrivate();
    virtual ~VisItDataServerPrivate();

    void SetPluginDir(const std::string &pluginDir);
    void AddArgument(const std::string &arg);
    void DebugEngine();

    void Open(int argc, char *argv[]);
    void Close();

    void DefineScalarExpression(const std::string &name, const std::string &def);
    void DefineVectorExpression(const std::string &name, const std::string &def);

    std::string                ExpandPath(const std::string &path);
    const avtDatabaseMetaData *GetMetaData(const std::string &filename);

    void OpenDatabase(const std::string &filename, int timeState);
    void ReadData(const std::string &var);

    AttributeSubject *CreateOperatorAttributes(const std::string id);
    void AddOperator(const std::string &id);
    void AddOperator(const std::string &id, const AttributeSubject *atts);

    AttributeSubject *CreatePlotAttributes(const std::string id);
    void AddPlot(const std::string &id);
    void AddPlot(const std::string &id, const AttributeSubject *atts);

    vtkDataSet **Execute(int &n);

protected:
    void        LoadPluginsNow(PluginManager &plugins);
    std::string NewPlotName();

    // Important objects
    MDServerProxy         mdserver;
    EngineProxy           engine;
    PlotPluginManager     plotPlugins;
    OperatorPluginManager operatorPlugins;
    ExpressionList        exprList;

    std::string           pluginDir;

    std::string           openFile;
    std::string           openFileFormat;
    int                   openTimeState;
    bool                  createdVisWin;
    int                   windowID;
    int                   plotCounter;
};

VisItDataServerPrivate::VisItDataServerPrivate() : mdserver(), engine(), 
    plotPlugins(), operatorPlugins(), exprList(), pluginDir(), openFile(),
    openFileFormat(), openTimeState(0), createdVisWin(false),
    windowID(0), plotCounter(0)
{
}

VisItDataServerPrivate::~VisItDataServerPrivate()
{
    Close();
}

void
VisItDataServerPrivate::SetPluginDir(const std::string &dir)
{
    pluginDir = dir;
}

void
VisItDataServerPrivate::AddArgument(const std::string &arg)
{
    mdserver.AddArgument(arg);
    engine.AddArgument(arg);
}

void
VisItDataServerPrivate::DebugEngine()
{
    engine.AddArgument("-gdb-engine");
    engine.AddArgument("-xterm");
}

void
VisItDataServerPrivate::Open(int argc, char *argv[])
{
    // Arguments to create proxies on localhost.
    std::string                             hostName("localhost");
    MachineProfile::ClientHostDetermination chd = MachineProfile::MachineName;
    std::string                             clientHostName("localhost");
    bool                                    manualSSHPort = false;
    int                                     sshPort = 22;
    bool                                    useTunneling = false;
    bool                                    useGateway = false;
    std::string                             gatewayHost;

    // Initialize
    VisItInit::SetComponentName("VisItDataServerPrivate");
    VisItInit::Initialize(argc, argv);

    //
    // Initialize the plugin managers and load the plugins.
    //
    plotPlugins.Initialize(PlotPluginManager::Scripting, false, pluginDir.c_str());
    operatorPlugins.Initialize(OperatorPluginManager::Scripting, false, pluginDir.c_str());
    LoadPluginsNow(plotPlugins);
    LoadPluginsNow(operatorPlugins);

    //
    // Create the mdserver
    //
    mdserver.Create(hostName,
                  chd,
                  clientHostName,
                  manualSSHPort,
                  sshPort,
                  useTunneling,
                  useGateway,
                  gatewayHost);
    mdserver.GetMDServerMethods()->LoadPlugins();

    //
    // Create the engine
    //
    engine.Create(hostName,
                  chd,
                  clientHostName,
                  manualSSHPort,
                  sshPort,
                  useTunneling,
                  useGateway,
                  gatewayHost);
}

void
VisItDataServerPrivate::Close()
{
    mdserver.Close();
    engine.Close();
}

std::string
VisItDataServerPrivate::ExpandPath(const std::string &path)
{
    return mdserver.GetMDServerMethods()->ExpandPath(path);
}

const avtDatabaseMetaData *
VisItDataServerPrivate::GetMetaData(const std::string &filename)
{
    return mdserver.GetMDServerMethods()->GetMetaData(filename);
}

void
VisItDataServerPrivate::OpenDatabase(const std::string &filename, int timeState)
{
    // Determine the file and file format.
    if(openFile.empty())
    {
        // Expand the filename.
        std::string expandedFile = ExpandPath(filename);

        // Determine the file format.
        const avtDatabaseMetaData *md = mdserver.GetMDServerMethods()->GetMetaData(expandedFile);
        if(md == NULL)
        {
            EXCEPTION1(VisItException, "Can't get the metadata.");
        }
        openFile = expandedFile;
        openFileFormat = md->GetFileFormat();
    }
    openTimeState = timeState;

    // Set some default arguments.
    bool createMeshQualityExpressions = false;
    bool createTimeDerivativeExpressions = false;
    bool ignoreExtents = true;

    //
    // Open the database on the engine.
    //
    engine.GetEngineMethods()->OpenDatabase(openFileFormat,
                        openFile,
                        openTimeState,
                        createMeshQualityExpressions,
                        createTimeDerivativeExpressions,
                        ignoreExtents);
}

void
VisItDataServerPrivate::DefineScalarExpression(const std::string &name, const std::string &def)
{
    Expression expr;
    expr.SetName(name);
    expr.SetDefinition(def);
    expr.SetType(Expression::ScalarMeshVar);
    exprList.AddExpressions(expr);
}

void
VisItDataServerPrivate::DefineVectorExpression(const std::string &name, const std::string &def)
{
    Expression expr;
    expr.SetName(name);
    expr.SetDefinition(def);
    expr.SetType(Expression::VectorMeshVar);
    exprList.AddExpressions(expr);
}

void
VisItDataServerPrivate::ReadData(const std::string &var)
{
    // Set some defaults.
    MaterialAttributes       mAtts;
    ExpressionList           combinedExprList(exprList);
    MeshManagementAttributes meshAtts;
    bool                     treatAllDbsAsTimeVarying = false;
    bool                     ignoreExtents = true;
    std::string              selectionName;
    int                      windowID = 0;

    //
    // Tickle the engine to create a vis window. Works around a bug in engine!
    //
    if(!createdVisWin)
    {
        createdVisWin = true;

        // Set some defaults.
        WindowAttributes     windowAtts;
        windowAtts.SetColorTables(*avtColorTables::Instance()->GetColorTables());
        AnnotationAttributes annotationAtts;
        AnnotationObjectList annotationObjectList;
        std::string          extStr("?");
        VisualCueList        visCues;
        int                  frameAndState[] = {0,0,0,0,0,0,0};
        double               viewExtents[] = {0., 1., 0., 1., 0., 1.};
        std::string          ctName("hot");

        engine.GetEngineMethods()->SetWinAnnotAtts(&windowAtts,
                               &annotationAtts,
                               &annotationObjectList,
                               extStr,
                               &visCues,
                               frameAndState,
                               viewExtents,
                               ctName,
                               windowID);
    }

    //
    // Determine the name of the mesh for this variable.
    //
    const avtDatabaseMetaData *md = GetMetaData(openFile);
    if(md == NULL)
    {
        EXCEPTION1(VisItException, "Can't get the metadata.");
    }
    std::string mesh = md->MeshForVar(var);
    // Add the metadata expressions to the known expressions.
    for(int i = 0; i < md->GetNumberOfExpressions(); ++i)
        combinedExprList.AddExpressions(*md->GetExpression(i));

    //
    // Get the SIL so we can make a SIL restriction.
    //
    const SILAttributes *silAtts = mdserver.GetMDServerMethods()->GetSIL(openFile, openTimeState,
                                                   treatAllDbsAsTimeVarying);
    if(silAtts == NULL)
    {
        EXCEPTION1(VisItException, "Can't get the SIL.");
    }
    avtSIL sil(*silAtts);
    avtSILRestriction_p silr = new avtSILRestriction(&sil);
    silr->SetTopSet(mesh.c_str());

    //
    // Start a new network
    //
    engine.GetEngineMethods()->ReadDataObject(openFileFormat,
                          openFile,
                          var,
                          openTimeState,
                          silr,
                          mAtts,  // lots of junk!
                          combinedExprList,
                          meshAtts,
                          treatAllDbsAsTimeVarying,
                          ignoreExtents,
                          selectionName,
                          windowID);
}

AttributeSubject *
VisItDataServerPrivate::CreateOperatorAttributes(const std::string id)
{
    AttributeSubject *atts = NULL;
    ScriptingOperatorPluginInfo *info = operatorPlugins.GetScriptingPluginInfo(id);
    if(info != 0)
        atts = info->AllocAttributes();
    return atts;
}


void
VisItDataServerPrivate::AddOperator(const std::string &id)
{
    AttributeSubject *atts = CreateOperatorAttributes(id);
    engine.GetEngineMethods()->ApplyOperator(id, atts);
    delete atts;
}

void
VisItDataServerPrivate::AddOperator(const std::string &id, const AttributeSubject *atts)
{
    engine.GetEngineMethods()->ApplyOperator(id, atts);
}

AttributeSubject *
VisItDataServerPrivate::CreatePlotAttributes(const std::string id)
{
    AttributeSubject *atts = NULL;
    ScriptingPlotPluginInfo *info = plotPlugins.GetScriptingPluginInfo(id);
    if(info != 0)
        atts = info->AllocAttributes();
    return atts;
}

std::string
VisItDataServerPrivate::NewPlotName()
{
    char name[20];
    SNPRINTF(name, 20, "Plot%06d", plotCounter++);
    return std::string(name);
}

void
VisItDataServerPrivate::AddPlot(const std::string &id)
{
    std::string plotName(NewPlotName());
    std::vector<double> extents;
    AttributeSubject *atts = CreatePlotAttributes(id);
    engine.GetEngineMethods()->MakePlot(NewPlotName(),
                    id,
                    atts,
                    extents,
                    windowID);
    delete atts;
}

void
VisItDataServerPrivate::AddPlot(const std::string &id, const AttributeSubject *atts)
{
    std::string plotName(NewPlotName());
    std::vector<double> extents;
    engine.GetEngineMethods()->MakePlot(NewPlotName(),
                    id,
                    atts,
                    extents,
                    windowID);
}

vtkDataSet **
VisItDataServerPrivate::Execute(int &n)
{
    //
    // Execute the plot on the engine.
    //
    avtDataObjectReader_p reader = engine.GetEngineMethods()->Execute(false, NULL, NULL);
    avtDataObject_p dob = reader->GetOutput();

    //
    // Return the data as VTK datasets.
    //
    avtDataset_p ds;
    CopyTo(ds, dob);
    avtDataset2VTK ds2vtk(ds);
    vtkDataSet **vtk = ds2vtk.GetAllDatasets(n);

    // Increase refcount on datasets
    for(int i = 0; i < n; ++i)
        vtk[i]->Register(NULL);

    return vtk;
}

// ****************************************************************************
// Method: LoadPluginsNow
//
// Purpose: 
//   Tells the plugin manager to load all plugins now.
//
// Arguments:
//   plugins : The plugin manager.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 23 15:45:01 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

void
VisItDataServerPrivate::LoadPluginsNow(PluginManager &plugins)
{
    // Enable all plugins
    for(int i = 0; i < plugins.GetNAllPlugins(); ++i)
    {
        // Get a pointer to the GUI portion of the Operator plugin information.
        plugins.EnablePlugin(plugins.GetAllID(i));
    }

    // Load the plugins
    plugins.LoadPluginsNow();
}

////////////////////////////////////////////////////////////////////////////////
///                            VisItDataServer
////////////////////////////////////////////////////////////////////////////////

VisItDataServer::VisItDataServer()
{
    d = new VisItDataServerPrivate;
}

VisItDataServer::~VisItDataServer()
{
    delete d;
}

void
VisItDataServer::SetPluginDir(const std::string &pluginDir)
{
    d->SetPluginDir(pluginDir);
}

void
VisItDataServer::AddArgument(const std::string &arg)
{
    d->AddArgument(arg);
}

void
VisItDataServer::DebugEngine()
{
    d->DebugEngine();
}

void
VisItDataServer::Open(int argc, char *argv[])
{
    d->Open(argc, argv);
}

void
VisItDataServer::Close()
{
    d->Close();
}

std::string
VisItDataServer::ExpandPath(const std::string &path)
{
    return d->ExpandPath(path);
}

const avtDatabaseMetaData *
VisItDataServer::GetMetaData(const std::string &filename)
{
    return d->GetMetaData(filename);
}

void
VisItDataServer::OpenDatabase(const std::string &filename, int timeState)
{
    d->OpenDatabase(filename, timeState);
}

void
VisItDataServer::ReadData(const std::string &var)
{
    d->ReadData(var);
}

void
VisItDataServer::DefineScalarExpression(const std::string &name, const std::string &def)
{
    d->DefineScalarExpression(name, def);
}

void
VisItDataServer::DefineVectorExpression(const std::string &name, const std::string &def)
{
    d->DefineVectorExpression(name, def);
}

AttributeSubject *
VisItDataServer::CreateOperatorAttributes(const std::string id)
{
    return d->CreateOperatorAttributes(id);
}

void             
VisItDataServer::AddOperator(const std::string &id)
{
    d->AddOperator(id);
}

void             
VisItDataServer::AddOperator(const std::string &id, const AttributeSubject *atts)
{
    d->AddOperator(id, atts);
}


AttributeSubject *
VisItDataServer::CreatePlotAttributes(const std::string id)
{
    return d->CreatePlotAttributes(id);
}

void             
VisItDataServer::AddPlot(const std::string &id)
{
    d->AddPlot(id);
}

void             
VisItDataServer::AddPlot(const std::string &id, const AttributeSubject *atts)
{
    d->AddPlot(id, atts);
}

vtkDataSet **
VisItDataServer::Execute(int &n)
{
    return d->Execute(n);
}
