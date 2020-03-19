// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <visitstream.h>
#include <VisItException.h>
#include <QvisGUIApplication.h>
#include <VisItInit.h>
#include <visit-config.h>
#include <QDebug>
#include <QApplication>

// ****************************************************************************
//  Function: GUIMain
//
//  Purpose:
//    This is the main function for the VisIt gui.
//
//  Notes:      
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jul 26 09:48:46 PDT 2000
//
//  Modifications:
//    Jeremy Meredith, Fri Nov 17 16:15:04 PST 2000
//    Removed initialization of exceptions and added general initialization.
//
//    Brad Whitlock, Mon Nov 27 16:10:21 PST 2000
//    I added a flag preventing the argument list from being messed with during
//    general initialization.
//
//    Jeremy Meredith, Fri May 11 13:52:51 PDT 2001
//    Added initialization of PlotPluginManager.
//
//    Jeremy Meredith, Thu Jul 26 03:34:42 PDT 2001
//    Added initialization of OperatorPluginManager.
//
//    Brad Whitlock, Wed Sep 26 09:46:36 PDT 2001
//    Modified code so it can handle when plugins fail to load.
//
//    Brad Whitlock, Mon Oct 22 18:25:42 PST 2001
//    Changed the exception handling keywords to macros.
//
//    Jeremy Meredith, Tue Jan 14 17:12:34 PST 2003
//    Added setColorSpec(ManyColor) so that it will try to get a 24-bit
//    visual for the GUI windows, even if 8-bit is the default.
//
//    Hank Childs, Tue Jun  1 14:07:05 PDT 2004
//    Added call to finalize.
//
//    Mark C. Miller, Thu Apr  3 14:36:48 PDT 2008
//    Moved setting of component name to before Initialize
//
//    Mark C. Miller, Wed Jun 17 14:27:08 PDT 2009
//    Replaced CATCHALL(...) with CATCHALL.
//
//    Cyrus Harrison, Fri Oct 11 15:40:29 PDT 2013
//    Clear any static lib paths (QCoreApplication::libraryPaths) to avoid conflicts with
//    loading qt after a make install or make package
//
// ****************************************************************************

int
GUIMain(int argc, char **argv)
{
    int retval = 0;

    TRY
    {
        // Initialize error logging.
        VisItInit::SetComponentName("gui");
        VisItInit::Initialize(argc, argv, 0, 1, false);

        TRY
        {
            // Create the application instance.
            QvisGUIApplication VisitGUI(argc, argv);
            // Execute the GUI and return its return code.
            retval = VisitGUI.Exec();
        }
        CATCHALL
        {
            retval = -1;
        }
        ENDTRY
    }
    CATCH2(VisItException, e)
    {
        cerr << "VisIt encountered the following fatal error during "
                "initialization: " << endl << e.Message().c_str() << endl;
        retval = -1;
    }
    ENDTRY

    VisItInit::Finalize();
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
    return GUIMain(__argc, __argv);
}
#else
int
main(int argc, char **argv)
{
    return GUIMain(argc, argv);
}
#endif
