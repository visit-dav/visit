/*****************************************************************************
*
* Copyright (c) 2000 - 2018, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#ifndef VTK_UNSTRUCTURED_GRID_FACELIST_FILTER_H
#define VTK_UNSTRUCTURED_GRID_FACELIST_FILTER_H
#include <visit_vtk_light_exports.h>

#include <vtkPolyDataAlgorithm.h>

//
// MCMiller-11Jan19: My understanding of what this class does, from a cursory
// review of the code, is that it iterates over all cells in the mesh (grid)
// adding the faces from each cell to various hashes. However, the face-adding
// logic is designed to add a face only if it has never been seen before
// and otherwise remove the face. For topological reasons, faces are shared by
// at most two cells. So a face will only ever be encountered once or twice.
// Once all faces from all cells have been iterated what remains are those
// faces that were only ever encountered once. Those are the *external* faces
// of the input mesh (grid). There is no logic here to address whether the
// cells involved (or their corresonding nodes) are associated with any
// ghosting.
//
// Note 1: a similar operation is performed in the Silo plugin to handle
// painting of enumerated scalar nodal values for annotation int zonelists
// except that multi-layer STL maps are used for the hashing.
//
// Note 2: This kind of algorithm is exactly the kind of thing something
// like Big Data Apache Spark filters handle very well.
//

class VISIT_VTK_LIGHT_API vtkUnstructuredGridFacelistFilter :
    public vtkPolyDataAlgorithm
{
  public:
    vtkTypeMacro(vtkUnstructuredGridFacelistFilter, vtkPolyDataAlgorithm);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    static vtkUnstructuredGridFacelistFilter *New();

  protected:
    vtkUnstructuredGridFacelistFilter() {;};
    ~vtkUnstructuredGridFacelistFilter() {;};
 
    virtual int RequestData(vtkInformation *,
                            vtkInformationVector **,
                            vtkInformationVector *) override;
    virtual int FillInputPortInformation(int port, vtkInformation *info) override;
};

#endif
