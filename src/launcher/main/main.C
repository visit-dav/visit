// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <LauncherApplication.h>
#include <VisItInit.h>
#include <VisItException.h>
#include <DebugStream.h>

// ****************************************************************************
// Function: VCLMain
//
// Purpose:
//   This is the main function for the VisIt component launcher.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 16:02:19 PST 2003
//
// Modifications:
//
//    Mark C. Miller, Thu Apr  3 14:36:48 PDT 2008
//    Moved setting of component name to before Initialize
// ****************************************************************************

int
VCLMain(int argc, char *argv[])
{
    int retval = 0;

    // Initialize error logging
    VisItInit::SetComponentName("launcher");
    VisItInit::Initialize(argc, argv);

    debug1 << "VisIt component launcher started." << endl;

    TRY
    {
        LauncherApplication::Instance()->Execute(&argc, &argv);
        delete LauncherApplication::Instance();
    }
    CATCH(VisItException)
    {
        delete LauncherApplication::Instance();
        // Catch exceptions but don't bother doing anything.
        retval = -1;
    }
    ENDTRY

    debug1 << "VisIt component launcher exited." << endl;

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
    return VCLMain(__argc, __argv);
}
#else
int
main(int argc, char **argv)
{
    return VCLMain(argc, argv);
}
#endif
