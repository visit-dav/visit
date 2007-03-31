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
