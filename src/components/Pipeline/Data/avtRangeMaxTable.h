// ************************************************************************* //
//                           avtRangeMaxTable.h                              //
// ************************************************************************* //

#ifndef AVT_RANGE_MAX_TABLE_H
#define AVT_RANGE_MAX_TABLE_H
#include <pipeline_exports.h>


// ****************************************************************************
//  Class: avtRangeMaxTable
//
//  Purpose:
//      A table that can efficiently find the maximum value over a range.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 2001
//
// ****************************************************************************

class PIPELINE_API avtRangeMaxTable
{
  public:
                                 avtRangeMaxTable();
    virtual                     ~avtRangeMaxTable();

    void                         SetTable(int, float *);
    float                        GetMaximumOverRange(int, int);

  protected:
    int                          numEntries;
    float                       *entries;
    float                       *powRange;
    int                          logRange;
};


#endif


