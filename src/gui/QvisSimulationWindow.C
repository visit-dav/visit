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

#include <QvisSimulationWindow.h>

#include <time.h>
#include <string>

#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QDial>
#include <QFile>
#include <QGroupBox>
#include <QLCDNumber>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QMetaObject>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton> 
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QSplitter>
#include <QTextEdit>
#include <QTreeWidget>

#include <DebugStream.h>
#include <EngineList.h>
#include <StatusAttributes.h>
#include <ViewerProxy.h>
#include <avtSimulationInformation.h>
#include <avtSimulationCommandSpecification.h>
#include <QualifiedFilename.h>

#include <QvisSimulationCommandWindow.h>
#include <QvisSimulationMessageWindow.h>
#include <QvisStripChart.h>
#include <QvisStripChartMgr.h>
#include <QvisNotepadArea.h>
#include <QvisUiLoader.h>
#include <SimCommandSlots.h>
#include <SimWidgetNames.h>

using std::string;
using std::vector;

#define CUSTOM_BUTTON 5
#define NUM_GENERIC_BUTTONS 6

// ****************************************************************************
// Method: QvisSimulationWindow::QvisSimulationWindow
//
// Purpose: 
//   This is the constructor for the QvisSimulationWindow class.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//   Shelly Prevost, Tue Jan 24 17:06:49 PST 2006
//   Added a custom simulation control window.
//
//   Brad Whitlock, Wed Apr  9 11:50:52 PDT 2008
//   QString for caption, shortName.
//
//   Brad Whitlock, Mon Jul  7 13:19:10 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisSimulationWindow::QvisSimulationWindow(EngineList *engineList,
    const QString &caption, const QString &shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(engineList, caption, shortName, notepad,
                               QvisPostableWindowObserver::NoExtraButtons),
    activeEngine(""), statusMap()
{
    engines = engineList;
    caller = engines;
    statusAtts = 0;
    metadata = new avtDatabaseMetaData;
    DynamicCommandsWin = NULL;
    uiLoader = new QvisUiLoader;
    stripCharts = 0;
    simMessages = 0;
}

// ****************************************************************************
// Method: QvisSimulationWindow::~QvisSimulationWindow
//
// Purpose: 
//   This is the destructor for the QvisSimulationWindow class.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//   Brad Whitlock, Mon Jul  7 10:55:12 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisSimulationWindow::~QvisSimulationWindow()
{
    // Delete the status attributes in the status map.
    SimulationStatusMap::Iterator pos;
    for (pos = statusMap.begin(); pos != statusMap.end(); ++pos)
    {
        delete pos.value();
    }

    // Detach from the status atts if they are still around.
    if (statusAtts)
        statusAtts->Detach(this);

    delete stripCharts;
}
  
// ****************************************************************************
// Method: QvisSimulationWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//   Shelly Prevost, Tue Jan 24 17:06:49 PST 2006
//   Added a custom simulation control window.
//
//   Brad Whitlock, Tue Jan 31 16:04:07 PST 2006
//   I moved the generic button creation code back to this method.
//
//   Shelly Prevost, Tue Sep 12 14:53:40 PDT 2006
//   I added a new set of time range text boxes and activation checkbox.
//   I Added an message box to display simulation code messages.
//
//   Shelly Prevost Tue Nov 28 17:01:58 PST 2006
//   I added the strip chart widget to the main simulatioin window.
//
//   Shelly Prevost Fri Apr 13 14:03:03 PDT 2007
//   added splitter to help with widow space issues.
//
//   Shelly Prevost Fri Oct 12 15:19:40 PDT 2007
//   modified splitter to help with widow space issues.
//   Factored out strip chart window widgets and replaced them
//   with a strip chart window manager. 
//
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Mon Jul  7 10:56:03 PDT 2008
//   Qt 4. Reorganized window.
//
// ****************************************************************************

void
QvisSimulationWindow::CreateWindowContents()
{
    isCreated = true;

    QGridLayout *grid1 = new QGridLayout(0);
    topLayout->addLayout(grid1);
    grid1->setColumnStretch(1, 10);

    simCombo = new QComboBox(central);
    connect(simCombo, SIGNAL(activated(int)), this, SLOT(selectEngine(int)));
    grid1->addWidget(simCombo, 0, 1);
    QLabel *engineLabel = new QLabel(tr("Simulation"), central);
    engineLabel->setBuddy(simCombo);
    grid1->addWidget(engineLabel, 0, 0);

    // Create the widgets needed to show the engine information.
    simInfo = new QTreeWidget(central);
    simInfo->setColumnCount(2);
    simInfo->setHeaderLabels(QStringList(tr("Attribute")) + 
                             QStringList(tr("Value")));
    simInfo->setAllColumnsShowFocus(true);
    simInfo->setAlternatingRowColors(true);
//    simInfo->setResizeMode(QTreeView::AllColumns);
    grid1->addWidget(simInfo, 1, 0, 1, 2);

    // Create status displays
    QGridLayout *statusLayout = new QGridLayout(0);
    topLayout->addLayout(statusLayout);
    simulationMode = new QLabel(tr("Not connected"), central);
    statusLayout->addWidget(new QLabel(tr("Simulation status"), central), 0, 0);  
    statusLayout->addWidget(simulationMode, 0, 1, 1, 3);

    totalProgressBar = new QProgressBar(central);
    totalProgressBar->setMaximum(100);
    statusLayout->addWidget(new QLabel(tr("VisIt status"), central), 1, 0);
    statusLayout->addWidget(totalProgressBar, 1, 1, 1, 3);

    // Create engine buttons for interruption, etc.
    interruptEngineButton = new QPushButton(tr("Interrupt"), central);
    connect(interruptEngineButton, SIGNAL(clicked()), this, SLOT(interruptEngine()));
    interruptEngineButton->setEnabled(false);
    statusLayout->addWidget(interruptEngineButton, 2, 1);

    closeEngineButton = new QPushButton(tr("Disconnect"), central);
    connect(closeEngineButton, SIGNAL(clicked()), this, SLOT(closeEngine()));
    closeEngineButton->setEnabled(false);
    statusLayout->addWidget(closeEngineButton, 2, 3);

    clearCacheButton = new QPushButton(tr("Clear cache"), central);
    connect(clearCacheButton, SIGNAL(clicked()), this, SLOT(clearCache()));
    clearCacheButton->setEnabled(false);
    statusLayout->addWidget(clearCacheButton, 2, 2);

    // Create the notepad area.
    QvisNotepadArea *notepadAux = new QvisNotepadArea(central);
    topLayout->addWidget(notepadAux);

    // Create the command window and post it to the notepad.
    simCommands = new QvisSimulationCommandWindow(tr("Simulation controls"), 
        tr("Controls"), notepadAux);
    simCommands->post();
    connect(simCommands, SIGNAL(executeButtonCommand(const QString &)),
            this, SLOT(executePushButtonCommand(const QString &)));
    connect(simCommands, SIGNAL(showCommandWindow()),
            this, SLOT(showCommandWindow()));
    connect(simCommands, SIGNAL(executeStart(const QString &)),
            this, SLOT(executeStartCommand(const QString &)));
    connect(simCommands, SIGNAL(executeStop(const QString &)),
            this, SLOT(executeStopCommand(const QString &)));
    connect(simCommands, SIGNAL(executeStep(const QString &)),
            this, SLOT(executeStepCommand(const QString &)));
    connect(simCommands, SIGNAL(timeRangingToggled(const QString &)),
            this, SLOT(executeEnableTimeRange(const QString &)));

    // Create the simulation message window and post it to the notepad.
    simMessages = new QvisSimulationMessageWindow(tr("Simulation messages"),
        tr("Messages"), notepadAux);
    simMessages->post();

    // Create the strip chart manager and post it to the notepad.
    int simindex = simCombo->currentIndex();
    int index = simulationToEngineListMap[simindex];
    stripCharts = new QvisStripChartMgr(0, GetViewerProxy(), engines, index, notepadAux);
    stripCharts->post();
}

// ****************************************************************************
// Method: QvisSimulationWindow::GetUIFileDirectory
//
// Purpose: 
//   Returns the name of the directory where VisIt looks for UI files.
//
// Returns:    The directory where VisIt looks for UI files.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//   Brad Whitlock, Tue Jan 31 16:40:02 PST 2006
//   Refactored code from elsewhere into this method.
//
//   Shelly Prevost, Mon Mar 13 14:49:07 PST 2006
//   Changed the default directory to check for ui files from UI to ui.
//
// ****************************************************************************

QString
QvisSimulationWindow::GetUIFileDirectory() const
{
    // First look in user defined variable
    QString dirName(getenv("VISITSIMDIR"));
    // if still not defined then look in the users home directory
    if (dirName.isEmpty())
    {
        dirName = getenv("HOME");
        if (!dirName.isEmpty()) dirName += "/.visit/ui/";
    }
    // if not defined then look in global location
    if ( dirName.isEmpty())
    {
        dirName = getenv("VISITDIR");
        if (!dirName.isEmpty()) dirName += "/ui/";
    }

    return dirName;
}

// ****************************************************************************
// Method: QvisSimulationWindow::GetUIfile
//
// Purpose: 
//   Returns the name of the directory where VisIt looks for UI files.
//
// Returns:    The directory name of the UI file that is appropriate for the
//             currently selected simulation, or an empty string if there
//             is no simulation or if it does not have a user interface.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//   Brad Whitlock, Tue Jan 31 16:40:02 PST 2006
//   Refactored code from elsewhere into this method.
//
// ****************************************************************************

QString
QvisSimulationWindow::GetUIFile() const
{
    QString retval;

    // Check for a valid engine.
    const stringVector &s = engines->GetEngines();
    int simindex = simCombo->currentIndex();
    int index = simulationToEngineListMap[simindex];
    if (index != -1 && s.size() >= 1)
    {
        // We have a "valid" engine in the list so try and get its
        // metadata so we can get the name of the UI file.
        QString key; 
        key.sprintf("%s:%s",
                    engines->GetEngines()[index].c_str(),
                    engines->GetSimulationName()[index].c_str());
        SimulationMetaDataMap::ConstIterator pos = metadataMap.find(key);
        if(pos != metadataMap.end())
        {
            // get ui filename from value array
            avtDatabaseMetaData *md = metadataMap[key];
            QString uiFilename;
            const stringVector &names  = md->GetSimInfo().GetOtherNames();
            const stringVector &values = md->GetSimInfo().GetOtherValues();
                  
            for (int i=0; i<names.size(); i++)
            {
                if (names[i] == "uiFile")
                    uiFilename = QString(values[i].c_str());
            }

            if (!uiFilename.isEmpty())
            {
                retval = GetUIFileDirectory() + uiFilename;
            }
        }
    }

    return retval;
}

// ****************************************************************************
// Method: void QvisSimulationWindow::CreateCommandUI
//
// Purpose:
//   Updates the ui components in the Costom UI popup. It check for matches
//   between ui updates sent from the simulations to ui components in the
//   custom ui popup. If it finds a match it update the ui component.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//   Brad Whitlock, Tue Jan 31 16:33:45 PST 2006
//   I renamed the method and moved a bunch of its code into helper methods.
//
//   Shelly Prevost Tue Jun 19 16:10:17 PDT 2007
//   Added signal connection for row col table widget.
//
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Mon Jul  7 13:17:33 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSimulationWindow::CreateCommandUI()
{
    // Try and get the name of the UI file.
    QString fname(GetUIFile());
    if(fname.isEmpty())
    {
        if(DynamicCommandsWin != NULL)
            delete DynamicCommandsWin;
        DynamicCommandsWin = NULL;
        simCommands->setButtonEnabled(CUSTOM_BUTTON, false);
        return;
    }
    
    debug5 << "UI_DIR = " << fname.toStdString() << endl;

    // Dynamically create the custom UI be reading in it's xml file.
    int simindex = simCombo->currentIndex();
    int index = simulationToEngineListMap[simindex];
    SimCommandSlots *CommandConnections = new SimCommandSlots(GetViewerProxy(),
        engines, index);
    QFile uiFile(fname);
    if(uiFile.open(QIODevice::ReadOnly))
        DynamicCommandsWin = uiLoader->load(&uiFile, this);
    
    // If creation failed then jump out 
    if (DynamicCommandsWin == NULL)
    {
        simCommands->setButtonEnabled(CUSTOM_BUTTON, false);
        QString msg = tr("VisIt could not locate the simulation's "
                         "user interface creation file at: %1. The custom user "
                         "interface for this simulation will be unavailable.").
                      arg(fname);
        Error(msg);
        return;
    }
     
    const QObjectList &GUI_Objects = DynamicCommandsWin->children();

    // Connect up handlers to all signals based on component type.
    for (int i = 0; i < GUI_Objects.size(); ++i)
    {
        QObject *ui = GUI_Objects[i];
        const QMetaObject *mo = ui->metaObject();       
        if (mo->indexOfSignal("clicked()") != -1)
            connect(ui, SIGNAL(clicked()), CommandConnections,
                   SLOT(ClickedHandler()));
        if (mo->indexOfSignal("valueChanged(int)") != -1)
            connect(ui, SIGNAL(valueChanged(int)), CommandConnections,
                    SLOT(ValueChangedHandler(int)));
        if (mo->indexOfSignal("valueChanged(const QDate&)") != -1)
            connect(ui, SIGNAL(valueChanged(const QDate&)), CommandConnections,
                    SLOT(ValueChangedHandler(const QDate &)));
        if (mo->indexOfSignal("valueChanged(const QTime&)") != -1)
            connect(ui, SIGNAL(valueChanged(const QTime&)), CommandConnections,
                    SLOT(ValueChangedHandler(const QTime &)));
        if (mo->indexOfSignal("stateChanged(int)") != -1)
            connect(ui, SIGNAL(stateChanged(int)), CommandConnections,
                    SLOT(StateChangedHandler(int)));
        if (mo->indexOfSignal("activated(int)") != -1)
            connect(ui, SIGNAL(activated(int)), CommandConnections,
                    SLOT(ActivatedHandler(int)));
        if (mo->indexOfSignal("textChanged(const QString&)") != -1)
            connect(ui, SIGNAL(textChanged(const QString &)),
                    CommandConnections,
                    SLOT(TextChangedHandler(const QString&)));
        if (mo->indexOfSignal("currentChanged(int,int)") != -1)
            connect(ui, SIGNAL(currentChanged(int, int)), CommandConnections,
                    SLOT(CurrentChangedHandler(int, int)));       
        if (mo->indexOfSignal("valueChanged(int,int)") != -1)
            connect(ui, SIGNAL(valueChanged(int, int)), CommandConnections,
                    SLOT(ValueChangedHandler(int, int)));
            //connect(ui, SIGNAL(dialMoved(int)), CommandConnections,
            //        SLOT(valueChangedHandler(int)));
    }

    // enable custom command UI button
    debug5 << "enabling custom command button" << endl;
    simCommands->setButtonEnabled(CUSTOM_BUTTON, true);
    debug5 << "successfully added simulation interface" << endl;
}

// ****************************************************************************
// Method: QvisSimulationWindow::Update
//
// Purpose:
//   This method is called when the subjects that the window observes are
//   modified.
//
// Arguments:
//   TheChangedSubject : A pointer to the subject that changed.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//
// ****************************************************************************

void
QvisSimulationWindow::Update(Subject *TheChangedSubject)
{
    caller = TheChangedSubject;

    if (isCreated)
        UpdateWindow(false);
}

// ****************************************************************************
// Method: QvisSimulationWindow::SubjectRemoved
//
// Purpose: 
//   This method is called when the subjects observed by the window are
//   destructed.
//
// Arguments:
//   TheRemovedSubject : A pointer to the subject to remove.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisSimulationWindow::SubjectRemoved(Subject *TheRemovedSubject)
{
    if (TheRemovedSubject == engines)
        engines = 0;
    else if (TheRemovedSubject == statusAtts)
        statusAtts = 0;
}

// ****************************************************************************
// Method: QvisSimulationWindow::UpdateCustomUIComponent
//
// Purpose:
//   Updates the ui components in the Costom UI popup. It check for matches
//   between ui updates sent from the simulations to ui components in the
//   custom ui popup. If it finds a match it update the ui component.
//
// Arguments:
//   cmd   : command specification that update the UI component
//   window: the parent window that contains the ui
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//   Brad Whitlock, Tue Jan 31 16:53:30 PST 2006
//   I made the method return early if the dynamic commands window has not
//   yet been created. It's now created more on-demand.
//
//   Shelly Prevost, Tue Sep 12 14:53:40 PDT 2006
//   I generalized this function to work with both the custom GUI
//   and this window. I also modified the QTextWidget to not append the
//   text if it is equal to "". This fixes the problem of not being able
//   to shut off messages.
//
//   Brad Whitlock, Mon Jul  7 13:37:13 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void 
QvisSimulationWindow::UpdateUIComponent (QWidget *window, avtSimulationCommandSpecification *cmd)
{
    if(window == NULL)
        return;

    QObject *ui = NULL;
    ui  = window->findChild<QWidget *>(QString(cmd->GetName().c_str()));
    if (ui)
    {
        debug5 << "Looking up component = " << cmd->GetName().c_str() << endl;

        // Block signals so updating the user interface does not cause a
        // command to go back to the simulation.
        ui->blockSignals(true);

        if (ui->inherits("QLabel"))
        {
            debug5 << "found label " << cmd->GetName().c_str() << " text = "
                   << cmd->GetText().c_str() << endl;
            const QString label(cmd->GetText().c_str());
            ((QLabel*)ui)->setText(label );
        }

        // just set the generic button attributes. more specific attributes will be set latter
        if (ui->inherits("QAbstractButton"))
        {
            debug5 << "found button " << cmd->GetName().c_str() << " text = "
                   << cmd->GetText().c_str() << endl;
            const QString label(cmd->GetText().c_str());
            ((QAbstractButton*)ui)->setText(label );
            ((QAbstractButton*)ui)->setEnabled(cmd->GetEnabled());
        }

        if (ui->inherits("QPushButton"))
        {
            debug5 << "found button" << cmd->GetName().c_str() << " text = "
                   << cmd->GetText().c_str() << endl;
            const QString label(cmd->GetText().c_str());
            ((QPushButton*)ui)->setText(label);
            ((QPushButton*)ui)->setEnabled(cmd->GetEnabled()); 
        }

        if (ui->inherits("QTabWidget"))
        {
            debug5 << "found button " << cmd->GetName().c_str() << " text = "
                   << cmd->GetText().c_str() << endl;
            ((QTabWidget*)ui)->setEnabled(cmd->GetEnabled());
        }

        if (ui->inherits( "QLineEdit"))
        {
            debug5 << "found button " << cmd->GetName().c_str() << " text = "
                   << cmd->GetText().c_str() << endl;
            const QString label(cmd->GetText().c_str());
            ((QLineEdit*)ui)->setText(label );
            ((QLineEdit*)ui)->setEnabled(cmd->GetEnabled());
        }

        if (ui->inherits("QRadioButton"))
        {
            debug5 << "found button " << cmd->GetName().c_str() << " text = "
                   << cmd->GetText().c_str() << endl;
            const QString label(cmd->GetText().c_str());
            ((QRadioButton*)ui)->setText(label);
            ((QRadioButton*)ui)->setChecked(cmd->GetIsOn());
            ((QRadioButton*)ui)->setEnabled(cmd->GetEnabled());
        }

        if (ui->inherits("QProgressBar"))
        {
            debug5 << "found ProgressBar " << cmd->GetName().c_str()
                   << " value = " << cmd->GetText().c_str() << endl;
            const QString label(cmd->GetValue().c_str());
            ((QProgressBar*)ui)->setValue(label.toInt());
        }

        if (ui->inherits("QSpinBox"))
        {
            debug5 << "found QSpinBox " << cmd->GetName().c_str() << " value = "
                   << cmd->GetValue().c_str() << endl;
            const QString label(cmd->GetValue().c_str());
            ((QSpinBox*)ui)->setValue(label.toInt());
            ((QSpinBox*)ui)->setEnabled(cmd->GetEnabled());
        }
 
        if (ui->inherits("QDial"))
        {
            debug5 << "found QDial " << cmd->GetName().c_str() << " value = "
                   << cmd->GetValue().c_str() << endl;
            const QString label(cmd->GetValue().c_str());
            ((QDial*)ui)->setValue(label.toInt());
            ((QDial*)ui)->setEnabled(cmd->GetEnabled());
        }

        if (ui->inherits("QSlider"))
        {
            debug5 << "found QSlider " << cmd->GetName().c_str() << " value = "
                   << cmd->GetValue().c_str() << endl;
            const QString label(cmd->GetValue().c_str());
            ((QSlider*)ui)->setValue(label.toInt());
            ((QSlider*)ui)->setEnabled(cmd->GetEnabled());
        }

        if (ui->inherits("QTextEdit"))
        {
            debug5 << "found QTextEdit " << cmd->GetName().c_str()
                   << " text = " << cmd->GetText().c_str() << endl;
            const QString message(cmd->GetText().c_str());
            // we need a way to keep from repeating whatever is
            // in the text field without deleting the command channel.
            // For now use a minus sign to say don't append the line
            // again. I'll add special commands to handle this case
            // in general in the next release.
            if ( message != "" )
                 ((QTextEdit*)ui)->append(message);
            ((QTextEdit*)ui)->setEnabled(cmd->GetEnabled());
        }

        if ( ui->inherits ( "QLineEdit"))
        {
            debug5 << "found QTextEdit " << cmd->GetName().c_str()
                   << " text = " << cmd->GetText().c_str() << endl;
            const QString label(cmd->GetText().c_str());
            ((QLineEdit*)ui)->setText(label);
            ((QLineEdit*)ui)->setEnabled(cmd->GetEnabled());
        }

        if ( ui->inherits ( "QLCDNumber"))
        {
            debug5 << "found QLCDNumber " << cmd->GetName().c_str()
                   << " value = " << cmd->GetValue().c_str() << endl;
            const QString label(cmd->GetValue().c_str());
            ((QLCDNumber*)ui)->display(label);
            ((QLCDNumber*)ui)->setEnabled(cmd->GetEnabled());
        }

        if ( ui->inherits ( "QTimeEdit"))
        {
            debug5 << "found QTimeEdit " << cmd->GetName().c_str()
                   << " text = " << cmd->GetText().c_str() << endl;
            const QString label(cmd->GetText().c_str());
            QTime time1 = QTime::fromString(cmd->GetText().c_str());
            ((QTimeEdit*)ui)->setTime(time1);
            ((QTimeEdit*)ui)->setEnabled(cmd->GetEnabled());
        }

        if (ui->inherits("QDateEdit"))
        {
            debug5 << "found QTDateEdit " << cmd->GetName().c_str()
                   << " text = " << cmd->GetText().c_str() << endl;
            const QString label(cmd->GetText().c_str());
            QDate date = QDate::fromString( label );
            ((QDateEdit*)ui)->setDate(date);
            ((QDateEdit*)ui)->setEnabled(cmd->GetEnabled());
        }
        if (ui->inherits("QCheckBox"))
        {
            debug5 << "found QCheckBox " << cmd->GetName().c_str()
                   << " value = " << cmd->GetValue().c_str() << endl;
            //const QString label(cmd->GetValue().c_str());
            ((QCheckBox*)ui)->setEnabled(cmd->GetEnabled());
            ((QCheckBox*)ui)->setChecked(cmd->GetIsOn());
        }

        // Unblock signals.
        ui->blockSignals(false);
    }
    else
        debug5 << "could not find UI component named "
               << cmd->GetName().c_str() << endl;
}

// ****************************************************************************
// Method: QvisSimulationWindow::ConnectStatusAttributes
//
// Purpose: 
//   Connects the status attributes subject that the window will observe.
//
// Arguments:
//   s : A pointer to the status attributes that the window will observe.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisSimulationWindow::ConnectStatusAttributes(StatusAttributes *s)
{
    if (s)
    {
        statusAtts = s;
        statusAtts->Attach(this);
    }
}

// ****************************************************************************
// Method: QvisSimulationWindow::SetNewMetaData
//
// Purpose:
//    Update the meta data for the given file.
//
// Arguments:
//   qf        the host+file
//   md        the new meta data
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//    Jeremy Meredith, Thu Apr 28 18:03:57 PDT 2005
//    It needed more info to construct the key correctly.
//
// ****************************************************************************

void
QvisSimulationWindow::SetNewMetaData(const QualifiedFilename &qf,
                                     const avtDatabaseMetaData *md)
{
    if (md && md->GetIsSimulation())
    {
        *metadata = *md;

        QString key;
        std::string host = qf.host;
        if (host == "localhost")
            host = GetViewerProxy()->GetLocalHostName();

        std::string path = qf.PathAndFile();

        key.sprintf("%s:%s", host.c_str(), path.c_str());
        UpdateMetaDataEntry(key);

        // If the sender of the status message is the engine that we're
        // currently looking at, update the status widgets.
        if (key == activeEngine)
        {
            UpdateStatusArea();
            UpdateInformation(key);
        }
    }
}

// ****************************************************************************
// Method: QvisSimulationWindow::UpdateWindow
//
// Purpose: 
//   This method is called to update the window's widgets when the subjects
//   change.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//   Brad Whitlock, Tue Jan 31 17:35:31 PST 2006
//   I added code to delete the dynamic commands window if we've created it
//   before and there are no hosts. If it is allowed to persist then it
//   causes a crash when you go to click on it because of its SimCommandSlots
//   object.
//
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Mon Jul  7 13:46:00 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSimulationWindow::UpdateWindow(bool doAll)
{
    if (caller == engines || doAll)
    {
        const stringVector &host = engines->GetEngines();
        const stringVector &sim  = engines->GetSimulationName();

        // If we have zero hosts but we have a dynamic command window then
        // we must have disconnected from the simulation. Blow away the
        // dynamic command window.
        if(host.size() < 1 && DynamicCommandsWin != NULL)
        {
            debug5 << "Destroying dynamic command window" << endl;
            delete DynamicCommandsWin;
            DynamicCommandsWin = NULL;
        }

        // Add the engines to the widget.
        simCombo->blockSignals(true);
        simCombo->clear();
        simulationToEngineListMap.clear();
        int current = -1;
        for (int i = 0; i < host.size(); ++i)
        {
            QString temp(host[i].c_str());
            if (!sim[i].empty())
            {
                simulationToEngineListMap[simCombo->count()] = i;

                int lastSlashPos = QString(sim[i].c_str()).lastIndexOf('/');
                QString newsim = QString(sim[i].substr(lastSlashPos+1).c_str());
                int lastDotPos =  newsim.lastIndexOf('.');
                int firstDotPos =  newsim.indexOf('.');

                QString name = newsim.mid(firstDotPos+1,
                                          lastDotPos-firstDotPos-1);
                temp = name + QString(" ") + tr("on") + QString(" ") + host[i].c_str();
                simCombo->addItem(temp);

                if (temp == activeEngine)
                    current = i;

            }
        }

        if (current == -1)
        {
            // Update the activeEngine string.
            if (simCombo->count() > 0)
            {
                simCombo->setCurrentIndex(0);
                int index = simulationToEngineListMap[0];
                current = index;
                if (sim[index]=="")
                    activeEngine = QString().sprintf("%s",host[index].c_str());
                else
                    activeEngine = QString().sprintf("%s:%s",
                                                     host[index].c_str(),
                                                     sim[index].c_str());

                // Add an entry if needed.
                AddStatusEntry(activeEngine);
                AddMetaDataEntry(activeEngine);
            }
            else
            {
                // The active engine was not in the list of engines returned
                // by the viewer. Remove the entry from the list.
                RemoveStatusEntry(activeEngine);
                RemoveMetaDataEntry(activeEngine);
                activeEngine = QString("");
            }
        }
        else
        {
            simCombo->setCurrentIndex(current);
        }
        simCombo->blockSignals(false);

        // Update the engine information.
        UpdateInformation(current);

        // Set the enabled state of the various widgets.
        interruptEngineButton->setEnabled(simCombo->count() > 0);
        closeEngineButton->setEnabled(simCombo->count() > 0);
        clearCacheButton->setEnabled(simCombo->count() > 0);
        simCombo->setEnabled(simCombo->count() > 0);
    }

    if (caller == statusAtts || doAll)
    {
        QString key(statusAtts->GetSender().c_str());
        debug5 << "Status being updated" << endl; 
        if (key != QString("viewer"))
        {
            UpdateStatusEntry(key);

            // If the sender of the status message is the engine that we're
            // currently looking at, update the status widgets.
            if (key == activeEngine)
            {
                UpdateStatusArea();
            }
        }
    }
   
}

// ****************************************************************************
// Method: QvisSimulationWindow::UpdateCustomUI
//
// Purpose:
//   Updates the ui components in the Custom UI popup.
//
// Arguments:
//   md : meta data from the simulation.
//
// Programmer: Shelly Prevost
// Creation:   December 9, 2005
//
// Modifications:
//   Shelly Prevost, Tue Sep 12 15:05:31 PDT 2006
//   The new version of UpdateUIComponent requires you pass in the
//   window as an arguement.
//
//   Brad Whitlock, Fri Mar 9 17:08:29 PST 2007
//   Updated so it uses new metadata interface.
//
// ****************************************************************************

void
QvisSimulationWindow::UpdateCustomUI (avtDatabaseMetaData *md)
{
    int numCustCommands = md->GetSimInfo().GetNumCustomCommands();
    // loop thru all command updates and updates the matching UI component.
    for (int c=0; c<numCustCommands; c++)
    {
        UpdateUIComponent (DynamicCommandsWin,&(md->GetSimInfo().GetCustomCommands(c)));
    }
}

// ****************************************************************************
// Method: QvisSimulationWindow::UpdateSimulationUI
//
// Purpose:
//   Updates the ui components in the simulation window UI.
//
// Arguments:
//   md : meta data from the simulation.
//
// Programmer: Shelly Prevost
// Creation:   August 25, 2006
//
// Modifications:
//   Shelly Prevost Fri Dec  1 10:36:07 PST 2006
//   To support the Strip Chart and other special widgets it
//   additional processing was required. I added a function call
//   to do this.
//
// ****************************************************************************

void
QvisSimulationWindow::UpdateSimulationUI (avtDatabaseMetaData *md)
{
    int numCommands = md->GetSimInfo().GetNumGenericCommands();
    // loop thru all command updates and updates the matching UI component.
    for (int c=NUM_GENERIC_BUTTONS; c<numCommands; c++)
    {
        UpdateUIComponent (this,&(md->GetSimInfo().GetGenericCommands(c)));
        SpecialWidgetUpdate (&(md->GetSimInfo().GetGenericCommands(c)));
    }
}


// ****************************************************************************
// Method: QvisSimulationWindow::SpecialWidgetUpdate
//
// Purpose:
//   Some Widgets need special processing in addition to their
//   data being updated. This method calls the proper methods
//   to do the processing.
//
// Arguments:
//   cmd:  ui data information
//
// Programmer: Shelly Prevost
// Creation:   Tue Nov 28 17:12:04 PST 2006
//
// Modifications:
//   Shelly Prevost Fri Oct 12 15:19:40 PDT 2007
//   added multiple strip chart data updating and 
//   special processing for tab label updates
//
//   Brad Whitlock, Tue Jul  8 09:10:38 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void 
QvisSimulationWindow::SpecialWidgetUpdate (avtSimulationCommandSpecification *cmd)
{
    QObject *ui = NULL;
    ui  = findChild<QObject *>(cmd->GetName().c_str());  
    if ( stripCharts->isStripChartWidget(cmd->GetName().c_str()))
    {    double maxY;
         double minY;
         const QString dataX(cmd->GetText().c_str());
         const QString dataY(cmd->GetValue().c_str());
         stripCharts->setEnable(cmd->GetName().c_str(),cmd->GetEnabled());
         bool outOfBounds = stripCharts->addDataPoint(cmd->GetName().c_str(),dataX.toDouble(),dataY.toDouble());   
         stripCharts->update(cmd->GetName().c_str());
         stripCharts->getMinMaxData(cmd->GetName().c_str(), minY, maxY);
 
         if ( outOfBounds )
         {
            QString warning;
            warning.sprintf( "ALERT;%s;Data;OutOfBounds;", cmd->GetName().c_str());
            int simIndex = simCombo->currentIndex();
            ViewerSendCMD ( simIndex, warning);
         }
    }
    if ( stripCharts->isStripChartTabLabel(cmd->GetName().c_str()))
    {    
        QString tabName(cmd->GetName().c_str());
        QString tabLabel( cmd->GetText().c_str());
        stripCharts->setTabLabel(tabName, tabLabel);
    }

    if ( !strcmp (MESSAGE_WIDGET_NAME,cmd->GetName().c_str()))
    {
         const QString dataX(cmd->GetText().c_str());
         const QString dataY(cmd->GetValue().c_str());
         if (ui != NULL && dataX != "")
         {
             QTextEdit *t = (QTextEdit*)ui;
             QPalette pal(t->palette());
             pal.setColor(QPalette::Text, getColor(dataY));
             t->setPalette(pal);
         }
    }
}

// ****************************************************************************
// Method: QvisSimulationWindow::UpdateStatusArea
//
// Purpose: 
//   Updates the window so it reflects the status information for the 
//   currently selected engine.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//   Brad Whitlock, Mon Jul  7 14:43:57 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSimulationWindow::UpdateStatusArea()
{
    SimulationStatusMap::Iterator pos;
    if ((pos = statusMap.find(activeEngine)) == statusMap.end())
        return;

    StatusAttributes *s = pos.value();
    if (s->GetClearStatus())
    {
        s->SetStatusMessage("");
        totalProgressBar->reset();
    }
    else
    {
        int total;
        if (s->GetMaxStage() > 0)
        {
            float pct0  = float(s->GetPercent()) / 100.;
            float pct1  = float(s->GetCurrentStage()-1) / float(s->GetMaxStage());
            float pct2  = float(s->GetCurrentStage())   / float(s->GetMaxStage());
            total = int(100. * ((pct0 * pct2) + ((1.-pct0) * pct1)));
        }
        else
            total = 0;

        // Set the progress bar percent done.
        totalProgressBar->setValue(total);
   }
}

// ****************************************************************************
// Method: QvisSimulationWindow::UpdateInformation
//
// Purpose: 
//   Updates the engine information.
//
// Arguments:
//   index : The index of the engine to update.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//   Jeremy Meredith, Thu Apr 28 18:05:36 PDT 2005
//   Added concept of disabled control command.
//   Added concept of a simulation mode (e.g. running/stopped).
//
//   Shelly Prevost, Tue Jan 24 17:06:49 PST 2006
//   Added a custom simulation control window.
//
//   Brad Whitlock, Tue Jan 31 16:32:36 PST 2006
//   Added code to set the enabled state of the custom command button.
//
//   Shelly Prevost, Tue Sep 12 15:05:31 PDT 2006
//   Remove hard coded number of buttons and use a definded const instead.
//   Added Update SimulationUi call now that the main window needs to be
//   updated also.
//
//   Shelly Prevost, Tue Nov 28 17:12:04 PST 2006
//   Removed hard code button number
//
//   Brad Whitlock, Fri Mar 9 17:10:40 PST 2007
//   Made it use new metadata api.
//
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Tue Jul  8 14:59:48 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSimulationWindow::UpdateInformation(int index)
{
    const stringVector &s = engines->GetEngines();
    debug5 << "Update Information was called" << endl;

    // Set the values of the engine information widgets.
    if (index == -1 || s.size() < 1)
    {
        simInfo->clear();
        simInfo->setEnabled(false);
        for (int c=0; c<NUM_GENERIC_BUTTONS; c++)
        {
            simCommands->setButtonEnabled(c, false);
        }
    }
    else
    {
        string host = engines->GetEngines()[index];
        string sim  = engines->GetSimulationName()[index];
        int    np   = engines->GetNumProcessors()[index];

        QString key;
        key.sprintf("%s:%s",
                    engines->GetEngines()[index].c_str(),
                    engines->GetSimulationName()[index].c_str());

        if (!metadataMap.contains(key))
        {
            simInfo->clear();
            simInfo->setEnabled(false);
            for (int c=0; c<NUM_GENERIC_BUTTONS; c++)
            {
                simCommands->setButtonEnabled(c, false);
            }
            return;
        }

        simInfo->clear();
        avtDatabaseMetaData *md = metadataMap[key];
       
        QString tmp1,tmp2;
        QTreeWidgetItem *item;

        item = new QTreeWidgetItem(simInfo, 
                                   QStringList(tr("Host")) + 
                                   QStringList(md->GetSimInfo().GetHost().c_str()));

        int lastSlashPos = QString(sim.c_str()).lastIndexOf('/');
        QString newsim = QString(sim.substr(lastSlashPos+1).c_str());
        int lastDotPos =  newsim.lastIndexOf('.');
        int firstDotPos =  newsim.indexOf('.');

        QString name = newsim.mid(firstDotPos+1,
                                  lastDotPos-firstDotPos-1);
        item = new QTreeWidgetItem(simInfo, 
            QStringList(tr("Name")) + 
            QStringList(name));


        QString timesecstr = newsim.left(firstDotPos);
        time_t timesec = timesecstr.toInt();
        QString timestr = ctime(&timesec);
        item = new QTreeWidgetItem(simInfo, 
            QStringList(tr("Date")) +
            QStringList(timestr.left(timestr.length()-1)));

        tmp1.sprintf("%d", np);
        item = new QTreeWidgetItem(simInfo, 
            QStringList(tr("Num Processors")) + 
            QStringList(tmp1));

        const vector<string> &names  = md->GetSimInfo().GetOtherNames();
        const vector<string> &values = md->GetSimInfo().GetOtherValues();

        for (int i=0; i<names.size(); i++)
        {
            item = new QTreeWidgetItem(simInfo,
                QStringList(names[i].c_str()) + 
                QStringList(values[i].c_str()));
        }

        debug5 << "Updating Status information" << endl;
        switch (md->GetSimInfo().GetMode())
        {
          case avtSimulationInformation::Unknown:
            simulationMode->setText(tr("Unknown"));
            break;
          case avtSimulationInformation::Running:
            simulationMode->setText(tr("Running"));
            break;
          case avtSimulationInformation::Stopped:
            simulationMode->setText(tr("Stopped"));
            break;
        }

        // If we've not created a dynamic commands window already and we can get a
        // decent-looking UI filename, enabled the custom command button
        // so we can create a window when that button is clicked.
        if(DynamicCommandsWin == NULL)
        {
            QString fname(GetUIFile());
            simCommands->setButtonEnabled(CUSTOM_BUTTON, !fname.isEmpty());
        }

        for (int c=0; c<NUM_GENERIC_BUTTONS; c++)
        {
            if (md->GetSimInfo().GetNumGenericCommands()<=c)
            {
                simCommands->setButtonEnabled(c, false);
            }
            else
            {
                avtSimulationCommandSpecification::CommandArgumentType t;
                t = md->GetSimInfo().GetGenericCommands(c).GetArgumentType();
                bool e = md->GetSimInfo().GetGenericCommands(c).GetEnabled();
                if (t == avtSimulationCommandSpecification::CmdArgNone)
                {
                    QString bName = QString(md->GetSimInfo().GetGenericCommands(c).GetName().c_str());
                    simCommands->setButtonCommand(c, bName);
                    simCommands->setButtonEnabled(c, (c != CUSTOM_BUTTON) ? e : true);
                }
                else
                {
                    simCommands->setButtonEnabled(c, false);
                }
            }
        }
        // update ui component informatinon in the meta data
        UpdateCustomUI(md);
        UpdateSimulationUI(md);
        simInfo->setEnabled(true);
    }
}

// ****************************************************************************
// Method: QvisSimulationWindow::UpdateInformation
//
// Purpose: 
//   Updates the engine information.
//
// Arguments:
//   key:      the key for the engine to update
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//
// ****************************************************************************

void
QvisSimulationWindow::UpdateInformation(const QString &key)
{
    int nengines = engines->GetEngines().size();
    for (int index = 0 ; index < nengines ; index++)
    {
        string host = engines->GetEngines()[index];
        string sim  = engines->GetSimulationName()[index];

        QString testkey;
        if (sim == "")
            testkey.sprintf("%s",
                            engines->GetEngines()[index].c_str());
        else
            testkey.sprintf("%s:%s",
                            engines->GetEngines()[index].c_str(),
                            engines->GetSimulationName()[index].c_str());

        if (testkey == key)
        {
            UpdateInformation(index);
        }
    }
}

// ****************************************************************************
// Method: QvisSimulationWindow::AddStatusEntry
//
// Purpose: 
//   Adds an engine to the internal status map.
//
// Arguments:
//   key : The name of the engine to add to the map.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisSimulationWindow::AddStatusEntry(const QString &key)
{
    // If the entry is in the map, return.
    if (statusMap.contains(key))
        return;

    // Add the entry to the map.
    StatusAttributes *s = new StatusAttributes;
    *s = *statusAtts;
    statusMap.insert(key, s);
}

// ****************************************************************************
// Method: QvisSimulationWindow::RemoveStatusEntry
//
// Purpose: 
//   Removes an engine from the internal status map.
//
// Arguments:
//   key : The name of the engine to remove from the map.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//   Brad Whitlock, Mon Jul  7 11:16:12 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSimulationWindow::RemoveStatusEntry(const QString &key)
{
    // If the entry is not in the map, return.
    SimulationStatusMap::Iterator pos;
    if ((pos = statusMap.find(key)) == statusMap.end())
        return;

    // Delete the status attributes that are in the map.
    delete pos.value();
    // Remove the key from the map.
    statusMap.remove(key);
}

// ****************************************************************************
// Method: QvisSimulationWindow::UpdateStatusEntry
//
// Purpose: 
//   Makes the specified entry in the status map update to the current status
//   in the status attributes.
//
// Arguments:
//   key : The name of the engine whose status entry we want to update.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//   Brad Whitlock, Mon Jul  7 11:16:27 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSimulationWindow::UpdateStatusEntry(const QString &key)
{
    // If the sender is in the status map, copy the status into the map entry.
    // If the sender is not in the map, add it.
    SimulationStatusMap::Iterator pos;
    if ((pos = statusMap.find(activeEngine)) != statusMap.end())
    {
        // Copy the status attributes.
        *(pos.value()) = *statusAtts;
    }
    else
        AddStatusEntry(key);
}

// ****************************************************************************
// Method: QvisSimulationWindow::AddMetaDataEntry
//
// Purpose: 
//   Adds an engine to the internal meta data map.
//
// Arguments:
//   key : The name of the engine to add to the map.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisSimulationWindow::AddMetaDataEntry(const QString &key)
{
    // If the entry is in the map, return.
    if (metadataMap.contains(key))
        return;

    // Add the entry to the map.
    avtDatabaseMetaData *md = new avtDatabaseMetaData;
    *md = *metadata;
    metadataMap.insert(key, md);
}

// ****************************************************************************
// Method: QvisSimulationWindow::RemoveMetaDataEntry
//
// Purpose: 
//   Removes an engine from the internal meta data map.
//
// Arguments:
//   key : The name of the engine to remove from the map.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//   Brad Whitlock, Mon Jul  7 11:16:41 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSimulationWindow::RemoveMetaDataEntry(const QString &key)
{
    // If the entry is not in the map, return.
    SimulationMetaDataMap::Iterator pos;
    if ((pos = metadataMap.find(key)) == metadataMap.end())
        return;

    // Delete the meta data that are in the map.
    delete pos.value();
    // Remove the key from the map.
    metadataMap.remove(key);
}

// ****************************************************************************
// Method: QvisSimulationWindow::UpdateMetaDataEntry
//
// Purpose: 
//   Makes the specified entry in the meta data map update to the current
//   meta data.
//
// Arguments:
//   key : The name of the engine whose meta data entry we want to update.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//   Brad Whitlock, Mon Jul  7 11:16:50 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSimulationWindow::UpdateMetaDataEntry(const QString &key)
{
    // If the sender is in the meta data map, copy the MD into the map entry.
    // If the sender is not in the map, add it.
    SimulationMetaDataMap::Iterator pos;
    if ((pos = metadataMap.find(activeEngine)) != metadataMap.end())
    {
        // Copy the status attributes.
        *(pos.value()) = *metadata;
    }
    else
        AddMetaDataEntry(key);
}

//
// Qt slots
//

// ****************************************************************************
// Method: QvisSimulationWindow::closeEngine
//
// Purpose: 
//   This is a Qt slot function that is called when the "Close engine" button
//   is clicked. Its job is to tell the viewer to close the engine being
//   displayed by the window.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Mon Jul  7 11:18:45 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSimulationWindow::closeEngine()
{
    int index = simCombo->currentIndex();
    if (index < 0)
        return;

    string host = engines->GetEngines()[index];
    string sim  = engines->GetSimulationName()[index];

    // Create a prompt for the user.
    QString msg;
    if (sim == "")
    {
        msg = tr("Really close the compute engine on host \"%1\"?\n\n").
              arg(host.c_str());
    }
    else
    {
        msg = tr("Really disconnect from the simulation \"%1\" on "
                 "host \"%2\"?\n\n").
              arg(sim.c_str()).
              arg(host.c_str());
    }

    // Ask the user if he really wants to close the engine.
    if (QMessageBox::warning(this, "VisIt", msg, QMessageBox::Ok | QMessageBox::Cancel)
        == QMessageBox::Ok)
    {
        // The user actually chose to close the engine.
        GetViewerMethods()->CloseComputeEngine(host, sim);
    }
}

// ****************************************************************************
// Method: QvisSimulationWindow::interruptEngine
//
// Purpose: 
//   This is a Qt slot function that is called to interrupt the engine that's
//   displayed in the window.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//   Brad Whitlock, Mon Jul  7 11:14:49 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSimulationWindow::interruptEngine()
{
    int index = simCombo->currentIndex();
    if (index < 0)
        return;
    string host = engines->GetEngines()[index];
    string sim  = engines->GetSimulationName()[index];

    GetViewerProxy()->InterruptComputeEngine(host, sim);
}

// ****************************************************************************
// Method: QvisSimulationWindow::selectEngine
//
// Purpose: 
//   This is a Qt slot function that is called when selecting a new engine
//   to display.
//
// Arguments:
//   index : The index to use for the next active engine.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//
// ****************************************************************************

void
QvisSimulationWindow::selectEngine(int simindex)
{
    int index = simulationToEngineListMap[simindex];
    if (engines->GetSimulationName()[index]=="")
        activeEngine = QString().sprintf("%s",
                                         engines->GetEngines()[index].c_str());
    else
        activeEngine = QString().sprintf("%s:%s",
                                  engines->GetEngines()[index].c_str(),
                                  engines->GetSimulationName()[index].c_str());

    // Update the rest of the widgets using the information for the
    // active engine.
    UpdateStatusArea();
    // Update the engine information.
    UpdateInformation(index);
}

// ****************************************************************************
// Method: QvisSimulationWindow::clearCache
//
// Purpose: 
//   This is a Qt slot function that is called to tell the current engine
//   to clear its cache.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2005
//
// Modifications:
//   Brad Whitlock, Mon Jul  7 11:13:39 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSimulationWindow::clearCache()
{
    int index = simCombo->currentIndex();
    if (index < 0)
        return;
    string host = engines->GetEngines()[index];
    string sim  = engines->GetSimulationName()[index];

    if (GetViewerProxy()->GetLocalHostName() == host)
        GetViewerMethods()->ClearCache("localhost", sim);
    else
        GetViewerMethods()->ClearCache(host, sim);
}

// ****************************************************************************
// Method: QvisSimulationWindow::showCommandWindow
//
// Purpose:
//   This method is called when the subjects that the window observes are
//   modified.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//    Brad Whitlock, Tue Jan 31 15:54:43 PST 2006
//    I made it create the commands window on the fly instead of creating
//    it at the same time as the regular Window is created.
//
// ****************************************************************************

void QvisSimulationWindow::showCommandWindow()
{
    if(DynamicCommandsWin == NULL)
        CreateCommandUI();

    if (DynamicCommandsWin != NULL)
        DynamicCommandsWin->show();
}

// ****************************************************************************
// Method: QvisSimulationWindow::showCommandWindow
//
// Purpose:
//   This method is called when the subjects that the window observes are
//   modified.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//   Brad Whitlock, Tue Jan 31 15:54:43 PST 2006
//   I made it create the commands window on the fly instead of creating
//   it at the same time as the regular Window is created.
//
//   Brad Whitlock, Mon Jul  7 11:14:13 PDT 2008
//   Qt 4.
//
// ****************************************************************************
void
QvisSimulationWindow::executePushButtonCommand(const QString &btncmd)
{
    int simindex = simCombo->currentIndex();
    if (simindex < 0)
        return;
    int index = simulationToEngineListMap[simindex];
    string host = engines->GetEngines()[index];
    string sim  = engines->GetSimulationName()[index];

    QString cmd("clicked();" + btncmd + ";QPushButton;Simulations;NONE");
    GetViewerMethods()->SendSimulationCommand(host, sim, cmd.toStdString());
}

// ****************************************************************************
// Method: QvisSimulationWindow::executeEnableTimeRange()
//
// Purpose:
//   This method is called when the user clicks on the enable button
//   for the time range ui
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//   Brad Whitlock, Mon Jul  7 11:15:38 PDT 2008
//   Qt 4.
//
// ****************************************************************************
void
QvisSimulationWindow::executeEnableTimeRange(const QString &value)
{
    int simindex = simCombo->currentIndex();
    if (simindex < 0)
        return;
    int index = simulationToEngineListMap[simindex];
    string host = engines->GetEngines()[index];
    string sim  = engines->GetSimulationName()[index];

    QString cmd("clicked();TimeLimitsEnabled;QCheckBox;Simulations;" + value);
    GetViewerMethods()->SendSimulationCommand(host, sim, cmd.toStdString());
}


// ****************************************************************************
// Method: QvisSimulationWindow::executeStartCommand()
//
// Purpose:
//   This method is called when the types into the start text box for time
//   range control.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//   Shelly Prevost Fri Dec  1 10:36:07 PST 2006
//   Corrected the widget name being passed to the simulation in the
//   command string.
//
//   Brad Whitlock, Tue Jul  8 14:53:05 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void 
QvisSimulationWindow::executeStartCommand(const QString &value)
{
    int simindex = simCombo->currentIndex();
    if (simindex < 0)
        return;
    int index = simulationToEngineListMap[simindex];
    string host = engines->GetEngines()[index];
    string sim  = engines->GetSimulationName()[index];

    QString cmd(QString("returnedPressed();StartCycle;QLineEdit;Simulations;") + value);
    GetViewerMethods()->SendSimulationCommand(host, sim, cmd.toStdString());
}

// ****************************************************************************
// Method: QvisSimulationWindow::executeStepCommand()
//
// Purpose:
//   This method is called when the types into the step text box for time
//   range control.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//    Shelly Prevost Fri Dec  1 10:36:07 PST 2006
//    Corrected the widget name being passed to the simulation in the
//    command string.
//
//   Brad Whitlock, Tue Jul  8 14:53:51 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void 
QvisSimulationWindow::executeStepCommand(const QString &value)
{
    int simindex = simCombo->currentIndex();
    if (simindex < 0)
        return;
    int index = simulationToEngineListMap[simindex];
    string host = engines->GetEngines()[index];
    string sim  = engines->GetSimulationName()[index];

    QString cmd(QString("returnedPressed();StepCycle;QLineEdit;Simulations;") + value);
    GetViewerMethods()->SendSimulationCommand(host, sim, cmd.toStdString());
}

// ****************************************************************************
// Method: QvisSimulationWindow::executeStopCommand()
//
// Purpose:
//   This method is called when the types into the stop text box for time
//   range control.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//    Shelly Prevost Fri Dec  1 10:36:07 PST 2006
//    Corrected the widget name being passed to the simulation in the
//    command string.
//
// ****************************************************************************
void 
QvisSimulationWindow::executeStopCommand(const QString &value)
{
    int simindex = simCombo->currentIndex();
    if (simindex < 0)
        return;
    int index = simulationToEngineListMap[simindex];
    string host = engines->GetEngines()[index];
    string sim  = engines->GetSimulationName()[index];
    
    QString cmd(QString("returnedPressed();StopCycle;QLineEdit;Simulations;") + value);
    GetViewerMethods()->SendSimulationCommand(host, sim, cmd.toStdString()); 
}


// ****************************************************************************
// Method: QvisSimulationWindow::ViewerSendCMD
//
// Purpose:
//   This method is called to send a command to the simulation
//
// Arguments:
//   simIndex : The index from the combo box for the simulation
//              that you want to send your command too.
//   cmd      : the properly form command string.
//
// Programmer: Shelly Prevost
// Creation:   Thu Mar 15 15:22:25 PDT 2007
//
// Modifications:
//   Brad Whitlock, Mon Jul  7 14:51:44 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSimulationWindow::ViewerSendCMD (int simIndex, QString cmd)
{
    if (simIndex < 0)
        return;
    int index = simulationToEngineListMap[simIndex];
    string host = engines->GetEngines()[index];
    string sim  = engines->GetSimulationName()[index];
    GetViewerMethods()->SendSimulationCommand(host, sim, cmd.toStdString().c_str());
}

// ****************************************************************************
// Method: QvisSimulationWindow::zoomIn()
//
// Purpose:
//   This method is called to increase the scale of the strip chart.
//
// Programmer: Shelly Prevost
// Creation:   Wed Mar 21 16:38:47 PDT 2007
//
// Modifications:
//   Shelly Prevost Fri Dec  1 10:36:07 PST 2006
//   modified to use the strip chart manager
//
//
// ****************************************************************************
void
QvisSimulationWindow::zoomIn()
{
    stripCharts->zoomIn();
}

// ****************************************************************************
// Method: QvisSimulationWindow::zoomOu()
//
// Purpose:
//   This method is called to decrease the scale of the strip chart.
//
// Programmer: Shelly Prevost
// Creation:   Wed Mar 21 16:38:47 PDT 2007
//
// Modifications:
//   Shelly Prevost Fri Dec  1 10:36:07 PST 2006
//   modified to use the strip chart manager
//
// ****************************************************************************
void
QvisSimulationWindow::zoomOut()
{
    stripCharts->zoomOut();
}

// ****************************************************************************
// Method: QvisSimulationWindow::focus()
//
// Purpose:
//   This method is called to center on the last data point in the strip chart.
//
// Programmer: Shelly Prevost
// Creation:   Wed Mar 21 16:38:47 PDT 2007
//
// Modifications:
//   Shelly Prevost Fri Dec  1 10:36:07 PST 2006
//   modified to use the strip chart manager
//
// ****************************************************************************
void
QvisSimulationWindow::focus()
{
    stripCharts->focus();
}


// ****************************************************************************
// Method: QvisSimulationWindow::getColor
//
// Purpose:
//   This method is called to translate a string color into a Qt color.
//
// Programmer: Shelly Prevost
// Creation:   Wed Mar 21 16:38:47 PDT 2007
//
// Modifications:
//
//
// ****************************************************************************
QColor
QvisSimulationWindow::getColor(const QString &color) const
{
    QColor newColor(color);
    if(!newColor.isValid())
        newColor = QColor(Qt::black);
    return newColor;
}
