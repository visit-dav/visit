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
// ****************************************************************************
void
Parser::Init()
{
    elems.clear();
    states.clear();
    states.push_back(0);
    accept = false;
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
// ****************************************************************************
void
Parser::Shift(Token *t, int s)
{
#ifdef MOREDEBUG
    cerr << "Shifting token "; t->PrintNode(cerr); cerr << endl;
#endif
    elems.push_back(ParseElem(t));
    states.push_back(s);

    PrintState(cerr);
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
// ****************************************************************************
void
Parser::Reduce(int r)
{
#ifdef MOREDEBUG
    cerr << "Reducing using rule " << *(G->GetRule(r)) << endl;
#endif
    const Rule   *rule = G->GetRule(r);
    const Symbol *lhs = rule->GetLHS();
    const Symbol &sym = *lhs;

    int len = rule->GetRHS().Length();
    vector<ParseTreeNode*> E;
    vector<Token*> T;
    int i;
    for (i=0; i<len; i++)
    {
        int index = elems.size() - len + i;
        E.push_back(elems[index].node);
        T.push_back(elems[index].token);
    }

    Pos p;
    if (len)
        p = Pos(E[0]->GetPos(), E[len-1]->GetPos());

    ParseTreeNode *node = ApplyRule(sym, rule, E, T, p);

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
//  Method:  Parser::PrintState
//
//  Purpose:
//    Prints the current state of the parser.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
void
Parser::PrintState(ostream &o)
{
#ifdef DEBUG
    //o << "state= "; for (int i=0; i<states.size(); i++) o << states[i] << " "; o << endl;
    o << "stack= "; for (int i=0; i<elems.size(); i++) o << *(elems[i].sym) << " "; o << endl;
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
// ****************************************************************************
void
Parser::ParseOneToken(Token *t)
{
    const Symbol *tokensym = Symbol::Get(t->GetType());
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
            throw UnexpectedEndException(t->GetPos());
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
            throw SyntacticException(t->GetPos(), allowed);
#else
            throw SyntacticException(t->GetPos());
#endif
        }
    }
}
