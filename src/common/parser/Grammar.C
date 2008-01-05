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

#include "Grammar.h"
#include "Colors.h"
#include "Token.h"
#include <stdio.h>
#include <set>

using     std::map;
using     std::set;
using     std::string;
using     std::vector;

// ****************************************************************************
//  Constructor:  Grammar::Grammar
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Jeremy Meredith, Wed Jun  8 17:05:56 PDT 2005
//    Added a symbol dictionary.
//
// ****************************************************************************
Grammar::Grammar(Dictionary &d)
    : dictionary(d),
      eof(dictionary, EOF_TOKEN_ID),  // TT_EOF==256 for all grammars
      start(dictionary, "START")
{
    out = NULL;
    rules.push_back(NULL);
}

// ****************************************************************************
//  Destructor:  Grammar::Grammar
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Jeremy Meredith, Mon Jun 13 15:52:32 PDT 2005
//    Free the rules.
//
// ****************************************************************************
Grammar::~Grammar()
{
    for (int i=1; i<rules.size(); i++)
        delete rules[i];
}

// ****************************************************************************
//  Method:  Grammar::SetPrinter
//
//  Purpose:
//    Sets the ostream where we will print debug output.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
void
Grammar::SetPrinter(ostream *o)
{
    out = o;
}

// ****************************************************************************
//  Method:  Grammar::Print
//
//  Purpose:
//    Print the rules and states.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
void
Grammar::Print(ostream &o)
{
    int i;
    o << endl << TermBold << TermBrown << "- - - ------------- rules ------------- - - -\n" << TermReset;
    for (i=0; i<rules.size(); i++)
        o << *(rules[i]) << endl;
    o << endl << TermBold << TermBrown << "- - - ------------- states ------------- - - -\n" << TermReset;
    for (i=0; i<sets.size(); i++)
    {
        if (sets[i].HasConflict())   o << TermRed;
        else                         o << TermBold << TermBlue;
        o << TermUnderline << "<< State #"<<i<<" >>" << TermReset << endl << sets[i] << endl;
    }
}

// ****************************************************************************
//  Method:  Grammar::SetAssoc
//
//  Purpose:
//    Sets the associativity of a symbol.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
void
Grammar::SetAssoc(const Symbol &s, Associativity a)
{
    assoc[&s] = a;
}

// ****************************************************************************
//  Method:  Grammar::SetPrec
//
//  Purpose:
//    Sets the precedence of a symbol.  ( higher number = higher precedence )
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
void
Grammar::SetPrec(const Symbol &s,  int p)
{
    prec[&s] = p;
}

// ****************************************************************************
//  Method:  Grammar::AddRule
//
//  Purpose:
//    Add a rule to the list, possibly assigning a precision.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
void
Grammar::AddRule(const Rule &r_, int prec)
{
    Rule *r = new Rule(r_);
    r->SetIndex(rules.size());
    if (prec != -1)
        r->SetPrec(prec);
    rules.push_back(r);
}

// ****************************************************************************
//  Method:  Grammar::SetStartSymbol
//
//  Purpose:
//    Sets the start symbol.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
void
Grammar::SetStartSymbol(const Symbol &ss)
{
    startrule = Rule(0, start) >> ss;
    startrule.SetIndex(0);
    rules[0] = &startrule;
}

// ****************************************************************************
//  Method:  Grammar::GetStartSymbol
//
//  Purpose:
//    Gets the start symbol.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
const Symbol*
Grammar::GetStartSymbol()
{
    return &start;
}

// ****************************************************************************
//  Method:  Grammar::GetState
//
//  Purpose:
//    Returns the "i"th state.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
State&
Grammar::GetState(int i)
{
    return states[i];
}

// ****************************************************************************
//  Method:  Grammar::GetRule
//
//  Purpose:
//    Returns the "i"th rule.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
const Rule*
Grammar::GetRule(int i)
{
    return rules[i];
}

// ****************************************************************************
//  Method:  Grammar::GetDictionary
//
//  Purpose:
//    Returns the symbol dictionary.
//
//  Programmer:  Jeremy Meredith
//  Creation:    June  8, 2005
//
// ****************************************************************************
Dictionary&
Grammar::GetDictionary()
{
    return dictionary;
}

// ****************************************************************************
//  Method:  Grammar::Configure
//
//  Purpose:
//    Makes all configurating sets/states to be used by the parser.
//    Resolves conflicts using precedence and associativity.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
bool
Grammar::Configure()
{
    int okay = true;

    ConfiguratingSet start;
    start.AddItem(ConfiguratingItem(rules[0], eof));
    start.Close(rules);
    sets.push_back(start);

    // For every state in the grammar
    for (int j=0; j<sets.size(); j++)
    {
        // Add new states to the grammar resulting from shifting a symbol
        // in the current state
        int i;
        SymbolSet shift = sets[j].GetShiftSymbols();
        for (i=0; i<shift.size(); i++)
        {
            const Symbol *ssym = shift.set[i];
            if (!ssym)
                continue;

            ConfiguratingSet cs = sets[j].GetShiftSet(ssym);
            cs.Close(rules);

            int match = sets.size();
            for (int k=0; k<sets.size(); k++)
            {
                if (cs == sets[k])
                {
                    match = k;
                    break;
                }
            }
            if (match == sets.size())
            {
                sets.push_back(cs);
            }
            sets[j].SetShiftTransition(ssym, match);
        }

        // Add the reduce rules for the current state, looking for 
        // and reduce-reduce conflicts
        SymbolSet reduce = sets[j].GetReduceSymbols();
        for (i=0; i<reduce.size(); i++)
        {
            const Symbol *rsym = reduce.set[i];
            if (!rsym)
                continue;

            vector<int> reducerules = sets[j].GetReduceRules(rsym);
            if (reducerules.size() < 1) (*out) << TermRed << "Internal error! 1\n";
            if (reducerules.size() > 1)
            {
                // We could probably resolve these using rule precedence, but
                // I never bothered since it never came up in our grammar
                (*out) << TermRed << "Reduce-reduce conflict in state " << j
                     << " for symbol " << *rsym << ":" << endl;
                for (int k=0; k<reducerules.size(); k++)
                {
                    (*out) << TermRed << "\tRule "<<reducerules[k]<<": " << TermReset;
                    rules[reducerules[k]]->Print((*out));
                    (*out) << endl;
                }
                (*out) << TermReset;
                sets[j].SetConflict(true);
                okay = false;
            }
            for (int k=0; k<reducerules.size(); k++)
                sets[j].SetReduceRule(rsym, reducerules[k]);
        }

        // Look for any shift-reduce conflicts
        for (i=0; i<shift.size(); i++)
        {
            const Symbol *ssym = shift.set[i];
            if (!ssym || !reduce.set[i])
                continue;

            const Rule *reducerule = rules[sets[j].GetReduceRules(ssym)[0]];
            const Symbol *rsym     = reducerule->GetRHS().GetLastTerminal();
            bool   same = (ssym == rsym);

            int sprec = prec.count(ssym) ? prec[ssym] : -1;
            int rprec = reducerule->GetPrec();
            if (rprec == -1)
                rprec = prec.count(rsym) ? prec[rsym] : -1;

            bool haveprec  = (sprec >= 0 && rprec >= 0);
            bool haveassoc = assoc.count(ssym);

            // Try to resolve using precedence or associativity
            if (!same && haveprec && sprec!=rprec)
            {
                if (sprec < rprec)
                    sets[j].RemoveShiftTransition(ssym);
                else
                    sets[j].RemoveReduceRule(ssym);
                (*out) << TermGreen << "SR conflict in state " << j << " for " << *ssym << " and " << *rsym << " resolved by precedence" << TermReset << endl;
            }
            else if (haveassoc && (same || (haveprec && sprec==rprec)))
            {
                if (assoc[ssym] != Right)
                    sets[j].RemoveShiftTransition(ssym);
                if (assoc[ssym] != Left)
                    sets[j].RemoveReduceRule(ssym);
                (*out) << TermGreen << "SR conflict in state " << j  << " for " << *ssym << " and " << *rsym << " resolved by " << (assoc[ssym] == Left ? "left" : "right") << " assoc "<< TermReset << endl;
            }
            else
            {
                // failed to resolve... set the error flag
                sets[j].SetConflict(true);
                okay = false;
                (*out) << TermRed << "SR conflict in state " << j << " for " << *ssym << " and " << *rsym << TermReset << endl;
            }
        }
    }

    // Print all rules and states if needed
    if (out)
        Print(*out);

    // Create the states directly from the configurating sets
    if (okay)
    {
        for (int i=0; i<sets.size(); i++)
            states.push_back(State(sets[i]));
    }

    return okay;
}

// ****************************************************************************
//  Method:  Grammar::WriteStateInitialization
//
//  Purpose:
//    Write out C++ code to initialize the grammar states without
//    having to go through the slower configuration.  This allows
//    the yacc/bison mode of operation.
//
//  Arguments:
//    name       Grammar derived class name
//    o          output file derived class name
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Jeremy Meredith, Thu Apr 11 12:56:46 PDT 2002
//    Cleaned up output -- made it hundreds of 10-line functions instead of
//    one function with thousands of lines.  Also added some comments.
//
//    Brad Whitlock, Fri Jun 28 15:12:49 PST 2002
//    Made it work on Windows.
//
//    Jeremy Meredith, Wed Jun  8 17:06:40 PDT 2005
//    Added a symbol dictionary to remove static data.
//
//    Jeremy Meredith, Tue Jun  5 13:29:33 EDT 2007
//    Added copyright notice.  Added parentheses to function definition.
//
// ****************************************************************************

void
Grammar::WriteStateInitialization(const string &name, ostream &o)
{
    int i;

    o << "/*****************************************************************************" << endl;
    o << "*" << endl;
    o << "* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC" << endl;
    o << "* Produced at the Lawrence Livermore National Laboratory" << endl;
    o << "* LLNL-CODE-400142" << endl;
    o << "* All rights reserved." << endl;
    o << "*" << endl;
    o << "* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The" << endl;
    o << "* full copyright notice is contained in the file COPYRIGHT located at the root" << endl;
    o << "* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html." << endl;
    o << "*" << endl;
    o << "* Redistribution  and  use  in  source  and  binary  forms,  with  or  without" << endl;
    o << "* modification, are permitted provided that the following conditions are met:" << endl;
    o << "*" << endl;
    o << "*  - Redistributions of  source code must  retain the above  copyright notice," << endl;
    o << "*    this list of conditions and the disclaimer below." << endl;
    o << "*  - Redistributions in binary form must reproduce the above copyright notice," << endl;
    o << "*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the" << endl;
    o << "*    documentation and/or other materials provided with the distribution." << endl;
    o << "*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may" << endl;
    o << "*    be used to endorse or promote products derived from this software without" << endl;
    o << "*    specific prior written permission." << endl;
    o << "*" << endl;
    o << "* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS \"AS IS\"" << endl;
    o << "* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE" << endl;
    o << "* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE" << endl;
    o << "* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY," << endl;
    o << "* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY" << endl;
    o << "* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL" << endl;
    o << "* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR" << endl;
    o << "* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER" << endl;
    o << "* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT" << endl;
    o << "* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY" << endl;
    o << "* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH" << endl;
    o << "* DAMAGE." << endl;
    o << "*" << endl;
    o << "*****************************************************************************/" << endl;
    o << "" << endl;

    o << "//" << endl;
    o << "//   Automatically generated!!!  Use 'make init' to rebuild." << endl;
    o << "//" << endl;
    o << endl;
    o << "#include \""<<name.c_str()<<".h\"" << endl;
    o << endl;
    o << "" << endl;
    o << "static void SetShiftState(State &s, int next, const Symbol *sym)" << endl;
    o << "{" << endl;
    o << "    s.shift[sym] = next;" << endl;
    o << "}" << endl;
    o << "" << endl;
    o << "static void SetReduceRule(State &s, int rule, const Symbol *sym)" << endl;
    o << "{" << endl;
    o << "    s.reduce[sym] = rule;" << endl;
    o << "}" << endl;
    o << "" << endl;

    for (i=0; i<states.size(); i++)
    {
        o << "static void InitState_"<<i<<"(Dictionary &d, State &s)" << endl;
        o << "{" << endl;

        // Write out the shift state symbol map
        if (!states[i].shift.empty())
            o << "    // shift transitions" << endl;
        std::map<const Symbol*,int>::iterator j;
        for (j = states[i].shift.begin();
             j != states[i].shift.end();
             j++)
        {
            const Symbol *s = j->first;
            char str[256];
            sprintf(str, "%4d", j->second);

            int tt = s->GetTerminalType();

            o << "    SetShiftState(s,"<<str<<", d.Get(";
            if (s->IsNonTerminal())
                o << "\"" << s->GetDisplayString().c_str() << "\"";
            else
                if (tt < 256)
                    o << "'" << char(tt) << "'";
                else
                    o << tt;
            o << "));" << endl;
        }

        if (!states[i].shift.empty() && !states[i].reduce.empty())
            o << endl;

        // Write out the reduce rule symbol map
        set<int> reductions;
        for (j = states[i].reduce.begin();
             j != states[i].reduce.end();
             j++)
        {
            reductions.insert(j->second);
        }
        std::set<int>::iterator k;
        for (k = reductions.begin();
             k != reductions.end();
             k++)
        {
            o << "    // reduce rule " << *k << ": ";
            GetRule(*k)->PrintNoColor(o);
            o << endl;
        }
        for (j = states[i].reduce.begin();
             j != states[i].reduce.end();
             j++)
        {
            const Symbol *s = j->first;
            char str[256];
            sprintf(str, "%4d", j->second);

            int tt = s->GetTerminalType();

            o << "    SetReduceRule(s,"<<str<<", d.Get(";
            if (s->IsNonTerminal())
                o << "\"" << s->GetDisplayString().c_str() << "\"";
            else
                if (tt < 256)
                    o << "'" << char(tt) << "'";
                else
                    o << tt;
            o << "));" << endl;
        }

        o << "}" << endl;
        o << endl;
    }
    o << "bool " << name.c_str() << "::Initialize()" << endl;
    o << "{" << endl;
    o << "    states.resize(" << states.size() << ");" << endl;
    o << endl;
    for (i=0; i<states.size(); i++)
    {
        o << "    InitState_"<<i<<"(dictionary, states[" << i << "]);" << endl;
    }
    o << endl;
    o << "    return true;" << endl;
    o << "}" << endl;
    o << endl;
}
