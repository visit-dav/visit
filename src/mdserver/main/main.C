#include <VisItException.h>
#include <MDServerApplication.h>
#include <DebugStream.h>
#include <TimingsManager.h>
#include <Init.h>
#include <InitVTK.h>
#include <avtDatabase.h>
#include <DatabasePluginManager.h>
#include <VisItException.h>

// Prototypes.
bool ProcessCommandLine(int argc, char *argv[]);

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
// ****************************************************************************

int
main(int argc, char *argv[])
{
    // Initialize error logging
    Init::Initialize(argc, argv);
    InitVTK::Initialize();

    bool runApp = ProcessCommandLine(argc, argv);

    debug1 << "MDSERVER started." << endl;

    TRY
    {
        // Connect back to the process that launched the MDServer.
        MDServerApplication::Instance()->AddConnection(&argc, &argv);

        avtDatabase::SetOnlyServeUpMetaData(true);
        DatabasePluginManager::Initialize(DatabasePluginManager::MDServer, false);
        DatabasePluginManager::Instance()->LoadPluginsNow();

        // Enter the program's main loop.
        if(runApp)
            MDServerApplication::Instance()->Execute();
        else
            MDServerApplication::Instance()->ExecuteDebug();

        // Clean up
        delete MDServerApplication::Instance();
    }
    CATCH(VisItException)
    {
        ; // Catch exceptions but don't bother doing anything.
    }
    ENDTRY

    debug1 << "MDSERVER exited." << endl;
    return 0;
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
    }

    return runApp;
}

