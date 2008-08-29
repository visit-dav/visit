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

// ************************************************************************* //
//                              StringHelpers.h                              //
// ************************************************************************* //

#ifndef STRINGHELPERS_H
#define STRINGHELPERS_H
#include <utility_exports.h>

#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <snprintf.h>

using std::string;
using std::vector;

namespace StringHelpers
{
   const string NON_RELEVANT_CHARS = "`~!@#$%^&*()-_=+{[}]|\\:;\"'<,>.?/0123456789";

   enum FindResult {FindNone = -1, FindError = -2};

   void UTILITY_API GroupStrings(vector<string> stringList,
                     vector<vector<string> > &stringGroups,
                     vector<string> &groupNames,
                     int numLeadingVals = 3,
                     string nonRelevantChars = NON_RELEVANT_CHARS);
   void UTILITY_API GroupStringsAsPaths(vector<string> stringList,
                     vector<vector<string> > &stringGroups,
                     vector<string> &groupNames);
   void UTILITY_API GroupStringsFixedAlpha(vector<string> stringList,
                     int numGroups,
                     vector<vector<string> > &stringGroups);
   void UTILITY_API GroupStringsFixedAlpha(
                     const std::set<std::string> &stringList,
                     int numGroups,
                     vector<std::set<std::string> > &stringGroups);


   int UTILITY_API FindRE(const char *stringToSearch, const char *re);
   string UTILITY_API ExtractRESubstr(const char *stringToSearch, const char *re);

   bool UTILITY_API ValidatePrintfFormatString(const char *fmtStr, const char *arg1Type, ... );

   const char UTILITY_API *Basename(const char *path);
   const char UTILITY_API *Dirname(const char *path);

// ****************************************************************************
//  Function: str_to_u_numeric
//
//  Purpose: Converts a string value into an unsigned numeric type as given by
//           the template parameter.
//           WARNING: This is likely to compile and silently fail if given a
//                    signed type in the template parameter.
//
//  Programmer: Tom Fogal
//  Creation:   August 11, 2008
//
// ****************************************************************************
    template<typename UT>
    UT str_to_u_numeric(const char * const s)
    {
        assert(strlen(s) >= 1);
        // strtoul() will happily convert a negative string into an unsigned
        // integer.  Do a simple check and bail out if we're given a negative
        // number.
        if(s[0] == '-')
        {
            std::cerr << __FUNCTION__ << "Negative input "
                      << "'" << s << "'" << std::endl;
            return 0;
        }

        const char *str = s;
        // get rid of leading 0's; they confuse strtoul.
        if(str[0] == '0' && str[1] != 'x')
        {
            while(*str == '0') { ++str; }
        }

        // One might want to think about switching this to an `unsigned long
        // long' and using `strtoull' below.  That will catch more cases, but
        // this is more portable.
        unsigned long ret;
        char *end;
        errno = 0;
        ret = strtoul(str, &end, 0);
        switch(errno)
        {
            case 0: /* success */ break;
            case ERANGE:
            {
                UT bytes = sizeof(UT);
                std::cerr << __FUNCTION__ << ": constant does not fit in "
                          << bytes << " bytes." << std::endl;
                break;
            }
            case EINVAL:
                perror("bad base (0)?");
                break;
            default:
            {
                char e_msg[1024];
                SNPRINTF(e_msg, 1024, "%s: cannot convert %s to "
                         "unsigned value: %s", __FUNCTION__, s,
                         strerror(errno));
                std::cerr << e_msg << std::endl;
                break;
            }
        }
        if(end == s) {
            std::cerr << __FUNCTION__ << ": illegal unsigned constant "
                      << "'" << s << "'" << std::endl;
        }
        if(ret < 0) {
            std::cerr << __FUNCTION__ << ": cannot convert "
                      << "'" << s << "'" << std::endl;
        }
        return static_cast<UT>(ret);
    }
}

#endif
