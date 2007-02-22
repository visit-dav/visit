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

#ifndef QVIS_CHECK_LIST_MANAGER_WIDGET_H
#define QVIS_CHECK_LIST_MANAGER_WIDGET_H
#include <gui_exports.h>
#include <qwidget.h>

class QButtonGroup;
class QCheckBox;
class QGrid;
class QScrollView;

// ****************************************************************************
// Class: QvisCheckListManagerWidget
//
// Purpose:
//   This class contains a list of checkboxes and puts them in a scrolled
//   window so they can be arranged easily.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 13 10:21:14 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

class GUI_API QvisCheckListManagerWidget : public QWidget
{
    Q_OBJECT
public:
    QvisCheckListManagerWidget(QWidget *parent = 0, const char *name = 0);
    virtual ~QvisCheckListManagerWidget();

    void addCheckBox(const QString &name, bool checked);
    void removeLastEntry();
    void setCheckBox(int index, const QString &name, bool checked);

    int numCheckBoxes() const;
signals:
    void checked(int index, bool on);
private slots:
    void emitChecked(int index);
private:
    QScrollView    *scrollView;
    QGrid          *grid;
    QButtonGroup   *checkBoxGroup;

    int            nCheckBoxes;
    QCheckBox      **checkBoxes;
};

#endif
