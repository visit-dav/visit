// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtSurfaceFilter.h                            //
// ************************************************************************* //

#ifndef AVT_SURFACE_FILTER_H
#define AVT_SURFACE_FILTER_H

#include <filters_exports.h>

#include <avtDataTreeIterator.h>

#include <SurfaceFilterAttributes.h>


// ****************************************************************************
//  Class: avtSurfaceFilter
//
//  Purpose:
//    A filter that turns a 2d dataset into a 3d dataset based upon
//    scaled point or cell data. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 05, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Mar 29 13:54:27 PST 2001 
//    Added method 'SkewTheValue'.
//
//    Kathleen Bonnell, Tue Apr 10 11:46:11 PDT 2001
//    Renamed ExecuteDomain as ExecuteData.
//
//    Hank Childs, Tue Jun 12 13:52:27 PDT 2001
//    Renamed several methods to reflect changes in the base class.
//
//    Kathleen Bonnell, Tue Oct  2 17:34:53 PDT 2001
//    Added filters to extract the edges, for wireframe rendering. 
//    Added PreExecute and ModifyContract.  Added members min, max,
//    Ms, Bs.
//
//    Kathleen Bonnell, Fri Oct 10 10:48:24 PDT 2003
//    Added PostExecute. 
//
//    Mark C. Miller, Sun Feb 29 18:08:26 PST 2004
//    Added zValMin, zValMax data members
//
//    Mark C. Miller, Tue Mar  2 17:55:30 PST 2004
//    Removed zValMin, zValMax
//
//    Kathleen Bonnell, Mon May 24 14:13:55 PDT 2004 
//    Moved geofilter, appendFilter and edgesFilter to avtWireframeFilter. 
//    Removed PostExecute. 
//
//    Hank Childs, Fri Jul 30 12:09:16 PDT 2004
//    Added PostExecute.
//
//    Hank Childs, Sun Jan 30 13:48:54 PST 2005
//    Changed attributes and more to support stand-alone filter so Elevate
//    operator can also use.
//
//    Hank Childs, Fri Mar  4 08:47:07 PST 2005
//    Removed cd2pd.
//
//    Eric Brugger, Tue Jul 22 09:06:43 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class AVTFILTERS_API avtSurfaceFilter : public avtDataTreeIterator
{
  public:
                            avtSurfaceFilter(const AttributeGroup*);
    virtual                ~avtSurfaceFilter();

    static avtFilter       *Create(const AttributeGroup*);

    virtual const char     *GetType(void)  { return "avtSurfaceFilter"; };
    virtual const char     *GetDescription(void)
                            { return "Elevating 2D data by variable values"; };

    virtual void            ReleaseData(void);
    virtual bool            Equivalent(const AttributeGroup*);

  protected:
    SurfaceFilterAttributes    atts;
    double                     min;
    double                     max;
    double                     Ms;
    double                     Bs;
    double                     zValMin, zValMax;
    bool                       stillNeedExtents;
    bool                       haveIssuedWarning;
    bool                       doScale;

    double                  SkewTheValue(const double);
    void                    CalculateScaleValues(double *, double*);

    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual void            UpdateDataObjectInfo(void);
    virtual void            VerifyInput(void);
    virtual void            PreExecute(void);
    virtual void            PostExecute(void);
    virtual avtContract_p
                            ModifyContract(avtContract_p);
};


#endif


