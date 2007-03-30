#ifndef EXPRESSION_TABLE_H
#define EXPRESSION_TABLE_H
#include <gui_exports.h>

#include <qtable.h>

// ****************************************************************************
// Class: ExpressionsTable
//
// Purpose:
//   This class overrides QTable in one way.  It allows us to easily stop
//   the editing of cells so that we can update the pertinent subject.
//
// Notes:      
//
// Programmer: Sean Ahern
// Creation:   Fri Sep 14 13:59:13 PDT 2001
//
// Modifications:
//   
// ****************************************************************************
class GUI_API ExpressionTable : public QTable
{
    Q_OBJECT
  public:
    ExpressionTable(int numRows, int numCols, QWidget * parent = 0,
                    const char * name = 0);
    void StopEditing();
};

#endif
