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

#include <VariableMenuPopulator.h>

#include <Init.h>
#include <StringHelpers.h>
#include <avtDatabaseMetaData.h>
#include <avtSIL.h>
#include <PlotPluginInfo.h>
#include <Expression.h>
#include <ExpressionList.h>
#include <QvisVariablePopupMenu.h>
#include <qobject.h>

#include <set>

//#define DEBUG_PRINT
#ifdef DEBUG_PRINT
#include <DebugStream.h>
#endif
#include <TimingsManager.h>

using std::map;
using std::string;
using std::vector;

// ****************************************************************************
// Function: Split
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
//   Brad Whitlock, Thu Aug 18 15:58:40 PST 2005
//   I made it be a static function since it has to work on data from two
//   classes and it did not have access to one of the class's data.
//
// ****************************************************************************

static void
Split(const std::string &varName, stringVector &pieces)
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
//   Brad Whitlock, Fri Apr 1 16:17:09 PST 2005
//   Added labelVars.
//
//   Hank Childs, Tue Jul 19 14:23:56 PDT 2005
//   Added arrayVars.
//
//   Brad Whitlock, Wed Mar 22 12:06:59 PDT 2006
//   I added groupingInfo.
//
// ****************************************************************************

VariableMenuPopulator::VariableMenuPopulator() :
    cachedDBName(), cachedExpressionList(),
    meshVars(), scalarVars(), materialVars(), vectorVars(), subsetVars(),
    speciesVars(), curveVars(), tensorVars(), symmTensorVars(), labelVars(),
    arrayVars(), groupingInfo()
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
//   Brad Whitlock, Wed Mar 22 12:07:33 PDT 2006
//   I made it call ClearGroupingInfo.
//
// ****************************************************************************

VariableMenuPopulator::~VariableMenuPopulator()
{
    ClearGroupingInfo();
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
// Method: VariableMenuPopulator::ClearGroupingInfo
//
// Purpose: 
//   Clear out the grouping info.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 22 12:09:03 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
VariableMenuPopulator::ClearGroupingInfo()
{
    for(IntGroupingInfoMap::iterator pos = groupingInfo.begin();
        pos != groupingInfo.end(); ++pos)
    {
        if(pos->second != 0)
            delete pos->second;
    }

    groupingInfo.clear();
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
//   Mark C. Miller, Wed Apr 14 10:51:23 PDT 2004
//   I added code to include the catch-all mesh name in the list of meshes
//   when the plugin has activiated this feature
//
//   Jeremy Meredith, Mon Jun 21 15:13:20 PDT 2004
//   SUBSET variables now no-longer include MATERIAL variables.  The
//   Filled Boundary and Boundary plots have been changed to MATERIAL, and
//   the Subset plot is the only one that will do other kinds of subsets like
//   domains -- at least in the GUI.
//
//   Brad Whitlock, Thu Aug 5 14:42:13 PST 2004
//   I made it use VariableList instead of StringBoolMap.
//
//   Jeremy Meredith, Tue Aug 24 16:18:19 PDT 2004
//   Force an update if it is metadata from a simulation.
//
//   Brad Whitlock, Fri Feb 18 11:39:20 PDT 2005
//   I made it use only the user-defined expressions and the expressions
//   from the metadata so we don't accidentally get menus that contain
//   database expressions from databases other than the one that we're
//   using here.
//
//   Brad Whitlock, Fri Apr 1 16:17:42 PST 2005
//   Added support for label vars.
//
//   Hank Childs, Tue Jul 19 14:23:56 PDT 2005
//   Added support for array vars.
//
//   Brad Whitlock, Wed Mar 22 12:10:30 PDT 2006
//   I made it clear out the variable grouping information. I also added
//   timing code.
//
//   Jeremy Meredith, Thu Aug 24 11:31:58 EDT 2006
//   For now, keep enumerated scalars out of the subset variable types.
//   When the other infrastructure is ready, we can add them back.
//
//   Brad Whitlock, Thu Mar 8 10:34:25 PDT 2007
//   Use new avtDatabaseMetaData interface.
//
//   Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//   Added support to treat all databases as time varying
//
//   Brad Whitlock, Thu Dec 20 17:10:57 PST 2007
//   Added code to compare the variable lists before and after to see if the
//   variable menus need to really be created. We often call this routine
//   many times for the same "treat as time varying" file and we don't have
//   to recreate menus each time; only when the variable lists change.
//
// ****************************************************************************

bool
VariableMenuPopulator::PopulateVariableLists(const std::string &dbName,
    const avtDatabaseMetaData *md, const avtSIL *sil,
    const ExpressionList *exprList, bool treatAllDBsAsTimeVarying)
{
    if(md == 0 || sil == 0 || exprList == 0)
        return false;

    const char *mName = "VariableMenuPopulator::PopulateVariableLists: ";
    int total = visitTimer->StartTimer();
    int id = visitTimer->StartTimer();

    // The expression list can sometimes contain database expressions for
    // a database that is not the database specified by dbName. To combat
    // this problem, we only use the user-defined expressions from the
    // passed in exprList and we supplement it with any database expressions
    // that are contained in the metadata md.
    ExpressionList newExpressionList;
    GetRelevantExpressions(newExpressionList, md, *exprList);
    visitTimer->StopTimer(id, "GetRelevantExpressions");

    //
    // If the database name is the same and the expression list is the
    // same then return false, indicating that no updates are required.
    // If this is a simulation, then the variable list might change at
    // any time, so treat that as equivalent to MustRepopulateOnStateChange.
    //
    bool expressionsSame = newExpressionList == cachedExpressionList;
    bool variableMetaData = md->GetMustRepopulateOnStateChange() ||
                            md->GetIsSimulation();
    if(dbName == cachedDBName && expressionsSame && !variableMetaData &&
       !treatAllDBsAsTimeVarying)
    {
        visitTimer->StopTimer(total, mName);
        return false;
    }

    //
    // Save the database name and the expression list so we can check them
    // again later so we can get out of this routine early if they are the
    // same.
    //
    id = visitTimer->StartTimer();
    cachedDBName = dbName;
    if(!expressionsSame)
        cachedExpressionList = newExpressionList;

    // Save off the current variable lists so we can compare in the 
    // treatAllDBsAsTimeVarying case.
    VariableList old_meshVars(meshVars);
    VariableList old_scalarVars(scalarVars);
    VariableList old_vectorVars(vectorVars);
    VariableList old_materialVars(materialVars);
    VariableList old_subsetVars(subsetVars);
    VariableList old_speciesVars(speciesVars);
    VariableList old_curveVars(curveVars);
    VariableList old_tensorVars(tensorVars);
    VariableList old_symmTensorVars(symmTensorVars);
    VariableList old_labelVars(labelVars);
    VariableList old_arrayVars(arrayVars);

    // Clear out the variable lists and set their sorting method..
    meshVars.Clear();        meshVars.SetSorted(md->GetMustAlphabetizeVariables());
    scalarVars.Clear();      scalarVars.SetSorted(md->GetMustAlphabetizeVariables());
    vectorVars.Clear();      vectorVars.SetSorted(md->GetMustAlphabetizeVariables());
    materialVars.Clear();    materialVars.SetSorted(md->GetMustAlphabetizeVariables());
    subsetVars.Clear();      subsetVars.SetSorted(md->GetMustAlphabetizeVariables());
    speciesVars.Clear();     speciesVars.SetSorted(md->GetMustAlphabetizeVariables());
    curveVars.Clear();       curveVars.SetSorted(md->GetMustAlphabetizeVariables());
    tensorVars.Clear();      tensorVars.SetSorted(md->GetMustAlphabetizeVariables());
    symmTensorVars.Clear();  symmTensorVars.SetSorted(md->GetMustAlphabetizeVariables());
    labelVars.Clear();       labelVars.SetSorted(md->GetMustAlphabetizeVariables());
    arrayVars.Clear();       arrayVars.SetSorted(md->GetMustAlphabetizeVariables());

    // Clear out the variable grouping info.
    ClearGroupingInfo();
    visitTimer->StopTimer(id, "Clearing vectors and grouping info");

    // Do stuff with the metadata
    id = visitTimer->StartTimer(); 
    int i;
    for (i = 0; i < md->GetNumMeshes(); ++i)
    {
        const avtMeshMetaData &mmd = md->GetMeshes(i);
        meshVars.AddVariable(mmd.name, mmd.validVariable);
    }
    if (md->GetUseCatchAllMesh())
        meshVars.AddVariable(Init::CatchAllMeshName, true);
    for (i = 0; i < md->GetNumScalars(); ++i)
    {
        const avtScalarMetaData &smd = md->GetScalars(i);
        scalarVars.AddVariable(smd.name, smd.validVariable);
    }
    for (i = 0; i < md->GetNumVectors(); ++i)
    {
        const avtVectorMetaData &vmd = md->GetVectors(i);
        vectorVars.AddVariable(vmd.name, vmd.validVariable);
    }
    for (i = 0; i < md->GetNumSpecies(); ++i)
    {
        const avtSpeciesMetaData &smd = md->GetSpecies(i);
        speciesVars.AddVariable(smd.name, smd.validVariable);
    }
    for (i = 0; i < md->GetNumCurves(); ++i)
    {
        const avtCurveMetaData &cmd = md->GetCurves(i);
        curveVars.AddVariable(cmd.name, cmd.validVariable);
    }
    for (i = 0; i < md->GetNumTensors(); ++i)
    {
        const avtTensorMetaData &tmd = md->GetTensors(i);
        tensorVars.AddVariable(tmd.name, tmd.validVariable);
    }
    for (i = 0; i < md->GetNumSymmTensors(); ++i)
    {
        const avtSymmetricTensorMetaData &tmd = md->GetSymmTensors(i);
        symmTensorVars.AddVariable(tmd.name, tmd.validVariable);
    }
    for (i = 0; i < md->GetNumLabels(); ++i)
    {
        const avtLabelMetaData &tmd = md->GetLabels(i);
        labelVars.AddVariable(tmd.name, tmd.validVariable);
    }
    for (i = 0; i < md->GetNumArrays(); ++i)
    {
        const avtArrayMetaData &tmd = md->GetArrays(i);
        arrayVars.AddVariable(tmd.name, tmd.validVariable);
    }
    visitTimer->StopTimer(id, "Adding variables from metadata");

    // Do stuff with the sil
    id = visitTimer->StartTimer(); 
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

            //
            // Also add the varName to the material variable map so we can
            // have plots, etc that just use materials instead of any type
            // of subset variable.
            //
            if (role == SIL_MATERIAL)
                materialVars.AddVariable(varName, validVariable);
            else if (role != SIL_ENUMERATION)
                subsetVars.AddVariable(varName, validVariable);
        }
    }
    visitTimer->StopTimer(id, "Adding SIL categories");

    //
    // Add the expressions from the cached expression list.
    //
    id = visitTimer->StartTimer(); 
    int nexp = cachedExpressionList.GetNumExpressions();
    for(i = 0; i < nexp; ++i)
    {
        const Expression &expr = cachedExpressionList[i];
        if(!expr.GetHidden())
            AddExpression(expr);
    }

    // Determine the return value.
    bool populationWillCauseUpdate = true;
    if(treatAllDBsAsTimeVarying || variableMetaData)
    {
        // This is a last opportunity for us to return false for the case where
        // we're treating all DB's as time varying. Remember, this update routine
        // could be called many times with the same inputs. We don't want to return
        // true for treatAllDBsAsTimeVarying or sims or mustRepopulateOnStateChange
        // if we've already cached the variables for its inputs on a previous call.
        // Returning true more than we need to causes the menus to be regenerated 
        // downstream, which is pretty costly.
        populationWillCauseUpdate =
            old_meshVars != meshVars ||
            old_scalarVars != scalarVars ||
            old_vectorVars != vectorVars ||
            old_materialVars != materialVars ||
            old_subsetVars != subsetVars ||
            old_speciesVars != speciesVars ||
            old_curveVars != curveVars ||
            old_tensorVars != tensorVars ||
            old_symmTensorVars != symmTensorVars ||
            old_labelVars != labelVars ||
            old_arrayVars != arrayVars;
    }

    visitTimer->StopTimer(id, "Adding expressions");
    visitTimer->StopTimer(total, mName);

    return populationWillCauseUpdate;
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
//   Brad Whitlock, Thu Aug 5 14:35:26 PST 2004
//   Made it use VariableList.
//
//   Hank Childs, Thu Jul 21 12:54:01 PDT 2005
//   Add arrays.
//
//   Kathleen Bonnell, Wed Aug 2 18:06:58 PDT 2006 
//   Add Curve expressions.
//
// ****************************************************************************

void
VariableMenuPopulator::AddExpression(const Expression &expr)
{
    // Figure out which list this expression should be added to.
    VariableList *m = 0;
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
    case Expression::ArrayMeshVar:
        m = &arrayVars;
        break;
    case Expression::CurveMeshVar:
        m = &curveVars;
        break;
    default:
        break;
    }

    // Check if the name is already in the list
    if(m)
    {
        if(!m->Contains(expr.GetName()))
            m->AddVariable(expr.GetName(), true);
        else
            m->AddVariable(expr.GetName() + " (expression)", true);
    }
}

// ****************************************************************************
// Method: VariableMenuPopulator::GetRelevantExpressions
//
// Purpose: 
//   Gets the list of user-defined expressions and expressions that come
//   from the metadata.
//
// Arguments:
//   newExpressionList : The new expression list.
//   md                : The metadata that we're searching for expressions.
//   exprList          : The expression list to use for the user-defined
//                       expressions.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 18 11:36:26 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
VariableMenuPopulator::GetRelevantExpressions(ExpressionList &newExpressionList,
    const avtDatabaseMetaData *md, const ExpressionList &exprList)
{
    int i;

    // Get the user-defined expressions.
    for(i = 0; i < exprList.GetNumExpressions(); ++i)
    {
        const Expression &e = exprList[i];
        if(!e.GetHidden() && !e.GetFromDB())
            newExpressionList.AddExpressions(e);
    }

    // Get the expressions from the metadata.
    for(i = 0; i < md->GetNumberOfExpressions(); ++i)
    {
        const Expression *e = md->GetExpression(i);
        if(e != 0 && !e->GetHidden())
            newExpressionList.AddExpressions(*e);
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
//   receiver  : The QObject whose slot function will be called.
//   slot      : The slot function that will be called on the receiver.
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
//   Brad Whitlock, Thu Aug 5 14:30:13 PST 2004
//   I made it use VariableList instead of StringBoolMap.
//
//   Brad Whitlock, Wed Dec 8 14:07:55 PST 2004
//   I changed how the slot functions for the menu are hooked up.
//
//   Brad Whitlock, Fri Apr 1 16:18:52 PST 2005
//   Added label var support.
//
//   Hank Childs, Tue Jul 19 14:23:56 PDT 2005
//   Added array var support.
//
//   Brad Whitlock, Wed Mar 22 11:59:46 PDT 2006
//   Added support for caching the variable grouping for different variable
//   types.
//
// ****************************************************************************

int
VariableMenuPopulator::UpdateSingleVariableMenu(QvisVariablePopupMenu *menu,
    int varTypes, QObject *receiver, const char *slot)
{
    int total = visitTimer->StartTimer();
    int numVarTypes = 0;
    int retval = 0;

#define UPDATE_SINGLE_MENU(V) \
    if(groupingInfo.find(varTypes) == groupingInfo.end())\
    {\
        groupingInfo[varTypes] = new GroupingInfo;\
        groupingInfo[varTypes]->required =\
            V.IsGroupingRequired(\
                groupingInfo[varTypes]->grouping);\
    }\
    UpdateSingleMenu(menu, V, receiver, slot,\
        groupingInfo[varTypes]);\
    retval = V.Size();

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
    if(varTypes & VAR_CATEGORY_LABEL)
       ++numVarTypes;
    if(varTypes & VAR_CATEGORY_ARRAY)
       ++numVarTypes;

    if(numVarTypes > 1)
    {
        VariableList vars;
        vars.SetSorted(meshVars.GetSorted());

        // Construct a variable list that contains variables from multiple
        // categories then sort the list.
        int realType = 0;
        if(varTypes & VAR_CATEGORY_MESH)
            realType |= AddVars(vars, meshVars) ? VAR_CATEGORY_MESH : 0;
        if(varTypes & VAR_CATEGORY_SCALAR)
            realType |= AddVars(vars, scalarVars) ? VAR_CATEGORY_SCALAR : 0;
        if(varTypes & VAR_CATEGORY_MATERIAL)
            realType |= AddVars(vars, materialVars) ? VAR_CATEGORY_MATERIAL : 0;
        if(varTypes & VAR_CATEGORY_VECTOR)
            realType |= AddVars(vars, vectorVars) ? VAR_CATEGORY_VECTOR : 0;
        if(varTypes & VAR_CATEGORY_SUBSET)
            realType |= AddVars(vars, subsetVars) ? VAR_CATEGORY_SUBSET : 0;
        if(varTypes & VAR_CATEGORY_SPECIES)
            realType |= AddVars(vars, speciesVars) ? VAR_CATEGORY_SPECIES : 0;
        if(varTypes & VAR_CATEGORY_CURVE)
            realType |= AddVars(vars, curveVars) ? VAR_CATEGORY_CURVE : 0;
        if(varTypes & VAR_CATEGORY_TENSOR)
            realType |= AddVars(vars, tensorVars) ? VAR_CATEGORY_TENSOR : 0;
        if(varTypes & VAR_CATEGORY_SYMMETRIC_TENSOR)
            realType |= AddVars(vars, symmTensorVars) ? VAR_CATEGORY_SYMMETRIC_TENSOR : 0;
        if(varTypes & VAR_CATEGORY_LABEL)
            realType |= AddVars(vars, labelVars) ? VAR_CATEGORY_LABEL : 0;
        if(varTypes & VAR_CATEGORY_ARRAY)
            realType |= AddVars(vars, arrayVars) ? VAR_CATEGORY_ARRAY : 0;
       
        // Update the menu with the composite variable list.
        if(realType > 0)
        {
            varTypes = realType;
            UPDATE_SINGLE_MENU(vars)
        }
    }
    else
    {
        // The menu only contains one type of variable so update the menu
        // based on the variable type.
        switch(varTypes)
        {
        case VAR_CATEGORY_MESH:
            UPDATE_SINGLE_MENU(meshVars)
            break;
        case VAR_CATEGORY_SCALAR:
            UPDATE_SINGLE_MENU(scalarVars)
            break;
        case VAR_CATEGORY_VECTOR:
            UPDATE_SINGLE_MENU(vectorVars)
            break;
        case VAR_CATEGORY_MATERIAL:
            UPDATE_SINGLE_MENU(materialVars)
            break;
        case VAR_CATEGORY_SUBSET:
            UPDATE_SINGLE_MENU(subsetVars)
            break;
        case VAR_CATEGORY_SPECIES:
            UPDATE_SINGLE_MENU(speciesVars)
            break;
        case VAR_CATEGORY_CURVE:
            UPDATE_SINGLE_MENU(curveVars)
            break;
        case VAR_CATEGORY_TENSOR:
            UPDATE_SINGLE_MENU(tensorVars)
            break;
        case VAR_CATEGORY_SYMMETRIC_TENSOR:
            UPDATE_SINGLE_MENU(symmTensorVars)
            break;
        case VAR_CATEGORY_LABEL:
            UPDATE_SINGLE_MENU(labelVars)
            break;
        case VAR_CATEGORY_ARRAY:
            UPDATE_SINGLE_MENU(arrayVars)
            break;
        }
    }

    visitTimer->StopTimer(total, "VariableMenuPopulator::UpdateSingleVariableMenu");

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
//   Brad Whitlock, Thu Aug 5 15:32:50 PST 2004
//   Made it use VariableList.
//
//   Brad Whitlock, Fri Apr 1 16:20:10 PST 2005
//   Added label var support.
//
//   Hank Childs, Tue Jul 19 14:23:56 PDT 2005
//   Added array var suppor.
//
// ****************************************************************************

bool
VariableMenuPopulator::ItemEnabled(int varType) const
{
    bool retval = false;

    if(varType & VAR_CATEGORY_MESH)
       retval |= (meshVars.Size() > 0);
    if(varType & VAR_CATEGORY_SCALAR)
       retval |= (scalarVars.Size() > 0);
    if(varType & VAR_CATEGORY_MATERIAL)
       retval |= (subsetVars.Size() > 0);
    if(varType & VAR_CATEGORY_VECTOR)
       retval |= (vectorVars.Size() > 0);
    if(varType & VAR_CATEGORY_SUBSET)
       retval |= (subsetVars.Size() > 0);
    if(varType & VAR_CATEGORY_SPECIES)
       retval |= (speciesVars.Size() > 0);
    if(varType & VAR_CATEGORY_CURVE)
       retval |= (curveVars.Size() > 0);
    if(varType & VAR_CATEGORY_TENSOR)
       retval |= (tensorVars.Size() > 0);
    if(varType & VAR_CATEGORY_SYMMETRIC_TENSOR)
       retval |= (symmTensorVars.Size() > 0);
    if(varType & VAR_CATEGORY_LABEL)
       retval |= (labelVars.Size() > 0);
    if(varType & VAR_CATEGORY_ARRAY)
       retval |= (arrayVars.Size() > 0);

    return retval;
}

// ****************************************************************************
// Method: VariableMenuPopulator::UpdateSingleMenu
//
// Purpose: 
//   Updates a variable list so it contains the correct variables.
//
// Arguments:
//   menu     : The menu that we want to update.
//   vars     : A map of strings and bools representing the variable list.
//   receiver : The QObject that will handle signals emitted by the menu.
//   slot     : The slot function that will be called on the receiver when
//              signals are emitted by the menu.
//   ginfo    : Variable grouping info for the variable list that is passed.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 17 14:31:20 PST 2003
//
// Modifications:
//   Brad Whitlock, Thu Aug 5 14:27:22 PST 2004
//   I made it use VariableList.
//
//   Brad Whitlock, Fri Dec 3 13:26:06 PST 2004
//   I removed the code to clear the menu and changed the slot hookup code
//   so it is more general.
//
//   Mark C. Miller, Tue Jul 26 17:22:22 PDT 2005
//   Modified to take into account possibility of grouping the hierarchy of
//   popup menus.
//
//   Mark C. Miller, Mon Nov 28 15:37:01 PST 2005
//   Fixed bug where when grouping was required AND a component in a variable's
//   pathname had 3 or fewer characters, we'd get a SEGV
//
//   Brad Whitlock, Mon Feb 27 17:20:36 PST 2006
//   I added a check to make sure that the var name is in the
//   originalNameToGroupedName map before accessing it because we were sometimes
//   accidentally adding empty strings to the map for expressions like 
//   mesh quality metrics.
//
//   Brad Whitlock, Wed Mar 22 11:53:47 PDT 2006
//   I added code to pass in GroupingInfo, which contains the grouped names
//   to original names map for this variable list. We pass it in so we can
//   calculate it and re-use it for multiple menus.
//
// ****************************************************************************

void
VariableMenuPopulator::UpdateSingleMenu(QvisVariablePopupMenu *menu,
    VariableList &vars, QObject *receiver, const char *slot, 
    const VariableMenuPopulator::GroupingInfo *ginfo)
{
    if (menu == 0)
        return;

    // Add each variable to the variable menu.
    std::map <std::string, QvisVariablePopupMenu *> popups;
    int j, varCount = menu->count();
    std::string var;
    bool        validVar;
    const StringStringMap &originalNameToGroupedName = ginfo->grouping;
    bool shouldUseGrouping = ginfo->required;
    vars.InitTraversal();
    while(vars.GetNextVariable(var, validVar))
    {
        if (shouldUseGrouping)
        {
            StringStringMap::const_iterator it = originalNameToGroupedName.find(var);
            if(it != originalNameToGroupedName.end())
                 var = it->second;
        }

        // Split the variable's path into a vector of strings.
        stringVector pathvar;
        Split(var, pathvar);

        // If nothing resulted from the split then continue.
        if(pathvar.size() < 1)
            continue;

        // Add the submenus.
        QvisVariablePopupMenu *parent = menu;
        string path, altpath;

        for (j = 0; j < pathvar.size() - 1; ++j)
        {
            // Create the current path.
            path += (pathvar[j] + "/");
            if (shouldUseGrouping)
            {
                if (path.size() > 3)
                {
                    if (string(path, path.size()-4, 4) != ".../")
                        altpath += (pathvar[j] + "/");
                }
                else
                {
                    altpath += (pathvar[j] + "/"); 
                }
            }
            else
            {
                altpath += (pathvar[j] + "/"); 
            }

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
                newPopup->setVarPath(altpath.c_str());
                if (receiver != 0 && slot != 0)
                {
                    QObject::connect(newPopup, SIGNAL(activated(int, const QString &)),
                        receiver, slot);
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
        parent->setItemEnabled(id, validVar);
    }
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
// Returns:    True if variables were added; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 18 08:26:22 PDT 2003
//
// Modifications:
//   Brad Whitlock, Thu Aug 5 14:23:48 PST 2004
//   Changed to VariableList instead of StringBoolMap.
//
// ****************************************************************************

bool
VariableMenuPopulator::AddVars(VariableMenuPopulator::VariableList &to,
    VariableMenuPopulator::VariableList &from)
{
    std::string var;
    bool        validVar;

    from.InitTraversal();
    while(from.GetNextVariable(var, validVar))
        to.AddVariable(var, validVar);

    return (from.Size() > 0);
}

//
// VariableMenuPopulator::VariableList
//

// ****************************************************************************
// Method: VariableMenuPopulator::VariableList::VariableList
//
// Purpose: 
//   Constructor for the VariableList class.
//
// Note:       This class is an interface on top of different containers that
//             allow us to create sorted or unsorted variable lists. The
//             containers used depend on whether we're sorting.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 5 14:45:02 PST 2004
//
// Modifications:
//   
// ****************************************************************************

VariableMenuPopulator::VariableList::VariableList() : sortedVariables(),
    sortedVariablesIterator(), unsortedVariableNames(), unsortedVariableValid()
{
    sorted = true;
    unsortedVariableIndex = -1;
}

VariableMenuPopulator::VariableList::VariableList(const VariableMenuPopulator::VariableList &obj) : 
    sortedVariables(obj.sortedVariables),
    sortedVariablesIterator(obj.sortedVariablesIterator),
    unsortedVariableNames(obj.unsortedVariableNames),
    unsortedVariableValid(obj.unsortedVariableValid)
{
    sorted = obj.sorted;
    unsortedVariableIndex = obj.unsortedVariableIndex;
}

// ****************************************************************************
// Method: VariableMenuPopulator::VariableList::~VariableList
//
// Purpose: 
//   Destructor for the VariableList class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 5 14:46:44 PST 2004
//
// Modifications:
//   
// ****************************************************************************

VariableMenuPopulator::VariableList::~VariableList()
{
}

// ****************************************************************************
// Method: VariableMenuPopulator::VariableList::operator == 
//
// Purpose: 
//   == operator
//
// Arguments:
//   obj : The variable list object that we're comparing.
//
// Returns:    true if the object is equal to this object; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 14 12:05:47 PST 2007
//
// Modifications:
//   
// ****************************************************************************

bool
VariableMenuPopulator::VariableList::operator == (const VariableMenuPopulator::VariableList &obj) const
{
    bool equal = false;
    if(sorted == obj.sorted)
    {
        if(sorted)
        {
            equal = sortedVariables == obj.sortedVariables;
        }
        else
        {
            equal = unsortedVariableNames == obj.unsortedVariableNames &&
                    unsortedVariableValid == obj.unsortedVariableValid;
        }
    }

    return equal;
}

// ****************************************************************************
// Method: VariableMenuPopulator::VariableList::operator != 
//
// Purpose: 
//   != operator.
//
// Arguments:
//   obj : The variable list object that we're comparing.
//
// Returns:    false if the object is equal to this object; true otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 14 12:07:08 PST 2007
//
// Modifications:
//   
// ****************************************************************************

bool
VariableMenuPopulator::VariableList::operator != (const VariableMenuPopulator::VariableList &obj) const
{
    return !this->operator == (obj);
}

// ****************************************************************************
// Method: VariableMenuPopulator::VariableList::AddVariable
//
// Purpose: 
//   Adds a variable to the appropriate container.
//
// Arguments:
//   var      : The name of the variable.
//   validVar : Whether the variable is valid.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 5 14:47:34 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
VariableMenuPopulator::VariableList::AddVariable(const std::string &var, bool validVar)
{
    if(sorted)
        sortedVariables[var] = validVar;
    else
    {
        unsortedVariableNames.push_back(var);
        unsortedVariableValid.push_back(validVar);
    }
}

// ****************************************************************************
// Method: VariableMenuPopulator::VariableList::Clear
//
// Purpose: 
//   Clears the variable list.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 5 15:28:48 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
VariableMenuPopulator::VariableList::Clear()
{
    sortedVariables.clear();
    unsortedVariableNames.clear();
    unsortedVariableValid.clear();
}

// ****************************************************************************
// Method: VariableMenuPopulator::VariableList::Contains
//
// Purpose: 
//   Returns whether the Variable list contains a variable.
//
// Arguments:
//   var : The variable to check for.
//
// Returns:    True if the list contains the variable; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 5 15:29:04 PST 2004
//
// Modifications:
//   
// ****************************************************************************

bool
VariableMenuPopulator::VariableList::Contains(const std::string &var) const
{
    if(sorted)
        return (sortedVariables.find(var) != sortedVariables.end());
    else
    {
        for(int i = 0; i < unsortedVariableNames.size(); ++i)
            if(unsortedVariableNames[i] == var)
                return true;
    }

    return false;
}

// ****************************************************************************
// Method: VariableMenuPopulator::VariableList::InitTraversal
//
// Purpose: 
//   Initializes the variable list for traversal.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 5 15:30:06 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
VariableMenuPopulator::VariableList::InitTraversal()
{
    unsortedVariableIndex = 0;
    sortedVariablesIterator = sortedVariables.begin();
}

// ****************************************************************************
// Method: VariableMenuPopulator::VariableList::GetNextVariable
//
// Purpose: 
//   Returns the current variable.
//
// Arguments:
//   var      : The name of the variable.
//   validVar : Whether the variable is valid.
//
// Returns:    True if a variable was returned; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 5 15:30:24 PST 2004
//
// Modifications:
//   
// ****************************************************************************

bool 
VariableMenuPopulator::VariableList::GetNextVariable(std::string &var, bool &validVar)
{
    bool retval;

    if(sorted)
    {
        retval = (sortedVariablesIterator != sortedVariables.end());
        if(retval)
        {
            var = sortedVariablesIterator->first;
            validVar = sortedVariablesIterator->second;
            ++sortedVariablesIterator;
        }
    }
    else
    {
        retval = (unsortedVariableIndex < unsortedVariableNames.size());
        if(retval)
        {
            var = unsortedVariableNames[unsortedVariableIndex];
            validVar = unsortedVariableValid[unsortedVariableIndex];
            ++unsortedVariableIndex;
        }
    }

    return retval;
}

// ****************************************************************************
// Method: VariableMenuPopulator::VariableList::Size
//
// Purpose: 
//   Returns the number of variables in the container.
//
// Returns:    The number of variables.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 5 15:33:52 PST 2004
//
// Modifications:
//   
// ****************************************************************************

int
VariableMenuPopulator::VariableList::Size() const
{
    return sorted ? sortedVariables.size() : unsortedVariableNames.size();
}

// ****************************************************************************
// Method: VariableMenuPopulator::VariableList::IsGroupingRequired
//
// Purpose: Determine if any levels in the menu hierarchy are so large that
// we should group them to reduce clutter.
//
// Returns:    True if grouping is required, false otherwise 
//
// Programmer: Mark C. Miller 
// Creation:   Tue Jul 26 17:22:22 PDT 2005 
//
// Modifications:
//    Brad Whitlock, Thu Aug 18 15:55:19 PST 2005
//    Fixed on win32.
//
//    Mark C. Miller, Tue Jan 17 18:18:17 PST 2006
//    Applied patch from Paul Selby of Wed Dec 14 13:27:45 GMT 2005
//    so that it will group variables in top level of path.
//
//    Brad Whitlock, Wed Mar 22 11:11:09 PDT 2006
//    Rewrote portions of the function to use std::set instead of vectors, 
//    which sped up the routine 10x. Another important change that I made to
//    this routine is that an entry in the originalNameToGroupedName map
//    is only added if one of the variable's paths was grouped. That is, not
//    all variables will have an entry in the map.
//
// ****************************************************************************

bool
VariableMenuPopulator::VariableList::IsGroupingRequired(
    StringStringMap& originalNameToGroupedName)
{
    const char *mName = "VariableMenuPopulator::VariableList::IsGroupingRequired";
    int total = visitTimer->StartTimer(); 

    string var;
    bool validVar;
    bool isGroupingRequired = false;
    const int GROUPING_CUTOFF = 90;

    //
    // STAGE 1: Do a quick and dirty check for whether we need to do
    //          any grouping. We use a map of ints to maintain a simple
    //          count of how many objects will go into each path. If any
    //          of the paths exceed the grouping cutoff then we proceed
    //          with more expensive work later. We do this quick first pass
    //          because it's pretty cheap.
    //
    int stage1 = visitTimer->StartTimer();
    if(Size() > GROUPING_CUTOFF)
    {
#define INCPATH \
        std::map<std::string, int>::const_iterator p2 =\
            entriesAtPathC.find(path);\
        if (p2 == entriesAtPathC.end())\
            entriesAtPathC[path] = 1;\
        else\
        {\
            ++entriesAtPathC[path];\
            if(entriesAtPathC[path])\
            {\
                isGroupingRequired = true;\
                break;\
            }\
        }

        std::map<std::string, int> entriesAtPathC;
        InitTraversal();
        while(GetNextVariable(var, validVar))
        {
            // Split the variable's path into a vector of strings.
            stringVector pathvar;
            Split(var, pathvar);

            string path;
            if(pathvar.size() == 1)
            {
                INCPATH
            }
            else if(pathvar.size() > 1)
            {
                for (int j = 0; j < pathvar.size()-1; j++)
                    path = path + "/" + pathvar[j];
                INCPATH
            }
        }
    }
    visitTimer->StopTimer(stage1, "Stage 1");

    //
    // Since we've now check to see if grouping is required, indicate that.
    // Also, if we've decided grouping is NOT required, just return.
    //
    if (!isGroupingRequired)
    {
        visitTimer->StopTimer(total, mName);
        return false;
    }

    //
    // STAGE 2: Create the entriesAtPath map, which is a map containing
    //          sets of names for each path. We use these sets later 
    //          and split them up if they are over the grouping threshold.
    //
    int stage2 = visitTimer->StartTimer();
    typedef std::set<std::string> StringSet;
    typedef std::map<std::string, StringSet> StringStringSetMap;
    StringStringSetMap entriesAtPath;
    InitTraversal();
    while(GetNextVariable(var, validVar))
    {
        // Split the variable's path into a vector of strings.
        stringVector pathvar;
        Split(var, pathvar);

        string path;
        for (int j = 0; j < pathvar.size(); j++)
        {
            // Get an iterator to the named set.
            StringStringSetMap::iterator p2 = entriesAtPath.find(path);
            if (p2 == entriesAtPath.end())
                entriesAtPath[path].insert(pathvar[j]);
            else
                p2->second.insert(pathvar[j]);

            path += (pathvar[j] + "/");
        }
    }
    visitTimer->StopTimer(stage2, "Stage 2");
#ifdef DEBUG_PRINT
    for(StringStringSetMap::const_iterator it = entriesAtPath.begin();
        it != entriesAtPath.end(); ++it)
    {
        debug1 << "key = \"" << it->first.c_str() << "\", values(sz="
               << it->second.size() << ") = {";
        for(StringSet::const_iterator si = it->second.begin();
            si != it->second.end(); ++si)
        {
            debug1 << "\"" << si->c_str() << "\", ";
        }
        debug1 << "}" << endl;
    }
    debug1 << "*********************************************" << endl;
#endif

    //
    // STAGE 3: In this stage, we iterate through the paths once more and
    //          use the entriesAtPath map to create groupings if there are
    //          too many entries in one set. We populate the
    //          originalNameToGroupedName map for items that have been 
    //          grouped so we know to use a different path for items that
    //          have been grouped so we populate the menus correctly.
    //
    int stage3 = visitTimer->StartTimer();
    typedef std::vector<StringSet> StringSetVector;
    typedef std::map<std::string, StringSetVector > StringStringSetVectorMap;
    StringStringSetVectorMap groupsAtPath;
    InitTraversal();
    while(GetNextVariable(var, validVar))
    {
        bool varNeedsGrouping = false;

        // Split the variable's path into a vector of strings.
        stringVector pathvar;
        Split(var, pathvar);

        // Force grouping to occur for variables in top level of path
        string path, newpath;
        int j;
        int jmax = (pathvar.size() == 1) ? 1 : (pathvar.size() - 1);
        for (j = 0; j < jmax; j++)
        {
            // If the set of strings at path is greater than the curoff then
            // we should break up entriesAtPath[path] into a list of smaller
            // groups of names so we can have smaller menus.
            if (entriesAtPath[path].size() > GROUPING_CUTOFF)
            {
                if (groupsAtPath.find(path) == groupsAtPath.end())
                {
                    StringSetVector groups;
                    StringHelpers::GroupStringsFixedAlpha(entriesAtPath[path], 20, groups);
                    groupsAtPath[path] = groups;
                }

                // Find a group that contains pathvar[j].
                StringStringSetVectorMap::const_iterator g = groupsAtPath.find(path);
                if(g != groupsAtPath.end())
                {
                    // g->second points to a StringSetVector. We want to
                    // determine which item StringSet in the vector contains 
                    // pathvar[j]. If we find a StringSet that contains 
                    // pathvar[j] then we need to augment the path because 
                    // some grouping has taken place.
                    for(int k = 0; k < g->second.size(); ++k)
                    {
                        if(g->second[k].find(pathvar[j]) != g->second[k].end())
                        {
                            newpath += (*g->second[k].begin()) + ".../";
                            varNeedsGrouping = true;
                            break;
                        }
                    }
                }
            }

            if (pathvar.size() != 1)
            {
                path += (pathvar[j] + "/");
                newpath += (pathvar[j] + "/");
            }
        }

        if(varNeedsGrouping)
        {
            if (pathvar.size() == 1)
                j = 0;
            originalNameToGroupedName[path + pathvar[j]] = newpath + pathvar[j];
        }
    }
    visitTimer->StopTimer(stage3, "Stage 3");
    visitTimer->StopTimer(total, mName);

    return true;
}

// ****************************************************************************
// Class: VariableMenuPopulator::GroupingInfo
//
// Purpose:
//   Contains grouping info for variable menu grouping.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 22 12:17:49 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

VariableMenuPopulator::GroupingInfo::GroupingInfo() : grouping()
{
    required = false;
}

VariableMenuPopulator::GroupingInfo::GroupingInfo(
    const VariableMenuPopulator::GroupingInfo &obj) :
        grouping(obj.grouping)
{
    required = obj.required;
}

VariableMenuPopulator::GroupingInfo::~GroupingInfo()
{
}

void VariableMenuPopulator::GroupingInfo::operator = (
    const VariableMenuPopulator::GroupingInfo &obj)
{
    grouping = obj.grouping;
    required = obj.required;
}

