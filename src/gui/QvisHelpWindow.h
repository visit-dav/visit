// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_HELP_WINDOW_H
#define QVIS_HELP_WINDOW_H
#include <gui_exports.h>
#include <QvisDelayedWindow.h>

class QAction;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QSplitter;
class QStackedWidget;
class QTextBrowser;
class QUrl;

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
//   Kathleen Biagas, Wed June 24, 2026
//   Replaced 'reload' with 'externalBrowser' argument to displayPage as the
//   reload arg not being used. Added a second displayTitle method and new
//   startOptsPath ivar.
//
//   Kathleen Biagas, Mon Aug 31, 2026
//   Restructure window to simplify. Since most content is displayed in
//   external browser, remove Index, Bookmarks, Search, tabs.
//   Topics now stored in QListWidget.
//   Remove Contributors.
//   Add 'About'. (link used to be in Main window's menu bar 'Help').
//   Home page now loads external Home page.
//   Add 'Getting Help' which opens to correct page in local docs in an
//   external broswer.
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
public slots:
    void displayCopyright();
    void displayReleaseNotes();
    void displayReleaseNotesIfAvailable();
    void openHelp(const QString &entry);
    virtual void show();
signals:
    void showAbout();
private slots:
    void displaySelectedTopic(QListWidgetItem *);
    void increaseFontSize();
    void decreaseFontSize();
    void anchorClicked(const QUrl &);
private:
    QString ReleaseNotesFile() const;
    QString CompleteFileName(const QString &page) const;
    void AddTopic(const QString &title, const QString &page);
    bool displayPage(const QString &page);
    bool displayExternalLink(const QString &title, const QString &page);
    bool displayExternalLink(const QString &title, const QUrl &url,
                             const QString &statusText,
                             const QString &page);
    void displayAbout();
    void displayTitle(const QString &title);
    void displayNoHelp();
    void displayReleaseNotesHelper(bool);
    void selectPage(const QString &page);
    void updateFontSizeActions(const QString &page);

    QString       locale;
    QString       helpPath;
    QString       aboutPath;
    QString       manualPath;
    QString       gettingHelpPath;
    QString       currentPage;
    QListWidget  *topics;
    QStackedWidget *pageStack;
    QTextBrowser *browser;
    QLabel       *externalLinkLabel;
    QSplitter    *splitter;
    QAction      *fontUpAction;
    QAction      *fontDownAction;
    bool         firstShow;
};

#endif
