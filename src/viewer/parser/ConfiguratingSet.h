#ifndef CONFIGURATINGSET_H
#define CONFIGURATINGSET_H
#include <viewer_parser_exports.h>

#include <vector>
#include <map>
#include "ConfiguratingItem.h"

// ****************************************************************************
//  Class:  ConfiguratingSet
//
//  Purpose:
//    The working set of ConfiguratingItems.  When finished, each
//    configurating set (or at least its shift/reduce maps) becomes a state
//    in the final parser.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  4, 2002
//
// ****************************************************************************
class VIEWER_PARSER_API ConfiguratingSet
{
    friend struct State;
  public:
                      ConfiguratingSet();
    bool              operator==(const ConfiguratingSet&) const;

    void              AddItem(const ConfiguratingItem&);
    void              Merge(const ConfiguratingSet&);
    void              Close(const std::vector<const Rule*>&);

    SymbolSet         GetShiftSymbols();
    ConfiguratingSet  GetShiftSet(const Symbol*);

    SymbolSet         GetReduceSymbols();
    std::vector<int>  GetReduceRules(const Symbol*);

    void SetShiftTransition(const Symbol *s, int t) { shift[s]  = t;   }
    void SetReduceRule(const Symbol *s, int r)      { reduce[s] = r;   }

    void RemoveShiftTransition(const Symbol *s)     { shift.erase(s);  }
    void RemoveReduceRule(const Symbol *s)          { reduce.erase(s); }

    void SetConflict(bool c)                        { conflict = c;    }
    bool HasConflict() const                        { return conflict; }

    friend ostream &operator<<(ostream&, const ConfiguratingSet&);
  private:
    bool                           conflict;
    std::vector<ConfiguratingItem> items;

    std::map<const Symbol*, int>   shift;
    std::map<const Symbol*, int>   reduce;
};

#endif
