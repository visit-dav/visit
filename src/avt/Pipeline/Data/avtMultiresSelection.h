// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtMultiresSelection.h                            //
// ************************************************************************* //

#ifndef AVT_MULTIRES_SELECTION_H
#define AVT_MULTIRES_SELECTION_H
#include <pipeline_exports.h>

#include <ref_ptr.h>

#include <avtDataSelection.h>

// ****************************************************************************
//  Class: avtMultiresSelection
//
//  Purpose:
//    Specify a data selection with a multi resolution representation
//    that may be specified by one of two methods. The first is by specifying
//    the composite projection transform matrix, the viewport, the window
//    size and the desired cell size. The second is by specifying the desired
//    extents, the view area and the desired cell size. The data selection
//    also contains the actual data extents and cell area provided.
//
//  Programmer: Eric Brugger
//  Creation:   December 20, 2013
//
//  Modifications:
//
// ****************************************************************************

class PIPELINE_API avtMultiresSelection : public avtDataSelection
{
  public:
                            avtMultiresSelection() {} ;
    virtual                ~avtMultiresSelection() {} ;

    virtual const char *    GetType() const
                                { return "Multi Resolution Data Selection"; }
    virtual std::string     DescriptionString(void);

    void                    SetCompositeProjectionTransformMatrix(
                                const double matrix[16]);
    void                    GetCompositeProjectionTransformMatrix(
                                double matrix[16]) const;
    void                    SetViewport(const double viewport[6]);
    void                    GetViewport(double viewport[6]) const;
    void                    SetSize(const int size[2])
                                { windowSize[0] = size[0];
                                  windowSize[1] = size[1]; }
    void                    GetSize(int size[2]) const
                                { size[0] = windowSize[0];
                                  size[1] = windowSize[1]; }
    void                    SetViewArea(double area)
                                { viewArea = area; }
    double                  GetViewArea() const
                                { return viewArea; }
    void                    SetDesiredExtents(const double extents[6]);
    void                    GetDesiredExtents(double extents[6]) const;
    void                    SetActualExtents(const double extents[6]);
    void                    GetActualExtents(double extents[6]) const;
    void                    SetDesiredCellArea(double area)
                                { desiredCellArea = area; }
    double                  GetDesiredCellArea() const
                                { return desiredCellArea; }
    void                    SetActualCellArea(double area)
                                { actualCellArea = area; }
    double                  GetActualCellArea() const
                                { return actualCellArea; }

  private:
    double transformMatrix[16];
    double viewport[6];
    int    windowSize[2];
    double viewArea;
    double desiredExtents[6];
    double actualExtents[6];
    double desiredCellArea;
    double actualCellArea;
};

typedef ref_ptr<avtMultiresSelection> avtMultiresSelection_p;

#endif
