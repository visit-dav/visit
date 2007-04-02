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

#ifndef GRAMMAR_H
#define GRAMMAR_H
#include <parser_exports.h>

#include "Symbol.h"
#include "Rule.h"
#include "ConfiguratingSet.h"
#include "State.h"
#include "Dictionary.h"

// ****************************************************************************
//  Class:  Grammar
//
//  Purpose:
//    Class implementing a generic LR(1) grammar.  Rules, symbols, and
//    associativities and precedences are added.  After this, the grammar
//    may be used several ways:
//        1) Call Configure, then use the grammar in a parser
//        2) Call Configure, then write out the initalization code to a file
//        3) Link this initalization code with the project and call it
//           instead of Configure before using it in a parser.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Jeremy Meredith, Wed Jun  8 17:07:55 PDT 2005
//    Added a symbol dictionary.
//
// ****************************************************************************
class PARSER_API Grammar
{
  public:
    enum Associativity { Left, Right, NonAssoc };

                   Grammar(Dictionary&);
    virtual       ~Grammar();
    void           Print(ostream&);

    void           SetAssoc(const Symbol&, Associativity);
    void           SetPrec(const Symbol&, int);

    void           AddRule(const Rule&, int prec=-1);
    void           SetStartSymbol(const Symbol&);

    bool           Configure();
    virtual bool   Initialize() = 0;

    Dictionary    &GetDictionary();
    const Symbol  *GetStartSymbol();
    const Rule    *GetRule(int i);
    State         &GetState(int i);

    void           SetPrinter(ostream *o);

    void           WriteStateInitialization(const std::string&, ostream&);

  protected:
    Dictionary &dictionary;
    ostream *out;

    Symbol eof;
    Symbol start;

    Rule                                   startrule;
    std::vector<const Rule*>               rules;
    std::map<const Symbol*, int>           prec;
    std::map<const Symbol*, Associativity> assoc;
    std::vector<ConfiguratingSet>          sets;

    std::vector<State>                     states;
};

#endif
