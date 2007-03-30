#ifndef CODEFILE_H
#define CODEFILE_H

#include <map>
#include <utility>
#include <qstring.h>
#include <fstream.h>

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
