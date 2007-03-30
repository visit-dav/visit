// ************************************************************************* //
//  File: avtInverseGhostZoneFilter.h
// ************************************************************************* //

#ifndef AVT_InverseGhostZone_FILTER_H
#define AVT_InverseGhostZone_FILTER_H


#include <avtPluginStreamer.h>
#include <InverseGhostZoneAttributes.h>


class vtkDataSet;


// ****************************************************************************
//  Class: avtInverseGhostZoneFilter
//
//  Purpose:
//      A plugin operator for InverseGhostZone.
//
//  Programmer: childs<generated>
//  Creation:   July27,2001
//
// ****************************************************************************

class avtInverseGhostZoneFilter : public avtPluginStreamer
{
  public:
                         avtInverseGhostZoneFilter();
    virtual             ~avtInverseGhostZoneFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtInverseGhostZoneFilter"; };
    virtual const char  *GetDescription(void)
                             { return "InverseGhostZone"; };

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

  protected:
    InverseGhostZoneAttributes   atts;

    virtual vtkDataSet   *ExecuteData(vtkDataSet *, int, std::string);
    virtual void          RefashionDataObjectInfo(void);
};


#endif


