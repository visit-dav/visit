#ifndef QVIS_POSTABLE_WINDOW_OBSERVER_H
#define QVIS_POSTABLE_WINDOW_OBSERVER_H
#include <gui_exports.h>
#include <QvisPostableWindow.h>
#include <Observer.h>

// ****************************************************************************
// Class: QvisPostableWindowObserver
//
// Purpose:
//   This is the base class for postable windows that observe state
//   objects and update themselves when the state objects change.
//   This class also has an Apply button in addition to Post and
//   Dismiss buttons.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 28 17:06:56 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Feb 12 13:44:14 PST 2001
//   Added the "extra_" argument to the contructor.
//
//   Brad Whitlock, Wed May 2 11:32:41 PDT 2001
//   Replaced extra argument with buttonCombination.
//
//   Brad Whitlock, Thu Aug 23 8:31:23 PDT 2001
//   Added a private slot function to help with make default.
//
//   Brad Whitlock, Fri Feb 15 11:17:34 PDT 2002
//   Changed the protection on some of the methods.
//
// ****************************************************************************

class GUI_API QvisPostableWindowObserver : public QvisPostableWindow, public Observer
{
    Q_OBJECT
public:
    static const int NoExtraButtons;
    static const int ApplyButton;
    static const int MakeDefaultButton;
    static const int ResetButton;
    static const int AllExtraButtons;

    QvisPostableWindowObserver(Subject *subj,
                               const char *caption = 0,
                               const char *shortName = 0,
                               QvisNotepadArea *notepad = 0,
                               int buttonCombo = AllExtraButtons,
                               bool stretch = true);
    virtual ~QvisPostableWindowObserver();
    virtual void Update(Subject *TheChangedSubject);
    virtual void CreateEntireWindow();

public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
protected:
    virtual void CreateWindowContents() = 0;
private slots:
    void makeDefaultHelper();
private:
    int          buttonCombination;
    bool         stretchWindow;
};

#endif
