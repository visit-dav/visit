#include "ConfiguratingItem.h"
#include "ConfiguratingSet.h"
#include "Colors.h"

using     std::vector;

// ****************************************************************************
//  Copy Constructor:  ConfiguratingItem::ConfiguratingItem
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
ConfiguratingItem::ConfiguratingItem(const ConfiguratingItem &ci)
{
    rule=ci.rule;
    follow=ci.follow;
    pos=ci.pos;
}

// ****************************************************************************
//  Constructor:  ConfiguratingItem::ConfiguratingItem
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
ConfiguratingItem::ConfiguratingItem(const Rule *r, const Symbol &f)
{
    rule = r;
    follow.insert(&f);
    pos = 0;
}

// ****************************************************************************
//  Constructor:  ConfiguratingItem::ConfiguratingItem
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
ConfiguratingItem::ConfiguratingItem(const Rule *r, const SymbolSet &f)
{
    rule = r;
    follow = f;
    pos = 0;
}

// ****************************************************************************
//  Method:  ConfiguratingItem::CreateClosure
//
//  Purpose:
//    Get the list of configurating items in the closure of the current item.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
ConfiguratingSet
ConfiguratingItem::CreateClosure(const vector<const Rule*> &rules)
{
    ConfiguratingSet closure;
    Sequence s = rule->GetRHS().GetSubsequence(pos);
    if (!s.Empty() && s[0]->IsNonTerminal())
    {
        Sequence remaining = s.GetSubsequence(1);
        SymbolSet closurefollow = remaining.GetFirstSet(rules);
        if (remaining.IsNullable(rules))
        {
            closurefollow.merge(follow);
        }
        for (int i=0; i<rules.size(); i++)
        {
            if (s[0] == rules[i]->GetLHS())
            {
                closure.AddItem(ConfiguratingItem(rules[i], closurefollow));
            }
        }
    }
    return closure;
}

// ****************************************************************************
//  Method:  ConfiguratingItem::CanMerge
//
//  Purpose:
//    True if another item can merge with this one.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
bool
ConfiguratingItem::CanMerge(const ConfiguratingItem &rhs)
{
    return pos == rhs.pos && rule == rhs.rule;
}

// ****************************************************************************
//  Method:  ConfiguratingItem::Merge
//
//  Purpose:
//    Merge another item into this one.  Since we know the rule and
//    pos must be the same, we only merge the follow sets.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
void
ConfiguratingItem::Merge(const ConfiguratingItem &rhs)
{
    follow.merge(rhs.follow);
}

// ****************************************************************************
//  Method:  operator<<(ostream, ConfiguratingItem)
//
//  Purpose:
//    Print the ConfiguratingItem.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
ostream &operator<<(ostream &o, const ConfiguratingItem &ci)
{
    ci.rule->Print(o, ci.pos);
    o << "    \tfollow={ " << TermGreen;
    for (int i = 0; i<ci.follow.size(); i++)
    {
        if (ci.follow.set[i])
            o << *(ci.follow.set[i]) << " ";
    }
    o << TermReset << "}";

    return o;
}

// ****************************************************************************
//  Equality operator:  ConfiguratingItem::operator==
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
bool
ConfiguratingItem::operator==(const ConfiguratingItem &rhs) const
{
    return (pos == rhs.pos) && (rule == rhs.rule) && (follow == rhs.follow);
}

