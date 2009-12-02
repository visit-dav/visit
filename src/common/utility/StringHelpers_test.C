/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include <StringHelpers.h>
#include <visitstream.h>
#include <vector>
#include <stdio.h>

using namespace StringHelpers;
using std::vector;

int main(int argc, char **argv)
{
    vector<int> falseNegatives;
    vector<int> falsePositives;

    //
    // conversions that should succeed
    //

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
    // more args that conversion specifiers is ok
    if (!ValidatePrintfFormatString("%d %d %d", "int", "int", "int", "int"))
        falseNegatives.push_back(__LINE__);
    if (!ValidatePrintfFormatString("hello world", "dummy"))
        falseNegatives.push_back(__LINE__);
    if (!ValidatePrintfFormatString("firstInt = %d\nsecondFlt=%G\n", "int", "float"))
        falseNegatives.push_back(__LINE__);

    size_t s_to_num_tmp;
    if(str_to_u_numeric<size_t>("42", &s_to_num_tmp) == false ||
       s_to_num_tmp != 42)
        falseNegatives.push_back(__LINE__);

    if(str_to_u_numeric<size_t>("0", &s_to_num_tmp) == false ||
       s_to_num_tmp != 0)
        falseNegatives.push_back(__LINE__);

    if(str_to_u_numeric<size_t>("2147483648", &s_to_num_tmp) == false ||
       s_to_num_tmp != 2147483648UL)
        falseNegatives.push_back(__LINE__);

    //
    // conversions that should fail
    //

    // fewer args than conversion specifiers
    if (ValidatePrintfFormatString("%d %d %d", "int", "int"))
        falsePositives.push_back(__LINE__);
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

#define CHECK_PLURAL(noun,pnoun) 			\
    if (Plural(#noun) != #pnoun)			\
    {							\
        cerr << "Problem pluralizing " #pnoun << endl;	\
        return 1;					\
    }

    CHECK_PLURAL(matrix,matrices);
    CHECK_PLURAL(stratum,strata);
    CHECK_PLURAL(patch,patches);
    CHECK_PLURAL(domain,domains);
    CHECK_PLURAL(source,sources);
    CHECK_PLURAL(assembly,assemblies);

    // test ExtractRESubstr
    string filename = "/usr/gapps/visit/data/foo.silo.bz2";
    string ext = ExtractRESubstr(filename.c_str(), "<\\.(gz|bz|bz2|zip)$>");
    const char *bname = Basename(filename.c_str());
    string dcname = StringHelpers::ExtractRESubstr(bname, "<(.*)\\.(gz|bz|bz2|zip)$> \\1");

    if (ext != ".bz2")
    {
        cerr << "Problem with ExtractRESubstr" << endl;
        return 1;
    }
    if (string(bname) != "foo.silo.bz2")
    {
        cerr << "Problem with Basename" << endl;
        return 1;
    }
    if (dcname != "foo.silo")
    {
        cerr << "Problem with ExtractRESubstr" << endl;
        return 1;
    }

    return falseNegatives.size() + falsePositives.size();

}
