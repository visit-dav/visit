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

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPixmap>
#include <QPushButton>
#include <QSplitter>
#include <QWidget>

#include <QvisFileSelectionWindow.h>
#include <QvisRecentPathRemovalWindow.h>
#include <FileServerList.h>
#include <DebugStream.h>
#include <HostProfileList.h>
#include <HostProfile.h>
#include <ViewerProxy.h>

#include <visit-config.h>

#include <algorithm>

using std::vector;
using std::string;

// ****************************************************************************
// Method: QvisFileSelectionWindow::QvisFileSelectionWindow
//
// Purpose: 
//   Constructor for the QvisFileSelectionWindow class.
//
// Arguments:
//   winCaption : The caption displayed in the window decorations.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 22 08:54:26 PDT 2000
//
// Modifications:
//   Brad Whitlock, Mon Sep 30 07:52:50 PDT 2002
//   I made the window be modal. I also set the file server's progress
//   callback function.
//
//   Brad Whitlock, Fri Mar 28 15:51:51 PST 2003
//   I added pixmaps.
//
//   Brad Whitlock, Mon Sep 29 16:00:15 PST 2003
//   This might be a bad move but I made the window be non-modal like it was
//   originally so we don't have problems dismissing the Error dialog and
//   so the GUI does not get into a hung state. Hopefully this won't cause
//   other problems.
//
//   Brad Whitlock, Fri Oct 10 15:05:15 PST 2003
//   I added the recentPathsRemoval window.
//
//   Brad Whitlock, Tue Dec 2 16:31:02 PST 2003
//   Initialized currentVirtualDatabaseDefinitions and invalidHosts.
//
//   Brad Whitlock, Wed Apr  9 10:30:09 PDT 2008
//   Changed ctor args.
//
//   Brad Whitlock, Thu Jul 10 16:17:12 PDT 2008
//   Moved code to base class.
//
// ****************************************************************************

QvisFileSelectionWindow::QvisFileSelectionWindow(const QString &winCaption) :
    QvisFileWindowBase(winCaption)
{
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::~QvisFileSelectionWindow
//
// Purpose: 
//   Destructor for the QvisFileSelectionWindow class
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 22 08:56:02 PDT 2000
//
// Modifications:
//   Brad Whitlock, Thu Sep 12 12:22:23 PDT 2002
//   I added code to detach the subjects.
//
//   Brad Whitlock, Fri Mar 28 15:52:13 PST 2003
//   I added pixmaps.
//
//   Brad Whitlock, Thu Jul 10 16:16:54 PDT 2008
//   Moved code to base class.
//
// ****************************************************************************

QvisFileSelectionWindow::~QvisFileSelectionWindow()
{
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::CreateWindowContents
//
// Purpose: 
//   This method is called to create the window the first time that
//   the window is requested. This delayed creation speeds startup.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 22 08:56:25 PDT 2000
//
// Modifications:
//   Brad Whitlock, Wed Oct 4 16:38:43 PST 2000
//   Added code to get the intermediate file list from the file server.
//
//   Sean Ahern, Wed Feb 28 17:12:32 PST 2001
//   Added a Group button to group together the selected files.
//
//   Sean Ahern, Wed Aug 15 16:51:16 PDT 2001
//   Hooked up some logic to make some buttons gray out when their actions
//   are not appropriate.
//
//   Brad Whitlock, Mon Aug 27 11:19:26 PDT 2001
//   Changed the names of the apply/dismiss buttons to ok/cancel.
//
//   Brad Whitlock, Thu Jan 3 13:33:13 PST 2002
//   Added refresh button.
//
//   Brad Whitlock, Wed Feb 13 10:38:32 PDT 2002
//   Changed the host and path line edits to comboboxes and added a splitter.
//
//   Brad Whitlock, Thu May 9 17:07:44 PST 2002
//   Changed to use the base class's copy of the fileServer pointer.
//
//   Brad Whitlock, Mon Nov 11 15:25:45 PST 2002
//   I made the three file columns have minimum widths.
//
//   Brad Whitlock, Thu Mar 27 09:40:13 PDT 2003
//   I added a toggle for automatic file grouping.
//
//   Brad Whitlock, Tue Apr 22 15:26:51 PST 2003
//   I made the Ok button also be on the right.
//
//   Brad Whitlock, Fri Oct 10 15:08:00 PST 2003
//   I added a pushbutton to activate the path removal window.
//
//   Brad Whitlock, Thu Jul 29 13:50:53 PST 2004
//   I added support for smart file grouping.
//
//   Brad Whitlock, Fri Jul 30 12:01:32 PDT 2004
//   I hooked up some more signals and slots so pressing the Enter key will
//   let you select files and navigate directories.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Thu Jun 26 09:54:36 PDT 2008
//   Initial Qt4 Port.
//
//   Cyrus Harrison, Tue Jul  8 14:34:17 PDT 2008
//   Fixed some slot connections.
//
//   Brad Whitlock, Thu Jul 10 16:18:10 PDT 2008
//   Moved code to base class.
//
//   Cyrus Harrison, Thu Dec  4 09:13:50 PST 2008
//   Removed unnecssary todo comment.
//
// ****************************************************************************

void
QvisFileSelectionWindow::CreateWindowContents()
{   
    CreateHostPathFilterControls();

    // Add a grid layout for the file and directory lists.

    QSplitter *listSplitter = new QSplitter(central);
    listSplitter->setOrientation(Qt::Horizontal);
    topLayout->addWidget(listSplitter);

    //
    // Create the directory list.
    //
    QWidget *directoryWidget = new QWidget(listSplitter);
    QVBoxLayout *directoryVBox = new QVBoxLayout(directoryWidget);
    directoryVBox->setMargin(0);
    directoryVBox->addWidget(new QLabel(tr("Directories"), directoryWidget));
    directoryList = new QListWidget(directoryWidget);
    directoryVBox->addWidget(directoryList);
    int minColumnWidth = fontMetrics().width("X");
    directoryList->setMinimumWidth(minColumnWidth * 20);
    
    connect(directoryList, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
            this, SLOT(changeDirectory(QListWidgetItem *)));
    connect(directoryList, SIGNAL(itemActivated(QListWidgetItem *)),
            this, SLOT(changeDirectory(QListWidgetItem *)));
    
    //
    // Create the file list.
    //
    QWidget     *fileWidget = new QWidget(listSplitter);
    QVBoxLayout *fileVBox = new QVBoxLayout(fileWidget);
    fileVBox->setMargin(0);
    fileVBox->addWidget(new QLabel(tr("Files"), listSplitter));
    fileList = CreateFileListWidget(listSplitter);
    fileVBox->addWidget(fileList);
    fileList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    fileList->setMinimumWidth(minColumnWidth * 20);
    
    connect(fileList, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
            this, SLOT(selectFileDblClick(QListWidgetItem *)));
    connect(fileList, SIGNAL(itemActivated(QListWidgetItem *)),
            this, SLOT(selectFileReturnPressed(QListWidgetItem *)));
    connect(fileList, SIGNAL(itemSelectionChanged()),
            this, SLOT(selectFileChanged()));
    
    //
    // Create the selection buttons.
    //
    QWidget     *selectWidget = new QWidget(listSplitter);
    QVBoxLayout *selectVBox = new QVBoxLayout(selectWidget);
    selectVBox->setMargin(0);
    selectButton = new QPushButton(tr("Select"), selectWidget);
    connect(selectButton, SIGNAL(clicked()), this, SLOT(selectFile()));
    selectButton->setEnabled(false);
    selectAllButton = new QPushButton(tr("Select all"), selectWidget);
    connect(selectAllButton, SIGNAL(clicked()), this, SLOT(selectAllFiles()));
    removeButton = new QPushButton(tr("Remove"), selectWidget);
    connect(removeButton, SIGNAL(clicked()), this, SLOT(removeFile()));
    removeButton->setEnabled(false);
    removeAllButton = new QPushButton(tr("Remove all"), selectWidget);
    connect(removeAllButton, SIGNAL(clicked()), this, SLOT(removeAllFiles()));
    groupButton = new QPushButton(tr("Group"), selectWidget);
    connect(groupButton, SIGNAL(clicked()), this, SLOT(groupFiles()));
    groupButton->setEnabled(false);     // Until we have some selections
    refreshButton = new QPushButton(tr("Refresh"), selectWidget);
    connect(refreshButton, SIGNAL(clicked()), this, SLOT(refreshFiles()));
    selectVBox->addWidget(selectButton);
    selectVBox->addWidget(selectAllButton);
    selectVBox->addWidget(removeButton);
    selectVBox->addWidget(removeAllButton);
    selectVBox->addWidget(groupButton);
    selectVBox->addWidget(refreshButton);
    listSplitter->addWidget(selectWidget);

    //
    // Create the selected file list.
    //
    QWidget     *selfileWidget = new QWidget(listSplitter);
    QVBoxLayout *selfileVBox= new QVBoxLayout(selfileWidget);
    selfileVBox->setMargin(0);
    
    selfileVBox->addWidget(new QLabel(tr("Selected files"), selfileWidget));
    selectedFileList = CreateFileListWidget(selfileWidget);
    selfileVBox->addWidget(selectedFileList);
    
    selectedFileList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    selectedFileList->setMinimumWidth(minColumnWidth * 30);
    
    connect(selectedFileList, SIGNAL(itemSelectionChanged()),
            this, SLOT(selectedFileSelectChanged()));
    connect(selectedFileList, SIGNAL(itemActivated(QListWidgetItem *)),
            this, SLOT(removeSelectedFiles(QListWidgetItem *)));
    listSplitter->addWidget(selfileWidget);
            
    // Create the Ok button
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    topLayout->addLayout(buttonLayout);
    QPushButton *okButton = new QPushButton(tr("OK"), central);
    connect(okButton, SIGNAL(clicked()), this, SLOT(okClicked()));
    buttonLayout->addStretch(10);
    buttonLayout->addWidget(okButton);

    // Create the Cancel button
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), central);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelClicked()));
    buttonLayout->addWidget(cancelButton);

    // Get the applied file list from the file server and store it in
    // the intermediateFile list.
    intermediateFileList = fileServer->GetAppliedFileList();
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::UpdateWindow
//
// Purpose: 
//   This method is called when one of the window's subjects is updated.
//
// Arguments:
//   doAll : This tells the window to ignore selection and update everything.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 12 12:31:33 PDT 2002
//
// Modifications:
//   Brad Whitlock, Thu Jul 29 14:51:12 PST 2004
//   I renamed a method to UpdateHostComboBox.
//
// ****************************************************************************

void
QvisFileSelectionWindow::UpdateWindow(bool doAll)
{
    if(fs == 0 || profiles == 0)
        return;

    if(SelectedSubject() == fs || doAll)
    {
        UpdateWindowFromFiles(doAll);
        doAll = true;
    }
    if(SelectedSubject() == profiles || doAll)
        UpdateHostComboBox();
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::UpdateWindowFromFiles
//
// Purpose: 
//   This method is called when the window is notified by the
//   FileServerList object that there is a new file list or there is
//   a new path, filter, etc. This function is responsible for putting
//   the new values into the window's widgets.
//
// Arguments:
//   doAll : If this is true, ignore any partial selection on the 
//           FileServerList's attributes and update all the widgets in
//           the window.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 22 09:14:23 PDT 2000
//
// Modifications:
//   Brad Whitlock, Thu Jul 10 16:49:33 PDT 2008
//   Moved code to base class.
//
// ****************************************************************************

void
QvisFileSelectionWindow::UpdateWindowFromFiles(bool doAll)
{
    QvisFileWindowBase::UpdateWindowFromFiles(doAll);

    // If the appliedFileList has changed, update the appliedFile list.
    if(fileServer->AppliedFileListChanged() || doAll)
    {
        UpdateSelectedFileList();
    }
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::UpdateFileList
//
// Purpose: 
//   This method gets the filtered file list from the file server and
//   adds it to the list of files for the current directory.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 23 10:58:12 PDT 2000
//
// Modifications:
//   Brad Whitlock, Wed Oct 4 16:39:13 PST 2000
//   I rewrote the code since the file filtering now happens in the 
//   file server.
//
//   Brad Whitlock, Thu May 9 17:08:17 PST 2002
//   Made it use the base class's fileServer pointer.
//
//   Brad Whitlock, Fri Mar 28 12:28:04 PDT 2003
//   I made it use QFileSelectionListBoxItems and QVirtualFileListBoxItem.
//
//   Brad Whitlock, Thu Jul 10 16:01:05 PDT 2008
//   Moved code to base class.
//
// ****************************************************************************

void
QvisFileSelectionWindow::UpdateFileList()
{
    QvisFileWindowBase::UpdateFileList();

    // Update the "Select All" button
    UpdateSelectAllButton();
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::UpdateSelectAllButton
//
// Purpose: 
//   This method updates the enabled state of the "select all" button based
//   on the count of the files in the fileList.
//
// Programmer: Sean Ahern
// Creation:   Wed Aug 15 16:52:37 PDT 2001
//
// Modifications:
//
// ****************************************************************************
void
QvisFileSelectionWindow::UpdateSelectAllButton(void)
{
    if (fileList->count() >= 1)
        selectAllButton->setEnabled(true);
    else
        selectAllButton->setEnabled(false);
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::UpdateAppliedFileList
//
// Purpose: 
//   Makes the selectedFileList widget display the fileServer's
//   applied file list.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 28 14:17:47 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Nov 11 16:09:48 PST 2002
//   I changed the code so the window does not display host or path
//   information if it is not needed to identify a file.
//
//   Brad Whitlock, Fri Mar 28 14:52:43 PST 2003
//   I made it use QFileSelectionListBoxItems.
//
//   Brad Whitlock, Thu Jul 10 16:01:21 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisFileSelectionWindow::UpdateSelectedFileList()
{
    QualifiedFilenameVector::const_iterator pos;
    bool needsHost = false;
    bool needsPath = false;
    int i;

    //
    // Search through the list of selected files and see if we'll need to
    // display host or path information.
    //
    if(intermediateFileList.size() > 0)
    {
        std::string host = intermediateFileList[0].host;
        std::string path = intermediateFileList[0].path;
        for(i = 1; i < intermediateFileList.size(); ++i)
        {
            bool differentHost = (intermediateFileList[i].host != host);
            bool differentPath = (intermediateFileList[i].path != path);
            needsHost |= differentHost;
            needsPath |= differentPath;
        }
    }

    //
    // Add all the files in the intermediate file list to the
    // selectedFileList widget.
    //
    selectedFileList->clear();
    for(pos = intermediateFileList.begin();
        pos != intermediateFileList.end(); ++pos)
    {
        char sep[2] = {'\0', '\0'};

        QString fileName;
        if(needsHost)
            fileName += QString((pos->host + ":").c_str());
        if(needsPath)
        {
            sep[0] = pos->separator;
            fileName += QString((pos->path + sep).c_str());
        }
        fileName += QString(pos->filename.c_str());

        // Add the file to the selected file list.
        AddFileItem(selectedFileList, fileName, *pos);
    }

    if (selectedFileList->count() >= 1)
        removeAllButton->setEnabled(true);
    else
        removeAllButton->setEnabled(false);
}

//
// Qt Slot functions
//

// ****************************************************************************
// Method: QvisFileSelectionWindow::setEnabled
//
// Purpose: 
//   This Qt slot function sets the enabled state for the window's widgets.
//
// Arguments:
//   val : The new enabled state.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 30 10:07:38 PDT 2002
//
// Modifications:
//   Brad Whitlock, Mon Jul 14 11:00:44 PDT 2008
//   Moved code to base class.
//
// ****************************************************************************

void
QvisFileSelectionWindow::setEnabled(bool val)
{
    QvisFileWindowBase::setEnabled(val);

    if(isCreated)
        selectedFileList->setEnabled(val);
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::okClicked
//
// Purpose: 
//   This is a Qt slot function that stores the intermediate file list
//   into the file server's selected file list and notifies the 
//   file server's observers.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 11:01:04 PDT 2000
//
// Modifications:
//   Brad Whitlock, Mon Aug 27 11:21:29 PDT 2001
//   Renamed method from apply() to okClicked().
//
//   Brad Whitlock, Thu May 9 17:08:17 PST 2002
//   Made it use the base class's fileServer pointer.
//
//   Brad Whitlock, Wed Apr 2 10:03:35 PDT 2003
//   I added code to reopen the database on the viewer if the open file is
//   a virtual database.
//
//   Brad Whitlock, Mon Oct 13 11:25:26 PDT 2003
//   Added code to hide the recent paths removal window.
//
//   Brad Whitlock, Tue Dec 2 16:27:02 PST 2003
//   I added code to make sure that the virtual database definition is
//   different before deciding to reopen it on the viewer.
//
//   Brad Whitlock, Mon Jul 26 15:51:00 PST 2004
//   I improved the code so it will now tell the viewer to check all different
//   virtual databases for new states.
//
//   Brad Whitlock, Tue Mar 7 10:12:47 PDT 2006
//   I made it emit a signal.
//
//   Brad Whitlock, Mon Jul 14 11:06:19 PDT 2008
//   Moved some code to the base class.
//
// ****************************************************************************

void
QvisFileSelectionWindow::okClicked()
{
    int i;

    // Hide the remove path window.
    recentPathsRemovalWindow->hide();

    // Hide the window.
    hide();

    // Sort the intermediate file list before storing it.
    std::sort(intermediateFileList.begin(), intermediateFileList.end());

    // build vector of assciated time states
    std::vector<int> timeStates;
    for (i = 0; i < intermediateFileList.size(); ++i)
        timeStates.push_back(GetStateForSource(intermediateFileList[i]));

    // Store the intermediate file list into the file server's 
    // selected file list.
    fileServer->SetAppliedFileList(intermediateFileList, timeStates);
    fileServer->Notify();

    //
    // Check all of the virtual databases that VisIt has opened for new
    // time states so they are up to date with what the user is doing
    // outside of VisIt because they *are* dumping files, etc.
    // See if the open file is in the intermediate file list and if it is
    // in there and it is a virtual file, check for new states on the viewer.
    //
    CheckForNewStates();

    // Tell VisIt that the selected files list changed.
    emit selectedFilesChanged();
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::cancelClicked
//
// Purpose: 
//   This is a Qt slot function that sets the window's selected file
//   list back to the file server's list of selected files. Then it
//   hides the window.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 11:14:50 PDT 2000
//
// Modifications:
//   Brad Whitlock, Mon Aug 27 11:21:55 PDT 2001
//   Renamed method from dismiss() to cancelClicked().
//
//   Brad Whitlock, Thu May 9 17:08:17 PST 2002
//   Made it use the base class's fileServer pointer.
//
//   Brad Whitlock, Fri Oct 10 16:46:16 PST 2003
//   Forced it to hide the recentPathsRemovalWindow.
//
// ****************************************************************************

void
QvisFileSelectionWindow::cancelClicked()
{
    // Set the intermediate file list back to the applied list
    // and update the window.
    intermediateFileList = fileServer->GetAppliedFileList();
    UpdateSelectedFileList();

    // Hide the remove path window.
    recentPathsRemovalWindow->hide();

    // Hide the window.
    hide();
}




// ****************************************************************************
// Method: QvisFileSelectionWindow::selectFileDblClick
//
// Purpose: 
//   This is a Qt slot function that is called when a filename is
//   double-clicked. The filename that was double clicked is added
//   to the intermediate selected file list.
//
// Arguments:
//   item : A pointer to the widget that was double clicked.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 29 13:56:07 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Mar 28 15:01:17 PST 2003
//   I made it use QualifiedFilename.
//
//   Cyrus Harrison, Thu Jun 26 09:54:36 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisFileSelectionWindow::selectFileDblClick(QListWidgetItem *item) 
{
    // Unselect all the files.
    for(int i = 0; i < fileList->count(); ++i)
        fileList->item(i)->setSelected(false);

    // Add the file to the list.
    QualifiedFilename fn(DecodeQualifiedFilename(item->data(Qt::UserRole)));
    if(AddFile(fn))
        UpdateSelectedFileList();

    // The selection could have changed without us being informed, so
    // update the remove button
    UpdateRemoveFileButton();
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::selectFileChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the selection changes
//   in the file list.  The sensitivity of various widgets is adjusted to
//   match.
//
// Arguments:
//
// Programmer: Sean Ahern
// Creation:   Wed Aug 15 13:46:28 PDT 2001
//
// Modifications:
//   Cyrus Harrison, Thu Jun 26 09:54:36 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************
void
QvisFileSelectionWindow::selectFileChanged(void)
{
    // Count the number of selected files
    int count = 0;
    for(int i = 0; i < fileList->count(); ++i)
        if(fileList->item(i)->isSelected())
            count++;

    if (count >= 1)
        selectButton->setEnabled(true);
    else
        selectButton->setEnabled(false);

    if (count >= 2)
        groupButton->setEnabled(true);
    else
        groupButton->setEnabled(false);
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::selectedFileSelectChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the selection changes in
//   the selected file list.  The sensitivity of various widgets is adjusted
//   to match.
//
// Arguments:
//
// Programmer: Sean Ahern
// Creation:   Wed Aug 15 13:46:28 PDT 2001
//
// Modifications:
//   
// ****************************************************************************
void
QvisFileSelectionWindow::selectedFileSelectChanged(void)
{
    UpdateRemoveFileButton();
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::UpdateRemoveFileButton
//
// Purpose: 
//   This method updates the enabled state of the "remove" button based
//   on the count of the selected files in the selectedFileList.
//
// Programmer: Sean Ahern
// Creation:   Wed Aug 15 16:52:37 PDT 2001
//
// Modifications:
//   Cyrus Harrison, Thu Jun 26 09:54:36 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************
void
QvisFileSelectionWindow::UpdateRemoveFileButton(void)
{
    // Count the number of selected files
    int count = 0;
    for(int i = 0; i < selectedFileList->count(); ++i)
        if(selectedFileList->item(i)->isSelected())
            count++;

    if (count >= 1)
        removeButton->setEnabled(true);
    else
        removeButton->setEnabled(false);
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::selectFile
//
// Purpose: 
//   This is a Qt slot function that adds all of the highlighted files
//   in the fileList and adds them to the intermediate file list.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 29 13:57:41 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Mar 28 15:04:09 PST 2003
//   I made it use QFileSelectionListBoxItem.
//
//   Cyrus Harrison, Thu Jun 26 09:54:36 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisFileSelectionWindow::selectFile()
{
    int i, addCount = 0;

    // Add all the selected files to the intermediate file list.
    for(i = 0; i < fileList->count(); ++i)
    {
        if(!fileList->item(i)->isSelected())
            continue;

        // Add the file to the list if it's not in it.
        QListWidgetItem *item = fileList->item(i);
        QualifiedFilename fn(DecodeQualifiedFilename(item->data(Qt::UserRole)));
        if(AddFile(fn))
            ++addCount;
    }

    // Unselect all the files.
    for(i = 0; i < fileList->count(); ++i)
        fileList->item(i)->setSelected(false);

    // Update the window's selected files if any were added.
    if(addCount > 0)
        UpdateSelectedFileList();
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::selectFileReturnPressed
//
// Purpose: 
//   This method lets us select multiple files from the file list into the
//   selected file list by hitting the Enter key.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 30 11:03:49 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileSelectionWindow::selectFileReturnPressed(QListWidgetItem *)
{
    selectFile();
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::selectAllFiles
//
// Purpose: 
//   This is a Qt slot function that adds all of the files in the
//   fileList widget to the intermediate file list.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 29 13:58:58 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Mar 28 15:04:50 PST 2003
//   I made it use QFileSelectionListBoxItem.
//
//   Cyrus Harrison, Thu Jun 26 09:54:36 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisFileSelectionWindow::selectAllFiles()
{
    int i, addCount = 0;

    // Add all the selected files to the intermediate file list.
    for(i = 0; i < fileList->count(); ++i)
    {
        // Add the file to the list if it's not in it.
        QListWidgetItem *item = fileList->item(i);
        QualifiedFilename fn(DecodeQualifiedFilename(item->data(Qt::UserRole)));
        if(AddFile(fn))
            ++addCount;
    }

    // Unselect all the files.
    for(i = 0; i < fileList->count(); ++i)
        fileList->item(i)->setSelected(false);

    // Update the window's selected files if any were added.
    if(addCount > 0)
        UpdateSelectedFileList();

    // The selection could have changed without us being informed, so
    // update the remove button
    UpdateRemoveFileButton();
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::removeFile
//
// Purpose: 
//   This is a Qt slot function that removes all of highlighted files
//   in the selectedFileList widget from the intermediate file list.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 29 13:59:45 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Nov 11 16:22:43 PST 2002
//   I rewrote the code so can handle the case where the strings in the 
//   intermediate file list and the selected files widget do not exactly
//   match.
//
//   Cyrus Harrison, Thu Jun 26 09:54:36 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisFileSelectionWindow::removeFile()
{
    int removeCount = 0;
    QualifiedFilenameVector newIntermediateFileList;

    //
    // Create a new intermediate file list that does not contain the files
    // that are being removed.
    //
    for(int i = 0; i < selectedFileList->count(); ++i)
    {
        if(!selectedFileList->item(i)->isSelected())
            newIntermediateFileList.push_back(intermediateFileList[i]);
        else
            ++removeCount;
    }

    // Update the window's selected files if any were added.
    if(removeCount > 0)
    {
        intermediateFileList = newIntermediateFileList;
        UpdateSelectedFileList();
    }
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::removeSelectedFiles
//
// Purpose: 
//   This method lets us remove files from the selected files list by hitting
//   the Enter key.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 30 11:05:05 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileSelectionWindow::removeSelectedFiles(QListWidgetItem *)
{
    removeFile();
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::removeAllFiles
//
// Purpose: 
//   This is a Qt slot function that removes all files from the 
//   intermediate file list.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 29 14:01:42 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileSelectionWindow::removeAllFiles()
{
    // Remove all the files in the selected file list.
    intermediateFileList.clear();

    // Update the window's selected files if any were added.
    UpdateSelectedFileList();

    // Update the remove file button
    UpdateRemoveFileButton();
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::groupFiles
//
// Purpose: 
//   This is a Qt slot function that groups all of the highlighted files
//   in the fileList into a time series
//
// Programmer: Sean Ahern
// Creation:   Mon Feb 12 17:14:36 PST 2001
//
// Modifications:
//   Sean Ahern, Tue Aug 14 16:41:37 PDT 2001
//   Fixed a problem where the list could be empty.
//   
//   Brad Whitlock, Thu May 9 17:08:17 PST 2002
//   Made it use the base class's fileServer pointer.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Thu Jun 26 09:54:36 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisFileSelectionWindow::groupFiles()
{
    int i;
    stringVector groupList;

    // Add all the selected files to the intermediate file list.
    for(i = 0; i < fileList->count(); ++i)
    {
        if(!fileList->item(i)->isSelected())
            continue;

        groupList.push_back(fileList->item(i)->text().toStdString());
    }

    // Unselect all the files.
    for(i = 0; i < fileList->count(); ++i)
        fileList->item(i)->setSelected(false);

    // Sort the group list before storing it.
    std::sort(groupList.begin(), groupList.end());

    // Create a "group" filename to refer to all of the files.
    string group_filename = groupList[0];

    // Remove all of the numbers
    string::size_type ind;
    while((ind = group_filename.find_first_of("0123456789")) != string::npos)
        group_filename.erase(ind,1);

    // Get rid of any extension
    if ((ind = group_filename.rfind('.')) != string::npos)
        group_filename.erase(ind);

    // Put on ".visit"
    group_filename += ".visit";

    // Send the groupList and the group filename to the MDserver.
    QString tmp = tr("Creating group list on %1").
                  arg(fileServer->GetHost().c_str());
    Status(tmp);
    fileServer->CreateGroupList(group_filename, groupList);

    // Update the file list because it (hopefully) has changed as a result
    // of our actions.
    UpdateFileList();
    ClearStatus();
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::show
//
// Purpose: 
//   This is a Qt slot function that shows the window. It also refreshes
//   the file list each time the window is shown.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 11 14:54:05 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Sep 12 13:07:26 PST 2002
//   Changed the base class.
//
//   Brad Whitlock, Tue Jul 27 11:43:14 PDT 2004
//   Added a call to GetVirtualDatabaseDefinitions and code to update the
//   selected files list.
//
// ****************************************************************************

void
QvisFileSelectionWindow::show()
{
    //
    // Get the virtual file definitions for all of the virtual databases in the
    // applied file list. We have to do this so we can compare the virtual
    // file definition before we do anything with the definition after we've
    // done something in order to prevent reopening the virtual database any
    // more often than we need to reopen it. Files that are not virtual
    // return an empty definition.
    //
    GetVirtualDatabaseDefinitions(currentVirtualDatabaseDefinitions);

    // Show the window.
    QvisDelayedWindowSimpleObserver::show();

    // Refresh the files so the virtual databases will be right.
    refreshFiles();
    UpdateSelectedFileList();
}
