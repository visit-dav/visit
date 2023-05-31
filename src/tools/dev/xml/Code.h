// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CODE_H
#define CODE_H

#include <QTextStream>

// ****************************************************************************
//  Class:  Code
//
//  Purpose:
//    Abstraction for a few lines of code.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 11, 2002
//
//  Modifications:
//   Brad Whitlock, Thu Feb 28 09:36:39 PDT 2008
//   Added target.
//
// ****************************************************************************
class Code
{
  public:
    QString name;
    QString prefix;
    QString postfix;
    QString target;
  public:
    Code(const QString &n, const QString &pre, const QString &post,
         const QString &t) : name(n), prefix(pre), postfix(post), target(t)
    {
    }
    void Print(QTextStream &out, const QString &generatorName = QString())
    {
        if(generatorName.isEmpty() || generatorName == target)
            out << "        Code: (" << target << ") " << name << Endl;
    }
};

#endif
