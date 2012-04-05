#include "pyqtviewer.h"

#include <QMainWindow>
#include <QList>
#include <ViewerSubjectProxy.h>
#include <VisItInit.h>
#include <InitVTK.h>
#include <InitVTKRendering.h>

#include <string>
//Default constructor should not be used for anything other than testing..

PyQtViewer::PyQtViewer()
{
    char  arg0[] = "gui";
    char  arg1[] = "-dv";
    char* argv[] = { &arg0[0], &arg1[0], NULL };
    int   argc   = (int)(sizeof(argv) / sizeof(argv[0])) - 1;

    try
    {
        VisItInit::SetComponentName("cli");
        VisItInit::Initialize(argc, argv, 0, 1, false);

        InitVTK::Initialize();
        InitVTKRendering::Initialize();
        cliproxy = new ViewerSubjectProxy();
        cliproxy->Initialize(argc,argv);
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

PyQtViewer::PyQtViewer(int& argc,char** argv)
{
    try
    {
        VisItInit::SetComponentName("cli");
        VisItInit::Initialize(argc, argv, 0, 1, false);

        InitVTK::Initialize();
        InitVTKRendering::Initialize();
        cliproxy = new ViewerSubjectProxy();
        cliproxy->Initialize(argc,argv);
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

PyQtViewer::~PyQtViewer(){}
    
QMainWindow* 
PyQtViewer::GetRenderWindow(int i)
{
    return (QMainWindow*) cliproxy->GetRenderWindow(i);
}
    
QList<int> 
PyQtViewer::GetRenderWindowIDs()
{
   return cliproxy->GetRenderWindowIDs();
}
