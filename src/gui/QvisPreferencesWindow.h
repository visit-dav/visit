#ifndef QVIS_PREFERENCES_WINDOW_H
#define QVIS_PREFERENCES_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>
#include <TimeFormat.h>

class GlobalAttributes;
class QButtonGroup;
class QCheckBox;
class QSpinBox;

// ****************************************************************************
// Class: QvisPreferencesWindow
//
// Purpose: 
//   Defines QvisPreferencesWindow class.
//
// Programmer: Eric Brugger
// Creation:   Thu Mar 13 11:13:18 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Sep 5 15:41:31 PST 2003
//   Added a toggle for post windows when shown.
//
//   Brad Whitlock, Mon Oct 13 16:52:39 PST 2003
//   Added a toggle for timeState display mode.
//
// ****************************************************************************

class GUI_API QvisPreferencesWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisPreferencesWindow(GlobalAttributes *subj,
                          const char *caption = 0,
                          const char *shortName = 0,
                          QvisNotepadArea *notepad = 0);
    virtual ~QvisPreferencesWindow();
    virtual void CreateWindowContents();

    void SetTimeStateFormat(const TimeFormat &fmt);
signals:
    void changeTimeFormat(const TimeFormat &);
public slots:
    virtual void apply();
protected:
    void UpdateWindow(bool doAll);
    void Apply(bool ignore = false);
private slots:
    void cloneWindowOnFirstRefToggled(bool val);
    void postWindowsWhenShownToggled(bool val);
    void handleTimeStateDisplayModeChange(int val);
    void timeStateNDigitsChanged(int val);
private:
    QCheckBox        *cloneWindowOnFirstRefToggle;
    QCheckBox        *postWindowsWhenShownToggle;
    QButtonGroup     *timeStateDisplayMode;
    QSpinBox         *timeStateNDigits;

    GlobalAttributes *atts;

    TimeFormat        tsFormat;
};

#endif
