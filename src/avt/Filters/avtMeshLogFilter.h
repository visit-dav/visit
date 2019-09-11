// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                  avtMeshLogFilter.h                                       //
// ************************************************************************* //

#ifndef AVT_MESH_LOG_FILTER_H
#define AVT_MESH_LOG_FILTER_H

#include <filters_exports.h>

#include <avtDataTreeIterator.h>

#include <enumtypes.h>


// ****************************************************************************
//  Class: avtMeshLogFilter
//
//  Purpose:
//    Scales the mesh's coordinates by base 10 logarithm.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 6, 2007 
//
//  Modifications:
//    Eric Brugger, Mon Jul 21 14:26:01 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class AVTFILTERS_API avtMeshLogFilter : public avtDataTreeIterator
{
  public:
                          avtMeshLogFilter();
    virtual              ~avtMeshLogFilter(){}; 

    virtual const char   *GetType(void) {return "avtMeshLogFilter";};
    virtual const char   *GetDescription(void) 
                              { return "Calculating base 10 logarithm."; };

    void                  SetXScaleMode(ScaleMode val) { xScaleMode = val; }
    void                  SetYScaleMode(ScaleMode val) { yScaleMode = val; }

    void                  SetUseInvLogX(bool val) { useInvLogX = val; }
    void                  SetUseInvLogY(bool val) { useInvLogY = val; }

  protected:
    ScaleMode             xScaleMode;
    ScaleMode             yScaleMode;
    bool                  useInvLogX;
    bool                  useInvLogY;

    virtual void          PostExecute(void);
    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual void          UpdateDataObjectInfo(void);

  private:
};


#endif


