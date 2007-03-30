#include <stdio.h>
#include <qcombobox.h>
#include <qtable.h>
#include <Expression.h>
#include <ExprTypeItem.h>

//
// Most of this code was stolen from the Qt examples from Trolltech, from
// the file examples/statistics/statistics.cpp.
//

// ****************************************************************************
// Method: ExprTypeItem::ExprTypeItem
//
// Purpose: 
//   Constructor for an ExprTypeItem.
//
// Arguments:
//   t      The table that this table item is associated with
//   et     The way that this item should be edited (see Qt docs)
//   _which Which type is selected by default
//
// Programmer: Sean Ahern
// Creation:   Wed Oct 10 17:01:11 PDT 2001
//
// ****************************************************************************
ExprTypeItem::ExprTypeItem(QTable *t, EditType et,
                           Expression::ExprType _which):
    QTableItem(t, et, "unset")
{
    which = _which;
    menu = NULL;

    // We do not want this item to be replaced
    setReplaceable(false);

    setText(Expression::GetTypeString(which));
}

// ****************************************************************************
// Method: ExprTypeItem::createEditor
//
// Purpose: 
//   Creates a QComboBox to edit the type.
//
// Note:
//   This method is required to be const to override the virtual function
//   correctly. 
//
// Note:
//   This function is only called internally by Qt.
//
// Arguments:
//
// Programmer: Sean Ahern
// Creation:   Wed Oct 10 17:01:11 PDT 2001
//
// ****************************************************************************
QWidget *
ExprTypeItem::createEditor() const
{
    // Create a comboBox editor.
    // 
    // The funky stuff with (ExirTypeItem*)this is to get around the fact
    // that we have to put "const" in our prototype.
    ((ExprTypeItem*)this)->menu = new QComboBox(table()->viewport());

    // Build up the menu so that it contains the list of all possible types.
    for(int i=0;i<Expression::GetNumTypes()-1;i++)
        menu->insertItem(Expression::GetTypeString((Expression::ExprType)i));
    menu->setCurrentItem(int(which));

    return menu;
}

// ****************************************************************************
// Method: ExprTypeItem::setContentFromEditor
//
// Purpose: 
//   Takes the editor that was created in ExprTypeItem::createEditor and
//   figures out what type is chosen.  It then sets the text to be the
//   string representation of that type.
//
// Note:
//   This function is only called internally by Qt.
//
// Arguments:
//   w      The editor widget.
//
// Programmer: Sean Ahern
// Creation:   Wed Oct 10 17:01:11 PDT 2001
//
// ****************************************************************************
void
ExprTypeItem::setContentFromEditor(QWidget *w)
{
    // The user changed the value of the menu, so synchronize the value of
    // the item (text) with the value of the menu.
    if (w->inherits("QComboBox"))
        setText(((QComboBox*)w)->currentText());
    else
        QTableItem::setContentFromEditor(w);
}

// ****************************************************************************
// Method: ExprTypeItem::setText
//
// Purpose: 
//   Sets the displayed content of the ExprTypeItem.  We also update the
//   menu editor while we're at it.
//
// Arguments:
//   s      The string to set.
//
// Programmer: Sean Ahern
// Creation:   Wed Oct 10 17:01:11 PDT 2001
//
// ****************************************************************************
void
ExprTypeItem::setText(const QString &s)
{
    if (menu)
        menu->setCurrentItem(Expression::GetTypeId(s.ascii()));
    QTableItem::setText(s);
}
