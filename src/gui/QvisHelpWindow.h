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
    bool         firstShow;
    int          activeTab;
    IndexMap     index;
    IndexMap     bookmarks;
};

#endif
