#ifndef EXPRNODE_H
#define EXPRNODE_H
#include <viewer_exports.h>

#include <vector>
#include <Pos.h>
#include <ExprGrammarNode.h>

class Identifier;
class Token;

class EngineProxy;
class ViewerPlot;

// ****************************************************************************
//  Class:  ExprNode
//
//  Purpose:
//    Base class for all nodes in VisIt expression trees.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//      Sean Ahern, Wed Apr 17 17:01:41 PDT 2002
//      Made the class able to contact the engine to generate plots.
//
// ****************************************************************************
class VIEWER_API ExprNode : public ExprGrammarNode
{
  public:
    ExprNode(const Pos &p) : ExprGrammarNode(p) { }
    virtual ~ExprNode() { }
    virtual void GeneratePlot(EngineProxy *, ViewerPlot *, const int time) = 0;
};

class VIEWER_API ConstExpr : public ExprNode
{
  public:
    ConstExpr(const Pos &p, Token *t);
    virtual ~ConstExpr() { }
    virtual void PrintNode(ostream &o);
    virtual void GeneratePlot(EngineProxy *, ViewerPlot *, const int time);
  protected:
    Token *token;
};

class VIEWER_API MathExpr : public ExprNode
{
  public:
    MathExpr(const Pos &p, char o);
    virtual ~MathExpr() { }
  protected:
    char op;
};

class VIEWER_API UnaryExpr : public MathExpr
{
  public:
    UnaryExpr(const Pos &p, char o, ExprNode *e);
    virtual ~UnaryExpr() { }
    virtual void PrintNode(ostream &o);
    virtual void GeneratePlot(EngineProxy *, ViewerPlot *, const int time);
  protected:
    ExprNode *expr;
};

class VIEWER_API BinaryExpr : public MathExpr
{
  public:
    BinaryExpr(const Pos &p, char o, ExprNode *l, ExprNode *r);
    virtual ~BinaryExpr() { }
    virtual void PrintNode(ostream &o);
    virtual void GeneratePlot(EngineProxy *, ViewerPlot *, const int time);
  protected:
    ExprNode *left;
    ExprNode *right;
};

class VIEWER_API VectorExpr : public ExprNode
{
  public:
    VectorExpr(const Pos &p, ExprNode *x, ExprNode *y, ExprNode *z=NULL);
    virtual ~VectorExpr() { }
    virtual void PrintNode(ostream &o);
    virtual void GeneratePlot(EngineProxy *, ViewerPlot *, const int time);
  protected:
    ExprNode *x, *y, *z;
};

class VIEWER_API ListElemExpr : public ExprGrammarNode
{
  public:
    ListElemExpr(const Pos &p, ExprNode *b, ExprNode *e=NULL, ExprNode *s=NULL);
    virtual ~ListElemExpr() { }
    virtual void PrintNode(ostream &o);
  protected:
    ExprNode *beg;
    ExprNode *end;
    ExprNode *skip;
};

class VIEWER_API ListExpr : public ExprGrammarNode
{
  public:
    ListExpr(const Pos &p, ListElemExpr *e);
    virtual ~ListExpr() { }
    void AddListElem(ListElemExpr *e);
    virtual void PrintNode(ostream &o);
  protected:
    std::vector<ListElemExpr*> *elems;
};

class VIEWER_API ArgExpr : public ExprGrammarNode
{
  public:
    ArgExpr(const Pos &p, ExprNode *e);
    virtual ~ArgExpr() { }
    ArgExpr(const Pos &p, Identifier *i, ExprNode *e);
    virtual void PrintNode(ostream &o);
    Identifier *GetId(void) {return id;};
    ExprNode *GetExpr(void) {return expr;};
  protected:
    Identifier *id;
    ExprNode   *expr;
};

class VIEWER_API ArgsExpr : public ExprGrammarNode
{
  public:
    ArgsExpr(const Pos &p, ArgExpr *e);
    virtual ~ArgsExpr() { }
    void AddArg(ArgExpr *e);
    virtual void PrintNode(ostream &o);
    std::vector<ArgExpr*>* GetArgs(void) {return args;};
  protected:
    std::vector<ArgExpr*> *args;
};

class VIEWER_API FunctionExpr : public ExprNode
{
  public:
    FunctionExpr(const Pos &p, Identifier *i, ArgsExpr *e=NULL);
    virtual ~FunctionExpr() { }
    virtual void PrintNode(ostream &o);
    virtual void GeneratePlot(EngineProxy *, ViewerPlot *, const int time);
  protected:
    Identifier *name;
    ArgsExpr   *args;
};

class VIEWER_API PathExpr : public ExprGrammarNode
{
  public:
    PathExpr(const Pos &p, const std::string&);
    virtual ~PathExpr() { }
    void Append(const std::string&);
    virtual void PrintNode(ostream &o);
    std::string GetFullpath(void) {return fullpath;};
  protected:
    std::string basename;
    std::string dirname;
    std::string fullpath;
};

class VIEWER_API MachExpr : public ExprGrammarNode
{
  public:
    MachExpr(const Pos &p, Identifier *i);
    virtual ~MachExpr() { }
    virtual void PrintNode(ostream &o);
  protected:
    Identifier *host;
};

class VIEWER_API TimeExpr : public ExprGrammarNode
{
  public:
    enum Type { Cycle, Time, Index, Unknown };
    TimeExpr(const Pos &p, ListExpr *l, Type t=Unknown);
    virtual ~TimeExpr() { }
    virtual void PrintNode(ostream &o);
  protected:
    Type      type;
    ListExpr *list;
};

class VIEWER_API DBExpr : public ExprGrammarNode
{
  public:
    DBExpr(const Pos &p, PathExpr *f, MachExpr *m, TimeExpr *t);
    virtual ~DBExpr() { }
    virtual void PrintNode(ostream &o);
  protected:
    PathExpr *file;
    MachExpr *mach;
    TimeExpr *time;
};

class VIEWER_API VarExpr : public ExprNode
{
  public:
    VarExpr(const Pos &p, DBExpr *d, PathExpr *v, bool exp);
    virtual ~VarExpr() { }
    virtual void PrintNode(ostream &o);
    virtual void GeneratePlot(EngineProxy *, ViewerPlot *, const int time);
  protected:
    DBExpr   *db;
    PathExpr *var;
    bool      canexpand;
};

#endif
