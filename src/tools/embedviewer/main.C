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
#ifdef Q_WS_MACX
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
#ifdef Q_WS_MACX
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

