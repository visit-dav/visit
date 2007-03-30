#include <ParserBase.h>
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
//  Constructor:  ParserBase::ParserBase
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
ParserBase::ParserBase()
{
    G = NULL;
    Init();
}

// ****************************************************************************
//  Method:  ParserBase::Init
//
//  Purpose:
//    Reinitialized the parse state, to get ready to parse a new string.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
void
ParserBase::Init()
{
    elems.clear();
    states.clear();
    states.push_back(0);
    accept = false;
}

// ****************************************************************************
//  Method:  ParserBase::Shift
//
//  Purpose:
//    Shift a token onto the stack, going to a new state.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
void
ParserBase::Shift(Token *t, int s)
{
#ifdef MOREDEBUG
    cerr << "Shifting token "; t->Print(cerr); cerr << endl;
#endif
    elems.push_back(ParseElem(t));
    states.push_back(s);

    PrintState(cerr);
}

// ****************************************************************************
//  Method:  ParserBase::Reduce
//
//  Purpose:
//    Reduces what is on the stack using the given rule.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
void
ParserBase::Reduce(int r)
{
#ifdef MOREDEBUG
    cerr << "Reducing using rule " << *(G->GetRule(r)) << endl;
#endif
    const Rule   *rule = G->GetRule(r);
    const Symbol *lhs = rule->GetLHS();
    const Symbol &sym = *lhs;

    int len = rule->GetRHS().Length();
    vector<ExprGrammarNode*> E;
    int i;
    for (i=0; i<len; i++)
        E.push_back(elems[elems.size() - len + i].node);

    Pos p;
    if (len)
        p = Pos(E[0]->GetPos(), E[len-1]->GetPos());

    ExprGrammarNode *node = ApplyRule(sym, rule, E, p);

    if (!node)
    {
        throw UnhandledReductionException(p, rule);
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
//  Method:  ParserBase::PrintState
//
//  Purpose:
//    Prints the current state of the parser.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
void
ParserBase::PrintState(ostream &o)
{
#ifdef DEBUG
    //o << "state= "; for (int i=0; i<states.size(); i++) o << states[i] << " "; o << endl;
    o << "stack= "; for (int i=0; i<elems.size(); i++) o << *(elems[i].sym) << " "; o << endl;
#endif
}

// ****************************************************************************
//  Method:  ParserBase::ParseOneToken
//
//  Purpose:
//    Parses a single token, setting the accept flag when finished.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
void
ParserBase::ParseOneToken(Token *t)
{
    const Symbol *tokensym = Symbol::Get(t->GetType());
#ifdef MOREDEBUG
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
        if (t->GetType() == TT_EOF)
            throw UnexpectedEndException(t->GetPos());
        else
            throw SyntacticException(t->GetPos());
    }
}
