/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
//  File: avtLCSFilter.C
// ************************************************************************* //

#include <avtLCSFilter.h>
#include <avtLCSIC.h>

// For now use the avtLCSIC as the state does not need to be recorded
// for the FSLE. That is because currently the integration is being
// done step by step rather than in chunks. However, the code is set up
// to use avtStreamlineIC. Which if the integration is done in chucks
// will probably be more efficient.

//#include <avtStreamlineIC.h>
#define avtStreamlineIC avtLCSIC

#include <avtExtents.h>
#include <avtMatrix.h>
#include <avtParallel.h>
#include <avtCallback.h>

#include <avtOriginatingSource.h>
#include <avtGradientExpression.h>
#include <vtkVisItScalarTree.h>

#include <VisItException.h>
#include <ImproperUseException.h>

#include <vtkMath.h>
#include <vtkUniformGrid.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkImageReslice.h>
#include <vtkMatrix4x4.h>
#include <vtkImageGaussianSmooth.h>

#ifdef PARALLEL
#include <mpi.h>
#include <avtParallel.h>
#include <vtkDataSetReader.h>
#include <vtkDataSetWriter.h>
#include <vtkCharArray.h>
#endif

#include <iostream>
#include <limits>
#include <cmath>

template <class T1, class T2, class Pred = std::greater<T2> >
struct sort_pair_max_second {
    bool operator()(const std::pair<T1,T2>&left, const std::pair<T1,T2>&right) {
        Pred p;
        return p(left.second, right.second);
    }
};

template <class T1, class T2, class Pred = std::less<T2> >
struct sort_pair_min_second {
    bool operator()(const std::pair<T1,T2>&left, const std::pair<T1,T2>&right) {
        Pred p;
        return p(left.second, right.second);
    }
};


// ****************************************************************************
//  Method: avtLCSFilter::NativeMeshSingleCalc
//
//  Purpose:
//      Computes the FTLE and similar values that are neighbor
//      independent after the particles have been advected.
//
//  Programmer: Hari Krishnan
//  Creation:   December 5, 2011
//
// ****************************************************************************

void avtLCSFilter::NativeMeshSingleCalc(std::vector<avtIntegralCurve*> &ics)
{
    //accumulate all of the points then do jacobian?
    //or do jacobian then accumulate?
    //picking the first.

    double minv   =  std::numeric_limits<double>::max();
    double maxv   = -std::numeric_limits<double>::max();
    int    offset = 0;

    avtDataTree_p outTree =
      MultiBlockSingleCalc(GetInputDataTree(), ics, offset, minv, maxv);

    // When data is replicated on all processors only the root
    // processor needs to pass along the results to a plot. Except as
    // noted below.
    if (GetInput()->GetInfo().GetAttributes().DataIsReplicatedOnAllProcessors())
      if (PAR_Rank() != 0)
        // When running in parallel and if the LCS operator is sending
        // its data to an IC operator then the resulting data must be
        // replicated on all processors so that all seeds are
        // advected. Otherwise only the root proc needs to have the
        // resulting data so create a blank data tree.
        if( !replicateData )
          outTree = new avtDataTree();

    SetOutputDataTree(outTree);

    avtDataAttributes &dataatts = GetOutput()->GetInfo().GetAttributes();
    avtExtents* e = dataatts.GetThisProcsActualDataExtents();

    double range[6];
    range[0] = minv;
    range[1] = maxv;
    range[2] = minv;
    range[3] = maxv;
    range[4] = minv;
    range[5] = maxv;
    e->Set(range);

    e = dataatts.GetThisProcsOriginalDataExtents();
    e->Set(range);

    e = dataatts.GetThisProcsActualSpatialExtents();
    e->Set(global_bounds);
    e = dataatts.GetThisProcsOriginalSpatialExtents();
    e->Set(global_bounds);
}


// ****************************************************************************
//  Method: avtLCSFilter::MultiBlockSingleCalc
//
//  Purpose:
//      Computes the FTLE and similar values that are neighbor
//      independent for the whole data set, using the final particle
//      locations, at the blocks native mesh resolution.
//
//  Programmer: Hari Krishnan
//  Creation:   December 5, 2011
//
// ****************************************************************************

avtDataTree_p
avtLCSFilter::MultiBlockSingleCalc( avtDataTree_p inDT,
                                    std::vector<avtIntegralCurve*> &ics,
                                    int &offset, double &minv, double &maxv )
{
    if (*inDT == NULL)
        return 0;

    int nc = inDT->GetNChildren();

    if (nc < 0 && !inDT->HasData())
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

        vtkDataSet *out_ds =
          SingleBlockSingleCalc( in_ds, ics, offset, dom, minv, maxv );
        avtDataTree_p rv = new avtDataTree(out_ds, dom, label);
        out_ds->Delete();
        return rv;
    }
    else
    {
      //
      // there is more than one input dataset to process
      // and we need an output datatree for each
      //
      avtDataTree_p *outDT = new avtDataTree_p[nc];
      for (int j = 0; j < nc; j++)
      {
          if (inDT->ChildIsPresent(j))
            outDT[j] = MultiBlockSingleCalc( inDT->GetChild(j), ics, 
                                                    offset, minv, maxv );
          else
            outDT[j] = NULL;
      }
      avtDataTree_p rv = new avtDataTree(nc, outDT);
      delete [] outDT;
      return rv;
    }
}


// ****************************************************************************
//  Method: avtLCSFilter::SingleBlockSingleCalc
//
//  Purpose:
//      Computes the FTLE and similar values that are neighbor
//      independent for a single block of a data set, using the final
//      particle locations, at the blocks native mesh resolution.
//
//  Arguments:
//      in_ds   The block to calculate the value on
//      ics     The list of particles for all blocks on this MPI task.
//      domain  The domain number of in_ds
//      minv    The minimum value (output)
//      maxv    The maximum value (output)
//
//  Returns:    The new version of in_ds that includes the scalar
//              variable.  The calling function is responsible for
//              dereferencing this VTK object.
//
//  Programmer: Hari Krishnan
//  Creation:   December 5, 2011
//
//  Modifications:
//
//    Hank Childs, Tue Feb  5 08:12:33 PST 2013
//    Fix parallelization bug and memory leak.
//
// ****************************************************************************

vtkDataSet *
avtLCSFilter::SingleBlockSingleCalc( vtkDataSet *in_ds,
                                     std::vector<avtIntegralCurve*> &ics,
                                     int &offset, int domain,
                                     double &minv, double &maxv )
{

    if (GetInput()->GetInfo().GetAttributes().DataIsReplicatedOnAllProcessors())
      std::cerr << __FUNCTION__ << "  " << PAR_Rank() << "  "
                << "DataIsReplicatedOnAllProcessors " << std::endl;

    // Variable name and number of points.
    std::string var = outVarRoot + outVarName;

    int nTuples = in_ds->GetNumberOfPoints();

    // Storage for the points and times
    std::vector<avtVector> remapPoints(nTuples*nAuxPts);
    std::vector<double>    remapTimes(nTuples*nAuxPts);

    // Zero out the points.
    for(size_t i = 0; i < remapPoints.size(); ++i)
    {
      remapPoints[i] = avtVector(0,0,0);
      remapTimes[i] = 0;
    }

    // ARS - This code does nothing of use that I can see. The
    // remapPoints just need to be zeroed out.

    // if (GetInput()->GetInfo().GetAttributes().DataIsReplicatedOnAllProcessors())
    // {
    //     // The parallel synchronization for when data is replicated involves
    //     // a sum across all processors.  So we want remapPoints to have the
    //     // location of the particle on one processor, and zero on the rest.
    //     // Do that here.
    //     // Special care is needed for the case where the particle never
    //     // advected.  Then we need to put the initial location on just one
    //     // processor.  We do this on rank 0.
    //     std::vector<int> iHavePoint(nTuples*nAuxPts, 0);
    //     std::vector<int> anyoneHasPoint;

    //     for (size_t i = 0; i < ics.size(); ++i)
    //     {
    //         size_t index = ics[i]->id;
    //         size_t l = (index-offset);

    //         if(l < remapPoints.size()) ///TODO: l >=0 is always true
    //         {
    //             iHavePoint[l] = 1;
    //         }
    //     }

    //     UnifyMaximumValue(iHavePoint, anyoneHasPoint);
    //     avtVector zero(0,0,0);

    //     for (size_t i = 0; i < remapPoints.size(); ++i)
    //         if (PAR_Rank() == 0 && !anyoneHasPoint[i])
    //             remapPoints[i] = seedPoints.at(offset + i);
    //         else
    //             remapPoints[i] = zero;
    // }
    // else
    // {
    //   //copy the original seed points
    //   for(size_t i = 0; i < remapPoints.size(); ++i)
    //     remapPoints[i] = seedPoints.at(offset + i);
    // }

    // The processor has a partial set of the curves so some values in
    // remapPoint will be zero.
    for(size_t i = 0; i < ics.size(); ++i)
    {
        size_t index = ics[i]->id;
        size_t l = (index-offset);

        if(l < remapPoints.size())
        {
          if( atts.GetOperationType() == LCSAttributes::EigenValue ||
              atts.GetOperationType() == LCSAttributes::EigenVector ||
              atts.GetOperationType() == LCSAttributes::Lyapunov )
          {
            if( doTime )
              remapTimes.at(l) = ((avtLCSIC*)ics[i])->GetTime();
            else if( doDistance )
              remapTimes.at(l) = ((avtLCSIC*)ics[i])->GetDistance();

            remapPoints.at(l) = ((avtLCSIC*)ics[i])->GetEndPoint();
          }
          else
          {
            double ave = 0;

            if( ((avtLCSIC*)ics[i])->GetNumSteps() )
              remapPoints.at(l) =
                avtVector( ((avtLCSIC*)ics[i])->GetTime(),
                           ((avtLCSIC*)ics[i])->GetDistance(),
                           (((avtLCSIC*)ics[i])->GetSummation0() /
                            (double) ((avtLCSIC*)ics[i])->GetNumSteps()) );
            else
              remapPoints.at(l) =
                avtVector( ((avtLCSIC*)ics[i])->GetTime(),
                           ((avtLCSIC*)ics[i])->GetDistance(),
                           0 );
          }
        }
    }

    //done with offset, increment it for the next call to this
    //function.
    offset += nTuples;

    //create new instance from old.
    vtkDataSet* out_grid = in_ds->NewInstance();
    out_grid->ShallowCopy(in_ds);

    //use static function in avtGradientExpression to calculate
    //gradients.  since this function only does scalar, break our
    //vectors into scalar components and calculate one at a time.
    vtkDoubleArray *outputArray = vtkDoubleArray::New();
    outputArray->SetName(var.c_str());
    if( atts.GetOperationType() == LCSAttributes::EigenVector )
      outputArray->SetNumberOfComponents(3);
    else
      outputArray->SetNumberOfComponents(1);
    outputArray->SetNumberOfTuples(nTuples);
    out_grid->GetPointData()->AddArray(outputArray);

    vtkDoubleArray *workingArray = vtkDoubleArray::New();
    workingArray->SetName("workingArray");
    workingArray->SetNumberOfComponents(1);
    workingArray->SetNumberOfTuples(nTuples);
    out_grid->GetPointData()->AddArray(workingArray);
    out_grid->GetPointData()->SetActiveScalars("workingArray");

    if( atts.GetOperationType() == LCSAttributes::EigenValue ||
        atts.GetOperationType() == LCSAttributes::EigenVector ||
        atts.GetOperationType() == LCSAttributes::Lyapunov )
    {
      // Save the times/distances so that points that do not fully
      // advect can be culled.
      for(size_t j = 0; j < (size_t)nTuples; ++j)
        outputArray->SetTuple1(j, remapTimes[j]);

      // remapTimes does not contain all of the values only the
      // ones for the integral curves on this processor. So sum
      // all of the values across all of the processors.
      if (GetInput()->GetInfo().GetAttributes().DataIsReplicatedOnAllProcessors())
      {
        double *newvals = new double[nTuples];
        double *origvals = (double *) outputArray->GetVoidPointer(0);
        SumDoubleArrayAcrossAllProcessors(origvals, newvals, nTuples);
        // copy newvals back into origvals
        memcpy(origvals, newvals, nTuples*sizeof(double));
        delete [] newvals;
      }

      // Calculate the jacobian.
      vtkDataArray* jacobian[3];
    
      // No auxiliary grid so use the values from the grid.
      if( auxIdx == LCSAttributes::None )
      {
        for(int i = 0; i < 3; ++i)
        {
          for(size_t j = 0; j < (size_t)nTuples; ++j)
            workingArray->SetTuple1(j, remapPoints[j][i]);

          // remapPoints does not contain all of the values only the
          // ones for the integral curves on this processor. So sum
          // all of the values across all of the processors.
          if (GetInput()->GetInfo().GetAttributes().DataIsReplicatedOnAllProcessors())
          {
            double *newvals = new double[nTuples];
            double *origvals = (double *) workingArray->GetVoidPointer(0);
            SumDoubleArrayAcrossAllProcessors(origvals, newvals, nTuples);
            // copy newvals back into origvals
            memcpy(origvals, newvals, nTuples*sizeof(double));
            delete [] newvals;
          }
          
          jacobian[i] =
            avtGradientExpression::CalculateGradient(out_grid, var.c_str());
        }
      }

      // Auxiliary grid so use the values from the curves.
      else //if( auxIdx == LCSAttributes::TwoDim ||
           //    auxIdx == LCSAttributes::ThreeDim )
      {
        double delta = 1.0 / (2.0 * auxSpacing);

        double *dx = new double[nTuples];
        double *dy = new double[nTuples];
        double *dz = new double[nTuples];

        for(int i = 0; i < 3; ++i)
        {
          jacobian[i] =
            avtExpressionDataTreeIterator::CreateArrayFromMesh(in_ds);
          jacobian[i]->SetNumberOfComponents(3);
          jacobian[i]->SetNumberOfTuples(nTuples);

          // Do the x, y, and possibly the z component.
          if( i <= auxIdx )
          {
            for(size_t j = 0, k = 0; j < nTuples; ++j, k+=nAuxPts)
            {
              dx[j] = (remapPoints[k+1][i] - remapPoints[k+0][i]) * delta;
              dy[j] = (remapPoints[k+3][i] - remapPoints[k+2][i]) * delta;

              if( auxIdx == LCSAttributes::ThreeDim )
                dz[j] = (remapPoints[k+5][i] - remapPoints[k+4][i]) * delta;
              else
                dz[j] = 0;
            }

            // remapPoints does not contain all of the values only the
            // ones for the integral curves on this processor. So sum
            // all of the values across all of the processors.
            if (GetInput()->GetInfo().GetAttributes().DataIsReplicatedOnAllProcessors())
            {
              double *dd[3] = {dx, dy, dz};

              for(size_t j = 0; j < 3; ++j)
              {
                double *newvals = new double[nTuples];
                SumDoubleArrayAcrossAllProcessors(dd[j], newvals, nTuples);
                // copy newvals back into origvals
                memcpy(dd[j], newvals, nTuples*sizeof(double));
                delete [] newvals;
              }
            }

            for(size_t j = 0; j < nTuples; ++j)
              jacobian[i]->SetTuple3(j, dx[j], dy[j], dz[j]);
          }
          // For a 2D auxiliary grid just zero out the z grid. No need
          // to sum across all processors as this sets all values.
          else
          {
            for (size_t j = 0; j < nTuples; ++j)
              jacobian[i]->SetTuple3(j, 0., 0., 1.);
          }
        }

        delete dx;
        delete dy;
        delete dz;
      }

      //now have the jacobian - 3 arrays with 3 workingArrays.
      if( atts.GetOperationType() == LCSAttributes::EigenValue )
        ComputeEigenValues(jacobian, outputArray);
      else if( atts.GetOperationType() == LCSAttributes::EigenVector )
      {
          ComputeEigenVectors(jacobian, workingArray, outputArray);
          
          bool clv = clampLogValues;
          clampLogValues = false;

          LCSAttributes::EigenComponent ec = eigenComponent;
          
          // Shrink lines - attracting
          if( eigenComponent == LCSAttributes::Largest )
          {
            // Get the smallest eigen values to go with the largest
            // eigen vectors.
            eigenComponent = LCSAttributes::Smallest;
            ComputeLyapunovExponent(jacobian, workingArray);
            
            // Get the locations of the minimal eigen values as seed
            // points. Note for incompressible flow the minimal
            // eigen value locations will be the same as the
            // maximial locations.
            GetSeedPoints( out_grid, false );
          }
          // Stretch lines - repelling
          else //if( eigenComponent == LCSAttributes::Smallest )
          {
            // Get the largest eigen values to go with the smallest
            // eigen vectors.
            eigenComponent = LCSAttributes::Largest;
            ComputeLyapunovExponent(jacobian, workingArray);
            
            // Get the locations of the maximal eigen values as seed
            // points. Note for incompressible flow the maximal
            // eigen value locatations will be the same as the
            // minimial locations.
            GetSeedPoints( out_grid, true );
          }

          clampLogValues = clv;
          eigenComponent = ec;              
      }
      else //if( atts.GetOperationType() == LCSAttributes::Lyapunov )
        ComputeLyapunovExponent(jacobian, outputArray);

      jacobian[0]->Delete();
      jacobian[1]->Delete();
      jacobian[2]->Delete();
    }

    // The value stored in the points is the arc length, integration
    // time, and average distance form the seed. So just move it into
    // the output array.
    else if( atts.GetOperatorType() == LCSAttributes::BaseValue )
    {
      int index = (atts.GetOperationType()-LCSAttributes::IntegrationTime);

      for(size_t j = 0; j < (size_t)nTuples; ++j)
        outputArray->SetTuple1(j, remapPoints[j][index]);

      // remapPoints does not contain all of the values only the ones
      // for the integral curves on this processor. So sum all of the
      // values across all of the processors.
      if (GetInput()->GetInfo().GetAttributes().DataIsReplicatedOnAllProcessors())
      {
        double *newvals = new double[nTuples];
        double *origvals = (double *) outputArray->GetVoidPointer(0);
        SumDoubleArrayAcrossAllProcessors(origvals, newvals, nTuples);
        // copy newvals back into origvals
        memcpy(origvals, newvals, nTuples*sizeof(double));
        delete [] newvals;
      }
    }
    else if( atts.GetOperatorType() == LCSAttributes::Gradient )
    {
      int index = (atts.GetOperationType()-LCSAttributes::IntegrationTime);
      
      // The base value is used to clamp the log values to be only
      // positive or both positive and negative.
      double baseValue;
      
      if (clampLogValues == true )
        baseValue = 1.0;
      else
        baseValue = std::numeric_limits<double>::epsilon();
      
      for (size_t l = 0; l < nTuples; l++)
        workingArray->SetTuple1(l, remapPoints[l][index]);

      // remapPoints does not contain all of the values only the
      // ones for the integral curves on this processor. So sum
      // all of the values across all of the processors.
      if (GetInput()->GetInfo().GetAttributes().DataIsReplicatedOnAllProcessors())
      {
        double *newvals = new double[nTuples];
        double *origvals = (double *) workingArray->GetVoidPointer(0);
        SumDoubleArrayAcrossAllProcessors(origvals, newvals, nTuples);
        // copy newvals back into origvals
        memcpy(origvals, newvals, nTuples*sizeof(double));
        delete [] newvals;
      }
      
      vtkDataArray* gradient =
        avtGradientExpression::CalculateGradient(out_grid, var.c_str());
      
      for (size_t l = 0; l < nTuples; l++)
      {
        double *grad = gradient->GetTuple3(l);
        
        double lambda = baseValue;
        lambda = std::max( lambda, grad[0]*grad[0] );
        lambda = std::max( lambda, grad[1]*grad[1] );
        lambda = std::max( lambda, grad[2]*grad[2] );
        lambda = log( sqrt( lambda ) );
        
        if( doTime )
          lambda /= maxTime;
        else if( doDistance )
          lambda /= maxDistance;
        
        outputArray->SetTuple1(l, lambda);
      }
      
      gradient->Delete();
    }

    if( atts.GetOperationType() == LCSAttributes::EigenVector )
    {
      minv = 0;
      maxv = 1;

      // for(int i = 0; i < nTuples; ++i)
      // {
      //   double *vals = outputArray->GetTuple3(i);
      //   double mag = sqrt(vals[0]*vals[0]+vals[1]*vals[1]+vals[2]*vals[2]);

      //   minv = std::min(mag, minv);
      //   maxv = std::max(mag, maxv);
      // }
    }
    else
    {
      for(int i = 0; i < nTuples; ++i)
      {
        minv = std::min(outputArray->GetTuple1(i), minv);
        maxv = std::max(outputArray->GetTuple1(i), maxv);
      }
    }

    outputArray->Delete();
    workingArray->Delete();

    // Set the vectors to be the active data
    if( atts.GetOperationType() == LCSAttributes::EigenVector )
      out_grid->GetPointData()->SetActiveVectors(var.c_str());
    else
      out_grid->GetPointData()->SetActiveScalars(var.c_str());

    // Remove the working array.
    out_grid->GetPointData()->RemoveArray("workingArray");


    bool storeResult = true;

    // When data is replicated on all processors only the root
    // processor needs to pass along the results to a plot. Except as
    // noted below.
    if (GetInput()->GetInfo().GetAttributes().DataIsReplicatedOnAllProcessors())
      if (PAR_Rank() != 0)
        // When running in parallel and if the LCS operator is sending
        // its data to an IC operator then the resulting data must be
        // replicated on all processors so that all seeds are
        // advected. Otherwise only the root proc needs to have the
        // resulting data so do not store the results.
        if( !replicateData )
          storeResult = false;

    if( storeResult )
    {
        //Store this dataset in Cache for next time.
        std::string str = CreateCacheString();
        StoreArbitraryVTKObject(SPATIAL_DEPENDENCE | DATA_DEPENDENCE,
                                outVarName.c_str(), domain, -1,
                                str.c_str(), out_grid);
    }

    // Calling function must free this.
    return out_grid;
}


// ****************************************************************************
//  Method: avtLCSFilter::RectilinearGridSingleCalc
//
//  Purpose:
//      Computes the FTLE and similar values that are neighbor
//      independent on a rectilinear grid.
//
//  Programmer: Hari Krishnan
//  Creation:   December 5, 2011
//
//    Mark C. Miller, Wed Aug 22 19:22:40 PDT 2012
//    Fix leak of all_indices, index_counts, all_points, result_counts on 
//    rank 0 (root).
// ****************************************************************************

void
avtLCSFilter::RectilinearGridSingleCalc(std::vector<avtIntegralCurve*> &ics)
{
    //variable name.
    std::string var = outVarRoot + outVarName;

    size_t nTuples =
          global_resolution[0] * global_resolution[1] * global_resolution[2];

    //algorithm sends index to global datastructure as well as end points.
    //Send List of index into global array to rank 0
    //Send end positions into global array to rank 0

    size_t nics = ics.size();

    //loop over all the integral curves and add it back to the
    //original list of seeds.
    intVector   indices(nics);
    doubleVector  times(nics);
    doubleVector points(nics*3);

    for(size_t i=0, j=0; i<nics; ++i, j+=3)
    {
        indices[i] = ics[i]->id;
        if( doTime )
          times[i] = ((avtLCSIC*)ics[i])->GetTime();
        else if( doDistance )
          times[i] = ((avtLCSIC*)ics[i])->GetDistance();

        // if( ics[i]->id == 749 )
        //   std::cerr << __FUNCTION__ << "  " << __LINE__ << "  "
        //          << PAR_Rank() << " indice " << ics[i]->id
        //          << "  time  " << ((avtLCSIC*)ics[i])->GetTime()
        //          << "  status  " << ics[i]->status
        //          << std::endl;

        if( atts.GetOperationType() == LCSAttributes::EigenValue ||
            atts.GetOperationType() == LCSAttributes::EigenVector ||
            atts.GetOperationType() == LCSAttributes::Lyapunov )
        {
          avtVector end_point = ((avtLCSIC*)ics[i])->GetEndPoint();
          
          points[j+0] = end_point[0];
          points[j+1] = end_point[1];
          points[j+2] = end_point[2];
        }
        else
        {
          points[j+0] = ((avtLCSIC*)ics[i])->GetTime();
          points[j+1] = ((avtLCSIC*)ics[i])->GetDistance();

          if( ((avtLCSIC*)ics[i])->GetNumSteps() )
            points[j+2] = (((avtLCSIC*)ics[i])->GetSummation0() /
                           (double) ((avtLCSIC*)ics[i])->GetNumSteps());
          else
            points[j+2] = 0;
        }
    }

    int* all_indices = 0;
    int* index_counts = 0;

    double* all_times = 0;
    int *time_counts = 0;

    double* all_points = 0;
    int *point_counts = 0;

    Barrier();

    CollectIntArraysOnRootProc(all_indices, index_counts,
                               &indices.front(), (int)indices.size());

    CollectDoubleArraysOnRootProc(all_times, time_counts,
                                  &times.front(), (int)times.size());

    CollectDoubleArraysOnRootProc(all_points, point_counts,
                                  &points.front(), (int)points.size());

    Barrier();

    // Dataset using all ICs.
    vtkDataSet* ds = 0;

    //min and max values over all datasets of the tree.
    double minv =  std::numeric_limits<double>::max();
    double maxv = -std::numeric_limits<double>::max();

    // The root proc root should now have index info and all
    // matching time and end positions.
    if(PAR_Rank() == 0)
    {
        //rank 0
        int par_size = PAR_Size();
        size_t total = 0;
        for(int i = 0; i < par_size; ++i)
        {
            if( index_counts[i]   ==  time_counts[i] &&
                index_counts[i]*3 == point_counts[i] )
            {
                total += index_counts[i];
            }
            else
            {
                EXCEPTION1(VisItException,
                           "Index count does not match the result count." );
            }
        }

        if( total != nTuples*nAuxPts )
        {
          EXCEPTION1(VisItException,
                     "Total count does not match the tuple count." );
        }

        // Update remapPoints with new value bounds from integral
        // curves.  This puts the data into the arrays based on the
        // curve index.
        std::vector<double> remapTimes(nTuples*nAuxPts);
        std::vector<avtVector> remapPoints(nTuples*nAuxPts);
      
        for(size_t j = 0, k = 0; j < nTuples*nAuxPts; ++j, k += 3)
        {
            size_t index = all_indices[j];

            if(index < nTuples*nAuxPts)
            {
              remapTimes[index] = all_times[j];
              remapPoints[index].set( all_points[k+0],
                                      all_points[k+1],
                                      all_points[k+2]);
            }
            else
            {
              EXCEPTION1(VisItException,
                         "More integral curves were generatated than "
                         "grid points." );
            }
        }

        // Now create a rectilinear grid.
        vtkRectilinearGrid* rect_grid = vtkRectilinearGrid::New();

        vtkDoubleArray* lxcoord = vtkDoubleArray::New();
        vtkDoubleArray* lycoord = vtkDoubleArray::New();
        vtkDoubleArray* lzcoord = vtkDoubleArray::New();

        rect_grid->SetDimensions(global_resolution);

        lxcoord->SetNumberOfTuples(global_resolution[0]);
        for (int i = 0; i < global_resolution[0]; i++)
        {
            double pcnt = 0;
            if (global_resolution[0] > 1)
                pcnt = ((double)i)/((double)global_resolution[0]-1);
            lxcoord->SetTuple1(i, global_bounds[0]*(1.0-pcnt) + global_bounds[1]*pcnt);
        }

        lycoord->SetNumberOfTuples(global_resolution[1]);
        for (int i = 0; i < global_resolution[1]; i++)
        {
            double pcnt = 0;
            if (global_resolution[1] > 1)
                pcnt = ((double)i)/((double)global_resolution[1]-1);
            lycoord->SetTuple1(i, global_bounds[2]*(1.0-pcnt) + global_bounds[3]*pcnt);
        }

        lzcoord->SetNumberOfTuples(global_resolution[2]);
        for (int i = 0; i < global_resolution[2]; i++)
        {
            double pcnt = 0;
            if (global_resolution[2] > 1)
                pcnt = ((double)i)/((double)global_resolution[2]-1);
            lzcoord->SetTuple1(i, global_bounds[4]*(1.0-pcnt) + global_bounds[5]*pcnt);
        }

        rect_grid->SetXCoordinates(lxcoord);
        rect_grid->SetYCoordinates(lycoord);
        rect_grid->SetZCoordinates(lzcoord);

        //cleanup
        lxcoord->Delete();
        lycoord->Delete();
        lzcoord->Delete();

        //now create the output and working array
        vtkDoubleArray *outputArray = vtkDoubleArray::New();
        outputArray->SetName(var.c_str());
        if( atts.GetOperationType() == LCSAttributes::EigenVector )
          outputArray->SetNumberOfComponents(3);
        else
          outputArray->SetNumberOfComponents(1);
        outputArray->SetNumberOfTuples(nTuples);
        rect_grid->GetPointData()->AddArray(outputArray);

        vtkDoubleArray *workingArray = vtkDoubleArray::New();
        workingArray->SetName("workingArray");
        workingArray->SetNumberOfComponents(1);
        workingArray->SetNumberOfTuples(nTuples);
        rect_grid->GetPointData()->AddArray(workingArray);
        rect_grid->GetPointData()->SetActiveScalars("workingArray");
    
        //calculate jacobian in parts (x,y,z).
        if( atts.GetOperationType() == LCSAttributes::EigenValue ||
            atts.GetOperationType() == LCSAttributes::EigenVector ||
            atts.GetOperationType() == LCSAttributes::Lyapunov )
        {
          for (size_t l = 0; l < nTuples; l++)
            outputArray->SetTuple1(l, remapTimes[l]);

          vtkDataArray* jacobian[3];

          if( auxIdx == LCSAttributes::None )
          {
            for(int i = 0; i < 3; ++i)
            {
              for (size_t l = 0; l < nTuples; l++)
                workingArray->SetTuple1(l, remapPoints[l][i]);

              jacobian[i] =
                avtGradientExpression::CalculateGradient(rect_grid, var.c_str());
            }
          }
          else //if( auxIdx == LCSAttributes::TwoDim ||
               //    auxIdx == LCSAttributes::ThreeDim )
          {
            double delta = 1.0 / (2.0 * auxSpacing);
            
            for(int i = 0; i < 3; ++i)
            {
              jacobian[i] =
                avtExpressionDataTreeIterator::CreateArrayFromMesh(rect_grid);
              jacobian[i]->SetNumberOfComponents(3);
              jacobian[i]->SetNumberOfTuples(nTuples);
              
              double dx, dy, dz;
              
              // Do the x, y, and possibly the z component.
              if( i <= auxIdx )
              {
                for(size_t j = 0, k = 0; j < nTuples; ++j, k+=nAuxPts)
                {
                  dx = (remapPoints[k+1][i] - remapPoints[k+0][i]) * delta;
                  dy = (remapPoints[k+3][i] - remapPoints[k+2][i]) * delta;
                  
                  if( auxIdx == LCSAttributes::ThreeDim )
                    dz = (remapPoints[k+5][i] - remapPoints[k+4][i]) * delta;
                  else
                    dz = 0;
                  
                  // if( PID == j )
                  //   std::cerr << "Jacobian "
                  //          << dx << "  "  << dy << "  "  << dz << "  "
                  //          << std::endl;
                    
                  jacobian[i]->SetTuple3(j, dx, dy, dz);
                }
              }
              else
              {
                for (size_t j = 0; j < nTuples; ++j)
                  jacobian[i]->SetTuple3(j, 0., 0., 1.);
              }
            }
          }

          if( atts.GetOperationType() == LCSAttributes::EigenValue )
            ComputeEigenValues(jacobian, outputArray);
          else if( atts.GetOperationType() == LCSAttributes::EigenVector )
          {
            ComputeEigenVectors(jacobian, workingArray, outputArray);

            bool clv = clampLogValues;
            clampLogValues = false;

            LCSAttributes::EigenComponent ec = eigenComponent;

            // Shrink lines - attracting
            if( eigenComponent == LCSAttributes::Largest )
            {
              // Get the smallest eigen values to go with the largest
              // eigen vectors.
              eigenComponent = LCSAttributes::Smallest;
              ComputeLyapunovExponent(jacobian, workingArray);

              // Get the locations of the minimal eigen values as seed
              // points. Note for incompressible flow the minimal
              // eigen value locations will be the same as the
              // maximial locations.
              GetSeedPoints( rect_grid, false );
            }
            // Stretch lines - repelling
            else //if( eigenComponent == LCSAttributes::Smallest )
            {
              // Get the largest eigen values to go with the smallest
              // eigen vectors.
              eigenComponent = LCSAttributes::Largest;
              ComputeLyapunovExponent(jacobian, workingArray);

              // Get the locations of the maximal eigen values as seed
              // points. Note for incompressible flow the maximal
              // eigen value locatations will be the same as the
              // minimial locations.
              GetSeedPoints( rect_grid, true );
            }

            clampLogValues = clv;
            eigenComponent = ec;              
          }
          else if( atts.GetOperationType() == LCSAttributes::Lyapunov )
            ComputeLyapunovExponent(jacobian, outputArray);

          jacobian[0]->Delete();
          jacobian[1]->Delete();
          jacobian[2]->Delete();
        }

        // The value stored in the points is the arc length, integration
        // time, and average distance form the seed. So just move it into
        // the output array.
        else if( atts.GetOperatorType() == LCSAttributes::BaseValue )
        {
          int index = (atts.GetOperationType()-LCSAttributes::IntegrationTime);

          // std::cerr << __FUNCTION__ << "  " << __LINE__ << "  " << index
          //        << std::endl;

          for (size_t l = 0; l < nTuples; l++)
            outputArray->SetTuple1(l, remapPoints[l][index]);
        }

        else if( atts.GetOperatorType() == LCSAttributes::Gradient )
        {
          int index = (atts.GetOperationType()-LCSAttributes::IntegrationTime);
          
          // The base value is used to clamp the log values to be only
          // positive or both positive and negative.
          double baseValue;

          if (clampLogValues == true )
            baseValue = 1.0;
          else
            baseValue = std::numeric_limits<double>::epsilon();

          for (size_t l = 0; l < nTuples; l++)
            workingArray->SetTuple1(l, remapPoints[l][index]);

          vtkDataArray* gradient =
            avtGradientExpression::CalculateGradient(rect_grid, var.c_str());

          for (size_t l = 0; l < nTuples; l++)
          {
            double *grad = gradient->GetTuple3(l);

            double lambda = baseValue;
            lambda = std::max( lambda, grad[0]*grad[0] );
            lambda = std::max( lambda, grad[1]*grad[1] );
            lambda = std::max( lambda, grad[2]*grad[2] );
            lambda = log( sqrt( lambda ) );

            if( doTime )
              lambda /= maxTime;
            else if( doDistance )
              lambda /= maxDistance;

            outputArray->SetTuple1(l, lambda);
          }

          gradient->Delete();
        }

        if( atts.GetOperationType() == LCSAttributes::EigenVector )
        {
          minv = 0;
          maxv = 1;

          // for(int i = 0; i < nTuples; ++i)
          // {
          //   double *vals = outputArray->GetTuple3(i);
          //   double mag = sqrt(vals[0]*vals[0]+vals[1]*vals[1]+vals[2]*vals[2]);

          //   minv = std::min(mag, minv);
          //   maxv = std::max(mag, maxv);
          // }
        }
        else
        {
          for(size_t l = 0; l < nTuples; ++l)
          {
            minv = std::min(outputArray->GetTuple1(l), minv);
            maxv = std::max(outputArray->GetTuple1(l), maxv);
          }
        }

        outputArray->Delete();
        workingArray->Delete();

        // Set the vectors to be the active data
        if( atts.GetOperationType() == LCSAttributes::EigenVector )
          rect_grid->GetPointData()->SetActiveVectors(var.c_str());
        else
          rect_grid->GetPointData()->SetActiveScalars(var.c_str());

        // Remove the working array.
        rect_grid->GetPointData()->RemoveArray("workingArray");

        if (all_indices)  delete [] all_indices;
        if (index_counts) delete [] index_counts;

        if (all_times)  delete [] all_times;
        if (time_counts) delete [] time_counts;

        if (all_points)   delete [] all_points;
        if (point_counts) delete [] point_counts;

        // Store this dataset in Cache for next time.
        std::string str = CreateCacheString();
        StoreArbitraryVTKObject(SPATIAL_DEPENDENCE | DATA_DEPENDENCE,
                                outVarName.c_str(), -1, -1,
                                str.c_str(), rect_grid);

        // Put the vtkDataset in the avtDataTree.
        int index = 0; // What does index mean in this context?
        avtDataTree* dt = new avtDataTree(rect_grid, index);
        int x = 0;
        dt->GetAllLeaves(x);

        SetOutputDataTree(dt);

        // Set atts.
        avtDataAttributes &dataatts = GetOutput()->GetInfo().GetAttributes();
        avtExtents* e = dataatts.GetThisProcsActualDataExtents();

        double range[6];
        range[0] = minv;
        range[1] = maxv;
        range[2] = minv;
        range[3] = maxv;
        range[4] = minv;
        range[5] = maxv;
        e->Set(range);

        ds = rect_grid;
    }
    
    // When running in parallel and if the LCS operator is sending its
    // data to an IC operator then the data must be replicated
    // (broadcast) on all processors so that all seeds are advected.
#ifdef PARALLEL
    if( replicateData )
    {
      // std::cerr << __FILE__ << "  " << __FUNCTION__ << "  " << __LINE__ << "  "
      //                <<  PAR_Rank() << "  replicating data on all processors"
      //                << std::endl;
      
      debug1 << "LCS: replicating data on all processors." << std::endl;

      // Setup for MPI communication
      int mpiMinTag = GetUniqueMessageTag();
      int mpiMaxTag = GetUniqueMessageTag();
      int mpiSizeTag = GetUniqueMessageTag();
      int mpiDataTag = GetUniqueMessageTag();
      
      // The root processor has the vtkDataSet so broadcast that out
      // to all other processors.
      if (PAR_Rank() == 0)
      {
        // Use a vtkDataSetWriter to dump out the dataset as a string
        // that will be broadcasted to the other processors.
        vtkDataSetWriter *writer = vtkDataSetWriter::New();
        writer->WriteToOutputStringOn();
        writer->SetFileTypeToBinary();
        
        writer->SetInputData(ds);
        writer->Write();

        // Get the string and it's size.
        int  size = writer->GetOutputStringLength();
        char *tmpstr = writer->RegisterAndGetOutputString();

        // Broadcast it out to all the other processors.
        for (int i = 1; i < PAR_Size(); i++)
        {
            MPI_Send(&minv, 1, MPI_DOUBLE, i, mpiMinTag, VISIT_MPI_COMM);
            MPI_Send(&maxv, 1, MPI_DOUBLE, i, mpiMaxTag, VISIT_MPI_COMM);

            MPI_Send(&size, 1, MPI_INT, i, mpiSizeTag, VISIT_MPI_COMM);
            MPI_Send(tmpstr, size, MPI_CHAR, i, mpiDataTag, VISIT_MPI_COMM);
        }

        delete [] tmpstr; // allocated by writer
        writer->Delete(); 
      }
      else //if (PAR_Rank() != 0)
      {
        // Receive the broadcasted string that makes up the vtkDataSet.
        MPI_Status stat;
        MPI_Status stat2;
        
        double minv = std::numeric_limits<double>::max();
        MPI_Recv(&minv, 1, MPI_DOUBLE, stat.MPI_SOURCE, mpiMinTag,
                 VISIT_MPI_COMM, &stat2);

        double maxv = -std::numeric_limits<double>::max();
        MPI_Recv(&maxv, 1, MPI_DOUBLE, stat.MPI_SOURCE, mpiMaxTag,
                 VISIT_MPI_COMM, &stat2);

        // Get the string size and the string.
        int size = 0;
        MPI_Recv(&size, 1, MPI_INT, stat.MPI_SOURCE, mpiSizeTag,
                 VISIT_MPI_COMM, &stat2);
        char *tmpstr = new char[size];
        MPI_Recv(tmpstr, size, MPI_CHAR, stat.MPI_SOURCE, mpiDataTag,
                 VISIT_MPI_COMM, &stat2);

        vtkCharArray *charArray = vtkCharArray::New();
        charArray->SetArray((char*)tmpstr, size, 1);

        // Use a vtkDataSetReader to read in the dataset as a string.
        vtkDataSetReader *reader = vtkDataSetReader::New(); 
        reader->ReadFromInputStringOn();
        reader->SetInputArray(charArray);
        reader->Update();

        vtkRectilinearGrid* rect_grid =
          (vtkRectilinearGrid *) reader->GetOutput();

        // Store this dataset in Cache for next time.
        std::string str = CreateCacheString();
        StoreArbitraryVTKObject(SPATIAL_DEPENDENCE | DATA_DEPENDENCE,
                                outVarName.c_str(), -1, -1,
                                str.c_str(), rect_grid);

        // Put the vtkDataset in the avtDataTree.
        int index = 0; // What does index mean in this context?
        avtDataTree* dt = new avtDataTree(rect_grid, index);
        int x = 0;
        dt->GetAllLeaves(x);

        SetOutputDataTree(dt);

        // Set atts.
        avtDataAttributes &dataatts = GetOutput()->GetInfo().GetAttributes();
        avtExtents* e = dataatts.GetThisProcsActualDataExtents();

        double range[6];
        range[0] = minv;
        range[1] = maxv;
        range[2] = minv;
        range[3] = maxv;
        range[4] = minv;
        range[5] = maxv;
        e->Set(range);

        delete [] tmpstr; // allocated above
        reader->Delete(); 
        charArray->Delete(); 
      }
    }

    // Not replicating the data so send a blank tree to the other
    // processors.
    else if(PAR_Rank() != 0)
    {
        debug1 << "LCS: data is only on the root processor." << std::endl;

        // std::cerr << __FILE__ << "  " << __FUNCTION__ << "  " << __LINE__ << "  "
        //        <<  PAR_Rank() << "  root processor only " << std::endl;
      

        avtDataTree* dt = new avtDataTree();
        SetOutputDataTree(dt);
    }
#endif
}


// ****************************************************************************
//  Method: avtLCSFilter::GetSeedPoints
//
//  Purpose:
//
//  Programmer: Allen Sanderson
//  Creation:   March 25, 2015
//
// ****************************************************************************

void
avtLCSFilter::GetSeedPoints( vtkDataSet *in_ds, bool getMax )
{
  bool getMin = !getMax;

  // double cx = (global_bounds[0]+global_bounds[1]) / 2.0;
  // double cy = (global_bounds[2]+global_bounds[3]) / 2.0;
  // double cz = (global_bounds[4]+global_bounds[5]) / 2.0;

  // double x = global_bounds[0];
  // double y = global_bounds[2];
  // double z = global_bounds[4];
  
  // double dx = ( (global_bounds[1] - global_bounds[0]) /
  //               (double) (global_resolution[0]-1) );
  
  // double dy = ( (global_bounds[3] - global_bounds[2]) /
  //               (double) (global_resolution[1]-1) );
  
  // double dz = ( (global_bounds[5] - global_bounds[4]) /
  //               (double) (global_resolution[2]-1) );
  
  // vtkImageData *image_ds = vtkImageData::New();

  // image_ds->SetDimensions(global_resolution);
  // image_ds->SetOrigin(x,y,z);
  // image_ds->SetSpacing(dx,dy,dz);
  // image_ds->ShallowCopy( in_ds );

  // // Extract a slice in the desired orientation
  // static double axialElements[16] = { 1, 0, 0, 0,
  //                                     0, 1, 0, 0,
  //                                     0, 0, 1, 0,
  //                                     0, 0, 0, 1 };
  
  // // Set the slice orientation
  // vtkMatrix4x4 *resliceAxes = vtkMatrix4x4::New();
  // resliceAxes->DeepCopy(axialElements);
  
  // // Set the point through which to slice
  // resliceAxes->SetElement(0, 3, cx);
  // resliceAxes->SetElement(1, 3, cy);
  // resliceAxes->SetElement(2, 3, cz);
  
  // vtkImageReslice *reslice = vtkImageReslice::New();
  // reslice->SetInputData( image_ds );
  // reslice->SetOutputDimensionality(2);
  // reslice->SetResliceAxes(resliceAxes);
  // reslice->SetInterpolationModeToCubic();
  // reslice->Update();

//          vtkImageGaussianSmooth *smooth = vtkImageGaussianSmooth::New();
//          smooth->SetInputData( reslice->GetOutput() );
//          smooth->SetDimensionality(2);
//          smooth->SetStandardDeviation(1, 1, 0);
// //       smooth->SetStandardDeviation(dx, dy, dz);
//          smooth->SetRadiusFactors(2.5, 2.5, 0);
//          smooth->Update();

//   vtkImageData *slice_ds = vtkImageData::New();
//   slice_ds->ShallowCopy(reslice->GetOutput());

//   resliceAxes->Delete();
//   reslice->Delete();
// //  smooth->Delete();

//   int dims[3];
//   slice_ds->SetOrigin(x,y,cz);
//   slice_ds->GetDimensions(dims);
//   slice_ds->GetOrigin(x,y,z);
//   slice_ds->GetSpacing(dx,dy,dz);

  std::cerr << "searching for seeds " << std::endl;

  vtkDataSet *slice_ds = in_ds;
  
  vtkDoubleArray *tmpArray =
    (vtkDoubleArray *) slice_ds->GetPointData()->GetScalars();

  int nTuples = tmpArray->GetNumberOfTuples();

  // Get te FTLE minimal and maximal values.
  double ftle;
  double minFTLE =  std::numeric_limits<double>::max();
  double maxFTLE = -std::numeric_limits<double>::max();

  double thresholdLimit = atts.GetThresholdLimit();
  double radialLimit = atts.GetRadialLimit();
  double boundaryLimit = atts.GetBoundaryLimit();
  double maxSeeds = atts.GetSeedLimit();
  
  for(size_t l = 0; l < nTuples; ++l)
  {
    ftle = tmpArray->GetTuple1( l );
    
    if( maxFTLE < ftle || ftle < minFTLE )
    {
      double *gridPt = slice_ds->GetPoint( l );
      
      bool inBounds = true;
      
      // Discard points near the boundaries
      for (int i=0, j=0; i<nDim; ++i, j+=2)
      {
        double t = ((gridPt[i]          - global_bounds[j]) /
                    (global_bounds[j+1] - global_bounds[j]));
        
        if( t < boundaryLimit || 1.0-boundaryLimit < t )
        {
          inBounds = false;
          break;
        }
      }
      
      if( inBounds )
      {
        if( maxFTLE < ftle )
          maxFTLE = ftle;
        else
          minFTLE = ftle;
      }
    }
  }
  
  double threshold;

  if( getMax )
    threshold = maxFTLE - (maxFTLE-minFTLE) * thresholdLimit;
  else //if( getMin )
    threshold = minFTLE + (maxFTLE-minFTLE) * thresholdLimit;

  std::cerr << "min/max " << minFTLE << "  " << maxFTLE << "  "
            << "threshold  " << threshold << std::endl;

  std::vector< std::pair< avtVector, double > > ptList;

  for(size_t l = 0; l < nTuples; ++l)
  {
    ftle = tmpArray->GetTuple1( l );

    if( (getMin && ftle < threshold) || (getMax && threshold < ftle) )          
    {
      double *gridPt = slice_ds->GetPoint( l );
      
      bool inBounds = true;
      
      // Discard points near the boundaries
      for (int i=0, j=0; i<nDim; ++i, j+=2)
      {
        double t = ((gridPt[i]          - global_bounds[j]) /
                    (global_bounds[j+1] - global_bounds[j]));
        
        if( t < boundaryLimit || 1.0-boundaryLimit < t )
        {
          inBounds = false;
          break;
        }
      }
      
      if( inBounds )
      {
        ptList.push_back( std::pair< avtVector,
                             double >( avtVector( gridPt[0],
                                                  gridPt[1],
                                                  gridPt[2] ),
                                       ftle ) );
      }
    }
  }
  
  // Sort the points either descending or ascending.
  if( getMax )
    std::sort( ptList.begin(), ptList.end(),
               sort_pair_max_second< avtVector, double >() ); 
  else //if( getMin )
    std::sort( ptList.begin(), ptList.end(),
               sort_pair_min_second< avtVector, double >() ); 

  std::vector< std::pair< avtVector, double > >::iterator iter =
    ptList.begin();

  // Start the first point and remove all those that are too close.
  while( iter != ptList.end() )
  {
    std::vector< std::pair< avtVector, double > >::iterator iter2 =
      ptList.end();
    
    --iter2;

    while( iter2 != iter )
    {
      if( avtVector( (*iter).first[0] - (*iter2).first[0],
                     (*iter).first[1] - (*iter2).first[1],
                     (*iter).first[2] - (*iter2).first[2] ).length() <
          radialLimit )
      {
        ptList.erase( iter2 );
      }
      
      --iter2;
    }
    
    ++iter;
  }
  
  if( getMax )
    std::cerr << "Have " << ptList.size() << " max seed points "<< std::endl;
  else
    std::cerr << "Have " << ptList.size() << " min seed points "<< std::endl;
  
  if( ptList.size() > maxSeeds ) ptList.resize( maxSeeds );

  vtkDoubleArray *seedPts = vtkDoubleArray::New();
  
  if( getMax )
    seedPts->SetName("Seed Points - Maximal");
  else
    seedPts->SetName("Seed Points - Minimal");

  // Set the number of components before setting the number of tuples
  // for proper memory allocation.
  seedPts->SetNumberOfComponents( 3 );
  seedPts->SetNumberOfTuples( ptList.size() );

  in_ds->GetFieldData()->AddArray(seedPts);

  for (int i = 0; i < ptList.size(); ++i)
  {
    std::cerr << ptList[i].second << "     "
              << ptList[i].first.x << "  "
              << ptList[i].first.y << "  "
              << ptList[i].first.z << "  "
              << std::endl;
    
    seedPts->SetTuple3(i,
                       ptList[i].first.x,
                       ptList[i].first.y,
                       ptList[i].first.z );
  }
  
  seedPts->Delete();

  // slice_ds->Delete();
  // image_ds->Delete();
}
