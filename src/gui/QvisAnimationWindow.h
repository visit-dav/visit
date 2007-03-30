#ifndef QVISANIMATIONWINDOW_H
#define QVISANIMATIONWINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

// Forward declarations.
class AnimationAttributes;
class QCheckBox;
class QSlider;

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
// ****************************************************************************

class GUI_API QvisAnimationWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisAnimationWindow(AnimationAttributes *subj,
                        const char *caption = 0,
                        const char *shortName = 0,
                        QvisNotepadArea *notepad = 0);
    virtual ~QvisAnimationWindow();
    virtual void CreateWindowContents();
protected:
    void UpdateWindow(bool doAll);
    void Apply(bool ignore = false);
private slots:
    virtual void apply();
    void pipelineCachingToggled(bool val);
    void timeoutChanged(int);
private:
    AnimationAttributes *animationAtts;

    QCheckBox *pipelineCachingToggle;
    QSlider   *timeoutSlider;
};

#endif
