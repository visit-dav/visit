// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
    for (size_t i = 0 ; i < bn.size() ; i++)
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
    if (idx < 0 || idx >= (int)tupleNames.size())
    {
        EXCEPTION2(BadIndexException, idx, (int)tupleNames.size());
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

    if (idx < 0 || idx >= (int)binBasedOn.size())
    {
        EXCEPTION2(BadIndexException, idx, (int)binBasedOn.size());
    }

    return binBasedOn[idx];
}


