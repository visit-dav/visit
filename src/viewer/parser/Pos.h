#ifndef POS_H
#define POS_H
#include <viewer_parser_exports.h>

#include <iostream.h>
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
// ****************************************************************************

struct VIEWER_PARSER_API Pos
{
  private:
    int p1, p2;

    int min(int a, int b) { return (a < b) ? a : b; }
    int max(int a, int b) { return (a > b) ? a : b; }
  public:
    Pos()             { p1 = -1;              p2 = -1;              }
    Pos(const Pos &p) { p1 = p.p1;            p2 = p.p2;            }
    Pos(int p)        { p1 = p;               p2 = p;               }
    Pos(int a, int b) { p1 = min(a,b);        p2 = max(a,b);        }
    Pos(Pos a, Pos b) { p1 = min(a.p1, b.p1); p2 = max(a.p2, b.p2); }

    void Join(const Pos &p)       { p1=min(p1, p.p1); p2=max(p2, p.p2); }
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
};

#endif
