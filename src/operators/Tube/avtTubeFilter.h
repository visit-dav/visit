// ************************************************************************* //
//  File: avtTubeFilter.h
// ************************************************************************* //

#ifndef AVT_Tube_FILTER_H
#define AVT_Tube_FILTER_H


#include <avtPluginStreamer.h>
#include <TubeAttributes.h>


class vtkDataSet;


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
//    Added RefashionDataObjectInfo to tell it the zone numbers are invalid
//    and that we don't need normals.
//
// ****************************************************************************

class avtTubeFilter : public avtPluginStreamer
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

    virtual void          RefashionDataObjectInfo(void);
    virtual vtkDataSet   *ExecuteData(vtkDataSet *, int, std::string);
};


#endif


