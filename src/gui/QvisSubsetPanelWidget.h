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

#ifndef QVIS_SUBSET_PANEL_WIDGET_H
#define QVIS_SUBSET_PANEL_WIDGET_H
#include <gui_exports.h>
#include <QWidget>


class QTreeWidget;
class QTreeWidgetItem;
class QLabel;
class QPushButton;
class QMenu;
class ViewerProxy;
class QvisSubsetPanelItem;

#include <avtSILRestrictionTraverser.h>


// ****************************************************************************
// Class: QvisSubsetPanelWidget
//
// Purpose:
//   Provide a selection panel widget or the subset window.
//
// Notes:      
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jun 30 08:45:28 PDT 2008
//
// Notes: The guts of this class were pulled out of QvisSubsetWindow.
//
// Modifications:
//
// ****************************************************************************

class GUI_API QvisSubsetPanelWidget : public QWidget
{
    Q_OBJECT
public:
    QvisSubsetPanelWidget(QWidget *parent, ViewerProxy *viewer_proxy);
    virtual ~QvisSubsetPanelWidget();

public slots:
    void SetTitle(const QString &title);
    void ViewSet(int id);
    void ViewCollection(int id);
    void UpdateView();
    void SetSelectedItemState(int state);
    void Clear();
    
signals:
    void itemSelected(int id, bool parent);
    void parentStateChanged(int value);
    void stateChanged();
    
protected slots:
    void onAllSetsButton();
    void onAllSetsActionReverse();
    void onAllSetsActionOn();
    void onAllSetsActionOff();
    
    void onSelectedSetsButton();
    void onSelectedSetsActionReverse();
    void onSelectedSetsActionOn();
    void onSelectedSetsActionOff();
    
    void onItemSelectionChanged();
    void onItemClicked(QTreeWidgetItem *,int);

private:
    void UpdateParentState();
    void EnableButtons(bool);
    void Reverse(bool);
    void TurnOnOff(bool,bool);
    
    ViewerProxy         *viewerProxy;
    
    QTreeWidget         *tree;
    QvisSubsetPanelItem *activeChild;
    int                  numCheckable;
    int                  numChecked;
    
    QLabel              *allSetsLabel;
    QPushButton         *allSetsButton;
    QPushButton         *allSetsActionButton;
    
    QLabel              *selectedSetsLabel;
    QPushButton         *selectedSetsButton;
    QPushButton         *selectedSetsActionButton;

};

#endif
