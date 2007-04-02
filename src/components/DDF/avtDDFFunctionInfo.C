// ************************************************************************* //
//                           avtDDFFunctionInfo.C                            //
// ************************************************************************* //

#include <avtDDFFunctionInfo.h>

#include <avtBinningScheme.h>

#include <BadIndexException.h>
#include <ImproperUseException.h>

using     std::string;
using     std::vector;


// ****************************************************************************
//  Method: avtDDFFunctionInfo constructor
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

avtDDFFunctionInfo::avtDDFFunctionInfo(avtBinningScheme *bs,
                                    const vector<string> &tn, const string &cv)
{
    if (tn.size() <= 0)
    {
        EXCEPTION0(ImproperUseException);
    }

    binningScheme = bs;
    tupleNames = tn;
    codomainVariable = cv;
}


// ****************************************************************************
//  Method: avtDDFFunctionInfo destructor
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

avtDDFFunctionInfo::~avtDDFFunctionInfo()
{
    delete binningScheme;
}


// ****************************************************************************
//  Method: avtDDFFunctionInfo::GetDomainTupleName
//
//  Purpose:
//      Gets the domain's tuple name.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

const std::string &
avtDDFFunctionInfo::GetDomainTupleName(int idx) const
{
    if (idx < 0 || idx >= tupleNames.size())
    {
        EXCEPTION2(BadIndexException, idx, tupleNames.size());
    }

    return tupleNames[idx];
}


