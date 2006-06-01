/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
