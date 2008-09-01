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
#include <string>
#include <vector>

#include <VisItDisplay.h>
#include <DebugStream.h>
#include <MesaDisplay.h>
#include <snprintf.h>
#include <StringHelpers.h>
#include <XDisplay.h>

// ****************************************************************************
//  Function: format
//
//  Purpose: Replace special formatters within a string with particular values.
//           Currently `%l' expands to the display, and `%n' expands to the
//           node ID.
//
//  Programmer: Tom Fogal
//  Creation:   August 5, 2008
//
//  Modifications:
//
//    Tom Fogal, Thu Aug  7 16:43:49 EDT 2008
//    Use a debug stream instead of cerr.  Use SNPRINTF macro instead of
//    calling the function directly.
//
// ****************************************************************************
std::string
format(std::string s, size_t node, size_t display)
{
    std::string::size_type percent;
    if((percent = s.find('%')) != std::string::npos) {
        std::string::iterator start = s.begin() + percent;
        /* assume all formatters are '%x': i.e., always 2 characters */
        std::string::iterator end = s.begin() + (percent+2);
        std::string::iterator type = s.begin() + (percent+1);

        if(*type == 'l') {
            char str_display[8];
            SNPRINTF(str_display, 8, "%zu", display);
            s.replace(start, end, str_display);
        } else if(*type == 'n') {
            char str_node[8];
            SNPRINTF(str_node, 8, "%zu", node);
            s.replace(start, end, str_node);
        } else {
            debug5 << "unknown formatter '" << *type << "'" << std::endl;
        }
    }
    // If they gave multiple %'s in the same string, recurse.
    if(s.find('%') != std::string::npos) {
        return format(s, node, display);
    }
    return s;
}

// ****************************************************************************
//  Function: split
//
//  Purpose: Splits a string into a vector of strings, separated by spaces.
//           Converts node and display IDs (via format specifiers) as it
//           proceeds.
//
//  Programmer: Tom Fogal
//  Creation:   August 5, 2008
//
// ****************************************************************************
std::vector<std::string>
split(std::string str, size_t node, size_t display)
{
    namespace SH = StringHelpers;
    std::vector<std::string> ret;
    ret.push_back(format(SH::car(str), node, display));
    if(str.find(' ') != std::string::npos) {
        SH::append(ret, split(SH::cdr(str), node, display));
    }
    return ret;
}

// ****************************************************************************
//  Function: Display::Create
//
//  Purpose:
//    Frontend method to create the appropriate type of display.
//
//  Programmer: Tom Fogal
//  Creation:   September 1, 2008
//
// ****************************************************************************
VisItDisplay *
Display::Create(enum visitDisplayType vtype)
{
    switch(vtype)
    {
        case D_MESA:
            return new MesaDisplay();
            break;
        case D_X:
            return new XDisplay();
            break;
    }
    return NULL;  // unreachable.
}
