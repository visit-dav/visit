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

#ifndef CONFIGURATINGITEM_H
#define CONFIGURATINGITEM_H
#include <parser_exports.h>

class ConfiguratingSet;
#include "SymbolSet.h"
#include "Rule.h"

// ****************************************************************************
//  Class:  ConfiguratingItem
//
//  Purpose:
//    An element of a configurating set.  Contains a rule, a position within
//    the rule, and a follow set of terminals.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  4, 2002
//
// ****************************************************************************
class PARSER_API ConfiguratingItem
{
  public:
    ConfiguratingItem(const ConfiguratingItem &ci);
    ConfiguratingItem(const Rule *, const Symbol&);
    ConfiguratingItem(const Rule *, const SymbolSet&);

    ConfiguratingSet CreateClosure(const std::vector<const Rule*> &rules);

    const Rule      *GetRule()   const { return rule;                           }
    const Symbol    *GetNextSymbol()   { return rule->GetRHS()[pos];            }
    const SymbolSet &GetFollow() const { return follow;                         }
    bool             CanReduce() const { return pos == rule->GetRHS().Length(); }
    void             Shift()           { pos++;                                 }

    bool          CanMerge(const ConfiguratingItem &rhs);
    void          Merge(const ConfiguratingItem &rhs);

    bool operator==(const ConfiguratingItem&) const;
    friend ostream &operator<<(ostream&, const ConfiguratingItem&);
  private:
    const Rule    *rule;
    SymbolSet      follow;
    int            pos;
};

#endif
