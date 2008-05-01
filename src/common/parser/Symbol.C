/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include "Symbol.h"
#include "Dictionary.h"
#include "Rule.h"
#include "Token.h"
using std::vector;
using std::string;
using std::map;

// ****************************************************************************
//  Constructor:  Symbol::Symbol
//
//  Purpose:
//    Creates a terminal from the token type.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Jeremy Meredith, Wed Jun  8 17:08:35 PDT 2005
//    All symbols are now added to a dictionary instead of static members.
//
// ****************************************************************************
Symbol::Symbol(Dictionary &dict, int tt)
    : type(Terminal), terminaltype(tt), displaystring(GetTokenTypeString(tt))
{
    index = dict.AddTerminal(this);
}

// ****************************************************************************
//  Constructor:  Symbol::Symbol
//
//  Purpose:
//    Creates a terminal from the token type, overriding the display
//    string with a known one.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 24, 2004
//
//  Modifications:
//    Jeremy Meredith, Wed Jun  8 17:08:35 PDT 2005
//    All symbols are now added to a dictionary instead of static members.
//
// ****************************************************************************
Symbol::Symbol(Dictionary &dict, int tt, const string &s)
    : type(Terminal), terminaltype(tt), displaystring(s)
{
    index = dict.AddTerminal(this);
}

// ****************************************************************************
//  Constructor:  Symbol::Symbol
//
//  Purpose:
//    Creates a nonterminal from its name.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Jeremy Meredith, Wed Jun  8 17:08:35 PDT 2005
//    All symbols are now added to a dictionary instead of static members.
//
// ****************************************************************************
Symbol::Symbol(Dictionary &dict, const string &s)
    : type(NonTerminal), terminaltype(0), displaystring(s)
{
    index = dict.AddNonTerminal(this);
}

// ****************************************************************************
//  Method:  Symbol::operator==
//
//  Purpose:
//    Tests for equality.  The displaystrings should always be unique.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
bool
Symbol::operator==(const Symbol &rhs) const
{
    return displaystring == rhs.displaystring;
}

// ****************************************************************************
//  Method:  operator<<(ostream, Symbol)
//
//  Purpose:
//    Prints the symbol.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
ostream &operator<<(ostream &o, const Symbol &s)
{
    o << s.displaystring.c_str();
    return o;
}

// ****************************************************************************
//  Method:  Symbol::IsNullable
//
//  Purpose:
//    Checks if a symbol can produce, even indirectly, the null string.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
bool
Symbol::IsNullable(const vector<const Rule*> &rules) const
{
    if (type == Terminal)
        return false;

    for (size_t i=0; i<rules.size(); i++)
    {
        if (rules[i]->GetLHS() == this)
        {
            if (rules[i]->IsNullable(rules))
                return true;
        }
    }
    return false;
}

// ****************************************************************************
//  Method:  Symbol::GetFirstSet
//
//  Purpose:
//    Gets the first set for any production of this symbol.
//    For terminals, the first set is the identity operation.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
SymbolSet
Symbol::GetFirstSet(const vector<const Rule*> &rules) const
{
    SymbolSet first;
    if (type == Terminal)
    {
        first.insert(this);
    }
    else
    {
        for (size_t i=0; i<rules.size(); i++)
        {
            if (rules[i]->GetLHS() == this &&
                // Try to avoid infinite recursion -- this should be improved!
                (rules[i]->GetRHS().Empty() || rules[i]->GetRHS()[0] != this))
            {
                first.merge( rules[i]->GetRHS().GetFirstSet(rules) );
            }
        }
    }
    return first;
}
