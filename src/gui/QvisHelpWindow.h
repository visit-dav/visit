// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_HELP_WINDOW_H
#define QVIS_HELP_WINDOW_H
#include <gui_exports.h>
#include <QMap>
#include <QIcon>
#include <QvisDelayedWindow.h>

class QAction;
class QDomElement;
class QLineEdit;
class QListWidget;
class QPushButton;
class QSplitter;
class QTabWidget;
class QTextBrowser;
class QTreeWidget;
class QTreeWidgetItem;
class QUrl;
class QWidget;

// ****************************************************************************
// Class: QvisHelpWindow
//
// Purpose:
//   This class creates a help window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed May 15 12:00:16 PDT 2002
//
// Modifications:
//   Brad Whitlock, Wed Jul 10 17:51:16 PST 2002
//   Finished it.
//
//   Brad Whitlock, Tue Sep 10 16:23:09 PST 2002
//   Added an internal convenience method.
//
//   Brad Whitlock, Thu Feb 17 12:14:33 PDT 2005
//   Added synchronizeContents.
//
//   Brad Whitlock, Tue Jan  8 14:37:12 PST 2008
//   Added displayContributors.
//
//   Brad Whitlock, Wed Apr  9 11:28:42 PDT 2008
//   QString for captionString.
//
//   Brad Whitlock, Mon Apr 21 15:26:37 PDT 2008
//   Added helper methods and the SetLocale method.
//
//   Brad Whitlock, Thu Jun 19 13:55:53 PDT 2008
//   Qt 4.
//
//   Alister Maguire, Wed Nov  6 08:11:16 PST 2019
//   Added manualPath.
//
// ****************************************************************************

class GUI_API QvisHelpWindow : public QvisDelayedWindow
{
    Q_OBJECT
public:
    QvisHelpWindow(const QString &captionString);
    virtual ~QvisHelpWindow();

    void SetLocale(const QString &);

    virtual void CreateWindowContents();
    virtual void CreateNode(DataNode *);
    virtual void SetFromNode(DataNode *, const int *borders);
public slots:
    void displayCopyright();
    void displayReleaseNotes();
    void displayReleaseNotesIfAvailable();
    void displayContributors();
    void openHelp(const QString& entry);
    virtual void show();
private slots:
    void activeTabChanged(int);
    void activateContentsTab();
    void activateIndexTab();
    void activateBookmarkTab();
    void openHelp(QTreeWidgetItem *);
    void topicExpanded(QTreeWidgetItem *);
    void topicCollapsed(QTreeWidgetItem *);
    void displayNoHelp();
    void displayTitle(const QString &title);
    void displayHome();
    bool displayPage(const QString &page, bool reload = false);
    void increaseFontSize();
    void decreaseFontSize();
    void displayIndexTopic();
    void lookForIndexTopic(const QString &topic);
    void displayBookmarkTopic();
    void addBookmark();
    void removeBookmark();
    void anchorClicked(const QUrl &);
private:
    typedef QMap<QString, QString> IndexMap;

    QString ReleaseNotesFile() const;
    void LoadHelp(const QString &helpFile);
    void BuildIndex();
    void AddToIndex(const QString &topic, const QString &doc);
    void BuildContents(QTreeWidgetItem *parentItem,
                       const QDomElement &parentElement);
    void BuildBookmarks();
    QString TopicFromDoc(const QString &doc);
    bool TopicFromDocHelper(QString &str, const QString &doc,
                            QTreeWidgetItem *item);
    QString CompleteFileName(const QString &page) const;
    void synchronizeContents(const QString &page);
    void displayReleaseNotesHelper(bool);

    QString       locale;
    QTabWidget   *helpTabs;
    QTreeWidget  *helpContents;
    QTextBrowser *helpBrowser;
    QSplitter    *splitter;
    QAction      *backAction;
    QAction      *forwardAction;

    QWidget      *helpIndexTab;
    QListWidget  *helpIndex;
    QLineEdit    *helpIndexText;

    QWidget      *helpBookmarksTab;
    QPushButton  *addBookmarkButton;
    QPushButton  *removeBookmarkButton;
    QListWidget  *helpBookMarks;

    QIcon        closedBookIcon;
    QIcon        openBookIcon;
    QIcon        helpIcon;
    QString      helpFile;
    QString      helpPath;
    QString      manualPath;
    bool         firstShow;
    int          activeTab;
    IndexMap     index;
    IndexMap     bookmarks;
};

#endif
