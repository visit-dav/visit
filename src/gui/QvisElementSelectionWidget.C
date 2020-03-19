// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisElementSelectionWidget.h>
#include <QvisPeriodicTableWidget.h>
#include <QApplication>
#include <QLayout>
#include <QMouseEvent>
#include <QPushButton>
#include <QTimer>

// ****************************************************************************
// Method: QvisElementSelectionWidget::QvisElementSelectionWidget
//
// Purpose: 
//   This is the constructor for the QvisElementSelectionWidget class.
//
// Arguments:
//   parent : The widget's parent.
//   name   : The name of the instance.
//   f      : The window flags.
//
// Note:       This widget acts as a layout for its children since it 
//   positions the children manually and dispatches mouse events to them.
//
// Programmer: Jeremy Meredith
// Creation:   February 11, 2008
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Tue Jun  3 14:44:38 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisElementSelectionWidget::QvisElementSelectionWidget(QWidget *parent,
    Qt::WindowFlags f) : QWidget(parent, f)
{
    // Create the timer.
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(hide()));

    // Create the periodic table widget.
    periodicTable = new QvisPeriodicTableWidget(this);
    periodicTable->setFrame(true);
    periodicTable->move(0, 0);
    periodicTable->resize(periodicTable->sizeHint());
    connect(periodicTable, SIGNAL(selectedElement(int)),
            this, SLOT(handleSelectedElement(int)));

    // Create the "Match any element" button
    matchAnyElementButton = new QPushButton(tr("Match any element"), this);
    matchAnyElementButton->move(0, periodicTable->sizeHint().height());
    matchAnyElementButton->resize(periodicTable->sizeHint().width(),
                                  matchAnyElementButton->sizeHint().height());
    connect(matchAnyElementButton, SIGNAL(clicked()),
            this, SLOT(matchAnyElementClicked()));

    // Turn on mouse tracking for this widget so we can dispatch mouse
    // events to child widgets.
    setMouseTracking(true);
}

// ****************************************************************************
// Method: QvisElementSelectionWidget::~QvisElementSelectionWidget
//
// Purpose: 
//   This is the destructor for the QvisElementSelectionWidget class.
//
// Programmer: Jeremy Meredith
// Creation:   February 11, 2008
//
// Modifications:
// ****************************************************************************

QvisElementSelectionWidget::~QvisElementSelectionWidget()
{
    // nothing here
}

// ****************************************************************************
// Method: QvisElementSelectionWidget::sizeHint
//
// Purpose: 
//   Calculates the widget's preferred size.
//
// Returns:    The widget's preferred size.
//
// Programmer: Jeremy Meredith
// Creation:   February 11, 2008
//
// Modifications:
// ****************************************************************************

QSize
QvisElementSelectionWidget::sizeHint() const
{
    QSize s1 = periodicTable->sizeHint();
    QSize s2 = matchAnyElementButton->sizeHint();

    // Find the largest width.
    int maxWidth = s1.width();
    if (s2.width() > maxWidth)
        maxWidth = s2.width();

    return QSize(maxWidth, s1.height() + s2.height());
}

// ****************************************************************************
// Method: QvisElementSelectionWidget::setSelectedElement
//
// Purpose: 
//   Tries to select the specified element in the periodic table widget.
//
// Arguments:
//   element : The element to select.
//
// Programmer: Jeremy Meredith
// Creation:   February 11, 2008
//
// Modifications:
// ****************************************************************************

void
QvisElementSelectionWidget::setSelectedElement(int element)
{
    periodicTable->setSelectedElement(element);
}

// ****************************************************************************
// Method: QvisElementSelectionWidget::mouseMoveEvent
//
// Purpose: 
//   This method is called in response to mouse movement in this widget. This
//   method's job is to dispatch the event to the proper child widget.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Jeremy Meredith
// Creation:   February 11, 2008
//
// Modifications:
// ****************************************************************************

void
QvisElementSelectionWidget::mouseMoveEvent(QMouseEvent *e)
{
    QRect standardRect(periodicTable->x(),
                       periodicTable->y(),
                       periodicTable->width(),
                       periodicTable->height());

    // If the mouse is in the periodic table gridd, forward the event
    if (standardRect.contains(QPoint(e->x(), e->y())))
    {
        QApplication::sendEvent(periodicTable, e);
    }
}

// ****************************************************************************
// Method: QvisElementSelectionWidget::mouseReleaseEvent
//
// Purpose: 
//   This method is called in response to the mouse button being released in
//   this widget. This method's job is to dispatch the event to the proper
//   child widget.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Jeremy Meredith
// Creation:   February 11, 2008
//
// Modifications:
// ****************************************************************************

void
QvisElementSelectionWidget::mouseReleaseEvent(QMouseEvent *e)
{
    QRect standardRect(periodicTable->x(),
                       periodicTable->y(),
                       periodicTable->width(),
                       periodicTable->height());
    QRect buttonRect(matchAnyElementButton->x(),
                     matchAnyElementButton->y(),
                     matchAnyElementButton->width(),
                     matchAnyElementButton->height());

    // If the mouse is in the grid, forward the event
    if (standardRect.contains(e->pos()))
    {
        QApplication::sendEvent(periodicTable, e);
    }
    else if (buttonRect.contains(e->pos()))
    {
        // Pretend that the button was clicked.
        matchAnyElementClicked();
    }
}

// ****************************************************************************
// Method: QvisElementSelectionWidget::matchAnyElementClicked
//
// Purpose: 
//   This selects the special wildcard element (index == -1).
//
// Programmer: Jeremy Meredith
// Creation:   February 11, 2008
//
// Modifications:
// ****************************************************************************

void
QvisElementSelectionWidget::matchAnyElementClicked()
{
    // Hide the widget
    hide();
 
    emit selectedElement(-1);
}

// ****************************************************************************
// Method: QvisElementSelectionWidget::show
//
// Purpose: 
//   This is a Qt slot function that shows the widget.
//
// Note:       Starts a timer to hide the widget.
//
// Programmer: Jeremy Meredith
// Creation:   February 11, 2008
//
// Modifications:
//   Brad Whitlock, Tue Jun  3 14:46:54 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisElementSelectionWidget::show()
{
    QWidget::show();
    timer->setSingleShot(true);
    timer->start(15000);
}

// ****************************************************************************
// Method: QvisElementSelectionWidget::hide()
//
// Purpose: 
//   This is a Qt slot function that hides the widget.
//
// Programmer: Jeremy Meredith
// Creation:   February 11, 2008
//
// Modifications:
// ****************************************************************************

void
QvisElementSelectionWidget::hide()
{
    QWidget::hide();
    timer->stop();
}

// ****************************************************************************
// Method: QvisElementSelectionWidget::handleSelectedElement
//
// Purpose: 
//   This is a Qt slot function that is called when an element is selected
//   using the periodic table grid.
//
// Arguments:
//   element : The selected element.
//
// Programmer: Jeremy Meredith
// Creation:   February 11, 2008
//
// Modifications:
// ****************************************************************************

void
QvisElementSelectionWidget::handleSelectedElement(int element)
{
    // Make sure the widget hides.
    hide();

    // Emit the element changed signal.
    emit selectedElement(element);
}


// ****************************************************************************
//  Method:  QvisElementSelectionWidget::setHintedElements
//
//  Purpose:
//    Accept a list of atomic numbers that we will use for hinting.
//
//  Arguments:
//    elements   the list of atomic numbers
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 12, 2008
//
// ****************************************************************************

void
QvisElementSelectionWidget::setHintedElements(const std::vector<int> &elements)
{
    periodicTable->setHintedElements(elements);
}
