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
//    Jeremy Meredith, Wed Jul 23 13:37:20 EDT 2008
//    Allow the character type to take into account a backslash-escape.
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
    int    GetCharType(const char c, bool escaped) const;
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
