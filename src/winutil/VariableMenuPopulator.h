#ifndef VARIABLE_MENU_POPULATOR_H
#define VARIABLE_MENU_POPULATOR_H
#include <winutil_exports.h>
#include <ExpressionList.h>
#include <string>
#include <map>
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
// ****************************************************************************

class WINUTIL_API VariableMenuPopulator
{
public:
    VariableMenuPopulator();
    virtual ~VariableMenuPopulator();

    bool PopulateVariableLists(const std::string &,
                               const avtDatabaseMetaData *,
                               const avtSIL *,
                               const ExpressionList *);

    int UpdateSingleVariableMenu(QvisVariablePopupMenu *menu,
                                 int varTypes,
                                 QObject *receiver = 0,
                                 const char *slot = 0);

    bool ItemEnabled(int varType) const;
    void ClearDatabaseName();

private:
    typedef std::map<std::string, bool> StringBoolMap;

    class VariableList
    {
    public:
        VariableList();
        virtual ~VariableList();
        void SetSorted(bool val) { sorted = val; }
        bool GetSorted() const { return sorted; }
        void AddVariable(const std::string &var, bool validVar);
        void Clear();
        int  Size() const;
        bool Contains(const std::string &var) const;
        void InitTraversal();
        bool GetNextVariable(std::string &var, bool &validVar);
    private:
        bool                    sorted;
        StringBoolMap           sortedVariables;
        StringBoolMap::iterator sortedVariablesIterator;
        stringVector            unsortedVariableNames;
        boolVector              unsortedVariableValid;
        int                     unsortedVariableIndex;
    };

    void UpdateSingleMenu(QvisVariablePopupMenu *, VariableList &vars,
                          QObject *, const char *slot);
    void Split(const std::string &varName, stringVector &pieces) const;
    void AddVars(VariableList &to, VariableList &from);
    void AddExpression(const Expression &);

    // Keep track of the name of the database for which we have variables.
    std::string    cachedDBName;
    // Keep track of the expression list too.
    ExpressionList cachedExpressionList;
    // Create some lists to keep track of the variable names.
    VariableList   meshVars, scalarVars, materialVars, vectorVars, subsetVars,
                   speciesVars, curveVars, tensorVars, symmTensorVars;
};

#endif
