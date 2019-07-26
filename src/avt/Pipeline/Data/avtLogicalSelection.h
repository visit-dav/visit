// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//  Purpose: Specify data selection by a node-centric logical indexing
//  scheme.  That is, all logical coordinates are specified in terms
//  of nodes, not zones. The starts and stops are inclusive. However,
//  if strides are such that an integral number of strides from starts
//  DOES NOT equal the associated value in stops, then stops is
//  non-inclusive.
//
//  Nodal based matches the IndexSelect operator functionality which
//  is passed in VisIt. Internally a class can use it how it
//  wants. For instance, ViSUS uses the bounds for zones instead of
//  nodes.
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
//  Modifications:
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
//    Hank Childs, Tue Dec 20 14:43:08 PST 2011
//    Add method DescriptionString.
//
// ****************************************************************************

class PIPELINE_API avtLogicalSelection : public avtDataSelection 
{
  public:
                            avtLogicalSelection();
    virtual                ~avtLogicalSelection() {} ;

    virtual const char *    GetType() const
                                { return "Logical Data Selection"; }; 
    virtual std::string     DescriptionString(void);

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

    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                         avtLogicalSelection(const avtLogicalSelection &) {;};
    avtLogicalSelection &operator=(const avtLogicalSelection &) 
                                                            { return *this; };
};

typedef ref_ptr<avtLogicalSelection> avtLogicalSelection_p;


#endif


