#include <QvisEngineWindow.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qmessagebox.h>

#include <EngineList.h>
#include <StatusAttributes.h>
#include <ViewerProxy.h>
#include <string>
#include <string>

using std::string;
using std::vector;

// ****************************************************************************
// Method: QvisEngineWindow::QvisEngineWindow
//
// Purpose: 
//   This is the constructor for the QvisEngineWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 2 16:30:53 PST 2001
//
// Modifications:
//   
// ****************************************************************************

QvisEngineWindow::QvisEngineWindow(EngineList *engineList,
    const char *caption, const char *shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(engineList, caption, shortName, notepad,
                               QvisPostableWindowObserver::NoExtraButtons),
    activeEngine(""), statusMap()
{
    engines = engineList;
    caller = engines;
    statusAtts = 0;
}

// ****************************************************************************
// Method: QvisEngineWindow::~QvisEngineWindow
//
// Purpose: 
//   This is the destructor for the QvisEngineWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 2 16:31:22 PST 2001
//
// Modifications:
//   
// ****************************************************************************

QvisEngineWindow::~QvisEngineWindow()
{
    // Delete the status attributes in the status map.
    EngineStatusMap::Iterator pos;
    for(pos = statusMap.begin(); pos != statusMap.end(); ++pos)
    {
        delete pos.data();
    }

    // Detach from the status atts if they are still around.
    if(statusAtts)
        statusAtts->Detach(this);
}

// ****************************************************************************
// Method: QvisEngineWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 2 16:32:48 PST 2001
//
// Modifications:
//    Jeremy Meredith, Fri Jun 29 15:12:08 PDT 2001
//    Separated the single status/progress bar into one which reports
//    total status and one which reports current stage progress.
//
//    Brad Whitlock, Wed Nov 27 13:59:01 PST 2002
//    I added widgets to show engine information.
//
//    Brad Whitlock, Wed Feb 25 09:25:01 PDT 2004
//    I added a button to clear the cache.
//
//    Jeremy Meredith, Tue Mar 30 09:32:57 PST 2004
//    I made the engine area label a class member so it could mutate.
//
// ****************************************************************************

void
QvisEngineWindow::CreateWindowContents()
{
    isCreated = true;

    QGridLayout *grid1 = new QGridLayout(topLayout, 2, 2);
    grid1->setColStretch(1, 10);

    engineCombo = new QComboBox(central, "engineCombo");
    connect(engineCombo, SIGNAL(activated(int)), this, SLOT(selectEngine(int)));
    grid1->addWidget(engineCombo, 0, 1);
    engineLabel = new QLabel(engineCombo, "Engine:", central, "engineLabel");
    grid1->addWidget(engineLabel, 0, 0);

    // Create the widgets needed to show the engine information.
    engineInfo = new QGroupBox(central, "activeGroup");
    engineInfo->setTitle("Engine Information");
    topLayout->addWidget(engineInfo);
    QVBoxLayout *infoTopLayout = new QVBoxLayout(engineInfo);
    infoTopLayout->setMargin(10);
    infoTopLayout->addSpacing(15);
    QGridLayout *infoLayout = new QGridLayout(infoTopLayout, 3, 2);
    infoLayout->setSpacing(5);

    engineNP = new QLabel(engineInfo, "engineNP");
    infoLayout->addWidget(engineNP, 0, 1);
    QLabel *engineNPLabel = new QLabel(engineNP, "Number of processors:",
        engineInfo, "engineNPLabel");
    infoLayout->addWidget(engineNPLabel, 0, 0);

    engineNN = new QLabel(engineInfo, "engineNN");
    infoLayout->addWidget(engineNN, 1, 1);
    QLabel *engineNNLabel = new QLabel(engineNN, "Number of nodes:",
        engineInfo, "engineNNLabel");
    infoLayout->addWidget(engineNNLabel, 1, 0);

    engineLB = new QLabel(engineInfo, "engineLB");
    infoLayout->addWidget(engineLB, 2, 1);
    QLabel *engineLBLabel = new QLabel(engineLB, "Load balancing:",
        engineInfo, "engineNPLabel");
    infoLayout->addWidget(engineLBLabel, 2, 0);

    // Create the status bars.
    totalStatusLabel = new QLabel("Total status:", central, "totalStatusLabel");
    topLayout->addWidget(totalStatusLabel);

    totalProgressBar = new QProgressBar(central, "totalProgressBar");
    totalProgressBar->setTotalSteps(100);
    topLayout->addWidget(totalProgressBar);

    stageStatusLabel = new QLabel("Stage status:", central, "stageStatusLabel");
    topLayout->addWidget(stageStatusLabel);

    stageProgressBar = new QProgressBar(central, "stageProgressBar");
    stageProgressBar->setTotalSteps(100);
    topLayout->addWidget(stageProgressBar);

    QGridLayout *buttonLayout1 = new QGridLayout(topLayout, 2, 3);
    buttonLayout1->setSpacing(10);
    interruptEngineButton = new QPushButton("Interrupt engine", central, "interruptEngineButton");
    connect(interruptEngineButton, SIGNAL(clicked()), this, SLOT(interruptEngine()));
    interruptEngineButton->setEnabled(false);
    buttonLayout1->addWidget(interruptEngineButton, 0, 0);
    buttonLayout1->setColStretch(1, 10);

    closeEngineButton = new QPushButton("Close engine", central, "closeEngineButton");
    connect(closeEngineButton, SIGNAL(clicked()), this, SLOT(closeEngine()));
    closeEngineButton->setEnabled(false);
    buttonLayout1->addWidget(closeEngineButton, 0, 2);

    clearCacheButton = new QPushButton("Clear cache", central, "clearCacheButton");
    connect(clearCacheButton, SIGNAL(clicked()), this, SLOT(clearCache()));
    clearCacheButton->setEnabled(false);
    buttonLayout1->addWidget(clearCacheButton, 1, 0);

    topLayout->addSpacing(10);
}

// ****************************************************************************
// Method: QvisEngineWindow::Update
//
// Purpose: 
//   This method is called when the subjects that the window observes are
//   modified.
//
// Arguments:
//   TheChangedSubject : A pointer to the subject that changed.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 2 16:33:06 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu May 3 14:36:08 PST 2001
//   Fixed a crash.
//
// ****************************************************************************

void
QvisEngineWindow::Update(Subject *TheChangedSubject)
{
    caller = TheChangedSubject;

    if(isCreated)
        UpdateWindow(false);
}

// ****************************************************************************
// Method: QvisEngineWindow::SubjectRemoved
//
// Purpose: 
//   This method is called when the subjects observed by the window are
//   destructed.
//
// Arguments:
//   TheRemovedSubject : A pointer to the subject to remove.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 2 16:33:44 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisEngineWindow::SubjectRemoved(Subject *TheRemovedSubject)
{
    if(TheRemovedSubject == engines)
        engines = 0;
    else if(TheRemovedSubject == statusAtts)
        statusAtts = 0;
}

// ****************************************************************************
// Method: QvisEngineWindow::UpdateWindow
//
// Purpose: 
//   This method is called to update the window's widgets when the subjects
//   change.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 2 16:34:26 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Nov 27 14:00:24 PST 2002
//   I added widgets to show engine information.
//
//   Brad Whitlock, Wed Feb 25 09:31:17 PDT 2004
//   I added code to set the enabled state for the clearCache button.
//
//   Jeremy Meredith, Tue Mar 30 09:33:25 PST 2004
//   I added support for simulations.
//
//   Jeremy Meredith, Mon Apr  4 16:02:22 PDT 2005
//   I made better names for simulations.
//
//   Kathleen Bonnell, Tue Apr 26 16:42:17 PDT 2005 
//   Don't enable interruptEngineButton until the process has been fixed. 
//
// ****************************************************************************

void
QvisEngineWindow::UpdateWindow(bool doAll)
{
    if(caller == engines || doAll)
    {
        const stringVector &host = engines->GetEngines();
        const stringVector &sim  = engines->GetSimulationName();

        // Add the engines to the widget.
        engineCombo->blockSignals(true);
        engineCombo->clear();
        int current = -1;
        for(int i = 0; i < host.size(); ++i)
        {
            QString temp(host[i].c_str());
            if (!sim[i].empty())
            {
                int lastSlashPos = QString(sim[i].c_str()).findRev('/');
                QString newsim = QString(sim[i].substr(lastSlashPos+1).c_str());
                int lastDotPos =  newsim.findRev('.');
                int firstDotPos =  newsim.find('.');

                QString name = newsim.mid(firstDotPos+1,
                                          lastDotPos-firstDotPos-1);

                temp = QString().sprintf("%s on %s", 
                                    name.latin1(), host[i].c_str());
            }
            engineCombo->insertItem(temp);

            if(temp == activeEngine)
                current = i;
        }

        if(current == -1)
        {
            // Update the activeEngine string.
            if(engineCombo->count() > 0)
            {
                current = 0;
                engineCombo->setCurrentItem(0);
                if (sim[0]=="")
                    activeEngine = QString().sprintf("%s",host[0].c_str());
                else
                    activeEngine = QString().sprintf("%s:%s",host[0].c_str(),
                                                     sim[0].c_str());

                // Add an entry if needed.
                AddStatusEntry(activeEngine);
            }
            else
            {
                // The active engine was not in the list of engines returned
                // by the viewer. Remove the entry from the list.
                RemoveStatusEntry(activeEngine);
                activeEngine = QString("");
            }
        }
        else
        {
            engineCombo->setCurrentItem(current);
        }
        engineCombo->blockSignals(false);

        // Update the engine information.
        UpdateInformation(current);

        // Set the enabled state of the various widgets.
        // KSB: When INTERRUPT ENGINE has been fixed, uncomment the next line.
        //interruptEngineButton->setEnabled(host.size() > 0);
        closeEngineButton->setEnabled(host.size() > 0);
        clearCacheButton->setEnabled(host.size() > 0);
        engineCombo->setEnabled(host.size() > 0);
    }

    if(caller == statusAtts || doAll)
    {
        QString key(statusAtts->GetSender().c_str());

        if(key != QString("viewer"))
        {    
            UpdateStatusEntry(key);

            // If the sender of the status message is the engine that we're
            // currently looking at, update the status widgets.
            if(key == activeEngine)
            {
                UpdateStatusArea();
            }
        }
    }
}

// ****************************************************************************
// Method: QvisEngineWindow::UpdateInformation
//
// Purpose: 
//   Updates the engine information.
//
// Arguments:
//   index : The index of the engine to update.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 27 14:46:52 PST 2002
//
// Modifications:
//    Jeremy Meredith, Tue Mar 30 09:34:03 PST 2004
//    I added support for simulations.
//
// ****************************************************************************

void
QvisEngineWindow::UpdateInformation(int index)
{
    const stringVector &s = engines->GetEngines();

    // Set the values of the engine information widgets.
    if(index == -1 || s.size() < 1)
    {
        engineLabel->setText("Engine:");
        engineNP->setText("");
        engineNN->setText("");
        engineLB->setText("");
        engineInfo->setEnabled(false);
    }
    else
    {
        if (engines->GetSimulationName()[index] != "")
        {
            engineLabel->setText("Simulation:");
            closeEngineButton->setText("Disconnect simulation");
        }
        else
        {
            engineLabel->setText("Engine:");
            closeEngineButton->setText("Close engine");
        }

        QString tmp;
        tmp.sprintf("%d", engines->GetNumProcessors()[index]);
        engineNP->setText(tmp);

        if(engines->GetNumNodes()[index] == -1)
            engineNN->setText("Default");
        else 
        {
            tmp.sprintf("%d", engines->GetNumNodes()[index]);
            engineNN->setText(tmp);
        }

        if(engines->GetLoadBalancing()[index] == 0)
            engineLB->setText("Static");
        else
            engineLB->setText("Dynamic");

        engineInfo->setEnabled(true);
    }
}

// ****************************************************************************
// Method: QvisEngineWindow::UpdateStatusArea
//
// Purpose: 
//   Updates the window so it reflects the status information for the 
//   currently selected engine.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 2 16:35:12 PST 2001
//
// Modifications:
//    Jeremy Meredith, Fri Jun 29 15:12:08 PDT 2001
//    Separated the single status/progress bar into one which reports
//    total status and one which reports current stage progress.
//   
//    Jeremy Meredith, Thu Jul  5 12:40:30 PDT 2001
//    Added an explicit cast to avoid a warning.
//
// ****************************************************************************

void
QvisEngineWindow::UpdateStatusArea()
{
    EngineStatusMap::Iterator pos;
    if((pos = statusMap.find(activeEngine)) == statusMap.end())
        return;

    StatusAttributes *s = pos.data();
    if(s->GetClearStatus())
    {
        s->SetStatusMessage("");
        totalStatusLabel->setText("Total Status:");
        totalProgressBar->reset();
        stageStatusLabel->setText("Stage Status:");
        stageProgressBar->reset();
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

        if(s->GetMessageType() == 1)
        {
            totalStatusLabel->setText(QString(s->GetStatusMessage().c_str()));
            stageStatusLabel->setText("Stage Status:");
        }
        else if (s->GetMessageType() == 2)
        {
            QString msg;
            msg.sprintf("Total Status: Stage %d/%d",
                        s->GetCurrentStage(),
                        s->GetMaxStage());
            totalStatusLabel->setText(msg);
            msg.sprintf("Stage Status: %s",
                        s->GetCurrentStageName().c_str());
            stageStatusLabel->setText(msg);
        }

        // Set the progress bar percent done.
        totalProgressBar->setProgress(total);
        stageProgressBar->setProgress(s->GetPercent());
    }
}

// ****************************************************************************
// Method: QvisEngineWindow::ConnectStatusAttributes
//
// Purpose: 
//   Connects the status attributes subject that the window will observe.
//
// Arguments:
//   s : A pointer to the status attributes that the window will observe.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 2 16:31:48 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisEngineWindow::ConnectStatusAttributes(StatusAttributes *s)
{
    if(s)
    {
        statusAtts = s;
        statusAtts->Attach(this);
    }
}

// ****************************************************************************
// Method: QvisEngineWindow::AddStatusEntry
//
// Purpose: 
//   Adds an engine to the internal status map.
//
// Arguments:
//   key : The name of the engine to add to the map.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 2 16:35:50 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisEngineWindow::AddStatusEntry(const QString &key)
{
    // If the entry is in the map, return.
    if(statusMap.contains(key))
        return;

    // Add the entry to the map.
    StatusAttributes *s = new StatusAttributes;
    *s = *statusAtts;
    statusMap.insert(key, s);
}

// ****************************************************************************
// Method: QvisEngineWindow::RemoveStatusEntry
//
// Purpose: 
//   Removes an engine from the internal status map.
//
// Arguments:
//   key : The name of the engine to remove from the map.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 2 16:35:50 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisEngineWindow::RemoveStatusEntry(const QString &key)
{
    // If the entry is not in the map, return.
    EngineStatusMap::Iterator pos;
    if((pos = statusMap.find(key)) == statusMap.end())
        return;

    // Delete the status attributes that are in the map.
    delete pos.data();
    // Remove the key from the map.
    statusMap.remove(key);
}

// ****************************************************************************
// Method: QvisEngineWindow::UpdateStatusEntry
//
// Purpose: 
//   Makes the specified entry in the status map update to the current status
//   in the status attributes.
//
// Arguments:
//   key : The name of the engine whose status entry we want to update.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 2 16:36:43 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisEngineWindow::UpdateStatusEntry(const QString &key)
{
    // If the sender is in the status map, copy the status into the map entry.
    // If the sender is not in the map, add it.
    EngineStatusMap::Iterator pos;
    if((pos = statusMap.find(activeEngine)) != statusMap.end())
    {
        // Copy the status attributes.
        *(pos.data()) = *statusAtts;
    }
    else
        AddStatusEntry(key);
}

//
// Qt slots
//

// ****************************************************************************
// Method: QvisEngineWindow::closeEngine
//
// Purpose: 
//   This is a Qt slot function that is called when the "Close engine" button
//   is clicked. Its job is to tell the viewer to close the engine being
//   displayed by the window.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 2 16:37:35 PST 2001
//
// Modifications:
//    Jeremy Meredith, Tue Mar 30 09:34:33 PST 2004
//    I added support for simulations.
//
// ****************************************************************************

void
QvisEngineWindow::closeEngine()
{
    int index = engineCombo->currentItem();
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
    if(QMessageBox::warning( this, "VisIt",
                             msg.latin1(),
                             "Ok", "Cancel", 0,
                             0, 1 ) == 0)
    {
        // The user actually chose to close the engine.
        viewer->CloseComputeEngine(host, sim);
    }
}

// ****************************************************************************
// Method: QvisEngineWindow::interruptEngine
//
// Purpose: 
//   This is a Qt slot function that is called to interrupt the engine that's
//   displayed in the window.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 2 16:38:41 PST 2001
//
// Modifications:
//    Jeremy Meredith, Tue Mar 30 09:34:33 PST 2004
//    I added support for simulations.
//   
// ****************************************************************************

void
QvisEngineWindow::interruptEngine()
{
    int index = engineCombo->currentItem();
    if (index < 0)
        return;
    string host = engines->GetEngines()[index];
    string sim  = engines->GetSimulationName()[index];

    viewer->InterruptComputeEngine(host, sim);
}

// ****************************************************************************
// Method: QvisEngineWindow::clearCache
//
// Purpose: 
//   This is a Qt slot function that is called to tell the current engine
//   to clear its cache.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 26 14:16:39 PST 2004
//
// Modifications:
//    Jeremy Meredith, Tue Mar 30 09:34:33 PST 2004
//    I added support for simulations.
//
// ****************************************************************************

void
QvisEngineWindow::clearCache()
{
    int index = engineCombo->currentItem();
    if (index < 0)
        return;
    string host = engines->GetEngines()[index];
    string sim  = engines->GetSimulationName()[index];

    if(viewer->GetLocalHostName() == host)
        viewer->ClearCache("localhost", sim);
    else
        viewer->ClearCache(host, sim);
}

// ****************************************************************************
// Method: QvisEngineWindow::selectEngine
//
// Purpose: 
//   This is a Qt slot function that is called when selecting a new engine
//   to display.
//
// Arguments:
//   index : The index to use for the next active engine.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 2 16:39:26 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Nov 27 14:47:32 PST 2002
//   I added code to update the engine.
//
//   Jeremy Meredith, Tue Mar 30 09:34:33 PST 2004
//   I added support for simulations.
//
// ****************************************************************************

void
QvisEngineWindow::selectEngine(int index)
{
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
