#ifndef EXPRSCANNER_H
#define EXPRSCANNER_H
#include <expr_exports.h>

#include <string.h>
#include <visitstream.h>
#include <string>
#include <deque>
using std::string;
using std::deque;

#include <Scanner.h>
#include <ExprToken.h>

// ****************************************************************************
//  Class:  ExprScanner
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
//    Added a new interface base class to allow other kinds of scanners.
//
//    Jeremy Meredith, Wed Nov 24 12:01:44 PST 2004
//    Major refactoring caused this class to be renamed to ExprScanner
//    and Token to ExprToken.
//
// ****************************************************************************
class EXPR_API ExprScanner : public Scanner
{
  public:
    ExprScanner() { }
    virtual ~ExprScanner() { }
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
