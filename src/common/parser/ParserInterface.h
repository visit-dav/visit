#ifndef PARSERINTERFACE_H
#define PARSERINTERFACE_H

#include <parser_exports.h>
#include <ExprGrammarNode.h>
#include <ExprNode.h>
#include <string>

class ExprNodeFactory;

// ****************************************************************************
//  Class:  ParserInterface
//
//  Purpose:
//    Abstract interface class for a parser.
//
//  Programmer:  Sean Ahern
//  Creation:    October 15, 2002
//
// ****************************************************************************
class PARSER_API ParserInterface
{
protected:
    ParserInterface() {};

public:
    virtual ~ParserInterface() {};
    static ParserInterface* MakeParser(ExprNodeFactory *f);
    virtual ExprNode *Parse(const std::string &) = 0;
};

#endif
