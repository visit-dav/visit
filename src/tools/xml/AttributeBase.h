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
#ifndef ATTRIBUTE_BASE_H
#define ATTRIBUTE_BASE_H
#include <vector>

#include <QTextStream>
#include "Function.h"
#include "Code.h"
#include "Constant.h"
#include "CodeFile.h"
#include "Include.h"

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
    vector<Function*>  functions;
    vector<Constant*>  constants;
    vector<Include*>   includes;
    vector<Code*>      codes;
    CodeFile          *codeFile;

public:
    AttributeBase(const QString &n, const QString &p, const QString &f,
                  const QString &e, const QString &ei)
        : name(n),
          purpose(p), 
          persistent(false),
          keyframe(true),
          exportAPI(e),
          exportInclude(ei),
          functions(),
          constants(),
          includes(),
          codes(),
          codeFile(NULL)
    {
        if (name.isNull()) name = "";
        if (purpose.isNull()) purpose = "";
        if (f.isNull())
            codeFile = NULL;
        else
            codeFile = new CodeFile(f);
        if (codeFile)
            codeFile->Parse();
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

    bool HasFunction(const QString &f, const QString &target = QString::null) const
    {
        for (size_t i=0; i<functions.size(); i++)
            if (functions[i]->name == f && functions[i]->user == false)
            {
                if(target.isEmpty() || target == functions[i]->target)
                    return true;
            }
        return false;
    }
    void PrintFunction(QTextStream &out, const QString &f, const QString &target = QString::null) const
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

    void DeleteFunction(QTextStream &out, const QString &f, const QString &target = QString::null)
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

    bool HasCode(const QString &cName, int part, const QString &target = QString::null) const
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
    void PrintCode(QTextStream &out, const QString &cName, int part, const QString &target = QString::null) const
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
