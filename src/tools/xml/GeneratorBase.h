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
#ifndef GENERATOR_BASE_H
#define GENERATOR_BASE_H
#include <AttributeBase.h>
#include <stdlib.h>
#include <sys/types.h>

#if !defined(_WIN32)
#include <sys/time.h>
#else
#include <time.h>
#endif
#include <visit-config.h>
#ifdef TIME_WITH_SYS_TIME
#include <time.h>
#endif

#include <snprintf.h>
#if defined(_WIN32)
#include <windows.h>
#endif

// ****************************************************************************
// Class: GeneratorBase
//
// Purpose:
//   Base class for field generator classes, which contains code that will
//   be needed by subclasses to generate code for their fields.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 5 11:45:12 PDT 2008
//
// Modifications:
//    Jeremy Meredith, Thu Aug  7 14:55:48 EDT 2008
//    Use const char * for string literals.
//   
// ****************************************************************************

class GeneratorBase : public AttributeBase
{
public:
    QString generatorName;
public:
    GeneratorBase(const QString &n, const QString &p, const QString &f,
                      const QString &e, const QString &ei, const QString &gName)
        : AttributeBase(n,p,f,e,ei), generatorName(gName)
    {
    }

    virtual ~GeneratorBase()
    {
    }

    bool HasFunction(const QString &f) const
    {
        return AttributeBase::HasFunction(f, generatorName);
    }
    void PrintFunction(ostream &out, const QString &f) const
    {
        AttributeBase::PrintFunction(out, f, generatorName);
    }
    void DeleteFunction(ostream &out, const QString &f)
    {
        AttributeBase::DeleteFunction(out, f, generatorName);
    }

    bool HasCode(const QString &cName, int part) const
    {
        return AttributeBase::HasCode(cName, part, generatorName);
    }
    void PrintCode(ostream &out, const QString &cName, int part) const
    {
        AttributeBase::PrintCode(out, cName, part, generatorName);
    }

    static QString
    CurrentTime()
    {
        const char *tstr[] = {"PDT", "PST"};
        char s1[10], s2[10], s3[10], tmpbuf[200];
        time_t t;
        char *c = NULL;
        int h,m,s,y;
        t = time(NULL);
        c = asctime(localtime(&t));
        // Read the hour.
        sscanf(c, "%s %s %s %d:%d:%d %d", s1, s2, s3, &h, &m, &s, &y);
        // Reformat the string a little.
        sprintf(tmpbuf, "%s %s %s %02d:%02d:%02d %s %d",
                s1, s2, s3, h, m, s, tstr[h > 12], y);

        return QString(tmpbuf);
    }

    void
    WriteMethodComment(ostream &out, const QString &className,
                       const QString &methodName, const QString &purposeString) const
    {
        out << "// ****************************************************************************" << endl;
        out << "// Method: " << className << "::" << methodName << endl;
        out << "//" << endl;
        out << "// Purpose: " << endl;
        out << "//   " << purposeString << endl;
        out << "//" << endl;
        out << "// Note:       Autogenerated by " << generatorName << "." << endl;
        out << "//" << endl;
        out << "// Programmer: " << generatorName << endl;
        out << "// Creation:   omitted" << endl;
        out << "//" << endl;
        out << "// Modifications:" << endl;
        out << "//   " << endl;
        out << "// ****************************************************************************" << endl;
        out << endl;
    }

    void
    WriteClassComment(ostream &h, const QString &purposeString) const
    {
        h << "// ****************************************************************************" << endl;
        h << "// Class: " << name << endl;
        h << "//" << endl;
        h << "// Purpose:" << endl;
        h << "//    " << purposeString << endl;
        h << "//" << endl;
        h << "// Notes:      Autogenerated by " << generatorName << "." << endl;
        h << "//" << endl;
        h << "// Programmer: " << generatorName << endl;
        h << "// Creation:   omitted" << endl;
        h << "//" << endl; 
        h << "// Modifications:" << endl;
        h << "//   " << endl;
        h << "// ****************************************************************************" << endl;
        h << endl;
    }

    void
    WriteClassComment(ostream &h, const QString &cName, const QString &purposeString) const
    {
        h << "// ****************************************************************************" << endl;
        h << "// Class: " << cName << endl;
        h << "//" << endl;
        h << "// Purpose:" << endl;
        h << "//    " << purposeString << endl;
        h << "//" << endl;
        h << "// Notes:      Autogenerated by " << generatorName << "." << endl;
        h << "//" << endl;
        h << "// Programmer: " << generatorName << endl;
        h << "// Creation:   omitted" << endl;
        h << "//" << endl; 
        h << "// Modifications:" << endl;
        h << "//   " << endl;
        h << "// ****************************************************************************" << endl;
        h << endl;
    }
};

#endif
