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

#include <qapplication.h>
#include <qcheckbox.h>
#include <qevent.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qsplitter.h>
#include <qvbox.h>

#include <QvisFileOpenWindow.h>
#include <QvisRecentPathRemovalWindow.h>
#include <FileServerList.h>
#include <BadHostException.h>
#include <CancelledConnectException.h>
#include <ChangeDirectoryException.h>
#include <CouldNotConnectException.h>
#include <DebugStream.h>
#include <GetFileListException.h>
#include <HostProfileList.h>
#include <HostProfile.h>
#include <ViewerProxy.h>
#include <DBPluginInfoAttributes.h>

#include <visit-config.h>

#include <algorithm>

using std::vector;
using std::string;

// Include icons
#include <icons/computer.xpm>
#include <icons/database.xpm>
#include <icons/folder.xpm>

#include <QFileSelectionListBoxItem.h>
#include <QVirtualFileListBoxItem.h>

// ****************************************************************************
// Method: QvisFileOpenWindow::QvisFileOpenWindow
//
// Purpose: 
//   Constructor for the QvisFileOpenWindow class.
//
// Arguments:
//   winCaption : The caption displayed in the window decorations.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//   Brad Whitlock, Wed Nov 15 15:35:41 PST 2006
//   Added usageMode.
//
// ****************************************************************************

QvisFileOpenWindow::QvisFileOpenWindow(const char *winCaption) :
    QvisDelayedWindowSimpleObserver(winCaption), intermediateFileList(),
    currentVirtualDatabaseDefinitions(), invalidHosts()
{
    fs = 0;
    profiles = 0;
    dbplugins = 0;

    computerPixmap = 0;
    folderPixmap = 0;
    databasePixmap = 0;

    recentPathsRemovalWindow = 0;

    usageMode = OpenFiles;

    // Set the progress callback that we want to use while we
    // connect to the mdserver.
    fileServer->SetProgressCallback(ProgressCallback, (void *)this);
}

// ****************************************************************************
// Method: QvisFileOpenWindow::~QvisFileOpenWindow
//
// Purpose: 
//   Destructor for the QvisFileOpenWindow class
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//
// ****************************************************************************

QvisFileOpenWindow::~QvisFileOpenWindow()
{
    if(fs)
        fs->Detach(this);
    if(profiles)
        profiles->Detach(this);
    if(dbplugins)
        dbplugins->Detach(this);

    delete computerPixmap;
    delete folderPixmap;
    delete databasePixmap;
}

// ****************************************************************************
// Method: QvisFileOpenWindow::SetUsageMode
//
// Purpose: 
//   Sets the usage mode for the window.
//
// Arguments:
//   m : The new usage mode.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 15 15:39:53 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileOpenWindow::SetUsageMode(QvisFileOpenWindow::UsageMode m)
{
    usageMode = m;
}

// ****************************************************************************
// Method: QvisFileOpenWindow::CreateWindowContents
//
// Purpose: 
//   This method is called to create the window the first time that
//   the window is requested. This delayed creation speeds startup.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//   Brad Whitlock, Wed Nov 15 15:37:44 PST 2006
//   Added usageMode.
//
// ****************************************************************************

void
QvisFileOpenWindow::CreateWindowContents()
{
    //
    // Create the pixmaps that we use in this window.
    //
    computerPixmap = new QPixmap(computer_xpm);
    folderPixmap = new QPixmap(folder_xpm);
    databasePixmap = new QPixmap(database_xpm);
    
    //
    // Create the path, filter
    //
    QGridLayout *pathLayout = new QGridLayout(topLayout, 3, 2);
    pathLayout->setSpacing(10);

    // Create the host
    hostComboBox = new QComboBox(true, central, "hostComboBox");
    hostComboBox->setDuplicatesEnabled(false);
    hostComboBox->setInsertionPolicy(QComboBox::AtTop);
    connect(hostComboBox, SIGNAL(activated(int)),
            this, SLOT(hostChanged(int)));
    QHBoxLayout *hostLayout = new QHBoxLayout;
    hostLayout->setSpacing(5);
    hostLayout->setMargin(0);
    QLabel *hostLabel = new QLabel(hostComboBox, "Host", central, "hostLabel");
    QLabel *hostImageLabel = new QLabel(central);
    hostImageLabel->setPixmap(*computerPixmap);
    hostImageLabel->setBuddy(hostComboBox);
    hostLayout->addWidget(hostImageLabel);
    hostLayout->addWidget(hostLabel);
    pathLayout->addLayout(hostLayout, 0, 0);
    pathLayout->addWidget(hostComboBox, 0, 1);

    // Create the path
    pathComboBox = new QComboBox(true, central, "pathComboBox");
    pathComboBox->setDuplicatesEnabled(false);
    pathComboBox->setInsertionPolicy(QComboBox::AtTop);
    connect(pathComboBox, SIGNAL(activated(int)),
            this, SLOT(pathChanged(int)));
    QHBoxLayout *pathLayout2 = new QHBoxLayout;
    pathLayout2->setSpacing(5);
    pathLayout2->setMargin(0);
    QLabel *pathLabel = new QLabel(pathComboBox, "Path", central, "pathLabel");
    QLabel *pathImageLabel = new QLabel(central);
    pathImageLabel->setPixmap(*folderPixmap);
    pathImageLabel->setBuddy(pathComboBox);
    pathLayout2->addWidget(pathImageLabel);
    pathLayout2->addWidget(pathLabel);
    pathLayout->addLayout(pathLayout2, 1, 0);
    pathLayout->addWidget(pathComboBox, 1, 1);

    // Create the filter
    filterLineEdit = new QLineEdit(central, "filterLineEdit");
    connect(filterLineEdit, SIGNAL(returnPressed()), this, SLOT(filterChanged()));
    QLabel *filterLabel = new QLabel(filterLineEdit, " Filter", central, "filterLabel");
    pathLayout->addWidget(filterLabel, 2, 0, Qt::AlignRight);
    pathLayout->addWidget(filterLineEdit, 2, 1);

    // Create the current dir toggle.
    QHBoxLayout *toggleLayout = new QHBoxLayout(topLayout);
    toggleLayout->setSpacing(10);
    currentDirToggle = new QCheckBox("Use \"current working directory\" by "
       "default", central, "currentDirToggle");
    connect(currentDirToggle, SIGNAL(toggled(bool)),
            this, SLOT(currentDir(bool)));
    toggleLayout->addWidget(currentDirToggle);

    // Create the file grouping checkbox.
    fileGroupingComboBox = new QComboBox("File grouping",
        central, "fileGroupingComboBox");
    fileGroupingComboBox->insertItem("Off", 0);
    fileGroupingComboBox->insertItem("On", 1);
    fileGroupingComboBox->insertItem("Smart", 2);
    fileGroupingComboBox->setEditable(false);
    connect(fileGroupingComboBox, SIGNAL(activated(int)),
            this, SLOT(fileGroupingChanged(int)));
    toggleLayout->addStretch(5);
    toggleLayout->addWidget(new QLabel(fileGroupingComboBox,
        "File grouping", central, "fileGroupingLabel"), 0, Qt::AlignRight);
    toggleLayout->addWidget(fileGroupingComboBox, 0, Qt::AlignLeft);
    toggleLayout->addStretch(5);

    // Create a window we can activate to remove recent paths.
    recentPathsRemovalWindow = new QvisRecentPathRemovalWindow(fileServer,
       "Remove recent paths");
    recentPathRemovalButton = new QPushButton("Remove paths . . .", central,
        "recentPathRemovalButton");
    connect(recentPathRemovalButton, SIGNAL(clicked()),
            recentPathsRemovalWindow, SLOT(show()));
    toggleLayout->addWidget(recentPathRemovalButton);

    // Add a grid layout for the file and directory lists.
    topLayout->addSpacing(10);
    QSplitter *listSplitter = new QSplitter(central, "listSplitter");
    listSplitter->setOrientation(Qt::Horizontal);
    topLayout->addWidget(listSplitter);

    //
    // Create the directory list.
    //
    QVBox *directoryVBox = new QVBox(listSplitter, "directoryVBox");
    new QLabel("Directories", directoryVBox, "directoryLabel");
    directoryList = new QListBox(directoryVBox, "directoryList");
    int minColumnWidth = fontMetrics().width("X");
    directoryList->setMinimumWidth(minColumnWidth * 20);
    connect(directoryList, SIGNAL(doubleClicked(QListBoxItem *)),
            this, SLOT(changeDirectory(QListBoxItem *)));
    connect(directoryList, SIGNAL(returnPressed(QListBoxItem *)),
            this, SLOT(changeDirectory(QListBoxItem *)));

    //
    // Create the file list.
    //
    QVBox *fileVBox = new QVBox(listSplitter, "fileVBox");
    new QLabel("Files", fileVBox, "fileLabel");
    fileList = new QListBox(fileVBox, "fileList");
    if(usageMode == OpenFiles)
        fileList->setSelectionMode(QListBox::Extended);
    else
        fileList->setSelectionMode(QListBox::Single);
    fileList->setMinimumWidth(minColumnWidth * 20);
    connect(fileList, SIGNAL(doubleClicked(QListBoxItem *)),
            this, SLOT(selectFileDblClick(QListBoxItem *)));
    connect(fileList, SIGNAL(returnPressed(QListBoxItem *)),
            this, SLOT(selectFileReturnPressed(QListBoxItem *)));
    connect(fileList, SIGNAL(selectionChanged()),
            this, SLOT(selectFileChanged()));

    // Create the file format combo box
    QHBoxLayout *fileFormatLayout = new QHBoxLayout(topLayout);
    fileFormatLayout->addWidget(new QLabel("Open file as type:", central));
    fileFormatComboBox = new QComboBox(false, central, "fileFormatComboBox");
    fileFormatLayout->addWidget(fileFormatComboBox, 10);
    fileFormatLayout->addStretch(5);

    // create the lower button layout
    QHBoxLayout *buttonLayout = new QHBoxLayout(topLayout);

    // Create the refresh button
    refreshButton = new QPushButton("Refresh", central, "refreshButton");
    connect(refreshButton, SIGNAL(clicked()), this, SLOT(refreshFiles()));
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch(10);

    // Create the Ok button
    okButton = new QPushButton("OK", central, "okButton");
    okButton->setEnabled(false);
    connect(okButton, SIGNAL(clicked()), this, SLOT(okClicked()));
    buttonLayout->addWidget(okButton);

    // Create the Cancel button
    cancelButton = new QPushButton("Cancel", central, "cancelButton");
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelClicked()));
    buttonLayout->addWidget(cancelButton);

    // Get the applied file list from the file server and store it in
    // the intermediateFile list.
    intermediateFileList = fileServer->GetAppliedFileList();
}

// ****************************************************************************
// Method: QvisFileOpenWindow::UpdateWindow
//
// Purpose: 
//   This method is called when one of the window's subjects is updated.
//
// Arguments:
//   doAll : This tells the window to ignore selection and update everything.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//
// ****************************************************************************

void
QvisFileOpenWindow::UpdateWindow(bool doAll)
{
    if (fs == 0 || profiles == 0 || dbplugins == 0)
        return;

    if (SelectedSubject() == fs || doAll)
    {
        UpdateWindowFromFiles(doAll);
        doAll = true;
    }

    if (SelectedSubject() == profiles || doAll)
        UpdateHostComboBox();

    if (SelectedSubject() == dbplugins || doAll)
        UpdateFileFormatComboBox();
}

// ****************************************************************************
// method: QvisFileOpenWindow::UpdateWindowFromFiles
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
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//
// ****************************************************************************

void
QvisFileOpenWindow::UpdateWindowFromFiles(bool doAll)
{
    // Set the working directory toggle.
    currentDirToggle->blockSignals(true);
    currentDirToggle->setChecked(fileServer->GetUseCurrentDirectory());
    currentDirToggle->blockSignals(false);

    // Set the file grouping combo box.
    if(fileServer->IsSelected(7) || fileServer->IsSelected(9) || doAll)
    {
        int index = 0;
        if(fileServer->GetAutomaticFileGrouping())
        {
            ++index;
            if(fileServer->GetSmartFileGrouping())
                ++index;
        }
        fileGroupingComboBox->blockSignals(true);
        fileGroupingComboBox->setCurrentItem(index);
        fileGroupingComboBox->blockSignals(false);
    }

    // If the host flag is set, update the host combo box.
    if(fileServer->HostChanged() || doAll)
    {
        // Fill the combo box with the recently visited hosts.
        UpdateHostComboBox();
    }

    // If the path flag is set, update the path combo box.
    if(fileServer->PathChanged() || fileServer->RecentPathsChanged() || doAll)
    {
        TRY
        {
            // Fill the combo box with the recently visited paths.
            UpdateComboBox(pathComboBox,
                           fileServer->GetRecentPaths(fileServer->GetHost()),
                           fileServer->GetPath().c_str());
        }
        CATCH(BadHostException)
        {
            stringVector paths;
            paths.push_back(fileServer->GetPath());
            UpdateComboBox(pathComboBox,
                           paths,
                           fileServer->GetPath().c_str());
        }
        ENDTRY
    }

    bool updateTheFileList = false;
    // If the filter flag is set, update the filter text field.
    if(fileServer->FilterChanged() || doAll)
    {
        filterLineEdit->setText(QString(fileServer->GetFilter().c_str()));
        updateTheFileList = true;
    }

    // If the host or the path changed, we must update both the directory
    // list and the file list.
    if(fileServer->HostChanged() || fileServer->PathChanged() || doAll)
    {
        UpdateDirectoryList();
        updateTheFileList = true;
    }

    // If we need to update the file list, then do it.
    if(updateTheFileList || fileServer->FileListChanged())
        UpdateFileList();
}

// ****************************************************************************
// Method: QvisFileOpenWindow::UpdateHostComboBox
//
// Purpose: 
//   This method is called when the host profile list changes. The purpose is
//   to add any new host names into the host combo box so they are easily
//   accessible.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//
// ****************************************************************************

void
QvisFileOpenWindow::UpdateHostComboBox()
{
    // Get the starting list of hosts from the file server.
    stringVector hosts(fileServer->GetRecentHosts());

    //
    // Add all of the hosts from the host profiles that are not already
    // in the hosts list.
    //
    for(int i = 0; i < profiles->GetNumHostProfiles(); ++i)
    {
        // Create a constant reference to the i'th profile.
        const HostProfile &p = profiles->operator[](i);

        stringVector hostNames(p.SplitHostPattern(p.GetHost()));
        for (int k = 0; k < hostNames.size(); ++k)
        {
            if(std::find(hosts.begin(), hosts.end(), hostNames[k]) == hosts.end())
                hosts.push_back(hostNames[k]);
        }
    }

    //
    // Remove any hosts that are in the invalidHosts list.
    //
    if(invalidHosts.size() > 0)
    {
        for(int i = 0; i < invalidHosts.size(); ++i)
        {
            stringVector::iterator pos = std::find(hosts.begin(), hosts.end(),
                invalidHosts[i]);
            if(pos != hosts.end())
                hosts.erase(pos);
        }
    }

    //
    // Sort the host list and update the combo box.
    //
    std::sort(hosts.begin(), hosts.end());
    UpdateComboBox(hostComboBox, hosts, fileServer->GetHost().c_str());
}

// ****************************************************************************
// Method: QivsFileSelectionWindow::UpdateComboBox
//
// Purpose: 
//   Populates the specified combo box with the appropriate entries.
//
// Arguments:
//   cb : The combo box to update.
//   s  : The list of entries.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//
// ****************************************************************************

void
QvisFileOpenWindow::UpdateComboBox(QComboBox *cb, const stringVector &s,
    const QString &activeItem)
{
    cb->blockSignals(true);
    cb->clear();

    // Populate the combo box.
    int i;
    for(i = 0; i < s.size(); ++i)
        cb->insertItem(s[i].c_str());

    // Set the current item.
    QListBox *lb = cb->listBox();
    int index = lb->index(lb->findItem(activeItem));
    if(index == -1)
    {
        cb->insertItem(activeItem);
        index = lb->index(lb->findItem(activeItem));
    }
    cb->setCurrentItem(index);
    cb->setEditText(activeItem);

    // Get the combo box's list box and set its minimum height so we don't
    // have to scroll so much.
    int h = 0;
    for(i = 0; i < s.size(); ++i)
        h += lb->itemRect(lb->item(i)).height();
    int maxH = qApp->desktop()->height() / 2;
    h = (h > maxH) ? maxH : h;
    lb->setMinimumHeight(h);

    cb->blockSignals(false);
}

// ****************************************************************************
// Method: QvisFileOpenWindow::UpdateDirectoryList
//
// Purpose: 
//   This method adds all of the directories in the FileServer's
//   file list to the window's directory listbox.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//
// ****************************************************************************

void
QvisFileOpenWindow::UpdateDirectoryList()
{
    const MDServerProxy::FileList &f = fileServer->GetFileList();

    // Get the strings for current directory and up directory.
    QString curDirString, upDirString;
    GetDirectoryStrings(curDirString, upDirString);

    // Iterate through the the directory list and add the dirs.
    directoryList->clear();
    MDServerProxy::FileEntryVector::const_iterator pos;
    for(pos = f.dirs.begin(); pos != f.dirs.end(); ++pos)
    {
        if(pos->name == std::string("."))
        {
            QualifiedFilename dirName(curDirString.latin1());
            dirName.SetAccess(pos->CanAccess());
            new QFileSelectionListBoxItem(directoryList, curDirString, dirName);
        }
        else if(pos->name == std::string(".."))
        {
            QualifiedFilename dirName(upDirString.latin1());
            dirName.SetAccess(pos->CanAccess());
            new QFileSelectionListBoxItem(directoryList, upDirString, dirName);
        }
        else
        {
            QualifiedFilename dirName(pos->name);
            dirName.SetAccess(pos->CanAccess());
            new QFileSelectionListBoxItem(directoryList, pos->name.c_str(),
                dirName, folderPixmap);
        }
    }
}

// ****************************************************************************
// Method: QvisFileOpenWindow::UpdateFileList
//
// Purpose: 
//   This method gets the filtered file list from the file server and
//   adds it to the list of files for the current directory.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//
// ****************************************************************************

void
QvisFileOpenWindow::UpdateFileList()
{
    fileList->clear();

    // Get the filtered file list from the file server. Then iterate through
    // the file list and add the files.
    QualifiedFilenameVector filteredFiles(fileServer->GetFilteredFileList());
    for(QualifiedFilenameVector::iterator pos = filteredFiles.begin();
        pos != filteredFiles.end(); ++pos)
    {
         QString name(pos->filename.c_str());
         if(pos->IsVirtual())
         {
             new QVirtualFileListBoxItem(fileList, name, *pos,
                 fileServer->GetVirtualFileDefinition(*pos), databasePixmap);
         }
         else
             new QFileSelectionListBoxItem(fileList, name, *pos);
    }
}

// ****************************************************************************
// Method: QvisFileOpenWindow::RemoveComboBoxItem
//
// Purpose: 
//   Removes an item from a combo box and makes another item active.
//
// Arguments:
//   cb        : The combo box that we're operating on.
//   remove    : The item to remove.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileOpenWindow::RemoveComboBoxItem(QComboBox *cb,
    const QString &remove)
{
    QListBox *lb = cb->listBox();
    int index = lb->index(lb->findItem(remove));
    if(index != -1)
    {
         cb->blockSignals(true);
         cb->removeItem(index);
         cb->setEditText(remove);
         cb->blockSignals(false);
    }
}

// ****************************************************************************
// Method: QvisFileOpenWindow::ActivateComboBoxItem
//
// Purpose: 
//   Makes an entry in the combo box active.
//
// Arguments:
//   cb        : The combobox to use.
//   newActive : The name of the item to activate.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileOpenWindow::ActivateComboBoxItem(QComboBox *cb,
    const QString &newActive)
{
    QListBox *lb = cb->listBox();
    int index = lb->index(lb->findItem(newActive));
    if(index != -1)
    {
         cb->blockSignals(true);
         cb->setCurrentItem(index);
         cb->setEditText(newActive);
         cb->blockSignals(false);
    }
}

// ****************************************************************************
// Method: QvisFileOpenWindow::HighlightComboBox
//
// Purpose: 
//   Highlights the combo box by selecting its text and giving it focus.
//
// Arguments:
//   cb : The combobox to highlight.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileOpenWindow::HighlightComboBox(QComboBox *cb)
{
    // Select the line edit's text.
    QLineEdit *le = cb->lineEdit();
    le->setSelection(0, le->text().length());
    le->setFocus();
}

// ****************************************************************************
// Method: QvisFileOpenWindow::AddFile
//
// Purpose: 
//   Adds a filename to the intermediate selected file list if
//   the filename is not already in the list.
//
// Arguments:
//   newFile : The file to add to the intermediate file list.
//
// Returns:    
//   true is the file had to be added, false otherwise.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//
// ****************************************************************************

bool
QvisFileOpenWindow::AddFile(const QualifiedFilename &newFile)
{
    // If newFile is not already in the intermediateFileList
    // then add it.
    QualifiedFilenameVector::iterator pos;
    bool found = false;
    for(pos = intermediateFileList.begin();
        pos != intermediateFileList.end() && !found; ++pos)
    {
        found = (newFile  == *pos);
    }
    if(!found)
        intermediateFileList.push_back(newFile);

    return !found || newFile.IsVirtual();
}

// ****************************************************************************
// Method: QvisFileOpenWindow::GetCurrentValues
//
// Purpose: 
//   Gets the current values for the host, path, and filter and applies the
//   ones that have changed.
//
// Arguments:
//   allowPathChange : If true, this allows the path to be changed regardless
//                     of whether or not the path is different. This is useful
//                     for re-reading a directory.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//
// ****************************************************************************

void
QvisFileOpenWindow::GetCurrentValues(bool allowPathChange)
{
    // Get the path string from before the possible host change.
    std::string beforePath(pathComboBox->currentText().stripWhiteSpace().latin1());

    // Changes the host if it is different from the host in the file server.
    bool errFlag = ChangeHosts();

    // Get the path from after the possible host change.
    std::string afterPath(fileServer->GetPath());

    // Changes the path if it is different from the path in the file server.
    if(!errFlag)
    {
        // If there was a host change and the paths are different, set the
        // before path
        if((beforePath.length() > 0) && (beforePath != afterPath) &&
           allowPathChange)
        {
            pathComboBox->setEditText(QString(beforePath.c_str()));
        }
        // Try and change the path.
        errFlag = ChangePath(allowPathChange);
    }

    // Changes the filter.
    if(!errFlag)
        ChangeFilter();
}

// ****************************************************************************
// Method: QvisFileOpenWindow::ChangeHosts
//
// Purpose: 
//   This method is called to change hosts.
//
// Returns:    true is there was an error.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//
// ****************************************************************************

bool
QvisFileOpenWindow::ChangeHosts()
{
    bool errFlag = false;

    // If the line edit is not empty, change the host name.
    if(!hostComboBox->currentText().isEmpty())
    {
        // Take the string from the text field and strip whitespace.
        std::string host(hostComboBox->currentText().stripWhiteSpace().latin1());
        std::string currentHost(fileServer->GetHost());

        if(host != fileServer->GetHost())
        {
            // Put a message on the status line.
            QString temp;
            temp.sprintf("Opening server on %s", host.c_str());
            Status(temp);

            // Change the application cursor to the wait cursor.
            SetWaitCursor();

            bool repeat;
            int  loopCount = 0;
            do
            {
                repeat = false;

                TRY
                {
                    // Try to set the host name
                    fileServer->SetHost(host);
                    fileServer->Notify();

                    // If the host is in the invalidHosts list then remove it.
                    stringVector::iterator pos = std::find(invalidHosts.begin(),
                        invalidHosts.end(), host);
                    if(pos != invalidHosts.end())
                        invalidHosts.erase(pos);
                }
                CATCH(BadHostException)
                {
                    // Tell the user that the hostname is not valid.
                    QString msgStr;
                    msgStr.sprintf("\"%s\" is not a valid host.", host.c_str());
                    Error(msgStr);

                    // Remove the invalid host from the combo box and make the
                    // active host active in the combo box.
                    invalidHosts.push_back(host);
                    UpdateHostComboBox();
                    hostComboBox->setEditText(host.c_str());
                    HighlightComboBox(hostComboBox);

                    // We had an error.
                    errFlag = true;
                }
                CATCH(CouldNotConnectException)
                {
                    // We had an error.
                    errFlag = true;
                }
                CATCH(CancelledConnectException)
                {
                    // We had an error.
                    errFlag = true;

                    // Let's restore the old host.
                    host = currentHost;
                    repeat = (loopCount++ == 0);
                }
                ENDTRY
            } while(repeat);

            // Clear the status line.
            ClearStatus();

            // Restore the last cursor.
            RestoreCursor();

            // Make this window be the active window and raise it.
            topLevelWidget()->setActiveWindow();
            topLevelWidget()->raise();
        }
    }

    return errFlag;
}

// ****************************************************************************
// Method: QvisFileOpenWindow::ChangePath
//
// Purpose: 
//   This method is called to change the path.
//
// Arguments:
//   allowPathChange : If set to true, path changes are allowed regardless
//                     of whether or not the path actually changed.
//
// Returns:    true is there was an error.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//
// ****************************************************************************

bool
QvisFileOpenWindow::ChangePath(bool allowPathChange)
{
    bool errFlag = false;

    // If the line edit is not empty, change the host name.
    if(!pathComboBox->currentText().isEmpty())
    {
        // Take the string from the text field and strip whitespace.
        std::string path(pathComboBox->currentText().stripWhiteSpace().latin1());
        bool pathNeedsSet = true;

        // If the paths are different or we are allowing a path change,
        // change the path to the new value.
        if(path != fileServer->GetPath() || allowPathChange)
        {
            // Put a message on the status line.
            QString temp("Changing directory.");
            Status(temp);

            // Set the path in the file server.
            TRY
            {
                fileServer->SetPath(path);
                fileServer->Notify();
            }
            CATCH(ChangeDirectoryException)
            {
                // Create a message and tell the user.
                QString msgStr;
                msgStr.sprintf("The MetaData server running on %s "
                    "could not change the current directory to %s.",
                    fileServer->GetHost().c_str(), path.c_str());
                Error(msgStr);
                errFlag = true;

                // Remove the invalid host from the combo box and make the
                // active host active in the combo box.
                RemoveComboBoxItem(pathComboBox, path.c_str());
                HighlightComboBox(pathComboBox);

                pathNeedsSet = false;
            }
            CATCH(GetFileListException)
            {
                // Update the file list, it will be empty.
                UpdateDirectoryList();
                UpdateFileList();

                // Create a message and tell the user.
                QString msgStr;
                msgStr.sprintf("The MetaData server running on %s "
                "could not get the file list for the current directory",
                fileServer->GetHost().c_str());
                Error(msgStr);
                errFlag = true;
            }
            ENDTRY

            // Clear the status line.
            ClearStatus();
        }

        // If the path is not the same as the filtered path then we have
        // put a duplicate into the combobox and we should remove it.
        if(path != fileServer->GetPath() && pathNeedsSet)
        {
            RemoveComboBoxItem(pathComboBox, path.c_str());
            ActivateComboBoxItem(pathComboBox, fileServer->GetPath().c_str());
        }
    }
    else
    {
        // Set the path to the last good value.
        pathComboBox->setEditText(QString(fileServer->GetPath().c_str()));
    }

    return errFlag;
}

// ****************************************************************************
// Method: QvisFileOpenWindow::ChangeFilter
//
// Purpose: 
//   This method is called to change filters.
//
// Returns:    true is there was an error.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//
// ****************************************************************************

bool
QvisFileOpenWindow::ChangeFilter()
{
    bool errFlag = false;
    bool forcedChange = false;
    std::string filter("*");

    // If the line edit is not empty, change the host name.
    if(!filterLineEdit->text().isEmpty())
    {
        // Take the string from the text field and simplify whitespace.
        filter = filterLineEdit->text().simplifyWhiteSpace().latin1();

        if(filter == "")
        {
            forcedChange = true;
            filter = "*";
        }
    }
    else
    {
        filter = "*";
        forcedChange = true;
    }

    // If the filters are different, modify the filter in the fileserver.
    bool exceptionErr = false;
    if(filter != fileServer->GetFilter() || forcedChange)
    {
        // Try and set the filter in the file server.
        if(filter.length() > 0)
        {
            TRY
            {
                fileServer->SetFilter(filter);
                fileServer->Notify();
            }
            CATCH(GetFileListException)
            {
                Error("The MetaData server running could not get the file "
                      "list for the current directory, which is required "
                      "before setting the file filter. Try entering a "
                      "valid path before changing the file filter.");
                exceptionErr = true;
            }
            CATCH(VisItException)
            {
                Error("An error occured when trying to set the file filter.");
                exceptionErr = true;
            }
            ENDTRY
        }
        else
        {
            errFlag = true;
        }
    }

    if(errFlag || forcedChange)
    {
        Error("An invalid filter was entered.");
        filterLineEdit->setText(QString(fileServer->GetFilter().c_str()));
    }

    return errFlag || exceptionErr;
}

// ****************************************************************************
// Method: QvisFileOpenWindow::GetDirectoryStrings
//
// Purpose: 
//   Creates the current directory and up directory strings based on the
//   current host.
//
// Returns:    The upDir and curDir strings.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//
// ****************************************************************************

void
QvisFileOpenWindow::GetDirectoryStrings(QString &curDir, QString &upDir)
{
    curDir = ". (current directory)";
    upDir = ".. (go up 1 directory level)";
}

// ****************************************************************************
// Method: QvisFileOpenWindow::SubjectRemoved
//
// Purpose: 
//   This function is called when a subject is removed.
//
// Arguments:
//   TheRemovedSubject : The subject being removed.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileOpenWindow::SubjectRemoved(Subject *TheRemovedSubject)
{
    if(TheRemovedSubject == fs)
        fs = 0;
    else if(TheRemovedSubject == profiles)
        profiles = 0;
    else if (TheRemovedSubject == dbplugins)
        dbplugins = 0;
}

// ****************************************************************************
// Method: QvisFileOpenWindow::ConnectSubjects
//
// Purpose: 
//   This function connects subjects so that the window observes them.
//
// Arguments:
//   hpl : The host profile list that we want to observe.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileOpenWindow::ConnectSubjects(HostProfileList *hpl,
                                    DBPluginInfoAttributes *dbp)
{
    fs = fileServer;
    fileServer->Attach(this);

    profiles = hpl;
    profiles->Attach(this);

    dbplugins = dbp;
    dbplugins->Attach(this);
}

// ****************************************************************************
// Method: QvisFileOpenWindow::ProgressCallback
//
// Purpose: 
//   This is a progress callback function for the FileServerList class and
//   eventually RemoteProcess. It is called when we have to connect to a new
//   mdserver. It prevents the user from changing anything in the window plus
//   the window is modal so the rest of the GUI cannot be changed.
//
// Arguments:  
//   data  : A void pointer to the file selection window.
//   stage : The stage of the process launch.
//
// Returns:    true
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//
// ****************************************************************************

bool
QvisFileOpenWindow::ProgressCallback(void *data, int stage)
{
    QvisFileOpenWindow *This = (QvisFileOpenWindow *)data;

    if(stage == 0)
    {
        // Disable the widgets in the file selection window.
        This->setEnabled(false);
    }
    else if(stage == 1)
    {
#if QT_VERSION >= 300 && !defined(Q_WS_MACX)
       if(qApp->hasPendingEvents())
#endif
           qApp->processOneEvent();
    }
    else
    {
        // Enable the widgets in the file selection window.
        This->setEnabled(true);
    }

    return true;
}

// ****************************************************************************
// Method: QvisFileOpenWindow::GetVirtualDatabaseDefinitions
//
// Purpose: 
//   Populates a map with virtual database definitions.
//
// Arguments:
//   defs : The map into which we'll store the virtual database definitions.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileOpenWindow::GetVirtualDatabaseDefinitions(
    StringStringVectorMap &defs)
{
    const QualifiedFilenameVector &files = fileServer->GetAppliedFileList();

    // Add the definitions for all virtual files to the map.
    defs.clear();
    for(int i = 0; i < files.size(); ++i)
    {
        if(files[i].IsVirtual())
        {
            defs[files[i].FullName()] = 
                fileServer->GetVirtualFileDefinition(files[i].FullName());
        }
    }
}

//
// Qt Slot functions
//

// ****************************************************************************
// Method: QvisFileOpenWindow::setEnabled
//
// Purpose: 
//   This Qt slot function sets the enabled state for the window's widgets.
//
// Arguments:
//   val : The new enabled state.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileOpenWindow::setEnabled(bool val)
{
    QvisDelayedWindowSimpleObserver::setEnabled(val);

    if(isCreated)
    {
        hostComboBox->setEnabled(val);
        pathComboBox->setEnabled(val);
        filterLineEdit->setEnabled(val);
        directoryList->setEnabled(val);
        fileList->setEnabled(val);
        currentDirToggle->setEnabled(val);
    }
}

// ****************************************************************************
// Method: QvisFileOpenWindow::okClicked
//
// Purpose: 
//   This is a Qt slot function that stores the intermediate file list
//   into the file server's selected file list and notifies the 
//   file server's observers.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//   Brad Whitlock, Wed Nov 15 15:15:01 PST 2006
//   Added code to emit a signal.
//
// ****************************************************************************

void
QvisFileOpenWindow::okClicked()
{
    // Add all the selected files to the intermediate file list.
    QualifiedFilename emitFile;
    for (int i = 0; i < fileList->count(); ++i)
    {
        if (!fileList->isSelected(i))
            continue;

        // Add the file to the list if it's not in it.
        QFileSelectionListBoxItem *item = (QFileSelectionListBoxItem *)fileList->item(i);
        AddFile(item->GetFilename());

        string forcedFormat = "";
        if (fileFormatComboBox->currentItem() != 0)
        {
            QString tmp = fileFormatComboBox->currentText();
            int ntypes = dbplugins->GetTypes().size();
            for (int j = 0 ; j < ntypes ; j++)
            {
                if (dbplugins->GetTypes()[j] == tmp.latin1())
                {
                    forcedFormat = dbplugins->GetTypesFullNames()[j];
                    break;
                }
            }
        }

        // Save the name of the first file.
        if(emitFile.Empty())
            emitFile = item->GetFilename();

        GetViewerMethods()->OpenDatabase(item->GetFilename().FullName().c_str(),
            0, true, forcedFormat);
    }

    // Hide the remove path window.
    recentPathsRemovalWindow->hide();

    // Hide the window.
    hide();

    // Sort the intermediate file list before storing it.
    std::sort(intermediateFileList.begin(), intermediateFileList.end());

    // Store the intermediate file list into the file server's 
    // selected file list.
    fileServer->SetAppliedFileList(intermediateFileList);
    fileServer->Notify();

    // Get the virtual file definitions now that we've selected files.
    StringStringVectorMap newDefinitions;
    GetVirtualDatabaseDefinitions(newDefinitions);

    //
    // Check all of the virtual databases that VisIt has opened for new
    // time states so they are up to date with what the user is doing
    // outside of VisIt because they *are* dumping files, etc.
    // See if the open file is in the intermediate file list and if it is
    // in there and it is a virtual file, check for new states on the viewer.
    //
    for(int i = 0; i < intermediateFileList.size(); ++i)
    {
        if(intermediateFileList[i].IsVirtual())
        {
            std::string fileName(intermediateFileList[i].FullName());
            StringStringVectorMap::const_iterator oldDef = 
                currentVirtualDatabaseDefinitions.find(fileName);
            StringStringVectorMap::const_iterator newDef =
                newDefinitions.find(fileName);

            if(oldDef != currentVirtualDatabaseDefinitions.end() &&
               newDef != newDefinitions.end())
            {
                //
                // Get the virtual file definition and compare it to the
                // definition that we obtained when we brought up the window.
                //
                if(oldDef->second == newDef->second)
                {
                    debug1 << "QvisFileOpenWindow::okClicked: The virtual "
                           << "database definition for "
                           << fileName.c_str()
                           << " did not change. No reopen is required."
                           << endl;
                }
                else
                {
                    debug1 << "QvisFileOpenWindow::okClicked: The virtual "
                           << "database definition for "
                           << fileName.c_str()
                           << " changed! Checking for new states is required on "
                           << "the viewer to update the length of the animations."
                           << endl;

                    //
                    // Tell the viewer to check the virtual database for new
                    // time states so the information about the file remains
                    // up to date.
                    //
                    GetViewerMethods()->CheckForNewStates(fileName);
                }
            }
        }
    }

    // We selected a file so emit the name of that file.
    if(usageMode == SelectFilename)
        emit selectedFile(QString(emitFile.FullName().c_str()));
}

// ****************************************************************************
// Method: QvisFileOpenWindow::cancelClicked
//
// Purpose: 
//   This is a Qt slot function that sets the window's selected file
//   list back to the file server's list of selected files. Then it
//   hides the window.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//   Brad Whitlock, Wed Nov 15 15:14:46 PST 2006
//   Added code to emit a signal.
//
// ****************************************************************************

void
QvisFileOpenWindow::cancelClicked()
{
    // Set the intermediate file list back to the applied list
    // and update the window.
    intermediateFileList = fileServer->GetAppliedFileList();

    // Hide the remove path window.
    recentPathsRemovalWindow->hide();

    // Hide the window.
    hide();

    // We did not select a file so tell clients.
    if(usageMode == SelectFilename)
        emit selectCancelled();
}

// ****************************************************************************
// Method: QvisFileOpenWindow::filterChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the contents of
//   filter text field change.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//
// ****************************************************************************

void
QvisFileOpenWindow::filterChanged()
{
    GetCurrentValues(false);
}

// ****************************************************************************
// Method: QvisFileOpenWindow::hostChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the contents of
//   the host text field change. This tells the FileServerList to
//   switch MetaData servers.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//
// ****************************************************************************

void
QvisFileOpenWindow::hostChanged(int)
{
    GetCurrentValues(false);
}

// ****************************************************************************
// Method: QvisFileOpenWindow::pathChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the contents of
//   the path text field change.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//
// ****************************************************************************

void
QvisFileOpenWindow::pathChanged(int)
{
    GetCurrentValues(true);
}

// ****************************************************************************
// Method: QvisFileOpenWindow::changeDirectory
//
// Purpose: 
//   This is a Qt slot function that is called when an item in the
//   directory menu is double clicked.
//
// Arguments:
//   item : The item in the directory list that was double-clicked.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//
// ****************************************************************************

void
QvisFileOpenWindow::changeDirectory(QListBoxItem *item) 
{
    std::string newPath(fileServer->GetPath());
    std::string separator(fileServer->GetSeparatorString());

    // Get the strings for current directory and up directory.
    QString curDirString, upDirString;
    GetDirectoryStrings(curDirString, upDirString);

    // don't do anything
    if(item->text() == curDirString)
        return;

    TRY
    {
        // Put a message on the status line.
        QString temp("Changing directory");
        Status(temp);

        if(item->text() == upDirString)
        {
            std::string curPath(fileServer->GetPath());
            if(curPath != separator)
            {
                if(separator == "\\" && curPath.size() == 2)
                {
                    if(curPath[1] == ':')
                        newPath = "My Computer";
                }
                else 
                {
                    int separatorPos = newPath.rfind(separator);
                    // If the last character is a slash, remove it.
                    if(separatorPos == newPath.size() - 1)
                        newPath = newPath.substr(separatorPos);

                    // Remove the last named directory in the path
                    newPath = newPath.substr(0, newPath.rfind(separator));

                    // See if there is anything left
                    if(newPath.size() == 0)
                        newPath = separator;
                }

                // Try and switch to the new path.
                fileServer->SetPath(newPath);
                fileServer->Notify();
            }
        }
        else
        {
            // Go into a sub-directory. Make sure that the last character
            // in the newPath is not the separator. If it is not the separator
            // then add a separator.
            if(newPath[newPath.length() - 1] != separator[0])
                newPath += separator;
            newPath += std::string(item->text().latin1());

            // Try and switch to the new path.
            fileServer->SetPath(newPath);
            fileServer->Notify();
        }
    }
    CATCH(ChangeDirectoryException)
    {
        // Create a message and tell the user.
        QString msgStr;
        msgStr.sprintf("The MetaData server running on %s "
            "could not change the current directory to %s.",
            fileServer->GetHost().c_str(), newPath.c_str());
        Error(msgStr);
    }
    CATCH(GetFileListException)
    {
        UpdateDirectoryList();
        UpdateFileList();

        // Create a message and tell the user.
        QString msgStr;
        msgStr.sprintf("The MetaData server running on %s could not "
             "get the file list for the current directory.",
             fileServer->GetHost().c_str());
        Error(msgStr);
    }
    ENDTRY

    // Clear the status bar.
    ClearStatus();
}

// ****************************************************************************
// Method: QvisFileOpenWindow::selectFileDblClick
//
// Purpose: 
//   This is a Qt slot function that is called when a filename is
//   double-clicked. The filename that was double clicked is added
//   to the intermediate selected file list.
//
// Arguments:
//   item : A pointer to the widget that was double clicked.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//
// ****************************************************************************

void
QvisFileOpenWindow::selectFileDblClick(QListBoxItem *item) 
{
    // Make this do an Open action instead of a Select action
    okClicked();
}

// ****************************************************************************
// Method: QvisFileOpenWindow::selectFileChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the selection changes
//   in the file list.  The sensitivity of various widgets is adjusted to
//   match.
//
// Arguments:
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//   
// ****************************************************************************
void
QvisFileOpenWindow::selectFileChanged(void)
{
    // Count the number of selected files
    int count = 0;
    for(int i = 0; i < fileList->count(); ++i)
        if(fileList->isSelected(i))
            count++;

    if (count >= 1)
        okButton->setEnabled(true);
    else
        okButton->setEnabled(false);
}

// ****************************************************************************
// Method: QvisFileOpenWindow::selectFile
//
// Purpose: 
//   This is a Qt slot function that adds all of the highlighted files
//   in the fileList and adds them to the intermediate file list.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//
// ****************************************************************************

void
QvisFileOpenWindow::selectFile()
{
    int i, addCount = 0;

    // Add all the selected files to the intermediate file list.
    for(i = 0; i < fileList->count(); ++i)
    {
        if(!fileList->isSelected(i))
            continue;

        // Add the file to the list if it's not in it.
        QFileSelectionListBoxItem *item = (QFileSelectionListBoxItem *)fileList->item(i);
        if(AddFile(item->GetFilename()))
            ++addCount;
    }

    // Unselect all the files.
    for(i = 0; i < fileList->count(); ++i)
        fileList->setSelected(i, false);
}

// ****************************************************************************
// Method: QvisFileOpenWindow::selectFileReturnPressed
//
// Purpose: 
//   This method lets us select multiple files from the file list into the
//   selected file list by hitting the Enter key.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileOpenWindow::selectFileReturnPressed(QListBoxItem *)
{
    selectFile();
}

// ****************************************************************************
// Method: QvisFileOpenWindow::refreshFiles
//
// Purpose: 
//   This is a Qt slot function that refreshes the list of files in the
//   current directory.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//
// ****************************************************************************

void
QvisFileOpenWindow::refreshFiles()
{
    std::string oldPath(fileServer->GetPath());

    // We set an invalid path in the file server object to circumvent the
    // equality check when changing paths. Then we set the path back to its
    // previous value. This will cause the file server object to reread the
    // directory from the metadata server.
    fileServer->SetPath("");
    fileServer->SetPath(oldPath);
    ChangePath(true);
}

// ****************************************************************************
// Method: QvisFileOpenWindow::show
//
// Purpose: 
//   This is a Qt slot function that shows the window. It also refreshes
//   the file list each time the window is shown.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//
// ****************************************************************************

void
QvisFileOpenWindow::show()
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

    UpdateFileFormatComboBox();
}

// ****************************************************************************
// Method: QvisFileOpenWindow::showMinimized
//
// Purpose: 
//   Iconifies the window.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileOpenWindow::showMinimized()
{
    QvisDelayedWindowSimpleObserver::showMinimized();
    if(recentPathsRemovalWindow && recentPathsRemovalWindow->isVisible())
        recentPathsRemovalWindow->showMinimized();
}

// ****************************************************************************
// Method: QvisFileOpenWindow::showNormal
//
// Purpose: 
//   De-iconifies the window.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileOpenWindow::showNormal()
{
    QvisDelayedWindowSimpleObserver::showNormal();
    if(recentPathsRemovalWindow && recentPathsRemovalWindow->isVisible())
    {
        recentPathsRemovalWindow->showNormal();
        recentPathsRemovalWindow->raise();
    }
}

// ****************************************************************************
// Method: QvisFileOpenWindow::currentDir
//
// Purpose: 
//   This is a Qt slot function that sets the "UseCurrentDirectory" flag
//   in the file server.
//
// Arguments:
//   val : The new "UseCurrentDirectory" flag.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileOpenWindow::currentDir(bool val)
{
    fileServer->SetUseCurrentDirectory(val);
}

// ****************************************************************************
// Method: QvisFileOpenWindow::fileGroupingChanged
//
// Purpose: 
//   This is a Qt slot function that sets the AutomaticFileGrouping flag in
//   the file server and updates the file list.
//
// Arguments:
//   val : The new AutomaticFileGrouping flag.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//
// ****************************************************************************

void
QvisFileOpenWindow::fileGroupingChanged(int val)
{
    if(val == 0)
    {
        fileServer->SetAutomaticFileGrouping(false);
        fileServer->SetSmartFileGrouping(false);
    }
    else if(val == 1)
    {
        fileServer->SetAutomaticFileGrouping(true);
        fileServer->SetSmartFileGrouping(false);
    }
    else if(val == 2)
    {
        fileServer->SetAutomaticFileGrouping(true);
        fileServer->SetSmartFileGrouping(true);
    }

    fileServer->Notify();
}

// ****************************************************************************
// Method: QvisFileOpenWindow::closeEvent
//
// Purpose: 
//   Closes the window and also makes sure that the recentPathRemoval window
//   gets closed if it is open.
//
// Arguments:
//   e : The close event to handle.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Note: Taken largely from QvisFileSelectWindow
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileOpenWindow::closeEvent(QCloseEvent *e)
{
    if(recentPathsRemovalWindow && recentPathsRemovalWindow->isVisible())
        recentPathsRemovalWindow->hide();
    
    QvisDelayedWindowSimpleObserver::closeEvent(e);
}


// ****************************************************************************
//  Method:  QvisFileOpenWindow::UpdateFileFormatComboBox
//
//  Purpose:
//    Update the combo box listing the available types of plugins.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August  9, 2006
//
// ****************************************************************************
void
QvisFileOpenWindow::UpdateFileFormatComboBox()
{
    QString oldtype = fileFormatComboBox->currentText();

    fileFormatComboBox->clear();
    int nTypes = dbplugins->GetTypes().size();
    fileFormatComboBox->insertItem("Guess from file name/extension");
    for (int i = 0 ; i < nTypes ; i++)
    {
        fileFormatComboBox->insertItem(dbplugins->GetTypes()[i].c_str());
    }

    if (!oldtype.isNull())
    {
        for (int i=0; i<fileFormatComboBox->count(); i++)
        {
            if (oldtype == fileFormatComboBox->text(i))
            {
                fileFormatComboBox->setCurrentItem(i);
                break;
            }
        }
    }
}
