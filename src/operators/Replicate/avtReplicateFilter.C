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
//  File: avtReplicateFilter.C
// ************************************************************************* //

#include <avtReplicateFilter.h>

#include <float.h>

#include <vtkCellData.h>
#include <vtkMatrix4x4.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkCellArray.h>

#include <avtDataTree.h>
#include <avtDatasetExaminer.h>
#include <avtExtents.h>
#include <vtkPolyData.h>

#include <DebugStream.h>
#include <ImproperUseException.h>

#include <vtkAppendPolyData.h>

using   std::string;


// ****************************************************************************
//  Method: avtReplicateFilter constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
// ****************************************************************************

avtReplicateFilter::avtReplicateFilter()
{
}


// ****************************************************************************
//  Method: avtReplicateFilter destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Modifications:
//
// ****************************************************************************

avtReplicateFilter::~avtReplicateFilter()
{
}


// ****************************************************************************
//  Method:  avtReplicateFilter::Create
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
// ****************************************************************************

avtFilter *
avtReplicateFilter::Create()
{
    return new avtReplicateFilter();
}


// ****************************************************************************
//  Method:      avtReplicateFilter::SetAtts
//
//  Purpose:
//      Sets the state of the filter based on the attribute object.
//
//  Arguments:
//      a        The attributes to use.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
// ****************************************************************************

void
avtReplicateFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const ReplicateAttributes*)a;
}


// ****************************************************************************
//  Method: avtReplicateFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtReplicateFilter with the given
//      parameters would result in an equivalent avtReplicateFilter.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
// ****************************************************************************

bool
avtReplicateFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(ReplicateAttributes*)a);
}


// ****************************************************************************
//  Method: avtReplicateFilter::PostExecute
//
//  Purpose:
//      Cleans up after the execution.  This manages extents.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Modifications:
// ****************************************************************************

void
avtReplicateFilter::PostExecute(void)
{
    avtDataTreeStreamer::PostExecute();

    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.GetTrueSpatialExtents()->Clear();
    outAtts.GetEffectiveSpatialExtents()->Clear();
    outAtts.GetCurrentSpatialExtents()->Clear();

    double bounds[6];
    avtDataset_p ds = GetTypedOutput();
    avtDatasetExaminer::GetSpatialExtents(ds, bounds);
    outAtts.GetCumulativeTrueSpatialExtents()->Set(bounds);
}


// ****************************************************************************
//  Method: avtReplicateFilter::ModifyContract
//
//  Purpose:
//      Performs a restriction on the specification.  This is in place to
//      insure that we do static load balancing when extent calculation is
//      required.
//
//  Arguments:
//      spec    The current specification.
//
//  Returns:    A specification with the balancing potentially restricted.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Modifications:
// ****************************************************************************

avtContract_p
avtReplicateFilter::ModifyContract(avtContract_p spec)
{
    avtContract_p ns = new avtContract(spec);

    if (spec->GetDataRequest()->MayRequireZones())
    {
        ns->GetDataRequest()->TurnZoneNumbersOn();
    }
    if (spec->GetDataRequest()->MayRequireNodes())
    {
        ns->GetDataRequest()->TurnNodeNumbersOn();
    }

    return ns;
}

// ****************************************************************************
//  Method: avtReplicateFilter::UpdateDataObjectInfo
//
//  Purpose:
//     Specifies that this filter transforms the original dataset points. 
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Modifications:
//
// ****************************************************************************
 
void
avtReplicateFilter::UpdateDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().SetPointsWereTransformed(true);

    //
    // This filter invalidates any transform matrix in the pipeline.
    //
    GetOutput()->GetInfo().GetAttributes().SetCanUseInvTransform(false);
    GetOutput()->GetInfo().GetAttributes().SetCanUseTransform(false);

    GetOutput()->GetInfo().GetValidity().InvalidateSpatialMetaData();
}


// ****************************************************************************
//  Method: avtReplicateFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the Replicate filter.
//
//  Arguments:
//      in_ds      The input dataset.
//      dom        The domain number.
//      str        An identifying string.
//
//  Returns:       The output dataset.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Tue Feb 27 11:04:04 EST 2007
//    Enhanced to support transformed rectilinear grids.  In these cases
//    the x/y/z vectors also need to have the transform accounted for.
//
//    Jeremy Meredith, Thu Mar 22 13:39:05 EDT 2007
//    Added support for replicating the atoms lying on cell boundaries.
//    If we were to add this to the Molecule plot, two problems appear: first,
//    it's too late to create bonds using these atoms, and second, we have to
//    scan the entire dataset attempting to discover if the normalized extents
//    across which to replicate have changed from [0,1,0,1,0,1].  Since
//    creating bonds should already happen after the replicate operator, the
//    first problem is solved.  Since the new normalized extents are simply
//    the number of replications in this operator attributes (i.e. they are
//    [0,nx,0,ny,0,nz]), the second problem is solved.  At the moment
//    this new kind of replication is only allowed when we are merging into
//    a single dataset, but this was mostly for convenience and this
//    constraint can be relaxed with a straightforward loop over all datasets.
//
// ****************************************************************************
static void TransformVector(const double m[16], double v[3])
{
    double tmpa[4] = {v[0], v[1], v[2],  0.0}, tmpb[4];
    vtkMatrix4x4::MultiplyPoint(m, tmpa, tmpb);
    v[0] = tmpb[0];    v[1] = tmpb[1];    v[2] = tmpb[2];
}

avtDataTree_p 
avtReplicateFilter::ExecuteDataTree(vtkDataSet *in_ds, int dom, string str)
{
    int  i, j, k;

    if (in_ds == NULL)
    {
        return NULL;
    }

    //
    // Perform the replications
    //
    int xrep = atts.GetXReplications();
    int yrep = atts.GetYReplications();
    int zrep = atts.GetZReplications();

    int nrep = xrep * yrep * zrep;


    double xvec[3];
    double yvec[3];
    double zvec[3];

    const avtDataAttributes &datts = GetInput()->GetInfo().GetAttributes();
    const float *unitcell = datts.GetUnitCellVectors();

    for (i=0; i<3; i++)
    {
        if (atts.GetUseUnitCellVectors())
        {
            xvec[i] = unitcell[0*3 + i];
            yvec[i] = unitcell[1*3 + i];
            zvec[i] = unitcell[2*3 + i];
        }
        else
        {
            xvec[i] = atts.GetXVector()[i];
            yvec[i] = atts.GetYVector()[i];
            zvec[i] = atts.GetZVector()[i];
        }
    }

    if (datts.GetRectilinearGridHasTransform())
    {
        double inv[16];
        vtkMatrix4x4::Invert(datts.GetRectilinearGridTransform(), inv);

        TransformVector(inv, xvec);
        TransformVector(inv, yvec);
        TransformVector(inv, zvec);
    }

    vtkDataSet **replications = new vtkDataSet*[nrep];
    int ctr = 0;
    for (i = 0 ; i < xrep ; i++)
    {
        for (j = 0 ; j < yrep ; j++)
        {
            for (k = 0 ; k < zrep ; k++)
            {
                double offset[3];
                offset[0] = i*xvec[0] + j*yvec[0] + k*zvec[0];
                offset[1] = i*xvec[1] + j*yvec[1] + k*zvec[1];
                offset[2] = i*xvec[2] + j*yvec[2] + k*zvec[2];
                replications[ctr++] = Replicate(in_ds, offset);
            }
        }
    }

    //
    // Construct a data tree out of our replications.
    //
    avtDataTree_p rv;
    if (atts.GetMergeResults())
    {
        if (in_ds->GetDataObjectType() == VTK_POLY_DATA)
        {
            vtkAppendPolyData *append = vtkAppendPolyData::New();
            for (i=0; i<nrep; i++)
            {
                append->AddInput((vtkPolyData*)replications[i]);
            }
            vtkPolyData *output = append->GetOutput();
            output->Update();
            // If we're merging into a single data set, we can also
            // replicate periodically the unit cell boundary atoms
            if (atts.GetReplicateUnitCellAtoms())
            {
                vtkPolyData *newoutput = ReplicateUnitCellAtoms(output);
                if (newoutput)
                {
                    //output->Delete();
                    output = newoutput;
                }
            }
            rv = new avtDataTree(1, (vtkDataSet**)(&output), dom, str);
        }
        else
        {
            rv = new avtDataTree(nrep, replications, dom, str);
        }
    }
    else
    {
        rv = new avtDataTree(nrep, replications, dom, str);
    }

    //
    // Clean up memory.
    //
    for (i = 0 ; i < nrep ; i++)
    {
        if (replications[i] != NULL)
        {
            replications[i]->Delete();
        }
    }
    delete[] replications;

    return rv;
}


// ****************************************************************************
//  Method: avtReplicateFilter::Replicate
//
//  Purpose:
//      Determines what the mesh type is and calls the appropriate replicate
//      routine.
//
//  Arguments:
//      ds      The dataset to replicate.
//      dim     The dimension to replicate in.
//
//  Returns:    The dataset appropriately replicated.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Note: taken largely from avtReflectFilter.C
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtReplicateFilter::Replicate(vtkDataSet *ds, double offset[3])
{
    int dstype = ds->GetDataObjectType();
    vtkDataSet *rv = NULL;
    switch (dstype)
    {
      case VTK_RECTILINEAR_GRID:
        rv = ReplicateRectilinear((vtkRectilinearGrid *) ds, offset);
        break;

      case VTK_STRUCTURED_GRID:
      case VTK_UNSTRUCTURED_GRID:
      case VTK_POLY_DATA:
        rv = ReplicatePointSet((vtkPointSet *) ds, offset);
        break;

      default:
        debug1 << "Unknown data type for replicate filter = " << dstype << endl;
        break;
    }

    return rv;
}


// ****************************************************************************
//  Method: avtReplicateFilter::ReplicateRectilinear
//
//  Purpose:
//      Replicates a rectilinear mesh.
//
//  Arguments:
//      ds      The dataset to replicate.
//      dim     The dimension to replicate in.
//
//  Returns:    The dataset replicated across val.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Note: taken largely from avtReflectFilter.C
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtReplicateFilter::ReplicateRectilinear(vtkRectilinearGrid *ds, double offset[3])
{
    vtkRectilinearGrid *out = (vtkRectilinearGrid *) ds->NewInstance();
    int nPts = ds->GetNumberOfPoints();
    int dims[3];
    ds->GetDimensions(dims);
    out->GetFieldData()->ShallowCopy(ds->GetFieldData());
    out->SetDimensions(dims);
    out->GetPointData()->CopyAllocate(ds->GetPointData());
    out->GetCellData()->CopyAllocate(ds->GetCellData());

    //
    // Replicate across X if appropriate.
    //
    if (offset[0] != 0)
    {
        vtkDataArray *tmp = OffsetDataArray(ds->GetXCoordinates(), offset[0]);
        out->SetXCoordinates(tmp);
        tmp->Delete();
    }
    else
        out->SetXCoordinates(ds->GetXCoordinates());

    //
    // Replicate across Y if appropriate.
    //
    if (offset[1] != 0)
    {
        vtkDataArray *tmp = OffsetDataArray(ds->GetYCoordinates(), offset[1]);
        out->SetYCoordinates(tmp);
        tmp->Delete();
    }
    else
        out->SetYCoordinates(ds->GetYCoordinates());

    //
    // Replicate across Z if appropriate.
    //
    if (offset[2] != 0)
    {
        vtkDataArray *tmp = OffsetDataArray(ds->GetZCoordinates(), offset[2]);
        out->SetZCoordinates(tmp);
        tmp->Delete();
    }
    else
        out->SetZCoordinates(ds->GetZCoordinates());


    //
    // Copy over the point data.
    //
    int i, j, k;
    vtkPointData *inPD  = ds->GetPointData();
    vtkPointData *outPD = out->GetPointData();
    for (k = 0 ; k < dims[2] ; k++)
        for (j = 0 ; j < dims[1] ; j++)
            for (i = 0 ; i < dims[0] ; i++)
            {
                int idx = k*dims[1]*dims[0] + j*dims[0] + i;
                outPD->CopyData(inPD, idx, idx);
            }
    vtkCellData *inCD  = ds->GetCellData();
    vtkCellData *outCD = out->GetCellData();

    int iEnd = dims[0]-1;
    if (iEnd <= 0)
        iEnd = 1;
    int jEnd = dims[1]-1;
    if (jEnd <= 0)
        jEnd = 1;
    int kEnd = dims[2]-1;
    if (kEnd <= 0)
        kEnd = 1;
    for (k = 0 ; k < kEnd ; k++)
        for (j = 0 ; j < jEnd ; j++)
            for (i = 0 ; i < iEnd ; i++)
            {
                int idx = k*(dims[1]-1)*(dims[0]-1) + j*(dims[0]-1) + i;
                outCD->CopyData(inCD, idx, idx);
            }
    
    return out;
}


// ****************************************************************************
//  Method: avtReplicateFilter::ReplicateDataArray
//
//  Purpose:
//      Replicates a single data array across a line.  This is used as a
//      convenience function for replicating rectilinear grids.
//
//  Arguments:
//      coords  The coordinates to replicate.
//      val     The point to replicate about.
//
//  Returns:    The coordinates replicated; this must be deleted by the calling
//              object.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Note: taken largely from avtReflectFilter.C
//
//  Modifications:
//    Jeremy Meredith, Fri Feb 23 12:45:16 EST 2007
//    Had to account for single vs double in coord arrays.
//
// ****************************************************************************

vtkDataArray *
avtReplicateFilter::OffsetDataArray(vtkDataArray *coords, double val)
{
    //
    // Replicate the coordinates across the line of replication.
    //
    vtkDataArray *newcoords = coords->NewInstance();
    int nc = coords->GetNumberOfTuples();
    newcoords->SetNumberOfTuples(nc);

    if (coords->GetDataType() == VTK_FLOAT)
    {
        float *c = (float*)coords->GetVoidPointer(0);
        float *n = (float*)newcoords->GetVoidPointer(0);
        for (int i = 0 ; i < nc ; i++)
        {
            n[i] = c[i] + val;
        }
    }
    else if (coords->GetDataType() == VTK_DOUBLE)
    {
        double *c = (double*)coords->GetVoidPointer(0);
        double *n = (double*)newcoords->GetVoidPointer(0);
        for (int i = 0 ; i < nc ; i++)
        {
            n[i] = c[i] + val;
        }
    }

    return newcoords;
}


// ****************************************************************************
//  Method: avtReplicateFilter::ReplicatePointSet
//
//  Purpose:
//      Replicates a dataset that is derived from point set.
//
//  Arguments:
//      ds      The dataset to replicate.
//      dim     The dimension to replicate in.
//
//  Returns:    The dataset replicated across val.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Note: taken largely from avtReflectFilter.C
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtReplicateFilter::ReplicatePointSet(vtkPointSet *ds, double offset[3])
{
    vtkPointSet *out = (vtkPointSet *) ds->NewInstance();
    out->ShallowCopy(ds);

    vtkPoints *inPts  = ds->GetPoints();
    vtkPoints *outPts = vtkPoints::New();
    int nPts = inPts->GetNumberOfPoints();
    outPts->SetNumberOfPoints(nPts);

    for (int i = 0 ; i < nPts ; i++)
    {
        double pt[3];
        inPts->GetPoint(i, pt);
        pt[0] += offset[0];
        pt[1] += offset[1];
        pt[2] += offset[2];
        outPts->SetPoint(i, pt);
    }

    out->SetPoints(outPts);
    outPts->Delete();

    return out;
}

// ****************************************************************************
//  Method:  avtReplicateFilter::ReplicateUnitCellAtoms
//
//  Purpose:
//    For atoms lying along the unit cell boundaries, replicate them to the
//    far boundaries.  In normalized coordinates, for example, an atom lying
//    at (0,.5,.5), i.e.  at the center of the x-min face, should be
//    replicated to (1,.5,.5).  As another example, an atom at any of the unit
//    cell corners, e.g. (1,0,1), should be replicated to all of the other 7
//    corners.  And similarly, an atom lying along an edge should be
//    replicated to 3 new locations.  Note, however, that the min/max values
//    are not necessarily 0 and 1.  If we're replicating the whole data set by
//    a displacement of the unit cell to make a supercell, the max value along
//    any axis will be an integer corresponding to the number of replications
//    along that normalized axis.  By placing this new operation inside the
//    Replicate operator, however, we actually know exactly what these numbers
//    are as they are part of these operator attributes.
//
//  Arguments:
//    in         the polydata containing potential atoms (Verts) to replicate
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 22, 2007
//
// ****************************************************************************
static void
CalculateBoundaryConditions(double *normpt, float maxx, float maxy, float maxz,
                            bool &ib, bool &jb, bool &kb)
{
    const double threshold = 0.001;
    bool i0 = fabs(normpt[0])      < threshold;
    bool i1 = fabs(normpt[0]-maxx) < threshold;
    bool j0 = fabs(normpt[1])      < threshold;
    bool j1 = fabs(normpt[1]-maxy) < threshold;
    bool k0 = fabs(normpt[2])      < threshold;
    bool k1 = fabs(normpt[2]-maxz) < threshold;
    ib = i0 || i1;
    jb = j0 || j1;
    kb = k0 || k1;
}

vtkPolyData *
avtReplicateFilter::ReplicateUnitCellAtoms(vtkPolyData *in)
{
    // Make sure we have molecular data; if there are polygons
    // in here, some of our assumptions will be wrong.
    if (in->GetNumberOfCells() != in->GetNumberOfVerts() + in->GetNumberOfLines() ||
        in->GetNumberOfVerts() != in->GetNumberOfPoints())
    {
        return NULL;
    }

    // Get the unit cell vectors and construct transform matrices from them.
    const avtDataAttributes &datts = GetInput()->GetInfo().GetAttributes();
    const float *unitCell = datts.GetUnitCellVectors();
    vtkMatrix4x4 *ucvM = vtkMatrix4x4::New();
    vtkMatrix4x4 *ucvI = vtkMatrix4x4::New();
    ucvM->Identity();
    for (int i=0; i<3; i++)
        for (int j=0; j<3; j++)
            ucvM->Element[j][i] = unitCell[3*i+j];
    vtkMatrix4x4::Invert(ucvM, ucvI);

    // Get some info from the input dataset.
    vtkPointData *inPD   = in->GetPointData();
    vtkCellData  *inCD   = in->GetCellData();
    vtkPoints    *inPts  = in->GetPoints();
    vtkCellArray *inLines= in->GetLines();
    int nPts = in->GetNumberOfPoints();
    int nLines = in->GetNumberOfLines();

    // We need to make our first pass over the dataset to figure out how
    // many new points we will need.
    int newpts=0;
    float maxx = float(atts.GetXReplications());
    float maxy = float(atts.GetYReplications());
    float maxz = float(atts.GetZReplications());
    for (int c=0; c<nPts; c++)
    {
        double pt[4] = {0,0,0,1};
        double normpt[4] = {0,0,0,1};
        in->GetPoint(c, pt);
        ucvI->MultiplyPoint(pt, normpt);
        bool ib, jb, kb;
        CalculateBoundaryConditions(normpt, maxx,maxy,maxz, ib,jb,kb);
        // Every atom has at least one image: the original.
        // Each boundary we match means a doubling of this number of images.
        int images=1;
        if (ib) images*=2;
        if (jb) images*=2;
        if (kb) images*=2;
        newpts += images;
    }

    // Construct an output data set
    vtkPolyData  *out      = (vtkPolyData*)in->NewInstance();
    vtkPointData *outPD    = out->GetPointData();
    vtkCellData  *outCD    = out->GetCellData();
    vtkPoints    *outPts   = vtkPoints::New();
    vtkCellArray *outVerts = vtkCellArray::New();
    vtkCellArray *outLines = vtkCellArray::New();
    out->GetFieldData()->ShallowCopy(in->GetFieldData());
    out->SetPoints(outPts);
    out->SetVerts(outVerts);
    out->SetLines(outLines);
    outPts->Delete();
    outVerts->Delete();
    outLines->Delete();

    // Allocate space for the number of points, verts, and lines we'll have
    outPts->SetNumberOfPoints(newpts);
    outPD->CopyAllocate(in->GetPointData(),newpts);
    outCD->CopyAllocate(in->GetCellData(),newpts + nLines);

    // Copy the original points and verts
    int outc = 0;
    for (int c=0; c<nPts; c++)
    {
        outPts->SetPoint(outc, inPts->GetPoint(c));
        outPD->CopyData(inPD, c, outc);
        outVerts->InsertNextCell(1);
        outVerts->InsertCellPoint(outc);
        outCD->CopyData(inCD, c, outc);
        outc++;
    }
    // Make new points/verts for those that matched a boundary condition
    for (int c=0; c<nPts; c++)
    {
        double pt[4]     = {0,0,0,1};
        double normpt[4] = {0,0,0,1};
        in->GetPoint(c, pt);
        ucvI->MultiplyPoint(pt, normpt);
        bool ib, jb, kb;
        CalculateBoundaryConditions(normpt, maxx,maxy,maxz, ib,jb,kb);

        double normnewpt[4] = {0,0,0,1};
        double newpt[4]     = {0,0,0,1};
        for (int itest=0; itest<=1; itest++)
        {
            for (int jtest=0; jtest<=1; jtest++)
            {
                for (int ktest=0; ktest<=1; ktest++)
                {
                    // we already added the original image of this
                    // atom; don't do it again.
                    if (!itest && !jtest && !ktest)
                        continue;

                    // Apply the tests for this image, and add it if it passes
                    if ((!itest || ib) &&
                        (!jtest || jb) &&
                        (!ktest || kb))
                    {
                        normnewpt[0] = itest ? maxx-normpt[0] : normpt[0];
                        normnewpt[1] = jtest ? maxy-normpt[1] : normpt[1];
                        normnewpt[2] = ktest ? maxz-normpt[2] : normpt[2];
                        ucvM->MultiplyPoint(normnewpt, newpt);
                        outPts->SetPoint(outc, newpt);
                        outPD->CopyData(inPD, c, outc);
                        outVerts->InsertNextCell(1);
                        outVerts->InsertCellPoint(outc);
                        outCD->CopyData(inCD, c, outc);
                        outc++;
                    }
                }
            }
        }
    }
    // And copy the cells and cell data for the lines (bonds).
    outLines->DeepCopy(inLines);
    for (int l=0; l<nLines; l++)
    {
        // The "line" cells are indexed following the verts, thus the "npts+l".
        outCD->CopyData(inCD, nPts + l, outc);
        outc++;
    }

    // Free out temporary memory
    ucvM->Delete();
    ucvI->Delete();

    // And return the new data set.
    return out;    
}


// ****************************************************************************
//  Method:  avtReplicateFilter::FilterUnderstandsTransformedRectMesh
//
//  Purpose:
//    If this filter returns true, this means that it correctly deals
//    with rectilinear grids having an implied transform set in the
//    data attributes.  It can do this conditionally if desired.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 23, 2007
//
// ****************************************************************************

bool
avtReplicateFilter::FilterUnderstandsTransformedRectMesh()
{
    // This has been optimized to deal with these data sets
    return true;
}
