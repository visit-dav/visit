// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ENUM_H
#define ENUM_H

#include <QTextStream>

#include <vector>

// ****************************************************************************
//  Class:  EnumType
//
//  Purpose:
//    Abstraction for an enumerated type.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 28, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Oct 17 15:57:44 PDT 2002
//    Moved the static data to a .C file.
//
//    Eric Brugger, Mon Jul 26 15:00:00 PDT 2004
//    I changed cout to out references in the Print method.
//
//    Brad Whitlock, Thu May  8 11:31:42 PDT 2008
//    Qt 4. Use QTextStream.
//
//    Brad Whitlock, Tue Sep 26 12:12:34 PDT 2017
//    Support adding enum int values.
//
// ****************************************************************************
class EnumType
{
  public:
    static std::vector<EnumType*> enums;
    static EnumType *FindEnum(const QString &s)
    {
        EnumType *e = NULL;
        for (size_t i=0; i<enums.size(); i++)
        {
            if (enums[i]->type == s)
            {
                e = enums[i];
            }
        }
        if (!e)
            throw QString("unknown enum subtype '%1'").arg(s);
        return e;
    }
  public:
    QString         type;
    std::vector<QString> values;
    std::vector<int>     ivalues;
  public:
    EnumType(const QString &s) : type(s), values(), ivalues()
    {
    }
    void AddValue(const QString &s)
    {
        int n;
        if((n = s.indexOf("=")) != -1)
        {
            values.push_back(s.left(n).simplified());
            bool ok = false;
            int ival = s.mid(n+1).simplified().toInt(&ok);
            ivalues.push_back(ok ? ival : -1);
        }
        else
        {
            values.push_back(s);
            ivalues.push_back(-1);
        }
    }
    const QString& GetValue(size_t index)
    {
        if (index >= values.size())
            throw QString("tried to access out-of-bounds enum type %1").arg(index);
        return values[index];
    }
    const int GetIValue(size_t index)
    {
        if (index >= values.size())
            throw QString("tried to access out-of-bounds enum type %1").arg(index);
        return ivalues[index];
    }
    void Print(QTextStream &out)
    {
        out << "Enum: " << type << Endl;
        for (size_t i=0; i<values.size(); i++)
        {
            out << "    " << values[i];
            if(ivalues[i] >= 0)
                out << " = " << ivalues[i];
            out << Endl;
        }
    }
};

#endif
