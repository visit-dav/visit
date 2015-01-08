/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
#include <ViewerText.h>

#include <cstring>
#include <stdio.h>
#include <snprintf.h>

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

bool
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
    SNPRINTF(s, MAX_VIEWERTEXT_BUFFER, "%d", val);
    args.push_back(s);
    return *this;
}

ViewerText
ViewerText::arg(unsigned int val)
{
    char s[MAX_VIEWERTEXT_BUFFER];
    memset(s, 0, sizeof(char) * MAX_VIEWERTEXT_BUFFER);
    SNPRINTF(s, MAX_VIEWERTEXT_BUFFER, "%u", val);
    args.push_back(s);
    return *this;
}

ViewerText
ViewerText::arg(long val)
{
    char s[MAX_VIEWERTEXT_BUFFER];
    memset(s, 0, sizeof(char) * MAX_VIEWERTEXT_BUFFER);
    SNPRINTF(s, MAX_VIEWERTEXT_BUFFER, "%ld", val);
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
    SNPRINTF(s, MAX_VIEWERTEXT_BUFFER, "%f", val);
    args.push_back(s);
    return *this;
}

ViewerText
ViewerText::arg(double val)
{
    char s[MAX_VIEWERTEXT_BUFFER];
    memset(s, 0, sizeof(char) * MAX_VIEWERTEXT_BUFFER);
    SNPRINTF(s, MAX_VIEWERTEXT_BUFFER, "%g", val);
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
        SNPRINTF(key, MAX_VIEWERTEXT_BUFFER, "%%%d", int(i+1));
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

ViewerText
ViewerText::append(const ViewerText &obj)
{
    if(obj.args.empty())
    {
        text.append(obj.text);
    }
    else
    {
        // Renumber the keys in the text.
        ViewerText tmp(obj.text);
        char newkey[MAX_VIEWERTEXT_BUFFER];
        int start = args.size();
        for(size_t i = 0; i < obj.args.size(); ++i)
        {
            SNPRINTF(newkey, MAX_VIEWERTEXT_BUFFER, "%%%d", int(start+i+1));
            tmp.args.push_back(newkey);
        }
        text += tmp.toStdString();
        for(size_t i = 0; i < obj.args.size(); ++i)
            args.push_back(obj.args[i]);
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
