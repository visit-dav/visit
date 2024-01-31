// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef FUNCTION_H
#define FUNCTION_H

#include <QTextStream>

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
//    Hank Childs, Tue Aug 28 14:36:10 PDT 2007
//    Add a Boolean to indicate whether the function was used.
//
//    Brad Whitlock, Wed Feb 27 15:55:37 PST 2008
//    Added target.
//
//    Brad Whitlock, Fri Apr 25 11:13:20 PDT 2008
//    Added accessType
//
//    Brad Whitlock, Thu May  8 11:35:41 PDT 2008
//    Qt 4. Use QTextStream.
//
// ****************************************************************************
class Function
{
  public:
    typedef enum {AccessPrivate, AccessProtected, AccessPublic} AccessType;

    QString name;
    QString decl;
    QString def;
    bool    user;
    bool    member;
    QString target;
    bool    usedThisFunction;
    AccessType accessType;
  public:
    Function(const QString &n, const QString &dc, const QString &df, bool u,
        bool m, const QString &t) : name(n), decl(dc), def(df), user(u),
        member(m), target(t), accessType(AccessPublic)
    {
        usedThisFunction = false;
    }
    void Print(QTextStream &out, const QString &generatorName = QString())
    {
        if(generatorName.isEmpty() || generatorName == target)
        {
            out << "        Function: (" << target;
            if(accessType == AccessPrivate)
                out << ",private";
            else if(accessType == AccessProtected)
                out << ",protected";
            out << ") " << decl << Endl;
        }
    }
};

#endif
