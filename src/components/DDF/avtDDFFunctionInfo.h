// ************************************************************************* //
//                            avtDDFFunctionInfo.h                           //
// ************************************************************************* //

#ifndef AVT_DDF_FUNCTION_INFO_H
#define AVT_DDF_FUNCTION_INFO_H

#include <ddf_exports.h>

#include <vector>
#include <string>


class     avtBinningScheme;


// ****************************************************************************
//  Class: avtDDFFunctionInfo
//
//  Purpose:
//      This contains the information about the domain, the codomain, and the
//      binning scheme.  It is the information needed to set up a mapping.
//
//  Note:       The codomain is a mathematical term that can be loosely 
//              interpreted as the range.  The distinction is a little subtle.
//              The "range" of a function is the values that function can 
//              possible create.  The "codomain" is the set of numbers that
//              the function maps onto.  For example, the function "absval"
//              has a range of 0->infinity, but a codomain of the real numbers.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

class AVTDDF_API avtDDFFunctionInfo
{
  public:
                           avtDDFFunctionInfo(avtBinningScheme *,
                                              const std::vector<std::string> &, 
                                              const std::string &);
    virtual               ~avtDDFFunctionInfo();
  
    int                    GetDomainNumberOfTuples(void) const
                                { return tupleNames.size(); };
    const std::string     &GetDomainTupleName(int) const;
    const std::string     &GetCodomainName(void) const
                                { return codomainVariable; };
    avtBinningScheme      *GetBinningScheme(void)
                                { return binningScheme; };

  private:
    avtBinningScheme         *binningScheme;
    std::vector<std::string>  tupleNames;
    std::string               codomainVariable;
};


#endif


