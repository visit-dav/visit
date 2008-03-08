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

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H
#include <set>
#include "AttributeBase.h"
#include "Field.h"

// ****************************************************************************
//  Class:  Attribute
//
//  Purpose:
//    Abstraction for an AttributeSubject definition.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 28, 2001
//
//  Modifications
//    Jeremy Meredith, Fri Apr 19 15:25:46 PDT 2002
//    Added constants, includes, and a codefile.  Made the HasFunction,
//    PrintFunction, DeleteFunction all check to make sure we are trying
//    to override a builtin function.
//
//    Jeremy Meredith, Wed May  8 15:25:18 PDT 2002
//    Added persistent flag.
//
//    Brad Whitlock, Tue May 14 10:20:00 PDT 2002
//    Added exportAPI and exportInclude members.
//
//    Jeremy Meredith, Thu Oct 17 15:58:29 PDT 2002
//    Added some enhancements for the XML editor.
//
//    Brad Whitlock, Mon Nov 18 12:54:53 PDT 2002
//    Ported it to Windows.
//
//    Jeremy Meredith, Tue Sep 23 17:11:06 PDT 2003
//    Added a check so enableval is not empty before writing the enabler.
//
//    Brad Whitlock, Tue Aug 17 17:23:37 PST 2004
//    Added some code to get around a crash using MSVC7.Net.
//
//    Brad Whitlock, Fri Dec 10 13:51:07 PST 2004
//    Added code to write out the vartypes attribute.
//
//    Jeremy Meredith, Tue Apr 26 12:05:29 PDT 2005
//    Default persistent to true.
//
//    Hank Childs, Tue Aug 16 14:53:44 PDT 2005
//    Add additional variable types.
//
//    Brad Whitlock, Wed Feb 28 19:15:07 PST 2007
//    Added access and keyframe.
//
//    Brad Whitlock, Thu Feb 28 10:01:01 PDT 2008
//    Update to deal with more restricted CodeFile. Also added support for
//    different code targets. Made it inherit from AttributeBase.
//
// ****************************************************************************

class Attribute : public AttributeBase
{
  public:
    vector<Field*> fields;
  public:
    Attribute(const QString &n, const QString &p, const QString &f,
              const QString &e, const QString &ei)
        : AttributeBase(n,p,f,e,ei), fields()
    {
    }

    virtual ~Attribute()
    {
        for (int i = 0; i < fields.size(); ++i)
            delete fields[i];
        fields.clear();
    }

    virtual void Print(ostream &out)
    {
        int i;
        out << "    Attribute: " << name << " (" << purpose << ")" << endl;
        for (i=0; i<fields.size(); i++)
            fields[i]->Print(out);
        for (i=0; i<includes.size(); i++)
            includes[i]->Print(out);
        for (i=0; i<functions.size(); i++)
            functions[i]->Print(out);
        for (i=0; i<constants.size(); i++)
            constants[i]->Print(out);
    }

    void SaveXML(ostream &out, QString indent)
    {
        StartOpenTag(out, "Attribute", indent);
        WriteTagAttr(out, "name", name);
        WriteTagAttr(out, "purpose", purpose);
        WriteTagAttr(out, "persistent", Bool2Text(persistent));
        WriteTagAttr(out, "keyframe", Bool2Text(keyframe));
        WriteTagAttr(out, "exportAPI", exportAPI);
        WriteTagAttr(out, "exportInclude", exportInclude);
        if (codeFile)
            WriteTagAttr(out, "codefile", codeFile->FileBase());
        FinishOpenTag(out);

        int i;
        for (i=0; i<EnumType::enums.size(); i++)
        {
            StartOpenTag(out, "Enum", indent);
            WriteTagAttr(out, "name", EnumType::enums[i]->type);
            FinishOpenTag(out);
            WriteValues(out, EnumType::enums[i]->values, indent);
            WriteCloseTag(out, "Enum", indent);
        }

        for (i=0; i<fields.size(); i++)
        {
            Field *f = fields[i];
            StartOpenTag(out, "Field", indent);
            WriteTagAttr(out, "name", f->name);
            WriteTagAttr(out, "label", f->label);
            WriteTagAttr(out, "type", f->type);
            if (f->type == "att" || f->type == "attVector")
                WriteTagAttr(out, "subtype", f->GetSubtype());
            if (f->type == "enum")
                WriteTagAttr(out, "subtype", f->GetSubtype());
            if (f->length > 1)
                WriteTagAttr(out, "length", Int2Text(f->length));
            if (f->internal)
                WriteTagAttr(out, "internal", Bool2Text(f->internal));
            if (f->ignoreEquality)
                WriteTagAttr(out, "ignoreeq", Bool2Text(f->ignoreEquality));
            if (f->enabler && !f->enableval.empty())
            {
                WriteTagAttr(out, "enabler",
                             f->enabler->name + ":" +
                             JoinValues(f->enableval,','));
            }
            if (f->initcode.size() > 0)
            {
                WriteTagAttr(out, "init", "true");
            }
            if (f->type == "variablename")
            {
                QString varTypeString;
                int mask = 1;
                for(int m = 0; m < 11; ++m)
                {
                    varTypeString += ((f->varTypes & mask) ? "1" : "0");
                    mask = mask << 1;
                }
                WriteTagAttr(out, "vartypes", varTypeString);
            }
            if (f->accessType == Field::AccessPublic)
            {
                WriteTagAttr(out, "access", "public");
            }
            else if (f->accessType == Field::AccessProtected)
            {
                WriteTagAttr(out, "access", "protected");
            }

            FinishOpenTag(out);

            WriteValues(out, f->GetValueAsText(), indent);

            WriteCloseTag(out, "Field", indent);
        }

        // Only take the first of the functions with the same name since we
        // don't need multiple Function XML entries for multiple targets.
        std::set<QString> usedNames;
        for (i=0; i<functions.size(); i++)
        {
            Function *f = functions[i];
            if(usedNames.count(f->name) == 0)
            {
                usedNames.insert(f->name);

                StartOpenTag(out, "Function", indent);
                WriteTagAttr(out, "name", f->name);
                WriteTagAttr(out, "user", Bool2Text(f->user));
                WriteTagAttr(out, "member", Bool2Text(f->member));
                FinishOpenTag(out);

                WriteCloseTag(out, "Function", indent);
            }
        }

        usedNames.clear();
        for (i=0; i<constants.size(); i++)
        {
            Constant *c = constants[i];
            if(usedNames.count(c->name) == 0)
            {
                usedNames.insert(c->name);

                StartOpenTag(out, "Constant", indent);
                WriteTagAttr(out, "name", c->name);
                WriteTagAttr(out, "member", Bool2Text(c->member));
                FinishOpenTag(out);

                WriteCloseTag(out, "Constant", indent);
            }
        }

        for (i=0; i<includes.size(); i++)
        {
            Include *n = includes[i];
            StartOpenTag(out, "Include", indent);
            WriteTagAttr(out, "file", n->destination);
            WriteTagAttr(out, "quoted", Bool2Text(n->quoted));
            if(n->target != "xml2atts")
                WriteTagAttr(out, "target", n->target);
            FinishOpenTag(out);

            WriteValue(out, n->include, indent);

            WriteCloseTag(out, "Include", indent);
        }

        WriteCloseTag(out, "Attribute", indent);
    }

    void SaveCodeFile()
    {
        if (!codeFile)
            return;

        ofstream out(codeFile->FileName(), ios::out);
        if (!out)
            throw "Could not open code file for saving\n";

        int i;
        QString currentTarget = "xml2atts";
        for (i=0; i<codes.size(); i++)
        {
            Code *c = codes[i];
            if(currentTarget != c->target)
            {
                out << "Target: " << c->target << endl;
                currentTarget = c->target;
            }
            out << "Code: " << c->name << endl;
            out << "Prefix:" << endl;
            if (! c->prefix.isEmpty())
            {
                out << c->prefix;
                if (c->prefix.right(1) != "\n")
                    out << endl;
            }
            out << "Postfix:" << endl;
            if (! c->postfix.isEmpty())
            {
                out << c->postfix;
                if (c->postfix.right(1) != "\n")
                    out << endl;
            }
            out << endl;
        }

        for (i=0; i<fields.size(); i++)
        {
            Field *f = fields[i];
            
            for(std::map<QString,QString>::const_iterator it = f->initcode.begin();
                it != f->initcode.end(); ++it)
            {
                QString initcode(it->second);
                if(!initcode.isEmpty())
                {
                    if(currentTarget != it->first)
                    {
                        out << "Target: " << it->first << endl;
                        currentTarget = it->first;
                    }

                    out << "Initialization: " << f->name << endl;
                    out << initcode << endl;
                }
                if (!(initcode.isEmpty()) && !(initcode.right(1) == "\n"))
                    out << endl;
            }
        }

        for (i=0; i<constants.size(); i++)
        {
            Constant *c = constants[i];
            if(currentTarget != c->target)
            {
                out << "Target: " << c->target << endl;
                currentTarget = c->target;
            }
            out << "Constant: " << c->name << endl;
            out << "Declaration: " << c->decl << endl;
            out << "Definition: " << c->def << endl;
            out << endl;
        }

        for (i=0; i<functions.size(); i++)
        {
            Function *f = functions[i];
            if(currentTarget != f->target)
            {
                out << "Target: " << f->target << endl;
                currentTarget = f->target;
            }
            out << "Function: " << f->name << endl;
            out << "Declaration: " << f->decl << endl;
            out << "Definition:" << endl << f->def << endl;
            if (!(f->def.isEmpty()) && !(f->def.right(1) == "\n"))
                out << endl;
        }

        out.close();
    }
};

#endif
