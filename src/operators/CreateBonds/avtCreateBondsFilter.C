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
//  Modifications:
//    Jeremy Meredith, Mon Feb 11 16:38:48 EST 2008
//    Support wildcards in matches, and bail out if we matched the
//    atom pattern but the distances were wrong.
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
        // a -1 in the element list means "any"
        int e1 = element1[i] + 1;
        int e2 = element2[i] + 1;
        bool match11 = (e1 == 0) || (atomicnumbers[a1] == e1);
        bool match12 = (e1 == 0) || (atomicnumbers[a2] == e1);
        bool match21 = (e2 == 0) || (atomicnumbers[a1] == e2);
        bool match22 = (e2 == 0) || (atomicnumbers[a2] == e2);
        if ((match11 && match22) || (match12 && match21))
        {
            double dmin = minDist[i];
            double dmax = maxDist[i];
            if (dist2 > dmin*dmin && dist2 < dmax*dmax)
            {
                return true;
            }
            else
            {
                return false;
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
//  Modifications:
//    Jeremy Meredith, Mon Feb 11 16:39:51 EST 2008
//    The manual bonding matches now supports wildcards, alleviating the need
//    for a "simple" mode.  (The default for the manual mode is actually
//    exactly what the simple mode was going to be anyway.)
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
//  Method:  avtCreateBondsFilter::ModifyContract
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
avtContract_p
avtCreateBondsFilter::ModifyContract(avtContract_p spec)
{
    //
    // Get the old specification.
    //
    avtDataRequest_p ds = spec->GetDataRequest();
    const char *primaryVariable = ds->GetVariable();

    //
    // Make a new one
    //
    avtDataRequest_p nds = new avtDataRequest(ds);

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
    avtContract_p rv = new avtContract(spec, nds);

    return rv;
}
