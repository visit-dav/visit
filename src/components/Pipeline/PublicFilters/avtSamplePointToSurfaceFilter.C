// ************************************************************************* //
//                      avtSamplePointToSurfaceFilter.C                      //
// ************************************************************************* //

#include <avtSamplePointToSurfaceFilter.h>

#include <float.h>

#include <vtkFloatArray.h>
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
//  Method: avtSamplePointToSurfaceFilter::Execute
//
//  Purpose:
//      Infers a surface from sample points.
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2003
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
    for (i = minX ; i < maxX ; i++)
    {
        for (j = minY ; j < maxY ; j++)
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
                int samp1 = ray->GetFirstSample();
                int samp2 = ray->GetLastSample();
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
        int npts = height*width;
        pts->SetNumberOfPoints(npts);
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
 
        vtkFloatArray *arr = vtkFloatArray::New();
        arr->SetNumberOfTuples(npts);
        for (i = 0 ; i < npts ; i++)
        {
            arr->SetTuple1(i, vals[i]);
        }

        vtkPolyData *pd = vtkPolyData::New();

        pd->SetPoints(pts);
        pts->Delete();

        pd->GetPointData()->SetScalars(arr);
        arr->Delete();

        int ncells = (height-1)*(width-1);
        pd->Allocate(ncells*4);
        for (i = 0 ; i < width-1 ; i++)
        {
            for (j = 0 ; j < height-1 ; j++)
            {
                int index0 = i*height + j;
                int index1 = i*height + j+1;
                int index2 = (i+1)*height + j+1;
                int index3 = (i+1)*height + j;
                bool index0_valid = (depths[index0] >= 0 ? true : false);
                bool index1_valid = (depths[index1] >= 0 ? true : false);
                bool index2_valid = (depths[index2] >= 0 ? true : false);
                bool index3_valid = (depths[index3] >= 0 ? true : false);
                int num_valid = 0;
                if (index0_valid)
                    num_valid++;
                if (index1_valid)
                    num_valid++;
                if (index2_valid)
                    num_valid++;
                if (index3_valid)
                    num_valid++;
                if (num_valid == 3)
                {
                    vtkIdType tri[3];
                    if (!index0_valid)
                    {
                        tri[0] = index1;
                        tri[1] = index2;
                        tri[2] = index3;
                    }
                    if (!index1_valid)
                    {
                        tri[0] = index0;
                        tri[1] = index2;
                        tri[2] = index3;
                    }
                    if (!index2_valid)
                    {
                        tri[0] = index0;
                        tri[1] = index1;
                        tri[2] = index3;
                    }
                    if (!index3_valid)
                    {
                        tri[0] = index0;
                        tri[1] = index1;
                        tri[2] = index2;
                    }
                    pd->InsertNextCell(VTK_TRIANGLE, 3, tri);
                }
                else if (num_valid == 4)
                {
                    vtkIdType quad[4];
                    quad[0] = index0;
                    quad[1] = index1;
                    quad[2] = index2;
                    quad[3] = index3;
                    pd->InsertNextCell(VTK_QUAD, 4, quad);
                }
            }
        }
        SetOutputDataTree(new avtDataTree(pd, -1));
        pd->Delete();
    }
}


