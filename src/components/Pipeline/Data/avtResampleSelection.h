// ************************************************************************* //
//                        avtResampleSelection.h                             //
// ************************************************************************* //

#ifndef AVT_RESAMPLE_SELECTION_H
#define AVT_RESAMPLE_SELECTION_H 
#include <pipeline_exports.h>

#include <ref_ptr.h>

#include <avtDataSelection.h>

// ****************************************************************************
//  Class: avtResampleSelection
//
//  Purpose: Specify data selection by a uniform resampling. The context in
//  which resampling behaves as a data "selection" operation is one in which
//  the input is a non-discrete (e.g. continuous) representation of the data.
//  In that case, resampling has the effect of selecting a finite set of 
//  points (the samples) fron an infinity of points (the continuum) in which
//  the data actually originates.
//
//  The default is a uniform sampling 100 samples in each of X, Y and Z
//  between -10.0 and 10.0 on each axis.
//
//  To specify samples on an orthogonal slice on the Y-axis, for example,
//  you would specify starts[1] = stops[1] = Y-index and counts[1] = 0
//  with other values as desired.
//
//  Programmer: Mark C. Miller 
//  Creation:   August 16, 2005 
//
// ****************************************************************************

class PIPELINE_API avtResampleSelection : public avtDataSelection 
{
  public:
                            avtResampleSelection();
    virtual                ~avtResampleSelection() {} ;

    virtual const char *    GetType() const
                                { return "Resample Data Selection"; }; 

    void                    SetNDims(int _ndims)
                                { ndims = _ndims; };
    void                    SetStarts(const double *_starts);
    void                    SetStops(const double *_stops);
    void                    SetCounts(const int *_counts);

    void                    GetNDims(int &_ndims) const
                                { _ndims = ndims; } ; 
    void                    GetStarts(double *_starts) const;
    void                    GetStops(double *_stops) const;
    void                    GetCounts(int *_counts) const;

    bool                    operator==(const avtResampleSelection &) const;

  private:

     int ndims;
     double starts[3];
     double stops[3];
     int counts[3];

};

typedef ref_ptr<avtResampleSelection> avtResampleSelection_p;


#endif
