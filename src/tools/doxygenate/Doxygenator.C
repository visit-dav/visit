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

// ************************************************************************* //
//                                Doxygenator.C                              //
// ************************************************************************* //

#include <visitstream.h>
#include <stdlib.h>

#include <Doxygenator.h>
#include <DoxygenDirective.h>


// ****************************************************************************
//  Method: Doxygenator constructor
//
//  Programmer: Hank Childs
//  Creation:   August 7, 2000
//
// ****************************************************************************

Doxygenator::Doxygenator()
{
    DirectiveGenerator = NULL;
    numDirectives      = 0;
    LexLexer           = NULL;
    lexString          = NULL;
    insideMethod       = 0;
    insideClass        = false;
    reachedEnd         = false; 
}


// ****************************************************************************
//  Method: Doxygenator::RegisterDirectives
//
//  Purpose:
//      Registers a function that creates doxygen directives.  This is what
//      customizes doxygenate.
//
//  Arguments:
//      dg      The directive generator function.
//      nd      The number of different directive the generator function
//              generates.
//
//  Programmer: Hank Childs
//  Creation:   August 7, 2000
//
// ****************************************************************************

void
Doxygenator::RegisterDirectives(DoxygenDirective* (*dg)(int), int nd)
{
    DirectiveGenerator = dg;
    numDirectives      = nd;
}


// ****************************************************************************
//  Method: Doxygenator::RegisterLexRoutines
//
//  Purpose:
//      Since this files cannot access standard lex routines, register the
//      ones we need here.
//
//  Arguments:
//      yylex   This should be the yylex function.
//      yytext  This should be the yytext pointer.
//
//  Programmer: Hank Childs
//  Creation:   August 7, 2000
//
// ****************************************************************************

void
Doxygenator::RegisterLexRoutines(int (*yylex)(void), char *yytext)
{
    LexLexer  = yylex;
    lexString = yytext;
}


// ****************************************************************************
//  Method: Doxygenator::Execute
//
//  Purpose:
//      Performs the actual doxygenation.  This means that it reads in the 
//      input using lex constructs and outputs the source modified for 
//      doxygen and in accordance to the registered directives.
//
//  Programmer: Hank Childs
//  Creation:   August 7, 2000
//
// ****************************************************************************

void
Doxygenator::Execute(void)
{
    if (DirectiveGenerator == NULL || LexLexer == NULL || lexString == NULL)
    {
        cerr << "Did not register necessary functions.  Cannot execute."
             << endl;
        return;
    }

    //
    // Set up the current stream as the first one.
    //
    DoxygenDirective *curStream = DirectiveGenerator(0);

    //
    // Process all of the tokens until EOF, GetNextToken exits the next
    // invocation after the EOF is reached (this way all the streams can close
    // off normally).
    //
    for (;;)
    {
        char     *expr;
        DoxToken  token = GetNextToken(&expr);

        //
        // Stream in the current token and expression.
        //
        curStream->StreamToken(token, expr);

        if (curStream->LifetimeOver())
        {
            DoxToken  *tokens;
            char     **expr;
            int        numTokens = curStream->ReturnTokenStack(&tokens, &expr);
            DoxygenDirective *newStream = FindStream(numTokens, tokens, expr);
            delete curStream;
            curStream = newStream;
        }
    }
}


// ****************************************************************************
//  Method: Doxygenator::FindStream
//
//  Purpose:
//      Finds a stream that can handle the token stream.  Removes tokens from
//      the front of the stream until a DoxygenDirective stream can be found
//      that accepts it.
//
//  Arguments:
//      numTokens   The number of tokens waiting to be processed.
//      tokens      A list of DoxTokens the size of numTokens.
//      exprs       A list of the expression strings corresponding to tokens.
//
//  Returns:    A valid DoxygenDirective stream object
//
//  Programmer: Hank Childs
//  Creation:   March 24, 2000
//
// ****************************************************************************

DoxygenDirective *
Doxygenator::FindStream(int numTokens, DoxToken *tokens, char **exprs)
{
    //
    // Base case
    //
    if (numTokens == 0)
    {
        return DirectiveGenerator(0);
    }

    //
    // Try each stream until a match is found.  If there is no
    // match, throw out tokens until there is a match or no
    // tokens left.
    //
    DoxygenDirective *stream = NULL;
    for (int i = 0 ; i < numDirectives ; i++)
    {
        //
        // Try to find a new candidate to take this stack.
        //
        stream = DirectiveGenerator(i);

        for (int j = 0 ; j < numTokens ; j++)
        {
            stream->StreamToken(tokens[j], exprs[j]);
            if (stream->LifetimeOver())
            {
                //
                // See if it could take any of the tokens.
                //
                DoxToken  *ltokens;
                char     **lexprs;
                int   lnumTokens = stream->ReturnTokenStack(&ltokens, &lexprs);
                if (lnumTokens != j+1)
                {
                    //
                    // This stream accepted some of the tokens.  Recurse.
                    //
                    DoxygenDirective *lStream;
                    lStream = FindStream(lnumTokens, ltokens, lexprs);
                    delete stream;
                    return lStream;
                }
            }
        }
        if (!stream->LifetimeOver())
        {
            //
            // We found a stream that could handle the input
            // stack.
            //
            return stream;
        }

        delete stream;
    }

    //
    // Remove the first token from the stack and recurse.
    //
    if (!insideMethod && tokens[0] == TK_COMMENT)
    {
        //
        // This is part of the function prologue and needs to be thrown
        // out so that doxygen can read in a block of directives.
        // (Other comments in between screws it up.)
        //
        while (numTokens > 0 && tokens[0] != TK_NEWLINE)
        {
            tokens++;
            exprs++;
            numTokens--;
        }
        return FindStream(numTokens, tokens, exprs);
    }
    cout << exprs[0];
    tokens++;
    exprs++;
    numTokens--;
    return FindStream(numTokens, tokens, exprs);
}


// ****************************************************************************
//  Method: Doxygenator::GetNextToken
//
//  Purpose:
//      Makes the lex call (yylex) to get the next token.  Also maintains
//      internal bookkeeping about whether we are in a method or class.
//
//  Arguments:
//      expr    A pointer to a string.  The expression read is copied here.
//
//  Returns:    The token that expression represents.
//
//  Programmer: Hank Childs
//  Creation:   March 24, 2000
//
// ****************************************************************************

DoxToken
Doxygenator::GetNextToken(char **expr)
{
    //
    // Want to send one last token to clean up if we have reached the end.
    // If we have already done that, exit.
    //
    if (reachedEnd)
    {
        exit(EXIT_SUCCESS);
    }

    //
    // Call yylex to do the work.  We cannot directly access it since this is
    // not a lex file.
    //  yylex  == LexLexer
    //  yytext == lexString
    //
    int  token = LexLexer();
    *expr      = lexString;

    //
    // Must determine if we are inside a method so that we can figure out
    // whether or not to throw away comments.  Also determine if we are in
    // a class declaration, which will require we put make the comments for
    // the fields something that doxygen can recognize.
    //
    if (token == TK_LEFT_BRACE)
    {
        insideMethod++;
        token = TK_OTHER;
    }
    else if (token == TK_RIGHT_BRACE)
    {
        insideMethod--;
        if (insideMethod == 0)
        {
            insideClass = false;
        }
        token = TK_OTHER;
    }
    else if (token == TK_CLASS)
    {
        insideClass = true;
        token = TK_OTHER;
    }

    //
    // Comments inside classes get transformed to doxygen comments
    // automatically.
    //
    if (token == TK_COMMENT && insideClass)
    {
        *expr = "///";
    }

    //
    // We have hit the end of the file.  Send a new line so that everyone can
    // clean up.  Mark that we have hit the EOF, so we can exit next time
    // through.
    //
    if (token == 0)
    {
        // Send a dummy new line in.
        token = TK_NEWLINE;
        *expr = "";
        reachedEnd = true;
    }
 
    return static_cast< DoxToken >(token);
}


