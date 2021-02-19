// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_FILE_OPEN_WINDOW_H
#define QVIS_FILE_OPEN_WINDOW_H
#include <gui_exports.h>
#include <QvisFileWindowBase.h>
#include <DBPluginInfoAttributes.h>


// ****************************************************************************
// Class: QvisFileOpenWindow
//
// Purpose:
//   This window allows the user to browse files on remote machines.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   August  9, 2006
//
// Modifications:
//   Brad Whitlock, Wed Nov 15 10:59:10 PDT 2006
//   I changed the window so it can be used as a file selection window in
//   other contexts.
//
//   Jeremy Meredith, Mon Jul 16 17:20:26 EDT 2007
//   I added support for DBPluginInfoAttributes to be associated with
//   separate hosts.
//
//   Jeremy Meredith, Wed Jan 23 15:32:24 EST 2008
//   Added button to let the user set the default opening options.
//
//   Brad Whitlock, Wed Apr  9 10:44:36 PDT 2008
//   Use QString for winCaption.
//
//   Brad Whitlock, Mon Jul 14 11:38:44 PDT 2008
//   Changed inheritance and moved some methods to base class.
//
//    David Camp, Thu Aug 27 09:40:00 PDT 2015
//    Added filename field if set to show. This is used by the Session dialog.
//    Also a flag to hide some fields not needed by the Session dialog
//
// ****************************************************************************

class GUI_API QvisFileOpenWindow : public QvisFileWindowBase
{
    Q_OBJECT
public:
    QvisFileOpenWindow(const QString &winCaption = QString());
    virtual ~QvisFileOpenWindow();
    virtual void CreateWindowContents();
    virtual void SubjectRemoved(Subject *TheRemovedSubject);
    void ConnectSubjects(HostProfileList *hpl,
                         DBPluginInfoAttributes *dbp);

    typedef enum
    {
        SelectFilename,
        OpenFiles
    } UsageMode;

    void SetUsageMode(UsageMode m);

    void SetHideFileFormat(bool value);
    void SetShowFilename(bool value);
    void SetFilename(const QString &);
signals:
    void selectedFile(const QString &);
    void selectCancelled();
public slots:
    virtual void show();
protected:
    virtual void UpdateWindow(bool doAll);
private:
    // Utility functions.
    void UpdateFileFormatComboBox();
    bool eventFilter(QObject *o, QEvent *e);
private slots:
    void okClicked();
    void cancelClicked();

    void selectFile();
    void selectFileDblClick(QListWidgetItem *item);
    void selectFileChanged();
    void selectFileReturnPressed(QListWidgetItem *);

    void setDefaultOptionsForFormatButtonClicked();
    void fileFormatChanged(const QString&);
    void filenameEditChanged(const QString &text);
private:
    DBPluginInfoAttributes *dbplugins;
    UsageMode               usageMode;
    std::map<std::string, DBPluginInfoAttributes> dbpluginmap;

    QPushButton     *refreshButton;
    QPushButton     *okButton;
    QPushButton     *cancelButton;
    QComboBox       *fileFormatComboBox;
    QPushButton     *setDefaultOptionsForFormatButton;

    // If true it will hide the fileFormatComboBox and setDefaultOptionsForFormatButton.
    // Used for the session dialog to load and save session files.
    bool             hideFileFormat;
    // Flag to show or hide the filename. Used for save session file dialog.
    bool             showFilename;
    QLineEdit        *filenameEdit;
};

#endif
