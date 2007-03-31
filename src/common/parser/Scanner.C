#include <string.h>
#include <iostream.h>
#include <string>
#include <deque>
using std::string;
using std::deque;

#include "Scanner.h"
#include "Token.h"
#include "ParseException.h"

// ----------------------------------------------------------------------------
//  Scanner modifications:
//    Jeremy Meredith, Mon Jul 28 14:36:21 PDT 2003
//    Allow dots in tokens that started with were clearly scanned as
//    identifiers already.  For example, "alpha.1" will scan as a single
//    identifier now, without the need for the "<>" notation.
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
//  Method:  Scanner::GetCharType
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
// ****************************************************************************
int
Scanner::GetCharType(const char c)
{
    ScanState s = scanstate.back();
    int type = Err;

    if (c)
    {
        if (c == '\"')
            type = Quo;
        else if (c == '+' || c == '-')
            type = Sgn;
        else if (strchr("*/+-[]{}()<>=,^%@:#", c))
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

        // If we're parsing a file or var name in <>, it can contain "+-."
        // Just treat these characters as alphabetic in these cases.
        if (s == FileSpec || s == VarSpec)
        {
            if (type==Sgn || type==Dot || type == Eee)
                type = Alp;
        }
    }

    return type;
}

// ****************************************************************************
//  Method:  Scanner::GetAcceptToken
//
//  Purpose:
//    Creates a token based on the state and the string.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
Token *
Scanner::GetAcceptToken(const Pos &pos, const std::string &parsed, int state)
{
    switch (state)
    {
      case  2:
        return new StringConst(pos, parsed);
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
    throw LexicalException(pos);    
}

// ****************************************************************************
//  Method:  Scanner::SetInput
//
//  Purpose:
//    Sets a new input and resets the scan state.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
void
Scanner::SetInput(const std::string &s)
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
//  Method:  Scanner::UpdateScanState
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
Scanner::UpdateScanState(const std::string &parsed)
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
//  Method:  Scanner::ScanOneToken
//
//  Purpose:
//    Scans the string when it left off and returns the next token.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
Token*
Scanner::ScanOneToken()
{
    string workstring = "";

    while (true)
    {
        // Get the next character
        char lookahead = 0;
        if (pos < text.length())
            lookahead = text[pos];

        // Nothing left to parse, and no unfinished token to accept
        if (state == 0 && lookahead == 0)
            return new EndOfInput(pos);

        // Get the character type, the next state, and an accept token
        int  type   = GetCharType(lookahead);
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
                    throw UnexpectedEndException(pos);
                else
                    throw LexicalException(pos);
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
        pos++;
        workstring += lookahead;
        state = next;
    }
}
