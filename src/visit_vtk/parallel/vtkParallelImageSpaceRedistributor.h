/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                    vtkParallelImageSpaceRedistributor.h                   //
// ************************************************************************* //

#ifndef __vtkParallelImageSpaceRedistributor_h
#define __vtkParallelImageSpaceRedistributor_h
#include <parallel_visit_vtk_exports.h>
#include <vtkRenderer.h>
#include <vector>
#include <vtkPolyDataToPolyDataFilter.h>
#include <vtkCell.h>
#ifdef PARALLEL
#include <mpi.h>
#endif

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
//    Brad Whitlock, Mon Nov 1 15:10:49 PST 2004
//    Added a method to set rank and size to eliminate a circular code
//    dependency. Changed the API.
//
//    Brad Whitlock, Fri Jan 23 15:07:24 PST 2009
//    I added a SetCommunicator method so this library is not dependent on
//    AVT.
//
// ****************************************************************************

class PARALLEL_VISIT_VTK_API vtkParallelImageSpaceRedistributor :
                                            public vtkPolyDataToPolyDataFilter
{
  public:
    static vtkParallelImageSpaceRedistributor      *New();

    vtkPolyData      *GetOutput();
    void             SetRenderer(vtkRenderer *renderer) {ren = renderer;};
    void             SetRankAndSize(int r, int s);
#ifdef PARALLEL
    void             SetCommunicator(const MPI_Comm &c);
#endif
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
#ifdef PARALLEL
    MPI_Comm         comm;
#endif

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
