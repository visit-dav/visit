#include <LauncherApplication.h>
#include <Init.h>
#include <VisItException.h>
#include <DebugStream.h>

// ****************************************************************************
// Function: main
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
// ****************************************************************************

int
main(int argc, char *argv[])
{
    int retval = 0;

    // Initialize error logging
    Init::Initialize(argc, argv);

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

