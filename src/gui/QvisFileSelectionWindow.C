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

#include <QvisFileSelectionWindow.h>
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
// Class: QFileSelectionListBoxItem
//
// Purpose:
//   This class represents a list box item that gets inserted into the file
//   list.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 28 12:49:33 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class QFileSelectionListBoxItem : public QListBoxItem
{
public:
    QFileSelectionListBoxItem(QListBox *listbox, const QString &name,
                              const QualifiedFilename &qf, QPixmap *pm = 0) :
        QListBoxItem(listbox), fileName(qf)
    {
        setText(name);
        pixmap = pm;
    }

    virtual ~QFileSelectionListBoxItem()
    {
    }

    virtual int height(const QListBox *lb) const
    {
        int textHeight = lb ? lb->fontMetrics().lineSpacing() + 2 : 0;
        if(pixmap)
            return QMAX(textHeight, pixmap->height() + 2);
        else
            return textHeight;
    }

    virtual int width(const QListBox *lb)  const
    {
        int textWidth = lb ? lb->fontMetrics().width(text()) + 6 : 0;
        textWidth += (pixmap ? (pixmap->width() + 6) : 0);
        return textWidth;
    }

    const QualifiedFilename &GetFilename() const
    {
        return fileName;
    }

protected:
    virtual void paintSpecial(QPainter *painter)
    {
    }

    virtual void paint(QPainter *painter)
    {
        QPen backupPen(painter->pen());
        QFontMetrics fm = painter->fontMetrics();

        // If we can't access the file, make it gray.
        if(!fileName.CanAccess())
            painter->setPen(Qt::gray);
        else if(listBox())
        {
            if(selected())
                painter->setPen(listBox()->colorGroup().highlightedText());
            else
                painter->setPen(listBox()->colorGroup().text());
        }
        else
            painter->setPen(Qt::black);

        // If the file is virtual, draw a database pixmap.
        int offset = 3;
        if(pixmap)
        {
            offset += (pixmap->width() + 3);
            painter->drawPixmap(3, 0, *pixmap);
        }

        // Draw the text.
        painter->drawText(offset, fm.ascent() + fm.leading()/2, text() );

        // Draw anything special.
        paintSpecial(painter);

        // Restore the old pen.
        painter->setPen(backupPen);
    }

protected:
    QualifiedFilename  fileName;
    QPixmap           *pixmap;
};

// ****************************************************************************
// Class: QVirtualFileListBoxItem
//
// Purpose: 
//   This class represents a list box item that corresponds to a virtual file.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 31 11:06:59 PDT 2003
//
// Modifications:
//   Brad Whitlock, Thu Dec 15 13:19:50 PST 2005
//   I shrank large virtual databases to 10 time states. The rest are omitted
//   to make displaying them work better.
//
// ****************************************************************************

class QVirtualFileListBoxItem : public QFileSelectionListBoxItem
{
public:
    static const int MAX_DISPLAYED_NAMES;

    QVirtualFileListBoxItem(QListBox *lb, const QString &name,
                            const QualifiedFilename &qf, const stringVector &n,
                            QPixmap *pm = 0) :
        QFileSelectionListBoxItem(lb, name, qf, pm), names(n)
    {
    }

    virtual ~QVirtualFileListBoxItem()
    {
    }

    virtual int height(const QListBox *lb) const
    {
        int textHeight = 0;

        if(lb)
        {
            if(names.size() > (MAX_DISPLAYED_NAMES*2))
                textHeight = ((MAX_DISPLAYED_NAMES*2) + 1 + 1 + 1) * lb->fontMetrics().lineSpacing() + 2;
            else
                textHeight = (names.size() + 1) * lb->fontMetrics().lineSpacing() + 2;
        }

        if(pixmap)
            return QMAX(textHeight, pixmap->height() + 2);
        else
            return textHeight;
    }

    virtual int width(const QListBox *lb)  const
    {
        int textWidth = 0;
        if(lb)
        {
            for(int i = 0; i < names.size(); ++i)
            {
                int w = lb->fontMetrics().width(names[i].c_str());
                textWidth = QMAX(w, textWidth);
            }
        }

        textWidth += (pixmap ? (pixmap->width() + 6) : 0);
        return textWidth;
    }

protected:
    virtual void paintSpecial(QPainter *painter)
    {
        QFileSelectionListBoxItem::paintSpecial(painter);

        // Draw the virtual filenames.
        if(fileName.IsVirtual())
        {
            QFontMetrics fm = painter->fontMetrics();
            int offset = pixmap ? (pixmap->width() + 6) : 3;
            int yIncr = fm.lineSpacing(); // + 2; //fm.ascent() + fm.leading()/2;
            int y = yIncr + fm.ascent() + fm.leading()/2;
            int i;

            if(names.size() > (MAX_DISPLAYED_NAMES*2))
            {
                QString nFilesString;
                nFilesString.sprintf("(%d total files)", names.size());
                painter->drawText(offset, y, nFilesString);
                y += yIncr;

                for(i = 0; i < MAX_DISPLAYED_NAMES; ++i)
                {
                    painter->drawText(offset, y, names[i].c_str());
                    y += yIncr;
                }

                painter->drawText(offset, y, "...");
                y += yIncr;

                for(i = names.size()-MAX_DISPLAYED_NAMES; i < names.size(); ++i)
                {
                    painter->drawText(offset, y, names[i].c_str());
                    y += yIncr;
                }
            }
            else
            {
                for(i = 0; i < names.size(); ++i)
                {
                    painter->drawText(offset, y, names[i].c_str());
                    y += yIncr;
                }
            }
        }
    }

private:
    stringVector names;
};

const int QVirtualFileListBoxItem::MAX_DISPLAYED_NAMES = 5;

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
// ****************************************************************************

QvisFileSelectionWindow::QvisFileSelectionWindow(const char *winCaption) :
    QvisDelayedWindowSimpleObserver(winCaption), intermediateFileList(),
    currentVirtualDatabaseDefinitions(), invalidHosts()
{
    fs = 0;
    profiles = 0;

    computerPixmap = 0;
    folderPixmap = 0;
    databasePixmap = 0;

    recentPathsRemovalWindow = 0;

    // Set the progress callback that we want to use while we
    // connect to the mdserver.
    fileServer->SetProgressCallback(ProgressCallback, (void *)this);
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
// ****************************************************************************

QvisFileSelectionWindow::~QvisFileSelectionWindow()
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
// ****************************************************************************

void
QvisFileSelectionWindow::CreateWindowContents()
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
    fileList->setSelectionMode(QListBox::Extended);
    fileList->setMinimumWidth(minColumnWidth * 20);
    connect(fileList, SIGNAL(doubleClicked(QListBoxItem *)),
            this, SLOT(selectFileDblClick(QListBoxItem *)));
    connect(fileList, SIGNAL(returnPressed(QListBoxItem *)),
            this, SLOT(selectFileReturnPressed(QListBoxItem *)));
    connect(fileList, SIGNAL(selectionChanged()),
            this, SLOT(selectFileChanged()));

    //
    // Create the selection buttons.
    //
    QVBox *selectVBox = new QVBox(listSplitter, "selectVBox");
    selectButton = new QPushButton("Select", selectVBox, "selectButton");
    connect(selectButton, SIGNAL(clicked()), this, SLOT(selectFile()));
    selectButton->setEnabled(false);
    selectAllButton = new QPushButton("Select all", selectVBox, "selectAllButton");
    connect(selectAllButton, SIGNAL(clicked()), this, SLOT(selectAllFiles()));
    removeButton = new QPushButton("Remove", selectVBox, "removeButton");
    connect(removeButton, SIGNAL(clicked()), this, SLOT(removeFile()));
    removeButton->setEnabled(false);
    removeAllButton = new QPushButton("Remove all", selectVBox, "removeAllButton");
    connect(removeAllButton, SIGNAL(clicked()), this, SLOT(removeAllFiles()));
    groupButton = new QPushButton("Group", selectVBox, "groupButton");
    connect(groupButton, SIGNAL(clicked()), this, SLOT(groupFiles()));
    groupButton->setEnabled(false);     // Until we have some selections
    refreshButton = new QPushButton("Refresh", selectVBox, "refreshButton");
    connect(refreshButton, SIGNAL(clicked()), this, SLOT(refreshFiles()));

    //
    // Create the selected file list.
    //
    QVBox *selfileVBox = new QVBox(listSplitter, "selfileVBox"); 
    new QLabel("Selected files", selfileVBox, "selectedFileLabel");
    selectedFileList = new QListBox(selfileVBox, "selectedFileList");
    selectedFileList->setSelectionMode(QListBox::Extended);
    selectedFileList->setMinimumWidth(minColumnWidth * 30);
    connect(selectedFileList, SIGNAL(selectionChanged()),
            this, SLOT(selectedFileSelectChanged()));
    connect(selectedFileList, SIGNAL(returnPressed(QListBoxItem *)),
            this, SLOT(removeSelectedFiles(QListBoxItem *)));

    // Create the Ok button
    QHBoxLayout *buttonLayout = new QHBoxLayout(topLayout);
    QPushButton *okButton = new QPushButton("OK", central, "okButton");
    connect(okButton, SIGNAL(clicked()), this, SLOT(okClicked()));
    buttonLayout->addStretch(10);
    buttonLayout->addWidget(okButton);

    // Create the Cancel button
    QPushButton *cancelButton = new QPushButton("Cancel", central, "cancelButton");
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
//   Brad Whitlock, Wed Feb 13 10:41:56 PDT 2002
//   Modified code to account for the change to comboboxes.
//
//   Brad Whitlock, Mon Feb 25 15:45:39 PST 2002
//   Changed how the host combobox is updated.
//
//   Brad Whitlock, Thu May 9 17:08:17 PST 2002
//   Made it use the base class's fileServer pointer.
//
//   Brad Whitlock, Thu Sep 12 12:31:08 PDT 2002
//   I renamed the method.
//
//   Brad Whitlock, Thu Mar 27 09:45:28 PDT 2003
//   I added automatic file grouping.
//
//   Brad Whitlock, Tue Apr 29 09:14:31 PDT 2003
//   I added code to catch an exception that has not come up here until
//   recently.
//
//   Brad Whitlock, Mon Oct 13 10:00:17 PDT 2003
//   I made it update the path combo box when the recent path list changes.
//
//   Brad Whitlock, Thu Jul 29 13:58:09 PST 2004
//   I added smart file grouping and I made it call UpdateHostComboBox so
//   hosts are always added to the host combo box in the same way.
//
// ****************************************************************************

void
QvisFileSelectionWindow::UpdateWindowFromFiles(bool doAll)
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

    // If the appliedFileList has changed, update the appliedFile list.
    if(fileServer->AppliedFileListChanged() || doAll)
    {
        UpdateSelectedFileList();
    }
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::UpdateHostComboBox
//
// Purpose: 
//   This method is called when the host profile list changes. The purpose is
//   to add any new host names into the host combo box so they are easily
//   accessible.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 12 12:33:19 PDT 2002
//
// Modifications:
//    Jeremy Meredith, Mon Apr 14 17:46:20 PDT 2003
//    Allowed the host name of the profile to contain mutliple real host
//    names.  This function now splits the host name pattern before 
//    adding them to the drop-down list box.
//
//    Brad Whitlock, Thu Jul 29 14:55:18 PST 2004
//    I made it use the recent hosts list and the host profiles and the
//    list of invalid files to populate the host combo box. Since all areas
//    of code in this file now call this method to update the host combo
//    box, there will be no more inconsistencies.
//
// ****************************************************************************

void
QvisFileSelectionWindow::UpdateHostComboBox()
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
// Programmer: Brad Whitlock
// Creation:   Mon Feb 25 15:48:01 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Apr 5 16:43:05 PST 2005
//   Added code to make the combo box's list box taller.
//
// ****************************************************************************

void
QvisFileSelectionWindow::UpdateComboBox(QComboBox *cb, const stringVector &s,
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
// Method: QvisFileSelectionWindow::UpdateDirectoryList
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
// ****************************************************************************

void
QvisFileSelectionWindow::UpdateDirectoryList()
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
// ****************************************************************************

void
QvisFileSelectionWindow::UpdateFileList()
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
// ****************************************************************************

void
QvisFileSelectionWindow::UpdateSelectedFileList()
{
    QualifiedFilenameVector::const_iterator pos;
    bool needsHost = false;
    bool needsPath = false;

    //
    // Search through the list of selected files and see if we'll need to
    // display host or path information.
    //
    if(intermediateFileList.size() > 0)
    {
        std::string host = intermediateFileList[0].host;
        std::string path = intermediateFileList[0].path;
        for(int i = 1; i < intermediateFileList.size(); ++i)
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
        if(pos->IsVirtual())
        {
            new QVirtualFileListBoxItem(selectedFileList, fileName, *pos,
                fileServer->GetVirtualFileDefinition(*pos), databasePixmap);
        }
        else
            new QFileSelectionListBoxItem(selectedFileList, fileName, *pos);
    }

    if (selectedFileList->count() >= 1)
        removeAllButton->setEnabled(true);
    else
        removeAllButton->setEnabled(false);
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::RemoveComboBoxItem
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
//   
// ****************************************************************************

void
QvisFileSelectionWindow::RemoveComboBoxItem(QComboBox *cb,
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
// Method: QvisFileSelectionWindow::ActivateComboBoxItem
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
//   
// ****************************************************************************

void
QvisFileSelectionWindow::ActivateComboBoxItem(QComboBox *cb,
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
// Method: QvisFileSelectionWindow::HighlightComboBox
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
QvisFileSelectionWindow::HighlightComboBox(QComboBox *cb)
{
    // Select the line edit's text.
    QLineEdit *le = cb->lineEdit();
    le->setSelection(0, le->text().length());
    le->setFocus();
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::AddFile
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
QvisFileSelectionWindow::AddFile(const QualifiedFilename &newFile)
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
// Method: QvisFileSelectionWindow::GetCurrentValues
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
QvisFileSelectionWindow::GetCurrentValues(bool allowPathChange)
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
// Method: QvisFileSelectionWindow::ChangeHosts
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
// ****************************************************************************

bool
QvisFileSelectionWindow::ChangeHosts()
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
// Method: QvisFileSelectionWindow::ChangePath
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
//    Brad Whitlock, Wed Sep 11 17:21:42 PST 2002
//    I made it highlight the combo box when the input is bad.
//
// ****************************************************************************

bool
QvisFileSelectionWindow::ChangePath(bool allowPathChange)
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
// ****************************************************************************

bool
QvisFileSelectionWindow::ChangeFilter()
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
// Method: QvisFileSelectionWindow::GetDirectoryStrings
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
// ****************************************************************************

void
QvisFileSelectionWindow::GetDirectoryStrings(QString &curDir, QString &upDir)
{
    curDir = ". (current directory)";
    upDir = ".. (go up 1 directory level)";
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::SubjectRemoved
//
// Purpose: 
//   This function is called when a subject is removed.
//
// Arguments:
//   TheRemovedSubject : The subject being removed.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 12 12:25:37 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileSelectionWindow::SubjectRemoved(Subject *TheRemovedSubject)
{
    if(TheRemovedSubject == fs)
        fs = 0;
    else if(TheRemovedSubject == profiles)
        profiles = 0;
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::ConnectSubjects
//
// Purpose: 
//   This function connects subjects so that the window observes them.
//
// Arguments:
//   hpl : The host profile list that we want to observe.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 12 12:26:30 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileSelectionWindow::ConnectSubjects(HostProfileList *hpl)
{
    fs = fileServer;
    fileServer->Attach(this);

    profiles = hpl;
    profiles->Attach(this);
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::ProgressCallback
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
// ****************************************************************************

bool
QvisFileSelectionWindow::ProgressCallback(void *data, int stage)
{
    QvisFileSelectionWindow *This = (QvisFileSelectionWindow *)data;

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
// Method: QvisFileSelectionWindow::GetVirtualDatabaseDefinitions
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
QvisFileSelectionWindow::GetVirtualDatabaseDefinitions(
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
//   
// ****************************************************************************

void
QvisFileSelectionWindow::setEnabled(bool val)
{
    QvisDelayedWindowSimpleObserver::setEnabled(val);

    if(isCreated)
    {
        hostComboBox->setEnabled(val);
        pathComboBox->setEnabled(val);
        filterLineEdit->setEnabled(val);
        directoryList->setEnabled(val);
        fileList->setEnabled(val);
        selectedFileList->setEnabled(val);
        currentDirToggle->setEnabled(val);
    }
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
// ****************************************************************************

void
QvisFileSelectionWindow::okClicked()
{
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
                    debug1 << "QvisFileSelectionWindow::okClicked: The virtual "
                           << "database definition for "
                           << fileName.c_str()
                           << " did not change. No reopen is required."
                           << endl;
                }
                else
                {
                    debug1 << "QvisFileSelectionWindow::okClicked: The virtual "
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
                    viewer->CheckForNewStates(fileName);
                }
            }
        }
    }

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
// Method: QvisFileSelectionWindow::filterChanged
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
QvisFileSelectionWindow::filterChanged()
{
    GetCurrentValues(false);
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::hostChanged
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
QvisFileSelectionWindow::hostChanged(int)
{
    GetCurrentValues(false);
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::pathChanged
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
QvisFileSelectionWindow::pathChanged(int)
{
    GetCurrentValues(true);
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::changeDirectory
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
// ****************************************************************************

void
QvisFileSelectionWindow::changeDirectory(QListBoxItem *item) 
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
// ****************************************************************************

void
QvisFileSelectionWindow::selectFileDblClick(QListBoxItem *item) 
{
    // Unselect all the files.
    for(int i = 0; i < fileList->count(); ++i)
        fileList->setSelected(i, false);

    // Add the file to the list.
    QFileSelectionListBoxItem *item2 = (QFileSelectionListBoxItem *)item;
    if(AddFile(item2->GetFilename()))
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
//   
// ****************************************************************************
void
QvisFileSelectionWindow::selectFileChanged(void)
{
    // Count the number of selected files
    int count = 0;
    for(int i = 0; i < fileList->count(); ++i)
        if(fileList->isSelected(i))
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
//
// ****************************************************************************
void
QvisFileSelectionWindow::UpdateRemoveFileButton(void)
{
    // Count the number of selected files
    int count = 0;
    for(int i = 0; i < selectedFileList->count(); ++i)
        if(selectedFileList->isSelected(i))
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
// ****************************************************************************

void
QvisFileSelectionWindow::selectFile()
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
QvisFileSelectionWindow::selectFileReturnPressed(QListBoxItem *)
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
// ****************************************************************************

void
QvisFileSelectionWindow::selectAllFiles()
{
    int i, addCount = 0;

    // Add all the selected files to the intermediate file list.
    for(i = 0; i < fileList->count(); ++i)
    {
        // Add the file to the list if it's not in it.
        QFileSelectionListBoxItem *item = (QFileSelectionListBoxItem *)fileList->item(i);
        if(AddFile(item->GetFilename()))
            ++addCount;
    }

    // Unselect all the files.
    for(i = 0; i < fileList->count(); ++i)
        fileList->setSelected(i, false);

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
        if(!selectedFileList->isSelected(i))
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
QvisFileSelectionWindow::removeSelectedFiles(QListBoxItem *)
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
// ****************************************************************************

void
QvisFileSelectionWindow::groupFiles()
{
    int i;
    stringVector groupList;

    // Add all the selected files to the intermediate file list.
    for(i = 0; i < fileList->count(); ++i)
    {
        if(!fileList->isSelected(i))
            continue;

        groupList.push_back(fileList->text(i).latin1());
    }

    // Unselect all the files.
    for(i = 0; i < fileList->count(); ++i)
        fileList->setSelected(i, false);

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
    QString tmp;
    tmp.sprintf("Creating grouplist on %s", fileServer->GetHost().c_str());
    Status(tmp);
    fileServer->CreateGroupList(group_filename, groupList);

    // Update the file list because it (hopefully) has changed as a result
    // of our actions.
    UpdateFileList();
    ClearStatus();
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::refreshFiles
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
QvisFileSelectionWindow::refreshFiles()
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

// ****************************************************************************
// Method: QvisFileSelectionWindow::showMinimized
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
QvisFileSelectionWindow::showMinimized()
{
    QvisDelayedWindowSimpleObserver::showMinimized();
    if(recentPathsRemovalWindow && recentPathsRemovalWindow->isVisible())
        recentPathsRemovalWindow->showMinimized();
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::showNormal
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
QvisFileSelectionWindow::showNormal()
{
    QvisDelayedWindowSimpleObserver::showNormal();
    if(recentPathsRemovalWindow && recentPathsRemovalWindow->isVisible())
    {
        recentPathsRemovalWindow->showNormal();
        recentPathsRemovalWindow->raise();
    }
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::currentDir
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
QvisFileSelectionWindow::currentDir(bool val)
{
    fileServer->SetUseCurrentDirectory(val);
}

// ****************************************************************************
// Method: QvisFileSelectionWindow::fileGroupingChanged
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
QvisFileSelectionWindow::fileGroupingChanged(int val)
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
// Method: QvisFileSelectionWindow::closeEvent
//
// Purpose: 
//   Closes the window and also makes sure that the recentPathRemoval window
//   gets closed if it is open.
//
// Arguments:
//   e : The close event to handle.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 13 11:39:34 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisFileSelectionWindow::closeEvent(QCloseEvent *e)
{
    if(recentPathsRemovalWindow && recentPathsRemovalWindow->isVisible())
        recentPathsRemovalWindow->hide();
    
    QvisDelayedWindowSimpleObserver::closeEvent(e);
}
