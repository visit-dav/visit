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
//  Modifications:
//    Jeremy Meredith, Wed Aug  6 15:56:21 EDT 2008
//    Handle symbol printing better.
//
// ****************************************************************************
void
Sequence::Print(ostream &o, int pos) const
{
    if (symbols.empty())
    {
        o << "e ";
        if (pos == 0)
            o << dot.c_str() << " ";
    }
    else
    {
        for (size_t i=0; i<symbols.size(); i++)
        {
            if ((size_t)pos == i) o << dot.c_str() << " ";
            {
                int tt = symbols[i]->GetTerminalType();
                if (symbols[i]->IsNonTerminal())
                    o << *(symbols[i]) << " ";
                else if (tt == '\\')
                    o << "(backslash) ";
                else if (tt < 256)
                    o << char(tt) << " ";
                else
                    o << *(symbols[i]) << " ";
            }
        }
        if ((size_t)pos == symbols.size()) o << dot.c_str() << " ";
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
 
    for (size_t i=0; i<symbols.size(); i++)
        if (symbols[i]->IsTerminal())
            return false;

    bool nullable = true;
    for (size_t i=0; i<symbols.size() && nullable; i++)
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
    for (size_t i=0; i<symbols.size(); i++)
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
    for (size_t i=pos; i<symbols.size(); i++)
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
    for (size_t i=0; i<symbols.size(); i++)
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

