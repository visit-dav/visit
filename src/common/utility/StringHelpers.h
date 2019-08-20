// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
#include <cerrno>
#include <cstdio>
#include <cstdlib>


#if __GNUC__ >= 3
#   define MUST_CHECK __attribute__ ((warn_unused_result))
#else
#   define MUST_CHECK /*nothing*/
#endif



namespace StringHelpers
{
    const std::string NON_RELEVANT_CHARS =
      "`~!@#$%^&*()-_=+{[}]|\\:;\"'<,>.?/0123456789";

    enum FindResult {FindNone = -1, FindError = -2};

    void UTILITY_API GroupStrings(std::vector<std::string> stringList,
                       std::vector<std::vector<std::string> > &stringGroups,
                       std::vector<std::string> &groupNames,
                       int numLeadingVals = 3,
                       std::string nonRelevantChars = NON_RELEVANT_CHARS);
    void UTILITY_API GroupStringsAsPaths(std::vector<std::string> stringList,
                       std::vector<std::vector<std::string> > &stringGroups,
                       std::vector<std::string> &groupNames);
    void UTILITY_API GroupStringsFixedAlpha(std::vector<std::string> stringList,
                       int numGroups,
                       std::vector<std::vector<std::string> > &stringGroups);
    void UTILITY_API GroupStringsFixedAlpha(
                       const std::set<std::string> &stringList,
                       int numGroups,
                       std::vector<std::set<std::string> > &stringGroups);

    int UTILITY_API FindRE(const std::string &s, const std::string &re);
    int UTILITY_API FindRE(const char *stringToSearch, const char *re);
    bool UTILITY_API ReplaceRE(std::string &s, const std::string &re,
                               const std::string &repl);
    std::string UTILITY_API ExtractRESubstr(const char *stringToSearch,
                                            const char *re);

    bool UTILITY_API ValidatePrintfFormatString(const char *fmtStr, ...);
    bool UTILITY_API ValidatePrintfFormatString(int nargs, const char *fmtStr, ...);

    std::string UTILITY_API car(const std::string, const char separator);
    std::string UTILITY_API cdr(const std::string, const char separator);
    void UTILITY_API append(std::vector<std::string> &,
                            std::vector<std::string>);
    std::vector<std::string> UTILITY_API split(const std::string,
                                               const char separator);
    void UTILITY_API rtrim(std::string &var);
    void UTILITY_API ltrim(std::string &var);
    void UTILITY_API  trim(std::string &var);

    std::string UTILITY_API Replace(const std::string &source,
                                    const std::string &before,
                                    const std::string &after);
    std::string UTILITY_API Plural(const std::string &noun);
    std::string UTILITY_API Plural(int, const std::string &noun);
    std::string UTILITY_API HumanReadableList(const std::vector<std::string>&);
    bool UTILITY_API IsPureASCII(const std::string &txt);
    bool UTILITY_API IsPureASCII(const char *const txt, size_t length);
    bool UTILITY_API CaseInsenstiveEqual(const std::string &str_a,
                                         const std::string &str_b);
    std::string UTILITY_API UpperCase(const std::string &src);

    bool UTILITY_API StringToInt(const std::string &, int &);
    bool UTILITY_API ParseRange(const std::string , std::vector<int> &);
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
//  Modifications:
//
//    Tom Fogal, Fri Aug 29 16:15:17 EDT 2008
//    Reorganized to propagate error upward.
//
//    Tom Fogal, Tue Sep 23 11:08:02 MDT 2008
//    Removed a statically-false branch which was causing an annoying warning.
//
// ****************************************************************************
    template<typename UT>
    MUST_CHECK bool str_to_u_numeric(const char * const s, UT *retval)
    {
        // strtoul() will happily convert a negative string into an unsigned
        // integer.  Do a simple check and bail out if we're given a negative
        // number.
        if(s[0] == '-')
        {
            return false;
        }

        const char *str = s;
        // get rid of leading 0's; they confuse strtoul.
        if(str[0] == '0' && str[1] != '\0' && str[1] != 'x')
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
        *retval = static_cast<UT>(ret);
        switch(errno)
        {
            case 0: /* success */ break;
            case ERANGE:
                // Constant does not fit in sizeof(unsigned long) bytes.
                return false;
                break;
            case EINVAL:
                // Bad base (3rd arg) given; this should be impossible.
                return false;
                break;
            default:
                // Unknown error.
                return false;
                break;
        }
        if(end == s) {
            // junk characters start the string .. is this a number?
            return false;
        }
        return true;
    }
}
#endif
