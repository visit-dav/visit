/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                        avtDataBinningFunctionInfo.C                       //
// ************************************************************************* //

#include <avtDataBinningFunctionInfo.h>

#include <avtBinningScheme.h>

#include <BadIndexException.h>
#include <ImproperUseException.h>

using     std::string;
using     std::vector;


// ****************************************************************************
//  Method: avtDataBinningFunctionInfo constructor
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
//  Modifications:
//
//    Hank Childs, Mon Aug  1 06:48:26 PDT 2011
//    Add support for bin types.
//
// ****************************************************************************

avtDataBinningFunctionInfo::avtDataBinningFunctionInfo(avtBinningScheme *bs,
                                    const vector<string> &tn, 
                                    const vector<unsigned char> &bn, 
                                    const string &cv)
{
    if (tn.size() <= 0)
    {
        EXCEPTION0(ImproperUseException);
    }

    binningScheme = bs;
    for (int i = 0 ; i < bn.size() ; i++)
        binBasedOn.push_back((BinBasedOn) bn[i]);
    tupleNames = tn;
    codomainVariable = cv;
}


// ****************************************************************************
//  Method: avtDataBinningFunctionInfo destructor
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

avtDataBinningFunctionInfo::~avtDataBinningFunctionInfo()
{
    delete binningScheme;
}


// ****************************************************************************
//  Method: avtDataBinningFunctionInfo::GetDomainTupleName
//
//  Purpose:
//      Gets the domain's tuple name.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

const std::string &
avtDataBinningFunctionInfo::GetDomainTupleName(int idx) const
{
    if (idx < 0 || idx >= tupleNames.size())
    {
        EXCEPTION2(BadIndexException, idx, tupleNames.size());
    }

    return tupleNames[idx];
}


// ****************************************************************************
//  Method: avtDataBinningFunctionInfo::GetBinBasedOnType
//
//  Purpose:
//      Get the category of for the bin ... is it based on X, Y, Z, or a 
//      variable.
//
//  Programmer: Hank Childs
//  Creation:   August 1, 2011
//
// ****************************************************************************

avtDataBinningFunctionInfo::BinBasedOn
avtDataBinningFunctionInfo::GetBinBasedOnType(int idx) const
{
    if (binBasedOn.size() == 0)
        return VARIABLE;

    if (idx < 0 || idx >= binBasedOn.size())
    {
        EXCEPTION2(BadIndexException, idx, binBasedOn.size());
    }

    return binBasedOn[idx];
}


