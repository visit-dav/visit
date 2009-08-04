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

#ifndef QVIS_EXPORT_DB_WINDOW_H
#define QVIS_EXPORT_DB_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowSimpleObserver.h>

// Forward declarations.
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSlider;
class QvisVariableButton;
class ExportDBAttributes;
class DBPluginInfoAttributes;

// ****************************************************************************
// Class: QvisExportDBWindow
//
// Purpose:
//   This class contains the code necessary to create a window for exporting
//   databases.
//
// Notes:      
//
// Programmer: Hank Childs
// Creation:   May 25, 2005
//
// Modifications:
//   Brad Whitlock, Mon Jun 27 13:51:01 PST 2005
//   Added a directory selection button.
//
//   Brad Whitlock, Thu Nov 2 18:01:00 PST 2006
//   I made a widget be a class member so I can disable it.
//
//   Jeremy Meredith, Tue Jul 17 11:29:24 EDT 2007
//   Converted to a simple observer so we can watch not only the 
//   export atts but also the DB plugin info atts.
//
//   Brad Whitlock, Wed Apr  9 11:57:54 PDT 2008
//   QString for caption, shortName.
//
// ****************************************************************************

class GUI_API QvisExportDBWindow : public QvisPostableWindowSimpleObserver
{
    Q_OBJECT
public:
    QvisExportDBWindow(const QString &caption = QString::null,
                       const QString &shortName = QString::null,
                       QvisNotepadArea *notepad = 0);
    virtual ~QvisExportDBWindow();

    virtual void SubjectRemoved(Subject *TheRemovedSubject);
    virtual void ConnectSubjects(ExportDBAttributes *edb,
                                 DBPluginInfoAttributes *dbp);
public slots:
    virtual void apply();
protected:
    virtual void CreateWindowContents();
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);
protected slots:
    void variableProcessText();
    void processFilenameText();
    void processDirectoryNameText();
    void fileFormatChanged(int);
    void exportDB();
    void exportButtonClicked();
    void addVariable(const QString &);
    void selectOutputDirectory();
private:
    DBPluginInfoAttributes *dbPluginInfoAtts;
    ExportDBAttributes     *exportDBAtts;

    QLineEdit           *filenameLineEdit;
    QLineEdit           *directoryNameLineEdit;
    QPushButton         *directorySelectButton;
    QLabel              *directoryNameLabel;
    QComboBox           *fileFormatComboBox;
    QvisVariableButton  *varsButton;
    QLineEdit           *varsLineEdit;
};

#endif
