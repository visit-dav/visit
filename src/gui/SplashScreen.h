#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <qframe.h>
#include <vector>

// Forward declarations.
class QLabel;
class QPushButton;
class QProgressBar;
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
//   Brad Whitlock, Wed Jun 18 17:53:09 PST 2003
//   I changed this class so it is a simple widget.
//
// ****************************************************************************

class SplashScreen : public QFrame
{
    Q_OBJECT
public:
    SplashScreen(bool cyclePictures = false, const char *name = 0);
    ~SplashScreen();

    void Progress(const char *msg, int progress);
    void SetDisplayAsSplashScreen(bool mode);
    void About();
public slots:
    virtual void show();
    virtual void hide();
    void nextPicture();
protected:
    QLabel                 *pictureLabel;
    std::vector<QPixmap>   pictures;
    int                    curPicture;
    bool                   splashMode;
    QTimer                 *timer;
    QLabel                 *text;
    QProgressBar           *progress;
    QPushButton            *dismissButton;
};

#endif
