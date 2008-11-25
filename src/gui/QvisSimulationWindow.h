/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
class QvisStripChartMgr;
class QvisUiLoader;

class EngineList;
class StatusAttributes;
class avtSimulationCommandSpecification;

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
// ****************************************************************************

class GUI_API QvisSimulationWindow : public QvisPostableWindowObserver
{
    Q_OBJECT

    typedef QMap<QString, StatusAttributes*> SimulationStatusMap;
    typedef QMap<QString, avtDatabaseMetaData*> SimulationMetaDataMap;
public:
    QvisSimulationWindow(EngineList *engineList,    
                     const QString &caption = QString::null,
                     const QString &shortName = QString::null,
                     QvisNotepadArea *notepad = 0);
    virtual ~QvisSimulationWindow();
    virtual void CreateWindowContents();
    virtual void Update(Subject *TheChangedSubject);
    virtual void SubjectRemoved(Subject *TheRemovedSubject);
    
    void UpdateUIComponent(QWidget *window,avtSimulationCommandSpecification *cmd);
    void ConnectStatusAttributes(StatusAttributes *s);
    void SetNewMetaData(const QualifiedFilename &qf,
                        const avtDatabaseMetaData *md);
    void SpecialWidgetUpdate(avtSimulationCommandSpecification *cmd);
    //void setMinMaxStripChartDataDisplay (double minY, double maxY);
private:
    void UpdateWindow(bool doAll);
    void UpdateCustomUI(avtDatabaseMetaData *md);
    void UpdateSimulationUI (avtDatabaseMetaData *md);
    void UpdateStatusArea();
    void UpdateInformation(int index);
    void UpdateInformation(const QString &key);
    void AddStatusEntry(const QString &key);
    void RemoveStatusEntry(const QString &key);
    void UpdateStatusEntry(const QString &key);
    void AddMetaDataEntry(const QString &key);
    void RemoveMetaDataEntry(const QString &key);
    void UpdateMetaDataEntry(const QString &key);
    void CreateCommandUI();
    QString GetUIFileDirectory() const;
    QString GetUIFile() const;
    void ViewerSendCMD ( int simIndex, QString cmd);
    QColor getColor(const QString &color) const;
private slots:
    void closeEngine();
    void interruptEngine();
    void selectEngine(int index);
    void clearCache();
    void showCommandWindow();
    void executePushButtonCommand(const QString &cmd);
    void executeEnableTimeRange(const QString &cmd);
    void executeStartCommand(const QString &cmd);
    void executeStopCommand(const QString &cmd);
    void executeStepCommand(const QString &cmd);
    void zoomOut();
    void zoomIn();
    void focus();
    
private:
    EngineList           *engines;
    StatusAttributes     *statusAtts;
    avtDatabaseMetaData  *metadata;
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
    QWidget            *DynamicCommandsWin;
    QvisUiLoader       *uiLoader;
    QMap<int,int>      simulationToEngineListMap;
    QvisStripChartMgr  *stripCharts;
    QvisSimulationMessageWindow *simMessages;
    QvisSimulationCommandWindow *simCommands;
};

#endif
