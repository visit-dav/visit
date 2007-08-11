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

#ifndef QVIS_LISTVIEW_FILE_ITEM_H
#define QVIS_LISTVIEW_FILE_ITEM_H
#include <gui_exports.h>
#include <qlistview.h>
#include <QualifiedFilename.h>

// ****************************************************************************
// Class: QvisListViewFileItem
//
// Purpose:
//   This is a listviewitem that contains a QualifiedFilename object.
//   It is used in the file panel's selected file list.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 13 12:15:46 PDT 2000
//
// Modifications:
//   Brad Whitlock, Wed Mar 21 00:36:23 PDT 2001
//   Added an argument to the constructor and added some convenience
//   functions.
//
//   Mark C. Miller, Fri Aug 10 23:11:55 PDT 2007
//   Added timeStateHasBeenForced to keep track of whether item has been
//   updated with cycle/time information from metadata that was forced to
//   have accurate cycles/times.
//
// ****************************************************************************

class GUI_API QvisListViewFileItem : public QListViewItem
{
public:
    static const int ROOT_NODE;
    static const int HOST_NODE;
    static const int DIRECTORY_NODE;
    static const int FILE_NODE;

    QvisListViewFileItem(QListView *parent, const QString &str,
        const QualifiedFilename &qf, int nodeType = 3, int state = -1,
	bool tsForced = false);
    QvisListViewFileItem(QListViewItem *parent, const QString &str,
        const QualifiedFilename &qf, int nodeType = 3, int state = -1,
	bool tsForced = false);
    virtual ~QvisListViewFileItem();

    virtual void paintCell(QPainter *p, const QColorGroup &cg,
                           int column, int width, int align);

    virtual QString key(int column, bool ascending) const;

    // Convenience functions
    bool isRoot() const { return (nodeType == ROOT_NODE); };
    bool isHost() const { return (nodeType == HOST_NODE); };
    bool isDirectory() const { return (nodeType == DIRECTORY_NODE); };
    bool isFile() const { return (nodeType == FILE_NODE); };
    static void resetNodeNumber();

    // public data members
    QualifiedFilename file;
    int               nodeType;
    int               timeState;
    bool              timeStateHasBeenForced;
private:
    static int        globalNodeNumber;
    int               nodeNumber;
};

#endif
