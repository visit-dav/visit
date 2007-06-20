/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <QvisSimulationWindow.h>
#include <QvisStripChart.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qcheckbox.h>
#include <qdial.h>
#include <qslider.h>
#include <qlayout.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qvgroupbox.h>
#include <qmessagebox.h>
#include <qtextedit.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qlcdnumber.h>
#include <qwidgetfactory.h>
#include <qobjectlist.h>
#include <qtable.h>
#include <qprogressbar.h>
#include <qradiobutton.h> 
#include <qspinbox.h>
#include <qmetaobject.h>
#include <qdatetimeedit.h>
#include <qfile.h>
#include <qscrollview.h>
#include <qsplitter.h>
#include <qcolor.h>

#include <DebugStream.h>
#include <EngineList.h>
#include <SimCommandSlots.h>
#include <StatusAttributes.h>
#include <ViewerProxy.h>
#include <avtSimulationInformation.h>
#include <avtSimulationCommandSpecification.h>
#include <QualifiedFilename.h>
#include <time.h>
#include <string>
#include <SimWidgetNames.h>

using std::string;
using std::vector;

#define CUSTOM_BUTTON 5
#define NUM_GENRIC_BUTTONS 6

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
// ****************************************************************************

QvisSimulationWindow::QvisSimulationWindow(EngineList *engineList,
    const char *caption, const char *shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(engineList, caption, shortName, notepad,
                               QvisPostableWindowObserver::NoExtraButtons),
    activeEngine(""), statusMap()
{
    engines = engineList;
    caller = engines;
    statusAtts = 0;
    metadata = new avtDatabaseMetaData;
    DynamicCommandsWin = NULL;
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
//   
// ****************************************************************************

QvisSimulationWindow::~QvisSimulationWindow()
{
    // Delete the status attributes in the status map.
    SimulationStatusMap::Iterator pos;
    for (pos = statusMap.begin(); pos != statusMap.end(); ++pos)
    {
        delete pos.data();
    }

    // Detach from the status atts if they are still around.
    if (statusAtts)
        statusAtts->Detach(this);
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
// ****************************************************************************

void
QvisSimulationWindow::CreateWindowContents()
{
    isCreated = true;

    QGridLayout *grid1 = new QGridLayout(topLayout, 2, 2);
    grid1->setColStretch(1, 10);

    simCombo = new QComboBox(central, "simCombo");
    connect(simCombo, SIGNAL(activated(int)), this, SLOT(selectEngine(int)));
    grid1->addWidget(simCombo, 0, 1);
    QLabel *engineLabel = new QLabel(simCombo,
                                     "Simulation:", central, "engineLabel");
    grid1->addWidget(engineLabel, 0, 0);

    // Create the widgets needed to show the engine information.
    simInfo = new QListView(central, "simInfo");
    simInfo->addColumn("Attribute");
    simInfo->addColumn("Value");
    simInfo->setAllColumnsShowFocus(true);
    simInfo->setResizeMode(QListView::AllColumns);
    topLayout->addWidget(simInfo, 10);

    simulationMode = new QLabel("Simulation Status: ", central);
    topLayout->addWidget(simulationMode);  

    QGridLayout *timeLayout = new QGridLayout(topLayout);
    QHBoxLayout *progressLayout2 = new QHBoxLayout(topLayout);

    progressLayout2->addWidget(new QLabel("VisIt Status", central));

    totalProgressBar = new QProgressBar(central, "totalProgressBar");
    totalProgressBar->setTotalSteps(100);
    progressLayout2->addWidget(totalProgressBar);
    
    startCycle = new QLineEdit(central,"StartLineEdit");
    startCycle->setEnabled(false); 
    startLabel = new QLabel(central,"StartLabel");
    startLabel->setText("Start");
    timeLayout->addWidget(startLabel,0,0);
    timeLayout->addWidget(startCycle,0,1);
    connect(startCycle,SIGNAL(returnPressed()),this,SLOT(executeSpinBoxStartCommand()));

    stepCycle = new QLineEdit(central,"StepLineEdit");
    stepCycle->setEnabled(false);    
    stepLabel = new QLabel(central,"StepLabel");
    stepLabel->setText("Step");
    timeLayout->addWidget(stepLabel,0,2);
    timeLayout->addWidget(stepCycle,0,3);
    connect(stepCycle,SIGNAL(returnPressed()),this,SLOT(executeSpinBoxStepCommand()));
    
    stopCycle = new QLineEdit(central,"StopLineEdit");
    stopCycle->setEnabled(false);    
    stopLabel = new QLabel(central,"StopLabel");
    stopLabel->setText("Stop");
    timeLayout->addWidget(stopLabel,0,4);
    timeLayout->addWidget(stopCycle,0,5);
    connect(stopCycle,SIGNAL(returnPressed()),this,SLOT(executeSpinBoxStopCommand()));

    enableTimeRange = new QCheckBox(central);
    enableTimeRange->setText("Enable Time Ranging");
    connect(enableTimeRange,SIGNAL(stateChanged(int)),this,SLOT(executeEnableTimeRange()));
    timeLayout->addMultiCellWidget(enableTimeRange,1,1,0,2);

    QGridLayout *buttonLayout1 = new QGridLayout(topLayout, 1, 3);
    buttonLayout1->setSpacing(10);
    interruptEngineButton = new QPushButton("Interrupt", central, "interruptEngineButton");
    connect(interruptEngineButton, SIGNAL(clicked()), this, SLOT(interruptEngine()));
    interruptEngineButton->setEnabled(false);
    buttonLayout1->addWidget(interruptEngineButton, 0, 0);
    buttonLayout1->setColStretch(1, 10);

    closeEngineButton = new QPushButton("Disconnect", central, "closeEngineButton");
    connect(closeEngineButton, SIGNAL(clicked()), this, SLOT(closeEngine()));
    closeEngineButton->setEnabled(false);
    buttonLayout1->addWidget(closeEngineButton, 0, 2);

    clearCacheButton = new QPushButton("Clear cache", central, "clearCacheButton");
    connect(clearCacheButton, SIGNAL(clicked()), this, SLOT(clearCache()));
    clearCacheButton->setEnabled(false);
    buttonLayout1->addWidget(clearCacheButton, 0, 1);

    // Create the group box and generic buttons.
    QGroupBox *commandGroup = new QGroupBox(central, "commandGroup");
    commandGroup->setTitle("Commands");
    topLayout->addWidget(commandGroup);
    QVBoxLayout *cmdTopLayout = new QVBoxLayout(commandGroup);
    cmdTopLayout->setMargin(10);
    cmdTopLayout->addSpacing(15);
    QGridLayout *buttonLayout2 = new QGridLayout(cmdTopLayout, 3, 3);
    for (int r=0; r<2; r++)
    {
        for (int c=0; c<3; c++)
        {
            cmdButtons[r*3+c] = new QPushButton("", commandGroup);
            buttonLayout2->addWidget(cmdButtons[r*3+c],r,c);
        }
    }

    // connect up the generic buttons
    connect(cmdButtons[0],SIGNAL(clicked()),this,SLOT(executePushButtonCommand0()));
    connect(cmdButtons[1],SIGNAL(clicked()),this,SLOT(executePushButtonCommand1()));
    connect(cmdButtons[2],SIGNAL(clicked()),this,SLOT(executePushButtonCommand2()));
    connect(cmdButtons[3],SIGNAL(clicked()),this,SLOT(executePushButtonCommand3()));
    connect(cmdButtons[4],SIGNAL(clicked()),this,SLOT(executePushButtonCommand4()));
    connect(cmdButtons[CUSTOM_BUTTON],SIGNAL(clicked()),this,SLOT(showCommandWindow()));
   
    topLayout->addSpacing(10);

    // create splitter to hold Stip chart widgets and message widgets
    QSplitter *s2 = new QSplitter (central);
    s2->setOrientation(QSplitter::Vertical);
    s2->show();

    // Create the status message widgets.
    QLabel *messageLabel = new QLabel(central,"MessageViewerLabel");
    messageLabel->setText("Message Viewer");
    topLayout->addWidget(messageLabel);

    QTextEdit *messageViewer = new QTextEdit(s2, MESSAGE_WIDGET_NAME);
    s2->setResizeMode(messageViewer,QSplitter::Auto);
    messageViewer->setReadOnly( true );
    messageViewer->setMaximumHeight( 100 );
    topLayout->addWidget(messageViewer);
 
    stripChart = new VisItSimStripChart(central,STRIP_CHART_WIDGET_NAME,4000,1000);
    sc = new QScrollView(s2,"StipChartScrollWindow");
    s2->setResizeMode(sc,QSplitter::Auto);
    sc->setCaption( "VisIt Strip Chart");                                     
    sc->addChild(stripChart);
    topLayout->addWidget(sc);
    sc->show();

    topLayout->addSpacing(10);
    // create the strip chart widgets
    // Create the group box and generic buttons.
    QGroupBox *stripChartGroup = new QGroupBox(s2, "StripChartGroup");
    s2->setResizeMode(stripChartGroup,QSplitter::Auto);
    stripChartGroup->setTitle("Strip Chart");

    chartLayout =  new QGridLayout(stripChartGroup);              
    chartLayout->setMargin(20);
    chartLayout->setSpacing(10);
 
    minLimitEdit = new QLineEdit(stripChartGroup,STRIP_MIN_LIMIT_WIDGET_NAME);
    minLimitEdit->setEnabled(false);
    minLimitLabel = new QLabel(stripChartGroup,"MinLimitLabel");
    minLimitLabel->setText("Min Limit");
    chartLayout->addWidget(minLimitLabel,1,0);
    chartLayout->addWidget(minLimitEdit,1,1);
    connect(minLimitEdit,SIGNAL(textChanged(const QString&)),this,SLOT(executeMinLimitStripChart()));
    
    maxLimitEdit = new QLineEdit(stripChartGroup,STRIP_MAX_LIMIT_WIDGET_NAME);
    maxLimitEdit->setEnabled(false); 
    maxLimitLabel = new QLabel(stripChartGroup,"MaxLimitLabel");
    maxLimitLabel->setText("Max Limit");
    chartLayout->addWidget(maxLimitLabel,1,2);
    chartLayout->addWidget(maxLimitEdit,1,3);
    connect(maxLimitEdit,SIGNAL(textChanged(const QString&)),this,SLOT(executeMaxLimitStripChart()));

    minEdit = new QLineEdit(stripChartGroup,STRIP_MIN_WIDGET_NAME );
    minEdit->setEnabled(false);
    minEdit->setText("0.0");
    minLabel = new QLabel(stripChartGroup,"MinLabel");
    minLabel->setText("Min value");
    chartLayout->addWidget(minLabel,0,0);
    chartLayout->addWidget(minEdit,0,1);

    maxEdit = new QLineEdit(stripChartGroup,STRIP_MIN_WIDGET_NAME);
    maxEdit->setEnabled(false);
    maxEdit->setText("0.0");
    maxLabel = new QLabel(stripChartGroup,"MaxLabel");
    maxLabel->setText("Max value");
    chartLayout->addWidget(maxLabel,0,2);
    chartLayout->addWidget(maxEdit,0,3);
    
    enableStripChartLimits = new QCheckBox(stripChartGroup,"EnableStripChartLimits");
    enableStripChartLimits->setText("Enable limits");
    connect(enableStripChartLimits,SIGNAL(stateChanged(int)),this,SLOT(executeEnableStripChartLimits()));
    chartLayout->addMultiCellWidget(enableStripChartLimits,2,2,0,1);

    // zoom and focus buttons
    // Create the group box and generic buttons.
    QGridLayout *zoomLayout = new QGridLayout(stripChartGroup, 1, 3);
    plusButton = new QPushButton("Zoom In",stripChartGroup);
    plusButton->setEnabled(true);
    connect(plusButton,SIGNAL(clicked()),this,SLOT(zoomIn()));
    zoomLayout->addWidget(plusButton,0,0);
    minusButton = new QPushButton("Zoom Out",stripChartGroup);
    minusButton->setEnabled(true);
    connect(minusButton,SIGNAL(clicked()),this,SLOT(zoomOut()));
    zoomLayout->addWidget(minusButton,0,2);
    focusButton = new QPushButton("Focus",stripChartGroup);
    focusButton->setEnabled(true);
    connect(focusButton,SIGNAL(clicked()),this,SLOT(focus()));
    zoomLayout->addWidget(focusButton,0,1);
    chartLayout->addLayout(zoomLayout,2,3);

    stripChartGroup->adjustSize();
    topLayout->addWidget(s2);
    sc->horizontalScrollBar()->setValue(sc->horizontalScrollBar()->maxValue());
    sc->updateContents();
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
    int simindex = simCombo->currentItem();
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
        cmdButtons[CUSTOM_BUTTON]->setEnabled(false);
        return;
    }
    
    debug5 << "UI_DIR = " << fname.latin1() << endl;

    // Dynamically create the custom UI be reading in it's xml file.
    int simindex = simCombo->currentItem();
    int index = simulationToEngineListMap[simindex];
    SimCommandSlots *CommandConnections = new SimCommandSlots(GetViewerProxy(),
        engines, index);
    DynamicCommandsWin  = QWidgetFactory::create(fname, CommandConnections);
    
    // If creation failed then jump out 
    if (DynamicCommandsWin == NULL)
    {
        cmdButtons[CUSTOM_BUTTON]->setEnabled(false);
        QString msg;
        msg.sprintf("VisIt could not locate the simulation's "
                    "user interface creation file at: %s. The custom user "
                    "interface for this simulation will be unavailable.",
                    fname.latin1());
        Error(msg);
        return;
    }
     
    const QObjectList *GUI_Objects  = DynamicCommandsWin->queryList();

    // Connect up handlers to all signals based on component type.
    QObject *ui = NULL;
    for (ui =  (( QObjectList *)GUI_Objects)->first(); ui;
        ui = (( QObjectList *)GUI_Objects)->next())
    {
        QMetaObject *mo = ui->metaObject();       
        QStrList thesSignalList = mo->signalNames(true);
        if (thesSignalList.find("clicked()") != -1)
            connect(ui, SIGNAL(clicked()), CommandConnections,
                   SLOT(ClickedHandler()));
        if (thesSignalList.find("valueChanged(int)") != -1)
            connect(ui, SIGNAL(valueChanged(int)), CommandConnections,
                    SLOT(ValueChangedHandler(int)));
        if (thesSignalList.find("valueChanged(const QDate&)") != -1)
            connect(ui, SIGNAL(valueChanged(const QDate&)), CommandConnections,
                    SLOT(ValueChangedHandler(const QDate &)));
        if (thesSignalList.find("valueChanged(const QTime&)") != -1)
            connect(ui, SIGNAL(valueChanged(const QTime&)), CommandConnections,
                    SLOT(ValueChangedHandler(const QTime &)));
        if (thesSignalList.find("stateChanged(int)") != -1)
            connect(ui, SIGNAL(stateChanged(int)), CommandConnections,
                    SLOT(StateChangedHandler(int)));
        if (thesSignalList.find("activated(int)") != -1)
            connect(ui, SIGNAL(activated(int)), CommandConnections,
                    SLOT(ActivatedHandler(int)));
        if (thesSignalList.find("textChanged(const QString&)") != -1)
            connect(ui, SIGNAL(textChanged(const QString &)),
                    CommandConnections,
                    SLOT(TextChangedHandler(const QString&)));
        if (thesSignalList.find("currentChanged(int,int)") != -1)
            connect(ui, SIGNAL(currentChanged(int, int)), CommandConnections,
                    SLOT(CurrentChangedHandler(int, int)));       
        if (thesSignalList.find("valueChanged(int,int)") != -1)
            connect(ui, SIGNAL(valueChanged(int, int)), CommandConnections,
                    SLOT(ValueChangedHandler(int, int)));
            //connect(ui, SIGNAL(dialMoved(int)), CommandConnections,
            //        SLOT(valueChangedHandler(int)));
    }

    // enable custom command UI button
    debug5 << "enabling custom command button" << endl;
    cmdButtons[CUSTOM_BUTTON]->setEnabled(true);
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
// ****************************************************************************

void 
QvisSimulationWindow::UpdateUIComponent (QWidget *window, avtSimulationCommandSpecification *cmd)
{
    if(window == NULL)
        return;

    QObject *ui = NULL;
    ui  = window->child(cmd->GetName().c_str());
    if (ui)
    {
        debug5 << "Looking up component = " << cmd->GetName().c_str() << endl;

        // Block signals so updating the user interface does not cause a
        // command to go back to the simulation.
        ui->blockSignals(true);

        if (ui->isA("QLabel"))
        {
            debug5 << "found label " << cmd->GetName().c_str() << " text = "
                   << cmd->GetText().c_str() << endl;
            const QString label(cmd->GetText().c_str());
            ((QLabel*)ui)->setText(label );
        }

        // just set the generic button attributes. more specific attributes will be set latter
        if (ui->inherits("QButton"))
        {
            debug5 << "found button " << cmd->GetName().c_str() << " text = "
                   << cmd->GetText().c_str() << endl;
            const QString label(cmd->GetText().c_str());
            ((QButton*)ui)->setText(label );
            ((QButton*)ui)->setEnabled(cmd->GetEnabled());
        }

        if (ui->isA("QPushButton"))
        {
            debug5 << "found button" << cmd->GetName().c_str() << " text = "
                   << cmd->GetText().c_str() << endl;
            const QString label(cmd->GetText().c_str());
            ((QButton*)ui)->setText(label);
            ((QButton*)ui)->setEnabled(cmd->GetEnabled()); 
        }

        if (ui->isA("QTabWidget"))
        {
            debug5 << "found button " << cmd->GetName().c_str() << " text = "
                   << cmd->GetText().c_str() << endl;
            const QString label(cmd->GetText().c_str());
            ((QButton*)ui)->setText(label);
            ((QButton*)ui)->setEnabled(cmd->GetEnabled());
        }

        if (ui->isA( "QLineEdit"))
        {
            debug5 << "found button " << cmd->GetName().c_str() << " text = "
                   << cmd->GetText().c_str() << endl;
            const QString label(cmd->GetText().c_str());
            ((QLineEdit*)ui)->setText(label );
            ((QLineEdit*)ui)->setEnabled(cmd->GetEnabled());
        }

        if (ui->isA("QRadioButton"))
        {
            debug5 << "found button " << cmd->GetName().c_str() << " text = "
                   << cmd->GetText().c_str() << endl;
            const QString label(cmd->GetText().c_str());
            ((QButton*)ui)->setText(label);
            ((QRadioButton*)ui)->setChecked(cmd->GetIsOn());
            ((QButton*)ui)->setEnabled(cmd->GetEnabled());
        }

        if (ui->isA("QProgressBar"))
        {
            debug5 << "found ProgressBar " << cmd->GetName().c_str()
                   << " value = " << cmd->GetText().c_str() << endl;
            const QString label(cmd->GetValue().c_str());
            ((QProgressBar*)ui)->setProgress(label.toInt());
        }

        if (ui->isA("QSpinBox"))
        {
            debug5 << "found QSpinBox " << cmd->GetName().c_str() << " value = "
                   << cmd->GetValue().c_str() << endl;
            const QString label(cmd->GetValue().c_str());
            ((QSpinBox*)ui)->setValue(label.toInt());
            ((QSpinBox*)ui)->setEnabled(cmd->GetEnabled());
        }
 
        if (ui->isA("QDial"))
        {
            debug5 << "found QDial " << cmd->GetName().c_str() << " value = "
                   << cmd->GetValue().c_str() << endl;
            const QString label(cmd->GetValue().c_str());
            ((QDial*)ui)->setValue(label.toInt());
            ((QDial*)ui)->setEnabled(cmd->GetEnabled());
        }

        if (ui->isA("QSlider"))
        {
            debug5 << "found QSlider " << cmd->GetName().c_str() << " value = "
                   << cmd->GetValue().c_str() << endl;
            const QString label(cmd->GetValue().c_str());
            ((QSlider*)ui)->setValue(label.toInt());
            ((QSlider*)ui)->setEnabled(cmd->GetEnabled());
        }

        if (ui->isA("QTextEdit"))
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

        if ( ui->isA ( "QLineEdit"))
        {
            debug5 << "found QTextEdit " << cmd->GetName().c_str()
                   << " text = " << cmd->GetText().c_str() << endl;
            const QString label(cmd->GetText().c_str());
            ((QLineEdit*)ui)->setText(label);
            ((QLineEdit*)ui)->setEnabled(cmd->GetEnabled());
        }

        if ( ui->isA ( "QLCDNumber"))
        {
            debug5 << "found QLCDNumber " << cmd->GetName().c_str()
                   << " value = " << cmd->GetValue().c_str() << endl;
            const QString label(cmd->GetValue().c_str());
            ((QLCDNumber*)ui)->display(label);
            ((QLCDNumber*)ui)->setEnabled(cmd->GetEnabled());
        }

        if ( ui->isA ( "QTimeEdit"))
        {
            debug5 << "found QTimeEdit " << cmd->GetName().c_str()
                   << " text = " << cmd->GetText().c_str() << endl;
            const QString label(cmd->GetText().c_str());
            QTime time1 = QTime::fromString(cmd->GetText().c_str());
            ((QTimeEdit*)ui)->setTime(time1);
            ((QTimeEdit*)ui)->setEnabled(cmd->GetEnabled());
        }

        if (ui->isA("QDateEdit"))
        {
            debug5 << "found QTDateEdit " << cmd->GetName().c_str()
                   << " text = " << cmd->GetText().c_str() << endl;
            const QString label(cmd->GetText().c_str());
            QDate date = QDate::fromString( label );
            ((QDateEdit*)ui)->setDate(date);
            ((QDateEdit*)ui)->setEnabled(cmd->GetEnabled());
        }
        if (ui->isA("QCheckBox"))
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

                int lastSlashPos = QString(sim[i].c_str()).findRev('/');
                QString newsim = QString(sim[i].substr(lastSlashPos+1).c_str());
                int lastDotPos =  newsim.findRev('.');
                int firstDotPos =  newsim.find('.');

                QString name = newsim.mid(firstDotPos+1,
                                          lastDotPos-firstDotPos-1);

                temp = QString().sprintf("%s on %s", 
                                    name.latin1(), host[i].c_str());
                simCombo->insertItem(temp);

                if (temp == activeEngine)
                    current = i;

            }
        }

        if (current == -1)
        {
            // Update the activeEngine string.
            if (simCombo->count() > 0)
            {
                simCombo->setCurrentItem(0);
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
            simCombo->setCurrentItem(current);
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
    for (int c=NUM_GENRIC_BUTTONS; c<numCommands; c++)
    {
        UpdateUIComponent (this,&(md->GetSimInfo().GetGenericCommands(c)));
        SpecialWidgetUpdate (&(md->GetSimInfo().GetGenericCommands(c)));
    }
}

// ****************************************************************************
// Method: QvisSimulationWindow::setMinMaxStripChartDataDisplaye
//
// Purpose:
//   sing.
//
// Arguments:
//   cmd:  ui data information
//
// Programmer: Shelly Prevost
// Creation:   Tue Nov 28 17:12:04 PST 2006
//
// Modifications:
//
// ****************************************************************************

void
QvisSimulationWindow::setMinMaxStripChartDataDisplay (double minY, double maxY)
{
  minEdit->setText(QString::number(minY));
  maxEdit->setText(QString::number(maxY));

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
//
// ****************************************************************************

void
QvisSimulationWindow::SpecialWidgetUpdate (avtSimulationCommandSpecification *cmd)
{
    QObject *ui = NULL;
    ui  = this->child(cmd->GetName().c_str());
    if ( !strcmp (STRIP_CHART_WIDGET_NAME,cmd->GetName().c_str()))
    {    double maxY;
         double minY;
         const QString dataX(cmd->GetText().c_str());
         const QString dataY(cmd->GetValue().c_str());
         stripChart->setEnable(cmd->GetEnabled());
         bool outOfBounds = stripChart->addDataPoint(dataX.toDouble(),dataY.toDouble());   
         stripChart->update();
         stripChart->getMinMaxData( minY, maxY);
         setMinMaxStripChartDataDisplay( minY,maxY);
 
         if ( outOfBounds )
         {
            QString cmd("ALERT;StripChart;Data;OutOfBounds;");
            int simIndex = simCombo->currentItem();
            ViewerSendCMD ( simIndex, cmd);
         }
    }
       if ( !strcmp (MESSAGE_WIDGET_NAME,cmd->GetName().c_str()))
    {
         const QString dataX(cmd->GetText().c_str());
         const QString dataY(cmd->GetValue().c_str());
         QColor *msgColor =  getColor(dataY);
         if ( ui != NULL && dataX != "")
             ((QTextEdit*)ui)->setColor(*msgColor);
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
//
// ****************************************************************************

void
QvisSimulationWindow::UpdateStatusArea()
{
    SimulationStatusMap::Iterator pos;
    if ((pos = statusMap.find(activeEngine)) == statusMap.end())
        return;

    StatusAttributes *s = pos.data();
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
        totalProgressBar->setProgress(total);
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
        for (int c=0; c<NUM_GENRIC_BUTTONS; c++)
        {
            cmdButtons[c]->hide();
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
            for (int c=0; c<NUM_GENRIC_BUTTONS; c++)
            {
                cmdButtons[c]->hide();
            }
            return;
        }

        simInfo->clear();
        avtDatabaseMetaData *md = metadataMap[key];
       
        QString tmp1,tmp2;
        QListViewItem *item;

        item = new QListViewItem(simInfo, "Host",
                                 md->GetSimInfo().GetHost().c_str());
        simInfo->insertItem(item);

        int lastSlashPos = QString(sim.c_str()).findRev('/');
        QString newsim = QString(sim.substr(lastSlashPos+1).c_str());
        int lastDotPos =  newsim.findRev('.');
        int firstDotPos =  newsim.find('.');

        QString name = newsim.mid(firstDotPos+1,
                                  lastDotPos-firstDotPos-1);
        item = new QListViewItem(simInfo, "Name", name);
        simInfo->insertItem(item);


        QString timesecstr = newsim.left(firstDotPos);
        time_t timesec = atoi(timesecstr.latin1());
        QString timestr = ctime(&timesec);
        item = new QListViewItem(simInfo, "Date",
                                 timestr.left(timestr.length()-1).latin1());
        simInfo->insertItem(item);

        tmp1.sprintf("%d", np);
        item = new QListViewItem(simInfo, "Num Processors", tmp1);
        simInfo->insertItem(item);

        const vector<string> &names  = md->GetSimInfo().GetOtherNames();
        const vector<string> &values = md->GetSimInfo().GetOtherValues();

        for (int i=0; i<names.size(); i++)
        {
            item = new QListViewItem(simInfo,
                                     names[i].c_str(), values[i].c_str());
            simInfo->insertItem(item);
        }

        debug5 << "Updating Status information" << endl;
        switch (md->GetSimInfo().GetMode())
        {
          case avtSimulationInformation::Unknown:
            simulationMode->setText("Simulation Status: ");
            break;
          case avtSimulationInformation::Running:
            simulationMode->setText("Simulation Status: Running");
            break;
          case avtSimulationInformation::Stopped:
            simulationMode->setText("Simulation Status: Stopped");
            break;
        }

        // If we've not created a dynamic commands window already and we can get a
        // decent-looking UI filename, enabled the custom command button
        // so we can create a window when that button is clicked.
        if(DynamicCommandsWin == NULL)
        {
            QString fname(GetUIFile());
            cmdButtons[CUSTOM_BUTTON]->setEnabled(!fname.isEmpty());
        }

        for (int c=0; c<NUM_GENRIC_BUTTONS; c++)
        {
            if (md->GetSimInfo().GetNumGenericCommands()<=c)
            {
                cmdButtons[c]->hide();
            }
            else
            {
                avtSimulationCommandSpecification::CommandArgumentType t;
                t = md->GetSimInfo().GetGenericCommands(c).GetArgumentType();
                bool e = md->GetSimInfo().GetGenericCommands(c).GetEnabled();
                if (t == avtSimulationCommandSpecification::CmdArgNone)
                {
                    QString bName = QString(md->GetSimInfo().GetGenericCommands(c).GetName().c_str());
                    cmdButtons[c]->setText(bName);
                    if ( c != CUSTOM_BUTTON  )cmdButtons[c]->setEnabled(e);
                    cmdButtons[c]->show();
                }
                else
                {
                    cmdButtons[c]->hide();
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
    delete pos.data();
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
        *(pos.data()) = *statusAtts;
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
    delete pos.data();
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
        *(pos.data()) = *metadata;
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
//
// ****************************************************************************

void
QvisSimulationWindow::closeEngine()
{
    int index = simCombo->currentItem();
    if (index < 0)
        return;

    string host = engines->GetEngines()[index];
    string sim  = engines->GetSimulationName()[index];

    // Create a prompt for the user.
    QString msg;
    if (sim == "")
    {
        msg.sprintf("Really close the compute engine on host \"%s\"?\n\n",
                    host.c_str());
    }
    else
    {
        msg.sprintf("Really disconnect from the simulation \"%s\" on "
                    "host \"%s\"?\n\n", sim.c_str(), host.c_str());
    }

    // Ask the user if he really wants to close the engine.
    if (QMessageBox::warning( this, "VisIt",
                             msg.latin1(),
                             "Ok", "Cancel", 0,
                             0, 1 ) == 0)
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
//   
// ****************************************************************************

void
QvisSimulationWindow::interruptEngine()
{
    int index = simCombo->currentItem();
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
//
// ****************************************************************************

void
QvisSimulationWindow::clearCache()
{
    int index = simCombo->currentItem();
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
//    Brad Whitlock, Tue Jan 31 15:54:43 PST 2006
//    I made it create the commands window on the fly instead of creating
//    it at the same time as the regular Window is created.
//
// ****************************************************************************
void
QvisSimulationWindow::executeSimCommand()
{
    int simindex = simCombo->currentItem();
    if (simindex < 0)
        return;
    int index = simulationToEngineListMap[simindex];
    string host = engines->GetEngines()[index];
    string sim  = engines->GetSimulationName()[index];

    QString cmd = simCommandEdit->displayText();
    if (!cmd.isEmpty())
    {
        GetViewerMethods()->SendSimulationCommand(host, sim, cmd.latin1());
        simCommandEdit->clear();
    }
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
void
QvisSimulationWindow::executePushButtonCommand(int bi)
{
    int simindex = simCombo->currentItem();
    if (simindex < 0)
        return;
    int index = simulationToEngineListMap[simindex];
    string host = engines->GetEngines()[index];
    string sim  = engines->GetSimulationName()[index];

    QString cmd = cmdButtons[bi]->text();
    cmd = "clicked();" + cmd + ";QPushButton;Simulations;NONE";
    if (!cmd.isEmpty())
    {
        GetViewerMethods()->SendSimulationCommand(host, sim, cmd.latin1());
    }
}

// ****************************************************************************
// Method: QvisSimulationWindow::executePushButtonCommand
//
// Purpose:
//   This method is called when the user presses one of the generic buttons
//   in the this window.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//   
//
// ****************************************************************************
void
QvisSimulationWindow::executePushButtonCommand0()
{
    executePushButtonCommand(0);
}

// ****************************************************************************
// Method: QvisSimulationWindow::executePushButtonCommand
//
// Purpose:
//   This method is called when the user presses one of the generic buttons
//   in the this window.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//
//
// ****************************************************************************
void
QvisSimulationWindow::executePushButtonCommand1()
{
    executePushButtonCommand(1);
}

// ****************************************************************************
// Method: QvisSimulationWindow::executePushButtonCommand
//
// Purpose:
//   This method is called when the user presses one of the generic buttons
//   in the this window.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//
//
// ****************************************************************************
void
QvisSimulationWindow::executePushButtonCommand2()
{
    executePushButtonCommand(2);
}

// ****************************************************************************
// Method: QvisSimulationWindow::executePushButtonCommand
//
// Purpose:
//   This method is called when the user presses one of the generic buttons
//   in the this window.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//
//
// ****************************************************************************
void
QvisSimulationWindow::executePushButtonCommand3()
{
    executePushButtonCommand(3);
}

// ****************************************************************************
// Method: QvisSimulationWindow::executePushButtonCommand
//
// Purpose:
//   This method is called when the user presses one of the generic buttons
//   in the this window.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//
//
// ****************************************************************************
void
QvisSimulationWindow::executePushButtonCommand4()
{
    executePushButtonCommand(4);
}

// ****************************************************************************
// Method: QvisSimulationWindow::executePushButtonCommand
//
// Purpose:
//   This method is called when the user presses one of the generic buttons
//   in the this window.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//
//
// ****************************************************************************
void
QvisSimulationWindow::executePushButtonCommand5()
{
    executePushButtonCommand(5);
}

// ****************************************************************************
// Method: QvisSimulationWindow::executePushButtonCommand
//
// Purpose:
//   This method is called when the user presses one of the generic buttons
//   in the this window.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//
//
// ****************************************************************************
void
QvisSimulationWindow::executePushButtonCommand6()
{
    executePushButtonCommand(6);
}

// ****************************************************************************
// Method: QvisSimulationWindow::executePushButtonCommand
//
// Purpose:
//   This method is called when the user presses one of the generic buttons
//   in the this window.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//
//
// ****************************************************************************
void
QvisSimulationWindow::executePushButtonCommand7()
{
    executePushButtonCommand(7);
}

// ****************************************************************************
// Method: QvisSimulationWindow::executePushButtonCommand
//
// Purpose:
//   This method is called when the user presses one of the generic buttons
//   in the this window.
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//
//
// ****************************************************************************
void
QvisSimulationWindow::executePushButtonCommand8()
{
    executePushButtonCommand(8);
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
//
//
// ****************************************************************************
void
QvisSimulationWindow::executeEnableTimeRange()
{
    int simindex = simCombo->currentItem();
    if (simindex < 0)
        return;
    int index = simulationToEngineListMap[simindex];
    string host = engines->GetEngines()[index];
    string sim  = engines->GetSimulationName()[index];

    QString cmd = startCycle->text();
    cmd = "clicked();TimeLimitsEnabled;QCheckBox;Simulations;" + cmd;

    if (!cmd.isEmpty())
    {
        GetViewerMethods()->SendSimulationCommand(host, sim, cmd.latin1());
    }
    
    bool enabled = enableTimeRange->isChecked();
    startCycle->setEnabled(enabled);    
    stepCycle->setEnabled(enabled);    
    stopCycle->setEnabled(enabled);    

}

// ****************************************************************************
// Method: QvisSimulationWindow::executeEnableStripChartLimits()
//
// Purpose:
//   This method is called when the user clicks on the enable button
//   for the stip chart ui. It will set the upper and lower bounds for
//   checking simiulation data for out of band problems.
//
// Programmer: Shelly Prevost
// Creation:   Fri Dec  1 10:36:07 PST 2006  
//
// Modifications:
//
//
// ****************************************************************************
void
QvisSimulationWindow::executeEnableStripChartLimits()
{
    int simindex = simCombo->currentItem();
    if (simindex < 0)
        return;
    int index = simulationToEngineListMap[simindex];
    string host = engines->GetEngines()[index];
    string sim  = engines->GetSimulationName()[index];

    QString cmd = enableStripChartLimits->text();
    cmd = "clicked();EnableStripChartLimits;QCheckBox;Simulations;" + cmd;
    GetViewerMethods()->SendSimulationCommand(host, sim, cmd.latin1());

    bool enabled = enableStripChartLimits->isChecked();
    maxLimitEdit->setEnabled(enabled);       
    minLimitEdit->setEnabled(enabled); 
    stripChart->enableOutOfBandLimits(enabled);      

}

// ****************************************************************************
// Method: QvisSimulationWindow::executeSpinBoxStartCommand()
//
// Purpose:
//   This method is called when the types into the start text box for time
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
QvisSimulationWindow::executeSpinBoxStartCommand()
{
    int simindex = simCombo->currentItem();
    if (simindex < 0)
        return;
    int index = simulationToEngineListMap[simindex];
    string host = engines->GetEngines()[index];
    string sim  = engines->GetSimulationName()[index];

    QString cmd = startCycle->text();
    cmd = "returnedPressed();StartCycle;QLineEdit;Simulations;" + cmd;
    
    if (!cmd.isEmpty())
    {
        GetViewerMethods()->SendSimulationCommand(host, sim, cmd.latin1());
    }
}

// ****************************************************************************
// Method: QvisSimulationWindow::executeSpinBoxStepCommand()
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
// ****************************************************************************
void 
QvisSimulationWindow::executeSpinBoxStepCommand()
{
    int simindex = simCombo->currentItem();
    if (simindex < 0)
        return;
    int index = simulationToEngineListMap[simindex];
    string host = engines->GetEngines()[index];
    string sim  = engines->GetSimulationName()[index];

    QString cmd1 = stepCycle->text();
    cmd1 = "returnedPressed();StepCycle;QLineEdit;Simulations;" + cmd1;
    
     if (!cmd1.isEmpty())
    {     
        GetViewerMethods()->SendSimulationCommand(host, sim, cmd1.latin1());
    }
}

// ****************************************************************************
// Method: QvisSimulationWindow::executeSpinBoxStopCommand()
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
QvisSimulationWindow::executeSpinBoxStopCommand()
{
    int simindex = simCombo->currentItem();
    if (simindex < 0)
        return;
    int index = simulationToEngineListMap[simindex];
    string host = engines->GetEngines()[index];
    string sim  = engines->GetSimulationName()[index];
    
    QString cmd2 = stopCycle->text();
    cmd2 = "returnedPressed();StopCycle;QLineEdit;Simulations;" + cmd2;
    if (!cmd2.isEmpty())
    {
        GetViewerMethods()->SendSimulationCommand(host, sim, cmd2.latin1()); 
    }
}

// ****************************************************************************
// Method: QvisSimulationWindow::executeMinLimitStripChart()
//
// Purpose:
//   This method is called when the when the user changes the value
//   in the limit strip chart line edit widget. It set the limits 
//   for the strip chart and sends them to the simulation.
//
// Programmer: Shelly Prevost
// Creation:   Thu Nov 30 17:21:39 PST 2006
//
// Modifications:
//
//
// ****************************************************************************
void 
QvisSimulationWindow::executeMinLimitStripChart()
{
    int simindex = simCombo->currentItem();
    QString cmd2 = minLimitEdit->text();
    QString cmd3 = maxLimitEdit->text();
    stripChart->setOutOfBandLimits( cmd3.toDouble(), cmd2.toDouble());
    cmd2 = "returnedPressed();MinLimitEdit;QLineEdit;Simulations;" + cmd2;
    ViewerSendCMD(simindex, cmd2);
}

// ****************************************************************************
// Method: QvisSimulationWindow::executeMaxLimitStripChart()
//
// Purpose:
//   This method is called when the when the user changes the value
//   in the limit strip chart line edit widget. It set the limits 
//   for the strip chart and sends them to the simulation.
//
// Programmer: Shelly Prevost
// Creation:   Thu Nov 30 17:21:39 PST 2006
//
// Modifications:
//
//
// ****************************************************************************
void 
QvisSimulationWindow::executeMaxLimitStripChart()
{
    int simindex = simCombo->currentItem();
    QString cmd2 = minLimitEdit->text();
    QString cmd3 = maxLimitEdit->text();
    stripChart->setOutOfBandLimits( cmd3.toDouble(), cmd2.toDouble());
    cmd2 = "returnedPressed();MaxLimitEdit;QLineEdit;Simulations;" + cmd3;
    ViewerSendCMD(simindex, cmd2);
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
//
//
// ****************************************************************************

void
QvisSimulationWindow::ViewerSendCMD ( int simIndex, QString cmd)
{
    if (simIndex < 0)
        return;
    int index = simulationToEngineListMap[simIndex];
    string host = engines->GetEngines()[index];
    string sim  = engines->GetSimulationName()[index];
    GetViewerMethods()->SendSimulationCommand(host, sim, cmd.ascii());
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
//
//
// ****************************************************************************
void
QvisSimulationWindow::zoomIn()
{
    stripChart->zoomIn();
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
//
//
// ****************************************************************************
void
QvisSimulationWindow::zoomOut()
{
    stripChart->zoomOut();
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
//
//
// ****************************************************************************
void
QvisSimulationWindow::focus()
{
    stripChart->focus(sc);
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
QColor *
QvisSimulationWindow::getColor(const QString color)
{
  QColor *newColor = new QColor( color );
  if ( newColor->isValid()) return newColor;
  else return new QColor(Qt::black);
}
