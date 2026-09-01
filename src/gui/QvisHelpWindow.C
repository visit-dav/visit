// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <visit-config.h>
#include <InstallationFunctions.h>

#include <QvisHelpWindow.h>
#include <QAction>
#include <QByteArray>
#include <QDesktopServices>
#include <QFile>
#include <QFont>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QList>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPixmap>
#include <QPushButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QStringList>
#include <QTextBrowser>
#include <QToolBar>
#include <QUrl>
#include <QVBoxLayout>

#include <icons/fontDown.xpm>
#include <icons/fontUp.xpm>

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
//   Brad Whitlock, Fri Oct 12 16:43:20 PDT 2012
//   I moved help under resources.
//
//   Alister Maguire, Wed Nov  6 08:11:16 PST 2019
//   Added manualPath for accessing the sphinx manual.
//
//   Kathleen Biagas, Wed June 24, 2026
//   Initialize startOptsPath.
//
//   Kathleen Biagas, Mon Aug 31, 2026
//   Restructure window for simplicity.
//   Remove startOptsPath, Add 'about', 'getting_help'.
//
// ****************************************************************************

QvisHelpWindow::QvisHelpWindow(const QString &captionString) :
    QvisDelayedWindow(captionString),
    locale(),
    helpPath(QString(GetVisItResourcesDirectory(VISIT_RESOURCES_HELP).c_str())),
    aboutPath("about"),
    manualPath(QString("manual") + QString(VISIT_SLASH_STRING) +
        QString("index.html")),
    gettingHelpPath(QString("manual") + QString(VISIT_SLASH_STRING) +
        QString("getting_help") + QString(VISIT_SLASH_STRING) +
        QString("index.html")),
    currentPage(),
    topics(0),
    pageStack(0),
    browser(0),
    externalLinkLabel(0),
    splitter(0),
    fontUpAction(0),
    fontDownAction(0),
    firstShow(true)
{
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
//   Brad Whitlock, Tue Aug 31 10:32:50 PDT 2010
//   I changed how links get clicked through. I also increased min window size.
//
//   Kathleen Biagas, Mon Aug 31, 2026
//   Removed navigation icons and their actions (forward, back, home).
//   Removed tabwidget, treewidget, index, bookmarks.
//
// ****************************************************************************

void
QvisHelpWindow::CreateWindowContents()
{
    // Create some icons.
    QIcon fontUpIcon = QIcon(QPixmap(fontUp_xpm));
    QIcon fontDownIcon = QIcon(QPixmap(fontDown_xpm));

    // Create a toolbar
    QToolBar *toolbar = addToolBar(tr("Help"));

    fontUpAction = toolbar->addAction(fontUpIcon, tr("Larger font"),
        this, SLOT(increaseFontSize()));
    fontDownAction = toolbar->addAction(fontDownIcon, tr("Smaller font"),
        this, SLOT(decreaseFontSize()));
    updateFontSizeActions(QString());

    // Create a splitter and add it to the layout.
    splitter = new QSplitter(central);
    splitter->setOrientation(Qt::Horizontal);
    topLayout->addWidget(splitter);

    topics = new QListWidget(splitter);
    topics->setMinimumWidth(220);
    AddTopic(tr("Release notes"), ReleaseNotesFile());
    AddTopic(tr("Manual"), manualPath);
    AddTopic(tr("Getting Help"), gettingHelpPath);
    AddTopic(tr("Copyright"), "copyright.html");
    AddTopic(tr("VisIt home page"), "home");
    AddTopic(tr("About"), aboutPath);
    connect(topics, SIGNAL(itemActivated(QListWidgetItem *)),
            this, SLOT(displaySelectedTopic(QListWidgetItem *)));
    connect(topics, SIGNAL(itemClicked(QListWidgetItem *)),
            this, SLOT(displaySelectedTopic(QListWidgetItem *)));

    pageStack = new QStackedWidget(splitter);
    pageStack->setMinimumWidth(400);
    pageStack->setMinimumHeight(500);

    browser = new QTextBrowser(pageStack);
    browser->setOpenLinks(false);
    QFont f(browser->font());
    f.setBold(false);
    browser->setFont(f);
    pageStack->addWidget(browser);
    connect(browser, SIGNAL(anchorClicked(const QUrl &)),
            this, SLOT(anchorClicked(const QUrl &)));

    externalLinkLabel = new QLabel(pageStack);
    externalLinkLabel->setAlignment(Qt::AlignCenter);
    externalLinkLabel->setOpenExternalLinks(true);
    externalLinkLabel->setTextFormat(Qt::RichText);
    externalLinkLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    externalLinkLabel->setWordWrap(true);
    pageStack->addWidget(externalLinkLabel);

    splitter->setStretchFactor(1, 10);

    // Create the Dismiss button
    QHBoxLayout *buttonLayout = new QHBoxLayout(0);
    topLayout->addLayout(buttonLayout);
    QPushButton *dismissButton = new QPushButton(tr("Dismiss"), central);
    connect(dismissButton, SIGNAL(clicked()), this, SLOT(hide()));
    buttonLayout->addStretch(10);
    buttonLayout->addWidget(dismissButton);
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
    QString relNotes, ver(VISIT_VERSION);
    if(ver.right(1) == "b")
        ver = ver.left(ver.length()-1);
    relNotes = QString("relnotes") + ver + QString(".html");
    return relNotes;
}

// ****************************************************************************
// Method: QvisHelpWindow::CompleteFileName
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
    QString file(helpPath + QString(VISIT_SLASH_STRING) +
                 locale + QString(VISIT_SLASH_STRING) +
                 page);
    if(!QFile(file).exists())
    {
        // The page did not exist for the desired locale, revert to the
        // en_US page.
        file = QString(helpPath + QString(VISIT_SLASH_STRING) +
                       QString("en_US") + QString(VISIT_SLASH_STRING) +
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

        if(currentPage.isEmpty())
            displayPage(ReleaseNotesFile());
    }
}


// ****************************************************************************
// Method: QvisHelpWindow::openHelp
//
// Purpose:
//   This is a Qt slot function that is called when an item in the contents
//   listview is clicked. We display the page associated with the item.
//
// Arguments:
//   entry : The item that was clicked.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 12 13:03:00 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Jun 19 16:27:10 PDT 2008
//   Qt 4.
//
//   Jeremy Meredith, Fri Sep 20 11:56:56 EDT 2013
//   Updated help searching logic.
//
//   Kathleen Biagas, Mon Aug 31, 2026
//   Items now stored in QListWidget.
//
// ****************************************************************************

void
QvisHelpWindow::openHelp(const QString &entry)
{
    show();

    if(entry.isEmpty())
    {
        displayPage(ReleaseNotesFile());
        return;
    }

    QStringList words = entry.split(' ');
    while(words.size() > 0)
    {
        QString query = words.join(" ");
        QString queryNoSpaces = words.join("");

        for(int i = 0; i < topics->count(); ++i)
        {
            QListWidgetItem *item = topics->item(i);
            QString title(item->text());
            if(title.startsWith(query, Qt::CaseInsensitive) ||
               title.contains(query, Qt::CaseInsensitive) ||
               title.contains(queryNoSpaces, Qt::CaseInsensitive))
            {
                topics->scrollToItem(item);
                topics->setCurrentItem(item);
                displayPage(item->data(Qt::UserRole).toString());
                return;
            }
        }

        words.pop_back();
    }

    displayTitle(entry + tr(" help not found..."));
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
//   Kathleen Biagas, Mon Aug 31, 2026
//   Set currentPage to empty string, clear selection on topics, update font
//   controls.
//
// ****************************************************************************

void
QvisHelpWindow::displayNoHelp()
{
    QString html = QString("<html><body bgcolor=\"#ffffff\"><center><b><h1>") +
                   tr("Help topic not found!") +
                   QString("</h1></b></center></body></html>");
    browser->setText(html);
    pageStack->setCurrentWidget(browser);
    currentPage = QString();
    topics->clearSelection();
    updateFontSizeActions(currentPage);
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
//   Kathleen Biagas, Mon Aug 31, 2026
//   Set currentPage to empty string, clear selection on topics, update font
//   controls.
//
// ****************************************************************************

void
QvisHelpWindow::displayTitle(const QString &title)
{
    QString html = QString("<html><body bgcolor=\"#ffffff\"><center><b><h1>") +
                   title +
                   QString("</h1></b></center></body></html>");
    browser->setText(html);
    pageStack->setCurrentWidget(browser);
    currentPage = QString();
    topics->clearSelection();
    updateFontSizeActions(currentPage);
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
//   Kathleen Biagas, Mon Aug 31, 2026
//   Remove synchronization.
//
// ****************************************************************************

void
QvisHelpWindow::displayCopyright()
{
    show();
    displayPage("copyright.html");
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
//   Kathleen Biagas, Mon Aug 31, 2026
//   Test for availability of Release notes first.
//
// ****************************************************************************

void
QvisHelpWindow::displayReleaseNotesHelper(bool showWin)
{
    QString relnotes(ReleaseNotesFile());
    if(!QFile(CompleteFileName(relnotes)).exists())
    {
        if(showWin)
            Message(tr("The release notes file cannot be opened."));
        return;
    }

    if(!isCreated)
    {
        CreateEntireWindow();
        isCreated = true;
        UpdateWindow(true);
    }

    displayPage(relnotes);
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
// Method: QvisHelpWindow::displayPage
//
// Purpose:
//   This is a Qt slot function that displays a help HTML page.
//
// Arguments:
//   page   : The name of the page to display.
//   externalBrowser : Whether or not to force the page to load in external browser.
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
//   Kathleen Bonnell, Thu Apr  8 17:20:52 PST 2010
//   Convert file to url so it will work on windows.
//
//   Kathleen Biagas, Wed June 24, 2026
//   Removed no-longer-used 'reload' arg. Added 'externalBrowswer' arg.
//
//   Kathleen Biagas, Mon Aug 31, 2026
//   Removed no-longer-used 'externalBrowswer' arg. Use 'displayExternalLink'
//   helper. Add 'about', 'home', 'getting help'.
//
// ****************************************************************************

bool
QvisHelpWindow::displayPage(const QString &page)
{
    if(page == aboutPath)
    {
        displayAbout();
        return true;
    }

    if(page == QString("home"))
        return displayExternalLink(tr("VisIt home page"),
            QUrl("https://visit-dav.github.io/visit-website/"),
            tr("opens in external browser"), page);

    if(page == manualPath)
        return displayExternalLink(tr("Manual"), page);

    if(page == gettingHelpPath)
        return displayExternalLink(tr("Getting Help"), page);

    if(page == currentPage)
    {
        pageStack->setCurrentWidget(browser);
        updateFontSizeActions(page);
        return true;
    }

    QString file(CompleteFileName(page));
    if(!QFile(file).exists())
    {
        displayNoHelp();
        return false;
    }

    pageStack->setCurrentWidget(browser);
    browser->setSource(QUrl::fromLocalFile(file));
    currentPage = page;
    selectPage(currentPage);
    updateFontSizeActions(currentPage);
    return true;
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
//   Kathleen Biagas, Mon Aug 31, 2026
//   'helpBrowser' is now simply 'browser'.
//
// ****************************************************************************

void
QvisHelpWindow::increaseFontSize()
{
    QFont f(browser->font());
    f.setPointSize(f.pointSize()+1);
    browser->setFont(f);
}

// ****************************************************************************
// Method: QvisHelpWindow::decreaseFontSize
//
// Purpose:
//   Decrease the browser font size.
//
// Programmer: Kathleen Biagas
// Creation:   Mon Aug 31, 2026
//
// Modifications:
//   Kathleen Biagas, Mon Aug 31, 2026
//   'helpBrowser' is now simply 'browser'.
//
// ****************************************************************************

void
QvisHelpWindow::decreaseFontSize()
{
    QFont f(browser->font());
    int ptSize = f.pointSize()-1;
    if(ptSize > 2)
    {
        f.setPointSize(ptSize);
        browser->setFont(f);
    }
}


// ****************************************************************************
// Method: QvisHelpWindow::anchorClicked
//
// Purpose:
//   Set the source of the help browser when a link is clicked. For external
//   links, we open the user's default web browser.
//
// Arguments:
//   link : The link that was clicked.
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 31 10:31:35 PDT 2010
//
// Modifications:
//   Kathleen Biagas, Mon Aug 31, 2026
//   'helpBrowser' is now simply 'browser'.
//
// ****************************************************************************

void
QvisHelpWindow::anchorClicked(const QUrl &link)
{
    if(link.scheme() != "file")
        QDesktopServices::openUrl(link);
    else
        browser->setSource(link);
}


// ****************************************************************************
// Method: QvisHelpWindow::AddTopic
//
// Purpose:
//   Add one fixed topic to the topic list.
//
// Programmer: Kathleen Biagas
// Creation:   Mon Aug 31, 2026
//
// ****************************************************************************

void
QvisHelpWindow::AddTopic(const QString &title, const QString &page)
{
    QListWidgetItem *item = new QListWidgetItem(title, topics);
    item->setData(Qt::UserRole, page);
}


// ****************************************************************************
// Method: QvisHelpWindow::displaySelectedTopic
//
// Purpose:
//   Display the selected topic.
//
// Programmer: Kathleen Biagas
// Creation:   Mon Aug 31, 2026
//
// ****************************************************************************

void
QvisHelpWindow::displaySelectedTopic(QListWidgetItem *item)
{
    if(item)
        displayPage(item->data(Qt::UserRole).toString());
}


// ****************************************************************************
// Method: QvisHelpWindow::displayExternalLink
//
// Purpose:
//   Display a clickable local help page link for the external browser.
//
// Programmer: Kathleen Biagas
// Creation:   Mon Aug 31, 2026
//
// ****************************************************************************

bool
QvisHelpWindow::displayExternalLink(const QString &title, const QString &page)
{
    QString file(CompleteFileName(page));
    if(!QFile(file).exists())
    {
        displayNoHelp();
        return false;
    }

    return displayExternalLink(title, QUrl::fromLocalFile(file),
        tr("opens in external browser"), page);
}


// ****************************************************************************
// Method: QvisHelpWindow::displayExternalLink
//
// Purpose:
//   Display a clickable URL for the external browser.
//
// Programmer: Kathleen Biagas
// Creation:   Mon Aug 31, 2026
//
// ****************************************************************************

bool
QvisHelpWindow::displayExternalLink(const QString &title, const QUrl &url,
    const QString &statusText, const QString &page)
{
    QString encodedUrl(QString::fromLatin1(url.toEncoded().constData()));
    QString html = QString("<html><body bgcolor=\"#ffffff\"><center>") +
                   QString("<b><h2><a href=\"") +
                   encodedUrl +
                   QString("\">") +
                   title +
                   QString("</a></h2></b><h3>") +
                   statusText +
                   QString("</h3></center></body></html>");

    externalLinkLabel->setText(html);
    pageStack->setCurrentWidget(externalLinkLabel);
    currentPage = page;
    selectPage(currentPage);
    updateFontSizeActions(currentPage);
    return true;
}


// ****************************************************************************
// Method: QvisHelpWindow::displayAbout
//
// Purpose:
//   Show the About dialog and mark the About topic current.
//
// Programmer: Kathleen Biagas
// Creation:   Mon Aug 31, 2026
//
// ****************************************************************************

void
QvisHelpWindow::displayAbout()
{
    QString html = QString("<html><body bgcolor=\"#ffffff\"><center><b><h1>") +
                   tr("About") +
                   QString("</h1></b></center></body></html>");
    browser->setText(html);
    pageStack->setCurrentWidget(browser);
    currentPage = aboutPath;
    selectPage(currentPage);
    updateFontSizeActions(currentPage);

    emit showAbout();
}


// ****************************************************************************
// Method: QvisHelpWindow::selectPage
//
// Purpose:
//   Select the topic associated with a logical page.
//
// Programmer: Kathleen Biagas
// Creation:   Mon Aug 31, 2026
//
// ****************************************************************************

void
QvisHelpWindow::selectPage(const QString &page)
{
    topics->blockSignals(true);
    topics->clearSelection();

    for(int i = 0; i < topics->count(); ++i)
    {
        QListWidgetItem *item = topics->item(i);
        if(item->data(Qt::UserRole).toString() == page)
        {
            topics->setCurrentItem(item);
            item->setSelected(true);
            break;
        }
    }

    topics->blockSignals(false);
}


// ****************************************************************************
// Method: QvisHelpWindow::updateFontSizeActions
//
// Purpose:
//   Enable font controls only for pages displayed in the text browser.
//
// Programmer: Kathleen Biagas
// Creation:   Mon Aug 31, 2026
//
// ****************************************************************************

void
QvisHelpWindow::updateFontSizeActions(const QString &page)
{
    bool enabled = (page == ReleaseNotesFile() ||
                    page == QString("copyright.html"));

    fontUpAction->setEnabled(enabled);
    fontDownAction->setEnabled(enabled);
}

