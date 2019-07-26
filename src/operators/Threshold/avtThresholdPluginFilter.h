// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtThresholdPluginFilter.h
// ************************************************************************* //

#ifndef AVT_THRESHOLD_PLUGIN_FILTER_H
#define AVT_THRESHOLD_PLUGIN_FILTER_H


#include <avtSingleFilterFacade.h>
#include <avtPluginFilter.h>
#include <ThresholdAttributes.h>


class vtkDataSet;
class avtThresholdFilter;


// ****************************************************************************
//  Class: avtThresholdPluginFilter
//
//  Purpose:
//      A plugin operator for threshold.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Dec 10 16:29:05 PST 2010
//
//  Modifications:
//
// ****************************************************************************

class avtThresholdPluginFilter : virtual public avtPluginFilter,
                                 virtual public avtSingleFilterFacade
{
  public:
                         avtThresholdPluginFilter();
    virtual             ~avtThresholdPluginFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtThresholdPluginFilter"; };
    virtual const char  *GetDescription(void)
                             { return "Thresholding"; };

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

  protected:
    avtThresholdFilter   *tf;
    ThresholdAttributes   atts;

    virtual avtFilter         *GetFacadedFilter();
    virtual const avtFilter   *GetFacadedFilter() const;

    static void UpdateDataObjectInfoCB(avtDataObject_p &input,
                                       avtDataObject_p &output, void *);
};


#endif
