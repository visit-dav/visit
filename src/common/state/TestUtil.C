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

#include <stdio.h>
#include <TestUtil.h>

// *******************************************************************
// Method: TestUtil::TestUtil
//
// Purpose: 
//   Creates a TestUtil object.
//
// Arguments:
//   argc    : The number of command line arguments
//   argv    : The list of command line arguments.
//   purpose : A string that tells the purpose of the test program.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 10 16:05:18 PST 2000
//
// Modifications:
//   
// *******************************************************************

TestUtil::TestUtil(int argc, char *argv[],
    const std::string &purpose)
{
    // Set default options
    verbose = false;

    // Extract the test program number from argv[0]
    sscanf(argv[0], "test%d", &programNumber);

    // Check the command line flags.
    for(int i = 1; i < argc; ++i)
    {
        std::string current = std::string(argv[i]);
        if(current == std::string("-v"))
            verbose = true;
        else if(current == std::string("-help"))
        {
            cout << "PURPOSE: " << purpose << ".\n";
            cout << "usage: " << argv[0] << " [-v] [-help]" << endl;
            cout << "    -v    = print verbose output" << endl;
            cout << "    -help = print usage and exit" << endl;
            exit(0);
        }
    }

    // Print message
    cout << "Test Program #" << programNumber << " (" << argv[0] << ")"
         << endl;
}

// *******************************************************************
// Method: TestUtil::PrintTestResult
//
// Purpose: 
//   Prints a result from a test case in a standard way... At some
//   point the output might be HTML.
//
// Arguments:
//   testnum   : The major test that was run. This corresponds to a
//               Run_Test# function.
//   subtest   : A subtest within a major test.
//   nsubtests : The number of subtests in the major test.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 10 16:06:21 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
TestUtil::PrintTestResult(int testnum, int subtest, int nsubtests,
    bool result)
{
    char *pf[] = {"fail", "pass"};

    cout << "Test " << programNumber << "." << testnum << ": " 
         << pf[(result ? 1 : 0)];

    if(result)
        cout << endl;
    else if(nsubtests > 1)
        cout << ". Failed on subtest " << subtest << " out of "
             << nsubtests << endl;
}

// *******************************************************************
// Method: TestUtil::PassFail
//
// Purpose: 
//   Looks through the test results array and determines if all tests
//   passed.
//
// Arguments:
//   test    : An array of bool values that are the test results.
//   n_tests : The length of the test array.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 10 16:17:39 PST 2000
//
// Modifications:
//   
// *******************************************************************

int
TestUtil::PassFail(bool *test, int n_tests)
{
    bool retval = true;
    for(int i = 0; (i < n_tests) && retval; ++i)
        retval = test[i];

    return retval ? 0 : -1;
}
