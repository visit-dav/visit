#ifndef XML_PARSER_UTIL_H
#define XML_PARSER_UTIL_H

#include <qstring.h>
#include <iostream.h>
#include <vector>
using std::vector;

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
    vector<QString> strings;
    void AddString(const QString &s)
    {
        bool found = false;
        for (int i=0; i<strings.size(); i++)
        {
            if (strings[i] == s)
                found = true;
        }
        if (!found)
            strings.push_back(s);
    }
    void Write(ostream &out)
    {
        for (int i=0; i<strings.size(); i++)
        {
            out << strings[i];
        }
    }
};

inline vector<QString>
SplitValues(const QString &buff)
{
    vector<QString> output;
    
    buff.stripWhiteSpace();
    QString tmp="";
    for (int i=0; i<buff.length(); i++)
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
JoinValues(const vector<QString> &strs, char j)
{
    QString output;
    
    for (int i=0; i<strs.size(); i++)
    {
        output += strs[i];
        if (i < strs.size() - 1)
            output += j;
    }

    return output;
}

inline QString
JoinValues(const vector<QString> &strs, QString &j)
{
    QString output;
    
    for (int i=0; i<strs.size(); i++)
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
    if (s.lower() == "true" || s.lower() == "t" || s.lower() == "yes")
        return true;
    else if (s.lower() == "false" || s.lower() == "f" || s.lower() == "no")
        return false;

    throw QString().sprintf("bad value '%s' for bool",s.latin1());
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
    return QString().sprintf("%d",i);
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
    for (int i=0; i<buff.length(); i++)
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
    for (int i=0; i<buff.length(); i++)
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
StartOpenTag(ostream &out, const QString &tag, QString &indent)
{
    indent += "  ";
    out << indent << "<" << tag;
}

inline void
WriteTagAttr(ostream &out, const QString &attr, const QString &value)
{
    if (! value.isNull())
    {
        out << " " << attr << "=\"" << value << "\"";
    }
}

inline void
FinishOpenTag(ostream &out)
{
    out << ">\n";
}

inline void
WriteOpenTag(ostream &out, const QString &tag, QString &indent)
{
    indent += "  ";
    out << indent << "<" << tag << ">\n";
}

inline void
WriteCloseTag(ostream &out, const QString &tag, QString &indent)
{
    out << indent << "</" << tag << ">" << endl;
    indent = indent.left(indent.length()-2);
}

inline void
WriteValues(ostream &out, const vector<QString> &values, QString &indent)
{
    indent += "  ";
    for (int i=0; i<values.size(); i++)
    {
        out << indent << values[i] << endl;
    }
    indent = indent.left(indent.length()-2);
}

inline void
WriteValue(ostream &out, const QString &value, QString &indent)
{
    indent += "  ";
    out << indent << value << endl;
    indent = indent.left(indent.length()-2);
}

#endif

