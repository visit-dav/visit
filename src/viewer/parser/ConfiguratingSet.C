#include "ConfiguratingSet.h"

using     std::map;
using     std::vector;

// ****************************************************************************
//  Constructor:  ConfiguratingSet::ConfiguratingSet
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
ConfiguratingSet::ConfiguratingSet()
{
    conflict = false;
}

// ****************************************************************************
//  Equality operator:  ConfiguratingSet::operator==
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
bool
ConfiguratingSet::operator==(const ConfiguratingSet &rhs) const
{
    if (items.size() != rhs.items.size())
        return false;
    int n1 = items.size();
    int n2 = rhs.items.size();
    for (int i=0; i<n1; i++)
    {
        bool found = false;
        for (int j=0; j<n2; j++)
        {
            if (items[i] == rhs.items[j])
            {
                found = true;
                break;
            }
        }
        if (!found)
            return false;
    }
    return true;
}

// ****************************************************************************
//  Method:  ConfiguratingSet::Merge
//
//  Purpose:
//    Add items in another ConfiguratingSet to this one, merging when needed.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
void
ConfiguratingSet::Merge(const ConfiguratingSet &cs)
{
    for (int i=0; i<cs.items.size(); i++)
    {
        AddItem(cs.items[i]);
    }
}

// ****************************************************************************
//  Method:  ConfiguratingSet::Close
//
//  Purpose:
//    Close all rules in the set.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
void
ConfiguratingSet::Close(const vector<const Rule*> &rules)
{
    for (int i=0; i<items.size(); i++)
    {
        Merge(items[i].CreateClosure(rules));
    }
}

// ****************************************************************************
//  Method:  ConfiguratingSet::AddItem
//
//  Purpose:
//    Add an item to this set.  Merges with an existing one if it can.
//
//  Arguments:
//    ci         the item to add
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
void
ConfiguratingSet::AddItem(const ConfiguratingItem &ci)
{
    int n = items.size();
    for (int i=0; i<n; i++)
    {
        if (items[i].CanMerge(ci))
        {
            items[i].Merge(ci);
            return;
        }
    }
    items.push_back(ci);
}

// ****************************************************************************
//  Method:  ConfiguratingSet::GetShiftSymbols
//
//  Purpose:
//    Get the set of symbols which can be shifted in this set.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
SymbolSet
ConfiguratingSet::GetShiftSymbols()
{
    SymbolSet shiftsymbols;
    for (int i=0; i<items.size(); i++)
    {
        if (! items[i].CanReduce())
            shiftsymbols.insert(items[i].GetNextSymbol());
    }
    return shiftsymbols;
}

// ****************************************************************************
//  Method:  ConfiguratingSet::GetReduceSymbols
//
//  Purpose:
//    Get the set of symbols which will cause a reduction.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
SymbolSet
ConfiguratingSet::GetReduceSymbols()
{
    SymbolSet reducesymbols;
    for (int i=0; i<items.size(); i++)
    {
        if (items[i].CanReduce())
        {
            reducesymbols.merge(items[i].GetFollow());
        }
    }
    return reducesymbols;
}

// ****************************************************************************
//  Method:  ConfiguratingSet::GetShiftSet
//
//  Purpose:
//    Gets the ConfiguratingSet created by shifting a symbol.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
ConfiguratingSet
ConfiguratingSet::GetShiftSet(const Symbol *s)
{
    ConfiguratingSet cs;
    for (int i=0; i<items.size(); i++)
    {
        if ((!items[i].CanReduce()) &&
            (items[i].GetNextSymbol() == s))
        {
            ConfiguratingItem ci = items[i];
            ci.Shift();
            cs.AddItem(ci);
        }
    }
    return cs;
}

// ****************************************************************************
//  Method:  ConfiguratingSet::GetReduceRules
//
//  Purpose:
//    Gets the list of rules which can fire by seeing a symbol.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
vector<int>
ConfiguratingSet::GetReduceRules(const Symbol *s)
{
    vector<int> rr;
    for (int i=0; i<items.size(); i++)
    {
        if (items[i].CanReduce() && items[i].GetFollow().contains(s))
            rr.push_back(items[i].GetRule()->GetIndex());
    }
    return rr;
}

// ****************************************************************************
//  Method:  operator<<(ostream, ConfiguratingSet)
//
//  Purpose:
//    Print the ConfiguratingSet.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Brad Whitlock, Fri Jun 28 14:58:30 PST 2002
//    Made it work on Windows.
//
// ****************************************************************************

ostream &
operator<<(ostream &o, const ConfiguratingSet &cs)
{
    for (int i=0; i<cs.items.size(); i++)
    {
        o << "\t";
        //o << "(#"<<cs.items[i].GetRule().GetIndex()<<") ";
        o << cs.items[i] << endl;
    }
    o << endl;

    std::map<const Symbol*,int>::const_iterator it;
    for (it = cs.shift.begin(); it != cs.shift.end(); it++)
    {
        o << "    On '" << *(it->first) << "', ";
        if (it->first->IsTerminal()) o << "shift and ";
        o << "go to state " << it->second << endl;
    }
    for (it = cs.reduce.begin(); it != cs.reduce.end(); it++)
    {
        o << "    On '" << *(it->first) << "', ";
        o << "reduce using rule " << it->second << endl;
    }
    return o;
}

