// ************************************************************************* //
//  File: avtExternalSurfaceFilter.h
// ************************************************************************* //

#ifndef AVT_ExternalSurface_FILTER_H
#define AVT_ExternalSurface_FILTER_H


#include <avtSingleFilterFacade.h>
#include <avtPluginFilter.h>
#include <ExternalSurfaceAttributes.h>


class vtkDataSet;
class avtFacelistFilter;


// ****************************************************************************
//  Class: avtExternalSurfaceFilter
//
//  Purpose:
//      A plugin operator for ExternalSurface.
//
//  Programmer: Jeremy Meredith
//  Creation:   Sat Aug  3 11:54:16 PDT 2002
//
// ****************************************************************************

class avtExternalSurfaceFilter : virtual public avtPluginFilter,
                                 virtual public avtSingleFilterFacade
{
  public:
                         avtExternalSurfaceFilter();
    virtual             ~avtExternalSurfaceFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtExternalSurfaceFilter"; };
    virtual const char  *GetDescription(void)
                             { return "Finding the external surfaces"; };

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

  protected:
    avtFacelistFilter          *ff;
    ExternalSurfaceAttributes   atts;

    virtual avtFilter   *GetFacadedFilter();
};


#endif
