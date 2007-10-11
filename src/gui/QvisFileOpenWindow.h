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

#ifndef QVIS_FILE_OPEN_WINDOW_H
#define QVIS_FILE_OPEN_WINDOW_H
#include <gui_exports.h>
#include <vectortypes.h>
#include <QvisDelayedWindowSimpleObserver.h>
#include <QualifiedFilename.h>
#include <maptypes.h>
#include <DBPluginInfoAttributes.h>

// Forward declarations.
class FileServerList;
class HostProfileList;
class QCheckBox;
class QCloseEvent;
class QComboBox;
class QLineEdit;
class QListBox;
class QListBoxItem;
class QPixmap;
class QPushButton;
class QvisRecentPathRemovalWindow;

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
// ****************************************************************************

class GUI_API QvisFileOpenWindow : public QvisDelayedWindowSimpleObserver
{
    Q_OBJECT
public:
    QvisFileOpenWindow(const char *winCaption = 0);
    virtual ~QvisFileOpenWindow();
    virtual void CreateWindowContents();
    virtual void SubjectRemoved(Subject *TheRemovedSubject);
    virtual void ConnectSubjects(HostProfileList *hpl,
                                 DBPluginInfoAttributes *dbp);

    typedef enum
    {
        SelectFilename,
        OpenFiles
    } UsageMode;

    void SetUsageMode(UsageMode m);
signals:
    void selectedFile(const QString &);
    void selectCancelled();
public slots:
    virtual void show();
    virtual void showMinimized();
    virtual void showNormal();
    virtual void setEnabled(bool);
protected:
    virtual void closeEvent(QCloseEvent *);
    virtual void UpdateWindow(bool doAll);
    void UpdateWindowFromFiles(bool doAll);
    void UpdateHostComboBox();
private:
    // Utility functions.
    void UpdateDirectoryList(void);
    void UpdateFileList(void);
    void UpdateComboBox(QComboBox *cb, const stringVector &s,
                        const QString &activeItem);
    bool AddFile(const QualifiedFilename &newFile);
    void GetCurrentValues(bool allowPathChange);
    bool ChangeHosts(void);
    bool ChangePath(bool allowPathChange);
    bool ChangeFilter(void);
    void RemoveComboBoxItem(QComboBox *cb, const QString &remove);
    void ActivateComboBoxItem(QComboBox *cb, const QString &newActive);
    void HighlightComboBox(QComboBox *cb);
    void GetDirectoryStrings(QString &curdir, QString &updir);
    void GetVirtualDatabaseDefinitions(StringStringVectorMap &);
    void UpdateFileFormatComboBox();

    static bool ProgressCallback(void *data, int stage);
private slots:
    void okClicked();
    void cancelClicked();
    void filterChanged();
    void hostChanged(int);
    void pathChanged(int);
    void changeDirectory(QListBoxItem *);
    void selectFile();
    void selectFileDblClick(QListBoxItem *item);
    void selectFileChanged();
    void selectFileReturnPressed(QListBoxItem *);
    void refreshFiles();
    void currentDir(bool val);
    void fileGroupingChanged(int val);
private:
    FileServerList         *fs;
    HostProfileList        *profiles;
    DBPluginInfoAttributes *dbplugins;
    UsageMode               usageMode;
    std::map<std::string, DBPluginInfoAttributes> dbpluginmap;

    QComboBox       *hostComboBox;
    QComboBox       *pathComboBox;
    QLineEdit       *filterLineEdit;
    QListBox        *directoryList;
    QListBox        *fileList;
    QPushButton     *refreshButton;
    QCheckBox       *currentDirToggle;
    QComboBox       *fileGroupingComboBox;
    QPushButton     *recentPathRemovalButton;
    QPushButton     *okButton;
    QPushButton     *cancelButton;
    QComboBox       *fileFormatComboBox;

    QvisRecentPathRemovalWindow *recentPathsRemovalWindow;

    QPixmap         *computerPixmap;
    QPixmap         *folderPixmap;
    QPixmap         *databasePixmap;

    QualifiedFilenameVector intermediateFileList;
    StringStringVectorMap   currentVirtualDatabaseDefinitions;
    stringVector            invalidHosts;
};

#endif
