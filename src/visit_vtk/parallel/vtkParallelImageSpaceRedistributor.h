// ************************************************************************* //
//                    vtkParallelImageSpaceRedistributor.h                   //
// ************************************************************************* //

#ifndef __vtkParallelImageSpaceRedistributor_h
#define __vtkParallelImageSpaceRedistributor_h
#include <visit_vtk_exports.h>
#include <vtkRenderer.h>
#include <vector>
#include <vtkPolyDataToPolyDataFilter.h>
#include <vtkCell.h>

// ****************************************************************************
//  Class: vtkParallelImageSpaceRedistributor
//
//  Purpose:
//      This will redistribute data into spatially disparate regions
//      by processor.  The idea is to sort the data front to back for
//      transparency calculations in image space, not in the data's
//      coordinate space, in order to avoid transparency errors.
//
//  Programmer: Chris Wojtan
//  Creation:   July 6, 2004
//
//  Modifications:
//    Jeremy Meredith, Thu Oct 21 18:20:33 PDT 2004
//    Renamed, reworked, refactored.
//
//    Jeremy Meredith, Tue Oct 26 22:13:51 PDT 2004
//    More refactoring.  Cloned WhichProcessorsForCell so it could count cells
//    in the first pass without having to create the destinations vector.
//
// ****************************************************************************

class VISIT_VTK_API vtkParallelImageSpaceRedistributor :
                                            public vtkPolyDataToPolyDataFilter
{
  public:
    static vtkParallelImageSpaceRedistributor      *New();

    vtkPolyData      *GetOutput();
    void             SetRenderer(vtkRenderer *renderer) {ren = renderer;};

  protected:
                     vtkParallelImageSpaceRedistributor();
    virtual         ~vtkParallelImageSpaceRedistributor();

    virtual void     Execute(void);

  private:
                     vtkParallelImageSpaceRedistributor(
                                   const vtkParallelImageSpaceRedistributor &);
    void             operator=(const vtkParallelImageSpaceRedistributor &);
    
    vtkRenderer     *ren;

    int             *x1,*y1,*x2,*y2;
    int              rank, size;
    int              width, height;

    unsigned char   *GetDataString(int &length, vtkPolyData *asVTK);
    vtkPolyData     *GetDataVTK(unsigned char *asChar,
                                unsigned int asCharLength);
    int              WhichProcessorsForCell(float *pts, vtkIdType npts,
                                     vtkIdType *cellPts, std::vector<int>&);
    void             IncrementOutgoingCellCounts(float *pts, vtkIdType npts,
                                     vtkIdType *cellPts, std::vector<int>&);
    float           *CreateTransformedPoints();
    vtkMatrix4x4    *CreateWorldToDisplayMatrix();
};


#endif
