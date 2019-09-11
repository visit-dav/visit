// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtDataBinningFunctionInfo.h                        //
// ************************************************************************* //

#ifndef AVT_DATA_BINNING_FUNCTION_INFO_H
#define AVT_DATA_BINNING_FUNCTION_INFO_H

#include <dbin_exports.h>

#include <vector>
#include <string>


class     avtBinningScheme;


// ****************************************************************************
//  Class: avtDataBinningFunctionInfo
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
//  Modifications:
//
//    Hank Childs, Sat Aug 21 14:05:14 PDT 2010
//    Renamed from DDF to DataBinning.
//
//    Hank Childs, Mon Aug  1 06:48:26 PDT 2011
//    Add support for spatial coordinates.
//
// ****************************************************************************

class AVTDBIN_API avtDataBinningFunctionInfo
{
  public:
    typedef enum
    {
          VARIABLE, X, Y, Z
    } BinBasedOn;

                         avtDataBinningFunctionInfo(avtBinningScheme *,
                                            const std::vector<std::string> &, 
                                            const std::vector<unsigned char> &, 
                                            const std::string &);
    virtual               ~avtDataBinningFunctionInfo();
  
    int                    GetDomainNumberOfTuples(void) const
                                { return (int)tupleNames.size(); };
    const std::string     &GetDomainTupleName(int) const;
    BinBasedOn             GetBinBasedOnType(int) const;
    const std::string     &GetCodomainName(void) const
                                { return codomainVariable; };
    avtBinningScheme      *GetBinningScheme(void)
                                { return binningScheme; };

  private:
    avtBinningScheme           *binningScheme;
    std::vector<std::string>    tupleNames;
    std::vector<BinBasedOn>     binBasedOn;
    std::string                 codomainVariable;
};


#endif


