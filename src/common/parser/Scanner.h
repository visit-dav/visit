#ifndef SCANNER_H
#define SCANNER_H
#include <parser_exports.h>

#include <string.h>
#include <iostream.h>
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
// ****************************************************************************
class PARSER_API Scanner
{
  public:
    // These are the characters belonging to each character class:
    /*
      Quo   "
      Sgn   + -
      Sym   * / + - [ ] { } ( ) < > = , ^ % @ :
      Eee   e E
      Alp   a-z A-Z _
      Dig   0-9
      Dot   .
    */

    enum CharType {
        Quo = 0,
        Sgn = 1,
        Sym = 2,
        Eee = 3,
        Alp = 4,
        Dig = 5,
        Dot = 6,
        Spc = 7,
        Err
    };

    // Certain characters get scanned differently in different states
    enum ScanState {
        Normal,
        FileSpec,
        TimeSpec,
        VarSpec
    };

  public:
    void   SetInput(const std::string &);
    Token *ScanOneToken();
    string GetTokenTypeString(int t);

  private:
    int    GetCharType(const char c);
    Token *GetAcceptToken(const Pos&,const std::string&, int);
    void   UpdateScanState(const std::string &parsed);

    // All state for scanning
    std::string      text;
    int              state;
    deque<ScanState> scanstate;

    int              pos;
    int              lastacceptstate;
    string           lastacceptstring;
    int              lastacceptpos;

};

#endif
