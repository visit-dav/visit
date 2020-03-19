// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtPointSelection.h                             //
// ************************************************************************* //

#ifndef AVT_POINT_SELECTION_H
#define AVT_POINT_SELECTION_H 

#include <pipeline_exports.h>

#include <ref_ptr.h>

#include <avtDataSelection.h>

#include <vector>


// ****************************************************************************
//  Class: avtPointSelection
//
//  Purpose: 
//      Specify data selection using a point location.
//
//  Programmer: Hank Childs
//  Creation:   March 22, 2009
//
//  Modifications:
//
//    Hank Childs, Tue Dec 20 14:43:08 PST 2011
//    Add method DescriptionString.
//
// ****************************************************************************

class PIPELINE_API avtPointSelection : public avtDataSelection 
{
  public:
                            avtPointSelection();
    virtual                ~avtPointSelection();

    static void             Destruct(void *);

    virtual const char *    GetType() const
                                { return "Point Selection"; }; 
    virtual std::string     DescriptionString(void);

    void                    SetPoint(const double *pt_) 
                                                  { pt[0] = pt_[0];
                                                    pt[1] = pt_[1];
                                                    pt[2] = pt_[2]; };
    const double           *GetPoint(void) const { return pt; };

    bool                    operator==(const avtPointSelection &) const;

  private:
    double                  pt[3];

    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                    avtPointSelection(const avtPointSelection &) {;};
    avtPointSelection &operator=(const avtPointSelection &) { return *this; };
};

typedef ref_ptr<avtPointSelection> avtPointSelection_p;


#endif


