// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SIMULATION_WINDOW_H
#define QVIS_SIMULATION_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>
#include <QMap>
#include <avtDatabaseMetaData.h>


// Forward declarations.
class QCheckBox;
class QColor;
class QComboBox;
class QGridLayout;
class QGroupBox;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QProgressBar;
class QPushButton;
class QScrollArea;
class QSpinBox;
class QString;
class QTreeWidget;

class QvisSimulationCommandWindow;
class QvisSimulationMessageWindow;
#ifdef HAVE_QWT
class QvisStripChartMgr;
#endif
class QvisUiLoader;

class EngineList;
class StatusAttributes;
class avtSimulationCommandSpecification;
class SimulationUIValues;
class SimCommandSlots;

class QwtPlot;

// ****************************************************************************
// Class: QvisSimulationWindow
//
// Purpose:
//   Simulation window.
//
// Notes:
//
// Programmer: Jeremy Meredith
// Creation:   Wed Apr  9 11:49:35 PDT 2005
//
// Modifications:
//   Brad Whitlock, Wed Apr  9 11:49:49 PDT 2008
//   QString for caption, shortName.
//
//   Brad Whitlock, Fri Nov 19 12:34:22 PST 2010
//   I made more methods private and added consts. I added some other methods
//   to help unify how we create engine keys and update information.
//
// ****************************************************************************

class GUI_API QvisSimulationWindow : public QvisPostableWindowObserver
{
    Q_OBJECT

    typedef QMap<QString, StatusAttributes*> SimulationStatusMap;
    typedef QMap<QString, avtDatabaseMetaData*> SimulationMetaDataMap;
public:
    QvisSimulationWindow(EngineList *engineList,
                     const QString &caption = QString(),
                     const QString &shortName = QString(),
                     QvisNotepadArea *notepad = 0);
    virtual ~QvisSimulationWindow();
    virtual void CreateWindowContents();
    virtual void Update(Subject *TheChangedSubject);
    virtual void SubjectRemoved(Subject *TheRemovedSubject);

    void ConnectStatusAttributes(StatusAttributes *s);
    void ConnectSimulationUIValues(SimulationUIValues *s);

    void SetNewMetaData(const QualifiedFilename &qf,
                        const avtDatabaseMetaData *md);
private:
    QString MakeKey(const std::string &host, const std::string &sim) const;
    int GetEngineListIndex(const QString &key) const;

    void UpdateWindow(bool doAll);
    void UpdateStatusArea();
    void UpdateInformation();

    void AddStatusEntry(const QString &key, const StatusAttributes &);
    void RemoveStatusEntry(const QString &key);
    void UpdateStatusEntry(const QString &key, const StatusAttributes &);

    void AddMetaDataEntry(const QString &key, const avtDatabaseMetaData &);
    void RemoveMetaDataEntry(const QString &key);
    void UpdateMetaDataEntry(const QString &key, const avtDatabaseMetaData &);

    QString GetUIFileDirectory() const;
    QString GetUIFile(const QString &key) const;

    void ConnectUIChildren(QObject *obj);
    void ConnectUI(QObject *ui);

    void CreateCustomUIWindow();
    void UpdateUIComponent(QWidget *window,
                           const QString &name, const QString &value, bool e);

    void parseCompositeCMD( const std::string cmd, std::string &ctrl,
                            std::string &text );
    void parseCompositeCMD( const std::string cmd, unsigned int &index,
                            std::string &text );
    void parseCompositeCMD( const std::string cmd,
                            unsigned int &row, unsigned int &column,
                            std::string &text );
    void parseCompositeCMD( const std::string cmd,
                            unsigned int &row, std::string &text,
                            unsigned int &editable );
    void parseCompositeCMD( const std::string cmd,
                            unsigned int &row, unsigned int &column,
                            double &x, double &y );
    void parseCompositeCMD( const std::string cmd,
                            unsigned int &chart,
                            unsigned int &curve,
                            unsigned int &npts,
                            double * &x, double * &y );
    std::string getNextString( std::string &cmd, const std::string delimiter );

private slots:
    void closeEngine();
    void interruptEngine();
    void selectEngine(int index);
    void clearMessages();
#ifdef HAVE_QWT
    void clearStripCharts();
#endif
    void clearCache();
    void showCustomUIWindow();
    void executePushButtonCommand(const QString &cmd);
    void executeEnableTimeRange(const QString &value);
    void executeStartCommand(const QString &value);
    void executeStopCommand(const QString &value);
    void executeStepCommand(const QString &value);

public:
#ifdef HAVE_QWT
    void setStripChartVar(const QString &value);
#endif

private:
    EngineList           *engines;
    StatusAttributes     *statusAtts;
    avtDatabaseMetaData  *metadata;
    SimulationUIValues   *uiValues;
    Subject              *caller;
    QString               activeEngine;

    SimulationStatusMap   statusMap;
    SimulationMetaDataMap metadataMap;

    QComboBox          *simCombo;
    QTreeWidget        *simInfo;
    QLabel             *simulationMode;
    QProgressBar       *totalProgressBar;
    QPushButton        *interruptEngineButton;
    QPushButton        *closeEngineButton;
    QPushButton        *clearCacheButton;
    QWidget            *CustomUIWindow;
    QvisUiLoader       *uiLoader;
#ifdef HAVE_QWT
    QvisStripChartMgr  *stripChartMgr;
#endif
    QvisSimulationCommandWindow  *simCommands;
    QvisSimulationMessageWindow  *simMessages;

    SimCommandSlots *CommandConnections;
};
#endif
