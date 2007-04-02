#ifndef VTK_CRACK_WIDTH_FILTER_H
#define VTK_CRACK_WIDTH_FILTER_H

#include <vtkDataSetToDataSetFilter.h>

class vtkCell;
class vtkVertex;
class vtkPolyVertex;
class vtkLine;
class vtkPolyLine;
class vtkTriangle;
class vtkTriangleStrip;
class vtkPolygon;
class vtkPixel;
class vtkQuad;
class vtkTetra;
class vtkVoxel;
class vtkHexahedron;
class vtkWedge;
class vtkPyramid;


// ****************************************************************************
//  Class:  vtkCrackWidthFilter
//
//  Purpose:
//    Calculates the width of cracks along given crack directions for
//    each cell, and stores each width in a cell array.  Also calculates
//    the cell centers.  
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    August 22, 2005 
//
//  Modifications:
//
// ****************************************************************************

class vtkCrackWidthFilter : public vtkDataSetToDataSetFilter
{
  public:
    vtkTypeRevisionMacro(vtkCrackWidthFilter,vtkDataSetToDataSetFilter);

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

    void Execute();

  private:
    vtkTriangle *triangle;
    vtkQuad *quad;
    double MaxCrack1Width;
    double MaxCrack2Width;
    double MaxCrack3Width;

    char *Crack1Var;
    char *Crack2Var;
    char *Crack3Var;
    char *StrainVar;

    double  CrackWidthForCell(vtkCell *cell, const double *center,
           const double cellLength, const double delta, const double *dir);

    int CellIntersectWithLine(vtkCell *, double [3], double [3], 
                                double&, double [3]);

    int VertexIntersectWithLine(vtkVertex *, double [3], double [3], 
                                double&, double [3]);

    int PolyVertexIntersectWithLine(vtkPolyVertex *, double [3], double [3], 
                                double&, double [3]);

    int LineIntersectWithLine(vtkLine *, double [3], double [3], 
                                double&, double [3]);

    int PolyLineIntersectWithLine(vtkPolyLine *, double [3], double [3], 
                                double&, double [3]);

    int TriangleIntersectWithLine(vtkTriangle *, double [3], double [3], 
                                double&, double [3]);

    int TriStripIntersectWithLine(vtkTriangleStrip *, double [3], double [3], 
                                double&, double [3]);

    int PolygonIntersectWithLine(vtkPolygon *, double [3], double [3], 
                                double&, double [3]);

    int PixelIntersectWithLine(vtkPixel *, double [3], double [3], 
                                double&, double [3]);

    int QuadIntersectWithLine(vtkQuad *, double [3], double [3], 
                                double&, double [3]);

    int TetraIntersectWithLine(vtkTetra *, double [3], double [3], 
                                double&, double [3]);

    int VoxelIntersectWithLine(vtkVoxel *, double [3], double [3], 
                                double&, double [3]);

    int HexIntersectWithLine(vtkHexahedron *, double [3], double [3], 
                                double&, double [3]);

    int WedgeIntersectWithLine(vtkWedge *, double [3], double [3], 
                                double&, double [3]);

    int PyramidIntersectWithLine(vtkPyramid *, double [3], double [3], 
                                double&, double [3]);

    int LineLineIsect(const double *, const double *, const double *, 
                      const double *, double *);

    int EdgeLineIsect(vtkCell *cell, const double *, const double *, double *);


    vtkCrackWidthFilter(const vtkCrackWidthFilter&);  // Not implemented.
    void operator=(const vtkCrackWidthFilter&);  // Not implemented.

};



#endif


