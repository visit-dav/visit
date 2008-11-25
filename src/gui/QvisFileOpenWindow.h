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
// ****************************************************************************

class GUI_API QvisFileOpenWindow : public QvisFileWindowBase
{
    Q_OBJECT
public:
    QvisFileOpenWindow(const QString &winCaption = QString::null);
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
private slots:
    void okClicked();
    void cancelClicked();

    void selectFile();
    void selectFileDblClick(QListWidgetItem *item);
    void selectFileChanged();
    void selectFileReturnPressed(QListWidgetItem *);

    void setDefaultOptionsForFormatButtonClicked();
    void fileFormatChanged(const QString&);
private:
    DBPluginInfoAttributes *dbplugins;
    UsageMode               usageMode;
    std::map<std::string, DBPluginInfoAttributes> dbpluginmap;

    QPushButton     *refreshButton;
    QPushButton     *okButton;
    QPushButton     *cancelButton;
    QComboBox       *fileFormatComboBox;
    QPushButton     *setDefaultOptionsForFormatButton;
};

#endif
