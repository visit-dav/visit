// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISANIMATIONWINDOW_H
#define QVISANIMATIONWINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

// Forward declarations.
class AnimationAttributes;
class QButtonGroup;
class QCheckBox;
class QSlider;
class QSpinBox;

// ****************************************************************************
// Class: QvisAnimationWindow
//
// Purpose: 
//   This class creates a window that has animation controls.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov 19 13:47:37 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue May 14 11:31:55 PDT 2002
//   Added controls for the animation playback speed.
//
//   Brad Whitlock, Mon Oct 6 16:19:59 PST 2003
//   Added a controls to let the users set the animation style.
//
//   Brad Whitlock, Wed Apr  9 10:59:35 PDT 2008
//   QString for caption, shortName.
//
//   Brad Whitlock, Wed Dec 10 16:29:45 PST 2008
//   I added a spinbox for animation frame increment.
//
// ****************************************************************************

class GUI_API QvisAnimationWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisAnimationWindow(AnimationAttributes *subj,
                        const QString &caption = QString(),
                        const QString &shortName = QString(),
                        QvisNotepadArea *notepad = 0);
    virtual ~QvisAnimationWindow();
    virtual void CreateWindowContents();
protected:
    void UpdateWindow(bool doAll);
    void Apply(bool ignore = false);
private slots:
    virtual void apply();
    void pipelineCachingToggled(bool val);
    void playbackModeChanged(int val);
    void timeoutChanged(int);
    void incrementChanged(int);
private:
    AnimationAttributes *animationAtts;

    QCheckBox    *pipelineCachingToggle;
    QSlider      *timeoutSlider;
    QButtonGroup *playbackModeButtonGroup;
    QSpinBox     *animationIncrement;
};

#endif
