// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
class avtGhostZoneAndFacelistFilter;


// ****************************************************************************
//  Class: avtExternalSurfaceFilter
//
//  Purpose:
//      A plugin operator for ExternalSurface.
//
//  Programmer: Jeremy Meredith
//  Creation:   Sat Aug  3 11:54:16 PDT 2002
//
//  Modifications:
//
//    Hank Childs, Tue Aug 23 09:49:57 PDT 2005
//    Add support for ghost zone removal.
//
//    Tom Fogal, Tue Jun 23 20:48:20 MDT 2009
//    Added const version of GetFacadedFilter.
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
    avtFacelistFilter               *ff;
    avtGhostZoneAndFacelistFilter   *gz_and_ff;
    ExternalSurfaceAttributes        atts;

    virtual avtFilter         *GetFacadedFilter();
    virtual const avtFilter   *GetFacadedFilter() const;

    static void UpdateDataObjectInfoCB(avtDataObject_p &input,
                                       avtDataObject_p &output, void *);
};

#endif
