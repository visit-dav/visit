#include <QvisParallelAxisPlotWindow.h>
#include <ParallelAxisAttributes.h>

#include <ViewerProxy.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <qhbuttongroup.h>
#include <qvbuttongroup.h>
#include <qbuttongroup.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qbutton.h>
#include <qlabel.h>
#include <qframe.h>
#include <qlayout.h>
#include <qstring.h>
#include <qbitmap.h>

#include <QvisVariableButton.h>

#include <DebugStream.h>

#include <vector>
#include <string>


// ****************************************************************************
// Method: QvisParallelAxisPlotWindow::QvisParallelAxisPlotWindow
//
// Purpose: Constructor for the QvisParallelAxisPlotWindow class.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//
// ****************************************************************************

QvisParallelAxisPlotWindow::QvisParallelAxisPlotWindow(const int type,
    ParallelAxisAttributes *parAxisAtts_, const char *caption,
    const char *shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(parAxisAtts_, caption, shortName, notepad)
{
    plotType = type;
    parAxisAtts = parAxisAtts_;

    latestGUIShownPos = parAxisAtts->GetShownVariableAxisPosition();
}


// ****************************************************************************
// Method: QvisParallelAxisPlotWindow::~QvisParallelAxisPlotWindow
//
// Purpose: Destructor for the QvisParallelAxisPlotWindow class.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QvisParallelAxisPlotWindow::~QvisParallelAxisPlotWindow()
{
    parAxisAtts = NULL;
}


// ****************************************************************************
// Method: QvisParallelAxisPlotWindow::CreateWindowContents
//
// Purpose: This method creates the widgets that are in the window and sets up
//          their signals/slots.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//
//      Mark Blair, Wed Aug 16 17:12:00 PDT 2006
//      Removed widgets that display axis extents and extents selected by
//      Extents tool.  These were considered unnecessary.
//
// ****************************************************************************

void
QvisParallelAxisPlotWindow::CreateWindowContents()
{
    static unsigned char leftArrow[8] =
        { 0x00, 0x01, 0x07, 0x1f, 0x7f, 0x1f, 0x07, 0x01 };
    static unsigned char rightArrow[8] =
        { 0x00, 0x80, 0xe0, 0xf8, 0xfe, 0xf8, 0xe0, 0x80 };

    QGridLayout *mainLayout = new QGridLayout(topLayout, 6, 2, 10, "mainLayout");

    mainLayout->addWidget(new QLabel("Axis var:", central, "axisVarLabel"), 0, 0);
    axisVariable = new QLabel("default", central, "axisVariable");
    mainLayout->addMultiCellWidget(axisVariable, 0, 0, 1, 2);
    mainLayout->addWidget(new QLabel("Axis pos:", central, "axisPosLabel"), 0, 3);
    axisPosition = new QLabel("0", central, "axisPosition");
    mainLayout->addWidget(axisPosition, 0, 4);

    QButtonGroup *prevAxisOrNext = new QButtonGroup(central, "prevAxisOrNext");
    prevAxisOrNext->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *prevOrNextLayout = new QHBoxLayout(prevAxisOrNext);
    prevOrNextLayout->setSpacing(0);
    showPrevAxis = new QPushButton(prevAxisOrNext);
    QBitmap *leftArrowBitmap = new QBitmap(8, 8, leftArrow);
    showPrevAxis->setPixmap(*leftArrowBitmap);
    prevOrNextLayout->addWidget(showPrevAxis);
    connect(showPrevAxis, SIGNAL(clicked()), this, SLOT(prevAxisClicked()));
    showNextAxis = new QPushButton(prevAxisOrNext);
    QBitmap* rightArrowBitmap = new QBitmap(8, 8, rightArrow);
    showNextAxis->setPixmap(*rightArrowBitmap);
    prevOrNextLayout->addWidget(showNextAxis);
    connect(showNextAxis, SIGNAL(clicked()), this, SLOT(nextAxisClicked()));
    mainLayout->addWidget(prevAxisOrNext, 0, 5);

    addAxis = new QvisVariableButton(false, true, true,
        QvisVariableButton::Scalars, central, "addAxis");
    addAxis->setText("Add axis");
    addAxis->setChangeTextOnVariableChange(false);
    connect(addAxis, SIGNAL(activated(const QString &)),
            this, SLOT(axisAdded(const QString &)));
    mainLayout->addMultiCellWidget(addAxis, 1, 1, 0, 1);
    deleteAxis = new QvisVariableButton(false, true, true,
        QvisVariableButton::Scalars, central, "deleteAxis");
    deleteAxis->setText("Delete axis");
    deleteAxis->setChangeTextOnVariableChange(false);
    connect(deleteAxis, SIGNAL(activated(const QString &)),
            this, SLOT(axisDeleted(const QString &)));
    mainLayout->addMultiCellWidget(deleteAxis, 1, 1, 2, 3);
    leftAxis = new QvisVariableButton(false, true, true,
        QvisVariableButton::Scalars, central, "leftAxis");
    leftAxis->setText("Left axis");
    leftAxis->setChangeTextOnVariableChange(false);
    connect(leftAxis, SIGNAL(activated(const QString &)),
            this, SLOT(leftAxisSelected(const QString &)));
    mainLayout->addMultiCellWidget(leftAxis, 1, 1, 4, 5);
}


// ****************************************************************************
// Method: QvisParallelAxisPlotWindow::UpdateWindow
//
// Purpose: This method is called when the window's subject is changed.  The
//          subject tells this window what attributes changed and the new
//          values are then assigned to corresponding widgets.
//
// Arguments:
//   doAll : If this flag is true, update all the widgets regardless of whether
//           or not they are selected.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//
//      Mark Blair, Wed Aug 16 17:12:00 PDT 2006
//      Removed widgets that display axis extents and extents selected by
//      Extents tool.  These were considered unnecessary.
//
//      Mark Blair, Wed Sep 20 10:59:41 PDT 2006
//      Accommodates time ordinals, for those operators and tools that need them.
//
// ****************************************************************************

void
QvisParallelAxisPlotWindow::UpdateWindow(bool doAll)
{
    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.

    for (int index = 0; index < parAxisAtts->NumAttributes(); index++)
    {
        if (!doAll) {
            if (!parAxisAtts->IsSelected(index)) continue;
        }

        switch (index)
        {
        case 0:   // Uses ParallelAxisAttributes::orderedAxisNames

            break;   // Do nothing.

        case 1:   // Uses ParallelAxisAttributes::shownVarAxisPosition

            UpdateShownFields(false);

            break;

        case 2:   // Uses ParallelAxisAttributes::axisMinima

            break;   // Do nothing.

        case 3:   // Uses ParallelAxisAttributes::axisMaxima

            break;   // Do nothing.

        case 4:   // Uses ParallelAxisAttributes::extentMinima

            break;   // Do nothing.

        case 5:   // Uses ParallelAxisAttributes::extentMaxima

            break;   // Do nothing.

        case 6:   // Uses ParallelAxisAttributes::extMinTimeOrds

            break;   // Do nothing.

        case 7:   // Uses ParallelAxisAttributes::extMaxTimeOrds

            break;   // Do nothing.
        }
    } // end for
}


// ****************************************************************************
// Method: QvisParallelAxisPlotWindow::GetCurrentValues
//
// Purpose: Gets the current values from the text fields and assigns those
//          values to the appropriate subject attributes.
//
// Arguments:
//   which_widget : A number indicating which line edit should have its current
//                  values read.  An index of -1 gets them all.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisParallelAxisPlotWindow::GetCurrentValues(int which_widget)
{
    // Eventually the ParallelAxis GUI may have some text fields.
    return;
}


// ****************************************************************************
// Method: QvisParallelAxisPlotWindow::Apply
//
// Purpose: This method applies the plot attributes and optionally tells the
//          viewer to apply them.
//
// Arguments:
//   ignore : This flag, when true, signifies that the AutoUpdate function
//            should be ignored and that the viewer should apply the plot
//            attributes.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//
// ****************************************************************************

void
QvisParallelAxisPlotWindow::Apply(bool ignore)
{
    if (AutoUpdate() || ignore)
    {
        // Get the current ParallelAxis plot attributes and tell the other
        // observers about them.
        GetCurrentValues(-1);
        parAxisAtts->Notify();

        // Tell the viewer to set the ParallelAxis plot attributes.
        viewer->SetPlotOptions(plotType);
    }
    else
    {
        parAxisAtts->Notify();
    }
}


/////////////////////////////////////////////////////////////////////////////////
// Qt Slot functions...
/////////////////////////////////////////////////////////////////////////////////


// ****************************************************************************
// Method: QvisParallelAxisPlotWindow::apply
//
// Purpose: This is a Qt slot function that is called when the window's Apply
//          button is clicked.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisParallelAxisPlotWindow::apply()
{
    Apply(true);
}


// ****************************************************************************
// Method: QvisParallelAxisPlotWindow::makeDefault
//
// Purpose: This is a Qt slot function that is called when the window's
//          "Make default" button is clicked.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisParallelAxisPlotWindow::makeDefault()
{
    // Tell the viewer to set the default high dimension plot attributes.
    GetCurrentValues(-1);
    parAxisAtts->Notify();
    viewer->SetDefaultPlotOptions(plotType);
}


// ****************************************************************************
// Method: QvisParallelAxisPlotWindow::reset
//
// Purpose: This is a Qt slot function that is called when the window's Reset
//          button is clicked.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisParallelAxisPlotWindow::reset()
{
    // Tell the viewer to reset the ParallelAxis plot attributes to the last
    // applied values.

/*
    viewer->ResetPlotOptions(plotType);
*/
}


// ****************************************************************************
// Method: QvisParallelAxisPlotWindow::prevAxisClicked
//
// Purpose: This is a Qt slot function that is called when the window's
//          "Previous Axis" button (a left arrow) is clicked.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisParallelAxisPlotWindow::prevAxisClicked()
{
    parAxisAtts->ShowPreviousAxisVariableData();
    latestGUIShownPos = parAxisAtts->GetShownVariableAxisPosition();

    UpdateShownFields(true);
}


// ****************************************************************************
// Method: QvisParallelAxisPlotWindow::prevAxisClicked
//
// Purpose: This is a Qt slot function that is called when the window's
//          "Next Axis" button (a right arrow) is clicked.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisParallelAxisPlotWindow::nextAxisClicked()
{
    parAxisAtts->ShowNextAxisVariableData();
    latestGUIShownPos = parAxisAtts->GetShownVariableAxisPosition();

    UpdateShownFields(true);
}


// ****************************************************************************
// Method: QvisParallelAxisPlotWindow::axisAdded
//
// Purpose: This is a Qt slot function that is called when the user adds an
//          axis to the plot by clicking the "Add axis" button and selecting
//          a scalar or an expression from a cascaded list.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisParallelAxisPlotWindow::axisAdded(const QString &axisToAdd)
{
    parAxisAtts->InsertAxis(axisToAdd.latin1());
    latestGUIShownPos = parAxisAtts->GetShownVariableAxisPosition();

    UpdateShownFields(true);
}


// ****************************************************************************
// Method: QvisParallelAxisPlotWindow::axisDeleted
//
// Purpose: This is a Qt slot function that is called when the user deletes an
//          axis from the plot by clicking the "Delete axis" button and
//          selecting a scalar or an expression from a cascaded list.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisParallelAxisPlotWindow::axisDeleted(const QString &axisToDelete)
{
    parAxisAtts->DeleteAxis(axisToDelete.latin1(), 2);
    latestGUIShownPos = parAxisAtts->GetShownVariableAxisPosition();

    UpdateShownFields(true);
}


// ****************************************************************************
// Method: QvisParallelAxisPlotWindow::leftAxisSelected
//
// Purpose: This is a Qt slot function that is called when the user specifies
//          an axis to be the plot's leftmost axis by clicking the "Left axis"
//          button and selecting a scalar or an expression from a cascaded list.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisParallelAxisPlotWindow::leftAxisSelected(const QString &axisToSelect)
{
    parAxisAtts->SwitchToLeftAxis(axisToSelect.latin1());
    latestGUIShownPos = parAxisAtts->GetShownVariableAxisPosition();

    UpdateShownFields(true);
}


// ****************************************************************************
// Method: QvisParallelAxisPlotWindow::UpdateShownFields
//
// Purpose: Updates all widgets in the window to display values corresponding
//          to the axis variable whose data is to be shown.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
//      Mark Blair, Wed Aug 16 17:12:00 PDT 2006
//      Removed widgets that display axis extents and extents selected by
//      Extents tool.  These were considered unnecessary.
//
//      Mark Blair, Wed Sep 20 10:59:41 PDT 2006
//      Fixed range error that can occur when shown variable and list of axis
//      are temporarily out of sync.
//
// ****************************************************************************

void
QvisParallelAxisPlotWindow::UpdateShownFields(bool applyvalues)
{
    if (latestGUIShownPos >= parAxisAtts->GetAxisMinima().size())
    {
        debug1 << "ParallelAxis plot attribute consistency error." << endl;
        latestGUIShownPos = parAxisAtts->GetAxisMinima().size() - 1;
    }

    parAxisAtts->SetShownVariableAxisPosition(latestGUIShownPos);

    QString fieldString = parAxisAtts->GetShownVariableAxisName().c_str();
    axisVariable->setText(fieldString);

    fieldString.setNum(parAxisAtts->GetShownVariableAxisNormalHumanPosition());
    axisPosition->setText(fieldString);

    if (applyvalues)
    {
        SetUpdate(false);
        Apply();
    }
}
