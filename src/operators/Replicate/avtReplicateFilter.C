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
//  File: avtReplicateFilter.C
// ************************************************************************* //

#include <avtReplicateFilter.h>

#include <float.h>

#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnsignedCharArray.h>

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
//  Method: avtReplicateFilter::PerformRestriction
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

avtPipelineSpecification_p
avtReplicateFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    avtPipelineSpecification_p ns = new avtPipelineSpecification(spec);

    if (spec->GetDataSpecification()->MayRequireZones())
    {
        ns->GetDataSpecification()->TurnZoneNumbersOn();
    }
    if (spec->GetDataSpecification()->MayRequireNodes())
    {
        ns->GetDataSpecification()->TurnNodeNumbersOn();
    }

    return ns;
}

// ****************************************************************************
//  Method: avtReplicateFilter::RefashionDataObjectInfo
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
avtReplicateFilter::RefashionDataObjectInfo(void)
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
// ****************************************************************************

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

    const float *unitcell = GetInput()->GetInfo().GetAttributes().
                                                         GetUnitCellVectors();

    for (int i=0; i<3; i++)
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
//      convenience function for replicateing rectilinear grids.
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
    double *c = coords->GetTuple(0);
    double *n = newcoords->GetTuple(0);
    for (int i = 0 ; i < nc ; i++)
    {
        n[i] = c[i] + val;
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

