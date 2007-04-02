/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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

#include <QNarrowLineEdit.h>
#include <QvisVariableButton.h>

#define STDMIN(A,B) (((A)<(B)) ? (A) : (B))
#define STDMAX(A,B) (((A)<(B)) ? (B) : (A))

// ****************************************************************************
//  Expression Lists
//
//  Purpose:
//    Categorized expression lists data.  These structures are self-contained
//    and require the terminating nulls to be parsed correctly.  Keep in mind
//    that 
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 23, 2004
//
//  Modifications:
//
//    Hank Childs, Sun Jan  2 15:32:00 PST 2005
//    Added curl, divergence, Laplacian, materror.
//
//    Hank Childs, Thu Jan 20 15:51:16 PST 2005
//    Added side volume, resrad.
//
//    Hank Childs, Thu Jun 30 15:00:42 PDT 2005
//    Added polar_radius, polar_theta, polar_phi, cylindrical_radius,
//    cylindrical_theta, cylindrical, mod, round, ceil, floor, zoneid,
//    nodeid, global_zoneid, and global_nodeid.
//
//    Hank Childs, Tue Aug 16 09:05:03 PDT 2005
//    Added image processing category.
//
//    Hank Childs, Wed Sep 21 17:31:59 PDT 2005
//    Added external_node, surface normal, min/max edge length/side volume.
//
//    Hank Childs, Sun Jan 22 12:47:29 PST 2006
//    Renamed comparison to relational.  Added new category for comparison.
//
//    Hank Childs, Tue Feb 14 14:06:20 PST 2006
//    Added ijk_gradient.
//
//    Hank Childs, Sun Mar  5 16:03:38 PST 2006
//    Added time.
//
//    Hank Childs, Mon Mar 13 16:48:23 PST 2006
//    Added minimum and maximum.
//
//    Hank Childs, Thu May 11 12:45:39 PDT 2006
//    Added mean_curvature, gauss_curvature, compose vector, and compose 
//    tensor.
//
// ****************************************************************************
struct ExprNameList
{
    const char *name;
    const char **list;
};

const char *expr_meshquality[] = {
    "aspect",
    "aspect_gamma",
    "condition",
    "degree",
    "diagonal",
    "dimension",
    "jacobian",
    "largest_angle",
    "max_edge_length",
    "max_side_volume",
    "min_edge_length",
    "min_side_volume",
    "neighbor",
    "node_degree",
    "oddy",
    "relative_size",
    "scaled_jacobian",
    "shape",
    "shape_and_size",
    "shear",
    "skew",
    "smallest_angle",
    "stretch",
    "taper",
    "warpage",
    NULL
};

const char *expr_math[] = {
    "abs",
    "ceil",
    "floor",
    "ln",
    "log10",
    "max",
    "min",
    "mod",
    "round",
    "sqr",
    "sqrt",
    "+",
    "-",
    "*",
    "/",
    "^",
    NULL
};

const char *expr_relational[] = {
    "eq",
    "ge",
    "gt",
    "le",
    "lt",
    "ne",
    NULL
};

const char *expr_conditional[] = {
    "if",
    NULL
};

const char *expr_logical[] = {
    "and",
    "not",
    "or",
    NULL
};

const char *expr_trig[] = {
    "acos",
    "asin",
    "atan",
    "cos",
    "deg2rad",
    "rad2deg",
    "sin",
    "tan",
    NULL
};

const char *expr_vector[] = {
    "cross",
    "dot",
    "magnitude",
    NULL
};

const char *expr_tensor[] = {
    "determinant",
    "effective_tensor",
    "eigenvalue",
    "eigenvector",
    "inverse",
    "principal_deviatoric_tensor",
    "principal_tensor",
    "tensor_maximum_shear",
    "trace",
    NULL
};

const char *expr_array[] = {
    "array_compose",
    "array_decompose",
    NULL
};

const char *expr_materials[] = {
    "materror",
    "matvf",
    "nmats",
    "specmf",
    NULL
};

const char *expr_mesh[] = {
    "area",
    "coord",
    "cylindrical",
    "cylindrical_radius",
    "cylindrical_theta",
    "external_node",
    "global_nodeid",
    "global_zoneid",
    "nodeid",
    "polar",
    "polar_radius",
    "polar_theta",
    "polar_phi",
    "revolved_surface_area",
    "revolved_volume",
    "volume",
    "zoneid",
    NULL
};

const char *expr_misc[] = {
    "curl",
    "divergence",
    "gauss_curvature",
    "gradient",
    "ij_gradient",
    "ijk_gradient",
    "Laplacian",
    "mean_curvature",
    "recenter",
    "resrad",
    "surface_normal",
    "   point_surface_normal",
    "   cell_surface_normal",
    "time",
    NULL
};

const char *expr_imageprocessing[] = {
    "conservative_smoothing",
    "mean_filter",
    "median_filter",
    NULL
};

const char *expr_comparison[] = {
    "conn_cmfe",
    "pos_cmfe",
    "symm_plane",
    "symm_transform",
    NULL
};

ExprNameList exprlist[] =
{
    {"Math",             expr_math},
    {"Vector",           expr_vector},
    {"Tensor",           expr_tensor},
    {"Array",            expr_array},
    {"Material",         expr_materials},
    {"Mesh",             expr_mesh},
    {"Mesh Quality",     expr_meshquality},
    {"Comparison",       expr_comparison},
    {"Image Processing", expr_imageprocessing},
    {"Miscellaneous",    expr_misc},
    {"Trigonometry",     expr_trig},
    {"Relational",       expr_relational},
    {"Conditional",      expr_conditional},
    {"Logical",          expr_logical},
    {NULL,NULL}
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
//    Jeremy Meredith, Sat Oct 23 11:50:35 PDT 2004
//    Changed the layout a bit: use group boxes with titles for the two
//    panes, change to use splitters, change the resize proportions,
//    move the "hidden" toggle to the bottom and change its sense,
//    move "insert function" to the right side and group expressions into
//    categorized submenus, reword some other labels to make them more
//    descriptive.
//
//    Brad Whitlock, Thu Dec 9 10:50:12 PDT 2004
//    I added a variable button that lets us insert variables.
//
//    Hank Childs, Thu Jul 21 11:12:43 PDT 2005
//    Add support for array mesh variables.
//
// ****************************************************************************

void
QvisExpressionsWindow::CreateWindowContents()
{
#define USE_SPLITTER
#ifndef USE_SPLITTER
    QHBoxLayout *mainLayout = new QHBoxLayout(topLayout, -1, "mainLayout");
#else
    QSplitter *mainSplitter = new QSplitter(central);
    topLayout->addWidget(mainSplitter);
    topLayout->setStretchFactor(mainSplitter, 100);
#endif

#ifndef USE_SPLITTER
    QGridLayout *listLayout = new QGridLayout(mainLayout, 3,2, 5, "listLayout");
    mainLayout->setStretchFactor(listLayout, 50);
#define f1 central
#else
    QGroupBox *f1 = new QGroupBox("Expression List",mainSplitter);
    QGridLayout *listLayout = new QGridLayout(f1, 4,2, 7, 5, "listLayout");
#endif

    listLayout->addRowSpacing(0, 10);

    exprListBox = new QListBox(f1);
    listLayout->addMultiCellWidget(exprListBox, 1,1, 0,1);

    newButton = new QPushButton("New", f1, "newButton");
    listLayout->addWidget(newButton, 2,0);

    delButton = new QPushButton("Delete", f1, "delButton");
    listLayout->addWidget(delButton, 2,1);

    displayAllVars = new QCheckBox("Display expressions from database", f1, "displayAllVars");
    listLayout->addMultiCellWidget(displayAllVars, 3,3, 0,1);

#ifndef USE_SPLITTER
    mainLayout->addSpacing(10);
    QGridLayout *editLayout = new QGridLayout(mainLayout, 6,2, 5, "editLayout");
    mainLayout->setStretchFactor(editLayout, 50);
#define f2 central
#else
    QGroupBox *f2 = new QGroupBox("Definition", mainSplitter);
    QGridLayout *editLayout = new QGridLayout(f2, 6,4, 7, 5, "editLayout");
#endif
    int row = 0;

    editLayout->addRowSpacing(row, 10);
    row++;

    nameEditLabel = new QLabel("Name", f2, "nameEditLabel");
    nameEdit = new QNarrowLineEdit(f2, "nameEdit");
    editLayout->addMultiCellWidget(nameEditLabel, row,row, 0,0);
    editLayout->addMultiCellWidget(nameEdit, row,row, 1,3);
    row++;

    typeLabel = new QLabel("Type", f2, "typeLabel");
    typeList = new QComboBox(f2, "typeList");
    int numtypes = Expression::GetNumTypes();
    numtypes = 6;  // HACK!!!  Variable types after 6 currently fail.  FIXME!!!
    int i;
    for (i=1; i < numtypes ; i++)
        typeList->insertItem(Expression::GetTypeString((Expression::ExprType)i));
    editLayout->addMultiCellWidget(typeLabel, row,row, 0,0);
    editLayout->addMultiCellWidget(typeList, row,row, 1,3);
    row++;

    definitionEditLabel = new QLabel("Definition", f2, "definitionEditLabel");
    editLayout->addWidget(definitionEditLabel, row, 0);
    row++;

    definitionEdit = new QMultiLineEdit(f2, "definitionEdit");
    definitionEdit->setWordWrap(QMultiLineEdit::WidgetWidth);
    definitionEdit->setWrapPolicy(QMultiLineEdit::AtWhiteSpace);
    editLayout->addMultiCellWidget(definitionEdit, row,row, 0,3);
    row++;

    notHidden = new QCheckBox("Show variable in plot menus", f2, "notHidden");
    editLayout->addWidget(notHidden, row, 0);

    insertFunctionButton = new QPushButton("Insert Function...", f2);
    insertFunctionMenu = new QPopupMenu(f2, "insertFunctionMenu");
    for (i=0; exprlist[i].name; i++)
    {
        QPopupMenu *tmpMenu = new QPopupMenu(f2, exprlist[i].name);
        for (int j=0; exprlist[i].list[j]; j++)
        {
            tmpMenu->insertItem(exprlist[i].list[j]);
        }
        insertFunctionMenu->insertItem(exprlist[i].name, tmpMenu);
        connect(tmpMenu, SIGNAL(activated(int)),
                this, SLOT(insertFunction(int)));
    }
    insertFunctionButton->setPopup(insertFunctionMenu);
    editLayout->addWidget(insertFunctionButton, row, 2);

    // Create a variable button so we can insert variables for the
    // active source.
    insertVariableButton = new QvisVariableButton(false, false, false, -1, f2,
        "insertVariableButton");
    insertVariableButton->setChangeTextOnVariableChange(false);
    insertVariableButton->setText("Insert Variable...");
    connect(insertVariableButton, SIGNAL(activated(const QString &)),
            this, SLOT(insertVariable(const QString &)));
    editLayout->addWidget(insertVariableButton, row, 3);
    row++;

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
    connect(notHidden, SIGNAL(clicked()),
            this, SLOT(notHiddenChanged()));
    connect(displayAllVars, SIGNAL(clicked()),
            this, SLOT(displayAllVarsChanged()));
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
//    Jeremy Meredith, Mon Oct 25 12:52:45 PDT 2004
//    Always access the expression list by index, just in case there
//    are two expressions with the same name.  Keep in mind we need
//    a map from the list-position index to expressionlist index.
//
// ****************************************************************************
void
QvisExpressionsWindow::UpdateWindow(bool)
{
    BlockAllSignals(true);

    exprListBox->clear();
    indexMap.clear();
    int pos = 0;
    for (int i=0; i<exprList->GetNumExpressions(); i++)
    {
        if (displayAllVars->isChecked() ||
            !exprList->GetExpression(i).GetFromDB())
        {
            exprListBox->insertItem(exprList->GetExpression(i).GetName().c_str());
            indexMap[pos++] = i;
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
//  Modifications:
//    Jeremy Meredith, Mon Oct 25 11:34:35 PDT 2004
//    Reversed the sense of the "hidden" button.
//    Always access the expression list by index, just in case there
//    are two expressions with the same name.
//
// ****************************************************************************
void
QvisExpressionsWindow::UpdateWindowSingleItem()
{
    BlockAllSignals(true);
    int index = exprListBox->currentItem();

    if (index <  0)
    {
        nameEdit->setText("");
        notHidden->setChecked(true);
        definitionEdit->setText("");
    }
    else
    {
        Expression &e = (*exprList)[indexMap[index]];

        nameEdit->setText(e.GetName().c_str());
        notHidden->setChecked(! e.GetHidden());
        typeList->setCurrentText(Expression::GetTypeString(e.GetType()));
        definitionEdit->setText(e.GetDefinition().c_str());
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
//  Modifications:
//    Jeremy Meredith, Mon Oct 25 11:34:35 PDT 2004
//    Reversed the sense of the "hidden" button.
//    Always access the expression list by index, just in case there
//    are two expressions with the same name.
//
//    Brad Whitlock, Thu Dec 9 14:04:22 PST 2004
//    Added a button to insert variable names.
//
// ****************************************************************************

void
QvisExpressionsWindow::UpdateWindowSensitivity()
{
    bool enable = true;
    int index = exprListBox->currentItem();

    if (index <  0)
    {
        enable = false;
    }
    else if ((*exprList)[indexMap[index]].GetFromDB())
    {
        enable = false;
    }

    nameEdit->setEnabled(enable);
    delButton->setEnabled(enable);
    notHidden->setEnabled(enable);
    typeList->setEnabled(enable);
    definitionEdit->setEnabled(enable);
    insertFunctionButton->setEnabled(enable);
    insertVariableButton->setEnabled(enable);
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
//    Jeremy Meredith, Mon Oct 25 12:39:53 PDT 2004
//    Always access the expression list by index, just in case there
//    are two expressions with the same name.
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
//    Jeremy Meredith, Mon Oct 17 10:39:56 PDT 2005
//    Made the delete action access the expression list by index in the
//    expression list (instead of assuming its index in the list box was
//    correct).  This was an oversight from my 10/25/04 change.  ('5682)
//   
// ****************************************************************************
void
QvisExpressionsWindow::delExpression()
{
    int index = exprListBox->currentItem();

    if (index <  0)
        return;

    exprList->RemoveExpression(indexMap[index]);

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
//  Modifications:
//    Jeremy Meredith, Mon Oct 25 12:40:01 PDT 2004
//    Always access the expression list by index, just in case there
//    are two expressions with the same name.
//
// ****************************************************************************
void
QvisExpressionsWindow::definitionTextChanged()
{
    int index = exprListBox->currentItem();

    if (index <  0)
        return;

    Expression &e = (*exprList)[indexMap[index]];

    e.SetDefinition(definitionEdit->text().latin1());
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
//  Modifications:
//    Jeremy Meredith, Mon Oct 25 12:40:31 PDT 2004
//    Always access the expression list by index, just in case there
//    are two expressions with the same name.
//
//    Jeremy Meredith, Mon Oct 17 10:42:08 PDT 2005
//    Never allow an empty name to get into the expression list.  This
//    could cause crashes.  ('6295)
//
// ****************************************************************************
void
QvisExpressionsWindow::nameTextChanged(const QString &text)
{
    int index = exprListBox->currentItem();

    if (index <  0)
        return;

    Expression &e = (*exprList)[indexMap[index]];

    QString newname = text.stripWhiteSpace();

    if (newname.isEmpty())
    {
        int newid = 1;
        bool okay = false;
        while (!okay)
        {
            newname.sprintf("unnamed%d", newid);
            if ((*exprList)[newname.latin1()])
                newid++;
            else
                okay = true;
        }
    }

    e.SetName(newname.latin1());
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
//  Modifications:
//    Jeremy Meredith, Mon Oct 25 12:40:37 PDT 2004
//    Always access the expression list by index, just in case there
//    are two expressions with the same name.
//
// ****************************************************************************
void
QvisExpressionsWindow::typeChanged(int value)
{
    int index = exprListBox->currentItem();

    if (index <  0)
        return;

    Expression &e = (*exprList)[indexMap[index]];

    // Add one because we skipped index 0 (unknown) when creating the
    // list box originally
    e.SetType(Expression::ExprType(value+1));
}

// ****************************************************************************
//  Method:  QvisExpressionsWindow::notHiddenChanged
//
//  Purpose:
//    Slot function for the notHidden toggle.
//    Sets the "notHidden" flag of an expression.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 10, 2004
//
//  Modifications:
//    Jeremy Meredith, Mon Oct 25 11:34:35 PDT 2004
//    Reversed the sense of the "hidden" button.
//    Always access the expression list by index, just in case there
//    are two expressions with the same name.
//
// ****************************************************************************
void
QvisExpressionsWindow::notHiddenChanged()
{
    int index = exprListBox->currentItem();

    if (index <  0)
        return;

    Expression &e = (*exprList)[indexMap[index]];

    e.SetHidden(!notHidden->isChecked());
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
//  Modifications:
//    Jeremy Meredith, Mon Oct 25 13:33:35 PDT 2004
//    Made it reselect an expression that was already selected, like Apply.
//
// ****************************************************************************
void
QvisExpressionsWindow::displayAllVarsChanged()
{
    // This updates the window, which rebuilds the expression list.
    // If we have an expression selected, reselect it afterwards.
    int reselect = (exprListBox->currentItem() != -1);
    QString item = exprListBox->currentText();

    UpdateWindow(true);

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
//  Modifications:
//    Jeremy Meredith, Mon Oct 25 11:33:43 PDT 2004
//    Make sure it's not just a single-char operator before adding the
//    function call parentheses.
//
//    Brad Whitlock, Thu Dec 9 14:15:01 PST 2004
//    Added code to set the focus to the definition edit.
//
//    Hank Childs, Sun Jan 22 12:49:57 PST 2006
//    Add special insertion code for complicated expressions.
//
//    Hank Childs, Fri Mar 10 14:28:11 PST 2006
//    Change ordering of arguments for pos_cmfe, since the first pass was
//    incorrect.
//
//    Hank Childs, Mon Mar 13 16:48:23 PST 2006
//    Added min and max.
//
// ****************************************************************************

void
QvisExpressionsWindow::insertFunction(int id)
{
    if (!definitionEdit->isEnabled())
        return;

    definitionEdit->insert(insertFunctionMenu->text(id));

    bool doParens = (insertFunctionMenu->text(id).length() >= 2);
    std::string str = insertFunctionMenu->text(id).latin1();
    if (str == "conn_cmfe")
    {
        definitionEdit->insert("(<filename:var>, <meshname>)");
        doParens = false;
    }
    else if (str == "pos_cmfe")
    {
        definitionEdit->insert("(<filename:var>, <meshname>, <default-var>)");
        doParens = false;
    }
    else if (str == "symm_plane")
    {
        definitionEdit->insert("(<var>, [Nx, Ny, Nz, Ox, Oy, Oz])");
        doParens = false;
    }
    else if (str == "symm_transform")
    {
        definitionEdit->insert("(<var>, [T00, T01, T02, T10, T11, T12, "
                                        "T20, T21, T22])");
        doParens = false;
    }
    else if (str == "matvf")
    {
        definitionEdit->insert("(<material-name>, [#, #, ... #])");
        doParens = false;
    }
    else if (str == "specmf")
    {
        definitionEdit->insert("(<material-name>, [#, #, ... #])");
        doParens = false;
    }
    else if (str == "if")
    {
        definitionEdit->insert("(<condition>, <then-var>, <else-var>)");
        doParens = false;
    }
    else if (str == "min" || str == "max")
    {
        definitionEdit->insert("(<var1>, <var2>)");
        doParens = false;
    }

    if (doParens)
    {
        definitionEdit->insert("()");

        int col,line;
        definitionEdit->getCursorPosition(&line, &col);
        col--;
        if (col < 0)
            col = 0;
        definitionEdit->setCursorPosition(line, col);
    }

    definitionEdit->setFocus();
}

// ****************************************************************************
// Method: QvisExpressionWindow::insertVariable
//
// Purpose: 
//   This is a Qt slot function that inserts a variable name into the 
//   current variable definition.
//
// Arguments:
//   var : The variable.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 10:52:37 PDT 2004
//
// Modifications:
//   
//    Jeremy Meredith, Fri Sep  2 16:26:42 PDT 2005
//    Made it be more aggressive about quoting.
//
// ****************************************************************************

void
QvisExpressionsWindow::insertVariable(const QString &var)
{
    if (!definitionEdit->isEnabled())
        return;

    // If the variable name contains special characters
    // then put the <> around the variable name.
    bool needs_quoting = false;

    for (int i=0; i<var.length(); i++)
    {
        char c = var[i];
        if ((c < 'A' || c > 'Z') &&
            (c < 'a' || c > 'z') &&
            (c < '0' || c > '9') &&
            c != '_')
        {
            needs_quoting = true;
            break;
        }
    }

    QString newVar;
    if (needs_quoting)
        newVar = QString("<") + var + QString(">");
    else
        newVar = var;

    definitionEdit->insert(newVar);
    definitionEdit->setFocus();
}

// ****************************************************************************
// Method: QvisExpressionsWindow::newExpression
//
// Purpose: 
//   This is a Qt slot function that can be called on the window to add a
//   new expression and set the focus of the window to the name line edit
//   and select all of its text.
//
// Note:       This method is used by the "Create new expression ..." option
//             in QvisVariableButtons.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 10:18:13 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisExpressionsWindow::newExpression()
{
    // Add a new expression
    addExpression();

    // Make the name line edit be the active widget and select its text.
    nameEdit->setFocus();
    nameEdit->setCursorPosition(0);
    nameEdit->selectAll();
}
