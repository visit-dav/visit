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
#include <SpreadsheetTabWidget.h>
#include <QTabBar>

// ****************************************************************************
// Method: SpreadsheetTabWidget::SpreadsheetTabWidget
//
// Purpose:
//   Constructor for the SpreadsheetTabWidget class.
//
// Arguments:
//   parent : The parent widget.
//   name   : The name of the widget.
//
// Note:       We set the tab bar to our own tab bar subclass so it will have
//             the ability to draw the text label in a highlight color.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 09:45:05 PDT 2007
//
// Modifications:
//   Brad Whitlock, Tue Aug 26 15:24:22 PDT 2008
//   Qt 4.
//
// ****************************************************************************

SpreadsheetTabWidget::SpreadsheetTabWidget(QWidget *parent) : QTabWidget(parent)
{
    highlight = palette().color(QPalette::Text);
    connect(tabBar(), SIGNAL(currentChanged(int)),
            this, SLOT(changeTabColors(int)));
}

// ****************************************************************************
// Method: SpreadsheetTabWidget::~SpreadsheetTabWidget
//
// Purpose: 
//   Destructor for the SpreadsheetTabWidget class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 09:46:24 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

SpreadsheetTabWidget::~SpreadsheetTabWidget()
{
}

// ****************************************************************************
// Method: SpreadsheetTabWidget::setHighlightColor
//
// Purpose: 
//   Sets the highlight color for the widget.
//
// Arguments:
//   c : The new highlight color.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 09:46:41 PDT 2007
//
// Modifications:
//   Brad Whitlock, Thu Aug 28 14:40:48 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
SpreadsheetTabWidget::setHighlightColor(const QColor &c)
{
    if(highlight != c)
    {
        highlight = c;
        changeTabColors(currentIndex());
    }
}

// ****************************************************************************
// Method: SpreadsheetTabWidget::highlightColor
//
// Purpose: 
//   Returns a reference to the highlight color.
//
// Returns:    The highlight color
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 09:47:06 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

const QColor &
SpreadsheetTabWidget::highlightColor() const
{
    return highlight;
}

//
// Qt slots
//

// ****************************************************************************
// Method: SpreadsheetWidget::changeTabColors
//
// Purpose: 
//   Sets the tab's highlight color when the active tab changes.
//
// Arguments:
//   index : The index of the new active tab.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 27 14:17:05 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetTabWidget::changeTabColors(int index)
{
    for(int i = 0; i < tabBar()->count(); ++i)
    {
        if(i == index)
            tabBar()->setTabTextColor(i, highlight);
        else
            tabBar()->setTabTextColor(i, palette().color(QPalette::Text));
    }
}
