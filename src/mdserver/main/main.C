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

#include <visit-config.h>

#include <VisItException.h>
#include <MDServerApplication.h>
#include <DebugStream.h>
#include <TimingsManager.h>
#include <Init.h>
#include <InitVTKNoGraphics.h>
#include <avtDatabase.h>
#include <avtDatabaseFactory.h>
#include <DatabasePluginManager.h>

// Prototypes.
bool ProcessCommandLine(int argc, char *argv[]);

#ifdef AIX
#ifdef HAVE_LIBBZ2
#include <bzlib.h>
void foobz2(void)
{
   BZ2_bzlibVersion();
}
#endif
#endif

// ****************************************************************************
// Function: main
//
// Purpose:
//   This is the main function for a metadata server.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 12 15:20:19 PST 2000
//
// Modifications:
//
//    Jeremy Meredith, Wed Aug  9 14:41:24 PDT 2000
//    Switched out plotAtts for plotRPC.
//
//    Jeremy Meredith, Wed Aug  9 14:41:24 PDT 2000
//    Cleaned up the way RPCs are handled.
//    Switched out quitAtts for quitRPC.
//
//    Brad Whitlock, Wed Aug 30 18:42:28 PST 2000
//    I added a line of code to close the outfile stream when
//    redirecting the output to a file. This eliminates a problem
//    that caused the server to core.
//
//    Hank Childs, Tue Sep 19 13:37:31 PDT 2000
//    Initialized exceptions.
//
//    Jeremy Meredith, Fri Nov 17 16:15:04 PST 2000
//    Removed initialization of exceptions and added general initialization.
//
//    Brad Whitlock, Thu Mar 15 12:31:14 PDT 2001
//    Added code to delete the MDServerApplication instance.
//
//    Hank Childs, Tue Apr 24 14:41:50 PDT 2001
//    Initialize vtk stream.
//
//    Hank Childs, Tue Sep 18 12:02:29 PDT 2001
//    Made return value be int.
//
//    Hank Childs, Mon Dec  3 09:55:54 PST 2001
//    Tell the databases that they are only going to be serving up meta-data.
//
//    Jeremy Meredith, Thu Aug 22 14:32:57 PDT 2002
//    Added database plugins.
//
//    Jeremy Meredith, Fri Feb 28 12:21:01 PST 2003
//    Renamed LoadPlugins to LoadPluginsNow (since there is a corresponding
//    LoadPluginsOnDemand).
//
//    Jeremy Meredith & Brad Whitlock, Mon Apr 14 18:35:49 PDT 2003
//    Moved database plugin loading to *after* the connection back to the
//    viewer.  This makes things a bit faster starting up for the user.
//
//    Brad Whitlock, Fri Apr 18 15:04:08 PST 2003
//    I made the ExecuteDebug method be called if -noconnect was given.
//
//    Brad Whitlock, Mon Jun 9 11:01:25 PDT 2003
//    I moved plugin loading.
//
//    Brad Whitlock, Mon Sep 8 16:58:33 PST 2003
//    I made it log uncaught exceptions.
//
//    Hank Childs, Thu Jan 22 17:46:08 PST 2004
//    Use InitVTKNoGraphics instead of InitVTK to avoid VTK/GL dependencies.
//
//    Hank Childs, Tue Jun  1 13:49:48 PDT 2004
//    Add call to finalize.
//
//    Jeremy Meredith, Tue Feb  8 08:49:46 PST 2005
//    Move initialization of the plugins from the LoadPlugins method to here.
//    It is much cheaper than the full loading of plugins and guarantees
//    that we can later query for their initialization errors and have a
//    meaningful answer.
//
//    Mark C. Miller, Thu Apr  3 14:36:48 PDT 2008
//    Moved setting of component name to before Initialize
// ****************************************************************************

int
main(int argc, char *argv[])
{
    int retval = 0;

    // Initialize error logging
    Init::SetComponentName("mdserver");
    Init::Initialize(argc, argv);
    InitVTKNoGraphics::Initialize();
    avtDatabase::SetOnlyServeUpMetaData(true);
    DatabasePluginManager::Initialize(DatabasePluginManager::MDServer, false);

    bool runApp = ProcessCommandLine(argc, argv);

    debug1 << "MDSERVER started." << endl;

    TRY
    {
        // Connect back to the process that launched the MDServer.
        MDServerApplication::Instance()->AddConnection(&argc, &argv);

        // Enter the program's main loop.
        if(runApp)
            MDServerApplication::Instance()->Execute();
        else
            MDServerApplication::Instance()->ExecuteDebug();

        // Clean up
        delete MDServerApplication::Instance();
    }
    CATCH2(VisItException, e)
    {
        debug1 << "VisIt's mdserver encountered the following uncaught "
               "exception: " << e.GetExceptionType().c_str()
               << " from (" << e.GetFilename().c_str()
               << ":" << e.GetLine() << ")" << endl
               << e.Message().c_str() << endl;
        retval = -1;
    }
    ENDTRY

    debug1 << "MDSERVER exited." << endl;
    Init::Finalize();
    return retval;
}

// ****************************************************************************
// Function: ProcessCommandLine
//
// Purpose:
//   Reads the command line arguments for the md server.
//
// Programmer: Eric Brugger
// Creation:   November 7, 2001
//
// Modifications:
//   Brad Whitlock, Fri Apr 18 15:07:07 PST 2003
//   I made it return a flag that tells whether we should connect back to
//   the calling program.
//
//   Hank Childs, Sun May  9 11:52:45 PDT 2004
//   Added the default_format argument.
//
//   Jeremy Meredith, Thu Jan 24 14:46:20 EST 2008
//   Added the assume_format argument.
//
//    Jeremy Meredith, Wed Mar 19 13:47:09 EDT 2008
//    Obsolete -default_format, add -fallback_format in its place.
//
// ****************************************************************************

bool
ProcessCommandLine(int argc, char *argv[])
{
    bool runApp = true;

    // process arguments.
    for (int i=1; i<argc; i++)
    {
        if (strcmp(argv[i], "-timing") == 0)
            visitTimer->Enable();
        else if (strcmp(argv[i], "-timeout") == 0)
        {
            MDServerApplication::Instance()->SetTimeout(atol(argv[i+1]));
            i++;
        }
        else if(strcmp(argv[i], "-noconnect") == 0)
        {
            runApp = false;
        }
        else if (strcmp(argv[i], "-fallback_format") == 0)
        {
            if ((i+1) < argc)
            {
                avtDatabaseFactory::SetFallbackFormat(argv[i+1]);
                i++;
            }
        }
        else if (strcmp(argv[i], "-assume_format") == 0)
        {
            if ((i+1) < argc)
            {
                avtDatabaseFactory::SetFormatToTryFirst(argv[i+1]);
                i++;
            }
        }
    }

    return runApp;
}

