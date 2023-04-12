// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CONSTANT_H
#define CONSTANT_H

#include <QTextStream>

// ****************************************************************************
//  Class:  Constant
//
//  Purpose:
//    Abstraction for a constant.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 12, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Feb 28 09:39:49 PDT 2008
//    Added target.
//
//    Brad Whitlock, Thu May  8 11:31:21 PDT 2008
//    Qt 4. Use QTextSTream.
//
// ****************************************************************************
class Constant
{
  public:
    QString name;
    QString decl;
    QString def;
    bool    member;
    QString target;
  public:
    Constant(const QString &n, const QString &dc, const QString &df, bool m,
             const QString &t)
        : name(n), decl(dc), def(df), member(m), target(t)
    {
    }
    void Print(QTextStream &out, const QString &generatorName = QString())
    {
        if(generatorName.isEmpty() || generatorName == target)
            out << "        Constant: (" << target << ") " << decl << Endl;
    }
};

#endif
