#include <stdio.h>
#include <ExprNode.h>
#include <Token.h>
#include <EngineProxy.h>
#include <ViewerPlot.h>
#include <DebugStream.h>
#include <ExpressionException.h>
#include <ViewerEngineManager.h>
#include <snprintf.h>

using     std::vector;

// class ConstExpr
ConstExpr::ConstExpr(const Pos &p, Token *t) : ExprNode(p)
{
    token = t;
}

void
ConstExpr::PrintNode(ostream &o)
{
    o << "Constant ";
    token->PrintNode(o);
}

void
ConstExpr::GeneratePlot(EngineProxy *engine, ViewerPlot *plot, const int time)
{
    debug1 << "ConstExpr::GeneratePlot: NOT YET IMPLEMENTED" << endl;
}

// class MathExpr
MathExpr::MathExpr(const Pos &p, char o) : ExprNode(p)
{
    op = o;
}

// class UnaryExpr
UnaryExpr::UnaryExpr(const Pos &p, char o, ExprNode *e) : MathExpr(p, o)
{
    expr = e;
}

void
UnaryExpr::PrintNode(ostream &o)
{
    o << "Unary operation: '"<<op<<"'" << endl;
    expr->Print(o,"Expr: ");
}

void
UnaryExpr::GeneratePlot(EngineProxy *engine, ViewerPlot *plot, const int time)
{
    expr->GeneratePlot(engine, plot, time);
    char opstring[2];
    opstring[0] = op;
    opstring[1] = '\0';
    engine->ApplyNamedFunction(opstring,1);
}

// class BinaryExpr
BinaryExpr::BinaryExpr(const Pos &p, char o, ExprNode *l, ExprNode *r) : MathExpr(p, o)
{
    left = l;
    right = r;
}

void
BinaryExpr::PrintNode(ostream &o)
{
    o << "Binary operation: '"<<op<<"'" << endl;
    left->Print(o,"Left:  ");
    right->Print(o,"Right: ");
}

void
BinaryExpr::GeneratePlot(EngineProxy *engine, ViewerPlot *plot, const int time)
{
    left->GeneratePlot(engine, plot, time);
    right->GeneratePlot(engine, plot, time);
    char opstring[2];
    opstring[0] = op;
    opstring[1] = '\0';
    engine->ApplyNamedFunction(opstring,2);
}

// class VectorExpr
VectorExpr::VectorExpr(const Pos &p, ExprNode *xe, ExprNode *ye, ExprNode *ze)
    : ExprNode(p)
{
    x = xe;
    y = ye;
    z = ze;
}

void
VectorExpr::PrintNode(ostream &o)
{
    o << "Vector: " << (z ? "3D" : "2D") << endl;
    x->Print(o);
    y->Print(o);
    if (z) z->Print(o);
}

void
VectorExpr::GeneratePlot(EngineProxy *engine, ViewerPlot *plot, const int time)
{
    debug1 << "VectorExpr::GeneratePlot: NOT YET IMPLEMENTED" << endl;
}

// class ListElemExpr
ListElemExpr::ListElemExpr(const Pos &p, ExprNode *b, ExprNode *e,
                           ExprNode *s) : ExprGrammarNode(p)
{
    beg  = b;
    end  = e;
    skip = s;
}

void
ListElemExpr::PrintNode(ostream &o)
{
    o << "List element: " << endl;
    beg->Print(o,"Beg:  ");
    if (end)  end->Print(o,"End:  ");
    if (skip) skip->Print(o,"Skip: ");
}

// class ListExpr
ListExpr::ListExpr(const Pos &p, ListElemExpr *e) : ExprGrammarNode(p)
{
    elems = new vector<ListElemExpr*>;
    elems->push_back(e);
}

void
ListExpr::AddListElem(ListElemExpr *e)
{
    elems->push_back(e);
}

void
ListExpr::PrintNode(ostream &o)
{
    o << "List size="<<elems->size()<<":" << endl;
    for (int i=0; i<elems->size(); i++)
    {
        char tmp[256];
        SNPRINTF(tmp, 256, "Element % 2d: ", i);
        (*elems)[i]->Print(o,tmp);
    }
}

// class ArgExpr
ArgExpr::ArgExpr(const Pos &p, ExprNode *e) : ExprGrammarNode(p)
{
    id = NULL;
    expr = e;
}

ArgExpr::ArgExpr(const Pos &p, Identifier *i, ExprNode *e) : ExprGrammarNode(p)
{
    id = i;
    expr = e;
}

void
ArgExpr::PrintNode(ostream &o)
{
    if (id)
    {
        o << "name='" << id->GetVal().c_str() << "':";
    }
    o << endl;
    expr->Print(o);
}

// class ArgsExpr
ArgsExpr::ArgsExpr(const Pos &p, ArgExpr *e) : ExprGrammarNode(p)
{
    args = new vector<ArgExpr*>;
    args->push_back(e);
}
void
ArgsExpr::AddArg(ArgExpr *e)
{
    args->push_back(e);
}

void
ArgsExpr::PrintNode(ostream &o)
{
    o << args->size()<<" arguments:" << endl;
    for (int i=0; i<args->size(); i++)
    {
        char tmp[256];
        SNPRINTF(tmp, 256, "Arg % 2d: ", i);
        (*args)[i]->Print(o, tmp);
    }
}

// class FunctionExpr
FunctionExpr::FunctionExpr(const Pos &p, Identifier *i, ArgsExpr *e) : ExprNode(p)
{
    name = i;
    args = e;
}

void
FunctionExpr::PrintNode(ostream &o)
{
    o << "Function '" << name->GetVal().c_str() << "' with ";
    if (args)
        args->PrintNode(o);
    else
        o << "no arguments\n";
}

void
FunctionExpr::GeneratePlot(EngineProxy *engine, ViewerPlot *plot, const int time)
{
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    std::vector<ArgExpr*>::iterator i;
    for(i=arguments->begin(); i != arguments->end(); i++)
    {
        (*i)->GetExpr()->GeneratePlot(engine, plot, time);
    }
    engine->ApplyNamedFunction(name->GetVal(),arguments->size());
}

// class PathExpr
PathExpr::PathExpr(const Pos &p, const std::string &s) : ExprGrammarNode(p)
{
    basename = s;
    fullpath = basename;
}

void
PathExpr::Append(const std::string &s)
{
    dirname += basename;
    basename = s;
    fullpath = dirname + basename;
}

void
PathExpr::PrintNode(ostream &o)
{
    o << "Name='" << basename.c_str() << "'";
    if (!dirname.empty())
        o << " Dir='" << dirname.c_str() << "'";
    o << endl;
}

// class MachExpr
MachExpr::MachExpr(const Pos &p, Identifier *i) : ExprGrammarNode(p)
{
    host = i;
}

void
MachExpr::PrintNode(ostream &o)
{
    o << "Machine="<<host->GetVal().c_str()<<endl;
}

// class TimeExpr
TimeExpr::TimeExpr(const Pos &p, ListExpr *l, Type t) : ExprGrammarNode(p)
{
    list = l;
    type = t;
}

void
TimeExpr::PrintNode(ostream &o)
{
    o << "Time type="
      << (type  == Time ? "TIME" :
          (type == Cycle ? "CYCLE" :
           (type == Index ? "INDEX" :
            "UNKNOWN")))
      << ":" << endl;
    list->Print(o);
}

// class DBExpr
DBExpr::DBExpr(const Pos &p, PathExpr *f, MachExpr *m, TimeExpr *t) : ExprGrammarNode(p)
{
    file = f;
    mach = m;
    time = t;
}
void
DBExpr::PrintNode(ostream &o)
{
    o << "Database:" << endl;
    if (file)
        file->Print(o, "File");
    if (mach)
        mach->Print(o);
    if (time)
        time->Print(o);
}

// class VarExpr
VarExpr::VarExpr(const Pos &p, DBExpr *d, PathExpr *v, bool exp) : ExprNode(p)
{
    db = d;
    var = v;
    canexpand = exp;
}

void
VarExpr::PrintNode(ostream &o)
{
    o << "Variable";
    if (canexpand)
        o << "/Alias";
    o << ":" << endl;
    if (db)
        db->Print(o);
    var->Print(o, "Var");
}

void
VarExpr::GeneratePlot(EngineProxy *engine, ViewerPlot *plot, const int time)
{
    // Tell the engine to read a single db of data.
    if (db != NULL)
    {
        debug1 << "VarExpr::GeneratePlot: Database specifications are not yet supported" << endl;
        EXCEPTION1(ExpressionException, "Database specifications are not yet supported");
    }
    engine->ReadDataObject(plot->GetDatabaseName(), var->GetFullpath(), time,
                           plot->GetSILRestriction(),
                           *ViewerEngineManager::Instance()->GetMaterialClientAtts());
}
