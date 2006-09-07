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
//  File: avtCreateBondsFilter.C
// ************************************************************************* //

#include <avtCreateBondsFilter.h>

#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkCellData.h>
#include <vtkCellArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <DebugStream.h>

// ****************************************************************************
//  Method: avtCreateBondsFilter constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
// ****************************************************************************

avtCreateBondsFilter::avtCreateBondsFilter()
{
}


// ****************************************************************************
//  Method: avtCreateBondsFilter destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Modifications:
//
// ****************************************************************************

avtCreateBondsFilter::~avtCreateBondsFilter()
{
}


// ****************************************************************************
//  Method:  avtCreateBondsFilter::Create
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
// ****************************************************************************

avtFilter *
avtCreateBondsFilter::Create()
{
    return new avtCreateBondsFilter();
}


// ****************************************************************************
//  Method:      avtCreateBondsFilter::SetAtts
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
avtCreateBondsFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const CreateBondsAttributes*)a;
}


// ****************************************************************************
//  Method: avtCreateBondsFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtCreateBondsFilter with the given
//      parameters would result in an equivalent avtCreateBondsFilter.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
// ****************************************************************************

bool
avtCreateBondsFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(CreateBondsAttributes*)a);
}


// ****************************************************************************
//  Method:  avtCreateBondsFilter::AtomsShouldBeBondedManual
//
//  Purpose:
//    Checks if atoms should be bonded based on a set of min/max bond
//    lengths for atom pairs where at least one is hydrogen and atom
//    pairs where neither is hydrogen.
//
//  Arguments:
//    atomicnumbers   the array of atomic numbers
//    pts             the vtkPoints of the data set, to get their locations
//    a1,a2           the indexes of the atoms to check
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2006
//
// ****************************************************************************
bool
avtCreateBondsFilter::AtomsShouldBeBondedSimple(float *atomicnumbers,
                                                vtkPoints *pts,
                                                int a1, int a2)
{
    double *p1 = pts->GetPoint(a1);
    double *p2 = pts->GetPoint(a2);
    float dx = p1[0] - p2[0];
    float dy = p1[1] - p2[1];
    float dz = p1[2] - p2[2];
    float dist2 = dx*dx + dy*dy + dz*dz;

    float dhmin = atts.GetSimpleMinDistWithH();
    float dhmax = atts.GetSimpleMaxDistWithH();

    float dmin  = atts.GetSimpleMinDistNonH();
    float dmax  = atts.GetSimpleMaxDistNonH();

    if (atomicnumbers[a1]==1 ||
        atomicnumbers[a2]==1)
    {
        if (dist2 > dhmin*dhmin && dist2 < dhmax*dhmax)
            return true;
    }
    else
    {
        if (dist2 > dmin*dmin && dist2 < dmax*dmax)
            return true;
    }
    return false;
}

// ****************************************************************************
//  Method:  avtCreateBondsFilter::AtomsShouldBeBondedManual
//
//  Purpose:
//    Checks if atoms should be bonded based on an explicit
//    manual selection of bonding species and min/max bond lengths.
//
//  Arguments:
//    atomicnumbers   the array of atomic numbers
//    pts             the vtkPoints of the data set, to get their locations
//    a1,a2           the indexes of the atoms to check
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2006
//
// ****************************************************************************
bool
avtCreateBondsFilter::AtomsShouldBeBondedManual(float *atomicnumbers,
                                                vtkPoints *pts,
                                                int a1, int a2)
{
    double p1[3];
    double p2[3];
    pts->GetPoint(a1,p1);
    pts->GetPoint(a2,p2);
    float dx = p1[0] - p2[0];
    float dy = p1[1] - p2[1];
    float dz = p1[2] - p2[2];
    float dist2 = dx*dx + dy*dy + dz*dz;

    vector<double> &minDist = atts.GetMinDist();
    vector<double> &maxDist = atts.GetMaxDist();
    vector<int>    &element1 = atts.GetAtomicNumber1();
    vector<int>    &element2 = atts.GetAtomicNumber2();

    int n1 = atts.GetAtomicNumber1().size();
    int n2 = atts.GetAtomicNumber2().size();
    int n3 = atts.GetMinDist().size();
    int n4 = atts.GetMaxDist().size();
    if (n1 != n2 || n1 != n3 || n1 != n4)
    {
        EXCEPTION1(ImproperUseException,
                   "Bond list data arrays were not all the same length.");
    }
    int n = n1;

    for (int i=0; i<n; i++)
    {
        int e1 = element1[i] + 1;
        int e2 = element2[i] + 1;
        if ((atomicnumbers[a1]==e1 && atomicnumbers[a2]==e2) ||
            (atomicnumbers[a2]==e1 && atomicnumbers[a1]==e2))
        {
            double dmin = minDist[i];
            double dmax = maxDist[i];
            if (dist2 > dmin*dmin && dist2 < dmax*dmax)
            {
                return true;
            }
        }
    }

    return false;
}

// ****************************************************************************
//  Method: avtCreateBondsFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the CreateBonds filter.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//      <unused>   The label.
//
//  Returns:       The output dataset.
//
//  Programmer: js9 -- generated by xml2avt
//  Creation:   Tue Apr 18 17:24:04 PST 2006
//
// ****************************************************************************

vtkDataSet *
avtCreateBondsFilter::ExecuteData(vtkDataSet *in_ds, int, std::string)
{
    if (in_ds->GetDataObjectType() != VTK_POLY_DATA)
    {
        EXCEPTION1(ImproperUseException,
                   "Expected a vtkPolyData in the avtCreateBondsFilter.");
    }
    vtkPolyData *in_pd = (vtkPolyData*)in_ds;
    vtkPoints   *in_pts = in_pd->GetPoints();

    vtkDataArray *element = in_ds->GetPointData()->GetArray("element");
    if (!element)
    {
        return in_ds;
    }
    if (element && !element->IsA("vtkFloatArray"))
    {
        debug4 << "avtCreateBondsFilter: found a non-float array\n";
        return in_ds;
    }

    float *elementnos = element ? (float*)element->GetVoidPointer(0) : NULL;

    vtkPolyData *out_pd = in_pd->NewInstance();
    out_pd->ShallowCopy(in_pd);

    int natoms = in_ds->GetNumberOfPoints();

    vtkCellArray *lines = vtkCellArray::New();
    out_pd->SetLines(lines);
    lines->Delete();

    int max_per_atom = atts.GetMaxBondsClamp();
    //CreateBonds_Slow(ptr, elementnos);
    for (int i=0; i<natoms; i++)
    {
        int ctr = 0;
        for (int j=natoms-1; j>i && ctr<max_per_atom; j--)
        {
            if (i==j)
                continue;

            bool shouldBeBonded;
            if (atts.GetSimpleAlgorithmFlag())
                shouldBeBonded = AtomsShouldBeBondedSimple(elementnos,in_pts,i,j);
            else
                shouldBeBonded = AtomsShouldBeBondedManual(elementnos,in_pts,i,j);

            if (shouldBeBonded)
            {
                lines->InsertNextCell(2);
                lines->InsertCellPoint(i);
                lines->InsertCellPoint(j);
                ctr++;
            }
        }
    }

    ManageMemory(out_pd);
    out_pd->Delete();
    return out_pd;
}

// ****************************************************************************
//  Method:  avtCreateBondsFilter::PerformRestriction
//
//  Purpose:
//    Add element numbers to the secondary variable request.
//
//  Arguments:
//    spec       the pipeline specification
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2006
//
// ****************************************************************************
avtPipelineSpecification_p
avtCreateBondsFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    //
    // Get the old specification.
    //
    avtDataSpecification_p ds = spec->GetDataSpecification();
    const char *primaryVariable = ds->GetVariable();

    //
    // Make a new one
    //
    avtDataSpecification_p nds = new avtDataSpecification(ds);

    // Remove the bonds variable; we're going to create it here.
    nds->RemoveSecondaryVariable("bonds");

    // We need those element numbers.
    if (string(primaryVariable) != "element")
    {
        nds->AddSecondaryVariable("element");
    }

    //
    // Create the new pipeline spec from the data spec, and return
    //
    avtPipelineSpecification_p rv = new avtPipelineSpecification(spec, nds);

    return rv;
}
