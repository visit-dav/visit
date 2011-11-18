#include <stdio.h>
#include <stdlib.h>

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
write_2d_curv_data()
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
    unsigned char *pressure =
        (unsigned char *) malloc(NX*NY * sizeof(unsigned char));

    for (int j = 0; j < NY; j++)
    {
        for (int i = 0; i < NX; i++)
        {
            int ndx = j * NX + i;
            pressure[ndx] = (unsigned char) j;
        }
    }

    char *velocityx = (char *) malloc((NX+1)*(NY+1) * sizeof(char));

    for (int j = 0; j < NY+1; j++)
    {
        for (int i = 0; i < NX+1; i++)
        {
            int ndx = j * (NX+1) + i;
            velocityx[ndx] = (char) i;
        }
    }

    /*
     * Write the binary data files.
     */
    FILE *pFile = fopen("XY.bin", "wb");
    fwrite(xy, sizeof(float), (NX+1)*(NY+1), pFile);
    fclose(pFile);

    pFile = fopen("Pressure_2D.bin", "wb");
    fwrite(pressure, sizeof(unsigned char), NX*NY, pFile);
    fclose(pFile);

    pFile = fopen("VelocityX_2D.bin", "wb");
    fwrite(velocityx, sizeof(char), (NX+1)*(NY+1), pFile);
    fclose(pFile);

    /*
     * Free the data.
     */
    free(xy);
    free(pressure);
    free(velocityx);
}

void
write_3d_rect_data()
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
     * Write the binary data files.
     */
    FILE *pFile = fopen("X_1D.bin", "wb");
    fwrite(x, sizeof(float), (NX+1), pFile);
    fclose(pFile);

    pFile = fopen("Y_1D.bin", "wb");
    fwrite(y, sizeof(float), (NY+1), pFile);
    fclose(pFile);

    pFile = fopen("Z_1D.bin", "wb");
    fwrite(z, sizeof(float), (NZ+1), pFile);
    fclose(pFile);

    /*
     * Free the data.
     */
    free(x);
    free(y);
    free(z);
}

void
write_3d_point_data()
{
    /*
     * Create an index list for the point array.
     */
    int *indexes = (int *) malloc((NX+1)*(NY+1)*(NZ+1) * sizeof(int));

    for (int i = 0; i < (NX+1)*(NY+1)*(NZ+1); i++)
        indexes[i] = i;
    
    /*
     * Write the binary data files.
     */
    FILE *pFile = fopen("Indexes.bin", "wb");
    fwrite(indexes, sizeof(int), (NX+1)*(NY+1)*(NZ+1), pFile);
    fclose(pFile);

    pFile = fopen("Indexes2.bin", "wb");
    fwrite(indexes, sizeof(int), (NX2+1)*(NY2+1)*(NZ2+1), pFile);
    fclose(pFile);

    /*
     * Free the data.
     */
    free(indexes);
}

void
write_3d_curv_data()
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
    unsigned int *pressure = (unsigned int *)
        malloc(NX*NY*NZ * sizeof(unsigned int));

    for (int k = 0; k < NZ; k++)
    {
        for (int j = 0; j < NY; j++)
        {
            for (int i = 0; i < NX; i++)
            {
                int ndx = k * NX * NY + j * NX + i;
                pressure[ndx] = (unsigned int) k;
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

    int *velocityz = (int *) malloc((NX+1)*(NY+1)*(NZ+1) * sizeof(int));

    for (int k = 0; k < NZ+1; k++)
    {
        for (int j = 0; j < NY+1; j++)
        {
            for (int i = 0; i < NX+1; i++)
            {
                int ndx = k * (NX+1) * (NY+1) + j * (NX+1) + i;
                velocityz[ndx] = (int) k;
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
     * Write the binary data files.
     */
    FILE *pFile = fopen("Origin.bin", "wb");
    fwrite(origin, sizeof(float), 3, pFile);
    fclose(pFile);

    pFile = fopen("DxDyDz.bin", "wb");
    fwrite(dxdydz, sizeof(float), 3, pFile);
    fclose(pFile);

    pFile = fopen("Origin2.bin", "wb");
    fwrite(origin, sizeof(float), 2, pFile);
    fclose(pFile);

    pFile = fopen("DxDy.bin", "wb");
    fwrite(dxdydz, sizeof(float), 2, pFile);
    fclose(pFile);

    pFile = fopen("X.bin", "wb");
    fwrite(x, sizeof(float), (NX+1)*(NY+1)*(NZ+1), pFile);
    fclose(pFile);

    pFile = fopen("Y.bin", "wb");
    fwrite(y, sizeof(float), (NX+1)*(NY+1)*(NZ+1), pFile);
    fclose(pFile);

    pFile = fopen("Z.bin", "wb");
    fwrite(z, sizeof(float), (NX+1)*(NY+1)*(NZ+1), pFile);
    fclose(pFile);

    pFile = fopen("XYZ.bin", "wb");
    fwrite(xyz, sizeof(float), (NX+1)*(NY+1)*(NZ+1)*3, pFile);
    fclose(pFile);

    pFile = fopen("XYZ2.bin", "wb");
    fwrite(xyz2, sizeof(float), (NX2+1)*(NY2+1)*(NZ2+1)*3, pFile);
    fclose(pFile);

    pFile = fopen("XYZ3.bin", "wb");
    fwrite(xyz3, sizeof(float), (NX3+1)*(NY3+1)*(NZ3+1)*3, pFile);
    fclose(pFile);

    pFile = fopen("Pressure.bin", "wb");
    fwrite(pressure, sizeof(float), NX*NY*NZ, pFile);
    fclose(pFile);

    pFile = fopen("Pressure2.bin", "wb");
    fwrite(pressure2, sizeof(float), NX2*NY2*NZ2, pFile);
    fclose(pFile);

    pFile = fopen("Pressure3.bin", "wb");
    fwrite(pressure3, sizeof(float), NX3*NY3*NZ3, pFile);
    fclose(pFile);

    pFile = fopen("VelocityZ.bin", "wb");
    fwrite(velocityz, sizeof(int), (NX+1)*(NY+1)*(NZ+1), pFile);
    fclose(pFile);

    pFile = fopen("VelocityZ2.bin", "wb");
    fwrite(velocityz2, sizeof(float), (NX2+1)*(NY2+1)*(NZ2+1), pFile);
    fclose(pFile);

    pFile = fopen("VelocityZ3.bin", "wb");
    fwrite(velocityz3, sizeof(float), (NX3+1)*(NY3+1)*(NZ3+1), pFile);
    fclose(pFile);

    pFile = fopen("Velocity.bin", "wb");
    fwrite(velocity, sizeof(float), (NX+1)*(NY+1)*(NZ+1)*3, pFile);
    fclose(pFile);

    pFile = fopen("Velocity2.bin", "wb");
    fwrite(velocity2, sizeof(float), (NX2+1)*(NY2+1)*(NZ2+1)*3, pFile);
    fclose(pFile);

    pFile = fopen("Velocity3.bin", "wb");
    fwrite(velocity3, sizeof(float), (NX3+1)*(NY3+1)*(NZ3+1)*3, pFile);
    fclose(pFile);

    pFile = fopen("Stress.bin", "wb");
    fwrite(stress, sizeof(float), (NX+1)*(NY+1)*(NZ+1)*6, pFile);
    fclose(pFile);

    pFile = fopen("Stress2.bin", "wb");
    fwrite(stress2, sizeof(float), (NX2+1)*(NY2+1)*(NZ2+1)*6, pFile);
    fclose(pFile);

    pFile = fopen("Stress3.bin", "wb");
    fwrite(stress3, sizeof(float), (NX3+1)*(NY3+1)*(NZ3+1)*6, pFile);
    fclose(pFile);

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
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", (NZ+1)*(NY+1)*(NX+1), 3);
    fprintf(xmf, "        XYZ.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"UInt\" Precision=\"4\" Format=\"Binary\">\n", NZ, NY, NX);
    fprintf(xmf, "        Pressure.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");


    fprintf(xmf, "   <Grid Name=\"mesh2\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ2+1, NY2+1, NX2+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", (NZ2+1)*(NY2+1)*(NX2+1), 3);
    fprintf(xmf, "        XYZ2.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", NZ2, NY2, NX2);
    fprintf(xmf, "        Pressure2.bin\n");
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
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", (NZ+1)*(NY+1)*(NX+1), 3);
    fprintf(xmf, "        XYZ.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "   </Grid>\n");


    fprintf(xmf, "   <Grid Name=\"mesh2\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ2+1, NY2+1, NX2+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", (NZ2+1)*(NY2+1)*(NX2+1), 3);
    fprintf(xmf, "        XYZ2.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", NZ2, NY2, NX2);
    fprintf(xmf, "        Pressure2.bin\n");
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
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", (NZ+1)*(NY+1)*(NX+1), 3);
    fprintf(xmf, "        XYZ.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"UInt\" Precision=\"4\" Format=\"Binary\">\n", NZ, NY, NX);
    fprintf(xmf, "        Pressure.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");


    fprintf(xmf, "   <Grid Name=\"mesh2\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ2+1, NY2+1, NX2+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", (NZ2+1)*(NY2+1)*(NX2+1), 3);
    fprintf(xmf, "        XYZ2.bin\n");
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
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", (NZ+1)*(NY+1)*(NX+1), 3);
    fprintf(xmf, "        XYZ.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", NZ, NY, NX);
    fprintf(xmf, "        Pressure.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");


    fprintf(xmf, "   <Grid Name=\"mesh2\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ2+1, NY2+1, NX2+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", (NZ2+1)*(NY2+1)*(NX2+1), 3);
    fprintf(xmf, "        XYZ2.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", NZ2, NY2, NX2);
    fprintf(xmf, "        Pressure2.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, "   <Grid Name=\"mesh3\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ3+1, NY3+1, NX3+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", (NZ3+1)*(NY3+1)*(NX3+1), 3);
    fprintf(xmf, "        XYZ3.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", NZ3, NY3, NX3);
    fprintf(xmf, "        Pressure3.bin\n");
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
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", (NZ+1)*(NY+1)*(NX+1), 3);
    fprintf(xmf, "        XYZ.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"UInt\" Precision=\"4\" Format=\"Binary\">\n", NZ, NY, NX);
    fprintf(xmf, "        Pressure.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, " </Domain>\n");

    fprintf(xmf, " <Domain>\n");

    fprintf(xmf, "   <Grid Name=\"mesh1\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ2+1, NY2+1, NX2+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", (NZ2+1)*(NY2+1)*(NX2+1), 3);
    fprintf(xmf, "        XYZ2.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", NZ2, NY2, NX2);
    fprintf(xmf, "        Pressure2.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, "   <Grid Name=\"mesh2\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ3+1, NY3+1, NX3+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", (NZ3+1)*(NY3+1)*(NX3+1), 3);
    fprintf(xmf, "        XYZ3.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", NZ3, NY3, NX3);
    fprintf(xmf, "        Pressure3.bin\n");
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
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", (NZ+1)*(NY+1)*(NX+1), 3);
    fprintf(xmf, "        XYZ.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"UInt\" Precision=\"4\" Format=\"Binary\">\n", NZ, NY, NX);
    fprintf(xmf, "        Pressure.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");

    fprintf(xmf, "   <Grid Name=\"mesh2\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"3DSMesh\" NumberOfElements=\"%d %d %d\"/>\n", NZ2+1, NY2+1, NX2+1);
    fprintf(xmf, "     <Geometry GeometryType=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", (NZ2+1)*(NY2+1)*(NX2+1), 3);
    fprintf(xmf, "        XYZ2.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", NZ2, NY2, NX2);
    fprintf(xmf, "        Pressure2.bin\n");
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
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", (NZ+1)*(NY+1)*(NX+1), 3);
    fprintf(xmf, "        XYZ.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"UInt\" Precision=\"4\" Format=\"Binary\">\n", NZ, NY, NX);
    fprintf(xmf, "        Pressure.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "     <Attribute Name=\"VelocityZ\" AttributeType=\"Scalar\" Center=\"Node\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Int\" Precision=\"4\" Format=\"Binary\">\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "        VelocityZ.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "     <Attribute Name=\"Velocity\" AttributeType=\"Vector\" Center=\"Node\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d 3\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "        Velocity.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "     <Attribute Name=\"Stress\" AttributeType=\"Tensor6\" Center=\"Node\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d 6\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "        Stress.bin\n");
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
    fprintf(xmf, "     <Geometry GeometryType=\"Origin_DxDy\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", 2);
    fprintf(xmf, "        Origin2.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", 2);
    fprintf(xmf, "        DxDy.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"UChar\" Precision=\"1\" Format=\"Binary\">\n", NY, NX);
    fprintf(xmf, "        Pressure_2D.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "     <Attribute Name=\"VelocityX\" AttributeType=\"Scalar\" Center=\"Node\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Char\" Precision=\"4\" Format=\"Binary\">\n", NY+1, NX+1);
    fprintf(xmf, "        VelocityX_2D.bin\n");
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
    fprintf(xmf, "       <DataItem Dimensions=\"%d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", 3);
    fprintf(xmf, "        Origin.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", 3);
    fprintf(xmf, "        DxDyDz.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"UInt\" Precision=\"4\" Format=\"Binary\">\n", NZ, NY, NX);
    fprintf(xmf, "        Pressure.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "     <Attribute Name=\"VelocityZ\" AttributeType=\"Scalar\" Center=\"Node\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Int\" Precision=\"4\" Format=\"Binary\">\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "        VelocityZ.bin\n");
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
    fprintf(xmf, "     <Geometry GeometryType=\"VXVY\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", NX+1);
    fprintf(xmf, "        X_1D.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", NY+1);
    fprintf(xmf, "        Y_1D.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"UChar\" Precision=\"4\" Format=\"Binary\">\n", NY, NX);
    fprintf(xmf, "        Pressure_2D.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "     <Attribute Name=\"VelocityX\" AttributeType=\"Scalar\" Center=\"Node\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d\" NumberType=\"Char\" Precision=\"4\" Format=\"Binary\">\n", NY+1, NX+1);
    fprintf(xmf, "        VelocityX_2D.bin\n");
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
    fprintf(xmf, "     <Geometry GeometryType=\"VXVYVZ\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", NX+1);
    fprintf(xmf, "        X_1D.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", NY+1);
    fprintf(xmf, "        Y_1D.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d\" NumberType=\"Float\" Precision=\"4\" Format=\"Binary\">\n", NZ+1);
    fprintf(xmf, "        Z_1D.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"Pressure\" AttributeType=\"Scalar\" Center=\"Cell\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"UInt\" Precision=\"4\" Format=\"Binary\">\n", NZ, NY, NX);
    fprintf(xmf, "        Pressure.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "     <Attribute Name=\"VelocityZ\" AttributeType=\"Scalar\" Center=\"Node\">\n");
    fprintf(xmf, "       <DataItem Dimensions=\"%d %d %d\" NumberType=\"Int\" Precision=\"4\" Format=\"Binary\">\n", NZ+1, NY+1, NX+1);
    fprintf(xmf, "        VelocityZ.bin\n");
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
create_point3d()
{
    FILE *xmf = 0;

    /*
     * Open the file and write the header.
     */
    xmf = fopen("point3d.xmf", "w");
    fprintf(xmf, "<?xml version=\"1.0\" ?>\n");
    fprintf(xmf, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
    fprintf(xmf, "<Xdmf Version=\"2.0\">\n");

    /*
     * Write the mesh description and the variables defined on the mesh.
     */
    fprintf(xmf, " <Domain>\n");

    fprintf(xmf, "   <Grid Name=\"points\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"Polyvertex\" Dimensions=\"%d\" NodesPerElement=\"1\">\n", (NX+1)*(NY+1)*(NZ+1));
    fprintf(xmf, "       <DataItem Format=\"Binary\" Dimensions=\"%d\" NumberType=\"Int\">\n", (NX+1)*(NY+1)*(NZ+1));
    fprintf(xmf, "        Indexes.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Topology>\n");
    fprintf(xmf, "     <Geometry Type=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Format=\"Binary\" Dimensions=\"%d 3\" NumberType=\"Float\">\n", (NX+1)*(NY+1)*(NZ+1));
    fprintf(xmf, "        XYZ.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"VelocityZ\" AttributeType=\"Scalar\" Center=\"Node\">\n");
    fprintf(xmf, "       <DataItem Format=\"Binary\" Dimensions=\"%d\" NumberType=\"Int\">\n", (NX+1)*(NY+1)*(NZ+1));
    fprintf(xmf, "        VelocityZ.bin\n");
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
create_multi_point3d()
{
    FILE *xmf = 0;

    /*
     * Open the file and write the header.
     */
    xmf = fopen("multi_point3d.xmf", "w");
    fprintf(xmf, "<?xml version=\"1.0\" ?>\n");
    fprintf(xmf, "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n");
    fprintf(xmf, "<Xdmf Version=\"2.0\">\n");

    /*
     * Write the mesh description and the variables defined on the mesh.
     */
    fprintf(xmf, " <Domain>\n");

    fprintf(xmf, "   <Grid Name=\"points\" GridType=\"Collection\">\n");
    fprintf(xmf, "   <Grid Name=\"points1\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"Polyvertex\" Dimensions=\"%d\" NodesPerElement=\"1\">\n", (NX+1)*(NY+1)*(NZ+1));
    fprintf(xmf, "       <DataItem Format=\"Binary\" Dimensions=\"%d\" NumberType=\"Int\">\n", (NX+1)*(NY+1)*(NZ+1));
    fprintf(xmf, "        Indexes.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Topology>\n");
    fprintf(xmf, "     <Geometry Type=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Format=\"Binary\" Dimensions=\"%d 3\" NumberType=\"Float\">\n", (NX+1)*(NY+1)*(NZ+1));
    fprintf(xmf, "        XYZ.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"VelocityZ\" AttributeType=\"Scalar\" Center=\"Node\">\n");
    fprintf(xmf, "       <DataItem Format=\"Binary\" Dimensions=\"%d\" NumberType=\"Int\">\n", (NX+1)*(NY+1)*(NZ+1));
    fprintf(xmf, "        VelocityZ.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Attribute>\n");
    fprintf(xmf, "   </Grid>\n");
    fprintf(xmf, "   <Grid Name=\"points2\" GridType=\"Uniform\">\n");
    fprintf(xmf, "     <Topology TopologyType=\"Polyvertex\" Dimensions=\"%d\" NodesPerElement=\"1\">\n", (NX2+1)*(NY2+1)*(NZ2+1));
    fprintf(xmf, "       <DataItem Format=\"Binary\" Dimensions=\"%d\" NumberType=\"Int\">\n", (NX2+1)*(NY2+1)*(NZ2+1));
    fprintf(xmf, "        Indexes2.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Topology>\n");
    fprintf(xmf, "     <Geometry Type=\"XYZ\">\n");
    fprintf(xmf, "       <DataItem Format=\"Binary\" Dimensions=\"%d 3\" NumberType=\"Float\">\n", (NX2+1)*(NY2+1)*(NZ2+1));
    fprintf(xmf, "        XYZ2.bin\n");
    fprintf(xmf, "       </DataItem>\n");
    fprintf(xmf, "     </Geometry>\n");
    fprintf(xmf, "     <Attribute Name=\"VelocityZ\" AttributeType=\"Scalar\" Center=\"Node\">\n");
    fprintf(xmf, "       <DataItem Format=\"Binary\" Dimensions=\"%d\" NumberType=\"Float\">\n", (NX2+1)*(NY2+1)*(NZ2+1));
    fprintf(xmf, "        VelocityZ2.bin\n");
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
    write_2d_curv_data();
    write_3d_rect_data();
    write_3d_curv_data();
    write_3d_point_data();

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
    create_point3d();
    create_multi_point3d();
}

