#include <QvisVariableButtonHelper.h>
#include <QvisVariableButton.h>

// ****************************************************************************
// Method: QvisVariableButtonHelper::QvisVariableButtonHelper
//
// Purpose: 
//   Constructor for the QvisVariableButtonHelper class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 16:35:42 PST 2004
//
// Modifications:
//   
// ****************************************************************************

QvisVariableButtonHelper::QvisVariableButtonHelper(QObject *parent,
    const char *name) : QObject(parent, name)
{
    btn = 0;
}

// ****************************************************************************
// Method: QvisVariableButtonHelper::~QvisVariableButtonHelper
//
// Purpose: 
//   Destructor for the QvisVariableButtonHelper class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 16:35:42 PST 2004
//
// Modifications:
//   
// ****************************************************************************

QvisVariableButtonHelper::~QvisVariableButtonHelper()
{
}

// ****************************************************************************
// Method: QvisVariableButtonHelper::setButton
//
// Purpose: 
//   Sets the button to use to call the changeVariable method.
//
// Arguments:
//   b : The variable button to use.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 16:36:16 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisVariableButtonHelper::setButton(QvisVariableButton *b)
{
    btn = b;
}

// ****************************************************************************
// Method: QvisVariableButtonHelper::activated
//
// Purpose: 
//   This is a Qt slot function that is called for all variable menus.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 16:36:53 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisVariableButtonHelper::activated(int val, const QString &sval)
{
    if(btn)
        btn->changeVariable(val, sval);
}
