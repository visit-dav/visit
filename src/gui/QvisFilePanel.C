#include <qlabel.h>
#include <qlistview.h> 
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlineedit.h> 
#include <qslider.h>
#include <qtimer.h>

#include <QvisFilePanel.h>
#include <QvisVCRControl.h>
#include <QvisListViewFileItem.h>

#include <FileServerList.h>
#include <GlobalAttributes.h>
#include <ViewerProxy.h>
#include <GetMetaDataException.h>
#include <avtDatabaseMetaData.h>

// Include the XPM files used for the icons.
#include <icons/database.xpm>
#include <icons/folder.xpm>
#include <icons/computer.xpm>

// ****************************************************************************
// Class: FileTree
//
// Purpose:
//   This class is used internally to represent a file tree that can undergo
//   reduce operations in order to simplify the tree.
//
// Notes:      FileTree::FileTreeNode is an internal class that is used to
//             help represent the file tree's nodes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 16:47:19 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Aug 26 16:32:05 PST 2002
//   I changed the code so it can handle different kinds of file separators.
//
//   Brad Whitlock, Wed May 14 15:38:40 PST 2003
//   I added support for virtual databases being expanded by default.
//
// ****************************************************************************

class FileTree
{
public:
    class FileTreeNode
    {
    public:
        static const int ROOT_NODE;
        static const int PATH_NODE;
        static const int FILE_NODE;
        static const int DATABASE_NODE;

        FileTreeNode(int nodeT = FILE_NODE);
        ~FileTreeNode();
        FileTreeNode *Add(int nType, const std::string &name,
                          const QualifiedFilename &fName, char separator);
        bool Reduce();
        FileTreeNode *Find(const std::string &path);
        int Size() const;
        bool HasChildrenOfType(int type);
        void AddElementsToListViewItem(QListViewItem *item, int &fileIndex,
                                       bool addRoot,
                                       const QPixmap &folderPixmap,
                                       const QPixmap &dbPixmap,
                                       QvisFilePanel *filePanel);
        void AddElementsToListView(QListView *listview, int &fileIndex,
                                   const QPixmap &folderPixmap,
                                   const QPixmap &databasePixmap,
                                   QvisFilePanel *filePanel);
        QString NumberedFilename(int fileIndex) const;

        std::string separator_str()
        {
            char str[2] = {separator, '\0'};
            return std::string(str);
        }

        char              separator;
        int               nodeType;
        std::string       nodeName;
        QualifiedFilename fileName;
        int               numChildren;
        FileTreeNode      **children;
    private:
        void Destroy();
    };

    FileTree(QvisFilePanel *fp);
    ~FileTree();

    void Add(const QualifiedFilename &fileName, char separator);
    void Reduce();
    int  Size() const;
    bool TreeContainsDirectories() const;

    void AddElementsToListViewItem(QListViewItem *item, int &fileIndex,
                                   const QPixmap &folderPixmap,
                                   const QPixmap &dbPixmap);
    void AddElementsToListView(QListView *item, int &fileIndex,
                               const QPixmap &folderPixmap,
                               const QPixmap &databasePixmap);
private:
    std::string separator_str()
    {
        char str[2] = {separator, '\0'};
        return std::string(str);
    }

    char           separator;
    FileTreeNode  *root;
    QvisFilePanel *filePanel;
};

const int FileTree::FileTreeNode::ROOT_NODE     = 0;
const int FileTree::FileTreeNode::PATH_NODE     = 1;
const int FileTree::FileTreeNode::FILE_NODE     = 2;
const int FileTree::FileTreeNode::DATABASE_NODE = 3;

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: QvisFilePanel::QvisFilePanel
//
// Purpose: 
//   Constructor for the QvisFilePanel class.
//
// Arguments:
//   parent     : A pointer to the widget's parent widget.
//   name       : The widget's name.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 31 10:35:07 PDT 2000
//
// Modifications:
//   Brad Whitlock, Thu Mar 22 11:24:16 PDT 2001
//   Added code to create the folder pixmap.
//
//   Brad Whitlock, Thu Feb 28 14:01:20 PST 2002
//   Connected slots for collapse and expand.
//
//   Jeremy Meredith, Mon Aug 19 16:22:44 PDT 2002
//   Initialized globalAtts.
//
//   Brad Whitlock, Fri May 16 12:28:49 PDT 2003
//   Initialized displayInfo.
//
// ****************************************************************************

QvisFilePanel::QvisFilePanel(QWidget *parent, const char *name) :
   QWidget(parent, name), SimpleObserver(), GUIBase(), displayInfo()
{
    // Create the top layout that will contain the widgets.
    QVBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->setSpacing(5);
    
    // Create the selected file list.
    fileListView = new QListView(this, "fileList");
    fileListView->addColumn("Selected files", 1);
    fileListView->setColumnWidth(0, fileListView->visibleWidth());
    fileListView->setVScrollBarMode(QScrollView::AlwaysOn);
    fileListView->setHScrollBarMode(QScrollView::Auto);
    connect(fileListView, SIGNAL(doubleClicked(QListViewItem *)),
            this, SLOT(openFileDblClick(QListViewItem *)));
    connect(fileListView, SIGNAL(selectionChanged(QListViewItem *)),
            this, SLOT(highlightFile(QListViewItem *)));
    connect(fileListView, SIGNAL(expanded(QListViewItem *)),
            this, SLOT(fileExpanded(QListViewItem *)));
    connect(fileListView, SIGNAL(collapsed(QListViewItem *)),
            this, SLOT(fileCollapsed(QListViewItem *)));
    topLayout->addWidget(fileListView, 10);

    // Create the file opening buttons.
    QHBoxLayout *buttonLayout = new QHBoxLayout(topLayout);
    topLayout->setStretchFactor(buttonLayout, 10);
    openButton = new QPushButton(tr("Open"), this, "openButton");
    openButton->setEnabled(false);
    connect(openButton, SIGNAL(clicked()), this, SLOT(openFile()));
    buttonLayout->addWidget(openButton);

    replaceButton = new QPushButton(tr("Replace"), this, "replaceButton");
    replaceButton->setEnabled(false);
    connect(replaceButton, SIGNAL(clicked()), this, SLOT(replaceFile()));
    buttonLayout->addWidget(replaceButton);

    overlayButton = new QPushButton(tr("Overlay"), this, "overlayButton");
    overlayButton->setEnabled(false);
    connect(overlayButton, SIGNAL(clicked()), this, SLOT(overlayFile()));
    buttonLayout->addWidget(overlayButton);

    // Create the animation position slider bar
    QHBoxLayout *animationLayout = new QHBoxLayout(topLayout);
    topLayout->setStretchFactor(animationLayout, 10);
    animationPosition = new QSlider(Qt::Horizontal, this, "animationPosition");
    animationPosition->setEnabled(false);
    connect(animationPosition, SIGNAL(sliderPressed()),
            this, SLOT(sliderStart()));
    connect(animationPosition, SIGNAL(sliderMoved(int)),
            this, SLOT(sliderMove(int)));
    connect(animationPosition, SIGNAL(sliderReleased()),
            this, SLOT(sliderEnd()));
    connect(animationPosition, SIGNAL(valueChanged(int)),
            this, SLOT(sliderChange(int)));
    animationLayout->addWidget(animationPosition, 1000);

    // Create the animation time field.
    timeField = new QLineEdit(this, "timeField");
    timeField->setMaxLength(5);
    timeField->setEnabled(false);
    connect(timeField, SIGNAL(returnPressed()), this, SLOT(processTimeText()));
    animationLayout->addWidget(timeField, 5);

    // Create the VCR controls.
    vcrControls = new QvisVCRControl(this, "vcr" );
    vcrControls->setEnabled(false);
    connect(vcrControls, SIGNAL(prevFrame()), this, SLOT(prevFrame()));
    connect(vcrControls, SIGNAL(reversePlay()), this, SLOT(reversePlay()));
    connect(vcrControls, SIGNAL(stop()), this, SLOT(stop()));
    connect(vcrControls, SIGNAL(play()), this, SLOT(play()));
    connect(vcrControls, SIGNAL(nextFrame()), this, SLOT(nextFrame()));
    topLayout->addWidget(vcrControls, 10);

    // Create the computer pixmap.
    computerPixmap = new QPixmap(computer_xpm);
    // Create the database pixmap.
    databasePixmap = new QPixmap(database_xpm);
    // Create the folder pixmap and add it to the pixmap cache.
    folderPixmap = new QPixmap(folder_xpm);

    // Initialize the attached subjects
    globalAtts = NULL;
    sliderDown = false;
}

// ****************************************************************************
// Method: QvisFilePanel::~QvisFilePanel
//
// Purpose: 
//   Destructor for the QvisFilePanel class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 31 10:36:46 PDT 2000
//
// Modifications:
//   Brad Whitlock, Fri Feb 1 14:28:06 PST 2002
//   Added code to delete the widget's pixmaps.
//
// ****************************************************************************

QvisFilePanel::~QvisFilePanel()
{
    if(fileServer)
        fileServer->Detach(this);

    if(globalAtts)
        globalAtts->Detach(this);

    // Delete the pixmaps.
    delete computerPixmap;
    delete databasePixmap;
    delete folderPixmap;
}

// ****************************************************************************
// Method: QvisFilePanel::Update
//
// Purpose: 
//   This method tells the widget to update itself when the subject
//   changes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 31 10:37:19 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilePanel::Update(Subject *TheChangedSubject)
{
    if(fileServer == 0 || globalAtts == 0)
        return;

    if(TheChangedSubject == fileServer)
        UpdateFileList(false);
    else if(TheChangedSubject == globalAtts)
        UpdateAnimationControls(false);
}

// ****************************************************************************
// Method: QvisFilePanel::UpdateFileList
//
// Purpose: 
//   This method updates the widget to reflect the new state stored
//   in the FileServerList that this widget is watching.
//
// Arguments:
//   doAll : Whether or not to update everything. If this is false,
//           then it should pay attention to what attributes actually
//           changed.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 31 10:38:11 PDT 2000
//
// Modifications:
//   Brad Whitlock, Thu Mar 22 11:25:03 PDT 2001
//   I rewrote the code that generates the widgets to put into the file list.
//
//   Brad Whitlock, Thu Feb 28 13:26:18 PST 2002
//   I added code to manage the expanded files.
//
//   Brad Whitlock, Mon Aug 19 13:38:02 PST 2002
//   I moved the code that updates the width of the list view into a slot
//   function that is called with a timer.
//
//   Brad Whitlock, Mon Mar 31 15:09:51 PST 2003
//   I added database pixmaps for virtual files.
//
//   Brad Whitlock, Fri Jun 20 14:08:30 PST 2003
//   I fixed a bug that caused the GUI to hang when there were files from
//   more than one host.
//
//   Brad Whitlock, Mon Sep 15 14:48:45 PST 2003
//   I removed some code that was no longer needed.
//
// ****************************************************************************

void
QvisFilePanel::UpdateFileList(bool doAll)
{
    if(fileServer == 0 || globalAtts == 0)
        return;

    // If the appliedFileList has changed, update the appliedFile list.
    if(fileServer->AppliedFileListChanged() || doAll)
    {
        const QualifiedFilenameVector &f = fileServer->GetAppliedFileList();
        // Holds pointers to the list item for each host.
        std::map<std::string, QListViewItem *> hostMap;

        // Go through all of the files and build a list of unique hosts.
        bool hostOtherThanLocalHost = false;
        QualifiedFilenameVector::const_iterator pos;
        for(pos = f.begin(); pos != f.end(); ++pos)
        {
            if(pos->host.size() > 0)
            {
                // Add an entry for the host.
                hostMap[pos->host] = 0;

                // See if the host is not localhost.
                if(pos->host != std::string("localhost"))
                    hostOtherThanLocalHost = true;
            }
        }

        // Clear out the fileListView widget.
        fileListView->clear();

        // Reset the node numbers that will be used to create the nodes.
        QvisListViewFileItem::resetNodeNumber();

        // If there are multiple hosts or just one and it is not localhost,
        // add nodes in the file tree for the host. This makes sure that it
        // is always clear which host a file came from.
        if(hostMap.size() > 1)
        {
            std::map<std::string, QListViewItem *>::iterator hpos;

            // Create the root for the host list.
            QualifiedFilename rootName("Hosts", "(root)", "(root)");
            QListViewItem *root = new QvisListViewFileItem(fileListView, 
                QString("Hosts"),
                rootName,
                QvisListViewFileItem::ROOT_NODE);

            // Always expand the root node.
            AddExpandedFile(rootName);

            // Create all of the host listview items.
            int fileIndex = 1;
            for(hpos = hostMap.begin(); hpos != hostMap.end(); ++hpos)
            {
                char separator = fileServer->GetSeparator(hpos->first);
                QualifiedFilename hostOnly(hpos->first, "(host)", "(host)");
                hostOnly.separator = separator;
                QListViewItem *newFile = new QvisListViewFileItem(root, 
                    QString(hpos->first.c_str()), hostOnly,
                    QvisListViewFileItem::HOST_NODE);
                newFile->setPixmap (0, *computerPixmap);
                // Add the widget to the host map.
                hpos->second = newFile;

                // Add the files to the file tree and reduce it.
                FileTree files(this);
                for(pos = f.begin(); pos != f.end(); ++pos)
                {
                    if(pos->host == hpos->first)
                        files.Add(*pos, separator);
                }
                files.Reduce();

                // Add all the elements in the files list to the host list
                // view item.
                files.AddElementsToListViewItem(newFile, fileIndex,
                                                *folderPixmap, *databasePixmap);
            }
        }
        else
        {
            // Add the files to the file tree and reduce it.
            FileTree files(this);
            for(pos = f.begin(); pos != f.end(); ++pos)
            {
                char separator = fileServer->GetSeparator(pos->host);
                files.Add(*pos, separator);
            }
            files.Reduce();

            // If there are top level directories in the file tree, then we
            // need to create a node for the host.
            if(files.TreeContainsDirectories() ||
               hostOtherThanLocalHost)
            {
                // Create the root for the host list.
                QualifiedFilename rootName(f[0].host, "(host)", "(host)");
                QListViewItem *root = new QvisListViewFileItem(fileListView, 
                    QString(f[0].host.c_str()), rootName,
                    QvisListViewFileItem::ROOT_NODE);
                root->setPixmap (0, *computerPixmap);

                // Make sure that the host is expanded.
                AddExpandedFile(rootName);

                int fileIndex = 1;
                files.AddElementsToListViewItem(root, fileIndex, *folderPixmap,
                                                *databasePixmap);
            }
            else
            {
                // Traverse the file tree and make widgets for the items in it.
                int fileIndex = 1;
                files.AddElementsToListView(fileListView, fileIndex, *folderPixmap,
                                            *databasePixmap);
            }
        }

        // Expand any databases that we know about. This just means that we add
        // the extra information that databases have, we don't expand the tree
        // until we enter UpdateFileSelection.
        ExpandDatabases();

        // Highlight the selected file.
        UpdateFileSelection();

        // Set the width of the zeroeth column.
        QTimer::singleShot(100, this, SLOT(updateHeaderWidth()));
    }
    else if(fileServer->FileChanged())
    {
        // Expand any databases that we know about. This just means that we add
        // the extra information that databases have, we don't expand the tree
        // until we enter UpdateFileSelection.
        ExpandDatabases();

        // Highlight the selected file.
        UpdateFileSelection();

        // Set the animation controls.
        const avtDatabaseMetaData *md = fileServer->GetMetaData();
        bool enabled = md ? (md->GetNumStates() > 1) : false;
        vcrControls->setEnabled(enabled);
        animationPosition->setEnabled(enabled);
        timeField->setEnabled(enabled);
    }
}

// ****************************************************************************
// Method: QvisFilePanel::UpdateAnimationControls
//
// Purpose: 
//   This method is called when the GlobalAttributes subject that this
//   widget watches is updated.
//
// Arguments:
//   doAll : A flag indicating whether to ignore any partial selection.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 26 16:04:38 PST 2000
//
// Modifications:
//   Brad Whitlock, Tue Aug 21 16:53:13 PST 2001
//   Moved the cycle/time code into a sub-function.
//
//   Eric Brugger, Fri Nov  2 17:07:24 PST 2001
//   I added code to handle the current file changing.
//
//   Eric Brugger, Mon Dec 16 13:05:01 PST 2002
//   I seperated the concepts of state and frame, since they are no longer
//   equivalent with keyframe support.
//
//   Brad Whitlock, Thu Mar 20 11:11:14 PDT 2003
//   I changed the ordering of fields in GlobalAttributes.
//
//   Brad Whitlock, Thu May 15 13:19:13 PST 2003
//   I changed the call to FileServer::OpenFile.
//
//   Brad Whitlock, Wed Jul 2 15:58:47 PST 2003
//   I added exception handling code for when the metadata cannot be read.
//
//   Brad Whitlock, Tue Sep 9 15:40:40 PST 2003
//   I made it return early if the slider is down.
//
// ****************************************************************************

void
QvisFilePanel::UpdateAnimationControls(bool doAll)
{
    if(fileServer == 0 || globalAtts == 0 || sliderDown)
        return;

    // currentFile changed.  Update the file server.
    if(globalAtts->IsSelected(6) || doAll)
    {
        if (globalAtts->GetCurrentFile() == "notset")
        {
            fileServer->CloseFile();
            fileServer->Notify();
        }
        else
        {
            QualifiedFilename qf(globalAtts->GetCurrentFile());

            TRY
            {
                fileServer->OpenFile(qf, globalAtts->GetCurrentState());
                fileServer->Notify();
            }
            CATCH(GetMetaDataException)
            {
                ; // Usually, the filename was bad.
            }
            ENDTRY
        }
    }

    // currentState changed.
    if(globalAtts->IsSelected(7) || globalAtts->IsSelected(8) || doAll)
    {
        // Update the selected file since we have a new current state.
        UpdateFileSelection();
    }

    // currentFrame or nFrames changed. Update the slider.
    if(globalAtts->IsSelected(9) || globalAtts->IsSelected(10) || doAll)
    {
        int nFrames = globalAtts->GetNFrames() - 1;
        if(nFrames < 0)
            nFrames = 0;
        animationPosition->blockSignals(true);
        animationPosition->setRange(0, nFrames);
        animationPosition->setPageStep(1);
        animationPosition->setValue(globalAtts->GetCurrentFrame());
        animationPosition->blockSignals(false);
    }

    // If the VCR controls are selected (animationMode), set the mode.
    if(globalAtts->IsSelected(11) || doAll)
    {
        vcrControls->blockSignals(true);
        vcrControls->SetActiveButton(globalAtts->GetAnimationMode());
        vcrControls->blockSignals(false);
    }

    // Set the time field to the cycle number.
    UpdateTimeFieldText(globalAtts->GetCurrentState());

    // Set the enabled state of the animation widgets based on the 
    // number of frames.
    vcrControls->setEnabled(globalAtts->GetNFrames() > 1);
    animationPosition->setEnabled(globalAtts->GetNFrames() > 1);

    // Set the enabled state of the time field based on the number
    // of states in the database.
    timeField->setEnabled(globalAtts->GetNStates() > 1);    
}

// ****************************************************************************
// Method: QvisFilePanel::UpdateTimeFieldText
//
// Purpose: 
//   Updates the text in the time/cycle text field.
//
// Arguments:
//   timeState : The timestate to update the cycle to.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 21 16:52:35 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilePanel::UpdateTimeFieldText(int timeState)
{
    if(fileServer == 0 || globalAtts == 0)
        return;

    // Set the time field to the cycle number.
    const avtDatabaseMetaData *md = fileServer->GetMetaData();
    if((md != 0) && (md->GetNumStates() > 1))
    {
        QString timeString;
        timeString.sprintf("%04d", md->GetCycles()[timeState]);
        timeField->setText(timeString);
    }
    else
        timeField->setText("");
}

// ****************************************************************************
// Method: QvisFilePanel::ExpandDatabases
//
// Purpose: 
//   This method traverses the listviewitem tree and looks for the
//   selected file. When it finds the selected file, it turns it into
//   a database if the file has more than 1 state.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 26 11:11:04 PDT 2000
//
// Modifications:
//   Brad Whitlock, Thu Oct 19 13:17:31 PST 2000
//   I removed a NULL pointer reference.
//
//   Brad Whitlock, Wed Mar 21 00:54:50 PDT 2001
//   Changed a call to the QvisListViewFileItem constructor.
//
//   Brad Whitlock, Thu Feb 28 13:39:38 PST 2002
//   Removed unnecessary recursion.
//
//   Brad Whitlock, Tue Mar 25 16:03:16 PST 2003
//   I added code to handle virtual databases.
//
//   Brad Whitlock, Wed May 14 16:19:50 PST 2003
//   I changed how we iterate though the items and I moved most of the
//   code to a new method.
//
//   Brad Whitlock, Wed Sep 17 18:18:50 PST 2003
//   Fixed a small bug with how the cycles are displayed when we have a
//   virtual file that does not have all of the cycle numbers.
//
// ****************************************************************************

void
QvisFilePanel::ExpandDatabases()
{
    // Count the number of items in the fileListView. I didn't see a way to
    // count them all without traversing them all.
    int i, count = 0;
    QListViewItemIterator it(fileListView);
    for ( ; it.current(); ++it )
        ++count;

    // If there are no items, return early.
    if(count < 1)
        return;

    // Create an array to store pointers to all of the items. We save the
    // pointers in an array because later when we expand databases, we
    // can't traverse using an iterator because as we add items, the
    // iterator is invalidated.
    QvisListViewFileItem **items = new QvisListViewFileItem*[count];
    it = QListViewItemIterator(fileListView);
    for (i = 0; it.current(); ++it, ++i)
        items[i] = (QvisListViewFileItem *)it.current();

    //
    // Iterate through the items and expand them if they are databases.
    //
    for(i = 0; i < count; ++i)
    {
        QvisListViewFileItem *item = items[i];
        if(item != 0)
        {
            if(item->firstChild() == 0)
            {
                ExpandDatabaseItem(item);
            }
            else if(HaveFileInformation(item->file) &&
                    !FileShowsCorrectCycles(item->file))
            {
                // See if the file is a database
                const avtDatabaseMetaData *md = fileServer->GetMetaData(item->file);
                if(md != 0 && md->GetNumStates() > 1)
                {
                    int maxts = QMIN(md->GetNumStates(), item->childCount());
                    QListViewItemIterator it(item); ++it;
                    for(int j = 0; j < maxts; ++j, ++it)
                    {
                         QString label;
                         int cycle = (j < md->GetCycles().size()) ? md->GetCycles()[j] : j;
                         if(md->GetIsVirtualDatabase())
                         {
                             QualifiedFilename name(md->GetTimeStepNames()[j]);
                             label.sprintf("%s cycle %04d", name.filename.c_str(),
                                 cycle);
                         }
                         else
                             label.sprintf("cycle %04d", cycle);

                         // Reset the label so that it shows the right values.
                         it.current()->setText(0, label);
                    }

                    // Remember that the item now has the correct information
                    // displayed through its children.
                    SetFileShowsCorrectCycles(item->file, true);
                }
            }
        }
    }

    // Delete the temporary array.
    delete [] items;
}

// ****************************************************************************
// Method: QvisFilePanel::ExpandDatabaseItem
//
// Purpose: 
//   Expands the item into a database if it is a database.
//
// Arguments:
//   item : The item to be expanded.
//
// Programmer: Brad Whitlock
// Creation:   Thu May 15 09:46:37 PDT 2003
//
// Modifications:
//   Brad Whitlock, Wed Sep 17 18:20:07 PST 2003
//   I corrected an error with how the cycle is displayed for virtual files
//   that don't know all of the cycles.
//
// ****************************************************************************

void
QvisFilePanel::ExpandDatabaseItem(QvisListViewFileItem *item)
{
    //
    // It could be that this method is being called on the child timestate of 
    // a previously expanded database. If that is the case, then the parent
    // will exist and it will have the same filename as the item.
    //
    QvisListViewFileItem *parent = (QvisListViewFileItem *)item->parent();
    if(parent != 0 && parent->file == item->file)
        return;

    if(fileServer->HaveOpenedFile(item->file))
    {
        // See if the file is a database
        const avtDatabaseMetaData *md = fileServer->GetMetaData(item->file);
        if(md != 0 && md->GetNumStates() > 1)
        {              
            fileListView->blockSignals(true);
            for(int i = 0; i < md->GetNumStates(); ++i)
            {
                QString label;
                int cycle = (i < md->GetCycles().size()) ? md->GetCycles()[i] : i;
                if(md->GetIsVirtualDatabase())
                {
                    QualifiedFilename name(md->GetTimeStepNames()[i]);
                    label.sprintf("%s cycle %04d", name.filename.c_str(),
                        cycle);
                }
                else
                    label.sprintf("cycle %04d", cycle);
                QvisListViewFileItem *fi = new QvisListViewFileItem(
                    item, label, fileServer->GetOpenFile(),
                    QvisListViewFileItem::FILE_NODE, i);
                fi->setOpen(false);
            }

            // Set the database pixmap.
            item->setPixmap(0, *databasePixmap);
            fileListView->blockSignals(false);

            // Remember that the item now has the correct information
            // displayed through its children.
            SetFileShowsCorrectCycles(item->file, true);
        }
    }
    else if(item->file.IsVirtual())
    {
        fileListView->blockSignals(true);

        // Get the virtual file definition instead of reading the metadata.
        stringVector files(fileServer->GetVirtualFileDefinition(item->file));

        for(int i = 0; i < files.size(); ++i)
        {
            QString label(files[i].c_str());
            QvisListViewFileItem *fi = new QvisListViewFileItem(
                item, label, item->file,
                QvisListViewFileItem::FILE_NODE, i);
            fi->setOpen(false);
        }

        // Set the database pixmap.
        item->setPixmap(0, *databasePixmap);
        fileListView->blockSignals(false);

        // Remember that the item does not have the correct information
        // displayed through its children since we have not opened it yet.
        SetFileShowsCorrectCycles(item->file, false);
    }
}

// ****************************************************************************
// Method: QvisFilePanel::UpdateFileSelection
//
// Purpose: 
//   Highlights the selected file in the file list view.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 13 11:48:35 PDT 2000
//
// Modifications:
//   Brad Whitlock, Tue Mar 20 23:39:14 PST 2001
//   Changed code so it takes directories into account.
//
//   Brad Whitlock, Thu Feb 28 13:34:11 PST 2002
//   Rewrote it so it handles preservation of tree expansion.
//
//   Eric Brugger, Mon Dec 16 13:05:01 PST 2002
//   I seperated the concepts of state and frame, since they are no longer
//   equivalent with keyframe support.
//
//   Brad Whitlock, Wed Apr 2 10:13:12 PDT 2003
//   I made it try and use the metadata for the open file instead of the
//   globalAtts which are not reliable.
//
// ****************************************************************************

void
QvisFilePanel::UpdateFileSelection()
{
    // Set the text for the open file button.
    if(fileServer->GetOpenFile().Empty())
        openButton->setText("Open");
    else
    {
        openButton->setText("ReOpen");
        openButton->setEnabled(true);
    }

    // If the file changed, then we have to update the selected item.
    blockSignals(true);
    fileListView->blockSignals(true);

    //
    // Iterate through all items of the listview and open up the appropriate items
    // so that expansion is preserved.
    //
    QListViewItemIterator it(fileListView);
    QvisListViewFileItem *selectedItem = 0;

    for(; it.current(); ++it)
    {
        QvisListViewFileItem *item = (QvisListViewFileItem *)it.current();

        // If it is the root of the tree, open it and continue.
        if(item->nodeType == QvisListViewFileItem::ROOT_NODE)
        {
            AddExpandedFile(item->file);
            fileListView->setOpen(item, true);
            fileListView->setSelected(item, false);
            continue;
        }

        if(item->file == fileServer->GetOpenFile())
        {
            QvisListViewFileItem *parentItem = (QvisListViewFileItem *)item->parent();

            // The file is open, so try and use its metadata to get the number
            // of time states.
            const avtDatabaseMetaData *md = fileServer->GetMetaData(item->file);
            int nStates = md ? md->GetNumStates() : globalAtts->GetNStates();

            if(nStates > 1)
            {
                // Check if the current item is for the current time state.
                bool currentTimeState = item->timeState == globalAtts->GetCurrentState();

                // If we have information for this file and it turns out that the file
                // is not expanded, then check to see if the item is the root of the
                // database.
                if(HaveFileInformation(item->file) && !FileIsExpanded(item->file))
                {
                    //
                    // The cycles are not expanded. If the time state that we're
                    // looking at is not -1 then it is a time step in the database
                    // but since the file is not expanded, we want to instead highlight
                    // the parent.
                    //
                    if(item->timeState != -1)
                    {
                        item = (QvisListViewFileItem *)item->parent();
                        parentItem = (QvisListViewFileItem *)parentItem->parent();
                    }
                }

                //
                // If we have no selected item so far and the item's timestate is the
                // current time state, then we want to select it.
                //
                if(currentTimeState)
                    selectedItem = item;
            }
            else if(selectedItem == 0)
            {
                // If this time step / database is selected then select it.
                selectedItem = item;
            }

            // Make sure the item is visible.
            QvisListViewFileItem *p = item;
            while((p = parentItem) != 0)
            {
                QvisListViewFileItem *fi = (QvisListViewFileItem *)p;
                if(fi->file != item->file)
                {
                    AddExpandedFile(fi->file);
                    fi->setOpen(true);
                }
                parentItem = (QvisListViewFileItem *)parentItem->parent();
            } 
        }
        else if(HaveFileInformation(item->file) &&
                FileIsExpanded(item->file))
        {
            item->setOpen(true);
        }
    }

    // Make sure the selected item is visible.
    if(selectedItem != 0)
    {
        fileListView->setSelected(selectedItem, true);
        fileListView->setCurrentItem(selectedItem);
        fileListView->ensureItemVisible(selectedItem);
    }

    // Restore signals.
    fileListView->blockSignals(false);
    blockSignals(false);
}

// ****************************************************************************
// Method: QvisFilePanel::SubjectRemoved
//
// Purpose: 
//   Removes the globalAtts or fileserver subjects that this widget
//   observes.
//
// Arguments:
//   TheRemovedSubject : The subject that is being removed.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 26 15:29:39 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu May 9 16:40:58 PST 2002
//   Removed file server.
//
// ****************************************************************************

void
QvisFilePanel::SubjectRemoved(Subject *TheRemovedSubject)
{
    if(TheRemovedSubject == globalAtts)
        globalAtts = 0;
}

//
// Methods to attach to the globalAtts and fileserver objects.
//

void
QvisFilePanel::ConnectFileServer(FileServerList *fs)
{
    fileServer->Attach(this);

    // Update the file list.
    UpdateFileList(true);
}

void
QvisFilePanel::ConnectGlobalAttributes(GlobalAttributes *ga)
{
    globalAtts = ga;
    globalAtts->Attach(this);

    // Update the animation area.
    UpdateAnimationControls(true);
}

// ****************************************************************************
// Method: QvisFilePanel::OpenFile
//
// Purpose: 
//   Tells the file server to open the selected file and notify all
//   observers that the file was opened.
//
// Arguments:
//   filename : The name of the file to open.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 31 11:01:32 PDT 2000
//
// Modifications:
//   Brad Whitlock, Fri May 18 09:57:29 PDT 2001
//   Added code to handle a meta-data exception.
//
//   Brad Whitlock, Mon Oct 22 18:25:42 PST 2001
//   Changed the exception handling keywords to macros.
//
//   Brad Whitlock, Thu Feb 7 11:17:40 PDT 2002
//   Modified the code so a more descriptive error message is given
//   in the case of a metadata exception.
//
//   Brad Whitlock, Thu May 9 16:33:34 PST 2002
//   Moved the code to GUIBase::OpenDataFile.
//
//   Brad Whitlock, Mon Jul 29 14:35:28 PST 2002
//   I added code to set the text for the open button.
//
//   Brad Whitlock, Fri Feb 28 08:30:24 PDT 2003
//   I made the reOpen flag be passed in instead of being calculated in here.
//
//   Brad Whitlock, Fri Sep 5 16:40:20 PST 2003
//   I added code that lets the replace button be active if there's more than
//   one state in the database.
//
// ****************************************************************************

bool
QvisFilePanel::OpenFile(const QualifiedFilename &qf, int timeState, bool reOpen)
{
    // Try and open the data file.
    bool retval = OpenDataFile(qf, timeState, this, reOpen);

    // Get a pointer to the file's metadata.
    const avtDatabaseMetaData *md = fileServer->GetMetaData(qf);
    int nTimeStates = md ? md->GetNumStates() : 1;

    replaceButton->setEnabled(nTimeStates > 1);
    overlayButton->setEnabled(false);

    return retval;
}

// ****************************************************************************
// Method: QvisFilePanel::ReplaceFile
//
// Purpose: 
//   Tells the file server to replace the current file with the
//   selected file and notify all observers that the file changed.
//
// Arguments:
//   filename : The name of the new file.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 31 11:01:32 PDT 2000
//
// Modifications:
//   Brad Whitlock, Wed Mar 6 16:21:17 PST 2002
//   Implemented it using the new viewer RPC.
//
//   Brad Whitlock, Fri Feb 28 08:44:52 PDT 2003
//   I added a bool to OpenFile.
//
//   Brad Whitlock, Thu May 15 12:30:28 PDT 2003
//   I added time state to ReplaceFile.
//
// ****************************************************************************

void
QvisFilePanel::ReplaceFile(const QualifiedFilename &filename)
{
    if(OpenFile(filename, 0, false))
    {
        viewer->ReplaceDatabase(filename.FullName().c_str());
    }
}

// ****************************************************************************
// Method: QvisFilePanel::OverlayFile
//
// Purpose: 
//   Tells the file server to overlay the current file with the
//   selected file and notify all observers that the file changed.
//
// Arguments:
//   filename : The name of the new file.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 31 11:01:32 PDT 2000
//
// Modifications:
//   Brad Whitlock, Wed Mar 6 16:21:17 PST 2002
//   Implemented it using the new viewer RPC.
//
//   Brad Whitlock, Fri Feb 28 08:45:11 PDT 2003
//   I added a bool to OpenFile.
//
//   Brad Whitlock, Thu May 15 12:31:13 PDT 2003
//   I added time state to OpenFile.
//
// ****************************************************************************

void
QvisFilePanel::OverlayFile(const QualifiedFilename &filename)
{
    if(OpenFile(filename, 0, false))
    {
        viewer->OverlayDatabase(filename.FullName().c_str());
    }
}

// ****************************************************************************
// Method: QvisFilePanel::AddExpandedFile
//
// Purpose: 
//   Adds a file to the expanded file list.
//
// Arguments:
//   filename : The file to add to the list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 1 13:20:23 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri May 16 12:42:15 PDT 2003
//   I rewrote it.
//   
// ****************************************************************************

void
QvisFilePanel::AddExpandedFile(const QualifiedFilename &filename)
{
    SetFileExpanded(filename, true);
}

// ****************************************************************************
// Method: QvisFilePanel::RemoveExpandedFile
//
// Purpose: 
//   Removes a file from the expanded file list.
//
// Arguments:
//   filename : The name of the file to remove from the list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 1 13:21:30 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri May 16 12:42:15 PDT 2003
//   I rewrote it.
//
// ****************************************************************************

void
QvisFilePanel::RemoveExpandedFile(const QualifiedFilename &filename)
{
    SetFileExpanded(filename, false);
}

// ****************************************************************************
// Method: QvisFilePanel::SetFileExpanded
//
// Purpose: 
//   Sets the expanded flag for the specified filename.
//
// Arguments:
//   filename : The file whose expanded flag we want to set.
//   val      : The expanded value.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 16 12:40:29 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilePanel::SetFileExpanded(const QualifiedFilename &filename, bool val)
{
    displayInfo[filename.FullName()].expanded = val;
}

// ****************************************************************************
// Method: QvisFilePanel::HaveFileInformation
//
// Purpose: 
//   Returns whether or not we have file information for a file.
//
// Arguments:
//   filename : The file whose information we want.
//
// Returns:    Whether or not we have file information for the specified file.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 16 13:11:19 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
QvisFilePanel::HaveFileInformation(const QualifiedFilename &filename) const
{
    return displayInfo.find(filename.FullName()) != displayInfo.end();
}

// ****************************************************************************
// Method: QvisFilePanel::FileIsExpanded
//
// Purpose: 
//   Returns whether or not a file is expanded.
//
// Arguments:
//   filename : The file we're checking.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 1 13:22:04 PST 2002
//
// Modifications:
//   
// ****************************************************************************

bool
QvisFilePanel::FileIsExpanded(const QualifiedFilename &filename)
{
    return displayInfo[filename.FullName()].expanded;
}

// ****************************************************************************
// Method: QvisFilePanel::FileShowsCorrectCycles
//
// Purpose: 
//   Returns whether or not we're showing the right cycles for the file.
//
// Arguments:
//   filename : The filename that we're checking.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 16 12:47:14 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
QvisFilePanel::FileShowsCorrectCycles(const QualifiedFilename &filename)
{
    return displayInfo[filename.FullName()].correctCycles;
}

// ****************************************************************************
// Method: QvisFilePanel::SetFileShowsCorrectCycles
//
// Purpose: 
//   Sets the flag that indicates whether or not we're showing the right
//   cycles for a file.
//
// Arguments:
//   filename : The file whose information we're setting.
//   val      : The new flag.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 16 13:05:20 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilePanel::SetFileShowsCorrectCycles(const QualifiedFilename &filename,
    bool val)
{
    displayInfo[filename.FullName()].correctCycles = val;
}

//
// Qt slot functions.
//

// ****************************************************************************
// Method: QvisFilePanel::prevFrame
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to switch to
//   the previous frame in an animation.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 1 10:28:22 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilePanel::prevFrame()
{
    // Tell the viewer to go to the previous frame.
    viewer->AnimationPreviousFrame();
}

// ****************************************************************************
// Method: QvisFilePanel::reversePlay
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to play the
//   current animation in reverse.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 1 10:28:22 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilePanel::reversePlay()
{
    // Tell the viewer to play the animation in reverse.
    viewer->AnimationReversePlay();
}

// ****************************************************************************
// Method: QvisFilePanel::stop
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to stop playing
//   the current animation.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 1 10:28:22 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilePanel::stop()
{
    // Tell the viewer to stop the animation.
    viewer->AnimationStop();
}

// ****************************************************************************
// Method: QvisFilePanel::play
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to play the
//   current animation.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 1 10:28:22 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilePanel::play()
{
    // Tell the viewer to play the animation.
    viewer->AnimationPlay();
}

// ****************************************************************************
// Method: QvisFilePanel::nextFrame
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to switch to
//   the next frame in an animation.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 1 10:28:22 PDT 2000
//
// Modifications:
//   Brad Whitlock, Wed Jul 30 16:52:23 PST 2003
//   I made it emit the new reopenOnNextFrame signal if we're on the last
//   frame of the animation.
//
// ****************************************************************************

void
QvisFilePanel::nextFrame()
{
    //
    // If we're not playing an animation and we're at the last frame in the
    // animation, then try and reopen the current file to see if there are
    // more timestates.
    //
    if(globalAtts->GetAnimationMode() == 2 &&
       globalAtts->GetCurrentFrame() == globalAtts->GetNFrames() - 1)
    {
        emit reopenOnNextFrame();
    }
    else
    {
        // Tell the viewer to go to the next frame.
        viewer->AnimationNextFrame();
    }
}

// ****************************************************************************
// Method: QvisFilePanel::fileCollapsed
//
// Purpose: 
//   This is a Qt slot function that collapses a file and thus removes it
//   from the expanded files list.
//
// Arguments:
//   item : The item that is being collapsed.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 1 13:22:59 PST 2002
//
// Modifications:
//   Eric Brugger, Mon Dec 16 13:05:01 PST 2002
//   I seperated the concepts of state and frame, since they are no longer
//   equivalent with keyframe support.
//
//   Brad Whitlock, Mon Sep 8 14:54:20 PST 2003
//   Changed the logic for removing files from the list of expanded files.
//
// ****************************************************************************

void
QvisFilePanel::fileCollapsed(QListViewItem *item)
{
    if(item == 0)
        return;

    // Cast to a derived type.
    QvisListViewFileItem *fileItem = (QvisListViewFileItem *)item;

    if(!fileItem->isRoot())
    {
        // If the file is a database then we want to update the file selection.
        if(fileItem->isFile())
        {
            //
            // If the item is the root of the current open database then remove
            // it from the list of expanded files.
            //
            if(fileItem->timeState == -1)
            {
                // Remove the file from the expanded file list.
                RemoveExpandedFile(fileItem->file);

                // If we collapsed the open file then update the file selection.
                if(fileItem->file == fileServer->GetOpenFile())
                    UpdateFileSelection();
            }
        }
        else
            RemoveExpandedFile(fileItem->file);
    }
}

// ****************************************************************************
// Method: QvisFilePanel::fileExpanded
//
// Purpose: 
//   This is a Qt slot function that expands a file thus adding it to the
//   expanded file list.
//
// Arguments:
//   item : The item that is being expanded.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 1 13:24:05 PST 2002
//
// Modifications:
//   Eric Brugger, Mon Dec 16 13:05:01 PST 2002
//   I seperated the concepts of state and frame, since they are no longer
//   equivalent with keyframe support.
//   
// ****************************************************************************

void
QvisFilePanel::fileExpanded(QListViewItem *item)
{
    if(item == 0)
        return;

    // Cast to a derived type.
    QvisListViewFileItem *fileItem = (QvisListViewFileItem *)item;
    AddExpandedFile(fileItem->file);

    // If the file is a database then we want to update the file selection so
    // the current cycle will be selected.
    if(fileItem->file == fileServer->GetOpenFile() &&
       globalAtts->GetNStates() > 1)
    {
        UpdateFileSelection();
    }
}

// ****************************************************************************
// Method: QvisFilePanel::highlightFile
//
// Purpose: 
//   This is a Qt slot function that sets the enabled flag on the
//   open, replace, overlay buttons depending on whether or not the
//   highlighted file is the active file.
//
// Arguments:
//   item : The qualified filename that is hightlighted.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 31 10:56:13 PDT 2000
//
// Modifications:
//   Brad Whitlock, Wed Mar 21 00:40:21 PDT 2001
//   Changed the coding that determines if it is a file that is being
//   highlighted. Temporarily disabled the replace and overlay buttons.
//
//   Brad Whitlock, Thu Feb 28 12:26:05 PDT 2002
//   Changed the code so it is okay to highlight an item that is not a file.
//
//   Brad Whitlock, Wed Mar 6 16:22:03 PST 2002
//   Enabled the replace and overlay buttons.
//
//   Brad Whitlock, Mon Jul 29 14:33:43 PST 2002
//   I added the concept of re-opening a file.
//
//   Brad Whitlock, Fri Feb 28 08:39:35 PDT 2003
//   I made it so highlighting a file that's been opened in the past makes
//   the highlighted file the new open file.
//
//   Brad Whitlock, Fri Sep 5 17:10:43 PST 2003
//   I made it so replace can be used to set the active time state.
//
// ****************************************************************************

void
QvisFilePanel::highlightFile(QListViewItem *item)
{
    if(item == 0)
        return;

    // Cast to a derived type.
    QvisListViewFileItem *fileItem = (QvisListViewFileItem *)item;

    // If the filename is not a file. Disable the open files button.
    if(!fileItem->isFile())
    {
        openButton->setEnabled(false);
        return;
    }

    // If we've opened the file before, make the open button say "ReOpen".
    bool fileOpenedBefore = fileServer->HaveOpenedFile(fileItem->file);
    if(fileOpenedBefore)
        openButton->setText("ReOpen");
    else
        openButton->setText("Open");
    openButton->setEnabled(true);

    // If the highlighted file is not the active file, then
    // enable the open, replace, overlay buttons.
    bool enable = (fileServer->GetOpenFile() != fileItem->file);

    // If we've opened the file before, highlighting it should make it
    // the open file.
    if(fileOpenedBefore && fileItem->timeState == -1)
        OpenFile(fileItem->file, 0, false);

    replaceButton->setEnabled(enable || (fileItem->timeState >= 0));
    overlayButton->setEnabled(enable);
}

// ****************************************************************************
// Method: QvisFilePanel::openFile
//
// Purpose: 
//   This is a Qt slot function that opens a file when the Open button
//   is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 31 10:58:13 PDT 2000
//
// Modifications:
//   Brad Whitlock, Wed Mar 21 00:39:32 PDT 2001
//   Added a check to make sure it's a file.
//
//   Brad Whitlock, Fri Feb 28 08:29:31 PDT 2003
//   I passed the reopen flag to the OpenFile method.
//
//   Brad Whitlock, Thu May 15 12:44:42 PDT 2003
//   I added support for opening a file at a later time state.
//
// ****************************************************************************

void
QvisFilePanel::openFile()
{
    QvisListViewFileItem *fileItem = (QvisListViewFileItem *)
        fileListView->currentItem();

    if((fileItem != 0) && fileItem->isFile() && (!fileItem->file.Empty()))
    {
        // Try and open the file.
        bool reOpen = (fileItem->file == fileServer->GetOpenFile());
        int  timeState = 0;

        // We're reopening so we should reopen to the current time state.
        if(reOpen)
            timeState = globalAtts->GetCurrentState();
        else if(fileItem->timeState != -1)
        {
            // We're not reopening, we're opening. The file that we're opening
            // has a valid timestep so use that so we open the right timestep
            // to begin with.
            timeState = fileItem->timeState;
        }

        OpenFile(fileItem->file, timeState, reOpen);
    }
}

// ****************************************************************************
// Method: QvisFilePanel::openFileDblClick
//
// Purpose: 
//   This is a Qt slot function that opens a file when the filename
//   is double clicked.
//
// Arguments:
//   item : The file that was double clicked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 31 10:58:36 PDT 2000
//
// Modifications:
//   Brad Whitlock, Wed Mar 21 00:39:06 PDT 2001
//   Added a check to make sure it's a file.
//
//   Brad Whitlock, Wed Mar 6 15:31:07 PST 2002
//   Added code to set the animation time step if the item is a database
//   timestep.
//
//   Brad Whitlock, Fri Feb 28 08:32:51 PDT 2003
//   I made it so double clicking on a filename never causes it to be reopened.
//
//   Brad Whitlock, Thu May 15 12:39:09 PDT 2003
//   I made it so that we can open a file directly at a later time state.
//
// ****************************************************************************

void
QvisFilePanel::openFileDblClick(QListViewItem *item)
{
    QvisListViewFileItem *fileItem = (QvisListViewFileItem *)item;

    if((fileItem != 0) && fileItem->isFile() && (!fileItem->file.Empty()))
    {
        if(fileItem->timeState != -1 &&
           fileItem->file == fileServer->GetOpenFile())
        {
            // It must be a database timestep if the time state is not -1 and
            // the file item has the same filename as the open file.
            viewer->AnimationSetFrame(fileItem->timeState);
        }
        else
        {
            AddExpandedFile(fileItem->file);

            // Pick the right timestate to open.
            int timeState = (fileItem->timeState == -1) ? 0 : fileItem->timeState;

            // Try and open the file.
            OpenFile(fileItem->file, timeState, false);
        }
    }
}

// ****************************************************************************
// Method: QvisFilePanel::replaceFile
//
// Purpose: 
//   This is a Qt slot function that replaces a file when the Replace
//   button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 31 10:59:50 PDT 2000
//
// Modifications:
//   Brad Whitlock, Wed Mar 21 00:38:22 PDT 2001
//   Added a check to make sure it's a file.
//
//   Brad Whitlock, Fri Sep 5 16:52:28 PST 2003
//   I let replace change time states so MeshTV users are happy.
//
// ****************************************************************************

void
QvisFilePanel::replaceFile()
{
    QvisListViewFileItem *fileItem = (QvisListViewFileItem *)
        fileListView->currentItem();

    if((fileItem != 0) && fileItem->isFile() && (!fileItem->file.Empty()))
    {
        if(fileItem->file == fileServer->GetOpenFile())
        {
            // It must be a database timestep if the time state is not -1 and
            // the file item has the same filename as the open file.
            if(fileItem->timeState != -1)
                viewer->AnimationSetFrame(fileItem->timeState);
        }
        else
        {
            // Try and replace the file.
            ReplaceFile(fileItem->file.FullName());
        }
    }
}

// ****************************************************************************
// Method: QvisFilePanel::overlayFile
//
// Purpose: 
//   This is a Qt slot function that overlays a file when the Overlay
//   button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 31 11:00:46 PDT 2000
//
// Modifications:
//   Brad Whitlock, Wed Mar 21 00:38:22 PDT 2001
//   Added a check to make sure it's a file.
//
// ****************************************************************************

void
QvisFilePanel::overlayFile()
{
    QvisListViewFileItem *fileItem = (QvisListViewFileItem *)
        fileListView->currentItem();

    if((fileItem != 0) && fileItem->isFile() && (!fileItem->file.Empty()))
    {
        // Try and open the file.
        OverlayFile(fileItem->file.FullName());
    }
}

// ****************************************************************************
// Method: QvisFilePanel::sliderStart
//
// Purpose: 
//   This is a Qt slot function that is called when the animation
//   slider is pressed.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 26 17:14:35 PST 2000
//
// Modifications:
//   Eric Brugger, Mon Dec 16 13:05:01 PST 2002
//   I seperated the concepts of state and frame, since they are no longer
//   equivalent with keyframe support.
//   
// ****************************************************************************

void
QvisFilePanel::sliderStart()
{
    sliderDown = true;
    sliderVal = globalAtts->GetCurrentFrame();
}

// ****************************************************************************
// Method: QvisFilePanel::sliderMove
//
// Purpose: 
//   This is a Qt slot function that is called when the animation
//   slider is moved.
//
// Arguments:
//   val : The new value to use for the current state.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 26 17:14:35 PST 2000
//
// Modifications:
//   Brad Whitlock, Tue Aug 21 16:55:44 PST 2001
//   Added code to update the cycle time text field as the slider moves.
//
// ****************************************************************************

void
QvisFilePanel::sliderMove(int val)
{
    sliderVal = val;

    // Update the cycle/time text field.
    UpdateTimeFieldText(sliderVal);
}

// ****************************************************************************
// Method: QvisFilePanel::sliderEnd
//
// Purpose: 
//   This is a Qt slot function that is called when the slider is
//   released. It will change the current frame of the animation to
//   the last slider value.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 26 18:22:15 PST 2000
//
// Modifications:
//   Eric Brugger, Mon Dec 16 13:05:01 PST 2002
//   I seperated the concepts of state and frame, since they are no longer
//   equivalent with keyframe support.
//   
// ****************************************************************************

void
QvisFilePanel::sliderEnd()
{
    sliderDown = false;

    // Update the clients that need to know about the new frame
    globalAtts->SetCurrentFrame(sliderVal);
    SetUpdate(false);
    globalAtts->Notify();

    // Tell the viewer to set the animation frame.
    viewer->AnimationSetFrame(sliderVal);
}

// ****************************************************************************
// Method: QvisFilePanel::sliderChange
//
// Purpose: 
//   This is a Qt slot function that is called when the slider's
//   valueChanged signal is emitted.
//
// Arguments:
//   val : The new slider value.
//
// Note:       This method is called when clicking on areas of the slider to
//             make it page up/down.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 27 11:59:05 PDT 2000
//
// Modifications:
//   Eric Brugger, Mon Dec 16 13:05:01 PST 2002
//   I seperated the concepts of state and frame, since they are no longer
//   equivalent with keyframe support.
//   
// ****************************************************************************

void
QvisFilePanel::sliderChange(int val)
{
    if(sliderDown)
        return;

    globalAtts->SetCurrentFrame(val);
    SetUpdate(false);
    globalAtts->Notify();

    // Tell the viewer to set the animation frame.
    viewer->AnimationSetFrame(val);
}

// ****************************************************************************
// Method: QvisFilePanel::processTimeText
//
// Purpose: 
//   This is a Qt slot function that processes the text entered by
//   the user and looks for the closest cycle number and sets the 
//   animation to that cycle number.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 26 17:16:01 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilePanel::processTimeText()
{
    // Convert the text to an integer value.
    QString temp(timeField->text().stripWhiteSpace());
    if(temp.isEmpty())
        return;
    bool okay = false;
    int cycle = temp.toInt(&okay);
    if(!okay)
    {
        timeField->setText("");
        return;
    }

    // Loop through the cycles for the current file while the
    // cycle that was entered is greater than or equal to the
    // cycle in the list.
    const avtDatabaseMetaData *md = fileServer->GetMetaData();
    int index = 0;
    for(int i = 0; i < md->GetNumStates(); ++i)
    {
        if(cycle <= md->GetCycles()[i])
            break;
        else
            ++index;
    }
    // Make sure that index is no larger than numstates.
    if(index >= md->GetNumStates())
        index = md->GetNumStates() - 1;

    // Call the slider's slot function.
    sliderVal = index;
    sliderEnd();
}

// ****************************************************************************
// Method: QvisFilePanel::updateHeaderWidth
//
// Purpose: 
//   This is a Qt flot function that updates the width of the list view that
//   shows the contents of the selected files list.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 19 13:37:20 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilePanel::updateHeaderWidth()
{
    fileListView->setColumnWidth(0, fileListView->visibleWidth());
}

///////////////////////////////////////////////////////////////////////////////
/// INTERNAL CLASS DEFINITIONS
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: FileTree::FileTree
//
// Purpose: 
//   Constructor for the FileTree class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 16:49:19 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed May 14 15:29:54 PST 2003
//   Added a pointer to the parent QvisFilePanel object.
//
// ****************************************************************************

FileTree::FileTree(QvisFilePanel *fp)
{
    // Keep a pointer to the file panel that created this object so we can
    // add virtual files to the list of expanded files.
    filePanel = fp;

    root = new FileTreeNode(FileTreeNode::ROOT_NODE);
    root->nodeName = std::string("root");
}

// ****************************************************************************
// Method: FileTree::~FileTree
//
// Purpose: 
//   Destructor for the FileTree class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 16:49:42 PST 2001
//
// Modifications:
//   
// ****************************************************************************

FileTree::~FileTree()
{
    delete root;
}

// ****************************************************************************
// Method: FileTree::Add
//
// Purpose: 
//   Adds the specified file to the file tree. This includes building the path
//   nodes in the file tree that need to be built.
//
// Arguments:
//   fileName : The filename to add to the file tree.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 16:50:02 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Apr 26 12:01:52 PDT 2002
//   Ported to windows.
//
//   Brad Whitlock, Mon Aug 26 16:27:33 PST 2002
//   I added code to handle different file separators.
//
//   Brad Whitlock, Mon Mar 31 15:10:48 PST 2003
//   I added code to handle virtual files.
//
//   Brad Whitlock, Mon Sep 15 13:50:26 PST 2003
//   I fixed the method so path nodes have the correct filename instead
//   of an empty filename. This ensures that they get expanded correctly
//   later on.
//
// ****************************************************************************

void
FileTree::Add(const QualifiedFilename &fileName, char separator_)
{
    static const int SCAN_MODE = 0;
    static const int PROCESS_MODE = 1;

    int len = fileName.path.length();
    int index = 1;
    int mode = SCAN_MODE;

    separator = separator_;
    std::string directory;
    std::string keyPath;
    FileTreeNode *current = root;
    QualifiedFilename empty;

    // Loop through the directory string either adding directories to the
    // tree or advancing the current pointer to the final directory.
    while(index < len)
    {
        if(mode == SCAN_MODE)
        {
            keyPath += fileName.path[index];
            if(fileName.path[index] == separator)
                mode = PROCESS_MODE;
            else
            {
                directory += fileName.path[index];
                if(index == len - 1)
                    mode = PROCESS_MODE;
                else 
                    ++index;
            }
        }
        else if(mode == PROCESS_MODE)
        {
            // Look for the directory in at the current level.
            FileTreeNode *dir = current->Find(directory);

            if(dir != NULL)
            {
                // Advance the current pointer down the directory structure.
                current = dir;
            }
            else
            {
                // Add the host and path but not the filename.
                QualifiedFilename key(fileName); key.path = keyPath; key.filename = "(path)";

                // The path was not found. We need to add it.
                FileTreeNode *newNode = current->Add(FileTreeNode::PATH_NODE,
                                                     directory, key,
                                                     separator);

                // If we were able to add the node, move to it.
                current = newNode;
            }
            ++index;
            directory="";
            mode = SCAN_MODE;
        }
    }

    // Add the file to the current node.
    int t = fileName.IsVirtual() ? FileTreeNode::DATABASE_NODE :
        FileTreeNode::FILE_NODE;
    current->Add(t, fileName.filename, fileName, separator);
}

// ****************************************************************************
// Method: FileTree::Reduce
//
// Purpose: 
//   Simplifies the file tree. This means condensing nodes with only one child
//   into its parent node.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 16:51:04 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Mar 29 13:01:48 PST 2001
//   Modified the file reduction rules so they work with a single file.
//
//   Brad Whitlock, Fri Apr 27 13:10:27 PST 2001
//   Fixed a memory problem.
//
//   Brad Whitlock, Fri Apr 26 12:03:08 PDT 2002
//   Ported to windows.
//
//   Brad Whitlock, Mon Aug 26 16:47:24 PST 2002
//   I changed the code so it can use different separators.
//
// ****************************************************************************

void
FileTree::Reduce()
{
    for(int i = 0; i < root->numChildren;)
    {
        bool retval = root->children[i]->Reduce();

        // If no reduction done, move to the next child.
        if(!retval)
           ++i;
    }

    // Try and reduce the root to see if we can get rid of a single top-level
    // directory. Do not reduce the root if it only has one child in it since
    // it has to be a file.
    if(root->Size() > 2)
        root->Reduce();
    else if(root->Size() == 1)
    {
        // The root has only one child and it must be a file. Since it has
        // been reduced, strip the path from it so it will match what happens
        // when there are multiple top level files.
        if(root->numChildren > 0)
        {
            std::string &s = root->children[0]->nodeName;

            int pos = s.rfind(separator_str()) + 1;
            s = s.substr(pos, s.length() - pos);
        }
    }
}

// ****************************************************************************
// Method: FileTree::Size
//
// Purpose: 
//   Returns the number of nodes in the file tree. This includes the root node.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 16:51:59 PST 2001
//
// Modifications:
//   
// ****************************************************************************

int
FileTree::Size() const
{
    return root->Size();
}

// ****************************************************************************
// Method: FileTree::TreeContainsDirectories
//
// Purpose: 
//   Searches the file tree to determine if it contains directories.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 16:52:22 PST 2001
//
// Modifications:
//   
// ****************************************************************************

bool
FileTree::TreeContainsDirectories() const
{
    bool retval = false;

    for(int i = 0; i < root->numChildren && !retval; ++i)
    {
        retval |= root->children[i]->HasChildrenOfType(FileTreeNode::PATH_NODE);
    }

    return retval;
}

// ****************************************************************************
// Method: FileTree::AddElementsToListViewItem
//
// Purpose: 
//   This method adds adds the appropriate list view items for the file tree
//   to the parent list view item.
//
// Arguments:
//   item           : The list view item that will be the parent of the new nodes.
//   fileIndex      : The index of the file being added.
//   folderPixmap   : An icon of a folder.
//   databasePixmap : An icon of a database.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 16:52:52 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Mar 31 15:12:01 PST 2003
//   I added databasePixmap.
//
//   Brad Whitlock, Wed May 14 15:31:14 PST 2003
//   I added filePanel.
//
// ****************************************************************************

void
FileTree::AddElementsToListViewItem(QListViewItem *item, int &fileIndex,
    const QPixmap &folderPixmap, const QPixmap &databasePixmap)
{
    root->AddElementsToListViewItem(item, fileIndex, false, folderPixmap,
                                    databasePixmap, filePanel);
}

// ****************************************************************************
// Method: FileTree::AddElementsToListView
//
// Purpose: 
//   This method adds adds the appropriate list view items for the file tree
//   to the parent list view.
//
// Arguments:
//   item : The list view that will be the parent of the new nodes.
//   fileIndex : The index of the file being added.
// 
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 16:52:52 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed May 14 15:31:21 PST 2003
//   I added filePanel.
//
// ****************************************************************************

void
FileTree::AddElementsToListView(QListView *listview, int &fileIndex,
    const QPixmap &folderPixmap, const QPixmap &databasePixmap)
{
    root->AddElementsToListView(listview, fileIndex, folderPixmap,
                                databasePixmap, filePanel);
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: FileTree::FileTreeNode::FileTreeNode
//
// Purpose: 
//   This is the constructor for the FileTree::FileTreeNode class.
//
// Arguments:
//   nodeT : The type of node we're instantiating.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 16:55:21 PST 2001
//
// Modifications:
//   
// ****************************************************************************

FileTree::FileTreeNode::FileTreeNode(int nodeT) : nodeName()
{
    nodeType = nodeT;
    numChildren = 0;
    children = NULL;
}

// ****************************************************************************
// Method: FileTree::FileTreeNode::~FileTreeNode
//
// Purpose: 
//   Destructor for the FileTree::FileTreeNode class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 16:56:05 PST 2001
//
// Modifications:
//   
// ****************************************************************************

FileTree::FileTreeNode::~FileTreeNode()
{
    Destroy();
}

// ****************************************************************************
// Method: FileTree::FileTreeNode::Destroy
//
// Purpose: 
//   Recursively (indirect) destroys all child nodes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 16:56:32 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
FileTree::FileTreeNode::Destroy()
{
    if(nodeType == ROOT_NODE || nodeType == PATH_NODE)
    {
        for(int i = 0; i < numChildren; ++i)
            delete children[i];

        delete [] children;
        children = NULL;
        numChildren = 0;
    }
}

// ****************************************************************************
// Method: FileTree::FileTreeNode::Add
//
// Purpose: 
//   Adds a new node of the specified type to the current node.
//
// Arguments:
//   nType    : The type of the node being added.
//   name     : The name of the node being added.
//   fileName : The filename associated with the new node.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 16:57:30 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Aug 26 16:43:46 PST 2002
//   I added the ability to have different kinds of separators.
//
// ****************************************************************************

FileTree::FileTreeNode *
FileTree::FileTreeNode::Add(int nType, const std::string &name,
    const QualifiedFilename &fileName, char separator_)
{
    FileTreeNode *retval = NULL;
    separator = separator_;

    if(nodeType == ROOT_NODE || nodeType == PATH_NODE)
    {
        if(numChildren < 1)
        {
            // Append it.
            children = new FileTreeNode*[1];
            children[0] = new FileTreeNode(nType);
            children[0]->nodeName = name;
            children[0]->fileName = fileName;
            numChildren = 1;

            retval = children[0];
        }
        else
        {
            // Add it in order.
            FileTreeNode **newChildren = new FileTreeNode*[numChildren + 1];

            // Create the new item.
            FileTreeNode *newNode = new FileTreeNode(nType);
            newNode->nodeName = name;
            newNode->fileName = fileName;
            retval = newNode;

            // Copy the child arrays.
            bool notInserted = true;
            int index = 0;
            for(int i = 0; i < numChildren; ++i, ++index)
            {
                if(notInserted && (children[i]->nodeName > newNode->nodeName))
                {
                    newChildren[index++] = newNode;
                    newChildren[index] = children[i];
                    notInserted = false;
                }
                else
                    newChildren[index] = children[i];
            }
            if(notInserted)
               newChildren[numChildren] = newNode;

            // Increase the number of children by one and delete the old array.
            ++numChildren;
            delete [] children;
            children = newChildren;
        }
    }

    return retval;
}

// ****************************************************************************
// Method: FileTree::FileTreeNode::Find
//
// Purpose: 
//   Searches for the node with the given path name.
//
// Arguments:
//   path : The name of the node to look for.
//
// Returns:    A pointer to the node or NULL if it cannot be found.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 16:58:52 PST 2001
//
// Modifications:
//   
// ****************************************************************************

FileTree::FileTreeNode *
FileTree::FileTreeNode::Find(const std::string &path)
{
    FileTree::FileTreeNode *retval = NULL;

    for(int i = 0; i < numChildren; ++i)
    {
        if(children[i]->nodeName == path &&
           children[i]->nodeType == PATH_NODE)
        {
            retval = children[i];
            break;
        }
    }

    return retval;
}

// ****************************************************************************
// Method: FileTree::FileTreeNode::Reduce
//
// Purpose: 
//   Simplifies the file tree.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 16:59:51 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue Aug 21 14:34:41 PST 2001
//   Added a new rule to prepend a slash onto reduced nodes that do not
//   already have a slash as the first character.
//
//   Brad Whitlock, Fri Apr 26 12:02:14 PDT 2002
//   Ported to windows.
//
//   Brad Whitlock, Mon Aug 26 16:41:34 PST 2002
//   I made it support different kinds of separators.
//
//   Brad Whitlock, Mon Mar 31 15:13:54 PST 2003
//   I added support for database nodes.
//
// ****************************************************************************

bool
FileTree::FileTreeNode::Reduce()
{
    bool retval = false;

    if(numChildren == 1)
    {
        // Suck the child node into the current node.
        nodeName += separator_str();
        nodeName += children[0]->nodeName;
        nodeType = children[0]->nodeType;
        fileName = children[0]->fileName;

        // If the reduced node does not have a separator as the first
        // character of its nodeName, add a separator.
        if((nodeType == FILE_NODE || nodeType == DATABASE_NODE) &&
           nodeName[0] != separator)
        {
            std::string slash(separator_str());
            slash += nodeName;
            nodeName = slash;
        }

        FileTreeNode *oldChild = children[0];
        delete [] children;
        children = oldChild->children;
        numChildren = oldChild->numChildren;

        // Delete the old child.
        oldChild->children = NULL;
        oldChild->numChildren = 0;
        delete oldChild;

        retval = true;
    }
    else
    {
        for(int i = 0; i < numChildren;)
        {
            retval = children[i]->Reduce();

            // If no reduction done, move to the next child.
            if(!retval)
               ++i;
        }
    }

    return retval;
}

// ****************************************************************************
// Method: FileTree::FileTreeNode::Size
//
// Purpose: 
//   Returns the number of children in the node.
//
// Returns:    The number of children in the node.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 17:00:21 PST 2001
//
// Modifications:
//   
// ****************************************************************************

int
FileTree::FileTreeNode::Size() const
{
    int retval = 1;

    if(numChildren > 0)
    {
        for(int i = 0; i < numChildren; ++i)
            retval += children[i]->Size();
    }

    return retval;
}

// ****************************************************************************
// Method: FileTree::FileTreeNode::HasChildrenOfType
//
// Purpose: 
//   Recursively determines whether the node has any children of the
//   specified type.
//
// Arguments:
//   type : The type we're looking for.
//
// Returns:    true if the type was found, else returns false.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 17:00:48 PST 2001
//
// Modifications:
//   
// ****************************************************************************

bool
FileTree::FileTreeNode::HasChildrenOfType(int type)
{
    bool retval = (nodeType == type);

    if(!retval && numChildren > 0)
    {
        // Look through the children and see if any of them have the
        // specified type.
        for(int i = 0; i < numChildren && retval; ++i)
        {
            retval &= children[i]->HasChildrenOfType(type);
        }
    }

    return retval;
}

// ****************************************************************************
// Method: FileTree::FileTreeNode::AddElementsToListViewItem
//
// Purpose: 
//   Creates the widgets used to represent the file tree.
//
// Arguments:
//   item         : The list view item that will act as a parent for the
//                  new items.
//   fileIndex    : A variable used to determine the file index to use.
//   addRoot      : Whether or not to add a root node that contains multiple
//                  child nodes.
//   folderPixmap : A reference to the pixmap used for file folders.
//   databasePixmap : A reference to the pixmap used for databases.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 17:02:12 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Apr 26 12:02:44 PDT 2002
//   Ported to windows.
//
//   Brad Whitlock, Mon Aug 26 16:40:49 PST 2002
//   I made it support different kinds of file separators.
//
//   Brad Whitlock, Mon Mar 31 15:14:29 PST 2003
//   I added support for virtual files.
//
//   Brad Whitlock, Wed May 14 15:31:57 PST 2003
//   I made virtual files be expanded by default.
//
// ****************************************************************************

void
FileTree::FileTreeNode::AddElementsToListViewItem(QListViewItem *item,
    int &fileIndex, bool addRoot, const QPixmap &folderPixmap,
    const QPixmap &databasePixmap, QvisFilePanel *filePanel)
{
    QListViewItem *root = item;

    // Add the current node.
    if(addRoot)
    {
        if(nodeType == FILE_NODE)
        {
            // Add a file node.
            new QvisListViewFileItem(item, NumberedFilename(fileIndex),
                    fileName, QvisListViewFileItem::FILE_NODE);
            ++fileIndex;
        }
        else if(nodeType == DATABASE_NODE)
        {
            // Add a file node.
            QvisListViewFileItem *node = new QvisListViewFileItem(item,
                NumberedFilename(fileIndex), fileName,
                QvisListViewFileItem::FILE_NODE);
            node->setPixmap(0, databasePixmap);
            ++fileIndex;

            // If the file is a virtual file, make it be expanded by default.
            if(fileName.IsVirtual())
            {
                if(!filePanel->HaveFileInformation(fileName))
                    filePanel->AddExpandedFile(fileName);
            }
        }
        else if(nodeType == PATH_NODE)
        {
            // Add a directory node.
            QString temp;
            temp.sprintf("%c%s", separator, nodeName.c_str());
            root = new QvisListViewFileItem(item, temp, fileName,
                           QvisListViewFileItem::DIRECTORY_NODE);
            root->setPixmap(0, folderPixmap);
        }
    }

    // Add any children there might be.
    if(numChildren > 0)
    {
        for(int i = 0; i < numChildren; ++i)
        {
            children[i]->AddElementsToListViewItem(root, fileIndex, true,
                                                   folderPixmap, databasePixmap,
                                                   filePanel);
        }
    }        
}

// ****************************************************************************
// Method: FileTree::FileTreeNode::AddElementsToListView
//
// Purpose: 
//   Creates the widgets used to represent the file tree.
//
// Arguments:
//   listView     : The list view that will act as a parent for the
//                  new items.
//   fileIndex    : A variable used to determine the file index to use.
//   addRoot      : Whether or not to add a root node that contains multiple
//                  child nodes.
//   folderPixmap : A reference to the pixmap used for file folders.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 17:02:12 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Mar 31 15:44:18 PST 2003
//   I added folder and database pixmaps.
//
//   Brad Whitlock, Wed May 14 15:35:32 PST 2003
//   I made virtual files come expanded by default.
//
// ****************************************************************************

void
FileTree::FileTreeNode::AddElementsToListView(QListView *listview,
    int &fileIndex, const QPixmap &folderPixmap, const QPixmap &databasePixmap,
    QvisFilePanel *filePanel)
{
    // Add any children there might be.
    for(int i = 0; i < numChildren; ++i)
    {
        if(children[i]->nodeType == PATH_NODE)
        {
            QvisListViewFileItem *item = new QvisListViewFileItem(listview,
                    children[i]->NumberedFilename(fileIndex),
                    children[i]->fileName,
                    QvisListViewFileItem::DIRECTORY_NODE);
            item->setPixmap(0, folderPixmap);
        }
        else if(children[i]->nodeType == DATABASE_NODE)
        {
            QvisListViewFileItem *item = new QvisListViewFileItem(listview,
                    children[i]->NumberedFilename(fileIndex),
                    children[i]->fileName,
                    QvisListViewFileItem::FILE_NODE);
            item->setPixmap(0, databasePixmap);

            // If the file is a virtual file, make it be expanded by default.
            if(children[i]->fileName.IsVirtual())
            {
                if(!filePanel->HaveFileInformation(children[i]->fileName))
                    filePanel->AddExpandedFile(children[i]->fileName);
            }
        }
        else
        {
            new QvisListViewFileItem(listview,
                    children[i]->NumberedFilename(fileIndex),
                    children[i]->fileName,
                    QvisListViewFileItem::FILE_NODE);
        }

        ++fileIndex;
    }
}

// ****************************************************************************
// Method: FileTree::FileTreeNode::NumberedFilename
//
// Purpose: 
//   Creates a string that contains the file index and the filename.
//
// Arguments:
//   fileIndex : The file index to use in creating the string.
//
// Returns:    The new string that contains both the file index and filename.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 17:04:48 PST 2001
//
// Modifications:
//   
// ****************************************************************************

QString
FileTree::FileTreeNode::NumberedFilename(int fileIndex) const
{
    QString label;
    if(fileIndex < 10)
        label.sprintf("  %d: %s", fileIndex, nodeName.c_str());
    else if(fileIndex < 100)
        label.sprintf(" %d: %s", fileIndex, nodeName.c_str());
    else
        label.sprintf("%d: %s", fileIndex, nodeName.c_str());

    return label;
}
