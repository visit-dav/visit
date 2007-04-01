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
