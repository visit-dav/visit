// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtDataRangeSelection.h                           //
// ************************************************************************* //

#ifndef AVT_DATA_RANGE_SELECTION_H
#define AVT_DATA_RANGE_SELECTION_H
#include <float.h>
#include <string>
 
#include <pipeline_exports.h>

#include <ref_ptr.h>

#include <avtDataSelection.h>

// ****************************************************************************
//  Class: avtDataRangeSelection
//
//  Purpose: Specify a data selection by a scalar range for a named variable.
// 
//  The default is a range from -FLT_MAX to +FLT_MAX for variable "default"
//
//  Programmer: Markus Glatter
//  Creation:   July 27, 2007
//
//  Modifications:
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
//    Hank Childs, Tue Dec 20 14:43:08 PST 2011
//    Add method DescriptionString.
//
// ****************************************************************************

class PIPELINE_API avtDataRangeSelection : public avtDataSelection 
{
  public:
                            avtDataRangeSelection();
                            avtDataRangeSelection(const std::string _var, 
                                                  const double _min, 
                                                  const double _max);
    virtual                ~avtDataRangeSelection();

    virtual const char *    GetType() const
                                { return "Data Range Selection"; }; 
    virtual std::string     DescriptionString(void);

    void                    SetVariable(const std::string _var)
                                { var = _var; };
    void                    SetMin(const double _min)
                                { min = _min; };
    void                    SetMax(const double _max)
                                { max = _max; };

    std::string             GetVariable() const
                                { return var; };
    double                  GetMin() const
                                { return min; };
    double                  GetMax() const
                                { return max; };

    bool                    operator==(const avtDataRangeSelection &s) const;

  private:
    std::string var;
    double min;
    double max;

    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                            avtDataRangeSelection(const avtDataRangeSelection&)
                                                                           {;};
    avtDataRangeSelection  &operator=(const avtDataRangeSelection &) 
                                                             { return *this; };
};

typedef ref_ptr<avtDataRangeSelection> avtDataRangeSelection_p;


#endif
