#include <qtable.h>
#include <ExpressionTable.h>

// ****************************************************************************
// Method: ExpressionTable::ExpressionTable
//
// Purpose: 
//   This is the constructor for the ExpressionTable class.
//
// Arguments:
//   numRows    The number of rows in the table initially
//   numCols    The number of columns in the table initially
//   parent     The parent widget
//   name       The name of the table
//
// Programmer: Sean Ahern
// Creation:   Fri Sep 14 13:34:45 PDT 2001
//
// ****************************************************************************
ExpressionTable::ExpressionTable(int numRows, int numCols,
                                 QWidget * parent, const char * name):
    QTable(numRows, numCols, parent, name)
{
}

// ****************************************************************************
// Method: ExpressionTable::StopEditing
//
// Purpose: 
//   Walks through all of the cells that could have editors working for
//   them currently and tells them to stop.  The contents of cell are then
//   updated with whatever the current contents of the editor are.
//
// Arguments:
//
// Programmer: Sean Ahern
// Creation:   Wed Oct 10 17:01:11 PDT 2001
//
// ****************************************************************************
void
ExpressionTable::StopEditing()
{
    for (int row = 0; row < numRows(); row++)
    {
        endEdit(row, 0, true, false);
        endEdit(row, 1, true, false);
        endEdit(row, 2, true, false);
    }
}
