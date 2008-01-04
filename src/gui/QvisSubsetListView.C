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

#include <QvisSubsetListView.h>
#include <QvisSubsetListViewItem.h>

// ****************************************************************************
// Method: QvisSubsetListView::QvisSubsetListView
//
// Purpose: 
//   This is the constructor for the QvisSubsetListView class.
//
// Arguments:
//   parent : The widget's parent widget.
//   name   : The name of this widget instance.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:47:57 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

QvisSubsetListView::QvisSubsetListView(QWidget *parent, const char *name) :
    QListView(parent,name)
{
    // nothing
}

// ****************************************************************************
// Method: QvisSubsetListView::~QvisSubsetListView
//
// Purpose: 
//   This is the destructor for the QvisSubsetListView class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:48:48 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

QvisSubsetListView::~QvisSubsetListView()
{
    // nothing
}

// ****************************************************************************
// Method: QvisSubsetListView::resize
//
// Purpose: 
//   This is a Qt slot function that resizes the widget.
//
// Arguments:
//   w : The new widget width.
//   h : The new widget height.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:49:10 PDT 2001
//
// Modifications:
//   Brad Whitlock, Mon Sep 24 16:48:49 PST 2001
//   Removed a stray debugging comment.
//
// ****************************************************************************

void
QvisSubsetListView::resize(int w, int h)
{
    // Call the parent's resize method.
    QListView::resize(w, h);

    // resize the width
    if(columns() > 0)
    {

        setColumnWidth(0, visibleWidth());
    }
}

// ****************************************************************************
// Method: QvisSubsetListView::emitChecked
//
// Purpose: 
//   This method tells the listview to emit a checked signal. This is used by
//   QvisSubsetListViewItem to tell the world when the item has been checked.
//
// Arguments:
//   item : A pointer to the item that was checked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 5 09:50:28 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisSubsetListView::emitChecked(QvisSubsetListViewItem *item)
{
    emit checked(item);
}
