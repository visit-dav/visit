#include <QvisExpressionsWindow.h>

#include <Expression.h>
#include <ExpressionList.h>
#include <ViewerProxy.h>

#include <qapplication.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qmultilineedit.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qsplitter.h>

#define STDMIN(A,B) (((A)<(B)) ? (A) : (B))
#define STDMAX(A,B) (((A)<(B)) ? (B) : (A))

const char *known_functions[] = {
        "abs",
        "acos",
        "and",
        "area",
        "asin",
        "aspect",
        "aspect_gamma",
        "atan",
        "condition",
        "coord",
        "coords",
        "cos",
        "cross",
        "deg2rad",
        "degree",
        "det",
        "determinant",
        "diagonal",
        "dimension",
        "dot",
        "effective_tensor",
        "eigenvalue",
        "eigenvector",
        "eq",
        "equal",
        "equals",
        "ge",
        "gradient",
        "gt",
        "gte",
        "if",
        "inverse",
        "jacobian",
        "largest_angle",
        "le",
        "ln",
        "log",
        "log10",
        "lt",
        "lte",
        "magnitude",
        "matvf",
        "ne",
        "neighbor",
        "neq",
        "nmats",
        "node_degree",
        "not",
        "notequal",
        "notequals",
        "oddy",
        "or",
        "polar",
        "principal_deviatoric_tensor",
        "principal_tensor",
        "procid",
        "rad2deg",
        "rand",
        "random",
        "recenter",
        "relative_size",
        "revolved_surface_area",
        "revolved_volume",
        "scaled_jacobian",
        "shape",
        "shape_and_size",
        "shear",
        "sin",
        "skew",
        "smallest_angle",
        "specmf",
        "sq",
        "sqr",
        "sqrt",
        "stretch",
        "tan",
        "taper",
        "tensor_maximum_shear",
        "trace",
        "volume",
        "warpage",
        NULL
};

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
// Programmer: Jeremy Meredith
// Creation:   October 10, 2004
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
// Programmer: Jeremy Meredith
// Creation:   October 10, 2004
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
// Programmer: Jeremy Meredith
// Creation:   October 10, 2004
//
// Note: as of writing, unsure if I want to use the splitter or not.
//       We should pick one of the two methods eventually.
//
// Modifications:
//   
// ****************************************************************************
void
QvisExpressionsWindow::CreateWindowContents()
{
//#define USE_SPLITTER
#ifndef USE_SPLITTER
    QHBoxLayout *mainLayout = new QHBoxLayout(topLayout, -1, "mainLayout");
#else
    QSplitter *mainSplitter = new QSplitter(central);
    topLayout->addWidget(mainSplitter);
    topLayout->setStretchFactor(mainSplitter, 100);
#endif

#ifndef USE_SPLITTER
    QGridLayout *listLayout = new QGridLayout(mainLayout, 3,2, 5, "listLayout");
    mainLayout->setStretchFactor(listLayout, 30);
#define f1 central
#else
    QFrame *f1 = new QFrame(mainSplitter);
    QGridLayout *listLayout = new QGridLayout(f1, 3,2, 0, 5, "listLayout");
#endif

    exprListBox = new QListBox(f1);
    listLayout->addMultiCellWidget(exprListBox, 0,0, 0,1);

    newButton = new QPushButton("New", f1, "newButton");
    listLayout->addWidget(newButton, 1,0);

    delButton = new QPushButton("Del", f1, "delButton");
    listLayout->addWidget(delButton, 1,1);

    displayAllVars = new QCheckBox("Display all", f1, "displayAllVars");
    listLayout->addMultiCellWidget(displayAllVars, 2,2, 0,1);

#ifndef USE_SPLITTER
    mainLayout->addSpacing(10);
    QGridLayout *editLayout = new QGridLayout(mainLayout, 6,2, 5, "editLayout");
    mainLayout->setStretchFactor(editLayout, 70);
#define f2 central
#else
    QFrame *f2 = new QFrame(mainSplitter);
    QGridLayout *editLayout = new QGridLayout(f2, 7,2, 0, 5, "editLayout");
#endif
    editLayout->addColSpacing(1,20);
    int row = 0;

    nameEditLabel = new QLabel("Name", f2, "nameEditLabel");
    nameEdit = new QLineEdit(f2, "nameEdit");
    editLayout->addWidget(nameEditLabel, row, 0);
    editLayout->addWidget(nameEdit, row, 1);
    row++;

    typeLabel = new QLabel("Type", f2, "typeLabel");
    typeList = new QComboBox(f2, "typeList");
    int numtypes = Expression::GetNumTypes();
    numtypes = 5;  // HACK!!!  Variable types after 5 currently fail.  FIXME!!!
    for (int i=1; i < numtypes ; i++)
        typeList->insertItem(Expression::GetTypeString((Expression::ExprType)i));
    editLayout->addWidget(typeLabel, row, 0);
    editLayout->addWidget(typeList, row, 1);
    row++;

    hidden = new QCheckBox("Hidden", f2, "hidden");
    editLayout->addMultiCellWidget(hidden, row,row, 0,1);
    row++;

    definitionEditLabel = new QLabel("Definition", f2, "definitionEditLabel");
    editLayout->addWidget(definitionEditLabel, row, 0);
    row++;

    definitionEdit = new QMultiLineEdit(f2, "definitionEdit");
    definitionEdit->setWordWrap(QMultiLineEdit::WidgetWidth);
    definitionEdit->setWrapPolicy(QMultiLineEdit::AtWhiteSpace);
    editLayout->addMultiCellWidget(definitionEdit, row,row, 0,1);
    row++;

    editLayout->addRowSpacing(row, 20);
    row++;

    insertFunctionButton = new QPushButton("Insert Function", f2);
    insertFunctionMenu = new QPopupMenu(f2, "insertFunctionMenu");
    for (int i=0; known_functions[i]; i++)
        insertFunctionMenu->insertItem(known_functions[i]);
    insertFunctionButton->setPopup(insertFunctionMenu);
    editLayout->addWidget(insertFunctionButton, row, 0);

    // connect signals

    connect(exprListBox, SIGNAL(selectionChanged()),
            this, SLOT(UpdateWindowSingleItem()));
    connect(nameEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(nameTextChanged(const QString&)));
    connect(definitionEdit, SIGNAL(textChanged()),
            this, SLOT(definitionTextChanged()));
    connect(newButton, SIGNAL(pressed()),
            this, SLOT(addExpression()));
    connect(delButton, SIGNAL(pressed()),
            this, SLOT(delExpression()));
    connect(typeList, SIGNAL(activated(int)),
            this, SLOT(typeChanged(int)));
    connect(hidden, SIGNAL(clicked()),
            this, SLOT(hiddenChanged()));
    connect(displayAllVars, SIGNAL(clicked()),
            this, SLOT(displayAllVarsChanged()));
    connect(insertFunctionMenu, SIGNAL(activated(int)),
            this, SLOT(insertFunction(int)));
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
//           of whether or not they are selected.  Currently ignored.
//
// Programmer: Jeremy Meredith
// Creation:   October 10, 2004
//
// Modifications:
//
// ****************************************************************************
void
QvisExpressionsWindow::UpdateWindow(bool)
{
    BlockAllSignals(true);

    exprListBox->clear();
    for (int i=0; i<exprList->GetNumExpressions(); i++)
    {
        if (displayAllVars->isChecked() ||
            !exprList->GetExpression(i).GetFromDB())
        {
            exprListBox->insertItem(exprList->GetExpression(i).GetName());
        }
    }
    BlockAllSignals(false);

    UpdateWindowSingleItem();
}

// ****************************************************************************
//  Method:  QvisExpressionsWindow::UpdateWindowSingleItem
//
//  Purpose:
//    Update the pane of the window where a single expression is being edited.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 10, 2004
//
// ****************************************************************************
void
QvisExpressionsWindow::UpdateWindowSingleItem()
{
    BlockAllSignals(true);
    int index = exprListBox->currentItem();

    if (index == -1)
    {
        nameEdit->setText("");
        hidden->setChecked(false);
        definitionEdit->setText("");
    }
    else
    {
        Expression *e = (*exprList)[exprListBox->currentText()];
        if (!e)
        {
            nameEdit->setText("");
            hidden->setChecked(false);
            definitionEdit->setText("");
        }
        else
        {
            nameEdit->setText(e->GetName());
            hidden->setChecked(e->GetHidden());
            typeList->setCurrentText(Expression::GetTypeString(e->GetType()));
            definitionEdit->setText(e->GetDefinition());
        }
    }
    
    UpdateWindowSensitivity();
    BlockAllSignals(false);
}

// ****************************************************************************
//  Method:  QvisExpressionsWindow::UpdateWindowSensitivity
//
//  Purpose:
//    Update the sensitivity of any widget needing it.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 10, 2004
//
// ****************************************************************************
void
QvisExpressionsWindow::UpdateWindowSensitivity()
{
    bool enable = true;
    if (exprListBox->currentItem() == -1)
    {
        enable = false;
    }
    else if ((*exprList)[exprListBox->currentText().latin1()] &&
             (*exprList)[exprListBox->currentText().latin1()]->GetFromDB())
    {
        enable = false;
    }

    nameEdit->setEnabled(enable);
    delButton->setEnabled(enable);
    hidden->setEnabled(enable);
    typeList->setEnabled(enable);
    definitionEdit->setEnabled(enable);
    insertFunctionButton->setEnabled(enable);
}

// ****************************************************************************
//  Method:  QvisExpressionsWindow::BlockAllSignals
//
//  Purpose:
//    Block signals for all important widgets.  Or unblock, of course.
//
//  Arguments:
//    block      true to block, false to unblock
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 10, 2004
//
// ****************************************************************************
void
QvisExpressionsWindow::BlockAllSignals(bool block)
{
    exprListBox->blockSignals(block);
    nameEdit->blockSignals(block);
    definitionEdit->blockSignals(block);
}

// ****************************************************************************
// Method: QvisExpressionsWindow::Apply
//
// Purpose: 
//   This is a Qt slot function that is called when the Apply button is clicked.
//
// Programmer: Jeremy Meredith
// Creation:   October 10, 2004
//
// Modifications:
//   
// ****************************************************************************
void
QvisExpressionsWindow::Apply(bool forceUpdate)
{
    exprList->Notify();

    if (AutoUpdate() || forceUpdate)
        viewer->ProcessExpressions();
}

// ****************************************************************************
// Method: QvisExpressionsWindow::apply
//
// Purpose: 
//   This is a Qt slot function that is called when the window's Apply
//   button is clicked.
//
// Programmer: Jeremy Meredith
// Creation:   October 10, 2004
//
// Modifications:
//   
// ****************************************************************************
void
QvisExpressionsWindow::apply()
{
    // Apply will recreate the window and unselect the current expression.
    // If we have an expression selected, reselect it afterwards.
    int reselect = (exprListBox->currentItem() != -1);
    QString item = exprListBox->currentText();

    Apply(true);

    if (reselect)
    {
        for (int i=0; i<exprListBox->count(); i++)
        {
            if (exprListBox->text(i) == item)
                exprListBox->setSelected(i, true);
        }
    }
}

// ****************************************************************************
// Method: QvisExpressionsWindow::addExpression
//
// Purpose: 
//   This is a Qt slot function that adds a new expression that is empty.
//
// Programmer: Jeremy Meredith
// Creation:   October 10, 2004
//
// Modifications:
//   
// ****************************************************************************
void
QvisExpressionsWindow::addExpression()
{
    // Find an unused expression name
    int newid = 1;
    bool okay = false;
    QString newName;
    while (!okay)
    {
        newName.sprintf("unnamed%d", newid);
        if ((*exprList)[newName.latin1()])
            newid++;
        else
            okay = true;
    }

    Expression e;
    e.SetName(newName.latin1());
    e.SetDefinition("");
    exprList->AddExpression(e);

    exprList->Notify();

    for (int i=0; i<exprListBox->count(); i++)
    {
        if (exprListBox->text(i) == newName)
            exprListBox->setSelected(i, true);
    }
}

// ****************************************************************************
// Method: QvisExpressionsWindow::delExpression
//
// Purpose: 
//   This is a Qt slot function that is called to delete an expression.
//
// Programmer: Jeremy Meredith
// Creation:   October 10, 2004
//
// Modifications:
//   
// ****************************************************************************
void
QvisExpressionsWindow::delExpression()
{
    int index = exprListBox->currentItem();

    if (index == -1)
        return;

    for (int i=0; i<exprList->GetNumExpressions(); i++)
    {
        if (exprList->GetExpression(i).GetName() == 
            exprListBox->currentText().latin1())
        {
            exprList->RemoveExpression(i);
            break;
        }
    }

    exprList->Notify();
}

// ****************************************************************************
//  Method:  QvisExpressionsWindow::definitionTextChanged
//
//  Purpose:
//    Slot function when definition text is changed.  This is called for
//    any change to the definition text.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 10, 2004
//
// ****************************************************************************
void
QvisExpressionsWindow::definitionTextChanged()
{
    int index = exprListBox->currentItem();
    if (index == -1)
        return;

    Expression *e = (*exprList)[exprListBox->currentText()];
    if (!e)
        return;

    e->SetDefinition(definitionEdit->text());
}

// ****************************************************************************
//  Method:  QvisExpressionsWindow::nameTextChanged
//
//  Purpose:
//    Slot function when any change happens to the expression names.
//
//  Arguments:
//    text       the new text.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 10, 2004
//
// ****************************************************************************
void
QvisExpressionsWindow::nameTextChanged(const QString &text)
{
    int index = exprListBox->currentItem();
    if (index == -1)
        return;

    Expression *e = (*exprList)[exprListBox->currentText()];
    if (!e)
        return;

    QString newname = text.stripWhiteSpace();
    e->SetName(newname);
    BlockAllSignals(true);
    exprListBox->changeItem(newname, index);
    BlockAllSignals(false);
}

// ****************************************************************************
//  Method:  QvisExpressionsWindow::typeChanged
//
//  Purpose:
//    Slot function for the "type" combo box.  Sets the expression type.
//
//  Arguments:
//    value      the new type
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 10, 2004
//
// ****************************************************************************
void
QvisExpressionsWindow::typeChanged(int value)
{
    int index = exprListBox->currentItem();
    if (index == -1)
        return;

    Expression *e = (*exprList)[exprListBox->currentText()];
    if (!e)
        return;

    // Add one because we skipped index 0 (unknown) when creating the
    // list box originally
    e->SetType(Expression::ExprType(value+1));
}

// ****************************************************************************
//  Method:  QvisExpressionsWindow::hiddenChanged
//
//  Purpose:
//    Slot function for the hidden toggle.
//    Sets the "hidden" flag of an expression.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 10, 2004
//
// ****************************************************************************
void
QvisExpressionsWindow::hiddenChanged()
{
    int index = exprListBox->currentItem();
    if (index == -1)
        return;

    Expression *e = (*exprList)[exprListBox->currentText()];
    if (!e)
        return;

    e->SetHidden(hidden->isChecked());
}

// ****************************************************************************
//  Method:  QvisExpressionsWindow::displayAllVarsChanged
//
//  Purpose:
//    Slot function, called when "display all variables" is toggled.
//    Just update the window because it will be taken care of.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 10, 2004
//
// ****************************************************************************
void
QvisExpressionsWindow::displayAllVarsChanged()
{
    UpdateWindow(true);
}

// ****************************************************************************
//  Method:  QvisExpressionsWindow::insertFunction
//
//  Purpose:
//    Slot function when a function was selected for insertion by 
//    the popup menu.
//
//  Arguments:
//    id         the menu item id of the function
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 10, 2004
//
// ****************************************************************************
void
QvisExpressionsWindow::insertFunction(int id)
{
    if (!definitionEdit->isEnabled())
        return;

    QString orig = definitionEdit->text();
    definitionEdit->insert(insertFunctionMenu->text(id));
    definitionEdit->insert("()");

    int col,line;
    definitionEdit->getCursorPosition(&line, &col);
    col--;
    if (col < 0)
        col = 0;
    definitionEdit->setCursorPosition(line, col);
}
