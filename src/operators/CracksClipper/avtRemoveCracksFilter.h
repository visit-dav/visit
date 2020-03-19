// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtRemoveCracksFilter.h
// ************************************************************************* //

#ifndef AVT_RemoveCracksFilter_H
#define AVT_RemoveCracksFilter_H

#include <avtPluginDataTreeIterator.h>

#include <CracksClipperAttributes.h>

class vtkDataSet;


// ****************************************************************************
//  Class: avtRemoveCracksFilter
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
//    Eric Brugger, Wed Jul 23 11:17:54 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class avtRemoveCracksFilter : public avtPluginDataTreeIterator
{
  public:
                         avtRemoveCracksFilter();
    virtual             ~avtRemoveCracksFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtRemoveCracksFilter"; };
    virtual const char  *GetDescription(void)
                             { return "RemoveCracksFilter"; };

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

  protected:
    CracksClipperAttributes   atts;

    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);

  private:
    bool                  NeedsProcessing(vtkDataSet *, bool *np);
    vtkDataSet           *RemoveCracks(vtkDataSet *inds);
    void                  RemoveExtraArrays(vtkDataSet *ds, bool v = false);
};


#endif
