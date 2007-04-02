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

#ifndef SYMBOL_H
#define SYMBOL_H
#include <parser_exports.h>

#include <vector>
#include <string>
#include <map>
#include <visitstream.h>

#define MAXSYMBOLS 64
struct SymbolSet;
class  Rule;
class  Dictionary;

// ****************************************************************************
//  Class:  Symbol
//
//  Purpose:
//    Used for terminals and nonterminals when expressing and
//    parsing a grammar.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  4, 2002
//
//  Modifications:
//    Jeremy Meredith, Wed Nov 24 12:04:23 PST 2004
//    Added a new constructor due to some major refactoring.
//
//    Jeremy Meredith, Wed Jun  8 11:28:01 PDT 2005
//    Moved static data to a new Dictionary object.
//
// ****************************************************************************
class PARSER_API Symbol
{
  public:
    enum Type { Terminal, NonTerminal };

  public:
    Symbol(Dictionary&,int tt);
    Symbol(Dictionary&,int tt, const std::string &s);
    Symbol(Dictionary&,const std::string &s);

    bool      operator==(const Symbol &rhs) const;
    bool      IsNullable(const std::vector<const Rule*>&)  const;
    SymbolSet GetFirstSet(const std::vector<const Rule*>&) const;

    int  GetIndex()      const { return index;               }
    bool IsTerminal()    const { return type == Terminal;    }
    bool IsNonTerminal() const { return type == NonTerminal; }

    friend ostream &operator<<(ostream&, const Symbol&);

    int         GetTerminalType()  const { return terminaltype;  }
    std::string GetDisplayString() const { return displaystring; }
  private:
    Type         type;
    int          terminaltype;
    std::string  displaystring;
    int          index;
};

#endif
