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

#ifndef EXPRPARSER_H
#define EXPRPARSER_H
#include <expr_exports.h>

#include <VisItParser.h>
#include <ExprScanner.h>
#include <string>
#include <ExprNode.h>
#include <ExprNodeFactory.h>

// ****************************************************************************
//  Class:  ExprParser
//
//  Purpose:
//    Implementation of a parser for the Expression grammar.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Brad Whitlock, Fri Jun 28 15:20:36 PST 2002
//    Changed the name of the base class.
//
//    Jeremy Meredith, Mon Jul 28 16:54:47 PDT 2003
//    Add the ability to set the error reporting message target to 
//    the console, the viewer error message mechanism, or neither.
//
//    Jeremy Meredith, Fri Aug 15 09:25:04 PDT 2003
//    Renamed EMT_VIEWER to EMT_COMPONENT and added EMT_EXCEPTION.
//
//    Jeremy Meredith, Wed Nov 24 11:55:13 PST 2004
//    Refactored.  There's a new base class for the ExprParser and the
//    return types became more general.
//
//    Jeremy Meredith, Tue Dec 28 11:18:09 PST 2004
//    Added the current parse text as a data member to help with macro
//    support.
//
// ****************************************************************************

class EXPR_API ExprParser : public Parser
{
  public:
    ExprParser(ExprNodeFactory *f);
    ParseTreeNode *Parse(const std::string &);

    enum ErrorMessageTarget
    {
        EMT_NONE,
        EMT_CONSOLE,
        EMT_COMPONENT,
        EMT_EXCEPTION
    };
    static void SetErrorMessageTarget(const ErrorMessageTarget emt)
    {
        errorMessageTarget = emt;
    }

  protected:
    ParseTreeNode *ApplyRule(const Symbol&, const Rule*,
                           std::vector<ParseTreeNode*>&,
                           std::vector<Token*>&, Pos);

  private:
    std::string text; // the current parse text

    ExprScanner scanner;
    ExprNodeFactory *factory;
    static ErrorMessageTarget errorMessageTarget;
};

#endif
