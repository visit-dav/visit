/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
//                      avtSamplePointToSurfaceFilter.C                      //
// ************************************************************************* //

#include <avtSamplePointToSurfaceFilter.h>

#include <float.h>

#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

#include <avtImagePartition.h>
#include <avtParallel.h>
#include <avtRay.h>
#include <avtVolume.h>

#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtSamplePointToSurfaceFilter constructor
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2003
//
// ****************************************************************************

avtSamplePointToSurfaceFilter::avtSamplePointToSurfaceFilter()
{
    surfType = NOT_SPECIFIED;
    imagePartition = NULL;
}


// ****************************************************************************
//  Method: avtSamplePointToSurfaceFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtSamplePointToSurfaceFilter::~avtSamplePointToSurfaceFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtSamplePointToSurfaceFilter::Execute
//
//  Purpose:
//      Infers a surface from sample points.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2003
//
//  Modifications:
//
//    Hank Childs, Wed Jul  2 13:18:25 PDT 2003
//    Made the surface be output in a form that will be more conducive to
//    comparisons.
//
//    Hank Childs, Mon Jul 14 20:27:31 PDT 2003
//    Better account for holes in volumes.
//
// ****************************************************************************

void
avtSamplePointToSurfaceFilter::Execute(void)
{
    int   i, j;

    if (surfType == NOT_SPECIFIED || imagePartition == NULL)
    {
        EXCEPTION0(ImproperUseException);
    }

    int minX, maxX, minY, maxY;
    imagePartition->GetThisPartition(minX, maxX, minY, maxY);

    int width  = imagePartition->GetWidth();
    int height = imagePartition->GetHeight();
    int buff_size = width*height;
    int num_samples = -1;
    float *vals = new float[buff_size];
    for (i = 0 ; i < buff_size ; i++)
    {
        vals[i] = -FLT_MAX;
    }
    int *depths = new int[buff_size];
    for (i = 0 ; i < buff_size ; i++)
    {
        depths[i] = -1;
    }

    avtVolume *volume = GetTypedInput()->GetVolume();
    float samples[AVT_VARIABLE_LIMIT];
    for (i = minX ; i <= maxX ; i++)
    {
        for (j = minY ; j <= maxY ; j++)
        {
            const avtRay *ray = volume->QueryGetRay(i, j);
            if (ray == NULL)
                continue;
            num_samples = ray->GetNumberOfSamples();
            int depth = -1;
            if (surfType == FRONT_SURFACE)
            {
                depth = ray->GetFirstSample();
            }
            else if (surfType == BACK_SURFACE)
            {
                depth = ray->GetLastSample();
            }
            else 
            {
                int samp1 = ray->GetFirstSampleOfLongestRun();
                int samp2 = ray->GetLastSampleOfLongestRun();
                depth = (samp1 + samp2) / 2;
            }
            if (depth >= 0)
            {
                ray->GetSample(depth, samples);
                vals[i*height + j] = samples[0];
                depths[i*height + j] = depth;
            }
        }
    }

    num_samples = UnifyMaximumValue(num_samples);
    Collect(depths, buff_size);
    if (Collect(vals, buff_size))
    {
        vtkPoints *pts = vtkPoints::New();
        int norigpts = height*width;
        int ndummypts = (height-1)*(width-1);
        int ntotalpts = norigpts + ndummypts;
        pts->SetNumberOfPoints(ntotalpts);
        for (i = 0 ; i < width ; i++)
        {
            for (j = 0 ; j < height ; j++)
            {
                int index = i*height + j;
                float x = (((float)i) / (width-1.)) * 2. - 1.;
                float y = (((float)j) /(height-1.)) * 2. - 1.;
                float z;
                if (depths[index] >= 0)
                    z = ((float)depths[index]) / (num_samples-1.);
                else
                    z = 0.;
                pts->SetPoint(index, x, y, z);
            }
        }
        for (i = 0 ; i < width-1 ; i++)
        {
            for (j = 0 ; j < height-1 ; j++)
            {
                int index = i*(height-1) + j + norigpts;
                float x = ((i+0.5) / (width-1.)) * 2. - 1.;
                float y = ((j+0.5) /(height-1.)) * 2. - 1.;
                float z = 0.;
                int numvalid = 0;
                int i0 = i*height + j;
                int i1 = (i+1)*height + j;
                int i2 = (i+1)*height + j+1;
                int i3 = i*height + j+1;
                if (depths[i0] >= 0)
                {
                    z += ((float)depths[i0]) / (num_samples-1.);
                    numvalid++;
                }
                if (depths[i1] >= 0)
                {
                    z += ((float)depths[i1]) / (num_samples-1.);
                    numvalid++;
                }
                if (depths[i2] >= 0)
                {
                    z += ((float)depths[i2]) / (num_samples-1.);
                    numvalid++;
                }
                if (depths[i3] >= 0)
                {
                    z += ((float)depths[i3]) / (num_samples-1.);
                    numvalid++;
                }
                if (numvalid > 0)
                    z /= numvalid;
                else
                    z = 0.;
                pts->SetPoint(index, x, y, z);
            }
        }
 
        vtkFloatArray *arr = vtkFloatArray::New();
        arr->SetNumberOfTuples(ntotalpts);
        for (i = 0 ; i < norigpts ; i++)
        {
            arr->SetTuple1(i, vals[i]);
        }
        for (i = 0 ; i < width-1 ; i++)
        {
            for (j = 0 ; j < height-1 ; j++)
            {
                int index = i*(height-1) + j + norigpts;
                float val = 0.;
                int numvalid = 0;
                int i0 = i*height + j;
                int i1 = (i+1)*height + j;
                int i2 = (i+1)*height + j+1;
                int i3 = i*height + j+1;
                if (depths[i0] >= 0)
                {
                    val += vals[i0];
                    numvalid++;
                }
                if (depths[i1] >= 0)
                {
                    val += vals[i1];
                    numvalid++;
                }
                if (depths[i2] >= 0)
                {
                    val += vals[i2];
                    numvalid++;
                }
                if (depths[i3] >= 0)
                {
                    val += vals[i3];
                    numvalid++;
                }
                if (numvalid > 0)
                    val /= numvalid;
                else
                    val = 0.;
                arr->SetTuple1(index, val);
            }
        }

        vtkPolyData *pd = vtkPolyData::New();

        pd->SetPoints(pts);
        pts->Delete();

        pd->GetPointData()->SetScalars(arr);
        arr->Delete();

        int ncells = 4*(height-1)*(width-1);
        vtkFloatArray *cell_valid = vtkFloatArray::New();
        cell_valid->SetName("cell_valid");
        cell_valid->SetNumberOfTuples(ncells);
        pd->Allocate(ncells*4);
        int cell_id = 0;
        for (i = 0 ; i < width-1 ; i++)
        {
            for (j = 0 ; j < height-1 ; j++)
            {
                int index0 = i*height + j;
                int index1 = i*height + j+1;
                int index2 = (i+1)*height + j+1;
                int index3 = (i+1)*height + j;
                int midpt  = i*(height-1) + j + norigpts;
                bool index0_valid = (depths[index0] >= 0 ? true : false);
                bool index1_valid = (depths[index1] >= 0 ? true : false);
                bool index2_valid = (depths[index2] >= 0 ? true : false);
                bool index3_valid = (depths[index3] >= 0 ? true : false);

                int num_valid = 0;
                num_valid += (index0_valid ? 1 : 0);
                num_valid += (index1_valid ? 1 : 0);
                num_valid += (index2_valid ? 1 : 0);
                num_valid += (index3_valid ? 1 : 0);
                bool all_should_be_invalid = (num_valid <= 2 ? true : false);
                vtkIdType tri[3];

                // 0, 1, midpt
                tri[0] = index0;
                tri[1] = midpt;
                tri[2] = index1;
                pd->InsertNextCell(VTK_TRIANGLE, 3, tri);
                if (all_should_be_invalid)
                    cell_valid->SetValue(cell_id++, 0);
                else
                    cell_valid->SetValue(cell_id++, 
                                     (index0_valid && index1_valid ? 1 : 0));
             
                // 1, 2, midpt
                tri[0] = index1;
                tri[1] = midpt;
                tri[2] = index2;
                pd->InsertNextCell(VTK_TRIANGLE, 3, tri);
                if (all_should_be_invalid)
                    cell_valid->SetValue(cell_id++, 0);
                else
                    cell_valid->SetValue(cell_id++, 
                                     (index1_valid && index2_valid ? 1 : 0));
                
                // 2, 3, midpt
                tri[0] = index2;
                tri[1] = midpt;
                tri[2] = index3;
                pd->InsertNextCell(VTK_TRIANGLE, 3, tri);
                if (all_should_be_invalid)
                    cell_valid->SetValue(cell_id++, 0);
                else
                    cell_valid->SetValue(cell_id++, 
                                     (index2_valid && index3_valid ? 1 : 0));
             
                // 3, 0, midpt
                tri[0] = index3;
                tri[1] = midpt;
                tri[2] = index0;
                pd->InsertNextCell(VTK_TRIANGLE, 3, tri);
                if (all_should_be_invalid)
                    cell_valid->SetValue(cell_id++, 0);
                else
                    cell_valid->SetValue(cell_id++, 
                                     (index3_valid && index0_valid ? 1 : 0));
            }
        }
        pd->GetCellData()->AddArray(cell_valid);
        cell_valid->Delete();

        SetOutputDataTree(new avtDataTree(pd, -1));
        pd->Delete();
    }
}


