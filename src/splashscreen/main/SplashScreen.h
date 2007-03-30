#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <qframe.h>
#include <SimpleObserver.h>
#include <vector>

// Forward declarations.
class QLabel;
class QPushButton;
class QProgressBar;
class SplashScreenAttributes;
class AppearanceAttributes;
class QPainter;
class QTimer;

// ****************************************************************************
// Class: SplashScreen
//
// Purpose:
//   This is class creates a splashscreen.
//
// Notes:      
//
// Programmer: Sean Ahern
// Creation:   Thu Sep 6 16:34:45 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Sep 6 16:35:18 PST 2001
//   Modified the widget so it observes two subjects.
//
// ****************************************************************************

class SplashScreen : public QFrame, public SimpleObserver
{
    Q_OBJECT
public:
    SplashScreen(const char *name = NULL, bool allowShow=true);
    ~SplashScreen();

    virtual void Update(Subject *);
    virtual void SubjectRemoved(Subject *);

    void ConnectSplashScreenAtts(SplashScreenAttributes *atts);
    void ConnectAppearanceAtts(AppearanceAttributes *atts);
public slots:
    void hideSelf();
    void nextPicture();
protected:
    void DropShadowText(QPainter*, int, int, QString);
    void SetDisplayMode(bool);
    void CustomizeAppearance();
protected:
    QLabel                 *pictureLabel;
    std::vector<QPixmap>   pictures;
    int                    curPicture;
    bool                   splashMode;
    QTimer                 *timer;
    QLabel                 *text;
    QProgressBar           *progress;
    QPushButton            *dismissButton;
    bool                   allowShow;

    SplashScreenAttributes *splashAtts;
    AppearanceAttributes   *appearanceAtts;
};

#endif
