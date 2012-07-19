#include "pyqtgui.h"
#include <QApplication>
#include <QMainWindow>
#include <QList>
#include <VisItInit.h>
#include <QvisMainWindow.h>
#include <SplashScreen.h>
#include <QvisGUIApplication.h>
#include <QvisPostableWindowObserver.h>
#include <QEventLoop>

//-- Viewer controls..

#include <ViewerSubjectProxy.h>

class QvisGUIApplicationDerived : public QvisGUIApplication
{
public:

    QvisGUIApplicationDerived(int argc,char* argv[],ViewerProxy* proxy = NULL):QvisGUIApplication(argc,argv,proxy)
    {
        //Ensure VisIt is ready..
        QEventLoop loop;
        connect(this,SIGNAL(VisItIsReady()),&loop,SLOT(quit()));
        loop.exec();
        GetMainWindow()->installEventFilter(this);
    }

    QMainWindow* GetMainWindow() { return (QMainWindow*)mainWin; }
    QApplication* GetApp() { return (QApplication*)mainApp; }

    QMainWindow* GetPlotWindow(int index)
    {
        if(index >= plotWindows.size() || index < 0)
            return NULL;

        ActivatePlotWindow(index);

        ((QMainWindow*)plotWindows[index])->hide();
        return (QMainWindow*)plotWindows[index];
    }

    QMainWindow* GetPlotWindow(const QString& name)
    {
        PlotPluginManager* mgr = GetViewerProxy()->GetPlotPluginManager();
        for(int i = 0; i < mgr->GetNEnabledPlugins(); ++i)
        {
            if(mgr->GetPluginName(mgr->GetEnabledID(i)) == name.toStdString())
                return GetPlotWindow(i);
        }
        return NULL;
    }

    QMainWindow* GetOperatorWindow(int index)
    {
        if(index >= operatorWindows.size() || index < 0)
            return NULL;

        ActivateOperatorWindow(index);

        ((QMainWindow*)operatorWindows[index])->hide();
        return (QMainWindow*)operatorWindows[index];
    }

    QMainWindow* GetOperatorWindow(const QString& name)
    {
        OperatorPluginManager* mgr = GetViewerProxy()->GetOperatorPluginManager();
        for(int i = 0; i < mgr->GetNEnabledPlugins(); ++i)
        {
            if(mgr->GetPluginName(mgr->GetEnabledID(i)) == name.toStdString())
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
                win->hide(); 
                return win;
            }
        }
        return NULL;
    }

    QStringList GetOtherWindowNames()
    {
        return windowNames;
    }

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

/*
PyQtGUI* 
PyQtGUI::instance(int& argc,char** argv)
{
    if (PyQtGUI::_instance == NULL)
        PyQtGUI::_instance = new PyQtGUI(argc,argv);
    return PyQtGUI::_instance;
}

PyQtGUI* 
PyQtGUI::instance()
{
    return PyQtGUI::_instance;
}
*/

PyQtGUI::PyQtGUI(int& argc,char** argv)
{
    try
    {
        VisItInit::SetComponentName("gui");
        VisItInit::Initialize(argc, argv, 0, 1, false);

        guiproxy = new ViewerSubjectProxy();
        guiproxy->Initialize(argc,argv);
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
PyQtGUI::GetRenderWindow(int i) 
{ 
    return (QMainWindow*)guiproxy->GetRenderWindow(i); 
}

QList<int> 
PyQtGUI::GetRenderWindowIDs() 
{ 
    return guiproxy->GetRenderWindowIDs(); 
} 

QMainWindow* 
PyQtGUI::GetUIWindow() 
{
    return (QMainWindow*)gui->GetMainWindow();
}

QMainWindow* 
PyQtGUI::GetPlotWindow(int index) 
{ 
    return gui->GetPlotWindow(index); 
}
 
QMainWindow* 
PyQtGUI::GetOperatorWindow(int index) 
{ 
    return gui->GetOperatorWindow(index); 
}

QMainWindow* 
PyQtGUI::GetPlotWindow(const QString& name) 
{ 
    return gui->GetPlotWindow(name); 
}

QMainWindow* 
PyQtGUI::GetOperatorWindow(const QString& name) 
{
    return gui->GetOperatorWindow(name); 
}

QStringList 
PyQtGUI::GetOtherWindowNames() 
{ 
    return gui->GetOtherWindowNames(); 
}

QMainWindow* 
PyQtGUI::GetOtherWindow(const QString& name) 
{ 
    return gui->GetOtherWindow(name); 
}


PyQtGUI::~PyQtGUI()
{ 
    std::cout << "being destroyed..." << std::endl;
    /*delete gui;*/ 
}
