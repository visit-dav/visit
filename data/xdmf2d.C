#include <stdio.h>
#include <stdlib.h>
#include <math.h>
 
// Define this symbol BEFORE including hdf5.h to indicate the HDF5 code
// in this file uses version 1.6 of the HDF5 API. This is harmless for
// versions of HDF5 before 1.8 and ensures correct compilation with
// version 1.8 and thereafter. When, and if, the HDF5 code in this file
// is explicitly upgraded to the 1.8 API, this symbol should be removed.
#define H5_USE_16_API
#include <hdf5.h>
 
// The number of cells in the X, Y dimensions
#define NX 30
#define NY 20
 
void
write_hdf5_data()
{
    hid_t     file_id;
    file_id = H5Fcreate("xdmf2d.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
 
    // Create the coordinate data.
    float *x = (float *) malloc((NX+1)*(NY+1) * sizeof(float));
    float *y = (float *) malloc((NX+1)*(NY+1) * sizeof(float));
    int ndx = 0;
    for (int j = 0; j < NY+1; j++)
    {
        float yt = float(j) / float(NY);
        float angle = yt * M_PI;
        for (int i = 0; i < NX+1; i++)
        {
            float xt = float(i) / float(NX);
            float R = (1.-xt)*2. + xt*5.;
 
            x[ndx] = R * cos(angle);
            y[ndx] = R * sin(angle);
            ndx++;
        }
    }
 
    // Create the scalar data.
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
 
    // Write the data file.
    hid_t     dataset_id, dataspace_id;
    hsize_t   dims[3];
    herr_t    status;
    const char *coordNames[] = {"/X", "/Y"};
 
    /* Write separate coordinate arrays for the x and y coordinates. */
    for(int did = 0; did < 2; ++did)
    {
        dims[0] = (NY + 1);
        dims[1] = (NX + 1);
        dataspace_id = H5Screate_simple(2, dims, NULL);
 
        dataset_id = H5Dcreate(file_id, coordNames[did], H5T_NATIVE_FLOAT, dataspace_id,
                               H5P_DEFAULT);
 
        status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                          H5P_DEFAULT, did == 0 ? x : y);
 
        status = H5Dclose(dataset_id);
 
        status = H5Sclose(dataspace_id);
    }
 
    // Write the scalar data.
    dims[0] = NY;
    dims[1] = NX;
    dataspace_id = H5Screate_simple(2, dims, NULL);
 
    dataset_id = H5Dcreate(file_id, "/Pressure", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);
 
    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, pressure);
 
    status = H5Dclose(dataset_id);
 
    status = H5Sclose(dataspace_id);
 
    dims[0] = NY + 1;
    dims[1] = NX + 1;
    dataspace_id = H5Screate_simple(2, dims, NULL);
 
    dataset_id = H5Dcreate(file_id, "/VelocityX", H5T_NATIVE_FLOAT,
                           dataspace_id, H5P_DEFAULT);
 
    status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
                      H5P_DEFAULT, velocityx);
 
    status = H5Dclose(dataset_id);
 
    status = H5Sclose(dataspace_id);
 
    // Free the data.
    free(x);
    free(y);
    free(pressure);
    free(velocityx);
 
    status = H5Fclose(file_id);
}
 
void
write_xdmf_xml()
{
    FILE *xmf = 0;
 
    /*
     * Open the file and write the XML description of the mesh..
     */
    xmf = fopen("xdmf2d.xmf", "w");
    fprintf(xmf, "<?xml version=\"1.0\" ?>\n");
    fprintf(xmf, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
    fprintf(xmf, "<Xdmf Version=\"2.0\">\n");
    fprintf(xmf, " <Domain>\n");
    fprintf(xmf, "   <Grid Name=\"mesh1\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"2DSMesh\" NumberOfElements=\"%d %d\"/>\n", NY+1, NX+1);
    fprintf(xmf, "     <Geometry GeometryType=\"X_Y\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NY+1), (NX+1));
    fprintf(xmf, "        xdmf2d.h5:/X\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", (NY+1), (NX+1));
    fprintf(xmf, "        xdmf2d.h5:/Y\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NY, NX);
    fprintf(xmf, "        xdmf2d.h5:/Pressure\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "     <Attribute Name=\"VelocityX\" AttributeType=\"Scalar\" Center=\"Node\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n", NY+1, NX+1);
    fprintf(xmf, "        xdmf2d.h5:/VelocityX\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");
    fprintf(xmf, " </Domain>\n");
    fprintf(xmf, "</Xdmf>\n");
    fclose(xmf);
}
 
int
main(int argc, char *argv[])
{
    write_hdf5_data();
    write_xdmf_xml();
 
    return 0;
}
