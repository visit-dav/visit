// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CONDITIONAL_H
#define CONDITIONAL_H

#include <QTextStream>

// ****************************************************************************
//  Class:  Conditional
//
//  Purpose:
//    Abstraction for conditional logic to be added to CMakeLists.txt.
//
//  Programmer:  Kathleen Biagas 
//  Creation:    Sep 29, 2025
//
//  Modifications:
//    Kathleen Biagas, Thu Oct 9, 2025
//    Added cxxflags.
//
// ****************************************************************************

class Conditional
{
  public:
    // CodeFile.h has more conditions listed in ParseCondition
    // But currently the only ones used in .code files are
    //  Definitions, MLinkLibraries and ELinkLibraries, so currently only
    //  handling those.
    QString target;
    QString condition;
    QString definitions;
    QString cxxflags;
    QString mlinklibs;
    QString elinklibs;
  public:
    Conditional(const QString &t,
                const QString &c):
        target(t),
        condition(c)
    {
    }
    void Print(QTextStream &out, const QString &generatorName = QString())
    {
        if(generatorName.isEmpty() || generatorName == target)
            out << "    Conditional: (" << target << ") " << condition << Endl;
    }
};

#endif
