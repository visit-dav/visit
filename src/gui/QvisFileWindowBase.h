/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//   
// ****************************************************************************

class GUI_API QvisFileWindowBase : public QvisDelayedWindowSimpleObserver
{
    Q_OBJECT
public:
    QvisFileWindowBase(const QString &winCaption = QString::null);
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
public slots:
    virtual void setEnabled(bool);
    virtual void showMinimized();
    virtual void showNormal();
protected slots:
    void filterChanged();
    void hostChanged(int);
    void pathChanged(int);
    void changeDirectory(QListWidgetItem *);
    void refreshFiles();
    void currentDir(bool val);
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
    QComboBox                   *fileGroupingComboBox;
    QPushButton                 *recentPathRemovalButton;
    QvisRecentPathRemovalWindow *recentPathsRemovalWindow;
    QListWidget                 *directoryList;
    QListWidget                 *fileList;

    QualifiedFilenameVector      intermediateFileList;
    StringStringVectorMap        currentVirtualDatabaseDefinitions;
    stringVector                 invalidHosts;
};

// Functions to encode QualifiedFilename into QVariant and back.
QVariant EncodeQualifiedFilename(const QualifiedFilename &filename);
QualifiedFilename DecodeQualifiedFilename(const QVariant &v);

#endif
