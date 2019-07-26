// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtCracksClipperFilter.h
// ************************************************************************* //

#ifndef AVT_CracksClipper_FILTER_H
#define AVT_CracksClipper_FILTER_H


#include <avtPluginFilter.h>
#include <avtDatasetToDatasetFilter.h>
#include <CracksClipperAttributes.h>


class vtkDataSet;


// ****************************************************************************
//  Class: avtCracksClipperFilter
//
//  Purpose:
//    A plugin operator for clipping away Cracks.
//
//  Programmer: Kathleen Bonnell
//  Creation:   Thu Oct 13 08:17:36 PDT 2005
//
//  Modifications:
//    Kathleen Bonnell, Fri Oct 13 11:05:01 PDT 2006
//    Removed int arg from RemoveCracks method.
//
//    Kathleen Bonnell, Thu May  3 07:51:38 PDT 2007
//    Changed inheritance for this filter, so that it can create a pipeline
//    with multiple filters.  Moved bulk of code to new avtRemoveCracksFilter.
//
//    Kathleen Bonnell, Wed Sep 29 08:58:10 PDT 2010
//    Add ivar 'calculateDensity'.
//
//    Kathleen Biagas, Tue Aug 14 15:05:23 MST 2012
//    Add ivar 'varname'.
//
// ****************************************************************************

class avtCracksClipperFilter : virtual public avtPluginFilter,
                               virtual public avtDatasetToDatasetFilter
{
  public:
                         avtCracksClipperFilter();
    virtual             ~avtCracksClipperFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtCracksClipperFilter"; };
    virtual const char  *GetDescription(void)
                             { return "CracksClipper"; };

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

  protected:
    CracksClipperAttributes   atts;
    bool                  calculateDensity;
    std::string           varname;

    virtual void          Execute(void);
    virtual void          PostExecute(void);
    virtual void          UpdateDataObjectInfo(void);
    virtual int           AdditionalPipelineFilters(void);
    virtual avtContract_p ModifyContract(avtContract_p);
};


#endif
