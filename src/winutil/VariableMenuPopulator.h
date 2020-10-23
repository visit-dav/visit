// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VARIABLE_MENU_POPULATOR_H
#define VARIABLE_MENU_POPULATOR_H

#include <winutil_exports.h>

#include <ExpressionList.h>
#include <OperatorPluginInfo.h>
#include <OperatorPluginManager.h>

#include <maptypes.h>
#include <vectortypes.h>

class avtDatabaseMetaData;
class avtSIL;
class Expression;
class QvisVariablePopupMenu;
class QObject;

// ****************************************************************************
// Class: VariableMenuPopulator
//
// Purpose:
//   This object stores information about the variables in a file and
//   uses that information to construct a cascading variable menu.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 17 14:50:59 PST 2003
//
// Modifications:
//   Hank Childs, Fri Aug  1 10:44:45 PDT 2003
//   Add support for curves.
//
//   Brad Whitlock, Tue Sep 2 09:47:15 PDT 2003
//   Added materialVars map so we can distinguish between materials and
//   subset variables if we want to.
//
//   Hank Childs, Tue Sep 23 22:05:54 PDT 2003
//   Added support for tensors.
//
//   Brad Whitlock, Fri Oct 24 15:41:54 PST 2003
//   Added an internal method that helps add expressions to the right list.
//
//   Brad Whitlock, Tue Feb 24 15:51:18 PST 2004
//   Added cachedDBName and cachedExpressionList so the class can skip
//   unnecessary work in PopulateVariableLists. I also made
//   PopulateVariableLists return a bool indicating whether or not updates
//   are needed.
//
//   Brad Whitlock, Fri Dec 3 13:27:10 PST 2004
//   I changed how slots are hooked up on the UpdateSingleVariableMenu method.
//
//   Brad Whitlock, Fri Feb 18 11:38:03 PDT 2005
//   I added the GetRelevantExpressions helper method.
//
//   Brad Whitlock, Fri Apr 1 16:21:46 PST 2005
//   I added label var support.
//
//   Hank Childs, Tue Jul 19 14:23:56 PDT 2005
//   Added array var support.
//
//   Mark C. Miller, Tue Jul 26 17:22:22 PDT 2005
//   Added support for grouping of the variable menu hierarchy
//
//   Brad Whitlock, Thu Aug 18 15:07:32 PST 2005
//   Removed Split method and made it be a static function in the .C file.
//
//   Brad Whitlock, Wed Mar 22 12:05:43 PDT 2006
//   I added GroupingInfo and added another argument to UpdateSingleMenu.
//
//   Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//   Added bool to support to treat all databases as time varying to 
//   PopulateVariableLists.
//
//   Brad Whitlock, Fri Dec 14 12:00:55 PST 2007
//   Added methods to the VariableList class.
//
//   Rob Sisneros, Sun Aug 29 20:13:10 CDT 2010
//   Add argument to PopulateVariableLists for expressions from operators.
//
//   Brad Whitlock, Fri Nov 19 15:56:34 PST 2010
//   I separated code into the new GetOperatorCreatedExpressions method.
//
//   Hank Childs, Sat Nov 27 16:46:07 PST 2010
//   Make GetOperatorCreatedExpressions be a static method, so the viewer can
//   make use of it any time, even if there is no VariableMenuPopulator.
//
//   Mark C. Miller, Wed Jun  5 13:48:56 PDT 2019
//   Change use of maps for variable lists to multimaps using case-insensitive
//   comparator function.
//
//   Kathleen Biagas, Fri Oct 23, 2020
//   Moved Comparator and typedefs into maptypes.h, so it they could be passed
//   as args to StringHelpers functions.
//
// ****************************************************************************

class WINUTIL_API VariableMenuPopulator
{
public:
    VariableMenuPopulator();
    virtual ~VariableMenuPopulator();

    bool PopulateVariableLists(const std::string &,
                               const avtDatabaseMetaData *,
                               const avtSIL *,
                               const ExpressionList *,
                               OperatorPluginManager *,
                               bool);

    bool IsSingleVariableMenuUpToDate(int varTypes, QvisVariablePopupMenu *menu,
             avtDatabaseMetaData const *md=0) const;

    int UpdateSingleVariableMenu(QvisVariablePopupMenu *menu,
                                 int varTypes,
                                 QObject *receiver = 0,
                                 const char *slot = 0);

    enum OpCreatedExprMode { GlobalAndNew, GlobalOnly };
    static void GetOperatorCreatedExpressions(ExpressionList &newExpressionList,
                    const avtDatabaseMetaData *md, OperatorPluginManager *oPm,
                    OpCreatedExprMode exprMode = GlobalAndNew);

    bool ItemEnabled(int varType) const;
    void ClearCachedInfo();

    bool CanSkipPopulateVariableLists(std::string const &dbKey,
        ExpressionList const *exprList, bool treatAllDBsAsTV,
        bool mustRePopMD, bool isSim);

private:

    class VariableList
    {
    public:
        VariableList();
        VariableList(const VariableList &);
        virtual ~VariableList();
        void SetSorted(bool val) { sorted = val; };
        bool GetSorted() const { return sorted; };
        void AddVariable(const std::string &var, bool validVar);
        void Clear();
        int  Size() const;
        bool Contains(const std::string &var) const;
        void InitTraversal();
        bool GetNextVariable(std::string &var, bool &validVar);
        bool IsGroupingRequired(CIStringStringMap& origNameToGroupedName);
        bool operator == (const VariableList &) const;
        bool operator != (const VariableList &) const;
        unsigned int GetHashVal() const { return myHashVal; };
        void SetHashVal(unsigned int hv) { myHashVal = hv; };
    private:
        bool                      sorted;
        CIStringBoolMap           sortedVariables;
        CIStringBoolMap::iterator sortedVariablesIterator;
        stringVector              unsortedVariableNames;
        boolVector                unsortedVariableValid;
        int                       unsortedVariableIndex;
        unsigned int              myHashVal;
    };

    class GroupingInfo
    {
    public:
        GroupingInfo();
        GroupingInfo(const GroupingInfo &obj);
       ~GroupingInfo();
        void operator = (const GroupingInfo &obj);

        CIStringStringMap grouping;
        bool            required;
    };

    typedef std::map<int, GroupingInfo *> IntGroupingInfoMap;

    void UpdateSingleMenu(QvisVariablePopupMenu *, VariableList &vars,
                          QObject *, const char *slot, const GroupingInfo *);
    bool AddVars(VariableList &to, VariableList &from);
    void AddExpression(const Expression &);
    void GetRelevantExpressions(ExpressionList &newExpressionList,
                                const avtDatabaseMetaData *md,
                                const ExpressionList &exprList);
    void ClearGroupingInfo();

    // Cached info to track changes to avoid re-comutes
    std::string    cachedDBKey;
    ExpressionList cachedExprList;
    ExpressionList cachedExpressionList;
    
    // Create some lists to keep track of the variable names.
    VariableList   meshVars, scalarVars, materialVars, vectorVars, subsetVars,
                   speciesVars, curveVars, tensorVars, symmTensorVars,
                   labelVars, arrayVars;
    IntGroupingInfoMap groupingInfo;
};

#endif
