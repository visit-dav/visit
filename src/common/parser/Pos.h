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

#ifndef POS_H
#define POS_H
#include <parser_exports.h>

#include <visitstream.h>
#include <string>

// ****************************************************************************
//  Class:  Pos
//
//  Purpose:
//    Stored the position of text using a start and end character position.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  4, 2002
//
//  Modifications:
//    Jeremy Meredith, Mon Jul 28 16:59:51 PDT 2003
//    Added the GetText method.
//
//    Brad Whitlock, Wed Aug 27 13:55:47 PST 2003
//    I replaced min,max with Minimum,Maximum since the former are defined
//    as macros that cause compilation errors on Windows.
//
//    Hank Childs, Tue Dec 28 16:04:59 PST 2004
//    Renamed GetText to be GetErrorText.  Added new method called GetText.
//
//    Jeremy Meredith, Mon Jun 13 12:14:21 PDT 2005
//    Let Join() work with null positions.
//
// ****************************************************************************

class PARSER_API Pos
{
    int p1, p2;

    int Minimum(int a, int b) { return (a < b) ? a : b; }
    int Maximum(int a, int b) { return (a > b) ? a : b; }
  public:
    Pos()             { p1 = 999999;          p2 = -1;              }
    Pos(const Pos &p) { p1 = p.p1;            p2 = p.p2;            }
    Pos(int p)        { p1 = p;               p2 = p;               }
    Pos(int a, int b) { p1 = Minimum(a,b);        p2 = Maximum(a,b);        }
    Pos(Pos a, Pos b) { p1 = Minimum(a.p1, b.p1); p2 = Maximum(a.p2, b.p2); }

    void Join(const Pos &p)       { p1=Minimum(p1, p.p1); p2=Maximum(p2, p.p2); }
    void operator=(const Pos &p)  { p1=p.p1;               p2=p.p2;     }
    bool IsNull()                 { return (p2 == -1); }

    void PrintErrorText(ostream &o, const std::string &s) const
    {
        int i;
        o << s.c_str() << endl;
        for (i=0; i<p1; i++)
            o << ' ';
        for (i=p1; i<=p2; i++)
            o << '^';
        o << endl;
    }
    std::string GetErrorText(const std::string &s) const
    {
        int i;
        std::string msg = s + '\n';
        for (i=0; i<p1; i++)
            msg += ' ';
        for (i=p1; i<=p2; i++)
            msg += '^';
        msg += '\n';
        return msg;
    }
    std::string GetText(const std::string &s) const
    {
        std::string msg = "";
        int slen = s.length();
        if (slen < p1 || slen < p2 || p1 < 0)
            return msg;
        msg = s.substr(p1, p2-p1+1);
        return msg;
    }
};

#endif
