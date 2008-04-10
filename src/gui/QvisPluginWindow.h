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

#ifndef QVIS_PLUGIN_WINDOW_H
#define QVIS_PLUGIN_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowSimpleObserver.h>
#include <vector>
#include <string>

// Forward declarations.
class PluginManagerAttributes;
class FileOpenOptions;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QTabWidget;
class QVBox;
class QListView;
class QListViewItem;
class QCheckListItem;
class QPushButton;

// ****************************************************************************
// Class: QvisPluginWindow
//
// Purpose:
//   This window displays and manipulates the available plugins.
//
// Programmer: Jeremy Meredith
// Creation:   August 31, 2001
//
// Modifications:
//    Jeremy Meredith, Fri Sep 28 13:52:35 PDT 2001
//    Removed default settings capabilities.
//    Added a pluginSettingsChanged signal.
//    Added ability to keep track of which items are checked/unchecked.
//
//    Jeremy Meredith, Wed Jan 23 16:49:01 EST 2008
//    Populate database tab.  Also observe FileOpenOptions.
//
//    Dave Pugmire, Wed Feb 13 15:43:24 EST 2008
//    Update the FileOpenOptions for enable/disable DB plugins.
//
//    Brad Whitlock, Wed Apr  9 11:03:33 PDT 2008
//    QString for caption, shortName.
//
// ****************************************************************************

class GUI_API QvisPluginWindow : public QvisPostableWindowSimpleObserver
{
    Q_OBJECT
public:
    QvisPluginWindow(const QString &caption = QString::null,
                     const QString &shortName = QString::null,
                     QvisNotepadArea *notepad = 0);
    virtual ~QvisPluginWindow();
    virtual void CreateWindowContents();

    virtual void CreateNode(DataNode *);
    virtual void SetFromNode(DataNode *, const int *borders);
    virtual void Update(Subject *TheChangedSubject);

    virtual void ConnectSubjects(PluginManagerAttributes *p,
                                 FileOpenOptions *f);
    virtual void SubjectRemoved(Subject*);
                     
signals:
    void pluginSettingsChanged();
protected:
    virtual void UpdateWindow(bool doAll);
    void Apply(bool dontIgnore = false);
private slots:
    virtual void apply();
    void tabSelected(const QString &tabLabel);
    void databaseOptionsSetButtonClicked();
    void databaseSelectedItemChanged(QListViewItem*);
    void selectAllReadersButtonClicked();
    void unSelectAllReadersButtonClicked();
private:
    PluginManagerAttributes *pluginAtts;
    FileOpenOptions         *fileOpenOptions;

    QTabWidget      *tabs;
    QVBox           *pagePlots;
    QListView       *listPlots;
    QVBox           *pageOperators;
    QListView       *listOperators;
    QVBox           *pageDatabases;
    QListView       *listDatabases;
    QPushButton     *databaseOptionsSetButton;
    QPushButton *selectAllReadersButton;
    QPushButton *unSelectAllReadersButton;

    std::vector<QCheckListItem*> plotItems;
    std::vector<std::string>     plotIDs;
    std::vector<QCheckListItem*> operatorItems;
    std::vector<std::string>     operatorIDs;
    std::vector<QCheckListItem*>  databaseItems;
    std::vector<int>             databaseIndexes;

    int             activeTab;
    bool            pluginsInitialized;
};

#endif
