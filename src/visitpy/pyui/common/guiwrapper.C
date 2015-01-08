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

#include "guiwrapper.h"

#include <QApplication>
#include <QMainWindow>
#include <QList>
#include <VisItInit.h>
#include <InitVTK.h>
#include <InitVTKRendering.h>
#include <QvisMainWindow.h>
#include <SplashScreen.h>
#include <QvisGUIApplication.h>
#include <QvisPostableWindowObserver.h>
#include <QvisPlotManagerWidget.h>
#include <QvisSourceManagerWidget.h>
#include <QvisTimeSliderControlWidget.h>
#include <QEventLoop>

//-- Viewer controls..

#include <ViewerSubjectProxy.h>

#include <vtkObject.h>

QvisGUIApplicationDerived::QvisGUIApplicationDerived(int argc,char* argv[],ViewerProxy* proxy):QvisGUIApplication(argc,argv,proxy)
{
        //Ensure VisIt is ready..
        QEventLoop loop;
        connect(this,SIGNAL(VisItIsReady()),&loop,SLOT(quit()));
        loop.exec();
        GetMainWindow()->installEventFilter(this);
        mainWin->setWindowTitle("Python Viewer");

        //create a plot manager widget..
        /*
        plotManagerWindow = new QMainWindow();
        QvisPlotManagerWidget* plotManager = new QvisPlotManagerWidget(mainWin->menuBar(), plotManagerWindow);
        //plotManager->ConnectPlotList(GetViewerState()->GetPlotList());
        plotManager->ConnectFileServer(fileServer);
        plotManager->ConnectGlobalAttributes(GetViewerState()->GetGlobalAttributes());
        plotManager->ConnectExpressionList(GetViewerState()->GetExpressionList());
        plotManager->ConnectWindowInformation(GetViewerState()->GetWindowInformation());
        plotManager->ConnectSelectionList(GetViewerState()->GetSelectionList());
        plotManagerWindow->setCentralWidget(plotManager);
        */

        sourceManagerWindow = new QMainWindow();
        QvisSourceManagerWidget* sourceManager = new QvisSourceManagerWidget();
        sourceManager->ConnectPlotList(GetViewerState()->GetPlotList());
        sourceManager->ConnectFileServer(fileServer);
        sourceManager->ConnectGlobalAttributes(GetViewerState()->GetGlobalAttributes());
        sourceManager->ConnectWindowInformation(GetViewerState()->GetWindowInformation());
        connect(sourceManager,SIGNAL(activateFileOpenWindow()), mainWin,SIGNAL(activateFileOpenWindow()));
        sourceManagerWindow->setCentralWidget(sourceManager);

        timesliderWindow = new QMainWindow();
        QvisTimeSliderControlWidget* tsControl = new QvisTimeSliderControlWidget();
        tsControl->ConnectFileServer(fileServer);
        tsControl->ConnectWindowInformation(GetViewerState()->GetWindowInformation());
        connect(tsControl, SIGNAL(reopenOnNextFrame()), mainWin, SIGNAL(reopenOnNextFrame()));
        timesliderWindow->setCentralWidget(tsControl);
}

QvisGUIApplicationDerived::~QvisGUIApplicationDerived()
{
    delete sourceManagerWindow;
    delete timesliderWindow;
}

QMainWindow* 
QvisGUIApplicationDerived::GetMainWindow() 
{
    return (QMainWindow*)mainWin; 
}

QApplication* 
QvisGUIApplicationDerived::GetApp() 
{ 
    return (QApplication*)mainApp; 
}

int QvisGUIApplicationDerived::GetPlotWindowSize()
{
    return plotWindows.size();
}

QMainWindow* QvisGUIApplicationDerived::GetPlotWindow(int index)
{
    if(index >= (int)plotWindows.size() || index < 0)
        return NULL;

    ActivatePlotWindow(index);
    QMainWindow* win = ((QMainWindow*)plotWindows[index]);
    win->hide();
    win->move(QCursor::pos());

    if(win->inherits("QvisPostableWindow"))
    {
        if(((QvisPostableWindow*)win)->posted())
            ((QvisPostableWindow*)win)->unpost();
    }

    return win;
}

QMainWindow* QvisGUIApplicationDerived::GetPlotWindow(const QString& name)
{
    PlotPluginManager* mgr = GetViewerProxy()->GetPlotPluginManager();
    for(int i = 0; i < mgr->GetNEnabledPlugins(); ++i)
    {
        if(mgr->GetPluginName(mgr->GetEnabledID(i)) == name.toStdString())
            return GetPlotWindow(i);
    }
    return NULL;
}

int QvisGUIApplicationDerived::GetOperatorWindowSize() {
    return operatorWindows.size();
}

QMainWindow* QvisGUIApplicationDerived::GetOperatorWindow(int index)
{
    if(index >= (int)operatorWindows.size() || index < 0)
        return NULL;

    ActivateOperatorWindow(index);

    QMainWindow* win = (QMainWindow*)operatorWindows[index];
    win->hide();
    win->move(QCursor::pos());

    if(win->inherits("QvisPostableWindow"))
    {
        if(((QvisPostableWindow*)win)->posted())
            ((QvisPostableWindow*)win)->unpost();
    }
    return win;
}

QMainWindow* QvisGUIApplicationDerived::GetOperatorWindow(const QString& name)
{
    OperatorPluginManager* mgr = GetViewerProxy()->GetOperatorPluginManager();
    for(int i = 0; i < mgr->GetNEnabledPlugins(); ++i)
    {
         if(mgr->GetPluginName(mgr->GetEnabledID(i)) == name.toStdString())
            return GetOperatorWindow(i);
    }
    return NULL;
}

QMainWindow* QvisGUIApplicationDerived::GetOtherWindow(const QString& name)
{
    for(int i = 0; i < windowNames.size(); ++i)
    {
        if( windowNames[i] == name ) 
        {
            QvisGUIApplication::GetInitializedWindowPointer(i)->show();
            QMainWindow* win = QvisGUIApplication::GetInitializedWindowPointer(i);
            if(win->inherits("QvisPostableWindow"))
            {
                if(((QvisPostableWindow*)win)->posted())
                    ((QvisPostableWindow*)win)->unpost();
            }
            win->move(QCursor::pos());
            win->hide(); 
            return win;
        }
    }
    return NULL;
}

QStringList 
QvisGUIApplicationDerived::GetOtherWindowNames() 
{ 
    return windowNames; 
}

    //QMainWindow* GetPlotManagerWindow() { return plotManagerWindow; }
QMainWindow* 
QvisGUIApplicationDerived::GetSourceManagerWindow() 
{ 
    return sourceManagerWindow; 
}

QMainWindow* QvisGUIApplicationDerived::GetTimeSliderWindow() 
{ 
    return timesliderWindow; 
}

bool 
QvisGUIApplicationDerived::eventFilter(QObject *o, QEvent *e)
{
    if(e->type() == QEvent::Close)
    {
        if(o == GetMainWindow())
        {
            e->ignore();
            GetMainWindow()->hide();
        }
        return true;
    }
    return false;
}

GUIWrapper::GUIWrapper(int& argc,char** argv)
{
    try
    {
        VisItInit::SetComponentName("gui");
        VisItInit::Initialize(argc, argv, 0, 1, false);

        InitVTK::Initialize();
        InitVTKRendering::Initialize();

        /// this turns off all warnings..
        vtkObject::GlobalWarningDisplayOff();

        guiproxy = new ViewerSubjectProxy(argc,argv);
        gui = new QvisGUIApplicationDerived(argc, argv,guiproxy);

        cliproxy = new ViewerSubjectProxy(guiproxy);
    }
    catch(VisItException e)
    {
        std::cout << e.Message() << std::endl;
    }
    catch(...)
    {
        std::cout << "Catch all..." << std::endl;
    }
}

QMainWindow* 
GUIWrapper::GetRenderWindow(int i)
{
    return (QMainWindow*)guiproxy->GetRenderWindow(i);
}

QList<int> 
GUIWrapper::GetRenderWindowIDs()
{
    return guiproxy->GetRenderWindowIDs();
}

QMainWindow* 
GUIWrapper::GetUIWindow()
{
    return (QMainWindow*)gui->GetMainWindow();
}

QMainWindow* 
GUIWrapper::GetPlotWindow(int index)
{
    return gui->GetPlotWindow(index);
}

QMainWindow* 
GUIWrapper::GetOperatorWindow(int index)
{
    return gui->GetOperatorWindow(index);
}

QMainWindow*
GUIWrapper::GetPlotWindow(const QString& name)
{
    return gui->GetPlotWindow(name);
}

QMainWindow*
GUIWrapper::GetOperatorWindow(const QString& name)
{
    return gui->GetOperatorWindow(name);
}

QStringList  
GUIWrapper::GetOtherWindowNames()
{
    return gui->GetOtherWindowNames();
}

QMainWindow* 
GUIWrapper::GetOtherWindow(const QString& name)
{
    return gui->GetOtherWindow(name);
}

//QMainWindow* GetPlotManagerWindow() { return gui->GetPlotManagerWindow(); }

QMainWindow* 
GUIWrapper::GetSourceManagerWindow()
{
    return gui->GetSourceManagerWindow();
}

QMainWindow* 
GUIWrapper::GetTimeSliderWindow()
{
    return gui->GetTimeSliderWindow();
}

QList<QMainWindow*>
GUIWrapper::GetPlotWindows() {

    QList<QMainWindow*> plots;

    int len = gui->GetPlotWindowSize();

    for(int i = 0; i < len; ++i) {
        plots.push_back(gui->GetPlotWindow(i));
    }

    return plots;
}

QList<QMainWindow*>
GUIWrapper::GetOperatorWindows() {

    QList<QMainWindow*> operators;

    int len = gui->GetOperatorWindowSize();
    for(int i = 0; i < len; ++i) {
        operators.push_back(gui->GetOperatorWindow(i));
    }

    return operators;
}

QList<QMainWindow*>
GUIWrapper::GetOtherWindows() {

    QList<QMainWindow*> windowNames;

    QStringList names = gui->GetOtherWindowNames();

    foreach(const QString& name, names) {
        windowNames.push_back(GetOtherWindow(name));
    }

    return windowNames;
}

long 
GUIWrapper::GetViewerProxyPtr()
{
    return (long)(dynamic_cast<ViewerProxy*>(cliproxy));
}

GUIWrapper::~GUIWrapper()
{
    //std::cout << "being destroyed..." << std::endl;
    delete gui;
}

