/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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
//                               avtXRayFilter.C                             //
// ************************************************************************* //

#include <avtXRayFilter.h>

#include <stdlib.h>

#ifdef PARALLEL
#include <mpi.h>
#endif 

#include <algorithm>

#include <vtkAppendPolyData.h>
#include <vtkCell.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCharArray.h>
#include <vtkDataSet.h>
#include <vtkDataSetWriter.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkIdList.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>

#include <vtkVisItCellLocator.h>
#include <vtkVisItUtility.h>

#include <avtCallback.h>
#include <avtDatasetExaminer.h>
#include <avtIntervalTree.h>
#include <avtParallel.h>

#include <Utility.h>

#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <TimingsManager.h>


using     std::string;


static int IntersectLineWithRevolvedSegment(const double *line_pt,
                                            const double *, const double *, 
                                            const double *, double *);


inline double Dot(const double v1[3], const double v2[3])
{
    return (v1[0] * v2[0]) + (v1[1] * v2[1]) +  (v1[2] * v2[2]);
}

inline void Cross(double result[3], const double v1[3], const double v2[3])
{
    result[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
    result[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
    result[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
}

static bool IntersectLineWithQuad(const double v_00[3], const double v_10[3],
    const double v_11[3], const double v_01[3], const double origin[3],
    const double direction[3], double& t)
{
    static const double eps = 10e-6;

    //
    // Reject rays that are parallel to Q, and rays that intersect the
    // plane of Q either on the left of the line V00V01 or on the right
    // of the line V00V10.
    //

    double E_01[3], E_03[3];
    E_01[0] = v_10[0] - v_00[0];
    E_01[1] = v_10[1] - v_00[1];
    E_01[2] = v_10[2] - v_00[2];
    E_03[0] = v_01[0] - v_00[0];
    E_03[1] = v_01[1] - v_00[1];
    E_03[2] = v_01[2] - v_00[2];
    double P[3];
    Cross(P, direction, E_03);
    double det = Dot(E_01, P);
    if (fabs(det) < eps) return false;
    double inv_det = 1.0 / det;
    double T[3];
    T[0] = origin[0] - v_00[0];
    T[1] = origin[1] - v_00[1];
    T[2] = origin[2] - v_00[2];
    double alpha = Dot(T, P) * inv_det;
    if (alpha < 0.0) return false;
    double Q[3];
    Cross(Q, T, E_01);
    double beta = Dot(direction, Q) * inv_det;
    if (beta < 0.0) return false; 

    if ((alpha + beta) > 1.0)
    {
        //
        // Rejects rays that intersect the plane of Q either on the
        // left of the line V11V10 or on the right of the line V11V01.
        //

        double E_23[3], E_21[3];
        E_23[0] = v_01[0] - v_11[0];
        E_23[1] = v_01[1] - v_11[1];
        E_23[2] = v_01[2] - v_11[2];
        E_21[0] = v_10[0] - v_11[0];
        E_21[1] = v_10[1] - v_11[1];
        E_21[2] = v_10[2] - v_11[2];
        double P_prime[3];
        Cross(P_prime, direction, E_21);
        double det_prime = Dot(E_23, P_prime);
        if (fabs(det_prime) < eps) return false;
        double inv_det_prime = double(1.0) / det_prime;
        double T_prime[3];
        T_prime[0] = origin[0] - v_11[0];
        T_prime[1] = origin[1] - v_11[1];
        T_prime[2] = origin[2] - v_11[2];
        double alpha_prime = Dot(T_prime, P_prime) * inv_det_prime;
        if (alpha_prime < double(0.0)) return false;
        double Q_prime[3];
        Cross(Q_prime, T_prime, E_23);
        double beta_prime = Dot(direction, Q_prime) * inv_det_prime;
        if (beta_prime < double(0.0)) return false;
    }

    //
    // Compute the ray parameter of the intersection point, and
    // reject the ray if it does not hit Q.
    //

    t = Dot(E_03, Q) * inv_det;
    if (t < 0.0) return false; 

    return true;
}


// ****************************************************************************
//  Method: avtXRayFilter constructor
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//
// ****************************************************************************

avtXRayFilter::avtXRayFilter()
{
    nLines = 1000;
    initialLine = 0;
    lines  = NULL;

    pos.x  = 0.0f;
    pos.y  = 0.0f;
    pos.z  = 0.0f;
    theta  = 0.0f;
    phi    = 0.0f;
}


// ****************************************************************************
//  Method: avtXRayFilter destructor
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//
// ****************************************************************************

avtXRayFilter::~avtXRayFilter()
{
    if (lines != NULL)
        delete [] lines;
}


// ****************************************************************************
//  Method:  avtXRayFilter::UpdateDataObjectInfo
//
//  Purpose:
//    Set up the atttributes and validity for the output of the filter.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//  
// ****************************************************************************

void
avtXRayFilter::UpdateDataObjectInfo(void)
{
    avtDataAttributes &inAtts      = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts     = GetOutput()->GetInfo().GetAttributes();

    if (inAtts.GetSpatialDimension() == 2 &&
        inAtts.GetMeshCoordType() == AVT_RZ)
        outAtts.SetSpatialDimension(3);

    outAtts.SetTopologicalDimension(1);
    GetOutput()->GetInfo().GetValidity().InvalidateSpatialMetaData();
}


// ****************************************************************************
//  Method: avtXRayFilter::SetNumberOfLines
//
//  Purpose:
//      Sets the number of lines to process in this pass.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//
// ****************************************************************************

void
avtXRayFilter::SetNumberOfLines(int nl)
{
    nLines = nl;
}


// ****************************************************************************
//  Method: avtXRayFilter::SetImageProperties
//
//  Purpose:
//    Set the x ray image properties.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//
// ****************************************************************************

void
avtXRayFilter::SetImageProperties(float *pos_, float  theta_, float  phi_,
    float  dx_, float  dy_, int nx_, int ny_)
{
    pos.x = pos_[0];
    pos.y = pos_[1];
    pos.z = pos_[2];
    theta = theta_;
    phi = phi_;
    dx = dx_;
    dy = dy_;
    nx = nx_;
    ny = ny_;
}


// ****************************************************************************
//  Method: avtXRayFilter::Execute
//
//  Purpose:
//      Processes a single domain.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//
// ****************************************************************************

void
avtXRayFilter::Execute(void)
{
    //
    // Get the input data tree to obtain the data sets.
    //
    avtDataTree_p tree = GetInputDataTree();

    //
    // Get the data sets.
    //
    vtkDataSet **dataSets = tree->GetAllLeaves(totalNodes);

    //
    // Intersect the data sets with the lines.
    //
    int *nLinesPerDataset = new int[totalNodes];
    vector<double> *dists = new vector<double>[totalNodes];
    vector<int> *line_ids = new vector<int>[totalNodes];
    float ***cellData = new float**[totalNodes];

    int t1 = visitTimer->StartTimer();
    if (GetInput()->GetInfo().GetAttributes().GetMeshCoordType() == AVT_RZ &&
        GetInput()->GetInfo().GetAttributes().GetSpatialDimension() == 2)
    {
        for (currentNode = 0; currentNode < totalNodes; currentNode++)
            CylindricalExecute(dataSets[currentNode],
                nLinesPerDataset[currentNode], dists[currentNode],
                line_ids[currentNode], cellData[currentNode]);
    }
    else
    {
        for (currentNode = 0; currentNode < totalNodes; currentNode++)
            CartesianExecute(dataSets[currentNode],
                nLinesPerDataset[currentNode], dists[currentNode],
                line_ids[currentNode], cellData[currentNode]);
    }
    visitTimer->StopTimer(t1, "avtXRayFilter::CartesianExecute");

    //
    // Calculate the number of cell arrays, the number of components per
    // cell array and the names of the cell arrays.  Note that any array
    // that is not of type VTK_FLOAT is stripped out.  We only need the
    // data arrays at this point and they should all be VTK_FLOAT.
    //
    int     nCellArrays = 0;
    int    *nComponentsPerCellArray = NULL;
    string *cellArrayNames = NULL;
    if (totalNodes > 0)
    {
        nCellArrays = dataSets[0]->GetCellData()->GetNumberOfArrays();
        nComponentsPerCellArray = new int[nCellArrays];
        cellArrayNames = new string[nCellArrays];
        int j = 0;
        for (int i = 0; i < nCellArrays; i++)
        {
            vtkDataArray *da=dataSets[0]->GetCellData()->GetArray(i);
            if (da->GetDataType() == VTK_FLOAT)
            {
                nComponentsPerCellArray[j] = da->GetNumberOfComponents();
                cellArrayNames[j] = da->GetName();
                j++;
            }
        }
        nCellArrays = j;
    }

#if 0
    //
    // Unify the number of cell arrays across all the processors.
    //
    int maxNCellArrays;
#ifdef PARALLEL
    MPI_AllReduce(&nCellArrays, &maxNCellArrays, 1, MPI_INT, MPI_MAX,
        VISIT_MPI_COMM);
#else
    maxNCellArray = nCellArrays;
#endif
    if (nCellArrays != maxNCellArray
    
    if (totalNodes == 0)
    {
        nComponentsPerCellArray = new int[nCellArrays];
        cellArrayNames = new string[nCellArrays];
        for (int i = 0; i < nCellArrays; i++)
            nComponentsPerCellArray[i] = 1;
    }
#endif

    //
    // Redistribute the line segments to processors that own them.
    //
    t1 = visitTimer->StartTimer();
    RedistributeLines(totalNodes, nLinesPerDataset, dists, line_ids,
        nCellArrays, cellArrayNames, nComponentsPerCellArray, cellData);
    visitTimer->StopTimer(t1, "avtXRayFilter::RedistributeLines");

    //
    // Clean up temporary arrays.
    //
    delete [] nLinesPerDataset;
    delete [] dists;
    delete [] line_ids;
    delete [] nComponentsPerCellArray;
    delete [] cellArrayNames;
    for (int i = 0; i < totalNodes; i++)
    {
        float **cellDataI = cellData[i];
        for (int j = 0; j < nCellArrays; j++)
        {
            float *vals = cellDataI[j];
            delete [] vals;
        }
        delete [] cellData[i];
    }
    delete [] cellData;
}


// ****************************************************************************
//  Method: avtXRayFilter::PreExecute
//
//  Purpose:
//      This is called before all of the domains are executed.  This defines
//      the lines that will be intersected with the cells.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//
// ****************************************************************************

void
avtXRayFilter::PreExecute(void)
{
    avtDatasetToDatasetFilter::PreExecute();

    if (lines != NULL)
        delete [] lines;
    lines = new double[6*nLines];

    int spatDim = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();

    double extents[6];
    avtDataset_p input = GetTypedInput();
    avtDatasetExaminer::GetSpatialExtents(input, extents);
    UnifyMinMax(extents, 6);

    if (GetInput()->GetInfo().GetAttributes().GetMeshCoordType() == AVT_RZ &&
        GetInput()->GetInfo().GetAttributes().GetSpatialDimension() == 2)
    {
        spatDim = 3;
        extents[4] = extents[0];
        extents[5] = extents[1];
        double max1 = fabs(extents[2]);
        double max2 = fabs(extents[3]);
        double max = (max1 > max2 ? max1 : max2);
        extents[0] = -max;
        extents[1] = +max;
        extents[2] = -max;
        extents[3] = +max;
    }
    double length = sqrt((extents[1]-extents[0])*(extents[1]-extents[0])+
                         (extents[3]-extents[2])*(extents[3]-extents[2])+
                         (extents[5]-extents[4])*(extents[5]-extents[4]));
    length = (length / 2.) * 1.01;
    double origin[3];
    origin[0] = (extents[0]+extents[1])/2.;
    origin[1] = (extents[2]+extents[3])/2.;
    origin[2] = (extents[4]+extents[5])/2.;

    if (spatDim == 3)
    {
        double cosT = cos(theta);
        double sinT = sin(theta);
        double cosP = cos(phi);
        double sinP = sin(phi);

        avtVector dir(sinT*cosP, sinT*sinP, cosT);

        double m11 = cosT*cosP;
        double m12 = cosT*sinP;
        double m13 = sinT;
        double m21 = -sinP;
        double m22 = cosP;
        double m23 = 0;
        double m31 = -sinT*cosP;
        double m32 = -sinT*sinP;
        double m33 = cosT;

        // Assumes (nLines % nx) == 0.
        double ddx = dx / nx;
        double ddy = dy / ny;
        int jstart = initialLine / nx;
        int jend = jstart + (nLines / nx);
        double y = - (dy / 2.) + ddy / 2. + jstart * ddy;
        int ii = 0;
        for (int j = jstart; j < jend; j++)
        {
            double x = - (dx / 2.) + ddx / 2.;
            for (int i = 0; i < nx; i++)
            {
                avtVector pixelLoc(x*m11+y*m21, x*m12+y*m22, x*m13+y*m23);

                lines[6*ii+0] = pos.x + pixelLoc.x - 2.0*length*dir.x;
                lines[6*ii+1] = pos.x + pixelLoc.x + 2.0*length*dir.x;
                lines[6*ii+2] = pos.y + pixelLoc.y - 2.0*length*dir.y;
                lines[6*ii+3] = pos.y + pixelLoc.y + 2.0*length*dir.y;
                lines[6*ii+4] = pos.z + pixelLoc.z - 2.0*length*dir.z;
                lines[6*ii+5] = pos.z + pixelLoc.z + 2.0*length*dir.z;

                x += ddx;
                ii++;
            }
            y += ddy;
        }
    }
    else
    {
    }
}


// ****************************************************************************
//  Method: avtXRayFilter::PostExecute
//
//  Purpose:
//      This is called after all of the domains are executed.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//
// ****************************************************************************

void
avtXRayFilter::PostExecute(void)
{
    avtDatasetToDatasetFilter::PostExecute();
}


// ****************************************************************************
//  Method: avtXRayFilter::CartesianExecute
//
//  Purpose:
//      Finds line intersections in cartesian space.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//  
// ****************************************************************************

void
avtXRayFilter::CartesianExecute(vtkDataSet *ds, int &nLinesPerDataset,
    vector<double> &dist, vector<int> &line_id, float **&cellData)
{
    int  i, j;

    //
    // Create an interval tree for the data set to find the intersections
    // of the lines with the data set.
    //
    int t1 = visitTimer->StartTimer();
    int nCells = ds->GetNumberOfCells();
    if (nCells == 0)
    {
        nLinesPerDataset = 0;

        int nCellArrays = ds->GetCellData()->GetNumberOfArrays();
        cellData = new float*[nCellArrays];
        for (int i = 0; i < nCellArrays; i++)
            cellData[i] = new float[0];

        return;
    }
    int dims   = 3;
    avtIntervalTree tree(nCells, dims);
    double bounds[6];
    vtkDataArray *ghosts = ds->GetCellData()->GetArray("avtGhostZones");
    bool hasGhost = (ghosts != NULL);
    for (i = 0 ; i < nCells ; i++)
    {
        ds->GetCellBounds(i, bounds);
        tree.AddElement(i, bounds);
    }
    tree.Calculate(true);
    visitTimer->StopTimer(t1, "avtXRayFilter::CreateIntervalTree");

    //
    // Loop over the lines.
    //
    int extraMsg = 100;
    int amtPerMsg = nLines / extraMsg + 1;
    int totalProg = totalNodes * extraMsg;
    UpdateProgress(extraMsg*currentNode, totalProg);
    int lastMilestone = 0;

    vector<int> cells_matched;

    if (ds->GetDataObjectType() == VTK_STRUCTURED_GRID)
    {
        vtkStructuredGrid *sgrid = (vtkStructuredGrid *) ds;
        vtkPoints *points = sgrid->GetPoints();

        int ndims[3];
        sgrid->GetDimensions(ndims);

        int zdims[3];
        zdims[0] = ndims[0] - 1;
        zdims[1] = ndims[1] - 1;
        zdims[2] = ndims[2] - 1;

        float *pts = (float *) points->GetVoidPointer(0);

        int nx = ndims[0];
        int ny = ndims[1];
        int nz = ndims[2];
        int nxy = nx * ny;

        int nx2 = zdims[0];
        int ny2 = zdims[1];
        int nz2 = zdims[2];
        int nxy2 = nx2 * ny2;

        for (i = 0 ; i < nLines ; i++)
        {
            double pt1[3];
            pt1[0] = lines[6*i];
            pt1[1] = lines[6*i+2];
            pt1[2] = lines[6*i+4];
            double pt2[3];
            pt2[0] = lines[6*i+1];
            pt2[1] = lines[6*i+3];
            pt2[2] = lines[6*i+5];
            double dir[3];
            dir[0] = pt2[0] - pt1[0];
            dir[1] = pt2[1] - pt1[1];
            dir[2] = pt2[2] - pt1[2];

            vector<int> list;
            tree.GetElementsList(pt1, dir, list);
            int nCells = list.size();
            if (nCells == 0)
                continue;  // No intersection

            double lineLength = sqrt((pt2[0]-pt1[0]) * (pt2[0]-pt1[0]) +
                                     (pt2[1]-pt1[1]) * (pt2[1]-pt1[1]) +
                                     (pt2[2]-pt1[2]) * (pt2[2]-pt1[2]));

            for (j = 0 ; j < nCells ; j++)
            {
                //
                // Determine the index into the look up table.
                //
                int iCell = list[j];

                int iZ = iCell / nxy2;
                int iXY = iCell % nxy2;
                int iY = iXY / nx2;
                int iX = iXY % nx2;
                int idx = iX+ iY*nx + iZ*nxy;

                int ids[8];
                ids[0] = idx;
                ids[1] = idx + 1;
                ids[2] = idx + 1 + nx;
                ids[3] = idx + nx;
                idx += nxy;
                ids[4] = idx;
                ids[5] = idx + 1;
                ids[6] = idx + 1 + nx;
                ids[7] = idx + nx;
        
                double p0[3]={pts[ids[0]*3],pts[ids[0]*3+1],pts[ids[0]*3+2]};
                double p1[3]={pts[ids[1]*3],pts[ids[1]*3+1],pts[ids[1]*3+2]};
                double p2[3]={pts[ids[2]*3],pts[ids[2]*3+1],pts[ids[2]*3+2]};
                double p3[3]={pts[ids[3]*3],pts[ids[3]*3+1],pts[ids[3]*3+2]};
                double p4[3]={pts[ids[4]*3],pts[ids[4]*3+1],pts[ids[4]*3+2]};
                double p5[3]={pts[ids[5]*3],pts[ids[5]*3+1],pts[ids[5]*3+2]};
                double p6[3]={pts[ids[6]*3],pts[ids[6]*3+1],pts[ids[6]*3+2]};
                double p7[3]={pts[ids[7]*3],pts[ids[7]*3+1],pts[ids[7]*3+2]};

                double t;
                int nInter = 0;
                double inter[6];

                if (IntersectLineWithQuad(p0, p1, p2, p3, pt1, dir, t))
                    inter[nInter++] = t;
                if (IntersectLineWithQuad(p4, p7, p6, p5, pt1, dir, t))
                    inter[nInter++] = t;
                if (IntersectLineWithQuad(p0, p4, p5, p1, pt1, dir, t))
                    inter[nInter++] = t;
                if (IntersectLineWithQuad(p1, p5, p6, p2, pt1, dir, t))
                    inter[nInter++] = t;
                if (IntersectLineWithQuad(p2, p6, p7, p3, pt1, dir, t))
                    inter[nInter++] = t;
                if (IntersectLineWithQuad(p0, p3, p7, p4, pt1, dir, t))
                    inter[nInter++] = t;

                if (nInter == 2)
                {
                    cells_matched.push_back(iCell);
                    dist.push_back(inter[0]*lineLength);
                    dist.push_back(inter[1]*lineLength);
                    line_id.push_back(i);
                }

                int currentMilestone = (int)(((float) i) / amtPerMsg);
                if (currentMilestone > lastMilestone)
                {
                    UpdateProgress(extraMsg*currentNode+currentMilestone, 
                                   extraMsg*totalNodes);
                    lastMilestone = currentMilestone;
                }
            }
        }
    }
    else
    {
        for (i = 0 ; i < nLines ; i++)
        {
            //
            // Determine which cells intersect the line.
            //
            double pt1[3];
            pt1[0] = lines[6*i];
            pt1[1] = lines[6*i+2];
            pt1[2] = lines[6*i+4];
            double pt2[3];
            pt2[0] = lines[6*i+1];
            pt2[1] = lines[6*i+3];
            pt2[2] = lines[6*i+5];
            double dir[3];
            dir[0] = pt2[0] - pt1[0];
            dir[1] = pt2[1] - pt1[1];
            dir[2] = pt2[2] - pt1[2];

            vector<int> list;
            tree.GetElementsList(pt1, dir, list);
            int nCells = list.size();
            if (nCells == 0)
                continue;  // No intersection

            double lineLength = sqrt((pt2[0]-pt1[0]) * (pt2[0]-pt1[0]) +
                                     (pt2[1]-pt1[1]) * (pt2[1]-pt1[1]) +
                                     (pt2[2]-pt1[2]) * (pt2[2]-pt1[2]));
            for (j = 0 ; j < nCells ; j++)
            {
                int id = list[j];
                if (hasGhost && ghosts->GetTuple1(id) != 0.)
                    continue;
                vtkCell *cell = ds->GetCell(id);

                int nInter = 0;
                double inter[100];
                if (cell->GetCellDimension() == 3)
                {
                    int nFaces = cell->GetNumberOfFaces();
                    for (int k = 0 ; k < nFaces ; k++)
                    {
                        vtkCell *face = cell->GetFace(k);
                        double x[3];
                        double pcoords[3];
                        double t;
                        int subId;
                        if (face->IntersectWithLine(pt1, pt2, 1e-10, t, x, pcoords, 
                                                    subId))
                            inter[nInter++] = t;
                    }
                }
                else if (cell->GetCellDimension() == 2)
                {
                    int nEdges = cell->GetNumberOfEdges();
                    for (int k = 0 ; k < nEdges ; k++)
                    {
                        vtkCell *edge = cell->GetEdge(k);
                        double x[3];
                        double pcoords[3];
                        double t;
                        int subId;
                        if (edge->IntersectWithLine(pt1, pt2, 1e-10, t, x, pcoords, 
                                                    subId))
                            inter[nInter++] = t;
                    }
                }
                if (nInter == 0 || nInter == 1)
                    continue;
                // See if we have any near duplicates.
                if (nInter > 2)
                {
                    for (int ii = 0 ; ii < nInter-1 ; ii++)
                    {
                        for (int jj = ii+1 ; jj < nInter ; jj++)
                        {
                            if (fabs(inter[ii]-inter[jj]) < 1e-10)
                            {
                                inter[ii] = inter[nInter-1];
                                nInter--;
                            }
                        }
                    }
                }
                if (nInter == 2)
                {
                    cells_matched.push_back(id);
                    dist.push_back(inter[0]*lineLength);
                    dist.push_back(inter[1]*lineLength);
                    line_id.push_back(i);
                }
                else
                {
                    // So this is technically an error state.  We have
                    // intersected the shape an odd number of times, which
                    // should mean that we are inside the shape.  We constructed
                    // our lines so that is not possible.  In reality, this occurs
                    // because of floating point precision issues.  In addition,
                    // every time it occurs, it is because we have a *very*
                    // small cell.  The queries that use this filter need to
                    // call "CleanPolyData" on it anyway, so cells this small
                    // will be "cleaned out".  So, rather than throwing an 
                    // exception, we can just continue.
                    continue;
                }

                int currentMilestone = (int)(((float) i) / amtPerMsg);
                if (currentMilestone > lastMilestone)
                {
                    UpdateProgress(extraMsg*currentNode+currentMilestone, 
                                   extraMsg*totalNodes);
                    lastMilestone = currentMilestone;
                }
            }
        }
    }

    nLinesPerDataset = cells_matched.size();

    //
    // Copy the cell data.
    //
    vtkCellData *inCD1 = ds->GetCellData();
    int nCellArrays = inCD1->GetNumberOfArrays();
    cellData = new float*[nCellArrays];
    int iCellData = 0;
    for (int i = 0; i < nCellArrays; i++)
    {
        vtkDataArray *da=inCD1->GetArray(i);
        int nComponents = da->GetNumberOfComponents();
        int nTuples = da->GetNumberOfTuples();

        if (da->GetDataType() == VTK_FLOAT)
        {
            float *inVals = vtkFloatArray::SafeDownCast(da)->GetPointer(0);
            float *outVals = new float[cells_matched.size()*nComponents];
            int ndx = 0;
            for (int j = 0; j < cells_matched.size(); j++)
                for (int k = 0; k < nComponents; k++)
                    outVals[ndx++] = inVals[cells_matched[j]*nComponents+k];

            cellData[iCellData++] = outVals;
        }
    }

    UpdateProgress(extraMsg*(currentNode+1), extraMsg*totalNodes);
}


// ****************************************************************************
//  Method: avtXRayFilter::CylindricalExecute
//
//  Purpose:
//      Finds line intersections in cylindrical space.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//  
// ****************************************************************************

void
avtXRayFilter::CylindricalExecute(vtkDataSet *ds, int &nLinesPerDataset,
    vector<double> &dist, vector<int> &line_id, float **&cellData)
{
    int  i, j;

    //
    // Create an interval tree for the data set to find the intersections
    // of the lines with the data set.
    //
    int nCells = ds->GetNumberOfCells();
    int dims   = 2;
    avtIntervalTree tree(nCells, dims);
    double bounds[6];
    vtkDataArray *ghosts = ds->GetCellData()->GetArray("avtGhostZones");
    bool hasGhost = (ghosts != NULL);
    for (i = 0 ; i < nCells ; i++)
    {
        ds->GetCellBounds(i, bounds);
        tree.AddElement(i, bounds);
    }
    tree.Calculate(true);

    //
    // Loop over the lines.
    //
    int extraMsg = 100;
    int amtPerMsg = nLines / extraMsg + 1;
    int totalProg = totalNodes * extraMsg;
    UpdateProgress(extraMsg*currentNode, totalProg);
    int lastMilestone = 0;

    vector<int> cells_matched;

    for (i = 0 ; i < nLines ; i++)
    {
        //
        // Determine which cells intersect the line.
        //
        double pt1[3];
        pt1[0] = lines[6*i];
        pt1[1] = lines[6*i+2];
        pt1[2] = lines[6*i+4];
        double pt2[3];
        pt2[0] = lines[6*i+1];
        pt2[1] = lines[6*i+3];
        pt2[2] = lines[6*i+5];
        double dir[3];
        dir[0] = pt2[0]-pt1[0];
        dir[1] = pt2[1]-pt1[1];
        dir[2] = pt2[2]-pt1[2];

        vector<int> list;
        tree.GetElementsFromAxiallySymmetricLineIntersection(pt1, dir, list);
        int nCells = list.size();
        if (nCells == 0)
            continue;  // No intersection

        double lineLength = sqrt((pt2[0]-pt1[0]) * (pt2[0]-pt1[0]) +
                                 (pt2[1]-pt1[1]) * (pt2[1]-pt1[1]) +
                                 (pt2[2]-pt1[2]) * (pt2[2]-pt1[2]));
        for (j = 0 ; j < nCells ; j++)
        {
            int id = list[j];
            if (hasGhost && ghosts->GetTuple1(id) != 0.)
                continue;
            vtkCell *cell = ds->GetCell(id);
            vector<double> inter;
            int nEdges = cell->GetNumberOfEdges();
            for (int k = 0 ; k < nEdges ; k++)
            {
                vtkCell *edge = cell->GetEdge(k);
                int id1 = edge->GetPointId(0);
                double ePt1[3];
                ds->GetPoint(id1, ePt1);
                int id2 = edge->GetPointId(1);
                double ePt2[3];
                ds->GetPoint(id2, ePt2);

                double curInter[100];  // shouldn't really be more than 4.
                int numInter =
                    IntersectLineWithRevolvedSegment(pt1, dir, ePt1, ePt2,
                                                     curInter);
                for (int l = 0 ; l < numInter ; l++)
                    inter.push_back(curInter[l]);
            }

            if (inter.size() % 2 == 0)
            {
                if (inter.size() > 0)
                {
                    std::sort(inter.begin(), inter.end());
                    for (int l = 0 ; l < inter.size() / 2 ; l++)
                    {
                        cells_matched.push_back(id);
                        dist.push_back(inter[2*l]*lineLength);
                        dist.push_back(inter[2*l+1]*lineLength);
                        line_id.push_back(i);
                    }
                }
            }
            else
            {
                // So this is technically an error state.  We have
                // intersected the shape an odd number of times, which
                // should mean that we are inside the shape.  We constructed
                // our lines so that is not possible.  In reality, this occurs
                // because of floating point precision issues.  In addition,
                // every time it occurs, it is because we have a *very*
                // small cell.  The queries that use this filter need to
                // call "CleanPolyData" on it anyway, so cells this small
                // will be "cleaned out".  So, rather than throwing an 
                // exception, we can just continue.
                continue;
            }

            int currentMilestone = (int)(((float) i) / amtPerMsg);
            if (currentMilestone > lastMilestone)
            {
                UpdateProgress(extraMsg*currentNode+currentMilestone, 
                               extraMsg*totalNodes);
                lastMilestone = currentMilestone;
            }
        }
    }

    nLinesPerDataset = cells_matched.size();

    //
    // Copy the cell data.
    //
    vtkCellData *inCD1 = ds->GetCellData();
    int nCellArrays = inCD1->GetNumberOfArrays();
    cellData = new float*[nCellArrays];
    int iCellData = 0;
    for (int i = 0; i < nCellArrays; i++)
    {
        vtkDataArray *da=inCD1->GetArray(i);
        int nComponents = da->GetNumberOfComponents();
        int nTuples = da->GetNumberOfTuples();

        if (da->GetDataType() == VTK_FLOAT)
        {
            float *inVals = vtkFloatArray::SafeDownCast(da)->GetPointer(0);
            float *outVals = new float[cells_matched.size()*nComponents];
            int ndx = 0;
            for (int j = 0; j < cells_matched.size(); j++)
                for (int k = 0; k < nComponents; k++)
                    outVals[ndx++] = inVals[cells_matched[j]*nComponents+k];

            cellData[iCellData++] = outVals;
        }
    }

    UpdateProgress(extraMsg*(currentNode+1), extraMsg*totalNodes);
}


#ifdef PARALLEL
static int
AssignToProc(int val, int nlines)
{
    static int nprocs = PAR_Size();

    int linesPerProc = nlines/nprocs + 1;
    int proc = val / linesPerProc;
    return proc;
}
#endif


// ****************************************************************************
//  Method: avtXRayFilter::RedistributeLines
//
//  Purpose:
//      Redistribute the lines to the processors.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//
// ****************************************************************************

void
avtXRayFilter::RedistributeLines(int nLeaves, int *nLinesPerDataset,
    vector<double> *dists, vector<int> *line_ids, int nCellArrays,
    string *cellArrayNames, int *nComponentsPerCellArray, float ***cellData)
{
#ifdef PARALLEL
    //
    // Assign lines to processors.
    //
    int nProcs = PAR_Size();
    int *sendCounts = new int[nProcs];
    for (int i = 0; i < nProcs; i++)
        sendCounts[i] = 0;
    for (int i = 0; i < nLeaves; i++)
    {
        vector<int> inLineIds = line_ids[i];
        for (int j = 0; j < nLinesPerDataset[i]; j++)
            sendCounts[AssignToProc(inLineIds[j], nLines)]++;
    }

    //
    // Determine the size of the send buffers.
    //
    int nLinesSend = 0;
    for (int i = 0; i < nLeaves; i++)
        nLinesSend += nLinesPerDataset[i];

    //
    // Create the send buffers.
    //
    int *sendLineIds = new int[nLinesSend];
    double *sendDists = new double[2*nLinesSend];
    float **sendCellData = new float*[nCellArrays];
    for (int i = 0; i < nCellArrays; i++)
        sendCellData[i] = new float[nComponentsPerCellArray[i]*nLinesSend];
    
    //
    // Fill the send buffers.
    //
    int *sendOffsets = new int[nProcs];
    sendOffsets[0] = 0;
    for (int i = 1; i < nProcs; i++)
        sendOffsets[i] = sendOffsets[i-1] + sendCounts[i-1];

    for (int i = 0; i < nLeaves; i++)
    {
        vector<int> inLineIds = line_ids[i];
        vector<double> inDists = dists[i];
        float **inCellData = cellData[i];
        for (int j = 0; j < nLinesPerDataset[i]; j++)
        {
            int iProc = AssignToProc(inLineIds[j], nLines);
            int iOffset = sendOffsets[iProc];
            sendLineIds[iOffset] = inLineIds[j];
            sendDists[iOffset*2] = inDists[j*2];
            sendDists[iOffset*2+1] = inDists[j*2+1];
            for (int k = 0; k < nCellArrays; k++)
            {
                float *inVar = inCellData[k];
                float *sendVar = sendCellData[k];
                int nComps = nComponentsPerCellArray[k];
                for (int l = 0; l < nComps; l++)
                    sendVar[iOffset*nComps+l] = inVar[j*nComps+l];
            }
            sendOffsets[iProc]++;
        }
    }

    //
    // Calculate the receive counts.
    //
    int *recvCounts = new int[nProcs];
    MPI_Alltoall(sendCounts, 1, MPI_INT, recvCounts, 1, MPI_INT,
        VISIT_MPI_COMM);

    //
    // Determine the size of the receive buffers.
    //
    int nLinesRecv = 0;
    for (int i = 0; i < nProcs; i++)
        nLinesRecv += recvCounts[i];

    //
    // Create the output arrays.
    //
    vtkIntArray *outLineIdsArray = vtkIntArray::New();
    outLineIdsArray->SetName("LineIds");
    outLineIdsArray->SetNumberOfComponents(1);
    outLineIdsArray->SetNumberOfTuples(nLinesRecv);
    int *outLineIds = outLineIdsArray->GetPointer(0);

    vtkDoubleArray *outDistsArray = vtkDoubleArray::New();
    outDistsArray->SetName("Dists");
    outDistsArray->SetNumberOfComponents(2);
    outDistsArray->SetNumberOfTuples(nLinesRecv);
    double *outDists = outDistsArray->GetPointer(0);

    float **outCellData = new float*[nCellArrays];
    vtkFloatArray **outCellDataArrays = new vtkFloatArray*[nCellArrays];
    for (int i = 0; i < nCellArrays; i++)
    {
        outCellDataArrays[i] = vtkFloatArray::New();
        outCellDataArrays[i]->SetName(cellArrayNames[i].c_str());
        outCellDataArrays[i]->SetNumberOfComponents(nComponentsPerCellArray[i]);
        outCellDataArrays[i]->SetNumberOfTuples(nLinesRecv);
        outCellData[i] = outCellDataArrays[i]->GetPointer(0);
    }

    //
    // Calculate the send and receive offsets for the line ids.  The
    // previously calculated send offsets have been modified so they need
    // to be recalculated.
    //
    sendOffsets[0] = 0;
    for (int i = 1; i < nProcs; i++)
        sendOffsets[i] = sendOffsets[i-1] + sendCounts[i-1];

    int *recvOffsets = new int[nProcs];
    recvOffsets[0] = 0;
    for (int i = 1; i < nProcs; i++)
        recvOffsets[i] = recvOffsets[i-1] + recvCounts[i-1];

    //
    // Exchange the line ids.
    //
    MPI_Alltoallv(sendLineIds, sendCounts, sendOffsets, MPI_INT,
                  outLineIds, recvCounts, recvOffsets, MPI_INT,
                  VISIT_MPI_COMM);

    //
    // Calculate the send and receive offsets for the dists.  The distances
    // are twice the amount of data as the line ids.
    //
    for (int i = 0; i < nProcs; i++)
        sendCounts[i] *= 2;
    sendOffsets[0] = 0;
    for (int i = 1; i < nProcs; i++)
        sendOffsets[i] = sendOffsets[i-1] + sendCounts[i-1];

    for (int i = 0; i < nProcs; i++)
        recvCounts[i] *= 2;
    recvOffsets[0] = 0;
    for (int i = 1; i < nProcs; i++)
        recvOffsets[i] = recvOffsets[i-1] + recvCounts[i-1];

    //
    // Exchange the dists.
    //
    MPI_Alltoallv(sendDists, sendCounts, sendOffsets, MPI_DOUBLE,
                  outDists, recvCounts, recvOffsets, MPI_DOUBLE,
                  VISIT_MPI_COMM);

    //
    // Exchange the cell data.
    //
    for (int i = 0; i < nProcs; i++)
        sendCounts[i] /= 2;
    for (int i = 0; i < nProcs; i++)
        recvCounts[i] /= 2;
    for (int i = 0; i < nCellArrays; i++)
    {
        //
        // Calculate the send and receive offsets for the data.
        //
        for (int j = 0; j < nProcs; j++)
            sendCounts[j] *= nComponentsPerCellArray[i];
        sendOffsets[0] = 0;
        for (int j = 1; j < nProcs; j++)
            sendOffsets[j] = sendOffsets[j-1] + sendCounts[j-1];

        for (int j = 0; j < nProcs; j++)
            recvCounts[j] *= nComponentsPerCellArray[i];
        recvOffsets[0] = 0;
        for (int j = 1; j < nProcs; j++)
            recvOffsets[j] = recvOffsets[j-1] + recvCounts[j-1];

        //
        // Exchange the cell data.
        //

        MPI_Alltoallv(sendCellData[i], sendCounts, sendOffsets, MPI_FLOAT,
                      outCellData[i], recvCounts, recvOffsets, MPI_FLOAT,
                      VISIT_MPI_COMM);

        //
        // Restore the send and receive counts.
        //
        for (int j = 0; j < nProcs; j++)
            sendCounts[j] /= nComponentsPerCellArray[i];
        for (int j = 0; j < nProcs; j++)
            recvCounts[j] /= nComponentsPerCellArray[i];
    }

    //
    // Clean up memory.
    //
    delete [] sendCounts;
    delete [] sendOffsets;

    delete [] recvCounts;
    delete [] recvOffsets;

    delete [] sendLineIds;
    delete [] sendDists;
    for (int i = 0; i < nCellArrays; i++)
        delete [] sendCellData[i];
    delete [] sendCellData;
#else
    int nLinesTotal = 0;
    for (int i = 0; i < nLeaves; i++)
        nLinesTotal += nLinesPerDataset[i];

    vtkIntArray *outLineIdsArray = vtkIntArray::New();
    outLineIdsArray->SetName("LineIds");
    outLineIdsArray->SetNumberOfComponents(1);
    outLineIdsArray->SetNumberOfTuples(nLinesTotal);
    int *outLineIds = outLineIdsArray->GetPointer(0);

    vtkDoubleArray *outDistsArray = vtkDoubleArray::New();
    outDistsArray->SetName("Dists");
    outDistsArray->SetNumberOfComponents(2);
    outDistsArray->SetNumberOfTuples(nLinesTotal);
    double *outDists = outDistsArray->GetPointer(0);

    float **outCellData = new float*[nCellArrays];
    vtkFloatArray **outCellDataArrays = new vtkFloatArray*[nCellArrays];
    for (int i = 0; i < nCellArrays; i++)
    {
        outCellDataArrays[i] = vtkFloatArray::New();
        outCellDataArrays[i]->SetName(cellArrayNames[i].c_str());
        outCellDataArrays[i]->SetNumberOfComponents(nComponentsPerCellArray[i]);
        outCellDataArrays[i]->SetNumberOfTuples(nLinesTotal);
        outCellData[i] = outCellDataArrays[i]->GetPointer(0);
    }

    int iLines = 0;
    int iPoints = 0;
    int *iCellStart = new int[nCellArrays];
    for (int i = 0; i < nCellArrays; i++)
        iCellStart[i] = 0;
    for (int i = 0; i < nLeaves; i++)
    {
        vector<int> inLineIds = line_ids[i];
        vector<double> inDists = dists[i];
        for (int j = 0; j < nLinesPerDataset[i]; j++)
        {
            outLineIds[iLines++] = inLineIds[j];
            outDists[iPoints++] = inDists[j*2];
            outDists[iPoints++] = inDists[j*2+1];
        }

        float **inCellData = cellData[i];
        for (int j = 0; j < nCellArrays; j++)
        {
            int iCell = iCellStart[j];
            float *inVar = inCellData[j];
            float *outVar = outCellData[j];
            for (int k = 0; k < nLinesPerDataset[i]*nComponentsPerCellArray[j]; k++)
                outVar[iCell++] = inVar[k];
            iCellStart[j] = iCell;
        }
    }
#endif

    //
    // Create a vtkPolyData set with no polydata to hold the fields.
    //
    vtkDataSet *outDataSet = vtkPolyData::New();
    outDataSet->GetPointData()->AddArray(outLineIdsArray);
    outDataSet->GetPointData()->CopyFieldOn("LineIds");
    outDataSet->GetPointData()->AddArray(outDistsArray);
    outDataSet->GetPointData()->CopyFieldOn("Dists");
    for (int i = 0; i < nCellArrays; i++)
    {
        outDataSet->GetCellData()->AddArray(outCellDataArrays[i]);
        outDataSet->GetCellData()->CopyFieldOn(cellArrayNames[i].c_str());
        outCellDataArrays[i]->Delete();
    }
    outLineIdsArray->Delete();
    outDistsArray->Delete();

    avtDataTree_p newtree = new avtDataTree(outDataSet, -1);
    SetOutputDataTree(newtree);
    outDataSet->Delete();

    delete [] outCellData;
}


// ****************************************************************************
//  Method: avtIntersectionTests::IntersectLineWithRevolvedSegment
//
//  Purpose:
//      Takes a segment that is in cylindrical coordinates and revolves it
//      into three-dimensional Cartesian space and finds the intersections
//      with a line.  The number of intersections can be 1, 2, or 4.
//
//  Arguments:
//      line_pt    A point on the line (Cartesian)
//      line_dir   The direction of the line (Cartesian)
//      seg_1      One endpoint of the segment (Cylindrical)
//      seg_2      The other endpoint of the segment (Cylindrical)
//      inter      The intersections found.  Output value.  They are 
//                 represented distances along line_dir from line_pt.
//
//  Returns:       The number of intersections
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//  
// ****************************************************************************

int
IntersectLineWithRevolvedSegment(const double *line_pt,
                                 const double *line_dir, const double *seg_p1, 
                                 const double *seg_p2, double *inter)
{
    if (seg_p1[1] < 0. || seg_p2[1] < 0.)
    {
        // Doesn't make sense to cross the axis line.
        avtCallback::IssueWarning("VisIt is unable to execute this query, "
                     "because it has an encountered an RZ mesh with "
                     "negative R values.");

        EXCEPTION0(ImproperUseException);
    }

    //
    // Note that in the logic below, we are using the Z-component
    // of the line to compare with the X-component of the cell,
    // since the cell's X-component is actually 'Z' in RZ-space.
    //
    if (seg_p1[0] == seg_p2[0])
    {
        // Vertical line .. revolves to hollow disc.
        // Disc is at some constant Z (seg_p1[0]) and ranges between some
        // Rmin and Rmax.
        double Rmin = (seg_p1[1] < seg_p2[1] ? seg_p1[1] : seg_p2[1]);
        double Rmax = (seg_p1[1] > seg_p2[1] ? seg_p1[1] : seg_p2[1]);
        if (line_dir[2] == 0.)
        {
            if (seg_p1[0] != line_pt[2])
                return 0;
            
            // Solving for inequalities is tough.  In this case, we will
            // solve for equalities.  Solve for R = Rmax and R = Rmin.
            // At^2 + Bt + C = 0
            //  A = Dx^2 + Dy^2
            //  B = 2*Dx*Px + 2*Dy*Py
            //  C = Px^2 + Py^2 - R^2
            double A = line_dir[0]*line_dir[0] + line_dir[1]*line_dir[1];
            double B = 2*line_dir[0]*line_pt[0] + 2*line_dir[1]*line_pt[1];
            double C0 = line_pt[0]*line_pt[0] + line_pt[1]*line_pt[1];
            double C = C0 - Rmax*Rmax;
            double det = B*B - 4*A*C;
            int nInter = 0;
            if (det > 0)
            {
                double soln1 = (-B + sqrt(det)) / (2*A);
                double soln2 = (-B - sqrt(det)) / (2*A);
                inter[nInter++] = soln1;
                inter[nInter++] = soln2;
            }
            C = C0 - Rmin*Rmin;
            det = B*B - 4*A*C;
            if (det > 0)
            {
                double soln1 = (-B + sqrt(det)) / (2*A);
                double soln2 = (-B - sqrt(det)) / (2*A);
                inter[nInter++] = soln1;
                inter[nInter++] = soln2;
            }
        }
        else
        {
            // Our line will go through the constant Z-plane that the segment
            // gets revolved into just once.  So calculate that plane and
            // determine if the line intersects the disc.
            double t = (seg_p1[0] - line_pt[2]) / line_dir[2];
            double x = line_pt[0] + t*line_dir[0];
            double y = line_pt[1] + t*line_dir[1];
            double Rsq = x*x + y*y;
            if (Rmin*Rmin < Rsq && Rsq < Rmax*Rmax)
            {
                inter[0] = t;
                return 1;
            }
        }
    }
    else if (seg_p1[1] == seg_p2[1])
    {
        // Horizonal line ... revolves to cylindrical shell.
        // Shell has constant radius (seg_p1[1]) and ranges between Z=seg_p1[0]
        // and Z=seg_p2[0].
        // Solve for t that has intersection.
        // ((Px + t*Dx)^2 + (Py + t*Dy)^2)^0.5 = R
        // ((Px + t*Dx)^2 + (Py + t*Dy)^2) = R^2
        // At^2 + Bt + C = 0
        // A = (Dx^2 + Dy^2)
        // B = (2*Dx*Px + 2*Dy*Py)
        // C = (Px^2 + Py^2 - R^2)
        double R = seg_p1[1];
        double A = line_dir[0]*line_dir[0] + line_dir[1]*line_dir[1];
        double B = 2*line_pt[0]*line_dir[0] + 2*line_pt[1]*line_dir[1];
        double C = line_pt[0]*line_pt[0] + line_pt[1]*line_pt[1] - R*R;
        double det = B*B - 4*A*C;
        if (det < 0)
            return 0;
        double soln1 = (-B + sqrt(det)) / (2*A);
        double soln2 = (-B - sqrt(det)) / (2*A);
        double Z1 = line_pt[2] + soln1*line_dir[2];
        double Z2 = line_pt[2] + soln2*line_dir[2];

        int nInter = 0;
        double Zmin = (seg_p1[0] < seg_p2[0] ? seg_p1[0] : seg_p2[0]);
        double Zmax = (seg_p1[0] > seg_p2[0] ? seg_p1[0] : seg_p2[0]);
        if (Zmin <= Z1 && Z1 <= Zmax)
        {
            inter[nInter] = soln1;
            nInter++;
        }
        if (Zmin <= Z2 && Z2 <= Zmax)
        {
            inter[nInter] = soln2;
            nInter++;
        }
        return nInter;
    }
    else
    {
        // We have a segment that is neither horizontal or vertical.  So the
        // revolution of this segment will result in a conic shell, truncated
        // in Z.  The conic shell will be oriented around the Z-axis.
        // The segment is along a line of form r = mz+b.  If we can calculate
        // m and b, then we can ask when a point on the line will coincide
        // with our line.  If they coincided, they would have the same r and
        // z values.  Since the line is in 3D, having the same r values
        // means having the same sqrt(x^2+y^2) values.
        // 
        // Then, for the line:
        // x = Px + t*Dx
        // y = Py + t*Dy
        // z = Pz + t*Dz
        // For the conic shell, we know:
        // sqrt(x^2+y^2) = m*z+b
        // Substituting for Z gives:
        // sqrt(x^2+y^2) = m*(Pz+tDz)+b
        // sqrt(x^2+y^2) = (m*Pz+b) + t*m*Dz
        // Introducing K for m*Pz+b (to simplify algebra)
        // sqrt(x^2+y^2) = K + t*m*Dz
        // Substituting for x and y and squaring gives:
        // (Px^2 + Py^2) + (2*Px*Dx + 2*Py*Dy)t + (Dx^2 + Dy^2)t^2 
        //    = K^2 + (2*K*m*Dz)*t + m^2*Dz^2*t^2
        // Combining like terms gives:
        //  At^2 + Bt + C = 0
        //  A = Dx^2 + Dy^2 - m^2*Dz^2
        //  B = 2*Px*Dx + 2*Py*Dy - 2*K*m*Dz
        //  C = Px^2 + Py^2 - K^2
        // And then we can solve for t to find the intersections.
        // At the end, we will restrict the answer to be between the
        // valid range for the segment.
        double m = (seg_p1[1] - seg_p2[1]) / (seg_p1[0] - seg_p2[0]);
        double b = seg_p1[1] - m*seg_p1[0];
        double K = m*line_pt[2]+b;
        double A = line_dir[0]*line_dir[0] + line_dir[1]*line_dir[1]
                 - m*m*line_dir[2]*line_dir[2];
        double B = 2*line_pt[0]*line_dir[0] + 2*line_pt[1]*line_dir[1]
                 - 2*K*m*line_dir[2];
        double C = line_pt[0]*line_pt[0] + line_pt[1]*line_pt[1] - K*K;
        double det = B*B - 4*A*C;
        if (det < 0)
            return 0;
        double soln1 = (-B + sqrt(det)) / (2*A);
        double soln2 = (-B - sqrt(det)) / (2*A);
        double Z1 = line_pt[2] + soln1*line_dir[2];
        double Z2 = line_pt[2] + soln2*line_dir[2];
        int nInter = 0;
        double Zmin = (seg_p1[0] < seg_p2[0] ? seg_p1[0] : seg_p2[0]);
        double Zmax = (seg_p1[0] > seg_p2[0] ? seg_p1[0] : seg_p2[0]);
        if (Zmin <= Z1 && Z1 <= Zmax)
        {
            inter[nInter] = soln1;
            nInter++;
        }
        if (Zmin <= Z2 && Z2 <= Zmax)
        {
            inter[nInter] = soln2;
            nInter++;
        }
        return nInter;
    }

    return 0;
}
