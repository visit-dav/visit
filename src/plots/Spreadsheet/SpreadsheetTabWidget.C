// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
