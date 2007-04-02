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

#ifndef QVIS_SUBSET_LISTVIEW_ITEM_H
#define QVIS_SUBSET_LISTVIEW_ITEM_H
#include <gui_exports.h>
#include <qlistview.h>

typedef enum {NotChecked, PartiallyChecked, CompletelyChecked} CheckedState;

// ****************************************************************************
// Class: QvisSubsetListViewItem
//
// Purpose: 
//   This class is a listview item that has a tri-state checkbox and contains
//   an integer that represents set or collection id's.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:46:29 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

class GUI_API QvisSubsetListViewItem : public QCheckListItem
{
public:
    QvisSubsetListViewItem(QListView * parent, const QString &text,
                           CheckedState s = NotChecked,
                           int id_ = 0);
    QvisSubsetListViewItem(QListViewItem *parent, const QString &text,
                           CheckedState s = NotChecked,
                           int id_ = 0);

    virtual ~QvisSubsetListViewItem();
    virtual QString key(int column, bool ascending) const;
    virtual void activate();
    virtual void paintCell(QPainter *p, const QColorGroup &cg,
                           int column, int width, int align);
    virtual void paintFocus(QPainter *p, const QColorGroup &cg,
                            const QRect &r);

    void         setCheckable(bool val);
    bool         getCheckable() const;
    void         setState(CheckedState s);
    CheckedState getState() const;
    int          id() const;
    void         setId(int id_);
    bool         isOn() const { return state != NotChecked; }

private:
    CheckedState state;
    int          itemId;
    bool         checkable;
};

#endif
