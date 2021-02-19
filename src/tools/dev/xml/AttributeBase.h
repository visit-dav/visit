// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ATTRIBUTE_BASE_H
#define ATTRIBUTE_BASE_H
#include <QTextStream>
#include "Function.h"
#include "Code.h"
#include "Constant.h"
#include "CodeFile.h"
#include "Include.h"

#include <vector>

// ****************************************************************************
// Class: AttributeBase
//
// Purpose:
//   Base class for Attribute
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 7 11:44:37 PDT 2008
//
// Modifications:
//
//    Mark C. Miller, Wed Aug 26 11:02:41 PDT 2009
//    Added support for custom base class for derived state objects.
//
//    Hank Childs, Mon Nov  8 21:26:55 PST 2010
//    Make default persistent value be true.
//
// ****************************************************************************

class AttributeBase
{
public:
    QString            name;
    QString            purpose;
    bool               persistent;
    bool               keyframe;
    QString            exportAPI;
    QString            exportInclude;
    std::vector<Function*>  functions;
    std::vector<Constant*>  constants;
    std::vector<Include*>   includes;
    std::vector<Code*>      codes;
    CodeFile          *codeFile;
    bool               custombase;
    QString            baseClass;

public:
    AttributeBase(const QString &n, const QString &p, const QString &f,
                  const QString &e, const QString &ei,
                  const QString &bc = "AttributeSubject")
        : name(n),
          purpose(p),
          persistent(true),
          keyframe(true),
          exportAPI(e),
          exportInclude(ei),
          functions(),
          constants(),
          includes(),
          codes(),
          codeFile(NULL),
          custombase(false),
          baseClass(bc)
    {
        if (name.isNull()) name = "";
        if (purpose.isNull()) purpose = "";
        if (f.isNull())
            codeFile = NULL;
        else
            codeFile = new CodeFile(f);
        if (codeFile)
            codeFile->Parse();
        if (baseClass.isNull() || baseClass == "")
            baseClass = "AttributeSubject";
        if (baseClass == "AttributeSubject")
            custombase = false;
    }

    virtual ~AttributeBase()
    {
        size_t i;
        for (i = 0; i < functions.size(); ++i)
            delete functions[i];
        functions.clear();
        for (i = 0; i < constants.size(); ++i)
            delete constants[i];
        constants.clear();
        for (i = 0; i < includes.size(); ++i)
            delete includes[i];
        includes.clear();
        for (i = 0; i < codes.size(); ++i)
            delete codes[i];
        codes.clear();

        delete codeFile;
    }

    bool HasFunction(const QString &f, const QString &target = QString()) const
    {
        for (size_t i=0; i<functions.size(); i++)
            if (functions[i]->name == f && functions[i]->user == false)
            {
                if(target.isEmpty() || target == functions[i]->target)
                    return true;
            }
        return false;
    }
    void PrintFunction(QTextStream &out, const QString &f, const QString &target = QString()) const
    {
        for (size_t i=0; i<functions.size(); i++)
            if (functions[i]->name == f && functions[i]->user == false)
            {
                if(target.isEmpty() || target == functions[i]->target)
                {
                    out << functions[i]->def;
                    functions[i]->usedThisFunction = true;
                    return;
                }
            }
    }

    void DeleteFunction(QTextStream &out, const QString &f, const QString &target = QString())
    {
        for (size_t i=0; i<functions.size(); i++)
            if (functions[i]->name == f && functions[i]->user == false)
            {
                if(target.isEmpty() || target == functions[i]->target)
                {
                    for (size_t j=i+1; j<functions.size(); j++)
                        functions[j-1] = functions[j];
                    return;
                }
            }
    }

    bool HasCode(const QString &cName, int part, const QString &target = QString()) const
    {
        for (size_t i=0; i<codes.size(); i++)
            if (codes[i]->name == cName)
            {
                if(target.isEmpty() || target == codes[i]->target)
                {
                    bool retval = false;
                    if(part == 0)
                        retval = !codes[i]->prefix.isNull();
                    else
                        retval = !codes[i]->postfix.isNull();
                    return retval;
                }
            }
        return false;
    }
    void PrintCode(QTextStream &out, const QString &cName, int part, const QString &target = QString()) const
    {
        for (size_t i=0; i<codes.size(); i++)
            if (codes[i]->name == cName)
            {
                if(target.isEmpty() || target == codes[i]->target)
                {
                    if(part == 0)
                        out << codes[i]->prefix;
                    else
                        out << codes[i]->postfix;
                    return;
                }
            }
    }

    virtual void Print(QTextStream &) const
    {
    }
};

#endif
