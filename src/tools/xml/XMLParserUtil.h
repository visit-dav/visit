/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef XML_PARSER_UTIL_H
#define XML_PARSER_UTIL_H

#include <qstring.h>
#include <visitstream.h>
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
        for (size_t i=0; i<strings.size(); i++)
        {
            if (strings[i] == s)
                found = true;
        }
        if (!found)
            strings.push_back(s);
    }
    void Write(ostream &out)
    {
        for (size_t i=0; i<strings.size(); i++)
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
JoinValues(const vector<QString> &strs, char j)
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
JoinValues(const vector<QString> &strs, QString &j)
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
    for (size_t i=0; i<values.size(); i++)
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

