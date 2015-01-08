/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
#ifndef VIEWER_TEXT_H
#define VIEWER_TEXT_H
#include <viewercore_exports.h>
#include <iostream>
#include <string>
#include <vector>
#include <visit-config.h>

// ****************************************************************************
// Class: ViewerText
//
// Purpose:
//   String class for viewer status/warning/error messages.
//
// Notes:    This doesn't do *everything* a proper string class does -- just
//           commonly used stuff.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 17:09:16 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API ViewerText
{
public:
    ViewerText();
    ViewerText(const char *s);
    ViewerText(const std::string &s);
    ViewerText(const ViewerText &obj);
    ~ViewerText();

    bool empty() const;
    bool size() const;

    ViewerText operator = (const char *obj);
    ViewerText operator = (const std::string &obj);
    ViewerText operator = (const ViewerText &obj);

    ViewerText arg(char   val);
    ViewerText arg(int    val);
    ViewerText arg(unsigned int val);
    ViewerText arg(long   val);
#if (SIZEOF_UNSIGNED_INT != SIZEOF_SIZE_T)
    ViewerText arg(size_t val);
#endif
    ViewerText arg(float  val);
    ViewerText arg(double val);
    ViewerText arg(const std::string &val);

    std::string toStdString() const;

    ViewerText append(const ViewerText &obj);
    ViewerText operator += (const ViewerText &obj);
    ViewerText operator + (const ViewerText &obj) const;
    ViewerText operator + (const std::string &obj) const;

    const std::string &GetText() const;
    const std::vector<std::string> &GetArguments() const;
private:
    std::string              text;
    std::vector<std::string> args;
};

// Use this decoration function to denote a string that needs translation.
ViewerText VIEWERCORE_API TR(const char *s);

#endif
