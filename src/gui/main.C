#include <visitstream.h>
#include <VisItException.h>
#include <QvisGUIApplication.h>
#include <Init.h>

// ****************************************************************************
//  Function: main
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
// ****************************************************************************

int
main(int argc, char **argv)
{
    int retval = 0;

    TRY
    {
        // Initialize error logging.
        Init::Initialize(argc, argv, 0, 1, false);
        Init::SetComponentName("gui");

        TRY
        {
            // Create the application instance.
            QvisGUIApplication VisitGUI(argc, argv);

            // Execute the GUI and return its return code.
            retval = VisitGUI.Exec();
        }
        CATCHALL(...)
        {
            retval = -1;
        }
        ENDTRY
    }
    CATCH2(VisItException, e)
    {
        cerr << "VisIt encountered the following fatal error during "
                "initialization: " << endl << e.GetMessage().c_str() << endl;
        retval = -1;
    }
    ENDTRY

    Init::Finalize();
    return retval;
}
