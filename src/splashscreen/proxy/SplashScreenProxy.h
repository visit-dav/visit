#ifndef SPLASHSCREENPROXY_H
#define SPLASHSCREENPROXY_H
#include <splashscreen_proxy_exports.h>
#include <SplashScreenAttributes.h>
#include <AppearanceAttributes.h>

// Forward declarations.
class RemoteProcess;
class Xfer;

// ****************************************************************************
// Class: SplashScreenProxy
//
// Purpose:
//   This class allows client programs to control the splash screen.
//
// Notes:      
//
// Programmer: Sean Ahern
// Creation:   Thu Sep 6 16:03:40 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Sep 6 16:04:08 PST 2001
//   Added to the interface so we can set the appearance attributes. I
//   also made the class *not* inherit from SimpleObserver.
//
//   Brad Whitlock, Mon Apr 15 11:46:59 PDT 2002
//   I added methods to set the raise behavior of the splash screen.
//
//   Sean Ahern, Tue Apr 16 15:31:09 PDT 2002
//   I removed the ability of the splash screen to raise itself.
//
// ****************************************************************************
class SPLASH_PROXY_API SplashScreenProxy
{
public:
    SplashScreenProxy();
    virtual ~SplashScreenProxy();
    void AddArgument(const char *arg);

    void Progress(char *, int p);
    void Progress(std::string &, int p);
    void Create();
    void Close();
    void Hide();
    void Show(bool AsSplashScreen = true);
    void About();
    void SetAppearance(const AppearanceAttributes &aa);

private:
    SplashScreenAttributes atts;
    AppearanceAttributes   appearanceAtts;
    RemoteProcess *splash;
    Xfer          *xfer;
    
    // Extra arguments to pass to the splash screen
    int            argc;
    char           **argv;
};

#endif
