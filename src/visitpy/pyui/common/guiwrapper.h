// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef GUIWRAPPER_H
#define GUIWRAPPER_H

#include "guiwrapper_exports.h"

#include <QApplication>
#include <QMainWindow>
#include <QList>
#include <VisItInit.h>
#include <QvisMainWindow.h>
#include <SplashScreen.h>
#include <QvisGUIApplication.h>
#include <QvisPostableWindowObserver.h>
#include <QvisPlotManagerWidget.h>
#include <QvisSourceManagerWidget.h>
#include <QvisTimeSliderControlWidget.h>
#include <QEventLoop>

//#include <InitVTK.h>
//#include <InitVTKRendering.h>
//-- Viewer controls..

#include <ViewerSubjectProxy.h>

class GUIWRAPPER_API QvisGUIApplicationDerived : public QvisGUIApplication
{
    //QMainWindow* plotManagerWindow;
    QMainWindow* sourceManagerWindow;
    QMainWindow* timesliderWindow;
public:

    QvisGUIApplicationDerived(int argc,char* argv[],ViewerProxy* proxy = NULL);
    ~QvisGUIApplicationDerived();

    QMainWindow* GetMainWindow();
    QApplication* GetApp();

    int          GetPlotWindowSize();
    QMainWindow* GetPlotWindow(int index);
    QMainWindow* GetPlotWindow(const QString& name);

    int          GetOperatorWindowSize();
    QMainWindow* GetOperatorWindow(int index);
    QMainWindow* GetOperatorWindow(const QString& name);

    QMainWindow* GetOtherWindow(const QString& name);
    QStringList GetOtherWindowNames();

    QMainWindow* GetSourceManagerWindow();
    QMainWindow* GetTimeSliderWindow();

    bool eventFilter(QObject *o, QEvent *e);
};

class GUIWRAPPER_API GUIWrapper
{
protected:
    QvisGUIApplicationDerived* gui;
    ViewerSubjectProxy *guiproxy,*cliproxy;

public:
    GUIWrapper(int& argc,char** argv);

    QMainWindow* GetRenderWindow(int i);
    
    QList<int> GetRenderWindowIDs();
    
    QMainWindow* GetUIWindow();

    QMainWindow* GetPlotWindow(int index);
    
    QMainWindow* GetOperatorWindow(int index);

    QMainWindow* GetPlotWindow(const QString& name);

    QMainWindow* GetOperatorWindow(const QString& name);

    QStringList  GetOtherWindowNames();
    
    QMainWindow* GetOtherWindow(const QString& name);

    /// Get list of all windows 
    QList<QMainWindow*> GetPlotWindows();
    QList<QMainWindow*> GetOperatorWindows();
    QList<QMainWindow*> GetOtherWindows();

    //QMainWindow* GetPlotManagerWindow(){ return gui->GetPlotManagerWindow(); }
    
    QMainWindow* GetSourceManagerWindow();

    QMainWindow* GetTimeSliderWindow();

    long GetViewerProxyPtr();

    ~GUIWrapper();
};
#endif
