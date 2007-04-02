/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
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

#include "Field.h"
#include "Function.h"
#include "Code.h"
#include "Constant.h"
#include "CodeFile.h"
#include "Include.h"

#include <visitstream.h>

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
// ****************************************************************************

class Attribute
{
  public:
    QString name;
    QString purpose;
    bool    persistent;
    bool    keyframe;
    QString exportAPI;
    QString exportInclude;
    vector<Field*> fields;
    vector<Function*> functions;
    vector<Constant*> constants;
    vector<Include*>  includes;
    vector<Code*>     codes;
    CodeFile *codeFile;
  public:
    Attribute(const QString &n, const QString &p, const QString &f,
              const QString &e, const QString &ei)
        : name(n), purpose(p), exportAPI(e), exportInclude(ei)
    {
        if (name.isNull()) name = "";
        if (purpose.isNull()) purpose = "";
        if (f.isNull())
            codeFile = NULL;
        else
            codeFile = new CodeFile(f);
        if (codeFile)
            codeFile->Parse();
        persistent = true;
        keyframe = true;
    }
    bool HasFunction(const QString &f)
    {
        for (int i=0; i<functions.size(); i++)
            if (functions[i]->name == f && functions[i]->user == false)
                return true;
        return false;
    }
    void PrintFunction(ostream &out, const QString &f)
    {
        for (int i=0; i<functions.size(); i++)
            if (functions[i]->name == f && functions[i]->user == false)
                out << functions[i]->def;
    }
    void DeleteFunction(ostream &out, const QString &f)
    {
        for (int i=0; i<functions.size(); i++)
            if (functions[i]->name == f && functions[i]->user == false)
            {
                for (int j=i+1; j<functions.size(); j++)
                    functions[j-1] = functions[j];
                return;
            }
    }
    void Print(ostream &out)
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
            WriteTagAttr(out, "codefile", codeFile->filebase);
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
            if (! f->initcode.isEmpty())
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

        for (i=0; i<functions.size(); i++)
        {
            Function *f = functions[i];
            StartOpenTag(out, "Function", indent);
            WriteTagAttr(out, "name", f->name);
            WriteTagAttr(out, "user", Bool2Text(f->user));
            WriteTagAttr(out, "member", Bool2Text(f->member));
            FinishOpenTag(out);

            WriteCloseTag(out, "Function", indent);
        }

        for (i=0; i<constants.size(); i++)
        {
            Constant *c = constants[i];
            StartOpenTag(out, "Constant", indent);
            WriteTagAttr(out, "name", c->name);
            WriteTagAttr(out, "member", Bool2Text(c->member));
            FinishOpenTag(out);

            WriteCloseTag(out, "Constant", indent);
        }

        for (i=0; i<includes.size(); i++)
        {
            Include *n = includes[i];
            StartOpenTag(out, "Include", indent);
            WriteTagAttr(out, "file", n->target);
            WriteTagAttr(out, "quoted", Bool2Text(n->quoted));
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

        ofstream out(codeFile->filename, ios::out);
        if (!out)
            throw "Could not open code file for saving\n";

        int i;
        for (i=0; i<codes.size(); i++)
        {
            Code *c = codes[i];
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
            if (! f->initcode.isEmpty())
            {
                out << "Initialization: " << f->name << endl;
                out << f->initcode << endl;
                if (!(f->initcode.isEmpty()) && !(f->initcode.right(1) == "\n"))
                    out << endl;
            }
        }

        for (i=0; i<constants.size(); i++)
        {
            Constant *c = constants[i];
            out << "Constant: " << c->name << endl;
            out << "Declaration: " << c->decl << endl;
            out << "Definition: " << c->def << endl;
            out << endl;
        }

        for (i=0; i<functions.size(); i++)
        {
            Function *f = functions[i];
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
