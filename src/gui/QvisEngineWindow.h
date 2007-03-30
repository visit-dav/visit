#ifndef ENGINE_WINDOW_H
#define ENGINE_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>
#include <qmap.h>

// Forward declarations.
class EngineList;
class QComboBox;
class QGroupBox;
class QLabel;
class QProgressBar;
class QPushButton;
class StatusAttributes;

// ****************************************************************************
// Class: QvisEngineWindow
//
// Purpose:
//   This class implements a window that presents the list of engines to the
//   user so the engines' statuses can be watched and engines can be killed.
//
// Notes:      This window obseves multiple subjects.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 2 16:27:54 PST 2001
//
// Modifications:
//    Jeremy Meredith, Fri Jun 29 15:12:08 PDT 2001
//    Separated the single status/progress bar into one which reports
//    total status and one which reports current stage progress.
//
//    Brad Whitlock, Wed Nov 27 13:47:30 PST 2002
//    I added widgets to show more engine information.
//
// ****************************************************************************

class GUI_API QvisEngineWindow : public QvisPostableWindowObserver
{
    Q_OBJECT

    typedef QMap<QString, StatusAttributes*> EngineStatusMap;
public:
    QvisEngineWindow(EngineList *engineList,
                     const char *caption = 0,
                     const char *shortName = 0,
                     QvisNotepadArea *notepad = 0);
    virtual ~QvisEngineWindow();
    virtual void CreateWindowContents();
    virtual void Update(Subject *TheChangedSubject);
    virtual void SubjectRemoved(Subject *TheRemovedSubject);

    void ConnectStatusAttributes(StatusAttributes *s);
private:
    void UpdateWindow(bool doAll);
    void UpdateStatusArea();
    void UpdateInformation(int index);
    void AddStatusEntry(const QString &key);
    void RemoveStatusEntry(const QString &key);
    void UpdateStatusEntry(const QString &key);
private slots:
    void closeEngine();
    void interruptEngine();
    void selectEngine(int index);
private:
    EngineList       *engines;
    StatusAttributes *statusAtts;
    Subject          *caller;
    QString           activeEngine;
    EngineStatusMap   statusMap;

    QComboBox        *engineCombo;
    QGroupBox        *engineInfo;
    QLabel           *engineNP;
    QLabel           *engineNN;
    QLabel           *engineLB;
    QLabel           *totalStatusLabel;
    QProgressBar     *totalProgressBar;
    QLabel           *stageStatusLabel;
    QProgressBar     *stageProgressBar;
    QPushButton      *interruptEngineButton;
    QPushButton      *closeEngineButton;
};

#endif
