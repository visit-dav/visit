// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CONDITIONAL_H
#define CONDITIONAL_H

#include <QTextStream>
#include <map>
#include <vector>


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
//    Kathleen Biagas, Tue Oct 21, 2025
//    Added operators == and !=.
//    Changed storage of the actual condition values to map<QString,QString>.
//    Moved ParseCondition logic from CodeFile to here, and restricted 'keys'
//    to the actual ones currently being used.
//     Should be easy to update when new keys are needed.
//
//    Kathleen Biagas, Tue Nov  4 16:38:29 PST 2025
//    When loopling through 'keys' in ParseCondition, use # of keys instead of
//    'sizeof(keys)'.  Prevents crash.
//
// ****************************************************************************

class Conditional
{
  public:
    QString target;
    QString condition;
    std::map<QString, QString> keyVals;

    Conditional(const QString &t,
                const QString &c):
        target(t),
        condition(c)
    {
    }

    bool operator==(const Conditional &c)
    {
        return (target == c.target && condition == c.condition);    
    }
    bool operator!=(const Conditional &c)
    {
        return (target != c.target || condition != c.condition);    
    }


    inline static const std::vector<QString> keys
    {
        "Definitions:",
        "CXXFlags:",
        "VLinkLibraries:",
        "MLinkLibraries:",
        "ELinkLibraries:"
    };

    bool ParseCondition(QString &buff)
    {
        bool success = false;
        for (const auto &key : keys)
        {
            if (success)
                break;

            if (buff.startsWith(key))
            {
                QString value(buff.mid(key.size()).trimmed());
                while (value.right(1) == "\n")
                    value = value.left(value.length() - 1);
                if (!value.isEmpty())
                {
                     keyVals[key] = value;
                     success = true;
                }
            }
        }
        return success;
    }

    void WriteToCodeFile(QTextStream &out)
    {
        out << "Target: " << target << Endl;
        out << "Condition: " << condition << Endl;
        for (auto& x : keyVals)
        {
            out << x.first << " " << x.second << Endl;
        }
        out << Endl;
    }

    void Print(QTextStream &out, const QString &generatorName = QString())
    {
        if(generatorName.isEmpty() || generatorName == target)
            out << "    Conditional: (" << target << ") " << condition << Endl;
    }
};

#endif
