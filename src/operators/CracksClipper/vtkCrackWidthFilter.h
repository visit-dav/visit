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

    vtkGetMacro(MaxCrack1Width, float);
    vtkGetMacro(MaxCrack2Width, float);
    vtkGetMacro(MaxCrack3Width, float);

    float GetMaxCrackWidth(int w);
    
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
    float MaxCrack1Width;
    float MaxCrack2Width;
    float MaxCrack3Width;

    char *Crack1Var;
    char *Crack2Var;
    char *Crack3Var;
    char *StrainVar;

    float  CrackWidthForCell(vtkCell *cell, const float *center,
           const float cellLength, const float delta, const float *dir);

    int CellIntersectWithLine(vtkCell *, float [3], float [3], 
                                float&, float [3]);

    int VertexIntersectWithLine(vtkVertex *, float [3], float [3], 
                                float&, float [3]);

    int PolyVertexIntersectWithLine(vtkPolyVertex *, float [3], float [3], 
                                float&, float [3]);

    int LineIntersectWithLine(vtkLine *, float [3], float [3], 
                                float&, float [3]);

    int PolyLineIntersectWithLine(vtkPolyLine *, float [3], float [3], 
                                float&, float [3]);

    int TriangleIntersectWithLine(vtkTriangle *, float [3], float [3], 
                                float&, float [3]);

    int TriStripIntersectWithLine(vtkTriangleStrip *, float [3], float [3], 
                                float&, float [3]);

    int PolygonIntersectWithLine(vtkPolygon *, float [3], float [3], 
                                float&, float [3]);

    int PixelIntersectWithLine(vtkPixel *, float [3], float [3], 
                                float&, float [3]);

    int QuadIntersectWithLine(vtkQuad *, float [3], float [3], 
                                float&, float [3]);

    int TetraIntersectWithLine(vtkTetra *, float [3], float [3], 
                                float&, float [3]);

    int VoxelIntersectWithLine(vtkVoxel *, float [3], float [3], 
                                float&, float [3]);

    int HexIntersectWithLine(vtkHexahedron *, float [3], float [3], 
                                float&, float [3]);

    int WedgeIntersectWithLine(vtkWedge *, float [3], float [3], 
                                float&, float [3]);

    int PyramidIntersectWithLine(vtkPyramid *, float [3], float [3], 
                                float&, float [3]);

    int LineLineIsect(const float *, const float *, const float *, 
                      const float *, float *);

    int EdgeLineIsect(vtkCell *cell, const float *, const float *, float *);


    vtkCrackWidthFilter(const vtkCrackWidthFilter&);  // Not implemented.
    void operator=(const vtkCrackWidthFilter&);  // Not implemented.

};



#endif


