// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QApplication>
#include <QDir>

#include <visit-config.h> // For LIB_VERSION_LE
#if LIB_VERSION_LE(VTK, 8,1,0)
#include <QVTKOpenGLWidget.h>
#else
#include <QVTKOpenGLNativeWidget.h>
#endif

#include <visitstream.h>
#include <VisItViewer.h>
#include <VisItException.h>

#include <MultiCurveViewer.h>
#include <MultiCurveProcessor.h>

// ****************************************************************************
//  Method: MCVMain
//
//  Purpose:
//      The embedded viewer main program.
//
//  Arguments:
//      argc    The number of command line arguments.
//      argv    The command line arguments.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Aug 18 16:49:40 PDT 2008
//
//  Modifications:
//    Eric Brugger, Fri Jan 24 10:04:53 PST 2020
//    I added QSurfaceFormat initialization so that Qt will create an
//    OpenGL 3.2 context. I also added support for "-debug" on the command
//    line.
//
// ****************************************************************************

int
MCVMain(int argc, char *argv[])
{
    int retval = 0;

    //
    // Process the command line.
    //
    bool nowin = false;
    char *commandFile = NULL;
    char *dataFile = NULL;

    // The length of argv2 assumes that each option is only specified once. 
    // If not, then it can overflow.
    char **argv2 = new char*[5];
    argv2[0] = argv[0];
    int argc2 = 1;
    char nowin_str[] = "-nowin";
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-G") == 0)
        {
            nowin = true;
            argv2[argc2++] = nowin_str;
        }
        else if (strcmp(argv[i], "-f") == 0)
        {
            commandFile = argv[i+1];
            i++;
        }
        else if (strcmp(argv[i], "-debug") == 0)
        {
            argv2[argc2++] = argv[i];
            argv2[argc2++] = argv[i+1];
            i++;
        }
        else
        {
            dataFile = argv[i];
        }
    }

    //
    // Do basic initialization. This is only done once to initialize the
    // viewer library.
    //
    VisItViewer::Initialize(&argc2, &argv2);

    TRY
    {
        //
        // Create the viewer.
        //
        VisItViewer viewer;

        //
        // Set up VISITHOME so it finds the rest of VisIt. If the environment
        // variable VISITHOME is set, then assume that we are running a public
        // installation and use that, otherwise assume we are running in the
        // repo and set it accordingly.
        //
        if (getenv("VISITHOME") == NULL)
        {
            QDir d("../../");
            std::string visithome(d.absolutePath().toStdString());
            viewer.SetVISITHOME(visithome);
        }
        else
        {
            std::string visithome(getenv("VISITHOME"));
            std::string visithomebase(visithome, 0, visithome.rfind("/"));
            viewer.SetVISITHOME(visithomebase);
        }

        //
        // Process the command line arguments first since some may be removed
        // by QApplication::QApplication.
        //
        viewer.ProcessCommandLine(argc2, argv2);

        //
        // Setting the default QSurfaceFormat required with QVTKOpenGLwidget.
        // This causes Qt to create an OpenGL 3.2 context.
        //
#if LIB_VERSION_LE(VTK, 8,1,0)
        auto surfaceFormat = QVTKOpenGLWidget::defaultFormat();
#else
        auto surfaceFormat = QVTKOpenGLNativeWidget::defaultFormat();
#endif
        surfaceFormat.setSamples(0);
        surfaceFormat.setAlphaBufferSize(0);
        QSurfaceFormat::setDefaultFormat(surfaceFormat);

        //
        // Create the QApplication. This sets the qApp pointer.
        //
        QApplication *mainApp = new QApplication(argc2, argv2, !nowin);

        if (!nowin)
        {
            //
            // Create our gui app. We have to do it before the call
            // to Setup() since we're embedding vis windows.
            //
            MultiCurveViewer *guiApp = new MultiCurveViewer(&viewer);

            //
            // Now that we've created the QApplication, let's call the viewer's
            // setup routine.
            //
            viewer.Setup();

            //
            // Attach the attribute subjects that the observes.
            //
            guiApp->AttachSubjects();

            //
            // Show our app's main window
            //
            guiApp->show();
            guiApp->raise();
        }
        else
        {
            //
            // Now that we've created the QApplication, let's call the viewer's
            // setup routine.
            //
            viewer.Setup();

            //
            // Create our cli app.
            //
            MultiCurveProcessor *cliApp = new MultiCurveProcessor(&viewer);

            //
            // Tell the cli the name of the command file and data file.
            //
            cliApp->ProcessBatchFile(commandFile, dataFile);
        }

        //
        // Execute the viewer.
        //
        retval = mainApp->exec();
    }
    CATCH2(VisItException, e)
    {
        cerr << "VisIt's viewer encountered the following fatal "
                "initialization error: " << endl
             << e.Message().c_str() << endl;
        retval = -1;
    }
    ENDTRY

    // Finalize the viewer library.
    VisItViewer::Finalize();

    return retval;
}

// ****************************************************************************
// Method: main/WinMain
//
// Purpose: 
//   The program entry point function.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 23 13:15:31 PST 2011
//
// Modifications:
//   
// ****************************************************************************

#if defined(_WIN32) && defined(VISIT_WINDOWS_APPLICATION)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int WINAPI
WinMain(HINSTANCE hInstance,     // handle to the current instance
        HINSTANCE hPrevInstance, // handle to the previous instance    
        LPSTR lpCmdLine,         // pointer to the command line
        int nCmdShow             // show state of window
)
{
    return MCVMain(__argc, __argv);
}
#else
int
main(int argc, char **argv)
{
    return MCVMain(argc, argv);
}
#endif
