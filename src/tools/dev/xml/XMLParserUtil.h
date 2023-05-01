// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef XML_PARSER_UTIL_H
#define XML_PARSER_UTIL_H

#include <QTextStream>

#include <vector>

// ****************************************************************************
//  Methods for text manipulation.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 23 17:01:21 PDT 2003
//    Added "yes" and "no" as legal bool values.
//
// ****************************************************************************
class UniqueStringList
{
  public:
    std::vector<QString> strings;
    void AddString(const QString &s)
    {
        bool found = false;
        for (size_t i=0; i<strings.size(); i++)
        {
            if (strings[i] == s)
                found = true;
        }
        if (!found)
            strings.push_back(s);
    }
    void Write(QTextStream &out)
    {
        for (size_t i=0; i<strings.size(); i++)
        {
            out << strings[i];
        }
    }
};

inline std::vector<QString>
SplitValues(const QString &buff_input)
{
    std::vector<QString> output;

    QString buff(buff_input.trimmed());
    QString tmp="";
    int len = buff.length();
    for (int i=0; i<len; i++)
    {
        if (buff[i].isSpace() ||
            buff[i]==','      ||
            buff[i]==':'      ||
            buff[i]==';'      )
        {
            if (!tmp.isEmpty())
                output.push_back(tmp);
            tmp = "";
        }
        else
        {
            tmp += buff[i];
        }
    }
    if (!tmp.isEmpty())
        output.push_back(tmp);

    return output;
}

inline QString
JoinValues(const std::vector<QString> &strs, char j)
{
    QString output;

    for (size_t i=0; i<strs.size(); i++)
    {
        output += strs[i];
        if (i < strs.size() - 1)
            output += j;
    }

    return output;
}

inline QString
JoinValues(const std::vector<QString> &strs, QString &j)
{
    QString output;

    for (size_t i=0; i<strs.size(); i++)
    {
        output += strs[i];
        if (i < strs.size() - 1)
            output += j;
    }

    return output;
}

inline bool
Text2Bool(const QString &s)
{
    if (s.toLower() == "true" || s.toLower() == "t" || s.toLower() == "yes")
        return true;
    else if (s.toLower() == "false" || s.toLower() == "f" || s.toLower() == "no")
        return false;

    throw QString("bad value '%1' for bool").arg(s);
}

inline QString
Bool2Text(bool b)
{
    if (b)
        return "true";
    else
        return "false";
}

inline QString
Int2Text(int i)
{
    return QString().asprintf("%d",i);
}

// ****************************************************************************
//  Methods for splitting a file into dirname and basename.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************

inline QString
FilePath(const QString &buff)
{
    QString output;

    QString tmp="";
    int len = buff.length();
    for (int i=0; i<len; i++)
    {
        tmp += buff[i];
        if (buff[i]=='/' ||
            buff[i]=='\\')
        {
            output += tmp;
            tmp = "";
        }
    }

    return output;
}

inline QString
FileBase(const QString &buff)
{
    QString tmp="";
    int len = buff.length();
    for (int i=0; i<len; i++)
    {
        tmp += buff[i];
        if (buff[i]=='/' ||
            buff[i]=='\\')
        {
            tmp = "";
        }
    }

    return tmp;
}

// ****************************************************************************
//  Helper methods for writing XML files.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************

inline void
StartOpenTag(QTextStream &out, const QString &tag, QString &indent)
{
    indent += "  ";
    out << indent << "<" << tag;
}

inline void
WriteTagAttr(QTextStream &out, const QString &attr, const QString &value)
{
    if (! value.isNull())
    {
        out << " " << attr << "=\"" << value << "\"";
    }
}

inline void
FinishOpenTag(QTextStream &out)
{
    out << ">\n";
}

inline void
WriteOpenTag(QTextStream &out, const QString &tag, QString &indent)
{
    indent += "  ";
    out << indent << "<" << tag << ">\n";
}

inline void
WriteCloseTag(QTextStream &out, const QString &tag, QString &indent)
{
    out << indent << "</" << tag << ">" << Endl;
    indent = indent.left(indent.length()-2);
}

inline void
WriteValues(QTextStream &out, const std::vector<QString> &values, QString &indent)
{
    indent += "  ";
    for (size_t i=0; i<values.size(); i++)
    {
        QString s(indent + values[i]);
        out << s << Endl;
    }
    indent = indent.left(indent.length()-2);
}

inline void
WriteValue(QTextStream &out, const QString &value, QString &indent)
{
    indent += "  ";
    out << (indent + value) << Endl;
    indent = indent.left(indent.length()-2);
}

#endif

