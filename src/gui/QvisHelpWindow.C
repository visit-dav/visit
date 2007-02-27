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

#include <stdlib.h>
#include <visit-config.h>
#include <DataNode.h>
#include <DebugStream.h>
#include <QvisHelpWindow.h>
#include <QvisHelpListViewItem.h>
#include <qaction.h>
#include <qdom.h>
#include <qfile.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtextbrowser.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qvbox.h>

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
//   
// ****************************************************************************

QvisHelpWindow::QvisHelpWindow(const char *captionString) :
    QvisDelayedWindow(captionString), helpFile(), index(), bookmarks()
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
// Method: QvisHelpWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the help window.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 12:57:59 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisHelpWindow::CreateWindowContents()
{
    QPixmap backIcon, forwardIcon, homeIcon, fontUpIcon, fontDownIcon;

    // Create some icons.
    backIcon = QPixmap(back_xpm);
    forwardIcon = QPixmap(forward_xpm);
    closedBookIcon = QPixmap(closedbook_xpm);
    openBookIcon = QPixmap(openbook_xpm);
    helpIcon = QPixmap(help_xpm);
    homeIcon = QPixmap(home_xpm);
    fontUpIcon = QPixmap(fontUp_xpm);
    fontDownIcon = QPixmap(fontDown_xpm);

    // Create a toolbar
    QToolBar *tb = new QToolBar(this);
    tb->setLabel("Help Navigation");

    // Create a splitter and add it to the layout.
    splitter = new QSplitter(central);
    splitter->setOrientation(Qt::Horizontal);
    topLayout->addWidget(splitter);

    // Create the tab widget
    helpTabs = new QTabWidget(splitter, "helpTabs");
    helpTabs->setMinimumWidth(200);
    connect(helpTabs, SIGNAL(currentChanged(QWidget *)),
            this, SLOT(activeTabChanged(QWidget *)));

    // Create the Contents tab.
    helpContents = new QListView(helpTabs, "helpContents");
    helpContents->addColumn("Contents");
    helpContents->setSorting(-1);
    connect(helpContents, SIGNAL(clicked(QListViewItem *)),
            this, SLOT(openHelp(QListViewItem *)));
    connect(helpContents, SIGNAL(collapsed(QListViewItem *)),
            this, SLOT(topicCollapsed(QListViewItem *)));
    connect(helpContents, SIGNAL(expanded(QListViewItem *)),
            this, SLOT(topicExpanded(QListViewItem *)));

    helpTabs->addTab(helpContents, tr("&Contents"));
    QAction *a = new QAction(this, tr("Contents"));
    a->setAccel(ALT + Key_C);
    connect(a, SIGNAL(activated()), this, SLOT(activateContentsTab()));

    // Create the Index tab.
    helpIndexTab = new QVBox(helpTabs, "helpIndex");
    helpIndexTab->setSpacing(5);
    helpIndexTab->setMargin(5);
    helpTabs->addTab(helpIndexTab, tr("&Index"));
    a = new QAction(this, tr("Index"));
    a->setAccel(ALT + Key_I);
    connect(a, SIGNAL(activated()), this, SLOT(activateIndexTab()));

    // Index widgets.
    helpIndexText = new QLineEdit(helpIndexTab, "helpIndexText");
    connect(helpIndexText, SIGNAL(textChanged(const QString &)),
            this, SLOT(lookForIndexTopic(const QString &)));
    helpIndex = new QListBox(helpIndexTab, "helpIndex");
    helpIndex->setSelectionMode(QListBox::Single);
    connect(helpIndex, SIGNAL(selectionChanged()),
            this, SLOT(displayIndexTopic()));

    // Create the Bookmark tab.
    helpBookmarksTab = new QVBox(helpTabs, "helpBookmarks");
    helpBookmarksTab->setSpacing(10);
    helpBookmarksTab->setMargin(5);
    helpTabs->addTab(helpBookmarksTab, tr("&Bookmarks"));
    a = new QAction(this, tr("Bookmarks"));
    a->setAccel(ALT + Key_B);
    connect(a, SIGNAL(activated()), this, SLOT(activateBookmarkTab()));

    QHBox *bookmarkHBox = new QHBox(helpBookmarksTab, "helpBookmarks");
    bookmarkHBox->setSpacing(20);
    addBookmarkButton = new QPushButton("Add", bookmarkHBox,
        "addBookmarkButton");
    connect(addBookmarkButton, SIGNAL(clicked()),
            this, SLOT(addBookmark()));
    removeBookmarkButton = new QPushButton("Remove", bookmarkHBox,
        "removeBookmarkButton");
    connect(removeBookmarkButton, SIGNAL(clicked()),
            this, SLOT(removeBookmark()));
    helpBookMarks = new QListBox(helpBookmarksTab, "helpIndex");
    helpBookMarks->setSelectionMode(QListBox::Single);
    connect(helpBookMarks, SIGNAL(selectionChanged()),
            this, SLOT(displayBookmarkTopic()));

    // Create the text browser
    helpBrowser = new QTextBrowser(splitter, "helpBrowser");
    helpBrowser->setMinimumWidth(300);
    QFont f(helpBrowser->font());
    f.setBold(false);
    helpBrowser->setFont(f);

#if QT_VERSION >= 300
    connect(helpBrowser, SIGNAL(linkClicked(const QString &)),
            helpBrowser, SLOT(setSource(const QString &)));
#endif

    QToolButton *backButton = new QToolButton(QIconSet(backIcon), "Back",
        "Back", helpBrowser, SLOT(backward()), tb, "backButton");
    backButton->setAccel(CTRL + Key_Prior);
    backButton->setUsesBigPixmap(true);
    backButton->setUsesTextLabel(true);
    connect(helpBrowser, SIGNAL(backwardAvailable(bool)),
            backButton, SLOT(setEnabled(bool)));

    tb->addSeparator();

    QToolButton *forwardButton = new QToolButton(QIconSet(forwardIcon), "Forward",
        "Forward", helpBrowser, SLOT(forward()), tb, "forwardButton");
    forwardButton->setAccel(CTRL + Key_Next);
    forwardButton->setUsesBigPixmap(true);
    forwardButton->setUsesTextLabel(true);
    connect(helpBrowser, SIGNAL(backwardAvailable(bool)),
            forwardButton, SLOT(setEnabled(bool)));

    tb->addSeparator();

    QToolButton *homeButton = new QToolButton(QIconSet(homeIcon), "Home",
        "Home", this, SLOT(displayHome()), tb, "homeButton");
    homeButton->setAccel(CTRL + Key_Home);
    homeButton->setUsesBigPixmap(true);
    homeButton->setUsesTextLabel(true);

    tb->addSeparator();

    // buttons to change the font size.
    QToolButton *upFont = new QToolButton(QIconSet(fontUpIcon),"Larger font",
        "Larger font", this, SLOT(increaseFontSize()), tb, "upFont");
    upFont->setUsesTextLabel(true);
    upFont->setUsesBigPixmap(true);

    tb->addSeparator();

    QToolButton *downFont = new QToolButton(QIconSet(fontDownIcon),"Smaller font",
        "Smaller font", this, SLOT(decreaseFontSize()), tb, "downFont");
    downFont->setUsesTextLabel(true);
    downFont->setUsesBigPixmap(true);

    tb->addSeparator();

    QToolButton *addBookmarkTB = new QToolButton(QIconSet(openBookIcon),"Add Bookmark",
        "Add Bookmark", this, SLOT(addBookmark()), tb, "addBookmarkTB");
    addBookmarkTB->setUsesTextLabel(true);
    addBookmarkTB->setUsesBigPixmap(true);

    // Create the Dismiss button
    QHBoxLayout *buttonLayout = new QHBoxLayout(topLayout);
    QPushButton *dismissButton = new QPushButton("Dismiss", central, "dismissButton");
    connect(dismissButton, SIGNAL(clicked()), this, SLOT(hide()));
    buttonLayout->addStretch(10);
    buttonLayout->addWidget(dismissButton);

    // Try and load the help index file.
    QString indexFile;
    indexFile = helpPath + QString("/visit.helpml");
    LoadHelp(indexFile);

    // Show the appropriate page based on the activeTab setting.
    helpTabs->blockSignals(true);
    if(activeTab == 0)
        helpTabs->showPage(helpContents);
    else if(activeTab == 1)
        helpTabs->showPage(helpIndexTab);
    else
        helpTabs->showPage(helpBookmarksTab);
    helpTabs->blockSignals(false);
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
// ****************************************************************************

void
QvisHelpWindow::LoadHelp(const QString &fileName)
{
    helpContents->blockSignals(true);

    // Read the XML file and create the DOM tree. Then use the tree to
    // build the User manual content.
    bool noHelp = false;
    QFile helpIndexFile(fileName);
    if(helpIndexFile.open(IO_ReadOnly))
    {
        QDomDocument domTree;
        if(domTree.setContent(&helpIndexFile))
        {
            // Create a root node for the User's manual.
            QvisHelpListViewItem *UMrootItem = new QvisHelpListViewItem(
                helpContents, 0);
            UMrootItem->setText(0, "VisIt User's Manual");
            UMrootItem->setDocument("list0000.html");
            UMrootItem->setPixmap(0, openBookIcon);

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
            helpContents->setOpen(UMrootItem, true);
        }
        else
            noHelp = true;
        helpIndexFile.close();
    }
    else
        noHelp = true;

    if(noHelp)
    {
        Message("VisIt cannot read the help index file! "
                "No online help will be available.");
        debug1 << "VisIt cannot read the help index file! "
                  "No online help will be available.\n";
    }

    // Create a root node for the VisIt FAQ page.
    QvisHelpListViewItem *copyrightPage = new QvisHelpListViewItem(
        helpContents, 0);
    copyrightPage->setText(0, "Copyright");
    copyrightPage->setDocument("copyright.html");
    copyrightPage->setPixmap(0, helpIcon);

    QvisHelpListViewItem *faqPage = new QvisHelpListViewItem(
        helpContents, 0);
    faqPage->setText(0, "Frequently asked questions");
    faqPage->setDocument("faq.html");
    faqPage->setPixmap(0, helpIcon);

    // Create a root node for the VisIt args page.
    QvisHelpListViewItem *argsPage = new QvisHelpListViewItem(
        helpContents, 0);
    argsPage->setText(0, "Command line arguments");
    argsPage->setDocument("args.html");
    argsPage->setPixmap(0, helpIcon);

    // Create a root node for the VisIt release notes.
    QvisHelpListViewItem *releaseNotes = new QvisHelpListViewItem(
        helpContents, 0);
    QString relNotes;
    relNotes.sprintf("relnotes%s.html", VERSION);
    releaseNotes->setText(0, "Release Notes");
    releaseNotes->setDocument(relNotes);
    releaseNotes->setPixmap(0, helpIcon);

    // Create a root node for the VisIt home page.
    QvisHelpListViewItem *homePage = new QvisHelpListViewItem(
        helpContents, 0);
    homePage->setText(0, "VisIt home page");
    homePage->setDocument("home.html");
    homePage->setPixmap(0, helpIcon);

    helpContents->blockSignals(false);

    // Make the VisIt home page selected by default.
    helpContents->setSelected(homePage, true);

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
//   
// ****************************************************************************

void
QvisHelpWindow::BuildContents(QListViewItem *parentItem,
    const QDomElement &parentElement)
{
    QvisHelpListViewItem *thisItem = 0;
    QDomNode node = parentElement.firstChild();
    while(!node.isNull())
    {
        if(node.isElement() && node.nodeName() == "help")
        {
            // add a new list view item for the outline
            if(parentItem == 0)
                thisItem = new QvisHelpListViewItem(helpContents, thisItem);
            else
                thisItem = new QvisHelpListViewItem(parentItem, thisItem);
        
            thisItem->setText(0, node.toElement().attribute("topic"));
            thisItem->setDocument(node.toElement().attribute("doc"));

            // Add the topic to the index.
            AddToIndex(node.toElement().attribute("topic"),
                       node.toElement().attribute("doc"));

            // recursive build of the tree
            BuildContents(thisItem, node.toElement());

            if(thisItem->childCount() > 0)
                thisItem->setPixmap(0, closedBookIcon);
            else
                thisItem->setPixmap(0, helpIcon);
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
// ****************************************************************************

void
QvisHelpWindow::BuildIndex()
{
    // Add a few more items to the index.
    AddToIndex("Copyright", "copyright.html");
    AddToIndex("Command line arguments", "args.html");
    AddToIndex("Frequently asked questions", "faq.html");
    AddToIndex("FAQ", "faq.html");
    AddToIndex("VisIt", "home.html");
    QString str; str.sprintf("relnotes%s.html", VERSION);
    AddToIndex("Release notes", str);

    // Populate the index list box.
    helpIndex->blockSignals(true);
    IndexMap::ConstIterator it;
    for(it = index.begin(); it != index.end(); ++it)
    {
         helpIndex->insertItem(it.key());
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
        filteredString = filteredString.left(1).upper() +
                         filteredString.right(filteredString.length() - 1);

        // Insert the item into the index.
        bool inserted = false;
        for(int j = 0; !inserted; ++j)
        {
            QString key(filteredString);
            if(j > 0)
                key.sprintf("%s (%d)", filteredString.latin1(), j+1);

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
        helpBookMarks->insertItem(it.key());
    }

    if(helpBookMarks->count() > 0)
    {
         helpBookMarks->blockSignals(true);
         helpBookMarks->setCurrentItem(0);
         helpBookMarks->ensureCurrentVisible();
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
//   
// ****************************************************************************

void
QvisHelpWindow::CreateNode(DataNode *parentNode)
{
    // Call the base class's method to save the generic window attributes.
    QvisDelayedWindow::CreateNode(parentNode);

    if(saveWindowDefaults)
    {
        DataNode *node = parentNode->GetNode(caption().latin1());

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
            DataNode *bmNode = new DataNode(bmName.latin1());

            // Add the topic and the doc to the bookmark node.
            bmNode->AddNode(new DataNode("topic", std::string(it.key().latin1())));
            bmNode->AddNode(new DataNode("doc", std::string(it.data().latin1())));

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
//   
// ****************************************************************************

void
QvisHelpWindow::SetFromNode(DataNode *parentNode, const int *borders)
{
    DataNode *winNode = parentNode->GetNode(std::string(caption().latin1()));
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
        DataNode *bmNode = winNode->GetNode(bmName.latin1());
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
//   
// ****************************************************************************

QString
QvisHelpWindow::TopicFromDoc(const QString &doc)
{
    QString retval;

    // Search all of the roots.
    QvisHelpListViewItem *root = (QvisHelpListViewItem *)helpContents->firstChild();
    bool found = false;
    while(root != 0 && !found)
    {
        // Search the tree for the topic.
        found = TopicFromDocHelper(retval, doc, root);
        root = (QvisHelpListViewItem *)root->nextSibling();
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
//   
// ****************************************************************************

bool
QvisHelpWindow::TopicFromDocHelper(QString &str, const QString &doc,
    QvisHelpListViewItem *item)
{
    bool retval = false;

    if(item->document() == doc)
    {
        retval = true;
        str = item->text(0);
    }
    else if(item->childCount() > 0)
    {
        QListViewItem *child = item->firstChild();
        for(int i = 0; i < item->childCount() && !retval; ++i)
        {
            retval = TopicFromDocHelper(str, doc, (QvisHelpListViewItem *)child);
            child = child->nextSibling();
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
//   
// ****************************************************************************

QString
QvisHelpWindow::CompleteFileName(const QString &page) const
{
    QString file;
    file.sprintf("%s%s%s", helpPath.latin1(), SLASH_STRING, page.latin1());
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
        QValueList<int> newSizes, sizes = splitter->sizes();
        QValueList<int>::ConstIterator it = sizes.begin();
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
//   
// ****************************************************************************

void
QvisHelpWindow::activeTabChanged(QWidget *)
{
    int index = helpTabs->currentPageIndex();
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
    helpTabs->setCurrentPage(0);
}

void
QvisHelpWindow::activateIndexTab()
{
    helpTabs->setCurrentPage(1);
}

void
QvisHelpWindow::activateBookmarkTab()
{
    helpTabs->setCurrentPage(2);
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
//   
// ****************************************************************************

void
QvisHelpWindow::openHelp(QListViewItem *item)
{
    if(item)
    {
        QvisHelpListViewItem *item2 = (QvisHelpListViewItem *)item;

        if(item2->document() != QString::null)
            displayPage(item2->document());
        else
            displayTitle(item2->text(0));
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
//   
// ****************************************************************************

void
QvisHelpWindow::topicExpanded(QListViewItem *item)
{
    if(item->childCount() > 0)
        item->setPixmap(0, openBookIcon);

    QvisHelpListViewItem *item2 = (QvisHelpListViewItem *)item;
    if(item2->document() != QString::null)
    {
        if(item2->document() != helpFile)
            displayPage(item2->document());
    }
    else
        displayTitle(item2->text(0));

    helpContents->setSelected(item, true);
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
//   
// ****************************************************************************

void
QvisHelpWindow::topicCollapsed(QListViewItem *item)
{
    if(item->childCount() > 0)
        item->setPixmap(0, closedBookIcon);

    QvisHelpListViewItem *item2 = (QvisHelpListViewItem *)item;
    if(item2->document() != QString::null)
    {
        if(item2->document() != helpFile)
            displayPage(item2->document());        
    }
    else
        displayTitle(item2->text(0));

    helpContents->setSelected(item, true);
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
//   
// ****************************************************************************

void
QvisHelpWindow::displayNoHelp()
{
    const char *html = "<html><body background=\"#ffffff\"><center><b><h1>Help topic not found!</h1></b></center></body></html>";
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
//   
// ****************************************************************************

void
QvisHelpWindow::displayTitle(const QString &title)
{
    const char *fmt = "<html><body background=\"#ffffff\"><center><b><h1>%s</h1></b></center></body></html>";
    QString msg;
    msg.sprintf(fmt, title.latin1());
    helpBrowser->setText(msg);
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
// ****************************************************************************

void
QvisHelpWindow::displayReleaseNotesHelper(bool showWin)
{
    QString relnotes;
    relnotes.sprintf("relnotes%s.html", VERSION);

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
        Message("The release notes file cannot be opened.");

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
//   
// ****************************************************************************

void
QvisHelpWindow::synchronizeContents(const QString &page)
{
    // Update the contents so the right page is highlighted.
    QListViewItemIterator it(helpContents);
    for( ; it.current(); ++it)
    {
        QvisHelpListViewItem *item =
            (QvisHelpListViewItem *)it.current();
        if(item->document() == page)
        {
            // Open up all of the parents until we arrive at
            // the top.
            helpContents->blockSignals(true);
            for(QListViewItem *p = it.current()->parent();
                p != 0; p = p->parent())
            {
                p->setOpen(true);
                if(p->childCount() > 0)
                    p->setPixmap(0, openBookIcon);
            }
            helpContents->setCurrentItem(it.current());
            helpContents->setSelected(it.current(), true);
            helpContents->ensureItemVisible(it.current());
            helpContents->blockSignals(false);
            break;
        }
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
// ****************************************************************************

void
QvisHelpWindow::increaseFontSize()
{
#ifdef Q_WS_WIN
    QFont f(helpBrowser->font());
    f.setPointSize(f.pointSize()/2+1);
    helpBrowser->setFont(f);
#else
    QFont f(helpBrowser->font());
    f.setPointSize(f.pointSize()+1);
    helpBrowser->setFont(f);
#if QT_VERSION < 300
    helpBrowser->setUpdatesEnabled(false);
    helpBrowser->blockSignals(true);
    QString source(helpBrowser->source());
    helpBrowser->setSource("blank.html");
    helpBrowser->setSource(source);
    helpBrowser->blockSignals(false);
    helpBrowser->setUpdatesEnabled(true);
    helpBrowser->repaint();
#endif
#endif
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
// ****************************************************************************

void
QvisHelpWindow::decreaseFontSize()
{
#ifdef Q_WS_WIN
    QFont f(helpBrowser->font());
    int ptSize = f.pointSize()/2-1;
    f.setPointSize(ptSize);
    helpBrowser->setFont(f);
#else
    QFont f(helpBrowser->font());
    int ptSize = f.pointSize()-1;
    if(ptSize > 2)
    {
        f.setPointSize(ptSize);
        helpBrowser->setFont(f);
#if QT_VERSION < 300
        helpBrowser->setUpdatesEnabled(false);
        helpBrowser->blockSignals(true);
        QString source(helpBrowser->source());
        helpBrowser->setSource("blank.html");
        helpBrowser->setSource(source);
        helpBrowser->blockSignals(false);
        helpBrowser->setUpdatesEnabled(true);
        helpBrowser->repaint();
#endif
    }
#endif
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
// ****************************************************************************

void
QvisHelpWindow::displayIndexTopic()
{
    IndexMap::ConstIterator it = index.find(helpIndex->currentText());
    if(it != index.end())
    {
        helpIndexText->blockSignals(true);
        helpIndexText->setText(helpIndex->currentText());
        helpIndexText->blockSignals(false);
        displayPage(it.data());
        synchronizeContents(it.data());
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
// ****************************************************************************

void
QvisHelpWindow::lookForIndexTopic(const QString &topic)
{
    if(topic == QString::null || topic == "")
        return;

    // Search through the helpIndex widget for a suitable match to the
    // topic that was typed.
    QString key(topic.upper());
    bool matchFound = false;
    int i;
    for(i = 0; i < helpIndex->count() && !matchFound; ++i)
    {
         QString title = helpIndex->text(i).upper();
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
         helpIndex->setCurrentItem(i - 1);
         helpIndex->ensureCurrentVisible();
         helpIndex->blockSignals(false);

         // Try and display a help page for the topic.
         IndexMap::ConstIterator it = index.find(helpIndex->currentText());
         if(it != index.end())
         {
             displayPage(it.data());
             synchronizeContents(it.data());
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
// ****************************************************************************

void
QvisHelpWindow::displayBookmarkTopic()
{
    IndexMap::ConstIterator it = bookmarks.find(helpBookMarks->currentText());
    if(it != bookmarks.end())
    {
        displayPage(it.data());
        synchronizeContents(it.data());
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
                 helpBookMarks->insertItem(helpTopic);
                 helpBookMarks->setCurrentItem(helpBookMarks->count() - 1);
                 helpBookMarks->ensureCurrentVisible();
                 helpBookMarks->blockSignals(false);

                 // Set the enabled state of the button.
                 removeBookmarkButton->setEnabled(true);
            }
            else
            {
                 Warning("The bookmark was not added because it is already in the list.");
            }
        }
        else
            Warning("The bookmark could not be added.");
    }
    else
        Warning("This page cannot be bookmarked.");
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
// ****************************************************************************

void
QvisHelpWindow::removeBookmark()
{
    if(bookmarks.count() > 0)
    {
        QString removeTopic(helpBookMarks->currentText());
        IndexMap::Iterator it = bookmarks.find(removeTopic);
        if(it != bookmarks.end())
        {
            int item = helpBookMarks->currentItem();
            helpBookMarks->blockSignals(true);
            helpBookMarks->removeItem(item);
            helpBookMarks->blockSignals(false);

            // Remove the bookmark from the list.
            bookmarks.remove(removeTopic);
        }
    }

    // Set the enabled state of the button.
    removeBookmarkButton->setEnabled(bookmarks.count() > 0);
}

