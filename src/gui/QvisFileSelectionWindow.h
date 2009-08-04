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

#ifndef QVIS_FILE_SELECTION_WINDOW_H
#define QVIS_FILE_SELECTION_WINDOW_H
#include <gui_exports.h>
#include <QvisFileWindowBase.h>

// Forward declarations.
class QPushButton;
class QListWidget;

// ****************************************************************************
// Class: QvisFileSelectionWindow
//
// Purpose:
//   This window allows the user to browse files on remote machines.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 22 10:45:17 PDT 2000
//
// Modifications:
//   Sean Ahern, Wed Feb 28 17:13:11 PST 2001
//   Added the groupFiles slot for grouping files together.
//
//   Brad Whitlock, Thu May 3 11:01:50 PDT 2001
//   Added internal methods for changing the path, filter, host.
//
//   Brad Whitlock, Mon Aug 27 11:19:06 PDT 2001
//   Changed the names of two slot functions.
//
//   Brad Whitlock, Thu Jan 3 13:32:24 PST 2002
//   Added refresh button.
//
//   Brad Whitlock, Wed Feb 13 09:56:59 PDT 2002
//   Replaced the host and path lineEdits with combo boxes.
//
//   Brad Whitlock, Mon Feb 25 15:55:27 PST 2002
//   Added the UpdateComboBox method.
//
//   Brad Whitlock, Thu Apr 11 14:52:54 PST 2002
//   Added override of show().
//
//   Brad Whitlock, Fri Jul 26 13:56:20 PST 2002
//   Added a new toggle button.
//
//   Brad Whitlock, Wed Aug 28 09:30:08 PDT 2002
//   I added a private helper method and I removed some static const strings.
//
//   Brad Whitlock, Wed Sep 11 17:19:16 PST 2002
//   I broke the RemoveComboBoxItem method into two methods. I also made
//   tbe window add the hosts in the host profiles to the list of available
//   hosts.
//
//   Brad Whitlock, Mon Sep 30 07:45:13 PDT 2002
//   I made the window be modal and I added a callback that updates the GUI
//   while we wait for information from the mdserver.
//
//   Brad Whitlock, Thu Mar 27 09:38:54 PDT 2003
//   I added a toggle for automatic file grouping.
//
//   Brad Whitlock, Fri Oct 10 14:43:31 PST 2003
//   I added a push button to open a window to remove recent paths.
//
//   Brad Whitlock, Tue Dec 2 16:30:37 PST 2003
//   I added currentVirtualDatabaseDefinition.
//
//   Brad Whitlock, Tue Jul 27 11:26:34 PDT 2004
//   I replaced currentVirtualDatabaseDefinition with a map.
//
//   Brad Whitlock, Thu Jul 29 13:50:30 PST 2004
//   I added smart file grouping support and I fixed a bug in how the hosts
//   are added to the host combo box.
//
//   Brad Whitlock, Fri Jul 30 10:59:14 PDT 2004
//   I added a new slot function so we can select multiple files by hitting
//   the return key in the file list.
//
//   Brad Whitlock, Tue Mar 7 10:11:46 PDT 2006
//   Added the selectedFilesChanged signal.
//
//   Brad Whitlock, Wed Apr  9 10:29:50 PDT 2008
//   Changed ctor args.
//
//   Brad Whitlock, Thu Jul 10 15:37:04 PDT 2008
//   Changed inheritance and moved some methods to base class.
//
// ****************************************************************************

class GUI_API QvisFileSelectionWindow : public QvisFileWindowBase
{
    Q_OBJECT
public:
    QvisFileSelectionWindow(const QString &winCaption);
    virtual ~QvisFileSelectionWindow();
    virtual void CreateWindowContents();
signals:
    void selectedFilesChanged();
public slots:
    virtual void show();
    virtual void setEnabled(bool);
protected:
    virtual void UpdateWindow(bool doAll);
    virtual void UpdateWindowFromFiles(bool doAll);
private:
    // Utility functions.
    virtual void UpdateFileList();

    void UpdateSelectedFileList(void);
    void UpdateRemoveFileButton(void);
    void UpdateSelectAllButton(void);

private slots:
    void okClicked();
    void groupFiles();
    void cancelClicked();
    void selectFile();
    void selectFileDblClick(QListWidgetItem *item);
    void selectFileChanged();
    void selectFileReturnPressed(QListWidgetItem *);
    void selectedFileSelectChanged();
    void selectAllFiles();
    void removeFile();
    void removeAllFiles();
    void removeSelectedFiles(QListWidgetItem *);
private:
    QPushButton     *groupButton;
    QPushButton     *selectButton;
    QPushButton     *selectAllButton;
    QPushButton     *removeButton;
    QPushButton     *removeAllButton;
    QPushButton     *refreshButton;
    QListWidget     *selectedFileList;
};

#endif
