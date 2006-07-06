// NOTE - This code incomplete and requires underlined portions
// to be replaced with code to read values from your file format.

#include <vtkPoints.h>
#include <vtkStructuredGrid.h>

vtkDataSet *
avtXXXFileFormat::GetMesh(const char *meshname)
{
    int ndims = 2;
    int dims[3] = {1,1,1};
    
    // Read the ndims and number of X,Y,Z nodes from file.
    ndims = NUMBER OF MESH DIMENSIONS;
    dims[0] = NUMBER OF NODES IN X-DIMENSION;
    dims[1] = NUMBER OF NODES IN Y-DIMENSION;
    dims[2] = NUMBER OF NODES IN Z-DIMENSION, OR 1 IF 2D;
    int nnodes = dims[0]*dims[1]*dims[2];

    // Read the X coordinates from the file.
    float *xarray = new float[nnodes];
    READ nnodes FLOAT VALUES INTO xarray

    // Read the Y coordinates from the file.
    float *yarray = new float[nnodes];
    READ nnodes FLOAT VALUES INTO yarray

    // Read the Z coordinates from the file.
    float *zarray = 0;
    if(ndims > 2)
    {
        zarray = new float[nnodes];
        READ dims[2] FLOAT VALUES INTO zarray
    }

    //
    // Create the vtkStructuredGrid and vtkPoints objects.
    //
    vtkStructuredGrid *sgrid  = vtkStructuredGrid::New(); 
    vtkPoints         *points = vtkPoints::New();
    sgrid->SetPoints(points);
    sgrid->SetDimensions(dims);
    points->Delete();
    points->SetNumberOfPoints(nnodes);

    //
    // Copy the coordinate values into the vtkPoints object.
    //
    float *pts = (float *) points->GetVoidPointer(0);
    float *xc = xarray;
    float *yc = yarray;
    float *zc = zarray;
    if(ndims == 3)
    {
        for(int k = 0; k < dims[2]; ++k)
        {
            for(int j = 0; j < dims[1]; ++j)
            {
                for(int i = 0; i < dims[0]; ++i)
                {
                    *pts++ = *xc++;
                    *pts++ = *yc++;
                    *pts++ = *zc++;
                }
            }
        }
    }
    else if(ndims == 2)
    {
        for(int j = 0; j < dims[1]; ++j)
        {
            for(int i = 0; i < dims[0]; ++i)
            {
                *pts++ = *xc++;
                *pts++ = *yc++;
                *pts++ = 0.;
            }
        }
    }

    // Delete temporary arrays.
    delete [] xarray;
    delete [] yarray;
    delete [] zarray;

    return sgrid;
}
