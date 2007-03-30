// ************************************************************************* //
//                                 Doxygenator.h                             //
// ************************************************************************* //

#ifndef DOXYGENATOR_H
#define DOXYGENATOR_H


#include <Tokens.h>


class DoxygenDirective;


// ****************************************************************************
//  Class: Doxygenator
//
//  Purpose:
//      This module is really only a group of functions.  It allows for the
//      doxygenate program to be more customizable by pushing the actual
//      doxygenating method out of the lex file, leaving only the customizable
//      information in that file.
//
//  Programmer: Hank Childs
//  Creation:   August 7, 2000
//
// ****************************************************************************

class Doxygenator
{
  public:
                          Doxygenator();

    void                  RegisterDirectives(DoxygenDirective* (*)(int), int);
    void                  RegisterLexRoutines(int (*)(void), char *);

    void                  Execute(void);

  protected:
    DoxygenDirective*   (*DirectiveGenerator)(int);
    int                   numDirectives;
    int                   insideMethod;
    bool                  insideClass;
    bool                  reachedEnd;

    int                 (*LexLexer)(void);
    char                 *lexString;

    DoxygenDirective     *FindStream(int, DoxToken *, char **);
    DoxToken              GetNextToken(char **);
};


#endif


