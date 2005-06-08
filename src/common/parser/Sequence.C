#include "Sequence.h"
#include "Colors.h"

using     std::vector;

//static const char dotchar = 183;
static const char dotchar = 'o';
static std::string dot = std::string() + TermBold + TermGreen + dotchar + TermReset;


// ****************************************************************************
//  Constructor:  Sequence::Sequence
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
Sequence::Sequence()
{
    len = 0;
}

// ****************************************************************************
//  Copy Constructor:  Sequence::Sequence
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
Sequence::Sequence(const Sequence &s)
{
    len = s.len;
    symbols = s.symbols;
}

// ****************************************************************************
//  Constructor:  Sequence::Sequence
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
Sequence::Sequence(const Symbol &s)
{
    len = 1;
    symbols.push_back(&s);
}

// ****************************************************************************
//  Method:  Sequence::AddSymbol
//
//  Purpose:
//    Append a symbol to this sequence.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
void
Sequence::AddSymbol(const Symbol &s)
{
    len++;
    symbols.push_back(&s);
}

// ****************************************************************************
//  Method:  Sequence::Print
//
//  Purpose:
//    Print the sequence.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
void
Sequence::Print(ostream &o, int pos) const
{
    if (symbols.empty())
    {
        o << "e ";
        if (pos == 0)
            o << dot << " ";
    }
    else
    {
        for (int i=0; i<symbols.size(); i++)
        {
            if (pos == i) o << dot << " ";
                o << *(symbols[i]) << " ";
        }
        if (pos == symbols.size()) o << dot << " ";
    }
}

// ****************************************************************************
//  Method:  Sequence::IsNullable
//
//  Purpose:
//    Returns true if this sequence can produce the empty set.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
bool
Sequence::IsNullable(const vector<const Rule*> &rules) const
{
    if (symbols.empty())
        return true;
 
    int i;
    for (i=0; i<symbols.size(); i++)
        if (symbols[i]->IsTerminal())
            return false;

    bool nullable = true;
    for (i=0; i<symbols.size() && nullable; i++)
    {
        nullable = symbols[i]->IsNullable(rules);
    }
    return nullable;
}

// ****************************************************************************
//  Method:  Sequence::GetFirstSet
//
//  Purpose:
//    Get the first set of this sequence.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
SymbolSet
Sequence::GetFirstSet(const vector<const Rule*> &rules) const
{
    SymbolSet first;
    int i;
    for (i=0; i<symbols.size(); i++)
    {
        if (symbols[i]->IsTerminal())
        {
            first.insert(symbols[i]);
            break;
        }
        first.merge( symbols[i]->GetFirstSet(rules) );
        if (! symbols[i]->IsNullable(rules))
            break;
    }
    return first;
}

// ****************************************************************************
//  Method:  operator+(Symbol, Symbol)
//
//  Purpose:
//    Creates a new sequence by joining two symbols.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
Sequence
operator+(const Symbol &l, const Symbol &r)
{
    Sequence s;
    s.AddSymbol(l);
    s.AddSymbol(r);
    return s;
}

// ****************************************************************************
//  Method:  operator+(Sequence, Symbol)
//
//  Purpose:
//    Creates a new sequence by appending a symbol.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
Sequence
operator+(const Sequence &l, const Symbol &r)
{
    Sequence s(l);
    s.AddSymbol(r);
    return s;
}

// ****************************************************************************
//  Method:  Sequence::GetSubsequence
//
//  Purpose:
//    Gets the right part of a sequence starting with the given symbol.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
Sequence
Sequence::GetSubsequence(int pos) const
{
    Sequence s;
    for (int i=pos; i<symbols.size(); i++)
        s.AddSymbol(*symbols[i]);
    return s;
}

// ****************************************************************************
//  Method:  Sequence::GetLastTerminal
//
//  Purpose:
//    Gets the rightmost terminal of the sequence if there is one.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
const Symbol*
Sequence::GetLastTerminal() const
{
    const Symbol *nt = NULL;
    for (int i=0; i<symbols.size(); i++)
        if (symbols[i]->IsTerminal())
            nt = symbols[i];
    return nt;
}

// ****************************************************************************
//  Equality operator:  Sequence::operator==
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
bool
Sequence::operator==(const Sequence &s) const
{
    if (len != s.len)
        return false;
    for (int i=0; i<len; i++)
        if (symbols[i] != s.symbols[i])
            return false;
    return true;
}

