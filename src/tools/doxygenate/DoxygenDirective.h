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
//                         DoxygenDirective.h                                //
// ************************************************************************* //

#ifndef DOXYGEN_DIRECTIVE_H
#define DOXYGEN_DIRECTIVE_H

#include <Tokens.h>


// ****************************************************************************
//  Class: DoxygenDirective
//
//  Purpose:
//      An abstract type that provides an object capable of accepting a stream
//      of tokens until its lifetime is over and then returning the extra
//      tokens it was given.
//
//  Note:       This is an abstract type.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Aug  7 08:17:30 PDT 2000
//    Made destructor virtual.
//
// ****************************************************************************

class DoxygenDirective
{
  public:
                      DoxygenDirective();
    virtual          ~DoxygenDirective();

    void              StreamToken(DoxToken, char *);
    virtual void      RealStreamToken(DoxToken, char *) = 0;
    virtual bool      LifetimeOver(void);
    int               ReturnTokenStack(DoxToken **, char ***);
  
  protected:
    void              AddToTokenStack(DoxToken, char *);
    void              InsertToken(int, DoxToken, char *);
    void              MakeDoxygenComment(void);
    void              OutputAndClearExpressionString(void);
    void              ClearExpressionString(void);
    void              ReplaceStackEntry(char *);
    bool              EmptyLine(void);
    int               PositionOfFirstRealChar(void);
   
    // Indicates whether this object has found the token that indicates it.
    bool              gotDirective;
 
    // Indicates whether this object has found a comment token.
    bool              receivedComment;

    // Indicates whether this object has found a token that has made further
    // streaming impossible.
    bool              badToken;
 
    // The expression to replace the directive token with.
    char             *replacementExpr;

    // The token that indicates this is the proper stream to be using.
    DoxToken          replacementToken;

  private:

    // A stack of all tokens found to date.
    DoxToken         *tokenStack;

    // The corresponding expressions with the token stack.
    char            **exprStack;

    // The number of tokens in the token stack.
    int               numTokens;

    // The amount of memory we have to store tokens.
    int               totalTokens;
};


// ****************************************************************************
//  Class: OneLineDoxygenDirective
//
//  Purpose:
//      A derived type of DoxygenDirective that handles tokens for one line
//      only.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2000
//
// ****************************************************************************

class OneLineDoxygenDirective : public DoxygenDirective
{
  public:
                     OneLineDoxygenDirective(DoxToken, char *);
    bool             LifetimeOver(void);
    void             RealStreamToken(DoxToken, char *);
  protected:

    // A boolean indicating if we have read in a line yet.
    bool             readInOneLine;
};


// ****************************************************************************
//  Class: MultiLineDoxygenDirective
//
//  Purpose:
//      A derived type of DoxygenDirective that handles tokens for multiple
//      lines.
//
//  Programmer: Hank Childs
//  Creation:   March 24, 2000
//
// ****************************************************************************

class MultiLineDoxygenDirective : public DoxygenDirective
{
  public:
                     MultiLineDoxygenDirective(DoxToken, char *);
    void             RealStreamToken(DoxToken, char *);

  protected:
    virtual          void InsertPrefix() {;} ;

    // A boolean indicating if this directive affect the first line of a 
    // multiline directive.  Objects with prefixes do, those without don't
    bool             affectsFirstLine;

  private:
    // A boolean indicating if we are past the first line.
    bool             pastFirstLine;
};


// ****************************************************************************
//  Class: MultiLinePrefixDoxygenDirective
//
//  Purpose:
//      A derived type of MultiLineDoxygenDirective that puts in a prefix
//      after each subsequent line.
//
//  Programmer: Hank Childs
//  Creation:   March 24, 2000
//
// ****************************************************************************

class MultiLinePrefixDoxygenDirective : public MultiLineDoxygenDirective
{
  public:
                    MultiLinePrefixDoxygenDirective(DoxToken, char *);
  protected:
    virtual void    InsertPrefix();
};


// ****************************************************************************
//  Class: MultiLinePrefixSplitLineDoxygenDirective
//
//  Purpose:
//      A derived type of MultiLineDoxygenDirective that puts in a prefix
//      on following lines, provided the whitespace matches.
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2000
//
// ****************************************************************************

class MultiLinePrefixSplitLineDoxygenDirective 
    : public MultiLineDoxygenDirective
{
  public:
                    MultiLinePrefixSplitLineDoxygenDirective(DoxToken, char *);

  protected:
    int             prefixDistance;
    virtual void    InsertPrefix();
};


#endif


