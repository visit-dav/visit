#ifndef QVIS_HELP_WINDOW_H
#define QVIS_HELP_WINDOW_H
#include <gui_exports.h>
#include <qmap.h>
#include <qpixmap.h>
#include <QvisDelayedWindow.h>

class QAction;
class QDomElement;
class QLineEdit;
class QListBox;
class QListView;
class QListViewItem;
class QPushButton;
class QSplitter;
class QTabWidget;
class QTextBrowser;
class QVBox;
class QvisHelpListViewItem;

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
// ****************************************************************************

class GUI_API QvisHelpWindow : public QvisDelayedWindow
{
    Q_OBJECT
public:
    QvisHelpWindow(const char *captionString);
    virtual ~QvisHelpWindow();

    virtual void CreateWindowContents();
    virtual void CreateNode(DataNode *);
    virtual void SetFromNode(DataNode *, const int *borders);
public slots:
    void displayCopyright();
    void displayReleaseNotes();
    void displayReleaseNotesIfAvailable();
    virtual void show();
private slots:
    void activeTabChanged(QWidget *);
    void activateContentsTab();
    void activateIndexTab();
    void activateBookmarkTab();
    void openHelp(QListViewItem *);
    void topicExpanded(QListViewItem *);
    void topicCollapsed(QListViewItem *);
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
private:
    typedef QMap<QString, QString> IndexMap;

    void LoadHelp(const QString &helpFile);
    void BuildIndex();
    void AddToIndex(const QString &topic, const QString &doc);
    void BuildContents(QListViewItem *parentItem,
                       const QDomElement &parentElement);
    void BuildBookmarks();
    QString TopicFromDoc(const QString &doc);
    bool TopicFromDocHelper(QString &str, const QString &doc,
                            QvisHelpListViewItem *item);
    QString CompleteFileName(const QString &page) const;
    void synchronizeContents(const QString &page);
    void displayReleaseNotesHelper(bool);

    QTabWidget   *helpTabs;
    QListView    *helpContents;
    QTextBrowser *helpBrowser;
    QSplitter    *splitter;
    QAction      *backAction;
    QAction      *forwardAction;

    QVBox        *helpIndexTab;
    QListBox     *helpIndex;
    QLineEdit    *helpIndexText;

    QVBox        *helpBookmarksTab;
    QPushButton  *addBookmarkButton;
    QPushButton  *removeBookmarkButton;
    QListBox     *helpBookMarks;

    QPixmap      closedBookIcon;
    QPixmap      openBookIcon;
    QPixmap      helpIcon;
    QString      helpFile;
    QString      helpPath;
    bool         firstShow;
    int          activeTab;
    IndexMap     index;
    IndexMap     bookmarks;
};

#endif
