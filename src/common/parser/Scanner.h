#ifndef SCANNER_H
#define SCANNER_H
#include <parser_exports.h>

#include <string.h>
#include <visitstream.h>
#include <string>
#include <deque>
using std::string;
using std::deque;

#include "Token.h"

// ****************************************************************************
//  Class:  Scanner
//
//  Purpose:
//    A table-driven scanner for the VisIt expression language.
//
//  Note: State transition diagram is in the doc files states.jpg/states.dia.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  2, 2002
//
//  Modifications:
//    Jeremy Meredith, Thu Nov 11 15:36:55 PST 2004
//    Refactored to allow other kinds of scanners.  This class's old
//    functionality moved to ExprScanner.
//
// ****************************************************************************
class PARSER_API Scanner
{
  public:
    Scanner() { }
    virtual ~Scanner() { }
    virtual void   SetInput(const std::string &) = 0;
    virtual Token *ScanOneToken() = 0;
};

#endif
