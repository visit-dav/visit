#include <stdio.h>
#include <ExprNode.h>
#include <ExprToken.h>
#include <DebugStream.h>
#include <ExpressionException.h>
#include <snprintf.h>

using     std::vector;

//    Jeremy Meredith, Wed Nov 24 11:46:43 PST 2004
//    Made expression language specific tokens have a more specific
//    base class.  Renamed GrammarNode to ParseTreeNode.
//


// class ConstExpr
ConstExpr::ConstExpr(const Pos &p, ExprToken *t) : ExprNode(p)
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
UnaryExpr::PrintNode(ostream &o)
{
    o << "Unary operation: '"<<op<<"'" << endl;
    expr->Print(o,"Expr: ");
}

void
BinaryExpr::PrintNode(ostream &o)
{
    o << "Binary operation: '"<<op<<"'" << endl;
    left->Print(o,"Left:  ");
    right->Print(o,"Right: ");
}

std::set<std::string>
BinaryExpr::GetVarLeaves()
{
    std::set<std::string> lset = left->GetVarLeaves();
    std::set<std::string> rset = right->GetVarLeaves();

    while (!rset.empty())
    {
        std::set<std::string>::iterator i = rset.begin();
        lset.insert(*i);
        rset.erase(i);
    }

    return lset;
}

void
IndexExpr::PrintNode(ostream &o)
{
    o << "Index operation: (" << ":" << endl;
    expr->Print(o, "Expr:  ");
    o << "    index: " << ind << endl;
}

void
VectorExpr::PrintNode(ostream &o)
{
    o << "Vector: " << (z ? "3D" : "2D") << endl;
    x->Print(o);
    y->Print(o);
    if (z) z->Print(o);
}

// ****************************************************************************
//  Method:  VectorExpr::GetVarLeaves
//
//  Programmer:  Sean Ahern
//
//  Modifications:
//    Jeremy Meredith, Thu Aug 14 10:24:14 PDT 2003
//    Allow 2D vectors.
//
// ****************************************************************************
std::set<std::string>
VectorExpr::GetVarLeaves()
{
    std::set<std::string> xset = x->GetVarLeaves();
    std::set<std::string> yset = y->GetVarLeaves();
    std::set<std::string> zset;
    if (z)
        zset = z->GetVarLeaves();

    while (!yset.empty())
    {
        std::set<std::string>::iterator i = yset.begin();
        xset.insert(*i);
        yset.erase(i);
    }

    while (!zset.empty())
    {
        std::set<std::string>::iterator i = zset.begin();
        xset.insert(*i);
        zset.erase(i);
    }

    return xset;
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
ListExpr::ListExpr(const Pos &p, ListElemExpr *e) : ExprParseTreeNode(p)
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

std::set<std::string>
ListExpr::GetVarLeaves()
{
    if (elems->size() != 0)
        return (*elems)[0]->GetVarLeaves();
    else
        return std::set<std::string>();
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
ArgsExpr::ArgsExpr(const Pos &p, ArgExpr *e) : ExprParseTreeNode(p)
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

void
FunctionExpr::PrintNode(ostream &o)
{
    o << "Function '" << name->GetVal().c_str() << "' with ";
    if (args)
        args->PrintNode(o);
    else
        o << "no arguments\n";
}

// ****************************************************************************
//  Method:  FunctionExpr::GetVarLeaves
//
//  Programmer:  Sean Ahern
//
//  Modifications:
//    Jeremy Meredith, Mon Aug 18 12:00:44 PDT 2003
//    Allow empty argument lists.
//
// ****************************************************************************
std::set<std::string>
FunctionExpr::GetVarLeaves()
{
    std::set<std::string> ret;

    if (!args)
        return ret;

    std::vector<ArgExpr*> *a = args->GetArgs();

    for (int i = 0; i < a->size(); i++)
    {
        std::set<std::string> vars = (*a)[i]->GetExpr()->GetVarLeaves();

        while (!vars.empty())
        {
            std::set<std::string>::iterator i = vars.begin();
            ret.insert(*i);
            vars.erase(i);
        }
    }

    return ret;
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

void
MachExpr::PrintNode(ostream &o)
{
    o << "Machine="<<host->GetVal().c_str()<<endl;
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

std::set<std::string>
VarExpr::GetVarLeaves()
{
    std::set<std::string> ret;

    ret.insert(var->GetFullpath());

    return ret;
}
