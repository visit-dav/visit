#include "Token.h"

using std::string;

// ****************************************************************************
//  Function:  GetTokenTypeString
//
//  Purpose:
//    Gets a printable name for a token type.
//
//  Arguments:
//    t          the token type
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Sean Ahern, Fri Feb 21 20:59:12 America/Los_Angeles 2003
//    Made this correctly say "Type".
//
//    Jeremy Meredith, Wed Nov 24 12:10:42 PST 2004
//    During refactoring, this became common to all grammars, so we could
//    not include VisIt Expression Language specific tokens here.
//
// ****************************************************************************
string
GetTokenTypeString(int t)
{
    if (t == 0)
        return "NoToken";
    else if (t < 256)
        return string() + char(t);
    else if (t == EOF_TOKEN_ID)
        return "$";
    else
        return "unknown";
}
