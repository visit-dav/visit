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

#include "Token.h"
#include "Scanner.h"
#include "Grammar.h"
#include "VisItParser.h"
#include "ParseException.h"
#include "Symbol.h"
#include "Dictionary.h"

using std::string;
using std::vector;

// ----------------------------------------------------------------------------
// dictionary
// ----------------------------------------------------------------------------
Dictionary D;

// ----------------------------------------------------------------------------
// terminals
// ----------------------------------------------------------------------------
enum TokenType {
    TT_NoToken    = 0,
    // All single-character tokens go in this range
    TT_EOF        = EOF_TOKEN_ID,
    TT_Var
};
Symbol T_Plus(D, '+');
Symbol T_Mult(D, '*');
Symbol T_Var(D, TT_Var, "Var");

// ----------------------------------------------------------------------------
// nonterminals
// ----------------------------------------------------------------------------
Symbol Expr(D, "Expr");

// ----------------------------------------------------------------------------
// Tokens
// ----------------------------------------------------------------------------
class Operator : public Token
{
  public:
    Operator(const Pos &p, char c) : Token(p,c), op(c) { }
    void PrintNode(ostream &o) { o << "Operator: "<<op<<endl; }
  public:
    char op;
};

class Var : public Token
{
  public:
    Var(const Pos &p, const std::string &v) : Token(p,TT_Var), name(v) { }
    void PrintNode(ostream &o) { o << "Var: "<<name<<endl; }
  public:
    string name;
};

// ----------------------------------------------------------------------------
// Scanner
// ----------------------------------------------------------------------------
class TestScanner : public Scanner
{
  public:
    TestScanner() { }
    virtual ~TestScanner() { }
    virtual void   SetInput(const std::string &s)
    {
        input = s;
        pos = 0;
    }
    virtual Token *ScanOneToken()
    {
        if (pos >= input.length())
            return new EndOfInput(pos);

        if (input[pos] == '*' || input[pos] == '+')
        {
            Token *t = new Operator(pos, input[pos]);
            pos++;
            return t;
        }

        if (input[pos] < 'a' || input[pos] > 'z')
            EXCEPTION1(LexicalException, pos);

        string var;
        while (input[pos] >= 'a' && input[pos] <= 'z')
        {
            var += input[pos];
            pos++;
        }
        return new Var(Pos(pos - var.length(), pos-1), var);
    }
  private:
    string input;
    int pos;
};

// ----------------------------------------------------------------------------
// ParseTreeNodes
// ----------------------------------------------------------------------------
class BinaryExpression : public ParseTreeNode 
{
  public:
    BinaryExpression(const Pos &p, char op, ParseTreeNode *l, ParseTreeNode *r)
        : ParseTreeNode(p), oper(op), left(l), right(r) { }
    virtual ~BinaryExpression()
    {
        delete left;
        delete right;        
    }
    virtual const std::string GetTypeName() { return "BinaryExpression"; }
    virtual void PrintNode(ostream &o)
    {
        o << "BinaryExpression: '"<<oper<<"'" << endl;
        left->Print(o,"Left:  ");
        right->Print(o,"Right: ");
    }
  public:
    char oper;
    ParseTreeNode *left;
    ParseTreeNode *right;
};

class VarExpression : public ParseTreeNode
{
  public:
    VarExpression(const Pos &p, string &v) : ParseTreeNode(p), name(v) { }
    virtual ~VarExpression() { }
    virtual const std::string GetTypeName() {return "VarExpression";}
    virtual void PrintNode(ostream &o) { o << "Var: "<<name<<"\n"; }
  private:
    string name;
};

// ----------------------------------------------------------------------------
// Grammar Definition
// ----------------------------------------------------------------------------
class TestGrammar : public Grammar
{
  public:
    TestGrammar() : Grammar(D)
    {
        SetStartSymbol(Expr);

        AddRule(Rule(0, Expr) >> Expr + T_Plus + Expr );
        AddRule(Rule(1, Expr) >> Expr + T_Mult + Expr );
        AddRule(Rule(2, Expr) >> T_Var );

        SetAssoc(T_Plus,  Grammar::Left);
        SetAssoc(T_Mult,  Grammar::Left);
        SetPrec(T_Plus,   1);
        SetPrec(T_Mult,   2);
    }
    bool Initialize() { return false; } // only for pre-configuration 
    
};

// ----------------------------------------------------------------------------
// Parser
//
// Modifications:
//
//   Hank Childs, Fri Jan 28 15:43:04 PST 2005
//   Use expression macros.
//
// ----------------------------------------------------------------------------
class TestParser : public Parser
{
  private:
    ParseTreeNode *ApplyRule(const Symbol &sym, const Rule *rule,
                           vector<ParseTreeNode*> &E,
                           vector<Token*> &T, Pos p)
    {
        ParseTreeNode *node = NULL;
        if (sym == Expr)
        {
            switch (rule->GetID())
            {
              case 0:
              case 1:
                node = new BinaryExpression(p,((Operator*)T[1])->op,E[0],E[2]);
                break;
              case 2:
                node = new VarExpression(p, ((Var*)T[0])->name);
                break;
            }
        }
        else if (sym == *G->GetStartSymbol())
        {
            node = E[0];
        }
        return node;
    }

  public:
    TestParser() : Parser()
    {
        Grammar *G = new TestGrammar;
#ifdef PRE_CONFIGURED
        if (!G->Initialize())
#else
        G->SetPrinter(&cout);
        if (!G->Configure())
#endif
        {
            cerr << "Error in initializion of Expression Grammar!\n";
        }
        else
        {
            /*
            cout<<"-------------------------------------------------------\n";
            cout<<"---------------- Grammar Configuration ----------------\n";
            cout<<"-------------------------------------------------------\n";
            G->WriteStateInitialization("TestGrammar", cout);
            cout<<"-------------------------------------------------------\n";
            cout<<"-------------- End Grammar Configuration --------------\n";
            cout<<"-------------------------------------------------------\n";
            */
            SetGrammar(G);
        }
    }

    TestParser::~TestParser()
    {
        delete G;
    }

    ParseTreeNode *Parse(const string &input)
    {
        TRY
        {
            Init();
            TestScanner scanner;
            scanner.SetInput(input);

            Token *token = NULL;
            while (!Accept())
            {
                token = scanner.ScanOneToken();
                ParseOneToken(token);
            }
            delete token;
        }
        CATCH2(UnhandledReductionException, e)
        {
            // This should only occur during debugging; print to cerr anyway
            cerr << e.Message() << endl;
            cerr << "Rule = " << *(e.GetRule()) << endl;
            cerr << e.GetPos().GetErrorText(input) << endl;
            CATCH_RETURN2(1, NULL);
        }
        CATCH2(ParseException, e)
        {
            cerr << e.Message() << endl;
            cerr << e.GetPos().GetErrorText(input) << endl;
            CATCH_RETURN2(1, NULL);
        }
        CATCHALL(...)
        {
            cerr << "Unknown exception!\n";
            CATCH_RETURN2(1, NULL);
        }
        ENDTRY
        return GetParseTree();
    }

};

// ****************************************************************************
//  Function:  main
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 24, 2004
//
//  Modifications:
//
// ****************************************************************************
int
main(int argc, char *argv[])
{
    if (argc<2) {cerr<<"needs an argument\n"; exit(-1);}

    Parser *parser = new TestParser();

    for (int i=1; i<argc; i++)
    {
        cout << "----- Parsing "<<argv[i]<<" -----\n";
        ParseTreeNode *node = parser->Parse(argv[i]);
        if (node)
        {
            cout << "----- PARSE TREE -----\n";
            node->Print(cout);
            delete node;
        }
        else
            cout << "ERROR\n";
    }

    delete parser;

    return 0;
}
