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
#include <TimingsManager.h>

#include <map>
using std::pair;
using std::map;

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
//    Finds min/max bond lengths for a pair of species.
//
//  Arguments:
//    elementA/B        the atomic numbers of the atom pair
//    dmin,dmax         the min/max allowed bonding distance (output)
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Mon Feb 11 16:38:48 EST 2008
//    Support wildcards in matches, and bail out if we matched the
//    atom pattern but the distances were wrong.
//
//    Jeremy Meredith, Wed May 20 11:49:18 EDT 2009
//    MAX_ELEMENT_NUMBER now means the actual max element number, not the
//    total number of known elements in visit.  Added a fake "0" element
//    which means "unknown", and hydrogen now starts at 1.  This
//    also means we don't have to correct for 1-origin atomic numbers.
//
//    Jeremy Meredith, Tue Jan 26 16:35:41 EST 2010
//    Split into two functions to allow better optimizations.
//
// ****************************************************************************
bool
avtCreateBondsFilter::AtomBondDistances(int elementA, int elementB,
                                        double &dmin, double &dmax)
{
    dmin = 0.;
    dmax = 0.;

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
        int e1 = element1[i];
        int e2 = element2[i];
        bool match11 = (e1 < 0) || (elementA == e1);
        bool match12 = (e1 < 0) || (elementB == e1);
        bool match21 = (e2 < 0) || (elementA == e2);
        bool match22 = (e2 < 0) || (elementB == e2);
        if ((match11 && match22) || (match12 && match21))
        {
            dmin = minDist[i];
            dmax = maxDist[i];
            return true;
        }
    }

    return false;
}

// ****************************************************************************
// Method:  ShouldAtomsBeBonded
//
// Purpose:
//   Check if two atoms are within the allowable distances.
//
// Arguments:
//   dmin,dmax     the min/max bonding distances
//   pA,pB         the atom locations
//
// Programmer:  Jeremy Meredith
// Creation:    January 26, 2010
//
// ****************************************************************************
inline bool
ShouldAtomsBeBonded(double dmin, double dmax,
                    double *pA, double *pB)
{
    // Check for errors, of in case of no match for this pair
    if (dmax <= 0 || dmax < dmin)
        return false;

    // Okay, now see if the atom pair matches the right distance
    float dx = pA[0] - pB[0];
    float dy = pA[1] - pB[1];
    float dz = pA[2] - pB[2];
    float dist2 = dx*dx + dy*dy + dz*dz;

    if (dist2 > dmin*dmin && dist2 < dmax*dmax)
        return true;
    else
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
//    Jeremy Meredith, Wed Jan 27 10:37:03 EST 2010
//    Added periodic atom matching support.  This is a bit slow for the moment,
//    about a 10x penalty compared to the non-periodic matching, so we should
//    add a faster version at some point....
//
// ****************************************************************************

vtkDataSet *
avtCreateBondsFilter::ExecuteData(vtkDataSet *in_ds, int, std::string)
{
    //
    // Extract some input data
    //
    if (in_ds->GetDataObjectType() != VTK_POLY_DATA)
    {
        EXCEPTION1(ImproperUseException,
                   "Expected a vtkPolyData in the avtCreateBondsFilter.");
    }
    vtkPolyData  *in = (vtkPolyData*)in_ds;
    vtkPoints    *inPts = in->GetPoints();
    vtkCellArray *inVerts = in->GetVerts();
    vtkPointData *inPD  = in->GetPointData();
    vtkCellData  *inCD  = in->GetCellData();
    int nPts   = in->GetNumberOfPoints();
    int nVerts = in->GetNumberOfVerts();

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

    //
    // Set up the output stuff
    //
    vtkPolyData  *out      = in->NewInstance();
    vtkPointData *outPD    = out->GetPointData();
    vtkCellData  *outCD    = out->GetCellData();
    vtkPoints    *outPts   = vtkPoints::New();
    vtkCellArray *outVerts = vtkCellArray::New();
    vtkCellArray *outLines = vtkCellArray::New();
    out->GetFieldData()->ShallowCopy(in->GetFieldData());
    out->SetPoints(outPts);
    out->SetLines(outLines);
    out->SetVerts(outVerts);
    outPts->Delete();
    outLines->Delete();
    outVerts->Delete();

    //outPts->SetNumberOfPoints(nPts * 1.1);
    outPD->CopyAllocate(inPD,nPts);
    outCD->CopyAllocate(inCD,nVerts*1.2);

    //
    // Copy the input points and verts over.
    //
    for (int p=0; p<nPts; p++)
    {
        double pt[4] = {0,0,0,1};
        in->GetPoint(p, pt);
        int outpt = outPts->InsertNextPoint(pt);
        outPD->CopyData(inPD, p, outpt);
    }
    vtkIdType *vertPtr = inVerts->GetPointer();
    for (int v=0; v<nVerts; v++)
    {
        if (*vertPtr == 1)
        {
            vtkIdType id = *(vertPtr+1);
            int outcell = outVerts->InsertNextCell(1);
            outVerts->InsertCellPoint(id);
            outCD->CopyData(inCD, v, outcell);
        }
        vertPtr += (*vertPtr+1);
    }

    int natoms = in_ds->GetNumberOfPoints();


    //
    // Extract unit cell vectors in case we want to add bonds
    // for periodic atom images
    //
    bool addPeriodicBonds = atts.GetAddPeriodicBonds();
    double xv[3], yv[3], zv[3];
    for (int j=0; j<3; j++)
    {
        if (atts.GetUseUnitCellVectors())
        {
            const avtDataAttributes &datts = GetInput()->GetInfo().GetAttributes();
            const float *unitCell = datts.GetUnitCellVectors();
            xv[j] = unitCell[3*0+j];
            yv[j] = unitCell[3*1+j];
            zv[j] = unitCell[3*2+j];
        }
        else
        {
            xv[j] = atts.GetXVector()[j];
            yv[j] = atts.GetYVector()[j];
            zv[j] = atts.GetZVector()[j];
        }
    }

    map<pair<int, int>, int> imageMap;

    //
    // Loop over the (n^2)/2 atom pairs
    //
    int max_per_atom = atts.GetMaxBondsClamp();
    int timerMain = visitTimer->StartTimer();
    for (int i=0; i<natoms; i++)
    {
        int ctr = 0;
        for (int j=natoms-1; j>i && ctr<max_per_atom; j--)
        {
            if (i==j)
                continue;

            double p1[3];
            double p2[3];
            inPts->GetPoint(i,p1);
            inPts->GetPoint(j,p2);

            int e1 = elementnos[i];
            int e2 = elementnos[j];

            double dmin, dmax;
            bool possible = AtomBondDistances(e1,e2, dmin,dmax);
            if (possible && ShouldAtomsBeBonded(dmin,dmax, p1,p2))
            {
                outLines->InsertNextCell(2);
                outLines->InsertCellPoint(i);
                outLines->InsertCellPoint(j);
                ctr++;
            }
        }
    }
    visitTimer->StopTimer(timerMain, "CreateBonds: Main n^2 loop");

    //
    // If we're adding periodic bonds, loop over all n^2 atoms again.
    // This time, look for ones where one of the two atoms is not
    // in the native image (but the other one is).  Note that we're
    // not keeping any earlier bond-per-atom counters, so when this
    // is added, we might get 2*max total bonds.  Since this is
    // typically non-physical and is only a limit for sanity, no big
    // harm done.
    //
    int minX = 0, maxX = 0;
    int minY = 0, maxY = 0;
    int minZ = 0, maxZ = 0;
    if (addPeriodicBonds && atts.GetPeriodicInX())
    {
        minX = -1; maxX = +1;
    }
    if (addPeriodicBonds && atts.GetPeriodicInY())
    {
        minY = -1; maxY = +1;
    }
    if (addPeriodicBonds && atts.GetPeriodicInZ())
    {
        minZ = -1; maxZ = +1;
    }

    int timerPer = visitTimer->StartTimer();
    for (int j=0; j<natoms && addPeriodicBonds; j++)
    {
        int e2 = elementnos[j];
        double p2[3];
        inPts->GetPoint(j,p2);

        int    newJ[27];
        double newX[27];
        double newY[27];
        double newZ[27];
        int image = 0;
        for (int px=minX; px<=maxX; px++)
        {
            for (int py=minY; py<=maxY; py++)
            {
                for (int pz=minZ; pz<=maxZ; pz++)
                {
                    double pj[3] = {p2[0],p2[1],p2[2]};
                    for (int c=0; c<3; c++)
                    {
                        pj[c] += xv[c] * double(px);
                        pj[c] += yv[c] * double(py);
                        pj[c] += zv[c] * double(pz);
                    }
                    newJ[image] = -1;
                    newX[image] = pj[0];
                    newY[image] = pj[1];
                    newZ[image] = pj[2];
                    image++;
                }
            }
        }

        int ctr = 0;
        for (int i=0; i<natoms && addPeriodicBonds && ctr<max_per_atom; i++)
        {
            if (i==j)
                continue;

            int e1 = elementnos[i];
            double p1[3];
            inPts->GetPoint(i,p1);

            double dmin, dmax;
            bool possible = AtomBondDistances(e1,e2, dmin,dmax);
            if (!possible)
                continue;

            int image = 0;
            for (int px=minX; px<=maxX; px++)
            {
                for (int py=minY; py<=maxY; py++)
                {
                    for (int pz=minZ; pz<=maxZ; pz++, image++)
                    {
                        bool nativeImage = (px==0 && py==0 && pz==0);
                        if (nativeImage)
                            continue;

                        double *pi = p1;
                        double pj[3] = {newX[image],newY[image],newZ[image]};

                        if (ShouldAtomsBeBonded(dmin,dmax,pi,pj))
                        {
                            if (newJ[image] == -1)
                            {
                                pair<int,int> jthImage(image,j);
                                if (imageMap.count(jthImage)!=0)
                                {
                                    newJ[image] = imageMap[jthImage];
                                }
                                else
                                {
                                    newJ[image] = outPts->InsertNextPoint(pj);
                                    imageMap[jthImage] = newJ[image];
                                    outPD->CopyData(inPD, j, newJ[image]);
                                        
                                    // We're not adding a vertex cell here;
                                    // these are not real atoms, and the
                                    // molecule plot will now avoid drawing
                                    // them if no Vert is associated w/ it.
                                    //int outcell = outVerts->InsertNextCell(1);
                                    //outVerts->InsertCellPoint(newJ[image]);
                                    //outCD->CopyData(inCD, j, outcell);
                                }
                            }
                            outLines->InsertNextCell(2);
                            outLines->InsertCellPoint(i);
                            outLines->InsertCellPoint(newJ[image]);
                            ctr++;
                        }
                    }
                }
            }
        }
    }
    visitTimer->StopTimer(timerPer, "CreateBonds: periodic n^2 * 27images loop");

    vtkDataArray *origCellNums = outPD->GetArray("avtOriginalNodeNumbers");
    if (origCellNums)
    {
        for (int i=nPts; i<out->GetNumberOfPoints(); i++)
            origCellNums->SetTuple1(i,-1);
    }

    ManageMemory(out);
    out->Delete();
    return out;
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
