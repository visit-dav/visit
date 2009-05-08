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
#include <QEvent>
#include <QComboBox>
#include <QItemDelegate>
#include <QLineEdit>
#include <QLayout>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QListWidget>
#include <QSplitter>
#include <QWidget>

#include <QvisFileWindowBase.h>
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

#include <visit-config.h>

#include <algorithm>

using std::vector;
using std::string;

// Include icons
#include <icons/computer.xpm>
#include <icons/database.xpm>
#include <icons/folder.xpm>

// ****************************************************************************
// Class: VirtualDBDelegate
//
// Purpose:
//   Delegate for QListWidget that draws the icon in the right place for
//   virtual databases.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 15 11:53:30 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class VirtualDBDelegate : public QItemDelegate
{
public:
    VirtualDBDelegate(QObject *parent = 0) : QItemDelegate(parent)
    {
    }

    virtual ~VirtualDBDelegate()
    {
    }

protected:
    virtual void drawDecoration(
        QPainter *painter, 
        const QStyleOptionViewItem &option, 
        const QRect &rect,
        const QPixmap &pixmap) const
    {
        if(!pixmap.isNull())
        {
            QPoint p0(rect.x() + rect.width() / 2,
                      option.rect.y() + rect.height() + 2);
            QPoint p1(rect.x() + rect.width() / 2,
                      option.rect.y() + option.rect.height() - 4);
            QPoint p2(rect.x() + rect.width(),
                      option.rect.y() + option.rect.height() - 4);
            painter->drawLine(p0, p1);
            painter->drawLine(p1, p2);

            painter->drawPixmap(QPoint(rect.x(),option.rect.y()), pixmap);
        }
    }
};

// ****************************************************************************
// Method: QvisFileWindowBase::QvisFileWindowBase
//
// Purpose: 
//   Constructor
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 15 12:05:08 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QvisFileWindowBase::QvisFileWindowBase(const QString &winCaption) :
    QvisDelayedWindowSimpleObserver(winCaption), intermediateFileList(),
    currentVirtualDatabaseDefinitions(), invalidHosts()
{
    fs = 0;
    profiles = 0;

    //
    // Create the pixmaps that we use in this window.
    //
    computerPixmap = new QPixmap(computer_xpm);
    folderPixmap = new QPixmap(folder_xpm);
    databasePixmap = new QPixmap(database_xpm);

    recentPathsRemovalWindow = 0;

    // Set the progress callback that we want to use while we
    // connect to the mdserver.
    fileServer->SetProgressCallback(ProgressCallback, (void *)this);
}

// ****************************************************************************
// Method: QvisFileWindowBase::~QvisFileWindowBase
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 15 12:05:22 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QvisFileWindowBase::~QvisFileWindowBase()
{
    if(fs)
        fs->Detach(this);
    if(profiles)
        profiles->Detach(this);

    delete computerPixmap;
    delete folderPixmap;
    delete databasePixmap;
}

// ****************************************************************************
// Method: QvisFileWindowBase::SubjectRemoved
//
// Purpose: 
//   Called when observed subjects are deleted so they can be removed from
//   observation.
//
// Arguments:
//   TheRemovedSubject : The subject to be removed.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 15 12:05:36 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileWindowBase::SubjectRemoved(Subject *TheRemovedSubject)
{
    if(TheRemovedSubject == fs)
        fs = 0;
    else if(TheRemovedSubject == profiles)
        profiles = 0;
}

// ****************************************************************************
// Method: QvisFileWindowBase::ConnectSubjects
//
// Purpose: 
//   Connect subjects for observation.
//
// Arguments:
//   hpl : The host profile state object that we want to observe.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 15 12:06:25 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileWindowBase::ConnectSubjects(HostProfileList *hpl)
{
    fs = fileServer;
    fileServer->Attach(this);

    profiles = hpl;
    profiles->Attach(this);
}

// ****************************************************************************
// Method: QvisFileWindowBase::GetDirectoryStrings
//
// Purpose: 
//   Creates the current directory and up directory strings based on the
//   current host.
//
// Returns:    The upDir and curDir strings.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 28 09:37:59 PDT 2002
//
// Modifications:
//   Brad Whitlock, Mon Mar 31 09:28:13 PDT 2003
//   I removed the separator from the string.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
QvisFileWindowBase::GetDirectoryStrings(QString &curDir, QString &upDir)
{
    curDir = QString(". ") + tr("(current directory)");
    upDir = QString(".. ") + tr("(go up 1 directory level)");
}

// ****************************************************************************
// Method: QvisFileWindowBase::ProgressCallback
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
// Programmer: Brad Whitlock
// Creation:   Mon Sep 30 07:59:57 PDT 2002
//
// Modifications:
//   Brad Whitlock, Mon Sep 29 11:47:06 PDT 2003
//   I changed the code so that the window's enabled state gets set regardless
//   of whether the window is visible. This fixes a bug where minimizing the
//   window in the middle of connecting to a remote mdserver causes the
//   window to stay disabled, thus hanging the gui.
//
//   Brad Whitlock, Fri Oct 17 09:15:26 PDT 2003
//   I disabled the call to hasPendingEvents on MacOS X since it tended to
//   hang the gui.
//
//   Brad Whitlock, Wed Aug 4 15:59:00 PST 2004
//   This method is called any time the file server needs to interact with
//   an mdserver such as when we're restoring sessions. Thus, the file
//   selection window is not necessarily visible so I removed code that
//   prevented VisIt from only handling events when the file selection window
//   was visible.
//
//   Cyrus Harrison, Thu Jun 26 09:54:36 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

bool
QvisFileWindowBase::ProgressCallback(void *data, int stage)
{
    QWidget *This = (QWidget *)data;

    if(stage == 0)
    {
        // Disable the widgets in the file selection window.
        This->setEnabled(false);
    }
    else if(stage == 1)
    {
        qApp->processEvents();
    }
    else
    {
        // Enable the widgets in the file selection window.
        This->setEnabled(true);
    }

    return true;
}

// ****************************************************************************
// Method: QvisFileWindowBase::UpdateComboBox
//
// Purpose: 
//   Populates the specified combo box with the appropriate entries.
//
// Arguments:
//   cb : The combo box to update.
//   s  : The list of entries.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 25 15:48:01 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Apr 5 16:43:05 PST 2005
//   Added code to make the combo box's list box taller.
//
//   Cyrus Harrison, Thu Jun 26 09:54:36 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisFileWindowBase::UpdateComboBox(QComboBox *cb, const stringVector &s,
    const QString &activeItem)
{
    cb->blockSignals(true);
    cb->clear();

    // Populate the combo box.
    int i;
    for(i = 0; i < s.size(); ++i)
        cb->addItem(s[i].c_str());

    // Set the current item.
    int index = cb->findText(activeItem);
    if(index == -1)
    {
        cb->addItem(activeItem);
        index = cb->findText(activeItem);
    }

    cb->setCurrentIndex(index);
    cb->setEditText(activeItem);

    // Note: in Qt4 combo boxes expand out to max possible size, we
    // don't need to set a min size as we did before.
    // (we may want to set a max size some time in the future, but
    //  I have not looked into that)

    cb->blockSignals(false);
}

// ****************************************************************************
// Method: QvisFileWindowBase::CreateHostPathFilterControls
//
// Purpose: 
//   Creates the top portion of the file selection window (host,path,filter) 
//   and some other controls.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 15 12:07:15 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileWindowBase::CreateHostPathFilterControls()
{   
    //
    // Create the path, filter
    //
    QGridLayout *pathLayout = new QGridLayout();
    topLayout->addLayout(pathLayout);
    pathLayout->setSpacing(10);

    // Create the host
    hostComboBox = new QComboBox(central);
    hostComboBox->setEditable(true);
    hostComboBox->setDuplicatesEnabled(false);
    hostComboBox->setInsertPolicy(QComboBox::InsertAtTop);
    connect(hostComboBox, SIGNAL(activated(int)),
            this, SLOT(hostChanged(int)));
    QHBoxLayout *hostLayout = new QHBoxLayout();
    hostLayout->setSpacing(5);
    hostLayout->setMargin(0);
    QLabel *hostLabel = new QLabel(tr("Host"), central);
    QLabel *hostImageLabel = new QLabel(central);
    hostImageLabel->setPixmap(*computerPixmap);
    hostImageLabel->setBuddy(hostComboBox);
    hostLayout->addWidget(hostImageLabel);
    hostLayout->addWidget(hostLabel);
    pathLayout->addLayout(hostLayout, 0, 0);
    pathLayout->addWidget(hostComboBox, 0, 1);

    // Create the path
    pathComboBox = new QComboBox(central);
    pathComboBox->setEditable(true);
    pathComboBox->setDuplicatesEnabled(false);
    pathComboBox->setInsertPolicy(QComboBox::InsertAtTop);
    connect(pathComboBox, SIGNAL(activated(int)),
            this, SLOT(pathChanged(int)));
    QHBoxLayout *pathLayout2 = new QHBoxLayout;
    pathLayout2->setSpacing(5);
    pathLayout2->setMargin(0);
    
    QLabel *pathLabel = new QLabel(tr("Path"), central);
    QLabel *pathImageLabel = new QLabel(central);
    pathImageLabel->setPixmap(*folderPixmap);
    pathImageLabel->setBuddy(pathComboBox);
    pathLayout2->addWidget(pathImageLabel);
    pathLayout2->addWidget(pathLabel);
    pathLayout->addLayout(pathLayout2, 1, 0);
    pathLayout->addWidget(pathComboBox, 1, 1);

    // Create the filter
    filterLineEdit = new QLineEdit(central);
    connect(filterLineEdit, SIGNAL(returnPressed()), this, SLOT(filterChanged()));
    QLabel *filterLabel = new QLabel(tr("Filter"), central);
    pathLayout->addWidget(filterLabel, 2, 0, Qt::AlignRight);
    pathLayout->addWidget(filterLineEdit, 2, 1);

    // Create the current dir toggle.
    QHBoxLayout *toggleLayout = new QHBoxLayout();
    topLayout->addLayout(toggleLayout);
    toggleLayout->setSpacing(10);
    currentDirToggle = new QCheckBox(tr("Use \"current working directory\" by default"), central);
    connect(currentDirToggle, SIGNAL(toggled(bool)),
            this, SLOT(currentDir(bool)));
    toggleLayout->addWidget(currentDirToggle);

    // Create the file grouping checkbox.
    fileGroupingComboBox = new QComboBox(central);
    fileGroupingComboBox->addItem(tr("Off"));
    fileGroupingComboBox->addItem(tr("On"));
    fileGroupingComboBox->addItem(tr("Smart"));
    fileGroupingComboBox->setEditable(false);
    connect(fileGroupingComboBox, SIGNAL(activated(int)),
            this, SLOT(fileGroupingChanged(int)));
    toggleLayout->addStretch(5);
    toggleLayout->addWidget(new QLabel(tr("File grouping"), central), 0, Qt::AlignRight);
    toggleLayout->addWidget(fileGroupingComboBox, 0, Qt::AlignLeft);
    toggleLayout->addStretch(5);

    // Create a window we can activate to remove recent paths.
    recentPathsRemovalWindow = new QvisRecentPathRemovalWindow(fileServer,
        tr("Remove recent paths"));
    recentPathRemovalButton = new QPushButton(tr("Remove paths . . ."), central);
    connect(recentPathRemovalButton, SIGNAL(clicked()),
            recentPathsRemovalWindow, SLOT(show()));
    toggleLayout->addWidget(recentPathRemovalButton);
}

// ****************************************************************************
// Method: QvisFileWindowBase::CreateListWidget
//
// Purpose: 
//   Creates a new list widget that uses a special delegate to aid in
//   rendering.
//
// Arguments:
//   parent : The new widget's parent.
//
// Returns:    A new QListWidget.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 15 11:58:07 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QListWidget *
QvisFileWindowBase::CreateFileListWidget(QWidget *parent) const
{
    QListWidget *w = new QListWidget(parent);
    w->setItemDelegate(new VirtualDBDelegate(w));
    return w;
}

// ****************************************************************************
// method: QvisFileWindowBase::UpdateWindowFromFiles
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
//   Brad Whitlock, Fri Dec 14 17:18:39 PST 2007
//   Made it use ids.
//
//   Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisFileWindowBase::UpdateWindowFromFiles(bool doAll)
{
    // Set the working directory toggle.
    currentDirToggle->blockSignals(true);
    currentDirToggle->setChecked(fileServer->GetUseCurrentDirectory());
    currentDirToggle->blockSignals(false);

    // Set the file grouping combo box.
    if(fileServer->IsSelected(FileServerList::ID_automaticFileGroupingFlag) ||
       fileServer->IsSelected(FileServerList::ID_smartFileGroupingFlag) ||
       doAll)
    {
        int index = 0;
        if(fileServer->GetAutomaticFileGrouping())
        {
            ++index;
            if(fileServer->GetSmartFileGrouping())
                ++index;
        }
        fileGroupingComboBox->blockSignals(true);
        fileGroupingComboBox->setCurrentIndex(index);
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
// Method: QvisFileWindowBase::UpdateHostComboBox
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
//    Hank Childs, Thu May  7 19:59:13 PDT 2009
//    Add support for nicknames.
//
// ****************************************************************************

void
QvisFileWindowBase::UpdateHostComboBox()
{
    // Get the starting list of hosts from the file server.
    stringVector hosts(fileServer->GetRecentHosts());

    //
    // Add all of the hosts from the host profiles that are not already
    // in the hosts list.
    //
    for(int i = 0; i < profiles->GetNumProfiles(); ++i)
    {
        // Create a constant reference to the i'th profile.
        const HostProfile &p = profiles->operator[](i);

        stringVector hostNames;
        if (p.GetHostNickname() != "")
            hostNames.push_back(p.GetHostNickname());
        else
            hostNames = p.SplitHostPattern(p.GetHost());

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
// Method: QvisFileWindowBase::UpdateDirectoryList
//
// Purpose: 
//   This method adds all of the directories in the FileServer's
//   file list to the window's directory listbox.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 23 10:59:39 PDT 2000
//
// Modifications:
//   Brad Whitlock, Thu May 9 17:08:17 PST 2002
//   Made it use the base class's fileServer pointer.
//
//   Brad Whitlock, Wed Aug 28 09:33:45 PDT 2002
//   I made it create the curDir and upDir strings based on the current host.
//
//   Brad Whitlock, Fri Mar 28 15:57:26 PST 2003
//   I made it use QFileSelectionListBoxItem.
//
//   Brad Whitlock, Tue Oct 21 13:28:25 PST 2003
//   I made directories get their associated permission flag so they appear
//   grayed out if they don't have read permission.
//
//   Brad Whitlock, Thu Jul 10 15:59:03 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisFileWindowBase::UpdateDirectoryList()
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
        QListWidgetItem *item = new QListWidgetItem(directoryList);
        if(pos->name == std::string("."))
        {
            QualifiedFilename dirName(curDirString.toStdString());
            dirName.SetAccess(pos->CanAccess());

            item->setText(curDirString);
            item->setData(Qt::UserRole, EncodeQualifiedFilename(dirName));
        }
        else if(pos->name == std::string(".."))
        {
            QualifiedFilename dirName(upDirString.toStdString());
            dirName.SetAccess(pos->CanAccess());

            item->setText(upDirString);
            item->setData(Qt::UserRole, EncodeQualifiedFilename(dirName));
        }
        else
        {
            QualifiedFilename dirName(pos->name);
            dirName.SetAccess(pos->CanAccess());

            item->setText(pos->name.c_str());
            item->setIcon(QIcon(*folderPixmap));
            item->setData(Qt::UserRole, EncodeQualifiedFilename(dirName));
        }
    }
}

// ****************************************************************************
// Method: QvisFileWindowBase::AddFileItem
//
// Purpose: 
//   Adds a file item to a list widget. The file item is initialized from a
//   qualified filename.
//
// Arguments:
//   parent      : The list widget that will contain the new item.
//   displayName : The name to show for the item.
//   fileInfo    : Information about the file that will ride along with the item.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 10 15:59:10 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileWindowBase::AddFileItem(QListWidget *parent, const QString &displayName,
    const QualifiedFilename &fileInfo)
{
    QListWidgetItem *item = new QListWidgetItem(parent);

    // Set an icon if we're looking at a virtual database.
    if(fileInfo.IsVirtual())
        item->setIcon(QIcon(*databasePixmap));

    // Set the item's text.
    QString itemText(displayName);
    if(fileInfo.IsVirtual())
    {
        // Append some filenames
        const int MAX_DISPLAYED_NAMES = 5;
        int i;
        stringVector names(fileServer->GetVirtualFileDefinition(fileInfo));
        if(names.size() > (MAX_DISPLAYED_NAMES*2))
        {
            QString nFilesString;
            nFilesString.sprintf("(%d total files)", names.size());
            itemText += QString("\n") + nFilesString;

            for(i = 0; i < MAX_DISPLAYED_NAMES; ++i)
                itemText += QString("\n") + names[i].c_str();

            itemText += QString("\n...");

            for(i = names.size()-MAX_DISPLAYED_NAMES; i < names.size(); ++i)
                itemText += QString("\n") + names[i].c_str();
        }
        else
        {
            for(i = 0; i < names.size(); ++i)
                itemText += QString("\n") + names[i].c_str();
        }
    }
    item->setText(itemText);

    // Gray out the text if we can't access the file.
    if(!fileInfo.CanAccess())
        item->setForeground(QBrush(QColor(Qt::gray)));

    // Stash the complete filename into the item as user data.
    item->setData(Qt::UserRole, EncodeQualifiedFilename(fileInfo));
}

// ****************************************************************************
// Method: QvisFileWindowBase::UpdateFileList
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
//   Qt 4.
//
// ****************************************************************************

void
QvisFileWindowBase::UpdateFileList()
{
    fileList->clear();

    // Get the filtered file list from the file server. Then iterate through
    // the file list and add the files.
    QualifiedFilenameVector filteredFiles(fileServer->GetFilteredFileList());
    for(QualifiedFilenameVector::iterator pos = filteredFiles.begin();
        pos != filteredFiles.end(); ++pos)
    {
        AddFileItem(fileList, pos->filename.c_str(), *pos);
    }
}

// ****************************************************************************
// Method: QvisFileWindowBase::RemoveComboBoxItem
//
// Purpose: 
//   Removes an item from a combo box and makes another item active.
//
// Arguments:
//   cb        : The combo box that we're operating on.
//   remove    : The item to remove.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 13 14:46:58 PST 2002
//
// Modifications:
//   Cyrus Harrison, Thu Jun 26 09:54:36 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisFileWindowBase::RemoveComboBoxItem(QComboBox *cb,
    const QString &remove)
{
    int index = cb->findText(remove);
    if(index != -1)
    {
         cb->blockSignals(true);
         cb->removeItem(index);
         cb->setEditText(remove);
         cb->blockSignals(false);
    }
}

// ****************************************************************************
// Method: QvisFileWindowBase::ActivateComboBoxItem
//
// Purpose: 
//   Makes an entry in the combo box active.
//
// Arguments:
//   cb        : The combobox to use.
//   newActive : The name of the item to activate.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 11 17:44:00 PST 2002
//
// Modifications:
//   Cyrus Harrison, Thu Jun 26 09:54:36 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisFileWindowBase::ActivateComboBoxItem(QComboBox *cb,
    const QString &newActive)
{
    int index = cb->findText(newActive);
    if(index != -1)
    {
         cb->blockSignals(true);
         cb->setCurrentIndex(index);
         cb->setEditText(newActive);
         cb->blockSignals(false);
    }
}

// ****************************************************************************
// Method: QvisFileWindowBase::HighlightComboBox
//
// Purpose: 
//   Highlights the combo box by selecting its text and giving it focus.
//
// Arguments:
//   cb : The combobox to highlight.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 11 17:45:50 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileWindowBase::HighlightComboBox(QComboBox *cb)
{
    // Select the line edit's text.
    QLineEdit *le = cb->lineEdit();
    le->setSelection(0, le->text().length());
    le->setFocus();
}

// ****************************************************************************
// Method: QvisFileWindowBase::AddFile
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
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 29 13:53:06 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu May 9 17:08:17 PST 2002
//   Made it use the base class's fileServer pointer.
//
//   Brad Whitlock, Fri Mar 28 14:58:56 PST 2003
//   I made the argument be a qualified filename and I made it return true if
//   the file is a virtual file.
//
// ****************************************************************************

bool
QvisFileWindowBase::AddFile(const QualifiedFilename &newFile)
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
// Method: QvisFileWindowBase::GetCurrentValues
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
// Programmer: Brad Whitlock
// Creation:   Thu May 3 13:05:10 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Feb 13 11:07:16 PDT 2002
//   Changed to work on combo boxes.
//
//   Brad Whitlock, Thu May 9 17:08:17 PST 2002
//   Made it use the base class's fileServer pointer.
//
//   Brad Whitlock, Mon Aug 26 15:23:20 PST 2002
//   I removed the code to filter the path since the mdserver now does it.
//
// ****************************************************************************

void
QvisFileWindowBase::GetCurrentValues(bool allowPathChange)
{
    // Get the path string from before the possible host change.
    std::string beforePath(pathComboBox->currentText().trimmed().toStdString());

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
// Method: QvisFileWindowBase::ChangeHosts
//
// Purpose: 
//   This method is called to change hosts.
//
// Returns:    true is there was an error.
//
// Programmer: Brad Whitlock
// Creation:   Thu May 3 13:06:51 PST 2001
//
// Modifications:
//    Brad Whitlock, Mon Oct 22 18:25:42 PST 2001
//    Changed the exception handling keywords to macros.
//
//    Brad Whitlock, Wed Feb 13 10:58:12 PDT 2002
//    Changed so it works on a combo box. Added code to catch objects of type
//    CouldNotConnectException. Added code to set the application cursor.
//
//    Jeremy Meredith, Thu Feb 14 15:24:21 PST 2002
//    Always replace the hostname with the one in the file server.  It will
//    be replaced with the fully qualified hostname under normal circumstances.
//
//    Brad Whitlock, Mon Feb 25 17:19:43 PST 2002
//    I removed a little code to remove the hostname from the combo box on
//    a successful change of host.
//
//    Brad Whitlock, Mon Mar 11 11:57:35 PDT 2002
//    Changed the cursor handling so it is done by methods of the base class.
//
//    Brad Whitlock, Thu May 9 17:08:17 PST 2002
//    Made it use the base class's fileServer pointer.
//
//    Brad Whitlock, Wed Sep 11 17:21:42 PST 2002
//    I made it highlight the combo box when the input is bad.
//
//    Brad Whitlock, Thu Jul 29 15:02:10 PST 2004
//    I changed how the routine handles bad hosts.
//
//    Brad Whitlock, Wed Nov 3 17:19:06 PST 2004
//    I made this window be the active window after changing hosts and I
//    made the window raise itself so it comes to the front after being
//    partially covered by the viewer window on MacOS X.
//
//    Brad Whitlock, Wed Feb 2 13:44:29 PST 2005
//    I removed a line of code that added the host to the list of invalid
//    hosts when we can't connect to it since most of the time the host to
//    which we could not connect is in the host profiles and removing it
//    confuses people.
//
//    Brad Whitlock, Thu Oct 27 15:43:13 PST 2005
//    Catching CancelledConnectException now that the file server throws it
//    instead of catching it internally.
//
//    Cyrus Harrison, Thu Jun 26 09:54:36 PDT 2008
//    Initial Qt4 Port.
//
//    Hank Childs, Thu May  7 20:08:04 PDT 2009
//    Added support for nicknames.
//
// ****************************************************************************

bool
QvisFileWindowBase::ChangeHosts()
{
    bool errFlag = false;

    // If the line edit is not empty, change the host name.
    if(!hostComboBox->currentText().isEmpty())
    {
        // Take the string from the text field and strip whitespace.
        std::string host(hostComboBox->currentText().trimmed().toStdString());
        std::string currentHost(fileServer->GetHost());
        for(int i = 0; i < profiles->GetNumProfiles(); ++i)
        {
            // Create a constant reference to the i'th profile.
            const HostProfile &p = profiles->operator[](i);
            if (p.GetHostNickname() == host)
                host = p.GetHost();
        }

        if(host != fileServer->GetHost())
        {
            // Put a message on the status line.
            QString temp = tr("Opening server on %1").arg(host.c_str());
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
                    QString msgStr = tr("\"%1\" is not a valid host.").
                                     arg(host.c_str());
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
            topLevelWidget()->activateWindow();
            topLevelWidget()->raise();
        }
    }

    return errFlag;
}

// ****************************************************************************
// Method: QvisFileWindowBase::ChangePath
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
// Programmer: Brad Whitlock
// Creation:   Thu May 3 13:06:51 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue Aug 21 14:58:18 PST 2001
//   Added code to filter out multiple slash characters in the path name.
//
//   Brad Whitlock, Wed Feb 13 11:00:47 PDT 2002
//   Changed to use combo boxes. Moved the path filtering code into the
//   FileServerList class.
//
//   Brad Whitlock, Thu May 9 17:08:17 PST 2002
//   Made it use the base class's fileServer pointer.
//
//   Brad Whitlock, Mon Aug 26 15:19:38 PST 2002
//   I removed the filtering code since it was moved to the mdserver.
//
//   Brad Whitlock, Wed Sep 11 17:21:42 PST 2002
//   I made it highlight the combo box when the input is bad.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

bool
QvisFileWindowBase::ChangePath(bool allowPathChange)
{
    bool errFlag = false;

    // If the line edit is not empty, change the host name.
    if(!pathComboBox->currentText().isEmpty())
    {
        // Take the string from the text field and strip whitespace.
        std::string path(pathComboBox->currentText().trimmed().toStdString());
        bool pathNeedsSet = true;

        // If the paths are different or we are allowing a path change,
        // change the path to the new value.
        if(path != fileServer->GetPath() || allowPathChange)
        {
            // Put a message on the status line.
            Status(tr("Changing directory."));

            // Set the path in the file server.
            TRY
            {
                fileServer->SetPath(path);
                fileServer->Notify();
            }
            CATCH(ChangeDirectoryException)
            {
                // Create a message and tell the user.
                QString msgStr = tr("The MetaData server running on %1 "
                                    "could not change the current directory to %2.").
                                 arg(fileServer->GetHost().c_str()).
                                 arg(path.c_str());
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
                QString msgStr = tr("The MetaData server running on %1 "
                                    "could not get the file list for the "
                                    "current directory").
                                 arg(fileServer->GetHost().c_str());
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
// Method: QvisFileSelectionWindow::ChangeFilter
//
// Purpose: 
//   This method is called to change filters.
//
// Returns:    true is there was an error.
//
// Programmer: Brad Whitlock
// Creation:   Thu May 3 13:06:51 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu May 9 17:08:17 PST 2002
//   Made it use the base class's fileServer pointer.
//
//   Brad Whitlock, Mon Sep 29 16:11:15 PST 2003
//   I changed the routine so it uses "*" for the filter if the user tries
//   to enter an invalid filter.
//
//   Brad Whitlock, Thu Apr 13 14:13:54 PST 2006
//   Added exception handling to catch exceptions that are set because of
//   of a previous inability to set the directory.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

bool
QvisFileWindowBase::ChangeFilter()
{
    bool errFlag = false;
    bool forcedChange = false;
    std::string filter("*");

    // If the line edit is not empty, change the host name.
    if(!filterLineEdit->text().isEmpty())
    {
        // Take the string from the text field and simplify whitespace.
        filter = filterLineEdit->text().trimmed().toStdString();

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
                Error(tr("The MetaData server running could not get the file "
                      "list for the current directory, which is required "
                      "before setting the file filter. Try entering a "
                      "valid path before changing the file filter."));
                exceptionErr = true;
            }
            CATCH(VisItException)
            {
                Error(tr("An error occured when trying to set the file filter."));
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
        Error(tr("An invalid filter was entered."));
        filterLineEdit->setText(QString(fileServer->GetFilter().c_str()));
    }

    return errFlag || exceptionErr;
}

// ****************************************************************************
// Method: QvisFileWindowBase::GetVirtualDatabaseDefinitions
//
// Purpose: 
//   Populates a map with virtual database definitions.
//
// Arguments:
//   defs : The map into which we'll store the virtual database definitions.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 27 11:52:16 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileWindowBase::GetVirtualDatabaseDefinitions(
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

// ****************************************************************************
// Method: QvisFileWindowBase::CheckForNewStates
//
// Purpose: 
//   Checks open virtual databases for new states.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 15 12:07:55 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileWindowBase::CheckForNewStates()
{
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
                    debug1 << "QvisFileWindowBase::okClicked: The virtual "
                           << "database definition for "
                           << fileName.c_str()
                           << " did not change. No reopen is required."
                           << endl;
                }
                else
                {
                    debug1 << "QvisFileWindowBase::okClicked: The virtual "
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
}

//
// Qt slot functions
//

// ****************************************************************************
// Method: QvisFileWindowBase::setEnabled
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
QvisFileWindowBase::setEnabled(bool val)
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
// Method: QvisFileWindowBase::filterChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the contents of
//   filter text field change.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 22 17:58:40 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu May 3 10:45:44 PDT 2001
//   Moved the guts into ChangeFilter which is called by GetCurrentValues.
//
// ****************************************************************************

void
QvisFileWindowBase::filterChanged()
{
    GetCurrentValues(false);
}

// ****************************************************************************
// Method: QvisFileWindowBase::hostChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the contents of
//   the host text field change. This tells the FileServerList to
//   switch MetaData servers.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 22 17:56:38 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu May 3 10:43:18 PDT 2001
//   Moved the guts into the ChangeHost method which is called by
//   GetCurrentValues.
//
// ****************************************************************************

void
QvisFileWindowBase::hostChanged(int)
{
    GetCurrentValues(false);
}

// ****************************************************************************
// Method: QvisFileWindowBase::pathChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the contents of
//   the path text field change.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 22 17:56:38 PST 2000
//
// Modifications:
//   Brad Whitlock, Wed Aug 30 15:56:45 PST 2000
//   Caught some exceptions that are now propagated from the 
//   FileServerList's Notify method. When I catch them, I display
//   an error message.
//
//   Brad Whitlock, Mon Oct 23 13:55:52 PST 2000
//   I removed some code that reset the path to the last good path if
//   a bad path was supplied. This gives the user the chance to
//   correct the mistake.
//
//   Brad Whitlock, Thu May 3 10:43:18 PDT 2001
//   Moved the guts into the ChangePath method which is called by
//   GetCurrentValues.
//
// ****************************************************************************

void
QvisFileWindowBase::pathChanged(int)
{
    GetCurrentValues(true);
}

// ****************************************************************************
// Method: QvisFileWindowBase::changeDirectory
//
// Purpose: 
//   This is a Qt slot function that is called when an item in the
//   directory menu is double clicked.
//
// Arguments:
//   item : The item in the directory list that was double-clicked.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 22 17:54:33 PST 2000
//
// Modifications:
//   Brad Whitlock, Tue Aug 21 15:01:52 PST 2001
//   Added a check to prevent extra slashes in the path name.
//
//   Brad Whitlock, Mon Oct 22 18:25:42 PST 2001
//   Changed the exception handling keywords to macros.
//
//   Brad Whitlock, Wed Feb 13 13:33:51 PST 2002
//   Removed some code to set the path in the path line edit.
//
//   Brad Whitlock, Thu May 9 17:08:17 PST 2002
//   Made it use the base class's fileServer pointer.
//
//   Brad Whitlock, Mon Aug 26 16:51:02 PST 2002
//   I made the code use new methods in the file server to get the file
//   separator so that different file separators can be used.
//
//   Brad Whitlock, Wed Sep 11 14:29:44 PST 2002
//   I changed the code so the Windows version can browse other disk drives.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Tue Jul  8 14:29:55 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisFileWindowBase::changeDirectory(QListWidgetItem *item) 
{
    // make sure current item is not null
    if(!item)
        return;

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
        Status(tr("Changing directory"));

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
            newPath += std::string(item->text().toStdString());

            // Try and switch to the new path.
            fileServer->SetPath(newPath);
            fileServer->Notify();
        }
    }
    CATCH(ChangeDirectoryException)
    {
        // Create a message and tell the user.
        QString msgStr = tr("The MetaData server running on %1 "
                            "could not change the current directory to %2.").
                         arg(fileServer->GetHost().c_str()).
                         arg(newPath.c_str());
        Error(msgStr);
    }
    CATCH(GetFileListException)
    {
        UpdateDirectoryList();
        UpdateFileList();

        // Create a message and tell the user.
        QString msgStr = tr("The MetaData server running on %1 could not "
                            "get the file list for the current directory.").
                         arg(fileServer->GetHost().c_str());
        Error(msgStr);
    }
    ENDTRY

    // Clear the status bar.
    ClearStatus();
}

// ****************************************************************************
// Method: QvisFileWindowBase::refreshFiles
//
// Purpose: 
//   This is a Qt slot function that refreshes the list of files in the
//   current directory.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 3 13:35:52 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu May 9 17:08:17 PST 2002
//   Made it use the base class's fileServer pointer.
//
// ****************************************************************************

void
QvisFileWindowBase::refreshFiles()
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
// Method: QvisFileWindowBase::currentDir
//
// Purpose: 
//   This is a Qt slot function that sets the "UseCurrentDirectory" flag
//   in the file server.
//
// Arguments:
//   val : The new "UseCurrentDirectory" flag.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 26 14:04:21 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileWindowBase::currentDir(bool val)
{
    fileServer->SetUseCurrentDirectory(val);
}

// ****************************************************************************
// Method: QvisFileWindowBase::fileGroupingChanged
//
// Purpose: 
//   This is a Qt slot function that sets the AutomaticFileGrouping flag in
//   the file server and updates the file list.
//
// Arguments:
//   val : The new AutomaticFileGrouping flag.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 27 09:47:28 PDT 2003
//
// Modifications:
//   Brad Whitlock, Thu Jul 29 14:02:06 PST 2004
//   Added support for smart file grouping.
//
// ****************************************************************************

void
QvisFileWindowBase::fileGroupingChanged(int val)
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
// Method: QvisFileWindowBase::showMinimized
//
// Purpose: 
//   Iconifies the window.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 10 16:50:12 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileWindowBase::showMinimized()
{
    QvisDelayedWindowSimpleObserver::showMinimized();
    if(recentPathsRemovalWindow && recentPathsRemovalWindow->isVisible())
        recentPathsRemovalWindow->showMinimized();
}

// ****************************************************************************
// Method: QvisFileWindowBase::showNormal
//
// Purpose: 
//   De-iconifies the window.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 10 16:50:12 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileWindowBase::showNormal()
{
    QvisDelayedWindowSimpleObserver::showNormal();
    if(recentPathsRemovalWindow && recentPathsRemovalWindow->isVisible())
    {
        recentPathsRemovalWindow->showNormal();
        recentPathsRemovalWindow->raise();
    }
}

// ****************************************************************************
// Method: QvisFileWindowBase::closeEvent
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
QvisFileWindowBase::closeEvent(QCloseEvent *e)
{
    if(recentPathsRemovalWindow && recentPathsRemovalWindow->isVisible())
        recentPathsRemovalWindow->hide();
    
    QvisDelayedWindowSimpleObserver::closeEvent(e);
}

//
// Helper functions
//

// ****************************************************************************
// Method: EncodeQualifiedFilename
//
// Purpose: 
//   Encodes QualifiedFilename into a QVariant that we can store in the
//   QListWidgetItem's user data.
//
// Arguments:
//   filename : The filename to encode.
//
// Returns:    A QVariant representation of the filename.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 10 15:57:11 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QVariant
EncodeQualifiedFilename(const QualifiedFilename &filename)
{
    QList<QVariant> values;
    values.append(QVariant(filename.host.c_str()));
    values.append(QVariant(filename.path.c_str()));
    values.append(QVariant(filename.filename.c_str()));
    values.append(QVariant(QChar(filename.separator)));
    values.append(QVariant(filename.CanAccess()));
    values.append(QVariant(filename.IsVirtual()));
    return QVariant(values);
}

// ****************************************************************************
// Method: DecodeQualifiedFilename
//
// Purpose: 
//   Decodes a QVariant into a QualifiedFilename.
//
// Arguments:
//   v : The QVariant to decode.
//
// Returns:    A QualifiedFilename initialized from the QVariant data.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 10 15:57:58 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QualifiedFilename
DecodeQualifiedFilename(const QVariant &v)
{
    QualifiedFilename f;
    QList<QVariant> values = v.toList();
    if(values.size() == 6)
    {
        f.host      = values[0].toString().toStdString();
        f.path      = values[1].toString().toStdString();
        f.filename  = values[2].toString().toStdString();
        f.separator = values[3].toChar().toAscii();
        f.SetAccess(values[4].toBool());
        f.SetVirtual(values[5].toBool());
    }

    return f;
}
