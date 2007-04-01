// ************************************************************************* //
//                        avtLogicalSelection.h                              //
// ************************************************************************* //

#ifndef AVT_LOGICAL_SELECTION_H
#define AVT_LOGICAL_SELECTION_H 
#include <pipeline_exports.h>

#include <ref_ptr.h>

#include <avtDataSelection.h>

// ****************************************************************************
//  Class: avtLogicalSelection
//
//  Purpose: Specify data selection by a zone-centric logical indexing scheme.
//  That is, all logical coordinates are specified in terms of zones, not
//  nodes. The starts and stops are inclusive. However, if strides are such
//  that an integral number of strides from starts DOES NOT equal the
//  associated value in stops, then stops is non-inclusive.
//
//  The default is a 3D box of stride 1 spanning all possible indices.
//
//  To specify an orthogonal slice on the Y-axis, for example, you would
//  specify starts[1] = stops[1] = Y-index and strides[1] = 0 with other
//  values as desired.
//
//  Programmer: Mark C. Miller 
//  Creation:   September 22, 2004 
//
// ****************************************************************************

class PIPELINE_API avtLogicalSelection : public avtDataSelection 
{
  public:
                            avtLogicalSelection();
    virtual                ~avtLogicalSelection() {} ;

    virtual const char *    GetType() const
                                { return "Logical Data Selection"; }; 

    void                    SetNDims(int _ndims)
                                { ndims = _ndims; };
    void                    SetStarts(const int *_starts);
    void                    SetStops(const int *_stops);
    void                    SetStrides(const int *_strides);

    void                    GetNDims(int &_ndims) const
                                { _ndims = ndims; } ; 
    void                    GetStarts(int *_starts) const;
    void                    GetStops(int *_stops) const;
    void                    GetStrides(int *_strides) const;

    bool                    operator==(const avtLogicalSelection &) const;

    void                    Compose(const avtLogicalSelection& sel);
                                          
    bool                    FactorBestPowerOf2(avtLogicalSelection& pow2Sel,
                                               avtLogicalSelection& otherSel) const;

  private:

     int ndims;
     int starts[3];
     int stops[3];
     int strides[3];

};

typedef ref_ptr<avtLogicalSelection> avtLogicalSelection_p;


#endif


