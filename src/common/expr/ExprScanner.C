/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include <string.h>
#include <visitstream.h>
#include <string>
#include <deque>
using std::string;
using std::deque;

#include "ExprScanner.h"
#include "ExprToken.h"
#include "ParseException.h"

using     std::string;

// ----------------------------------------------------------------------------
//  Scanner modifications:
//    Jeremy Meredith, Mon Jul 28 14:36:21 PDT 2003
//    Allow dots in tokens that were starting to be clearly scanned as
//    identifiers already.  For example, "alpha.1" will scan as a single
//    identifier now, without the need for the "<>" notation.
//
//    Jeremy Meredith, Wed Nov 24 12:01:44 PST 2004
//    Major refactoring caused this class to be renamed to ExprScanner
//    and Token to ExprToken.
//
// ----------------------------------------------------------------------------

static const int StateTransition[13][8] = {
//       Quo  Sgn  Sym  Eee  Alp  Dig  Dot  Spc
/* 0*/ {  1,   3,   3,   4,   4,   5,   6,  12  },
/* 1*/ {  2,   1,   1,   1,   1,   1,   1,   1  },
/* 2*/ { -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1  },
/* 3*/ { -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1  },
/* 4*/ { -1,  -1,  -1,   4,   4,   4,   4,  -1  },
/* 5*/ { -1,  -1,  -1,  11,   4,   5,   7,  -1  },
/* 6*/ { -1,  -1,  -1,  -1,  -1,   7,  -1,  -1  },
/* 7*/ { -1,  -1,  -1,   8,  -1,   7,  -1,  -1  },
/* 8*/ { -1,   9,  -1,  -1,  -1,  10,  -1,  -1  },
/* 9*/ { -1,  -1,  -1,  -1,  -1,  10,  -1,  -1  },
/*10*/ { -1,  -1,  -1,  -1,  -1,  10,  -1,  -1  },
/*11*/ { -1,   9,  -1,   4,   4,  10,  -1,  -1  },
/*12*/ { -1,  -1,  -1,  -1,  -1,  -1,  -1,  12  }
};

static const bool StateAcceptance[13] = {
/* 0*/ false,
/* 1*/ false,
/* 2*/  true,
/* 3*/  true,
/* 4*/  true,
/* 5*/  true,
/* 6*/ false,
/* 7*/  true,
/* 8*/ false,
/* 9*/ false,
/*10*/  true,
/*11*/  true,
/*12*/  true
};

// ****************************************************************************
//  Method:  ExprScanner::GetCharType
//
//  Purpose:
//    Returns the category of character just scanned.  This reduction
//    keeps the state transition table from being 256 columns.
//
//  Note:
//    This is slightly specialized for the expression grammar.  If we
//    are inside <> and not inside a nested [], then our identifiers
//    may look like file names and may contain "+","-","." .
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Jeremy Meredith, Fri Sep  2 16:27:14 PDT 2005
//    Made it be more relaxed about allowing special characters
//    inside the "<>" angle brackets.  This is important as it is our
//    only means of quoting these.
//
//    Mark C. Miller, Mon Apr 14 15:41:21 PDT 2008
//    Added support for '&' binary, bitwise, and expression
//
//    Jeremy Meredith, Wed Jul 23 13:13:42 EDT 2008
//    Allow backslashes.  Add quoting support.
//
//    Jeremy Meredith, Tue Mar 24 11:19:04 EDT 2009
//    Added a whole bunch of symbols that should be supported inside
//    quoted strings.  These don't have associated tokens, however, so
//    will still result in parse errors outside of strings.
//
// ****************************************************************************
int
ExprScanner::GetCharType(const char c, bool escaped) const
{
    ScanState s = scanstate.back();
    int type = Err;

    if (escaped)
    {
        if (c == '\\' || c == '/')
            type = Sym;
        else if (c)
            type = Alp;
    }
    else if (c)
    {
        if (c == '\"')
            type = Quo;
        else if (c == '+' || c == '-')
            type = Sgn;
        else if (strchr("*/+-[]{}()<>=,^%@:#&\\';!|~?`$", c))
            type = Sym;
        else if (c == 'e' || c == 'E')
            type = Eee;
        else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
            type = Alp;
        else if (c >= '0' && c <= '9')
            type = Dig;
        else if (c == '.')
            type = Dot;
        else if (c == ' ' || c == '\t' || c == '\n')
            type = Spc;
        else
            type = Err;

        // If we're parsing a file or var name in <>, we should treat
        // it (as much as possible) as if it were quoting special chars.
        // Just treat these characters as alphabetic in these cases.
        if (s == FileSpec || s == VarSpec)
        {
            if (c != '<' &&
                c != '>' &&
                c != '[' &&
                c != ']' &&
                c != ':' &&
                c != '@' &&
                c != '/' &&
                c != '\\')
            {
                type = Alp;
            }
        }
    }

    return type;
}

// ****************************************************************************
//  Method:  ExprScanner::GetAcceptToken
//
//  Purpose:
//    Creates a token based on the state and the string.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//
//    Hank Childs, Wed Aug 13 09:36:22 PDT 2003
//    Take out the quotes around string constants.
//
// ****************************************************************************
Token *
ExprScanner::GetAcceptToken(const Pos &pos, const std::string &parsed, int state)
{
    switch (state)
    {
      case  2:
        // Remove the quotes on either side of the string.
        return new StringConst(pos, parsed.substr(1, parsed.size()-2));
      case  3:
        return new Character(pos, parsed);
      case  5:
        return new IntegerConst(pos, parsed);
      case  7:
      case 10:
        return new FloatConst(pos, parsed);
      case  4:
      case 11: // Check for all keywords here.
        if (parsed == "true"  ||
            parsed == "false" ||
            parsed == "on"    ||
            parsed == "off")
            return new BoolConst(pos, parsed);
        else
            return new Identifier(pos, parsed);
      case 12:
        return new Space(pos, parsed);
    }
    EXCEPTION1(LexicalException, pos);    
}

// ****************************************************************************
//  Method:  ExprScanner::SetInput
//
//  Purpose:
//    Sets a new input and resets the scan state.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
void
ExprScanner::SetInput(const std::string &s)
{
    // Set the input text
    text = s;

    // Reset all state info
    scanstate.clear();
    scanstate.push_back(Normal);

    pos = 0;
    state = 0;
    lastacceptstate  = -1;
    lastacceptstring = "";
    lastacceptpos    = -1;
}

// ****************************************************************************
//  Method:  ExprScanner::UpdateScanState
//
//  Purpose:
//    This is specialized for our grammar since identifiers inside <>
//    and not inside a nested [] may contain "+","-",".".
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
void
ExprScanner::UpdateScanState(const std::string &parsed)
{
    switch (scanstate.back())
    {
      case Normal:
        if (parsed == "<")
            scanstate.push_back(FileSpec);
        break;
      case FileSpec:
        if (parsed == ">")
            scanstate.pop_back();
        else if (parsed == "[")
            scanstate.push_back(TimeSpec);
        else if (parsed == ":")
            scanstate.back() = VarSpec;
        break;
      case TimeSpec:
        if (parsed == "]")
            scanstate.pop_back();
        break;
      case VarSpec:
        if (parsed == ">")
            scanstate.pop_back();
        break;
    }
    if (scanstate.empty())
        scanstate.push_back(Normal);
}

// ****************************************************************************
//  Method:  ExprScanner::ScanOneToken
//
//  Purpose:
//    Scans the string when it left off and returns the next token.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Jeremy Meredith, Wed Jul 23 13:12:22 EDT 2008
//    Add support for using a backslash as an escaping mechanism.
//
//    Cyrus Harrison, Thu Feb 11 21:21:46 PST 2010
//    Add support escaped newlines.
//
// ****************************************************************************
Token*
ExprScanner::ScanOneToken()
{
    string workstring = "";

    while (true)
    {
        // Get the next character
        char lookahead = 0;
        if (pos < text.length())
            lookahead = text[pos];

        // If it was a backslash, just use quoting
        bool backslashEscape = lookahead == '\\';
        if (backslashEscape)
        {
            lookahead = 0;
            if (pos+1 < text.length())
            {
                lookahead = text[pos+1];
                if(lookahead == 'n')
                    lookahead ='\n';
            }
        }

        // Nothing left to parse, and no unfinished token to accept
        if (state == 0 && lookahead == 0)
            return new EndOfInput(pos);

        // Get the character type, the next state, and an accept token
        int  type   = GetCharType(lookahead, backslashEscape);
        int  next   = (type == Err) ? -1 : StateTransition[state][type];
        bool accept = StateAcceptance[state];

        // If we can accept right now, save this info
        if (accept)
        {
            lastacceptstate  = state;
            lastacceptstring = workstring;
            lastacceptpos    = pos;
        }

        // If we can't transition, either reject or accept
        if (next == -1)
        {
            if (lastacceptstate == -1)
            {
                if (lookahead == 0)
                    EXCEPTION1(UnexpectedEndException, pos)
                else
                    EXCEPTION1(LexicalException, pos);
            }

            UpdateScanState(lastacceptstring);
            Token *token = GetAcceptToken(Pos(lastacceptpos -
                                                     lastacceptstring.length(),
                                              lastacceptpos-1),
                                          lastacceptstring, 
                                          lastacceptstate);
            state           = 0;
            pos             = lastacceptpos;
            lastacceptstate = -1;
            return token;
        }

        // Push the lookahead and keep going
        if (backslashEscape) // skip the backslash too if we got one
            pos++;
        pos++;
        workstring += lookahead;
        state = next;
    }
}
