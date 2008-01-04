/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

// ************************************************************************* //
//                         DoxygenDirective.C                                //
// ************************************************************************* //

#include <ctype.h>
#include <visitstream.h>
#include <string.h>

#include <DoxygenDirective.h>


// ****************************************************************************
//  Method: DoxygenDirective Constructor
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2000
//
// ****************************************************************************

DoxygenDirective::DoxygenDirective()
{
    totalTokens      = 1024;
    tokenStack       = new DoxToken[totalTokens];
    exprStack        = new char*[totalTokens];
    numTokens        = 0;
    gotDirective     = false;
    receivedComment  = false;
    badToken         = false;
    replacementExpr  = NULL;
    replacementToken = TK_OTHER;
}


// ****************************************************************************
//  Method: DoxygenDirective Destructor
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2000
//
// ****************************************************************************

DoxygenDirective::~DoxygenDirective()
{
    if (tokenStack != NULL)
    {
        delete [] tokenStack;
    }
    if (exprStack != NULL)
    {
        for (int i = 0 ; i < numTokens ; i++)
        {
            if (exprStack[i] != NULL)
            {
                delete [] exprStack[i];
            }
        }
        delete [] exprStack;
    }
}


// ****************************************************************************
//  Method: DoxygenDirective::AddToTokenStack
//
//  Purpose:
//      Adds a token to the token stack.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2000
//
// ****************************************************************************

void
DoxygenDirective::AddToTokenStack(DoxToken token, char *expr)
{
    if (numTokens >= totalTokens)
    {
        totalTokens *= 2;

        //
        // Create new arrays and copy over old stacks.
        //
        DoxToken  *oldTokenStack  = tokenStack;
        char     **oldExprStack   = exprStack;
        tokenStack = new DoxToken[totalTokens];
        exprStack  = new char*[totalTokens];
        for (int i = 0 ; i < numTokens ; i++)
        {
            tokenStack[i] = oldTokenStack[i];
            exprStack[i]  = oldExprStack[i];
        }
        delete [] oldTokenStack;
        delete [] oldExprStack;
    }

    //
    // Now add the new token and expression.
    //
    tokenStack[numTokens] = token;
    exprStack[numTokens] = new char[strlen(expr) + 1];
    strcpy(exprStack[numTokens], expr);
    numTokens++;
}


// ****************************************************************************
//  Method: DoxygenDirective::InsertToken
//
//  Purpose:
//      Inserts a token into the middle of the stack. 
//
//  Note:
//      This should only be done when it is known that this is a valid stream
//      for the input stack.
//
//  Arguments:
//      position      The position to insert the token in the stack.
//      token         The token to insert.
//      expr          The expr to put along with the token.
//
//  Programmer: Hank Childs
//  Creation:   March 24, 2000
//
// ****************************************************************************

void
DoxygenDirective::InsertToken(int position, DoxToken token, char *expr)
{
    //
    // Rather than rewrite the code to add a token to the stack, add this
    // to the end so we make sure that we haven't written over our arrays
    // and then swap.
    //
    AddToTokenStack(token, expr);

    //
    // The expr stack now has a copy of the string, so use that.
    //
    expr = exprStack[numTokens-1];

    //
    // Move everything after the position down one position.
    //
    for (int i = numTokens - 1 ; i > position ; i--)
    {
        tokenStack[i] = tokenStack[i-1];
        exprStack[i]  = exprStack[i-1];
    }

    //
    // Insert the token into the proper place.
    //
    tokenStack[position] = token;
    exprStack[position]  = expr;
}


// ****************************************************************************
//  Method: DoxygenDirective::MakeDoxygenComment
//
//  Purpose:
//      Replaces the previous comment expression with a doxygen comment 
//      expression.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2000
//
// ****************************************************************************

void
DoxygenDirective::MakeDoxygenComment(void)
{
    //
    // Token stack always has a comment as the first entry.
    //
    delete [] exprStack[0];

    char *doxComment = "///";
    exprStack[0] = new char[strlen(doxComment)+1];
    strcpy(exprStack[0], doxComment);
}


// ****************************************************************************
//  Method: DoxygenDirective::ReplaceStackEntry
//
//  Purpose:
//      Replaces the specified stack entry with the new value.
//
//  Arguments:
//      expr     The expression that replaces the last expression on the stack.
// 
//  Programmer: Hank Childs
//  Creation:   March 24, 2000
//
// ****************************************************************************

void
DoxygenDirective::ReplaceStackEntry(char *expr)
{
    delete [] exprStack[numTokens-1];
    exprStack[numTokens-1] = new char[strlen(expr)+1];
    strcpy(exprStack[numTokens-1], expr);
}


// ****************************************************************************
//  Method: DoxygenDirective::LifetimeOver
//
//  Purpose:
//      Indicates if the lifetime is over.  May be overridden by derived
//      classes.
//
//  Returns:     A boolean indicating if the lifetime of this stream is over.
//
//  Programmer:  Hank Childs
//  Creation:    March 24, 2000
//
// ****************************************************************************

bool
DoxygenDirective::LifetimeOver(void)
{
    return badToken;
}


// ****************************************************************************
//  Method: DoxygenDirective::OutputAndClearExpressionString
//
//  Purpose:
//      Outputs the expression string and then calls a method to clear it.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2000
//
// ****************************************************************************

void
DoxygenDirective::OutputAndClearExpressionString(void)
{
    for (int i = 0 ; i < numTokens ; i++)
    {
        cout << exprStack[i];
    }
    ClearExpressionString();
} 


// ****************************************************************************
//  Method: DoxygenDirective::ClearExpressionString
//
//  Purpose:
//      Clears the expression string.
//
//  Programmer: Hank Childs
//  Creation:   March 24, 2000
//
// ****************************************************************************

void
DoxygenDirective::ClearExpressionString(void)
{
    for (int i = 0 ; i < numTokens ; i++)
    {
        delete [] exprStack[i];
        exprStack[i] = NULL;
    }
    numTokens = 0;
}


// ****************************************************************************
//  Method: DoxygenDirective::ReturnTokenStack
//
//  Purpose:
//      The lifetime for this doxygen directive is over.  Return the token
//      stack so that it can be fed into another doxygen directive.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2000
//
//  Returns:    The size of the stack
//
//  Arguments:
//        tokens       The place to put the token stack.
//        exprs        The place to put the expression stack.
//
//  Note:       The token stack should not be used after this object is 
//              deleted.
//
// ****************************************************************************

int
DoxygenDirective::ReturnTokenStack(DoxToken **tokens, char ***exprs)
{
    if (!LifetimeOver())
    {
        //
        // Do not allow the token stack to be taken unless the lifetime is
        // over.
        //
        return -1;
    }

    *tokens = tokenStack;
    *exprs  = exprStack;

    return numTokens;
}


// ****************************************************************************
//  Method: DoxygenDirective::EmptyLine
//
//  Purpose:
//      Looks at all of the tokens past the first one (always a comment) and
//      determines if there is any text on this line.
//
//  Returns:    true if there is no alphanumeric characters on this line,
//              false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   March 24, 2000
//
// ****************************************************************************

bool
DoxygenDirective::EmptyLine(void)
{
    for (int i = 1 ; i < numTokens ; i++)
    {
        if (tokenStack[i] != TK_OTHER && tokenStack[i] != TK_NEWLINE)
        {
            return false;
        }

        int len = strlen(exprStack[i]);
        for (int j = 0 ; j < len ; j++)
        {
            if (isalnum(exprStack[i][j]))
            {
                return false;
            }
        }
    }
    return true;
}


// ****************************************************************************
//  Method: DoxygenDirection::PositionOfFirstRealChar
//
//  Purpose:
//      Goes through the expression stack and determines what the distance
//      is to the first real character.  A real character is defined here
//      to be the first non-whitespace, non-'/' character.
//
//  Returns:    The position of the first real character.
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2000
//
// ****************************************************************************

int
DoxygenDirective::PositionOfFirstRealChar(void)
{
    int  length = 0;
    for (int i = 0 ; i < numTokens ; i++)
    {
        for (int j = 0 ; exprStack[i][j] != '\0' ; j++)
        {
            if (!isspace(exprStack[i][j]) && exprStack[i][j] != '/')
            {
                return length;
            }
            length++;
        }
    }

    return -1;
}


// ****************************************************************************
//  Method: DoxygenDirective::StreamToken
//
//  Purpose:
//      Front end to all of the derived types token streaming.  Checks to make
//      sure a comment has been received.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2000
//
//  Arguments:
//      token       The token just received.
//      expr        The corresponding string with that token.
//
// ****************************************************************************

void
DoxygenDirective::StreamToken(DoxToken token, char *expr)
{
    //
    // Always add the token to the stack so that we can return the tokens
    // if the doxygen directives lifetime ends prematurely.
    //
    AddToTokenStack(token, expr);

    if (! receivedComment)
    {
        if (token == TK_COMMENT)
        {
            receivedComment = true;
        }
        else
        {
            badToken = true;
        }
        // 
        // Return a value indicating the stream should not look at this token.
        //
        return;
    }

    RealStreamToken(token, expr);
}


// ****************************************************************************
//  Method: OneLineDoxygenDirective Constructor
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2000
//
// ****************************************************************************

OneLineDoxygenDirective::OneLineDoxygenDirective(DoxToken token, char *expr)
{
    replacementExpr  = expr;
    replacementToken = token;
    readInOneLine    = false;
}


// ****************************************************************************
//  Method: OneLineDoxygenDirective::LifetimeOver
//
//  Purpose:
//      Determines if the lifetime is over for this doxygen directive.
//
//  Returns:    A boolean indicating if the lifetime is over for this
//              doxygen directive.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2000
//
// ****************************************************************************

bool
OneLineDoxygenDirective::LifetimeOver(void)
{
    return readInOneLine || badToken;
}


// ****************************************************************************
//  Method: OneLineDoxygenDirective::RealStreamToken
//
//  Purpose:
//      Allows for streaming of tokens.
//
//  Arguments:
//      token       The token just received.
//      <unnamed>   The corresponding string with that token.
//
//  Note:       This routine will only be called if the comment token has
//              been found.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2000
//
// ****************************************************************************

void
OneLineDoxygenDirective::RealStreamToken(DoxToken token, char *)
{
    if (token == replacementToken)
    {
        gotDirective = true;
        MakeDoxygenComment();
        ReplaceStackEntry(replacementExpr);
    }
    else if (token == TK_NEWLINE)
    {
        if (gotDirective)
        {
            OutputAndClearExpressionString();
            readInOneLine = true;
        }   
        else
        {
            badToken = true;
        }
    }
    //
    // All other tokens are ok and handled by the base class stream token.
    //
}


// ****************************************************************************
//  Method: MultiLineDoxygenDirective Constructor
//
//  Programmer: Hank Childs
//  Creation:   March 24, 2000
//
// ****************************************************************************

MultiLineDoxygenDirective::MultiLineDoxygenDirective(DoxToken token, char *exp)
{
    replacementToken = token;
    replacementExpr  = exp;
    pastFirstLine    = false;
    affectsFirstLine = true;
}


// ****************************************************************************
//  Method: MultiLineDoxygenDirective::RealStreamToken
//
//  Purpose:
//      Takes the next token in the stack and determines if this stream still
//      is appropriate for the token and previous tokens inputted.
//
//  Arguments:
//      token         The current token.
//      <unnamed>     The expression string that generated the token.
//
//  Programmer: Hank Childs
//  Creation:   March 24, 2000
//
// ****************************************************************************

void
MultiLineDoxygenDirective::RealStreamToken(DoxToken token, char *)
{
    if (token == replacementToken && pastFirstLine == false)
    {
        gotDirective = true;
        if (affectsFirstLine)
        {
            MakeDoxygenComment();
            ReplaceStackEntry(replacementExpr);
        }
    }
    else if (token == TK_NEWLINE)
    {
        if (gotDirective && !EmptyLine())
        {
            if (pastFirstLine == true)
            {
                InsertPrefix();
                MakeDoxygenComment();
            }
            if (affectsFirstLine || pastFirstLine)
            {
                OutputAndClearExpressionString();
            }
            else
            {
                ClearExpressionString();
            }
            receivedComment = false;
            pastFirstLine = true;
        }   
        else
        {
            badToken = true;
        }
    }
    else if (token != TK_OTHER)
    {
        badToken = true;
    }
}


// ****************************************************************************
//  Method: MultiLinePrefixDoxygenDirective Constructor
//
//  Programmer: Hank Childs
//  Creation:   March 24, 2000
//
// ****************************************************************************

MultiLinePrefixDoxygenDirective::MultiLinePrefixDoxygenDirective(DoxToken t, 
                                                                 char *e) 
    : MultiLineDoxygenDirective(t, e)
{
    affectsFirstLine = false;
}


// ****************************************************************************
//  Method: MultiLinePrefixDoxygenDirective::InsertPrefix
//
//  Purpose:
//      Inserts the prefix in each line after the first line.
//
//  Programmer: Hank Childs
//  Creation:   March 24, 2000
//
// ****************************************************************************

void
MultiLinePrefixDoxygenDirective::InsertPrefix(void)
{
    //
    // The first token is a prefix, so add the directive in the second 
    // position.
    //
    InsertToken(1, TK_OTHER, replacementExpr);
}


// ****************************************************************************
//  Method: MultiLinePrefixSplitLineDoxygenDirective constructor
//
//  Programmer:  Hank Childs
//  Creation:    June 9, 2000
//
// ****************************************************************************

MultiLinePrefixSplitLineDoxygenDirective
    ::MultiLinePrefixSplitLineDoxygenDirective(DoxToken t, char *e)
         : MultiLineDoxygenDirective(t, e)
{
    affectsFirstLine = false;
    prefixDistance   = -1;
}


// ****************************************************************************
//  Method: MultiLinePrefixSplitLineDoxygenDirective::InsertPrefix
//
//  Purpose:
//      Inserts the prefix conditionally.  It determines what the position
//      of the first real character is and inserts the prefix only if
//      the prefix distance matches.
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2000
//
// ****************************************************************************

void
MultiLinePrefixSplitLineDoxygenDirective::InsertPrefix(void)
{
    if (prefixDistance == -1)
    {
        //
        // This is the first argument, so set the prefix distance to be
        // however far out it is.
        //
        prefixDistance = PositionOfFirstRealChar();
    }

    if (prefixDistance == PositionOfFirstRealChar())
    {
        InsertToken(1, TK_OTHER, replacementExpr);
    }
}


