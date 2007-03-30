#include <SplashScreen.h>
#include <qapplication.h>
#include <qsocketnotifier.h>
#include <SplashScreenApplication.h>
#include <IncompatibleVersionException.h>
#include <LostConnectionException.h>
#include <SocketConnection.h>

// ****************************************************************************
// Method: SplashScreenApplication::SplashScreenApplication
//
// Purpose: 
//   This is the constructor for the SplashScreenApplication class.
//
// Arguments:
//   argc : An int reference to the number of command line arguments.
//   argv : An array of strings containing the command line arguments.
//
// Programmer: Sean Ahern
// Creation:   Fri Aug 31 08:51:26 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Sep 4 08:52:41 PDT 2001
//   Moved some code around to try and get the splash screen to appear
//   first thing.
//
//   Sean Ahern, Wed Sep 12 15:05:16 PDT 2001
//   Added a -nosplash argument that makes the splash screen not appear
//   initially.
//
//   Brad Whitlock, Mon Mar 25 12:59:01 PDT 2002
//   I modified how communication is done.
//
// ****************************************************************************

SplashScreenApplication::SplashScreenApplication(int &argc, char **argv) :
    atts(), appearanceAtts()
{
    // Start up the QApplication.
    app = new QApplication(argc, argv);

    // Look for the "-nosplash" argument
    bool    displaySplash = true;
    for (int i = 1; i < argc; ++i)
    {
        if (std::string(argv[i]) == std::string("-nosplash"))
        {
            displaySplash = false;
        }
    }

    // Create the splash screen.
    splash = new SplashScreen("SplashScreen", displaySplash);
    splash->ConnectSplashScreenAtts(&atts);
    splash->ConnectAppearanceAtts(&appearanceAtts);

    // Connect to the GUI.
    TRY
    {
        theGUI.Connect(&argc, &argv, 1);
    }
    CATCH(IncompatibleVersionException)
    {
        cerr << "The splash screen has a different version than the GUI." <<
            endl;
        RETHROW;
    }
    ENDTRY

    // Hook up the GUI to the attributes.
    xfer.SetInputConnection(theGUI.GetWriteConnection());
    xfer.Add(&atts);
    xfer.Add(&appearanceAtts);

    // Set up the socket notifier.
    QSocketNotifier *notif = new QSocketNotifier(
        theGUI.GetWriteConnection()->GetDescriptor(), QSocketNotifier::Read);
    connect(notif, SIGNAL(activated(int)), this, SLOT(ReadFromGUI(int)));
}

// ****************************************************************************
// Method: SplashScreenApplication::Go
//
// Purpose: 
//   Executes the application's main loop.
//
// Programmer: Sean Ahern
// Creation:   Fri Aug 31 08:51:26 PDT 2001
//
// Modifications:
//
// ****************************************************************************
void
SplashScreenApplication::Go()
{
    // Tell Qt to go ahead
    app->exec();
}

// ****************************************************************************
// Method: SplashScreenApplication::ReadFromGUI
//
// Purpose: 
//   Executes the application's main loop.
//
// Programmer: Sean Ahern
// Creation:   Fri Aug 31 08:51:26 PDT 2001
//
// Modifications:
//   Brad Whitlock, Wed Mar 20 17:42:13 PST 2002
//   Abstracted the read code.
//
//   Brad Whitlock, Mon Apr 7 17:21:02 PST 2003
//   I added code to make the application quit if it gets a lost connection
//   exception so that it does not core on Linux when the GUI dies.
//
// ****************************************************************************
void
SplashScreenApplication::ReadFromGUI(int)
{
    TRY
    {
        // Try and read from the GUI.
        int amountRead = theGUI.GetWriteConnection()->Fill();

        // Try and process the input.
        if (amountRead > 0)
            xfer.Process();
    }
    CATCH(LostConnectionException)
    {
        app->quit();
    }
    ENDTRY
}
