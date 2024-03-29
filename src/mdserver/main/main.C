// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <visit-config.h>

#include <VisItException.h>
#include <MDServerApplication.h>
#include <DebugStream.h>
#include <TimingsManager.h>
#include <VisItInit.h>
#include <InitVTKLite.h>
#include <avtCallback.h>
#include <avtDatabase.h>
#include <avtDatabaseFactory.h>
#include <DatabasePluginManager.h>

#ifdef HAVE_CONDUIT
#include <avtConduitBlueprintDataAdaptor.h>
#endif

#include <cstring>

// Prototypes.
bool ProcessCommandLine(int argc, char *argv[]);

// HACK: Needed to force linking of libz on AIX
#ifdef AIX
#ifdef HAVE_LIBZ
#include <zlib.h>
void fooz(void)
{
   zlibVersion();
}
#endif
#endif

// ****************************************************************************
// Function: MDServerMain
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
//
//    Brad Whitlock, Tue Jun 24 15:15:39 PDT 2008
//    I changed how the plugin info gets read.
//
//    Brad Whitlock, Wed Nov  4 12:19:46 PST 2009
//    I renamed a namespace to InitVTKLite.
// 
//    Justin Privitera, Wed Aug 24 11:08:51 PDT 2022
//    Call `avtConduitBlueprintDataAdaptor::Initialize();`.
//
// ****************************************************************************

int
MDServerMain(int argc, char *argv[])
{
    int retval = 0;

    // Initialize error logging
    VisItInit::SetComponentName("mdserver");
    VisItInit::Initialize(argc, argv);
    InitVTKLite::Initialize();
    avtDatabase::SetOnlyServeUpMetaData(true);
#ifdef HAVE_CONDUIT
    avtConduitBlueprintDataAdaptor::Initialize();
#endif

    bool runApp = ProcessCommandLine(argc, argv);

    debug1 << "MDSERVER started." << endl;

    TRY
    {
        // Read the plugin info.
        MDServerApplication::Instance()->InitializePlugins();

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
    VisItInit::Finalize();
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
//   Jeremy Meredith, Wed Mar 19 13:47:09 EDT 2008
//   Obsolete -default_format, add -fallback_format in its place.
//
//   Jeremy Meredith, Wed Dec 30 14:46:31 EST 2009
//   Removed assume and fallback format options.  This is obsolete with
//   the new file format detection now in visit proper.
//
//   Hank Childs, Tue Jan 18 09:39:17 PST 2011
//   Add support for -auxsessionkey.
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
        else if(strcmp(argv[i], "-auxsessionkey") == 0)
        {
            if (i+1 < argc)
            {
                std::string s = argv[i+1];
                avtCallback::SetAuxSessionKey(s);
                i++;
            }
        }
    }

    return runApp;
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
    return MDServerMain(__argc, __argv);
}
#else
int
main(int argc, char **argv)
{
    return MDServerMain(argc, argv);
}
#endif
