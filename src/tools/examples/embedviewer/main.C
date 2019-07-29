// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <visit-config.h>

#include <qapplication.h>
#include <qdir.h>

#include <visitstream.h>
#include <VisItViewer.h>
#include <VisItException.h>

#include <QWidget>

extern QWidget *create_application_main_window(VisItViewer *v,
    int *argc, char ***argv);
extern void show_application_main_window(QWidget *w);

// ****************************************************************************
//  Method: EmbeddedMain
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
//    Brad Whitlock, Tue Sep 14 11:44:35 PDT 2010
//    I recoded the logic that sets VISITHOME so it is more tolerant of where
//    the executable gets placed in the VisIt build tree. I think it only works
//    for UNIX in-source builds though. A real app would do something better.
//
//    Brad Whitlock, Tue Nov 30 11:09:15 PST 2010
//    Get the window from an external routine so we can reuse main.C
//
//    Mark C. Miller, Tue Jan 11 17:58:10 PST 2011
//    Pass argc/argv to app to open file from command line.
// ****************************************************************************

int
EmbeddedMain(int argc, char *argv[])
{
    int retval = 0;

    //
    // Do basic initialization. This is only done once to initialize the
    // viewer library.
    //
    VisItViewer::Initialize(&argc, &argv);

    TRY
    {
        //
        // Create the viewer.
        //
        VisItViewer viewer;

        // Go up the dierctory structures until we find the "src" directory,
        // which is what we'll use for VISITHOME so it finds our development 
        // version of VisIt.
#ifndef _WIN32
#if defined(Q_OS_MAC)
        QDir d(argv[0]);
#else
        QDir d;
#endif
        bool okay = true;
        while(okay && !d.absolutePath().endsWith("src"))
            okay = d.cdUp();
        std::string visithome(d.absolutePath().toStdString());
        if (visithome == "/")
        {
#if defined(Q_OS_MAC)
            QDir d2(argv[0]);
#else
            QDir d2;
#endif
            okay = true;
            while(okay && !d2.absolutePath().endsWith("test"))
                okay = d2.cdUp();
            okay = d2.cdUp();
            if (okay) okay = d2.cd("src");
            if (okay) visithome = std::string(d2.absolutePath().toStdString());
        }

        // See if there's an override on the command line. We need this for
        // out of source builds currently.
        for(int i = 1; i < argc; ++i)
        {
           if(strcmp(argv[i], "-visithome") == 0 && (i+1)<argc)
           {
               visithome = argv[i+1];
               ++i;
           }
        }

        qDebug("Setting VISITHOME to %s", visithome.c_str());
        viewer.SetVISITHOME(visithome);
#endif

        //
        // Process the command line arguments first since some may be removed
        // by QApplication::QApplication.
        //
        viewer.ProcessCommandLine(argc, argv);

        //
        // Create the QApplication. This sets the qApp pointer.
        //
        QApplication *mainApp = new QApplication(argc, argv);

        //
        // Create our visualization app. We have to do it before the call to Setup()
        // since we're embedding vis windows.
        //
        QWidget *visapp = create_application_main_window(&viewer, &argc, &argv);

        //
        // Now that we've created the QApplication, let's call the viewer's
        // setup routine.
        //
        viewer.Setup();

        //
        // Show our app's main window
        //
        show_application_main_window(visapp);

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
    return EmbeddedMain(__argc, __argv);
}
#else
int
main(int argc, char **argv)
{
    return EmbeddedMain(argc, argv);
}
#endif

