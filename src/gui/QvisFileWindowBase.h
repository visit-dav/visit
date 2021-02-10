// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_FILE_WINDOW_BASE_H
#define QVIS_FILE_WINDOW_BASE_H
#include <gui_exports.h>
#include <QvisDelayedWindowSimpleObserver.h>
#include <QualifiedFilename.h>
#include <vectortypes.h>
#include <maptypes.h>

// Forward declarations.
class FileServerList;
class HostProfileList;

class QCheckBox;
class QComboBox;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPixmap;
class QPushButton;
class QvisRecentPathRemovalWindow;

// ****************************************************************************
// Class: QvisFileWindowBase
//
// Purpose:
//   Base class that contains some methods useful to derived file window classes.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 10 16:08:54 PDT 2008
//
// Modifications:
//   Brad Whitlock, Fri Jan 11 15:50:56 PST 2013
//   Remove setEnabled method.
//
//   David Camp, Thu Aug 27 09:40:00 PDT 2015
//   Added a flag to hide fields for the session file dialog.
//
// ****************************************************************************

class GUI_API QvisFileWindowBase : public QvisDelayedWindowSimpleObserver
{
    Q_OBJECT
public:
    QvisFileWindowBase(const QString &winCaption = QString());
    virtual ~QvisFileWindowBase();
    virtual void SubjectRemoved(Subject *TheRemovedSubject);
    void ConnectSubjects(HostProfileList *hpl);
protected:
    virtual void closeEvent(QCloseEvent *);

    static bool ProgressCallback(void *data, int stage);
    void CreateHostPathFilterControls();
    QListWidget *CreateFileListWidget(QWidget *parent) const;

    virtual void UpdateWindowFromFiles(bool doAll);
    virtual void UpdateFileList();
    void UpdateDirectoryList();
    void UpdateHostComboBox();

    void UpdateComboBox(QComboBox *cb, const stringVector &s,
                        const QString &activeItem);
    void GetDirectoryStrings(QString &curdir, QString &updir);
    void AddFileItem(QListWidget *parent,
                     const QString &displayName,
                     const QualifiedFilename &fileInfo);
    bool AddFile(const QualifiedFilename &newFile);

    void RemoveComboBoxItem(QComboBox *cb, const QString &remove);
    void ActivateComboBoxItem(QComboBox *cb, const QString &newActive);
    void HighlightComboBox(QComboBox *cb);

    bool ChangeHosts();
    bool ChangePath(bool allowPathChange);
    bool ChangeFilter();
    void GetCurrentValues(bool allowPathChange);
    void GetVirtualDatabaseDefinitions(StringStringVectorMap &);

    void CheckForNewStates();

    void SetHideOptions(bool value);

public slots:
    virtual void showMinimized();
    virtual void showNormal();
protected slots:
    void filterChanged();
    void hostChanged(int);
    void pathChanged(int);
    void changeDirectory(QListWidgetItem *);
    void refreshFiles();
    void currentDir(bool val);
    void showDotFiles(bool val);
    void fileGroupingChanged(int val);
protected:
    FileServerList              *fs;
    HostProfileList             *profiles;

    QPixmap                     *computerPixmap;
    QPixmap                     *folderPixmap;
    QPixmap                     *databasePixmap;

    QComboBox                   *hostComboBox;
    QComboBox                   *pathComboBox;
    QLineEdit                   *filterLineEdit;
    QCheckBox                   *currentDirToggle;
    QCheckBox                   *showDotFilesToggle;
    QComboBox                   *fileGroupingComboBox;
    QPushButton                 *recentPathRemovalButton;
    QvisRecentPathRemovalWindow *recentPathsRemovalWindow;
    QListWidget                 *directoryList;
    QListWidget                 *fileList;

    QualifiedFilenameVector      intermediateFileList;
    StringStringVectorMap        currentVirtualDatabaseDefinitions;
    stringVector                 invalidHosts;

    // Flag to hide the options, currentDirToggle, showDotFilesToggle, ... from
    // showing. This is used in the save and load of session files.
    bool                         hideOptions;
};

// Functions to encode QualifiedFilename into QVariant and back.
QVariant EncodeQualifiedFilename(const QualifiedFilename &filename);
QualifiedFilename DecodeQualifiedFilename(const QVariant &v);

#endif
