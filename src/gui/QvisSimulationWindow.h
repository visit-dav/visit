#ifndef QVIS_SIMULATION_WINDOW_H
#define QVIS_SIMULATION_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>
#include <qmap.h>
#include <avtDatabaseMetaData.h>

// Forward declarations.
class EngineList;
class QComboBox;
class QGroupBox;
class QLabel;
class QProgressBar;
class QPushButton;
class StatusAttributes;
class QLineEdit;
class QListView;
class QString;

class avtSimulationCommandSpecification;

// ****************************************************************************
// Class: QvisSimulationWindow
//
// Purpose:
//   This class implements a window that presents the list of engines to the
//   user so the engines' statuses can be watched and engines can be killed.
//
// Notes:      This window obseves multiple subjects.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//    Jeremy Meredith, Thu Apr 28 18:03:57 PDT 2005
//    SetNewMetaData needed more info to construct the key correctly.
//    Added simulation mode.
//
//    Shelly Prevost, Tue Jan 24 12:29:29 PST 2006
//    Added methods to allow the addition of a custom simulation control
//    window.
//
// ****************************************************************************

class GUI_API QvisSimulationWindow : public QvisPostableWindowObserver
{
    Q_OBJECT

    typedef QMap<QString, StatusAttributes*> SimulationStatusMap;
    typedef QMap<QString, avtDatabaseMetaData*> SimulationMetaDataMap;
public:
    QvisSimulationWindow(EngineList *engineList,
                     const char *caption = 0,
                     const char *shortName = 0,
                     QvisNotepadArea *notepad = 0);
    virtual ~QvisSimulationWindow();
    virtual void CreateWindowContents();
    virtual void CreateCommandUI();
    virtual void Update(Subject *TheChangedSubject);
    virtual void SubjectRemoved(Subject *TheRemovedSubject);
    
    void UpdateCustomUIComponent(avtSimulationCommandSpecification *cmd);
    void ConnectStatusAttributes(StatusAttributes *s);
    void SetNewMetaData(const QualifiedFilename &qf,
                        const avtDatabaseMetaData *md);
private:
    void UpdateWindow(bool doAll);
    void UpdateCustomUI(avtDatabaseMetaData *md);
    void UpdateStatusArea();
    void UpdateInformation(int index);
    void UpdateInformation(const QString &key);
    void AddStatusEntry(const QString &key);
    void RemoveStatusEntry(const QString &key);
    void UpdateStatusEntry(const QString &key);
    void AddMetaDataEntry(const QString &key);
    void RemoveMetaDataEntry(const QString &key);
    void UpdateMetaDataEntry(const QString &key);
private slots:
    void closeEngine();
    void interruptEngine();
    void selectEngine(int index);
    void clearCache();
    void showCommandWindow();
    void executeSimCommand();
    void executePushButtonCommand(int);
    void executePushButtonCommand0();
    void executePushButtonCommand1();
    void executePushButtonCommand2();
    void executePushButtonCommand3();
    void executePushButtonCommand4();
    void executePushButtonCommand5();
    void executePushButtonCommand6();
    void executePushButtonCommand7();
    void executePushButtonCommand8();

private:
    EngineList           *engines;
    StatusAttributes     *statusAtts;
    avtDatabaseMetaData  *metadata;
    Subject              *caller;
    QString               activeEngine;

    SimulationStatusMap   statusMap;
    SimulationMetaDataMap metadataMap;

    QComboBox        *simCombo;
    QListView        *simInfo;
    QLabel           *simulationMode;
    QProgressBar     *totalProgressBar;
    QPushButton      *interruptEngineButton;
    QPushButton      *closeEngineButton;
    QPushButton      *clearCacheButton;
    QLineEdit        *simCommandEdit;
    QWidget          *DynamicCommandsWin;
    QPushButton      *cmdButtons[9];
    QMap<int,int>     simulationToEngineListMap;
};

#endif
