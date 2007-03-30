// ************************************************************************* //
//                  avtCurrentExtentFilter.h                                 //
// ************************************************************************* //

#ifndef AVT_CURRENT_EXTENT_FILTER_H
#define AVT_CURRENT_EXTENT_FILTER_H
#include <pipeline_exports.h>


#include <avtDatasetToDatasetFilter.h>

// ****************************************************************************
//  Class: avtCurrentExtentFilter
//
//  Purpose:
//    Calculates the current extents, both spatial and data.  Stores them
//    in the output's info. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 2, 2001 
//
// ****************************************************************************

class PIPELINE_API avtCurrentExtentFilter : public avtDatasetToDatasetFilter
{
  public:
                          avtCurrentExtentFilter(){};
    virtual              ~avtCurrentExtentFilter(){}; 

    virtual const char   *GetType(void) {return "avtCurrentExtentFilter";};
    virtual const char   *GetDescription(void) 
                              { return "Calculating Current Extents."; };

  protected:
    virtual void          Execute(void);
    virtual void          RefashionDataObjectInfo(void);

};


#endif


