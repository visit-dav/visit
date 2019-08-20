// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QFrame>
#include <vector>

// Forward declarations.
class QLabel;
class QPushButton;
class QProgressBar;
class QPainter;
class QTimer;
class QVBoxLayout;

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
//   Brad Whitlock, Tue Jan  8 13:43:02 PST 2008
//   Added signals for showing copyright and contributors.
//
//   Brad Whitlock, Wed Apr  9 10:26:06 PDT 2008
//   Use QString instead of const char *.
//
//   Brad Whitlock, Fri May 30 15:21:37 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class SplashScreen : public QFrame
{
    Q_OBJECT
public:
    SplashScreen(bool cyclePictures = false);
    ~SplashScreen();

    void Progress(const QString &msg, int progress);
    void SetDisplayAsSplashScreen(bool mode);
    void About();
signals:
    void showCopyright();
    void showContributors();
public slots:
    virtual void show();
    virtual void hide();
    void nextPicture();
private slots:
    void emitShowCopyright();
    void emitShowContributors();
protected:
    void CreateAboutButtons();

    QLabel                 *pictureLabel;
    std::vector<QPixmap>   pictures;
    int                    curPicture;
    bool                   splashMode;
    QTimer                 *timer;
    QLabel                 *text;
    QProgressBar           *progress;
    QPushButton            *dismissButton;
    QPushButton            *copyrightButton;
    QPushButton            *contributorButton;
    QVBoxLayout            *topLayout;
    QVBoxLayout            *lLayout;
    QVBoxLayout            *rLayout;
};

#endif
