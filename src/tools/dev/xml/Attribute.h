// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H
#include <QFile>
#include <QTextStream>
#include "AttributeBase.h"
#include "Field.h"

#include <set>
#include <vector>

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
//    Brad Whitlock, Fri Apr 25 11:57:45 PDT 2008
//    Added different access types for functions.
//
//    Brad Whitlock, Thu May  8 11:15:30 PDT 2008
//    Qt 4. Use QTextStream.
//
//    Cyrus Harrison, Mon Sep 29 08:42:39 PDT 2008
//    Fixed attempt to open the code file twice.
//
//    Mark C. Miller, Wed Aug 26 10:57:41 PDT 2009
//    Added custom base class for derived state objects.
//
//    Kathleen Biagas, Thu Aug 25 14:17:22 MST 2011
//    Added persistent flag for fields.
//
// ****************************************************************************

class Attribute : public AttributeBase
{
  public:
    std::vector<Field*> fields;
  public:
    Attribute(const QString &n, const QString &p, const QString &f,
              const QString &e, const QString &ei, const QString &bc)
        : AttributeBase(n,p,f,e,ei,bc), fields()
    {
    }

    virtual ~Attribute()
    {
        for (size_t i = 0; i < fields.size(); ++i)
            delete fields[i];
        fields.clear();
    }

    virtual void Print(QTextStream &out) const
    {
        size_t i;
        out << "    Attribute: " << name << " (" << purpose << ")" << Endl;
        for (i=0; i<fields.size(); i++)
            fields[i]->Print(out);
        for (i=0; i<includes.size(); i++)
            includes[i]->Print(out);
        for (i=0; i<functions.size(); i++)
            functions[i]->Print(out);
        for (i=0; i<constants.size(); i++)
            constants[i]->Print(out);
    }

    void SaveXML(QTextStream &out, QString indent)
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
        if (custombase)
            WriteTagAttr(out, "baseClass", baseClass);

        FinishOpenTag(out);

        size_t i;
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
            if (!f->persistent)
                WriteTagAttr(out, "persistent", Bool2Text(f->persistent));
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
                if(f->accessType == Function::AccessProtected)
                    WriteTagAttr(out, "access", "protected");
                else if(f->accessType == Function::AccessPrivate)
                    WriteTagAttr(out, "access", "private");
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

        QFile *f = new QFile(codeFile->FileName());
        if (!f->open(QIODevice::WriteOnly | QIODevice::Text))
        {
            delete f;
            throw "Could not open code file for saving\n";
        }
        QTextStream out(f);

        size_t i;
        QString currentTarget = "xml2atts";
        for (i=0; i<codes.size(); i++)
        {
            Code *c = codes[i];
            if(currentTarget != c->target)
            {
                out << "Target: " << c->target << Endl;
                currentTarget = c->target;
            }
            out << "Code: " << c->name << Endl;
            out << "Prefix:" << Endl;
            if (! c->prefix.isEmpty())
            {
                out << c->prefix;
                if (c->prefix.right(1) != "\n")
                    out << Endl;
            }
            out << "Postfix:" << Endl;
            if (! c->postfix.isEmpty())
            {
                out << c->postfix;
                if (c->postfix.right(1) != "\n")
                    out << Endl;
            }
            out << Endl;
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
                        out << "Target: " << it->first << Endl;
                        currentTarget = it->first;
                    }

                    out << "Initialization: " << f->name << Endl;
                    out << initcode << Endl;
                }
                if (!(initcode.isEmpty()) && !(initcode.right(1) == "\n"))
                    out << Endl;
            }
        }

        for (i=0; i<constants.size(); i++)
        {
            Constant *c = constants[i];
            if(currentTarget != c->target)
            {
                out << "Target: " << c->target << Endl;
                currentTarget = c->target;
            }
            out << "Constant: " << c->name << Endl;
            out << "Declaration: " << c->decl << Endl;
            out << "Definition: " << c->def << Endl;
            out << Endl;
        }

        for (i=0; i<functions.size(); i++)
        {
            Function *f = functions[i];
            if(currentTarget != f->target)
            {
                out << "Target: " << f->target << Endl;
                currentTarget = f->target;
            }
            out << "Function: " << f->name << Endl;
            out << "Declaration: " << f->decl << Endl;
            out << "Definition:" << Endl << f->def << Endl;
            if (!(f->def.isEmpty()) && !(f->def.right(1) == "\n"))
                out << Endl;
        }

        f->close();
        delete f;
    }
};

#endif
