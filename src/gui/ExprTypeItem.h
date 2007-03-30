#ifndef EXPR_TYPE_ITEM_H
#define EXPR_TYPE_ITEM_H
#include <gui_exports.h>

#include <qtable.h>
#include <Expression.h>

// Forward declarations
class QComboBox;

// ****************************************************************************
// Class: ExprTypeItem
//
// Purpose:
//   This class overrides QTableItem to display expression types.  It
//   understands how to create a QComboBox to edit the expression types. 
//   The list of types is queried from the Expression object.
//
// Notes:      
//
// Programmer: Sean Ahern
// Creation:   Wed Oct 10 17:02:51 PDT 2001
//
// Modifications:
//   
// ****************************************************************************
class GUI_API ExprTypeItem : public QObject, public QTableItem
{
    Q_OBJECT
  public:
    ExprTypeItem(QTable *t, EditType et, Expression::ExprType which);
    QWidget *createEditor() const;
    void setContentFromEditor(QWidget *w);
    void setText(const QString &s);

  private:
    QComboBox *menu;
    Expression::ExprType which;
};

#endif
