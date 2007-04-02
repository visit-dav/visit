/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                             avtVerdictFilter.C                            //
// ************************************************************************* //

#include <avtVerdictFilter.h>

#include <verdict.h>

#include <vtkCell.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPoints.h>

#include <avtDataTree.h>
#include <avtDataRepresentation.h>

#include <ExpressionException.h>
#include <ImproperUseException.h>
#include <DebugStream.h>

struct SizeData
{
    double hexSize;
    double tetSize;
    double triSize;
    double quadSize;

    int hexCount;
    int tetCount;
    int triCount;
    int quadCount;

    void Clear()
    {
        hexSize = tetSize = triSize = quadSize = 0;
        hexCount = tetCount = triCount = quadCount = 0;
    }
};

SizeData VerdictSizeData;

void SumSize(avtDataRepresentation &, void *, bool &);

inline void
Swap1(double &a, double &b)
{
    double tmp = a;
    a = b;
    b = tmp;
}

inline void
Swap3(double c[][3], int a, int b)
{
    Swap1(c[a][0], c[b][0]);
    Swap1(c[a][1], c[b][1]);
    Swap1(c[a][2], c[b][2]);
}

// ****************************************************************************
//  Method: avtVerdictFilter::avtVerdictFilter
//
//  Purpose:
//      Constructor. Only purpose is to throw an exception if verdict is not
//      installed.
//
//  Programmer:   Akira Haddox
//  Creation:     June 11, 2002
//
// ****************************************************************************

avtVerdictFilter::avtVerdictFilter()
{
    #ifndef HAVE_VERDICT
    EXCEPTION1(ExpressionException, "Verdict was not distributed with this "
                                    "release of VisIt.");
    #endif
}


// ****************************************************************************
//  Method: avtVerdictFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtVerdictFilter::~avtVerdictFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtVerdictFilter::DeriveVariable
//
//  Purpose:
//      Derives a variable based on the input dataset, using a metric call.
//
//  Arguments:
//      in_ds     The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Akira Haddox
//  Creation:     June 11, 2002
//
//
//  Modifications:
//
//    Akira Haddox, Wed Jul  2 08:26:30 PDT 2003
//    Added conversion from pixel cell type.
//
//    Hank Childs, Thu May 19 10:47:08 PDT 2005
//    Allow for sub-types to speed up execution by operating directly on the
//    mesh.
//
// ****************************************************************************

vtkDataArray *
avtVerdictFilter::DeriveVariable(vtkDataSet *in_ds)
{
    int i, j;

    int nCells = in_ds->GetNumberOfCells();

    //
    // Set up a VTK variable reflecting the results we have calculated.
    //
    vtkFloatArray *dv = vtkFloatArray::New();
    dv->SetNumberOfTuples(nCells);

    //
    // Iterate over each cell in the mesh and if it matches a
    // testData prerequisites, run the corresponding metric
    //
    if (OperateDirectlyOnMesh(in_ds))
    {
        MetricForWholeMesh(in_ds, dv);
    }
    else
    {
        const int MAXPOINTS = 100;
        double coordinates[MAXPOINTS][3];
        for (i = 0; i < nCells; i++)    
        {
            vtkCell *cell = in_ds->GetCell(i);
            
            int numPointsForThisCell = cell->GetNumberOfPoints();
            // Grab a pointer to the cell's points' underlying data array
            vtkDataArray *pointData = cell->GetPoints()->GetData();
    
            //
            // Since the Verdict functions make their own copy of the data
            // anyway it would be nice to get the coordinate data without
            // copying (to cut down on unneeded copying). However, this might 
            // be infeasible since Verdict expects doubles, and vtk 
            //(potentially) uses floats.
            //
            
            if (pointData->GetNumberOfComponents() != 3)
            {
                EXCEPTION0(ImproperUseException);
            }
    
            // Fortunately, Verdict will convert to a double[3] for us
            for (j = 0; j < numPointsForThisCell; j++)
            {
                coordinates[j][2] = 0; // In case of 2d coordinates
                pointData->GetTuple(j,coordinates[j]);
            }
    
            int cellType = cell->GetCellType();
            
            // Convert Voxel format into hexahedron format.
            if (cellType == VTK_VOXEL)
            {
                Swap3(coordinates, 2,3);
                Swap3(coordinates, 6,7);
            }
    
            // Convert Pixel format into quad format.
            if (cellType == VTK_PIXEL)
            {
                Swap3(coordinates, 2, 3);
                cellType = VTK_QUAD;
            }
    
            float result = Metric(coordinates, cellType);
            dv->SetTuple1(i, result);
        }
    }
    
    return dv;
}

// ****************************************************************************
//  Method: avtVerdictFilter::PreExecute
//
//  Purpose:
//      If this metric requires size calculation, then before the tree is
//      executed, this function will calculate and set the verdict size
//      variables.
//
//  Arguments:
//      in_ds     The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Akira Haddox
//  Creation:     June 11, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Oct 17 08:07:53 PDT 2002
//    Update for new verdict interface.
//
// ****************************************************************************

void
avtVerdictFilter::PreExecute()
{
#ifdef HAVE_VERDICT
    if (!RequiresSizeCalculation())
        return;

    avtExpressionFilter::PreExecute();

    VerdictSizeData.Clear();

    avtDataTree_p tree = GetInputDataTree();
    bool suc;
    tree->Traverse(SumSize,NULL,suc);

    if (VerdictSizeData.hexCount)
        v_set_hex_size(VerdictSizeData.hexSize/
                     (double)(VerdictSizeData.hexCount));
    if (VerdictSizeData.tetCount)
        v_set_tet_size(VerdictSizeData.tetSize/
                     (double)(VerdictSizeData.tetCount));
    if (VerdictSizeData.triCount)
        v_set_tri_size(VerdictSizeData.triSize/
                     (double)(VerdictSizeData.triCount));
    if (VerdictSizeData.quadCount)
        v_set_quad_size(VerdictSizeData.quadSize/
                      (double)(VerdictSizeData.quadCount));
#endif
}


// ****************************************************************************
//  Method: avtVerdictFilter::MetricForWholeMesh
//
//  Purpose:
//      Calculates a metric for the whole mesh.  This should be re-defined
//      by derived types that re-define OperateDirectlyOnMesh to return true.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2005
//
// ****************************************************************************

void
avtVerdictFilter::MetricForWholeMesh(vtkDataSet *ds, vtkDataArray *rv)
{
    EXCEPTION0(ImproperUseException);
}


// ****************************************************************************
//  Function: SumSize
//
//  Purpose:
//      Used for data tree traversal to collect sums of area/volume && quantity
//      of different cell types. Stores information in VerdictSizeData.
//
//  Arguments:
//      adr       The data.
//      <not used>
//      <not used>
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Akira Haddox
//  Creation:     June 11, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Oct 17 08:07:53 PDT 2002
//    Update for new verdict interface.
//
//    Akira Haddox, Wed Jul  2 08:26:30 PDT 2003
//    Added conversion from pixel cell type.
//
// ****************************************************************************

void SumSize(avtDataRepresentation &adr, void *, bool &)
{
#ifdef HAVE_VERDICT
    vtkDataSet *in_ds = adr.GetDataVTK();
    int i, j;

    int nCells = in_ds->GetNumberOfCells();

    double *results = new double[nCells];

    const int MAXPOINTS = 100;
    double coordinates[MAXPOINTS][3];
    for (i = 0; i < nCells; i++)    
    {
        vtkCell *cell = in_ds->GetCell(i);
        
        int numPointsForThisCell = cell->GetNumberOfPoints();
        // Grab a pointer to the cell's points' underlying data array
        vtkDataArray *pointData = cell->GetPoints()->GetData();

        if (pointData->GetNumberOfComponents() != 3)
        {
            EXCEPTION0(ImproperUseException);
        }

        // Fortunately, Verdict will convert to a double[3] for us
        for (j = 0; j < numPointsForThisCell; j++)
        {
            coordinates[j][2] = 0; // In case of 2d coordinates
            pointData->GetTuple(j,coordinates[j]);
        }

        // Convert Voxel format into hexahedron format.
        if (cell->GetCellType() == VTK_VOXEL)
        {
                Swap3(coordinates, 2,3);
                Swap3(coordinates, 6,7);
        }

        // Convert Pixel format into quad format.
        if (cell->GetCellType() == VTK_PIXEL)
        {
            Swap3(coordinates, 2, 3);
        }

        switch (cell->GetCellType())
        {
            case VTK_VOXEL:
            case VTK_HEXAHEDRON:
                ++VerdictSizeData.hexCount;
                VerdictSizeData.hexSize+=v_hex_volume(8, coordinates);
                break;
            case VTK_TETRA:
                ++VerdictSizeData.tetCount;
                VerdictSizeData.tetSize+=v_tet_volume(4, coordinates);
                break;
            case VTK_TRIANGLE:
                ++VerdictSizeData.triCount;
                VerdictSizeData.triSize+=v_tri_area(3, coordinates);
                break;
            case VTK_PIXEL:
            case VTK_QUAD:
                ++VerdictSizeData.quadCount;
                VerdictSizeData.quadSize+=v_quad_area(3, coordinates);
            default:
                break;
        }
    }
    delete []results;
#endif
}
