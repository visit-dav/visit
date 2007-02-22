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

#ifndef ENUM_H
#define ENUM_H

#include <qstring.h>
#include <visitstream.h>
#include <vector>
using std::vector;

// ****************************************************************************
//  Class:  EnumType
//
//  Purpose:
//    Abstraction for an enumerated type.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 28, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Oct 17 15:57:44 PDT 2002
//    Moved the static data to a .C file.
//
//    Eric Brugger, Mon Jul 26 15:00:00 PDT 2004
//    I changed cout to out references in the Print method.
//
// ****************************************************************************
class EnumType
{
  public:
    static vector<EnumType*> enums;
    static EnumType *FindEnum(const QString &s)
    {
        EnumType *e = NULL;
        for (int i=0; i<enums.size(); i++)
        {
            if (enums[i]->type == s)
            {
                e = enums[i];
            }
        }
        if (!e)
            throw QString().sprintf("unknown enum subtype '%s'",s.latin1());
        return e;
    }
  public:
    QString         type;
    vector<QString> values;
  public:
    EnumType(const QString &s) : type(s) 
    { 
    }
    void AddValue(const QString &s)
    {
        values.push_back(s);
    }
    const QString& GetValue(int index)
    {
        if (index < 0  ||  index >= values.size())
            throw QString().sprintf("tried to access out-of-bounds enum type %d",index);
        return values[index];
    }
    void Print(ostream &out)
    {
        out << "Enum: " << type << endl;
        for (int i=0; i<values.size(); i++)
        {
            out << "    " << values[i] << endl;
        }
    }
};

#endif
