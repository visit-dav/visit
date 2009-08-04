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

#include <QvisVariableButton.h>

#ifndef DESIGNER_PLUGIN
#include <QvisVariableButtonHelper.h>
#include <QvisVariablePopupMenu.h>
#include <VariableMenuPopulator.h>
#else
#include <QMenu>
#endif

#include <PlotPluginInfo.h>
#include <QTimer>

#define DELETE_MENU_TO_FREE_POPUPS

#define N_VAR_CATEGORIES 11

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

#ifndef DESIGNER_PLUGIN
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
//   Brad Whitlock, Wed Apr  9 12:10:04 PDT 2008
//   Support for internationalization.
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
//   Brad Whitlock, Thu May 22 11:36:40 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisVariableButton::VariablePopupInfo::Initialize()
{
    if(varMenus == 0)
    {
        helper = new QvisVariableButtonHelper(0);

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
//   Brad Whitlock, Fri May  9 12:14:35 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisVariableButton::VariablePopupInfo::CreateMenu(int i)
{
    varMenus[i] = new QvisVariablePopupMenu(0, 0);
    varMenus[i]->setTitle(categoryMenuNames->operator[](i));
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
QList<QObject*> QvisVariableButton::instances;
QvisVariableButton::VariablePopupInfo *QvisVariableButton::activeSourceInfo = 0;
QvisVariableButton::VariablePopupInfo *QvisVariableButton::plotSourceInfo = 0;
QObject *QvisVariableButton::expressionCreator = 0;
const char *QvisVariableButton::expressionSlot = 0;
#endif
QStringList *QvisVariableButton::categoryMenuNames = 0;

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
//   Brad Whitlock, Fri May  9 12:05:04 PDT 2008
//   Qt 4. Moved some initialization code to here. Added support for a
//   stripped down version that can stand in for the real thing in designer.
//
// ****************************************************************************

QvisVariableButton::QvisVariableButton(QWidget *parent) :
    QPushButton(parent), defaultVariable("default")
{
    InitializeCategoryNames();

    addDefault = true;
    addExpr = true;
    usePlotSource = true;
    changeTextOnVarChange = true;
    varTypes = -1;
    setText(defaultVariable);

#ifndef DESIGNER_PLUGIN
    if(activeSourceInfo == 0)
        activeSourceInfo = new VariablePopupInfo;
    if(plotSourceInfo == 0)
        plotSourceInfo = new VariablePopupInfo;
    instances.append(this);

    //
    // Create this button's menu and add the menus for the
    // variable types in which this variable button is interested.
    //
    menu = new QvisVariablePopupMenu(0, this);
    connect(menu, SIGNAL(activated(int, const QString &)),
            this, SLOT(changeVariable(int, const QString &)));
    connect(menu, SIGNAL(aboutToShow()),
            this, SLOT(connectMenu()));
#else
    menu = new QMenu(this);
#endif
    // Insert some standard menu options.
    UpdateMenu();
    setMenu(menu);
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
//   Brad Whitlock, Fri May  9 12:05:42 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisVariableButton::QvisVariableButton(bool addDefault_, bool addExpr_,
    bool usePlot, int mask, QWidget *parent) :
    QPushButton(parent), variable("default"), defaultVariable("default")
{
    InitializeCategoryNames();

    addDefault = addDefault_;
    addExpr = addExpr_;
    usePlotSource = usePlot;
    changeTextOnVarChange = true;
    varTypes = mask;
    setText(defaultVariable);

#ifndef DESIGNER_PLUGIN
    if(activeSourceInfo == 0)
        activeSourceInfo = new VariablePopupInfo;
    if(plotSourceInfo == 0)
        plotSourceInfo = new VariablePopupInfo;
    instances.append(this);

    //
    // Create this button's menu and add the menus for the
    // variable types in which this variable button is interested.
    //
    menu = new QvisVariablePopupMenu(0, this);
    connect(menu, SIGNAL(activated(int, const QString &)),
            this, SLOT(changeVariable(int, const QString &)));
    connect(menu, SIGNAL(aboutToShow()),
            this, SLOT(connectMenu()));
#else
    menu = new QMenu(this);
#endif 
    // Insert some standard menu options.
    UpdateMenu();
    setMenu(menu);
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
//   Brad Whitlock, Fri May  9 12:15:47 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisVariableButton::~QvisVariableButton()
{
#ifndef DESIGNER_PLUGIN
    instances.removeAll(this);

    if(instances.size() == 0)
    {
        delete activeSourceInfo;
        activeSourceInfo = 0;

        delete plotSourceInfo;
        plotSourceInfo = 0;

        delete categoryMenuNames;
        categoryMenuNames = 0;
    }
#endif
}

// ****************************************************************************
// Method: QvisVariableButton::InitializeCategoryNames
//
// Purpose: 
//   Initializes the category menu names.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 16 15:07:54 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisVariableButton::InitializeCategoryNames()
{
    if(categoryMenuNames == 0)
        categoryMenuNames = new QStringList;
    categoryMenuNames->clear();
    (*categoryMenuNames) << tr("Scalars")
                      << tr("Vectors")
                      << tr("Meshes")
                      << tr("Materials")
                      << tr("Subsets")
                      << tr("Species")
                      << tr("Curves")
                      << tr("Tensors")
                      << tr("Symmetric Tensors")
                      << tr("Labels")
                      << tr("Arrays");
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
//   Brad Whitlock, Wed Apr  9 12:11:08 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Fri May  9 12:18:55 PDT 2008
//   Qt 4. Designer support.
//
// ****************************************************************************

void
QvisVariableButton::UpdateMenu()
{ 
    menu->clear();

    // Insert some extra menu options.
    if(addDefault)
        menu->addAction(defaultVariable);
    if(addExpr)
        menu->addAction(tr("Create new expression ..."));

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
            menu->addSeparator();
    }

    for(int i = 0; i < N_VAR_CATEGORIES; ++i)
    {
        if(varTypes & categoryMasks[i])
        {
#ifndef DESIGNER_PLUGIN
            QvisVariablePopupMenu **vm = usePlotSource ? 
                plotSourceInfo->varMenus :
                activeSourceInfo->varMenus;
            if(vm != 0 && vm[i]->count() > 0)
            {
                menu->addMenu(vm[i]);
            }
            else
            {
#endif
                // The menu has no items in it. Insert a regular
                // menu entry and disable it.
                QAction *a = menu->addAction(categoryMenuNames->operator[](i));
                a->setEnabled(false);
#ifndef DESIGNER_PLUGIN
            }
#endif
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
//   Brad Whitlock, Fri May  9 12:09:15 PDT 2008
//   Qt 4.
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
        int index = var.lastIndexOf("/");
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
            setToolTip(var);
        else
            setToolTip("");
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
// Method: QvisVariableButton::setVarTypes
//
// Purpose: 
//   Sets the varTypes.
//
// Arguments:
//   t : The var types.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 16:50:40 PST 2004
//
// Modifications:
//   Kathleen Bonnell, Tue Jun 24 11:38:02 PDT 2008
//   Moved from header file, added call to UpdateMenu.
//   
// ****************************************************************************

void
QvisVariableButton::setVarTypes(int t)
{
    if (varTypes != t)
    {
        varTypes = t;
        UpdateMenu();
    }
}

bool
QvisVariableButton::getAddExpr() const
{
    return addExpr;
}

void
QvisVariableButton::setAddExpr(bool val)
{
    if(val != addExpr)
    {
        addExpr = val;
        UpdateMenu();
    }
}

bool
QvisVariableButton::getAddDefault() const
{
    return addDefault;
}

void
QvisVariableButton::setAddDefault(bool val)
{
    if(val != addDefault)
    {
        addDefault = val;
        UpdateMenu();
    }
}

#ifndef DESIGNER_PLUGIN
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
//   Brad Whitlock, Fri May  9 12:07:54 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisVariableButton::UpdateActiveSourceButtons(VariableMenuPopulator *pop)
{
    // Create the menus if they have not been created yet.
    activeSourceInfo->Initialize();

    // Update the menus.
    if(pop)
        activeSourceInfo->UpdateMenus(pop);
 
    //
    // Iterate over the variable buttons and create any missing populators.
    //
    for(QList<QObject*>::const_iterator it = instances.constBegin();
        it != instances.end(); ++it)
    {
        QvisVariableButton *button = (QvisVariableButton *)*it;
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
//   Qt 4.
//
// ****************************************************************************

void
QvisVariableButton::UpdatePlotSourceButtons(VariableMenuPopulator *pop)
{
    // Create the menus if they have not been created yet.
    plotSourceInfo->Initialize();

    // Update the menus.
    if(pop)
        plotSourceInfo->UpdateMenus(pop);
 
    //
    // Iterate over the variable buttons and create any missing populators.
    //
    for(QList<QObject*>::const_iterator it = instances.constBegin();
        it != instances.end(); ++it)
    {
        QvisVariableButton *button = (QvisVariableButton *)*it;
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
#endif

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
//   Brad Whitlock, Wed Apr  9 12:11:33 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Fri Aug  8 15:19:54 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisVariableButton::changeVariable(int i, const QString &var)
{
    // We chose a menu option so it's safe to schedule the disconnect of the menu.
    deferredDisconnectMenu();

    if(var == tr("Create new expression ..."))
    {
#ifndef DESIGNER_PLUGIN
        // Call the expression creator object's slot.
        if(expressionCreator != 0 && expressionSlot)
            QTimer::singleShot(0, expressionCreator, expressionSlot);
#endif
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
//   Brad Whitlock, Thu Oct 16 15:00:53 PDT 2008
//   Designer support.
//
// ****************************************************************************

void
QvisVariableButton::connectMenu()
{
#ifndef DESIGNER_PLUGIN
    //
    // Connect all of the relevant variable menus so we don't
    // ever accidentally select variables for a different variable button.
    //
    if(usePlotSource)
        plotSourceInfo->connect(this);
    else
        activeSourceInfo->connect(this);
#endif
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
//   Brad Whitlock, Thu Oct 16 15:00:44 PDT 2008
//   Designer support.
//
// ****************************************************************************

void
QvisVariableButton::disconnectMenu()
{
#ifndef DESIGNER_PLUGIN
    if(usePlotSource)
        plotSourceInfo->disconnect();
    else
        activeSourceInfo->disconnect();
#endif
}
