// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtPlaneSelection.h                             //
// ************************************************************************* //

#ifndef AVT_PLANE_SELECTION_H
#define AVT_PLANE_SELECTION_H 

#include <pipeline_exports.h>

#include <ref_ptr.h>

#include <avtDataSelection.h>


// ****************************************************************************
//  Class: avtPlaneSelection
//
//  Purpose: Specify a data selection that is a plane.
//
//  Programmer: Hank Childs
//  Creation:   January 5, 2009
//
//  Modifications:
//
//    Hank Childs, Tue Dec 20 14:43:08 PST 2011
//    Add method DescriptionString.
//
// ****************************************************************************

class PIPELINE_API avtPlaneSelection : public avtDataSelection 
{
  public:
                            avtPlaneSelection();
    virtual                ~avtPlaneSelection() {} ;

    virtual const char *    GetType() const
                                { return "Plane Selection"; }; 
    virtual std::string     DescriptionString(void);

    void                    SetNormal(const double *_normal);
    void                    SetOrigin(const double *_origin);

    void                    GetNormal(double *_normal) const;
    void                    GetOrigin(double *_origin) const;

  private:
    double normal[3];
    double origin[3];
};

typedef ref_ptr<avtPlaneSelection> avtPlaneSelectiont_p;


#endif


