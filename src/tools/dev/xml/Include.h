// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef INCLUDE_H
#define INCLUDE_H

#include <QTextStream>

// ****************************************************************************
//  Class:  Include
//
//  Purpose:
//    Abstraction for an include file.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 15, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Feb 28 10:09:16 PDT 2008
//    Added target, renamed old target to destination.
//
// ****************************************************************************
class Include
{
  public:
    QString include;
    QString destination;
    bool    quoted;
    QString target;
  public:
    Include(const QString &d, bool q, const QString &t) : destination(d), quoted(q), target(t)
    {
        if (d != "header" && d != "source")
            throw QString("Bad destination file '%1' for include.").arg(d);
    }
    void SetInclude(const QString &i)
    {
        include = i;
    }
    void Print(QTextStream &out, const QString &generatorName = QString())
    {
        if(generatorName.isEmpty() || generatorName == target)
            out << "        Include: (" << target << ") "
                << include << Endl;
    }
};

#endif
