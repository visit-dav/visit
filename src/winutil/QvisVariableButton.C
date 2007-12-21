/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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

#include <QvisVariableButton.h>

#include <QvisVariableButtonHelper.h>
#include <QvisVariablePopupMenu.h>
#include <VariableMenuPopulator.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <PlotPluginInfo.h>

#define DELETE_MENU_TO_FREE_POPUPS

//
// String representation of the types of variables and they are in
// the order that we want them to appear in the menu; not their
// real order.
//
#define N_VAR_CATEGORIES 11
static const char *categoryMenuNames[] = {
    "Scalars",
    "Vectors",
    "Meshes",
    "Materials",
    "Subsets",
    "Species",
    "Curves",
    "Tensors",
    "Symmetric Tensors",
    "Labels",
    "Arrays"
};
//
// Masks of the types of variables in the order that we want them
// to appear in the menu; not their real order.
//
static int categoryMasks[] = {
    VAR_CATEGORY_SCALAR,
    VAR_CATEGORY_VECTOR,
    VAR_CATEGORY_MESH,
    VAR_CATEGORY_MATERIAL,
    VAR_CATEGORY_SUBSET,
    VAR_CATEGORY_SPECIES,
    VAR_CATEGORY_CURVE,
    VAR_CATEGORY_TENSOR,
    VAR_CATEGORY_SYMMETRIC_TENSOR,
    VAR_CATEGORY_LABEL,
    VAR_CATEGORY_ARRAY
};

// ****************************************************************************
// Method: QvisVariableButton::VariablePopupInfo::VariablePopupInfo
//
// Purpose: 
//   Constructor for the VariablePopupInfo class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 16:41:32 PST 2004
//
// Modifications:
//   
// ****************************************************************************

QvisVariableButton::VariablePopupInfo::VariablePopupInfo()
{
    varMenus = 0;
    helper = 0;
}

// ****************************************************************************
// Method: QvisVariableButton::VariablePopupInfo::~VariablePopupInfo
//
// Purpose: 
//   Destructor for the VariablePopupInfo class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 16:41:32 PST 2004
//
// Modifications:
//   
// ****************************************************************************

QvisVariableButton::VariablePopupInfo::~VariablePopupInfo()
{
    if(varMenus != 0)
    {
        for(int cat = 0; cat < N_VAR_CATEGORIES; ++cat)
            delete varMenus[cat];
    
        delete [] varMenus;

        if(helper != 0 && helper->parent() == 0)
            delete helper;
    }
}

// ****************************************************************************
// Method: QvisVariableButton::VariablePopupInfo::Initialize
//
// Purpose: 
//   Initializes the object and creates the shared menus.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 16:42:11 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisVariableButton::VariablePopupInfo::Initialize()
{
    if(varMenus == 0)
    {
        helper = new QvisVariableButtonHelper(0, "helper");

        varMenus = new QvisVariablePopupMenu*[N_VAR_CATEGORIES];        
        for(int i = 0; i < N_VAR_CATEGORIES; ++i)
            CreateMenu(i);
    }
}

// ****************************************************************************
// Method: QvisVariableButton::VariablePopupInfo::CreateMenu
//
// Purpose: 
//   Create the i'th menu.
//
// Arguments:
//   i : The index of the menu to create.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 20 12:39:47 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisVariableButton::VariablePopupInfo::CreateMenu(int i)
{
    varMenus[i] = new QvisVariablePopupMenu(0, 0, categoryMenuNames[i]);
    QObject::connect(varMenus[i], SIGNAL(activated(int, const QString &)),
                     helper, SLOT(activated(int, const QString &)));
}

// ****************************************************************************
// Method: QvisVariableButton::VariablePopupInfo::DeleteMenu
//
// Purpose: 
//   Destroy the i'th menu.
//
// Arguments:
//   i : The index of the menu to destroy.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 20 12:40:11 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisVariableButton::VariablePopupInfo::DeleteMenu(int i)
{
    if(varMenus[i] != 0)
    { 
        QObject::disconnect(varMenus[i], SIGNAL(activated(int, const QString &)),
                            helper, SLOT(activated(int, const QString &)));
        delete varMenus[i];
        varMenus[i] = 0;
    }
}

// ****************************************************************************
// Method: QvisVariableButton::VariablePopupInfo::UpdateMenus
//
// Purpose: 
//   Updates the menus so they contain the variables prescribed by the
//   specified variable populator.
//
// Arguments:
//   pop : The variable menu populator to use.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 16:43:03 PST 2004
//
// Modifications:
//   Brad Whitlock, Thu Dec 20 12:38:49 PST 2007
//   I made it delete and recreate the menus when updating them so we really
//   free up the menus that get created.
//
// ****************************************************************************

void
QvisVariableButton::VariablePopupInfo::UpdateMenus(VariableMenuPopulator *pop)
{
    Initialize();

    // Insert the real list of variables.
    for(int i = 0; i < N_VAR_CATEGORIES; ++i)
    {
#ifdef DELETE_MENU_TO_FREE_POPUPS
        DeleteMenu(i);
        CreateMenu(i);
#else
        varMenus[i]->clear();
#endif
        // Update the menu with only 1 type of variable.
        pop->UpdateSingleVariableMenu(varMenus[i], categoryMasks[i],
            helper, SLOT(activated(int, const QString &)));
    }
}

// ****************************************************************************
// Method: QvisVariableButton::VariablePopupInfo::connect
//
// Purpose: 
//   Makes a particular button be associated with the variable menus.
//
// Arguments:
//   b : The button to associate with the menus.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 16:44:00 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisVariableButton::VariablePopupInfo::connect(QvisVariableButton *b)
{
    Initialize();
    helper->setButton(b);
}

// ****************************************************************************
// Method: QvisVariableButton::VariablePopupInfo::disconnect
//
// Purpose: 
//   Dissociates a variable button from the menus.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 16:45:10 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisVariableButton::VariablePopupInfo::disconnect()
{
    Initialize();
    helper->setButton(0);
}


//
// Static
//
QObjectList QvisVariableButton::instances;
QvisVariableButton::VariablePopupInfo QvisVariableButton::activeSourceInfo;
QvisVariableButton::VariablePopupInfo QvisVariableButton::plotSourceInfo;
QObject *QvisVariableButton::expressionCreator = 0;
const char *QvisVariableButton::expressionSlot = 0;

//
// The VAR_CATEGORY_... macros come from PlotPluginInfo.h
//
const int QvisVariableButton::Scalars          = VAR_CATEGORY_SCALAR;
const int QvisVariableButton::Vectors          = VAR_CATEGORY_VECTOR;
const int QvisVariableButton::Meshes           = VAR_CATEGORY_MESH;
const int QvisVariableButton::Materials        = VAR_CATEGORY_MATERIAL;
const int QvisVariableButton::Subsets          = VAR_CATEGORY_SUBSET;
const int QvisVariableButton::Species          = VAR_CATEGORY_SPECIES;
const int QvisVariableButton::Curves           = VAR_CATEGORY_CURVE;
const int QvisVariableButton::Tensors          = VAR_CATEGORY_TENSOR;
const int QvisVariableButton::SymmetricTensors = VAR_CATEGORY_SYMMETRIC_TENSOR;
const int QvisVariableButton::Labels           = VAR_CATEGORY_LABEL;
const int QvisVariableButton::Arrays           = VAR_CATEGORY_ARRAY;

// ****************************************************************************
// Method: QvisVariableButton::QvisVariableButton
//
// Purpose: 
//   Constructor for the QvisVariableButton class.
//
// Arguments:
//   addDefault_ : Tells the object to add a menu option for the "default" var.
//   addExpr_    : Tells the object to add a menu option for creating expressions.
//   usePlot     : Tells the object whether it should use the menus for the 
//                 plot source or active source.
//   mask        : The types of variables that should appear in the menu.
//   parent      : The widget's parent.
//   name        : The widget's name.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 16:45:44 PST 2004
//
// Modifications:
//   
// ****************************************************************************

QvisVariableButton::QvisVariableButton(QWidget *parent, const char *name) :
    QPushButton(parent, name), defaultVariable("default")
{
    instances.append(this);

    addDefault = true;
    addExpr = true;
    usePlotSource = true;
    changeTextOnVarChange = true;
    varTypes = -1;
    setText(defaultVariable);

    //
    // Create this button's menu and add the menus for the
    // variable types in which this variable button is interested.
    //
    menu = new QvisVariablePopupMenu(0, this, "varMenu");
    connect(menu, SIGNAL(activated(int, const QString &)),
            this, SLOT(changeVariable(int, const QString &)));
    connect(menu, SIGNAL(aboutToShow()),
            this, SLOT(connectMenu()));
    connect(menu, SIGNAL(aboutToHide()),
            this, SLOT(deferredDisconnectMenu()));
    // Insert some standard menu options.
    UpdateMenu();
    setPopup(menu);
}

// ****************************************************************************
// Method: QvisVariableButton::QvisVariableButton
//
// Purpose: 
//   Constructor for the QvisVariableButton class.
//
// Arguments:
//   addDefault_ : Tells the object to add a menu option for the "default" var.
//   addExpr_    : Tells the object to add a menu option for creating expressions.
//   usePlot     : Tells the object whether it should use the menus for the 
//                 plot source or active source.
//   mask        : The types of variables that should appear in the menu.
//   parent      : The widget's parent.
//   name        : The widget's name.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 16:45:44 PST 2004
//
// Modifications:
//   Brad Whitlock, Thu Nov 16 16:56:03 PST 2006
//   Added variable.
//
// ****************************************************************************

QvisVariableButton::QvisVariableButton(bool addDefault_, bool addExpr_,
    bool usePlot, int mask, QWidget *parent, const char *name) :
    QPushButton(parent, name),
    variable("default"), defaultVariable("default")
{
    instances.append(this);

    addDefault = addDefault_;
    addExpr = addExpr_;
    usePlotSource = usePlot;
    changeTextOnVarChange = true;
    varTypes = mask;
    setText(defaultVariable);

    //
    // Create this button's menu and add the menus for the
    // variable types in which this variable button is interested.
    //
    menu = new QvisVariablePopupMenu(0, this, "varMenu");
    connect(menu, SIGNAL(activated(int, const QString &)),
            this, SLOT(changeVariable(int, const QString &)));
    connect(menu, SIGNAL(aboutToShow()),
            this, SLOT(connectMenu()));
    connect(menu, SIGNAL(aboutToHide()),
            this, SLOT(deferredDisconnectMenu()));
    // Insert some standard menu options.
    UpdateMenu();
    setPopup(menu);
}

// ****************************************************************************
// Method: QvisVariableButton::~QvisVariableButton
//
// Purpose: 
//   Destructor for the QvisVariableButton class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 16:47:56 PST 2004
//
// Modifications:
//   
// ****************************************************************************

QvisVariableButton::~QvisVariableButton()
{
    instances.remove(this);
}

// ****************************************************************************
// Method: QvisVariableButton::UpdateMenu
//
// Purpose: 
//   Updates the button's menu.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 16:48:18 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisVariableButton::UpdateMenu()
{
//qDebug("update variable button %p", this);
    menu->clear();

    // Insert some extra menu options.
    if(addDefault)
        menu->insertItem(defaultVariable);
    if(addExpr)
        menu->insertItem("Create new expression ...");

    if(addDefault || addExpr)
    {
        // Count the number of menus that there will be.
        int menuCount = 0;
        for(int i = 0; i < N_VAR_CATEGORIES; ++i)
        {
            if(varTypes & categoryMasks[i])
                ++menuCount;
        }

        // If we're going to insert menus then add a separator.
        if(menuCount > 0)
            menu->insertSeparator();
    }

    QvisVariablePopupMenu **vm = usePlotSource ? 
        plotSourceInfo.varMenus :
        activeSourceInfo.varMenus;
    for(int i = 0; i < N_VAR_CATEGORIES; ++i)
    {
        if(varTypes & categoryMasks[i])
        {
            if(vm != 0 && vm[i]->count() > 0)
            {
//                qDebug("\tinserting menu for %s", categoryMenuNames[i]);
                menu->insertItem(categoryMenuNames[i],
                                 vm[i]);
            }
            else
            {
//                qDebug("\tinserting disabled option for %s",
//                       categoryMenuNames[i]);
                // The menu has no items in it. Insert a regular
                // menu entry and disable it.
                int id = menu->insertItem(categoryMenuNames[i]);
                menu->setItemEnabled(id, false);
            }
        }
    }
}

// ****************************************************************************
// Method: QvisVariableButton::getVariable
//
// Purpose: 
//   Gets the variable.
//
// Returns:    The variable.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 16 16:54:59 PST 2006
//
// Modifications:
//   
// ****************************************************************************

const QString &
QvisVariableButton::getVariable() const
{
    return variable;
}

// ****************************************************************************
// Method: QvisVariableButton::setVariable
//
// Purpose: 
//   Sets the variable for the menu.
//
// Arguments:
//   var : the new variable.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 16 16:55:21 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisVariableButton::setVariable(const QString &var)
{
    changeVariable(0, var);
}

// ****************************************************************************
// Method: QvisVariableButton::setChangeTextOnVariableChange
//
// Purpose: 
//   Sets whether we change the button's text when changing the variable.
//
// Arguments:
//   val : Whether to change the button text.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 10 14:31:33 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisVariableButton::setChangeTextOnVariableChange(bool val)
{
    changeTextOnVarChange = val;
}

// ****************************************************************************
// Method: QvisVariableButton::setText
//
// Purpose: 
//   Sets tbe button's text so that long variable names are truncated and
//   the button is given a tooltip.
//
// Arguments:
//   var : The text to use for the button.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 16:49:39 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisVariableButton::setText(const QString &var)
{
    QString displayVar(var);

    if(changeTextOnVarChange)
    {
        bool tip = false;

        // If the variable contains a slash then display everything to the
        // right of the slash.
        int index = var.findRev("/");
        if(index != -1)
        {
            displayVar = var.right(var.length() - index - 1);
            tip = true;
        }

        // Make the variable short enough to probably fit in the button.
        if(fontMetrics().boundingRect(displayVar).width() > width())
        {
            int reduceBy = 3;
            QString shortVar(displayVar);
            do
            {
                shortVar = displayVar.left(displayVar.length()-reduceBy)+"...";
                ++reduceBy;
            } while(shortVar.length() > 1 &&
                    fontMetrics().boundingRect(shortVar).width() > width());
            displayVar = shortVar;
            tip = true;
        }

        // Put a tool tip on the pushbutton so the user can see
        // the whole variable name.
        if(tip)
            QToolTip::add(this, var);
        else
            QToolTip::remove(this);
    }

    // Set the text to display into the button.
    QPushButton::setText(displayVar);
}

// ****************************************************************************
// Method: QvisVariableButton::setDefaultVariable
//
// Purpose: 
//   Sets the default variable to be displayed in the menu.
//
// Arguments:
//   var : The default variable.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 16:50:40 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisVariableButton::setDefaultVariable(const QString &var)
{
    bool showingDefaultVar = (var == text());
    defaultVariable = var;
    if(showingDefaultVar && changeTextOnVarChange)
        setText(defaultVariable);
    UpdateMenu();
}

// ****************************************************************************
// Method: QvisVariableButton::UpdateActiveSourceButtons
//
// Purpose: 
//   Static method that updates all buttons that use the active source's menu.
//
// Arguments:
//   pop : The active source's menu populator.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 16:51:10 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisVariableButton::UpdateActiveSourceButtons(VariableMenuPopulator *pop)
{
    // Create the menus if they have not been created yet.
    activeSourceInfo.Initialize();

    // Update the menus.
    if(pop)
        activeSourceInfo.UpdateMenus(pop);
 
    //
    // Iterate over the variable buttons and create any missing populators.
    //
    QObjectListIt it(instances);
    QObject *obj;
    while((obj = it.current()) != 0)
    {
        ++it;
        QvisVariableButton *button = (QvisVariableButton *)obj;
        if(!button->usePlotSource)
            button->UpdateMenu();
    }
}

// ****************************************************************************
// Method: QvisVariableButton::UpdatePlotSourceButtons
//
// Purpose: 
//   Static method that updates all buttons that use the plot source's menu.
//
// Arguments:
//   pop : The plot source's menu populator.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 16:51:10 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisVariableButton::UpdatePlotSourceButtons(VariableMenuPopulator *pop)
{
    // Create the menus if they have not been created yet.
    plotSourceInfo.Initialize();

    // Update the menus.
    if(pop)
        plotSourceInfo.UpdateMenus(pop);
 
    //
    // Iterate over the variable buttons and create any missing populators.
    //
    QObjectListIt it(instances);
    QObject *obj;
    while((obj = it.current()) != 0)
    {
        ++it;
        QvisVariableButton *button = (QvisVariableButton *)obj;
        if(button->usePlotSource)
            button->UpdateMenu();
    }
}

// ****************************************************************************
// Method: QvisVariableButton::ConnectExpressionCreation
//
// Purpose: 
//   Sets the object and slot function that will be called when the user
//   clicks on the option to create an expression.
//
// Arguments:
//   receiver : The object that will handle the event.
//   slot     : The slot function that will be called.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 16:52:26 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisVariableButton::ConnectExpressionCreation(QObject *receiver,
    const char *slot)
{
    expressionCreator = receiver;
    expressionSlot = slot;
}

//
// Qt slot functions.
//

// ****************************************************************************
// Method: QvisVariableButton::changeVariable
//
// Purpose: 
//   This is a Qt slot function that is called by the helper object when
//   the user selects a variable from the list.
//
// Arguments:
//   var : The variable that the user selected.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 16:53:50 PST 2004
//
// Modifications:
//   Brad Whitlock, Thu Nov 16 16:53:34 PST 2006
//   Keep track of the variable that the user selected so it can be queried.
//
// ****************************************************************************

void
QvisVariableButton::changeVariable(int, const QString &var)
{
    // We chose a menu option so it's safe to disconnect the menu.
    disconnectMenu();

    if(var == "Create new expression ...")
    {
        // Call the expression creator object's slot.
        if(expressionCreator != 0 && expressionSlot)
            QTimer::singleShot(0, expressionCreator, expressionSlot);
    }
    else
    {
        // Set the text on the button.
        if(changeTextOnVarChange)
            setText(var);

        variable = var;

        // Tell other Qt objects that we chose a variable.
        emit activated(var);
    }
}

// ****************************************************************************
// Method: QvisVariableButton::connectMenu
//
// Purpose: 
//   This is a Qt slot function that connects the button to the shared
//   menus.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 16:54:44 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisVariableButton::connectMenu()
{
    //
    // Connect all of the relevant variable menus so we don't
    // ever accidentally select variables for a different variable button.
    //
    if(usePlotSource)
        plotSourceInfo.connect(this);
    else
        activeSourceInfo.connect(this);
}

// ****************************************************************************
// Method: QvisVariableButton::deferredDisconnectMenu
//
// Purpose: 
//   This method disconnects the button from the menu in a deferred manner
//   to the slots are called in the right order.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 16:55:20 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisVariableButton::deferredDisconnectMenu()
{
    QTimer::singleShot(100, this, SLOT(disconnectMenu()));
}

// ****************************************************************************
// Method: QvisVariableButton::disconnectMenu
//
// Purpose: 
//   This is a Qt slot function that disconnects the button from the shared
//   menus.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 16:56:04 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisVariableButton::disconnectMenu()
{
    if(usePlotSource)
        plotSourceInfo.disconnect();
    else
        activeSourceInfo.disconnect();
}
