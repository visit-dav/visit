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

#ifndef FUNCTION_H
#define FUNCTION_H

#include <QTextStream>

// ****************************************************************************
//  Class:  Function
//
//  Purpose:
//    Abstraction for a function, either a new (user-defined) one or one which
//    overrides an existing one.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 28, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Apr 19 15:32:51 PDT 2002
//    Renamed some stuff.  Pulled most of the code into CodeFile.h.
//
//    Hank Childs, Tue Aug 28 14:36:10 PDT 2007
//    Add a Boolean to indicate whether the function was used.
//
//    Brad Whitlock, Wed Feb 27 15:55:37 PST 2008
//    Added target.
//
//    Brad Whitlock, Fri Apr 25 11:13:20 PDT 2008
//    Added accessType
//
//    Brad Whitlock, Thu May  8 11:35:41 PDT 2008
//    Qt 4. Use QTextStream.
//
// ****************************************************************************
class Function
{
  public:
    typedef enum {AccessPrivate, AccessProtected, AccessPublic} AccessType;

    QString name;
    QString decl;
    QString def;
    bool    user;
    bool    member;
    QString target;
    bool    usedThisFunction;
    AccessType accessType;
  public:
    Function(const QString &n, const QString &dc, const QString &df, bool u,
        bool m, const QString &t) : name(n), decl(dc), def(df), user(u), 
        member(m), target(t), accessType(AccessPublic)
    {
        usedThisFunction = false;
    }
    void Print(QTextStream &out, const QString &generatorName = QString::null)
    {
        if(generatorName.isEmpty() || generatorName == target)
        {
            out << "        Function: (" << target;
            if(accessType == AccessPrivate)
                out << ",private";
            else if(accessType == AccessProtected)
                out << ",protected";
            out << ") " << decl << endl;
        }
    }
};

#endif
