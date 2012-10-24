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

#ifndef GUIWRAPPER_H
#define GUIWRAPPER_H

#include "pyui_common_exports.h"

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

class PYUI_COMMON_API QvisGUIApplicationDerived : public QvisGUIApplication
{
    //QMainWindow* plotManagerWindow;
    QMainWindow* sourceManagerWindow;
    QMainWindow* timesliderWindow;
public:

    QvisGUIApplicationDerived(int argc,char* argv[],ViewerProxy* proxy = NULL);
    ~QvisGUIApplicationDerived();

    QMainWindow* GetMainWindow();
    QApplication* GetApp();

    QMainWindow* GetPlotWindow(int index);
    QMainWindow* GetPlotWindow(const QString& name);

    QMainWindow* GetOperatorWindow(int index);
    QMainWindow* GetOperatorWindow(const QString& name);

    QMainWindow* GetOtherWindow(const QString& name);
    QStringList GetOtherWindowNames();

    QMainWindow* GetSourceManagerWindow();
    QMainWindow* GetTimeSliderWindow();

    bool eventFilter(QObject *o, QEvent *e);
};

class PYUI_COMMON_API GUIWrapper
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

    //QMainWindow* GetPlotManagerWindow() { return gui->GetPlotManagerWindow(); }
    
    QMainWindow* GetSourceManagerWindow();

    QMainWindow* GetTimeSliderWindow();

    long GetViewerProxyPtr();

    ~GUIWrapper();
};
#endif
