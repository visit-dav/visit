#ifndef QVIS_RECENT_PATH_REMOVAL_WINDOW_H
#define QVIS_RECENT_PATH_REMOVAL_WINDOW_H
#include <gui_exports.h>
#include <QvisDelayedWindowObserver.h>
#include <QualifiedFilename.h>

// Forward declarations
class QGroupBox;
class QListBox;
class QPushButton;

// ****************************************************************************
// Class: QvisRecentPathRemovalWindow
//
// Purpose:
//   Removes paths from the recently visited path list in the file selection
//   window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 10 14:57:48 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class GUI_API QvisRecentPathRemovalWindow : public QvisDelayedWindowObserver
{
    Q_OBJECT
public:
    QvisRecentPathRemovalWindow(Subject *s, const char *captionString);
    virtual ~QvisRecentPathRemovalWindow();
public slots:
    virtual void show();
protected:
    virtual void CreateWindowContents();
    virtual void UpdateWindow(bool doAll);
    void UpdateWidgets();
private slots:
    void removePaths();
    void removeAllPaths();
    void invertSelection();
    void applyDismiss();
    void handleCancel();
private:
    QGroupBox   *removalControlsGroup;
    QListBox    *removalListBox;
    QPushButton *removeButton;
    QPushButton *removeAllButton;
    QPushButton *invertSelectionButton;

    QPushButton *okButton;
    QPushButton *cancelButton;

    QualifiedFilenameVector paths;
};

#endif
