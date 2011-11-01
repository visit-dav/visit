#include <iostream>
#include <ViewerProxy.h>
#include <ViewerSubject.h>
#include <ViewerSubjectProxy.h>
#include <VisItInit.h>
#include <RuntimeSetting.h>


#include <ViewerState.h>
#include <ViewerMethods.h>
#include <PlotPluginManager.h>
#include <OperatorPluginManager.h>
#include <PluginManagerAttributes.h>
#include <PlotPluginInfo.h>
#include <OperatorPluginInfo.h>


#include <ClientInformation.h>
#include <ClientMethod.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <ParentProcess.h>
#include <PlotInfoAttributes.h>
#include <PluginManagerAttributes.h>
#include <RemoteProcess.h>
#include <SILRestrictionAttributes.h>
#include <SocketConnection.h>
#include <ViewerMethods.h>
#include <ViewerRPC.h>
#include <ViewerState.h>
#include <VisItException.h>
#include <Xfer.h>


#undef Always
#include <QtVisWindow.h>


std::map<int,vtkQtRenderWindow*> ViewerSubjectProxy::viswindows;


void ViewerSubjectProxy::InitializePlugins(PluginManager::PluginCategory t, const char *pluginDir)
{

    //std::cout << "initializing plugins.." << " " << std::endl;
    //t = PluginManager::GUI | PluginManager::Scripting;


//    if(t == PluginManager::GUI)
//    {
//        std::cout << "now loading plugins" << std::endl;
//        std::string dir = "/work/visit/branches/PySideIntegration/build/plugins/";
//        cli_plotplugin->Initialize(t, false, dir.c_str());
//        cli_operatorplugin->Initialize(t, false, dir.c_str());
//        LoadPlugins();
//    }
//    else
//    {
        cli_plotplugin->Initialize(t, false, pluginDir);
        cli_operatorplugin->Initialize(t, false, pluginDir);
//    }
}

void ViewerSubjectProxy::LoadPlugins()
{
    //std::cout << "Loading plugins" << std::endl;

    //ViewerState* state = vissubject->GetViewerState();
    ViewerState* state = gstate;
    PlotPluginManager* plotPlugins = cli_plotplugin;
    OperatorPluginManager* operatorPlugins = cli_operatorplugin;

    int i;
    int nPlots = state->GetNumPlotStateObjects();
    int nOperators = state->GetNumOperatorStateObjects();

    if (nPlots > 0 || nOperators > 0)
    {
        //std::cout << "path has stuff" << " " << nPlots << " " << nOperators << std::endl;
        //return;
    }

    if(plotPlugins == 0 || operatorPlugins == 0)
    {
        EXCEPTION1(ImproperUseException, "ViewerProxy::InitializePlugins "
            "must be called before ViewerProxy::LoadPlugins")
    }

    //
    // Go through the plugin atts and en/disable the ones specified
    // by the plugin attributes
    //
    PluginManagerAttributes *pluginManagerAttributes = state->GetPluginManagerAttributes();
    for (i=0; i<pluginManagerAttributes->GetId().size(); i++)
    {
        if (! pluginManagerAttributes->GetEnabled()[i]) // not enabled
        {
            std::string id = pluginManagerAttributes->GetId()[i];
            if (pluginManagerAttributes->GetType()[i] == "plot")
            {
                if (plotPlugins->PluginExists(id))
                    plotPlugins->DisablePlugin(id);
            }
            else if (pluginManagerAttributes->GetType()[i] == "operator")
            {
                if (operatorPlugins->PluginExists(id))
                    operatorPlugins->DisablePlugin(id);
            }
        }
        else // is enabled -- it had better be available
        {
            std::string id = pluginManagerAttributes->GetId()[i];
            if (pluginManagerAttributes->GetType()[i] == "plot")
            {
                if (plotPlugins->PluginExists(id))
                    plotPlugins->EnablePlugin(id);
                else
                {
                    std::string msg(std::string("The ") + id +
                        std::string(" plot plugin enabled by the viewer "
                            "was not available in the client."));
                    EXCEPTION1(VisItException, msg)
                }
            }
            else if (pluginManagerAttributes->GetType()[i] == "operator")
            {
                if (operatorPlugins->PluginExists(id))
                    operatorPlugins->EnablePlugin(id);
                else
                {
                    std::string msg(std::string("The ") + id +
                        std::string(" operator plugin enabled by the viewer "
                            "was not available in the client."));
                    EXCEPTION1(VisItException, msg)
                }
            }
        }
    }

    //
    // Now load dynamic libraries
    //
    plotPlugins->LoadPluginsNow();
    operatorPlugins->LoadPluginsNow();

    //
    // Initialize the plot attribute state objects.
    //
    nPlots = plotPlugins->GetNEnabledPlugins();
    for (i = 0; i < nPlots; ++i)
    {
        CommonPlotPluginInfo *info =
            plotPlugins->GetCommonPluginInfo(plotPlugins->GetEnabledID(i));
        AttributeSubject *obj = info->AllocAttributes();
        //std::cout << obj->TypeName() << std::endl;
        state->RegisterPlotAttributes(obj);
    }

    //
    // Initialize the operator attribute state objects.
    //
    nOperators = operatorPlugins->GetNEnabledPlugins();
    for (i = 0; i < nOperators; ++i)
    {
        CommonOperatorPluginInfo *info =
            operatorPlugins->GetCommonPluginInfo(operatorPlugins->GetEnabledID(i));
        AttributeSubject *obj = info->AllocAttributes();
        //std::cout << obj->TypeName() << std::endl;
        state->RegisterOperatorAttributes(obj);
    }

    //std::cout << "ending" << std::endl;
}

//return the new cli plot and operator manager so that the visit cli works like normal..
PlotPluginManager* ViewerSubjectProxy::GetPlotPluginManager() const
{
    //std::cout << "Getting plot plugins" << std::endl;
    //return vissubject->GetPlotPluginManager();
    return cli_plotplugin;
}

OperatorPluginManager *ViewerSubjectProxy::GetOperatorPluginManager() const
{
    //std::cout << "operator plugins" << std::endl;
    //return vissubject->GetOperatorPluginManager();
    return cli_operatorplugin;
}

ViewerState   *ViewerSubjectProxy::GetViewerState() const
{
   //std::cout << "Getting viewer State" << " " << gstate << std::endl;
   //return vissubject->GetViewerState();
   return gstate;
}

ViewerMethods *ViewerSubjectProxy::GetViewerMethods() const
{
    //std::cout << "Getting viewer Methods" << std::endl;
    //return vissubject->GetViewerMethods();
    return gmethods;
}

void ViewerSubjectProxy::Initialize(int argc,char* argv[])
{
    //std::cout << "initializing" << std::endl;
    //do not initialize if gstate or gmethods have already been set..
//    if(gstate || gmethods)
//    {
//        std::cout << "Already Initialized" << std::endl;
//        return;
//    }
    vissubject = new ViewerSubject();
    testconn = new TestConnection();
    cli_plotplugin = new PlotPluginManager();
    cli_operatorplugin = new OperatorPluginManager();

    gstate = vissubject->GetViewerState();
    gmethods = vissubject->GetViewerMethods();

    //std::cout << "here" << std::endl;
    //std::string hostname = "localhost";
    //stringVector args;
    //GetViewerMethods()->ConnectToMetaDataServer(hostname,args);
    //ViewerFileServer::Instance()->ConnectServer(hostname,args);
    //std::cout << "and now here" << std::endl;

    try
    {
        vissubject->GetViewerProperties()->SetUseWindowMetrics(false);
        QtVisWindow::SetWindowCreationCallback(renderWindow, this);
        //let VisIt own all actions as if it was its
        QtVisWindow::SetOwnerShipOfAllWindows(true);
        //std::cout << "initializing" << std::endl;
        vissubject->ProcessCommandLine(argc,argv);
        vissubject->Initialize();
    }
    catch(VisItException e)
    {
        std::cout << e.Message() << std::endl;
    }
    //std::cout << "blah" << std::endl;
}


vtkQtRenderWindow* ViewerSubjectProxy::GetRenderWindow(int i)
{
    //VisIt has not initialized any windows..
    if(viswindows.size() == 0)
        return NULL;

    if(viswindows.find(i) != viswindows.end())
        return viswindows[i];

    std::cerr << "Window " << i << " does not exist" << std::endl;
    return NULL;
}

vtkQtRenderWindow* ViewerSubjectProxy::renderWindow(void* data)
{
    vtkQtRenderWindow* win = new ViewerSubjectProxy::NonClosableQtRenderWindow();

    //add signal to let us know when windowDeleted.
    win->connect(win,SIGNAL(destroyed(QObject*)),(QObject*)data,SLOT(windowDeleted(QObject*)));

    //find next free id (okay to do this loop since VisIt
    //has very limited opengl windows it can support

    int id = 1;
    while(viswindows.count(id) > 0) id++;

    viswindows[id] = win;

    return viswindows[id];
}

void ViewerSubjectProxy::windowDeleted(QObject * obj)
{
    std::map<int,vtkQtRenderWindow*>::iterator itr = viswindows.begin();

    for(itr = viswindows.begin(); itr != viswindows.end(); ++itr)
    {
        if(itr->second == obj)
        {
            std::cerr << "Deleting window: " << itr->first
                    << ". References to Rendering Window are now invalid. "
                    << std::endl;
            viswindows.erase(itr);
        }
    }
}

QList<int> ViewerSubjectProxy::GetRenderWindowIDs()
{

    QList<int> windowIDs;

    std::map<int,vtkQtRenderWindow*>::iterator itr = viswindows.begin();

    for(itr = viswindows.begin(); itr != viswindows.end(); ++itr)
    {
        windowIDs.append(itr->first);
    }

    return windowIDs;
}

// Constructor
ViewerSubjectProxy::ViewerSubjectProxy():ViewerProxy()
{
    initialize = true;
    //setting to ViewerBase works because they are static
    gstate = ViewerBase::GetViewerState();
    gmethods = ViewerBase::GetViewerMethods();
    vissubject = 0;
    cli_plotplugin = 0;
    cli_operatorplugin = 0;
    ViewerProxy::CreateViewerProxy(this);
}

// This constructor emulates as if it has already been started..
ViewerSubjectProxy::ViewerSubjectProxy(ViewerState* istate, ViewerMethods* imethods):ViewerProxy()
{
    initialize = false;
    vissubject = 0;
    this->gstate = istate;
    this->gmethods = imethods;

    testconn = new TestConnection();
    cli_plotplugin = new PlotPluginManager();
    cli_operatorplugin = new OperatorPluginManager();
    ViewerProxy::CreateViewerProxy(this);
}

// Destructor (free allocated memory)
ViewerSubjectProxy::~ViewerSubjectProxy()
{
    delete testconn;
    delete cli_plotplugin;
    delete cli_operatorplugin;
}
