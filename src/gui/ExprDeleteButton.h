#ifndef EXPR_DELETE_BUTTON_H
#define EXPR_DELETE_BUTTON_H
#include <gui_exports.h>

#include <qpushbutton.h>

// ****************************************************************************
// Class: ExprDeleteButton
//
// Purpose:
//   This class overrides QPushButton in one small way.  It takes a "row"
//   that's passed back in the "delete" signal.  That way, many of these
//   can be hooked to the same slot and have the slot differentiate between
//   the widgets.
//
// Notes:      
//
// Programmer: Sean Ahern
// Creation:   Fri Sep 28 15:07:39 PDT 2001
//
// Modifications:
//   
// ****************************************************************************
class GUI_API ExprDeleteButton : public QPushButton
{
    Q_OBJECT
  public:
    ExprDeleteButton(int row, const QString & text,
                     QWidget *parent = 0, const char *name = 0);

  signals:
    void deleted(int);

  protected slots:
    void sendDelete();

  private:
    int row;
};

#endif
