#ifndef PYSIDEVIEWER_H
#define PYSIDEVIEWER_H

#include <QApplication>
#include <QMainWindow>
#include <QList>
#include <VisItInit.h>
//#include <InitVTK.h>
//#include <InitVTKRendering.h>
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

#include <string>

class QvisGUIApplicationDerived : public QvisGUIApplication
{
    QMainWindow* plotManagerWindow;
    QMainWindow* sourceManagerWindow;
    QMainWindow* timesliderWindow;
public:

    QvisGUIApplicationDerived(int argc,char* argv[],ViewerProxy* proxy = NULL):QvisGUIApplication(argc,argv,proxy)
    {
        //Ensure VisIt is ready..
        QEventLoop loop;
        connect(this,SIGNAL(VisItIsReady()),&loop,SLOT(quit()));
        loop.exec();
        GetMainWindow()->installEventFilter(this);

        //create a plot manager widget..
        plotManagerWindow = new QMainWindow();
        QvisPlotManagerWidget* plotManager = new QvisPlotManagerWidget(mainWin->menuBar(), plotManagerWindow);
        //plotManager->ConnectPlotList(GetViewerState()->GetPlotList());
        plotManager->ConnectFileServer(fileServer);
        plotManager->ConnectGlobalAttributes(GetViewerState()->GetGlobalAttributes());
        plotManager->ConnectExpressionList(GetViewerState()->GetExpressionList());
        plotManager->ConnectWindowInformation(GetViewerState()->GetWindowInformation());
        plotManager->ConnectSelectionList(GetViewerState()->GetSelectionList());
        plotManagerWindow->setCentralWidget(plotManager);

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

    QMainWindow* GetMainWindow() { return (QMainWindow*)mainWin; }
    QApplication* GetApp() { return (QApplication*)mainApp; }

    QMainWindow* GetPlotWindow(int index)
    {
        if(index >= plotWindows.size() || index < 0)
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

    QMainWindow* GetPlotWindow(const std::string& name)
    {
        PlotPluginManager* mgr = GetViewerProxy()->GetPlotPluginManager();
        for(int i = 0; i < mgr->GetNEnabledPlugins(); ++i)
        {
            if(mgr->GetPluginName(mgr->GetEnabledID(i)) == name)
                return GetPlotWindow(i);
        }
        return NULL;
    }

    QMainWindow* GetOperatorWindow(int index)
    {
        if(index >= operatorWindows.size() || index < 0)
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

    QMainWindow* GetOperatorWindow(const std::string& name)
    {
        OperatorPluginManager* mgr = GetViewerProxy()->GetOperatorPluginManager();
        for(int i = 0; i < mgr->GetNEnabledPlugins(); ++i)
        {
            if(mgr->GetPluginName(mgr->GetEnabledID(i)) == name)
                return GetOperatorWindow(i);
        }
        return NULL;
    }

    QMainWindow* GetOtherWindow(const QString& name)
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

    QStringList GetOtherWindowNames() { return windowNames; }

    QMainWindow* GetPlotManagerWindow() { return plotManagerWindow; }
    QMainWindow* GetSourceManagerWindow() { return sourceManagerWindow; }
    QMainWindow* GetTimeSliderWindow() { return timesliderWindow; }

    bool eventFilter(QObject *o, QEvent *e)
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
};

class PySideGUI
{
    static PySideGUI *_instance;
    QvisGUIApplicationDerived* gui;
    ViewerSubjectProxy *guiproxy,*cliproxy;
public:

    static PySideGUI* instance(int& argc,char** argv)
    {
        if (PySideGUI::_instance == NULL)
            PySideGUI::_instance = new PySideGUI(argc,argv);
        return PySideGUI::_instance;
    }

    static PySideGUI* instance()
    {
        return PySideGUI::_instance;
    }

    PySideGUI(int& argc,char** argv)
    {
        try
        {
            VisItInit::SetComponentName("gui");
            VisItInit::Initialize(argc, argv, 0, 1, false);

            guiproxy = new ViewerSubjectProxy();
            guiproxy->Initialize(argc,argv);
            gui = new QvisGUIApplicationDerived(argc, argv,guiproxy);

            cliproxy = new ViewerSubjectProxy(guiproxy);
            //cliproxy->SetAlternateSubjectProxy(guiproxy);
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

    QMainWindow* GetRenderWindow(int i) { return (QMainWindow*)guiproxy->GetRenderWindow(i); }
    QList<int> GetRenderWindowIDs() { return guiproxy->GetRenderWindowIDs(); } 
    QMainWindow* GetUIWindow() {return (QMainWindow*)gui->GetMainWindow();}

    QMainWindow* GetPlotWindow(int index) { return gui->GetPlotWindow(index); } 
    QMainWindow* GetOperatorWindow(int index) { return gui->GetOperatorWindow(index); }

    QMainWindow* GetPlotWindow(const std::string& name) { return gui->GetPlotWindow(name); }
    QMainWindow* GetOperatorWindow(const std::string& name) { return gui->GetOperatorWindow(name); }

    QStringList  GetOtherWindowNames() { return gui->GetOtherWindowNames(); }
    QMainWindow* GetOtherWindow(const QString& name) { return gui->GetOtherWindow(name); }

    QMainWindow* GetPlotManagerWindow() { return gui->GetPlotManagerWindow(); }
    QMainWindow* GetSourceManagerWindow() { return gui->GetSourceManagerWindow(); }
    QMainWindow* GetTimeSliderWindow() { return gui->GetTimeSliderWindow(); }

    ~PySideGUI()
    { 
        //std::cout << "being destroyed..." << std::endl;
        /*delete gui;*/ 
    }
};

#endif
