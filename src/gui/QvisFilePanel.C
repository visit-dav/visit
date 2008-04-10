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

#include <visitstream.h>

#include <qcombobox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlistview.h> 
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlineedit.h> 
#include <qtimer.h>

#include <QvisFilePanel.h>
#include <QvisAnimationSlider.h>
#include <QvisVCRControl.h>
#include <QvisListViewFileItem.h>

#include <DatabaseCorrelation.h>
#include <DatabaseCorrelationList.h>
#include <DebugStream.h>
#include <FileServerList.h>
#include <GlobalAttributes.h>
#include <KeyframeAttributes.h>
#include <NameSimplifier.h>
#include <Plot.h>
#include <PlotList.h>
#include <WindowInformation.h>
#include <Utility.h>
#include <ViewerProxy.h>
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
//   Brad Whitlock, Thu Aug 5 17:36:02 PST 2004
//   I added methods to look for long node names.
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
        bool HasNodeNameExceeding(int len) const;
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

        void Print(ostream &os, const std::string &indent) const;

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
    bool HasNodeNameExceeding(int len) const;

    void AddElementsToListViewItem(QListViewItem *item, int &fileIndex,
                                   const QPixmap &folderPixmap,
                                   const QPixmap &dbPixmap);
    void AddElementsToListView(QListView *item, int &fileIndex,
                               const QPixmap &folderPixmap,
                               const QPixmap &databasePixmap);

    friend ostream &operator << (ostream &os, const FileTree &t);
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
//   Brad Whitlock, Mon Oct 13 15:24:56 PST 2003
//   Initialized timeStateFormat.
//
//   Brad Whitlock, Tue Dec 30 14:32:32 PST 2003
//   I made it use QvisAnimationSlider.
//
//   Brad Whitlock, Tue Jan 27 18:14:38 PST 2004
//   I renamed some slots and added the active time slider.
//
//   Brad Whitlock, Tue Apr 6 14:07:34 PST 2004
//   I added allowFileSelectionChange.
//
//   Brad Whitlock, Tue Apr 27 12:11:29 PDT 2004
//   I improved the spacing for newer versions of Qt.
//
//   Jeremy Meredith, Wed Oct 13 20:38:30 PDT 2004
//   Prevent the header of the file list from being clicked.  Leaving it
//   enabled allows changing of the sort and in some circumstances broke
//   time sequences.  ('5391)
//
// ****************************************************************************

QvisFilePanel::QvisFilePanel(QWidget *parent, const char *name) :
   QWidget(parent, name), SimpleObserver(), GUIBase(), displayInfo(),
   timeStateFormat()
{
    showSelectedFiles = true;
    allowFileSelectionChange = true;

    // Create the top layout that will contain the widgets.
    QVBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->setSpacing(5);
    
    // Create the selected file list.
    fileListView = new QListView(this, "fileList");
    fileListView->header()->setClickEnabled(false);
    fileListView->addColumn(tr("Selected files"), 1);
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

    // Create the active time slider.
    QHBoxLayout *tsLayout = new QHBoxLayout(topLayout);
    activeTimeSlider = new QComboBox(this, "activeTimeSlider");
    connect(activeTimeSlider, SIGNAL(activated(int)),
            this, SLOT(changeActiveTimeSlider(int)));
    activeTimeSliderLabel = new QLabel(activeTimeSlider,
        tr("Active time slider"), this, "activeTimeSliderLabel");
    tsLayout->addWidget(activeTimeSliderLabel);
    activeTimeSliderLabel->hide();
    tsLayout->addWidget(activeTimeSlider, 10);
    activeTimeSlider->hide();

    // Create the animation position slider bar
    QHBoxLayout *animationLayout = new QHBoxLayout(topLayout);
    topLayout->setStretchFactor(animationLayout, 10);
    animationPosition = new QvisAnimationSlider(Qt::Horizontal, this, "animationPosition");
    animationPosition->setEnabled(false);
    connect(animationPosition, SIGNAL(sliderPressed()),
            this, SLOT(sliderStart()));
    connect(animationPosition, SIGNAL(sliderMoved(int)),
            this, SLOT(sliderMove(int)));
    connect(animationPosition, SIGNAL(sliderWasReleased()),
            this, SLOT(sliderEnd()));
    connect(animationPosition, SIGNAL(sliderValueChanged(int)),
            this, SLOT(sliderChange(int)));
#if QT_VERSION >= 0x030000
    animationLayout->addWidget(animationPosition, 25);
#else
    animationLayout->addWidget(animationPosition, 1000);
#endif
    // Create the animation time field.
    timeField = new QLineEdit(this, "timeField");
    timeField->setEnabled(false);
    connect(timeField, SIGNAL(returnPressed()), this, SLOT(processTimeText()));
    animationLayout->addWidget(timeField, 5);

    // Create the VCR controls.
    vcrControls = new QvisVCRControl(this, "vcr" );
    vcrControls->setEnabled(false);
    connect(vcrControls, SIGNAL(prevFrame()), this, SLOT(backwardStep()));
    connect(vcrControls, SIGNAL(reversePlay()), this, SLOT(reversePlay()));
    connect(vcrControls, SIGNAL(stop()), this, SLOT(stop()));
    connect(vcrControls, SIGNAL(play()), this, SLOT(play()));
    connect(vcrControls, SIGNAL(nextFrame()), this, SLOT(forwardStep()));
    topLayout->addWidget(vcrControls);

    // Create the computer pixmap.
    computerPixmap = new QPixmap(computer_xpm);
    // Create the database pixmap.
    databasePixmap = new QPixmap(database_xpm);
    // Create the folder pixmap and add it to the pixmap cache.
    folderPixmap = new QPixmap(folder_xpm);

    // Initialize the attached subjects
    windowInfo = NULL;
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
//   Brad Whitlock, Sun Jan 25 01:10:56 PDT 2004
//   I made it use windowInfo instead of globalAtts.
//
// ****************************************************************************

QvisFilePanel::~QvisFilePanel()
{
    if(fileServer)
        fileServer->Detach(this);

    if(windowInfo)
        windowInfo->Detach(this);

    // Delete the pixmaps.
    delete computerPixmap;
    delete databasePixmap;
    delete folderPixmap;
}

// ****************************************************************************
// Method: QvisFilePanel::SetTimeStateFormat
//
// Purpose: 
//   Sets the display mode for the file panel. We can make it display files
//   using cycle information or we can make it use time information.
//
// Arguments: 
//   m : The new timestate display mode.
//
// Notes:      This method resets the text on the expanded databases, which
//             are open databases with more than one time state, so that
//             they show time using the new timestate display mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 13 16:15:46 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Dec 29 10:13:15 PDT 2003
//   I added code to help determine how labels are displayed if a database is
//   virtual.
//
//   Brad Whitlock, Sun Jan 25 01:28:55 PDT 2004
//   I added support for multiple time sliders.
//
//   Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//   Changed interface to FileServerList::GetMetaData
//
//   Mark C. Miller, Fri Aug 10 23:11:55 PDT 2007
//   Propogated knowledge that item was updated with metadata that was forced
//   to get accurate cycles/times.
//
// ****************************************************************************

void
QvisFilePanel::SetTimeStateFormat(const TimeFormat &m)
{
    if(m != timeStateFormat)
    {
        timeStateFormat = m;

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
                if(HaveFileInformation(item->file))
                {
                    // See if the file is a database
                    const avtDatabaseMetaData *md =
                        fileServer->GetMetaData(item->file,
                                                GetStateForSource(item->file),
                                                FileServerList::ANY_STATE,
                                               !FileServerList::GET_NEW_MD);
                    if(md != 0 && md->GetNumStates() > 1)
                    {
                        int j, maxts = QMIN(md->GetNumStates(), item->childCount());
                        QListViewItemIterator it(item); ++it;
                        bool useVirtualDBInfo = DisplayVirtualDBInformation(item->file);

                        // Set the label so that it shows the right values.
                        for(j = 0; j < maxts; ++j, ++it)
                        {
                             it.current()->setText(0, CreateItemLabel(md, j,
                                 useVirtualDBInfo));
                        }
			item->timeStateHasBeenForced =
			    fileServer->GetForceReadAllCyclesTimes();
                    }
                }
            }
        }

        // Delete the temporary array.
        delete [] items;

        //
        // Update the time text field using the current time slider.
        //
        int activeTS = windowInfo->GetActiveTimeSlider();
        int currentState = 0;
        if(activeTS >= 0)
            currentState = windowInfo->GetTimeSliderCurrentStates()[activeTS];
        UpdateTimeFieldText(currentState);
    }
}

// ****************************************************************************
// Method: QvisFilePanel::GetTimeStateFormat
//
// Purpose: 
//   Returns the time state format.
//
// Returns:    The time state format.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 13 17:19:20 PST 2003
//
// Modifications:
//   
// ****************************************************************************

const TimeFormat &
QvisFilePanel::GetTimeStateFormat() const
{
    return timeStateFormat;
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
//   Brad Whitlock, Sun Jan 25 01:28:23 PDT 2004
//   I made it use windowInfo instead of globalAtts.
//
// ****************************************************************************

void
QvisFilePanel::Update(Subject *TheChangedSubject)
{
    if(fileServer == 0 || windowInfo == 0)
        return;

    if(TheChangedSubject == fileServer)
        UpdateFileList(false);
    else if(TheChangedSubject == windowInfo)
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
//   Brad Whitlock, Sun Jan 25 01:29:46 PDT 2004
//   I moved the guts into RepopulateFileList.
//
// ****************************************************************************

void
QvisFilePanel::UpdateFileList(bool doAll)
{
    if(fileServer == 0 || windowInfo == 0)
        return;

    // If the appliedFileList has changed, update the appliedFile list.
    if(fileServer->AppliedFileListChanged() || doAll)
    {
        if(showSelectedFiles)
            RepopulateFileList();
    }
    else if(fileServer->FileChanged())
    {
        if(showSelectedFiles)
        {
            // Expand any databases that we know about. This just means that we add
            // the extra information that databases have, we don't expand the tree
            // until we enter UpdateFileSelection.
            ExpandDatabases();

            // Highlight the selected file.
            UpdateFileSelection();
        }

        // Set the enabled state for the animation controls.
        UpdateAnimationControlsEnabledState();
    }
}

// ****************************************************************************
// Method: QvisFilePanel::RepopulateFileList
//
// Purpose: 
//   Clears out the selected files and refills the list with the new list
//   of selected files.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 30 11:54:15 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Jul 28 17:22:59 PST 2004
//   Added code to force file lists that have MT databases always have a root
//   so they can be collapsed.
//
//   Brad Whitlock, Thu Aug 5 17:36:33 PST 2004
//   I added code to detect when node names are long and set the horizontal
//   scrollbar mode of the listview appropriately.
//
//   Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//   Changed interface to FileServerList::GetMetaData
// ****************************************************************************

void
QvisFilePanel::RepopulateFileList()
{
    const QualifiedFilenameVector &f = fileServer->GetAppliedFileList();
    // Holds pointers to the list item for each host.
    std::map<std::string, QListViewItem *> hostMap;

    // Go through all of the files and build a list of unique hosts.
    bool hostOtherThanLocalHost = false;
    bool someFilesHaveMultipleTimeStates = false;
    QualifiedFilenameVector::const_iterator pos;
    for(pos = f.begin(); pos != f.end(); ++pos)
    {
        // Populate the hostMap variable.
        if(pos->host.size() > 0)
        {
            // Add an entry for the host.
            hostMap[pos->host] = 0;

            // See if the host is not localhost.
            if(pos->host != std::string("localhost"))
                hostOtherThanLocalHost = true;
        }

        // See if the current file is MT
        if(!someFilesHaveMultipleTimeStates)
        {
            if(pos->IsVirtual())
            {
                someFilesHaveMultipleTimeStates = true;
            }
            else if(fileServer->HaveOpenedFile(*pos))
            {
                const avtDatabaseMetaData *md =
                    fileServer->GetMetaData(*pos,
                                    GetStateForSource(*pos),
                                    FileServerList::ANY_STATE,
                                   !FileServerList::GET_NEW_MD);
                if(md != 0 && md->GetNumStates() > 1)
                    someFilesHaveMultipleTimeStates = true;
            }
        }
    }

    // Clear out the fileListView widget.
    fileListView->clear();

    // Reset the node numbers that will be used to create the nodes.
    QvisListViewFileItem::resetNodeNumber();

    // Assume that we want automatic scrollbars instead of always having them.
    QScrollView::ScrollBarMode hScrollMode = QScrollView::Auto;

    // If there are multiple hosts or just one and it is not localhost,
    // add nodes in the file tree for the host. This makes sure that it
    // is always clear which host a file came from.
    if(hostMap.size() > 1)
    {
        std::map<std::string, QListViewItem *>::iterator hpos;

        // Create the root for the host list.
        QualifiedFilename rootName("Hosts", "(root)", "(root)");
        QListViewItem *root = new QvisListViewFileItem(fileListView, 
            QString(tr("Hosts")),
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

            // If there are any nodes with long names then we should have
            // scrollbars in the selected files list.
            if(files.HasNodeNameExceeding(30))
                hScrollMode = QScrollView::AlwaysOn;

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

        // debug1 << "File Tree:\n" << files << endl << endl;

        // If there are any nodes with long names then we should have
        // scrollbars in the selected files list.
        if(files.HasNodeNameExceeding(30))
            hScrollMode = QScrollView::AlwaysOn;

        // If there are top level directories in the file tree, then we
        // need to create a node for the host.
        if(files.TreeContainsDirectories() ||
           hostOtherThanLocalHost ||
           someFilesHaveMultipleTimeStates)
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

    //
    // Set the list view's scrollbar mode.
    //
    bool hScrollModeChanged = (hScrollMode != fileListView->hScrollBarMode());
    if(hScrollModeChanged)
        fileListView->setHScrollBarMode(hScrollMode);

    // Expand any databases that we know about. This just means that we add
    // the extra information that databases have, we don't expand the tree
    // until we enter UpdateFileSelection.
    ExpandDatabases();

    // Highlight the selected file.
    UpdateFileSelection();

    // Set the width of the zeroeth column.
    if(hScrollMode == QScrollView::AlwaysOn)
        QTimer::singleShot(100, this, SLOT(updateHeaderWidthForLongName()));
    else
        QTimer::singleShot(100, this, SLOT(updateHeaderWidth()));

    // If the scroll mode changed, force the list view to update.
    if(hScrollModeChanged)
        QTimer::singleShot(110, fileListView, SLOT(triggerUpdate()));
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
//   Brad Whitlock, Mon Dec 8 15:39:56 PST 2003
//   I changed the code so the file server's open file is never changed unless
//   it is changed to the same file that is open. The purpose of that is to
//   expand databases that we previously displayed as a single file (like
//   .visit files). This also allows the selected files list to show the active
//   time state for the database.
//
//   Brad Whitlock, Tue Dec 30 14:31:03 PST 2003
//   I made it use the animation slider instead of sliderDown.
//
//   Brad Whitlock, Sat Jan 24 22:53:05 PST 2004
//   I made it use the new time and file scheme. I also added code to set
//   the values in the new activeTimeSlider combobox.
//
//   Brad Whitlock, Fri Mar 18 13:39:02 PST 2005
//   I improved how the time slider names are shortened so all of the sources
//   are taken into account, which can make it easier to distinguish between
//   time sliders when there are multiple time sliders whose names only
//   differ by the path to their database.
//
//   Brad Whitlock, Mon Dec 17 11:02:28 PST 2007
//   Made it use ids.
//
// ****************************************************************************

void
QvisFilePanel::UpdateAnimationControls(bool doAll)
{
    if(fileServer == 0 || windowInfo == 0 ||
       (animationPosition != 0 && animationPosition->sliderIsDown()))
        return;

    //
    // Try and find a correlation for the active time slider.
    //
    int activeTS = windowInfo->GetActiveTimeSlider();

    // activeSource changed.  Update the file server.
    if(windowInfo->IsSelected(WindowInformation::ID_activeSource) || doAll)
    {
        OpenActiveSourceInFileServer();
    }

    //
    // If the active source, time slider, or time slider states change then
    // we need to update the file selection. Note - without this code, the
    // file selection never updates from what the user clicked.
    //
    if((windowInfo->IsSelected(WindowInformation::ID_activeSource) ||
        windowInfo->IsSelected(WindowInformation::ID_activeTimeSlider) ||
        windowInfo->IsSelected(WindowInformation::ID_timeSliderCurrentStates) ||
        doAll) &&
        showSelectedFiles)
    {
        ExpandDatabases();
        // Highlight the selected file.
        UpdateFileSelection();
    }

    // timeSliders changed. That's the list of time slider names.
    if(windowInfo->IsSelected(WindowInformation::ID_timeSliders) || doAll)
    {
        //
        // Set the values in the active time slider selector and set its
        // visibility. If we don't have an active time slider then we must
        // have no time sliders and thus should not show the time slider
        // selector.
        //
        if(activeTS >= 0)
        {
            int i;
            activeTimeSlider->blockSignals(true);
            activeTimeSlider->clear();
            const stringVector &tsNames = windowInfo->GetTimeSliders();

            //
            // Use a name simplifier to shorten the source names.
            //
            NameSimplifier simplifier;
            const stringVector &sources = GetViewerState()->
                GetGlobalAttributes()->GetSources();
            for(i = 0; i < sources.size(); ++i)
                simplifier.AddName(sources[i]);
            stringVector shortNames;
            simplifier.GetSimplifiedNames(shortNames);

            //
            // Fill in the combo box using the short name for sources and
            // time slider names for items that are not sources.
            //
            for(i = 0; i < tsNames.size(); ++i)
            {
                int index = -1;
                for(int j = 0; j < sources.size(); ++j)
                { 
                    if(sources[j] == tsNames[i])
                    {
                        index = j;
                        break;
                    }
                }
                if(index == -1)
                {
                    // The time slider is not a source so use the original
                    // time slider name.
                    activeTimeSlider->insertItem(tsNames[i].c_str());
                }
                else
                {
                    // The time slider was a source, use the short name.
                    activeTimeSlider->insertItem(shortNames[index].c_str());
                }
            }
            activeTimeSlider->setCurrentItem(activeTS);
            activeTimeSlider->blockSignals(false);

            bool enableSlider = windowInfo->GetTimeSliders().size() > 1;
            activeTimeSlider->setEnabled(enableSlider);
            activeTimeSliderLabel->setEnabled(enableSlider);

            if(enableSlider && !activeTimeSlider->isVisible())
            {
                activeTimeSlider->show();
                activeTimeSliderLabel->show();
                updateGeometry();
            }
            else if(!enableSlider && activeTimeSlider->isVisible())
            {
                activeTimeSlider->hide();
                activeTimeSliderLabel->hide();
                updateGeometry();
            }
        }
        else if(activeTimeSlider->isVisible())
        {
            activeTimeSlider->setEnabled(false);
            activeTimeSliderLabel->setEnabled(false);
            activeTimeSlider->hide();
            activeTimeSliderLabel->hide();
            updateGeometry();
        }
    }

    // Update the animation controls.
    if(windowInfo->IsSelected(WindowInformation::ID_activeTimeSlider) ||
       windowInfo->IsSelected(WindowInformation::ID_timeSliderCurrentStates) || doAll)
    {
        DatabaseCorrelationList *cL = GetViewerState()->
            GetDatabaseCorrelationList();
        DatabaseCorrelation *activeTSCorrelation = 0;
        if(activeTS >= 0)
        {
            // Try and find a correlation for the active time slider so we
            // can get the number of states in the correlation.
            const std::string &activeTSName = windowInfo->GetTimeSliders()[activeTS];
            activeTSCorrelation = cL->FindCorrelation(activeTSName);
        }

        int currentState = 0;
        int nTotalStates = 1;
        if(activeTSCorrelation)
        {
            currentState = windowInfo->GetTimeSliderCurrentStates()[activeTS];
            nTotalStates = activeTSCorrelation->GetNumStates();
        }
        else if(GetViewerState()->GetKeyframeAttributes()->GetEnabled())
        {
            //
            // Keyframing is enabled so we must be using the keyframing time
            // slider if we didn't find a correlation for the active time
            // slider. Get the number of keyframes and use that as the
            // length of the time slider.
            //
            currentState = windowInfo->GetTimeSliderCurrentStates()[activeTS];
            nTotalStates = GetViewerState()->GetKeyframeAttributes()->GetNFrames();
        }

        animationPosition->blockSignals(true);
        animationPosition->setRange(0, nTotalStates - 1);
        animationPosition->setPageStep(1);
        animationPosition->setValue(currentState);
        animationPosition->blockSignals(false);

        // Set the time field to the cycle number.
        UpdateTimeFieldText(currentState);

        // Set the enabled state of the animation widgets.
        UpdateAnimationControlsEnabledState();
    }

    // If the VCR controls are selected (animationMode), set the mode.
    if(windowInfo->IsSelected(WindowInformation::ID_animationMode) || doAll)
    {
        vcrControls->blockSignals(true);
        vcrControls->SetActiveButton(windowInfo->GetAnimationMode());
        vcrControls->blockSignals(false);
    }
}

// ****************************************************************************
// Method: QvisFilePanel::UpdateAnimationControlsEnabledState
//
// Purpose: 
//   Set the enabled state for the animation controls.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 30 12:01:29 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilePanel::UpdateAnimationControlsEnabledState()
{
    //
    // Set the enabled state for the animation controls. They should be
    // available whenever we have an active time slider.
    //
    bool enabled = (windowInfo->GetActiveTimeSlider() >= 0);
    vcrControls->setEnabled(enabled);
    animationPosition->setEnabled(enabled);
    timeField->setEnabled(enabled);
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
//   Brad Whitlock, Mon Oct 13 16:01:58 PST 2003
//   Added support for showing time in the text field.
//
//   Brad Whitlock, Sat Jan 24 20:43:30 PST 2004
//   Updated for the new time and file scheme.
//
// ****************************************************************************

void
QvisFilePanel::UpdateTimeFieldText(int timeState)
{
    if(fileServer == 0 || windowInfo == 0)
        return;

    QString timeString;

    // Set the time field to the cycle number.
    int activeTS = windowInfo->GetActiveTimeSlider();
    if(activeTS >= 0)
    {
        const std::string &tsName = windowInfo->GetTimeSliders()[activeTS];

        //
        // Look for a database correlation with that name.
        //
        DatabaseCorrelationList *dbCorrelations = GetViewerState()->
            GetDatabaseCorrelationList();
        DatabaseCorrelation *correlation = dbCorrelations->FindCorrelation(tsName);
        if(correlation)
        {
            //
            // Figure out how to display
            //
            if(correlation->GetNumDatabases() == 1)
            {
//
// Multi DB correlations that are correlated in time could also show time but I'm not
// quite sure how to get it out of the correlation. Only time though - not cycle.
//
                if(timeStateFormat.GetDisplayMode() == TimeFormat::Cycles ||
                   timeStateFormat.GetDisplayMode() == TimeFormat::CyclesAndTimes)
                {
                    const intVector &cycles = correlation->GetDatabaseCycles();
                    if(timeState < cycles.size())
                        timeString = FormattedCycleString(cycles[timeState]);
                }
                else if(timeStateFormat.GetDisplayMode() == TimeFormat::Times)
                {
                    const doubleVector &times = correlation->GetDatabaseTimes();
#if 0
//
// There's nothing in the correlation that says whether we can believe the times.
//
                    const unsignedCharVector &timesAccurate =
                        correlation->GetDatabaseTimesAccurate();
                    if(timeState < times.size())
                    {
                        timeString = FormattedTimeString(times[timeState],
                            timesAccurate[timeState] == 1);
                    }
                    else
                        timeString = "?";
#else
                    if(timeState < times.size())
                    {
                        timeString = FormattedTimeString(times[timeState], true);
                    }
                    else
                        timeString = "?";
#endif
                }

                timeField->setText(timeString);
            }
            else
            {
                // The correlation has multiple databases so how we display time will vary
                // depending on the correlation method.
                bool timeNeedsToBeSet = true;
                if(timeState < correlation->GetNumStates())
                {
                    if(correlation->GetMethod() == DatabaseCorrelation::TimeCorrelation)
                    {
                        timeNeedsToBeSet = false;
                        timeString = FormattedTimeString(
                            correlation->GetCondensedTimeForState(timeState), true);
                        timeField->setText(timeString);
                    }
                    else if(correlation->GetMethod() == DatabaseCorrelation::CycleCorrelation)
                    {
                        timeNeedsToBeSet = false;
                        timeString = FormattedCycleString(
                            correlation->GetCondensedCycleForState(timeState));
                        timeField->setText(timeString);
                    }
                }

                // If we did not set the time yet, do it with the index.
                if(timeNeedsToBeSet)
                {
                    timeString.sprintf("%d", timeState);
                    timeField->setText(timeString);
                }
            }
        }
        else
        {
            // There was no correlation but we know the time state that we want to
            // display so let's show that in the time line edit.
            timeString.sprintf("%d", timeState);
            timeField->setText(timeString);
        }
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
//   Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//   Changed interface to FileServerList::GetMetaData
//
//   Mark C. Miller, Fri Aug 10 23:11:55 PDT 2007
//   Checked to see if item has had its time/state forced and if not, but
//   the file server is currently forcing, set showing correct file info to
//   false. Also, Propogate knowledge that item was updated with metadata
//   that was forced to get accurate cycles/times.
// ****************************************************************************

void
QvisFilePanel::ExpandDatabases()
{
debug5 << "In QvisFilePanel::ExpandDatabases " << endl;
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
    int nDBWithDifferentNStates = 0;

    for(i = 0; i < count; ++i)
    {
        QvisListViewFileItem *item = items[i];

        if(item != 0)
        {
            if(item->firstChild() == 0)
            {
                if(item->timeState == -1)
                    ExpandDatabaseItem(item);
            }
            else if(HaveFileInformation(item->file))
            {
                // See if the file is a database
                const avtDatabaseMetaData *md =
                    fileServer->GetMetaData(item->file,
                                            GetStateForSource(item->file),
                                            FileServerList::ANY_STATE,
                                           !FileServerList::GET_NEW_MD);
                if(md != 0 && md->GetNumStates() > 1)
                {
		    if (fileServer->GetForceReadAllCyclesTimes() &&
		        !item->timeStateHasBeenForced)
                        SetFileShowsCorrectData(item->file, false);

                    if(md->GetNumStates() != item->childCount())
                    {
                        //
                        // We likely have a virtual database that has multiple time states
                        // per file. It would be a pain to fix it here so just note that
                        // it happened and do an update later.
                        //
                        ++nDBWithDifferentNStates;
                        SetFileShowsCorrectData(item->file, false);
                    }
                    else if(!FileShowsCorrectData(item->file))
                    {
                        bool useVirtualDBInfo = DisplayVirtualDBInformation(item->file);
                        int j;
                        QListViewItemIterator it(item); ++it;
                        for(j = 0; j < item->childCount(); ++j, ++it)
                        {
                             // Reset the label so that it shows the right values.
                             it.current()->setText(0, CreateItemLabel(md, j, 
                                 useVirtualDBInfo));
                        }

                        // Remember that the item now has the correct information
                        // displayed through its children.
                        SetFileShowsCorrectData(item->file, true);
		        item->timeStateHasBeenForced =
			    fileServer->GetForceReadAllCyclesTimes();
                    }
                }
            }           
        }
    }

    //
    // If we had databases for which we were not showing the correct number of time
    // states update the file list later from the event loop so we have the
    // opportunity to correct the problem without having to try and do it in this
    // method.
    //
    if(nDBWithDifferentNStates > 0)
    {
        QTimer::singleShot(100, this, SLOT(internalUpdateFileList()));
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
//   Brad Whitlock, Mon Oct 13 15:37:56 PST 2003
//   Moved code into CreateItemLabel method.
//
//   Brad Whitlock, Fri Oct 24 14:12:45 PST 2003
//   Fixed a bug that caused expanded databases to sometimes get the wrong
//   database in their child time states.
//
//   Brad Whitlock, Mon Dec 29 11:34:42 PDT 2003
//   I changed the code so we can correctly display virtual databases that
//   have multiple time states per file.
//
//   Brad Whitlock, Tue Apr 6 12:23:11 PDT 2004
//   I changed the code so it sets the time state for files that we've
//   never seen before to 0 so it is not the default -1.
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
        ExpandDatabaseItemUsingMetaData(item);
    }
    else if(item->file.IsVirtual())
    {
        ExpandDatabaseItemUsingVirtualDBDefinition(item);
    }
}

// ****************************************************************************
// Method: QvisFilePanel::ExpandDatabaseItemUsingMetaData
//
// Purpose: 
//   Expands a database item as a database, which means that it uses the
//   number of time states to display the database.
//
// Arguments:
//   item : The item to expand.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 29 12:22:41 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Apr 6 12:22:24 PDT 2004
//   I made it set files with 1 time state to have their time state be 0.
//
//   Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//   Changed interface to FileServerList::GetMetaData
//   Mark C. Miller, Fri Aug 10 23:11:55 PDT 2007
//
//   Propogated knowledge that item was updated with metadata that was forced
//   to get accurate cycles/times.
// ****************************************************************************

void
QvisFilePanel::ExpandDatabaseItemUsingMetaData(QvisListViewFileItem *item)
{
    // See if the file is a database
    const avtDatabaseMetaData *md =
        fileServer->GetMetaData(item->file,
                                GetStateForSource(item->file),
                                FileServerList::ANY_STATE,
                               !FileServerList::GET_NEW_MD);
    if(md != 0)
    {
        if(md->GetNumStates() > 1)
        {
            fileListView->blockSignals(true);
            bool useVirtualDBInfo = DisplayVirtualDBInformation(item->file);
            for(int i = 0; i < md->GetNumStates(); ++i)
            {
                QvisListViewFileItem *fi = new QvisListViewFileItem(
                    item, CreateItemLabel(md, i, useVirtualDBInfo),
                    item->file, QvisListViewFileItem::FILE_NODE, i);
                fi->setOpen(false);
            }
	    item->timeStateHasBeenForced =
	        fileServer->GetForceReadAllCyclesTimes();

            // Set the database pixmap.
            item->setPixmap(0, *databasePixmap);
            fileListView->blockSignals(false);

            // Remember that the item now has the correct information
            // displayed through its children.
            SetFileShowsCorrectData(item->file, true);
        }
#if 0
        else
            item->timeState = 0;
#endif
    }
}

// ****************************************************************************
// Method: QvisFilePanel::ExpandDatabaseItemUsingVirtualDBDefinition
//
// Purpose: 
//   Expands a database item as a virtual database when possible. Otherwise, if
//   the virtual database has more time states than files, it is displayed
//   as a database.
//
// Arguments:
//   item : The item that gets expanded.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 29 12:23:37 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilePanel::ExpandDatabaseItemUsingVirtualDBDefinition(QvisListViewFileItem *item)
{
    if(DisplayVirtualDBInformation(item->file))
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
        SetFileShowsCorrectData(item->file, false);
    }
    else if(item->file.IsVirtual())
    {
        // The database is virtual but it must have more time states than
        // files so we should expand it as a database.
        ExpandDatabaseItemUsingMetaData(item);
    }
}

// ****************************************************************************
// Method: QvisFilePanel::CreateItemLabel
//
// Purpose: 
//   Creates the label for the item at the requested database timestate.
//
// Arguments:
//   md : The metadata to use when computing the label.
//   ts : The timestate to use when computing the label.
//   useVirtualDBInformation : Whether or not to use virtual DB information
//                             if it is available.
//
// Returns:    A string to use in the file panel to help display the file.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 13 15:42:17 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Dec 29 10:15:24 PDT 2003
//   I added the useVirtualDBInformation argument so we can force this method
//   to ignore the fact that a database may be virtual.
//
// ****************************************************************************

QString
QvisFilePanel::CreateItemLabel(const avtDatabaseMetaData *md, int ts,
    bool useVirtualDBInformation)
{
    QString label, space(" ");

    if(timeStateFormat.GetDisplayMode() == TimeFormat::Cycles)
    {
        int cycle = (ts < md->GetCycles().size()) ? md->GetCycles()[ts] : ts;
        if(useVirtualDBInformation && md->GetIsVirtualDatabase())
        {
            QualifiedFilename name(md->GetTimeStepNames()[ts]);
            label = QString(name.filename.c_str()) +
                    space + QString(tr("cycle")) + space + FormattedCycleString(cycle);
        }
        else
            label = QString(tr("cycle")) + space + FormattedCycleString(cycle);
    }
    else if(timeStateFormat.GetDisplayMode() == TimeFormat::Times)
    {
        double t = (ts < md->GetTimes().size()) ? md->GetTimes()[ts] : double(ts);
        bool   accurate = (ts < md->GetTimes().size()) ?
                          md->IsTimeAccurate(ts) : false;
        if(useVirtualDBInformation && md->GetIsVirtualDatabase())
        {
            QualifiedFilename name(md->GetTimeStepNames()[ts]);

            label = QString(name.filename.c_str()) +
                    space + QString(tr("time")) + space + FormattedTimeString(t, accurate);
        }
        else
            label = QString(tr("time")) + space + FormattedTimeString(t, accurate);
    }
    else if(timeStateFormat.GetDisplayMode() == TimeFormat::CyclesAndTimes)
    {
        int    cycle = (ts < md->GetCycles().size()) ? md->GetCycles()[ts] : ts;
        double t = (ts < md->GetTimes().size()) ? md->GetTimes()[ts] : double(ts);
        bool   accurate = (ts < md->GetTimes().size()) ?
                          md->IsTimeAccurate(ts) : false;
        if(useVirtualDBInformation && md->GetIsVirtualDatabase())
        {
            QualifiedFilename name(md->GetTimeStepNames()[ts]);

            label = QString(name.filename.c_str()) +
                    space + QString(tr("cycle")) + space + FormattedCycleString(cycle) +
                    space + space + QString(tr("time")) + space + FormattedTimeString(t, accurate);
        }
        else
        {
            label = QString(tr("cycle")) + space + FormattedCycleString(cycle) +
                    space + space + QString(tr("time")) + space + FormattedTimeString(t, accurate);
        }
    }

    return label;
}

// ****************************************************************************
// Method: QvisFilePanel::FormattedCycleString
//
// Purpose: 
//   Returns a formatted cycle string.
//
// Arguments:
//   cycle : The cycle that we want to convert to a string.
//
// Returns:    A formatted cycle string.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 14 11:31:42 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

QString
QvisFilePanel::FormattedCycleString(const int cycle) const
{
    QString retval;
    retval.sprintf("%04d", cycle);
    return retval;
}

// ****************************************************************************
// Method: QvisFilePanel::FormattedTimeString
//
// Purpose: 
//   Returns a formatted time string.
//
// Arguments:
//   t        : The time value to format.
//   accurate : Whether the time can be believed. If it can't then we return
//              a question mark string.
//
// Returns:    A formatted time string.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 13 16:03:37 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QString
QvisFilePanel::FormattedTimeString(const double t, bool accurate) const
{
    QString retval("?");
    if(accurate)
    {
        QString formatString;
        formatString.sprintf("%%.%dg", timeStateFormat.GetPrecision());
        retval.sprintf((const char *)formatString.latin1(), t);
    }
    return retval;
}

// ****************************************************************************
// Method: QvisFilePanel::DisplayVirtualDBInformation
//
// Purpose: 
//   Returns whether or not we should display a virtual database as a
//   virtual database. Sometimes, virtual databases have more states than
//   files. In that case, we want to show them as databases instead of
//   virtual databases.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 29 11:28:59 PDT 2003
//
// Modifications:
//   
//   Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//   Changed interface to FileServerList::GetMetaData
// ****************************************************************************

bool
QvisFilePanel::DisplayVirtualDBInformation(const QualifiedFilename &file) const
{
    bool retval = true;

    const avtDatabaseMetaData *md =
        fileServer->GetMetaData(file, GetStateForSource(file),
                                FileServerList::ANY_STATE,
                               !FileServerList::GET_NEW_MD);
    if(md != 0 && md->GetNumStates() > 1 && md->GetIsVirtualDatabase())
    {
        int nts = fileServer->GetVirtualFileDefinitionSize(file);
        retval = (nts == md->GetNumStates());
    }

    return retval;
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
//   Brad Whitlock, Sat Jan 24 21:38:48 PST 2004
//   I modified it to use the active time slider and database correlation.
//
//   Brad Whitlock, Tue Apr 6 14:05:42 PST 2004
//   I changed it so it returns early if we're not allowing file
//   selection changes.
//
//   Brad Whitlock, Mon Dec 20 16:19:23 PST 2004
//   Added code to update the state of the Replace button.
//
// ****************************************************************************

void
QvisFilePanel::UpdateFileSelection()
{
    // Set the text for the open file button.
    if(fileServer->GetOpenFile().Empty())
        openButton->setText(tr("Open"));
    else
    {
        openButton->setText(tr("ReOpen"));
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

    //
    // Try and find a correlation for the active time slider if there is
    // and active time slider.
    //
    QualifiedFilename activeSource(windowInfo->GetActiveSource());
    DatabaseCorrelation *correlation = 0;
    int dbStateForActiveSource = -1;
    // Get the index of the active time slider.
    int activeTS = windowInfo->GetActiveTimeSlider();
    if(activeTS >= 0)
    {
        // Get the name of the active time slider.
        const std::string &activeTSName = windowInfo->GetTimeSliders()[activeTS];

        // Get the correlation for the active time slider
        DatabaseCorrelationList *correlations = GetViewerState()->
            GetDatabaseCorrelationList();
        correlation = correlations->FindCorrelation(activeTSName);

        if(correlation)
        {
            // Get the state for the active time slider.
            int activeTSState = windowInfo->GetTimeSliderCurrentStates()[activeTS];
            dbStateForActiveSource = correlation->GetCorrelatedTimeState(
                windowInfo->GetActiveSource(), activeTSState);
        }
    }

    //
    // Iterate through the file items and look for the right item to highlight.
    //
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

        if(item->file == activeSource)
        {
            QvisListViewFileItem *parentItem = (QvisListViewFileItem *)item->parent();

            //
            // If the correlation involves the active source then we
            // can highlight using the correlation. If the correlation
            // does not use the active source then dbStateForActiveSource
            // will be set to -1.
            //
            if(dbStateForActiveSource > -1)
            {
                // Check if the current item is for the current time state.
                bool currentTimeState = item->timeState == dbStateForActiveSource;
                
                // If we have information for this file and it turns out that
                // the file is not expanded, then check to see if the item is
                // the root of the database.
                if(HaveFileInformation(item->file) && !FileIsExpanded(item->file))
                {
                    //
                    // The time states are not expanded. If the time state 
                    // that we're looking at is not -1 then it is a time
                    // step in the database but since the file is not
                    // expanded, we want to instead highlight the parent.
                    //
                    if(item->timeState != -1)
                    {
                        item = (QvisListViewFileItem *)item->parent();
                        parentItem = (QvisListViewFileItem *)parentItem->parent();
                    }
                }

                //
                // If we have no selected item so far and the item's
                // timestate is the current time state, then we want
                // to select it.
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
                FileIsExpanded(item->file) && item->timeState == -1)
        {
            item->setOpen(true);
        }
    }

    // Make sure the selected item is visible if we're allowing selection
    // change or if the currently highlighted item is invalid.
    if(selectedItem != 0 && (allowFileSelectionChange ||
       HighlightedItemIsInvalid()))
    {
        fileListView->setSelected(selectedItem, true);
        fileListView->setCurrentItem(selectedItem);
        fileListView->ensureItemVisible(selectedItem);
    }

    // Restore signals.
    fileListView->blockSignals(false);
    blockSignals(false);

    // Update the state of the Replace button.
    UpdateReplaceButtonEnabledState();
}

// ****************************************************************************
// Method: QvisFilePanel::HighlightedItemIsInvalid
//
// Purpose: 
//   Returns whether the currently highlighted item is an invalid selection.
//
// Returns:    True if the highlighted item is not valid; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 13 14:00:08 PST 2004
//
// Modifications:
//   
//   Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//   Changed interface to FileServerList::GetMetaData
// ****************************************************************************

bool
QvisFilePanel::HighlightedItemIsInvalid() const
{
    bool currentItemInvalid = false;
    if(fileListView->currentItem() != 0)
    {
        QvisListViewFileItem *ci = (QvisListViewFileItem *)
            fileListView->currentItem();

        // If the highlighted item is not a file, then it is not valid and
        // we are allowed to change the highlight.
        if(ci->isFile())
        {
            // If we've opened the file before and it is supposed to be
            // expanded
            if(fileServer->HaveOpenedFile(ci->file) &&
               HaveFileInformation(ci->file) &&
               FileIsExpanded(ci->file))
            {
                const avtDatabaseMetaData *md = fileServer->
                    GetMetaData(ci->file, GetStateForSource(ci->file),
                                FileServerList::ANY_STATE,
                               !FileServerList::GET_NEW_MD);
                if(md != 0)
                {
                    // We've opened the file before. If the highlighted item
                    // is an MT database and a time state < 0 is highlighted
                    // then the highlight is wrong. Change it.
                    currentItemInvalid = (md->GetNumStates() > 1) &&
                       (ci->timeState < 0);
                }
            }
        }
        else
            currentItemInvalid = true;
    }

    return currentItemInvalid;
}

// ****************************************************************************
// Method: QvisFilePanel::UpdateReplaceButtonEnabledState
//
// Purpose: 
//   This method updates the enabled state of the Replace button.
//
// Arguments:
//
// Returns:    True if the button was enabled; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 20 16:17:55 PST 2004
//
// Modifications:
//   
//   Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//   Changed interface to FileServerList::GetMetaData
// ****************************************************************************

bool
QvisFilePanel::UpdateReplaceButtonEnabledState()
{
    bool enabled = false;
    if(!fileServer->GetOpenFile().Empty())
    {
        QListViewItem *item = fileListView->currentItem();
        if(item != 0)
        {
            QvisListViewFileItem *ci = (QvisListViewFileItem *)item;
            bool differentFiles = fileServer->GetOpenFile() != ci->file;

            stringVector defs(fileServer->GetVirtualFileDefinition(ci->file));
            if(defs.size() > 1)
            {
                // Only allow the user to replace if they click on one of
                // the real time states.
                enabled = ci->timeState >= 0;
            }
            else
            {
                const avtDatabaseMetaData *md =
                    fileServer->GetMetaData(ci->file, GetStateForSource(ci->file),
                                            FileServerList::ANY_STATE,
                                           !FileServerList::GET_NEW_MD);
                if(md != 0)
                {
                    if(md->GetNumStates() > 1)
                    {
                        // Only allow the user to replace if they click on
                        // one of the real time states.
                        enabled = ci->timeState >= 0;
                    }
                    else
                        enabled = differentFiles;
                }
                else
                    enabled = differentFiles;
            }

            if(!enabled)
            {
                std::string highlightFile(ci->file.FullName());
                PlotList *pl = GetViewerState()->GetPlotList();
                for(int i = 0; i < pl->GetNumPlots(); ++i)
                {
                    const Plot &current = pl->operator[](i);
                    if(highlightFile != current.GetDatabaseName())
                    {
                        enabled = true;
                        break;
                    }
                }
            }
        }
    }

    replaceButton->setEnabled(enabled);
    return enabled;
}

// ****************************************************************************
// Method: QvisFilePanel::SubjectRemoved
//
// Purpose: 
//   Removes the windowInfo or fileserver subjects that this widget
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
//   Brad Whitlock, Sat Jan 24 23:44:56 PST 2004
//   I made it observe windowInfo instead of globalAtts.
//
// ****************************************************************************

void
QvisFilePanel::SubjectRemoved(Subject *TheRemovedSubject)
{
    if(TheRemovedSubject == windowInfo)
        windowInfo = 0;
}

//
// Methods to attach to the windowInfo and fileserver objects.
//

void
QvisFilePanel::ConnectFileServer(FileServerList *fs)
{
    fileServer->Attach(this);

    // Update the file list.
    UpdateFileList(true);
}

void
QvisFilePanel::ConnectWindowInformation(WindowInformation *wi)
{
    windowInfo = wi;
    windowInfo->Attach(this);

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
//   Brad Whitlock, Wed Oct 22 12:18:23 PDT 2003
//   I added the addDefaultPlots flag to OpenDataFile.
//
//   Brad Whitlock, Mon Nov 3 10:42:23 PDT 2003
//   I renamed OpenDataFile to SetOpenDataFile and I moved the code that
//   told the viewer to open the data file to here so file replacement would
//   no longer do more work than is required.
//
//   Brad Whitlock, Mon Dec 20 16:20:49 PST 2004
//   Changed how the Replace button is updated.
//
// ****************************************************************************

bool
QvisFilePanel::OpenFile(const QualifiedFilename &qf, int timeState, bool reOpen)
{
    // Try and open the data file.
    bool retval = SetOpenDataFile(qf, timeState, this, reOpen);

    if(reOpen)
    {
        // Tell the viewer to replace all of the plots having
        // databases that match the file we're re-opening.
        GetViewerMethods()->ReOpenDatabase(qf.FullName().c_str(), false);
    }
    else
    {
        // Tell the viewer to open the database.
        GetViewerMethods()->OpenDatabase(qf.FullName().c_str(), timeState, true);
    }

    // Update the Replace and Overlay buttons.
    UpdateReplaceButtonEnabledState();
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
//   I added time state to OpenFile.
//
//   Brad Whitlock, Wed Oct 15 15:26:31 PST 2003
//   I made it possible to replace a file at a later time state.
//
//   Brad Whitlock, Mon Nov 3 10:46:18 PDT 2003
//   Rewrote so replace does not first tell the viewer to open the database.
//
//   Brad Whitlock, Mon Dec 20 16:21:30 PST 2004
//   Changed how the Replace button's enabled state is set.
//
// ****************************************************************************

void
QvisFilePanel::ReplaceFile(const QualifiedFilename &filename, int timeState)
{
    // Try and set the open the data file.
    SetOpenDataFile(filename, timeState, this, false);

    // Tell the viewer to replace the database.
    GetViewerMethods()->ReplaceDatabase(filename.FullName().c_str(), timeState);

    // Update the Replace and Overlay buttons.
    UpdateReplaceButtonEnabledState();
    overlayButton->setEnabled(false);
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
//   Brad Whitlock, Mon Nov 3 10:50:01 PDT 2003
//   I rewrote the routine so it no longer ends up telling the viewer to
//   open the database before overlaying.
//
//   Brad Whitlock, Mon Dec 20 16:22:19 PST 2004
//   Changed how the enabled state for the Replace button is set.
//
// ****************************************************************************

void
QvisFilePanel::OverlayFile(const QualifiedFilename &filename)
{
    // Try and set the open the data file.
    SetOpenDataFile(filename, 0, this, false);

    // Tell the viewer to replace the database.
    GetViewerMethods()->OverlayDatabase(filename.FullName().c_str());

    // Set the enabled state for the Replace and Overlay buttons.
    UpdateReplaceButtonEnabledState();
    overlayButton->setEnabled(false);
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
//   Brad Whitlock, Tue Apr 13 14:07:48 PST 2004
//   I made the method be const.
//
// ****************************************************************************

bool
QvisFilePanel::FileIsExpanded(const QualifiedFilename &filename) const
{
    bool retval = true;
    FileDisplayInformationMap::const_iterator pos = 
        displayInfo.find(filename.FullName());
    if(pos != displayInfo.end())
        retval = pos->second.expanded;
    return retval;
}

// ****************************************************************************
// Method: QvisFilePanel::FileShowsCorrectData
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
QvisFilePanel::FileShowsCorrectData(const QualifiedFilename &filename)
{
    return displayInfo[filename.FullName()].correctData;
}

// ****************************************************************************
// Method: QvisFilePanel::SetFileShowsCorrectData
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
QvisFilePanel::SetFileShowsCorrectData(const QualifiedFilename &filename,
    bool val)
{
    displayInfo[filename.FullName()].correctData = val;
}

// ****************************************************************************
// Method: QvisFilePanel::SetTimeSliderState
//
// Purpose: 
//   Sets the animation frame.
//
// Arguments:
//   index            : The index of the new time state.
//   indexIsTimeState : Whether the index is a time state or an animation frame
//                      index.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 24 14:36:50 PST 2003
//
// Modifications:
//   Brad Whitlock, Sun Jan 25 01:39:50 PDT 2004
//   I made it compare the state against the active time slider's current state.
//
// ****************************************************************************

void
QvisFilePanel::SetTimeSliderState(int state)
{
    // Figure out the time state for the current time slider.
    int activeTS = windowInfo->GetActiveTimeSlider();
    if(activeTS >= 0)
    {
         int currentState = windowInfo->GetTimeSliderCurrentStates()[activeTS];

         //
         // Set the state for the viewer's active time slider.
         //
         if(state != currentState)
         {
             GetViewerMethods()->SetTimeSliderState(state);
         }
         else
         {  
             QString msg;
             msg.sprintf(" %d.", state);
             Message(tr("The active time slider is already at state") + msg);
         }
    }
}

// ****************************************************************************
// Method: QvisFilePanel::SetShowSelectedFiles
//
// Purpose: 
//   This method sets the visibility for the selected files list and the
//   open, replace, overlay buttons.
//
// Arguments:
//   val : Whether the selected files should be showing.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 30 14:56:14 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilePanel::SetShowSelectedFiles(bool val)
{
    if(val != showSelectedFiles)
    {
        showSelectedFiles = val;

        if(!showSelectedFiles)
        {
            if(fileListView->isVisible())
            {
                fileListView->hide();
                openButton->hide();
                replaceButton->hide();
                overlayButton->hide();
                updateGeometry();
            }
        }
        else if(!fileListView->isVisible())
        {
            fileListView->show();
            openButton->show();
            replaceButton->show();
            overlayButton->show();
            updateGeometry();
            UpdateFileList(true);
        }
    }
}

// ****************************************************************************
// Method: QvisFilePanel::GetShowSelectedFiles
//
// Purpose: 
//   Returns whether or not the selected files are showing.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 30 14:57:19 PST 2004
//
// Modifications:
//   
// ****************************************************************************

bool
QvisFilePanel::GetShowSelectedFiles() const
{
    return showSelectedFiles;
}

// ****************************************************************************
// Method: QvisFilePanel::SetAllowFileSelectionChange
//
// Purpose: 
//   Sets whether setting the file selection is allowed.
//
// Arguments:
//   val : The new value for allowFileSelectionChange.
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 6 14:10:10 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilePanel::SetAllowFileSelectionChange(bool val)
{
    if(allowFileSelectionChange != val)
    {
        allowFileSelectionChange = val;
        if(val)
            UpdateFileSelection();
    }
}

// ****************************************************************************
// Method: QvisFilePanel::GetAllowFileSelectionChange
//
// Purpose: 
//   Returns whether the file panel will update the file selection.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 9 14:57:47 PST 2004
//
// Modifications:
//   
// ****************************************************************************

bool 
QvisFilePanel::GetAllowFileSelectionChange() const
{
    return allowFileSelectionChange;
}

// ****************************************************************************
// Method: QvisFilePanel::UpdateOpenButtonState
//
// Purpose: 
//   Updates the text and enabled state of the Open button based on what
//   file is currently selected in the list view.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 27 14:53:26 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilePanel::UpdateOpenButtonState()
{
    QListViewItem *item = fileListView->selectedItem();
    if(item != 0)
    {
        // Cast to a derived type.
        QvisListViewFileItem *fileItem = (QvisListViewFileItem *)item;
        UpdateOpenButtonState(fileItem);
    }
}

//
// Qt slot functions.
//


// ****************************************************************************
// Method: QvisFilePanel::internalUpdateFileList
//
// Purpose: 
//   Updates the file list.
//
// Note:       This method should only be called on a timer because updating
//             the file list affects a lot of things.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 19 17:02:15 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilePanel::internalUpdateFileList()
{
    UpdateFileList(true);
}

// ****************************************************************************
// Method: QvisFilePanel::backwardStep
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to switch to
//   the previous time state for the active time slider.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 1 10:28:22 PDT 2000
//
// Modifications:
//   Brad Whitlock, Tue Jan 27 18:13:18 PST 2004
//   I renamed the method.
//
// ****************************************************************************

void
QvisFilePanel::backwardStep()
{
    // Tell the viewer to go to the previous frame.
    GetViewerMethods()->TimeSliderPreviousState();
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
    GetViewerMethods()->AnimationReversePlay();
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
    // Tell the viewer to stop the animation. Use the viewer proxy because
    // it has an additional mechanism for making the animation stop quicker.
    GetViewerProxy()->AnimationStop();
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
    GetViewerMethods()->AnimationPlay();
}

// ****************************************************************************
// Method: QvisFilePanel::forwardStep
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
//   Brad Whitlock, Tue Jan 27 18:09:28 PST 2004
//    I renamed it to forwardStep and changed the coding to support multiple
//    time sliders.
//
// ****************************************************************************

void
QvisFilePanel::forwardStep()
{
#ifdef BEFORE_NEW_FILE_SELECTION
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
        GetViewerMethods()->AnimationNextFrame();
    }
#else
    GetViewerMethods()->TimeSliderNextState();
#endif
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
//   Brad Whitlock, Tue Jan 27 18:18:59 PST 2004
//   I made the check for multiple states use the metadata instead of the
//   globalAtts.
//
//   Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//   Changed interface to FileServerList::GetMetaData
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
    const QualifiedFilename &qf = fileServer->GetOpenFile();
    if(fileItem->file == qf && fileServer->
                               GetMetaData(qf, GetStateForSource(qf),
                                               FileServerList::ANY_STATE,
                                              !FileServerList::GET_NEW_MD)->
                                               GetNumStates() > 1)
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
//   Brad Whitlock, Tue Feb 3 18:23:21 PST 2004
//   I made the open button turn into the activate button if we highlighted
//   a file that we've opened before that is not the currently open file.
//
//   Brad Whitlock, Mon Dec 20 12:13:42 PDT 2004
//   I moved the code to update the enabled state of the replace button into
//   its own method.
//
//   Brad Whitlock, Mon Jun 27 14:49:43 PST 2005
//   I moved the code that updates the text and enabled state for the Open
//   button into its own method.
//
// ****************************************************************************

void
QvisFilePanel::highlightFile(QListViewItem *item)
{
    if(item == 0)
        return;

    // Cast to a derived type.
    QvisListViewFileItem *fileItem = (QvisListViewFileItem *)item;
    UpdateOpenButtonState(fileItem);

    //
    // If the highlighted file is not the active file, then
    // enable the open, replace, overlay buttons.
    //
    bool enable = UpdateReplaceButtonEnabledState();
    overlayButton->setEnabled(enable);
}

// ****************************************************************************
// Method: QvisFilePanel::UpdateOpenButtonState
//
// Purpose: 
//   Updates the state of the Open button based on what item is highlighted.
//
// Arguments:
//   fileItem : The highlighted item in the list view.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 27 14:57:22 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilePanel::UpdateOpenButtonState(QvisListViewFileItem *fileItem)
{
    // If the filename is not a file. Disable the open files button.
    if(!fileItem->isFile())
    {
        openButton->setEnabled(false);
        return;
    }

    // If we've opened the file before, make the open button say "ReOpen".
    bool fileOpenedBefore = fileServer->HaveOpenedFile(fileItem->file);
    if(fileOpenedBefore)
    {
        if(fileServer->GetOpenFile() != fileItem->file)
            openButton->setText(tr("Activate"));
        else
            openButton->setText(tr("ReOpen"));
    }
    else
        openButton->setText(tr("Open"));
    openButton->setEnabled(true);
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
//   Brad Whitlock, Tue Jan 27 20:27:02 PST 2004
//   I changed how we calculate the current time state for a database that
//   we're reopening so it takes into account the active time slider.
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
        {
            //
            // Get the current state for the specified database taking into
            // account the active time slider.
            //
            timeState = GetStateForSource(fileItem->file);

            //
            // If we're reopening and we're trying to use a later time state
            // then issue an error message because we don't want to let users
            // change time states with reopen because they'll do it and
            // complain that VisIt is slow.
            //
            if(fileItem->timeState != -1 &&
               fileItem->timeState != timeState)
            {
                UpdateFileSelection();
                Error(tr("Reopen cannot be used to change the active time state "
                      "for an animation because reopen discards all cached "
                      "networks and causes the database to actually be "
                      "reopened. If you want to change the active time state, "
                      "use the animation slider or select a new time state "
                      "and click the Replace button."));
                return;
            }
        }
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
//   Brad Whitlock, Fri Oct 24 14:33:12 PST 2003
//   I made it use the new SetTimeSliderState method.
//
//   Brad Whitlock, Tue Feb 3 18:47:50 PST 2004
//   I changed it so it tries to set the time slider to the right state for
//   the database with the time state that we want to open.
//
//   Brad Whitlock, Mon May 3 17:23:46 PST 2004
//   I added code to make sure that the time slider is related to the
//   database that we're opening before trying to figure out the time state
//   of the database.
//
// ****************************************************************************

void
QvisFilePanel::openFileDblClick(QListViewItem *item)
{
    QvisListViewFileItem *fileItem = (QvisListViewFileItem *)item;

    if((fileItem != 0) && fileItem->isFile() && (!fileItem->file.Empty()))
    {
        int state = (fileItem->timeState == -1) ? 0 : fileItem->timeState;

        if(fileItem->timeState != -1 &&
           fileItem->file == fileServer->GetOpenFile())
        {
            // It must be a database timestep if the time state is not -1 and
            // the file item has the same filename as the open file.

            //
            // If the correlation for the active time slider does not use the
            // database that we just double-clicked, 
            //
            WindowInformation *windowInfo = GetViewerState()->GetWindowInformation();
            int activeTS = windowInfo->GetActiveTimeSlider();
            std::string activeTSName, src(fileItem->file.FullName());
            DatabaseCorrelationList *cL = GetViewerState()->GetDatabaseCorrelationList();

            if(activeTS >= 0)
            {
                activeTSName = windowInfo->GetTimeSliders()[activeTS];
                DatabaseCorrelation *c = cL->FindCorrelation(activeTSName);
                DatabaseCorrelation *csrc = cL->FindCorrelation(src);
                if (c != 0 && csrc != 0 && !c->UsesDatabase(src))
                {
                    debug1 << "Have to set the time slider before setting the "
                              "time state because the active time slider does "
                              "not use the database that we double clicked.\n";
                    activeTSName = src;
                    GetViewerMethods()->SetActiveTimeSlider(activeTSName);
                }
            }

            //
            // Get the inverse correlated time state, which is the time state
            // for the active time slider's correlation where the given
            // database has the database time state that we want opened.
            //
            state = GetTimeSliderStateForDatabaseState(activeTSName, src, state);
            GetViewerMethods()->SetTimeSliderState(state);
        }
        else
        {
            AddExpandedFile(fileItem->file);

            // Try and open the file.
            OpenFile(fileItem->file, state, false);
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
//   Brad Whitlock, Wed Oct 15 16:18:40 PST 2003
//   I added code to let replace open files at the selected time state instead
//   of using time state 0.
//
//   Brad Whitlock, Fri Oct 24 14:33:40 PST 2003
//   I made it use the new SetTimeSliderState method.
//
//   Brad Whitlock, Mon Nov 3 11:33:05 PDT 2003
//   I made it always use the ReplaceFile method and I changed the
//   ReplaceDatabase functionality in the viewer so changes the animation
//   time state if we're replacing with the same database.
//
// ****************************************************************************

void
QvisFilePanel::replaceFile()
{
    QvisListViewFileItem *fileItem = (QvisListViewFileItem *)
        fileListView->currentItem();

    if((fileItem != 0) && fileItem->isFile() && (!fileItem->file.Empty()))
    {
        // Make sure that we use a valid time state. Some file items
        // have a time state of -1 if they are the parent item of several
        // time step items.
        int timeState = (fileItem->timeState < 0) ? 0 : fileItem->timeState;

        // Try and replace the file.
        ReplaceFile(fileItem->file.FullName(), timeState);
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
//   Brad Whitlock, Tue Dec 30 14:32:02 PST 2003
//   I removed sliderDown.
//
//   Brad Whitlock, Tue Jan 27 20:31:25 PST 2004
//   I added support for multiple time sliders.
//
// ****************************************************************************

void
QvisFilePanel::sliderStart()
{
    int activeTS = windowInfo->GetActiveTimeSlider();
    if(activeTS >= 0)
        sliderVal = windowInfo->GetTimeSliderCurrentStates()[activeTS];
    else 
        sliderVal = 0;
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
//   Brad Whitlock, Fri Oct 24 14:31:33 PST 2003
//   I made it use the new SetTimeSliderState method and I removed old code to
//   set the current frame in the global atts. The viewer now sends back the
//   current frame but it didn't used to a long time ago.
//
// ****************************************************************************

void
QvisFilePanel::sliderEnd()
{
    // Set the new frame.
    SetTimeSliderState(sliderVal);
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
//   Brad Whitlock, Fri Oct 24 14:31:33 PST 2003
//   I made it use the new SetTimeSliderState method and I removed old code to
//   set the current frame in the global atts. The viewer now sends back the
//   current frame but it didn't used to a long time ago.
//
//   Brad Whitlock, Tue Dec 30 14:31:03 PST 2003
//   I made it use the animation slider instead of sliderDown.
//
// ****************************************************************************

void
QvisFilePanel::sliderChange(int val)
{
    if(animationPosition->sliderIsDown())
        return;

    // Set the new frame.
    SetTimeSliderState(val);
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
//   Brad Whitlock, Mon Oct 13 16:05:11 PST 2003
//   I changed the code so it's possible to enter time into the text field.
//
//   Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//   Changed interface to FileServerList::GetMetaData
// ****************************************************************************

void
QvisFilePanel::processTimeText()
{
    // Convert the text to an integer value.
    QString temp(timeField->text().stripWhiteSpace());
    if(temp.isEmpty())
        return;

    const avtDatabaseMetaData *md = fileServer->GetMetaData(
                                        fileServer->GetOpenFile(),
                                        GetStateForSource(fileServer->GetOpenFile()),
                                        FileServerList::ANY_STATE,
                                       !FileServerList::GET_NEW_MD);

    int  index = 0;
    bool okay = false;

    //
    // If the string has a decimal in it then assume it is a time.
    //
    if(temp.find('.') != -1)
    {
        double t = temp.toDouble(&okay);
        if(!okay)
        {
            timeField->setText("");
            return;
        } 

        // Loop through the times for the current file while the
        // time that was entered is greater than or equal to the
        // time in the list.
        if(md->GetTimes().size() == md->GetNumStates())
        {
            for(int i = 0; i < md->GetNumStates(); ++i)
            {
                if(t <= md->GetTimes()[i])
                    break;
                else
                    ++index;
            }
        }
    }
    else
    {
        int cycle = temp.toInt(&okay);
        if(!okay)
        {
            timeField->setText("");
            return;
        } 

        // Loop through the cycles for the current file while the
        // cycle that was entered is greater than or equal to the
        // cycle in the list.
        for(int i = 0; i < md->GetNumStates(); ++i)
        {
            if(cycle <= md->GetCycles()[i])
                break;
            else
                ++index;
        }
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

// ****************************************************************************
// Method: QvisFilePanel::updateHeaderWidthForLongName
//
// Purpose: 
//   This is a Qt slot function that updates the width of the first column
//   so it is as wide as the longest node name in the file tree. This allows
//   the horizontal scrollbar to be active.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 6 12:17:46 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilePanel::updateHeaderWidthForLongName()
{
    if(fileListView->childCount() < 1)
        updateHeaderWidth();
    else
    {
        int maxWidth = 0;
        QListViewItemIterator it(fileListView);
        for ( ; it.current(); ++it )
        {
            int w = it.current()->width(fontMetrics(), fileListView, 0);
            if(w > maxWidth)
                maxWidth = w;
        }

        if(maxWidth < fileListView->visibleWidth())
            maxWidth = fileListView->visibleWidth();

        fileListView->setColumnWidth(0, maxWidth);
    }
}

// ****************************************************************************
// Method: QvisFilePanel::changeActiveTimeSlider
//
// Purpose: 
//   Tell the viewer to switch to the new active time slider.
//
// Arguments:
//   tsName : The name of the new active time slider.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 27 21:30:12 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisFilePanel::changeActiveTimeSlider(int tsIndex)
{
    if(windowInfo->GetTimeSliders().size() > 1)
    {
        const stringVector &tsNames = windowInfo->GetTimeSliders();
        if(tsIndex >= 0 && tsIndex < tsNames.size())
            GetViewerMethods()->SetActiveTimeSlider(tsNames[tsIndex]);
    }
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
//   Brad Whitlock, Tue Feb 24 15:43:28 PST 2004
//   I fixed the code so single filenames are again shown without the path.
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
    int rootSize = root->Size();
    if(rootSize > 2)
        root->Reduce();
    else if(rootSize <= 2)
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

// ****************************************************************************
// Method: FileTree::HasNodeNameExceeding
//
// Purpose: 
//   Returns whether the nodes in the tree have a node name that exceeds the
//   specified length.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 5 17:34:58 PST 2004
//
// Modifications:
//   
// ****************************************************************************

bool
FileTree::HasNodeNameExceeding(int len) const
{
    return (root != 0) ? root->HasNodeNameExceeding(len) : false;
}

// ****************************************************************************
// Method: FileTree::operator <<
//
// Purpose: 
//   Prints the FileTree to a stream.
//
// Arguments:
//   os : The stream to which we want to print.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 29 10:08:20 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

ostream &
operator <<(ostream &os, const FileTree &t)
{
    t.root->Print(os, "");
    return os;
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
//   Brad Whitlock, Mon Oct 27 13:42:32 PST 2003
//   I made the new node get inserted into the list using the fileName to
//   determine where to insert because the fileName is sorted using the
//   numeric string comparison routine. This fixes cases where files like
//   foo10z0000.silo would come before foo2z0000.silo.
//
//   Brad Whitlock, Thu Jul 29 10:57:59 PDT 2004
//   I changed the node insertion scheme so the node name is used
//   (as it must be) with a numeric string comparison so we preserve
//   compatibility with the file indices in the file server list.
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
                if(notInserted &&
                   NumericStringCompare(newNode->nodeName, children[i]->nodeName))
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
// Method: FileTree::FileTreeNode::HasNodeNameExceeding
//
// Purpose: 
//   Returns whether any of the nodes in the tree have a node name that is 
//   longer than the specified length.
//
// Arguments:
//   len : The specified node name length.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 5 17:32:29 PST 2004
//
// Modifications:
//   
// ****************************************************************************

bool
FileTree::FileTreeNode::HasNodeNameExceeding(int len) const
{
    if(nodeName.size() > len)
        return true;

    // Check the children.
    for(int i = 0; i < numChildren; ++i)
    {
        if(children[i]->HasNodeNameExceeding(len))
            return true;
    }
 
    return false;
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
//   Brad Whitlock, Thu Jul 29 10:00:51 PDT 2004
//   I fixed a bug where if the first file in a directory lacked read access
//   then the directory would be displayed as having no access.
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
            //
            // Paths must always have access. Paths that were created by a file
            // without read permission would have been reduced so any paths
            // that remain at this point must have access.
            //
            fileName.SetAccess(true);

            // Add a directory node.
            QString temp;
            if(nodeName == fileName.path && nodeName[0] != separator)
                temp.sprintf("%c%s", separator, nodeName.c_str());
            else
                temp = QString(nodeName.c_str());
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

// ****************************************************************************
// Method: FileTree::FileTreeNode::Print
//
// Purpose: 
//   Prints the node to a stream.
//
// Arguments:
//   os     : The stream to which we want to print.
//   indent : The string to prepend to all of the output.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 29 10:08:58 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
FileTree::FileTreeNode::Print(ostream &os, const std::string &indent) const
{
    os << indent.c_str() << "[" << nodeName.c_str() << "] " << fileName.FullName().c_str() << endl;

    // Add any children there might be.
    if(numChildren > 0)
    {
        std::string newIndent(std::string("    ") + indent);

        for(int i = 0; i < numChildren; ++i)
            children[i]->Print(os, newIndent);
    }
}
