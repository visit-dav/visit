// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PYQTGUI_H
#define PYQTGUI_H

#include <QApplication>
#include <QMainWindow>
#include <QList>
#include <VisItInit.h>

#include <guiwrapper.h>
#include <ViewerSubjectProxy.h>

class PyQtGUI : public GUIWrapper
{
    //static PyQtGUI *_instance;
public:

    //static PyQtGUI* instance(int& argc,char** argv);
    //static PyQtGUI* instance();

    PyQtGUI(int& argc,char** argv): GUIWrapper(argc,argv){}
    ~PyQtGUI(){}
};

#endif
