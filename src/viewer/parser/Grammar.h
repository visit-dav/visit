#ifndef GRAMMAR_H
#define GRAMMAR_H
#include <viewer_parser_exports.h>

#include "Symbol.h"
#include "Rule.h"
#include "ConfiguratingSet.h"
#include "State.h"

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
// ****************************************************************************
class VIEWER_PARSER_API Grammar
{
  public:
    enum Associativity { Left, Right, NonAssoc };

                   Grammar();
    virtual       ~Grammar();
    void           Print(ostream&);

    void           SetAssoc(const Symbol&, Associativity);
    void           SetPrec(const Symbol&, int);

    void           AddRule(const Rule&, int prec=-1);
    void           SetStartSymbol(const Symbol&);

    bool           Configure();
    virtual bool   Initialize() = 0;

    const Symbol  *GetStartSymbol();
    const Rule    *GetRule(int i);
    State         &GetState(int i);

    void           SetPrinter(ostream *o);

    void           WriteStateInitialization(const std::string&, ostream&);

  protected:
    ostream *out;

    static Symbol eof;
    static Symbol start;

    Rule                                   startrule;
    std::vector<const Rule*>               rules;
    std::map<const Symbol*, int>           prec;
    std::map<const Symbol*, Associativity> assoc;
    std::vector<ConfiguratingSet>          sets;

    std::vector<State>                     states;
};

#endif
