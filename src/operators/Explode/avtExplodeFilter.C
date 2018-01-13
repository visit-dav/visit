/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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

// ************************************************************************* //
//  File: avtExplodeFilter.C
// ************************************************************************* //

#include <avtExplodeFilter.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkCellIterator.h>
#include <vtkUnstructuredGrid.h>
#include <vtkIdList.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkCell.h>
#include <vtkFieldData.h>
#include <vtkDoubleArray.h>

#include <vtkIntArray.h>
#include <vtkCellArray.h>
#include <vtkDataSet.h>

#include <vtkVisItUtility.h>
#include <DebugStream.h>
#include <UnexpectedValueException.h>
#include <ImproperUseException.h>

#include <avtDatasetExaminer.h>
#include <avtDataTree.h>
#include <avtDatabaseMetaData.h>
#include <avtMaterialMetaData.h>

#define SCALE 0.2f


// ****************************************************************************
//  Method: Explosion constructor
//
//  Purpose: 
//      Initialize the explosion variables. 
//
//  Programmer: Alister Maguire
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
//  Modifications:
//
// ****************************************************************************

Explosion::Explosion()
{
    materialName         = "";
    explodeMaterialCells = false;
    explodeAllCells      = false;
    explosionPattern     = 0;
    matExplosionFactor   = 0.0;
    cellExplosionFactor  = 0.0;
    cylinderRadius       = 0.0;
    explosionPoint       = NULL;
    planePoint           = NULL;
    planeNorm            = NULL;
    cylinderPoint1       = NULL;
    cylinderPoint2       = NULL;
    for (int i = 0; i < 3; ++i)
    {
        displaceVec[i]    = 0.0;
    }
}


// ****************************************************************************
//  Method: Explosion::DisplaceMaterial
//
//  Purpose: 
//      Translate/Displace the spatial coordinates of an entire
//      dataset.     
//
//  Arguments:
//      ugrid    An unstructured grid to be displaced/translated. 
//
//  Programmer: Alister Maguire
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
//  Modifications:
//
// ****************************************************************************

void
Explosion::DisplaceMaterial(vtkUnstructuredGrid *ugrid)
{
    double bounds[6];
    double dataCenter[3];
    ugrid->GetBounds(bounds);
    for (int i = 0; i < 3; ++i)
        dataCenter[i] = (bounds[i*2] + bounds[(i*2)+1]) / 2.0;

    //
    // Calculate the explosion displacement. The method used
    // for displacement will depend and the child class's
    // implementation. 
    //
    CalcDisplacement(dataCenter, matExplosionFactor, false);

    int numPoints        = ugrid->GetNumberOfPoints();
    vtkPoints *ugridPts  = ugrid->GetPoints();
    vtkPoints *newPoints = vtkPoints::New();
    newPoints->DeepCopy(ugridPts);

    //
    // Do the actual displacement. 
    //
    for (int ptIdx = 0; ptIdx < numPoints; ++ptIdx)
    {
        double *point; 
        point = ugridPts->GetPoint(ptIdx);
        for (int j = 0; j < 3; ++j)
        {
            point[j] += displaceVec[j];
        }
        newPoints->SetPoint(ptIdx, point);
    }
    ugrid->SetPoints(newPoints);
    newPoints->Delete();
}


// ****************************************************************************
//  Method: Explosion::ExplodeAndDisplaceMaterial
//
//  Purpose: 
//      Displace an entire material and then explode the cells
//      within that material. 
//
//  Arguments:
//      ugrid    An unstructured grid to be displaced/translated. 
//
//  Programmer: Alister Maguire
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
//  Modifications:
//
// ****************************************************************************

void
Explosion::ExplodeAndDisplaceMaterial(vtkUnstructuredGrid *ugrid)
{
    double bounds[6];
    double dataCenter[3];
    ugrid->GetBounds(bounds);
    for (int i = 0; i < 3; ++i)
        dataCenter[i] = (bounds[i*2] + bounds[(i*2)+1]) / 2.0;

    vtkPoints *ugridPts  = ugrid->GetPoints();
    vtkPoints *newPoints = vtkPoints::New();
    vtkIdList *cellPts   = vtkIdList::New();
    newPoints->DeepCopy(ugridPts);

    //
    // First, we need to calculate a displacement of the
    // entire material and copy this into a container. 
    //
    double initialDisplacement[3];
    CalcDisplacement(dataCenter, matExplosionFactor, false);
    for (int i = 0; i < 3; ++i)
    {
        initialDisplacement[i] = displaceVec[i];
    }

    bool normalize;
    switch (explosionPattern)
    {
        case ExplodeAttributes::Impact:
        {
            normalize = true;
        }
        break;
        case ExplodeAttributes::Scatter:
        {
            normalize = false;
        }
        break;
    }
    
    //
    // Iterate over all cells and displace them by both material
    // and cell displacements. Since we are displacing cells, we 
    // need to disconect the dataset as well.  
    //
    vtkCellIterator *it = ugrid->NewCellIterator();
    for (it->InitTraversal(); !it->IsDoneWithTraversal(); it->GoToNextCell())
    {
        vtkIdType  cellId = it->GetCellId();
        ugrid->GetCellPoints(cellId, cellPts);
        vtkCell *cell     = ugrid->GetCell(cellId);

        //
        // Get the cell's center for calculating displacement. 
        //
        int    subId;
        double cellCenter[3];
        double weights[8];
        double pCenter[3];
        cell->GetParametricCenter(pCenter);
        cell->EvaluateLocation(subId, pCenter, cellCenter, weights);

        //
        // Calculate the displacement for this cell. 
        //
        CalcDisplacement(cellCenter, cellExplosionFactor, normalize); 

        //
        // Create a new set of points that are identical to the old
        // but displaced. 
        //
        int numIds = cellPts->GetNumberOfIds();
        for (int i = 0; i < numIds; ++i)
        {
            double point[3];
            ugrid->GetPoint(cellPts->GetId(i), point);
            for (int j = 0; j < 3; ++j)
            {
               point[j] += displaceVec[j] + initialDisplacement[j];
            }
            newPoints->SetPoint(cellPts->GetId(i), point);
        }
        cellPts->Reset(); 
    }

    ugrid->SetPoints(newPoints);
    newPoints->Delete();
    cellPts->Delete();
    it->Delete();
}



// ****************************************************************************
//  Method: Explosion::ExplodeAllCells
//
//  Purpose:
//      Explode all of the cells within a dataset. 
//
//  Arguments:
//      ugrid      The output dataset. 
//
//  Programmer: Alister Maguire
//  Creation:   Tue Nov 21 11:19:55 PST 2017
//
//  Modifications:
//
// ****************************************************************************

void
Explosion::ExplodeAllCells(vtkDataSet *in_ds,
                           vtkUnstructuredGrid *out_grid)
{
    //
    // Get the data ready for transfer
    //  
    int nCells               = in_ds->GetNumberOfCells();
    vtkPointData *inPD       = in_ds->GetPointData();
    vtkCellData  *inCD       = in_ds->GetCellData();
    vtkPointData *outPD      = out_grid->GetPointData();
    vtkCellData  *outCD      = out_grid->GetCellData();

    out_grid->Allocate(nCells);
    outCD->CopyAllocate(inCD);
    outPD->CopyAllocate(inPD);
    
    int numCells        = in_ds->GetNumberOfCells();
    vtkCellIterator *it = in_ds->NewCellIterator();
    vtkPoints *pts      = vtkPoints::New();
    vtkIdList *cellPts  = vtkIdList::New();
    vtkIdList *ptIds    = vtkIdList::New();

    // 
    // Since we are removing connectivity between nodes, 
    // we will need more space to store them. The following
    // is a rough (usually over) estimation which imagines
    // that each cell could be a  VTK VOXEL or HEXAHEDRON 
    // type (8 nodes). We squeeze out the extra space later. 
    // NOTE: VTK has less common cell types that can have 
    // as many as 19 nodes, in which case this could actually 
    // be an under-estimation...
    //
    outPD->Allocate(numCells * 8);
    pts->Allocate(numCells * 8);
    ptIds->Allocate(numCells * 8);
    cellPts->Allocate(numCells);

    bool normalize = true;
    switch (explosionPattern)
    {
        case ExplodeAttributes::Impact:
        {
            normalize = true;
        }
        break;
        case ExplodeAttributes::Scatter:
        {
            normalize = false;
        }
        break;
    }

    //
    // Iterate over cells, displace their position based 
    // on an explosion, and insert these new cells into 
    // our out_grid. 
    //
    for (it->InitTraversal(); !it->IsDoneWithTraversal(); it->GoToNextCell())
    {
        int cellType      = it->GetCellType();
        vtkIdType  cellId = it->GetCellId();
        in_ds->GetCellPoints(cellId, cellPts);
        vtkCell *cell     = in_ds->GetCell(cellId);

        //
        // Get the cell's center for calculating displacement. 
        //
        int    subId;
        double cellCenter[3];
        double weights[8];
        double pCenter[3];
        cell->GetParametricCenter(pCenter);
        cell->EvaluateLocation(subId, pCenter, cellCenter, weights);
        
        //
        // Calculate the displacement for this cell. 
        //
        CalcDisplacement(cellCenter, cellExplosionFactor, normalize); 

        //
        // Create a new set of points that are identical to the old
        // but displaced. 
        //
        int numIds = cellPts->GetNumberOfIds();
        for (int i = 0; i < numIds; ++i)
        {
            double point[3];
            in_ds->GetPoint(cellPts->GetId(i), point);
            for (int j = 0; j < 3; ++j)
            {
               point[j] += displaceVec[j];
            }
            int nxtPtId = pts->InsertNextPoint(point);
            ptIds->InsertNextId(nxtPtId);
            outPD->CopyData(inPD, cellPts->GetId(i), nxtPtId);
        }

        int newId = out_grid->InsertNextCell(cellType, ptIds);
        outCD->CopyData(inCD, cellId, newId);
       
        cellPts->Reset(); 
        ptIds->Reset();
    }

    out_grid->SetPoints(pts); 
 
    //
    // Reclaim unused space.
    //
    outPD->Squeeze();

    //
    // Clean-up memory.
    //
    pts->Delete();
    cellPts->Delete();
    ptIds->Delete();
    it->Delete();
}


// ****************************************************************************
//  Method: PointExplosion constructor
//
//  Purpose:
//      Empty constructor. 
//
//  Programmer: maguire7 -- generated by xml2avt
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
//  Modifications:
//
// ****************************************************************************

PointExplosion::PointExplosion() 
{}


// ****************************************************************************
//  Method: PointExplosion::CalcDisplacement
//
//  Purpose:
//      Calculate the displacement for a particular piece of data 
//      (this could be a chunk of data representing a material
//      or an individual cell). This displacement will be calculated
//      by exploding from a given point in space. 
//
//  Arguments:
//      dataCenter    The x,y,z coords of the center of the data
//                    to be displaced. 
//
//      factor        A factor to displace the data by. 
//
//      normalize     Should we normalize the displacement vector?
//                    Yes produces 'impact' explosion, and no produces
//                    'scatter' explosion. 
//
//  Programmer: maguire7 -- generated by xml2avt
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
//  Modifications:
//
// ****************************************************************************

void
PointExplosion::CalcDisplacement(double *dataCenter, double factor, 
                                 bool normalize)
{
    //
    //  Find the distance between the data center and the
    //  explosion point. This becomes the basis for our
    //  displacement. 
    //
    for (int i = 0; i < 3; ++i)
    {
        displaceVec[i] = dataCenter[i] - explosionPoint[i];
    }

    double mag = sqrt(displaceVec[0]*displaceVec[0]+
                      displaceVec[1]*displaceVec[1]+
                      displaceVec[2]*displaceVec[2]);

    if (normalize)
    {
        if (mag == 0.0)
        {
            mag = 1.0;
        }

        factor /= mag;
        for (int i = 0; i < 3; ++i)
        {
            displaceVec[i] *= factor;
        }
    }
    else
    {
        factor *= SCALE * mag;
        for (int i = 0; i < 3; ++i)
        {
            displaceVec[i] *= factor;
        }
    }
}


// ****************************************************************************
//  Method: PlaneExplosion constructor
//
//  Purpose:
//      Empty constructor. 
//
//  Programmer: maguire7 -- generated by xml2avt
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
//  Modifications:
//
// ****************************************************************************

PlaneExplosion::PlaneExplosion() {}


// ****************************************************************************
//  Method: PlaneExplosion::CalcDisplacement
//
//  Purpose:
//      Calculate the displacement for a particular piece of data 
//      (this could be a chunk of data representing a material
//      or an individual cell). This displacement will be calculated
//      by a given plane in space. 
//
//  Arguments:
//      dataCenter    The x,y,z coords of the center of the data
//                    to be displaced. 
//
//      factor        A factor to displace the data by. 
//
//      normalize     Should we normalize the displacement vector?
//                    Yes produces 'impact' explosion, and no produces
//                    'scatter' explosion. 
//
//  Programmer: maguire7 -- generated by xml2avt
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
//  Modifications:
//
// ****************************************************************************

void
PlaneExplosion::CalcDisplacement(double *dataCenter, double factor, 
                                 bool normalize)
{
    //
    // Project from our data center onto a plane. 
    //
    double denom = 0.0;
    double alpha = 0.0;
    for (int i = 0; i < 3; ++i)
    {
        alpha += (planeNorm[i] * planePoint[i]) - (planeNorm[i] * dataCenter[i]);
        denom += planeNorm[i] * planeNorm[i];
    }

    alpha /= denom;

    //
    // Subtract the projection from the cell center
    // to get the distance from the plane. 
    //
    for (int i = 0; i < 3; ++i)
        displaceVec[i] = dataCenter[i] -(dataCenter[i] + (alpha * planeNorm[i]));


    double mag = sqrt(displaceVec[0]*displaceVec[0] +
                      displaceVec[1]*displaceVec[1] +
                      displaceVec[2]*displaceVec[2]);

    if (normalize)
    {
        if (mag == 0.0)
        {
            mag = 1.0;
        }

        factor /= mag;

        for (int i = 0; i < 3; ++i)
        {
            displaceVec[i] *= factor;
        }
    }
    else
    {
        factor *= SCALE * mag;
        for (int i = 0; i < 3; ++i)
        {
            displaceVec[i] *= factor;
        }
    }
}


// ****************************************************************************
//  Method: CylinderExplosion constructor
//
//  Programmer: maguire7 -- generated by xml2avt
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
// ****************************************************************************

CylinderExplosion::CylinderExplosion() {}


// ****************************************************************************
//  Method: CylinderExplosion::CalcDisplacement
//
//  Purpose:
//      Calculate the displacement for a particular piece of data 
//      (this could be a chunk of data representing a material
//      or an individual cell). This displacement will be calculated
//      by a given cylinder in space. If the radius of the cylinder 
//      is 0, then we explode from a line. If the radius > 0, then 
//      anything in the radius is unaffected by the explosion. 
//
//  Arguments:
//      dataCenter    The x,y,z coords of the center of the data
//                    to be displaced. 
//
//      factor        A factor to displace the data by. 
//
//      normalize     Should we normalize the displacement vector?
//                    Yes produces 'impact' explosion, and no produces
//                    'scatter' explosion. 
//
//  Programmer: maguire7 -- generated by xml2avt
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
//  Modifications:
//
// ****************************************************************************

void
CylinderExplosion::CalcDisplacement(double *dataCenter, double factor, 
                                    bool normalize)
{
    //
    // Project from our data center to the line which runs 
    // through the center of our cylinder. 
    //
    double AP[3];
    double AB[3];
    for (int i = 0; i < 3; ++i)
    {
        AP[i] = dataCenter[i] - cylinderPoint1[i];
        AB[i] = cylinderPoint2[i] - cylinderPoint1[i];
    }

    double dot1 = 0.0;
    double dot2 = 0.0;
    for (int i = 0; i < 3; ++i)
    {
       dot1 += AP[i] * AB[i];
       dot2 += AB[i] * AB[i];
    }
    
    double projection[3];
    for (int i = 0; i < 3; ++i)
        projection[i] = cylinderPoint1[i] + ((dot1 / dot2) * AB[i]);

    //
    // If the data is within the cylinder's radius, we don't want to 
    // displace it, so just set the displacement to 0.0 and return. 
    //
    double dist = 0.0;
    for (int i = 0; i < 3; ++i)
        dist += (dataCenter[i] - projection[i]) * (dataCenter[i] - projection[i]);
    if (dist <= cylinderRadius)
    {
        for (int i = 0; i < 3; ++i)
            displaceVec[i] = 0.0;
        return;
    }

    //
    // Find the distance between our data center and the
    // projection onto the line. 
    //
    for (int i = 0; i < 3; ++i)
    {
        displaceVec[i] = dataCenter[i] - projection[i];
    }

    double mag = sqrt(displaceVec[0]*displaceVec[0] +
                      displaceVec[1]*displaceVec[1] +
                      displaceVec[2]*displaceVec[2]);

    if (normalize)
    {
        if (mag == 0.0)
        {
            mag = 1.0;
        }

        factor /= mag;
        for (int i = 0; i < 3; ++i)
        {
            displaceVec[i] *= factor;
        }
    }
    else
    {
        factor *= SCALE * mag;
        for (int i = 0; i < 3; ++i)
        {
            displaceVec[i] *= factor;
        }
    }
}


// ****************************************************************************
//  Method: avtExplodeFilter constructor
//
//  Purpose:
//      Empty constructor.
//
//  Programmer: maguire7 -- generated by xml2avt
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
//  Modifications:
//
// ****************************************************************************

avtExplodeFilter::avtExplodeFilter()
{}


// ****************************************************************************
//  Method: avtExplodeFilter destructor
//
//  Purpose:
//      Handle any needed memory clean-up. 
//
//  Programmer: maguire7 -- generated by xml2avt
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
//  Modifications:
//
// ****************************************************************************

avtExplodeFilter::~avtExplodeFilter()
{
    if (explosion != NULL)
    {
        delete explosion;
    }
}


// ****************************************************************************
//  Method:  avtExplodeFilter::Create
//
//  Programmer: maguire7 -- generated by xml2avt
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
// ****************************************************************************

avtFilter *
avtExplodeFilter::Create()
{
    return new avtExplodeFilter();
}


// ****************************************************************************
//  Method:      avtExplodeFilter::SetAtts
//
//  Purpose:
//      Sets the state of the filter based on the attribute object.
//
//  Arguments:
//      a        The attributes to use.
//
//  Programmer: maguire7 -- generated by xml2avt
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
// ****************************************************************************

void
avtExplodeFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const ExplodeAttributes*)a;
}


// ****************************************************************************
//  Method: avtExplodeFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtExplodeFilter with the given
//      parameters would result in an equivalent avtExplodeFilter.
//
//  Programmer: maguire7 -- generated by xml2avt
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
// ****************************************************************************

bool
avtExplodeFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(ExplodeAttributes*)a);
}


// ****************************************************************************
//  Method: avtExplodeFilter::GetMaterialSubsets
//
//  Purpose:
//      Create a dataset for every material within our input data, 
//      and create a tree whose leaves are material datasets. 
//
//  Arguments:
//      in_dr    The input data representation. 
//
//  Returns:
//      An avtDataTree whose leaves are material datasets. 
//
//  Programmer:  Alister Maguire
//  Creation:    Thu Dec 21 14:05:14 PST 2017 
//
//  Note:  Much of this was taken from the boundary plot filter. The 
//         output conversion has been changed from polydata to 
//         unstructured grid. The resulting meshes are fully disconnected. 
//
//  Modifications:
//
// ****************************************************************************

avtDataTree_p
avtExplodeFilter::GetMaterialSubsets(avtDataRepresentation *in_dr)
{
    //
    // Get the VTK data set, the domain number, and the label.
    //
    vtkDataSet *in_ds = in_dr->GetDataVTK();
    int domain = in_dr->GetDomain();
    std::string label = in_dr->GetLabel();

    if (in_ds                      == NULL || 
        in_ds->GetNumberOfPoints() == 0    ||
        in_ds->GetNumberOfCells()  == 0)
    {
        return NULL;
    }

    stringVector   labels;
    int            nDataSets     = 0;
    vtkDataSet   **out_ds        = NULL;
    vtkDataArray  *boundaryArray = in_ds->GetCellData()->GetArray("avtSubsets");

    //
    // If we have a boundary array, then we have materials to 
    // work with. 
    //
    if (boundaryArray)
    {
        if (label.find(";") == std::string::npos)
        {
            debug1 << "POSSIBLE ERROR CONDITION:  " << endl;
            debug1 << "    avtExplodeFilter encountered a label ("
                   << label.c_str() << ")" << endl;
            debug1 << "    that cannot be parsed correctly.  This can happen "
                   << "if" << endl;
            debug1 << "    another filter has over-written the boundary labels "
                   << "in" << endl;
            debug1 << "    its output data tree.  avtExplodeFilter is returning"
                   << endl;
            debug1 << "    an empty data tree." << endl;
            avtDataTree_p rv = new avtDataTree();
            return rv;
        }

        //
        // Break up the dataset into a collection of datasets, one
        // per boundary.
        //
        int *boundaryList = ((vtkIntArray*)boundaryArray)->GetPointer(0);
        
        vtkUnstructuredGrid *in_ug = vtkUnstructuredGrid::New();
        in_ug->DeepCopy(in_ds);

        //
        // Get the data ready for transfer
        //
        int nCells          = in_ug->GetNumberOfCells();
        vtkPointData *inPD  = in_ug->GetPointData();
        vtkCellData  *inCD  = in_ug->GetCellData();

        int numCells        = in_ug->GetNumberOfCells();
        vtkPoints *pts      = vtkPoints::New();
        vtkIdList *cellPts  = vtkIdList::New();
        vtkIdList *ptIds    = vtkIdList::New();

        // 
        // Since we are removing connectivity between nodes, 
        // we will need more space to store them. The following
        // is a rough (usually over) estimation which imagines
        // that each cell could be a  VTK VOXEL or HEXAHEDRON 
        // type (8 nodes). We squeeze out the extra space later. 
        // NOTE: VTK has less common cell types that can have 
        // as many as 19 nodes, in which case this could actually 
        // be an under-estimation...
        //
        pts->Allocate(numCells * 8);
        ptIds->Allocate(numCells * 8);
        cellPts->Allocate(numCells);

        int ntotalcells = in_ug->GetNumberOfCells();

        //
        // Determine the total number of boundarys
        // and the labels for the boundarys.
        //
        char *cLabelStorage = new char[label.length()+1];
        strcpy(cLabelStorage, label.c_str());
        char *cLabel = cLabelStorage;

        int nSelectedBoundaries = 0;
        sscanf(cLabel, "%d", &nSelectedBoundaries);
        cLabel = strchr(cLabel, ';') + 1;

        int i, *selectedBoundaries = new int[nSelectedBoundaries];
        char **selectedBoundaryNames = new char*[nSelectedBoundaries];
        for (i = 0; i < nSelectedBoundaries; i++)
        {
            sscanf(cLabel, "%d", &selectedBoundaries[i]);
            cLabel = strchr(cLabel, ';') + 1;
            selectedBoundaryNames[i] = cLabel;
            cLabel = strchr(cLabel, ';');
            cLabel[0] = '\0';
            cLabel = cLabel + 1;
        }

        int maxBoundary = selectedBoundaries[0];
        for (i = 1; i < nSelectedBoundaries; i++)
        {
            maxBoundary = selectedBoundaries[i] > maxBoundary ?
                        selectedBoundaries[i] : maxBoundary;
        }

        //
        // Count the number of cells of each boundary.
        //
        int *boundaryCounts = new int[maxBoundary+1];
        for (int s = 0; s < maxBoundary + 1; s++)
        {
            boundaryCounts[s] = 0;
        }
        for (i = 0; i < ntotalcells; i++)
        {
            boundaryCounts[boundaryList[i]]++;
        }

        //
        // Create a dataset for each boundary.
        //
        out_ds = new vtkDataSet *[nSelectedBoundaries];

        //
        // The following call is a workaround for a VTK bug.  It turns
        // out that when GetCellType if called for the first time for a
        // PolyData it calls its BuildCells method which causes the iterator
        // used by InitTraversal and GetNextCell to be put at the end of
        // the list.
        //
        in_ug->GetCellType(0);

        //
        // For each boundary, create a new unstructured grid that
        // is fully disconnected.
        //
        for (i = 0; i < nSelectedBoundaries; i++)
        {
            int s = selectedBoundaries[i];

            if (boundaryCounts[s] > 0)
            {
                //
                // Create a new unstructured grid
                //
                vtkUnstructuredGrid *out_ug = vtkUnstructuredGrid::New();
                vtkPointData *outPD = out_ug->GetPointData();
                vtkCellData  *outCD = out_ug->GetCellData();
                vtkCellIterator *it = in_ug->NewCellIterator();

                out_ug->Allocate(nCells);
                outCD->CopyAllocate(inCD);
                outPD->CopyAllocate(inPD);
                outPD->Allocate(numCells * 8);

                for (it->InitTraversal(); !it->IsDoneWithTraversal(); it->GoToNextCell())
                {
                 
                    vtkIdType cellId = it->GetCellId();
                    if (boundaryList[cellId] == s)
                    {
                        int cellType      = it->GetCellType();
                        in_ug->GetCellPoints(cellId, cellPts);
                        int numIds        = cellPts->GetNumberOfIds();

                        for (int i = 0; i < numIds; ++i)
                        {
                            double point[3];
                            in_ug->GetPoint(cellPts->GetId(i), point);
                            int nxtPtId = pts->InsertNextPoint(point);
                            ptIds->InsertNextId(nxtPtId);
                            outPD->CopyData(inPD, cellPts->GetId(i), nxtPtId);
                        }
                        int newId = out_ug->InsertNextCell(cellType, ptIds);
                        outCD->CopyData(inCD, cellId, newId);
                        cellPts->Reset(); 
                        ptIds->Reset();
                    }
                }
        
                //
                // Reclaim unused space.
                //
                outPD->Squeeze();

                //
                // Remove the avtSubsets array and set the label
                // for this dataset to be the material label. 
                //
                out_ug->SetPoints(pts); 
                outCD->RemoveArray("avtSubsets");
                labels.push_back(selectedBoundaryNames[i]);
                out_ds[nDataSets] = out_ug;
                nDataSets++;
            }
        }

        delete [] boundaryCounts;
        delete [] selectedBoundaryNames;
        delete [] selectedBoundaries;
        delete [] cLabelStorage;

        in_ug->Delete();
    }
    else
    {
        //
        // The dataset represents a single boundary, so just turn it into
        // a data tree.
        //
        labels.push_back(label);
        out_ds = new vtkDataSet *[1];
        out_ds[0] = in_ds;
        out_ds[0]->Register(NULL);  // This makes it symmetric with the 'if'
                                    // case so we can delete it blindly later.

        nDataSets = 1;
    }

    if (nDataSets == 0)
    {
        delete [] out_ds;

        return NULL;
    }

    avtDataTree_p outDT = new avtDataTree(nDataSets, out_ds, domain, labels);

    for (int i = 0 ; i < nDataSets ; i++)
    {
        if (out_ds[i] != NULL)
        {
            out_ds[i]->Delete();
        }
    }
    delete [] out_ds;

    return outDT;
}


// ****************************************************************************
//  Method: avtExplodeFilter::PreExecute
//
//  Purpose:
//      Create an Explosion that will contain all the information
//      needed to perform our actual explosion. 
//
//  Programmer: Aliseter Maguire
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
//  Modifications:
//
// ****************************************************************************

void
avtExplodeFilter::PreExecute(void)
{
    //
    // Create the explosion object 
    //
    switch (atts.GetExplosionType())
    {
        case ExplodeAttributes::Point:
        {
            explosion                  = new PointExplosion();
            explosion->explosionPoint  = atts.GetExplosionPoint();
        }
        break;
        case ExplodeAttributes::Plane:
        {
            explosion                  = new PlaneExplosion();
            explosion->planePoint      = atts.GetPlanePoint(); 
            explosion->planeNorm       = atts.GetPlaneNorm();
        }
        break;
        case ExplodeAttributes::Cylinder:
        {
            explosion                  = new CylinderExplosion();
            explosion->cylinderPoint1  = atts.GetCylinderPoint1(); 
            explosion->cylinderPoint2  = atts.GetCylinderPoint2();
            explosion->cylinderRadius  = atts.GetCylinderRadius(); 
        }
        break;
    }

    explosion->materialName         = atts.GetMaterial();
    explosion->matExplosionFactor   = atts.GetMaterialExplosionFactor();
    explosion->explosionPattern     = atts.GetExplosionPattern();

    if (atts.GetExplodeMaterialCells() && !atts.GetExplodeAllCells())
    {
        explosion->explodeMaterialCells = true;
        explosion->matExplosionFactor   = atts.GetMaterialExplosionFactor();
        explosion->cellExplosionFactor  = atts.GetCellExplosionFactor();
    }
    else if (atts.GetExplodeAllCells())
    {
        explosion->cellExplosionFactor = atts.GetCellExplosionFactor();
    }
}


// ****************************************************************************
//  Method: avtExplodeFilter::ExecuteDataTree
//
//  Purpose:
//      Explode a dataset based on the given attributes. 
//
//  Arguments:
//      in_dr      The input data representation.
//
//  Returns:       An output data tree. 
//
//  Programmer: maguire7 -- generated by xml2avt
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
//  Modifications:
//
// ****************************************************************************

avtDataTree_p
avtExplodeFilter::ExecuteDataTree(avtDataRepresentation *in_dr)
{
    //
    // Get the VTK data set, the domain number, and the label.
    //
    vtkDataSet *in_ds = in_dr->GetDataVTK();
    int domain        = in_dr->GetDomain();
    std::string label = in_dr->GetLabel();

    //
    // If we are exploding all cells, we don't need to 
    // worry about materials. Also, if we don't have 
    // any materials, cell explosion is the only option. 
    //
    if (atts.GetExplodeAllCells() || label.empty()) 
    {
        vtkUnstructuredGrid *out_grid = vtkUnstructuredGrid::New();

        //
        // Do the actual explosion. 
        //
        explosion->ExplodeAllCells(in_ds, out_grid);

        stringVector labels;
        labels.push_back(label);

        vtkDataSet **ugrids   = new vtkDataSet*[1];
        ugrids[0]             = out_grid;
        avtDataTree_p outTree = new avtDataTree(1, ugrids, domain, labels);

        if (ugrids[0] != NULL)
        {
            ugrids[0]->Delete();
        }
        delete [] ugrids;

        return outTree;
    }

    //
    // Create a data tree where each leaf is a 
    // material dataset. 
    //
    avtDataTree_p materialTree = GetMaterialSubsets(in_dr);
   
    if (*materialTree == NULL)
    {
        debug1 << "GetMaterialSubsets returned a NULL materialTree..." << endl;
        return materialTree;
    }

    int nLeaves;
    stringVector labels; 
    materialTree->GetAllLabels(labels);
    vtkDataSet **dsets = materialTree->GetAllLeaves(nLeaves);

    if (nLeaves == 0)
    {
        delete [] dsets;
        return materialTree; 
    }
    
    if (static_cast<int>(labels.size()) < nLeaves)
    {
        char expected[256];
        char recieved[256];
        sprintf(expected, "Expected number of labels to be >= "
            "number of leaves");
        sprintf(recieved, "Num labels: %d  Num leaves: %d  ", 
            static_cast<int>(labels.size()), nLeaves);
        EXCEPTION2(UnexpectedValueException, expected, recieved);
        return materialTree;
    }
    
    vtkDataSet **ugrids = new vtkDataSet*[nLeaves];

    //
    // Look for the material that has been selected for
    // explosion, and blow it up. 
    //
    for (int i = 0; i < nLeaves; ++i)
    {
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
        ugrid->DeepCopy(dsets[i]);

        if ( explosion->materialName == labels[i] )
        {
            if ( explosion->explodeMaterialCells )
            {
                explosion->ExplodeAndDisplaceMaterial(ugrid);
            }
            else
            {
                explosion->DisplaceMaterial(ugrid);
            }
        }

        ugrids[i] = ugrid;
    }
    
    avtDataTree_p outTree = new avtDataTree(nLeaves, ugrids, domain, labels);

    //
    // Clean up memory.
    //
    for (int i = 0; i < nLeaves; ++i)
    {
        if (ugrids[i] != NULL)
            ugrids[i]->Delete();
    }
    delete [] ugrids;
    
    return outTree;
}


// ****************************************************************************
//  Method: avtExplodeFilter::PostExecute
//
//  Purpose:
//      Update attributes after execution. 
//
//  Programmer: Alister Maguire
//  Creation:   Wed Nov  8 10:12:34 PST 2017
//
//  Modifications:
//
// ****************************************************************************

void
avtExplodeFilter::PostExecute(void)
{
    // Spatial extents could have all changed, so we 
    // need to clear them. 
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.GetOriginalSpatialExtents()->Clear();
    outAtts.GetDesiredSpatialExtents()->Clear();
    outAtts.GetActualSpatialExtents()->Clear();
    
    // Update the spatial extents. 
    double bounds[6];
    avtDataset_p ds = GetTypedOutput();
    avtDatasetExaminer::GetSpatialExtents(ds, bounds);
    outAtts.GetThisProcsOriginalSpatialExtents()->Set(bounds);

    stringVector treeLabels;
    GetDataTree()->GetAllUniqueLabels(treeLabels);
    GetOutput()->GetInfo().GetAttributes().SetLabels(treeLabels);
}


// ****************************************************************************
//  Method: avtExplodeFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Update the information about our output. 
//
//  Programmer: Alister Maguire
//  Creation:   Wed Nov  8 10:12:34 PST 2017
// 
//  Modifications:
//
// ****************************************************************************

void
avtExplodeFilter::UpdateDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateDataMetaData();
    GetOutput()->GetInfo().GetValidity().InvalidateSpatialMetaData();

    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.SetLabels(atts.GetBoundaryNames());
    outAtts.AddFilterMetaData("Explode");
}


// ****************************************************************************
//  Method: avtExplodeFilter::ModifyContract
//
//  Purpose:
//      Update the contract. In particular, we need to tell VisIt 
//      to construct materials and material labels. 
//
//  Programmer: Alister Maguire
//  Creation:   Wed Nov  8 10:12:34 PST 2017
//
// ****************************************************************************

avtContract_p   
avtExplodeFilter::ModifyContract(avtContract_p contract)
{
    avtContract_p rv = new avtContract(contract);

    rv->GetDataRequest()->ForceMaterialInterfaceReconstructionOn();
    rv->GetDataRequest()->ForceMaterialLabelsConstructionOn();
    rv->SetCalculateMeshExtents(true); 

    if (contract->GetDataRequest()->MayRequireZones())
    {
        rv->GetDataRequest()->TurnZoneNumbersOn();
    }
    if (contract->GetDataRequest()->MayRequireNodes())
    {
        rv->GetDataRequest()->TurnNodeNumbersOn();
    }
    
    return rv;
}
