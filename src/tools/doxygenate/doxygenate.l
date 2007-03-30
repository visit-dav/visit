%{
#include <stdlib.h>
#include <Tokens.h>
%}
leftbrace    [{]
rightbrace   [}]
newline      [\n]
whitespace   [ \t]
charnum      [a-zA-Z0-9]
star         [\*]
id           {charnum}{charnum}*
whitestring  {whitespace}{whitespace}*
stars        {star}{star}*
%%

%{
 //
 // All of these tokens must remain and are important to doxygenate and its
 // parsing.
 //
%}
"//"               return TK_COMMENT; 
{leftbrace}        return TK_LEFT_BRACE;
{rightbrace}       return TK_RIGHT_BRACE;
{newline}          return TK_NEWLINE;
{id}               return TK_OTHER;
{whitestring}      return TK_OTHER;
{stars}            return TK_OTHER;
"class"            return TK_CLASS;

%{
 //
 // These four tokens are specific to VisIt and may be changed without 
 // affecting parsing.  They correspond to the DoxygenDirectives created in
 // CreateDoxygenDirective.
 //
%}
"Purpose:"         return TK_PURPOSE;
"Arguments:"       return TK_ARGUMENTS;
"Returns:"         return TK_RETURN;
"Note:"            return TK_WARNING;

%{
 //
 // Must have this rule to pick up everything remaining.
 //
%}
.                  return TK_OTHER;

%%
// ************************************************************************* //
//                                 Main.C                                    //
// ************************************************************************* //

#include <iostream.h>
#include <string.h>

#include <DoxygenDirective.h>
#include <Doxygenator.h>


DoxygenDirective  *CreateDoxygenDirective(int);

const int numStreams = 4;


// *************************************************************************
//  Function:  main
//
//  Purpose:
//      Uses tokenizer from lex to create doxygen directive streams and
//      transform standard MeshTV headers into doxygen directives.
//
//  Programmer: Hank Childs
//  Creation:   March 24, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Aug  7 08:30:56 PDT 2000
//    Rewrote function to use doxygenator module, which makes the lex file
//    more easily customizable.
//
// ****************************************************************************

int 
main()
{
    Doxygenator doxygenator;

    //
    // Register the routine that creates are specific doxygen directives.
    //
    doxygenator.RegisterDirectives(CreateDoxygenDirective, numStreams);

    //
    // Register the lex routines that are apparently symbols that are 
    // unavailable to other (non-lex) files.
    //
    doxygenator.RegisterLexRoutines(yylex, yytext);

    doxygenator.Execute();
}


// ****************************************************************************
//  Function: CreateDoxygenDirective
//
//  Purpose:
//      Creates a doxygen directive stream corresponding to the input number.
//
//  Note:       When adding a new stream type, you MUST update numStreams to
//              have the stream be instantiated during run-time.
//
// Antiquated:
//
//    LEX:  "Programmer:"      return TK_AUTHOR;
//    OBJ:   OneLineDoxygenDirective(TK_AUTHOR, " \\author");
//
//    LEX:  "Creation:"        return TK_CREATION;
//    OBJ:   OneLineDoxygenDirective(TK_CREATION, " \\date");
//
//  Programmer: Hank Childs
//  Creation:   March 24, 2000
//
// ****************************************************************************

DoxygenDirective *
CreateDoxygenDirective(int i)
{
    if (i == 0)
    {
        return new MultiLineDoxygenDirective(TK_PURPOSE, " ");
    }
    else if (i == 1)
    {
        return new MultiLineDoxygenDirective(TK_RETURN, " \\return");
    }
    else if (i == 2)
    {
        return new MultiLineDoxygenDirective(TK_WARNING, " \\warning");
    }
    else if (i == 3)
    {
        return new MultiLinePrefixSplitLineDoxygenDirective(TK_ARGUMENTS, 
                                                            " \\param");
    }

    //
    // No match.
    //
    return NULL;
}


