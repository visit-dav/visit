/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
//  File: avtFTLEFilter.C
// ************************************************************************* //

#include <avtFTLEFilter.h>

#include <avtExtents.h>
#include <avtMatrix.h>
#include <vtkStructuredGrid.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <InvalidVariableException.h>
#include <limits>
#include <cmath>

#include <avtOriginatingSource.h>
#include <vtkVisItScalarTree.h>
#include <avtGradientExpression.h>

#include <avtParallel.h>

// ****************************************************************************
//  Class: avtFTLEFilterIC
//
//  Purpose:
//  Serves as integration curve that evaluates the end of integration
//  step and stores the last position.
//
//  Modifications:
//
//   David Camp, Wed Mar  7 10:43:07 PST 2012
//   Added a Serialize flag to the arguments. This is to support the restore
//   ICs code.
//
// ****************************************************************************

class avtFTLEFilterIC : public avtIntegralCurve
{
public:
    /** constructor */
    avtFTLEFilterIC(
            int maxSteps_,
            double maxTime_,
            const avtIVPSolver* model,
            Direction dir,
            const double& t_start, //start time
            const avtVector& p_start_, //start position
            const avtVector &v_start_,
            long ID) //identification
                : avtIntegralCurve(model, dir, t_start, p_start_, v_start_, ID)
    {
        numStep = 0;
        maxSteps = maxSteps_; /** set maximum steps to integrate */
        maxTime = maxTime_; /** set maximum time to integrate */
        p_start = p_start_;
        p_end = p_start; /* the end and start begin at the same location */
    }

    /** default constructor */
    avtFTLEFilterIC() : avtIntegralCurve()
    {
        numStep = 0;
        maxSteps = 0;
        maxTime = 0;
    }

    bool LessThan(const avtIntegralCurve *ic) const
    {
        if (ic != NULL && (ic->id == id))
        {
            return numStep < ((avtFTLEFilterIC *)ic)->numStep;
        }
        return avtIntegralCurve::LessThan(ic);
    }

    avtIntegralCurve* MergeIntegralCurveSequence(std::vector<avtIntegralCurve *> &v)
    {
        for (int i = 0 ; i < v.size() ; i++)
        {
             avtFTLEFilterIC *ic = (avtFTLEFilterIC *) v[i];
             if (ic->numStep > numStep)
             {
                 numStep = ic->numStep;
                 p_end = ic->p_end;
             }
        }
        return this;
    }

    virtual void AnalyzeStep( avtIVPStep &step,
                              avtIVPField *field)
    {
        /* Check for termination:
        Code used from avtStreamline.C 
        */
        bool shouldTerminate = false;

        if( (direction == DIRECTION_FORWARD && step.GetT1() >= maxTime) ||
            (direction == DIRECTION_BACKWARD && step.GetT1() <= maxTime) )
            shouldTerminate = true;

        /** if the integration is taking too long */
        if( !shouldTerminate && numStep >= maxSteps )
        {
            shouldTerminate = true;
        }

        p_end = step.GetP1();
        CurrentLocation(p_end);

        /** update other termination criteria */
        numStep += 1;

        if (field->VelocityIsInstantaneous() &&
           (step.GetLength() / std::abs(step.t1 - step.t0) < 1e-8) &&
           numStep > 5)
           shouldTerminate = true;

        /* if Code has to be terminated */
        if( shouldTerminate )
        {
            status = STATUS_FINISHED;
        }
    }

    virtual void Serialize(MemStream::Mode mode, MemStream &buff, avtIVPSolver *solver, SerializeFlags serializeFlags)
    {
        avtIntegralCurve::Serialize(mode,buff,solver,serializeFlags);
        buff.io(mode, numStep);
        buff.io(mode, maxSteps);
        buff.io(mode, maxTime);
        buff.io(mode, p_start);
        buff.io(mode, p_end);
    }

    virtual bool   UseFixedTerminationTime(void) { return true; }
    virtual double FixedTerminationTime(void)    { return maxTime; }

    avtVector GetStartPoint() { return p_start; }
    avtVector GetEndPoint() { return p_end; }
protected:
    int numStep;
    int maxSteps;
    double maxTime;
    avtVector p_start, p_end;
};

// ****************************************************************************
//  Method: avtFTLEFilter constructor
//
//  Programmer: hchilds -- generated by xml2avt
//  Creation:   Mon Jan 10 07:15:51 PDT 2011
//
// ****************************************************************************

avtFTLEFilter::avtFTLEFilter()
{
    //outVarName ="operators/FTLE/mesh";
    //doPathlines();
    //SetPathlines(atts.GetPathlines(),
    //              atts.GetPathlinesOverrideStartingTimeFlag(),
    //              atts.GetPathlinesOverrideStartingTime(),
    //              atts.GetPathlinesCMFE());
    SetPathlines(true,false,0,0);
    //SetPathlines(false,false,0,0);

    // These initializations prevent harmless UMRs when we do our first
    // cache lookups.
    global_bounds[0] = global_bounds[2] = global_bounds[4] = 0;
    global_bounds[1] = global_bounds[3] = global_bounds[5] = 1;
    global_resolution[0] = global_resolution[1] = global_resolution[2] = 10;
}


// ****************************************************************************
//  Method: avtFTLEFilter destructor
//
//  Programmer: hchilds -- generated by xml2avt
//  Creation:   Mon Jan 10 07:15:51 PDT 2011
//
//  Modifications:
//
// ****************************************************************************

avtFTLEFilter::~avtFTLEFilter()
{
}


// ****************************************************************************
//  Method:  avtFTLEFilter::Create
//
//  Programmer: hchilds -- generated by xml2avt
//  Creation:   Mon Jan 10 07:15:51 PDT 2011
//
// ****************************************************************************

avtFilter *
        avtFTLEFilter::Create()
{
    return new avtFTLEFilter();
}


// ****************************************************************************
//  Method:      avtFTLEFilter::SetAtts
//
//  Purpose:
//      Sets the state of the filter based on the attribute object.
//
//  Arguments:
//      a        The attributes to use.
//
//  Programmer: hchilds -- generated by xml2avt
//  Creation:   Mon Jan 10 07:15:51 PDT 2011
//
// ****************************************************************************

void
avtFTLEFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const FTLEAttributes*)a;
}


// ****************************************************************************
//  Method: avtFTLEFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtFTLEFilter with the given
//      parameters would result in an equivalent avtFTLEFilter.
//
//  Programmer: hchilds -- generated by xml2avt
//  Creation:   Mon Jan 10 07:15:51 PDT 2011
//
// ****************************************************************************

bool
avtFTLEFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(FTLEAttributes*)a);
}


// ****************************************************************************
//  Method: avtFTLEFilter::GetInitialLocationsFromMesh
//
//  Purpose:
//      Walks through an AVT data tree and sets up the initial locations from
//      each point in the mesh.
//
//  Arguments:
//      inDT          A pointer to a data tree.  
//      setupList     Whether or not we should write to the list.
//      idx           The index into the list.
//
//  Programmer: Hank Childs
//  Creation:   December 5, 2011
//
// ****************************************************************************

int
avtFTLEFilter::GetInitialLocationsFromMesh(avtDataTree_p inDT, 
                                           std::vector<avtVector> &seedPoints,
                                           bool setupList, int idx)
{
    if (*inDT == NULL)
        return 0;

    int nc = inDT->GetNChildren();

    if (nc <= 0 && !inDT->HasData())
    {
        return 0;
    }

    if (nc == 0)
    {
        //
        // there is only one dataset to process
        //
        vtkDataSet *in_ds = inDT->GetDataRepresentation().GetDataVTK();
        size_t pts = in_ds->GetNumberOfPoints();
        if (setupList)
        {
            double points[3];
            for(size_t i = 0; i < pts; ++i)
            {
                in_ds->GetPoint(i,points);
                seedPoints[idx+i].set(points);
            }
        }
        return pts;
    }

    //
    // there is more than one input dataset to process
    // and we need an output datatree for each
    //
    int rv = 0;
    for (int j = 0; j < nc; j++)
    {
        if (inDT->ChildIsPresent(j))
        {
            int numNewPts = GetInitialLocationsFromMesh(inDT->GetChild(j),
                                                        seedPoints,
                                                        setupList, idx);
            rv += numNewPts;
            idx += numNewPts;
        }
    }
    return rv;
}


// ****************************************************************************
//  Method: avtFTLEFilter::GetInitialLocations
//
//  Purpose:
//      Tells the PICS filter where to place the initial seed locations.
//
//  Programmer: Hari Krishnan
//  Creation:   December 5, 2011
//
// ****************************************************************************

std::vector<avtVector>
avtFTLEFilter::GetInitialLocations()
{
    seedPoints.clear();

    //compute total number of seeds that will be generated..
    size_t totalNumberOfSeeds = 0;
    if (atts.GetRegionType() == FTLEAttributes::NativeResolutionOfMesh)
        totalNumberOfSeeds = GetInitialLocationsFromMesh(GetInputDataTree(), 
                                                         seedPoints, false, 0);
    else
        totalNumberOfSeeds = global_resolution[0]*global_resolution[1]*global_resolution[2];

    seedPoints.resize(totalNumberOfSeeds);

    if (atts.GetRegionType() == FTLEAttributes::NativeResolutionOfMesh)
    {
        GetInitialLocationsFromMesh(GetInputDataTree(), seedPoints, true, 0);
    }
    else
    {
        size_t l = 0; //current line of the seed..

        //add sample points by looping over in x,y,z
        for(int k = 0; k < global_resolution[2]; ++k)
        {
            double zpcnt = ((double)k)/((double)global_resolution[2]-1);
            double z = global_bounds[4]*(1.0-zpcnt) + global_bounds[5]*zpcnt;

            for(int j = 0; j < global_resolution[1]; ++j)
            {
                double ypcnt = ((double)j)/((double)global_resolution[1]-1);
                double y = global_bounds[2]*(1.0-ypcnt) + global_bounds[3]*ypcnt;

                for(int i = 0; i < global_resolution[0]; ++i)
                {
                    double xpcnt = ((double)i)/((double)global_resolution[0]-1);
                    double x = global_bounds[0]*(1.0-xpcnt) + global_bounds[1]*xpcnt;

                    size_t index = (global_resolution[1]*global_resolution[0]*k) + (global_resolution[0]*j)+i;
                    if(l >= seedPoints.size())
                        std::cout << "ERROR" << std::endl;
                    seedPoints[l].set(x,y,z);
                    l++;
                }
            }
        }

        if (l != totalNumberOfSeeds)
        {
            debug1 << "FTLE expected to generated " << totalNumberOfSeeds << " but only generated "
                   << l << endl;
            EXCEPTION0(ImproperUseException);
        }
    }//else

    return seedPoints;
}


// ****************************************************************************
//  Method: avtFTLEFilter::CreateIntegralCurve
//
//  Purpose:
//      Create an uninitialized integral curve.
//
//  Programmer: Hari Krishnan
//  Creation:   December 5, 2011
//
// ****************************************************************************

avtIntegralCurve*
avtFTLEFilter::CreateIntegralCurve(void)
{
    avtFTLEFilterIC *fic = new avtFTLEFilterIC();
    return fic;
}


// ****************************************************************************
//  Method: avtFTLEFilter::CreateIntegralCurve
//
//  Purpose:
//      Create an integral curve with specific IDs and parameters.
//
//  Programmer: Hari Krishnan
//  Creation:   December 5, 2011
//
// ****************************************************************************

avtIntegralCurve*
avtFTLEFilter::CreateIntegralCurve(const avtIVPSolver* model,
                                           avtIntegralCurve::Direction dir,
                                           const double& t_start,
                                           const avtVector &p_start,
                                           const avtVector& v_start,long ID)
{

    int maxSteps = 10000; /** TODO pass this value? */
    double timeToIntegrate =  atts.GetIntegrationTime();

    double maxTime = t_start + (dir == avtIntegralCurve::DIRECTION_FORWARD ?
                     timeToIntegrate : -timeToIntegrate);

    avtFTLEFilterIC *fic = new avtFTLEFilterIC(maxSteps,
                                               maxTime,
                                               model,
                                               dir,
                                               t_start,
                                               p_start,
                                               v_start,
                                               ID);

    return fic;
}

// ****************************************************************************
//  Method: avtFTLEFilter::ComputeFtle
//
//  Purpose:
//      Computes the FTLE gives a Jacobian.
//
//  Programmer: Hari Krishnan
//  Creation:   December 5, 2011
//
// ****************************************************************************

void avtFTLEFilter::ComputeFtle(vtkDataArray *jacobian[3],vtkDataArray *result)
{
    size_t nvalues = result->GetNumberOfTuples();
    for(size_t l = 0; l < nvalues; ++l)
    {
        double * j0 = jacobian[0]->GetTuple3(l);
        double * j1 = jacobian[1]->GetTuple3(l);
        double * j2 = jacobian[2]->GetTuple3(l);

        //x,y,z components compute left,right
        avtVector dx(j0[0],j1[0],j2[0]);
        //x,y,z components compute top,bottom
        avtVector dy(j0[1],j1[1],j2[1]);
        //x,y,z components compute front,back
        avtVector dz(j0[2],j1[2],j2[2]);

        //std::cout << dx << " " << dy << " " << dz << std::endl;

        //J*J^T
        //float a = dx.dot(dx), b = dx.dot(dy), c = dx.dot(dz);
        //float d = dy.dot(dy), e = dy.dot(dz), f = dz.dot(dz);

        float a = dx.x*dx.x + dx.y*dx.y + dx.z*dx.z;
        float b = dx.x*dy.x + dx.y*dy.y + dx.z*dy.z;
        float c = dx.x*dz.x + dx.y*dz.y + dx.z*dz.z;
        float d = dy.x*dy.x + dy.y*dy.y + dy.z*dy.z;
        float e = dy.x*dz.x + dy.y*dz.y + dy.z*dz.z;
        float f = dz.x*dz.x + dz.y*dz.y + dz.z*dz.z;
        float x = ( a + d + f ) / 3.0f;

        a -= x;
        d -= x;
        f -= x;

        float q = (a*d*f + b*e*c + c*b*e - c*d*c - e*e*a - f*b*b) / 2.0f;
        float r = (a*a + b*b + c*c + b*b + d*d + e*e + c*c + e*e + f*f);
        r /= 6.0f;

        float D = (r*r*r - q*q);
        float phi = 0.0f;

        //std::cout << a << " " << b << " " << c << " " << d << " "
        //          << e << " " << f << " " << x << " " << q << " " << r << std::endl;
        if( D < std::numeric_limits<float>::epsilon())
            phi = 0.0f;
        else
        {
            phi = atanf( sqrtf(D)/q ) / 3.0f;

            if( phi < 0 )
                phi += 3.1415926536f;
        }

        const float sqrt3 = sqrtf(3.0f);
        const float sqrtr = sqrtf(r);

        float sinphi = 0.0f, cosphi = 0.0f;
        sinphi = sinf(phi);
        cosphi = cosf(phi);

        float lambda = 1.0f;
        lambda = std::max( lambda, x + 2.0f*sqrtr*cosphi );
        lambda = std::max( lambda, x - sqrtr*(cosphi + sqrt3*sinphi) );
        lambda = std::max( lambda, x - sqrtr*(cosphi - sqrt3*sinphi) );

        //lambda = log( sqrtf( lambda ) ) + 0.000000001;
        //                    std::cout << "s: " << lambda << std::endl;
        lambda = log( sqrtf( lambda ) );
        lambda /= (float) atts.GetIntegrationTime();

        //                    std::cout << "lambda :" << lambda << std::endl;
        result->SetTuple1(l,lambda);
    }
}


// ****************************************************************************
//  Method: avtFTLEFilter::MultiBlockComputeFTLE
//
//  Purpose:
//      Computes the FTLE for the whole data set, using the 
//      final particle locations, at the blocks native resolution.
//
//  Programmer: Hari Krishnan
//  Creation:   December 5, 2011
//
// ****************************************************************************

avtDataTree_p
avtFTLEFilter::MultiBlockComputeFTLE(avtDataTree_p inDT, std::vector<avtIntegralCurve*> &ics,
                                     int &offset, double &minv, double &maxv)
{
    if (*inDT == NULL)
        return 0;

    int nc = inDT->GetNChildren();

    if (nc <= 0 && !inDT->HasData())
    {
        return 0;
    }

    if (nc == 0)
    {
        //
        // there is only one dataset to process
        //
        vtkDataSet *in_ds = inDT->GetDataRepresentation().GetDataVTK();
        int dom = inDT->GetDataRepresentation().GetDomain();
        std::string label = inDT->GetDataRepresentation().GetLabel();
        vtkDataSet *out_ds = SingleBlockComputeFTLE(in_ds, ics, offset, dom, minv, maxv);
        avtDataTree_p rv = new avtDataTree(out_ds, dom, label);
        out_ds->Delete();
        return rv;
    }

    //
    // there is more than one input dataset to process
    // and we need an output datatree for each
    //
    avtDataTree_p *outDT = new avtDataTree_p[nc];
    for (int j = 0; j < nc; j++)
    {
        if (inDT->ChildIsPresent(j))
            outDT[j] = MultiBlockComputeFTLE(inDT->GetChild(j), ics, 
                                             offset, minv, maxv);
        else
            outDT[j] = NULL;
    }
    avtDataTree_p rv = new avtDataTree(nc, outDT);
    delete [] outDT;
    return rv;
}


// ****************************************************************************
//  Method: avtFTLEFilter::SingleBlockComputeFTLE
//
//  Purpose:
//      Computes the FTLE for a single block of a data set, using the 
//      final particle locations, at the blocks native resolution.
//
//  Arguments:
//      in_ds   The block to calculate the FTLE on
//      ics     The list of particles for all blocks on this MPI task.
//      domain  The domain number of in_ds
//      minv    The minimum FTLE value (output)
//      maxv    The maximum FTLE value (output)
//
//  Returns:    The new version of in_ds that includes the FTLE scalar
//              variable.  The calling function is responsible for dereferencing
//              this VTK object.
//
//  Programmer: Hari Krishnan
//  Creation:   December 5, 2011
//
// ****************************************************************************

vtkDataSet *
avtFTLEFilter::SingleBlockComputeFTLE(vtkDataSet *in_ds, std::vector<avtIntegralCurve*> &ics,
                                      int &offset, int domain, double &minv, double &maxv)
{
    //variable name..
    std::string var = std::string("operators/FTLE/") + outVarName;

    //create new instance from old..
    vtkDataSet* out_grid = in_ds->NewInstance();
    out_grid->ShallowCopy(in_ds);
    int ntuples = in_ds->GetNumberOfPoints();

    //copy the initial locations
    std::vector<avtVector> remapVector;
    remapVector.resize(ntuples);

    //copy the original seed points
    for(size_t i = 0; i < remapVector.size(); ++i)
        remapVector[i] = seedPoints.at(offset + i);

#if 1
    for(int i = 0; i < ics.size(); ++i)
    {
        size_t index = ics[i]->id;
        int l = (index-offset);
        //std::cout << "l = " << l << " " << ntuples << std::endl;
        if(l >= 0 && l < remapVector.size())
        {
            //remapVector[l] = ((avtFTLEFilterIC*)ics[i])->GetEndPoint() - ((avtFTLEFilterIC*)ics[i])->GetStartPoint();
            remapVector.at(l) = ((avtFTLEFilterIC*)ics[i])->GetEndPoint();
        }
    }

    // We are done with offset, increment it for the next call to this function.
    offset += ntuples;

    //use static function in avtGradientExpression to calculate gradients..
    //since this function only does scalar, break our vectors into scalar components and calculate one at a time..

    vtkDataArray* jacobian[3];
    vtkFloatArray *component = vtkFloatArray::New();
    component->SetName(var.c_str());
    component->SetNumberOfTuples(ntuples);

    //temporarily add point data to output grid in order to calculate gradient per component..

    //is this soft copy or deep copy?
    out_grid->GetPointData()->AddArray(component);
    out_grid->GetPointData()->SetActiveScalars(var.c_str());

    for(int i = 0; i < 3; ++i)
    {
        for(size_t j = 0; j < ntuples; ++j)
            component->SetTuple1(j,remapVector[j][i]);
        if (GetInput()->GetInfo().GetAttributes().DataIsReplicatedOnAllProcessors())
        {
            float *newvals = new float[ntuples];
            float *origvals = (float *) component->GetVoidPointer(0);
            SumFloatArrayAcrossAllProcessors(origvals, newvals, ntuples);
            memcpy(origvals, newvals, ntuples*sizeof(float));  // copy newvals back into origvals
        }

        jacobian[i] = avtGradientExpression::CalculateGradient(out_grid,var.c_str());
    }

    //remove array once done..
    component->Delete();
    out_grid->GetPointData()->RemoveArray(var.c_str());

    //not reusing component because the resolution of the original dataset and new dataset may be different?
    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetNumberOfTuples(ntuples);

    for (int i = 0 ; i < ntuples ; i++)
        arr->SetTuple1(i, std::numeric_limits<float>::epsilon());

    //now I should have components of the jacobian 3 arrays of 3x3 array..
    ComputeFtle(jacobian,arr);

    for(int i = 0; i < ntuples; ++i)
    {
        minv = std::min(arr->GetTuple1(i),minv);
        maxv = std::max(arr->GetTuple1(i),maxv);
    }

    arr->SetName(var.c_str());

    out_grid->GetPointData()->AddArray(arr);
    arr->Delete();
    out_grid->GetPointData()->SetActiveScalars(var.c_str());

    jacobian[0]->Delete();
    jacobian[1]->Delete();
    jacobian[2]->Delete();
#else
    // This code shows the distance a particle traveled ... very useful for debugging.
    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetNumberOfTuples(ntuples);

    for (int i = 0 ; i < ntuples ; i++)
        arr->SetTuple1(i, -1);

    for(int i = 0; i < ics.size(); ++i)
    {
        size_t index = ics[i]->id;
        int l = (index-offset);
        if(l >= 0 && l < ntuples)
        {
            arr->SetTuple1(l, 
            ((avtFTLEFilterIC*)ics[i])->GetEndPoint()[0] - ((avtFTLEFilterIC*)ics[i])->GetStartPoint()[0]);
        }
    }
    for(int i = 0; i < ntuples; ++i)
    {
        minv = std::min(arr->GetTuple1(i),minv);
        maxv = std::max(arr->GetTuple1(i),maxv);
    }

    arr->SetName(var.c_str());

    out_grid->GetPointData()->AddArray(arr);
    arr->Delete();
    out_grid->GetPointData()->SetActiveScalars(var.c_str());
    offset += ntuples;

#endif


    //Store this dataset in Cache for next time..
    std::string str = CreateNativeResolutionCacheString();
    StoreArbitraryVTKObject(SPATIAL_DEPENDENCE | DATA_DEPENDENCE,
                            outVarName.c_str(),domain, -1,
                            str.c_str(), out_grid);

    // Calling function must free this.
    return out_grid;
}


// ****************************************************************************
//  Method: avtFTLEFilter::ComputeNativeDataSetResolution
//
//  Purpose:
//      Computes the FTLE after the particles have been advected.
//
//  Programmer: Hari Krishnan
//  Creation:   December 5, 2011
//
// ****************************************************************************

void avtFTLEFilter::ComputeNativeDataSetResolution(std::vector<avtIntegralCurve*> &ics)
{
    //accumulate all of the points then do jacobian?
    //or do jacobian then accumulate?
    //picking the first..

    double minv   = std::numeric_limits<double>::max();
    double maxv   = std::numeric_limits<double>::min();
    int    offset = 0;

    avtDataTree_p outTree = MultiBlockComputeFTLE(GetInputDataTree(), ics, offset,
                                                  minv, maxv);
    if (GetInput()->GetInfo().GetAttributes().DataIsReplicatedOnAllProcessors())
        if (PAR_Rank() != 0)
            outTree = new avtDataTree();
    SetOutputDataTree(outTree);

    avtDataAttributes &dataatts = GetOutput()->GetInfo().GetAttributes();
    avtExtents* e = dataatts.GetThisProcsActualDataExtents();

    double range[2];
    range[0] = minv;
    range[1] = maxv;
    e->Set(range);

    e = dataatts.GetThisProcsOriginalDataExtents();
    e->Set(range);

    e = dataatts.GetThisProcsActualSpatialExtents();
    e->Set(global_bounds);
    e = dataatts.GetThisProcsOriginalSpatialExtents();
    e->Set(global_bounds);
}


// ****************************************************************************
//  Method: avtFTLEFilter::ComputeRectilinearDataResolution
//
//  Purpose:
//      Computes the FTLE on a rectilinear grid.
//
//  Programmer: Hari Krishnan
//  Creation:   December 5, 2011
//
// ****************************************************************************

void avtFTLEFilter::ComputeRectilinearDataResolution(std::vector<avtIntegralCurve*> &ics)
{

    //variable name..
    std::string var = std::string("operators/FTLE/") + outVarName;

    //algorithm sends index to global datastructure as well as end points..
    //Send List of index into global array to rank 0
    //Send end positions into global array to rank 0

    //loop over all the intelgral curves and add it back to the original list of seeds..
    intVector indices(ics.size());
    doubleVector end_results(ics.size()*3);

    for(size_t i = 0,j = 0; i < ics.size(); ++i,j += 3)
    {
        indices[i] = ics[i]->id;

        //avtVector distance = ((avtFTLEFilterIC*)ics[i])->GetEndPoint() - ((avtFTLEFilterIC*)ics[i])->GetStartPoint();

        //if(distance.length() > 0)
        //    std::cout << "distance: " << indices[i] << " " << distance.length() << std::endl;

        avtVector end_point = ((avtFTLEFilterIC*)ics[i])->GetEndPoint();
        //std::cout << PAR_Rank() << " ics: " << indices[i] << " " << end_point << std::endl;
        end_results[j+0] = end_point[0];
        end_results[j+1] = end_point[1];
        end_results[j+2] = end_point[2];
    }
    std::cout << PAR_Rank() << " total integral pts: " << ics.size() << std::endl;

    std::flush(cout);

    int* all_indices;
    int* index_counts;
    double* all_result;
    int *result_counts;

    Barrier();

    CollectIntArraysOnRootProc(all_indices,index_counts,&indices.front(),indices.size());
    CollectDoubleArraysOnRootProc(all_result,result_counts,&end_results.front(),end_results.size());

    Barrier();
    //root should now have index into global structure and all matching end positions..
    if(PAR_Rank() != 0)
    {
        std::cout << PAR_Rank() << " creating dummy output" << std::endl;
        avtDataTree* dummy = new avtDataTree();
        SetOutputDataTree(dummy);
    }
    else
    {
        //rank 0 ..
        //now create a rectilinear grid ..
        vtkRectilinearGrid* rect_grid = vtkRectilinearGrid::New();

        vtkFloatArray* lxcoord = vtkFloatArray::New();
        vtkFloatArray* lycoord = vtkFloatArray::New();
        vtkFloatArray* lzcoord = vtkFloatArray::New();

        rect_grid->SetDimensions(global_resolution);

        lxcoord->SetNumberOfTuples(global_resolution[0]);
        for (int i = 0 ; i < global_resolution[0] ; i++)
        {
            double pcnt = ((double)i)/((double)global_resolution[0]-1);
            lxcoord->SetTuple1(i, global_bounds[0]*(1.0-pcnt) + global_bounds[1]*pcnt);
        }

        lycoord->SetNumberOfTuples(global_resolution[1]);
        for (int i = 0 ; i < global_resolution[1] ; i++)
        {
            double pcnt = ((double)i)/((double)global_resolution[1]-1);
            lycoord->SetTuple1(i, global_bounds[2]*(1.0-pcnt) + global_bounds[3]*pcnt);
        }

        lzcoord->SetNumberOfTuples(global_resolution[2]);
        for (int i = 0 ; i < global_resolution[2] ; i++)
        {
            double pcnt = ((double)i)/((double)global_resolution[2]-1);
            lzcoord->SetTuple1(i, global_bounds[4]*(1.0-pcnt) + global_bounds[5]*pcnt);
        }

        rect_grid->SetXCoordinates(lxcoord);
        rect_grid->SetYCoordinates(lycoord);
        rect_grid->SetZCoordinates(lzcoord);

        //cleanup
        lxcoord->Delete();
        lycoord->Delete();
        lzcoord->Delete();

        //now global grid has been created..
        size_t leafSize = global_resolution[0]*global_resolution[1]*global_resolution[2];

        std::cout << "final resolution: " << PAR_Rank() << " " << global_resolution[0] << " "
                << global_resolution[1] << " "
                << global_resolution[2] << std::endl;


        vtkDataArray* jacobian[3];
        vtkFloatArray *component = vtkFloatArray::New();

        component->SetName(var.c_str());
        component->SetNumberOfTuples(leafSize);

        rect_grid->GetPointData()->AddArray(component);
        rect_grid->GetPointData()->SetActiveScalars(var.c_str());

        //calculate jacobian in parts (x,y,z)..
        std::vector<avtVector> remapVector(leafSize);

        //recreate initial grid..
        size_t l = 0;
        for(int k = 0; k < global_resolution[2]; ++k)
        {
            double zpcnt = ((double)k)/((double)global_resolution[2]-1);
            double z = global_bounds[4]*(1.0-zpcnt) + global_bounds[5]*zpcnt;

            for(int j = 0; j < global_resolution[1]; ++j)
            {
                double ypcnt = ((double)j)/((double)global_resolution[1]-1);
                double y = global_bounds[2]*(1.0-ypcnt) + global_bounds[3]*ypcnt;

                for(int i = 0; i < global_resolution[0]; ++i)
                {
                    double xpcnt = ((double)i)/((double)global_resolution[0]-1);
                    double x = global_bounds[0]*(1.0-xpcnt) + global_bounds[1]*xpcnt;

                    remapVector[l].set(x,y,z);
                    l++;
                }
            }
        }

        //update remapVector with new value bounds from integral curves..
        int par_size = PAR_Size();
        size_t total = 0;
        for(int i = 0; i < par_size; ++i)
        {
            if(index_counts[i]*3 != result_counts[i])
            {
                std::cout << "Throw exception: mismatching counts" << std::endl;
            }
            total += index_counts[i];
        }

        std::cout << "total number integrated: " << total << std::endl;
        for(int j = 0,k = 0; j < total; ++j, k += 3)
        {
            size_t index = all_indices[j];

            if(index >= leafSize)
            {
                std::cout << "Throw exception: index > leafSize" << std::endl;
            }

            //std::cout << index << " before " << remapVector[index] << std::endl;
            remapVector[index].set(all_result[k+0],all_result[k+1],all_result[k+2]);
            //std::cout << PAR_Rank() << " " << index << " " << remapVector[index] << std::endl;
            //std::cout << "middle: " << avtVector(all_result[k+0],all_result[k+1],all_result[k+2]) << std::endl;
            //std::cout << index << " after " << remapVector[index] << std::endl;
        }

        for(int i = 0; i < 3; ++i)
        {
            for(size_t j = 0; j < leafSize; ++j)
                component->SetTuple1(j,remapVector[j][i]);

            jacobian[i] = avtGradientExpression::CalculateGradient(rect_grid,var.c_str());
        }

        for (int i = 0 ; i < leafSize ; i++)
            component->SetTuple1(i, std::numeric_limits<float>::epsilon());

        ComputeFtle(jacobian,component);

        //min and max values over all datasets of the tree..
        double minv = std::numeric_limits<double>::max();
        double maxv = std::numeric_limits<double>::min();

        for(int i = 0; i < leafSize; ++i)
        {
            minv = std::min(component->GetTuple1(i),minv);
            maxv = std::max(component->GetTuple1(i),maxv);
        }

        //cleanup..
        component->Delete();
        jacobian[0]->Delete();
        jacobian[1]->Delete();
        jacobian[2]->Delete();

        //Store this dataset in Cache for next time..
        double bounds[6];
        rect_grid->GetBounds(bounds);
        std::cout << "final size and bounds: " << PAR_Rank() << " " << leafSize << " "
                << bounds[0] << " " << bounds[1] << " " << bounds[2]
                << " " << bounds[3] << " " << bounds[4] << " "
                << bounds[5] << std::endl;

        std::string str = CreateResampledCacheString();
        StoreArbitraryVTKObject(SPATIAL_DEPENDENCE | DATA_DEPENDENCE,
                                outVarName.c_str(), -1, -1,
                                str.c_str(), rect_grid);

        int index = 0;//what does index mean in this context?
        avtDataTree* dt = new avtDataTree(rect_grid,index);
        int x = 0;
        dt->GetAllLeaves(x);
        std::cout << "total leaves:: " << x << std::endl;
        SetOutputDataTree(dt);

        //set atts..
        avtDataAttributes &dataatts = GetOutput()->GetInfo().GetAttributes();
        avtExtents* e = dataatts.GetThisProcsActualDataExtents();

        double range[2];
        range[0] = minv;
        range[1] = maxv;
        e->Set(range);
    }
}

// ****************************************************************************
//  Method: avtFTLEFilter::CreateIntegralCurveOutput
//
//  Purpose:
//      Computes the FTLE output (via sub-routines) after the PICS filter has
//      calculated the final particle positions.
//
//  Programmer: Hari Krishnan
//  Creation:   December 5, 2011
//
// ****************************************************************************

void 
avtFTLEFilter::CreateIntegralCurveOutput(std::vector<avtIntegralCurve*> &ics)
{
    if (atts.GetRegionType() == FTLEAttributes::NativeResolutionOfMesh)
        ComputeNativeDataSetResolution(ics);
    else
        ComputeRectilinearDataResolution(ics);
}


// ****************************************************************************
//  Method: avtFTLEFilter::PreExecute
//
//  Purpose:
//      Initialize data attributes for this filter and its base type (PICS).
//
//  Programmer: Hari Krishnan
//  Creation:   December 5, 2011
//
// ****************************************************************************

void 
avtFTLEFilter::PreExecute(void)
{
    SetActiveVariable(outVarName.c_str());
    SetStreamlineAlgorithm(STREAMLINE_VISIT_SELECTS, 10, 3, 10);

    GetSpatialExtents(global_bounds);

    if(!atts.GetUseDataSetStart())
    {
        double* a = atts.GetStartPosition();
        global_bounds[0] = a[0];
        global_bounds[2] = a[1];
        global_bounds[4] = a[2];
    }

    if(!atts.GetUseDataSetEnd())
    {
        double* a = atts.GetEndPosition();
        global_bounds[1] = a[0];
        global_bounds[3] = a[1];
        global_bounds[5] = a[2];
    }

    const int* res = atts.GetResolution();
    global_resolution[0] = res[0];
    global_resolution[1] = res[1];
    global_resolution[2] = res[2];
    avtPICSFilter::PreExecute();
}


// ****************************************************************************
//  Method: avtFTLEFilter::Execute
//
//  Purpose:
//      Executes the FTLE.  If we already have a cached version, then it
//      just returns that version.  If not, it calls PICS execute, which will
//      call our FTLE set up routines via CreateIntegralCurveOutput.
//
//  Programmer: Hari Krishnan
//  Creation:   December 5, 2011
//
// ****************************************************************************

void
avtFTLEFilter::Execute(void)
{
    avtDataTree_p dt = GetCachedDataSet();
    if (*dt != NULL)
    {
        debug1 << "FTLE: using cached version" << std::endl;
        if (GetInput()->GetInfo().GetAttributes().DataIsReplicatedOnAllProcessors())
            if (PAR_Rank() != 0)
                dt = new avtDataTree();
        SetOutputDataTree(dt);
        return;
    }

    debug1 << "FTLE: no cached version, must re-execute" << std::endl;
    avtPICSFilter::Execute();
}

// ****************************************************************************
//  Method: avtFTLEFilter::ModifyContract
//
//  Purpose:
//      Creates a contract the removes the operator-created-expression.
//
//  Programmer: hchilds -- generated by xml2avt
//  Creation:   Mon Jan 10 07:15:51 PDT 2011
//
// ****************************************************************************

avtContract_p
avtFTLEFilter::ModifyContract(avtContract_p in_contract)
{
    avtDataRequest_p in_dr = in_contract->GetDataRequest();
    std::string var =  in_dr->GetOriginalVariable();
//    in_contract->SetReplicateSingleDomainOnAllProcessors(true);
//    in_contract->SetOnDemandStreaming(false);
//    in_contract->GetDataRequest();
    in_dr->SetUsesAllDomains(true);
    if( strncmp(var.c_str(), "operators/FTLE/", strlen("operators/FTLE/")) == 0)
    {
        std::string justTheVar = var.substr(strlen("operators/FTLE/"));

        outVarName = justTheVar;
        avtDataRequest_p out_dr = new avtDataRequest(in_dr,justTheVar.c_str());
        //out_dr->SetDesiredGhostDataType(GHOST_NODE_DATA);
        //out_dr->SetDesiredGhostDataType(GHOST_ZONE_DATA);


        return avtPICSFilter::ModifyContract( new avtContract(in_contract,out_dr) );
    }
    return avtPICSFilter::ModifyContract(in_contract);
}


// ****************************************************************************
//  Method: avtFTLEFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Tells output that we have a new variable.
//
//  Programmer: hchilds -- generated by xml2avt
//  Creation:   Mon Jan 10 07:15:51 PDT 2011
//
// ****************************************************************************

void
avtFTLEFilter::UpdateDataObjectInfo(void)
{
    avtDataAttributes &in_atts = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &atts = GetOutput()->GetInfo().GetAttributes();

    timeState = in_atts.GetTimeIndex();

    //the outvarname has been assigned and will be added..
    //outVarName = "velocity";
    if (outVarName != "")
    {
        std::string fullVarName = std::string("operators/FTLE/") + outVarName;
        atts.RemoveVariable(in_atts.GetVariableName());

        if (! atts.ValidVariable(fullVarName) )
        {
            //std::cout << "Adding variable: " << outVarName << std::endl;
            //atts.AddVariable(outVarName.c_str());
            atts.AddVariable((fullVarName).c_str());
            atts.SetActiveVariable(fullVarName.c_str());
            atts.SetVariableDimension(1);
            //atts.SetTopologicalDimension(3);
            atts.SetVariableType(AVT_SCALAR_VAR);
            atts.SetCentering(AVT_NODECENT);
        }
    }

    avtPICSFilter::UpdateDataObjectInfo();
}


// ****************************************************************************
//  Method: avtFTLEFilter::CreateResampledCacheString
//
//  Purpose:
//      A routine that calculates a string for caching that encodes all the
//      parameters of the FTLE: bounds, integration time, and variable name.
//
//  Programmer: Hari Krishnan
//  Creation:   December 5, 2011
//
// ****************************************************************************

std::string
avtFTLEFilter::CreateResampledCacheString(void)
{
    double integrationTime = atts.GetIntegrationTime();
    char str[1024];
    sprintf(str, "FTLE_OF_%s_BOUNDS_%f_%f_%f_%f_%f_%f_RESOLUTION_%d_%d_%d_INTEGRATION_%f_timeindex_%d",
                 outVarName.c_str(), 
                 global_bounds[0], global_bounds[1], global_bounds[2], 
                 global_bounds[3], global_bounds[4], global_bounds[5], 
                 global_resolution[0], global_resolution[1], global_resolution[2],
                 integrationTime, timeState);
    return std::string(str);
}


// ****************************************************************************
//  Method: avtFTLEFilter::CreateNativeResolutionCacheString
//
//  Purpose:
//      A routine that calculates a string for caching that encodes all the
//      parameters of the FTLE: bounds, integration time, and variable name.
//
//  Programmer: Hari Krishnan
//  Creation:   December 5, 2011
//
// ****************************************************************************

std::string
avtFTLEFilter::CreateNativeResolutionCacheString(void)
{
    double integrationTime = atts.GetIntegrationTime();
    char str[1024];
    sprintf(str, "FTLE_OF_%s_BOUNDS_%f_%f_%f_%f_%f_%f_INTEGRATION_%f_timeindex_%d",
                 outVarName.c_str(), 
                 global_bounds[0], global_bounds[1], global_bounds[2], 
                 global_bounds[3], global_bounds[4], global_bounds[5], 
                 integrationTime, timeState);
    return std::string(str);
}

// ****************************************************************************
//  Method: avtFTLEFilter::GetCachedDataSet
//
//  Purpose:
//      Checks to see if we have already calculated the FTLE.  Works for
//      both resampling and native options (realized through sub-routine calls).
//      This routine uses collective communication to decided whether
//      it can use a cached data set.
//      - For the native resolution, all domains on all MPI tasks must have
//      the FTLE cached.
//      - For the resampled version, one MPI task somewhere must find it.
//
//  Programmer: Hari Krishnan
//  Creation:   December 5, 2011
//
// ****************************************************************************

avtDataTree_p
avtFTLEFilter::GetCachedDataSet()
{
    avtDataTree_p rv = NULL;
    if (atts.GetRegionType() == FTLEAttributes::NativeResolutionOfMesh)
    {
        rv = GetCachedNativeDataSet(GetInputDataTree());
        int looksOK = 1;
        if ((*rv == NULL) && (*(GetInputDataTree()) != NULL))
            looksOK = 0;
        looksOK = UnifyMinimumValue(looksOK); // if any fails, we all fail
        if (looksOK == 0)
            rv = NULL;
    }
    else
    {
        rv = GetCachedResampledDataSet();
        int looksOK = (*rv == NULL ? 0 : 1);
        looksOK = UnifyMaximumValue(looksOK); // if one has it, we're all OK
        if (looksOK == 0)
            rv = NULL;
        else if ((looksOK == 1) && (*rv == NULL))
            rv = new avtDataTree();
    }
    return rv;
}

// ****************************************************************************
//  Method: avtFTLEFilter::GetCachedNativeDataSet
//
//  Purpose:
//      Checks the cache to see if we have calculated the FTLE on this
//      domain previously.  Checks to make sure parameters match: integration
//      time, etc.
//
//  Programmer: Hank Childs
//  Creation:   December 5, 2011
//
// ****************************************************************************

avtDataTree_p
avtFTLEFilter::GetCachedNativeDataSet(avtDataTree_p inDT)
{
    if (*inDT == NULL)
        return NULL;

    int nc = inDT->GetNChildren();

    if (nc <= 0 && !inDT->HasData())
    {
        // degenerate.  just return.
        return NULL;
    }

    if (nc == 0)
    {
        //
        // there is only one dataset to process
        //
        vtkDataSet *in_ds = inDT->GetDataRepresentation().GetDataVTK();
        int dom = inDT->GetDataRepresentation().GetDomain();
        std::string label = inDT->GetDataRepresentation().GetLabel();
        std::string str = CreateNativeResolutionCacheString();
        vtkDataSet *rv = (vtkDataSet *)
                         FetchArbitraryVTKObject(SPATIAL_DEPENDENCE | DATA_DEPENDENCE,
                                                 outVarName.c_str(), dom, -1, str.c_str());
        if (rv == NULL)
            return NULL;
        else
            return new avtDataTree(rv, dom, label);
    }

    //
    // there is more than one input dataset to process
    // and we need an output datatree for each
    //
    avtDataTree_p *outDT = new avtDataTree_p[nc];
    bool badOne = false;
    for (int j = 0; j < nc; j++)
    {
        if (inDT->ChildIsPresent(j))
        {
            outDT[j] = GetCachedNativeDataSet(inDT->GetChild(j));
            if (*(outDT[j]) == NULL)
                badOne = true;
        }
        else
        {
            outDT[j] = NULL;
        }
    }
    avtDataTree_p rv = NULL;
    if (!badOne) // if we don't have FTLE for one domain, then just re-calc whole thing
        rv = new avtDataTree(nc, outDT);
    delete [] outDT;
    return rv;
}


// ****************************************************************************
//  Method: avtFTLEFilter::GetCachedResampledDataSet
//
//  Purpose:
//      Checks the cache to see if we have calculated the FTLE before.  It
//      also ensures that the parameters of previous calculations are the
//      same: integration time, bounds, variable, etc.
//
//  Programmer: Hari Krishnan
//  Creation:   December 5, 2011
//
// ****************************************************************************

avtDataTree_p
avtFTLEFilter::GetCachedResampledDataSet()
{
    std::string str = CreateResampledCacheString();
    vtkRectilinearGrid *rv = (vtkRectilinearGrid *)
                     FetchArbitraryVTKObject(SPATIAL_DEPENDENCE | DATA_DEPENDENCE,
                                             outVarName.c_str(), -1, -1,
                                             str.c_str());

    if(rv != NULL)
    {
        return new avtDataTree(rv, -1);
    }

    return NULL;
}


