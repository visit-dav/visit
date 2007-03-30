#ifndef SPLASHSCREENAPPLICATION_H
#define SPLASHSCREENAPPLICATION_H
#include <qobject.h>

#include <AppearanceAttributes.h>
#include <SplashScreenAttributes.h>
#include <Xfer.h>
#include <ParentProcess.h>

// Forward declarations
class SplashScreen;
class QApplication;

// ****************************************************************************
// Class: SplashScreenApplication
//
// Purpose:
//   This class is the main application class for the splash screen program.
//
// Notes:      
//
// Programmer: Sean Ahern
// Creation:   Fri Aug 31 09:04:46 PDT 2001
//
// Modifications:
//   Brad Whitlock, Thu Sep 6 16:29:29 PST 2001
//   Added appearance attributes.
//
// ****************************************************************************

class SplashScreenApplication : public QObject
{
    Q_OBJECT
public:
    SplashScreenApplication(int &argc, char **argv);
    void Go();

public slots:
    void ReadFromGUI(int);

private:
    ParentProcess          theGUI;
    Xfer                   xfer;
    SplashScreenAttributes atts;
    AppearanceAttributes   appearanceAtts;
    SplashScreen           *splash;
    QApplication           *app;
};

#endif
