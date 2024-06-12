// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <cassert>
#include <cstdio>
#include <StringHelpers.h>
#include <FileFunctions.h>
#include <visitstream.h>
#include <vector>

using namespace StringHelpers;
using namespace FileFunctions;
using std::vector;
using std::string;

static string slash_swap_for_os(string const in)
{
    string out = in;
#ifdef WIN32
    for (size_t i = 0; i < in.size(); i++)
        if (in[i] == '/') out[i] = '\\';
#endif
    return out;
}

int main(int argc, char **argv)
{
    vector<int> falseNegatives;
    vector<int> falsePositives;

    //
    // Test validation of printf style format strings
    //

    // Validations that should succeed
    if (!ValidatePrintfFormatString("%d %d %d", "int", "int", "int"))
        falseNegatives.push_back(__LINE__);
    if (!ValidatePrintfFormatString("%.3f %#0d", "float", "int"))
        falseNegatives.push_back(__LINE__);
    if (!ValidatePrintfFormatString("%4.f %#0d", "float", "int"))
        falseNegatives.push_back(__LINE__);
    if (!ValidatePrintfFormatString("%f", "float"))
        falseNegatives.push_back(__LINE__);
    if (!ValidatePrintfFormatString("%12.7Lf %d %c", "long double", "int", "char"))
        falseNegatives.push_back(__LINE__);
    // Below is valid. It will use %4.A as the conv. spec and print the 'f' explicitly
    if (!ValidatePrintfFormatString("%4.Af", "float"))
        falseNegatives.push_back(__LINE__);
    if (!ValidatePrintfFormatString("%.f", "float"))
        falseNegatives.push_back(__LINE__);
    if (!ValidatePrintfFormatString("%4.4hd:%.12zX", "short", "size_t"))
        falseNegatives.push_back(__LINE__);
    if (!ValidatePrintfFormatString("hello world %d", "int"))
        falseNegatives.push_back(__LINE__);
    // More args than conversion specifiers is ok
    if (!ValidatePrintfFormatString("%d %d %d", "int", "int", "int", "int"))
        falseNegatives.push_back(__LINE__);
    if (!ValidatePrintfFormatString("hello world", "dummy"))
        falseNegatives.push_back(__LINE__);
    if (!ValidatePrintfFormatString("firstInt = %d\nsecondFlt=%G\n", "int", "float"))
        falseNegatives.push_back(__LINE__);
    // (Strict) Number of conversion specs doesn't match expected number should fail
    if (ValidatePrintfFormatString(4, "%d %d %d", "int", "int", "int", "int"))
        falsePositives.push_back(__LINE__);
    // (Strict) Number of conversion specs matches expected number should succeed 
    if (!ValidatePrintfFormatString(3, "%d %d %d", "int", "int", "int", "int"))
        falseNegatives.push_back(__LINE__);

    size_t s_to_num_tmp;
    s_to_num_tmp = vstrtonum<size_t>("42");
    if (s_to_num_tmp != 42)
        falseNegatives.push_back(__LINE__);

    s_to_num_tmp = vstrtonum<size_t>("0");
    if (s_to_num_tmp != 0)
        falseNegatives.push_back(__LINE__);

    s_to_num_tmp = vstrtonum<size_t>("2147483648");
    if (s_to_num_tmp != 2147483648UL)
        falseNegatives.push_back(__LINE__);

    {
        const std::string lgl_one = "libGL.so.1";
        const std::string lgl = "libGL.so";
        const std::string abs = "/path/to/Whatever.so";
        const std::string spaces = "/spaces/in the/path";
        std::string s = lgl_one + ":" + lgl + ":" + abs + ":" + spaces;
        std::vector<std::string> splitstr = split(s, ':');
        assert(splitstr.size() == 4); // HOOKS_IGNORE
        if(splitstr[0] != lgl_one) { falseNegatives.push_back(__LINE__); }
        if(splitstr[1] != lgl)     { falseNegatives.push_back(__LINE__); }
        if(splitstr[2] != abs)     { falseNegatives.push_back(__LINE__); }
        if(splitstr[3] != spaces)  { falseNegatives.push_back(__LINE__); }
    }

    // Validations that should fail
    {
        std::string s = "::::";
        std::vector<std::string> splitstr = split(s, ':');
        assert(splitstr.size() == 4); // HOOKS_IGNORE
    }
    {
        std::string s = "a:b:c:";
        std::vector<std::string> splitstr = split(s, ':');
        assert(splitstr.size() == 3); // HOOKS_IGNORE
        if(splitstr[0] != "a") { falsePositives.push_back(__LINE__); }
        if(splitstr[1] != "b") { falsePositives.push_back(__LINE__); }
        if(splitstr[2] != "c") { falsePositives.push_back(__LINE__); }
    }
    {
        std::string no_colon = "test";
        std::vector<std::string> splitstr = split(no_colon, ':');
        assert(splitstr.size() == 1); // HOOKS_IGNORE
        if(splitstr[0] != no_colon) { falsePositives.push_back(__LINE__); }
    }

    // fewer args than conversion specifiers
    // Note a valid test on all systems because va_args cannot determine end of arg list
#if 0
    if (ValidatePrintfFormatString("%d %d %d", "int", "int"))
        falsePositives.push_back(__LINE__);
#endif
    // incompatible types and conversion specs
    if (ValidatePrintfFormatString("%d %d", "short", "unsigned"))
        falsePositives.push_back(__LINE__);
    if (ValidatePrintfFormatString("%g %A", "float", "long"))
        falsePositives.push_back(__LINE__);
    // invalid conversion specifiers
    if (ValidatePrintfFormatString("%y %w", "int", "double"))
        falsePositives.push_back(__LINE__);
    // use of special format for width 
    if (ValidatePrintfFormatString("%0+*d", "int", "int"))
        falsePositives.push_back(__LINE__);
    // use of special format for width 
    if (ValidatePrintfFormatString("%0+*2$d", "int", "int"))
        falsePositives.push_back(__LINE__);
    // attempt to use %n
    if (ValidatePrintfFormatString("%s%n", "char*", "int"))
        falsePositives.push_back(__LINE__);
    // attempt to use %m
    if (ValidatePrintfFormatString("%s%m", "char*", "int"))
        falsePositives.push_back(__LINE__);
    // bad data type name
    if (ValidatePrintfFormatString("%d", "foo"))
        falsePositives.push_back(__LINE__);

    if (falseNegatives.size())
    {
        cerr << "Strings in \"" << argv[0] << "\" on the following lines" << endl;
        cerr << "failed when they should have succeeded..." << endl;
        cerr << "    ";
        for (size_t i = 0; i < falseNegatives.size(); i++)
            cerr << falseNegatives[i]-1 << ", ";
        cerr << endl;
    }
    if (falsePositives.size())
    {
        cerr << "Strings in \"" << argv[0] << "\" on the following lines" << endl;
        cerr << "succeeded when they should have failed..." << endl;
        cerr << "    ";
        for (size_t i = 0; i < falsePositives.size(); i++)
            cerr << falsePositives[i]-1 << ", ";
        cerr << endl;
    }

    //
    // Test noun pluralization
    //
#define CHECK_PLURAL(noun,pnoun)                       \
    if (Plural(#noun) != #pnoun)                       \
    {                                                  \
        cerr << "Problem pluralizing " #pnoun << endl; \
        pluralizing_errors++;                          \
    }

    int pluralizing_errors = 0;
    //           Singular noun          Plural form
    //           ----------------------------------
    CHECK_PLURAL(matrix,                matrices);
    CHECK_PLURAL(stratum,               strata);
    CHECK_PLURAL(patch,                 patches);
    CHECK_PLURAL(domain,                domains);
    CHECK_PLURAL(source,                sources);
    CHECK_PLURAL(assembly,              assemblies);

    //
    // test ExtractRESubstr
    //
    string filename = "/usr/gapps/visit/data/foo.silo.bz2";
    string ext = ExtractRESubstr(filename.c_str(), "<\\.(gz|bz|bz2|zip)$>");
    const char *bname = Basename(filename.c_str());
    string dcname = ExtractRESubstr(bname, "<(.*)\\.(gz|bz|bz2|zip)$> \\1");

    int extract_substr_errors = 0;
    if (ext != ".bz2")
    {
        cerr << "Problem with ExtractRESubstr" << endl;
        extract_substr_errors++;
    }
    if (string(bname) != "foo.silo.bz2")
    {
        cerr << "Problem with Basename" << endl;
        extract_substr_errors++;
    }
    if (dcname != "foo.silo")
    {
        cerr << "Problem with ExtractRESubstr" << endl;
        extract_substr_errors++;
    }

    //
    // Test Basename and Dirname
    //

    // Modified: KSB 11-12-20
    // Don't perform 'slash_swap_for_os', which only applied to Windows
    // and isn't necessary for these tests.
#define CHECK_BASENAME_WITH_SUFFIX(_path,_suffix,_expected)           \
    {                                                                 \
        if (string(Basename(_path,_suffix)) != string(_expected))     \
        {                                                             \
            cerr << "Basename(" << _path << "," << _suffix << ") = \""\
                 << Basename(_path,_suffix) << "\", expected \""      \
                 << _expected << "\"" << endl;                        \
            pathname_errors++;                                        \
        }                                                             \
    }
        
#define CHECK_PATHNAMES(_path,_dir,_base)                             \
    {                                                                 \
        if (string(Basename(_path)) != string(_base))                 \
        {                                                             \
            cerr << "Got Basename(" << _path << ") = \""              \
                 << Basename(_path) << "\", expected " << _base << endl; \
            pathname_errors++;                                        \
        }                                                             \
        if (string(Dirname(_path)) != string(_dir))                   \
        {                                                             \
            cerr << "Got Dirname(" << _path << ") = \""               \
                 << Dirname(_path) << "\", expected " << _dir << endl; \
            pathname_errors++;                                        \
        }                                                             \
    }

    int pathname_errors = 0;

    CHECK_BASENAME_WITH_SUFFIX("/foo/bar/gorfo.txt",  ".txt",      "gorfo");
    CHECK_BASENAME_WITH_SUFFIX("../../gorfo.txt.zip", ".txt.zip",  "gorfo");
    CHECK_BASENAME_WITH_SUFFIX("/foo/bar/gorfo.txt",  "gorfo.txt", "gorfo.txt");
    CHECK_BASENAME_WITH_SUFFIX("/foo/bar/gorfo.txt",  "",          "gorfo.txt");

#ifdef WIN32
    // test with unix-style path separators
    CHECK_PATHNAMES("C:/usr/lib",    "C:/usr",        "lib");
    CHECK_PATHNAMES("D:/usr/",       "D:/usr",        "");
    CHECK_PATHNAMES("A:/usr",        "A:/",           "usr");
    CHECK_PATHNAMES("usr",           ".",             "usr");
    CHECK_PATHNAMES("C:/",           "C:/",           "");
    CHECK_PATHNAMES(".",             ".",             ".");
    CHECK_PATHNAMES("..",            ".",             "..");
    // test with windows-style path separators
    CHECK_PATHNAMES("C:\\windows\\test",     "C:\\windows",   "test");
    CHECK_PATHNAMES("C:\\users\\testuser\\", "C:\\users\\testuser", "");
    // test some UNC style paths as well
    CHECK_PATHNAMES("//servername/path/somefile", "//servername/path", "somefile");
    CHECK_PATHNAMES("//servername/path/somedir/", "//servername/path/somedir", "");

#else
    //              Expected behavior as documented in
    //              section 3 of unix manual...
    //              --------------------------------------
    //              path           dirname        basename
    CHECK_PATHNAMES("/usr/lib",    "/usr",        "lib");
    CHECK_PATHNAMES("/usr/",       "/",           "usr");
    CHECK_PATHNAMES("/usr",        "/",           "usr");
    CHECK_PATHNAMES("usr",         ".",           "usr");
    CHECK_PATHNAMES("/",           "/",           "/");
    CHECK_PATHNAMES(".",           ".",           ".");
    CHECK_PATHNAMES("..",          ".",           "..");
#endif

#define CHECK_NORMALIZE(path,result)                                    \
    {                                                                   \
        string _path = slash_swap_for_os(path);                         \
        string _result = slash_swap_for_os(result);                     \
        if (string(Normalize(_path)) != string(_result))                \
        {                                                               \
            cerr << "Got \"" << Normalize(_path)                        \
                 << "\" normalizing " << _path << endl                  \
                 << "Expected " << _result << endl;                     \
            pathname_errors++;                                          \
        }                                                               \
    }

    CHECK_NORMALIZE("./././a/b/c/d/../../e/././f////../..///", "a/b");
    CHECK_NORMALIZE("b/..", ".");
    CHECK_NORMALIZE("a/b/c/d/..////.////./../../..", ".");
    CHECK_NORMALIZE("a/b/c/../../../..", "");

#define CHECK_ABSNAME(cwd,path,result)                                  \
    {                                                                   \
        bool chkval;                                                    \
        string _cwd  = slash_swap_for_os(cwd?cwd:"");                   \
        string _path = slash_swap_for_os(path?path:"");                 \
        string _result = slash_swap_for_os(result);                     \
        if (cwd && path)                                                \
            chkval = string(Absname(_cwd,_path)) != string(_result);    \
        else if (cwd)                                                   \
            chkval = string(Absname(_cwd.c_str(),path)) != string(_result);\
        else                                                            \
            chkval = string(Absname(cwd,_path.c_str())) != string(_result);\
        if (chkval)                                                     \
        {                                                               \
            cerr << "Got \"" << Absname(_cwd,_path)                     \
                 << "\" when forming absolute path name from..." << endl\
                 << _cwd << endl                                        \
                 << _path << endl                                       \
                 << "Expected " << _result << endl;                     \
            pathname_errors++;                                          \
        }                                                               \
    }
    

    CHECK_ABSNAME("/a/b/c",
                  "d/e/f",
                  "/a/b/c/d/e/f");

    CHECK_ABSNAME(0x0,
                  "/d/e/f",
                  "/d/e/f");

    CHECK_ABSNAME("",
                  "/d/e/f",
                  "/d/e/f");

    CHECK_ABSNAME("/a/b/c",
                  0x0,
                  "/a/b/c");

    CHECK_ABSNAME("/a/b/c",
                  "",
                  "/a/b/c");

    CHECK_ABSNAME("/a/b/c",
                  "/d/e/f",
                  "/d/e/f");

    CHECK_ABSNAME("/foo/bar/gorfo/",
                  "./peanut/orange/",
                  "/foo/bar/gorfo/peanut/orange");

    CHECK_ABSNAME("/foo/bar/mark//./../sandy/sue/..////.././steve",
                  "../kerry/apple///banana/./././///../../../grape",
                  "/foo/bar/grape");

#if 0
#define CHECK_STRTONUM(T, S, V, DF, L, E) \
T v = vstrtonum<T>(S); \
if (v != V



    CHECK_STRTONUM("555", 555, 555
    //
    // Check strtonum conversion cases
    //
    char *numstrs[] = {"555", "0555", "0x555", "0x555.5", "555.5", " \t-1", "abcdef", "1.5e-1207",
        "0xFFFFFFFF", "4290000000", "15E+3", "7.652abc"};
    for (int i = 0; i < sizeof(numstrs)/sizeof(numstrs[0]); i++)
    {
        int iv = vstrtonum<int>(numstrs[i], 0, i, "int", &std::cerr);
        double dv = vstrtonum<double>(numstrs[i], 0, i, "double", &std::cerr);
        unsigned int uiv = vstrtonum<unsigned int>(numstrs[i], 0, i, "unsigned", &std::cerr);
        printf("str = \"%s\", int=%d, double=%g, unsigned int=%u\n", numstrs[i], iv, dv, uiv);
    }
#endif



    int all_errors = falseNegatives.size() +
                     falsePositives.size() +
                     pluralizing_errors +
                     extract_substr_errors +
                     pathname_errors;

    return all_errors > 0;
}
