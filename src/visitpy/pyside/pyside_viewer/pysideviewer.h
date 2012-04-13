#ifndef PYSIDEVIEWER_H
#define PYSIDEVIEWER_H

#include <QMainWindow>
#include <QList>
#include <ViewerSubjectProxy.h>
#include <VisItInit.h>
#include <InitVTK.h>
#include <InitVTKRendering.h>

#include <string>

class PySideViewer
{
    static PySideViewer *_instance;
    ViewerSubjectProxy  *cliproxy;

public:

    static PySideViewer* instance(int& argc,char** argv)
    {
        if (PySideViewer::_instance == NULL)
            PySideViewer::_instance = new PySideViewer(argc,argv);
        return PySideViewer::_instance;
    }

    static PySideViewer* instance()
    {
        return PySideViewer::_instance;
    }

    //Default constructor should not be used for anything other than testing..
    PySideViewer()
    {
        char  arg0[] = "gui";
        char  arg1[] = "-dv";
        char* argv[] = { &arg0[0], &arg1[0], NULL };
        int   argc   = (int)(sizeof(argv) / sizeof(argv[0])) - 1;

        try{
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

    PySideViewer(int& argc,char** argv)
    {
        try{
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

    ~PySideViewer(){}

    QMainWindow* GetRenderWindow(int i)
    {
        return (QMainWindow*) cliproxy->GetRenderWindow(i);
    }

    QList<int> GetRenderWindowIDs()
    {
        return cliproxy->GetRenderWindowIDs();
    }
};

#endif
