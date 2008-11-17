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

#include <VisItParser.h>
#include <Token.h>
#include <SymbolSet.h>
#include <Sequence.h>
#include <Rule.h>
#include <ConfiguratingItem.h>
#include <ConfiguratingSet.h>
#include <Colors.h>
#include <ParseException.h>

#include <vector>
using std::vector;

//#define MOREDEBUG
//#define DEBUG

// ****************************************************************************
//  Constructor:  Parser::Parser
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
Parser::Parser()
{
    G = NULL;
    Init();
}

// ****************************************************************************
//  Method:  Parser::Init
//
//  Purpose:
//    Reinitialized the parse state, to get ready to parse a new string.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Jeremy Meredith, Mon Nov 17 17:08:46 EST 2008
//    We're now storing the parse tree result directly instead of pretending
//    it makes sense to leave it on the parse element stack.
//
// ****************************************************************************
void
Parser::Init()
{
    elems.clear();
    states.clear();
    states.push_back(0);
    accept = false;
    parseTree = NULL;
}

// ****************************************************************************
//  Method:  Parser::Shift
//
//  Purpose:
//    Shift a token onto the stack, going to a new state.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Jeremy Meredith, Wed Jun  8 17:08:35 PDT 2005
//    Added a symbol dictionary.
//
// ****************************************************************************
void
Parser::Shift(Token *t, int s)
{
#ifdef MOREDEBUG
    cerr << "Shifting token "; t->PrintNode(cerr);
#endif
    elems.push_back(ParseElem(G->GetDictionary().Get(t->GetType()), t));
    states.push_back(s);

    PrintState(cerr);
#ifdef MOREDEBUG
    cerr << endl;
#endif
}

// ****************************************************************************
//  Method:  Parser::Reduce
//
//  Purpose:
//    Reduces what is on the stack using the given rule.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Jeremy Meredith, Wed Nov 24 09:02:32 PST 2004
//    Added list of tokens.  Also, significant refactoring.
//
//    Hank Childs, Fri Jan 28 13:19:33 PST 2005
//    Use exception macros.
//
//    Jeremy Meredith, Mon Jun 13 15:53:51 PDT 2005
//    I made other changes to enforce that no Token will be stored
//    inside a parse tree node, leaving us free to delete them after
//    applying the reduction.  This fixes a number of memory leaks.
//    I also ensured that pos was well populated in all cases.
//
//    Jeremy Meredith, Wed Jul 23 13:26:26 EDT 2008
//    Add the rule index to the debug info.
//
//    Kathleen Bonnell,  Thu Nov 6 11:57:28 PST 2008
//    To prevent a crash on windows for referencing (from GetParseTree)
//    elems[0] when elems is empty, don't pop elems if rule->index is 0. 
//
//    Jeremy Meredith, Mon Nov 17 17:07:04 EST 2008
//    The windows bug (see previous comment) was truly a multi-platform
//    bug.  Now we store the result parse tree directly instead of
//    trying to pretend it's still on the parse elem stack (which
//    wasn't even happening anyway; it only worked due to a lot of luck!)...
//
// ****************************************************************************
void
Parser::Reduce(int r)
{
#ifdef MOREDEBUG
    cerr << "Reducing using rule " << G->GetRule(r)->GetIndex() << ": "
         << *(G->GetRule(r)) << endl;
#endif
    const Rule   *rule = G->GetRule(r);
    const Symbol *lhs = rule->GetLHS();
    const Symbol &sym = *lhs;

    int len = rule->GetRHS().Length();
    vector<ParseTreeNode*> E;
    vector<Token*> T;
    Pos p;
    int i;
    for (i=0; i<len; i++)
    {
        int index = elems.size() - len + i;
        E.push_back(elems[index].node);
        T.push_back(elems[index].token);
        p.Join(elems[index].pos);
    }

    ParseTreeNode *node = ApplyRule(sym, rule, E, T, p);

    // We're done with the tokens now that we've applied the rule
    for (i=0; i<len; i++)
    {
        delete T[i];
    }

    if (!node)
    {
        EXCEPTION2(UnhandledReductionException, p, rule);
    }

    node->GetPos().Join(p);

    for (i=0; i<len; i++)
    {
        states.pop_back();
        elems.pop_back();
    }

    State &state = G->GetState(states.back());

    if (G->GetStartSymbol() == lhs)
    {
        parseTree = node;
        PrintState(cerr);
#ifdef MOREDEBUG
        cerr << "Accepting!\n\n";
#endif
        accept = true;
        return;
    }

    // now do the goto
    elems.push_back(ParseElem(lhs, node));
    states.push_back(state.shift[lhs]);

    PrintState(cerr);
}

// ****************************************************************************
//  Method:  Parser::PrintState
//
//  Purpose:
//    Prints the current state of the parser.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Jeremy Meredith, Wed Jul 23 13:26:44 EDT 2008
//    Uncomment the state debug info, but only print in MOREDEBUG.
//
// ****************************************************************************
void
Parser::PrintState(ostream &o)
{
#ifdef DEBUG
#ifdef MOREDEBUG
    o << "state= "; for (size_t i=0; i<states.size(); i++) o << states[i] << " "; o << endl;
#endif
    o << "stack= "; for (size_t i=0; i<elems.size(); i++) o << *(elems[i].sym) << " "; o << endl;
#endif
}

// ****************************************************************************
//  Method:  Parser::ParseOneToken
//
//  Purpose:
//    Parses a single token, setting the accept flag when finished.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Jeremy Meredith, Mon Jul 28 16:57:00 PDT 2003
//    Added extra info to the error message.
//
//    Jeremy Meredith, Wed Nov 24 12:43:27 PST 2004
//    Added list of allowed tokens to the error message.  Did not yet
//    enable it by default because it may be exposing the guts too much.
//
//    Hank Childs, Fri Jan 28 13:19:33 PST 2005
//    Use exception macros.
//
//    Jeremy Meredith, Wed Jun  8 17:08:35 PDT 2005
//    Added a symbol dictionary.
//
// ****************************************************************************
void
Parser::ParseOneToken(Token *t)
{
    const Symbol *tokensym = G->GetDictionary().Get(t->GetType());
#ifdef MOREDEBUG
    if (!tokensym)
    {
        cerr << "Unknown token: type="<<t->GetType()<<endl;
    }
    cerr << "\nParse("<<*tokensym<<")\n";
#endif
    State &state = G->GetState(states.back());
    if (state.shift.count(tokensym))
    {
        Shift(t, state.shift[tokensym]);
    }
    else if (state.reduce.count(tokensym))
    {
        Reduce(state.reduce[tokensym]);
        if (!accept)
            ParseOneToken(t);
    }
    else
    {
        if (t->GetType() == EOF_TOKEN_ID)
        {
            EXCEPTION1(UnexpectedEndException, t->GetPos());
        }
        else
        {
#ifdef DEBUG
            std::string allowed = "Expected one of: ";
            std::map<const Symbol*, int>::iterator it;
            for (it = state.reduce.begin(); it != state.reduce.end(); it++)
            {
                allowed += it->first->GetDisplayString() + " ";
            }
            for (it = state.shift.begin(); it != state.shift.end(); it++)
            {
                allowed += it->first->GetDisplayString() + " ";
            }
            EXCEPTION2(SyntacticException, t->GetPos(), allowed);
#else
            EXCEPTION1(SyntacticException, t->GetPos());
#endif
        }
    }
}
