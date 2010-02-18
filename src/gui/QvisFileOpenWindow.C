/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
#include <QDesktopWidget>
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
// ****************************************************************************

void
QvisFileOpenWindow::CreateWindowContents()
{  
    CreateHostPathFilterControls();

    // Add a grid layout for the file and directory lists.
    topLayout->addSpacing(10);
    QSplitter *listSplitter = new QSplitter(central);
    listSplitter->setOrientation(Qt::Horizontal);
    topLayout->addWidget(listSplitter);

    //
    // Create the directory list.
    //
    QWidget     *directoryWidget = new QWidget(listSplitter);
    QVBoxLayout *directoryVBox = new QVBoxLayout(directoryWidget);
    directoryVBox->setMargin(0);
    directoryVBox->addWidget(new QLabel(tr("Directories"), directoryWidget));

    directoryList = new QListWidget(directoryWidget);
    directoryVBox->addWidget(directoryList);
    
    int minColumnWidth = fontMetrics().width("X");
    directoryList->setMinimumWidth(minColumnWidth * 20);
    
    connect(directoryList, SIGNAL(itemActivated(QListWidgetItem *)),
            this, SLOT(changeDirectory(QListWidgetItem *)));
    listSplitter->addWidget(directoryWidget);

    //
    // Create the file list.
    //
    QWidget     *fileWidget = new QWidget(listSplitter);
    QVBoxLayout *fileVBox = new QVBoxLayout(fileWidget );
    fileVBox->setMargin(0);

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
    connect(fileList, SIGNAL(itemActivated(QListWidgetItem *)),
            this, SLOT(selectFileReturnPressed(QListWidgetItem *)));
    connect(fileList, SIGNAL(itemSelectionChanged()),
            this, SLOT(selectFileChanged()));

    listSplitter->addWidget(fileWidget);
    // Create the file format combo box
    QHBoxLayout *fileFormatLayout = new QHBoxLayout();
    topLayout->addLayout(fileFormatLayout);
    fileFormatLayout->addWidget(new QLabel(tr("Open file as type:")));
    fileFormatComboBox = new QComboBox(central);
    fileFormatLayout->addWidget(fileFormatComboBox, 10);
    setDefaultOptionsForFormatButton = new QPushButton(
           tr("Set default open options..."), central);
    setDefaultOptionsForFormatButton->setEnabled(false);
    fileFormatLayout->addWidget(setDefaultOptionsForFormatButton, 1);
    fileFormatLayout->addStretch(5);
    connect(fileFormatComboBox, SIGNAL(activated(const QString&)),
            this, SLOT(fileFormatChanged(const QString&)));
    connect(setDefaultOptionsForFormatButton, SIGNAL(clicked()),
            this, SLOT(setDefaultOptionsForFormatButtonClicked()));

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
    
    for (int i = 0 ; i < nTypes ; i++)
    {
        for (int j=0; j<opts->GetNumOpenOptions(); j++)
        {
            if (opts->GetTypeNames()[j] == plugins.GetTypes()[i] && opts->GetEnabled()[j] )
            {
                fileFormatComboBox->addItem(plugins.GetTypes()[i].c_str());
                break;
            }
        }
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
// ****************************************************************************

void
QvisFileOpenWindow::okClicked()
{
    // Add all the selected files to the intermediate file list.
    QualifiedFilename emitFile;
    for (int i = 0; i < fileList->count(); ++i)
    {
        if (!fileList->item(i)->isSelected())
            continue;

        // Add the file to the list if it's not in it.
        QualifiedFilename fn(DecodeQualifiedFilename(fileList->item(i)->
            data(Qt::UserRole)));
        AddFile(fn);

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

        // Save the name of the first file.
        if(emitFile.Empty())
            emitFile = fn;

        GetViewerMethods()->OpenDatabase(fn.FullName().c_str(),
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
//
// ****************************************************************************

void
QvisFileOpenWindow::selectFileDblClick(QListWidgetItem *item) 
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
//   Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************
void
QvisFileOpenWindow::selectFileChanged(void)
{
    // Count the number of selected files
    int count = 0;
    for(int i = 0; i < fileList->count(); ++i)
        if(fileList->item(i)->isSelected())
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
    selectFile();
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
