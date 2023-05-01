// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QSplitter>
#include <QWidget>
#include <QKeyEvent>

#include <QvisFileOpenWindow.h>
#include <QvisRecentPathRemovalWindow.h>
#include <FileServerList.h>
#include <DebugStream.h>
#include <HostProfileList.h>
#include <MachineProfile.h>
#include <ViewerProxy.h>
#include <DBPluginInfoAttributes.h>
#include <DBOptionsAttributes.h>
#include <QvisDBOptionsDialog.h>

#include <visit-config.h>

#include <algorithm>

using std::vector;
using std::string;

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
//   Brad Whitlock, Wed Apr  9 10:45:02 PDT 2008
//   Use QString for winCaption.
//
//   Brad Whitlock, Thu Jul 10 16:25:42 PDT 2008
//   Moved code to the base class.
//
// ****************************************************************************

QvisFileOpenWindow::QvisFileOpenWindow(const QString &winCaption) :
    QvisFileWindowBase(winCaption)
{
    usageMode = OpenFiles;
    hideFileFormat = false;

    showFilename = false;
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
//   Brad Whitlock, Thu Jul 10 16:26:26 PDT 2008
//   Moved code to the base class.
//
// ****************************************************************************

QvisFileOpenWindow::~QvisFileOpenWindow()
{
    if(dbplugins)
        dbplugins->Detach(this);
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
// Method: QvisFileOpenWindow::SetHideFileFormat
//
// Purpose:
//   Sets the flag to hide the data file gui options.
//
// Arguments:
//   value : flag to show or hide data file gui options.
//
// Programmer: David Camp
// Creation:   Thu Aug  6 09:05:15 PDT 2015
//
// Modifications:
//
// ****************************************************************************

void
QvisFileOpenWindow::SetHideFileFormat(bool value)
{
    hideFileFormat = value;
}

void
QvisFileOpenWindow::SetFilename(const QString &f)
{
    if(showFilename)
    {
        filenameEdit->setText(f);
    }
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
//   Jeremy Meredith, Wed Jan 23 15:32:24 EST 2008
//   Added button to let the user set the default opening options.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//   Initial Qt4 Port.
//
//   Cyrus Harrison, Tue Jul  8 14:34:17 PDT 2008
//   Fixed some slot connections.
//
//   Brad Whitlock, Thu Jul 10 16:26:43 PDT 2008
//   Moved code to the base class.
//
//   Brad Whitlock, Fri Jan 11 15:56:08 PST 2013
//   Pass central to the progress callback on newer Qt's.
//
//    David Camp, Thu Aug 27 09:40:00 PDT 2015
//    Added a filename field if showFilename is true. For Session dialog.
//    Also hide files if Session dialog.
// 
//   Justin Privitera, Tue May 17 11:06:11 PDT 2022
//   1) Fixed an issue where the list of plugins in the file open window could 
//   disappear when selecting plugins from the list multiple times.
//   2) In the plugin selection in the file open window, users can now delete 
//   the text that is there and start typing the name of the plugin that they 
//   wish to open files with, and VisIt will autocomplete the entered text to 
//   select a plugin. This was accomplished by making the fileFormatComboBox
//   "editable", but making its "insertPolicy" be "NoInsert". In this way,
//   users can type whatever they need in the box and the box will attempt
//   to autocomplete what they are typing, but they cannot add new entries to
//   the plugin list.
//
//    Kathleen Biagas, Wed Apr 6, 2022
//    Fix QT_VERSION test to use Qt's QT_VERSION_CHECK.
//
//    Kathleen Biagas, Wed Apr 19 14:42:07 PDT 2023
//    Replace deprecated `activated` SIGNAL with `currentIndexChanged`.
//
// ****************************************************************************

void
QvisFileOpenWindow::CreateWindowContents()
{  
    CreateHostPathFilterControls();

    // Add a grid layout for the file and directory lists.
    QSplitter *listSplitter = new QSplitter(central);
    listSplitter->setOrientation(Qt::Horizontal);
    topLayout->addWidget(listSplitter);

    //
    // Create the directory list.
    //
    QWidget     *directoryWidget = new QWidget(listSplitter);
    QVBoxLayout *directoryVBox = new QVBoxLayout(directoryWidget);
    directoryVBox->setContentsMargins(0,0,0,0);
    directoryVBox->addWidget(new QLabel(tr("Directories"), directoryWidget));

    directoryList = new QListWidget(directoryWidget);
    directoryVBox->addWidget(directoryList);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    int minColumnWidth = fontMetrics().horizontalAdvance("X");
#else
    int minColumnWidth = fontMetrics().width("X");
#endif
    directoryList->setMinimumWidth(minColumnWidth * 20);
    
    connect(directoryList, SIGNAL(itemActivated(QListWidgetItem *)),
            this, SLOT(changeDirectory(QListWidgetItem *)));
    listSplitter->addWidget(directoryWidget);

    //
    // Create the file list.
    //
    QWidget     *fileWidget = new QWidget(listSplitter);
    QVBoxLayout *fileVBox = new QVBoxLayout(fileWidget );
    fileVBox->setContentsMargins(0,0,0,0);

    fileVBox->addWidget(new QLabel(tr("Files"), listSplitter));
    fileList = CreateFileListWidget(listSplitter);
    fileVBox->addWidget(fileList);

    if(usageMode == OpenFiles)
        fileList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    else
        fileList->setSelectionMode(QAbstractItemView::SingleSelection);
    fileList->setMinimumWidth(minColumnWidth * 20);
    connect(fileList, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
            this, SLOT(selectFileDblClick(QListWidgetItem *)));
    //connect(fileList, SIGNAL(itemActivated(QListWidgetItem *)),
    //        this, SLOT(selectFileReturnPressed(QListWidgetItem *)));
    connect(fileList, SIGNAL(itemSelectionChanged()),
            this, SLOT(selectFileChanged()));
    fileList->installEventFilter(this);
    listSplitter->addWidget(fileWidget);

    // Create the file format combo box
    QHBoxLayout *fileFormatLayout = new QHBoxLayout();
    topLayout->addLayout(fileFormatLayout);
    QLabel *openFileAsTypeLabel = new QLabel(tr("Open file as type:"));
    fileFormatLayout->addWidget(openFileAsTypeLabel);
    fileFormatComboBox = new QComboBox(central);
    fileFormatComboBox->setEditable(true);
    fileFormatComboBox->setInsertPolicy(QComboBox::NoInsert);
    fileFormatComboBox->setMaxVisibleItems(50);
    // needs qt 5.15; at the time of writing we use qt 5.14
    // fileFormatComboBox->setPlaceholderText("Guess from file name/extension");
    fileFormatLayout->addWidget(fileFormatComboBox, 10);
    setDefaultOptionsForFormatButton = new QPushButton(
           tr("Set default open options..."), central);
    setDefaultOptionsForFormatButton->setEnabled(false);
    fileFormatLayout->addWidget(setDefaultOptionsForFormatButton, 1);
    fileFormatLayout->addStretch(5);
    connect(fileFormatComboBox, SIGNAL(currentIndexChanged(const QString&)),
            this, SLOT(fileFormatChanged(const QString&)));
    connect(setDefaultOptionsForFormatButton, SIGNAL(clicked()),
            this, SLOT(setDefaultOptionsForFormatButtonClicked()));
    if(hideFileFormat)
    {
        openFileAsTypeLabel->hide();
        fileFormatComboBox->hide();
        setDefaultOptionsForFormatButton->hide();
    }

    if(showFilename)
    {
        QGridLayout *pathLayout = new QGridLayout();
        topLayout->addLayout(pathLayout);
        pathLayout->setSpacing(10);

        // Create the filename
        filenameEdit = new QLineEdit(central);
        connect(filenameEdit, SIGNAL(returnPressed()), this, SLOT(okClicked()));
        connect(filenameEdit, SIGNAL(textChanged(const QString &)), this, SLOT(filenameEditChanged(const QString &)));
        filenameEdit->setFocus();
        QLabel *filenameLabel = new QLabel(tr("Filename"), central);
        pathLayout->addWidget(filenameLabel, 3, 0, Qt::AlignRight);
        pathLayout->addWidget(filenameEdit, 3, 1);
    }

    // create the lower button layout
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    topLayout->addLayout(buttonLayout);

    // Create the refresh button
    refreshButton = new QPushButton(tr("Refresh"), central);
    connect(refreshButton, SIGNAL(clicked()), this, SLOT(refreshFiles()));
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch(10);

    // Create the Ok button
    okButton = new QPushButton(tr("OK"), central);
    okButton->setEnabled(false);
    connect(okButton, SIGNAL(clicked()), this, SLOT(okClicked()));
    buttonLayout->addWidget(okButton);

    // Create the Cancel button
    cancelButton = new QPushButton(tr("Cancel"), central);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelClicked()));
    buttonLayout->addWidget(cancelButton);

    // Get the applied file list from the file server and store it in
    // the intermediateFile list.
    intermediateFileList = fileServer->GetAppliedFileList();

#if defined(Q_OS_MAC)
    // On Mac with Qt 4.8, we run into problems with the window not
    // coming back fully after calling setEnabled(true) on the window.
    // As a workaround, we disable the central widget instead of the
    // window itself. This lets the host, path, and filter widgets work
    // after we connected to a remote computer.
    fileServer->SetProgressCallback(ProgressCallback, (void *)central);
#endif
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
//    Jeremy Meredith, Tue Jul 17 11:34:59 EDT 2007
//    Store the DB plugin info atts that we obtain in a map based on
//    the host name.  This is because DBPluginInfoAttributes follows
//    the active plots, but we want to show the ones for the host
//    chosen in this window's combo box.
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
    {
        dbpluginmap[dbplugins->GetHost()] = *dbplugins;
        UpdateFileFormatComboBox();
    }
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
//  Modifications:
//    Jeremy Meredith, Tue Jul 17 11:39:10 EDT 2007
//    Use the DB plugin info atts obtained for this host.  This is because
//    DBPluginInfoAttributes now follows the active plots, but we want to
//    show the ones for the host chosen in this window's combo box.
//
//    Dave Pugmire, Wed Feb 13 15:43:24 EST 2008
//    Only add this DB plugin if the user has enabled it for loading.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//   Initial Qt4 Port.
// 
//   Justin Privitera, Tue Apr 19 12:07:06 PDT 2022
//   Sorted the filetype names alphabetically so the lower case filetypes are 
//   not at the end of the list.
//
// ****************************************************************************

void
QvisFileOpenWindow::UpdateFileFormatComboBox()
{
    if (dbpluginmap.count(fileServer->GetHost().c_str()) == 0)
    {
        return;
    }

    const DBPluginInfoAttributes &plugins =
        dbpluginmap[fileServer->GetHost().c_str()];

    QString oldtype = fileFormatComboBox->currentText();

    fileFormatComboBox->clear();
    int nTypes = plugins.GetTypes().size();
    fileFormatComboBox->addItem(tr("Guess from file name/extension"));
    FileOpenOptions *opts = GetViewerState()->GetFileOpenOptions();

    QStringList *filetypes = new QStringList();
    
    for (int i = 0 ; i < nTypes ; i++)
    {
        for (int j=0; j<opts->GetNumOpenOptions(); j++)
        {
            if (opts->GetTypeNames()[j] == plugins.GetTypes()[i] && opts->GetEnabled()[j] )
            {
                filetypes->push_back(plugins.GetTypes()[i].c_str());
                break;
            }
        }
    }

    filetypes->sort(Qt::CaseInsensitive);

    for (int i = 0; i < filetypes->size(); i ++)
    {
        fileFormatComboBox->addItem((*filetypes)[i]);
    }

    if (!oldtype.isNull())
    {
        for (int i=0; i<fileFormatComboBox->count(); i++)
        {
            if (oldtype == fileFormatComboBox->itemText(i))
            {
                fileFormatComboBox->setCurrentIndex(i);
                break;
            }
        }
    }
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
//   Brad Whitlock, Thu Jul 10 16:21:12 PDT 2008
//   Moved some code to base class.
//
// ****************************************************************************

void
QvisFileOpenWindow::SubjectRemoved(Subject *TheRemovedSubject)
{
    QvisFileWindowBase::SubjectRemoved(TheRemovedSubject);

    if (TheRemovedSubject == dbplugins)
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
//   Brad Whitlock, Thu Jul 10 16:23:46 PDT 2008
//   Moved some code to the base class.
//
// ****************************************************************************

void
QvisFileOpenWindow::ConnectSubjects(HostProfileList *hpl,
                                    DBPluginInfoAttributes *dbp)
{
    QvisFileWindowBase::ConnectSubjects(hpl);

    dbplugins = dbp;
    dbplugins->Attach(this);
}

//
// Qt Slot functions
//

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
//   Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//   Initial Qt4 Port.
//
//   Brad Whitlock, Mon Jul 14 11:07:38 PDT 2008
//   Moved code to the base class. Qt 4.
//
//   Brad Whitlock, Tue May 11 12:01:11 PDT 2010
//   I added code to set the gui's open file too.
//
//   Brad Whitlock, Mon Oct 11 16:26:08 PDT 2010
//   Don't open the file if you're just selecting a filename.
//
//    David Camp, Thu Aug 27 09:40:00 PDT 2015
//    If showFilename is true, then we just want a single filename.
//    Added for session dialog.
//
// ****************************************************************************

void
QvisFileOpenWindow::okClicked()
{
    QualifiedFilename emitFile;

    if( showFilename )
    {
        emitFile.host = fileServer->GetHost();
        emitFile.path = fileServer->GetPath();
        emitFile.filename  = filenameEdit->text().toStdString();
        emitFile.separator = fileServer->GetSeparator();
    }
    else
    {
        // Add all the selected files to the intermediate file list.
        for (int i = 0; i < fileList->count(); ++i)
        {
            if (!fileList->item(i)->isSelected())
                continue;

            // Add the file to the list if it's not in it.
            QualifiedFilename fn(DecodeQualifiedFilename(fileList->item(i)->
                data(Qt::UserRole)));
            AddFile(fn);

            // Save the name of the first file.
            if(emitFile.Empty())
                emitFile = fn;
        }
    }

    // If we selected a file, open it.
    if(!emitFile.Empty() &&
       usageMode != SelectFilename)
    {
        // Identify the file format used to open the file.
        string forcedFormat = "";
        if (fileFormatComboBox->currentIndex() != 0)
        {
            QString tmp = fileFormatComboBox->currentText();
            int ntypes = dbplugins->GetTypes().size();
            for (int j = 0 ; j < ntypes ; j++)
            {
                if (dbplugins->GetTypes()[j] == tmp.toStdString())
                {
                    forcedFormat = dbplugins->GetTypesFullNames()[j];
                    break;
                }
            }
        }
        // If there was a format, tell the file server to associate the filename
        // with the format so we don't guess when we open the file.
        if(!forcedFormat.empty())
            fileServer->SetFilePlugin(emitFile, forcedFormat);

        // Try and open the data file. This is the part that retrieves new metadata
        // for the gui and updates the menus, etc.
        SetOpenDataFile(emitFile, 0, 0, false);

        // Tell the viewer to open the file.
        GetViewerMethods()->OpenDatabase(emitFile.FullName().c_str(),
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

    //
    // Check all of the virtual databases that VisIt has opened for new
    // time states so they are up to date with what the user is doing
    // outside of VisIt because they *are* dumping files, etc.
    // See if the open file is in the intermediate file list and if it is
    // in there and it is a virtual file, check for new states on the viewer.
    //
    CheckForNewStates();

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
//    David Camp, Thu Aug 27 09:40:00 PDT 2015
//    If showFilename is true, then set the filename field.
//    Added for session dialog.
//
// ****************************************************************************

void
QvisFileOpenWindow::selectFileDblClick(QListWidgetItem *item) 
{
    if( showFilename )
    {
        filenameEdit->setText(fileList->currentItem()->text());
    }

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
//   Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//   Initial Qt4 Port.
//
//    David Camp, Thu Aug 27 09:40:00 PDT 2015
//    If showFilename is true, then set the filename field.
//    Added for session dialog.
//
// ****************************************************************************
void
QvisFileOpenWindow::selectFileChanged(void)
{
    // Count the number of selected files
    int count = 0;
    for(int i = 0; i < fileList->count(); ++i)
    {
        if(fileList->item(i)->isSelected())
        {
            count = 1;
            if(showFilename)
            {
                hideFileFormat = false;
                filenameEdit->setText(fileList->currentItem()->text());
                hideFileFormat = true;
            }
            break;
        }
    }

    if (count)
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
//   Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisFileOpenWindow::selectFile()
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
QvisFileOpenWindow::selectFileReturnPressed(QListWidgetItem *)
{
    //selectFile();
    okClicked();
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
//  Method:  QvisFileOpenWindow::setDefaultOptionsForFormatButtonClicked
//
//  Purpose:
//    When the button allowing the user to set the default opening options
//    is clicked, bake it happen.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2008
//
//  Modifications:
//   Cyrus Harrison, Mon Feb  4 09:45:22 PST 2008
//   Resolved AIX linking error w/ auto std::string to QString conversion.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************
void
QvisFileOpenWindow::setDefaultOptionsForFormatButtonClicked()
{
    QString format = fileFormatComboBox->currentText();
    FileOpenOptions *foo = GetViewerState()->GetFileOpenOptions();
    for (int i=0; i<foo->GetNumOpenOptions(); i++)
    {
        if (foo->GetTypeNames()[i].c_str() == format)
        {
            DBOptionsAttributes &opts = foo->GetOpenOptions(i);
            QvisDBOptionsDialog *optsdlg = new QvisDBOptionsDialog(&opts, NULL);
            QString caption = tr("Default file opening options for %1 reader").
                              arg(format);
            optsdlg->setWindowTitle(caption);
            int result = optsdlg->exec();
            delete optsdlg;
            if (result == QDialog::Accepted)
            {
                foo->Notify();
                GetViewerMethods()->SetDefaultFileOpenOptions();
            }
            break;
        }
    }
}

// ****************************************************************************
//  Method:  QvisFileOpenWindow::fileFormatChanged
//
//  Purpose:
//    Set the enabled state of the button to set the default opening options
//    based on whether or not the selected plugin has any options to set.
//
//  Arguments:
//    format     the name of the format plugin
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2008
//
//  Modifications:
//   Cyrus Harrison, Mon Feb  4 09:45:22 PST 2008
//   Resolved AIX linking error w/ auto std::string to QString conversion.
//
// ****************************************************************************
void
QvisFileOpenWindow::fileFormatChanged(const QString &format)
{
    bool enabled = false;
    FileOpenOptions *opts = GetViewerState()->GetFileOpenOptions();
    for (int i=0; i<opts->GetNumOpenOptions(); i++)
    {
        if (opts->GetTypeNames()[i].c_str() == format)
        {
            if (opts->GetOpenOptions(i).GetNumberOfOptions() > 0)
            {
                enabled = true;
            }
            break;
        }
    }
    setDefaultOptionsForFormatButton->setEnabled(enabled);
}

bool
QvisFileOpenWindow::eventFilter(QObject *o, QEvent *e)
{
    if(e->type() == QEvent::KeyPress)
    {
        QKeyEvent* event = dynamic_cast<QKeyEvent*>(e);
        if(event->key() == Qt::Key_Return)
        {
            okClicked();
            return true;
        }
    }
    return false;
}

// ****************************************************************************
//  Method:  QvisFileOpenWindow::SetShowFilename
//
//  Purpose:
//    Set flag to show filename field
//
//  Arguments:
//    value     bool value true to show. Default is false
//
//  Programmer:  David Camp
//  Creation:    Thu Aug 27 09:40:00 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

void
QvisFileOpenWindow::SetShowFilename(bool value)
{
    showFilename = value;
}

// ****************************************************************************
//  Method:  QvisFileOpenWindow::filenameEditChanged
//
//  Purpose:
//    Signal for the filename field if the user changes the text.
//
//  Arguments:
//    text     new text in field.
//
//  Programmer:  David Camp
//  Creation:    Thu Aug 27 09:40:00 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

void 
QvisFileOpenWindow::filenameEditChanged(const QString &text)
{
    if(hideFileFormat)
        fileList->clearSelection();
    if(text.isEmpty())
        okButton->setEnabled(false);
    else
        okButton->setEnabled(true);
}

