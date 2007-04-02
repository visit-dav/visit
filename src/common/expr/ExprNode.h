#ifndef EXPRNODE_H
#define EXPRNODE_H

#include <expr_exports.h>
#include <vector>
#include <set>
#include <string>
#include <ExprParseTreeNode.h>

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
//    Jeremy Meredith, Tue Dec 28 11:21:13 PST 2004
//    Added the original text for an argument to ArgExpr because it is
//    useful for implementing macros.
//
//    Jeremy Meredith, Mon Jun 13 15:46:22 PDT 2005
//    Made ConstExpr abstract and split it into multiple concrete
//    base classes.  Made FunctionExpr and MachExpr use names
//    instead of Identifier tokens.  These two changes were to
//    remove Token references from the parse tree node classes.
//    Also added some destructors.
//
//    Hank Childs, Tue Aug 30 13:20:47 PDT 2005
//    Added access to variable and database names.
//
//    Hank Childs, Thu Sep  1 11:25:35 PDT 2005
//    Added access for TimeExpr's.
//
//    Hank Childs, Thu Sep  8 15:25:03 PDT 2005
//    Added GetVarLeafNodes, which is like GetVarLeaves, but returns
//    ExprNodes instead.
//
// ****************************************************************************
class EXPR_API ExprNode : public ExprParseTreeNode
{
  public:
    ExprNode(const Pos &p)
        : ExprParseTreeNode(p) {}
    virtual ~ExprNode() { }
    virtual std::set<std::string> GetVarLeaves() = 0;
    virtual std::set<ExprParseTreeNode *> GetVarLeafNodes() = 0;
    virtual const std::string GetTypeName() = 0;
};

class EXPR_API ConstExpr : public virtual ExprNode
{
  public:
    enum ConstType { Integer, Float, String, Boolean };
    ConstExpr(const Pos &p, ConstType ct);
    virtual ~ConstExpr() { }
    virtual std::set<std::string> GetVarLeaves()
                            { return std::set<std::string>(); }
    virtual std::set<ExprParseTreeNode *> GetVarLeafNodes()
                            { return std::set<ExprParseTreeNode *>(); }
    virtual ConstType GetConstantType() { return constType; }
  protected:
    ConstType constType;
};

class EXPR_API IntegerConstExpr : public ConstExpr
{
  public:
    IntegerConstExpr(const Pos &p, int v)
        : ConstExpr(p, ConstExpr::Integer), ExprNode(p), value(v) {}
    virtual ~IntegerConstExpr();
    virtual const std::string GetTypeName() { return "IntegerConst"; }
    virtual void PrintNode(ostream &o);
    int GetValue() { return value; }
  protected:
    int value;
};

class EXPR_API FloatConstExpr : public ConstExpr
{
  public:
    FloatConstExpr(const Pos &p, float v)
        : ConstExpr(p, ConstExpr::Float), ExprNode(p), value(v) {}
    virtual ~FloatConstExpr();
    virtual const std::string GetTypeName() { return "FloatConst"; }
    virtual void PrintNode(ostream &o);
    float GetValue() { return value; }
  protected:
    float value;
};

class EXPR_API StringConstExpr : public ConstExpr
{
  public:
    StringConstExpr(const Pos &p, std::string v)
        : ConstExpr(p, ConstExpr::String), ExprNode(p), value(v) {}
    virtual ~StringConstExpr();
    virtual const std::string GetTypeName() { return "StringConst"; }
    virtual void PrintNode(ostream &o);
    std::string GetValue() { return value; }
  protected:
    std::string value;
};

class EXPR_API BooleanConstExpr : public ConstExpr
{
  public:
    BooleanConstExpr(const Pos &p, bool v)
        : ConstExpr(p, ConstExpr::Boolean), ExprNode(p), value(v) {}
    virtual ~BooleanConstExpr();
    virtual const std::string GetTypeName() { return "BooleanConst"; }
    virtual void PrintNode(ostream &o);
    bool GetValue() { return value; }
  protected:
    bool value;
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
    virtual ~UnaryExpr() { delete expr; }
    virtual void PrintNode(ostream &o);
    virtual std::set<std::string> GetVarLeaves()
        {return expr->GetVarLeaves();}
    virtual std::set<ExprParseTreeNode *> GetVarLeafNodes()
        { return std::set<ExprParseTreeNode *>(); }
    virtual const std::string GetTypeName() { return "Unary"; }
    ExprNode *GetExpr(void) { return expr; };
  protected:
    ExprNode *expr;
};

class EXPR_API BinaryExpr : public MathExpr
{
  public:
    BinaryExpr(const Pos &p, char o, ExprNode *l, ExprNode *r)
        : MathExpr(p, o), ExprNode(p), left(l), right(r) {}
    virtual ~BinaryExpr() { delete left; delete right; }
    virtual void PrintNode(ostream &o);
    virtual std::set<std::string> GetVarLeaves();
    virtual std::set<ExprParseTreeNode *> GetVarLeafNodes();
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
    virtual ~IndexExpr() { delete expr; }
    virtual void PrintNode(ostream &o);
    virtual std::set<std::string> GetVarLeaves()
        {return expr->GetVarLeaves();}
    virtual std::set<ExprParseTreeNode *> GetVarLeafNodes()
        { return expr->GetVarLeafNodes(); }
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
    virtual ~VectorExpr() { delete x; delete y; delete z; }
    virtual void PrintNode(ostream &o);
    virtual std::set<std::string> GetVarLeaves();
    virtual std::set<ExprParseTreeNode *> GetVarLeafNodes();
    virtual const std::string GetTypeName() { return "Vector"; }
  protected:
    ExprNode *x, *y, *z;
};

class EXPR_API ListElemExpr : public ExprParseTreeNode
{
  public:
    ListElemExpr(const Pos &p, ExprNode *b, ExprNode *e=NULL, ExprNode *s=NULL)
        : ExprParseTreeNode(p), beg(b), end(e), skip(s) {}
    virtual ~ListElemExpr() { delete beg; delete end; delete skip; }
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
    virtual ~ListExpr();
    void AddListElem(ListElemExpr *e);
    virtual void PrintNode(ostream &o);
    virtual const std::string GetTypeName() { return "List"; }
    std::vector<ListElemExpr*> *GetElems(void) { return elems; }
    virtual std::set<std::string> GetVarLeaves();
    virtual std::set<ExprParseTreeNode *> GetVarLeafNodes();
  protected:
    std::vector<ListElemExpr*> *elems;
};

class EXPR_API ArgExpr : public ExprParseTreeNode
{
  public:
    ArgExpr(const Pos &p, ExprParseTreeNode *e, const std::string &t)
        : ExprParseTreeNode(p), identifier(""), expr(e), text(t) {}
    virtual ~ArgExpr() { delete expr; }
    ArgExpr(const Pos &p, std::string id, ExprParseTreeNode *e,
            const std::string &t)
        : ExprParseTreeNode(p), identifier(id), expr(e), text(t) {}
    virtual void PrintNode(ostream &o);
    ExprParseTreeNode *GetExpr(void) {return expr;};
    virtual const std::string GetTypeName() { return "Arg"; }
    const std::string &GetText(void) const { return text; };
  protected:
    std::string identifier;
    ExprParseTreeNode *expr;
    std::string text;
};

class EXPR_API ArgsExpr : public ExprParseTreeNode
{
  public:
    ArgsExpr(const Pos &p, ArgExpr *e);
    virtual ~ArgsExpr();
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
    FunctionExpr(const Pos &p, std::string n, ArgsExpr *e=NULL)
        : ExprNode(p), name(n), args(e) {}
    virtual ~FunctionExpr() { delete args; }
    virtual void PrintNode(ostream &o);
    virtual std::set<std::string> GetVarLeaves();
    virtual std::set<ExprParseTreeNode *> GetVarLeafNodes();
    virtual const std::string GetTypeName() { return "Function"; }
  protected:
    std::string name;
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
    MachExpr(const Pos &p, std::string h)
        : ExprParseTreeNode(p), host(h) {}
    virtual ~MachExpr() { }
    virtual void PrintNode(ostream &o);
    virtual const std::string GetTypeName() { return "Mach"; }
  protected:
    std::string host;
};

class EXPR_API TimeExpr : public ExprParseTreeNode
{
  public:
    enum Type { Cycle, Time, Index, Unknown };
    TimeExpr(const Pos &p, ListExpr *l, Type t=Unknown)
        : ExprParseTreeNode(p), type(t), list(l) { isDelta = false; }
    virtual ~TimeExpr() { delete list; }
    virtual void PrintNode(ostream &o);
    virtual const std::string GetTypeName() { return "Time"; }
    bool GetIsDelta(void) { return isDelta; };
    void SetIsDelta(bool b) { isDelta = b; };
    Type GetType(void) { return type; };
    ListExpr *GetList(void) { return list; };
  protected:
    Type      type;
    bool      isDelta;
    ListExpr *list;
};

class EXPR_API DBExpr : public ExprParseTreeNode
{
  public:
    DBExpr(const Pos &p, PathExpr *f, MachExpr *m, TimeExpr *t)
        : ExprParseTreeNode(p), file(f), mach(m), time(t) {}
    virtual ~DBExpr() { delete file; delete mach; delete time; }
    virtual void PrintNode(ostream &o);
    virtual const std::string GetTypeName() { return "DBExpr"; }
    PathExpr *GetFile(void) { return file; };
    TimeExpr *GetTime(void) { return time; };
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
    virtual ~VarExpr() { delete db; delete var; }
    virtual void PrintNode(ostream &o);
    virtual std::set<std::string> GetVarLeaves();
    virtual std::set<ExprParseTreeNode *> GetVarLeafNodes();
    virtual const std::string GetTypeName() { return "Var"; }
    PathExpr *GetVar(void) { return var; };
    DBExpr   *GetDB(void) { return db; };
    static bool      GetVarLeavesRequiresCurrentDB(void);
    static void      SetGetVarLeavesRequiresCurrentDB(bool);
  protected:
    DBExpr   *db;
    PathExpr *var;
    bool      canexpand;
    static bool    getVarLeavesRequiresCurrentDB;
};


#endif
