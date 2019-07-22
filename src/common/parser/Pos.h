// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    Mark C. Miller, Wed Mar 17 10:02:23 PDT 2010
//    Handle truncation of error text to specified buffer size.
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
    std::string GetErrorText(const std::string &s, int _n) const
    {
        int i;
        int n = 0;
        std::string tmp, msg;
        if (_n <= 2*(p2+1))
            n = p2 - (_n/2) + 8; // '8' is fudge
        msg = std::string(s,n) + '\n';
        for (i=0; i<p1; i++)
            tmp += ' ';
        for (i=p1; i<=p2; i++)
            tmp += '^';
        msg += (std::string(tmp,n) + '\n');
        return msg;
    }
    std::string GetText(const std::string &s) const
    {
        std::string msg = "";
        int slen = static_cast<int>(s.length());
        if (slen < p1 || slen < p2 || p1 < 0)
            return msg;
        msg = s.substr(p1, p2-p1+1);
        return msg;
    }
};

#endif
