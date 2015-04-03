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

#include <iostream>
#include <limits>
#include <cmath>

// ****************************************************************************
//  Method: avtLCSFilter::CreateIterativeCalcDataTree
//
//  Purpose:
//      Create the output tree for the whole data set, using the 
//      input tree using the blocks native resolution.
//
//  Programmer: Allen Sanderson
//  Creation:   September 5, 2013
//
//  Modifications:
//
// ****************************************************************************

avtDataTree_p
avtLCSFilter::CreateIterativeCalcDataTree(avtDataTree_p inDT)
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
        //variable name.
        std::string var = outVarRoot + outVarName;

        //
        // there is only one dataset to process
        //
        vtkDataSet *in_ds = inDT->GetDataRepresentation().GetDataVTK();

        //create new instance from old.
        vtkDataSet* out_ds = in_ds->NewInstance();
        out_ds->ShallowCopy(in_ds);
        int nTuples = in_ds->GetNumberOfPoints();

        // Create storage for the components that need to be used
        // for calculating the exponent. All arrays but the exponent
        // array will be deleted when done.
        vtkDoubleArray *exponents = vtkDoubleArray::New();
        exponents->SetName(var.c_str());
        exponents->SetNumberOfTuples(nTuples);
        out_ds->GetPointData()->AddArray(exponents);
        // Will set the exponents to be the active scalars when
        // finished. In the mean time the component is the working
        // active scalars.
//      out_ds->GetPointData()->SetActiveScalars(var.c_str());

        for (size_t i = 0; i < (size_t)nTuples; i++)
          exponents->SetTuple1(i, std::numeric_limits<double>::min() );

        vtkDoubleArray *component = vtkDoubleArray::New();
        component->SetName("component");
        component->SetNumberOfTuples(nTuples);
        out_ds->GetPointData()->AddArray(component);
        out_ds->GetPointData()->SetActiveScalars("component");
        
        vtkDoubleArray *times = vtkDoubleArray::New();
        times->SetName("times");
        times->SetNumberOfTuples(nTuples);
        out_ds->GetPointData()->AddArray(times);

        //cleanup
        exponents->Delete();
        component->Delete();
        times->Delete();
        
        int dom = inDT->GetDataRepresentation().GetDomain();
        std::string label = inDT->GetDataRepresentation().GetLabel();
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
            outDT[j] = CreateIterativeCalcDataTree(inDT->GetChild(j));
          else
            outDT[j] = NULL;
      }
      avtDataTree_p rv = new avtDataTree(nc, outDT);
      delete [] outDT;
      return rv;
    }
}


// ****************************************************************************
//  Method: avtLCSFilter::CreateIterativeCalcDataSet
//
//  Purpose:
//      Create the output tree for the whole data set, using the 
//      input tree using the blocks native resolution.
//
//  Programmer: Allen Sanderson
//  Creation:   September 5, 2013
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet*
avtLCSFilter::CreateIterativeCalcDataSet()
{
  //rank 0
  //variable name.
  std::string var = outVarRoot + outVarName;

  //now create a rectilinear grid.
  size_t nTuples =
    global_resolution[0] * global_resolution[1] * global_resolution[2];

          
  // The grid is stored so not to be created as each curve is
  // extended.
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

  // Create storage for the components that need to be used
  // for calculating the exponent. All arrays but the exponent
  // array will be deleted when done.
  vtkDoubleArray *exponents = vtkDoubleArray::New();
  exponents->SetName(var.c_str());
  exponents->SetNumberOfTuples(nTuples);
  rect_grid->GetPointData()->AddArray(exponents);
  // Will set the exponents to be the active scalars when
  // finished. In the mean time the component is the working
  // active scalars.
  // rect_grid->GetPointData()->SetActiveScalars(var.c_str());

  for (size_t i = 0; i < nTuples; i++)
    exponents->SetTuple1(i, std::numeric_limits<double>::min() );

  vtkDoubleArray *component = vtkDoubleArray::New();
  component->SetName("component");
  component->SetNumberOfTuples(nTuples);
  rect_grid->GetPointData()->AddArray(component);
  rect_grid->GetPointData()->SetActiveScalars("component");

  vtkDoubleArray *times = vtkDoubleArray::New();
  times->SetName("times");
  times->SetNumberOfTuples(nTuples);
  rect_grid->GetPointData()->AddArray(times);

  //cleanup
  exponents->Delete();
  component->Delete();
  times->Delete();

  return rect_grid;
}
  

// ****************************************************************************
//  Method: avtLCSFilter::NativeMeshIterativeCalc
//
//  Purpose:
//      Computes the FSLE and similar values that are neighbor
//      dependent on a native resolution grid.
//
//  Programmer: Hari Krishnan
//  Creation:   December 5, 2011
//
//    Mark C. Miller, Wed Aug 22 19:22:40 PDT 2012
//    Fix leak of all_indices, index_counts, all_result, result_counts on 
//    rank 0 (root).
// ****************************************************************************

bool
avtLCSFilter::NativeMeshIterativeCalc(std::vector<avtIntegralCurve*> &ics)
{
    int offset = 0;

    if( *fsle_dt == NULL )
    {
      fsle_dt = CreateIterativeCalcDataTree(GetInputDataTree());
      
      if (GetInput()->GetInfo().GetAttributes().DataIsReplicatedOnAllProcessors())
        if (PAR_Rank() != 0)
          fsle_dt = new avtDataTree();

      SetOutputDataTree(fsle_dt);
    }

    return MultiBlockIterativeCalc(fsle_dt, ics, offset);
}


// ****************************************************************************
//  Method: avtLCSFilter::MultiBlockIterativeCalc
//
//  Purpose:
//      Computes the FSLE and similar values that are neighbor
//      dependent for the whole data set, using the final particle
//      locations, at the blocks native resolution.
//
//  Programmer: Allen Sanderson
//  Creation:   September 5, 2013
//
// ****************************************************************************

bool
avtLCSFilter::MultiBlockIterativeCalc(avtDataTree_p inDT,
                                     std::vector<avtIntegralCurve*> &ics,
                                     int &offset)
{
    if (*inDT == NULL)
        return true;

    int nc = inDT->GetNChildren();

    if (nc < 0 && !inDT->HasData())
        return true;

    if (nc == 0)
    {
        //
        // there is only one dataset to process
        //
        vtkDataSet *in_ds = inDT->GetDataRepresentation().GetDataVTK();

        return SingleBlockIterativeCalc( in_ds, ics, offset );
    }
    else
    {
      bool haveAllExponents = true;

      //
      // there is more than one input dataset to process
      // and we need an input datatree for each
      //
      for (int j = 0; j < nc; j++)
      {
        if (inDT->ChildIsPresent(j) )
        {
          if( MultiBlockIterativeCalc( inDT->GetChild(j), ics, offset ) == false )
            haveAllExponents = false;
        }
      }

      return haveAllExponents;
    }
}


// ****************************************************************************
//  Method: avtLCSFilter::SingleBlockIterativeCalc
//
//  Purpose:
//      Computes the FSLE and similar values that are neighbor
//      dependent for a single block of a data set, using the final
//      particle locations, at the blocks native resolution.
//
//  Arguments:
//      out_ds  The block to calculate the FSLE on
//      ics     The list of particles for all blocks on this MPI task.
//
//  Returns:    The new version of in_ds that includes the FSLE scalar
//              variable.  The calling function is responsible for
//              dereferencing this VTK object.
//
//  Programmer: Allen Sanderson
//  Creation:   September 5, 2013
//
//  Modifications:
//
//    Hank Childs, Tue Feb  5 08:12:33 PST 2013
//    Fix parallelization bug and memory leak.
//
// ****************************************************************************

bool
avtLCSFilter::SingleBlockIterativeCalc( vtkDataSet *in_ds,
                                        std::vector<avtIntegralCurve*> &ics,
                                        int &offset )
{
    int dims[3];

    if (in_ds->GetDataObjectType() == VTK_UNIFORM_GRID)
    {
      ((vtkUniformGrid*)in_ds)->GetDimensions(dims);
    }      
    else if (in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
      ((vtkRectilinearGrid*)in_ds)->GetDimensions(dims);
    }      
    else if (in_ds->GetDataObjectType() == VTK_STRUCTURED_GRID)
    {
      ((vtkStructuredGrid*)in_ds)->GetDimensions(dims);
    }      
    else
    {
      EXCEPTION1(VisItException,
                 "Can only compute SingleBlockIterativeCalc on "
                 "imagedata, rectilinear grids, or structured grids. ");
    }

    // Variable name and number of points.
    std::string var = outVarRoot + outVarName;

    int nTuples = in_ds->GetNumberOfPoints();
    
    // Storage for the points and times
    std::vector<avtVector> remapPoints(nTuples);
    std::vector<double>    remapTimes(nTuples);

    // Zero out the points.
    for(size_t i = 0; i < remapPoints.size(); ++i)
    {
      remapPoints[i] = avtVector(0,0,0);
      remapTimes[i] = 0;
    }

    // ARS - This code does not nothing of use that I can see. The
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
    //     std::vector<int> iHavePoint(nTuples, 0);
    //     std::vector<int> anyoneHasPoint;

    //     for (size_t idx = 0; idx < ics.size(); idx++)
    //     {
    //         size_t index = ics[idx]->id;
    //         size_t l = (index-offset);
    //         if(l < remapPoints.size()) ///l >= 0 is always true
    //         {
    //             iHavePoint[l] = 1;
    //         }
    //     }

    //     UnifyMaximumValue(iHavePoint, anyoneHasPoint);
    //     avtVector zero;
    //     zero.x = zero.y = zero.z = 0.;
    //     for (size_t i = 0; i < (size_t)nTuples; i++)
    //     {
    //         if (PAR_Rank() == 0 && !anyoneHasPoint[i])
    //         {
    //             remapPoints[i] = seedPoints.at(offset + i);
    //             remapTimes[i] = 0;
    //         }
    //         else
    //         {
    //             remapPoints[i] = zero;
    //             remapTimes[i] = 0;
    //         }
    //     }
    // }
    // else
    // {
    //     //copy the original seed points
    //     for(size_t i = 0; i < remapPoints.size(); ++i)
    //     {
    //         remapPoints[i] = seedPoints.at(offset + i);
    //         remapTimes[i] = 0;
    //     }
    // }

    for(size_t i = 0; i < ics.size(); ++i)
    {
        avtStreamlineIC * ic = (avtStreamlineIC *) ics[i];

        size_t index = ic->id;
        int l = (int)(index-offset);

        if(l >= 0 && l < (int)remapPoints.size())
        {
          remapPoints.at(l) = ic->GetEndPoint();

          if( doPathlines )
            remapTimes.at(l) = ic->GetTime() - seedTime0;
          else
            remapTimes.at(l) = ic->GetTime();
        }
    }

    // Get the stored data arrays
    vtkDataArray* jacobian[3];
    
    vtkDoubleArray *exponents = (vtkDoubleArray *)
      in_ds->GetPointData()->GetArray(var.c_str());
    vtkDoubleArray *component = (vtkDoubleArray *)
      in_ds->GetPointData()->GetArray("component");
    vtkDoubleArray *times = (vtkDoubleArray *)
      in_ds->GetPointData()->GetArray("times");

    //use static function in avtGradientExpression to calculate
    //gradients.  since this function only does scalar, break our
    //vectors into scalar components and calculate one at a time.

    // Note the mesh is uniform with all distances being one.
    for(int i = 0; i < 3; ++i)
    {
        for(size_t j = 0; j < (size_t)nTuples; ++j)
            component->SetTuple1(j, remapPoints[j][i]);

        if (GetInput()->GetInfo().GetAttributes().DataIsReplicatedOnAllProcessors())
        {
            float *newvals = new float[nTuples];
            float *origvals = (float *) component->GetVoidPointer(0);
            SumFloatArrayAcrossAllProcessors(origvals, newvals, nTuples);
            // copy newvals back into origvals
            memcpy(origvals, newvals, nTuples*sizeof(float));
            delete [] newvals;
        }

        jacobian[i] =
          avtGradientExpression::CalculateGradient(in_ds, var.c_str());
    }

    // Store the times for the exponent.
    for(size_t l = 0; l < (size_t)nTuples; ++l)
    {
      times->SetTuple1(l, remapTimes[l]);
    }

    for (int i = 0; i < nTuples; i++)
      component->SetTuple1(i, std::numeric_limits<double>::epsilon());

    //now have the jacobian - 3 arrays with 3 components.
    ComputeLyapunovExponent(jacobian, component);
    
    jacobian[0]->Delete();
    jacobian[1]->Delete();
    jacobian[2]->Delete();
    
    // Compute the FSLE
    ComputeFSLE( component, times, exponents );

    bool haveAllExponents = true;

    // For each integral curve check it's mask value to see it
    // additional integration is required.
    for(size_t i=0; i<ics.size(); ++i)
    {
      avtStreamlineIC * ic = (avtStreamlineIC *) ics[i];

      size_t index = ic->id;
      size_t l = (index-offset);

      int ms = ic->GetMaxSteps();

      if( ms < (unsigned int)maxSteps )
      {
        ic->SetMaxSteps(ms+1);
        ic->status.ClearTerminationMet();
      }

      // Check to see if all exponents have been found.
      if( exponents->GetTuple1(l) == std::numeric_limits<double>::min() &&
          ic->GetMaxSteps() < (unsigned int)maxSteps )
        haveAllExponents = false;
    }
    
    //done with offset, increment it for the next call to this
    //function.
    offset += nTuples;

    return haveAllExponents;
}


// ****************************************************************************
//  Method: avtLCSFilter::RectilinearGridIterativeCalc
//
//  Purpose:
//      Computes the FSLE and similar values that are neighbor
//      dependent on a rectilinear grid.
//
//  Programmer: Allen Sanderson
//  Creation:   September 5, 2013
//
//    Mark C. Miller, Wed Aug 22 19:22:40 PDT 2012
//    Fix leak of all_indices, index_counts, all_result, result_counts on 
//    rank 0 (root).
// ****************************************************************************

bool
avtLCSFilter::RectilinearGridIterativeCalc( std::vector<avtIntegralCurve*> &ics )
{
    //algorithm sends index to global datastructure as well as end points.
    //Send List of index into global array to rank 0
    //Send end positions into global array to rank 0

    //loop over all the intelgral curves and add it back to the
    //original list of seeds.
    intVector indices(ics.size());
    doubleVector points(ics.size()*3);
    doubleVector times(ics.size());

    for(size_t i=0, j=0; i<ics.size(); ++i, j+=3)
    {
        avtStreamlineIC * ic = (avtStreamlineIC *) ics[i];

        indices[i] = ic->id;
        avtVector point = ic->GetEndPoint();

        points[j+0] = point[0];
        points[j+1] = point[1];
        points[j+2] = point[2];

        if( doPathlines )
          times[i] = ic->GetTime() - seedTime0;
        else
          times[i] = ic->GetTime();
    }

    int* all_indices = 0;
    int* index_counts = 0;

    double* all_points = 0;
    int *point_counts = 0;

    double* all_times = 0;
    int *time_counts = 0;

    Barrier();

    CollectIntArraysOnRootProc(all_indices, index_counts,
                               &indices.front(), (int)indices.size());

    CollectDoubleArraysOnRootProc(all_points, point_counts,
                                  &points.front(), (int)points.size());

    CollectDoubleArraysOnRootProc(all_times, time_counts,
                                  &times.front(), (int)times.size());

    Barrier();

    //root should now have index into global structure and all
    //matching end positions.
    if(PAR_Rank() != 0)
    {
        return true;
    }
    else
    {
        //variable name.
        std::string var = outVarRoot + outVarName;

        //now global grid has been created.
        if( fsle_ds == 0 )
          fsle_ds = CreateIterativeCalcDataSet();

        // Get the stored data arrays
        vtkDoubleArray *exponents = (vtkDoubleArray *)
          fsle_ds->GetPointData()->GetArray(var.c_str());
        vtkDoubleArray *component = (vtkDoubleArray *)
          fsle_ds->GetPointData()->GetArray("component");
        vtkDoubleArray *times = (vtkDoubleArray *)
          fsle_ds->GetPointData()->GetArray("times");

        size_t nTuples = exponents->GetNumberOfTuples();

        // Storage for the points and times
        std::vector<avtVector> remapPoints(nTuples);
        std::vector<double> remapTimes(nTuples);

        //update remapPoints with new value bounds from integral curves.
        int par_size = PAR_Size();
        size_t total = 0;
        for(int i = 0; i < par_size; ++i)
        {
            if(index_counts[i]*3 != point_counts[i] ||
               index_counts[i]   != time_counts[i])
            {
              EXCEPTION1(VisItException,
                         "Index count does not the result count." );
            }

            total += index_counts[i];
        }

        for(size_t j=0, k=0; j<total; ++j, k+=3)
        {
            size_t index = all_indices[j];

            if(nTuples <= index)
            {
              EXCEPTION1(VisItException,
                         "More integral curves were generatated than "
                         "grid points." );
            }

            remapPoints[index].set( all_points[k+0],
                                    all_points[k+1],
                                    all_points[k+2]);

            remapTimes[index] = all_times[j];
        }

        // Store the times for the exponent.
        for(size_t l=0; l<nTuples; ++l)
          times->SetTuple1(l, remapTimes[l]);

        //use static function in avtGradientExpression to calculate
        //gradients.  since this function only does scalar, break our
        //vectors into scalar components and calculate one at a time.

        vtkDataArray* jacobian[3];

        for(int i = 0; i < 3; ++i)
        {
            // Store the point component by component
            for(size_t l=0; l<nTuples; ++l)
                component->SetTuple1(l, remapPoints[l][i]);

            jacobian[i] =
              avtGradientExpression::CalculateGradient(fsle_ds, "component");
        }

        for (size_t i = 0; i < nTuples; i++)
          component->SetTuple1(i, std::numeric_limits<double>::epsilon());

        //now have the jacobian - 3 arrays with 3 components.
        ComputeLyapunovExponent(jacobian, component);
        
        jacobian[0]->Delete();
        jacobian[1]->Delete();
        jacobian[2]->Delete();
      
        // Compute the FSLE
        ComputeFSLE( component, times, exponents );

        bool haveAllExponents = true;

        // For each integral curve check it's mask value to see it
        // additional integration is required.

        for(size_t i=0; i<ics.size(); ++i)
        {
          avtStreamlineIC * ic = (avtStreamlineIC *) ics[i];

          int ms = ic->GetMaxSteps();

          if( ms < (unsigned int)maxSteps )
          {
            ic->SetMaxSteps(ms+1);
            ic->status.ClearTerminationMet();
          }

          size_t l = ic->id; // The curve id is the index into the VTK data.

          // Check to see if all exponents have been found.
          if( exponents->GetTuple1(l) == std::numeric_limits<double>::min() &&
              ic->GetMaxSteps() < (unsigned int)maxSteps )
            haveAllExponents = false;
        }

        //cleanup.
        if (all_indices)   delete [] all_indices;
        if (index_counts)  delete [] index_counts;

        if (all_points)    delete [] all_points;
        if (point_counts)  delete [] point_counts;

        if (all_times)    delete [] all_times;
        if (time_counts)  delete [] time_counts;

        return haveAllExponents;
    }
}


// ****************************************************************************
//  Method: avtLCSFilter::ComputeFSLE
//
//  Purpose:
//      Computes the FSLE given the distances.
//
//  Programmer: Allen Sanderson
//  Creation:   September 5, 2013
//
//  Modifications:
//
// ****************************************************************************

void avtLCSFilter::ComputeFSLE( vtkDataArray *component,
                                vtkDataArray *times,
                                vtkDataArray *exponents )
{
  size_t nTuples = component->GetNumberOfTuples();

  for(size_t l = 0; l < nTuples; ++l)
  {
    double lambda = exponents->GetTuple1(l);

    // If the exponent was previously set skip checking it.
    if( lambda == std::numeric_limits<double>::min() )
    {
      // Check for a curve that has terminated which will not have
      // taken a step forward or backwards.
      if( floor( fabs(times->GetTuple1(l)) / maxStepLength + 0.5) != numSteps )
      {
        // If a curve has terminated set the exponent to zero.
        lambda = 0;
        exponents->SetTuple1(l, lambda);
      }            

      // Check the distances between neighbors.
      else
      {
        double size = component->GetTuple1(l);

        minSizeValue = std::min(size, minSizeValue);
        maxSizeValue = std::max(size, maxSizeValue);

        // Record the Lyapunov exponent if the max
        // size has been reached.
        if( maxSize < size )
        {
          lambda = log( size ) / fabs(times->GetTuple1(l));
              
          exponents->SetTuple1(l, lambda);
        }
      }
    }
  }
}


// ****************************************************************************
//  Method: avtLCSFilter::InBounds
//
//  Purpose:
//      Returns true if indexes are in bounds.
//
//  Programmer: Allen Sanderson
//  Creation:   September 5, 2013
//
//  Modifications:
//
// ****************************************************************************

int avtLCSFilter::InBounds( int x, int y, int z,
                            int x_max, int y_max, int z_max )
{
  if( 0 <= x && x < x_max &&
      0 <= y && y < y_max &&
      0 <= z && z < z_max )
    return (z * y_max + y) * x_max + x;
  else
    return -1;
}


// ****************************************************************************
//  Method: avtLCSFilter::Increment
//
//  Purpose:
//      Increments a counter
//
//  Programmer: Allen Sanderson
//  Creation:   September 5, 2013
//
//  Modifications:
//
// ****************************************************************************

void avtLCSFilter::Increment( int x, int y, int z, vtkDataArray *array,
                              int x_max, int y_max, int z_max )
{
  int l = InBounds( x, y, z, x_max, y_max, z_max );
  
  if( 0 <= l && l < array->GetNumberOfTuples() )
  {      
    int cc = array->GetTuple1(l);
    ++cc;
    array->SetTuple1(l, cc);
  }
}


// ****************************************************************************
//  Method: avtLCSFilter::Value
//
//  Purpose:
//      Returns the value for a coordinate
//
//  Programmer: Allen Sanderson
//  Creation:   September 5, 2013
//
//  Modifications:
//
// ****************************************************************************

double avtLCSFilter::Value( int x, int y, int z, vtkDataArray *array,
                            int x_max, int y_max, int z_max )
{
  int l = InBounds( x, y, z, x_max, y_max, z_max );

  if( 0 <= l && l < array->GetNumberOfTuples() )
    return array->GetTuple1(l);
  else
    return 0;
}


// ****************************************************************************
//  Method: avtLCSFilter::CreateNativeMeshIterativeCalcOutput
//
//  Purpose:
//      Computes the IterativeCalc output (via sub-routines) after the PICS filter
//      has calculated the particle positions.
//
//  Programmer: Allen Sanderson
//  Creation:   September 5, 2013
//
// ****************************************************************************

void 
avtLCSFilter::CreateNativeMeshIterativeCalcOutput(std::vector<avtIntegralCurve*> &ics)
{
    //accumulate all of the points then do jacobian?
    //or do jacobian then accumulate?
    //picking the first.

    int    offset = 0;
    double minv   =  std::numeric_limits<double>::max();
    double maxv   = -std::numeric_limits<double>::max();
    int    count  = 0;

    CreateMultiBlockIterativeCalcOutput(GetInputDataTree(), GetDataTree(),
                                        ics, offset, minv, maxv, count);

    int nTuples = (int)ics.size();

    if( 1 || count <= nTuples/10 )
    {
      if( minSizeValue == std::numeric_limits<double>::max() )
        minSizeValue = 0.0;
      
      if( maxSizeValue == -std::numeric_limits<double>::max() )
        maxSizeValue = 0.0;
      
      char str[1028];
      
      SNPRINTF(str, 1028, "\n%d%% of the nodes (%d of %d nodes) "
               "exaimed produced a valid exponent (%f to %f). "
               "This may be due to too large of a size limit (%f), "
               "too small of an integration step (%f), or "
               "too few integration steps (%d out of %d where taken), or "
               "simply due to the nature of the data. "
               "The size range was from %f to %f. ",
               (int) (100.0 * (double) count / (double) nTuples),
               count, nTuples,
               minv, maxv,
               maxSize, maxStepLength, numSteps, maxSteps,
               minSizeValue, maxSizeValue );
      
      avtCallback::IssueWarning(str);
    }

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
//  Method: avtLCSFilter::CreateMultiBlockIterativeCalcOutput
//
//  Purpose:
//      Computes the finial FSLE and similar values that are neighbor
//      dependent for the whole data set, using the final particle
//      locations, at the blocks native resolution.
//
//  Programmer: Allen Sanderson
//  Creation:   September 5, 2013
//
// ****************************************************************************

void
avtLCSFilter::CreateMultiBlockIterativeCalcOutput( avtDataTree_p inDT,
                                                   avtDataTree_p outDT,
                                                   std::vector<avtIntegralCurve*> &ics,
                                                   int &offset,
                                                   double &minv, double &maxv,
                                                   int &count)
{
    if (*inDT == NULL || *outDT == NULL)
        return;

    int nc = inDT->GetNChildren();

    if (nc < 0 && !inDT->HasData())
        return;

    if (nc == 0)
    {
        //
        // there is only one dataset to process
        //
        vtkDataSet  *in_ds =  inDT->GetDataRepresentation().GetDataVTK();
        vtkDataSet *out_ds = outDT->GetDataRepresentation().GetDataVTK();

        int dom = inDT->GetDataRepresentation().GetDomain();
        std::string label = inDT->GetDataRepresentation().GetLabel();

        CreateSingleBlockIterativeCalcOutput(in_ds, out_ds, ics,
                                             offset, dom, minv, maxv, count);
    }
    else
    {
      //
      // there is more than one input dataset to process
      // and we need an output datatree for each
      //
      for (int j = 0; j < nc; j++)
      {
          if (inDT->ChildIsPresent(j))
            CreateMultiBlockIterativeCalcOutput(inDT->GetChild(j),
                                                outDT->GetChild(j),
                                                ics, offset, minv, maxv, count);
      }
    }
}


// ****************************************************************************
//  Method: avtLCSFilter::CreateSingleBlockIterativeCalcOutput
//
//  Purpose:
//      Computes the finial FSLE and similar values that are neighbor
//      dependent for a single block of a data set, using the final
//      particle locations, at the blocks native resolution.
//
//  Arguments:
//      in_ds   The block to calculate the value on
//      ics     The list of particles for all blocks on this MPI task.
//      domain  The domain number of in_ds
//      minv    The minimum value (output)
//      maxv    The maximum value (output)
//      count   The number of nodes with a valid exponent
//
//  Returns:    The new version of in_ds that includes the scalar
//              variable.  The calling function is responsible for
//              dereferencing this VTK object.
//
//  Programmer: Allen Sanderson
//  Creation:   September 5, 2013
//
//  Modifications:
//
//    Hank Childs, Tue Feb  5 08:12:33 PST 2013
//    Fix parallelization bug and memory leak.
//
// ****************************************************************************

void
avtLCSFilter::CreateSingleBlockIterativeCalcOutput( vtkDataSet *in_ds,
                                                    vtkDataSet *out_ds,
                                                    std::vector<avtIntegralCurve*> &ics,
                                                    int &offset, int domain,
                                                    double &minv, double &maxv,
                                                    int &count )
{
  //variable name.
  std::string var = outVarRoot + outVarName;

  vtkDoubleArray *exponents = (vtkDoubleArray *)
    out_ds->GetPointData()->GetArray(var.c_str());

  int nTuples = exponents->GetNumberOfTuples();

  for(size_t i=0; i<ics.size(); ++i)
  {
    avtStreamlineIC * ic = (avtStreamlineIC *) ics[i];
    
    size_t index = ic->id;
    size_t l = (index-offset);

    double lambda = exponents->GetTuple1(l);
    
    if( lambda == -std::numeric_limits<double>::max() )
    {
      lambda = 0;
      exponents->SetTuple1(l, lambda );
    }
    else
    {
      ++count;
      ic->status.ClearTerminationMet();

      if( clampLogValues && lambda < 0 )
      {
        lambda = 0;
        exponents->SetTuple1(l, lambda );
      }
    }
    
    minv = std::min(lambda, minv);
    maxv = std::max(lambda, maxv);
    
  }

  // Make the exponents the the active scalars.
  out_ds->GetPointData()->SetActiveScalars(var.c_str());

  // Remove the working arrays.
  out_ds->GetPointData()->RemoveArray("component");
  out_ds->GetPointData()->RemoveArray("times");

  //done with offset, increment it for the next call to this
  //function.
  offset += nTuples;
  
  //Store this dataset in Cache for next time.
  std::string str = CreateCacheString();  
  StoreArbitraryVTKObject(SPATIAL_DEPENDENCE | DATA_DEPENDENCE,
                          outVarName.c_str(), domain, -1,
                          str.c_str(), out_ds);
}


// ****************************************************************************
//  Method: avtLCSFilter::CreateRectilinearGridIterativeCalcOutput
//
//  Purpose:
//      Computes the finial FSLE and similar values that are neighbor
//      dependent output (via sub-routines) after the PICS filter has
//      calculated the particle positions.
//
//  Programmer: Allen Sanderson
//  Creation:   September 5, 2013
//
// ****************************************************************************

void 
avtLCSFilter::CreateRectilinearGridIterativeCalcOutput(std::vector<avtIntegralCurve*> &ics)
{
    //root should now have index into global structure and all
    //matching end positions.
    if(PAR_Rank() != 0)
    {
        avtDataTree* dummy = new avtDataTree();
        SetOutputDataTree(dummy);
    }
    else
    {
      if (fsle_ds->GetDataObjectType() != VTK_RECTILINEAR_GRID)
      {
        EXCEPTION1(VisItException,
                   "Can only compute CreateRectilinearGridIterativeCalcOutput on "
                   "rectilinear grids. ");
      }

      //variable name.
      std::string var = outVarRoot + outVarName;

      vtkDoubleArray *exponents = (vtkDoubleArray *)
        fsle_ds->GetPointData()->GetArray(var.c_str());

      int nTuples = exponents->GetNumberOfTuples();

      //min and max values over all datasets of the tree.
      double minv =  std::numeric_limits<double>::max();
      double maxv = -std::numeric_limits<double>::max();
      
      int count = 0;

      for(size_t i=0; i<ics.size(); ++i)
      {
        avtStreamlineIC * ic = (avtStreamlineIC *) ics[i];

        size_t l = ic->id; // The curve id is the index into the VTK data.

        double lambda = exponents->GetTuple1(l);

        if( lambda == -std::numeric_limits<double>::max() )
        {
          lambda = 0;
          exponents->SetTuple1(l, lambda );
        }
        else
        {
          ++count;
          ic->status.ClearTerminationMet();

          if( clampLogValues && lambda < 0 )
          {
            lambda = 0;
            exponents->SetTuple1(l, lambda );
          }
        }

        minv = std::min(lambda, minv);
        maxv = std::max(lambda, maxv);
      }

      if( count <= nTuples/10 )
      {
        if( minSizeValue == std::numeric_limits<double>::max() )
          minSizeValue = 0.0;

        if( maxSizeValue == -std::numeric_limits<double>::max() )
          maxSizeValue = 0.0;

        char str[1028];

        SNPRINTF(str, 1028, "\n%d%% of the nodes (%d of %d nodes) "
                 "exaimed produced a valid exponent (%f to %f). "
                 "This may be due to too large of a size limit (%f), "
                 "too small of an integration step (%f), or "
                 "too few integration steps (%d out of %d where taken), or "
                 "simply due to the nature of the data. "
                 "The size range was from %f to %f. ",
                 (int) (100.0 * (double) count / (double) nTuples),
                 count, nTuples,
                 minv, maxv,
                 maxSize, maxStepLength, numSteps, maxSteps,
                 minSizeValue, maxSizeValue );

        avtCallback::IssueWarning(str);
      }
      
      // Make the exponents the the active scalars.
      fsle_ds->GetPointData()->SetActiveScalars(var.c_str());

      // Remove the working arrays.
      fsle_ds->GetPointData()->RemoveArray("component");
      fsle_ds->GetPointData()->RemoveArray("times");

      std::string str = CreateCacheString();
      StoreArbitraryVTKObject(SPATIAL_DEPENDENCE | DATA_DEPENDENCE,
                              outVarName.c_str(), -1, -1,
                              str.c_str(), fsle_ds);
      
      int index = 0;//what does index mean in this context?
      avtDataTree* dt = new avtDataTree(fsle_ds,index);
      int x = 0;
      dt->GetAllLeaves(x);
      
      SetOutputDataTree(dt);
      
      //set atts.
      avtDataAttributes &dataatts = GetOutput()->GetInfo().GetAttributes();
      avtExtents* e = dataatts.GetThisProcsActualDataExtents();
      
      double range[2];
      range[0] = minv;
      range[1] = maxv;
      e->Set(range);
    }
}
