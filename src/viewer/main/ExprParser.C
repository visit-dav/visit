#include <ExprParser.h>
#include <ExprNode.h>
#include <Token.h>
#include <SymbolSet.h>
#include <Sequence.h>
#include <Rule.h>
#include <ConfiguratingItem.h>
#include <ConfiguratingSet.h>
#include <Colors.h>
#include <ExprGrammar.h>
#include <ParseException.h>
#include <ViewerMessaging.h>
#include <vector>
using std::vector;

ExprParser::ErrorMessageTarget ExprParser::errorMessageTarget = EMT_VIEWER;

class DummyNode : public ExprGrammarNode
{
  public:
    DummyNode(const Pos &p) : ExprGrammarNode(p) { }
    virtual void PrintNode(ostream &o) { o << "DummyNode\n"; }
};

// ****************************************************************************
//  Constructor:  ExprParser::ExprParser
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
ExprParser::ExprParser() : ParserBase()
{
    Grammar *G = new ExprGrammar;
    if (!G->Initialize())
    {
        cerr << "Error in initializion of Expression Grammar!\n";
        exit(-1);
    }

    SetGrammar(G);
}

// ****************************************************************************
//  Method:  ExprParser::ApplyRule
//
//  Purpose:
//    This is the implementation of a Grammar's pure virtual function when
//    applying reductions based on grammar rules.  We group them by 
//    the rule's LHS for readability, then simply key off the identifying
//    integer we set for the rule when creating it in ExprGrammar::ExprGrammar.
//
//  Arguments:
//    sym        the LHS of the rule
//    rule       the rule to apply
//    E          an array of ExprGrammarNodes as long as the length of the rule's RHS
//    p          the position encompassing the sequence in the expression
//
//  Note:
//    All ExprGrammarNodes must be downcasted here.  One must be careful that they
//    create the right types of nodes for what they are expecting in later
//    reductions.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Jeremy Meredith, Mon Jul 28 16:03:49 PDT 2003
//    Added more info to the error message in the TimeSpec.
//
// ****************************************************************************
ExprGrammarNode*
ExprParser::ApplyRule(const Symbol &sym, const Rule *rule,
                      vector<ExprGrammarNode*> &E, Pos p)
{
    ExprGrammarNode *node = NULL;
    if (sym == Expr)
    {
        switch (rule->GetID())
        {
        case 0:
            node = new BinaryExpr(p, (static_cast<Character *>(E[1]))->GetVal(),
                                  static_cast < ExprNode * >(E[0]),
                                  static_cast < ExprNode * >(E[2]));
            break;

        case 1:
            node =
                new BinaryExpr(p, (static_cast<Character *>(E[1]))->GetVal(),
                               static_cast<ExprNode *>(E[0]),
                               static_cast<ExprNode *>(E[2]));
            break;
        case 2:
            node =
                new BinaryExpr(p, (static_cast<Character *>(E[1]))->GetVal(),
                               static_cast<ExprNode*>(E[0]),
                               static_cast<ExprNode*>(E[2]));
            break;
        case 3:
            node =
                new BinaryExpr(p, (static_cast<Character *>(E[1]))->GetVal(),
                               static_cast<ExprNode*>(E[0]),
                               static_cast<ExprNode*>(E[2]));
            break;
        case 4:
            node =
                new BinaryExpr(p, (static_cast<Character *>(E[1]))->GetVal(),
                               static_cast<ExprNode*>(E[0]),
                               static_cast<ExprNode*>(E[2]));
            break;
        case 5:
            node =
                new BinaryExpr(p, (static_cast<Character *>(E[1]))->GetVal(),
                               static_cast<ExprNode*>(E[0]),
                               static_cast<ExprNode*>(E[2]));
            break;
        case 6:
            node = new UnaryExpr(p, (static_cast<Character *>(E[0]))->GetVal(),
                                 static_cast<ExprNode*>(E[1]));
            break;
        case 7:
            node = E[1];
            break;
        case 8:
            node = E[0];
            break;
        case 9:
            node = E[0];
            break;
        case 10:
            node = E[0];
            break;
        case 11:
            node = E[0];
            break;
        case 12:
            node = E[0];
            break;
        case 13:
            node = E[0];
            break;
        }
    } else if (sym == Constant)
    {
        switch (rule->GetID())
        {
        case 0:
            node = new ConstExpr(p, static_cast<Token *>(E[0]));
            break;
        case 1:
            node = new ConstExpr(p, static_cast<Token *>(E[0]));
            break;
        case 2:
            node = new ConstExpr(p, static_cast<Token *>(E[0]));
            break;
        case 3:
            node = new ConstExpr(p, static_cast<Token *>(E[0]));
            break;
        }
    } else if (sym == Vector)
    {
        switch (rule->GetID())
        {
        case 0:
            node = new VectorExpr(p, static_cast<ExprNode*>(E[1]),
                                  static_cast<ExprNode*>(E[3]));
            break;
        case 1:
            node = new VectorExpr(p, static_cast<ExprNode*>(E[1]),
                                  static_cast<ExprNode*>(E[3]),
                                  static_cast<ExprNode*>(E[5]));
            break;
        }
    } else if (sym == List)
    {
        switch (rule->GetID())
        {
        case 0:
            node = E[1];
            break;
        }
    } else if (sym == ListElems)
    {
        switch (rule->GetID())
        {
        case 0:
            ((ListExpr *) (node = E[0]))->AddListElem(static_cast<ListElemExpr *>(E[2]));
            break;
        case 1:
            node = new ListExpr(p, static_cast<ListElemExpr *>(E[0]));
            break;
        }
    } else if (sym == ListElem)
    {
        switch (rule->GetID())
        {
        case 0:
            node = new ListElemExpr(p, static_cast<ExprNode*>(E[0]));
            break;
        case 1:
            node = new ListElemExpr(p, static_cast<ExprNode*>(E[0]),
                                    static_cast<ExprNode*>(E[2]));
            break;
        case 2:
            node = new ListElemExpr(p, static_cast<ExprNode*>(E[0]),
                                    static_cast<ExprNode*>(E[2]),
                                    static_cast<ExprNode*>(E[4]));
            break;
        }
    } else if (sym == Function)
    {
        switch (rule->GetID())
        {
        case 0:
            node = new FunctionExpr(p, static_cast<Identifier *>(E[0]));
            break;
        case 1:
            node =
                new FunctionExpr(p, static_cast<Identifier *>(E[0]),
                                 static_cast<ArgsExpr *>(E[2]));
            break;
        }
    } else if (sym == Args)
    {
        switch (rule->GetID())
        {
        case 0:
            ((ArgsExpr *) (node = E[0]))->AddArg(static_cast<ArgExpr *>(E[2]));
            break;
        case 1:
            node = new ArgsExpr(p, static_cast<ArgExpr *>(E[0]));
            break;
        }
    } else if (sym == Arg)
    {
        switch (rule->GetID())
        {
        case 0:
            node = new ArgExpr(p, static_cast<ExprNode*>(E[0]));
            break;
        case 1:
            node = new ArgExpr(p, static_cast<Identifier *>(E[0]),
                               static_cast<ExprNode*>(E[2]));
            break;
        }
    } else if (sym == PathSpec)
    {
        switch (rule->GetID())
        {
        case 0:
            node = E[0];
            ((PathExpr *) node)->Append("/");
            ((PathExpr *) node)->Append((static_cast<Identifier *>(E[2]))->GetVal());

            delete  E[1];
            break;
        case 1:
            node = new PathExpr(p, "/");
            ((PathExpr *) node)->Append((static_cast<Identifier *>(E[1]))->GetVal());
            delete  E[0];
            break;
        case 2:
            node = new PathExpr(p, (static_cast<Identifier *>(E[0]))->GetVal());
            break;
        }
    } else if (sym == MultiSlash)
    {
        switch (rule->GetID())
        {
        case 0:
            node = new DummyNode(p);
            break;
        case 1:
            node = new DummyNode(p);
            break;
        }
    } else if (sym == Variable)
    {
        switch (rule->GetID())
        {
        case 0:
            node =
                new VarExpr(p, NULL,
                            new PathExpr(p, (static_cast<Identifier *>(E[0]))->GetVal()),
                            true);
            break;
        case 1:
            node = new VarExpr(p, NULL, static_cast<PathExpr *>(E[1]), false);
            break;
        case 2:
            node = new VarExpr(p, static_cast<DBExpr *>(E[1]),
                               static_cast<PathExpr *>(E[3]), false);
            break;
        }
    } else if (sym == Database)
    {
        switch (rule->GetID())
        {
        case 0:
            node = E[1];
            break;
        }
    } else if (sym == DBSpec)
    {
        switch (rule->GetID())
        {
        case 0:
            node = new DBExpr(p, static_cast<PathExpr *>(E[0]), NULL, NULL);
            break;
        case 1:
            node = new DBExpr(p, static_cast<PathExpr *>(E[0]),
                              static_cast<MachExpr *>(E[1]), NULL);
            break;
        case 2:
            node = new DBExpr(p, NULL, NULL, static_cast<TimeExpr *>(E[0]));
            break;
        case 3:
            node = new DBExpr(p, static_cast<PathExpr *>(E[0]), NULL,
                              static_cast<TimeExpr *>(E[1]));
            break;
        case 4:
            node =
                new DBExpr(p, static_cast<PathExpr *>(E[0]),
                           static_cast<MachExpr *>(E[1]),
                           static_cast<TimeExpr *>(E[2]));
            break;
        }
    } else if (sym == MachSpec)
    {
        switch (rule->GetID())
        {
        case 0:
            node = new MachExpr(p, static_cast<Identifier *>(E[1]));
            break;
        }
    } else if (sym == TimeSpec)
    {
        switch (rule->GetID())
        {
        case 0:
            {
                const   string & id = static_cast<Identifier *>(E[3])->GetVal();
                if (id.length() != 1)
                    throw SyntacticException(E[3]->GetPos(),
                                             "needs to be 'i', 'c', or 't'");
                char    c = id[0];
                TimeExpr::Type t;
                if (c == 'c' || c == 'C')
                    t = TimeExpr::Cycle;
                else if (c == 't' || c == 'T')
                    t = TimeExpr::Time;
                else if (c == 'i' || c == 'I')
                    t = TimeExpr::Index;
                else
                    throw SyntacticException(E[3]->GetPos(),
                                             "needs to be 'i', 'c', or 't'");
                node = new TimeExpr(p, static_cast<ListExpr *>(E[1]), t);
                break;
            }
        case 1:
            node = new TimeExpr(p, static_cast<ListExpr *>(E[1]));
            break;
        case 2:
            node = new TimeExpr(p, static_cast<ListExpr *>(E[2]),
                                TimeExpr::Index);
            break;
        }
    } else if (sym == *G->GetStartSymbol())
    {
        node = E[0];
    }
    return node;
}

// ****************************************************************************
//  Method:  ExprParser::Parse
//
//  Purpose:
//    Main external routine for clients.
//    Re-initialize the parser and scanner, and process one at a time until
//    we get an error or acceptance.  Return the parse tree when finished.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Jeremy Meredith, Mon Jul 28 16:13:35 PDT 2003
//    Made it report normal parse errors through the viewer window
//    mechanism if needed.  Unhandled reductions will only occur during
//    debugging and can still go to cerr.
//
// ****************************************************************************
ExprNode*
ExprParser::Parse(const std::string &s)
{
    string text(s);
    // Change weird spacing (tabs, newlines) to normal ones.
    // This will make printing error messages later much easier.
    for (int i=0; i<text.length(); i++)
        if (text[i] == '\t' || text[i] == '\n') text[i] = ' ';

    try
    {
        Init();
        scanner.SetInput(text);
        Token *token = NULL;
        while (!Accept()) 
        {
            token = scanner.ScanOneToken();
            if (token->GetType() != TT_Space)
                ParseOneToken(token);
        }
    }
    catch (UnhandledReductionException &e)
    {
        // This should only occur during debugging; print to cerr anyway
        cerr << e.Message() << endl;
        cerr << "Rule = " << *(e.GetRule()) << endl;
        e.GetPos().PrintText(cerr, text);
        return NULL;
    }
    catch (ParseException &e)
    {
        char error[1024];
        SNPRINTF(error, 1024, "%s\n%s",
                 e.Message(), e.GetPos().GetText(text).c_str());

        if (errorMessageTarget == EMT_VIEWER)
            Error(error);
        else if (errorMessageTarget == EMT_CONSOLE)
            cerr << error;

        return NULL;
    }

    return static_cast<ExprNode*>(GetParseTree());
}

