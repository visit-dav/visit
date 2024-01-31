// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                    vtkParallelImageSpaceRedistributor.h                   //
// ************************************************************************* //

#ifndef __vtkParallelImageSpaceRedistributor_h
#define __vtkParallelImageSpaceRedistributor_h

#include <plotter_exports.h>

#include <visit-config.h> // For LIB_VERSION_LE

#include <vector>

#include <vtkCell.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkRenderer.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

// ****************************************************************************
//  Class: vtkParallelImageSpaceRedistributor
//
//  Purpose:
//    This will redistribute data into spatially disparate regions
//    by processor. The idea is to sort the data front to back for
//    transparency calculations in image space, not in the data's
//    coordinate space, in order to avoid transparency errors.
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
//    Brad Whitlock, Mon Nov 1 15:10:49 PST 2004
//    Added a method to set rank and size to eliminate a circular code
//    dependency. Changed the API.
//
//    Brad Whitlock, Fri Jan 23 15:07:24 PST 2009
//    I added a SetCommunicator method so this library is not dependent on
//    AVT.
//
//    Hank Childs, Sun May 23 16:12:11 CDT 2010
//    Add argument to IncrementOutgoingCellTypes.
//
//    Eric Brugger, Wed Jan  9 10:32:26 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
//    Kathleen Biagas, Thu Aug 11, 2022
//    New signatures for VTK9 support, requiring const for cellPts.
//
// ****************************************************************************

class PLOTTER_API vtkParallelImageSpaceRedistributor :
    public vtkPolyDataAlgorithm
{
  public:
    static vtkParallelImageSpaceRedistributor      *New();

    vtkPolyData     *GetOutput();
    void             SetRenderer(vtkRenderer *renderer) {ren = renderer;};
    void             SetRankAndSize(int r, int s);
#ifdef PARALLEL
    void             SetCommunicator(const MPI_Comm &c);
#endif
  protected:
                     vtkParallelImageSpaceRedistributor();
    virtual         ~vtkParallelImageSpaceRedistributor();


    int              RequestData(vtkInformation *, vtkInformationVector **,
                                 vtkInformationVector *) override;

  private:
                     vtkParallelImageSpaceRedistributor(
                         const vtkParallelImageSpaceRedistributor &);
    void             operator=(const vtkParallelImageSpaceRedistributor &);

    vtkRenderer     *ren;

    int             *x1,*y1,*x2,*y2;
    int              rank, size;
    int              width, height;
#ifdef PARALLEL
    MPI_Comm         comm;
#endif

    unsigned char   *GetDataString(int &length, vtkPolyData *asVTK);
    vtkPolyData     *GetDataVTK(unsigned char *asChar,
                                unsigned int asCharLength);
#if LIB_VERSION_LE(VTK,8,1,0)
    int              WhichProcessorsForCell(double *pts, vtkIdType npts,
                                     vtkIdType *cellPts, std::vector<int>&);
    void             IncrementOutgoingCellCounts(double *pts, vtkIdType npts,
                                     vtkIdType *cellPts, std::vector<int>&,
                                     std::vector<int>&);
#else
    int              WhichProcessorsForCell(double *pts, vtkIdType npts,
                                     const vtkIdType *cellPts, std::vector<int>&);
    void             IncrementOutgoingCellCounts(double *pts, vtkIdType npts,
                                     const vtkIdType *cellPts, std::vector<int>&,
                                     std::vector<int>&);
#endif
    vtkMatrix4x4    *CreateWorldToDisplayMatrix();
};


#endif
