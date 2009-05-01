/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include <qapplication.h>
#include <qdir.h>

#include <visitstream.h>
#include <VisItViewer.h>
#include <VisItException.h>

#include <MultiCurveViewer.h>
#include <MultiCurveProcessor.h>

// ****************************************************************************
//  Method: main
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
//
// ****************************************************************************

int
main(int argc, char *argv[])
{
    int retval = 0;

    //
    // Process the command line.
    //
    bool nowin = false;
    char *commandFile = NULL;
    char *dataFile = NULL;

    char **argv2 = new char*[5];
    argv2[0] = argv[0];
    int argc2 = 1;
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-G") == 0)
        {
            nowin = true;
            argv2[argc2++] = "-nowin";
        }
        else if (strcmp(argv[i], "-f") == 0)
        {
            commandFile = argv[i+1];
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
