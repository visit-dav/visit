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

#ifndef QVIS_SUBSET_PANEL_ITEM_H
#define QVIS_SUBSET_PANEL_ITEM_H
#include <gui_exports.h>

#include <QTreeWidget>


typedef enum {NotChecked, PartiallyChecked, CompletelyChecked, Unset} CheckedState;


// ****************************************************************************
// Class: QvisSubsetPanelItem
//
// Purpose: 
//   This class is a tree viewitem that has a tri-state checkbox and 
//   contains an integer that represents a set or collection id.
//
// Notes:
//  Adapted from the QvisSubsetListViewItem class.
//
// Programmer: Cyrus Harrison
// Creation:   June 26, 2008
//
// Modifications:
//
// ****************************************************************************

class QvisSubsetPanelItem: public QTreeWidgetItem
{
public:
    QvisSubsetPanelItem(QTreeWidget *parent,
                        const QString &text,
                        CheckedState s = NotChecked,
                        int id = 0);
                           
    QvisSubsetPanelItem(QTreeWidgetItem *parent,
                        const QString &text,
                        int id = 0);

    virtual ~QvisSubsetPanelItem();

    void         setCheckable(bool val);
    bool         getCheckable() const;
    
    void         setState(CheckedState s);
    CheckedState getState() const;
    
    void         toggleState();
    
    int          id() const;
    void         setId(int id);
    bool         isOn() const { return state != NotChecked; }

private:
    static void  InitIcons();
    static bool  iconsReady;
    static QIcon iconChecked;
    static QIcon iconMixed;
    static QIcon iconUnchecked;
    

    CheckedState state;
    int          itemId;
    bool         checkable;
};

#endif
