#ifndef FUNCTION_H
#define FUNCTION_H

#include <qstring.h>
#include "CodeFile.h"

// ****************************************************************************
//  Class:  Function
//
//  Purpose:
//    Abstraction for a function, either a new (user-defined) one or one which
//    overrides an existing one.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 28, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Apr 19 15:32:51 PDT 2002
//    Renamed some stuff.  Pulled most of the code into CodeFile.h.
//
// ****************************************************************************
class Function
{
  public:
    QString name;
    QString decl;
    QString def;
    bool    user;
    bool    member;
  public:
    Function(const QString &n, const QString &dc, const QString &df, bool u, bool m)
        : name(n), decl(dc), def(df), user(u), member(m)
    {
    }
    void Print(ostream &out)
    {
        out << "        Function: " << decl << endl;
        //out << func;
    }
};

#endif
