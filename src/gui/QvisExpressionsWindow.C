#include <QvisExpressionsWindow.h>
#include <Expression.h>
#include <ExpressionList.h>
#include <qapplication.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <ExprDeleteButton.h>
#include <ExprTypeItem.h>
#include <ExpressionTable.h>

#include <ViewerProxy.h>

#define STDMIN(A,B) (((A)<(B)) ? (A) : (B))
#define STDMAX(A,B) (((A)<(B)) ? (B) : (A))

// ****************************************************************************
// Method: QvisExpressionsWindow::QvisExpressionsWindow
//
// Purpose: 
//   This is the constructor for the QvisExpressionsWindow class.
//
// Arguments:
//   exprList_  The ExpressionList subject to observe
//   
//
// Programmer: Sean Ahern
// Creation:   Fri Sep 14 13:34:45 PDT 2001
//
// ****************************************************************************
QvisExpressionsWindow::QvisExpressionsWindow(
    ExpressionList *exprList_, const char *caption,
    const char *shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(exprList_, caption, shortName, notepad,
                               QvisPostableWindowObserver::ApplyButton,
                               false)
{
    exprList = exprList_;
}

// ****************************************************************************
// Method: QvisExpressionsWindow::~QvisExpressionsWindow
//
// Purpose: 
//   Destructor for the QvisExpressionsWindow class.
//
// Programmer: Sean Ahern
// Creation:   Fri Sep 14 13:34:32 PDT 2001
//
// Modifications:
//   
// ****************************************************************************
QvisExpressionsWindow::~QvisExpressionsWindow()
{
    exprList = NULL;
}

// ****************************************************************************
// Method: QvisExpressionsWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: Sean Ahern
// Creation:   Fri Sep 14 13:34:33 PDT 2001
//
// Modifications:
//   
// ****************************************************************************
void
QvisExpressionsWindow::CreateWindowContents()
{
    // Create the table that contains all of the expressions.
    table = new ExpressionTable(0, 5, central, "table");
    table->setShowGrid(true);
    table->setSelectionMode(QTable::NoSelection);
    QHeader *header = table->horizontalHeader();
    header->setLabel(0, tr("Name"));
    header->setLabel(1, tr("Definition"));
    header->setLabel(2, tr("Type"));
    header->setLabel(3, tr("Hidden"));
    header->setLabel(4, tr(""));
    topLayout->addWidget(table);

    // Create the list management buttons.
    QHBoxLayout *buttonArea = new QHBoxLayout(topLayout);

    newButton = new QPushButton("New", central, "newButton");
    buttonArea->addWidget(newButton);
    buttonArea->addStretch(1);
    connect(newButton, SIGNAL(clicked()), this, SLOT(addExpression()));
}

// ****************************************************************************
// Method: QvisExpressionsWindow::UpdateWindow
//
// Purpose: 
//   This method is called when the window's subject is changed. The
//   subject tells this window what attributes changed and we put the
//   new values into those widgets.
//
// Arguments:
//   doAll : If this flag is true, update all the widgets regardless
//           of whether or not they are selected.
//
// Returns:    
//
// Note:       
//
// Programmer: Sean Ahern
// Creation:   Fri Sep 14 13:34:34 PDT 2001
//
// Modifications:
//
// ****************************************************************************
void
QvisExpressionsWindow::UpdateWindow(bool doAll)
{
    // Update the table
    table->blockSignals(true);

    // Clear the old table
    for (int col = 0; col < 5; col++)
        for (int row = 0; row < table->numRows(); row++)
            table->clearCellWidget(row, col);

    // Check to make sure the table is big enough.
    int     nexp = exprList->GetNumExpressions();
    table->setNumRows(nexp);

    // Put in the expressions
    int     widths[] = {        // Minimum widths for the columns
        fontMetrics().width("Name"),
        fontMetrics().width("Definition"),
        fontMetrics().width("Type")
    };
    for (int row = 0; row < nexp; row++)
    {
        // The name of the expression
        const char *name = (*exprList)[row].GetName().c_str();
        table->setText(row, 0, QString(name));
        widths[0] = STDMAX(widths[0], fontMetrics().width(name));

        // Its definition
        const char *def = (*exprList)[row].GetDefinition().c_str();
        table->setText(row, 1, QString(def));
        widths[1] = STDMAX(widths[1], fontMetrics().width(def));

        // The type
        ExprTypeItem *ti = new ExprTypeItem(table, QTableItem::WhenCurrent,
                                            (*exprList)[row].GetType());
        table->setItem(row, 2, ti);
        widths[2] =
            STDMAX(widths[2],
                     fontMetrics().
                     width(Expression::
                           GetTypeString((*exprList)[row].GetType())));

        // The "hidden" flag
        QCheckBox *hid = new QCheckBox(NULL, table, "hidden");
        hid->setBackgroundColor(white); // XXX Hack.  I'm not positive
                                        // that the rest of the table cells
                                        // are drawn in white, though they
                                        // seem to be.  I can't query any
                                        // of the cells because none exist
                                        // at this point.
        hid->setChecked((*exprList)[row].GetHidden());
        table->setCellWidget(row, 3, hid);

        // The delete button
        ExprDeleteButton *d =
            new ExprDeleteButton(row, "Delete", table, "deleteButton");
        connect(d, SIGNAL(deleted(int)), this, SLOT(deleteExpression(int)));
        table->setCellWidget(row, 4, d);
    }

    // Fix up the column widths.
    int     pads[] = { 16, 16, 30 };
    if (nexp != 0)
    {
        for (int i = 0; i < 3; i++)
            table->setColumnWidth(i, widths[i] + pads[i]);
        table->adjustColumn(3);        // Hidden flag is automatic
    }

    table->blockSignals(false);
}

// ****************************************************************************
// Method: QvisExpressionsWindow::Apply
//
// Purpose: 
//   This is a Qt slot function that is called when the Apply button is clicked.
//
// Programmer: Sean Ahern
// Creation:   Fri Sep 14 13:34:37 PDT 2001
//
// Modifications:
//   
// ****************************************************************************
void
QvisExpressionsWindow::Apply(bool forceUpdate)
{
    // Tell all the cells to stop editing themselves
    table->StopEditing();

    // Read the values from the table
    int     nrows = table->numRows();
    for (int row = 0; row < nrows; row++)
    {
        QCheckBox *hidButton = (QCheckBox *) table->cellWidget(row, 3);

        (*exprList)[row].SetName(table->text(row, 0).ascii());
        (*exprList)[row].SetDefinition(table->text(row, 1).ascii());
        (*exprList)[row].SetType(Expression::GetTypeId(table->text(row,2).ascii()));
        (*exprList)[row].SetHidden(hidButton->isChecked());
    }

    exprList->Notify();

    if (AutoUpdate() || forceUpdate)
        viewer->ProcessExpressions();
}

//
// Qt Slot functions...
//

// ****************************************************************************
// Method: QvisExpressionsWindow::apply
//
// Purpose: 
//   This is a Qt slot function that is called when the window's Apply
//   button is clicked.
//
// Programmer: Sean Ahern
// Creation:   Fri Sep 14 13:34:37 PDT 2001
//
// Modifications:
//   
// ****************************************************************************
void
QvisExpressionsWindow::apply()
{
    Apply(true);
}

// ****************************************************************************
// Method: QvisExpressionsWindow::addExpression
//
// Purpose: 
//   This is a Qt slot function that adds a new expression that is empty.
//
// Programmer: Sean Ahern
// Creation:   Fri Sep 14 13:34:42 PDT 2001
//
// Modifications:
//   
// ****************************************************************************
void
QvisExpressionsWindow::addExpression()
{
    // Update the current contents of the window.
    Apply();

    Expression *e = new Expression();
    e->SetName("?");
    e->SetDefinition("?");

    exprList->AddExpression(*e);
    delete(e);

    exprList->Notify();
}

// ****************************************************************************
// Method: QvisExpressionsWindow::deleteExpression
//
// Purpose: 
//   This is a Qt slot function that is called to delete an expression.
//
// Programmer: Sean Ahern
// Creation:   Fri Sep 14 13:34:42 PDT 2001
//
// Modifications:
//   
// ****************************************************************************
void
QvisExpressionsWindow::deleteExpression(int row)
{
    // Update the current contents of the window
    Apply();

    exprList->RemoveExpression(row);
    exprList->Notify();
}
