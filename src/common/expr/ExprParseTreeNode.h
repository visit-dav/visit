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

// ************************************************************************* //
//                            ExprParseTreeNode.h                            //
// ************************************************************************* //

#ifndef EXPRPARSETREENODE_H
#define EXPRPARSETREENODE_H

#include <expr_exports.h>

#include <vector>
#include <Pos.h>
#include <set>

#include <ParseTreeNode.h>

class Identifier;
class Token;

// ****************************************************************************
//  Class:  ExprParseTreeNode
//
//  Purpose:
//    Base class for all grammar nodes in VisIt expression trees.
//
//  Programmer:  Sean Ahern
//  Creation:    Thu Apr 18 11:22:27 PDT 2002
//
//  Modifications:
//
//    Hank Childs, Thu Aug 14 08:21:14 PDT 2003
//    Un-inlined Print method, since it contained statics.
//
//    Jeremy Meredith, Thu Nov 11 12:35:11 PST 2004
//    Refactored the non-expression related things to another class.
//
//    Hank Childs, Thu Sep  8 15:37:05 PDT 2005
//    Added method GetVarLeafNodes.
//
// ****************************************************************************
class EXPR_API ExprParseTreeNode : public ParseTreeNode
{
  public:
    ExprParseTreeNode(const Pos &p) : ParseTreeNode(p) { }
    virtual ~ExprParseTreeNode() { }
    virtual const std::string GetTypeName() {return "ExprParseTreeNode";}
    virtual std::set<std::string> GetVarLeaves() 
                                            {return std::set<std::string>();}
    virtual std::set<ExprParseTreeNode *> GetVarLeafNodes() 
                                      {return std::set<ExprParseTreeNode *>();}
};

#endif
