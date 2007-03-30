#ifndef CONSTANT_H
#define CONSTANT_H

#include <qstring.h>
#include "CodeFile.h"

// ****************************************************************************
//  Class:  Constant
//
//  Purpose:
//    Abstraction for a constant.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 12, 2002
//
// ****************************************************************************
class Constant
{
  public:
    QString name;
    QString decl;
    QString def;
    bool    member;
  public:
    Constant(const QString &n, const QString &dc, const QString &df, bool m)
        : name(n), decl(dc), def(df), member(m)
    {
    }
    void Print(ostream &out)
    {
        out << "        Constant: " << decl << endl;
        //out << func;
    }
};

#endif
