#ifndef EXPRNODE_H
#define EXPRNODE_H

#include <expr_exports.h>
#include <vector>
#include <set>
#include <string>
#include <ExprParseTreeNode.h>
#include <ExprToken.h>

class Identifier;
class Pos;

// ****************************************************************************
//  Class:  ExprNode and other Expression Grammar parse tree nodes
//
//  Purpose:
//    ExprNode is the base class for all nodes in an Expression tree
//    that are themselves expressions.  Other things like paths and
//    function arguments are not directly Expressions, but can contain
//    them -- these derive directly from ExprParseTreeNode.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Sean Ahern, Wed Apr 17 17:01:41 PDT 2002
//    Made the class able to contact the engine to generate plots.
//
//    Sean Ahern, Wed Oct 16 16:41:32 PDT 2002
//    Removed the engine communication code to a separate set of classes
//    owned by the viewer.
//
//    Jeremy Meredith, Wed Nov 24 11:47:45 PST 2004
//    Made expression language specific tokens have a more specific
//    base class.  Renamed GrammarNode to ParseTreeNode.  Refactored
//    expression specific stuff to its own library.  Had ArgExpr contain
//    the true base class for expressions -- a List is a valid argument
//    to a function but currently is not an Expression (i.e. ExprNode).
//
// ****************************************************************************
class EXPR_API ExprNode : public ExprParseTreeNode
{
  public:
    ExprNode(const Pos &p)
        : ExprParseTreeNode(p) {}
    virtual ~ExprNode() { }
    virtual std::set<std::string> GetVarLeaves() = 0;
    virtual const std::string GetTypeName() = 0;
};

class EXPR_API ConstExpr : public virtual ExprNode
{
  public:
    ConstExpr(const Pos &p, ExprToken *t);
    virtual ~ConstExpr() { }
    virtual void PrintNode(ostream &o);
    virtual std::set<std::string> GetVarLeaves() {return std::set<std::string>();}
    virtual const std::string GetTypeName() { return "Const"; }
    ExprToken * GetToken() { return token; }
  protected:
    ExprToken *token;
};

class EXPR_API MathExpr : public virtual ExprNode
{
  public:
    MathExpr(const Pos &p, char o)
        : ExprNode(p), op(o) {}
    virtual ~MathExpr() { }
    virtual const std::string GetTypeName() { return "Math"; }
  protected:
    char op;
};

class EXPR_API UnaryExpr : public MathExpr
{
  public:
    UnaryExpr(const Pos &p, char o, ExprNode *e)
        : MathExpr(p, o), ExprNode(p), expr(e) {}
    virtual ~UnaryExpr() { }
    virtual void PrintNode(ostream &o);
    virtual std::set<std::string> GetVarLeaves()
        {return expr->GetVarLeaves();}
    virtual const std::string GetTypeName() { return "Unary"; }
  protected:
    ExprNode *expr;
};

class EXPR_API BinaryExpr : public MathExpr
{
  public:
    BinaryExpr(const Pos &p, char o, ExprNode *l, ExprNode *r)
        : MathExpr(p, o), ExprNode(p), left(l), right(r) {}
    virtual ~BinaryExpr() { }
    virtual void PrintNode(ostream &o);
    virtual std::set<std::string> GetVarLeaves();
    virtual const std::string GetTypeName() { return "Binary"; }
  protected:
    ExprNode *left;
    ExprNode *right;
};

class EXPR_API IndexExpr : public virtual ExprNode
{
  public:
    IndexExpr(const Pos &p, ExprNode *e, int i)
        : ExprNode(p), expr(e), ind(i) {}
    virtual ~IndexExpr() { }
    virtual void PrintNode(ostream &o);
    virtual std::set<std::string> GetVarLeaves()
        {return expr->GetVarLeaves();}
    virtual const std::string GetTypeName() { return "Index"; }
  protected:
    ExprNode *expr;
    int ind;
};

class EXPR_API VectorExpr : public virtual ExprNode
{
  public:
    VectorExpr(const Pos &p, ExprNode *xi, ExprNode *yi, ExprNode *zi=NULL)
        : ExprNode(p), x(xi), y(yi), z(zi) {}
    virtual ~VectorExpr() { }
    virtual void PrintNode(ostream &o);
    virtual std::set<std::string> GetVarLeaves();
    virtual const std::string GetTypeName() { return "Vector"; }
  protected:
    ExprNode *x, *y, *z;
};

class EXPR_API ListElemExpr : public ExprParseTreeNode
{
  public:
    ListElemExpr(const Pos &p, ExprNode *b, ExprNode *e=NULL, ExprNode *s=NULL)
        : ExprParseTreeNode(p), beg(b), end(e), skip(s) {}
    virtual ~ListElemExpr() { }
    virtual void PrintNode(ostream &o);
    virtual const std::string GetTypeName() { return "ListElem"; }
    ExprNode *GetItem(void) { return beg; }
    ExprNode *GetBeg(void) { return GetItem(); }
    ExprNode *GetEnd(void) { return end; }
    ExprNode *GetSkip(void) { return skip; }
  protected:
    ExprNode *beg;
    ExprNode *end;
    ExprNode *skip;
};

class EXPR_API ListExpr : public ExprParseTreeNode
{
  public:
    ListExpr(const Pos &p, ListElemExpr *e);
    virtual ~ListExpr() { }
    void AddListElem(ListElemExpr *e);
    virtual void PrintNode(ostream &o);
    virtual const std::string GetTypeName() { return "List"; }
    std::vector<ListElemExpr*> *GetElems(void) { return elems; }
    virtual std::set<std::string> GetVarLeaves();
  protected:
    std::vector<ListElemExpr*> *elems;
};

class EXPR_API ArgExpr : public ExprParseTreeNode
{
  public:
    ArgExpr(const Pos &p, ExprParseTreeNode *e)
        : ExprParseTreeNode(p), id(NULL), expr(e) {}
    virtual ~ArgExpr() { }
    ArgExpr(const Pos &p, Identifier *i, ExprParseTreeNode *e)
        : ExprParseTreeNode(p), id(i), expr(e) {}
    virtual void PrintNode(ostream &o);
    Identifier *GetId(void) {return id;};
    ExprParseTreeNode *GetExpr(void) {return expr;};
    virtual const std::string GetTypeName() { return "Arg"; }
  protected:
    Identifier *id;
    ExprParseTreeNode *expr;
};

class EXPR_API ArgsExpr : public ExprParseTreeNode
{
  public:
    ArgsExpr(const Pos &p, ArgExpr *e);
    virtual ~ArgsExpr() { }
    void AddArg(ArgExpr *e);
    virtual void PrintNode(ostream &o);
    std::vector<ArgExpr*>* GetArgs(void) {return args;};
    virtual const std::string GetTypeName() { return "Args"; }
  protected:
    std::vector<ArgExpr*> *args;
};

class EXPR_API FunctionExpr : public virtual ExprNode
{
  public:
    FunctionExpr(const Pos &p, Identifier *i, ArgsExpr *e=NULL)
        : ExprNode(p), name(i), args(e) {}
    virtual ~FunctionExpr() { }
    virtual void PrintNode(ostream &o);
    virtual std::set<std::string> GetVarLeaves();
    virtual const std::string GetTypeName() { return "Function"; }
  protected:
    Identifier *name;
    ArgsExpr   *args;
};

class EXPR_API PathExpr : public ExprParseTreeNode
{
  public:
    PathExpr(const Pos &p, const std::string &s)
        : ExprParseTreeNode(p), basename(s), fullpath(s) {}
    virtual ~PathExpr() { }
    void Append(const std::string&);
    virtual void PrintNode(ostream &o);
    std::string GetFullpath(void) {return fullpath;};
    std::string GetBasename(void) {return basename;};
    std::string GetDirname(void) {return dirname;};
    virtual const std::string GetTypeName() { return "Path"; }
  protected:
    std::string basename;
    std::string dirname;
    std::string fullpath;
};

class EXPR_API MachExpr : public ExprParseTreeNode
{
  public:
    MachExpr(const Pos &p, Identifier *i)
        : ExprParseTreeNode(p), host(i) {}
    virtual ~MachExpr() { }
    virtual void PrintNode(ostream &o);
    virtual const std::string GetTypeName() { return "Mach"; }
  protected:
    Identifier *host;
};

class EXPR_API TimeExpr : public ExprParseTreeNode
{
  public:
    enum Type { Cycle, Time, Index, Unknown };
    TimeExpr(const Pos &p, ListExpr *l, Type t=Unknown)
        : ExprParseTreeNode(p), type(t), list(l) {}
    virtual ~TimeExpr() { }
    virtual void PrintNode(ostream &o);
    virtual const std::string GetTypeName() { return "Time"; }
  protected:
    Type      type;
    ListExpr *list;
};

class EXPR_API DBExpr : public ExprParseTreeNode
{
  public:
    DBExpr(const Pos &p, PathExpr *f, MachExpr *m, TimeExpr *t)
        : ExprParseTreeNode(p), file(f), mach(m), time(t) {}
    virtual ~DBExpr() { }
    virtual void PrintNode(ostream &o);
    virtual const std::string GetTypeName() { return "DBExpr"; }
  protected:
    PathExpr *file;
    MachExpr *mach;
    TimeExpr *time;
};

class EXPR_API VarExpr : public virtual ExprNode
{
  public:
    VarExpr(const Pos &p, DBExpr *d, PathExpr *v, bool exp)
        : ExprNode(p), db(d), var(v), canexpand(exp) {}
    virtual ~VarExpr() { }
    virtual void PrintNode(ostream &o);
    virtual std::set<std::string> GetVarLeaves();
    virtual const std::string GetTypeName() { return "Var"; }
  protected:
    DBExpr   *db;
    PathExpr *var;
    bool      canexpand;
};


#endif
