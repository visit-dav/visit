// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtSliceFilter.h                               //
// ************************************************************************* //

#ifndef AVT_SLICE_FILTER_H
#define AVT_SLICE_FILTER_H

#include <avtPluginDataTreeIterator.h>

#include <SliceAttributes.h>

#include <vtkType.h>

class avtPointAttribute;

class vtkDataSet;
class vtkRectilinearGrid;
class vtkTransformFilter;
class vtkMatrix4x4;
class vtkSlicer;


// ****************************************************************************
//  Class: avtSliceFilter
//
//  Purpose:
//      A filter that takes a slice of domains of an avtDataSet.
//
//  Programmer: Hank Childs
//  Creation:   July 25, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Aug  8 16:52:28 PDT 2000
//    Made constructor take a plane instead of a cut-filter, since we want
//    this filter to be driven by a cut-plane and the cut-filter allowed
//    for other implicit functions besides planes.
//
//    Jeremy Meredith, Tue Sep 19 22:30:05 PDT 2000
//    Made constructor take raw origin and normal, added origin and
//    normal data members, and added Equivalent method.
//
//    Jeremy Meredith, Thu Sep 28 12:50:55 PDT 2000
//    Removed CreateOutputDatasets.  Changed interface to ExecuteDomain.
//
//    Jeremy Meredith, Thu Mar  1 13:29:27 PST 2001
//    Made attributes be stored as an SliceAttributes class.
//
//    Jeremy Meredith, Sun Mar  4 16:59:57 PST 2001
//    Added a static Create method.
//
//    Kathleen Bonnell, Tue Apr 10 10:49:10 PDT 2001 
//    Changed ExecuteDomain to ExecuteData. 
//
//    Hank Childs, Wed Jun  6 08:44:38 PDT 2001
//    Renamed some methods to fit changes in base class.
//
//    Hank Childs, Fri Mar 15 19:33:24 PST 2002
//    Made use of dynamic resolves points.
//
//    Hank Childs, Tue Aug  6 10:30:25 PDT 2002
//    Calculate the cells that intersect a rectilinear slice before slicing.
//
//    Kathleen Bonnell, Thu Apr 10 11:25:01 PDT 2003   
//    Added PostExecute method. Store inverse transform matrix for possible
//    use later in the pipeline (project-2d only). 
//
//    Jeremy Meredith, Mon May  5 14:31:45 PDT 2003
//    Removed "point" for now.  The slice window has changed, and dynamically
//    resolved attributes will work differently soon.
//
//    Kathleen Bonnell, Wed Jun  2 09:11:01 PDT 2004
//    Store transform matrix for possible use later in the pipeline. 
//
//    Hank Childs, Thu Jan 20 10:36:10 PST 2005
//    Added extra argument to ProjectExtents.
//
//    Hank Childs, Fri Aug 19 08:57:27 PDT 2005
//    Use vtkTransformFilter instead of vtkTransformPolyDataFilter, since 
//    vtkTransformPolyDataFilter does not pass on names of vectors, which
//    can screw us up down stream. ['6471]
//
//    Hank Childs, Fri Dec 29 15:02:34 PST 2006
//    Add some new methods.  The "ExecuteData" method got too crowded and
//    some aspects are being put in their own method.
//
//    Dave Pugmire, Mon Oct 22 10:25:42 EDT 2007
//    Added cachedNormal data member.
//
//    Brad Whitlock, Wed Jan 23 15:55:37 PST 2008
//    Added doTransformVectors.
//
//    Brad Whitlock, Tue Mar 13 14:42:27 PDT 2012
//    Replace float with double.
//
//    Eric Brugger, Thu Aug 14 16:33:47 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
//    Eric Brugger, Mon Sep 17 14:06:29 PDT 2018
//    Modified the class to work with VTKM.
//
// ****************************************************************************

class avtSliceFilter : public avtPluginDataTreeIterator
{
  public:
                            avtSliceFilter();
    virtual                ~avtSliceFilter();

    static avtFilter       *Create();

    virtual const char     *GetType(void) { return "avtSliceFilter"; };
    virtual const char     *GetDescription(void) { return "Slicing"; };
    virtual void            ReleaseData(void);

    virtual void            SetAtts(const AttributeGroup*);
    virtual bool            Equivalent(const AttributeGroup*);
    void                    ProjectExtents(const double *, double *);

  protected:
    SliceAttributes               atts;
    double                        D;
    double                        cachedOrigin[3];
    double                        cachedNormal[3];
    bool                          doTransformVectors;

    vtkSlicer                    *slicer;
    vtkTransformFilter           *transform;
    vtkIdType                    *celllist;
    vtkMatrix4x4                 *invTrans;
    vtkMatrix4x4                 *origTrans;

    virtual avtContract_p   ModifyContract(avtContract_p);
    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    avtDataRepresentation  *ExecuteData_VTK(avtDataRepresentation *);
    avtDataRepresentation  *ExecuteData_VTKM(avtDataRepresentation *);
    virtual void            PreExecute(void);
    virtual void            PostExecute(void);

    virtual void            UpdateDataObjectInfo(void);

    void                    CalculateRectilinearCells(vtkRectilinearGrid *);
    void                    SetPlaneOrientation(double *);

    void                    GetOrigin(double &, double &, double &);
    void                    GetNormal(double &, double &, double &);
    void                    SetUpProjection(void);

    bool                    CanIntersectPlane(vtkDataSet *);
    bool                    OutputCanBeRectilinear(vtkRectilinearGrid *);
    vtkRectilinearGrid     *RectilinearToRectilinearSlice(vtkRectilinearGrid*);
};


#endif


