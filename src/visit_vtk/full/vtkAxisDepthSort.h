// ************************************************************************* //
//                             vtkAxisDepthSort.h                            //
// ************************************************************************* //

#ifndef __vtkAxisDepthSort_h
#define __vtkAxisDepthSort_h
#include <visit_vtk_exports.h>
#include <vtkPolyDataToPolyDataFilter.h>


// ****************************************************************************
//  Class: vtkAxisDepthSort
//
//  Purpose:
//      This will sort poly data along all six axes (+x, -x, +y, -y, +z, -z).
//
//  Programmer: Hank Childs
//  Creation:   July 13, 2002
//
//  Modifications:
//    Brad Whitlock, Mon Jul 15 15:43:22 PST 2002
//    Added API.
//
// ****************************************************************************

class VISIT_VTK_API vtkAxisDepthSort : public vtkPolyDataToPolyDataFilter
{
  public:
    static vtkAxisDepthSort      *New();

    vtkPolyData                  *GetPlusXOutput();
    vtkPolyData                  *GetMinusXOutput();
    vtkPolyData                  *GetPlusYOutput();
    vtkPolyData                  *GetMinusYOutput();
    vtkPolyData                  *GetPlusZOutput();
    vtkPolyData                  *GetMinusZOutput();

  protected:
                                  vtkAxisDepthSort();
    virtual                      ~vtkAxisDepthSort() {;};

    virtual void                  Execute(void);

  private:
                                  vtkAxisDepthSort(const vtkAxisDepthSort &);
                                  void operator=(const vtkAxisDepthSort &);
};


#endif


