#include <VariableMenuPopulator.h>

#include <avtDatabaseMetaData.h>
#include <avtSIL.h>
#include <PlotPluginInfo.h>
#include <Expression.h>
#include <ExpressionList.h>
#include <QvisVariablePopupMenu.h>
#include <qobject.h>

using std::string;
using std::map;

// ****************************************************************************
// Method: VariableMenuPopulator::VariableMenuPopulator
//
// Purpose: 
//   Constructor for the VariableMenuPopulator class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 17 14:58:14 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Sep 2 09:56:54 PDT 2003
//   I added materialVars.
//
//   Brad Whitlock, Tue Feb 24 15:50:54 PST 2004
//   I added cachedDBName and cachedExpressionList.
//
// ****************************************************************************

VariableMenuPopulator::VariableMenuPopulator() :
    cachedDBName(), cachedExpressionList(),
    meshVars(), scalarVars(), materialVars(), vectorVars(), subsetVars(),
    speciesVars(), curveVars(), tensorVars(), symmTensorVars()
{
}

// ****************************************************************************
// Method: VariableMenuPopulator::VariableMenuPopulator
//
// Purpose: 
//   Destructor for the VariableMenuPopulator class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 17 14:58:14 PST 2003
//
// Modifications:
//   
// ****************************************************************************

VariableMenuPopulator::~VariableMenuPopulator()
{
}

// ****************************************************************************
// Method: VariableMenuPopulator::ClearDatabaseName
//
// Purpose: 
//   Clears out the database name that is used to determine whether the
//   list of variables needs to be regenerated.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 26 08:43:58 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
VariableMenuPopulator::ClearDatabaseName()
{
    cachedDBName = "";
}

// ****************************************************************************
// Method: VariableMenuPopulator::PopulateVariableLists
//
// Purpose: 
//   Populates internal variable lists using the metadata, sil, and expression
//   list.
//
// Arguments:
//   md       : A pointer to metadata for the file we're interested in.
//   sil      : A pointer to SIL for the file we're interested in.
//   exprList : A pointer to the list of expression definitions.
//
// Returns:    
//
// Note:  I adapted this code from code that I originally wrote for 
//        QvisPlotManagerWidget.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 17 14:19:06 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue May 20 12:49:43 PDT 2003
//   Made it work with an updated version of Expression::ExprType..
//
//   Hank Childs, Fri Aug  1 10:44:45 PDT 2003
//   Add support for curves.
//
//   Kathleen Bonnell, Fri Aug 22 18:03:55 PDT 2003
//   Create 'compound' subset vars (e.g. 'materials(mesh1)') only when
//   necessary.
//
//   Brad Whitlock, Tue Sep 2 09:55:50 PDT 2003
//   I added the materialVars map, which only contains materials, so the
//   VAR_CATEGORY_MATERIALS flag works and does not populate the variable
//   list with all of the subset variables.
//
//   Hank Childs, Tue Sep 23 22:05:54 PDT 2003
//   Add support for tensors.
//
//   Brad Whitlock, Thu Oct 23 16:27:38 PST 2003
//   I made the code take into account whether material and subset variables
//   are valid variables using information from the metadata. I also changed
//   how expressions are added to the list so that expressions from the
//   expression list and expressions from metadata are added separately.
//
//   Brad Whitlock, Tue Feb 24 15:49:58 PST 2004
//   I added code to check the name of the database that we're using against
//   the name of the database that we've used before.
//
// ****************************************************************************

bool
VariableMenuPopulator::PopulateVariableLists(const std::string &dbName,
    const avtDatabaseMetaData *md, const avtSIL *sil,
    const ExpressionList *exprList)
{
    if(md == 0 || sil == 0 || exprList == 0)
        return false;

    //
    // If the database name is the same and the expression list is the
    // same then return false, indicating that no updates are required.
    //
    bool expressionsSame = *exprList == cachedExpressionList;
    bool variableMetaData = md->GetMustRepopulateOnStateChange();
    if(dbName == cachedDBName && expressionsSame && !variableMetaData)
        return false;

    //
    // Save the database name and the expression list so we can check them
    // again later so we can get out of this routine early if they are the
    // same.
    //
    cachedDBName = dbName;
    if(!expressionsSame)
        cachedExpressionList = *exprList;

    // Clear out the variable maps.
    meshVars.clear();
    scalarVars.clear();
    vectorVars.clear();
    materialVars.clear();
    subsetVars.clear();
    speciesVars.clear();
    curveVars.clear();
    tensorVars.clear();
    symmTensorVars.clear();

    // Do stuff with the metadata
    int i;
    for (i = 0; i < md->GetNumMeshes(); ++i)
    {
        const avtMeshMetaData *mmd = md->GetMesh(i);
        meshVars[mmd->name] = mmd->validVariable;
    }
    for (i = 0; i < md->GetNumScalars(); ++i)
    {
        const avtScalarMetaData *smd = md->GetScalar(i);
        scalarVars[smd->name] = smd->validVariable;
    }
    for (i = 0; i < md->GetNumVectors(); ++i)
    {
        const avtVectorMetaData *vmd = md->GetVector(i);
        vectorVars[vmd->name] = vmd->validVariable;
    }
    for (i = 0; i < md->GetNumSpecies(); ++i)
    {
        const avtSpeciesMetaData *smd = md->GetSpecies(i);
        speciesVars[smd->name] = smd->validVariable;
    }
    for (i = 0; i < md->GetNumCurves(); ++i)
    {
        const avtCurveMetaData *cmd = md->GetCurve(i);
        curveVars[cmd->name] = cmd->validVariable;
    }
    for (i = 0; i < md->GetNumTensors(); ++i)
    {
        const avtTensorMetaData *tmd = md->GetTensor(i);
        tensorVars[tmd->name] = tmd->validVariable;
    }
    for (i = 0; i < md->GetNumSymmTensors(); ++i)
    {
        const avtSymmetricTensorMetaData *tmd = md->GetSymmTensor(i);
        symmTensorVars[tmd->name] = tmd->validVariable;
    }

    // Do stuff with the sil
    const intVector &topSets = sil->GetWholes();

    // There are currently 9 role types, lets count their occurrance
    // in the top sets.
    int roleCount[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0};

    for(i = 0; i < topSets.size(); ++i)
    {
        int tsIndex = topSets[i];
        const intVector &maps = sil->GetSILSet(tsIndex)->GetMapsOut();
        for (int j = 0; j < maps.size(); ++j)
        {
            int idx = maps[j];
            int role = (int)sil->GetSILCollection(idx)->GetRole();
            if (role > 0 && role < 8)
                roleCount[role]++;
        }
    }

    for(i = 0; i < topSets.size(); ++i)
    {
        int tsIndex = topSets[i];
        const intVector &maps = sil->GetSILSet(tsIndex)->GetMapsOut();
        string setName("(" + sil->GetSILSet(tsIndex)->GetName() + ")");
        for(int j = 0; j < maps.size(); ++j)
        {
            int     idx = maps[j];

            //
            // DISABLE SPECIES FOR SUBSET PLOTS.  THIS IS A HACK AND SHOULD
            // BE REMOVED WHEN EVERYTHING WORKS DOWNSTREAM.
            //
            if (sil->GetSILCollection(idx)->GetRole() == SIL_SPECIES)
            {
                continue;
            }

            string  varName = sil->GetSILCollection(idx)->GetCategory();
            int     role = sil->GetSILCollection(idx)->GetRole();
            bool    validVariable = true;

            //
            // If we're looking at a material, test to see if the material
            // is a valid variable
            //
            if(role == SIL_MATERIAL)
            {
                for(int m = 0; m < md->GetNumMaterials(); ++m)
                {
                    const avtMaterialMetaData *mmd = md->GetMaterial(m);
                    if(mmd->name == varName)
                    {
                        validVariable = mmd->validVariable;
                        break;
                    }
                }
            }

            //
            // Only add the set name if necessary.
            //
            if (roleCount[role] > 1)
                varName += setName;
            subsetVars[varName] = validVariable;

            //
            // Also add the varName to the material variable map so we can
            // have plots, etc that just use materials instead of any type
            // of subset variable.
            //
            if (role == SIL_MATERIAL)
                materialVars[varName] = validVariable;
        }
    }

    //
    // Add the expressions from the expression list.
    //
    int nexp = exprList->GetNumExpressions();
    for(i = 0; i < nexp; ++i)
    {
        const Expression &expr = exprList->operator[](i);
        if(!expr.GetHidden())
            AddExpression(expr);
    }

    return true;
}

// ****************************************************************************
// Method: VariableMenuPopulator::AddExpression
//
// Purpose: 
//   This method adds an expression to the appropriate variable list.
//
// Arguments:
//   expr : The expression to add to a varialbe list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 24 15:39:47 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
VariableMenuPopulator::AddExpression(const Expression &expr)
{
    // Figure out which list this expression should be added to.
    StringBoolMap *m = 0;
    switch (expr.GetType())
    {
    case Expression::ScalarMeshVar:
        m = &scalarVars;
        break;
    case Expression::VectorMeshVar:
        m = &vectorVars;
        break;
    case Expression::Mesh:
        m = &meshVars;
        break;
    case Expression::Material:
        m = &materialVars;
        break;
    case Expression::Species:
        m = &speciesVars;
        break;
    case Expression::TensorMeshVar:
        m = &tensorVars;
        break;
    case Expression::SymmetricTensorMeshVar:
        m = &symmTensorVars;
        break;
    default:
        break;
    }

    // Check if the name is already in the list
    if(m)
    {
        if(m->find(expr.GetName()) == m->end())
            m->insert(StringBoolMap::value_type(expr.GetName(), true));
        else
        {
            string name(expr.GetName() + " (expression)");
            m->insert(StringBoolMap::value_type(name, true));
        }
    }
}

// ****************************************************************************
// Method: VariableMenuPopulator::UpdateSingleVariableMenu
//
// Purpose: 
//   Updates the a variable menu with a list of variables that is a combination
//   of the variables in the variable lists.
//
// Arguments:
//   menu      : A pointer to the variable menu that will be updated.
//   varTypes  : A flag that contains various variable types orred together.
//   changeVar : A flag that tells the functions called by this routine that
//               it is being called for the change variable menu.
//
// Returns:    The number of variables in the list that was added to the menu.
//
// Note:       I moved this code from QvisPlotManagerWidget.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 17 14:55:17 PST 2003
//
// Modifications:
//   Hank Childs, Fri Aug  1 10:44:45 PDT 2003
//   Added support for curves.
//
//   Brad Whitlock, Tue Sep 2 09:45:45 PDT 2003
//   I made it possible for materials to be treated differently than subset
//   variables. The materialVars map is a subset of subsetVars.
//
//   Hank Childs, Tue Sep 23 22:09:33 PDT 2003
//   Added support for tensors.
//
// ****************************************************************************

int
VariableMenuPopulator::UpdateSingleVariableMenu(QvisVariablePopupMenu *menu,
    QObject *receiver, int varTypes, bool changeVar)
{
    int numVarTypes = 0;
    int retval = 0;

    // See if we need to create a variable list that has more than one
    // category of variable in it.
    if(varTypes & VAR_CATEGORY_MESH)
       ++numVarTypes;
    if(varTypes & VAR_CATEGORY_SCALAR)
       ++numVarTypes;
    if(varTypes & VAR_CATEGORY_MATERIAL)
       ++numVarTypes;
    if(varTypes & VAR_CATEGORY_VECTOR)
       ++numVarTypes;
    if(varTypes & VAR_CATEGORY_SUBSET)
       ++numVarTypes;
    if(varTypes & VAR_CATEGORY_SPECIES)
       ++numVarTypes;
    if(varTypes & VAR_CATEGORY_CURVE)
       ++numVarTypes;
    if(varTypes & VAR_CATEGORY_TENSOR)
       ++numVarTypes;
    if(varTypes & VAR_CATEGORY_SYMMETRIC_TENSOR)
       ++numVarTypes;

    if(numVarTypes > 1)
    {
        StringBoolMap vars;

        // Construct a variable list that contains variables from multiple
        // categories then sort the list.
        if(varTypes & VAR_CATEGORY_MESH)
            AddVars(vars, meshVars);
        if(varTypes & VAR_CATEGORY_SCALAR)
            AddVars(vars, scalarVars);
        if(varTypes & VAR_CATEGORY_MATERIAL)
            AddVars(vars, materialVars);
        if(varTypes & VAR_CATEGORY_VECTOR)
            AddVars(vars, vectorVars);
        if(varTypes & VAR_CATEGORY_SUBSET)
            AddVars(vars, subsetVars);
        if(varTypes & VAR_CATEGORY_SPECIES)
            AddVars(vars, speciesVars);
        if(varTypes & VAR_CATEGORY_CURVE)
            AddVars(vars, curveVars);
        if(varTypes & VAR_CATEGORY_TENSOR)
            AddVars(vars, tensorVars);
        if(varTypes & VAR_CATEGORY_SYMMETRIC_TENSOR)
            AddVars(vars, symmTensorVars);
       
        // Update the menu with the composite variable list.
        UpdateSingleMenu(menu, receiver, vars, changeVar);
        retval = vars.size();
    }
    else
    {
        // The menu only contains one type of variable so update the menu
        // based on the variable type.
        switch(varTypes)
        {
        case VAR_CATEGORY_MESH:
            UpdateSingleMenu(menu, receiver, meshVars, changeVar);
            retval = meshVars.size();
            break;
        case VAR_CATEGORY_SCALAR:
            UpdateSingleMenu(menu, receiver, scalarVars, changeVar);
            retval = scalarVars.size();
            break;
        case VAR_CATEGORY_VECTOR:
            UpdateSingleMenu(menu, receiver, vectorVars, changeVar);
            retval = vectorVars.size();
            break;
        case VAR_CATEGORY_MATERIAL:
            UpdateSingleMenu(menu, receiver, materialVars, changeVar);
            retval = materialVars.size();
            break;
        case VAR_CATEGORY_SUBSET:
            UpdateSingleMenu(menu, receiver, subsetVars, changeVar);
            retval = subsetVars.size();
            break;
        case VAR_CATEGORY_SPECIES:
            UpdateSingleMenu(menu, receiver, speciesVars, changeVar);
            retval = speciesVars.size();
            break;
        case VAR_CATEGORY_CURVE:
            UpdateSingleMenu(menu, receiver, curveVars, changeVar);
            retval = curveVars.size();
            break;
        case VAR_CATEGORY_TENSOR:
            UpdateSingleMenu(menu, receiver, tensorVars, changeVar);
            retval = curveVars.size();
            break;
        case VAR_CATEGORY_SYMMETRIC_TENSOR:
            UpdateSingleMenu(menu, receiver, symmTensorVars, changeVar);
            retval = curveVars.size();
            break;
        }
    }

    return retval;
}

// ****************************************************************************
// Method: VariableMenuPopulator::ItemEnabled
//
// Purpose: 
//   Returns whether a plot with the given variable type signature should be
//   enabled given the current variables.
//
// Arguments:
//   varType : The types of variables that the plot handles.
//
// Returns:    true if the plot is enabled, false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 18 08:31:34 PDT 2003
//
// Modifications:
//   
//   Hank Childs, Fri Aug  1 10:44:45 PDT 2003
//   Add support for curves.
//
//   Hank Childs, Tue Sep 23 22:09:33 PDT 2003
//   Added support for tensors.
//
// ****************************************************************************

bool
VariableMenuPopulator::ItemEnabled(int varType) const
{
    bool retval = false;

    if(varType & VAR_CATEGORY_MESH)
       retval |= (meshVars.size() > 0);
    if(varType & VAR_CATEGORY_SCALAR)
       retval |= (scalarVars.size() > 0);
    if(varType & VAR_CATEGORY_MATERIAL)
       retval |= (subsetVars.size() > 0);
    if(varType & VAR_CATEGORY_VECTOR)
       retval |= (vectorVars.size() > 0);
    if(varType & VAR_CATEGORY_SUBSET)
       retval |= (subsetVars.size() > 0);
    if(varType & VAR_CATEGORY_SPECIES)
       retval |= (speciesVars.size() > 0);
    if(varType & VAR_CATEGORY_CURVE)
       retval |= (curveVars.size() > 0);
    if(varType & VAR_CATEGORY_TENSOR)
       retval |= (tensorVars.size() > 0);
    if(varType & VAR_CATEGORY_SYMMETRIC_TENSOR)
       retval |= (symmTensorVars.size() > 0);

    return retval;
}

// ****************************************************************************
// Method: VariableMenuPopulator::UpdateSingleMenu
//
// Purpose: 
//   Updates a variable list so it contains the correct variables.
//
// Arguments:
//   type : This corresponds to the type of plot whose menu
//          we're updating.
//   vars : A map of strings and bools representing the variable list.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 17 14:31:20 PST 2003
//
// Modifications:
//
// ****************************************************************************

void
VariableMenuPopulator::UpdateSingleMenu(QvisVariablePopupMenu *menu,
    QObject *receiver, const StringBoolMap &vars, bool changeVar)
{
    if (menu == 0)
        return;

    // Remove the existing menu entries.
    menu->clear();

    // Add each variable to the variable menu.
    std::map <std::string, QvisVariablePopupMenu *> popups;
    int j, varCount = 0;
    StringBoolMap::const_iterator pos;
    for(pos = vars.begin(); pos != vars.end(); ++pos)
    {
        // Split the variable's path into a vector of strings.
        stringVector pathvar;
        Split(pos->first, pathvar);

        // Add the submenus.
        QvisVariablePopupMenu *parent = menu;
        string path;
        for (j = 0; j < pathvar.size() - 1; ++j)
        {
            // Create the current path.
            path += (pathvar[j] + "/");

            // See if the current path is in the map. If it is then
            // do nothing. If the path is not in the map then we
            // add a new popup menu.
            std::map<std::string, QvisVariablePopupMenu *>::const_iterator p =
                popups.find(path);
            if(p == popups.end())
            {
                QvisVariablePopupMenu *newPopup =
                    new QvisVariablePopupMenu(menu->getPlotType(), parent,
                                              path.c_str());
                newPopup->setVarPath(path.c_str());
                if (changeVar)
                {
                    QObject::connect(newPopup, SIGNAL(activated(int, const QString &)),
                                     receiver, SLOT(changeVariable(int, const QString &)));
                }
                else
                {
                    QObject::connect(newPopup, SIGNAL(activated(int, const QString &)),
                                     receiver, SLOT(addPlot(int, const QString &)));
                }

                popups[path] = newPopup;
                parent->insertItem(pathvar[j].c_str(), newPopup, -1, parent->count());
                parent = newPopup;
            }
            else
                parent = p->second;
        }

        // Add the variable.
        int id = parent->insertItem(pathvar[j].c_str(), varCount++, parent->count());
        parent->setItemEnabled(id, pos->second);
    }
}

// ****************************************************************************
// Method: VariableMenuPopulator::Split
//
// Purpose: 
//   Splits a variable name but it ignores slashes that are enclosed in
//   parenthesis so Subset variables in subdirectories are split correctly.
//
// Arguments:
//   varName : The path to split.
//   pieces  : The pieces of the path.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 20 17:23:19 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
VariableMenuPopulator::Split(const std::string &varName, stringVector &pieces) const
{
    std::string word;
    int         parenthesis = 0;

    // Iterate through the characters in the word splitting
    for(int i = 0; i < varName.size(); ++i)
    {
        char c = varName[i];
        if(c == '(')
        {
            ++parenthesis;
            word += c;
        }
        else if(c == ')')
        {
            --parenthesis;
            word += c;
        }
        else if(c == '/')
        {
            if(parenthesis > 0)
                word += c;
            else
            {
                pieces.push_back(word);
                word = "";
            }
        }
        else
            word += c;
    }

    if(word.size() > 0)
        pieces.push_back(word);
}

// ****************************************************************************
// Method: VariableMenuPopulator::AddVars
//
// Purpose: 
//   Internal helper method that adds the contents of one map to another map.
//
// Arguments:
//   to   : The destination map.
//   from : The source map.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 18 08:26:22 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
VariableMenuPopulator::AddVars(VariableMenuPopulator::StringBoolMap &to,
    const VariableMenuPopulator::StringBoolMap &from)
{
    StringBoolMap::const_iterator pos;
    for(pos = from.begin(); pos != from.end(); ++pos)
    {
        to[pos->first] = pos->second;
    }
}
