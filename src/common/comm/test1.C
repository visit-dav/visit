#include <RemoteProcess.h>
#include <BadHostException.h>
#include <TestUtil.h>

#define VERBOSE
#define N_TESTS 1

// Prototypes
bool Run_Test1(bool verbose, int *subtest, int *nsubtests);

// *******************************************************************
// Function: main
//
// Purpose:
//   The main function for this test program. It executes the test
//   cases.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 10 15:34:38 PST 2000
//
// Modifications:
//   
// *******************************************************************

int
main(int argc, char *argv[])
{
    int      subtest, nsubtests;
    bool     test[N_TESTS];
    TestUtil util(argc, argv, "Tests RemoteProcess class");

    // Test the RemoteProcess class with a bad hostname.
    test[0] = Run_Test1(util.verbose, &subtest, &nsubtests);
    util.PrintTestResult(1, subtest, nsubtests, test[0]);
    
    return util.PassFail(test, N_TESTS);
}

///////////////////////////////////////////////////////////////////////////
/// Test 1 - Test the RemoteProcess class.
///
/// Notes:
///    This test is designed to test the RemoteProcess class with a bad
///    host name to see if it throws a BadHostException.
///
///////////////////////////////////////////////////////////////////////////

// *******************************************************************
// Function: Run_Test1
//
// Purpose:
//   Test the RemoteProcess class. Try giving it a bad machine name.
//
// Arguments:
//   verbose   : Whether or not to do lots of output.
//   subtest   : A return code to indicate the last subtest to be
//               executed.
//   nsubtests : The number of subtests.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 10 16:09:30 PST 2000
//
// Modifications:
//   
// *******************************************************************

bool
Run_Test1(bool verbose, int *subtest, int *nsubtests)
{
    // We have 1 subtests
    *nsubtests = 1;

#ifdef VERBOSE
    if(verbose)
    {
        cout << "=================================================" << endl;
        cout << "Running Test 1" << endl;
        cout << "=================================================" << endl;
    }
#endif
    bool retval = false;

    // Create a RemoteProcess
    *subtest = 1;
    RemoteProcess remote("foo");

    // Actually try to run the remote process on a machine that
    // does not exist.
    TRY
    {
        remote.Open("nonexistantmachine", 1, 1);
    }
    CATCH2(BadHostException, e)
    {
        cout << e.GetHostName() << "is an invalid host name." << endl;
        retval = true;
    }
    ENDTRY

    return retval;
}
