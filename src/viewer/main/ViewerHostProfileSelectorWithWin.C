#include <ViewerHostProfileSelectorWithWin.h>

#include <HostProfileList.h>
#include <ViewerSubject.h>

#include <stdio.h>

#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qspinbox.h>

#include <DebugStream.h>

#include <string>
using std::string;
#include <vector>
using std::vector;

extern ViewerSubject *viewerSubject;

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
// ****************************************************************************

ViewerHostProfileSelectorWithWin::ViewerHostProfileSelectorWithWin(QWidget *parent, const char *name)
    : QDialog(parent, name, true)
{
    waitingOnUser = false;

    QVBoxLayout *topLayout = new QVBoxLayout(this);

    QGridLayout *layout = new QGridLayout(topLayout, 5, 4);
    layout->setMargin(10);
    layout->setSpacing(5);
    layout->setRowStretch(0, 100);
    layout->setRowStretch(1, 0);
    layout->setRowStretch(2, 0);
    layout->setRowStretch(3, 0);

    layout->setColStretch(0, 0);
    layout->setColStretch(1, 100);
    layout->setColStretch(2, 0);
    layout->setColStretch(3, 100);

    profiles = new QListBox(this, "profiles");
    profiles->setMinimumHeight(100);
    layout->addMultiCellWidget(profiles, 0,0, 0,3);
    connect(profiles, SIGNAL(selectionChanged()),
            this,     SLOT(newProfileSelected()));

    numProcsLabel = new QLabel("Num procs", this, "numProcsLabel");
    numProcs = new QSpinBox(1, 99999, 1, this, "numProcs");
    layout->addMultiCellWidget(numProcsLabel, 1,1, 0,0);
    layout->addMultiCellWidget(numProcs,      1,1, 1,1);

    numNodesLabel = new QLabel("Num nodes", this, "numNodesLabel");
    numNodes = new QSpinBox(1, 99999, 1, this, "numNodes");
    layout->addMultiCellWidget(numNodesLabel, 1,1, 2,2);
    layout->addMultiCellWidget(numNodes,      1,1, 3,3);

    bankNameLabel = new QLabel("Bank", this, "bankNameLabel");
    bankName = new QLineEdit(this, "bankName");
    layout->addMultiCellWidget(bankNameLabel, 2,2, 0,0);
    layout->addMultiCellWidget(bankName,      2,2, 1,1);

    timeLimitLabel = new QLabel("Time limit", this, "timeLimitLabel");
    timeLimit = new QLineEdit(this, "timeLimit");
    layout->addMultiCellWidget(timeLimitLabel, 2,2, 2,2);
    layout->addMultiCellWidget(timeLimit,      2,2, 3,3);

    machinefileLabel = new QLabel("Machine file", this, "machinefileLabel");
    machinefile = new QLineEdit(this, "machinefile");
    layout->addMultiCellWidget(machinefileLabel, 3,3, 0,0);
    layout->addMultiCellWidget(machinefile,      3,3, 1,3);

    QHBoxLayout *buttonLayout = new QHBoxLayout(topLayout);
    buttonLayout->setMargin(10);

    okayButton = new QPushButton("OK", this, "okayButton");
    buttonLayout->addWidget(okayButton, 0);
    connect(okayButton, SIGNAL(clicked()), this, SLOT(accept()));

    buttonLayout->addStretch(100);

    cancelButton = new QPushButton("Cancel", this, "cancelButton");
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
// ****************************************************************************

bool 
ViewerHostProfileSelectorWithWin::SelectProfile(
    HostProfileList *hostProfileList, const string &hostName, bool skipChooser)
{
    int  i;
    char title[256];
    sprintf(title, "Select options for '%s'", hostName.c_str());
    setCaption(title);

    profile = HostProfile();

    if (skipChooser)
    {
        // do nothing; leave the profile completely blank
    }
    else if (cachedProfile.count(hostName))
    {
        profile = cachedProfile[hostName];
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
        matchingProfiles = 
                    hostProfileList->FindAllMatchingProfileForHost(hostName);

        if (matchingProfiles.size() > 0)
        {
            profile = *matchingProfiles[0];
        }
        
        if (matchingProfiles.size() > 1 ||
            (matchingProfiles.size() == 1 &&
             matchingProfiles[0]->GetParallel()))
        {
            profiles->clear();
            for (i=0; i<matchingProfiles.size(); i++)
            {
                profiles->insertItem(matchingProfiles[i]->GetProfileName().c_str());
            }
            profiles->setSelected(0, true);
            for (i=0; i<matchingProfiles.size(); i++)
            {
                if (matchingProfiles[i]->GetActive())
                {
                    // this signals the callback to set the default profile
                    profiles->setSelected(i, true);
                }
            }

            viewerSubject->BlockSocketSignals(true);
            waitingOnUser = true;
            bool rejected = (exec() == QDialog::Rejected);
            waitingOnUser = false;
            viewerSubject->BlockSocketSignals(false);

            if (rejected)
            {
                return false;
            }

            profile.SetNumProcessors(numProcs->value());
            profile.SetNumNodes(numNodes->value());
            profile.SetBank(bankName->text().latin1());
            profile.SetTimeLimit(timeLimit->text().latin1());
            profile.SetMachinefile(machinefile->text().latin1());
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
//   Jeremy Meredith, Thu Sep 15 16:37:24 PDT 2005
//   Added machinefile for some mpich implementations.
//
// ****************************************************************************

void
ViewerHostProfileSelectorWithWin::newProfileSelected()
{
    int index = profiles->currentItem();
    if (index == -1)
        return;

    profile = *matchingProfiles[index];
    bool parallel = profile.GetParallel();

    numProcsLabel->setEnabled(parallel);
    numProcs->setEnabled(parallel);
    numProcs->setValue(profile.GetNumProcessors());

    numNodesLabel->setEnabled(parallel && profile.GetNumNodesSet());
    numNodes->setEnabled(parallel && profile.GetNumNodesSet());
    numNodes->setValue(profile.GetNumNodes());

    bankNameLabel->setEnabled(parallel && profile.GetBankSet());
    bankName->setEnabled(parallel && profile.GetBankSet());
    bankName->setText(profile.GetBank().c_str());

    timeLimitLabel->setEnabled(parallel && profile.GetTimeLimitSet());
    timeLimit->setEnabled(parallel && profile.GetTimeLimitSet());
    timeLimit->setText(profile.GetTimeLimit().c_str());

    machinefileLabel->setEnabled(parallel && profile.GetMachinefileSet());
    machinefile->setEnabled(parallel && profile.GetMachinefileSet());
    machinefile->setText(profile.GetMachinefile().c_str());
}


