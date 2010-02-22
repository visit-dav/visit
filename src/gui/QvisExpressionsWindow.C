/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
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

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QTextEdit>
#include <QMenu>
#include <QPushButton>
#include <QSplitter>

#include <QNarrowLineEdit.h>
#include <QvisVariableButton.h>
#include <QvisPythonFilterEditor.h>

#define STDMIN(A,B) (((A)<(B)) ? (A) : (B))
#define STDMAX(A,B) (((A)<(B)) ? (B) : (A))

// ****************************************************************************
//  Expression Lists
//
//  Purpose:
//    Categorized expression lists data.  These structures are self-contained
//    and to be parsed correctly require only the terminating nulls.
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
//    Hank Childs, Fri Oct  6 15:47:04 PDT 2006
//    Added abel_inversion.
//
//    Mark C. Miller, Wed Nov 15 12:47:05 PST 2006
//    Added zonetype expression
//
//    Hank Childs, Fri Dec 22 10:27:45 PST 2006
//    Added symm_point.
//
//    Hank Childs, Fri Jan 12 13:20:33 PST 2007
//    Added array_compose_with_bins
//
//    Cyrus Harrison, Tue Feb 20 12:02:00 PST 2007
//    Added conn_components
//
//    Cyrus Harrison, Fri Jun  1 14:49:49 PDT 2007
//    Added contraction and viscous_stress
//
//    Jeremy Meredith, Thu Aug 30 16:02:58 EDT 2007
//    Added hsvcolor.
//
//    Gunther H. Weber, Thu Jan 10 12:00:23 PST 2008
//    Added colorlookup.
//
//    Cyrus Harrison, Thu Jan 31 09:45:30 PST 2008
//    Added value_for_material
//
//    Hank Childs, Thu Feb 21 15:50:36 PST 2008
//    Added transpose.
//
//    Brad Whitlock, Tue Apr  8 12:17:11 PDT 2008
//    Added support for internalization of the expression category names.
//
//    Eric Brugger, Wed Aug  6 17:49:59 PDT 2008
//    Renamed smallest_angle to minimum_angle and largest_angle to
//    maximum_angle.
//
//    Hank Childs, Mon Dec 29 11:56:50 PST 2008
//    Added dominant_mat.
//
//    Hank Childs, Sun Feb 22 08:56:33 PST 2009
//    Added category for time iterating expressions.
//
//    Jeremy Meredith, Mon Mar 16 14:40:13 EDT 2009
//    Added timestep and cycle.
//
//    Kathleen Bonnell, Mon Apr 27 17:35:25 PDT 2009
//    Added sinh, cosh, tanh to expr_trig.
//
// ****************************************************************************

struct ExprNameList
{
    QString      name;
    const char **list;
};

const char *expr_meshquality[] = {
    "aspect",
    "aspect_gamma",
    "condition",
    "degree",
    "diagonal_ratio",
    "min_diagonal",
    "max_diagonal",
    "dimension",
    "jacobian",
    "max_edge_length",
    "max_side_volume",
    "maximum_angle",
    "min_edge_length",
    "min_side_volume",
    "minimum_angle",
    "neighbor",
    "node_degree",
    "oddy",
    "relative_size",
    "scaled_jacobian",
    "shape",
    "shape_and_size",
    "shear",
    "skew",
    "stretch",
    "taper",
    "warpage",
    "face_planarity",
    "relative_face_planarity",
    NULL
};

const char *expr_math[] = {
    "abs",
    "ceil",
    "floor",
    "exp",
    "ln",
    "log10",
    "max",
    "min",
    "mod",
    "round",
    "sqr",
    "sqrt",
    "random",
    "+",
    "-",
    "*",
    "/",
    "^",
    "&",
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
    "cosh",
    "deg2rad",
    "rad2deg",
    "sin",
    "sinh",
    "tan",
    "tanh",
    NULL
};

const char *expr_vector[] = {
    "color",
    "color4",
    "colorlookup",
    "cross",
    "dot",
    "hsvcolor",
    "magnitude",
    "normalize",
    NULL
};

const char *expr_tensor[] = {
    "contraction",
    "determinant",
    "effective_tensor",
    "eigenvalue",
    "eigenvector",
    "inverse",
    "principal_deviatoric_tensor",
    "principal_tensor",
    "tensor_maximum_shear",
    "trace",
    "transpose",
    "viscous_stress",
    NULL
};

const char *expr_array[] = {
    "array_compose",
    "array_compose_with_bins",
    "array_decompose",
    NULL
};

const char *expr_materials[] = {
    "dominant_mat",
    "materror",
    "matvf",
    "nmats",
    "specmf",
    "value_for_material",
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
    "zonetype",
    NULL
};

const char *expr_misc[] = {
    "cell_constant",
    "conn_components",
    "curl",
    "cycle",
    "divergence",
    "enumerate",
    "gauss_curvature",
    "gradient",
    "ij_gradient",
    "ijk_gradient",
    "Laplacian",
    "mean_curvature",
    "point_constant",
    "recenter",
    "resrad",
    "surface_normal",
    "   point_surface_normal",
    "   cell_surface_normal",
    "time",
    "timestep",
    NULL
};

const char *expr_imageprocessing[] = {
    "abel_inversion",
    "conservative_smoothing",
    "mean_filter",
    "median_filter",
    NULL
};

const char *expr_comparison[] = {
    "conn_cmfe",
    "pos_cmfe",
    "symm_point",
    "symm_plane",
    "symm_transform",
    NULL
};

const char *expr_time_iteration[] = {
    "average_over_time",
    "cycle_at_minimum",
    "cycle_at_maximum",
    "first_cycle_when_condition_is_true",
    "first_time_when_condition_is_true",
    "first_time_index_when_condition_is_true",
    "last_cycle_when_condition_is_true",
    "last_time_when_condition_is_true",
    "last_time_index_when_condition_is_true",
    "min_over_time", 
    "max_over_time", 
    "sum_over_time", 
    "time_at_minimum",
    "time_at_maximum",
    "time_index_at_minimum",
    "time_index_at_maximum",
    "value_at_minimum",
    "value_at_maximum",
    "var_when_condition_is_first_true",
    "var_when_condition_is_last_true",
    NULL
};

#define NUM_EXPRESSION_CATEGORIES 15
ExprNameList exprlist[NUM_EXPRESSION_CATEGORIES];

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
// Modifications:
//   Brad Whitlock, Tue Apr  8 12:15:08 PDT 2008
//   Support for internationalization.
//
//   Hank Childs, Sun Feb 22 09:01:29 PST 2009
//   Add time iteration category.
//
// ****************************************************************************

QvisExpressionsWindow::QvisExpressionsWindow(
    ExpressionList *exprList_, const QString &caption,
    const QString &shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(exprList_, caption, shortName, notepad,
                               QvisPostableWindowObserver::ApplyButton,
                               false)
{
    // Populate the expression categories. If you add a new one, increment
    // the NUM_EXPRESSION_CATEGORIES macro.
    exprlist[0].name = tr("Math");
    exprlist[0].list = expr_math;
    exprlist[1].name = tr("Vector");
    exprlist[1].list = expr_vector;
    exprlist[2].name = tr("Tensor");
    exprlist[2].list = expr_tensor;
    exprlist[3].name = tr("Array");
    exprlist[3].list = expr_array;
    exprlist[4].name = tr("Material");
    exprlist[4].list = expr_materials;
    exprlist[5].name = tr("Mesh");
    exprlist[5].list = expr_mesh;
    exprlist[6].name = tr("Mesh Quality");
    exprlist[6].list = expr_meshquality;
    exprlist[7].name = tr("Comparison");
    exprlist[7].list = expr_comparison;
    exprlist[8].name = tr("Image Processing");
    exprlist[8].list = expr_imageprocessing;
    exprlist[9].name = tr("Miscellaneous");
    exprlist[9].list = expr_misc;
    exprlist[10].name = tr("Trigonometry");
    exprlist[10].list = expr_trig;
    exprlist[11].name = tr("Relational");
    exprlist[11].list = expr_relational;
    exprlist[12].name = tr("Conditional");
    exprlist[12].list = expr_conditional;
    exprlist[13].name = tr("Logical");
    exprlist[13].list = expr_logical;
    exprlist[14].name = tr("Time Iteration");
    exprlist[14].list = expr_time_iteration;

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
//    Kathleen Bonnell, Thu Aug  3 08:42:33 PDT 2006 
//    Changed numtypes to 7 to support CurveMeshVar.
//
//    Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//    Support for internationalization.
//
//    Mark C. Miller, Mon Apr 14 15:41:21 PDT 2008
//    Added '&' bitwise binary and operation
//
//    Cyrus Harrison, Wed Jun 11 13:49:19 PDT 2008
//    Initial Qt4 Port.
//
//    Jeremy Meredith, Thu Aug  7 15:37:17 EDT 2008
//    changed an erroneous pointer-based string comparison to a true
//    string based comparison.
//
// ****************************************************************************

void
QvisExpressionsWindow::CreateWindowContents()
{

    QSplitter *mainSplitter = new QSplitter(central);
    topLayout->addWidget(mainSplitter);
    topLayout->setStretchFactor(mainSplitter, 100);

    QGroupBox *f1 = new QGroupBox(tr("Expression List"));
    QGridLayout *listLayout = new QGridLayout(f1);

    exprListBox = new QListWidget(f1);
    listLayout->addWidget(exprListBox, 1,0, 1,2);

    newButton = new QPushButton(tr("New"), f1);
    listLayout->addWidget(newButton, 2,0);

    delButton = new QPushButton(tr("Delete"), f1);
    listLayout->addWidget(delButton, 2,1);

    displayAllVars = new QCheckBox(tr("Display expressions from database"), f1);
    listLayout->addWidget(displayAllVars, 3,0, 1,2);

    mainSplitter->addWidget(f1);

    QGroupBox *f2 = new QGroupBox(tr("Definition"));
    QGridLayout *definitionLayout = new QGridLayout(f2);

    int row = 0;

    nameEditLabel = new QLabel(tr("Name"), f2);
    nameEdit = new QNarrowLineEdit(f2);
    definitionLayout->addWidget(nameEditLabel, row,0, 1,1);
    definitionLayout->addWidget(nameEdit, row,1, 1,3);
    row++;

    typeLabel = new QLabel(tr("Type"), f2);
    typeList = new QComboBox(f2);
    // Extracted from Expression::GetTypeString so we can internationalize.
    typeList->addItem(tr("Scalar Mesh Variable"));
    typeList->addItem(tr("Vector Mesh Variable"));
    typeList->addItem(tr("Tensor Mesh Variable"));
    typeList->addItem(tr("Symmetric Tensor Mesh Variable"));
    typeList->addItem(tr("Array Mesh Variable"));
    typeList->addItem(tr("Curve Mesh Variable"));

    definitionLayout->addWidget(typeLabel, row,0, 1,1);
    definitionLayout->addWidget(typeList, row,1, 1,3);
    row++;

    notHidden = new QCheckBox(tr("Show variable in plot menus"), f2);
    definitionLayout->addWidget(notHidden, row,1, 1,3);
    row++;

    editorTabs = new QTabWidget(f2);

    // tab 1 -> standard editor
    CreateStandardEditor();
    editorTabs->addTab(stdEditorWidget, "Standard Editor");
    // tab 2 -> python filter editor
    CreatePythonFilterEditor();
    editorTabs->addTab(pyEditorWidget, "Python Expression Editor");


    definitionLayout->addWidget(editorTabs,row,0,1,4);

    mainSplitter->addWidget(f2);

    // connect signals

    connect(exprListBox, SIGNAL(itemSelectionChanged()),
            this, SLOT(UpdateWindowSingleItem()));
    connect(nameEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(nameTextChanged(const QString&)));

    connect(newButton, SIGNAL(pressed()),
            this, SLOT(addExpression()));
    connect(delButton, SIGNAL(pressed()),
            this, SLOT(delExpression()));
    connect(typeList, SIGNAL(activated(int)),
            this, SLOT(typeChanged(int)));
    connect(displayAllVars, SIGNAL(clicked()),
            this, SLOT(displayAllVarsChanged()));
    connect(notHidden, SIGNAL(clicked()),
            this, SLOT(notHiddenChanged()));

}

// ****************************************************************************
// Method: QvisExpressionsWindow::CreateStandardEditor()
//
// Purpose:
//   Creates widgets used to construct standard visit expressions.
//   (pulled mostly from previous CreateWindowContents() method.)
//
//
// Programmer: Cyrus Harrison
// Creation:   Thu Feb 11 10:33:20 PST 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisExpressionsWindow::CreateStandardEditor()
{
    stdEditorWidget = new QWidget();

    QGridLayout *layout = new QGridLayout(stdEditorWidget);
    int row = 0;

    stdDefinitionEditLabel = new QLabel(tr("Definition"), stdEditorWidget);
    layout->addWidget(stdDefinitionEditLabel, row, 0);
    row++;

    stdDefinitionEdit = new QTextEdit(stdEditorWidget);
    layout->addWidget(stdDefinitionEdit, row,0,1,4);
    row++;


    stdInsertFunctionButton = new QPushButton(tr("Insert Function..."));
    stdInsertFunctionMenu = new QMenu(stdEditorWidget);
    for (int i=0; i < NUM_EXPRESSION_CATEGORIES; i++)
    {
        QMenu *submnu = stdInsertFunctionMenu->addMenu(exprlist[i].name);
        for (int j=0; exprlist[i].list[j]; j++)
        {
            if (std::string(exprlist[i].list[j])=="&")
                submnu->addAction("&&");
            else
                submnu->addAction(exprlist[i].list[j]);
        }
        connect(submnu, SIGNAL(triggered(QAction *)),
                this, SLOT(stdInsertFunction(QAction *)));
    }
    stdInsertFunctionButton->setMenu(stdInsertFunctionMenu);
    layout->addWidget(stdInsertFunctionButton, row, 2);

    // Create a variable button so we can insert variables for the
    // active source.
    stdInsertVariableButton = new QvisVariableButton(false, false, false, -1,stdEditorWidget);
    stdInsertVariableButton->setChangeTextOnVariableChange(false);
    stdInsertVariableButton->setText(tr("Insert Variable..."));
    connect(stdInsertVariableButton, SIGNAL(activated(const QString &)),
            this, SLOT(stdInsertVariable(const QString &)));
    layout->addWidget(stdInsertVariableButton, row, 3);

    connect(stdDefinitionEdit, SIGNAL(textChanged()),
            this, SLOT(stdDefinitionTextChanged()));

}

// ****************************************************************************
// Method: QvisExpressionsWindow::CreatePythonFilterEditor()
//
// Purpose:
//   Creates widgets that compose the python expression filter editor.
//
//
// Programmer: Cyrus Harrison
// Creation:   2010 Thu Feb 11 10:47:51 PST 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisExpressionsWindow::CreatePythonFilterEditor()
{
    pyEditorWidget = new QWidget();

    QGridLayout *layout = new QGridLayout(pyEditorWidget);
    int row = 0;

    pyArgsEditLabel = new QLabel(tr("Arguments"), pyEditorWidget);
    layout->addWidget(pyArgsEditLabel, row, 0);
    row++;

    pyArgsEdit = new QLineEdit(pyEditorWidget);
    layout->addWidget(pyArgsEdit, row,0,1,4);
    row++;


    connect(pyArgsEdit, SIGNAL(textChanged (const QString &)),
            this, SLOT(pyArgsTextChanged()));


    pyInsertFunctionButton = new QPushButton(tr("Insert Function..."));
    pyInsertFunctionMenu = new QMenu(pyEditorWidget);

    for (int i=0; i < NUM_EXPRESSION_CATEGORIES; i++)
    {
        QMenu *submnu = pyInsertFunctionMenu->addMenu(exprlist[i].name);
        for (int j=0; exprlist[i].list[j]; j++)
        {
            if (std::string(exprlist[i].list[j])=="&")
                submnu->addAction("&&");
            else
                submnu->addAction(exprlist[i].list[j]);
        }

        connect(submnu, SIGNAL(triggered(QAction *)),
                this, SLOT(pyInsertFunction(QAction *)));
    }

    // Create a variable button so we can insert variables for the
    // active source.
    pyInsertVariableButton = new QvisVariableButton(false, false, false, -1,pyEditorWidget);
    pyInsertVariableButton->setChangeTextOnVariableChange(false);
    pyInsertVariableButton->setText(tr("Insert Variable..."));

    connect(pyInsertVariableButton, SIGNAL(activated(const QString &)),
            this, SLOT(pyInsertVariable(const QString &)));

    layout->addWidget(pyInsertVariableButton, row, 3);

    pyInsertFunctionButton->setMenu(pyInsertFunctionMenu);
    layout->addWidget(pyInsertFunctionButton, row, 2);
    row++;

    pyFilterEditLabel = new QLabel(tr("Python Expression Script"), pyEditorWidget);

    pyFilterEdit = new QvisPythonFilterEditor();
    layout->addWidget(pyFilterEditLabel, row, 0);
    row++;
    layout->addWidget(pyFilterEdit,row,0,1,4);

    connect(pyFilterEdit, SIGNAL(sourceTextChanged()),
            this, SLOT(pyFilterSourceChanged()));

    connect(pyFilterEdit, SIGNAL(templateSelected(const QString &)),
            this, SLOT(pyTemplateSelected(const QString &)));

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
//    Jeremy Meredith, Tue Feb 19 14:20:57 EST 2008
//    Don't display expressions that were created by operators.
//
//    Cyrus Harrison, Wed Jun 11 13:49:19 PDT 2008
//    Initial Qt4 Port.
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
        if (exprList->GetExpressions(i).GetFromOperator())
            continue;
        if (displayAllVars->isChecked() ||
            !exprList->GetExpressions(i).GetFromDB())
        {
            exprListBox->addItem(exprList->GetExpressions(i).GetName().c_str());
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
//    Cyrus Harrison, Wed Jun 11 13:49:19 PDT 2008
//    Initial Qt4 Port.
//
// ****************************************************************************
void
QvisExpressionsWindow::UpdateWindowSingleItem()
{
    int index = exprListBox->currentRow();
    if (index <  0)
    {
        nameEdit->setText("");
        notHidden->setChecked(true);

        stdDefinitionEdit->setText("");
        pyArgsEdit->setText("");
        pyFilterEdit->setSource("");
    }
    else
    {
        Expression &e = (*exprList)[indexMap[index]];

        nameEdit->setText(e.GetName().c_str());

        int tidx = typeList->findText(Expression::GetTypeString(e.GetType()));
        typeList->setCurrentIndex(tidx);
        notHidden->setChecked(! e.GetHidden());

        QString expr_def = QString(e.GetDefinition().c_str());

        stdDefinitionEdit->setText(expr_def);
        UpdatePythonExpressionEditor(expr_def);

    }

    UpdateWindowSensitivity();
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
//    Cyrus Harrison,  Mon Oct  1 16:24:32 PDT 2007
//    Changed definitionEdit to be readOnly instead of disabled
//    for database expressions so users can select/copy them.
//
//    Cyrus Harrison, Wed Jun 11 13:49:19 PDT 2008
//    Initial Qt4 Port.
//
// ****************************************************************************

void
QvisExpressionsWindow::UpdateWindowSensitivity()
{
    bool enable = true;
    int index = exprListBox->currentRow();

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

    typeList->setEnabled(enable);
    notHidden->setEnabled(enable);

    stdEditorWidget->setEnabled(enable);
    pyEditorWidget->setEnabled(enable && pyExprActive);
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
    stdEditorWidget->blockSignals(block);
    pyEditorWidget->blockSignals(block);
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
        GetViewerMethods()->ProcessExpressions();
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
//    Cyrus Harrison, Wed Jun 11 13:49:19 PDT 2008
//    Initial Qt4 Port.
//
//    Jeremy Meredith, Tue Jun  2 10:40:48 EDT 2009
//    Fix null pointer dereference when there's no current item (e.g. empty
//    window).
//
// ****************************************************************************
void
QvisExpressionsWindow::apply()
{
    // Apply will recreate the window and unselect the current expression.
    // If we have an expression selected, reselect it afterwards.
    bool reselect = (exprListBox->currentRow() != -1);
    QString item;
    if (reselect)
        item = exprListBox->currentItem()->text();

    Apply(true);

    if (reselect)
    {
        for (int i=0; i<exprListBox->count(); i++)
        {
            if (exprListBox->item(i)->text() == item)
            {
                exprListBox->item(i)->setSelected(true);
                exprListBox->setCurrentRow(i);
                UpdateWindowSingleItem();
                break;
            }
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
//    Cyrus Harrison, Wed Jun 11 13:49:19 PDT 2008
//    Initial Qt4 Port.
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
        if ((*exprList)[newName.toStdString().c_str()])
            newid++;
        else
            okay = true;
    }

    Expression e;
    e.SetName(newName.toStdString());
    e.SetDefinition("");
    exprList->AddExpressions(e);

    exprList->Notify();

    for (int i=0; i<exprListBox->count(); i++)
    {
        if (exprListBox->item(i)->text() == newName)
        {
            exprListBox->item(i)->setSelected(true);
            exprListBox->setCurrentRow(i);
            UpdateWindowSingleItem();
            break;
        }
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
//    Cyrus Harrison, Wed Jun 11 13:49:19 PDT 2008
//    Initial Qt4 Port.
//
//    Cyrus Harrison, Mon Jul 21 16:22:30 PDT 2008
//    Fixed a crash when last expression was deleted. 
//
// ****************************************************************************
void
QvisExpressionsWindow::delExpression()
{
    int index = exprListBox->currentRow();

    if (index <  0)
        return;

    exprList->RemoveExpressions(indexMap[index]);
    exprList->Notify();

    // try to select sensible expression:
    // if del expr was last expr: before
    // else: after

    int nrows = exprListBox->count();

    if(nrows > 0)
    {
        if(index == nrows)
            index--;
        exprListBox->item(index)->setSelected(true);
        exprListBox->setCurrentRow(index);
        UpdateWindowSingleItem();
    }

}

// ****************************************************************************
//  Method:  QvisExpressionsWindow::stdDefinitionTextChanged
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
//    Cyrus Harrison, Wed Jun 11 13:49:19 PDT 2008
//    Initial Qt4 Port.
//
//    Cyrus Harrison, Thu Feb 11 12:14:15 PST 2010
//    Renamed from definitionTextChanged.
//
// ****************************************************************************
void
QvisExpressionsWindow::stdDefinitionTextChanged()
{
    int index = exprListBox->currentRow();

    if (index <  0)
        return;

    Expression &e = (*exprList)[indexMap[index]];

    e.SetDefinition(stdDefinitionEdit->toPlainText().toStdString());
}


// ****************************************************************************
//  Method:  QvisExpressionsWindow::pyArgsTextChanged
//
//  Purpose:
//    Slot function when python args text is changed.
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Feb 18 12:45:38 PST 2010
//
//  Modifications:
//
// ****************************************************************************
void
QvisExpressionsWindow::pyArgsTextChanged()
{
    UpdatePythonExpression();
}

// ****************************************************************************
//  Method:  QvisExpressionsWindow::pyFilterSourceChanged
//
//  Purpose:
//    Slot function when python filter script text is changed.
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Feb 18 12:45:38 PST 2010
//
//  Modifications:
//
// ****************************************************************************
void
QvisExpressionsWindow::pyFilterSourceChanged()
{
    UpdatePythonExpression();
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
//    Cyrus Harrison, Wed Jun 11 13:49:19 PDT 2008
//    Initial Qt4 Port.
//
// ****************************************************************************
void
QvisExpressionsWindow::nameTextChanged(const QString &text)
{
    int index = exprListBox->currentRow();

    if (index <  0)
        return;

    Expression &e = (*exprList)[indexMap[index]];

    QString newname = text.trimmed();

    if (newname.isEmpty())
    {
        int newid = 1;
        bool okay = false;
        while (!okay)
        {
            newname.sprintf("unnamed%d", newid);
            if ((*exprList)[newname.toStdString().c_str()])
                newid++;
            else
                okay = true;
        }
    }

    e.SetName(newname.toStdString());
    BlockAllSignals(true);
    exprListBox->item(index)->setText(newname);
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
//    Cyrus Harrison, Wed Jun 11 13:49:19 PDT 2008
//    Initial Qt4 Port.
//
// ****************************************************************************
void
QvisExpressionsWindow::typeChanged(int value)
{
    int index = exprListBox->currentRow();

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
//    Cyrus Harrison, Wed Jun 11 13:49:19 PDT 2008
//    Initial Qt4 Port.
//
// ****************************************************************************
void
QvisExpressionsWindow::notHiddenChanged()
{
    int index = exprListBox->currentRow();

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
//    Cyrus Harrison, Wed Jun 11 13:49:19 PDT 2008
//    Initial Qt4 Port.
//
//    Cyrus Harrison, Tue Jul 14 16:02:15 PDT 2009
//    Fixed crash when no user expressions are defined and the user clicks
//    show database expressions.
//
// ****************************************************************************
void
QvisExpressionsWindow::displayAllVarsChanged()
{

    // This updates the window, which rebuilds the expression list.
    // If we have an expression selected, reselect it afterwards.
    int reselect = (exprListBox->currentRow() != -1);

    QString item ="";

    if (reselect)
        item = exprListBox->currentItem()->text();

    UpdateWindow(true);

    if (reselect)
    {
        QString item = exprListBox->currentItem()->text();
        for (int i=0; i<exprListBox->count(); i++)
        {
            if (exprListBox->item(i)->text() == item)
            {
                exprListBox->item(i)->setSelected(true);
                exprListBox->setCurrentRow(i);
                UpdateWindowSingleItem();
                break;
            }
        }
    }

}

// ****************************************************************************
//  Method:  QvisExpressionsWindow::UpdatePythonExpression
//
//  Purpose:
//    Slot function when python args text is changed.
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Feb 18 12:45:38 PST 2010
//
//  Modifications:
//
// ****************************************************************************
void
QvisExpressionsWindow::UpdatePythonExpression()
{
    int index = exprListBox->currentRow();

    if (index <  0)
        return;

    Expression &e = (*exprList)[indexMap[index]];

    QString expr_def("py(");
    expr_def += pyArgsEdit->text();
    expr_def += QString(",\"");
    expr_def += pyFilterEdit->getSource(true);
    expr_def += QString("\")");

    e.SetDefinition(expr_def.toStdString());
}

// ****************************************************************************
//  Method:  QvisExpressionsWindow::UpdatePythonExpressionEditor
//
//  Purpose:
//    Attempts to extract a python expression from given expression
//    definition. If successful it will return true after setting res_args to
//    the extracted arguments string and res_script to script source string.
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Feb 18 12:26:48 PST 2010
//
//  Modifications:
//
// ****************************************************************************
bool
QvisExpressionsWindow::ParsePythonExpression(const QString &expr_def,
                                             QString &res_args,
                                             QString &res_script)
{
    QString edef = expr_def.trimmed();
    if(edef == QString(""))
    {
        res_args   = QString("");
        res_script = QString("");
        return true;
    }

    int start_idx = -1;

    if(start_idx =edef.indexOf("py(") == 0)
        start_idx = 3;
    else if (start_idx =edef.indexOf("python(") == 0)
        start_idx = 7;

    if(start_idx == -1)
        return false;

    int args_stop_idx = -1;

    // look for instances of <ws>* , <ws>* " [anything] " <ws>* )
    // all instances of " within the python expression should be escaped.

    QRegExp regx("(\\s*\\,\\s*\\\")(.+)(\\\"\\s*\\))");
    int match_pos = 0;
    while((match_pos = regx.indexIn(edef, match_pos)) != -1)
    {
        args_stop_idx = regx.pos(1);
        res_script    = regx.cap(2);
        match_pos    += regx.matchedLength();
    }

    if(args_stop_idx == -1)
        return false;

    res_args = edef.mid(start_idx,args_stop_idx - start_idx).trimmed();
    res_script.trimmed();

    return true;
}

// ****************************************************************************
//  Method:  QvisExpressionsWindow::UpdatePythonExpressionEditor
//
//  Purpose:
//   Updates the python expression editor if the given expression def
//   is valid python expression.
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Feb 18 12:26:48 PST 2010
//
//  Modifications:
//
// ****************************************************************************
void
QvisExpressionsWindow::UpdatePythonExpressionEditor(const QString &expr_def)
{
    QString res_args;
    QString res_script;
    BlockAllSignals(true);
    if(ParsePythonExpression(expr_def,res_args,res_script))
    {
        pyExprActive = true;
        pyArgsEdit->setText(res_args);
        pyFilterEdit->setSource(res_script,true);
        if(res_script != QString(""))
            editorTabs->setCurrentIndex(1);
    }
    else
    {
        pyExprActive = false;
        pyArgsEdit->setText(QString(""));
        pyFilterEdit->setSource(QString(""));
        editorTabs->setCurrentIndex(0);
    }
    BlockAllSignals(false);
}


// ****************************************************************************
//  Method:  QvisExpressionsWindow::ExpandFunction
//
//  Purpose:
//    Slot function when a function was selected for insertion by 
//    the popup menu.
//
//  Arguments:
//    fun_name: Name of the function to expand.
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Feb 18 12:55:12 PST 2010
//
//  Notes: Refactored from insertFunction
//
//  Modifications:
//
// ****************************************************************************

QString
QvisExpressionsWindow::ExpandFunction(const QString &func_name)
{

    QString res;
    bool doParens = (func_name.length() >= 2);

    if (func_name == "&&")
    {
        res = QString("&");
        doParens = false;
    }
    else
    {
        res = func_name;
    }


    if (func_name == "conn_cmfe")
    {
        res += QString("(<filename:var>, <meshname>)");
        doParens = false;
    }
    else if (func_name == "pos_cmfe")
    {
        res += QString("(<filename:var>, <meshname>, <fill-var-for-uncovered-regions>)");
        doParens = false;
    }
    else if (func_name == "symm_point")
    {
        res += QString("(<var>, [Px, Py, Pz])");
        doParens = false;
    }
    else if (func_name == "symm_plane")
    {
        res += QString("(<var>, [Nx, Ny, Nz, Ox, Oy, Oz])");
        doParens = false;
    }
    else if (func_name == "symm_transform")
    {
        res += QString("(<var>, [T00, T01, T02, T10, T11, T12, T20, T21, T22])");
        doParens = false;
    }
    else if (func_name == "matvf")
    {
        res += QString("(<material-object-name>, [#, #, ... #])");
        doParens = false;
    }
    else if (func_name == "specmf")
    {
        res += QString("(<species-variable-name>, <material-name-or-number>, [#, #, ... #])");
        doParens = false;
    }
    else if (func_name == "if")
    {
        res += QString("(<condition>, <then-var>, <else-var>)");
        doParens = false;
    }
    else if (func_name == "min" || func_name == "max" || func_name == "cross" || func_name == "mod" ||
             func_name == "and" || func_name == "or")
    {
        res += QString("(<var1>, <var2>)");
        doParens = false;
    }
    else if (func_name == "array_compose")
    {
        res += QString("(<var1>, <var2>, ..., <varN>)");
        doParens = false;
    }
    else if (func_name == "array_compose_with_bins")
    {
        res += QString("(<var1>, <var2>, ..., <varN>, [b1, b2, ..., bN+1])");
        doParens = false;
    }
    else if (func_name == "eq" || func_name == "ge" || func_name == "gt" || 
             func_name == "le" || func_name == "lt" || func_name == "ne")
    {
        res += QString("(<var-LHS>, <var-RHS>)");
        doParens = false;
    }
    else if(func_name == "color4")
    {
        res += QString("(<var1>, <var2>, <var3>, <var4>)");
        doParens = false;
    }
    else if(func_name == "color")
    {
        res += QString("(<var1>, <var2>, <var3>)");
        doParens = false;
    }
    else if (func_name == "recenter")
    {
        res += QString("(<var>, [\"nodal\", \"zonal\", \"toggle\"])");
        doParens = false;
    }
    else if(func_name == "value_for_material")
    {
        stdDefinitionEdit->insertPlainText("(<var>, <material-name-or-number>)");
        doParens = false;
    }
    else if (func_name == "enumerate")
    {
        res += QString("(<var>, [<val-if-0>, <val-if-1>, ...])");
        doParens = false;
    }
    else if (func_name == "cell_constant" || func_name == "point_constant")
    {
        res += QString("(<meshvar>, <constantvalue>)");
        doParens = false;
    }
    else if(func_name == "average_over_time" || func_name == "min_over_time"
            || func_name == "max_over_time" || func_name == "sum_over_time")
    {
        res += QString("(<var>, [, \"pos_cmfe\", <fillvar-for-uncovered-regions>] [, start-time-index, stop-time-index, stride])");
        doParens = false;
    }
    else if(func_name == "last_cycle_when_condition_is_true" ||
            func_name == "first_cycle_when_condition_is_true" ||
            func_name == "last_time_index_when_condition_is_true" ||
            func_name == "first_time_index_when_condition_is_true" ||
            func_name == "last_time_when_condition_is_true" ||
            func_name == "first_time_when_condition_is_true")
    {
        res += QString("(<condition>, <fillvar-for-when-condition-is-never-true> [, \"pos_cmfe\", <fillvar-for-uncovered-regions>] [, start-time-index, stop-time-index, stride])");
        doParens = false;
    }
    else if(func_name == "var_when_condition_is_first_true" ||
            func_name == "var_when_condition_is_last_true")
    {
        res += QString("(<cond>, <var-for-output>, <fillvar-for-when-cond-is-never-true> [, \"pos_cmfe\", <fillvar-for-uncovered-regions>] [, start-time-index, stop-time-index, stride])");
        doParens = false;
    }
    else if(func_name == "cycle_at_minimum" ||
            func_name == "time_at_minimum" ||
            func_name == "time_index_at_minimum")
    {
        res += QString("(<var-to-find-minimum-of> [, \"pos_cmfe\", <fillvar-for-uncovered-regions>] [, start-time-index, stop-time-index, stride])");
        doParens = false;
    }
    else if(func_name == "cycle_at_maximum" ||
            func_name == "time_at_maximum" ||
            func_name == "time_index_at_maximum")
    {
        res += QString("(<var-to-find-maximum-of> [, \"pos_cmfe\", <fillvar-for-uncovered-regions>] [, start-time-index, stop-time-index, stride])");
        doParens = false;
    }
    else if (func_name == "value_at_minimum")
    {
        res += QString("(<var-to-find-minimum-of>, <var-for-output> [, \"pos_cmfe\", <fillvar-for-uncovered-regions>] [, start-time-index, stop-time-index, stride])");
        doParens = false;
    }
    else if (func_name == "value_at_maximum")
    {
        res += QString("(<var-to-find-maximum-of>, <var-for-output> [, \"pos_cmfe\", <fillvar-for-uncovered-regions>] [, start-time-index, stop-time-index, stride])");
        doParens = false;
    }

    if (doParens)
    {
        res += QString("()");
    }

    return res;
}

// ****************************************************************************
//  Method:  QvisExpressionsWindow::stdInsertFunction
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
//    Hank Childs, Fri Dec 22 10:31:40 PST 2006
//    Added symm_point.
//
//    Hank Childs, Fri Jan 12 13:20:33 PST 2007
//    Added array_compose and array_compose_with_bins.
//
//    Hank Childs, Sat Mar 17 15:33:54 PDT 2007
//    Fix problem with specmf.
//
//    Brad Whitlock, Mon Apr 23 17:35:13 PST 2007
//    Added color expression.
//
//    Cyrus Harrison, Thu Jan 31 09:45:30 PST 2008
//    Added value_for_material
//
//    Jeremy Meredith, Wed Feb 13 12:27:50 EST 2008
//    Added enumerate
//
//    Jeremy Meredith, Wed Feb 20 10:01:59 EST 2008
//    Added cell_constant and point_constant.
//
//    Mark C. Miller, Mon Apr 14 15:41:21 PDT 2008
//    Added support for '&' bitwise binary and operation
//
//    Cyrus Harrison, Wed Jun 11 13:49:19 PDT 2008
//    Initial Qt4 Port.
//
//    Sean Ahern, Wed Sep 10 12:44:22 EDT 2008
//    Added support for recenter with optional argument.
//
//    Hank Childs, Wed Oct  8 16:31:05 PDT 2008
//    Make pos_cmfe fillin be more comprehensible.
//
//    Hank Childs, Sun Feb 22 12:42:21 PST 2009
//    Add fillins for time iterating expressions.
//
//    Cyrus Harrison, Thu Feb 18 12:55:42 PST 2010
//    Refactored from insertFunction. Expansion logic was
//    moved to the 'ExpandFunction' method.
//
// ****************************************************************************

void
QvisExpressionsWindow::stdInsertFunction(QAction * action)
{
   if (!stdDefinitionEdit->isEnabled())
        return;
    QString func_name = action->text();

    QString func_res =ExpandFunction(func_name);
    stdDefinitionEdit->insertPlainText(func_res);
    if(func_res.lastIndexOf("()") == (func_res.size() -2))
        stdDefinitionEdit->moveCursor(QTextCursor::Left,QTextCursor::MoveAnchor);
    stdDefinitionEdit->setFocus();
}

// ****************************************************************************
//  Method:  QvisExpressionsWindow::pyInsertFunction
//
//  Purpose:
//    Slot function called when a 'function' was insertion by the popup menu.
//
//  Arguments:
//    action Triggered menu action.
//
//  Programmer:  Cyrus Harrison
//  Creation:    Thu Feb 11 15:21:11 PST 2010
//
//  Modifications:
//
// ****************************************************************************

void
QvisExpressionsWindow::pyInsertFunction(QAction * action)
{
   if (!pyArgsEdit->isEnabled())
        return;

    QString func_name = action->text();

    QString func_res =ExpandFunction(func_name);
    pyArgsEdit->insert(func_res);
    pyArgsEdit->setFocus();
}

// ****************************************************************************
// Method: QvisExpressionWindow::QuoteVariableName
//
// Purpose:
//    Helper to 'quote' variable names used as expression arguments.
//
// Arguments:
//   var : The variable.
//
// Programmer: Cyrus
// Creation:   Thu Feb 11 15:22:08 PST 2010
//
// Modifications:
//
// ****************************************************************************

QString
QvisExpressionsWindow::QuoteVariable(const QString &var)
{
    // If the variable name contains special characters
    // then put the <> around the variable name.
    bool needs_quoting = false;

    for (int i=0; i<var.length(); i++)
    {
        char c = var[i].toAscii();
        if ((c < 'A' || c > 'Z') &&
            (c < 'a' || c > 'z') &&
            (c < '0' || c > '9') &&
            c != '_')
        {
            needs_quoting = true;
            break;
        }
    }

    QString res;
    if (needs_quoting)
        res = QString("<") + var + QString(">");
    else
        res = var;

    return res;
}

// ****************************************************************************
// Method: QvisExpressionWindow::stdInsertVariable
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
//    Cyrus Harrison, Wed Jun 11 13:49:19 PDT 2008
//    Initial Qt4 Port.
//
// ****************************************************************************

void
QvisExpressionsWindow::stdInsertVariable(const QString &var)
{
    if (!stdDefinitionEdit->isEnabled())
        return;

    stdDefinitionEdit->insertPlainText(QuoteVariable(var));
    stdDefinitionEdit->setFocus();
}

// ****************************************************************************
// Method: QvisExpressionWindow::pyInsertVariable
//
// Purpose:
//   This is a Qt slot function that inserts a variable name into the
//   current variable definition.
//
// Arguments:
//   var : The variable.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Feb 11 15:22:08 PST 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisExpressionsWindow::pyInsertVariable(const QString &var)
{
    if (!pyArgsEdit->isEnabled())
        return;

    pyArgsEdit->insert(QuoteVariable(var));
    pyArgsEdit->setFocus();
}

// ****************************************************************************
// Method: QvisExpressionWindow::pyTemplateSelected
//
// Purpose:
//   This is a Qt slot function that loads a filter template.
//
// Arguments:
//   tname : Template type.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Feb 11 15:22:08 PST 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisExpressionsWindow::pyTemplateSelected(const QString &tname)
{
    QString fname("");
    QString tdir = QvisPythonFilterEditor::templateDirectory();

    if(tname == QString("Advanced"))
        fname = tdir + QString("advanced_expression.py");
    else if(tname == QString("Simple"))
        fname = tdir + QString("simple_expression.py");

    pyFilterEdit->loadScript(fname);
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
