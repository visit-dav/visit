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

#include <stdlib.h>
#include <visit-config.h>
#include <DataNode.h>
#include <DebugStream.h>
#include <QvisHelpWindow.h>
#include <QAction>
#include <QDomDocument>
#include <QFile>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QSplitter>
#include <QTabWidget>
#include <QTextBrowser>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QWidget>

#include <string>

#include <icons/back.xpm>
#include <icons/forward.xpm>
#include <icons/openbook.xpm>
#include <icons/closedbook.xpm>
#include <icons/help.xpm>
#include <icons/home.xpm>
#include <icons/fontUp.xpm>
#include <icons/fontDown.xpm>

// ****************************************************************************
// Method: QvisHelpWindow::QvisHelpWindow
//
// Purpose: 
//   Constructor for the QvisHelpWindow class.
//
// Arguments:
//   captionString : The window caption.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 12:57:06 PDT 2002
//
// Modifications:
//   Brad Whitlock, Wed Apr  9 11:28:58 PDT 2008
//   QString for captionString.
//
//   Brad Whitlock, Mon Apr 21 15:24:47 PDT 2008
//   Added locale.
//
//   Jeremy Meredith, Thu Aug  7 15:42:23 EDT 2008
//   Fixed initializer order to match true order.
//
// ****************************************************************************

QvisHelpWindow::QvisHelpWindow(const QString &captionString) :
    QvisDelayedWindow(captionString), locale(), helpFile(), index(), bookmarks()
{
    // Set the help path from an environment variable.
    char *helpHome = getenv("VISITHELPHOME");
    if(helpHome)
        helpPath = QString(helpHome);
    else
        helpPath = "";

    firstShow = true;
    activeTab = 0;
}

// ****************************************************************************
// Method: QvisHelpWindow::~QvisHelpWindow
//
// Purpose: 
//   Destructor for the QvisHelpWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 12:57:38 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

QvisHelpWindow::~QvisHelpWindow()
{
}

// ****************************************************************************
// Method: QvisHelpWindow::SetLocale
//
// Purpose: 
//   Set the locale to be used when searching for help files.
//
// Arguments:
//   s : The locale to use.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 21 15:25:26 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisHelpWindow::SetLocale(const QString &s)
{
    locale = s;
}

// ****************************************************************************
// Method: QvisHelpWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the help window.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 12:57:59 PDT 2002
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Wed Nov 19 15:51:04 PST 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisHelpWindow::CreateWindowContents()
{
    QIcon backIcon, forwardIcon, homeIcon, fontUpIcon, fontDownIcon;

    // Create some icons.
    backIcon = QIcon(QPixmap(back_xpm));
    forwardIcon = QIcon(QPixmap(forward_xpm));
    closedBookIcon = QIcon(QPixmap(closedbook_xpm));
    openBookIcon = QIcon(QPixmap(openbook_xpm));
    helpIcon = QIcon(QPixmap(help_xpm));
    homeIcon = QIcon(QPixmap(home_xpm));
    fontUpIcon = QIcon(QPixmap(fontUp_xpm));
    fontDownIcon = QIcon(QPixmap(fontDown_xpm));

    // Create a toolbar
    QToolBar *tb = addToolBar(tr("Help Navigation"));

    // Create a splitter and add it to the layout.
    splitter = new QSplitter(central);
    splitter->setOrientation(Qt::Horizontal);
    topLayout->addWidget(splitter);

    // Create the tab widget
    helpTabs = new QTabWidget(splitter);
    helpTabs->setMinimumWidth(200);
    connect(helpTabs, SIGNAL(currentChanged(int)),
            this, SLOT(activeTabChanged(int)));

    // Create the Contents tab.
    helpContents = new QTreeWidget(helpTabs);
    helpContents->setHeaderLabel(tr("Contents"));
    connect(helpContents, SIGNAL(itemClicked(QTreeWidgetItem *,int)),
            this, SLOT(openHelp(QTreeWidgetItem *)));
    connect(helpContents, SIGNAL(itemCollapsed(QTreeWidgetItem *)),
            this, SLOT(topicCollapsed(QTreeWidgetItem *)));
    connect(helpContents, SIGNAL(itemExpanded(QTreeWidgetItem *)),
            this, SLOT(topicExpanded(QTreeWidgetItem *)));

    helpTabs->addTab(helpContents, tr("&Contents"));
    QAction *a = new QAction(this);
    a->setShortcut(QKeySequence(Qt::Key_Alt, Qt::Key_C));
    connect(a, SIGNAL(triggered(bool)), this, SLOT(activateContentsTab()));

    // Create the Index tab.
    helpIndexTab = new QWidget(helpTabs);
    QVBoxLayout *helpIndexLayout = new QVBoxLayout(helpIndexTab);
    helpIndexLayout->setSpacing(5);
    helpIndexLayout->setMargin(5);
    helpTabs->addTab(helpIndexTab, tr("&Index"));
    a = new QAction(this);
    a->setShortcut(QKeySequence(Qt::Key_Alt, Qt::Key_I));
    connect(a, SIGNAL(triggered(bool)), this, SLOT(activateIndexTab()));

    // Index widgets.
    helpIndexText = new QLineEdit(helpIndexTab);
    connect(helpIndexText, SIGNAL(textChanged(const QString &)),
            this, SLOT(lookForIndexTopic(const QString &)));
    helpIndexLayout->addWidget(helpIndexText);
    helpIndex = new QListWidget(helpIndexTab);
    connect(helpIndex, SIGNAL(itemSelectionChanged()),
            this, SLOT(displayIndexTopic()));
    helpIndexLayout->addWidget(helpIndex);

    // Create the Bookmark tab.
    helpBookmarksTab = new QWidget(helpTabs);
    QVBoxLayout *helpBookmarksLayout = new QVBoxLayout(helpBookmarksTab);
    helpBookmarksLayout->setSpacing(10);
    helpBookmarksLayout->setMargin(5);
    helpTabs->addTab(helpBookmarksTab, tr("&Bookmarks"));
    a = new QAction(this);
    a->setShortcut(QKeySequence(Qt::Key_Alt, Qt::Key_B));
    connect(a, SIGNAL(triggered(bool)), this, SLOT(activateBookmarkTab()));

    QWidget *bookmarkHBox = new QWidget(helpBookmarksTab);
    QHBoxLayout *bookmarkHBoxLayout = new QHBoxLayout(bookmarkHBox);
    bookmarkHBoxLayout->setSpacing(20);
    bookmarkHBoxLayout->setMargin(0);
    helpBookmarksLayout->addWidget(bookmarkHBox);
    addBookmarkButton = new QPushButton(tr("Add"), bookmarkHBox);
    bookmarkHBoxLayout->addWidget(addBookmarkButton);
    connect(addBookmarkButton, SIGNAL(clicked()),
            this, SLOT(addBookmark()));
    removeBookmarkButton = new QPushButton(tr("Remove"), bookmarkHBox);
    bookmarkHBoxLayout->addWidget(removeBookmarkButton);
    connect(removeBookmarkButton, SIGNAL(clicked()),
            this, SLOT(removeBookmark()));
    helpBookMarks = new QListWidget(helpBookmarksTab);
    helpBookmarksLayout->addWidget(helpBookMarks);
    connect(helpBookMarks, SIGNAL(itemSelectionChanged()),
            this, SLOT(displayBookmarkTopic()));

    // Create the text browser
    helpBrowser = new QTextBrowser(splitter);
    helpBrowser->setMinimumWidth(300);
    QFont f(helpBrowser->font());
    f.setBold(false);
    helpBrowser->setFont(f);
    connect(helpBrowser, SIGNAL(anchorClicked(const QUrl &)),
            helpBrowser, SLOT(setSource(const QUrl &)));

    QAction *backAction = tb->addAction(QIcon(backIcon), tr("Back"), 
        helpBrowser, SLOT(backward()));
    backAction->setShortcut(QKeySequence(Qt::Key_Control, Qt::Key_Left));
    connect(helpBrowser, SIGNAL(backwardAvailable(bool)),
            backAction, SLOT(setEnabled(bool)));

    QAction *forwardAction = tb->addAction(QIcon(forwardIcon), tr("Forward"), 
        helpBrowser, SLOT(forward()));
    forwardAction->setShortcut(QKeySequence(Qt::Key_Control, Qt::Key_Right));
    connect(helpBrowser, SIGNAL(forwardAvailable(bool)),
            forwardAction, SLOT(setEnabled(bool)));

    QAction *homeAction = tb->addAction(QIcon(homeIcon), tr("Home"), 
        helpBrowser, SLOT(home()));
    homeAction->setShortcut(QKeySequence(Qt::Key_Control, Qt::Key_Home));

    // buttons to change the font size.
    QAction *upFontAction = tb->addAction(QIcon(fontUpIcon), tr("Larger font"), 
        this, SLOT(increaseFontSize()));

    QAction *downFontAction = tb->addAction(QIcon(fontDownIcon), tr("Smaller font"), 
        this, SLOT(decreaseFontSize()));

    QAction *addBookmarkAction = tb->addAction(QIcon(openBookIcon), tr("Add bookmark"), 
        this, SLOT(addBookmark()));

    // Create the Dismiss button
    QHBoxLayout *buttonLayout = new QHBoxLayout(0);
    topLayout->addLayout(buttonLayout);
    QPushButton *dismissButton = new QPushButton(tr("Dismiss"), central);
    connect(dismissButton, SIGNAL(clicked()), this, SLOT(hide()));
    buttonLayout->addStretch(10);
    buttonLayout->addWidget(dismissButton);

    // Try and load the help index file.
    QString indexFile(CompleteFileName("visit.helpml"));
    LoadHelp(indexFile);

    // Show the appropriate page based on the activeTab setting.
    helpTabs->blockSignals(true);
    helpTabs->setCurrentIndex(activeTab);
    helpTabs->blockSignals(false);
}

// ****************************************************************************
// Method: QvisHelpWindow::ReleaseNotesFile
//
// Purpose: 
//   Returns the name of the release notes file.
//
// Returns:    The name of the release notes file.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 21 15:05:10 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QString
QvisHelpWindow::ReleaseNotesFile() const
{
    QString relNotes, ver(VERSION);
    if(ver.right(1) == "b")
        ver = ver.left(ver.length()-1);
    relNotes = QString("relnotes") + ver + QString(".html");
    return relNotes;
}

// ****************************************************************************
// Method: QvisHelpWindow::LoadHelp
//
// Purpose: 
//   Loads the help index file and populates the help widgets with information
//   from the index file.
//
// Arguments:
//   fileName : The name of the help index XML file.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 12:58:27 PDT 2002
//
// Modifications:
//   Brad Whitlock, Tue Jul 23 12:32:29 PDT 2002
//   I made the window aware of the root list HTML file.
//
//   Brad Whitlock, Tue Sep 10 15:56:11 PST 2002
//   I removed the version from the VisIt home page since it does not seem
//   to ever get updated in time for a release.
//
//   Brad Whitlock, Wed Sep 11 10:23:33 PDT 2002
//   I added the copyright page.
//
//   Brad Whitlock, Fri Feb 18 13:44:43 PST 2005
//   I changed the error message about visit.helpml not found into a message
//   so it does not distract the user if running a help-less distribution
//   for the first time.
//
//   Brad Whitlock, Tue Jan  8 14:55:51 PST 2008
//   Added a contrib page.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Mon Apr 21 15:06:00 PDT 2008
//   Changed how we find the release notes file.
//
//   Brad Whitlock, Thu Jun 19 16:39:30 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisHelpWindow::LoadHelp(const QString &fileName)
{
    helpContents->blockSignals(true);

    // Create a root node for the VisIt home page.
    QTreeWidgetItem *homePage = new QTreeWidgetItem(
        helpContents, 0);
    homePage->setText(0, tr("VisIt home page"));
    homePage->setData(0, Qt::UserRole, QVariant("home.html"));
    homePage->setIcon(0, helpIcon);

    QTreeWidgetItem *releaseNotes = new QTreeWidgetItem(
        helpContents, 0);
    releaseNotes->setText(0, tr("Release Notes"));
    releaseNotes->setData(0, Qt::UserRole, QVariant(ReleaseNotesFile()));
    releaseNotes->setIcon(0, helpIcon);

    QTreeWidgetItem *argsPage = new QTreeWidgetItem(
        helpContents, 0);
    argsPage->setText(0, tr("Command line arguments"));
    argsPage->setData(0, Qt::UserRole, QVariant("args.html"));
    argsPage->setIcon(0, helpIcon);

    QTreeWidgetItem *faqPage = new QTreeWidgetItem(
        helpContents, 0);
    faqPage->setText(0, tr("Frequently asked questions"));
    faqPage->setData(0, Qt::UserRole, QVariant("faq.html"));
    faqPage->setIcon(0, helpIcon);

    QTreeWidgetItem *copyrightPage = new QTreeWidgetItem(
        helpContents, 0);
    copyrightPage->setText(0, tr("Copyright"));
    copyrightPage->setData(0, Qt::UserRole, QVariant("copyright.html"));
    copyrightPage->setIcon(0, helpIcon);

    QTreeWidgetItem *contribPage = new QTreeWidgetItem(
        helpContents);
    contribPage->setText(0, tr("VisIt Contributors"));
    contribPage->setData(0, Qt::UserRole, QVariant("contributors.html"));
    contribPage->setIcon(0, helpIcon);

    // Read the XML file and create the DOM tree. Then use the tree to
    // build the User manual content.
    bool noHelp = false;
    QFile helpIndexFile(fileName);
    if(helpIndexFile.open(QIODevice::ReadOnly))
    {
        QDomDocument domTree;
        if(domTree.setContent(&helpIndexFile))
        {
            // Create a root node for the User's manual.
            QTreeWidgetItem *UMrootItem = new QTreeWidgetItem(helpContents);
            UMrootItem->setIcon(0, openBookIcon);
            UMrootItem->setText(0, tr("VisIt User's Manual"));
            UMrootItem->setData(0, Qt::UserRole, QVariant("list0000.html"));

            // Create the tree view out of the DOM
            QDomElement root = domTree.documentElement();
            QDomNode node = root.firstChild();
            while(!node.isNull())
            {
                if(node.isElement() && node.nodeName() == "body")
                {
                    QDomElement body = node.toElement();
                    BuildContents(UMrootItem, body);
                    break;
                }
                node = node.nextSibling();
            }

            // Make the root node open by default.
            helpContents->expandItem(UMrootItem);
        }
        else
            noHelp = true;
        helpIndexFile.close();
    }
    else
        noHelp = true;

    if(noHelp)
    {
        Message(tr("VisIt cannot read the help index file! "
                "No online help will be available."));
        debug1 << "VisIt cannot read the help index file! "
                  "No online help will be available.\n";
    }

    helpContents->blockSignals(false);

    // Make the VisIt home page selected by default.
    homePage->setSelected(true);

    // Build the index.
    BuildIndex();

    // Build the bookmarks.
    BuildBookmarks();
}

// ****************************************************************************
// Method: QvisHelpWindow::BuildContents
//
// Purpose: 
//   Scans through the contents of the XML tree and populates the contents
//   tab and the index.
//
// Arguments:
//   parentItem    : The listview item to which new listview items are added.
//   parentElement : The XML document node that we're examining.
//
// Note:       This function is recursive.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 12:59:15 PDT 2002
//
// Modifications:
//   Brad Whitlock, Thu Jun 19 16:20:28 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisHelpWindow::BuildContents(QTreeWidgetItem *parentItem,
    const QDomElement &parentElement)
{
    QTreeWidgetItem *thisItem = 0;
    QDomNode node = parentElement.firstChild();
    while(!node.isNull())
    {
        if(node.isElement() && node.nodeName() == "help")
        {
            // add a new list view item for the outline
            if(parentItem == 0)
                thisItem = new QTreeWidgetItem(helpContents);
            else
                thisItem = new QTreeWidgetItem(parentItem);
        
            thisItem->setText(0, node.toElement().attribute("topic"));
            thisItem->setData(0, Qt::UserRole, node.toElement().attribute("doc"));

            // Add the topic to the index.
            AddToIndex(node.toElement().attribute("topic"),
                       node.toElement().attribute("doc"));

            // recursive build of the tree
            BuildContents(thisItem, node.toElement());

            if(thisItem->childCount() > 0)
                thisItem->setIcon(0, closedBookIcon);
            else
                thisItem->setIcon(0, helpIcon);
        }

        node = node.nextSibling();
    }
}

// ****************************************************************************
// Method: QvisHelpWindow::BuildIndex
//
// Purpose: 
//   Populates the index list box.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 13:01:03 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Sep 10 15:57:02 PST 2002
//   I added a release notes and a copyright link.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Thu Jun 19 16:45:54 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisHelpWindow::BuildIndex()
{
    // Add a few more items to the index.
    AddToIndex(tr("Copyright"), "copyright.html");
    AddToIndex(tr("Command line arguments"), "args.html");
    AddToIndex(tr("Frequently asked questions"), "faq.html");
    AddToIndex(tr("FAQ"), "faq.html");
    AddToIndex(tr("VisIt"), "home.html");
    AddToIndex(tr("Release notes"), ReleaseNotesFile());

    // Populate the index list box.
    helpIndex->blockSignals(true);
    IndexMap::ConstIterator it;
    for(it = index.begin(); it != index.end(); ++it)
    {
         helpIndex->addItem(it.key());
    }
    helpIndex->blockSignals(false);
}

// ****************************************************************************
// Method: QvisHelpWindow::AddToIndex
//
// Purpose: 
//   Adds a topic to the index.
//
// Arguments:
//   topic : The name of the topic to add.
//   doc   : The name of the document containing the topic.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 13:01:25 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Jun 19 16:47:37 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisHelpWindow::AddToIndex(const QString &topic, const QString &doc)
{
    // filter out the items we don't want in the index.
    if(topic == "Overview" || doc == QString::null)
        return;

    const char *filters[] = {"Setting the ", "Setting ", "Changing the ",
                             "Changing ", "The "};

    // Filter out certain words.
    QString filteredString(topic);
    for(int i = 0; i < 5; ++i)
    {
        QString f(filters[i]);
        if(filteredString.length() > f.length())
        {
            if(filteredString.left(f.length()) == f)
            {
                 int rc = filteredString.length() - f.length();
                 filteredString = filteredString.right(rc);
            }
        }
    }

    if(filteredString.length() > 1)
    {
        // Make sure the first character is upper case.
        filteredString = filteredString.left(1).toUpper() +
                         filteredString.right(filteredString.length() - 1);

        // Insert the item into the index.
        bool inserted = false;
        for(int j = 0; !inserted; ++j)
        {
            QString key(filteredString);
            if(j > 0)
                key = QString("%1 (%2)").arg(filteredString).arg(j+1);

            IndexMap::ConstIterator it = index.find(key);
            if(it == index.end())
            {
                index[key] = doc;
                inserted = true;
            }
        }
    }
}

// ****************************************************************************
// Method: QvisHelpWindow::BuildBookmarks
//
// Purpose: 
//   Adds all of the book marks to the bookmark widget.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 13:24:22 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisHelpWindow::BuildBookmarks()
{
    IndexMap::ConstIterator it = bookmarks.begin();
    for(; it != bookmarks.end(); ++it)
    {
        helpBookMarks->addItem(it.key());
    }

    if(helpBookMarks->count() > 0)
    {
         helpBookMarks->blockSignals(true);
         helpBookMarks->setCurrentItem(0);
//         helpBookMarks->ensureCurrentVisible();
         helpBookMarks->blockSignals(false);
    }
}

// ****************************************************************************
// Method: QvisHelpWindow::CreateNode
//
// Purpose: 
//   Writes the window's extra information to the config file.
//
// Arguments:
//   parentNode : The node to which the window's attributes are added.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 13:16:59 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Jun 19 15:20:40 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisHelpWindow::CreateNode(DataNode *parentNode)
{
    // Call the base class's method to save the generic window attributes.
    QvisDelayedWindow::CreateNode(parentNode);

    if(saveWindowDefaults)
    {
        DataNode *node = parentNode->GetNode(windowTitle().toStdString());

        // Save the current tab.
        node->AddNode(new DataNode("activeTab", activeTab));

        // Save the bookmark list.
        IndexMap::ConstIterator it;
        int i = 1;
        for(it = bookmarks.begin(); it != bookmarks.end(); ++it, ++i)
        {
            // Create a node for the bookmark.
            QString bmName;
            bmName.sprintf("Bookmark%d", i);
            DataNode *bmNode = new DataNode(bmName.toStdString());

            // Add the topic and the doc to the bookmark node.
            bmNode->AddNode(new DataNode("topic", std::string(it.key().toStdString())));
            bmNode->AddNode(new DataNode("doc", std::string(it.value().toStdString())));

            node->AddNode(bmNode);
        }
    }
}

// ****************************************************************************
// Method: QvisHelpWindow::SetFromNode
//
// Purpose: 
//   Reads window attributes from the DataNode representation of the config
//   file.
//
// Arguments:
//   parentNode : The data node that contains the window's attributes.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 13:16:54 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Jun 19 15:20:46 PDT 2008
//   Qt 4.
//  
// ****************************************************************************

void
QvisHelpWindow::SetFromNode(DataNode *parentNode, const int *borders)
{
    DataNode *winNode = parentNode->GetNode(windowTitle().toStdString());
    if(winNode == 0)
        return;

    // Get the active tab and show it.
    DataNode *node;
    if((node = winNode->GetNode("activeTab")) != 0)
    {
        activeTab = node->AsInt();
        if(activeTab != 0 && activeTab != 1 && activeTab != 2)
            activeTab = 0;
    }

    // Read the bookmarks from the settings.
    int i = 1;
    bool bmFound = false;
    do
    {
        QString bmName;
        bmName.sprintf("Bookmark%d", i);
        DataNode *bmNode = winNode->GetNode(bmName.toStdString());
        bmFound = (bmNode != 0);
        if(bmFound)
        {
            DataNode *topicNode = bmNode->GetNode("topic");
            DataNode *docNode = bmNode->GetNode("doc");
            if(topicNode && docNode)
            {
                QString topic(topicNode->AsString().c_str());
                QString doc(docNode->AsString().c_str());
                IndexMap::ConstIterator it = bookmarks.find(topic);
                if(it == bookmarks.end())
                    bookmarks[topic] = doc;
            }
        }
        ++i;
    } while(bmFound);

    // Call the base class's function.
    QvisDelayedWindow::SetFromNode(parentNode, borders);
}

// ****************************************************************************
// Method: QvisHelpWindow::TopicFromDoc
//
// Purpose: 
//   Returns the topic name given the document name.
//
// Arguments:
//   doc : The document name.
//
// Returns:    The name of the topic associated with the document.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 14:43:57 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Jun 19 16:50:58 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QString
QvisHelpWindow::TopicFromDoc(const QString &doc)
{

    QString retval;

    // Search all of the roots.
    QTreeWidgetItem *root = (QTreeWidgetItem *)helpContents->invisibleRootItem();
    bool found = false;
    for(int i = 0; i < root->childCount() && !found; ++i)
    {
        // Search the tree for the topic.
        found = TopicFromDocHelper(retval, doc, root->child(i));
    }

    return retval;
}

// ****************************************************************************
// Method: QvisHelpWindow::TopicFromDocHelper
//
// Purpose: 
//   This is a helper function for TopicFromDoc that recursively searches
//   the contents for the topic associated with the doc.
//
// Arguments:
//   str  : The return value.
//   doc  : The document name
//   item : The node that we're searching.
//
// Returns:    True if the topic was found, false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 14:44:40 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Jun 19 16:17:13 PDT 2008
//   Qt 4.
//
// ****************************************************************************

bool
QvisHelpWindow::TopicFromDocHelper(QString &str, const QString &doc,
    QTreeWidgetItem *item)
{
    bool retval = false;

    if(item->data(0,Qt::UserRole).toString() == doc)
    {
        retval = true;
        str = item->text(0);
    }
    else if(item->childCount() > 0)
    {
        for(int i = 0; i < item->childCount() && !retval; ++i)
        {
            QTreeWidgetItem *child = item->child(i);
            retval = TopicFromDocHelper(str, doc, child);
        }
    }

    return retval;
}

// ****************************************************************************
// Method: QvisHelpWindow::completeFileName
//
// Purpose: 
//   Prepends the help directory to the filename and returns the string.
//
// Arguments:
//   pagee : The name of the help page.
//
// Returns:    The entire path and filename.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 10 16:24:46 PST 2002
//
// Modifications:
//   Brad Whitlock, Mon Apr 21 15:32:04 PDT 2008
//   Added support for multiple locales.
//
// ****************************************************************************

QString
QvisHelpWindow::CompleteFileName(const QString &page) const
{
    QString file(helpPath + QString(SLASH_STRING) + 
                 locale + QString(SLASH_STRING) + 
                 page);
    if(!QFile(file).exists())
    {
        // The page did not exist for the desired locale, revert to the
        // en_US page.
        file = QString(helpPath + QString(SLASH_STRING) + 
                       QString("en_US") + QString(SLASH_STRING) + 
                       page);
    }

    return file;
}

//
// Qt Slot functions
//

// ****************************************************************************
// Method: QvisHelpWindow::show
//
// Purpose: 
//   This is a Qt slot function that shows the window.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 13:02:14 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Feb 18 15:24:42 PST 2005
//   Only display home page if we're not already showing something else.
//
// ****************************************************************************

void
QvisHelpWindow::show()
{
    QvisDelayedWindow::show();
    if(firstShow)
    {
        firstShow = false;

        // Make the window a little larger if no default size was read.
        if(!saveWindowDefaults)
            resize(int(width() * 1.5), height());

        // Set the proportions for each panel.
        int sum = 0;
        QList<int> newSizes, sizes = splitter->sizes();
        QList<int>::ConstIterator it = sizes.begin();
        while(it != sizes.end())
        {
            sum += (*it);
            ++it;
        }

        if(sum > 0)
        {
            int leftWidth = int(0.33 * sum);
            int rightWidth = sum - leftWidth;
            newSizes.append(leftWidth);
            newSizes.append(rightWidth);
            splitter->setSizes(newSizes);
        }

        // Make sure we display the home page first.
        if(helpFile.isEmpty())
            displayHome();

        // Set the enabled state of the remove bookmark button.
        removeBookmarkButton->setEnabled(bookmarks.count() > 0);
    }
}

// ****************************************************************************
// Method: QvisHelpWindow::activeTabChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the active tab changes.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 13:02:34 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Nov 19 15:51:37 PST 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisHelpWindow::activeTabChanged(int index)
{
    if(index == 1)
    {
        helpIndexText->blockSignals(true);
        helpIndexText->setText("");
        helpIndexText->blockSignals(false);
    }
    activeTab = index;
}

void
QvisHelpWindow::activateContentsTab()
{
    helpTabs->setCurrentIndex(0);
}

void
QvisHelpWindow::activateIndexTab()
{
    helpTabs->setCurrentIndex(1);
}

void
QvisHelpWindow::activateBookmarkTab()
{
    helpTabs->setCurrentIndex(2);
}

// ****************************************************************************
// Method: QvisHelpWindow::openHelp
//
// Purpose: 
//   This is a Qt slot function that is called when an item in the contents
//   listview is clicked. We display the page associated with the item.
//
// Arguments:
//   item : The item that was clicked.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 13:03:00 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Jun 19 16:27:10 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisHelpWindow::openHelp(QTreeWidgetItem *item)
{
    if(item)
    {
        QString document(item->data(0, Qt::UserRole).toString());

        if(!document.isEmpty())
            displayPage(document);
        else
            displayTitle(item->text(0));
    }
}

// ****************************************************************************
// Method: QvisHelpWindow::topicExpanded
//
// Purpose: 
//   This is a Qt slot function that is called when an item in the contents
//   listview is expanded. We display the page associated with the item and
//   make sure the item gets selected.
//
// Arguments:
//   item : The item that was clicked.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 13:03:00 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Jun 19 16:29:18 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisHelpWindow::topicExpanded(QTreeWidgetItem *item)
{
    if(item->childCount() > 0)
        item->setIcon(0, openBookIcon);

    QString document(item->data(0, Qt::UserRole).toString());
    if(!document.isEmpty())
    {
        if(document != helpFile)
            displayPage(document);
    }
    else
        displayTitle(item->text(0));

    item->setSelected(true);
}

// ****************************************************************************
// Method: QvisHelpWindow::topicCollapsed
//
// Purpose: 
//   This is a Qt slot function that is called when an item in the contents
//   listview is collapsed. We display the page associated with the item and
//   make sure the item gets selected.
//
// Arguments:
//   item : The item that was clicked.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 13:03:00 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Jun 19 16:29:33 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisHelpWindow::topicCollapsed(QTreeWidgetItem *item)
{
    if(item->childCount() > 0)
        item->setIcon(0, closedBookIcon);

    QString document(item->data(0, Qt::UserRole).toString());
    if(!document.isEmpty())
    {
        if(document != helpFile)
            displayPage(document);
    }
    else
        displayTitle(item->text(0));

    item->setSelected(true);
}

// ****************************************************************************
// Method: QvisHelpWindow::displayNoHelp
//
// Purpose: 
//   This is a Qt slot function that displays a "not found" message.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 13:05:23 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//   
// ****************************************************************************

void
QvisHelpWindow::displayNoHelp()
{
    QString html = QString("<html><body background=\"#ffffff\"><center><b><h1>") +
                   tr("Help topic not found!") + 
                   QString("</h1></b></center></body></html>");
    helpBrowser->setText(html);
    helpFile = "none";
}

// ****************************************************************************
// Method: QvisHelpWindow::displayTitle
//
// Purpose: 
//   This is a Qt slot function that displays a title.
//
// Arguments:
//   title : The title to display.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 13:05:58 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//   
// ****************************************************************************

void
QvisHelpWindow::displayTitle(const QString &title)
{
    QString html = QString("<html><body background=\"#ffffff\"><center><b><h1>") +
                   title + 
                   QString("</h1></b></center></body></html>");
    helpBrowser->setText(html);
    helpFile = "none";
}

// ****************************************************************************
// Method: QvisHelpWindow::displayCopyright
//
// Purpose: 
//   This is a Qt slot function that displays VisIt's copyright information.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 13:08:08 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Sep 11 10:20:43 PDT 2002
//   I made it display the copyright as a web page so it can be bookmarked.
//
//   Brad Whitlock, Thu Feb 17 12:11:49 PDT 2005
//   Added code to synchronize the contents.
//
// ****************************************************************************

void
QvisHelpWindow::displayCopyright()
{
    show();
    QString page("copyright.html");
    displayPage(page);
    synchronizeContents(page);
}

// ****************************************************************************
// Method: QvisHelpWindow::displayContributors
//
// Purpose: 
//   This is a Qt slot function that displays VisIt's contributors.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan  8 14:38:37 PST 2008
//
// Modifications:
//
// ****************************************************************************

void
QvisHelpWindow::displayContributors()
{
    show();
    QString page("contributors.html");
    displayPage(page);
    synchronizeContents(page);
}

// ****************************************************************************
// Method: QvisHelpWindow::displayReleaseNotesHelper
//
// Purpose: 
//   This is a Qt slot function that display's VisIt's release notes for the
//   current version of VisIt.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 13:08:36 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Feb 17 12:11:49 PDT 2005
//   Added code to synchronize the contents. I also restructured the code
//   so that the window gets created first if it has not been created so we
//   can try to update the page without actually having to show the window
//   unless the release notes are present.
//
//   Brad Whitlock, Mon Apr 21 15:08:45 PDT 2008
//   Changed how we find the release notes file.
//
// ****************************************************************************

void
QvisHelpWindow::displayReleaseNotesHelper(bool showWin)
{
    QString relnotes(ReleaseNotesFile());

    // Since we want to try and display the page before ever showing the
    // window, we have to create the window first so we won't try to display
    // the page into a bunch of uninitialized widgets. This is somewhat
    // unconventional and we only do it so we can not show the window if
    // the release notes are not found when we show release notes on startup.
    if(!isCreated)
    {
        CreateEntireWindow();
        isCreated = true;
        UpdateWindow(true);
    }

    bool showWindow = showWin;
    if(displayPage(relnotes))
    {
        synchronizeContents(relnotes);
        showWindow = true;
    }
    else
        Message(tr("The release notes file cannot be opened."));

    if(showWindow)
        show();
}

//
// Always tries to show release notes.
//
void
QvisHelpWindow::displayReleaseNotes()
{
    displayReleaseNotesHelper(true);
}

//
// Only shows release notes if they are available.
//
void
QvisHelpWindow::displayReleaseNotesIfAvailable()
{
    displayReleaseNotesHelper(false);
}

// ****************************************************************************
// Method: QvisHelpWindow::displayHome
//
// Purpose: 
//   This is a Qt slot function that displays the VisIt home page.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 13:09:19 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Feb 17 12:11:49 PDT 2005
//   Added code to synchronize the contents.
//   
// ****************************************************************************

void
QvisHelpWindow::displayHome()
{
    QString page("home.html");
    displayPage(page);
    synchronizeContents(page);
}

// ****************************************************************************
// Method: QvisHelpWindow::displayPage
//
// Purpose: 
//   This is a Qt slot function that displays a help HTML page.
//
// Arguments:
//   page   : The name of the page to display.
//   reload : Whether or not to force the page to reload.
//
// Returns:   This method return true if the page is displayed; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 13:09:48 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Sep 10 16:24:18 PST 2002
//   I made it use a new helper method.
//
//   Brad Whitlock, Fri Feb 18 13:35:31 PST 2005
//   I made it return a bool indicating whether or not it displayed the page.
//
// ****************************************************************************

bool
QvisHelpWindow::displayPage(const QString &page, bool reload)
{
    bool retval = false;

    if(page != helpFile || reload)
    {
        QString file(CompleteFileName(page));
        if(QFile(file).exists())
        {
            helpBrowser->setSource(file);
            helpFile = page;
            retval = true;
        }
        else
            displayNoHelp();
    }

    return retval;
}

// ****************************************************************************
// Method: QvisHelpWindow::synchronizeContents
//
// Purpose: 
//   Synchronizes the contents to the page being displayed.
//
// Arguments:
//   page : The name of the page being displayed.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 17 12:08:08 PDT 2005
//
// Modifications:
//   Brad Whitlock, Thu Jun 19 16:58:05 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisHelpWindow::synchronizeContents(const QString &page)
{
    // Update the contents so the right page is highlighted.
    QTreeWidgetItemIterator it(helpContents);
    while(*it)
    {
        QString document((*it)->data(0, Qt::UserRole).toString());
        if(document == page)
        {
            // Open up all of the parents until we arrive at
            // the top.
            helpContents->blockSignals(true);
            for(QTreeWidgetItem *p = (*it)->parent();
                p != 0; p = p->parent())
            {
                helpContents->expandItem(p);
                if(p->childCount() > 0)
                    p->setIcon(0, openBookIcon);
            }
            helpContents->setCurrentItem(*it);
            (*it)->setSelected(true);
//            helpContents->ensureItemVisible(it.current());
            helpContents->blockSignals(false);
            break;
        }
        ++it;
    }
}

// ****************************************************************************
// Method: QvisHelpWindow::increaseFontSize
//
// Purpose: 
//   This is Qt slot function that increases the font size of the help page.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 13:11:01 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Sep 10 16:29:49 PST 2002
//   I added code to make the page redraw with the new font size.
//
//   Kathleen Bonnell, Wed Jun 27 12:42:47 PDT 2007 
//   Removed Q_WS_WIN specific code.
//
//   Brad Whitlock, Thu Jun 19 15:09:21 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisHelpWindow::increaseFontSize()
{
    QFont f(helpBrowser->font());
    f.setPointSize(f.pointSize()+1);
    helpBrowser->setFont(f);
}

// ****************************************************************************
// Method: QvisHelpWindow::decreaseFontSize
//
// Purpose: 
//   This is Qt slot function that decreases the font size of the help page.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 13:11:01 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Sep 10 16:29:49 PST 2002
//   I added code to make the page redraw with the new font size.
//   
//   Kathleen Bonnell, Wed Jun 27 12:42:47 PDT 2007 
//   Removed Q_WS_WIN specific code.
//
//   Brad Whitlock, Thu Jun 19 15:09:33 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisHelpWindow::decreaseFontSize()
{
    QFont f(helpBrowser->font());
    int ptSize = f.pointSize()-1;
    if(ptSize > 2)
    {
        f.setPointSize(ptSize);
        helpBrowser->setFont(f);
    }
}

// ****************************************************************************
// Method: QvisHelpWindow::displayIndexTopic
//
// Purpose: 
//   This is a Qt slot function that displays the help topic when an item in
//   the index list is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 13:11:49 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Feb 17 12:09:32 PDT 2005
//   Added code to synchronize the help contents.
//
//   Brad Whitlock, Thu Jun 19 15:24:05 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisHelpWindow::displayIndexTopic()
{
    IndexMap::ConstIterator it = index.find(helpIndex->currentItem()->text());
    if(it != index.end())
    {
        helpIndexText->blockSignals(true);
        helpIndexText->setText(helpIndex->currentItem()->text());
        helpIndexText->blockSignals(false);
        displayPage(it.value());
        synchronizeContents(it.value());
    }
}

// ****************************************************************************
// Method: QvisHelpWindow::lookForIndexTopic
//
// Purpose: 
//   This is a Qt slot function that is called when the user types into the
//   index text field. The purpose is to try and match what the user types
//   with a help document. If there is a match, we display the help page.
//
// Arguments:
//   topic : The help topic typed by the user.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 13:12:37 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Feb 17 12:10:06 PDT 2005
//   Added code to synchronize contents.
//
//   Brad Whitlock, Thu Jun 19 15:24:13 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisHelpWindow::lookForIndexTopic(const QString &topic)
{
    if(topic == QString::null || topic == "")
        return;

    // Search through the helpIndex widget for a suitable match to the
    // topic that was typed.
    QString key(topic.toUpper());
    bool matchFound = false;
    size_t i;
    for(i = 0; i < helpIndex->count() && !matchFound; ++i)
    {
         QString title = helpIndex->item(i)->text().toUpper();
         if(key.length() < title.length())
             matchFound = (title.left(key.length()) == key);
         else if(key == title)
             matchFound = true;
         else
             matchFound = (key.left(title.length()) == title);
    }

    if(matchFound)
    {
         // Select the right item in the helpIndex widget
         helpIndex->blockSignals(true);
         helpIndex->setCurrentRow(i - 1);
//         helpIndex->ensureCurrentVisible();
         helpIndex->blockSignals(false);

         // Try and display a help page for the topic.
         IndexMap::ConstIterator it = index.find(helpIndex->currentItem()->text());
         if(it != index.end())
         {
             displayPage(it.value());
             synchronizeContents(it.value());
         }
    }
}

// ****************************************************************************
// Method: QvisHelpWindow::displayBookmarkTopic
//
// Purpose: 
//   This is a Qt slot function that displays a bookmarked topic.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 13:44:43 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Sep 11 11:02:38 PDT 2002
//   I fixed an error that caused bookmarks to fail sometimes.
//
//   Brad Whitlock, Thu Feb 17 12:10:42 PDT 2005
//   Added code to synchronize the contents.
//
//   Brad Whitlock, Thu Jun 19 15:29:48 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisHelpWindow::displayBookmarkTopic()
{
    IndexMap::ConstIterator it = bookmarks.find(helpBookMarks->currentItem()->text());
    if(it != bookmarks.end())
    {
        displayPage(it.value());
        synchronizeContents(it.value());
    }
}

// ****************************************************************************
// Method: QvisHelpWindow::addBookmark
//
// Purpose: 
//   This is a Qt slot function that adds a bookmark to the bookmark list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 13:34:30 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//   
//   Brad Whitlock, Thu Jun 19 15:30:47 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisHelpWindow::addBookmark()
{
    if(helpFile != "none" && helpFile != QString::null && helpFile != "")
    {
        // Search through the help contents for the topic that matches the 
        // active page.
        QString helpTopic = TopicFromDoc(helpFile);
        if(helpTopic != QString::null && helpTopic != "")
        {
            IndexMap::ConstIterator it = bookmarks.find(helpTopic);
            if(it == bookmarks.end())
            {
                 bookmarks[helpTopic] = helpFile;

                 // Add it to the bookmark widget.
                 helpBookMarks->blockSignals(true);
                 helpBookMarks->addItem(helpTopic);
                 helpBookMarks->setCurrentRow(helpBookMarks->count() - 1);
//                 helpBookMarks->ensureCurrentVisible();
                 helpBookMarks->blockSignals(false);

                 // Set the enabled state of the button.
                 removeBookmarkButton->setEnabled(true);
            }
            else
            {
                 Warning(tr("The bookmark was not added because it is already in the list."));
            }
        }
        else
            Warning(tr("The bookmark could not be added."));
    }
    else
        Warning(tr("This page cannot be bookmarked."));
}

// ****************************************************************************
// Method: QvisHelpWindow::removeBookmark
//
// Purpose: 
//   This is a Qt slot function that removes the currently selected bookmark.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 13:49:06 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Sep 11 11:21:59 PDT 2002
//   I fixed some bugs that prevented bookmarks from being removed. I also
//   removed the code to set the active bookmark since it did not match the
//   page that was displayed.
//
//   Brad Whitlock, Thu Jun 19 15:31:31 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisHelpWindow::removeBookmark()
{
    if(bookmarks.count() > 0)
    {
        QString removeTopic(helpBookMarks->currentItem()->text());
        IndexMap::Iterator it = bookmarks.find(removeTopic);
        if(it != bookmarks.end())
        {
            int item = helpBookMarks->currentRow();
            helpBookMarks->blockSignals(true);
            QListWidgetItem *ptr = helpBookMarks->takeItem(item);
            delete ptr;
            helpBookMarks->blockSignals(false);

            // Remove the bookmark from the list.
            bookmarks.remove(removeTopic);
        }
    }

    // Set the enabled state of the button.
    removeBookmarkButton->setEnabled(bookmarks.count() > 0);
}

