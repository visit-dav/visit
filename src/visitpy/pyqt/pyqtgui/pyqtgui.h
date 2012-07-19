#ifndef PYQTGUI_H
#define PYQTGUI_H

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
#include <QEventLoop>

//-- Viewer controls..

#include <ViewerSubjectProxy.h>

class QvisGUIApplicationDerived;

class PyQtGUI
{
    //static PyQtGUI *_instance;
    QvisGUIApplicationDerived* gui;
    ViewerSubjectProxy *guiproxy,*cliproxy;
public:

    //static PyQtGUI* instance(int& argc,char** argv);
    //static PyQtGUI* instance();

    PyQtGUI(int& argc,char** argv);

    QMainWindow* GetRenderWindow(int i); 
    QList<int> GetRenderWindowIDs();
    QMainWindow* GetUIWindow();

    QMainWindow* GetPlotWindow(int index);
    QMainWindow* GetOperatorWindow(int index);

    QMainWindow* GetPlotWindow(const QString& name);
    QMainWindow* GetOperatorWindow(const QString& name);

    QStringList GetOtherWindowNames();
    QMainWindow* GetOtherWindow(const QString& name);

    ~PyQtGUI();
};

#endif
