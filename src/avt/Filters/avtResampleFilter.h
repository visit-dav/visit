// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtResampleFilter.h                            //
// ************************************************************************* //

#ifndef AVT_RESAMPLE_FILTER_H
#define AVT_RESAMPLE_FILTER_H

#include <filters_exports.h>

#include <avtDatasetToDatasetFilter.h>

#include <InternalResampleAttributes.h>


// ****************************************************************************
//  Class: avtResampleFilter
//
//  Purpose:
//      Resamples a dataset onto a rectilinear grid.
//
//  Programmer: Hank Childs
//  Creation:   March 26, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Apr  6 17:39:40 PDT 2001
//    Added ability to bypass filter with already valid rectilinear grids.
//
//    Mark C. Miller, Tue Sep 13 20:09:49 PDT 2005
//    Added selID to support data selections
//
//    Hank Childs, Sat Apr 29 15:53:13 PDT 2006
//    Add argument to GetDimensions.
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
//    Hank Childs, Fri Jun  1 16:17:51 PDT 2007
//    Added support for cell-centered output.
//
//    Hank Childs, Wed Dec 31 13:45:03 PST 2008
//    Change name of attributes from ResampleAtts to InternalResampleAtts.
//
//    Tom Fogal, Tue Jun 23 20:28:01 MDT 2009
//    Added GetBounds method.
//
//    Hank Childs, Tue Nov 30 21:54:43 PST 2010
//    Remove const qualification for GetBounds.
//
// ****************************************************************************

class AVTFILTERS_API avtResampleFilter : public avtDatasetToDatasetFilter
{
  public:
                          avtResampleFilter(const AttributeGroup*);
    virtual              ~avtResampleFilter();

    static avtFilter     *Create(const AttributeGroup*);

    virtual const char   *GetType(void)  { return "avtResampleFilter"; };
    virtual const char   *GetDescription(void) { return "Resampling"; };

    void                  MakeOutputCellCentered(bool doIt)
                                { cellCenteredOutput = doIt; };

    void                  SetRayCasting(bool _rayCasting)
                                { rayCasting = _rayCasting; }


  protected:
    InternalResampleAttributes atts;
    char                 *primaryVariable;
    int                   selID;
    bool                  cellCenteredOutput;

    bool                  rayCasting;

    virtual void          Execute(void);
    virtual void          UpdateDataObjectInfo(void);

    void                  GetDimensions(int &, int &, int &, const double *,
                                        bool);
    bool                  GetBounds(double[6]);
    void                  ResampleInput(void);

    virtual int           AdditionalPipelineFilters(void) { return 2; };

    virtual avtContract_p ModifyContract(avtContract_p);
    
    virtual bool          FilterUnderstandsTransformedRectMesh();
};


#endif


