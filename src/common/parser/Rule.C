#include "Rule.h"
#include "Colors.h"

using     std::vector;

// ****************************************************************************
//  Default Constructor:  Rule::Rule
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
Rule::Rule()
{
    lhs = NULL;
    index = -1;
    prec = -1;
    id = -1;
}

// ****************************************************************************
//  Constructor:  Rule::Rule
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
Rule::Rule(int i, const Symbol &nt)
{
    lhs = &nt;
    index = -1;
    prec = -1;
    id = i;
}

// ****************************************************************************
//  Method:  Rule::operator>>
//
//  Purpose:
//    Sets the right hand side of the rule.  Makes it look like a production.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
Rule&
Rule::operator>>(const Sequence &s)
{
    rhs = s;
    return *this;
}

// ****************************************************************************
//  Method:  Rule::IsNullable
//
//  Purpose:
//    True if the RHS sequence is nullable.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
bool
Rule::IsNullable(const vector<const Rule*> &rules) const
{
    return rhs.IsNullable(rules);
}

// ****************************************************************************
//  Method:  operator<<(ostream, Rule)
//
//  Purpose:
//    Prints the Rule.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
ostream &operator<<(ostream &o, const Rule &r)
{
    r.Print(o);
    return o;
}

// ****************************************************************************
//  Method:  Rule::Print
//
//  Purpose:
//    Prints the Rule.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
void
Rule::Print(ostream &o, int pos) const
{
    o << *lhs << TermBold << TermCyan << " ==> " << TermReset;
    rhs.Print(o, pos);
}

// ****************************************************************************
//  Method:  Rule::PrintNoColor
//
//  Purpose:
//    Prints the Rule without highliting.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 11, 2002
//
// ****************************************************************************
void
Rule::PrintNoColor(ostream &o, int pos) const
{
    o << *lhs << " ==> ";
    rhs.Print(o, pos);
}

