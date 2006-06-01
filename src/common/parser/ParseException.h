/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

#ifndef PARSEEXEPTION_H
#define PARSEEXEPTION_H

#include "Pos.h"
#include <VisItException.h>
#include <snprintf.h>
class Rule;

// ****************************************************************************
//  Class:  ParseException
//
//  Purpose:
//    Classes for exceptions thrown by parsing expressions.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Jeremy Meredith, Mon Jul 28 16:57:42 PDT 2003
//    Added a little extra info to the syntax error messages.
//
//    Jeremy Meredith, Fri Aug 15 12:49:53 PDT 2003
//    Changed the error messages to be a little friendlier.
//
//    Jeremy Meredith, Wed Nov 24 11:56:52 PST 2004
//    Added a SyntacticException with no specific token type name, for the
//    times that information is not readily available.
//
//    Hank Childs, Fri Jan 28 14:57:14 PST 2005
//    Inherit from VisItException.
//
// ****************************************************************************
class ParseException : public VisItException
{
  public:
    ParseException(Pos p) : pos(p) { }
    virtual const char *Message() = 0;
    Pos GetPos() { return pos; }
  private:
    Pos pos;
};

class LexicalException : public ParseException
{
  public:
    LexicalException(Pos p) : ParseException(p) { }
    virtual const char *Message() { return "The text scanner encountered an unexpected character:"; }
};

class SyntacticException : public ParseException
{
  public:
    SyntacticException(Pos p) : ParseException(p)
    {
        SNPRINTF(msg, 1024, "The expression parser encountered an unexpected token:");
    }
    SyntacticException(Pos p, const std::string &s) : ParseException(p)
    {
        SNPRINTF(msg, 1024, "The expression parser encountered an unexpected token (%s):", s.c_str());
    }
    virtual const char *Message() { return msg; }
  private:
    char msg[1024];
};

class UnexpectedEndException : public ParseException
{
  public:
    UnexpectedEndException(Pos p) : ParseException(p) { }
    virtual const char *Message() { return "The parser expected more stuff after the end of the expression:"; }
};

class SemanticException : public ParseException
{
  public:
    SemanticException(Pos p) : ParseException(p) { }
    virtual const char *Message() { return "Semantic error:"; }
};

class UnhandledReductionException : public ParseException
{
  public:
    UnhandledReductionException(Pos p, const Rule *r) : ParseException(p), rule(r) { }
    virtual const char *Message() { return "Parse error -- unhandled reduction:"; }
    virtual const Rule *GetRule() { return rule; }
  private:
    const Rule *rule;
};

#endif
