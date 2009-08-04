/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
#include <QFile>
#include <QStringList>
#include <QTextStream>

#include <map>
#include <utility>

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
//    Brad Whitlock, Wed Feb 27 14:35:40 PST 2008
//    Added support for other XML tool targets in the code file. Encapsulated
//    the data and added accessor methods.
//
//    Brad Whitlock, Thu May  8 11:26:07 PDT 2008
//    Qt 4. Use QTextStream.
//
//    Cyrus Harrison, Mon Nov 24 09:10:42 PST 2008
//    Fixed problem where last char of a function could be truncated.
//
// ****************************************************************************
class CodeFile
{
  private:
    typedef std::map<QString, std::pair<QString,QString> > QStringPairMap;
    typedef std::map<QString, QString> QStringQStringMap;
    QString currentTarget;
    QString filename;
    QString filepath;
    QString filebase;
    QStringPairMap    code;
    QStringPairMap    func;
    QStringPairMap    var;
    QStringPairMap    constant;
    QStringQStringMap init;
  public:
    CodeFile(const QString &f) : filename(f)
    {
        currentTarget = "xml2atts";
        filepath = ::FilePath(filename);
        filebase = ::FileBase(filename);
    }
    const QString &FileBase() const { return filebase; }
    void           SetFileBase(const QString &s) { filebase = s; }

    const QString &FilePath() const              { return filepath; }
    void           SetFilePath(const QString &s) { filepath = s; }

    const QString &FileName() const { return filename; }
    void           SetFileName(const QString &s) { filename = s; }

    void Parse()
    {
        QFile *f = new QFile(filename);
        if (!f->open(QIODevice::ReadOnly | QIODevice::Text))
        {
            delete f;
            throw QString("Couldn't find file %1\n").arg(filename);
        }
        QTextStream in(f);

        QString buff = in.readLine();

        while (!in.atEnd())
        {
            QString keyword = GetKeyword(buff);
            if (!keyword.isNull())
            {
                QString name = buff.mid(keyword.length()).simplified();
                if (keyword      == "Target:")
                {
                    ParseTarget(buff, name, in);
                }
                else if (keyword      == "Function:")
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

        f->close();
        delete f;
    }

    bool HasCode(const QString &name) const
    { 
        return HasItem(code, name); 
    }
    bool GetCode(const QString &name, QStringList &targets, QStringList &first, 
                 QStringList &second) const
    {
        return GetItem(code, name, targets, first, second);
    }
    void GetAllCodes(QStringList &targets, QStringList &names, QStringList &first, 
                 QStringList &second) const
    {
        GetAllItems(code, targets, names, first, second);
    }

    bool HasFunction(const QString &name) const
    { 
        return HasItem(func, name); 
    }
    bool GetFunction(const QString &name, QStringList &targets, QStringList &first, 
                 QStringList &second) const
    {
        return GetItem(func, name, targets, first, second);
    }
    void GetAllFunctions(QStringList &targets, QStringList &names, QStringList &first, 
                 QStringList &second) const
    {
        GetAllItems(func, targets, names, first, second);
    }

    bool HasVariable(const QString &name) const
    { 
        return HasItem(var, name); 
    }
    bool GetVariable(const QString &name, QStringList &targets, QStringList &first, 
                 QStringList &second) const
    {
        return GetItem(var, name, targets, first, second);
    }
    void GetAllVariables(QStringList &targets, QStringList &names, QStringList &first, 
                 QStringList &second) const
    {
        GetAllItems(var, targets, names, first, second);
    }

    bool HasConstant(const QString &name) const
    { 
        return HasItem(constant, name); 
    }
    bool GetConstant(const QString &name, QStringList &language, QStringList &first, 
                     QStringList &second) const
    {
        return GetItem(constant, name, language, first, second);
    }
    void GetAllConstants(QStringList &targets, QStringList &names, QStringList &first, 
                 QStringList &second) const
    {
        GetAllItems(constant, targets, names, first, second);
    }

    bool HasInit(const QString &name) const
    { 
        return HasItem(init, name); 
    }
    bool GetInit(const QString &name, QStringList &targets, QStringList &def) const
    {
        return GetItem(init, name, targets, def);
    }
    void GetAllInits(QStringList &targets, QStringList &names, QStringList &def) const
    {
        GetAllItems(init, targets, names, def);
    }
private:
    QString GetKeyword(const QString &buff)
    {
        QString keyword;

        if (buff.left(9)       == "Function:")
        {
            keyword = buff.left(9);
        }
        else if (buff.left(7)  == "Target:")
        {
            keyword = buff.left(7);
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
    void ParseTarget(QString &buff, const QString &name, QTextStream &in)
    {
        currentTarget = name.trimmed();

        // Get the next line.
        buff = in.readLine();
    }
    void ParseFunction(QString &buff, const QString &name, QTextStream &in)
    {
        QString decl;
        QString def;

        buff = in.readLine();
        if (buff.left(12) != "Declaration:")
            throw "'Declaration:' must follow 'Function:'";
        decl=buff.mid(12).trimmed();

        buff = in.readLine();
        if (buff.left(11) != "Definition:")
            throw "'Definition:' must follow 'Declaration:'";

        buff = in.readLine();
        while (!in.atEnd() && GetKeyword(buff).isNull())
        {
            def += buff + "\n";
            buff = in.readLine();
            if(in.atEnd())
                def += buff + "\n";
        }

        while (def.right(2) == "\n\n")
            def = def.left(def.length() - 1);

        func[Key(name)] = std::pair<QString,QString>(decl,def);
    }
    void ParseVariable(QString &buff, const QString &name, QTextStream &in)
    {
    }
    void ParseConstant(QString &buff, const QString &name, QTextStream &in)
    {
        QString decl;
        QString def;

        buff = in.readLine();
        if (buff.left(12) != "Declaration:")
            throw "'Declaration:' must follow 'Function:'";
        decl=buff.mid(12).trimmed();

        buff = in.readLine();
        if (buff.left(11) != "Definition:")
            throw "'Definition:' must follow 'Declaration:'";
        def=buff.mid(11).trimmed();

        buff = in.readLine();
        while (!in.atEnd() && GetKeyword(buff).isNull())
        {
            def += buff + "\n";
            buff = in.readLine();
        }

        while (def.right(2) == "\n\n")
            def = def.left(def.length() - 1);

        constant[Key(name)] = std::pair<QString,QString>(decl,def);
    }
    void ParseCode(QString &buff, const QString &name, QTextStream &in)
    {
        QString prefix,postfix;

        buff = in.readLine();
        if (buff.left(7) != "Prefix:")
            throw "'Prefix:' must follow 'Code:'";
        buff = in.readLine();
        while (!in.atEnd() && buff.left(8) != "Postfix:")
        {
            prefix += buff + "\n";
            buff = in.readLine();
        }

        if (buff.left(8) != "Postfix:")
            throw "'Postfix:' must follow 'Prefix:'";
        buff = in.readLine();
        while (!in.atEnd() && GetKeyword(buff).isNull())
        {
            postfix += buff + "\n";
            buff = in.readLine();
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

        code[Key(name)] = std::pair<QString,QString>(prefix,postfix);
    }
    void ParseInitialization(QString &buff, const QString &name, QTextStream &in)
    {
        QString initcode;

        buff = in.readLine();

        while (!in.atEnd() && GetKeyword(buff).isNull())
        {
            initcode += buff + "\n";
            buff = in.readLine();
        }

        while (initcode.right(2) == "\n\n")
        {
            initcode = initcode.left(initcode.length() - 1);
        }

        init[Key(name)] = initcode;
    }

    QString Key(const QString &key) const
    {
        return MakeKey(currentTarget, key);
    }

    QString MakeKey(const QString &target, const QString &key) const
    {
        return target + "," + key;
    }

    void SplitKey(const QString &key, QString &target, QString &name) const
    {
        int comma = key.indexOf(",");
        if(comma != -1)
        {
            target = key.left(comma);
            name = key.mid(comma+1);
        }
        else
        {
            target = "xml2atts";
            name = key;
        }
    }

    bool HasItem(const QStringPairMap &m, const QString &name) const
    {
        for(QStringPairMap::const_iterator it = m.begin(); it != m.end(); ++it)
        {
            QString target, key;
            SplitKey(it->first, target, key);
            if(key == name)
                return true;
        }
        return false;
    }

    bool GetItem(const QStringPairMap &m, const QString &name, 
                 QStringList &targets, QStringList &first, 
                 QStringList &second) const
    {
        bool retval = false;
        for(QStringPairMap::const_iterator it = m.begin(); it != m.end(); ++it)
        {
            QString target, key;
            SplitKey(it->first, target, key);
            if(key == name)
            {
                targets += target;
                first += it->second.first;
                second += it->second.second;
                retval = true;
            }
        }
        return retval;
    }

    void GetAllItems(const QStringPairMap &m,
                     QStringList &targets,
                     QStringList &names,
                     QStringList &first, 
                     QStringList &second) const
    {
        for(QStringPairMap::const_iterator it = m.begin(); it != m.end(); ++it)
        {
            QString target, key;
            SplitKey(it->first, target, key);
            
            targets += target;
            names += key;
            first += it->second.first;
            second += it->second.second;
        }
    }

    void GetAllItems(const QStringQStringMap &m,
                     QStringList &targets,
                     QStringList &names,
                     QStringList &defs) const
    {
        for(QStringQStringMap::const_iterator it = m.begin(); it != m.end(); ++it)
        {
            QString target, key;
            SplitKey(it->first, target, key);
            
            targets += target;
            names += key;
            defs += it->second;
        }
    }

    bool HasItem(const QStringQStringMap &m, const QString &name) const
    {
        for(QStringQStringMap::const_iterator it = m.begin(); it != m.end(); ++it)
        {
            QString target, key;
            SplitKey(it->first, target, key);
            if(key == name)
                return true;
        }
        return false;
    }

    bool GetItem(const QStringQStringMap &m, const QString &name, 
                 QStringList &targets, QStringList &first) const
    {
        bool retval = false;
        for(QStringQStringMap::const_iterator it = m.begin(); it != m.end(); ++it)
        {
            QString target, key;
            SplitKey(it->first, target, key);
            if(key == name)
            {
                targets += target;
                first += it->second;
                retval = true;
            }
        }
        return retval;
    }
};

#endif
