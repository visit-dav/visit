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

#include <QvisCheckListManagerWidget.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qgrid.h>
#include <qlayout.h>
#include <qscrollview.h>

static const int MAX_CHECKBOXES = 150;

// ****************************************************************************
// Method: QvisCheckListManagerWidget::QvisCheckListManagerWidget
//
// Purpose: 
//   This is the constructor for the QvisCheckListManagerWidget class.
//
// Arguments:
//   parent : A pointer to the widget's parent widget.
//   name   : The name of this widget instance.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 13 10:13:12 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

QvisCheckListManagerWidget::QvisCheckListManagerWidget(QWidget *parent,
    const char *name) : QWidget(parent, name), checkBoxes()
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);

    // How many checkboxes are visible
    nCheckBoxes = 0;

    // Create the scrollview.
    scrollView = new QScrollView(this, "scrollView");
    scrollView->setVScrollBarMode(QScrollView::Auto);
    scrollView->setHScrollBarMode(QScrollView::Auto);
    scrollView->viewport()->setBackgroundColor(colorGroup().background());
    topLayout->addWidget(scrollView);

    // Create the QGrid widget that will manage the layout of the buttons.
    grid = new QGrid(6, QGrid::Horizontal, scrollView->viewport(), "grid");
    grid->setSpacing(10);
    scrollView->addChild(grid);

    // Create the QButtonGroup used for all of the check boxes. It will not
    // be visible in the window, it will only be used to link the checkBoxes
    // together.
    checkBoxGroup = new QButtonGroup(0, "checkBoxParent");
    connect(checkBoxGroup, SIGNAL(clicked(int)),
            this, SLOT(emitChecked(int)));

    // Create a bunch of checkboxes
    checkBoxes = new QCheckBox * [MAX_CHECKBOXES];
    for(int i = 0; i < MAX_CHECKBOXES; ++i)
    {
        QString temp, cbName;
        temp.sprintf("newCheckBox_%d", i);
        cbName.sprintf("%d", i);
        checkBoxes[i] = new QCheckBox(cbName, grid, temp);
        checkBoxes[i]->setChecked(true);
        checkBoxes[i]->hide();
        checkBoxGroup->insert(checkBoxes[i], i);
    }
}

// ****************************************************************************
// Method: QvisCheckListManagerWidget::~QvisCheckListManagerWidget
//
// Purpose: 
//   This is the destructor for the QvisCheckListManagerWidget class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 13 10:14:09 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

QvisCheckListManagerWidget::~QvisCheckListManagerWidget()
{
    for(int i = 0; i < MAX_CHECKBOXES; ++i)
    {
        checkBoxGroup->remove(checkBoxes[i]);
        delete checkBoxes[i];
    }
    delete [] checkBoxes;
}

// ****************************************************************************
// Method: QvisCheckListManagerWidget::removeLastEntry()
//
// Purpose: 
//   This method removes the last checkbox in the list from the widget.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 13 10:15:58 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisCheckListManagerWidget::removeLastEntry()
{
    // If there are checkboxes, then remove the last one.
    if(nCheckBoxes > 0)
    {
        --nCheckBoxes;
        checkBoxes[nCheckBoxes]->hide();
    }
}

// ****************************************************************************
// Method: QvisCheckListManagerWidget::setCheckBox
//
// Purpose: 
//   This method sets the name and checked state of the specified checkbox.
//
// Arguments:
//   index   : The index of the checkbox we want to change.
//   name    : The new text to display with the specified checkbox.
//   checked : The new checked state of the specified checkbox.
//
// Note:       If the specified index is greater than the number of checkboxes
//   int the list, a new checkbox is added to the list.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 13 10:16:34 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisCheckListManagerWidget::setCheckBox(int index, const QString &name,
    bool checked)
{
    if(index >= MAX_CHECKBOXES)
        return;

    if(index >= nCheckBoxes)
    {
        ++nCheckBoxes;
        checkBoxes[index]->show();
    }

    checkBoxes[index]->setText(name);
    checkBoxGroup->blockSignals(true);
    checkBoxes[index]->setChecked(checked);
    checkBoxGroup->blockSignals(false);
}

// ****************************************************************************
// Method: QvisCheckListManagerWidget::numCheckBoxes
//
// Purpose: 
//   Returns the number of checkboxes in the list.
//
// Returns:    The number of checkboxes in the list.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 13 10:18:53 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

int
QvisCheckListManagerWidget::numCheckBoxes() const
{
    return nCheckBoxes;
}

// ****************************************************************************
// Method: QvisCheckListManagerWidget::emitChecked
//
// Purpose: 
//   This is a private Qt slot function that is hooked up to the QButtonGroup
//   of which the checkboxes are members. It is emitted when a checkbox changes
//   state.
//
// Arguments:
//   index : The index of the checkbox that whose state changed.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 13 10:19:20 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisCheckListManagerWidget::emitChecked(int index)
{
    if(index >= 0 && index < nCheckBoxes)
    {
        emit checked(index, checkBoxes[index]->isChecked());
    }
}
