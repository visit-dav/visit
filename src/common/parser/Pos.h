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
// ****************************************************************************

class PARSER_API Pos
{
    int p1, p2;

    int Minimum(int a, int b) { return (a < b) ? a : b; }
    int Maximum(int a, int b) { return (a > b) ? a : b; }
  public:
    Pos()             { p1 = -1;              p2 = -1;              }
    Pos(const Pos &p) { p1 = p.p1;            p2 = p.p2;            }
    Pos(int p)        { p1 = p;               p2 = p;               }
    Pos(int a, int b) { p1 = Minimum(a,b);        p2 = Maximum(a,b);        }
    Pos(Pos a, Pos b) { p1 = Minimum(a.p1, b.p1); p2 = Maximum(a.p2, b.p2); }

    void Join(const Pos &p)       { p1=Minimum(p1, p.p1); p2=Maximum(p2, p.p2); }
    void operator=(const Pos &p)  { p1=p.p1;               p2=p.p2;     }
    bool IsNull()                 { return (p1 == -1) && (p2 == -1); }

    void PrintText(ostream &o, const std::string &s) const
    {
        int i;
        o << s.c_str() << endl;
        for (i=0; i<p1; i++)
            o << ' ';
        for (i=p1; i<=p2; i++)
            o << '^';
        o << endl;
    }
    std::string GetText(const std::string &s) const
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
};

#endif
