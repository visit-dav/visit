#include <QvisSimulationWindow.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qmessagebox.h>
#include <qlineedit.h>
#include <qlistview.h>

#include <EngineList.h>
#include <StatusAttributes.h>
#include <ViewerProxy.h>
#include <avtSimulationInformation.h>
#include <avtSimulationCommandSpecification.h>
#include <QualifiedFilename.h>
#include <time.h>
#include <string>
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
    progressLayout->addWidget(totalProgressBar);

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

    QGroupBox *commandGroup = new QGroupBox(central, "commandGroup");
    commandGroup->setTitle("Commands");
    topLayout->addWidget(commandGroup);
    QVBoxLayout *cmdTopLayout = new QVBoxLayout(commandGroup);
    cmdTopLayout->setMargin(10);
    cmdTopLayout->addSpacing(15);

    QGridLayout *buttonLayout2 = new QGridLayout(cmdTopLayout, 3, 3);
    for (int r=0; r<3; r++)
    {
        for (int c=0; c<3; c++)
        {
            cmdButtons[r*3+c] = new QPushButton("", commandGroup);
            buttonLayout2->addWidget(cmdButtons[r*3+c],r,c);
        }
    }
    connect(cmdButtons[0],SIGNAL(clicked()),this,SLOT(executePushButtonCommand0()));
    connect(cmdButtons[1],SIGNAL(clicked()),this,SLOT(executePushButtonCommand1()));
    connect(cmdButtons[2],SIGNAL(clicked()),this,SLOT(executePushButtonCommand2()));
    connect(cmdButtons[3],SIGNAL(clicked()),this,SLOT(executePushButtonCommand3()));
    connect(cmdButtons[4],SIGNAL(clicked()),this,SLOT(executePushButtonCommand4()));
    connect(cmdButtons[5],SIGNAL(clicked()),this,SLOT(executePushButtonCommand5()));
    connect(cmdButtons[6],SIGNAL(clicked()),this,SLOT(executePushButtonCommand6()));
    connect(cmdButtons[7],SIGNAL(clicked()),this,SLOT(executePushButtonCommand7()));
    connect(cmdButtons[8],SIGNAL(clicked()),this,SLOT(executePushButtonCommand8()));
    /*
      simCommandEdit = new QLineEdit(central, "simCommandEdit");
      topLayout->addWidget(simCommandEdit);
      connect(simCommandEdit, SIGNAL(returnPressed()),
              this, SLOT(executeSimCommand()));
    */

    topLayout->addSpacing(10);
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
                QString newsim = sim[i].substr(lastSlashPos+1);
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
//    Jeremy Meredith, Thu Apr 28 18:05:36 PDT 2005
//    Added concept of disabled control command.
//    Added concept of a simulation mode (e.g. running/stopped).
//
// ****************************************************************************

void
QvisSimulationWindow::UpdateInformation(int index)
{
    const stringVector &s = engines->GetEngines();

    // Set the values of the engine information widgets.
    if (index == -1 || s.size() < 1)
    {
        simInfo->clear();
        simInfo->setEnabled(false);
        for (int c=0; c<9; c++)
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
            for (int c=0; c<9; c++)
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
        QString newsim = sim.substr(lastSlashPos+1);
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

        for (int c=0; c<9; c++)
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
                    cmdButtons[c]->setText(md->GetSimInfo().
                            GetAvtSimulationCommandSpecification(c).GetName());
                    cmdButtons[c]->setEnabled(e);
                    cmdButtons[c]->show();
                }
                else
                {
                    cmdButtons[c]->hide();
                }
            }
        }
        
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
