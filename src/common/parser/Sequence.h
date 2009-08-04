/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef SEQUENCE_H
#define SEQUENCE_H
#include <parser_exports.h>

#include "Symbol.h"
#include "SymbolSet.h"

class Rule;

// ****************************************************************************
//  Class:  Sequence
//
//  Purpose:
//    A series of terminals and nonterminals.  Used as the RHS of a
//    production (Rule).
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Jeremy Meredith, Wed Nov  3 12:16:49 PST 2004
//    Added prototypes before the friend declarations.
//
//    Brad Whitlock, Tue Jan 4 14:37:51 PST 2005
//    Added API macros to the friend function declarations.
//
// ****************************************************************************

// Some compilers (xlC) require prototypes before friend declarations
class Sequence;
PARSER_API Sequence operator+(const Symbol&, const Symbol&);
PARSER_API Sequence operator+(const Sequence&, const Symbol&);

class PARSER_API Sequence
{
  public:
    Sequence();
    Sequence(const Sequence&);
    Sequence(const Symbol&);

    void AddSymbol(const Symbol&);

    int           Length()          const { return len;        }
    bool          Empty()           const { return len == 0;   }
    const Symbol *operator[](int i) const { return symbols[i]; }

    void Print(ostream &o, int pos = -1) const;
    bool operator==(const Sequence &s) const;

    bool          IsNullable(const std::vector<const Rule*> &rules) const;
    SymbolSet     GetFirstSet(const std::vector<const Rule*> &rules) const;
    Sequence      GetSubsequence(int pos) const;
    const Symbol *GetLastTerminal() const;

    friend PARSER_API Sequence operator+(const Symbol&, const Symbol&);
    friend PARSER_API Sequence operator+(const Sequence&, const Symbol&);
  protected:
    int len;
    std::vector<const Symbol*> symbols;
};

#endif
