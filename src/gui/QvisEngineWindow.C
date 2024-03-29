// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisEngineWindow.h>
#include <QLabel>
#include <QLayout>
#include <QProgressBar>
#include <QPushButton>
#include <QComboBox>
#include <QGroupBox>
#include <QMessageBox>

#include <EngineList.h>
#include <EngineProperties.h>
#include <StatusAttributes.h>
#include <UnicodeHelper.h>
#include <ViewerProxy.h>
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
//   Brad Whitlock, Wed Apr  9 11:00:55 PDT 2008
//   QString for caption, shortName.
//
// ****************************************************************************

QvisEngineWindow::QvisEngineWindow(EngineList *engineList,
    const QString &caption, const QString &shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(engineList, caption, shortName, notepad,
                               QvisPostableWindowObserver::NoExtraButtons),
    statusMap(), activeEngine("")
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
//   Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

QvisEngineWindow::~QvisEngineWindow()
{
    // Delete the status attributes in the status map.

    QMapIterator<QString, StatusAttributes*> itr(statusMap);

    while(itr.hasNext())
    {
        itr.next();
        delete itr.value();
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
//    Brad Whitlock, Wed Jun 22 09:43:48 PDT 2005
//    I moved the "Clear cache" button so the window is a little more
//    consistent with the simulation window and so it takes less vertical
//    space.
//
//    Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//    Initial Qt4 Port.
//
//    Brad Whitlock, Mon Oct 10 12:55:52 PDT 2011
//    I added some information.
//
//    Alister Maguire, Thu Nov 12 09:58:35 PST 2020
//    Removed the interrupt engine logic as it is no longer used.
//
// ****************************************************************************

void
QvisEngineWindow::CreateWindowContents()
{
    isCreated = true;

    QGridLayout *grid1 = new QGridLayout();
    topLayout->addLayout(grid1);
    grid1->setColumnStretch(1, 10);

    engineCombo = new QComboBox(central);
    connect(engineCombo, SIGNAL(activated(int)), this, SLOT(selectEngine(int)));
    grid1->addWidget(engineCombo, 0, 1);
    engineLabel = new QLabel(tr("Engine:"), central);
    grid1->addWidget(engineLabel, 0, 0);

    // Create the widgets needed to show the engine information.
    engineInfo = new QGroupBox(central);
    engineInfo->setTitle(tr("Engine information"));
    topLayout->addWidget(engineInfo);
    QVBoxLayout *infoTopLayout = new QVBoxLayout(engineInfo);

    QGridLayout *infoLayout = new QGridLayout();
    infoTopLayout->addLayout(infoLayout);

    int row = 0;
    engineNN = new QLabel(engineInfo);
    infoLayout->addWidget(engineNN, row, 1);
    QLabel *engineNNLabel = new QLabel(tr("Nodes:"),engineInfo);
    infoLayout->addWidget(engineNNLabel, row, 0);
    ++row;

    engineNP = new QLabel(engineInfo);
    infoLayout->addWidget(engineNP, row, 1);
    QLabel *engineNPLabel = new QLabel(tr("Processors:"),engineInfo);
    infoLayout->addWidget(engineNPLabel, row, 0);
    ++row;

    QString gpuTxt(tr("The number of processors using GPUs is affected using "
        "the controls of the GPU Acceleration tab in the Host Profiles window."));
    engineNP_GPU = new QLabel(engineInfo);
    engineNP_GPU->setToolTip(gpuTxt);
    infoLayout->addWidget(engineNP_GPU, row, 1);
    QLabel *engineNPGPULabel = new QLabel(tr("Processors using GPUs:"),engineInfo);
    engineNPGPULabel->setToolTip(gpuTxt);
    infoLayout->addWidget(engineNPGPULabel, row, 0);
    ++row;

    engineLB = new QLabel(engineInfo);
    infoLayout->addWidget(engineLB, row, 1);
    QLabel *engineLBLabel = new QLabel(tr("Load balancing:"),engineInfo);
    infoLayout->addWidget(engineLBLabel, row, 0);
    ++row;

    engineLB_Scheme = new QLabel(engineInfo);
    infoLayout->addWidget(engineLB_Scheme, row, 1);
    QLabel *engineLBSLabel = new QLabel(tr("Domain assignment:"),engineInfo);
    infoLayout->addWidget(engineLBSLabel, row, 0);
    ++row;

    // Create the status bars.
    totalStatusLabel = new QLabel(tr("Total status:"), central);
    topLayout->addWidget(totalStatusLabel);

    totalProgressBar = new QProgressBar(central);
    totalProgressBar->setMinimum(0);
    totalProgressBar->setMaximum(100);
    topLayout->addWidget(totalProgressBar);

    stageStatusLabel = new QLabel(tr("Stage status:"), central);
    topLayout->addWidget(stageStatusLabel);

    stageProgressBar = new QProgressBar(central);
    stageProgressBar->setMinimum(0);
    stageProgressBar->setMaximum(100);
    topLayout->addWidget(stageProgressBar);

    QHBoxLayout *buttonLayout1 = new QHBoxLayout();
    topLayout->addLayout(buttonLayout1);

    clearCacheButton = new QPushButton(tr("Clear cache"), central);
    connect(clearCacheButton, SIGNAL(clicked()), this, SLOT(clearCache()));
    clearCacheButton->setEnabled(false);
    buttonLayout1->addWidget(clearCacheButton);

    closeEngineButton = new QPushButton(tr("Close engine"), central);
    connect(closeEngineButton, SIGNAL(clicked()), this, SLOT(closeEngine()));
    closeEngineButton->setEnabled(false);
    buttonLayout1->addWidget(closeEngineButton);

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
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//    Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//    Initial Qt4 Port.
//
//   Alister Maguire, Thu Nov 12 09:58:35 PST 2020
//   Remove interrupt engine logic.
//
//   Kathlen Biagas, Thu Jan 21, 2021
//   Replace QString::asprintf with QString::arg as suggested in Qt docs.
//
// ****************************************************************************

void
QvisEngineWindow::UpdateWindow(bool doAll)
{
    if(caller == engines || doAll)
    {
        const stringVector &host = engines->GetEngineName();
        const stringVector &sim  = engines->GetSimulationName();

        // Add the engines to the widget.
        engineCombo->blockSignals(true);
        engineCombo->clear();
        int current = -1;
        for(size_t i = 0; i < host.size(); ++i)
        {
            QString temp(host[i].c_str());
            if (!sim[i].empty())
            {
                int lastSlashPos = QString(sim[i].c_str()).lastIndexOf('/');
                QString newsim(sim[i].substr(lastSlashPos+1).c_str());
                int lastDotPos =  newsim.lastIndexOf('.');
                int firstDotPos =  newsim.indexOf('.');

                QString name = newsim.mid(firstDotPos+1,
                                          lastDotPos-firstDotPos-1);

                temp = name + tr(" on ") + QString(host[i].c_str());
            }
            engineCombo->addItem(temp);

            if(temp == activeEngine)
                current = i;
        }

        if(current == -1)
        {
            // Update the activeEngine string.
            if(engineCombo->count() > 0)
            {
                current = 0;
                engineCombo->setCurrentIndex(0);
                activeEngine = QString(host[0].c_str()); 
                if (sim[0]!="")
                    activeEngine += QString(":%1").arg(sim[0].c_str());

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
            engineCombo->setCurrentIndex(current);
        }
        engineCombo->blockSignals(false);

        // Update the engine information.
        UpdateInformation(current);

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
//    Brad Whitlock, Mon Oct 10 12:49:25 PDT 2011
//    I added more information.
//
// ****************************************************************************

void
QvisEngineWindow::UpdateInformation(int index)
{
    const stringVector &s = engines->GetEngineName();

    // Set the values of the engine information widgets.
    if(index == -1 || s.size() < 1)
    {
        engineLabel->setText(tr("Engine:"));
        engineNN->setText("");
        engineNP->setText("");
        engineNP_GPU->setText("");
        engineLB->setText("");
        engineLB_Scheme->setText("");
        engineInfo->setEnabled(false);
    }
    else
    {
        if (engines->GetSimulationName()[index] != "")
        {
            engineLabel->setText(tr("Simulation:"));
            closeEngineButton->setText(tr("Disconnect"));
        }
        else
        {
            engineLabel->setText(tr("Engine:"));
            closeEngineButton->setText(tr("Close engine"));
        }

        const EngineProperties &props = engines->GetProperties(index);

        if(props.GetNumNodes() == -1)
            engineNN->setText(tr("Default"));
        else
            engineNN->setText(QString("%1").arg(props.GetNumNodes()));

        engineNP->setText(QString("%1").arg(props.GetNumProcessors()));
        engineNP_GPU->setText(QString("%1").arg(props.GetNumProcessorsUsingGPUs()));

        if(props.GetDynamicLoadBalancing())
        {
            engineLB->setText(tr("Dynamic"));
            engineLB_Scheme->setText(tr("Dynamic"));
        }
        else
        {
            engineLB->setText(tr("Static"));
            engineLB_Scheme->setText(props.GetLoadBalancingScheme().c_str());
        }

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
//    Brad Whitlock, Tue Apr 29 10:45:23 PDT 2008
//    Support for internationalization.
//
//    Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//    Initial Qt4 Port.
//
//    Kathleen Biagas, Tue Mar  2 15:49:07 PST 2021
//    Fix QString arg indexing.
//
// ****************************************************************************

void
QvisEngineWindow::UpdateStatusArea()
{

    if(!statusMap.contains(activeEngine))
        return;

    StatusAttributes *s = statusMap[activeEngine];
    if(s->GetClearStatus())
    {
        s->SetStatusMessage("");
        totalStatusLabel->setText(tr("Total Status:"));
        totalProgressBar->reset();
        stageStatusLabel->setText(tr("Stage Status:"));
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
            totalStatusLabel->setText(StatusAttributes_GetStatusMessage(*s));
            stageStatusLabel->setText(tr("Stage Status:"));
        }
        else if (s->GetMessageType() == 2)
        {
            QString msg = QString("%1/%2").arg(s->GetCurrentStage()).arg(s->GetMaxStage());
            msg = tr("Total Status: Stage ") + msg;
            totalStatusLabel->setText(msg);
            msg = tr("Stage Status: ") + QString(s->GetCurrentStageName().c_str());
            stageStatusLabel->setText(msg);
        }

        // Set the progress bar percent done.
        totalProgressBar->setValue(total);
        stageProgressBar->setValue(s->GetPercent());
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
//    Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//    Initial Qt4 Port.
//
// ****************************************************************************

void
QvisEngineWindow::RemoveStatusEntry(const QString &key)
{
    // If the entry is not in the map, return.
    if(!statusMap.contains(key))
        return;

    // Delete the status attributes that are in the map.
    delete statusMap[key];
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
//    Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//    Initial Qt4 Port.
//
//
// ****************************************************************************

void
QvisEngineWindow::UpdateStatusEntry(const QString &key)
{
    // If the sender is in the status map, copy the status into the map entry.
    // If the sender is not in the map, add it.
    QMapIterator<QString, StatusAttributes*> itr(statusMap);

    if(statusMap.contains(activeEngine))
    {
        *statusMap[activeEngine] = *statusAtts;
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
//   Jeremy Meredith, Tue Mar 30 09:34:33 PST 2004
//   I added support for simulations.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisEngineWindow::closeEngine()
{
    int index = engineCombo->currentIndex();
    if (index < 0)
        return;

    string host = engines->GetEngineName()[index];
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
    if(QMessageBox::warning( this, "VisIt",
                             msg,
                             tr("Ok"), tr("Cancel"), 0,
                             0, 1 ) == 0)
    {
        // The user actually chose to close the engine.
        GetViewerMethods()->CloseComputeEngine(host, sim);
    }
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
//    Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//    Initial Qt4 Port.
//
// ****************************************************************************

void
QvisEngineWindow::clearCache()
{
    int index = engineCombo->currentIndex();
    if (index < 0)
        return;
    string host = engines->GetEngineName()[index];
    string sim  = engines->GetSimulationName()[index];

    if(GetViewerProxy()->GetLocalHostName() == host)
        GetViewerMethods()->ClearCache("localhost", sim);
    else
        GetViewerMethods()->ClearCache(host, sim);
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
    activeEngine = QString(engines->GetEngineName()[index].c_str());
    if (engines->GetSimulationName()[index]!="")
        activeEngine += QString(":%1").arg(engines->GetSimulationName()[index].c_str());

    // Update the rest of the widgets using the information for the
    // active engine.
    UpdateStatusArea();
    // Update the engine information.
    UpdateInformation(index);
}
