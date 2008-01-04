/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                           vtkSurfaceFromVolume.h                          //
// ************************************************************************* //

#ifndef VTK_SURFACE_FROM_VOLUME_H
#define VTK_SURFACE_FROM_VOLUME_H

#include <vtkDataSetFromVolume.h>

#include <vector>


class vtkCellData;
class vtkPointData;
class vtkPolyData;


// ****************************************************************************
//  Class: vtkSurfaceFromVolume
//
//  Purpose:
//      This class is a data object.  It stores out surfaces, making it similar
//      to vtkPolyData.  However, it assumes that the surfaces it is creating
//      stem from a volume.  In addition, it is assumed that each triangle from
//      the new surface is contained in a cell from the volume.  Finally, it
//      is assumed that each endpoint of the triangle is located on an edge of
//      a cell in the original volume.
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Aug  7 15:55:08 PDT 2003
//    Refactored the surface-independent part into a new vtkDataSetFromVolume.
//    Left the part dealing with triangles and polydata here.
//
// ****************************************************************************

class vtkSurfaceFromVolume : public vtkDataSetFromVolume
{

class TriangleList
{
  public:
                   TriangleList();
    virtual       ~TriangleList();
 
    void           AddTriangle(int, int, int, int);
 
    int            GetTotalNumberOfTriangles(void) const;
    int            GetNumberOfLists(void) const;
    int            GetList(int, const int *&) const;
 
  protected:
    int          **list;
    int            currentList;
    int            currentTriangle;
    int            listSize;
    int            trianglesPerList;
};

  public:
                      vtkSurfaceFromVolume(int ptSizeGuess)
                           : vtkDataSetFromVolume(ptSizeGuess), tris()
                           { ; };
    virtual          ~vtkSurfaceFromVolume() { ; };

    void              ConstructPolyData(vtkPointData *, vtkCellData *,
                                        vtkPolyData *, float *);
    void              ConstructPolyData(vtkPointData *, vtkCellData *,
                                        vtkPolyData *, int *, float *, float *,
                                        float *);

    void              AddTriangle(int zone, int v0, int v1, int v2)
                            { tris.AddTriangle(zone, v0, v1, v2); };

  protected:
    TriangleList      tris;
};


#endif


