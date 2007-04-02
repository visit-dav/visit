/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

#ifndef QVIS_PLOT_LIST_BOX_H
#define QVIS_PLOT_LIST_BOX_H
#include <gui_exports.h>
#include <qlistbox.h>
#include <vectortypes.h>

class PlotList;

// ****************************************************************************
// Class: QvisPlotListBox
//
// Purpose:
//   This has the same functionality as QListBox except that its
//   doubleClicked signal also sends the position of where the mouse
//   was clicked.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 11 11:25:17 PDT 2000
//
// Notes:
//   This class can only contain QvisPlotListBoxItem objects because it
//   needs that class's extanded functionality in order to function correctly.
//
// Modifications:
//   Brad Whitlock, Tue Apr 8 11:09:55 PDT 2003
//   I renamed the class to QvisPlotListBox and made enhancements that
//   enable operators to be moved within a plot.
//
//   Brad Whitlock, Fri Dec 5 16:22:48 PST 2003
//   I added NeedToUpdateSelection.
//
//   Brad Whitlock, Wed Jul 28 17:33:21 PST 2004
//   I added prefixes to arguments for NeedsToBeRegenerated.
//
// ****************************************************************************

class GUI_API QvisPlotListBox : public QListBox
{
    Q_OBJECT
public:
    QvisPlotListBox(QWidget *parent = 0, const char *name = 0, WFlags f=0);
    virtual ~QvisPlotListBox();

    bool isExpanded(int) const;
    int  activeOperatorIndex(int) const;

    bool NeedsToBeRegenerated(const PlotList *, const stringVector &indices) const;
    bool NeedToUpdateSelection(const PlotList *) const;
signals:
    void activateSubsetWindow();
    void activatePlotWindow(int plotType);
    void activateOperatorWindow(int operatorType);
    void promoteOperator(int operatorIndex);
    void demoteOperator(int operatorIndex);
    void removeOperator(int operatorIndex);
protected:
    virtual void viewportMousePressEvent(QMouseEvent *e);
    virtual void viewportMouseDoubleClickEvent(QMouseEvent *e);
    void clickHandler(const QPoint &p, bool);
    void itemClicked(QListBoxItem *item, const QPoint &point, bool);
};

#endif
