/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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

// ************************************************************************* //
//                          DBYieldedNoDataException.C                       //
// ************************************************************************* //

#include <snprintf.h>
#include <DBYieldedNoDataException.h>
#include <string>

using std::string;

// ****************************************************************************
//  Method: DBYieldedNoDataException constructor
//
//  Programmer: Mark C. Miller
//  Creation:   28Oct10
//
// ****************************************************************************

DBYieldedNoDataException::DBYieldedNoDataException(const string &filename)
{
    char str[1024];
    SNPRINTF(str, sizeof(str),
        "Although VisIt has opened the file, \"%s\",\n"
        "no data was found in the file for VisIt to work with.",
        filename.c_str());
    msg = str;
}

// ****************************************************************************
//  Method: DBYieldedNoDataException constructor
//
//  Programmer: Mark C. Miller
//  Creation:   28Oct10
//
// ****************************************************************************

DBYieldedNoDataException::DBYieldedNoDataException(const string &filename,
    const string &plugin)
{
    char str[1024];
    SNPRINTF(str, sizeof(str),
        "Although VisIt has opened the file, \"%s\",\n"
        "with the %s plugin, no data was found in the file for VisIt to\n"
        "work with.",
        filename.c_str(), plugin.c_str());
    msg = str;
}

// ****************************************************************************
//  Method: DBYieldedNoDataException constructor
//
//  Programmer: Mark C. Miller
//  Creation:   28Oct10
//
// ****************************************************************************

DBYieldedNoDataException::DBYieldedNoDataException(const string &filename,
    const string &plugin, const string &msg2)
{
    char str[2048];
    SNPRINTF(str, sizeof(str),
        "Although VisIt has opened the file, \"%s\",\n"
        "with the %s plugin, no data was found in the file for VisIt to\n"
        "work with. The plugin issued the following error message which may\n"
        "(or may not) be indicative of the problem...\n%s",
        filename.c_str(), plugin.c_str(), msg2.c_str());
    msg = str;
}
