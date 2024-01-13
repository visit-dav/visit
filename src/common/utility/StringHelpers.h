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
#include <fstream>
#include <set>
#include <string>
#include <vector>
#include <cerrno>
#include <cstdio>
#include <cstdlib>

#include <maptypes.h>  // for CIStringSet, CIStringSetVector defs

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
    // The following override has containers using CaseInsenstive comparator
    void UTILITY_API GroupStringsFixedAlpha(
                       const CIStringSet &stringList,
                       int numGroups,
                       CIStringSetVector &stringGroups);

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

    std::vector<std::string> UTILITY_API split(const std::string &input,
                                               const char separator);

    void UTILITY_API rtrim(std::string &var);
    void UTILITY_API ltrim(std::string &var);
    void UTILITY_API  trim(std::string &var);
    // name aligns with C++20 std::string::ends_with
    bool UTILITY_API ends_with(const std::string &var, const std::string &test);

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
    bool UTILITY_API ParseRange(const std::string &, std::vector<int> &);

    std::string UTILITY_API  EscapeSpecialChars(const std::string &str);

    // ****************************************************************************
    //  Function: vstrtonum
    //
    //  Purpose: Replacement for strtoX() and atoX() methods.
    //
    //           Instead of any of these kinds of uses...
    //
    //               int k = atoi(numstr);
    //               float f1 = atof(numstr);
    //
    //               unsigned u = (unsigned) strtoul(numstr, 0);
    //               if (errno != 0) u = 0xFFFFFFFF; // set to default val
    //
    //               float f2 = (float) strtod(numstr, 0);
    //               if (errno != 0) {
    //                   f2 = -1.0; // set to default value
    //                   debug5 << numstr << " bad value" << endl; // log error
    //               }
    //
    //           ...do this...
    //
    //               int k = vstrtonum<int>(numstr);
    //               float f1 = vstrtonum<float>(numstr);
    //
    //               unsigned u = vstrtonum<unsigned>(numstr, 0xFFFFFFFF);
    //               float f = vstrtonum<float>(numstr, -1.0, debug5);
    //
    //           Templatized methods to convert strings to language-native typed
    //           numeric values, perform some minimal error checking and optionally
    //           emit error messages with potentially useful context when errors
    //           are encountered.
    //
    //           This should always be used in place of strtoX() or atoX() when
    //           reading ascii numerical data.
    //
    //           We do a minimal amount of error checking for a signed conversion
    //           by checking if first non-whitespace character is a minus sign and
    //           artificially setting errno to EDOM (not something strtoX/atoX
    //           would ever do). We could add more error checking for different
    //           cases too by, for example, checking value read for int type and
    //           seeing if it is too big to fit in an int. We currently do not
    //           do this but it would be easy to add. We could also easily add
    //           logic for other, less frequently used types such as shorts or
    //           maybe int64_t, etc.
    //
    //           The default method treats all ascii as long double for the strtoX
    //           conversion and then casts it to correct type. We specialize some
    //           cases for slightly better behavior.
    //
    //           I ran performance tests on macOS doing 1 million conversions with
    //           these methods (including error checking) and 1 million directly
    //           with strtoX and atoX methods and observed no significant diffs
    //           in performance. In addition, keep in mind that these methods are
    //           typically being used in conjunction with file I/O, which almost
    //           certainly dominates performance. The only time this might not be
    //           true is for memory resident "files", mmaps, and/or SSDs.
    //
    //           At some point, it would make sense to enhance this to use locale
    //           so we can handle different character encodings as well as regional
    //           specific interpretations (e.g. European 1.234.456,89). The idea
    //           would be to set the locale VisIt is using (a pref. maybe) and then
    //           these methods would just use that locale in calls to strtoX. That
    //           could be achieved globally in VisIt with a call to setlocale().
    //           However, when in the United States reading an ascii data file
    //           formatted for human readability in Germany the desire would be
    //           to specify "de_DE" for the locale during the read of just that
    //           file suggesting something more complicated than just a global
    //           setting.
    //
    //  Mark C. Miller, Wed Jan 10 17:10:21 PST 2024
    // ****************************************************************************
    template<typename T> inline T _vstrtonum(char const *numstr, char **eptr, int /* unused */) { return static_cast<T>(strtold(numstr, eptr)); }

    // Specialize int/long cases to use int conversion strtol which with base of 0 can handle octal and hex also
    #define _VSTRTONUMI(T,F) template<> inline T _vstrtonum<T>(char const *numstr, char **eptr, int base) { return static_cast<T>(F(numstr, eptr, base)); }
    _VSTRTONUMI(int,std::strtol)
    _VSTRTONUMI(long,std::strtol)
    _VSTRTONUMI(long long,std::strtoll)

    // Specialize unsigned cases to use unsigned conversion strtoul and error checking passing negated arg
    // Note that size_t is an alias almost certainly to one of these types and so we do not have to
    // explicitly handle it here but any caller can still use it.
    #define _VSTRTONUMU(T,F) template<> inline T _vstrtonum<T>(char const *numstr, char **eptr, int base) { char const *s=numstr; while (isspace(*s)) s++; T retval = static_cast<T>(F(numstr, eptr, base)); if (*s=='-') errno = EDOM; return retval;}
    _VSTRTONUMU(unsigned int,std::strtoul)
    _VSTRTONUMU(unsigned long,std::strtoul)
    _VSTRTONUMU(unsigned long long,std::strtoull)

    // dummy ostream for default (no ostream) cases 
    static std::ostream NO_OSTREAM(std::cerr.rdbuf());

    template<typename T> T
    inline vstrtonum(char const *numstr, int base = 10, T dfltval = 0, std::ostream& errstrm = NO_OSTREAM)
    {
        char *eptr = 0;
        errno = 0;
        T retval = _vstrtonum<T>(numstr, &eptr, base);
        int errno_save = errno;
    
        // emit possible error messages
        if (errno_save != 0)
        {
            retval = dfltval;
            if (&errstrm != &NO_OSTREAM)
            {
                errstrm << "Problem converting \"" << numstr << "\" to a number (\"" << strerror(errno_save) << "\")" << std::endl;
            }
        }
        if (eptr == numstr)
        {
            retval = dfltval;
            if (&errstrm != &NO_OSTREAM)
            {
                errstrm << "Problem converting \"" << numstr << "\" to a number (\"no digits to convert\")" << std::endl;
            }
        }
    
        return retval;
    }
}
#endif
