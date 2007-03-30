#ifndef CODE_H
#define CODE_H

#include <qstring.h>
#include "CodeFile.h"

// ****************************************************************************
//  Class:  Code
//
//  Purpose:
//    Abstraction for a few lines of code.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 11, 2002
//
// ****************************************************************************
class Code
{
  public:
    QString name;
    QString prefix;
    QString postfix;
  public:
    Code(const QString &n, const QString &pre, const QString &post)
        : name(n), prefix(pre), postfix(post)
    {
    }
    void Print(ostream &out)
    {
        out << "        Code: " << name << endl;
    }
};

#endif
