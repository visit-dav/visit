#ifndef PYQTVIEWER_H
#define PYQTVIEWER_H

#include <QMainWindow>
#include <QList>

class ViewerSubjectProxy;

class PyQtViewer
{
    ViewerSubjectProxy* cliproxy;
public:

    //Default constructor should not be used for anything other than testing..
    PyQtViewer();
    PyQtViewer(int& argc,char** argv);
    ~PyQtViewer();
    
    QMainWindow* GetRenderWindow(int i);
    QList<int> GetRenderWindowIDs();
};

#endif
