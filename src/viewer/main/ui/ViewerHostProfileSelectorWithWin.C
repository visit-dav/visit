/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#include <ViewerHostProfileSelectorWithWin.h>

#include <HostProfileList.h>
#include <MachineProfile.h>
#include <ViewerBase.h>
#include <ViewerMessaging.h>

#include <QComboBox>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QSpinBox>

#include <DebugStream.h>
#include <stdio.h>

#include <string>
#include <vector>

using std::string;
using std::vector;

#define MAX_NUM_PROCESSORS 1000000
#define MAX_NUM_NODES      300000

// ****************************************************************************
//  Constructor:  ViewerHostProfileSelectorWithWin::ViewerHostProfileSelectorWithWin
//
//  Notes:  
//    Taken from ViewerEngineChooser by Jeremy Meredith.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    February 5, 2003 
//
//  Modifications:
//   Kathleen Bonnell, Tue Jan 11 16:14:14 PST 2005
//   Set minimum for numNodes spinbox to 1.
//
//   Jeremy Meredith, Thu Sep 15 16:37:24 PDT 2005
//   Added machinefile for some mpich implementations.
//
//   Brad Whitlock, Tue Apr 29 15:07:23 PDT 2008
//   Added tr()'s
//
//   Brad Whitlock, Fri May 23 11:36:45 PDT 2008
//   Qt 4.
//
//   Brad Whitlock, Thu Oct 23 10:50:31 PDT 2014
//   Add fixed node/proc controls.
//
// ****************************************************************************

ViewerHostProfileSelectorWithWin::ViewerHostProfileSelectorWithWin(QWidget *parent)
    : QDialog(parent), ViewerHostProfileSelector()
{
    setWindowModality(Qt::ApplicationModal);

    waitingOnUser = false;

    QVBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->setMargin(0);
    QGridLayout *layout = new QGridLayout;
    topLayout->addLayout(layout);
    layout->setMargin(10);
    layout->setSpacing(5);
    layout->setRowStretch(0, 100);
    layout->setRowStretch(1, 0);
    layout->setRowStretch(2, 0);
    layout->setRowStretch(3, 0);

    layout->setColumnStretch(0, 0);
    layout->setColumnStretch(1, 100);
    layout->setColumnStretch(2, 0);
    layout->setColumnStretch(3, 100);

    profiles = new QListWidget(this);
    profiles->setMinimumHeight(100);
    layout->addWidget(profiles, 0,0, 1,4);
    connect(profiles, SIGNAL(itemSelectionChanged()),
            this,     SLOT(newProfileSelected()));
    connect(profiles, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(accept()));

    // Processor controls
    numProcsLabel = new QLabel(tr("Num procs"), this);
    layout->addWidget(numProcsLabel, 1,0);
    QWidget *np = new QWidget(this);
    layout->addWidget(np,            1,1);
    QHBoxLayout *npLayout = new QHBoxLayout(np);
    npLayout->setMargin(1);
    npLayout->setSpacing(1);

    numProcs = new QSpinBox(np);
    numProcs->setMinimum(1);
    numProcs->setMaximum(MAX_NUM_PROCESSORS);
    npLayout->addWidget(numProcs);

    numProcsFixed = new QComboBox(np);
    npLayout->addWidget(numProcsFixed);
    connect(numProcsFixed, SIGNAL(activated(int)),
            this, SLOT(numProcsFixedActivated(int)));
    numProcsFixed->setVisible(false);

    // Node controls
    numNodesLabel = new QLabel(tr("Num nodes"), this);
    layout->addWidget(numNodesLabel, 1,2);
    QWidget *nn = new QWidget(this);
    layout->addWidget(nn,            1,3);
    QHBoxLayout *nnLayout = new QHBoxLayout(nn);
    nnLayout->setMargin(1);
    nnLayout->setSpacing(1);

    numNodes = new QSpinBox(nn);
    numNodes->setMinimum(1);
    numNodes->setMaximum(MAX_NUM_NODES);
    nnLayout->addWidget(numNodes);

    numNodesFixed = new QComboBox(nn);
    nnLayout->addWidget(numNodesFixed);
    connect(numNodesFixed, SIGNAL(activated(int)),
            this, SLOT(numNodesFixedActivated(int)));
    numNodesFixed->setVisible(false);

    bankNameLabel = new QLabel(tr("Bank"), this);
    bankName = new QLineEdit(this);
    layout->addWidget(bankNameLabel, 2,0);
    layout->addWidget(bankName,      2,1);

    timeLimitLabel = new QLabel(tr("Time limit"), this);
    timeLimit = new QLineEdit(this);
    layout->addWidget(timeLimitLabel, 2,2);
    layout->addWidget(timeLimit,      2,3);

    machinefileLabel = new QLabel(tr("Machine file"), this);
    machinefile = new QLineEdit(this);
    layout->addWidget(machinefileLabel, 3,0);
    layout->addWidget(machinefile,      3,1, 1,3);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    topLayout->addLayout(buttonLayout);
    buttonLayout->setMargin(10);

    okayButton = new QPushButton(tr("OK"), this);
    buttonLayout->addWidget(okayButton, 0);
    connect(okayButton, SIGNAL(clicked()), this, SLOT(accept()));

    buttonLayout->addStretch(100);

    cancelButton = new QPushButton(tr("Cancel"), this);
    buttonLayout->addWidget(cancelButton, 0);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

// ****************************************************************************
//  Destructor:  ViewerHostProfileSelectorWithWin::~ViewerHostProfileSelectorWithWin
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    February 5, 2003 
//
// ****************************************************************************

ViewerHostProfileSelectorWithWin::~ViewerHostProfileSelectorWithWin()
{
}

// ****************************************************************************
//  Method:  ViewerHostProfileSelectorWithWin::SelectProfile
//
//  Purpose:
//    Selects a profile to use when launching an Engine.  Allows user
//    selection via the chooser unless 'skipChooser' is true. 
//
//  Notes:
//    Taken from ViewerEngineChooser by Jeremy Meredith.
//
//  Arguments:
//    hostProfileList  : a list of possible host profiles. 
//    hostName         : the host name
//    skipChooser      : option to completely avoid the chooser. 
//
//  Returns: true if valid profile was selected, false otherwise.
//   
//  Programmer:  Jeremy Meredith
//  Creation:    August 14, 2002
//
//  Modifications:
//    Jeremy Meredith, Sat Aug 17 10:59:48 PDT 2002
//    Added support for nowin mode.
//
//    Brad Whitlock, Wed Nov 27 13:39:50 PST 2002
//    I added code to set certain parallel options into the EngineProxy
//    so that they can be queried easily later.
//
//    Jeremy Meredith, Wed Dec 18 17:18:05 PST 2002
//    Added skip option so that we completely avoid the chooser.
//    Also added code to temporarily block socket signals to the viewer.
//    Not doing so was breaking synchronization with the CLI through
//    a race condition to grab the signals.
//
//    Kathleen Bonnell, Wed Feb  5 09:46:31 PST 2003
//    Extracted code from ViewerHostProfileSelector::GetNewEngine.
//
//    Jeremy Meredith, Tue Mar 23 13:18:27 PST 2004
//    Disabled the code to cache the profile.  With our big parallel machines, 
//    resources change so frequently that it will irritate users to assume
//    that they want the same number of processors every time the engine
//    launches.
//
//    Jeremy Meredith, Wed Oct 27 13:59:14 PDT 2004
//    Prevented recursion into QDialog::exec().  See VisIt00005532.
//
//    Jeremy Meredith, Thu Sep 15 16:37:24 PDT 2005
//    Added machinefile for some mpich implementations.
//
//    Brad Whitlock, Tue Apr 29 15:08:19 PDT 2008
//    Support for internationalization.
//
//    Brad Whitlock, Fri May 23 11:54:42 PDT 2008
//    Qt 4.
//
//    Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//    Split HostProfile int MachineProfile and LaunchProfile.
//
//    Jeremy Meredith, Thu Feb 25 10:16:15 EST 2010
//    Adding debug info.
//
//    Jeremy Meredith, Fri Feb 26 18:14:55 EST 2010
//    Don't forget to block signals when you mess with the list.
//
//    Jeremy Meredith, Fri Mar  5 14:19:21 EST 2010
//    Actually, we don't want to block signals, as this sets up the
//    window correctly the first time.  Just don't set it to a bad
//    index before you set it to the right one.
//
//    Brad Whitlock, Thu Oct  6 16:16:10 PDT 2011
//    Constrain the number of processors and nodes.
//
//    Brad Whitlock, Thu Oct 23 11:29:00 PDT 2014
//    Add controls for constrained nodes/procs.
//
// ****************************************************************************

bool 
ViewerHostProfileSelectorWithWin::SelectProfile(
    HostProfileList *hostProfileList, const string &hostName, bool skipChooser)
{
    int  i;
    QString title = tr("Select options for '%1'").arg(hostName.c_str());
    setWindowTitle(title);

    profile = MachineProfile();
    debug2 << "ViewerHostProfileSelectorWithWin::SelectProfile\n";

    if (skipChooser)
    {
        // do nothing; leave the profile completely blank
        debug2 << "   Exiting: told to skip\n";
    }
    else if (cachedProfile.count(hostName))
    {
        profile = cachedProfile[hostName];
        debug2 << "   Exiting: found cached\n";
    }
    else
    {
        //
        // If someone tries to do something while we already have the
        // selector open and waiting on a user, just pretend they didn't
        // want to launch one.  This will cause the new operations to fail,
        // but at least it won't recurse into the exec() call and crash.
        //
        if (waitingOnUser)
            return false;

        //
        // Check for a host profile for the hostName. If one exists, add
        // any arguments to the command line for the engine proxy.  If
        // more than one exists, let the user choose the profile and
        // some options.
        //
        MachineProfile *mp = hostProfileList->GetMachineProfileForHost(hostName);

        if (mp)
        {
            profile = *mp;
            debug2 << "   Got a machine profile for host '"<<hostName<<"'\n";
            debug2 << "     It has "<<profile.GetNumLaunchProfiles()
                   <<" launch profiles\n";
            for (int i=0; i<profile.GetNumLaunchProfiles(); i++)
            {
                debug2 << "        launch profile #"<<i<<" has parallel="
                     <<(profile.GetLaunchProfiles(i).GetParallel()?"true":"false")
                     <<endl;
            }
        }
        else
        {
            debug2 << "   Found no matching machine profiles\n";
        }
        
        if (profile.GetNumLaunchProfiles() > 1 ||
            (profile.GetNumLaunchProfiles() == 1 &&
             profile.GetLaunchProfiles(0).GetParallel()))
        {
            debug2 << "   Presenting a choice to the user.\n";

            // Constrain the max nodes and processors.
            int maxP = profile.GetMaximumProcessorsValid() ? 
                profile.GetMaximumProcessors() : MAX_NUM_PROCESSORS;
            int maxN = profile.GetMaximumNodesValid() ? 
                profile.GetMaximumNodes() : MAX_NUM_NODES;
            numProcs->setMaximum(maxP);
            numNodes->setMaximum(maxN);

            profiles->blockSignals(true);
            profiles->clear();
            for (i=0; i<profile.GetNumLaunchProfiles(); i++)
            {
                profiles->addItem(profile.GetLaunchProfiles(i).GetProfileName().c_str());
            }
            profiles->blockSignals(false);
            if (profile.GetActiveProfile()>=0 &&
                profile.GetActiveProfile()<profile.GetNumLaunchProfiles())
            {
                // this signals the callback to set the default profile
                profiles->setCurrentRow(profile.GetActiveProfile());
            }
            else
            {
                profiles->setCurrentRow(0);
            }

            ViewerBase::GetViewerMessaging()->BlockClientInput(true);
            waitingOnUser = true;
            bool rejected = (exec() == QDialog::Rejected);
            waitingOnUser = false;
            ViewerBase::GetViewerMessaging()->BlockClientInput(false);

            if (rejected)
            {
                debug2 << "   Cancel engine selection." << endl;
                return false;
            }

            if(!profile.GetActiveLaunchProfile()->GetConstrainNodeProcs())
            {
                profile.GetActiveLaunchProfile()->SetNumProcessors(numProcs->value());
                profile.GetActiveLaunchProfile()->SetNumNodes(numNodes->value());
            }
            else
            {
                bool okay = false;
                int np = numProcsFixed->currentText().toInt(&okay);
                if(okay && np > 0)
                    profile.GetActiveLaunchProfile()->SetNumProcessors(np);
                int nn = numNodesFixed->currentText().toInt(&okay);
                if(okay && nn > 0)
                    profile.GetActiveLaunchProfile()->SetNumNodes(nn);
            }
            profile.GetActiveLaunchProfile()->SetBank(bankName->text().toStdString());
            profile.GetActiveLaunchProfile()->SetTimeLimit(timeLimit->text().toStdString());
            profile.GetActiveLaunchProfile()->SetMachinefile(machinefile->text().toStdString());
        }
        else
        {
            debug2 << "   No need to present a choice to the user.\n";
        }

        // Save it for use later

        // DISABLED 3/23/04 JSM:  No!  This was causing more usability
        // issues than it solved.  With our big parallel machines, 
        // resources change so frequently that it will irritate a 
        // user to assume that they want the same number of processors
        // every time the engine launches.
        //cachedProfile[hostName] = profile;
    }
    return true;
}


// ****************************************************************************
//  Method:  ViewerHostProfileSelectorWithWin::newProfileSelected
//
//  Purpose:
//    Qt slot for when a new profile is selected.
//    Change the selected profile and update the related widgets.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 14, 2002
//
//  Modifications:
//    Jeremy Meredith, Fri Jan 24 10:14:17 PST 2003
//    Made it grey out the other parallel options if the profile wasn't
//    parallel.
//
//    Jeremy Meredith, Thu Sep 15 16:37:24 PDT 2005
//    Added machinefile for some mpich implementations.
//
//    Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//    Split HostProfile int MachineProfile and LaunchProfile.
//
//    Jeremy Meredith, Thu Feb 25 10:11:36 EST 2010
//    Fixed a bug where it didn't let the user change from the default profile.
//
//    Brad Whitlock, Thu Oct  6 16:13:25 PDT 2011
//    Make sure that the number of nodes and processors is never more than
//    the maximum allowable.
//
//    Brad Whitlock, Thu Oct 23 11:29:00 PDT 2014
//    Add controls for constrained nodes/procs.
//
// ****************************************************************************

void
ViewerHostProfileSelectorWithWin::newProfileSelected()
{
    int index = profiles->currentRow();
    if (index < 0 || index >= profile.GetNumLaunchProfiles())
        return;

    profile.SetActiveProfile(index);
    LaunchProfile &lp = profile.GetLaunchProfiles(index);
    bool parallel = lp.GetParallel();

    int np = lp.GetNumProcessors();
    if(profile.GetMaximumProcessorsValid() && 
       np > profile.GetMaximumProcessors())
    {
        np = profile.GetMaximumProcessors();
    }
    numProcsLabel->setEnabled(parallel);
    numProcs->setEnabled(parallel);
    numProcs->setValue(np);
    numProcsFixed->setEnabled(parallel && lp.GetConstrainNodeProcs());

    int nn = lp.GetNumNodesSet() ? lp.GetNumNodes() : 1;
    if(profile.GetMaximumNodesValid() && 
       nn > profile.GetMaximumNodes())
    {
        nn = profile.GetMaximumNodes();
    }
    numNodesLabel->setEnabled(parallel && lp.GetNumNodesSet());
    numNodes->setEnabled(parallel && lp.GetNumNodesSet());
    numNodes->setValue(nn);
    numNodesFixed->setEnabled(parallel && lp.GetConstrainNodeProcs());

    // Show the node/proc constraints.
    if(parallel && lp.GetConstrainNodeProcs())
    {
        int nnodes = (int)lp.GetAllowableNodes().size();
        int nprocs = (int)lp.GetAllowableProcs().size();
        int n = std::min(nnodes, nprocs);
        numNodesFixed->blockSignals(true);
        numProcsFixed->blockSignals(true);
        numNodesFixed->clear();
        numProcsFixed->clear();
        int idx = 0;
        for(int i = 0; i < n; ++i)
        {
            numNodesFixed->addItem(QString().setNum(lp.GetAllowableNodes()[i]));
            numProcsFixed->addItem(QString().setNum(lp.GetAllowableProcs()[i]));

            if(lp.GetNumNodesSet() && lp.GetAllowableNodes()[i] == nn)
                idx = i;
            if(lp.GetAllowableProcs()[i] == np)
                idx = i;
        }
        numNodesFixed->setCurrentIndex(idx);
        numProcsFixed->setCurrentIndex(idx);

        numNodesFixed->blockSignals(false);
        numProcsFixed->blockSignals(false);

        numNodes->setVisible(false);
        numProcs->setVisible(false);
        numNodesFixed->setVisible(true);
        numProcsFixed->setVisible(true);

        numNodesLabel->setEnabled(true);
    }
    else
    {
        numNodes->setVisible(true);
        numProcs->setVisible(true);
        numNodesFixed->setVisible(false);
        numProcsFixed->setVisible(false);
    }

    bankNameLabel->setEnabled(parallel && lp.GetBankSet());
    bankName->setEnabled(parallel && lp.GetBankSet());
    bankName->setText(lp.GetBank().c_str());

    timeLimitLabel->setEnabled(parallel && lp.GetTimeLimitSet());
    timeLimit->setEnabled(parallel && lp.GetTimeLimitSet());
    timeLimit->setText(lp.GetTimeLimit().c_str());

    machinefileLabel->setEnabled(parallel && lp.GetMachinefileSet());
    machinefile->setEnabled(parallel && lp.GetMachinefileSet());
    machinefile->setText(lp.GetMachinefile().c_str());
}

// ****************************************************************************
// Method: ViewerHostProfileSelectorWithWin::numNodesFixedActivated
//
// Purpose:
//   This Qt slot function makes sure we show the right number of procs for the
//   selected number of nodes.
//
// Arguments:
//   index : The index of the new #nodes in the list.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 23 11:21:23 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerHostProfileSelectorWithWin::numNodesFixedActivated(int index)
{
    // Make sure that we also show the right #procs for this #nodes.
    numProcsFixed->blockSignals(true);
    numProcsFixed->setCurrentIndex(index);
    numProcsFixed->blockSignals(false);
}

// ****************************************************************************
// Method: ViewerHostProfileSelectorWithWin::numProcsFixedActivated
//
// Purpose:
//   This Qt slot function makes sure we show the right number of procs for the
//   selected number of nodes.
//
// Arguments:
//   index : The index of the new #nodes in the list.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 23 11:21:23 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerHostProfileSelectorWithWin::numProcsFixedActivated(int index)
{
    // Make sure that we also show the right #nodes for this #procs.
    numNodesFixed->blockSignals(true);
    numNodesFixed->setCurrentIndex(index);
    numNodesFixed->blockSignals(false);
}

