#include <stdio.h>
#include <stdlib.h>

#include <hdf5.h>

#define TIME 1.5

#define NX 30
#define NY 20
#define NZ 10

#define NX2 20
#define NY2 20
#define NZ2 10

#define NX3 25
#define NY3 20
#define NZ3 10

void
write_hdf5_2d_curv_data(hid_t file_id)
{
    /*
     * Create the coordinate data.
     */
    float *xy = (float *) malloc((NX+1)*(NY+1)*2 * sizeof(float));

    int ndx = 0;
    for (int j = 0; j < NY+1; j++)
    {
        for (int i = 0; i < NX+1; i++)
        {
            xy[ndx++] = (float) i;
            xy[ndx++] = (float) j;
        }
    }

    /*
     * Create the scalar data.
     */
    float *pressure = (float *) malloc(NX*NY * sizeof(float));

    for (int j = 0; j < NY; j++)
    {
        for (int i = 0; i < NX; i++)
        {
            int ndx = j * NX + i;
            pressure[ndx] = (float) j;
        }
    }

    float *velocityx = (float *) malloc((NX+1)*(NY+1) * sizeof(float));

    for (int j = 0; j < NY+1; j++)
    {
        for (int i = 0; i < NX+1; i++)
        {
            int ndx = j * (NX+1) + i;
            velocityx[ndx] = (float) i;
        }
    }

    /*
     * Write the data file.
     */
    /*
     * Data types: H5T_NATIVE_CHAR
     *             H5T_NATIVE_INT
     *             H5T_NATIVE_LONG
     *             H5T_NATIVE_FLOAT
     *             H5T_NATIVE_DOUBLE
     */
    hid_t     dataset_id, dataspace_id;
    hsize_t   dims[3];
    herr_t    status;

    /* Write a single coordinate array with x and y coordinates. */
    dims[0] = (NY + 1) * (NX + 1);
    dims[1] = 2;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(file_id, "/XY", H5T_NATIVE_FLOAT, dataspace_id,
                           H5P_DEFAULT);

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, xy);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    /*
     * Write the scalar data.
     */ 
    dims[0] = NY;
    dims[1] = NX;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(file_id, "/Pressure_2D", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, pressure);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    dims[0] = NY + 1;
    dims[1] = NX + 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(file_id, "/VelocityX_2D", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, velocityx);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    /*
     * Free the data.
     */
    free(xy);
    free(pressure);
    free(velocityx);
}

void
write_hdf5_3d_rect_data(hid_t file_id)
{
    /*
     * Create the coordinate data.
     */
    float *x = (float *) malloc((NX+1) * sizeof(float));
    float *y = (float *) malloc((NY+1) * sizeof(float));
    float *z = (float *) malloc((NZ+1) * sizeof(float));

    for (int i = 0; i < NX+1; i++)
        x[i] = (float) i;
    for (int j = 0; j < NY+1; j++)
        y[j] = (float) j;
    for (int k = 0; k < NZ+1; k++)
        z[k] = (float) k;

    /*
     * Write the data file.
     */
    /*
     * Data types: H5T_NATIVE_CHAR
     *             H5T_NATIVE_INT
     *             H5T_NATIVE_LONG
     *             H5T_NATIVE_FLOAT
     *             H5T_NATIVE_DOUBLE
     */
    hid_t     dataset_id, dataspace_id;
    hsize_t   dims[4];
    herr_t    status;

    /* Write the x coordinate array. */
    dims[0] = (NX + 1);
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(file_id, "/X_1D", H5T_NATIVE_FLOAT, dataspace_id,
                           H5P_DEFAULT);

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, x);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    /* Write the y coordinate array. */
    dims[0] = (NY + 1);
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(file_id, "/Y_1D", H5T_NATIVE_FLOAT, dataspace_id,
                           H5P_DEFAULT);

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, y);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    /* Write the z coordinate array. */
    dims[0] = (NZ + 1);
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(file_id, "/Z_1D", H5T_NATIVE_FLOAT, dataspace_id,
                           H5P_DEFAULT);

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, z);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    /*
     * Free the data.
     */
    free(x);
    free(y);
    free(z);
}

void
write_hdf5_3d_curv_data(hid_t file_id)
{
    /*
     * Create the coordinate data.
     */
    float origin[3] = {0., 0., 0.};
    float dxdydz[3] = {1., 2., 3.};

    float *x = (float *) malloc((NX+1)*(NY+1)*(NZ+1) * sizeof(float));
    float *y = (float *) malloc((NX+1)*(NY+1)*(NZ+1) * sizeof(float));
    float *z = (float *) malloc((NX+1)*(NY+1)*(NZ+1) * sizeof(float));

    int ndx = 0;
    for (int k = 0; k < NZ+1; k++)
    {
        for (int j = 0; j < NY+1; j++)
        {
            for (int i = 0; i < NX+1; i++)
            {
                x[ndx]   = (float) i;
                y[ndx]   = (float) j;
                z[ndx++] = (float) k;
            }
        }
    }

    float *xyz = (float *) malloc((NX+1)*(NY+1)*(NZ+1)*3 * sizeof(float));

    ndx = 0;
    for (int k = 0; k < NZ+1; k++)
    {
        for (int j = 0; j < NY+1; j++)
        {
            for (int i = 0; i < NX+1; i++)
            {
                xyz[ndx++] = (float) i;
                xyz[ndx++] = (float) j;
                xyz[ndx++] = (float) k;
            }
        }
    }

    float *xyz2 = (float *) malloc((NX2+1)*(NY2+1)*(NZ2+1)*3 * sizeof(float));

    ndx = 0;
    for (int k = 0; k < NZ2+1; k++)
    {
        for (int j = 0; j < NY2+1; j++)
        {
            for (int i = 0; i < NX2+1; i++)
            {
                xyz2[ndx++] = (float) (i + NX);
                xyz2[ndx++] = (float) j;
                xyz2[ndx++] = (float) k;
            }
        }
    }

    float *xyz3 = (float *) malloc((NX3+1)*(NY3+1)*(NZ3+1)*3 * sizeof(float));

    ndx = 0;
    for (int k = 0; k < NZ3+1; k++)
    {
        for (int j = 0; j < NY3+1; j++)
        {
            for (int i = 0; i < NX3+1; i++)
            {
                xyz3[ndx++] = (float) (i + NX + NX2);
                xyz3[ndx++] = (float) j;
                xyz3[ndx++] = (float) k;
            }
        }
    }

    /*
     * Create the scalar data.
     */
    float *pressure = (float *) malloc(NX*NY*NZ * sizeof(float));

    for (int k = 0; k < NZ; k++)
    {
        for (int j = 0; j < NY; j++)
        {
            for (int i = 0; i < NX; i++)
            {
                int ndx = k * NX * NY + j * NX + i;
                pressure[ndx] = (float) k;
            }
        }
    }

    float *pressure2 = (float *) malloc(NX2*NY2*NZ2 * sizeof(float));

    for (int k = 0; k < NZ2; k++)
    {
        for (int j = 0; j < NY2; j++)
        {
            for (int i = 0; i < NX2; i++)
            {
                int ndx = k * NX2 * NY2 + j * NX2 + i;
                pressure2[ndx] = (float) k;
            }
        }
    }

    float *pressure3 = (float *) malloc(NX3*NY3*NZ3 * sizeof(float));

    for (int k = 0; k < NZ3; k++)
    {
        for (int j = 0; j < NY3; j++)
        {
            for (int i = 0; i < NX3; i++)
            {
                int ndx = k * NX3 * NY3 + j * NX3 + i;
                pressure3[ndx] = (float) k;
            }
        }
    }

    float *velocityz = (float *) malloc((NX+1)*(NY+1)*(NZ+1) * sizeof(float));

    for (int k = 0; k < NZ+1; k++)
    {
        for (int j = 0; j < NY+1; j++)
        {
            for (int i = 0; i < NX+1; i++)
            {
                int ndx = k * (NX+1) * (NY+1) + j * (NX+1) + i;
                velocityz[ndx] = (float) k;
            }
        }
    }

    float *velocityz2 = (float *) malloc((NX2+1)*(NY2+1)*(NZ2+1) * sizeof(float));

    for (int k = 0; k < NZ2+1; k++)
    {
        for (int j = 0; j < NY2+1; j++)
        {
            for (int i = 0; i < NX2+1; i++)
            {
                int ndx = k * (NX2+1) * (NY2+1) + j * (NX2+1) + i;
                velocityz2[ndx] = (float) k;
            }
        }
    }

    float *velocityz3 = (float *) malloc((NX3+1)*(NY3+1)*(NZ3+1) * sizeof(float));

    for (int k = 0; k < NZ3+1; k++)
    {
        for (int j = 0; j < NY3+1; j++)
        {
            for (int i = 0; i < NX3+1; i++)
            {
                int ndx = k * (NX3+1) * (NY3+1) + j * (NX3+1) + i;
                velocityz3[ndx] = (float) k;
            }
        }
    }

    /*
     * Create the vector data.
     */
    float *velocity = (float *) malloc(3*(NX+1)*(NY+1)*(NZ+1) * sizeof(float));

    ndx = 0;
    for (int k = 0; k < NZ+1; k++)
    {
        for (int j = 0; j < NY+1; j++)
        {
            for (int i = 0; i < NX+1; i++)
            {
                velocity[ndx++] = 0.;
                velocity[ndx++] = 0.;
                velocity[ndx++] = (float) k;
            }
        }
    }

    float *velocity2 = (float *) malloc(3*(NX2+1)*(NY2+1)*(NZ2+1) * sizeof(float));

    ndx = 0;
    for (int k = 0; k < NZ2+1; k++)
    {
        for (int j = 0; j < NY2+1; j++)
        {
            for (int i = 0; i < NX2+1; i++)
            {
                velocity2[ndx++] = 0.;
                velocity2[ndx++] = 0.;
                velocity2[ndx++] = (float) k;
            }
        }
    }

    float *velocity3 = (float *) malloc(3*(NX3+1)*(NY3+1)*(NZ3+1) * sizeof(float));

    ndx = 0;
    for (int k = 0; k < NZ3+1; k++)
    {
        for (int j = 0; j < NY3+1; j++)
        {
            for (int i = 0; i < NX3+1; i++)
            {
                velocity3[ndx++] = 0.;
                velocity3[ndx++] = 0.;
                velocity3[ndx++] = (float) k;
            }
        }
    }

    /*
     * Create the tensor data.
     */
    float *stress = (float *) malloc(6*(NX+1)*(NY+1)*(NZ+1) * sizeof(float));

    ndx = 0;
    for (int k = 0; k < NZ+1; k++)
    {
        for (int j = 0; j < NY+1; j++)
        {
            for (int i = 0; i < NX+1; i++)
            {
                stress[ndx++] = 1.;
                stress[ndx++] = 0.;
                stress[ndx++] = 0.;
                stress[ndx++] = 2.;
                stress[ndx++] = 0.;
                stress[ndx++] = 1.;
            }
        }
    }

    float *stress2 = (float *) malloc(6*(NX2+1)*(NY2+1)*(NZ2+1) * sizeof(float));

    ndx = 0;
    for (int k = 0; k < NZ2+1; k++)
    {
        for (int j = 0; j < NY2+1; j++)
        {
            for (int i = 0; i < NX2+1; i++)
            {
                stress2[ndx++] = 1.;
                stress2[ndx++] = 0.;
                stress2[ndx++] = 0.;
                stress2[ndx++] = 2.;
                stress2[ndx++] = 0.;
                stress2[ndx++] = 1.;
            }
        }
    }

    float *stress3 = (float *) malloc(6*(NX3+1)*(NY3+1)*(NZ3+1) * sizeof(float));

    ndx = 0;
    for (int k = 0; k < NZ3+1; k++)
    {
        for (int j = 0; j < NY3+1; j++)
        {
            for (int i = 0; i < NX3+1; i++)
            {
                stress3[ndx++] = 1.;
                stress3[ndx++] = 0.;
                stress3[ndx++] = 0.;
                stress3[ndx++] = 2.;
                stress3[ndx++] = 0.;
                stress3[ndx++] = 1.;
            }
        }
    }

    /*
     * Write the data file.
     */
    /*
     * Data types: H5T_NATIVE_CHAR
     *             H5T_NATIVE_INT
     *             H5T_NATIVE_LONG
     *             H5T_NATIVE_FLOAT
     *             H5T_NATIVE_DOUBLE
     */
    hid_t     dataset_id, dataspace_id;
    hsize_t   dims[3];
    herr_t    status;

    /*
     * Write the coordinate data.
     */ 
    /* Write Origin and DxDyDz. */
    dims[0] = 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(file_id, "/Origin", H5T_NATIVE_FLOAT, dataspace_id,
                           H5P_DEFAULT);

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, origin);

    status = H5Dclose(dataset_id);

    dataset_id = H5Dcreate(file_id, "/DxDyDz", H5T_NATIVE_FLOAT, dataspace_id,
                           H5P_DEFAULT);

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, dxdydz);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    /* Write individual x, y and z coordinate arrays. */
    dims[0] = (NZ + 1) * (NY + 1) * (NX + 1);
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(file_id, "/X", H5T_NATIVE_FLOAT, dataspace_id,
                           H5P_DEFAULT);

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, x);

    status = H5Dclose(dataset_id);

    dataset_id = H5Dcreate(file_id, "/Y", H5T_NATIVE_FLOAT, dataspace_id,
                           H5P_DEFAULT);

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, y);

    status = H5Dclose(dataset_id);

    dataset_id = H5Dcreate(file_id, "/Z", H5T_NATIVE_FLOAT, dataspace_id,
                           H5P_DEFAULT);

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, z);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    /* Write a single coordinate array with x, y and z coordinates. */
    dims[0] = (NZ + 1) * (NY + 1) * (NX + 1);
    dims[1] = 3;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(file_id, "/XYZ", H5T_NATIVE_FLOAT, dataspace_id,
                           H5P_DEFAULT);

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, xyz);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    /* Write a second single coordinate array with x, y and z coordinates. */
    dims[0] = (NZ2 + 1) * (NY2 + 1) * (NX2 + 1);
    dims[1] = 3;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(file_id, "/XYZ2", H5T_NATIVE_FLOAT, dataspace_id,
                           H5P_DEFAULT);

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, xyz2);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    /* Write a third single coordinate array with x, y and z coordinates. */
    dims[0] = (NZ3 + 1) * (NY3 + 1) * (NX3 + 1);
    dims[1] = 3;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(file_id, "/XYZ3", H5T_NATIVE_FLOAT, dataspace_id,
                           H5P_DEFAULT);

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, xyz3);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    /*
     * Write the scalar data.
     */ 
    dims[0] = NZ;
    dims[1] = NY;
    dims[2] = NX;
    dataspace_id = H5Screate_simple(3, dims, NULL);

    dataset_id = H5Dcreate(file_id, "/Pressure", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, pressure);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    dims[0] = NZ2;
    dims[1] = NY2;
    dims[2] = NX2;
    dataspace_id = H5Screate_simple(3, dims, NULL);

    dataset_id = H5Dcreate(file_id, "/Pressure2", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, pressure2);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    dims[0] = NZ3;
    dims[1] = NY3;
    dims[2] = NX3;
    dataspace_id = H5Screate_simple(3, dims, NULL);

    dataset_id = H5Dcreate(file_id, "/Pressure3", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, pressure3);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    dims[0] = NZ+1;
    dims[1] = NY+1;
    dims[2] = NX+1;
    dataspace_id = H5Screate_simple(3, dims, NULL);

    dataset_id = H5Dcreate(file_id, "/VelocityZ", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, velocityz);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    dims[0] = NZ2+1;
    dims[1] = NY2+1;
    dims[2] = NX2+1;
    dataspace_id = H5Screate_simple(3, dims, NULL);

    dataset_id = H5Dcreate(file_id, "/VelocityZ2", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, velocityz2);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    dims[0] = NZ3+1;
    dims[1] = NY3+1;
    dims[2] = NX3+1;
    dataspace_id = H5Screate_simple(3, dims, NULL);

    dataset_id = H5Dcreate(file_id, "/VelocityZ3", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, velocityz3);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    /*
     * Write the vector data.
     */ 
    dims[0] = NZ+1;
    dims[1] = NY+1;
    dims[2] = NX+1;
    dims[3] = 3;
    dataspace_id = H5Screate_simple(4, dims, NULL);

    dataset_id = H5Dcreate(file_id, "/Velocity", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, velocity);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    dims[0] = NZ2+1;
    dims[1] = NY2+1;
    dims[2] = NX2+1;
    dataspace_id = H5Screate_simple(4, dims, NULL);

    dataset_id = H5Dcreate(file_id, "/Velocity2", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, velocity2);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    dims[0] = NZ3+1;
    dims[1] = NY3+1;
    dims[2] = NX3+1;
    dataspace_id = H5Screate_simple(4, dims, NULL);

    dataset_id = H5Dcreate(file_id, "/Velocity3", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, velocity3);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    /*
     * Write the tensor data.
     */ 
    dims[0] = NZ+1;
    dims[1] = NY+1;
    dims[2] = NX+1;
    dims[3] = 6;
    dataspace_id = H5Screate_simple(4, dims, NULL);

    dataset_id = H5Dcreate(file_id, "/Stress", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, stress);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    dims[0] = NZ2+1;
    dims[1] = NY2+1;
    dims[2] = NX2+1;
    dataspace_id = H5Screate_simple(4, dims, NULL);

    dataset_id = H5Dcreate(file_id, "/Stress2", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, stress2);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    dims[0] = NZ3+1;
    dims[1] = NY3+1;
    dims[2] = NX3+1;
    dataspace_id = H5Screate_simple(4, dims, NULL);

    dataset_id = H5Dcreate(file_id, "/Stress3", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, stress3);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    /*
     * Free the data.
     */
    free(x);
    free(y);
    free(z);
    free(xyz);
    free(pressure);
    free(velocityz);
    free(velocity);
    free(stress);
    free(xyz2);
    free(pressure2);
    free(velocityz2);
    free(velocity2);
    free(stress2);
    free(xyz3);
    free(pressure3);
    free(velocityz3);
    free(velocity3);
    free(stress3);
}

void
create_hdf5_file()
{
    /*
     * Create the data file.
     */
    hid_t     file_id;
    file_id = H5Fcreate("mesh.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

    write_hdf5_2d_curv_data(file_id);
    write_hdf5_3d_rect_data(file_id);
    write_hdf5_3d_curv_data(file_id);

    /*
     * Close the file.
     */
    herr_t    status;
    status = H5Fclose(file_id);
}

void
create_collection3d()
{
    FILE *xmf = 0;

    /*
     * Open the file and write the header.
     */
    xmf = fopen("collection3d.xmf", "w");
    fprintf(xmf, "<?xml version=\"1.0\" ?>\n");
    fprintf(xmf, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
    fprintf(xmf, "<Xdmf Version=\"2.0\">\n");

    /*
     * Write the mesh description and the variables defined on the mesh.
     */
    fprintf(xmf, " <Domain>\n");

    fprintf(xmf, "   <Grid Name=\"mesh\" GridType=\"Collection\">\n");

    fprintf(xmf, "   <Grid Name=\"mesh1\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ+1)*(NY+1)*(NX+1), 3);
    fprintf(xmf, "        mesh.h5:/XYZ\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ, NY, NX);
    fprintf(xmf, "        mesh.h5:/Pressure\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");


    fprintf(xmf, "   <Grid Name=\"mesh2\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ2+1, NY2+1, NX2+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ2+1)*(NY2+1)*(NX2+1), 3);
    fprintf(xmf, "        mesh.h5:/XYZ2\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ2, NY2, NX2);
    fprintf(xmf, "        mesh.h5:/Pressure2\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, " </Domain>\n");

    /*
     * Write the footer and close the file.
     */
    fprintf(xmf, "</Xdmf>\n");
    fclose(xmf);
}

void
create_collection_partial_var()
{
    FILE *xmf = 0;

    /*
     * Open the file and write the header.
     */
    xmf = fopen("collection_partial_var.xmf", "w");
    fprintf(xmf, "<?xml version=\"1.0\" ?>\n");
    fprintf(xmf, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
    fprintf(xmf, "<Xdmf Version=\"2.0\">\n");

    /*
     * Write the mesh description and the variables defined on the mesh.
     */
    fprintf(xmf, " <Domain>\n");

    fprintf(xmf, "   <Grid Name=\"mesh\" GridType=\"Collection\">\n");

    fprintf(xmf, "   <Grid Name=\"mesh1\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ+1)*(NY+1)*(NX+1), 3);
    fprintf(xmf, "        mesh.h5:/XYZ\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "   </Grid>\n");


    fprintf(xmf, "   <Grid Name=\"mesh2\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ2+1, NY2+1, NX2+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ2+1)*(NY2+1)*(NX2+1), 3);
    fprintf(xmf, "        mesh.h5:/XYZ2\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ2, NY2, NX2);
    fprintf(xmf, "        mesh.h5:/Pressure2\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, " </Domain>\n");

    /*
     * Write the footer and close the file.
     */
    fprintf(xmf, "</Xdmf>\n");
    fclose(xmf);
}

void
create_collection_partial_var2()
{
    FILE *xmf = 0;

    /*
     * Open the file and write the header.
     */
    xmf = fopen("collection_partial_var2.xmf", "w");
    fprintf(xmf, "<?xml version=\"1.0\" ?>\n");
    fprintf(xmf, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
    fprintf(xmf, "<Xdmf Version=\"2.0\">\n");

    /*
     * Write the mesh description and the variables defined on the mesh.
     */
    fprintf(xmf, " <Domain>\n");

    fprintf(xmf, "   <Grid Name=\"mesh\" GridType=\"Collection\">\n");

    fprintf(xmf, "   <Grid Name=\"mesh1\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ+1)*(NY+1)*(NX+1), 3);
    fprintf(xmf, "        mesh.h5:/XYZ\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ, NY, NX);
    fprintf(xmf, "        mesh.h5:/Pressure\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");


    fprintf(xmf, "   <Grid Name=\"mesh2\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ2+1, NY2+1, NX2+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ2+1)*(NY2+1)*(NX2+1), 3);
    fprintf(xmf, "        mesh.h5:/XYZ2\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, " </Domain>\n");

    /*
     * Write the footer and close the file.
     */
    fprintf(xmf, "</Xdmf>\n");
    fclose(xmf);
}

void
create_tree3d()
{
    FILE *xmf = 0;

    /*
     * Open the file and write the header.
     */
    xmf = fopen("tree3d.xmf", "w");
    fprintf(xmf, "<?xml version=\"1.0\" ?>\n");
    fprintf(xmf, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
    fprintf(xmf, "<Xdmf Version=\"2.0\">\n");

    /*
     * Write the mesh description and the variables defined on the mesh.
     */
    fprintf(xmf, " <Domain>\n");

    fprintf(xmf, "   <Grid Name=\"mesh\" GridType=\"Tree\">\n");

    fprintf(xmf, "   <Grid Name=\"mesh\" GridType=\"Collection\">\n");

    fprintf(xmf, "   <Grid Name=\"mesh1\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ+1)*(NY+1)*(NX+1), 3);
    fprintf(xmf, "        mesh.h5:/XYZ\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ, NY, NX);
    fprintf(xmf, "        mesh.h5:/Pressure\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");


    fprintf(xmf, "   <Grid Name=\"mesh2\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ2+1, NY2+1, NX2+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ2+1)*(NY2+1)*(NX2+1), 3);
    fprintf(xmf, "        mesh.h5:/XYZ2\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ2, NY2, NX2);
    fprintf(xmf, "        mesh.h5:/Pressure2\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, "   <Grid Name=\"mesh3\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ3+1, NY3+1, NX3+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ3+1)*(NY3+1)*(NX3+1), 3);
    fprintf(xmf, "        mesh.h5:/XYZ3\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ3, NY3, NX3);
    fprintf(xmf, "        mesh.h5:/Pressure3\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, " </Domain>\n");

    /*
     * Write the footer and close the file.
     */
    fprintf(xmf, "</Xdmf>\n");
    fclose(xmf);
}

void
create_multi_domain3d()
{
    FILE *xmf = 0;

    /*
     * Open the file and write the header.
     */
    xmf = fopen("multi_domain3d.xmf", "w");
    fprintf(xmf, "<?xml version=\"1.0\" ?>\n");
    fprintf(xmf, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
    fprintf(xmf, "<Xdmf Version=\"2.0\">\n");

    /*
     * Write the mesh description and the variables defined on the mesh.
     */
    fprintf(xmf, " <Domain>\n");

    fprintf(xmf, "   <Grid Name=\"mesh1\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ+1)*(NY+1)*(NX+1), 3);
    fprintf(xmf, "        mesh.h5:/XYZ\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ, NY, NX);
    fprintf(xmf, "        mesh.h5:/Pressure\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, " </Domain>\n");

    fprintf(xmf, " <Domain>\n");

    fprintf(xmf, "   <Grid Name=\"mesh1\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ2+1, NY2+1, NX2+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ2+1)*(NY2+1)*(NX2+1), 3);
    fprintf(xmf, "        mesh.h5:/XYZ2\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ2, NY2, NX2);
    fprintf(xmf, "        mesh.h5:/Pressure2\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, "   <Grid Name=\"mesh2\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ3+1, NY3+1, NX3+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ3+1)*(NY3+1)*(NX3+1), 3);
    fprintf(xmf, "        mesh.h5:/XYZ3\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ3, NY3, NX3);
    fprintf(xmf, "        mesh.h5:/Pressure3\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, " </Domain>\n");

    /*
     * Write the footer and close the file.
     */
    fprintf(xmf, "</Xdmf>\n");
    fclose(xmf);
}

void
create_multi_grid3d()
{
    FILE *xmf = 0;

    /*
     * Open the file and write the header.
     */
    xmf = fopen("multi_grid3d.xmf", "w");
    fprintf(xmf, "<?xml version=\"1.0\" ?>\n");
    fprintf(xmf, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
    fprintf(xmf, "<Xdmf Version=\"2.0\">\n");

    /*
     * Write the mesh description and the variables defined on the mesh.
     */
    fprintf(xmf, " <Domain>\n");

    fprintf(xmf, "   <Grid Name=\"mesh1\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ+1)*(NY+1)*(NX+1), 3);
    fprintf(xmf, "        mesh.h5:/XYZ\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ, NY, NX);
    fprintf(xmf, "        mesh.h5:/Pressure\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, "   <Grid Name=\"mesh2\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ2+1, NY2+1, NX2+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ2+1)*(NY2+1)*(NX2+1), 3);
    fprintf(xmf, "        mesh.h5:/XYZ2\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ2, NY2, NX2);
    fprintf(xmf, "        mesh.h5:/Pressure2\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, " </Domain>\n");

    /*
     * Write the footer and close the file.
     */
    fprintf(xmf, "</Xdmf>\n");
    fclose(xmf);
}

void
create_uniform3d()
{
    FILE *xmf = 0;

    /*
     * Open the file and write the header.
     */
    xmf = fopen("uniform3d.xmf", "w");
    fprintf(xmf, "<?xml version=\"1.0\" ?>\n");
    fprintf(xmf, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
    fprintf(xmf, "<Xdmf Version=\"2.0\">\n");

    /*
     * Write the mesh description and the variables defined on the mesh.
     */
    fprintf(xmf, " <Domain>\n");

    fprintf(xmf, "   <Grid Name=\"mesh\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ+1)*(NY+1)*(NX+1), 3);
    fprintf(xmf, "        mesh.h5:/XYZ\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ, NY, NX);
    fprintf(xmf, "        mesh.h5:/Pressure\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "     <Attribute Name=\"VelocityZ\" AttributeType=\"Scalar\" Center=\"Node\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "        mesh.h5:/VelocityZ\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "     <Attribute Name=\"Velocity\" AttributeType=\"Vector\" Center=\"Node\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d 3\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "        mesh.h5:/Velocity\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "     <Attribute Name=\"Stress\" AttributeType=\"Tensor6\" Center=\"Node\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d 6\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "        mesh.h5:/Stress\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, " </Domain>\n");

    /*
     * Write the footer and close the file.
     */
    fprintf(xmf, "</Xdmf>\n");
    fclose(xmf);
}

void
create_xml_data()
{
    FILE *xmf = 0;

    /*
     * Open the file and write the header.
     */
    xmf = fopen("xml_data.xmf", "w");
    fprintf(xmf, "<?xml version=\"1.0\" ?>\n");
    fprintf(xmf, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
    fprintf(xmf, "<Xdmf Version=\"2.0\">\n");

    /*
     * Write the mesh description and the variables defined on the mesh.
     */
    fprintf(xmf, " <Domain>\n");

    fprintf(xmf, "   <Grid Name=\"mesh\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DCoRectMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"Origin_DxDyDz\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d\" NumberType=\"Float\" Precision=\"4\" Format=\"XML\">\n", 3);
    fprintf(xmf, "        0. 0. 0.\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d\" NumberType=\"Float\" Precision=\"4\" Format=\"XML\">\n", 3);
    fprintf(xmf, "        1. 2. 3.\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"XML\">\n", NZ, NY, NX);
    for (int k = 0; k < NZ; k++)
    {
        for (int j = 0; j < NY; j++)
        {
            for (int i = 0; i < NX; i += 5)
            {
                fprintf(xmf, "       ");
                for (int ii = i; ii < ((i + 5 < NX) ? i + 5 : NX); ii++)
                    fprintf(xmf, " %f", (float) k);
                fprintf(xmf, "\n");
            }
        }
    }
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "     <Attribute Name=\"VelocityZ\" AttributeType=\"Scalar\" Center=\"Node\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"XML\">\n", NZ+1, NY+1, NX+1);
    for (int k = 0; k < NZ+1; k++)
    {
        for (int j = 0; j < NY+1; j++)
        {
            for (int i = 0; i < NX+1; i += 5)
            {
                fprintf(xmf, "       ");
                for (int ii = i; ii < ((i + 5 < NX+1) ? i + 5 : NX+1); ii++)
                    fprintf(xmf, " %f", (float) k);
                fprintf(xmf, "\n");
            }
        }
    }
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, " </Domain>\n");

    /*
     * Write the footer and close the file.
     */
    fprintf(xmf, "</Xdmf>\n");
    fclose(xmf);
}

void
create_corect2d()
{
    FILE *xmf = 0;

    /*
     * Open the file and write the header.
     */
    xmf = fopen("corect2d.xmf", "w");
    fprintf(xmf, "<?xml version=\"1.0\" ?>\n");
    fprintf(xmf, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
    fprintf(xmf, "<Xdmf Version=\"2.0\">\n");

    /*
     * Write the mesh description and the variables defined on the mesh.
     */
    fprintf(xmf, " <Domain>\n");

    fprintf(xmf, "   <Grid Name=\"mesh\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"2DCoRectMesh\" NumberOfElements=\"%d %d\"/>\n", NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"Origin_DxDyDz\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", 3);
    fprintf(xmf, "        mesh.h5:/Origin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", 3);
    fprintf(xmf, "        mesh.h5:/DxDyDz\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NY, NX);
    fprintf(xmf, "        mesh.h5:/Pressure_2D\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "     <Attribute Name=\"VelocityX\" AttributeType=\"Scalar\" Center=\"Node\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NY+1, NX+1);
    fprintf(xmf, "        mesh.h5:/VelocityX_2D\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, " </Domain>\n");

    /*
     * Write the footer and close the file.
     */
    fprintf(xmf, "</Xdmf>\n");
    fclose(xmf);
}

void
create_corect3d()
{
    FILE *xmf = 0;

    /*
     * Open the file and write the header.
     */
    xmf = fopen("corect3d.xmf", "w");
    fprintf(xmf, "<?xml version=\"1.0\" ?>\n");
    fprintf(xmf, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
    fprintf(xmf, "<Xdmf Version=\"2.0\">\n");

    /*
     * Write the mesh description and the variables defined on the mesh.
     */
    fprintf(xmf, " <Domain>\n");

    fprintf(xmf, "   <Grid Name=\"mesh\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DCoRectMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"Origin_DxDyDz\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", 3);
    fprintf(xmf, "        mesh.h5:/Origin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", 3);
    fprintf(xmf, "        mesh.h5:/DxDyDz\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ, NY, NX);
    fprintf(xmf, "        mesh.h5:/Pressure\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "     <Attribute Name=\"VelocityZ\" AttributeType=\"Scalar\" Center=\"Node\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "        mesh.h5:/VelocityZ\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, " </Domain>\n");

    /*
     * Write the footer and close the file.
     */
    fprintf(xmf, "</Xdmf>\n");
    fclose(xmf);
}

void
create_rect2d()
{
    FILE *xmf = 0;

    /*
     * Open the file and write the header.
     */
    xmf = fopen("rect2d.xmf", "w");
    fprintf(xmf, "<?xml version=\"1.0\" ?>\n");
    fprintf(xmf, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
    fprintf(xmf, "<Xdmf Version=\"2.0\">\n");

    /*
     * Write the mesh description and the variables defined on the mesh.
     */
    fprintf(xmf, " <Domain>\n");

    fprintf(xmf, "   <Grid Name=\"mesh\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"2DRectMesh\" NumberOfElements=\"%d %d\"/>\n", NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"X_Y_Z\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NX+1);
    fprintf(xmf, "        mesh.h5:/X_1D\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NY+1);
    fprintf(xmf, "        mesh.h5:/Y_1D\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NY, NX);
    fprintf(xmf, "        mesh.h5:/Pressure_2D\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "     <Attribute Name=\"VelocityX\" AttributeType=\"Scalar\" Center=\"Node\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NY+1, NX+1);
    fprintf(xmf, "        mesh.h5:/VelocityX_2D\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, " </Domain>\n");

    /*
     * Write the footer and close the file.
     */
    fprintf(xmf, "</Xdmf>\n");
    fclose(xmf);
}

void
create_rect3d()
{
    FILE *xmf = 0;

    /*
     * Open the file and write the header.
     */
    xmf = fopen("rect3d.xmf", "w");
    fprintf(xmf, "<?xml version=\"1.0\" ?>\n");
    fprintf(xmf, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
    fprintf(xmf, "<Xdmf Version=\"2.0\">\n");

    /*
     * Write the mesh description and the variables defined on the mesh.
     */
    fprintf(xmf, " <Domain>\n");

    fprintf(xmf, "   <Grid Name=\"mesh\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DRectMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"X_Y_Z\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NX+1);
    fprintf(xmf, "        mesh.h5:/X_1D\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NY+1);
    fprintf(xmf, "        mesh.h5:/Y_1D\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ+1);
    fprintf(xmf, "        mesh.h5:/Z_1D\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ, NY, NX);
    fprintf(xmf, "        mesh.h5:/Pressure\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "     <Attribute Name=\"VelocityZ\" AttributeType=\"Scalar\" Center=\"Node\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "        mesh.h5:/VelocityZ\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, " </Domain>\n");

    /*
     * Write the footer and close the file.
     */
    fprintf(xmf, "</Xdmf>\n");
    fclose(xmf);
}

void
create_err_dataitem1()
{
    FILE *xmf = 0;

    /*
     * Open the file and write the header.
     */
    xmf = fopen("err_dataitem1.xmf", "w");
    fprintf(xmf, "<?xml version=\"1.0\" ?>\n");
    fprintf(xmf, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
    fprintf(xmf, "<Xdmf Version=\"2.0\">\n");

    /*
     * Write the mesh description and the variables defined on the mesh.
     */
    fprintf(xmf, " <Domain>\n");

    fprintf(xmf, "   <Grid Name=\"mesh\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ+1)*(NY+1)*(NX+1), 3);
    fprintf(xmf, "        mesh.h5:/XYZ\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\"></DataItem>\n", NZ, NY, NX);
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, " </Domain>\n");

    /*
     * Write the footer and close the file.
     */
    fprintf(xmf, "</Xdmf>\n");
    fclose(xmf);
}

void
create_err_dataitem2()
{
    FILE *xmf = 0;

    /*
     * Open the file and write the header.
     */
    xmf = fopen("err_dataitem2.xmf", "w");
    fprintf(xmf, "<?xml version=\"1.0\" ?>\n");
    fprintf(xmf, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
    fprintf(xmf, "<Xdmf Version=\"2.0\">\n");

    /*
     * Write the mesh description and the variables defined on the mesh.
     */
    fprintf(xmf, " <Domain>\n");

    fprintf(xmf, "   <Grid Name=\"mesh\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ+1)*(NY+1)*(NX+1), 3);
    fprintf(xmf, "        :/XYZ\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ, NY, NX);
    fprintf(xmf, "        mesh.h5:/Pressure\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, " </Domain>\n");

    /*
     * Write the footer and close the file.
     */
    fprintf(xmf, "</Xdmf>\n");
    fclose(xmf);
}

void
create_err_dataitem3()
{
    FILE *xmf = 0;

    /*
     * Open the file and write the header.
     */
    xmf = fopen("err_dataitem3.xmf", "w");
    fprintf(xmf, "<?xml version=\"1.0\" ?>\n");
    fprintf(xmf, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
    fprintf(xmf, "<Xdmf Version=\"2.0\">\n");

    /*
     * Write the mesh description and the variables defined on the mesh.
     */
    fprintf(xmf, " <Domain>\n");

    fprintf(xmf, "   <Grid Name=\"mesh\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ+1)*(NY+1)*(NX+1), 3);
    fprintf(xmf, "        /XYZ\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ, NY, NX);
    fprintf(xmf, "        mesh.h5:/Pressure\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, " </Domain>\n");

    /*
     * Write the footer and close the file.
     */
    fprintf(xmf, "</Xdmf>\n");
    fclose(xmf);
}

void
create_err_dataitem4()
{
    FILE *xmf = 0;

    /*
     * Open the file and write the header.
     */
    xmf = fopen("err_dataitem4.xmf", "w");
    fprintf(xmf, "<?xml version=\"1.0\" ?>\n");
    fprintf(xmf, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
    fprintf(xmf, "<Xdmf Version=\"2.0\">\n");

    /*
     * Write the mesh description and the variables defined on the mesh.
     */
    fprintf(xmf, " <Domain>\n");

    fprintf(xmf, "   <Grid Name=\"mesh\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ+1)*(NY+1)*(NX+1), 3);
    fprintf(xmf, "        mesh.h5:\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ, NY, NX);
    fprintf(xmf, "        mesh.h5:/Pressure\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, " </Domain>\n");

    /*
     * Write the footer and close the file.
     */
    fprintf(xmf, "</Xdmf>\n");
    fclose(xmf);
}

void
create_err_dataitem5()
{
    FILE *xmf = 0;

    /*
     * Open the file and write the header.
     */
    xmf = fopen("err_dataitem5.xmf", "w");
    fprintf(xmf, "<?xml version=\"1.0\" ?>\n");
    fprintf(xmf, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
    fprintf(xmf, "<Xdmf Version=\"2.0\">\n");

    /*
     * Write the mesh description and the variables defined on the mesh.
     */
    fprintf(xmf, " <Domain>\n");

    fprintf(xmf, "   <Grid Name=\"mesh\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ+1)*(NY+1)*(NX+1), 3);
    fprintf(xmf, "        xxxx.h5:/XYZ\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ, NY, NX);
    fprintf(xmf, "        mesh.h5:/Pressure\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, " </Domain>\n");

    /*
     * Write the footer and close the file.
     */
    fprintf(xmf, "</Xdmf>\n");
    fclose(xmf);
}

void
create_err_dataitem6()
{
    FILE *xmf = 0;

    /*
     * Open the file and write the header.
     */
    xmf = fopen("err_dataitem6.xmf", "w");
    fprintf(xmf, "<?xml version=\"1.0\" ?>\n");
    fprintf(xmf, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
    fprintf(xmf, "<Xdmf Version=\"2.0\">\n");

    /*
     * Write the mesh description and the variables defined on the mesh.
     */
    fprintf(xmf, " <Domain>\n");

    fprintf(xmf, "   <Grid Name=\"mesh\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ+1)*(NY+1)*(NX+1), 3);
    fprintf(xmf, "        mesh.h5:/xxxx\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ, NY, NX);
    fprintf(xmf, "        mesh.h5:/Pressure\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, " </Domain>\n");

    /*
     * Write the footer and close the file.
     */
    fprintf(xmf, "</Xdmf>\n");
    fclose(xmf);
}

void
create_err_dataitem7()
{
    FILE *xmf = 0;

    /*
     * Open the file and write the header.
     */
    xmf = fopen("err_dataitem7.xmf", "w");
    fprintf(xmf, "<?xml version=\"1.0\" ?>\n");
    fprintf(xmf, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
    fprintf(xmf, "<Xdmf Version=\"2.0\">\n");

    /*
     * Write the mesh description and the variables defined on the mesh.
     */
    fprintf(xmf, " <Domain>\n");

    fprintf(xmf, "   <Grid Name=\"mesh1\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"VXVYVZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ+2)*(NY+1)*(NX+1), 1);
    fprintf(xmf, "        mesh.h5:/X\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ+1)*(NY+1)*(NX+1), 1);
    fprintf(xmf, "        mesh.h5:/Y\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ+1)*(NY+1)*(NX+1), 1);
    fprintf(xmf, "        mesh.h5:/Z\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, "   <Grid Name=\"mesh2\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ+2, NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"VXVYVZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ+1)*(NY+1)*(NX+1), 1);
    fprintf(xmf, "        mesh.h5:/X\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ+1)*(NY+1)*(NX+1), 1);
    fprintf(xmf, "        mesh.h5:/Y\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ+1)*(NY+1)*(NX+1), 1);
    fprintf(xmf, "        mesh.h5:/Z\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, "   <Grid Name=\"mesh3\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"2DSMesh\" NumberOfElements=\"%d %d\"/>\n", NZ+1, NY+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XY\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NY+2)*(NX+1), 2);
    fprintf(xmf, "        mesh.h5:/XY\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, "   <Grid Name=\"mesh4\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"2DSMesh\" NumberOfElements=\"%d %d\"/>\n", NZ+2, NY+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XY\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NY+1)*(NX+1), 2);
    fprintf(xmf, "        mesh.h5:/XY\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, "   <Grid Name=\"mesh5\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ+2)*(NY+1)*(NX+1), 3);
    fprintf(xmf, "        mesh.h5:/XYZ\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, "   <Grid Name=\"mesh6\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ+2, NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ+1)*(NY+1)*(NX+1), 3);
    fprintf(xmf, "        mesh.h5:/XYZ\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, "   <Grid Name=\"mesh7\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ+1)*(NY+1)*(NX+1), 3);
    fprintf(xmf, "        mesh.h5:/XYZ\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ+1, NY, NX);
    fprintf(xmf, "        mesh.h5:/Pressure\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, " </Domain>\n");

    /*
     * Write the footer and close the file.
     */
    fprintf(xmf, "</Xdmf>\n");
    fclose(xmf);
}

void
create_err_dup_meshnames()
{
    FILE *xmf = 0;

    /*
     * Open the file and write the header.
     */
    xmf = fopen("err_dup_meshnames.xmf", "w");
    fprintf(xmf, "<?xml version=\"1.0\" ?>\n");
    fprintf(xmf, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
    fprintf(xmf, "<Xdmf Version=\"2.0\">\n");

    /*
     * Write the mesh description and the variables defined on the mesh.
     */
    fprintf(xmf, " <Domain>\n");

    fprintf(xmf, "   <Grid Name=\"mesh\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ+1)*(NY+1)*(NX+1), 3);
    fprintf(xmf, "        mesh.h5:/XYZ\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ, NY, NX);
    fprintf(xmf, "        mesh.h5:/Pressure\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");


    fprintf(xmf, "   <Grid Name=\"mesh\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ2+1, NY2+1, NX2+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ2+1)*(NY2+1)*(NX2+1), 3);
    fprintf(xmf, "        mesh.h5:/XYZ2\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ2, NY2, NX2);
    fprintf(xmf, "        mesh.h5:/Pressure2\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, " </Domain>\n");

    /*
     * Write the footer and close the file.
     */
    fprintf(xmf, "</Xdmf>\n");
    fclose(xmf);
}

void
create_err_dup_varnames()
{
    FILE *xmf = 0;

    /*
     * Open the file and write the header.
     */
    xmf = fopen("err_dup_varnames.xmf", "w");
    fprintf(xmf, "<?xml version=\"1.0\" ?>\n");
    fprintf(xmf, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
    fprintf(xmf, "<Xdmf Version=\"2.0\">\n");

    /*
     * Write the mesh description and the variables defined on the mesh.
     */
    fprintf(xmf, " <Domain>\n");

    fprintf(xmf, "   <Grid Name=\"mesh\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ+1)*(NY+1)*(NX+1), 3);
    fprintf(xmf, "        mesh.h5:/XYZ\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ, NY, NX);
    fprintf(xmf, "        mesh.h5:/Pressure\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ2, NY2, NX2);
    fprintf(xmf, "        mesh.h5:/Pressure2\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, " </Domain>\n");

    /*
     * Write the footer and close the file.
     */
    fprintf(xmf, "</Xdmf>\n");
    fclose(xmf);
}

void
create_err_dup_varnames2()
{
    FILE *xmf = 0;

    /*
     * Open the file and write the header.
     */
    xmf = fopen("err_dup_varnames2.xmf", "w");
    fprintf(xmf, "<?xml version=\"1.0\" ?>\n");
    fprintf(xmf, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
    fprintf(xmf, "<Xdmf Version=\"2.0\">\n");

    /*
     * Write the mesh description and the variables defined on the mesh.
     */
    fprintf(xmf, " <Domain>\n");

    fprintf(xmf, "   <Grid Name=\"mesh\" GridType=\"Collection\">\n");

    fprintf(xmf, "   <Grid Name=\"mesh1\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ+1)*(NY+1)*(NX+1), 3);
    fprintf(xmf, "        mesh.h5:/XYZ\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ, NY, NX);
    fprintf(xmf, "        mesh.h5:/Pressure\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ2, NY2, NX2);
    fprintf(xmf, "        mesh.h5:/Pressure2\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");


    fprintf(xmf, "   <Grid Name=\"mesh2\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ2+1, NY2+1, NX2+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NZ2+1)*(NY2+1)*(NX2+1), 3);
    fprintf(xmf, "        mesh.h5:/XYZ2\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NZ2, NY2, NX2);
    fprintf(xmf, "        mesh.h5:/Pressure2\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, " </Domain>\n");

    /*
     * Write the footer and close the file.
     */
    fprintf(xmf, "</Xdmf>\n");
    fclose(xmf);
}

int
main()
{
    create_hdf5_file();

    create_collection3d();
    create_collection_partial_var();
    create_collection_partial_var2();
    create_tree3d();
    create_multi_domain3d();
    create_multi_grid3d();
    create_uniform3d();
    create_xml_data();
    create_corect2d();
    create_corect3d();
    create_rect2d();
    create_rect3d();

    create_err_dataitem1();
    create_err_dataitem2();
    create_err_dataitem3();
    create_err_dataitem4();
    create_err_dataitem5();
    create_err_dataitem6();
    create_err_dataitem7();

    create_err_dup_meshnames();
    create_err_dup_varnames();
    create_err_dup_varnames2();
}
