#include "Token.h"

using     std::string;

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
//      Sean Ahern, Fri Feb 21 20:59:12 America/Los_Angeles 2003
//      Made this correctly say "Type".
//
// ****************************************************************************
string
GetTokenTypeString(int t)
{
    if (t == 0)
        return "NoToken";
    else if (t < 256)
        return string() + char(t);
    else if (t == TT_EOF)
        return "$";
    else if (t == TT_Identifier)
        return "Identifier";
    else if (t == TT_IntegerConst)
        return "Integer";
    else if (t == TT_FloatConst)
        return "Float";
    else if (t == TT_StringConst)
        return "String";
    else if (t == TT_BoolConst)
        return "Bool";
    else if (t == TT_Space)
        return "Space";
    else
        return "unknown";
}
