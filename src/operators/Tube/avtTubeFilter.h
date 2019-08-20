// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtTubeFilter.h
// ************************************************************************* //

#ifndef AVT_Tube_FILTER_H
#define AVT_Tube_FILTER_H

#include <avtPluginDataTreeIterator.h>

#include <TubeAttributes.h>


// ****************************************************************************
//  Class: avtTubeFilter
//
//  Purpose:
//      A plugin operator for Tube.
//
//  Programmer: childs<generated>
//  Creation:   August28,2001
//
//  Modifications:
//    Jeremy Meredith, Fri Nov  1 19:28:27 PST 2002
//    Added UpdateDataObjectInfo to tell it the zone numbers are invalid
//    and that we don't need normals.
//
//    Hank Childs, Wed Oct 20 20:30:37 PDT 2010
//    Add support for scaling by fraction of the bounding box.
//
//    Eric Brugger, Tue Aug 19 09:44:48 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class avtTubeFilter : public avtPluginDataTreeIterator
{
  public:
                         avtTubeFilter();
    virtual             ~avtTubeFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtTubeFilter"; };
    virtual const char  *GetDescription(void)
                             { return "Transforming lines into tubes"; };

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

  protected:
    TubeAttributes   atts;
    double           scaleFactor;

    virtual void          PreExecute(void);
    virtual void          UpdateDataObjectInfo(void);
    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual avtContract_p ModifyContract(avtContract_p);
};


#endif


