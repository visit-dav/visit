// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VTK_CRACK_WIDTH_FILTER_H
#define VTK_CRACK_WIDTH_FILTER_H

#include <vtkDataSetAlgorithm.h>

class vtkCell;
class vtkMassProperties;
class vtkSlicer;

// ****************************************************************************
//  Class: vtkCrackWidthFilter
//
//  Purpose:
//    Calculates the width of cracks along given crack directions for
//    each cell, and stores each width in a cell array.  Also calculates
//    the cell centers.  
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 22, 2005 
//
//  Modifications:
//    Kathleen Bonnell, Wed Sep 13 07:42:59 PDT 2006
//    Remove individual cell intersection methods, use vtkCellIntersections 
//    class instead.
//
//    Kathleen Bonnell, Fri Oct 13 11:05:01 PDT 2006 
//    Removed use of vtkCellIntersections, added vtkMassProperties, vtkSlicer.
//    Changed args for CrackWidthForCell.
//
//    Kathleen Biagas, Tue Aug 14 15:55:27 MST 2012
//    Removed unused vtkQuad and vtkTriangle.
//
//    Eric Brugger, Wed Jan  9 16:20:40 PST 2013
//    Modified to inherit from vtkDataSetAlgorithm.
//
//    Eric Brugger, Thu Sep  6 16:40:01 PDT 2018
//    Changed CrackWidthForCell to LengthForCell.
//
// ****************************************************************************

class vtkCrackWidthFilter : public vtkDataSetAlgorithm
{
  public:
    vtkTypeMacro(vtkCrackWidthFilter,vtkDataSetAlgorithm);

    static vtkCrackWidthFilter *New();

    vtkGetMacro(MaxCrack1Width, double);
    vtkGetMacro(MaxCrack2Width, double);
    vtkGetMacro(MaxCrack3Width, double);

    double GetMaxCrackWidth(int w);
    
    vtkSetStringMacro(Crack1Var);
    vtkSetStringMacro(Crack2Var);
    vtkSetStringMacro(Crack3Var);
    vtkSetStringMacro(StrainVar);

  protected:
    vtkCrackWidthFilter();
    ~vtkCrackWidthFilter();

    int RequestData(vtkInformation *,
                    vtkInformationVector **,
                    vtkInformationVector *) override;

  private:
    double MaxCrack1Width;
    double MaxCrack2Width;
    double MaxCrack3Width;

    char *Crack1Var;
    char *Crack2Var;
    char *Crack3Var;
    char *StrainVar;

    vtkSlicer *Slicer;
    vtkMassProperties *MassProp;

    double LengthForCell(vtkCell *cell, vtkIdType cellId,
                         const double *center, const double *dir,
                         const double zvol, const double L1L2);

    vtkCrackWidthFilter(const vtkCrackWidthFilter&);  // Not implemented.
    void operator=(const vtkCrackWidthFilter&);  // Not implemented.
};


#endif
