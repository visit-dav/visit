// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtLabelActor.h                              //
// ************************************************************************* //


#ifndef AVT_LABEL_ACTOR_H
#define AVT_LABEL_ACTOR_H
#include <plotter_exports.h>
#include <visitstream.h>
#include <ref_ptr.h>

class vtkFollower;
class vtkPolyDataMapper;
class vtkRenderer;


// ****************************************************************************
//  Class:  avtLabelActor
//
//  Purpose:  Responsible for creating label actors used in decorations. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 12, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 19 08:39:04 PDT 2002
//    Add ComputeScaleFactor.
//
//    Eric Brugger, Tue Dec  9 16:16:49 PST 2008
//    Added the ability to display a marker instead of a text string.
//
//    Eric Brugger, Mon Mar  9 17:26:13 PDT 2009
//    Added an overloaded version of SetForegroundColor that allows setting
//    rgba instead of just rgb.
//
//    Eric Brugger, Thu Feb 19 13:24:04 PST 2013
//    Added the ability to set a scale factor and the line width. 
//
// ****************************************************************************

class PLOTTER_API avtLabelActor
{
  public:
                       avtLabelActor();
    virtual           ~avtLabelActor();

    void               Add(vtkRenderer *ren);
    void               Remove();
    void               Hide();
    void               UnHide();

    void               SetAttachmentPoint(const double newPos[3]);
    const double *     GetAttachmentPoint() { return attach; };
    void               SetScale(double);
    void               SetScaleFactor(double);
    void               SetLineWidth(int);
    void               SetDesignator(const char *l);
    void               SetMarker(const int index);
    void               SetForegroundColor(double fgr, double fgg, double fgb);
    void               SetForegroundColor(double fgr, double fgg, double fgb,
                           double fga);
    void               SetForegroundColor(double fg[3]);
    void               Shift(const double vec[3]);
    double             ComputeScaleFactor();

  protected:
    double             attach[3];
    double             scaleFactor;
    vtkFollower       *labelActor;

    vtkRenderer       *renderer; 

  private:
};

typedef ref_ptr<avtLabelActor> avtLabelActor_p;

#endif
