// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtDataBinning.h                              //
// ************************************************************************* //

#ifndef AVT_DATA_BINNING_H
#define AVT_DATA_BINNING_H

#include <dbin_exports.h>

#include <vector>
#include <string>


class     vtkDataArray;
class     vtkDataSet;

class     avtBinningScheme;
class     avtDataBinningFunctionInfo;


// ****************************************************************************
//  Class: avtDataBinning
//
//  Purpose:
//      This class represents a data binning, which allows for data
//      to be represented on new sampling spaces.  
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
//  Modifications:
//
//    Hank Childs, Thu Mar 30 12:43:24 PST 2006
//    Add method OutputDDF.
//
//    Hank Childs, Sat Aug 21 14:05:14 PDT 2010
//    Renamed from DDF to DataBinning.
//
// ****************************************************************************

class AVTDBIN_API avtDataBinning
{
  public:
                                   avtDataBinning(avtDataBinningFunctionInfo *, float *);
    virtual                       ~avtDataBinning();

    avtDataBinningFunctionInfo    *GetFunctionInfo(void) { return functionInfo; };
    vtkDataArray                  *ApplyFunction(vtkDataSet *);
    vtkDataSet                    *CreateGrid(void);
    void                           OutputDataBinning(const std::string &);

  protected: 
    avtDataBinningFunctionInfo    *functionInfo;
    float                         *vals;
};


#endif


