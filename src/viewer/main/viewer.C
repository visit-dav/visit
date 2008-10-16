/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

// ************************************************************************* //
//                                 viewer.C                                  //
// ************************************************************************* //

#include <visit-config.h>
#include <stdio.h>

#include <qapplication.h>

#include <VisItViewer.h>
#include <AppearanceAttributes.h>
#include <DebugStream.h>
#include <LostConnectionException.h>
#include <ViewerMethods.h>
#include <ViewerState.h>
#include <VisItException.h>

// ****************************************************************************
// Method: Viewer_LogQtMessages
//
// Purpose: 
//   Message handler that routes Qt messages to the debug logs.
//
// Arguments:
//   type : The message type.
//   msg  : The message.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 15 18:17:47 PST 2007
//
// Modifications:
//   
// ****************************************************************************

static void
Viewer_LogQtMessages(QtMsgType type, const char *msg)
{
    switch(type)
    {
    case QtDebugMsg:
        debug1 << "Qt: Debug: " << msg << endl;
        break;
    case QtWarningMsg:
        debug1 << "Qt: Warning: " << msg << endl;
        break;
    case QtFatalMsg:
        debug1 << "Qt: Fatal: " << msg << endl;
        abort();
        break;
    }
}

// ****************************************************************************
//  Method: main
//
//  Purpose:
//      The viewer main program.
//
//  Arguments:
//      argc    The number of command line arguments.
//      argv    The command line arguments.
//
//  Programmer: Eric Brugger
//  Creation:   August 16, 2000
//
//  Modifications:
//    Brad Whitlock, Thu Aug 14 10:09:17 PDT 2008
//    Rewrote so it uses the VisItViewer class, which encapsulates some of 
//    the stuff needed to set up the viewer. The new design permits us to 
//    embed the viewer in other Qt applications.
//
//    Jeremy Meredith, Thu Oct 16 19:29:12 EDT 2008
//    Added a flag for whether or not ProcessCommandLine should force 
//    the identical version.  It's necessary for new viewer-apps but
//    might confuse VisIt-proper's smart versioning.
//
// ****************************************************************************

int
main(int argc, char *argv[])
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

        //
        // Connect back to the client.
        //
        viewer.Connect(&argc, &argv);

        //
        // Process the command line arguments first since some may be removed
        // by QApplication::QApplication.
        //
        viewer.ProcessCommandLine(argc, argv, false);

        //
        // Create the QApplication. This sets the qApp pointer.
        //
        char **argv2 = new char *[argc + 3];
        int argc2 = argc + 2;
        for(int i = 0; i < argc; ++i)
            argv2[i] = argv[i];
        argv2[argc] = (char*)"-font";
        argv2[argc+1] = (char*)viewer.State()->GetAppearanceAttributes()->GetFontName().c_str();
        argv2[argc+2] = NULL;
        debug1 << "Viewer using font: " << argv2[argc+1] << endl;
        qInstallMsgHandler(Viewer_LogQtMessages);
        QApplication *mainApp = new QApplication(argc2, argv2, !viewer.GetNowinMode());

        //
        // Now that we've created the QApplication, let's call the viewer's
        // setup routine.
        //
        viewer.Setup();

        //
        // Execute the viewer.
        //
        bool keepGoing = true;
        while (keepGoing)
        {
            TRY
            { 
                retval = mainApp->exec();
                keepGoing = false;

                // Remove the crash recovery file on a successful exit.
                viewer.RemoveCrashRecoveryFile();
            }
            CATCH(LostConnectionException)
            {
                cerr << "The component that launched VisIt's viewer has terminated "
                        "abnormally." << endl;
                keepGoing = false;
                retval = -1;
            }
            CATCH2(VisItException, ve)
            {
                QString msg = QObject::tr("VisIt has encountered the following error: %1.\n"
                    "VisIt will attempt to continue processing, but it may "
                    "behave unreliably.  Please save this error message and "
                    "give it to a VisIt developer.  In addition, you may want "
                    "to save your session and re-start.  Of course, this "
                    "session may still cause VisIt to malfunction.").
                    arg(ve.Message().c_str());
                viewer.Error(msg);
                keepGoing = true;
            }
            ENDTRY
        }
    }
    CATCH2(VisItException, e)
    {
        debug1 << "VisIt's viewer encountered the following fatal "
                  "initialization error: " << endl
               << e.Message().c_str() << endl;
        retval = -1;
    }
    ENDTRY

    // Finalize the viewer library.
    VisItViewer::Finalize();

    return retval;
}



