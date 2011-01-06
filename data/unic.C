#include <stdio.h>
#include <stdlib.h>

#include <iostream>

using std::cerr;
using std::endl;

// Define this symbol BEFORE including hdf5.h to indicate the HDF5 code
// in this file uses version 1.6 of the HDF5 API. This is harmless for
// versions of HDF5 before 1.8 and ensures correct compilation with
// version 1.8 and thereafter. When, and if, the HDF5 code in this file
// is explicitly upgraded to the 1.8 API, this symbol should be removed.
#define H5_USE_16_API
#include <hdf5.h>

void
write_header(const hid_t file_id, const int ndims, const int nblocks)
{
    herr_t status;
    hsize_t dims[3];
    hid_t dataspace_id, dataset_id;

    //
    // Create the control structure.
    //
    dims[0] = 5;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(file_id, "CONTROL", H5T_NATIVE_INT,
                           dataspace_id, H5P_DEFAULT);

    int info[5];
    info[0] = nblocks;   // Number of blocks.
    info[1] = ndims;     // Spatial dimension.
    info[2] = 1;         // Number of point variables.
    info[3] = 1;         // Number of cell variables.
    info[4] = 20;        // Length of each variable name.

    status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, info);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    //
    // Create the vertex variable names.
    //
    dims[0] = 20;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(file_id, "VERTEX_VECTOR_NAMES", H5T_NATIVE_CHAR,
                           dataspace_id, H5P_DEFAULT);

    char point_names[21] = "xcoord              ";
    status = H5Dwrite(dataset_id, H5T_NATIVE_CHAR, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, point_names);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    //
    // Create the element variable names.
    //
    dims[0] = 20;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(file_id, "ELEMENT_VECTOR_NAMES", H5T_NATIVE_CHAR,
                           dataspace_id, H5P_DEFAULT);

    char cell_names[21] = "density             ";
    status = H5Dwrite(dataset_id, H5T_NATIVE_CHAR, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, cell_names);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);
}

void
write_quads(const hid_t file_id, const int iblock)
{
    herr_t status;
    hsize_t dims[3];
    hid_t dataspace_id, dataset_id;

    char block_name[40];
    sprintf(block_name, "BLOCK%012d", iblock);

    hid_t block_id;
    block_id = H5Gcreate(file_id, block_name, 100);

    // Write the block information.
    dims[0] = 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "INFO", H5T_NATIVE_INT,
                           dataspace_id, H5P_DEFAULT);

    int info[3];
    info[0] = 2;   // Number of elements.
    info[1] = 4;   // Vertices per element.
    info[2] = 9;   // Element type (VTK_QUAD).

    status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, info);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the coordinate information.
    int nx = 2, ny = 1;
    dims[0] = nx * ny * 4 * 2;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "XYZ", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *xyz = new float[nx*ny*4*2];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            xyz[(iy*nx+ix)*4*2+0] = (float)ix;
            xyz[(iy*nx+ix)*4*2+1] = (float)(ix+1);
            xyz[(iy*nx+ix)*4*2+2] = (float)(ix+1);
            xyz[(iy*nx+ix)*4*2+3] = (float)ix;
            xyz[(iy*nx+ix)*4*2+4+0] = (float)iy;
            xyz[(iy*nx+ix)*4*2+4+1] = (float)iy;
            xyz[(iy*nx+ix)*4*2+4+2] = (float)(iy+1);
            xyz[(iy*nx+ix)*4*2+4+3] = (float)(iy+1);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, xyz);

    delete [] xyz;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the point data.
    dims[1] = nx * ny * 4;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "VERTEXDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *vdata = new float[nx*ny*2];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            vdata[(iy*nx+ix)*4+0] = (float)ix;
            vdata[(iy*nx+ix)*4+1] = (float)(ix+1);
            vdata[(iy*nx+ix)*4+2] = (float)(ix+1);
            vdata[(iy*nx+ix)*4+3] = (float)ix;
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, vdata);

    delete [] vdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the cell data.
    dims[1] = nx * ny;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "ELEMENTDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *cdata = new float[nx*ny];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            cdata[(iy*nx+ix)] = (float)ix;
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, cdata);

    delete [] cdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Close the block.
    status = H5Gclose(block_id);
}

void
write_vtk100(const hid_t file_id, const int iblock)
{
    herr_t status;
    hsize_t dims[3];
    hid_t dataspace_id, dataset_id;

    char block_name[40];
    sprintf(block_name, "BLOCK%012d", iblock);

    hid_t block_id;
    block_id = H5Gcreate(file_id, block_name, 100);

    // Write the block information.
    dims[0] = 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "INFO", H5T_NATIVE_INT,
                           dataspace_id, H5P_DEFAULT);

    int info[3];
    info[0] = 2;   // Number of elements.
    info[1] = 3;   // Vertices per element.
    info[2] = 100; // Element type (linear triangle).

    status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, info);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the coordinate information.
    int nx = 2, ny = 1;
    dims[0] = nx * ny * 3 * 2;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "XYZ", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    int xoffset = 0, yoffset = 1;
    float *xyz = new float[nx*ny*3*2];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            xyz[(iy*nx+ix)*3*2+0] = (float)(xoffset+ix);
            xyz[(iy*nx+ix)*3*2+1] = (float)(xoffset+ix+1);
            xyz[(iy*nx+ix)*3*2+2] = (float)(xoffset+ix);
            xyz[(iy*nx+ix)*3*2+3+0] = (float)(yoffset+iy);
            xyz[(iy*nx+ix)*3*2+3+1] = (float)(yoffset+iy);
            xyz[(iy*nx+ix)*3*2+3+2] = (float)(yoffset+iy+1);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, xyz);

    delete [] xyz;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the point data.
    dims[1] = nx * ny * 3;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "VERTEXDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *vdata = new float[nx*ny*3];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            vdata[(iy*nx+ix)*3+0] = (float)(xoffset+ix);
            vdata[(iy*nx+ix)*3+1] = (float)(xoffset+ix+1);
            vdata[(iy*nx+ix)*3+2] = (float)(xoffset+ix);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, vdata);

    delete [] vdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the cell data.
    dims[1] = nx * ny;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "ELEMENTDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *cdata = new float[nx*ny];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            cdata[(iy*nx+ix)] = (float)(xoffset+ix);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, cdata);

    delete [] cdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Close the block.
    status = H5Gclose(block_id);
}

void
write_vtk101(const hid_t file_id, const int iblock)
{
    herr_t status;
    hsize_t dims[3];
    hid_t dataspace_id, dataset_id;

    char block_name[40];
    sprintf(block_name, "BLOCK%012d", iblock);

    hid_t block_id;
    block_id = H5Gcreate(file_id, block_name, 100);

    // Write the block information.
    dims[0] = 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "INFO", H5T_NATIVE_INT,
                           dataspace_id, H5P_DEFAULT);

    int info[3];
    info[0] = 2;   // Number of elements.
    info[1] = 6;   // Vertices per element.
    info[2] = 101; // Element type (quadratic triangle).

    status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, info);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the coordinate information.
    int nx = 2, ny = 1;
    dims[0] = nx * ny * 6 * 2;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "XYZ", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    int xoffset = 2, yoffset = 1;
    float *xyz = new float[nx*ny*6*2];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            xyz[(iy*nx+ix)*6*2+0] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*6*2+1] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*6*2+2] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*6*2+3] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*6*2+4] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*6*2+5] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*6*2+6+0] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*6*2+6+1] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*6*2+6+2] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*6*2+6+3] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*6*2+6+4] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*6*2+6+5] = (float)(yoffset+iy*2+1);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, xyz);

    delete [] xyz;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the point data.
    dims[1] = nx * ny * 6;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "VERTEXDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *vdata = new float[nx*ny*6];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            vdata[(iy*nx+ix)*6+0] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*6+1] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*6+2] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*6+3] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*6+4] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*6+5] = (float)(xoffset+ix*2);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, vdata);

    delete [] vdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the cell data.
    dims[1] = nx * ny;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "ELEMENTDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *cdata = new float[nx*ny];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            cdata[(iy*nx+ix)] = (float)(xoffset+ix);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, cdata);

    delete [] cdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Close the block.
    status = H5Gclose(block_id);
}

void
write_vtk111(const hid_t file_id, const int iblock)
{
    herr_t status;
    hsize_t dims[3];
    hid_t dataspace_id, dataset_id;

    char block_name[40];
    sprintf(block_name, "BLOCK%012d", iblock);

    hid_t block_id;
    block_id = H5Gcreate(file_id, block_name, 100);

    // Write the block information.
    dims[0] = 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "INFO", H5T_NATIVE_INT,
                           dataspace_id, H5P_DEFAULT);

    int info[3];
    info[0] = 2;   // Number of elements.
    info[1] = 6;   // Vertices per element.
    info[2] = 111; // Element type (quadratic triangle).

    status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, info);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the coordinate information.
    int nx = 2, ny = 1;
    dims[0] = nx * ny * 6 * 2;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "XYZ", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    int xoffset = 0, yoffset = 3;
    float *xyz = new float[nx*ny*6*2];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            xyz[(iy*nx+ix)*6*2+0] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*6*2+1] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*6*2+2] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*6*2+3] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*6*2+4] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*6*2+5] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*6*2+6+0] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*6*2+6+1] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*6*2+6+2] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*6*2+6+3] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*6*2+6+4] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*6*2+6+5] = (float)(yoffset+iy*2+2);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, xyz);

    delete [] xyz;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the point data.
    dims[1] = nx * ny * 6;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "VERTEXDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *vdata = new float[nx*ny*6];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            vdata[(iy*nx+ix)*6+0] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*6+1] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*6+2] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*6+3] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*6+4] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*6+5] = (float)(xoffset+ix*2);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, vdata);

    delete [] vdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the cell data.
    dims[1] = nx * ny;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "ELEMENTDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *cdata = new float[nx*ny];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            cdata[(iy*nx+ix)] = (float)(xoffset+ix);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, cdata);

    delete [] cdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Close the block.
    status = H5Gclose(block_id);
}

void
write_vtk112(const hid_t file_id, const int iblock)
{
    herr_t status;
    hsize_t dims[3];
    hid_t dataspace_id, dataset_id;

    char block_name[40];
    sprintf(block_name, "BLOCK%012d", iblock);

    hid_t block_id;
    block_id = H5Gcreate(file_id, block_name, 100);

    // Write the block information.
    dims[0] = 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "INFO", H5T_NATIVE_INT,
                           dataspace_id, H5P_DEFAULT);

    int info[3];
    info[0] = 2;   // Number of elements.
    info[1] = 10;  // Vertices per element.
    info[2] = 112; // Element type (cubic triangle).

    status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, info);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the coordinate information.
    int nx = 2, ny = 1;
    dims[0] = nx * ny * 10 * 2;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "XYZ", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    int xoffset = 4, yoffset = 3;
    float *xyz = new float[nx*ny*10*2];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            xyz[(iy*nx+ix)*10*2+0] = (float)(xoffset+ix*3);
            xyz[(iy*nx+ix)*10*2+1] = (float)(xoffset+ix*3+1);
            xyz[(iy*nx+ix)*10*2+2] = (float)(xoffset+ix*3+2);
            xyz[(iy*nx+ix)*10*2+3] = (float)(xoffset+ix*3+3);
            xyz[(iy*nx+ix)*10*2+4] = (float)(xoffset+ix*3);
            xyz[(iy*nx+ix)*10*2+5] = (float)(xoffset+ix*3+1);
            xyz[(iy*nx+ix)*10*2+6] = (float)(xoffset+ix*3+2);
            xyz[(iy*nx+ix)*10*2+7] = (float)(xoffset+ix*3);
            xyz[(iy*nx+ix)*10*2+8] = (float)(xoffset+ix*3+1);
            xyz[(iy*nx+ix)*10*2+9] = (float)(xoffset+ix*3);

            xyz[(iy*nx+ix)*10*2+10+0] = (float)(yoffset+iy*3);
            xyz[(iy*nx+ix)*10*2+10+1] = (float)(yoffset+iy*3);
            xyz[(iy*nx+ix)*10*2+10+2] = (float)(yoffset+iy*3);
            xyz[(iy*nx+ix)*10*2+10+3] = (float)(yoffset+iy*3);
            xyz[(iy*nx+ix)*10*2+10+4] = (float)(yoffset+iy*3+1);
            xyz[(iy*nx+ix)*10*2+10+5] = (float)(yoffset+iy*3+1);
            xyz[(iy*nx+ix)*10*2+10+6] = (float)(yoffset+iy*3+1);
            xyz[(iy*nx+ix)*10*2+10+7] = (float)(yoffset+iy*3+2);
            xyz[(iy*nx+ix)*10*2+10+8] = (float)(yoffset+iy*3+2);
            xyz[(iy*nx+ix)*10*2+10+9] = (float)(yoffset+iy*3+3);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, xyz);

    delete [] xyz;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the point data.
    dims[1] = nx * ny * 10;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "VERTEXDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *vdata = new float[nx*ny*10];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            vdata[(iy*nx+ix)*10+0] = (float)(xoffset+ix*3);
            vdata[(iy*nx+ix)*10+1] = (float)(xoffset+ix*3+1);
            vdata[(iy*nx+ix)*10+2] = (float)(xoffset+ix*3+2);
            vdata[(iy*nx+ix)*10+3] = (float)(xoffset+ix*3+3);
            vdata[(iy*nx+ix)*10+4] = (float)(xoffset+ix*3);
            vdata[(iy*nx+ix)*10+5] = (float)(xoffset+ix*3+1);
            vdata[(iy*nx+ix)*10+6] = (float)(xoffset+ix*3+2);
            vdata[(iy*nx+ix)*10+7] = (float)(xoffset+ix*3);
            vdata[(iy*nx+ix)*10+8] = (float)(xoffset+ix*3+1);
            vdata[(iy*nx+ix)*10+9] = (float)(xoffset+ix*3);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, vdata);

    delete [] vdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the cell data.
    dims[1] = nx * ny;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "ELEMENTDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *cdata = new float[nx*ny];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            cdata[(iy*nx+ix)] = (float)(xoffset+ix);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, cdata);

    delete [] cdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Close the block.
    status = H5Gclose(block_id);
}

void
write_vtk150(const hid_t file_id, const int iblock)
{
    herr_t status;
    hsize_t dims[3];
    hid_t dataspace_id, dataset_id;

    char block_name[40];
    sprintf(block_name, "BLOCK%012d", iblock);

    hid_t block_id;
    block_id = H5Gcreate(file_id, block_name, 100);

    // Write the block information.
    dims[0] = 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "INFO", H5T_NATIVE_INT,
                           dataspace_id, H5P_DEFAULT);

    int info[3];
    info[0] = 2;   // Number of elements.
    info[1] = 4;   // Vertices per element.
    info[2] = 150; // Element type (linear quad).

    status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, info);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the coordinate information.
    int nx = 2, ny = 1;
    dims[0] = nx * ny * 4 * 2;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "XYZ", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    int xoffset = 0, yoffset = 6;
    float *xyz = new float[nx*ny*4*2];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            xyz[(iy*nx+ix)*4*2+0] = (float)(xoffset+ix);
            xyz[(iy*nx+ix)*4*2+1] = (float)(xoffset+ix+1);
            xyz[(iy*nx+ix)*4*2+2] = (float)(xoffset+ix+1);
            xyz[(iy*nx+ix)*4*2+3] = (float)(xoffset+ix);
            xyz[(iy*nx+ix)*4*2+4+0] = (float)(yoffset+iy);
            xyz[(iy*nx+ix)*4*2+4+1] = (float)(yoffset+iy);
            xyz[(iy*nx+ix)*4*2+4+2] = (float)(yoffset+iy+1);
            xyz[(iy*nx+ix)*4*2+4+3] = (float)(yoffset+iy+1);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, xyz);

    delete [] xyz;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the point data.
    dims[1] = nx * ny * 4;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "VERTEXDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *vdata = new float[nx*ny*4];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            vdata[(iy*nx+ix)*4+0] = (float)(xoffset+ix);
            vdata[(iy*nx+ix)*4+1] = (float)(xoffset+ix+1);
            vdata[(iy*nx+ix)*4+2] = (float)(xoffset+ix+1);
            vdata[(iy*nx+ix)*4+3] = (float)(xoffset+ix);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, vdata);

    delete [] vdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the cell data.
    dims[1] = nx * ny;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "ELEMENTDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *cdata = new float[nx*ny];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            cdata[(iy*nx+ix)] = (float)(xoffset+ix);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, cdata);

    delete [] cdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Close the block.
    status = H5Gclose(block_id);
}

void
write_vtk151(const hid_t file_id, const int iblock)
{
    herr_t status;
    hsize_t dims[3];
    hid_t dataspace_id, dataset_id;

    char block_name[40];
    sprintf(block_name, "BLOCK%012d", iblock);

    hid_t block_id;
    block_id = H5Gcreate(file_id, block_name, 100);

    // Write the block information.
    dims[0] = 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "INFO", H5T_NATIVE_INT,
                           dataspace_id, H5P_DEFAULT);

    int info[3];
    info[0] = 2;   // Number of elements.
    info[1] = 8;   // Vertices per element.
    info[2] = 151; // Element type (quadratic quad).

    status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, info);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the coordinate information.
    int nx = 2, ny = 1;
    dims[0] = nx * ny * 8 * 2;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "XYZ", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    int xoffset = 2, yoffset = 6;
    float *xyz = new float[nx*ny*8*2];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            xyz[(iy*nx+ix)*8*2+0] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*8*2+1] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*8*2+2] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*8*2+3] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*8*2+4] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*8*2+5] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*8*2+6] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*8*2+7] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*8*2+8+0] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*8*2+8+1] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*8*2+8+2] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*8*2+8+3] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*8*2+8+4] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*8*2+8+5] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*8*2+8+6] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*8*2+8+7] = (float)(yoffset+iy*2+1);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, xyz);

    delete [] xyz;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the point data.
    dims[1] = nx * ny * 8;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "VERTEXDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *vdata = new float[nx*ny*8];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            vdata[(iy*nx+ix)*8+0] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*8+1] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*8+2] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*8+3] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*8+4] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*8+5] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*8+6] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*8+7] = (float)(xoffset+ix*2);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, vdata);

    delete [] vdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the cell data.
    dims[1] = nx * ny;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "ELEMENTDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *cdata = new float[nx*ny];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            cdata[(iy*nx+ix)] = (float)(xoffset+ix);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, cdata);

    delete [] cdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Close the block.
    status = H5Gclose(block_id);
}

void
write_vtk161(const hid_t file_id, const int iblock)
{
    herr_t status;
    hsize_t dims[3];
    hid_t dataspace_id, dataset_id;

    char block_name[40];
    sprintf(block_name, "BLOCK%012d", iblock);

    hid_t block_id;
    block_id = H5Gcreate(file_id, block_name, 100);

    // Write the block information.
    dims[0] = 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "INFO", H5T_NATIVE_INT,
                           dataspace_id, H5P_DEFAULT);

    int info[3];
    info[0] = 2;   // Number of elements.
    info[1] = 9;   // Vertices per element.
    info[2] = 161; // Element type (quadratic quad).

    status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, info);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the coordinate information.
    int nx = 2, ny = 1;
    dims[0] = nx * ny * 9 * 2;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "XYZ", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    int xoffset = 0, yoffset = 8;
    float *xyz = new float[nx*ny*9*2];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            xyz[(iy*nx+ix)*9*2+0] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*9*2+1] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*9*2+2] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*9*2+3] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*9*2+4] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*9*2+5] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*9*2+6] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*9*2+7] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*9*2+8] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*9*2+9+0] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*9*2+9+1] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*9*2+9+2] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*9*2+9+3] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*9*2+9+4] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*9*2+9+5] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*9*2+9+6] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*9*2+9+7] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*9*2+9+8] = (float)(yoffset+iy*2+2);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, xyz);

    delete [] xyz;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the point data.
    dims[1] = nx * ny * 9;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "VERTEXDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *vdata = new float[nx*ny*9];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            vdata[(iy*nx+ix)*9+0] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*9+1] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*9+2] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*9+3] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*9+4] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*9+5] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*9+6] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*9+7] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*9+8] = (float)(xoffset+ix*2+2);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, vdata);

    delete [] vdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the cell data.
    dims[1] = nx * ny;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "ELEMENTDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *cdata = new float[nx*ny];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            cdata[(iy*nx+ix)] = (float)(xoffset+ix);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, cdata);

    delete [] cdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Close the block.
    status = H5Gclose(block_id);
}

void
write_vtk162(const hid_t file_id, const int iblock)
{
    herr_t status;
    hsize_t dims[3];
    hid_t dataspace_id, dataset_id;

    char block_name[40];
    sprintf(block_name, "BLOCK%012d", iblock);

    hid_t block_id;
    block_id = H5Gcreate(file_id, block_name, 100);

    // Write the block information.
    dims[0] = 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "INFO", H5T_NATIVE_INT,
                           dataspace_id, H5P_DEFAULT);

    int info[3];
    info[0] = 2;   // Number of elements.
    info[1] = 16;  // Vertices per element.
    info[2] = 162; // Element type (cubic quad).

    status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, info);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the coordinate information.
    int nx = 2, ny = 1;
    dims[0] = nx * ny * 16 * 2;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "XYZ", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    int xoffset = 4, yoffset = 8;
    float *xyz = new float[nx*ny*16*2];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            xyz[(iy*nx+ix)*16*2+0] = (float)(xoffset+ix*3);
            xyz[(iy*nx+ix)*16*2+1] = (float)(xoffset+ix*3+1);
            xyz[(iy*nx+ix)*16*2+2] = (float)(xoffset+ix*3+2);
            xyz[(iy*nx+ix)*16*2+3] = (float)(xoffset+ix*3+3);
            xyz[(iy*nx+ix)*16*2+4] = (float)(xoffset+ix*3);
            xyz[(iy*nx+ix)*16*2+5] = (float)(xoffset+ix*3+1);
            xyz[(iy*nx+ix)*16*2+6] = (float)(xoffset+ix*3+2);
            xyz[(iy*nx+ix)*16*2+7] = (float)(xoffset+ix*3+3);
            xyz[(iy*nx+ix)*16*2+8] = (float)(xoffset+ix*3);
            xyz[(iy*nx+ix)*16*2+9] = (float)(xoffset+ix*3+1);
            xyz[(iy*nx+ix)*16*2+10] = (float)(xoffset+ix*3+2);
            xyz[(iy*nx+ix)*16*2+11] = (float)(xoffset+ix*3+3);
            xyz[(iy*nx+ix)*16*2+12] = (float)(xoffset+ix*3);
            xyz[(iy*nx+ix)*16*2+13] = (float)(xoffset+ix*3+1);
            xyz[(iy*nx+ix)*16*2+14] = (float)(xoffset+ix*3+2);
            xyz[(iy*nx+ix)*16*2+15] = (float)(xoffset+ix*3+3);
            xyz[(iy*nx+ix)*16*2+16+0] = (float)(yoffset+iy*3);
            xyz[(iy*nx+ix)*16*2+16+1] = (float)(yoffset+iy*3);
            xyz[(iy*nx+ix)*16*2+16+2] = (float)(yoffset+iy*3);
            xyz[(iy*nx+ix)*16*2+16+3] = (float)(yoffset+iy*3);
            xyz[(iy*nx+ix)*16*2+16+4] = (float)(yoffset+iy*3+1);
            xyz[(iy*nx+ix)*16*2+16+5] = (float)(yoffset+iy*3+1);
            xyz[(iy*nx+ix)*16*2+16+6] = (float)(yoffset+iy*3+1);
            xyz[(iy*nx+ix)*16*2+16+7] = (float)(yoffset+iy*3+1);
            xyz[(iy*nx+ix)*16*2+16+8] = (float)(yoffset+iy*3+2);
            xyz[(iy*nx+ix)*16*2+16+9] = (float)(yoffset+iy*3+2);
            xyz[(iy*nx+ix)*16*2+16+10] = (float)(yoffset+iy*3+2);
            xyz[(iy*nx+ix)*16*2+16+11] = (float)(yoffset+iy*3+2);
            xyz[(iy*nx+ix)*16*2+16+12] = (float)(yoffset+iy*3+3);
            xyz[(iy*nx+ix)*16*2+16+13] = (float)(yoffset+iy*3+3);
            xyz[(iy*nx+ix)*16*2+16+14] = (float)(yoffset+iy*3+3);
            xyz[(iy*nx+ix)*16*2+16+15] = (float)(yoffset+iy*3+3);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, xyz);

    delete [] xyz;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the point data.
    dims[1] = nx * ny * 16;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "VERTEXDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *vdata = new float[nx*ny*16];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            vdata[(iy*nx+ix)*16+0] = (float)(xoffset+ix*3);
            vdata[(iy*nx+ix)*16+1] = (float)(xoffset+ix*3+1);
            vdata[(iy*nx+ix)*16+2] = (float)(xoffset+ix*3+2);
            vdata[(iy*nx+ix)*16+3] = (float)(xoffset+ix*3+3);
            vdata[(iy*nx+ix)*16+4] = (float)(xoffset+ix*3);
            vdata[(iy*nx+ix)*16+5] = (float)(xoffset+ix*3+1);
            vdata[(iy*nx+ix)*16+6] = (float)(xoffset+ix*3+2);
            vdata[(iy*nx+ix)*16+7] = (float)(xoffset+ix*3+3);
            vdata[(iy*nx+ix)*16+8] = (float)(xoffset+ix*3);
            vdata[(iy*nx+ix)*16+9] = (float)(xoffset+ix*3+1);
            vdata[(iy*nx+ix)*16+10] = (float)(xoffset+ix*3+2);
            vdata[(iy*nx+ix)*16+11] = (float)(xoffset+ix*3+3);
            vdata[(iy*nx+ix)*16+12] = (float)(xoffset+ix*3);
            vdata[(iy*nx+ix)*16+13] = (float)(xoffset+ix*3+1);
            vdata[(iy*nx+ix)*16+14] = (float)(xoffset+ix*3+2);
            vdata[(iy*nx+ix)*16+15] = (float)(xoffset+ix*3+3);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, vdata);
   
    delete [] vdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the cell data.
    dims[1] = nx * ny;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "ELEMENTDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *cdata = new float[nx*ny];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            cdata[(iy*nx+ix)] = (float)(xoffset+ix);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, cdata);

    delete [] cdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Close the block.
    status = H5Gclose(block_id);
}

void
write_hexes(const hid_t file_id, const int iblock)
{
    herr_t status;
    hsize_t dims[3];
    hid_t dataspace_id, dataset_id;

    char block_name[40];
    sprintf(block_name, "BLOCK%012d", iblock);

    hid_t block_id;
    block_id = H5Gcreate(file_id, block_name, 100);

    // Write the block information.
    dims[0] = 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "INFO", H5T_NATIVE_INT,
                           dataspace_id, H5P_DEFAULT);

    int info[3];
    info[0] = 2;   // Number of elements.
    info[1] = 8;   // Vertices per element.
    info[2] = 12;  // Element type (VTK_HEXAHEDRON).

    status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, info);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the coordinate information.
    int nx = 2, ny = 1;
    dims[0] = nx * ny * 8 * 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "XYZ", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *xyz = new float[nx*ny*8*3];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            xyz[(iy*nx+ix)*8*3+0] = (float)ix;
            xyz[(iy*nx+ix)*8*3+1] = (float)(ix+1);
            xyz[(iy*nx+ix)*8*3+2] = (float)(ix+1);
            xyz[(iy*nx+ix)*8*3+3] = (float)ix;
            xyz[(iy*nx+ix)*8*3+4] = (float)ix;
            xyz[(iy*nx+ix)*8*3+5] = (float)(ix+1);
            xyz[(iy*nx+ix)*8*3+6] = (float)(ix+1);
            xyz[(iy*nx+ix)*8*3+7] = (float)ix;
            xyz[(iy*nx+ix)*8*3+8+0] = (float)iy;
            xyz[(iy*nx+ix)*8*3+8+1] = (float)iy;
            xyz[(iy*nx+ix)*8*3+8+2] = (float)(iy+1);
            xyz[(iy*nx+ix)*8*3+8+3] = (float)(iy+1);
            xyz[(iy*nx+ix)*8*3+8+4] = (float)iy;
            xyz[(iy*nx+ix)*8*3+8+5] = (float)iy;
            xyz[(iy*nx+ix)*8*3+8+6] = (float)(iy+1);
            xyz[(iy*nx+ix)*8*3+8+7] = (float)(iy+1);
            xyz[(iy*nx+ix)*8*3+16+0] = (float)0;
            xyz[(iy*nx+ix)*8*3+16+1] = (float)0;
            xyz[(iy*nx+ix)*8*3+16+2] = (float)0;
            xyz[(iy*nx+ix)*8*3+16+3] = (float)0;
            xyz[(iy*nx+ix)*8*3+16+4] = (float)1;
            xyz[(iy*nx+ix)*8*3+16+5] = (float)1;
            xyz[(iy*nx+ix)*8*3+16+6] = (float)1;
            xyz[(iy*nx+ix)*8*3+16+7] = (float)1;
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, xyz);

    delete [] xyz;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the point data.
    dims[1] = nx * ny * 8;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "VERTEXDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *vdata = new float[nx*ny*8];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            vdata[(iy*nx+ix)*8+0] = (float)ix;
            vdata[(iy*nx+ix)*8+1] = (float)(ix+1);
            vdata[(iy*nx+ix)*8+2] = (float)(ix+1);
            vdata[(iy*nx+ix)*8+3] = (float)ix;
            vdata[(iy*nx+ix)*8+4] = (float)ix;
            vdata[(iy*nx+ix)*8+5] = (float)(ix+1);
            vdata[(iy*nx+ix)*8+6] = (float)(ix+1);
            vdata[(iy*nx+ix)*8+7] = (float)ix;
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, vdata);

    delete [] vdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the cell data.
    dims[1] = nx * ny;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "ELEMENTDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *cdata = new float[nx*ny];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            cdata[(iy*nx+ix)] = (float)ix;
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, cdata);

    delete [] cdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Close the block.
    status = H5Gclose(block_id);
}

void
write_vtk200(const hid_t file_id, const int iblock)
{
    herr_t status;
    hsize_t dims[3];
    hid_t dataspace_id, dataset_id;

    char block_name[40];
    sprintf(block_name, "BLOCK%012d", iblock);

    hid_t block_id;
    block_id = H5Gcreate(file_id, block_name, 100);

    // Write the block information.
    dims[0] = 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "INFO", H5T_NATIVE_INT,
                           dataspace_id, H5P_DEFAULT);

    int info[3];
    info[0] = 2;   // Number of elements.
    info[1] = 4;   // Vertices per element.
    info[2] = 200; // Element type (linear tetrahedron).

    status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, info);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the coordinate information.
    int nx = 2, ny = 1;
    dims[0] = nx * ny * 4 * 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "XYZ", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    int xoffset = 0, yoffset = 1;
    float *xyz = new float[nx*ny*4*3];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            xyz[(iy*nx+ix)*4*3+0] = (float)(xoffset+ix);
            xyz[(iy*nx+ix)*4*3+1] = (float)(xoffset+ix+1);
            xyz[(iy*nx+ix)*4*3+2] = (float)(xoffset+ix);
            xyz[(iy*nx+ix)*4*3+3] = (float)(xoffset+ix);
            xyz[(iy*nx+ix)*4*3+4+0] = (float)(yoffset+iy);
            xyz[(iy*nx+ix)*4*3+4+1] = (float)(yoffset+iy);
            xyz[(iy*nx+ix)*4*3+4+2] = (float)(yoffset+iy+1);
            xyz[(iy*nx+ix)*4*3+4+3] = (float)(yoffset+iy);
            xyz[(iy*nx+ix)*4*3+8+0] = (float)0;
            xyz[(iy*nx+ix)*4*3+8+1] = (float)0;
            xyz[(iy*nx+ix)*4*3+8+2] = (float)0;
            xyz[(iy*nx+ix)*4*3+8+3] = (float)1;
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, xyz);

    delete [] xyz;
    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the point data.
    dims[1] = nx * ny * 4;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "VERTEXDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *vdata = new float[nx*ny*4];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            vdata[(iy*nx+ix)*4+0] = (float)(xoffset+ix);
            vdata[(iy*nx+ix)*4+1] = (float)(xoffset+ix+1);
            vdata[(iy*nx+ix)*4+2] = (float)(xoffset+ix);
            vdata[(iy*nx+ix)*4+3] = (float)(xoffset+ix);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, vdata);

    delete [] vdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the cell data.
    dims[1] = nx * ny;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "ELEMENTDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *cdata = new float[nx*ny];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            cdata[(iy*nx+ix)] = (float)(xoffset+ix);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, cdata);

    delete [] cdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Close the block.
    status = H5Gclose(block_id);
}

void
write_vtk201(const hid_t file_id, const int iblock)
{
    herr_t status;
    hsize_t dims[3];
    hid_t dataspace_id, dataset_id;

    char block_name[40];
    sprintf(block_name, "BLOCK%012d", iblock);

    hid_t block_id;
    block_id = H5Gcreate(file_id, block_name, 100);

    // Write the block information.
    dims[0] = 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "INFO", H5T_NATIVE_INT,
                           dataspace_id, H5P_DEFAULT);

    int info[3];
    info[0] = 2;   // Number of elements.
    info[1] = 10;  // Vertices per element.
    info[2] = 201; // Element type (quadratic tetrahedron).

    status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, info);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the coordinate information.
    int nx = 2, ny = 1;
    dims[0] = nx * ny * 10 * 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "XYZ", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    int xoffset = 2, yoffset = 1;
    float *xyz = new float[nx*ny*10*3];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            xyz[(iy*nx+ix)*10*3+0] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*10*3+1] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*10*3+2] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*10*3+3] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*10*3+4] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*10*3+5] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*10*3+6] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*10*3+7] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*10*3+8] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*10*3+9] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*10*3+10+0] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*10*3+10+1] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*10*3+10+2] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*10*3+10+3] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*10*3+10+4] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*10*3+10+5] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*10*3+10+6] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*10*3+10+7] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*10*3+10+8] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*10*3+10+9] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*10*3+20+0] = (float)0;
            xyz[(iy*nx+ix)*10*3+20+1] = (float)0;
            xyz[(iy*nx+ix)*10*3+20+2] = (float)0;
            xyz[(iy*nx+ix)*10*3+20+3] = (float)0;
            xyz[(iy*nx+ix)*10*3+20+4] = (float)0;
            xyz[(iy*nx+ix)*10*3+20+5] = (float)0;
            xyz[(iy*nx+ix)*10*3+20+6] = (float)1;
            xyz[(iy*nx+ix)*10*3+20+7] = (float)1;
            xyz[(iy*nx+ix)*10*3+20+8] = (float)1;
            xyz[(iy*nx+ix)*10*3+20+9] = (float)2;
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, xyz);

    delete [] xyz;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the point data.
    dims[1] = nx * ny * 10;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "VERTEXDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *vdata = new float[nx*ny*10];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            vdata[(iy*nx+ix)*10+0] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*10+1] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*10+2] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*10+3] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*10+4] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*10+5] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*10+6] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*10+7] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*10+8] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*10+9] = (float)(xoffset+ix*2);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, vdata);

    delete [] vdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the cell data.
    dims[1] = nx * ny;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "ELEMENTDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *cdata = new float[nx*ny];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            cdata[(iy*nx+ix)] = (float)(xoffset+ix);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, cdata);

    delete [] cdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Close the block.
    status = H5Gclose(block_id);
}

void
write_vtk211(const hid_t file_id, const int iblock)
{
    herr_t status;
    hsize_t dims[3];
    hid_t dataspace_id, dataset_id;

    char block_name[40];
    sprintf(block_name, "BLOCK%012d", iblock);

    hid_t block_id;
    block_id = H5Gcreate(file_id, block_name, 100);

    // Write the block information.
    dims[0] = 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "INFO", H5T_NATIVE_INT,
                           dataspace_id, H5P_DEFAULT);

    int info[3];
    info[0] = 2;   // Number of elements.
    info[1] = 10;  // Vertices per element.
    info[2] = 211; // Element type (quadratic tetrahedron).

    status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, info);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the coordinate information.
    int nx = 2, ny = 1;
    dims[0] = nx * ny * 10 * 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "XYZ", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    int xoffset = 0, yoffset = 3;
    float *xyz = new float[nx*ny*10*3];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            xyz[(iy*nx+ix)*10*3+0] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*10*3+1] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*10*3+2] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*10*3+3] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*10*3+4] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*10*3+5] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*10*3+6] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*10*3+7] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*10*3+8] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*10*3+9] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*10*3+10+0] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*10*3+10+1] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*10*3+10+2] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*10*3+10+3] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*10*3+10+4] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*10*3+10+5] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*10*3+10+6] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*10*3+10+7] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*10*3+10+8] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*10*3+10+9] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*10*3+20+0] = (float)0;
            xyz[(iy*nx+ix)*10*3+20+1] = (float)0;
            xyz[(iy*nx+ix)*10*3+20+2] = (float)0;
            xyz[(iy*nx+ix)*10*3+20+3] = (float)0;
            xyz[(iy*nx+ix)*10*3+20+4] = (float)0;
            xyz[(iy*nx+ix)*10*3+20+5] = (float)0;
            xyz[(iy*nx+ix)*10*3+20+6] = (float)1;
            xyz[(iy*nx+ix)*10*3+20+7] = (float)1;
            xyz[(iy*nx+ix)*10*3+20+8] = (float)1;
            xyz[(iy*nx+ix)*10*3+20+9] = (float)2;
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, xyz);

    delete [] xyz;
    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the point data.
    dims[1] = nx * ny * 10;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "VERTEXDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *vdata = new float[nx*ny*10];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            vdata[(iy*nx+ix)*10+0] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*10+1] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*10+2] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*10+3] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*10+4] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*10+5] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*10+6] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*10+7] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*10+8] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*10+9] = (float)(xoffset+ix*2);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, vdata);

    delete [] vdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the cell data.
    dims[1] = nx * ny;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "ELEMENTDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *cdata = new float[nx*ny];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            cdata[(iy*nx+ix)] = (float)(xoffset+ix);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, cdata);

    delete [] cdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Close the block.
    status = H5Gclose(block_id);
}

void
write_vtk212(const hid_t file_id, const int iblock)
{
    herr_t status;
    hsize_t dims[3];
    hid_t dataspace_id, dataset_id;

    char block_name[40];
    sprintf(block_name, "BLOCK%012d", iblock);

    hid_t block_id;
    block_id = H5Gcreate(file_id, block_name, 100);

    // Write the block information.
    dims[0] = 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "INFO", H5T_NATIVE_INT,
                           dataspace_id, H5P_DEFAULT);

    int info[3];
    info[0] = 2;   // Number of elements.
    info[1] = 20;  // Vertices per element.
    info[2] = 212; // Element type (quadratic tetrahedron).

    status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, info);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the coordinate information.
    int nx = 2, ny = 1;
    dims[0] = nx * ny * 20 * 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "XYZ", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    int xoffset = 4, yoffset = 3;
    float *xyz = new float[nx*ny*20*3];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            int ndx = 0;
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4 - i; j++)
                {
                    for (int k = 0; k < 4 - i - j; k++)
                    {
                        xyz[(iy*nx+ix)*20*3+ndx] = (float)(xoffset+ix*3+k);
                        xyz[(iy*nx+ix)*20*3+20+ndx] = (float)(yoffset+iy*3+j);
                        xyz[(iy*nx+ix)*20*3+40+ndx] = (float)(i);
                        ndx++;
                    }
                }
            }
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, xyz);

    delete [] xyz;
    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the point data.
    dims[1] = nx * ny * 20;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "VERTEXDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *vdata= new float[nx*ny*20];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            int ndx = 0;
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4 - i; j++)
                {
                    for (int k = 0; k < 4 - i - j; k++)
                    {
                        vdata[(iy*nx+ix)*20+ndx] = (float)(xoffset+ix*3+k);
                        ndx++;
                    }
                }
            }
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, vdata);

    delete [] vdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the cell data.
    dims[1] = nx * ny;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "ELEMENTDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *cdata = new float[nx*ny];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            cdata[(iy*nx+ix)] = (float)(xoffset+ix);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, cdata);

    delete [] cdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Close the block.
    status = H5Gclose(block_id);
}

void
write_vtk250(const hid_t file_id, const int iblock)
{
    herr_t status;
    hsize_t dims[3];
    hid_t dataspace_id, dataset_id;

    char block_name[40];
    sprintf(block_name, "BLOCK%012d", iblock);

    hid_t block_id;
    block_id = H5Gcreate(file_id, block_name, 100);

    // Write the block information.
    dims[0] = 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "INFO", H5T_NATIVE_INT,
                           dataspace_id, H5P_DEFAULT);

    int info[3];
    info[0] = 2;   // Number of elements.
    info[1] = 6;   // Vertices per element.
    info[2] = 250; // Element type (linear wedge).

    status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, info);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the coordinate information.
    int nx = 2, ny = 1;
    dims[0] = nx * ny * 6 * 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "XYZ", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    int xoffset = 0, yoffset = 6;
    float *xyz = new float[nx*ny*6*3];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            xyz[(iy*nx+ix)*6*3+0] = (float)(xoffset+ix);
            xyz[(iy*nx+ix)*6*3+1] = (float)(xoffset+ix+1);
            xyz[(iy*nx+ix)*6*3+2] = (float)(xoffset+ix);
            xyz[(iy*nx+ix)*6*3+3] = (float)(xoffset+ix);
            xyz[(iy*nx+ix)*6*3+4] = (float)(xoffset+ix+1);
            xyz[(iy*nx+ix)*6*3+5] = (float)(xoffset+ix);
            xyz[(iy*nx+ix)*6*3+6+0] = (float)(yoffset+iy);
            xyz[(iy*nx+ix)*6*3+6+1] = (float)(yoffset+iy);
            xyz[(iy*nx+ix)*6*3+6+2] = (float)(yoffset+iy+1);
            xyz[(iy*nx+ix)*6*3+6+3] = (float)(yoffset+iy);
            xyz[(iy*nx+ix)*6*3+6+4] = (float)(yoffset+iy);
            xyz[(iy*nx+ix)*6*3+6+5] = (float)(yoffset+iy+1);
            xyz[(iy*nx+ix)*6*3+12+0] = (float)0;
            xyz[(iy*nx+ix)*6*3+12+1] = (float)0;
            xyz[(iy*nx+ix)*6*3+12+2] = (float)0;
            xyz[(iy*nx+ix)*6*3+12+3] = (float)1;
            xyz[(iy*nx+ix)*6*3+12+4] = (float)1;
            xyz[(iy*nx+ix)*6*3+12+5] = (float)1;
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, xyz);

    delete [] xyz;
    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the point data.
    dims[1] = nx * ny * 6;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "VERTEXDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *vdata = new float[nx*ny*6];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            vdata[(iy*nx+ix)*6+0] = (float)(xoffset+ix);
            vdata[(iy*nx+ix)*6+1] = (float)(xoffset+ix+1);
            vdata[(iy*nx+ix)*6+2] = (float)(xoffset+ix);
            vdata[(iy*nx+ix)*6+3] = (float)(xoffset+ix);
            vdata[(iy*nx+ix)*6+4] = (float)(xoffset+ix+1);
            vdata[(iy*nx+ix)*6+5] = (float)(xoffset+ix);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, vdata);

    delete [] vdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the cell data.
    dims[1] = nx * ny;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "ELEMENTDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *cdata = new float[nx*ny];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            cdata[(iy*nx+ix)] = (float)(xoffset+ix);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, cdata);

    delete [] cdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Close the block.
    status = H5Gclose(block_id);
}

void
write_vtk251(const hid_t file_id, const int iblock)
{
    herr_t status;
    hsize_t dims[3];
    hid_t dataspace_id, dataset_id;

    char block_name[40];
    sprintf(block_name, "BLOCK%012d", iblock);

    hid_t block_id;
    block_id = H5Gcreate(file_id, block_name, 100);

    // Write the block information.
    dims[0] = 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "INFO", H5T_NATIVE_INT,
                           dataspace_id, H5P_DEFAULT);

    int info[3];
    info[0] = 2;   // Number of elements.
    info[1] = 15;  // Vertices per element.
    info[2] = 251; // Element type (quadratic wedge).

    status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, info);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the coordinate information.
    int nx = 2, ny = 1;
    dims[0] = nx * ny * 15 * 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "XYZ", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    int xoffset = 2, yoffset = 6;
    float *xyz = new float[nx*ny*15*3];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            xyz[(iy*nx+ix)*15*3+0] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*15*3+1] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*15*3+2] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*15*3+3] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*15*3+4] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*15*3+5] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*15*3+6] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*15*3+7] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*15*3+8] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*15*3+9] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*15*3+10] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*15*3+11] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*15*3+12] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*15*3+13] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*15*3+14] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*15*3+15+0] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*15*3+15+1] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*15*3+15+2] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*15*3+15+3] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*15*3+15+4] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*15*3+15+5] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*15*3+15+6] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*15*3+15+7] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*15*3+15+8] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*15*3+15+9] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*15*3+15+10] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*15*3+15+11] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*15*3+15+12] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*15*3+15+13] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*15*3+15+14] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*15*3+30+0] = (float)0;
            xyz[(iy*nx+ix)*15*3+30+1] = (float)0;
            xyz[(iy*nx+ix)*15*3+30+2] = (float)0;
            xyz[(iy*nx+ix)*15*3+30+3] = (float)0;
            xyz[(iy*nx+ix)*15*3+30+4] = (float)0;
            xyz[(iy*nx+ix)*15*3+30+5] = (float)0;
            xyz[(iy*nx+ix)*15*3+30+6] = (float)1;
            xyz[(iy*nx+ix)*15*3+30+7] = (float)1;
            xyz[(iy*nx+ix)*15*3+30+8] = (float)1;
            xyz[(iy*nx+ix)*15*3+30+9] = (float)2;
            xyz[(iy*nx+ix)*15*3+30+10] = (float)2;
            xyz[(iy*nx+ix)*15*3+30+11] = (float)2;
            xyz[(iy*nx+ix)*15*3+30+12] = (float)2;
            xyz[(iy*nx+ix)*15*3+30+13] = (float)2;
            xyz[(iy*nx+ix)*15*3+30+14] = (float)2;
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, xyz);

    delete [] xyz;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the point data.
    dims[1] = nx * ny * 15;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "VERTEXDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *vdata = new float[nx*ny*15];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            vdata[(iy*nx+ix)*15+0] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*15+1] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*15+2] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*15+3] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*15+4] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*15+5] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*15+6] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*15+7] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*15+8] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*15+9] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*15+10] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*15+11] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*15+12] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*15+13] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*15+14] = (float)(xoffset+ix*2);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, vdata);

    delete [] vdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the cell data.
    dims[1] = nx * ny;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "ELEMENTDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *cdata = new float[nx*ny];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            cdata[(iy*nx+ix)] = (float)(xoffset+ix);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, cdata);

    delete [] cdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Close the block.
    status = H5Gclose(block_id);
}

void
write_vtk261(const hid_t file_id, const int iblock)
{
    herr_t status;
    hsize_t dims[3];
    hid_t dataspace_id, dataset_id;

    char block_name[40];
    sprintf(block_name, "BLOCK%012d", iblock);

    hid_t block_id;
    block_id = H5Gcreate(file_id, block_name, 100);

    // Write the block information.
    dims[0] = 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "INFO", H5T_NATIVE_INT,
                           dataspace_id, H5P_DEFAULT);

    int info[3];
    info[0] = 2;   // Number of elements.
    info[1] = 18;  // Vertices per element.
    info[2] = 261; // Element type (quadratic wedge).

    status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, info);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the coordinate information.
    int nx = 2, ny = 1;
    dims[0] = nx * ny * 18 * 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "XYZ", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    int xoffset = 0, yoffset = 8;
    float *xyz = new float[nx*ny*18*3];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            xyz[(iy*nx+ix)*18*3+0] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*18*3+1] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*18*3+2] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*18*3+3] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*18*3+4] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*18*3+5] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*18*3+6] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*18*3+7] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*18*3+8] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*18*3+9] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*18*3+10] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*18*3+11] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*18*3+12] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*18*3+13] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*18*3+14] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*18*3+15] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*18*3+16] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*18*3+17] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*18*3+18+0] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*18*3+18+1] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*18*3+18+2] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*18*3+18+3] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*18*3+18+4] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*18*3+18+5] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*18*3+18+6] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*18*3+18+7] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*18*3+18+8] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*18*3+18+9] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*18*3+18+10] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*18*3+18+11] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*18*3+18+12] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*18*3+18+13] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*18*3+18+14] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*18*3+18+15] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*18*3+18+16] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*18*3+18+17] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*18*3+36+0] = (float)0;
            xyz[(iy*nx+ix)*18*3+36+1] = (float)0;
            xyz[(iy*nx+ix)*18*3+36+2] = (float)0;
            xyz[(iy*nx+ix)*18*3+36+3] = (float)0;
            xyz[(iy*nx+ix)*18*3+36+4] = (float)0;
            xyz[(iy*nx+ix)*18*3+36+5] = (float)0;
            xyz[(iy*nx+ix)*18*3+36+6] = (float)1;
            xyz[(iy*nx+ix)*18*3+36+7] = (float)1;
            xyz[(iy*nx+ix)*18*3+36+8] = (float)1;
            xyz[(iy*nx+ix)*18*3+36+9] = (float)1;
            xyz[(iy*nx+ix)*18*3+36+10] = (float)1;
            xyz[(iy*nx+ix)*18*3+36+11] = (float)1;
            xyz[(iy*nx+ix)*18*3+36+12] = (float)2;
            xyz[(iy*nx+ix)*18*3+36+13] = (float)2;
            xyz[(iy*nx+ix)*18*3+36+14] = (float)2;
            xyz[(iy*nx+ix)*18*3+36+15] = (float)2;
            xyz[(iy*nx+ix)*18*3+36+16] = (float)2;
            xyz[(iy*nx+ix)*18*3+36+17] = (float)2;
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, xyz);

    delete [] xyz;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the point data.
    dims[1] = nx * ny * 18;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "VERTEXDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *vdata = new float[nx*ny*18];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            vdata[(iy*nx+ix)*18+0] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*18+1] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*18+2] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*18+3] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*18+4] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*18+5] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*18+6] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*18+7] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*18+8] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*18+9] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*18+10] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*18+11] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*18+12] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*18+13] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*18+14] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*18+15] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*18+16] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*18+17] = (float)(xoffset+ix*2);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, vdata);

    delete [] vdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the cell data.
    dims[1] = nx * ny;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "ELEMENTDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *cdata = new float[nx*ny];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            cdata[(iy*nx+ix)] = (float)(xoffset+ix);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, cdata);

    delete [] cdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Close the block.
    status = H5Gclose(block_id);
}

void
write_vtk262(const hid_t file_id, const int iblock)
{
    herr_t status;
    hsize_t dims[3];
    hid_t dataspace_id, dataset_id;

    char block_name[40];
    sprintf(block_name, "BLOCK%012d", iblock);

    hid_t block_id;
    block_id = H5Gcreate(file_id, block_name, 100);

    // Write the block information.
    dims[0] = 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "INFO", H5T_NATIVE_INT,
                           dataspace_id, H5P_DEFAULT);

    int info[3];
    info[0] = 2;   // Number of elements.
    info[1] = 40;  // Vertices per element.
    info[2] = 262; // Element type (quadratic wedge).

    status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, info);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the coordinate information.
    int nx = 2, ny = 1;
    dims[0] = nx * ny * 40 * 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "XYZ", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    int xoffset = 4, yoffset = 8;
    float *xyz = new float[nx*ny*40*3];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            int ndx = 0;
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    for (int k = 0; k < 4 - j; k++)
                    {
                        xyz[(iy*nx+ix)*40*3+ndx] = (float)(xoffset+ix*3+k);
                        xyz[(iy*nx+ix)*40*3+40+ndx] = (float)(yoffset+iy*3+j);
                        xyz[(iy*nx+ix)*40*3+80+ndx] = (float)(i);
                        ndx++;
                    }
                }
            }
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, xyz);

    delete [] xyz;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the point data.
    dims[1] = nx * ny * 40;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "VERTEXDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *vdata = new float[nx*ny*40];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            int ndx = 0;
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    for (int k = 0; k < 4 - j; k++)
                    {
                        vdata[(iy*nx+ix)*40+ndx] = (float)(xoffset+ix*3+k);
                        ndx++;
                    }
                }
            }
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, vdata);

    delete [] vdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the cell data.
    dims[1] = nx * ny;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "ELEMENTDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *cdata = new float[nx*ny];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            cdata[(iy*nx+ix)] = (float)(xoffset+ix);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, cdata);

    delete [] cdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Close the block.
    status = H5Gclose(block_id);
}

void
write_vtk300(const hid_t file_id, const int iblock)
{
    herr_t status;
    hsize_t dims[3];
    hid_t dataspace_id, dataset_id;

    char block_name[40];
    sprintf(block_name, "BLOCK%012d", iblock);

    hid_t block_id;
    block_id = H5Gcreate(file_id, block_name, 100);

    // Write the block information.
    dims[0] = 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "INFO", H5T_NATIVE_INT,
                           dataspace_id, H5P_DEFAULT);

    int info[3];
    info[0] = 2;   // Number of elements.
    info[1] = 8;   // Vertices per element.
    info[2] = 300; // Element type (linear hexahedron).

    status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, info);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the coordinate information.
    int nx = 2, ny = 1;
    dims[0] = nx * ny * 8 * 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "XYZ", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    int xoffset = 0, yoffset = 11;
    float *xyz = new float[nx*ny*8*3];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            xyz[(iy*nx+ix)*8*3+0] = (float)(xoffset+ix);
            xyz[(iy*nx+ix)*8*3+1] = (float)(xoffset+ix+1);
            xyz[(iy*nx+ix)*8*3+2] = (float)(xoffset+ix+1);
            xyz[(iy*nx+ix)*8*3+3] = (float)(xoffset+ix);
            xyz[(iy*nx+ix)*8*3+4] = (float)(xoffset+ix);
            xyz[(iy*nx+ix)*8*3+5] = (float)(xoffset+ix+1);
            xyz[(iy*nx+ix)*8*3+6] = (float)(xoffset+ix+1);
            xyz[(iy*nx+ix)*8*3+7] = (float)(xoffset+ix);
            xyz[(iy*nx+ix)*8*3+8+0] = (float)(yoffset+iy);
            xyz[(iy*nx+ix)*8*3+8+1] = (float)(yoffset+iy);
            xyz[(iy*nx+ix)*8*3+8+2] = (float)(yoffset+iy+1);
            xyz[(iy*nx+ix)*8*3+8+3] = (float)(yoffset+iy+1);
            xyz[(iy*nx+ix)*8*3+8+4] = (float)(yoffset+iy);
            xyz[(iy*nx+ix)*8*3+8+5] = (float)(yoffset+iy);
            xyz[(iy*nx+ix)*8*3+8+6] = (float)(yoffset+iy+1);
            xyz[(iy*nx+ix)*8*3+8+7] = (float)(yoffset+iy+1);
            xyz[(iy*nx+ix)*8*3+16+0] = (float)0;
            xyz[(iy*nx+ix)*8*3+16+1] = (float)0;
            xyz[(iy*nx+ix)*8*3+16+2] = (float)0;
            xyz[(iy*nx+ix)*8*3+16+3] = (float)0;
            xyz[(iy*nx+ix)*8*3+16+4] = (float)1;
            xyz[(iy*nx+ix)*8*3+16+5] = (float)1;
            xyz[(iy*nx+ix)*8*3+16+6] = (float)1;
            xyz[(iy*nx+ix)*8*3+16+7] = (float)1;
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, xyz);

    delete [] xyz;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the point data.
    dims[1] = nx * ny * 8;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "VERTEXDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *vdata = new float[nx*ny*8];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            vdata[(iy*nx+ix)*8+0] = (float)(xoffset+ix);
            vdata[(iy*nx+ix)*8+1] = (float)(xoffset+ix+1);
            vdata[(iy*nx+ix)*8+2] = (float)(xoffset+ix+1);
            vdata[(iy*nx+ix)*8+3] = (float)(xoffset+ix);
            vdata[(iy*nx+ix)*8+4] = (float)(xoffset+ix);
            vdata[(iy*nx+ix)*8+5] = (float)(xoffset+ix+1);
            vdata[(iy*nx+ix)*8+6] = (float)(xoffset+ix+1);
            vdata[(iy*nx+ix)*8+7] = (float)(xoffset+ix);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, vdata);

    delete [] vdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the cell data.
    dims[1] = nx * ny;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "ELEMENTDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *cdata = new float[nx*ny];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            cdata[(iy*nx+ix)] = (float)(xoffset+ix);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, cdata);

    delete [] cdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Close the block.
    status = H5Gclose(block_id);
}

void
write_vtk301(const hid_t file_id, const int iblock)
{
    herr_t status;
    hsize_t dims[3];
    hid_t dataspace_id, dataset_id;

    char block_name[40];
    sprintf(block_name, "BLOCK%012d", iblock);

    hid_t block_id;
    block_id = H5Gcreate(file_id, block_name, 100);

    // Write the block information.
    dims[0] = 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "INFO", H5T_NATIVE_INT,
                           dataspace_id, H5P_DEFAULT);

    int info[3];
    info[0] = 2;   // Number of elements.
    info[1] = 20;  // Vertices per element.
    info[2] = 301; // Element type (quadratic hexahedron).

    status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, info);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the coordinate information.
    int nx = 2, ny = 1;
    dims[0] = nx * ny * 20 * 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "XYZ", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    int xoffset = 2, yoffset = 11;
    float *xyz = new float[nx*ny*20*3];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            xyz[(iy*nx+ix)*20*3+0] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*20*3+1] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*20*3+2] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*20*3+3] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*20*3+4] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*20*3+5] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*20*3+6] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*20*3+7] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*20*3+8] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*20*3+9] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*20*3+10] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*20*3+11] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*20*3+12] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*20*3+13] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*20*3+14] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*20*3+15] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*20*3+16] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*20*3+17] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*20*3+18] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*20*3+19] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*20*3+20+0] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*20*3+20+1] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*20*3+20+2] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*20*3+20+3] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*20*3+20+4] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*20*3+20+5] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*20*3+20+6] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*20*3+20+7] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*20*3+20+8] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*20*3+20+9] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*20*3+20+10] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*20*3+20+11] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*20*3+20+12] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*20*3+20+13] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*20*3+20+14] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*20*3+20+15] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*20*3+20+16] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*20*3+20+17] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*20*3+20+18] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*20*3+20+19] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*20*3+40+0] = (float)0;
            xyz[(iy*nx+ix)*20*3+40+1] = (float)0;
            xyz[(iy*nx+ix)*20*3+40+2] = (float)0;
            xyz[(iy*nx+ix)*20*3+40+3] = (float)0;
            xyz[(iy*nx+ix)*20*3+40+4] = (float)0;
            xyz[(iy*nx+ix)*20*3+40+5] = (float)0;
            xyz[(iy*nx+ix)*20*3+40+6] = (float)0;
            xyz[(iy*nx+ix)*20*3+40+7] = (float)0;
            xyz[(iy*nx+ix)*20*3+40+8] = (float)1;
            xyz[(iy*nx+ix)*20*3+40+9] = (float)1;
            xyz[(iy*nx+ix)*20*3+40+10] = (float)1;
            xyz[(iy*nx+ix)*20*3+40+11] = (float)1;
            xyz[(iy*nx+ix)*20*3+40+12] = (float)2;
            xyz[(iy*nx+ix)*20*3+40+13] = (float)2;
            xyz[(iy*nx+ix)*20*3+40+14] = (float)2;
            xyz[(iy*nx+ix)*20*3+40+15] = (float)2;
            xyz[(iy*nx+ix)*20*3+40+16] = (float)2;
            xyz[(iy*nx+ix)*20*3+40+17] = (float)2;
            xyz[(iy*nx+ix)*20*3+40+18] = (float)2;
            xyz[(iy*nx+ix)*20*3+40+19] = (float)2;
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, xyz);

    delete [] xyz;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the point data.
    dims[1] = nx * ny * 20;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "VERTEXDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *vdata = new float[nx*ny*20];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            vdata[(iy*nx+ix)*20+0] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*20+1] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*20+2] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*20+3] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*20+4] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*20+5] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*20+6] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*20+7] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*20+8] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*20+9] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*20+10] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*20+11] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*20+12] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*20+13] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*20+14] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*20+15] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*20+16] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*20+17] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*20+18] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*20+19] = (float)(xoffset+ix*2);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, vdata);

    delete [] vdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the cell data.
    dims[1] = nx * ny;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "ELEMENTDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *cdata = new float[nx*ny];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            cdata[(iy*nx+ix)] = (float)(xoffset+ix);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, cdata);

    delete [] cdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Close the block.
    status = H5Gclose(block_id);
}

void
write_vtk311(const hid_t file_id, const int iblock)
{
    herr_t status;
    hsize_t dims[3];
    hid_t dataspace_id, dataset_id;

    char block_name[40];
    sprintf(block_name, "BLOCK%012d", iblock);

    hid_t block_id;
    block_id = H5Gcreate(file_id, block_name, 100);

    // Write the block information.
    dims[0] = 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "INFO", H5T_NATIVE_INT,
                           dataspace_id, H5P_DEFAULT);

    int info[3];
    info[0] = 2;   // Number of elements.
    info[1] = 27;  // Vertices per element.
    info[2] = 311; // Element type (quadratic hexahedron).

    status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, info);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the coordinate information.
    int nx = 2, ny = 1;
    dims[0] = nx * ny * 27 * 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "XYZ", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    int xoffset = 6, yoffset = 11;
    float *xyz = new float[nx*ny*27*3];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            xyz[(iy*nx+ix)*27*3+0] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*27*3+1] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*27*3+2] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*27*3+3] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*27*3+4] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*27*3+5] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*27*3+6] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*27*3+7] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*27*3+8] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*27*3+9] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*27*3+10] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*27*3+11] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*27*3+12] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*27*3+13] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*27*3+14] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*27*3+15] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*27*3+16] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*27*3+17] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*27*3+18] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*27*3+19] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*27*3+20] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*27*3+21] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*27*3+22] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*27*3+23] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*27*3+24] = (float)(xoffset+ix*2);
            xyz[(iy*nx+ix)*27*3+25] = (float)(xoffset+ix*2+1);
            xyz[(iy*nx+ix)*27*3+26] = (float)(xoffset+ix*2+2);
            xyz[(iy*nx+ix)*27*3+27+0] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*27*3+27+1] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*27*3+27+2] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*27*3+27+3] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*27*3+27+4] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*27*3+27+5] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*27*3+27+6] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*27*3+27+7] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*27*3+27+8] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*27*3+27+9] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*27*3+27+10] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*27*3+27+11] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*27*3+27+12] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*27*3+27+13] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*27*3+27+14] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*27*3+27+15] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*27*3+27+16] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*27*3+27+17] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*27*3+27+18] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*27*3+27+19] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*27*3+27+20] = (float)(yoffset+iy*2);
            xyz[(iy*nx+ix)*27*3+27+21] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*27*3+27+22] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*27*3+27+23] = (float)(yoffset+iy*2+1);
            xyz[(iy*nx+ix)*27*3+27+24] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*27*3+27+25] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*27*3+27+26] = (float)(yoffset+iy*2+2);
            xyz[(iy*nx+ix)*27*3+54+0] = (float)0;
            xyz[(iy*nx+ix)*27*3+54+1] = (float)0;
            xyz[(iy*nx+ix)*27*3+54+2] = (float)0;
            xyz[(iy*nx+ix)*27*3+54+3] = (float)0;
            xyz[(iy*nx+ix)*27*3+54+4] = (float)0;
            xyz[(iy*nx+ix)*27*3+54+5] = (float)0;
            xyz[(iy*nx+ix)*27*3+54+6] = (float)0;
            xyz[(iy*nx+ix)*27*3+54+7] = (float)0;
            xyz[(iy*nx+ix)*27*3+54+8] = (float)0;
            xyz[(iy*nx+ix)*27*3+54+9] = (float)1;
            xyz[(iy*nx+ix)*27*3+54+10] = (float)1;
            xyz[(iy*nx+ix)*27*3+54+11] = (float)1;
            xyz[(iy*nx+ix)*27*3+54+12] = (float)1;
            xyz[(iy*nx+ix)*27*3+54+13] = (float)1;
            xyz[(iy*nx+ix)*27*3+54+14] = (float)1;
            xyz[(iy*nx+ix)*27*3+54+15] = (float)1;
            xyz[(iy*nx+ix)*27*3+54+16] = (float)1;
            xyz[(iy*nx+ix)*27*3+54+17] = (float)1;
            xyz[(iy*nx+ix)*27*3+54+18] = (float)2;
            xyz[(iy*nx+ix)*27*3+54+19] = (float)2;
            xyz[(iy*nx+ix)*27*3+54+20] = (float)2;
            xyz[(iy*nx+ix)*27*3+54+21] = (float)2;
            xyz[(iy*nx+ix)*27*3+54+22] = (float)2;
            xyz[(iy*nx+ix)*27*3+54+23] = (float)2;
            xyz[(iy*nx+ix)*27*3+54+24] = (float)2;
            xyz[(iy*nx+ix)*27*3+54+25] = (float)2;
            xyz[(iy*nx+ix)*27*3+54+26] = (float)2;
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, xyz);

    delete [] xyz;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the point data.
    dims[1] = nx * ny * 27;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "VERTEXDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *vdata = new float[nx*ny*27];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            vdata[(iy*nx+ix)*27+0] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*27+1] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*27+2] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*27+3] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*27+4] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*27+5] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*27+6] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*27+7] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*27+8] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*27+9] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*27+10] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*27+11] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*27+12] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*27+13] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*27+14] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*27+15] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*27+16] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*27+17] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*27+18] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*27+19] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*27+20] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*27+21] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*27+22] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*27+23] = (float)(xoffset+ix*2+2);
            vdata[(iy*nx+ix)*27+24] = (float)(xoffset+ix*2);
            vdata[(iy*nx+ix)*27+25] = (float)(xoffset+ix*2+1);
            vdata[(iy*nx+ix)*27+26] = (float)(xoffset+ix*2+2);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, vdata);

    delete [] vdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the cell data.
    dims[1] = nx * ny;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "ELEMENTDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *cdata = new float[nx*ny];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            cdata[(iy*nx+ix)] = (float)(xoffset+ix);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, cdata);

    delete [] cdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Close the block.
    status = H5Gclose(block_id);
}

void
write_vtk312(const hid_t file_id, const int iblock)
{
    herr_t status;
    hsize_t dims[3];
    hid_t dataspace_id, dataset_id;

    char block_name[40];
    sprintf(block_name, "BLOCK%012d", iblock);

    hid_t block_id;
    block_id = H5Gcreate(file_id, block_name, 100);

    // Write the block information.
    dims[0] = 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "INFO", H5T_NATIVE_INT,
                           dataspace_id, H5P_DEFAULT);

    int info[3];
    info[0] = 2;   // Number of elements.
    info[1] = 64;  // Vertices per element.
    info[2] = 312; // Element type (cubic hexahedron).

    status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, info);

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the coordinate information.
    int nx = 2, ny = 1;
    dims[0] = nx * ny * 64 * 3;
    dataspace_id = H5Screate_simple(1, dims, NULL);

    dataset_id = H5Dcreate(block_id, "XYZ", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    int xoffset = 10, yoffset = 11;
    float *xyz = new float[nx*ny*64*3];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    for (int k = 0; k < 4; k++)
                    {
                        xyz[(iy*nx+ix)*64*3+i*16+j*4+k] =
                            (float)(xoffset+ix*3+k);
                        xyz[(iy*nx+ix)*64*3+64+i*16+j*4+k] =
                            (float)(yoffset+iy*3+j);
                        xyz[(iy*nx+ix)*64*3+128+i*16+j*4+k] =
                            (float)(i);
                    }
                }
            }
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, xyz);

    delete [] xyz;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the point data.
    dims[1] = nx * ny * 64;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "VERTEXDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *vdata = new float[nx*ny*64];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    for (int k = 0; k < 4; k++)
                    {
                        vdata[(iy*nx+ix)*64+i*16+j*4+k] =
                            (float)(xoffset+ix*3+k);
                    }
                }
            }
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, vdata);

    delete [] vdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Write the cell data.
    dims[1] = nx * ny;
    dims[0] = 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    dataset_id = H5Dcreate(block_id, "ELEMENTDATA", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);

    float *cdata = new float[nx*ny];
    for (int iy = 0; iy < ny; iy++)
    {
        for (int ix = 0; ix < nx; ix++)
        {
            cdata[(iy*nx+ix)] = (float)(xoffset+ix);
        }
    }

    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, cdata);

    delete [] cdata;

    status = H5Dclose(dataset_id);

    status = H5Sclose(dataspace_id);

    // Close the block.
    status = H5Gclose(block_id);
}

void
write_2d_data()
{
    //
    // Create the file.
    //
    hid_t file_id;
    file_id = H5Fcreate("unic_data_2d.h5", H5F_ACC_TRUNC, H5P_DEFAULT,
                        H5P_DEFAULT);

    int nblocks = 9;
    int iblock = 1;
    //
    // Write out the header.
    //
    write_header(file_id, 2, nblocks);

    //
    // Write out a block of quads.
    //
    write_quads(file_id, iblock++);

    //
    // Write out blocks of cell type 100 and 101.
    //
    write_vtk100(file_id, iblock++);
    write_vtk101(file_id, iblock++);

    //
    // Write out blocks of cell type 111 and 112.
    //
    write_vtk111(file_id, iblock++);
    write_vtk112(file_id, iblock++);

    //
    // Write out blocks of cell type 150 and 151.
    //
    write_vtk150(file_id, iblock++);
    write_vtk151(file_id, iblock++);

    //
    // Write out blocks of cell type 161 and 162.
    //
    write_vtk161(file_id, iblock++);
    write_vtk162(file_id, iblock++);

    //
    // Close the file.
    //
    H5Fclose(file_id);

    if (iblock - 1 != nblocks)
        cerr << "ERROR: 2D file, the number of blocks is incorrect." << endl;
}

void
write_3d_data()
{
    //
    // Create the file.
    //
    hid_t file_id;
    file_id = H5Fcreate("unic_data_3d.h5", H5F_ACC_TRUNC, H5P_DEFAULT,
                        H5P_DEFAULT);

    int nblocks = 13;
    int iblock = 1;
    //
    // Write out the header.
    //
    write_header(file_id, 3, nblocks);

    //
    // Write out a block of quads.
    //
    write_hexes(file_id, iblock++);

    //
    // Write out blocks of cell type 200 and 201.
    //
    write_vtk200(file_id, iblock++);
    write_vtk201(file_id, iblock++);

    //
    // Write out blocks of cell type 211 and 212.
    //
    write_vtk211(file_id, iblock++);
    write_vtk212(file_id, iblock++);

    //
    // Write out blocks of cell type 250 and 251.
    //
    write_vtk250(file_id, iblock++);
    write_vtk251(file_id, iblock++);

    //
    // Write out blocks of cell type 261 and 262.
    //
    write_vtk261(file_id, iblock++);
    write_vtk262(file_id, iblock++);

    //
    // Write out blocks of cell type 300 and 301.
    //
    write_vtk300(file_id, iblock++);
    write_vtk301(file_id, iblock++);

    //
    // Write out blocks of cell type 311 and 312.
    //
    write_vtk311(file_id, iblock++);
    write_vtk312(file_id, iblock++);

    //
    // Close the file.
    //
    H5Fclose(file_id);

    if (iblock - 1 != nblocks)
        cerr << "ERROR: 3D file, the number of blocks is incorrect." << endl;
}

int
main()
{
    //
    // Write out a 2d sample file.
    //
    write_2d_data();

    //
    // Write out a 2d sample file.
    //
    write_3d_data();
}
