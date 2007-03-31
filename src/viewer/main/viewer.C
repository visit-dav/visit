// ************************************************************************* //
//                                 viewer.C                                  //
// ************************************************************************* //
#include <DebugStream.h>
#include <ViewerSubject.h>
#include <VisItException.h>
#include <Init.h>
#include <InitVTK.h>
#include <RemoteProcess.h>
#include <ViewerPasswordWindow.h>
#include <PlotPluginManager.h>
#include <OperatorPluginManager.h>

#if defined(_WIN32)
// Get around a macro problem on Windows
#define GetMessageA GetMessage
#endif

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
//    Brad Whitlock, Fri Oct 27 14:49:04 PST 2000
//    I passed argc, argv to the ViewerSubject constructor and made it
//    return an error code.
//
//    Jeremy Meredith, Fri Nov 17 16:15:04 PST 2000
//    Removed initialization of exceptions and added general initialization.
//
//    Brad Whitlock, Mon Nov 27 17:24:37 PST 2000
//    Changed the call to Init::Initialize.
//
//    Hank Childs, Tue Apr 24 15:23:35 PDT 2001
//    Initialize VTK modules.
//
//    Brad Whitlock, Thu Apr 26 13:34:18 PST 2001
//    Removed top-level catch so uncaught exceptions will cause the code to
//    core. That makes it easier to find the exceptions and more likely that
//    they will be caught.
//
//    Jeremy Meredith, Fri Apr 27 15:46:47 PDT 2001
//    Added initialization of the remote process instantiation
//    authentication callback.
//
//    Jeremy Meredith, Thu May 10 14:56:48 PDT 2001
//    Addded plot plugin manager initialization.
//
//    Jeremy Meredith, Thu Jul 26 03:14:23 PDT 2001
//    Added operator plugin manager initialization.
//
//    Brad Whitlock, Wed Jul 18 09:12:22 PDT 2001
//    Registered a view callback.
//
//    Hank Childs, Mon Aug 20 21:41:59 PDT 2001
//    Changed format for view callbacks.
//
//    Eric Brugger, Tue Aug 21 10:21:18 PDT 2001
//    I removed the registration of the view callback.
//
//    Kathleen Bonnell, Fri Feb  7 09:09:47 PST 2003 
//    I moved the registration of the authentication callback to ViewerSubject.
//
// ****************************************************************************

int
main(int argc, char *argv[])
{
    int retval = 0;

    TRY
    {
        //
        // Initialize the error logging.
        //
        Init::Initialize(argc, argv, 0, 1, false);
        Init::SetComponentName("viewer");
        InitVTK::Initialize();

        //
        // Load the plugins.
        //
        PlotPluginManager::Initialize(PlotPluginManager::Viewer);
        OperatorPluginManager::Initialize(OperatorPluginManager::Viewer);

        //
        // Create the viewer subject. This connects back to the client.
        //
        ViewerSubject viewerSubject(&argc, &argv);

        //
        // Connect various internal state objects that must be done after 
        // reading the plugins.
        //
        viewerSubject.Connect(&argc, &argv);

        //
        // Execute the viewer.
        //
        TRY
        {
            retval = viewerSubject.Execute();
        }
        CATCH2(VisItException, e)
        {
            debug1 << "VisIt's viewer encountered the following uncaught "
                   "exception: " << e.GetExceptionType().c_str()
                   << " from (" << e.GetFilename().c_str()
                   << ":" << e.GetLine() << ")" << endl
                   << e.GetMessage().c_str() << endl;
            retval = -1;
        }
        ENDTRY
    }
    CATCH2(VisItException, e)
    {
        debug1 << "VisIt's viewer encountered the following fatal "
                  "initialization error: " << endl
               << e.GetMessage().c_str() << endl;
        retval = -1;
    }
    ENDTRY

    return retval;
}
