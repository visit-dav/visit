#include <ExprDeleteButton.h>

// ****************************************************************************
// Method: ExprDeleteButton::ExprDeleteButton
//
// Purpose: 
//   This is the constructor for the ExprDeleteButton class.
//
// Arguments:
//   
// Programmer: Sean Ahern
// Creation:   Fri Sep 14 13:34:45 PDT 2001
//
// ****************************************************************************
ExprDeleteButton::ExprDeleteButton(int _row, const QString & text,
                                           QWidget *parent,
                                           const char *name):
    QPushButton(text, parent, name )
{
    row = _row;
    connect(this, SIGNAL(clicked()),
            this, SLOT(sendDelete()));
}

// ****************************************************************************
// Method: ExprDeleteButton::sendDelete
//
// Purpose: 
//   This slot just emits the "deleted" signal, containing the row
//   information we were given at startup.
//
// Arguments:
//   
// Programmer: Sean Ahern
// Creation:   Fri Sep 28 15:06:13 PDT 2001
//
// ****************************************************************************
void
ExprDeleteButton::sendDelete()
{
    emit deleted(row);
}
