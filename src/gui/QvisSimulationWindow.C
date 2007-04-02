#include <QvisSimulationWindow.h>
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
#include <qmessagebox.h>
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

using std::string;
using std::vector;

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
    topLayout->addWidget(simInfo);

    // Create the status bars.
    //QLabel *totalStatusLabel = new QLabel("Total progress:", central, "totalStatusLabel");
    //topLayout->addWidget(totalStatusLabel);

    simulationMode = new QLabel("Simulation Status: ", central);
    topLayout->addWidget(simulationMode);

    QHBoxLayout *progressLayout = new QHBoxLayout(topLayout);

    progressLayout->addWidget(new QLabel("VisIt Status", central));

    totalProgressBar = new QProgressBar(central, "totalProgressBar");
    totalProgressBar->setTotalSteps(100);
    //progressLayout->addWidget(totalProgressBar);

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

    CreateCommandUI();

    topLayout->addSpacing(10);
}

// ****************************************************************************
// Method: void QvisSimulationWindow::CreateCommandUI()
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
//
// ****************************************************************************

void
QvisSimulationWindow::CreateCommandUI()
{
    int simindex = simCombo->currentItem();
    int index = simulationToEngineListMap[simindex];

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
    connect(cmdButtons[5],SIGNAL(clicked()),this,SLOT(showCommandWindow()));
    
    // get ui filename from value array
    QString key;
    QString uiFilename("defaultUI.ui");

    // Check for a valid engine.
    const stringVector &s = engines->GetEngines();
    if (index == -1 || s.size() < 1)
    {
        QMessageBox::warning(this, "No Valid engines found", "",
                             "Ok", 0, 0, 1);
        QDialog *NoUI = new QDialog(0, "No Custom Interface");
        DynamicCommandsWin = NoUI;
        return;
    }
 
    key.sprintf("%s:%s",
                engines->GetEngines()[index].c_str(),
                engines->GetSimulationName()[index].c_str());
                    
    avtDatabaseMetaData *md = metadataMap[key];
    QListViewItem *item;

    const vector<string> &names  = md->GetSimInfo().GetOtherNames();
    const vector<string> &values = md->GetSimInfo().GetOtherValues();
                  
    for (int i=0; i<names.size(); i++)
    {
        if ( !strcmp(names[i].c_str(),"uiFile"))  uiFilename = values[i];
    }

    if ((simindex < 0)  || (uiFilename == "defaultUI.ui"))
    {
        QMessageBox::warning(this, "VisIt", "Custom UI NOT Found",
                             "Ok", 0, 0, 0, 1);
    }

    // Now get the directory prefix for the file name    
    QString fname;
    // First look in user defined variable
    QString dirName(getenv("VISITSIMDIR"));
    // if still not defined then look in the users home directory
    if (dirName.isEmpty())
    {
        dirName = getenv("HOME");
        if (!dirName.isEmpty()) dirName += "/.visit/UI/";
    }
    // if not defined then look in global location
    if ( dirName.isEmpty())
    {
        dirName = getenv("VISITDIR");
        if (!dirName.isEmpty()) dirName += "/UI/";
    }
    
    fname = dirName + uiFilename;
    debug5 << "UI_DIR = " << fname.latin1() << endl;

    // Dynamically create the custom UI be reading in it's xml file.
    SimCommandSlots *CommandConnections = new SimCommandSlots(viewer,
        engines, index);
    DynamicCommandsWin  = QWidgetFactory::create(fname, CommandConnections);
    
    // if creation failed then jump out 
    if (DynamicCommandsWin == NULL)
    {
        cmdButtons[5]->setEnabled(false);
        QMessageBox::warning(this, "VisIt File NOT Found", fname.latin1(),
                             "Ok", 0, 0, 1);
        QDialog *NoUI = new QDialog(0, "No Custom Interface");
        DynamicCommandsWin = NoUI;
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
            //connect(ui, SIGNAL(dialMoved(int)), CommandConnections,
            //        SLOT(valueChangedHandler(int)));
    }

    // enable custom command UI button
    debug5 << "enabling custom command button" << endl;
    cmdButtons[5]->setEnabled(true);
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
//   cmd : command specification that update the UI component
//
// Programmer: Shelly Prevost
// Creation:   December 21, 2005
//
// Modifications:
//
// ****************************************************************************

void 
QvisSimulationWindow::UpdateCustomUIComponent (avtSimulationCommandSpecification *cmd)
{
    QObject *ui = NULL;
    ui  = DynamicCommandsWin->child(cmd->GetName().c_str());
    if (ui)
    {
        debug5 << "Looking up component = " << cmd->GetName().c_str() << endl;
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
                   << " text = " << cmd->GetText().c_str() << endl;
            const QString label(cmd->GetValue().c_str());
            ((QProgressBar*)ui)->setProgress(label.toInt());
        }

        if (ui->isA("QSpinBox"))
        {
            debug5 << "found QSpinBox " << cmd->GetName().c_str() << " text = "
                   << cmd->GetValue().c_str() << endl;
            const QString label(cmd->GetValue().c_str());
            ((QSpinBox*)ui)->setValue(label.toInt());
            ((QSpinBox*)ui)->setEnabled(cmd->GetEnabled());
        }

        if (ui->isA("QDial"))
        {
            debug5 << "found QDial " << cmd->GetName().c_str() << " text = "
                   << cmd->GetValue().c_str() << endl;
            const QString label(cmd->GetValue().c_str());
            ((QDial*)ui)->setValue(label.toInt());
            ((QDial*)ui)->setEnabled(cmd->GetEnabled());
        }

        if (ui->isA("QSlider"))
        {
            debug5 << "found QSlider " << cmd->GetName().c_str() << " text = "
                   << cmd->GetValue().c_str() << endl;
            const QString label(cmd->GetValue().c_str());
            ((QSlider*)ui)->setValue(label.toInt());
            ((QSlider*)ui)->setEnabled(cmd->GetEnabled());
        }

        if (ui->isA("QTextEdit"))
        {
            debug5 << "found QTextEdit " << cmd->GetName().c_str()
                   << " text = " << cmd->GetText().c_str() << endl;
            const QString label(cmd->GetText().c_str());
            ((QTextEdit*)ui)->setText(label);
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
            host = viewer->GetLocalHostName();

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
//
// ****************************************************************************

void
QvisSimulationWindow::UpdateWindow(bool doAll)
{
    if (caller == engines || doAll)
    {
        const stringVector &host = engines->GetEngines();
        const stringVector &sim  = engines->GetSimulationName();

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
//   Updates the ui components in the Costom UI popup.
//
// Arguments:
//   md : meta data from the simulation.
//
// Programmer: Shelly Prevost
// Creation:   December 9, 2005
//
// Modifications:
//
// ****************************************************************************

void
QvisSimulationWindow::UpdateCustomUI (avtDatabaseMetaData *md)
{
    int numCustCommands = md->GetSimInfo().GetNumAvtSimulationCustCommandSpecifications();
    // loop thru all command updates and updates the matching UI component.
    for (int c=0; c<numCustCommands; c++)
    {
        UpdateCustomUIComponent (&(md->GetSimInfo().GetAvtSimulationCustCommandSpecification(c)));
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
        for (int c=0; c<6; c++)
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
            for (int c=0; c<6; c++)
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

        for (int c=0; c<6; c++)
        {
            if (md->GetSimInfo().GetNumAvtSimulationCommandSpecifications()<=c)
            {
                cmdButtons[c]->hide();
            }
            else
            {
                avtSimulationCommandSpecification::CommandArgumentType t;
                t = md->GetSimInfo().GetAvtSimulationCommandSpecification(c).GetArgumentType();
                bool e = md->GetSimInfo().GetAvtSimulationCommandSpecification(c).GetEnabled();
                if (t == avtSimulationCommandSpecification::CmdArgNone)
                {
                    cmdButtons[c]->setText(QString(md->GetSimInfo().
                            GetAvtSimulationCommandSpecification(c).
                            GetName().c_str()));
                    // **** MSP **** remove after fixing custome UI enabling
                    if ( c != 5 )cmdButtons[c]->setEnabled(e);
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
        viewer->CloseComputeEngine(host, sim);
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

    viewer->InterruptComputeEngine(host, sim);
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

    if (viewer->GetLocalHostName() == host)
        viewer->ClearCache("localhost", sim);
    else
        viewer->ClearCache(host, sim);
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
//
// ****************************************************************************

void QvisSimulationWindow::showCommandWindow()
{
    if (DynamicCommandsWin != NULL)
        DynamicCommandsWin->show();
    else
        QMessageBox::warning(this, "VisIt", "Custom UI NOT Found", "Ok", 0,
                             0, 0, 1);
}

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
        viewer->SendSimulationCommand(host, sim, cmd.latin1());
        simCommandEdit->clear();
    }
}

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
    if (!cmd.isEmpty())
    {
        viewer->SendSimulationCommand(host, sim, cmd.latin1());
    }
}

void
QvisSimulationWindow::executePushButtonCommand0()
{
    executePushButtonCommand(0);
}

void
QvisSimulationWindow::executePushButtonCommand1()
{
    executePushButtonCommand(1);
}

void
QvisSimulationWindow::executePushButtonCommand2()
{
    executePushButtonCommand(2);
}

void
QvisSimulationWindow::executePushButtonCommand3()
{
    executePushButtonCommand(3);
}

void
QvisSimulationWindow::executePushButtonCommand4()
{
    executePushButtonCommand(4);
}

void
QvisSimulationWindow::executePushButtonCommand5()
{
    executePushButtonCommand(5);
}

void
QvisSimulationWindow::executePushButtonCommand6()
{
    executePushButtonCommand(6);
}

void
QvisSimulationWindow::executePushButtonCommand7()
{
    executePushButtonCommand(7);
}

void
QvisSimulationWindow::executePushButtonCommand8()
{
    executePushButtonCommand(8);
}
