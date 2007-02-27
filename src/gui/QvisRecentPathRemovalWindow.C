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

#include <QvisRecentPathRemovalWindow.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qpushbutton.h>

#include <algorithm>

#include <FileServerList.h>

// ****************************************************************************
// Method: QvisRecentPathRemovalWindow::QvisRecentPathRemovalWindow
//
// Purpose: 
//   Constructor for the QvisRecentPathRemovalWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 10 16:03:20 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QvisRecentPathRemovalWindow::QvisRecentPathRemovalWindow(Subject *s,
    const char *captionString) : QvisDelayedWindowObserver(s, captionString),
    paths()
{
}

// ****************************************************************************
// Method: QvisRecentPathRemovalWindow::~QvisRecentPathRemovalWindow
//
// Purpose: 
//   Destructor for QvisRecentPathRemovalWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 10 16:08:32 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QvisRecentPathRemovalWindow::~QvisRecentPathRemovalWindow()
{
}

// ****************************************************************************
// Method: QvisRecentPathRemovalWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 10 16:08:50 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisRecentPathRemovalWindow::CreateWindowContents()
{
    // Create the widgets needed to remove the paths.
    removalControlsGroup = new QGroupBox(central, "removalControlsGroup");
    removalControlsGroup->setTitle("Select paths to remove");
    topLayout->addWidget(removalControlsGroup, 5);

    QVBoxLayout *innerTopLayout = new QVBoxLayout(removalControlsGroup);
    innerTopLayout->setMargin(10);
    innerTopLayout->addSpacing(15);
    innerTopLayout->setSpacing(10);

    // Create the listbox that lets us select the paths to remove.
    removalListBox = new QListBox(removalControlsGroup, "removalListBox");
    removalListBox->setSelectionMode(QListBox::Multi);
    innerTopLayout->addWidget(removalListBox);

    // Create the pushbuttons that actually call the removal routines.
    QHBoxLayout *hLayout = new QHBoxLayout(innerTopLayout);
    hLayout->setSpacing(5);
    removeButton = new QPushButton("Remove", removalControlsGroup,
        "removeButton");
    connect(removeButton, SIGNAL(clicked()),
            this, SLOT(removePaths()));
    hLayout->addWidget(removeButton);
    removeAllButton = new QPushButton("Remove all", removalControlsGroup,
        "removeAllButton");
    connect(removeAllButton, SIGNAL(clicked()),
            this, SLOT(removeAllPaths()));
    hLayout->addWidget(removeAllButton);
    invertSelectionButton = new QPushButton("Invert selection",
        removalControlsGroup, "invertSelectionButton");
    connect(invertSelectionButton, SIGNAL(clicked()),
            this, SLOT(invertSelection()));
    hLayout->addWidget(invertSelectionButton);    

    // Create the ok and cancel buttons.
    QHBoxLayout *buttonLayout = new QHBoxLayout(topLayout);
    buttonLayout->addStretch(10);
    okButton = new QPushButton("Ok", central, "Ok");
    connect(okButton, SIGNAL(clicked()),
            this, SLOT(applyDismiss()));
    buttonLayout->addWidget(okButton);
    cancelButton = new QPushButton("Cancel", central, "cancelButton");
    connect(cancelButton, SIGNAL(clicked()),
            this, SLOT(handleCancel()));
    buttonLayout->addWidget(cancelButton);
}

// ****************************************************************************
// Method: QvisRecentPathRemovalWindow::UpdateWindow
//
// Purpose: 
//   This method gets called when the window must be updated.
//
// Arguments:
//   doAll : Whether we should unconditionally updated the window.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 10 16:09:12 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisRecentPathRemovalWindow::UpdateWindow(bool doAll)
{
    if(fileServer->HostChanged() || fileServer->PathChanged() || doAll)
    {
        // Clear out the paths vector.
        paths.clear();

        // Display the file paths for all of the hosts.
        stringVector hosts(fileServer->GetRecentHosts());
        for(int i = 0; i < hosts.size(); ++i)
        {
            const stringVector &p = fileServer->GetRecentPaths(hosts[i]);
            for(int j = 0; j < p.size(); ++j)
                paths.push_back(QualifiedFilename(hosts[i], p[j], "a"));
        }

        // Make sure the paths are sorted.
        std::sort(paths.begin(), paths.end());

        // Update the window's widgets.
        UpdateWidgets();
    }
}

// ****************************************************************************
// Method: QvisRecentPathRemovalWindow::UpdateWidgets
//
// Purpose: 
//   Updates the widgets in the window.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 10 16:09:56 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisRecentPathRemovalWindow::UpdateWidgets()
{
    // Clear the list box.
    removalListBox->clear();

    // Display the file paths for all of the hosts.
    for(int i = 0; i < paths.size(); ++i)
    {
        QString f;
        f.sprintf("%s:%s", paths[i].host.c_str(), paths[i].path.c_str());
        removalListBox->insertItem(f);
    }

    // Don't enable the removal buttons unless there are paths.
    bool havePaths = (removalListBox->count() > 0);
    removeButton->setEnabled(havePaths);
    removeAllButton->setEnabled(havePaths);
    invertSelectionButton->setEnabled(havePaths);
}

//
// Qt slot functions
//

// ****************************************************************************
// Method: QvisRecentPathRemovalWindow::show
//
// Purpose: 
//   Forces the window to update when it is shown.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 10 16:41:26 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisRecentPathRemovalWindow::show()
{
    QvisDelayedWindowObserver::show();
    UpdateWindow(true);
}

// ****************************************************************************
// Method: QvisRecentPathRemovalWindow::removePaths
//
// Purpose: 
//   This is a Qt slot function that is called when we want to remove selected
//   paths from the list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 10 16:10:14 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisRecentPathRemovalWindow::removePaths()
{
    QualifiedFilenameVector newPaths;
    for(int i = 0; i < removalListBox->count(); ++i)
    {
        if(!removalListBox->isSelected(i))
            newPaths.push_back(paths[i]);
    }
    paths = newPaths;

    UpdateWidgets();
}

// ****************************************************************************
// Method: QvisRecentPathRemovalWindow::removeAllPaths
//
// Purpose: 
//   This is a Qt slot function that is called when we want to remove all
//   paths from the list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 10 16:10:14 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisRecentPathRemovalWindow::removeAllPaths()
{
    paths.clear();
    UpdateWidgets();
}

// ****************************************************************************
// Method: QvisRecentPathRemovalWindow::invertSelection
//
// Purpose: 
//   This is a Qt slot function that is called when we want to invert the
//   selection for the selected paths.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 10 16:10:14 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisRecentPathRemovalWindow::invertSelection()
{
    removalListBox->invertSelection();
}

// ****************************************************************************
// Method: QvisRecentPathRemovalWindow::applyDismiss
//
// Purpose: 
//   This is a Qt slot function that is called when we want to remove selected
//   paths from the list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 10 16:10:14 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisRecentPathRemovalWindow::applyDismiss()
{
    // Clear out the recent path list.
    fileServer->ClearRecentPathList();
    for(int i = 0; i < paths.size(); ++i)
    {
        fileServer->AddPathToRecentList(paths[i].host, paths[i].path);
    }

    // Update the other observers.
    SetUpdate(false);
    fileServer->Notify();

    // Hide the window.
    hide();
}

// ****************************************************************************
// Method: QvisRecentPathRemovalWindow::handleCancel
//
// Purpose: 
//   Cancels everything and hides the window.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 10 16:13:43 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisRecentPathRemovalWindow::handleCancel()
{
    hide();
}
