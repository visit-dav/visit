#include <SplashScreenProxy.h>
#include <RemoteProcess.h>
#include <SocketConnection.h>
#include <Xfer.h>
#include <LostConnectionException.h>
#include <iostream.h>

// ****************************************************************************
// Method: SplashScreenProxy::SplashScreenProxy
//
// Purpose: 
//   This is the constructor for the SplashScreenProxy class.
//
// Programmer: Sean Ahern
// Creation:   Thu Sep 6 16:06:55 PST 2001
//
// Modifications:
//   
// ****************************************************************************
SplashScreenProxy::SplashScreenProxy() : atts(), appearanceAtts()
{
    argc = 0;
    argv = NULL;
}

// ****************************************************************************
// Method: SplashScreenProxy::~SplashScreenProxy
//
// Purpose: 
//   This is the destructor for the SplashScreenProxy class.
//
// Programmer: Sean Ahern
// Creation:   Thu Sep 6 16:06:55 PST 2001
//
// Modifications:
//   
// ****************************************************************************
SplashScreenProxy::~SplashScreenProxy()
{
    delete  xfer;
    delete  splash;

    // Delete the extra command line arguments
    for (int i = 0; i < argc; ++i)
        delete[] argv[i];
    if (argc > 0)
        delete[] argv;
}

// ****************************************************************************
// Method: ViewerProxy::AddArgument
//
// Purpose: 
//   Adds an argument to the splashscreen's argument list.
//
// Arguments:
//   arg : The argument to be added.
//
// Note:
//   This only has an effect if called before the Create call.
//
// Programmer: Sean Ahern
// Creation:   Fri Aug 31 23:53:27 PDT 2001
//
// ****************************************************************************
void
SplashScreenProxy::AddArgument(const char *arg)
{
    if (arg == NULL)
        return;

    int     len;
    if ((len = strlen(arg)) == 0)
        return;

    // Copy the argument string.
    char   *newStr = new char[len + 1];
    strcpy(newStr, arg);

    // Grow the argv array.
    char  **newargv = new char *[argc + 1];
    for (int i = 0; i < argc; ++i)
        newargv[i] = argv[i];
    newargv[argc] = newStr;

    // Assign the new argv array to the old pointer.
    if (argc > 0)
        delete[] argv;
    argv = newargv;
    ++argc;
}

// ****************************************************************************
// Method: SplashScreenProxy::Create
//
// Purpose: 
//   Launches the splashscreen program and connects to it.
//
// Programmer: Sean Ahern
// Creation:   Thu Sep 6 16:09:28 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue Mar 26 10:01:10 PDT 2002
//   Modified the connections a little.
//
// ****************************************************************************
void
SplashScreenProxy::Create()
{
    // Create the splashscreen process.  The splash screen is executed
    // using "visit -splashscreen".
    splash = new RemoteProcess(std::string("visit"));
    splash->AddArgument(std::string("-splashscreen"));

    // Add any extra arguments to the splash screen before opening it
    for (int i = 0; i < argc; ++i)
        splash->AddArgument(std::string(argv[i]));

    // 
    // Open it. Note that we only create a write socket since we do not
    // have a need to get information back from the splashscreen.
    // 
    splash->Open("localhost", 1, 0);

    // Form the xfer object and hook it up
    xfer = new Xfer;
    xfer->SetInputConnection(splash->GetWriteConnection());
    xfer->SetOutputConnection(splash->GetReadConnection());

    xfer->Add(&atts);
    xfer->Add(&appearanceAtts);
}

// ****************************************************************************
// Method: SplashScreenProxy::Progress
//
// Purpose: 
//   Send a progress message to the splashscreen. This is displayed in a
//   progress bar.
//
// Arguments:
//   m : The message to display.
//   p : A percentage done.
//
// Programmer: Sean Ahern
// Creation:   Thu Sep 6 16:11:40 PST 2001
//
// Modifications:
//   
// ****************************************************************************
void
SplashScreenProxy::Progress(char *m, int p)
{
    atts.SetText(m);
    atts.SetProgress(p);
    atts.Notify();
}

// ****************************************************************************
// Method: SplashScreenProxy::Progress
//
// Purpose: 
//   Send a progress message to the splashscreen. This is displayed in a
//   progress bar.
//
// Arguments:
//   m : The message to display.
//   p : A percentage done.
//
// Programmer: Sean Ahern
// Creation:   Thu Sep 6 16:11:40 PST 2001
//
// Modifications:
//   
// ****************************************************************************
void
SplashScreenProxy::Progress(std::string &m, int p)
{
    atts.SetText(m);
    atts.SetProgress(p);
    atts.Notify();
}

// ****************************************************************************
// Method: SplashScreenProxy::Hide
//
// Purpose: 
//   Hides the splashscreen.
//
// Programmer: Sean Ahern
// Creation:   Thu Sep 6 16:13:22 PST 2001
//
// Modifications:
//   
// ****************************************************************************
void
SplashScreenProxy::Hide()
{
    atts.SetVisible(false);
    atts.Notify();
}

// ****************************************************************************
// Method: SplashScreenProxy::Show
//
// Purpose: 
//   Tells the splashscreen to show.
//
// Arguments:
//   AsSplashScreen : If this is true then it will show the splashscreen as
//                    it first comes up. Otherwise, it will show it as an
//                    about box.
//
// Programmer: Sean Ahern
// Creation:   Thu Sep 6 16:14:22 PST 2001
//
// Modifications:
//      Sean Ahern, Wed Sep 12 15:16:21 PDT 2001
//      Made the parameter actually take effect.
//   
// ****************************************************************************
void
SplashScreenProxy::Show(bool AsSplashScreen)
{
    atts.SetDisplayAsSplashScreen(AsSplashScreen);
    atts.SetVisible(true);
    atts.Notify();
}

// ****************************************************************************
// Method: SplashScreenProxy::About
//
// Purpose: 
//   Tells the splashscreen to show itself as an about box.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 16:16:48 PST 2001
//
// Modifications:
//   
// ****************************************************************************
void
SplashScreenProxy::About()
{
    Show(false);
}

// ****************************************************************************
// Method: SplashScreenProxy::Close
//
// Purpose: 
//   Closes down the splashscreen application.
//
// Programmer: Sean Ahern
// Creation:   Thu Sep 6 16:15:42 PST 2001
//
// Modifications:
//   
// ****************************************************************************
void
SplashScreenProxy::Close()
{
    atts.SetQuit(true);
    atts.Notify();
}

// ****************************************************************************
// Method: SplashScreenProxy::SetAppearance
//
// Purpose: 
//   Sets the splashscreen proxy's appearance attributes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 16:17:40 PST 2001
//
// Modifications:
//   
// ****************************************************************************
void
SplashScreenProxy::SetAppearance(const AppearanceAttributes &aa)
{
    appearanceAtts = aa;
    appearanceAtts.Notify();
}
