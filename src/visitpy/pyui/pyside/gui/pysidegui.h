// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PYSIDEVIEWER_H
#define PYSIDEVIEWER_H

#include <QApplication>
#include <QMainWindow>
#include <QList>
#include <VisItInit.h>

#include <guiwrapper.h>
#include <ViewerSubjectProxy.h>

class PySideGUI : public GUIWrapper
{
    static PySideGUI *_instance;
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

    PySideGUI(int& argc,char** argv) : GUIWrapper(argc,argv)
    {}

    ~PySideGUI(){}
};

#endif
