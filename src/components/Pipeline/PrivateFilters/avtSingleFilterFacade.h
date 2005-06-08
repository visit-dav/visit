// ************************************************************************* //
//                           avtSingleFilterFacade.h                         //
// ************************************************************************* //

#ifndef AVT_SINGLE_FILTER_FACADE_H
#define AVT_SINGLE_FILTER_FACADE_H

#include <pipeline_exports.h>

#include <avtFacadeFilter.h>


// ****************************************************************************
//  Class: avtSingleFilterFacade
//
//  Purpose:
//      This is for a class that wants to acts as a facade, but will only be
//      a facade to a single filter (so that class probably is trying to change
//      the interface, like make it a plugin filter).  This class' only 
//      purpose is to decrease the number of virtual functions that the derived
//      types must define.
//
//  Programmer: Hank Childs
//  Creation:   March 16, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Jun  7 15:04:09 PDT 2005
//    Use new interface for creating facaded filters.
//
// ****************************************************************************

class PIPELINE_API avtSingleFilterFacade : public avtFacadeFilter
{
  public:
                              avtSingleFilterFacade();
    virtual                  ~avtSingleFilterFacade();

  protected:
    virtual int               GetNumberOfFacadedFilters(void) { return 1; };
    virtual avtFilter        *GetIthFacadedFilter(int);

    virtual avtFilter        *GetFacadedFilter(void) = 0;
};


#endif


