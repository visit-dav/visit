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

#ifndef QVIS_SUBSET_WINDOW_H
#define QVIS_SUBSET_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>
#include <vector>

// Forward declarations.
class QComboBox;
class QFrame;
class QGridLayout;
class QLabel;
class QTreeView;
class QTreeViewItem;
class QPushButton;
class QMenu;
class QScrollArea;
class QSplitter;
class QvisSubsetPanelWidget;
class QvisSubsetPanelItem;
class FileServerList;
class avtSIL;


// ****************************************************************************
// Class: QvisSubsetWindow
//
// Purpose:
//   This class contains the code for the subset window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:06:29 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Aug 21 16:00:21 PST 2001
//   Made the window postable.
//
//   Brad Whitlock, Tue Sep 25 14:30:35 PST 2001
//   Added post(), unpost(), specialResize().
//
//   Brad Whitlock, Fri Feb 8 15:52:11 PST 2002
//   Modified to use a splitter.
//
//   Brad Whitlock, Mon Mar 4 15:12:38 PST 2002
//   Added autoUpdate support.
//
//   Brad Whitlock, Wed Apr 23 14:14:33 PST 2003
//   I changed the interface of ItemClicked.
//
//   Brad Whitlock, Fri Aug 6 13:56:01 PST 2004
//   Made changes that allow multiple sets to be selected.
//
//   Brad Whitlock, Wed Apr  9 11:04:31 PDT 2008
//   QString for caption, shortName.
//
//   Cyrus Harrison, Fri Jul 18 09:02:29 PDT 2008
//   Refactored for Qt4.
//
// ****************************************************************************

class GUI_API QvisSubsetWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisSubsetWindow(Subject *subj,
                     const QString &caption = QString::null,
                     const QString &shortName = QString::null,
                     QvisNotepadArea *notepad = 0);
    virtual ~QvisSubsetWindow();
    virtual void CreateWindowContents();

protected:
    virtual void UpdateWindow(bool doAll);
    
    void Apply(bool ignore = false);
    int  AddPanel(bool visible = true);
    void UpdatePanels(int index = -1, bool panels_after = false);
    void ClearPanelsToTheRight(int index);
        
private slots:
    void apply();
    void onPanelItemSelected(int id,bool parent);
    void onPanelStateChanged();
    
private:
    int FindPanelIndex(QObject *obj);
    QScrollArea                    *scrollView;
    QSplitter                      *panelSplitter;
    QList<QvisSubsetPanelWidget *>  panels;

    // A few attributes of the current SIL restriction that we use to
    // determine how the window must be redrawn.
    int                  sil_TopSet;
    int                  sil_NumSets;
    int                  sil_NumCollections;
};

#endif
