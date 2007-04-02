/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <stdio.h>
#include <ExprNode.h>
#include <ExprToken.h>
#include <DebugStream.h>
#include <ExpressionException.h>
#include <snprintf.h>

using     std::vector;

bool    VarExpr::getVarLeavesRequiresCurrentDB = true;

//    Jeremy Meredith, Wed Nov 24 11:46:43 PST 2004
//    Made expression language specific tokens have a more specific
//    base class.  Renamed GrammarNode to ParseTreeNode.
//
//    Jeremy Meredith, Mon Jun 13 15:46:22 PDT 2005
//    Made ConstExpr abstract and split it into multiple concrete
//    base classes.  Made FunctionExpr and MachExpr use names
//    instead of Identifier tokens.  These two changes were to
//    remove Token references from the parse tree node classes.
//    Also added some destructors.
//


// class ConstExpr
ConstExpr::ConstExpr(const Pos &p, ConstType ct) : ExprNode(p)
{
    constType = ct;
}

void
IntegerConstExpr::PrintNode(ostream &o)
{
    o << "IntegerConstant: "<<value << endl;
}

IntegerConstExpr::~IntegerConstExpr()
{
}

void
FloatConstExpr::PrintNode(ostream &o)
{
    o << "FloatConstant: "<<value << endl;
}

FloatConstExpr::~FloatConstExpr()
{
}

void
StringConstExpr::PrintNode(ostream &o)
{
    o << "StringConstant: "<<value.c_str() << endl;
}

StringConstExpr::~StringConstExpr()
{
}

void
BooleanConstExpr::PrintNode(ostream &o)
{
    o << "BooleanConstant: "<<value << endl;
}

BooleanConstExpr::~BooleanConstExpr()
{
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

std::set<ExprParseTreeNode *>
BinaryExpr::GetVarLeafNodes()
{
    std::set<ExprParseTreeNode *> lset = left->GetVarLeafNodes();
    std::set<ExprParseTreeNode *> rset = right->GetVarLeafNodes();

    while (!rset.empty())
    {
        std::set<ExprParseTreeNode *>::iterator i = rset.begin();
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

std::set<ExprParseTreeNode *>
VectorExpr::GetVarLeafNodes()
{
    std::set<ExprParseTreeNode *> xset = x->GetVarLeafNodes();
    std::set<ExprParseTreeNode *> yset = y->GetVarLeafNodes();
    std::set<ExprParseTreeNode *> zset;
    if (z)
        zset = z->GetVarLeafNodes();

    while (!yset.empty())
    {
        std::set<ExprParseTreeNode *>::iterator i = yset.begin();
        xset.insert(*i);
        yset.erase(i);
    }

    while (!zset.empty())
    {
        std::set<ExprParseTreeNode *>::iterator i = zset.begin();
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

ListExpr::~ListExpr()
{
    for (int i=0; i<elems->size(); i++)
    {
        delete (*elems)[i];
    }
    delete elems;
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

std::set<ExprParseTreeNode *>
ListExpr::GetVarLeafNodes()
{
    if (elems->size() != 0)
        return (*elems)[0]->GetVarLeafNodes();
    else
        return std::set<ExprParseTreeNode *>();
}

void
ArgExpr::PrintNode(ostream &o)
{
    if (identifier != "")
    {
        o << "name='" << identifier.c_str() << "':";
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

ArgsExpr::~ArgsExpr()
{
    for (int i=0; i<args->size(); i++)
    {
        delete (*args)[i];
    }
    delete args;
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
    o << "Function '" << name.c_str() << "' with ";
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

std::set<ExprParseTreeNode *>
FunctionExpr::GetVarLeafNodes()
{
    std::set<ExprParseTreeNode *> ret;

    if (!args)
        return ret;

    std::vector<ArgExpr*> *a = args->GetArgs();

    for (int i = 0; i < a->size(); i++)
    {
        std::set<ExprParseTreeNode *> vars = (*a)[i]->GetExpr()->GetVarLeafNodes();

        while (!vars.empty())
        {
            std::set<ExprParseTreeNode *>::iterator i = vars.begin();
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
    o << "Machine="<<host.c_str()<<endl;
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

// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Tue Aug 30 13:11:51 PDT 2005
//    Only consider variables that are for the currently active database.
//    Other variables (for CMFE) are handled differently.
//
// ****************************************************************************

std::set<std::string>
VarExpr::GetVarLeaves()
{
    std::set<std::string> ret;

    if (db == NULL || !getVarLeavesRequiresCurrentDB)
        ret.insert(var->GetFullpath());

    return ret;
}

std::set<ExprParseTreeNode *>
VarExpr::GetVarLeafNodes()
{
    std::set<ExprParseTreeNode *> ret;

    if (db == NULL || !getVarLeavesRequiresCurrentDB)
        ret.insert(this);

    return ret;
}

bool
VarExpr::GetVarLeavesRequiresCurrentDB(void)
{
    return getVarLeavesRequiresCurrentDB;
}

void
VarExpr::SetGetVarLeavesRequiresCurrentDB(bool b)
{
    getVarLeavesRequiresCurrentDB = b;
}


