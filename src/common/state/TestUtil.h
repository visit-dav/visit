#ifndef TEST_UTIL_H
#define TEST_UTIL_H
#include <string>

// *******************************************************************
// Struct: TestUtil
//
// Purpose:
//   This struct does a couple operations that I do commonly in my
//   test programs like read a command line, print a test result.
//   I plan on using this to standardize my test output so the 
//   results output could be changed in the future.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 10 15:49:54 PST 2000
//
// Modifications:
//   
// *******************************************************************

struct TestUtil
{
    TestUtil(int, char *[], const std::string &);
    void PrintTestResult(int, int, int, bool);
    int  PassFail(bool *test, int n_tests);

    bool        verbose;
    int         programNumber;
    std::string purpose;
};

#endif
