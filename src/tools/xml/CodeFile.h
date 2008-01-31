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

#ifndef CODEFILE_H
#define CODEFILE_H

#include <map>
#include <utility>
#include <qstring.h>
#include <visitstream.h>

#include <XMLParserUtil.h>

// ****************************************************************************
//  Class:  CodeFile
//
//  Purpose:
//    Encapsulates all code for an attribute.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 12, 2002
//
//  Modifications:
//    Jeremy Meredith, Thu Oct 17 15:58:29 PDT 2002
//    Added some enhancements for the XML editor.
//
//    Jeremy Meredith, Thu Jan 31 14:55:32 EST 2008
//    Prevent hang on badly formatted .code files.
//
// ****************************************************************************
class CodeFile
{
  public:
    QString filename;
    QString filepath;
    QString filebase;
    std::map<QString, std::pair<QString,QString> > code;
    std::map<QString, std::pair<QString,QString> > func;
    std::map<QString, std::pair<QString,QString> > var;
    std::map<QString, std::pair<QString,QString> > constant;
    std::map<QString, QString>                     init;
  public:
    CodeFile(const QString &f) : filename(f)
    {
        filepath=FilePath(filename);
        filebase=FileBase(filename);
    }
    void Parse()
    {
        ifstream in(filename.latin1(), ios::in);
        if (!in)
            throw QString().sprintf("Couldn't find file %s\n",filename.latin1());

        char cbuff[4096];
        QString buff;

        in.getline(cbuff, 4096);
        buff=cbuff;

        while (in)
        {
            QString keyword = GetKeyword(buff);
            if (!keyword.isNull())
            {
                QString name = buff.mid(keyword.length()).simplifyWhiteSpace();
                if (keyword      == "Function:")
                {
                    ParseFunction(buff, name, in);
                }
                else if (keyword == "Code:")
                {
                    ParseCode(buff, name, in);
                }
                else if (keyword == "Variable:")
                {
                    ParseVariable(buff, name, in);
                }
                else if (keyword == "Constant:")
                {
                    ParseConstant(buff, name, in);
                }
                else if (keyword == "Initialization:")
                {
                    ParseInitialization(buff, name, in);
                }
            }
            else
                break;
        }

        in.close();
    }
    QString GetKeyword(const QString &buff)
    {
        QString keyword;

        if (buff.left(9)       == "Function:")
        {
            keyword = buff.left(9);
        }
        else if (buff.left(5)  == "Code:")
        {
            keyword = buff.left(5);
        }
        else if (buff.left(9)  == "Variable:")
        {
            keyword = buff.left(9);
        }
        else if (buff.left(9)  == "Constant:")
        {
            keyword = buff.left(9);
        }
        else if (buff.left(15) == "Initialization:")
        {
            keyword = buff.left(15);
        }

        return keyword;
    }
    void ParseFunction(QString &buff, const QString &name, istream &in)
    {
        QString decl;
        QString def;

        char cbuff[4096];
        in.getline(cbuff, 4096);
        buff=cbuff;
        if (buff.left(12) != "Declaration:")
            throw "'Declaration:' must follow 'Function:'";
        decl=buff.mid(12).stripWhiteSpace();

        in.getline(cbuff, 4096);
        buff=cbuff;
        if (buff.left(11) != "Definition:")
            throw "'Definition:' must follow 'Declaration:'";
        in.getline(cbuff, 4096);
        buff=cbuff;
        while (in && GetKeyword(buff).isNull())
        {
            def += buff + "\n";
            in.getline(cbuff, 4096);
            buff=cbuff;
        }

        while (def.right(2) == "\n\n")
            def = def.left(def.length() - 1);

        func[name] = std::pair<QString,QString>(decl,def);
    }
    void ParseVariable(QString &buff, const QString &name, istream &in)
    {
    }
    void ParseConstant(QString &buff, const QString &name, istream &in)
    {
        QString decl;
        QString def;

        char cbuff[4096];
        in.getline(cbuff, 4096);
        buff=cbuff;
        if (buff.left(12) != "Declaration:")
            throw "'Declaration:' must follow 'Function:'";
        decl=buff.mid(12).stripWhiteSpace();

        in.getline(cbuff, 4096);
        buff=cbuff;
        if (buff.left(11) != "Definition:")
            throw "'Definition:' must follow 'Declaration:'";
        def=buff.mid(11).stripWhiteSpace();

        in.getline(cbuff, 4096);
        buff=cbuff;
        while (in && GetKeyword(buff).isNull())
        {
            def += buff + "\n";
            in.getline(cbuff, 4096);
            buff=cbuff;
        }

        while (def.right(2) == "\n\n")
            def = def.left(def.length() - 1);

        constant[name] = std::pair<QString,QString>(decl,def);
    }
    void ParseCode(QString &buff, const QString &name, istream &in)
    {
        QString prefix,postfix;

        char cbuff[4096];
        in.getline(cbuff, 4096);
        buff=cbuff;

        if (buff.left(7) != "Prefix:")
            throw "'Prefix:' must follow 'Code:'";
        in.getline(cbuff, 4096);
        buff=cbuff;
        while (in && buff.left(8) != "Postfix:")
        {
            prefix += buff + "\n";
            in.getline(cbuff, 4096);
            buff=cbuff;
        }

        if (buff.left(8) != "Postfix:")
            throw "'Postfix:' must follow 'Prefix:'";
        in.getline(cbuff, 4096);
        buff=cbuff;
        while (in && GetKeyword(buff).isNull())
        {
            postfix += buff + "\n";
            in.getline(cbuff, 4096);
            buff=cbuff;
        }

        // chomp extra trailing whitespace
        while (prefix.right(2) == "\n\n")
            prefix = prefix.left(prefix.length() - 1);
        if (prefix == "\n")
            prefix = QString();

        while (postfix.right(2) == "\n\n")
            postfix = postfix.left(postfix.length() - 1);
        if (postfix == "\n")
            postfix = QString();

        code[name] = std::pair<QString,QString>(prefix,postfix);
    }
    void ParseInitialization(QString &buff, const QString &name, istream &in)
    {
        QString initcode;

        char cbuff[4096];
        in.getline(cbuff, 4096);
        buff=cbuff;

        while (in && GetKeyword(buff).isNull())
        {
            initcode += buff + "\n";
            in.getline(cbuff, 4096);
            buff=cbuff;
        }

        while (initcode.right(2) == "\n\n")
        {
            initcode = initcode.left(initcode.length() - 1);
        }

        init[name] = initcode;
    }
};

#endif
