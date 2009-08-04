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

#include "ConfiguratingItem.h"
#include "ConfiguratingSet.h"
#include "Colors.h"

using     std::vector;

// ****************************************************************************
//  Copy Constructor:  ConfiguratingItem::ConfiguratingItem
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
ConfiguratingItem::ConfiguratingItem(const ConfiguratingItem &ci)
{
    rule=ci.rule;
    follow=ci.follow;
    pos=ci.pos;
}

// ****************************************************************************
//  Constructor:  ConfiguratingItem::ConfiguratingItem
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
ConfiguratingItem::ConfiguratingItem(const Rule *r, const Symbol &f)
{
    rule = r;
    follow.insert(&f);
    pos = 0;
}

// ****************************************************************************
//  Constructor:  ConfiguratingItem::ConfiguratingItem
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
ConfiguratingItem::ConfiguratingItem(const Rule *r, const SymbolSet &f)
{
    rule = r;
    follow = f;
    pos = 0;
}

// ****************************************************************************
//  Method:  ConfiguratingItem::CreateClosure
//
//  Purpose:
//    Get the list of configurating items in the closure of the current item.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
ConfiguratingSet
ConfiguratingItem::CreateClosure(const vector<const Rule*> &rules)
{
    ConfiguratingSet closure;
    Sequence s = rule->GetRHS().GetSubsequence(pos);
    if (!s.Empty() && s[0]->IsNonTerminal())
    {
        Sequence remaining = s.GetSubsequence(1);
        SymbolSet closurefollow = remaining.GetFirstSet(rules);
        if (remaining.IsNullable(rules))
        {
            closurefollow.merge(follow);
        }
        for (size_t i=0; i<rules.size(); i++)
        {
            if (s[0] == rules[i]->GetLHS())
            {
                closure.AddItem(ConfiguratingItem(rules[i], closurefollow));
            }
        }
    }
    return closure;
}

// ****************************************************************************
//  Method:  ConfiguratingItem::CanMerge
//
//  Purpose:
//    True if another item can merge with this one.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
bool
ConfiguratingItem::CanMerge(const ConfiguratingItem &rhs)
{
    return pos == rhs.pos && rule == rhs.rule;
}

// ****************************************************************************
//  Method:  ConfiguratingItem::Merge
//
//  Purpose:
//    Merge another item into this one.  Since we know the rule and
//    pos must be the same, we only merge the follow sets.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
void
ConfiguratingItem::Merge(const ConfiguratingItem &rhs)
{
    follow.merge(rhs.follow);
}

// ****************************************************************************
//  Method:  operator<<(ostream, ConfiguratingItem)
//
//  Purpose:
//    Print the ConfiguratingItem.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
ostream &operator<<(ostream &o, const ConfiguratingItem &ci)
{
    ci.rule->Print(o, ci.pos);
    o << "    \tfollow={ " << TermGreen;
    for (size_t i = 0; i<ci.follow.size(); i++)
    {
        if (ci.follow.set[i])
            o << *(ci.follow.set[i]) << " ";
    }
    o << TermReset << "}";

    return o;
}

// ****************************************************************************
//  Equality operator:  ConfiguratingItem::operator==
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
bool
ConfiguratingItem::operator==(const ConfiguratingItem &rhs) const
{
    return (pos == rhs.pos) && (rule == rhs.rule) && (follow == rhs.follow);
}

