// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <ViewerText.h>

#include <cstring>
#include <stdio.h>

#define MAX_VIEWERTEXT_BUFFER 100

ViewerText::ViewerText() : text(), args()
{
}

ViewerText::ViewerText(const char *s) : text(std::string(s)), args()
{
}

ViewerText::ViewerText(const std::string &s) : text(s), args()
{
}

ViewerText::ViewerText(const ViewerText &obj) : text(obj.text), args(obj.args)
{
}

ViewerText::~ViewerText()
{
}

bool
ViewerText::empty() const
{
    return text.empty();
}

size_t
ViewerText::size() const
{
    return text.size();
}

ViewerText
ViewerText::operator = (const char *s)
{
    text = std::string(s);
    args.clear();
    return *this;
}

ViewerText
ViewerText::operator = (const std::string &s)
{
    text = s;
    args.clear();
    return *this;
}

ViewerText
ViewerText::operator = (const ViewerText &obj)
{
    text = obj.text;
    args = obj.args;
    return *this;
}

ViewerText
ViewerText::arg(char val)
{
    char s[2] = {'\0', '\0'};
    s[0] = val;
    args.push_back(s);
    return *this;
}

ViewerText
ViewerText::arg(int val)
{
    char s[MAX_VIEWERTEXT_BUFFER];
    memset(s, 0, sizeof(char) * MAX_VIEWERTEXT_BUFFER);
    snprintf(s, MAX_VIEWERTEXT_BUFFER, "%d", val);
    args.push_back(s);
    return *this;
}

ViewerText
ViewerText::arg(unsigned int val)
{
    char s[MAX_VIEWERTEXT_BUFFER];
    memset(s, 0, sizeof(char) * MAX_VIEWERTEXT_BUFFER);
    snprintf(s, MAX_VIEWERTEXT_BUFFER, "%u", val);
    args.push_back(s);
    return *this;
}

ViewerText
ViewerText::arg(long val)
{
    char s[MAX_VIEWERTEXT_BUFFER];
    memset(s, 0, sizeof(char) * MAX_VIEWERTEXT_BUFFER);
    snprintf(s, MAX_VIEWERTEXT_BUFFER, "%ld", val);
    args.push_back(s);
    return *this;
}

#if (SIZEOF_UNSIGNED_INT != SIZEOF_SIZE_T)
ViewerText
ViewerText::arg(size_t val)
{
    return arg((unsigned int)val);
}
#endif

ViewerText
ViewerText::arg(float val)
{
    char s[MAX_VIEWERTEXT_BUFFER];
    memset(s, 0, sizeof(char) * MAX_VIEWERTEXT_BUFFER);
    snprintf(s, MAX_VIEWERTEXT_BUFFER, "%f", val);
    args.push_back(s);
    return *this;
}

ViewerText
ViewerText::arg(double val)
{
    char s[MAX_VIEWERTEXT_BUFFER];
    memset(s, 0, sizeof(char) * MAX_VIEWERTEXT_BUFFER);
    snprintf(s, MAX_VIEWERTEXT_BUFFER, "%g", val);
    args.push_back(s);
    return *this;
}

ViewerText
ViewerText::arg(const std::string &val)
{
    args.push_back(val);
    return *this;
}

static void
ReplaceAll(std::string &str, const std::string& from, const std::string& to)
{
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

std::string
ViewerText::toStdString() const
{
    char key[MAX_VIEWERTEXT_BUFFER];
    std::string transformed(text);
    for(size_t i = 0; i < args.size(); ++i)
    {
        snprintf(key, MAX_VIEWERTEXT_BUFFER, "%%%d", int(i+1));
        std::string keyStr(key);
        ReplaceAll(transformed, keyStr, args[i]);
    }
    return transformed;
}

const std::string &ViewerText::GetText() const
{
    return text;
}

const std::vector<std::string> &
ViewerText::GetArguments() const
{
    return args;
}

std::ostream &
operator << (std::ostream &os, const ViewerText &obj)
{
    os << obj.toStdString();
    return os;
}


// ****************************************************************************
//  Modifications:
//    Kathleen Biagas, Wed Dec 14 17:25:28 PST 2016
//    Reworked logic to do the %d old-key->new-key replacement in tmp here,
//    rather than relying on toStdString, as the call to ReplaceAll doesn't
//    remember position for subsequent calls. In the case of one arg in this,
//    and two args in obj, you would end up with %1 %3 %3 instead of %1 %2 %3.
//
// ****************************************************************************

ViewerText
ViewerText::append(const ViewerText &obj)
{
    if(obj.args.empty())
    {
        text.append(obj.text);
    }
    else
    {
        // Renumber the keys in the obj.text.
        std::string tmp(obj.text);

        char oldkey[MAX_VIEWERTEXT_BUFFER];
        char newkey[MAX_VIEWERTEXT_BUFFER];
        int next = int(args.size()+1);
        size_t pos = 0;
        for(size_t i = 0; i < obj.args.size(); ++i)
        {
            snprintf(oldkey, MAX_VIEWERTEXT_BUFFER, "%%%d", int(i+1));
            snprintf(newkey, MAX_VIEWERTEXT_BUFFER, "%%%d", next++);
            pos = tmp.find(oldkey, pos);
            if (pos != std::string::npos)
            {
                tmp.replace(pos, strlen(oldkey), newkey);
                pos+=strlen(newkey);
            }
            args.push_back(obj.args[i]);
        }
        text.append(tmp);
    }

    return *this;
}

ViewerText
ViewerText::operator += (const ViewerText &obj)
{
    return append(obj);
}

ViewerText
ViewerText::operator + (const ViewerText &obj) const
{
    ViewerText tmp(*this);
    return tmp.append(obj);
}

ViewerText
ViewerText::operator + (const std::string &obj) const
{
    ViewerText tmp(*this);
    tmp.text.append(obj);
    return *this;
}

ViewerText
TR(const char *s)
{
    return ViewerText(s);
}
