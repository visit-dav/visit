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

#include <math.h>
#include <stdio.h>
#include <visit-config.h>
#include <string.h>

static const float minvals[] = {10.,10.,10.};
static const float maxvals[] = {20.,20.,20.};
static const int bx = 17;
static const int by = 23;
static const int bz = 29;
static const int nx = bx * 4;
static const int ny = by * 5;
static const int nz = bz * 3;
static const int full_size[] = {nx, ny, nz};
static const int bricklet_size[] = {bx, by, bz};
static int nels = nx * ny * nz;
static const int offset = 16;


typedef float BOV_t;

template <class T>
void
write_BOV_data(const char *filename, const T *data, int nels)
{
    FILE *fp = fopen(filename, "wb");
    fwrite((void *)data, sizeof(T), nels, fp);
    fclose(fp);
}

void
write_BOV_header(const char *tname, const char *sname, int nc,
    const char *header, const char *bovfile, int offset_bytes,
    bool subdiv, bool bt)
{
    // Write the BOV header.
    FILE *fp = fopen(header, "wt");
    fprintf(fp, "TIME: 0\n");
    fprintf(fp, "DATA_FILE: %s\n", bovfile);
    fprintf(fp, "DATA_SIZE: %d %d %d\n", 
        full_size[0], full_size[1], full_size[2]);

    fprintf(fp, "DATA_FORMAT: %s\n", sname);
    if(strcmp(tname, "COMPLEX") == 0)
    {
        fprintf(fp, "DATA_COMPONENTS: COMPLEX\n");
    }
    else if(nc > 1)
    {
        fprintf(fp, "DATA_COMPONENTS: %d\n", nc);
    }
    fprintf(fp, "VARIABLE: myvar\n");
#ifdef WORDS_BIGENDIAN
    fprintf(fp, "DATA_ENDIAN: BIG\n");
#else
    fprintf(fp, "DATA_ENDIAN: LITTLE\n");
#endif
    fprintf(fp, "CENTERING: zonal\n");
    fprintf(fp, "BRICK_ORIGIN: %g %g %g\n",
        minvals[0], minvals[1], minvals[2]);
    fprintf(fp, "BRICK_SIZE: %g %g %g\n",
        maxvals[0]-minvals[0],
        maxvals[1]-minvals[1],
        maxvals[2]-minvals[2]);
    if(offset_bytes > 0)
        fprintf(fp, "BYTE_OFFSET: %d\n", offset_bytes);
    if(subdiv)
    {
        fprintf(fp, "DIVIDE_BRICK: true\n");
        fprintf(fp, "DATA_BRICKLETS: %d %d %d\n",
            bricklet_size[0], bricklet_size[1], bricklet_size[2]);
    }
    if(bt)
    {
        fprintf(fp, "VARIABLE_MIN: 10.\n");
        fprintf(fp, "VARIABLE_MAX: 50.\n");
    }
    fclose(fp);
}

template <class T>
void
fill_BOV_indices(T *data, const int *full_size)
{
    T index = 0;
    T *ptr = data;
    for(int k = 0; k < full_size[2]; ++k)
    {
        for(int j = 0; j < full_size[1]; ++j)
        {
            for(int i = 0; i < full_size[0]; ++i)
            {
                *ptr++ = index;
                index = index + 1;
            }
        }
    }
}

// Template specialization for unsigned char.
void
fill_BOV_indices(unsigned char *data, const int *full_size)
{
    unsigned char *ptr = data;
    float root3 = sqrt(3.);

    for(int k = 0; k < full_size[2]; ++k)
    {
        float z = float(k) / float(full_size[2]-1);
        for(int j = 0; j < full_size[1]; ++j)
        {
            float y = float(j) / float(full_size[1]-1);
            for(int i = 0; i < full_size[0]; ++i)
            {
                float x = float(i) / float(full_size[0]-1);

                float r = sqrt(x*x + y*y + z*z);
                unsigned char c = (unsigned char)((int)(255.* r / root3));
                *ptr++ = c;
            }
        }
    }
}

template <class T>
void
write_BOV_types(const char *tname, const char *sname, int nc, T *data,
    bool bt)
{
    char filename[1000], headername[1000];

    //
    // Populate the array with indices and save out BOV.
    //
    snprintf(filename, 1000, "%s_indices.values", tname);
    printf("Creating %s\n", filename);
    fill_BOV_indices(data, full_size);
    write_BOV_data(filename, data, nx*ny*nz);
    snprintf(headername, 1000, "%s_indices.bov", tname);
    write_BOV_header(tname, sname, nc, headername, filename, 0, false, bt);
    // Write another header where the object is divided.
    snprintf(headername, 1000, "%s_indices_div.bov", tname);
    write_BOV_header(tname, sname, nc, headername, filename, 0, true, bt);

    //
    // Do the same thing but give the file a 16 element header
    // that we'll have to skip when we read it.
    //
    snprintf(filename, 1000, "%s_indices_div_with_header.values", tname);
    printf("Creating %s\n", filename);
    unsigned char *cptr = (unsigned char *)data;
    fill_BOV_indices(data + offset, full_size);
    for(int i = 0; i < offset * sizeof(T); ++i)
        cptr[i] = (unsigned char)i;
    write_BOV_data(filename, data, nx*ny*nz + offset);
    snprintf(headername, 1000, "%s_indices_div_with_header.bov", tname);
    write_BOV_header(tname, sname, nc, headername, filename, offset * sizeof(T),
                     true, bt);
}

class complex_t
{
public:
    complex_t() { real = 0; imag = 0; }
    complex_t(int v) { real = v; imag = 2*v; }
    complex_t(const complex_t &obj) { real = obj.real; imag = obj.imag; }
    ~complex_t() { ; }

    complex_t operator = (const complex_t &obj)
    {
        real = obj.real;
        imag = obj.imag;
        return *this;
    }

    complex_t operator + (int val)
    {
        real += val;
        imag = real * 2;
        return *this;
    }

    double real, imag;
};

class triple_t
{
public:
    triple_t() { data[0] = data[1] = data[2] = 0; }
    triple_t(int v) { data[0] = data[1] = data[2] = v; }
    triple_t(const triple_t &obj)
    { 
        data[0] = obj.data[0];
        data[1] = obj.data[1];
        data[2] = obj.data[2];
    }
    ~triple_t() { ; }

    triple_t operator = (const triple_t &obj)
    {
        data[0] = obj.data[0];
        data[1] = obj.data[1];
        data[2] = obj.data[2];
        return *this;
    }

    triple_t operator + (int val)
    {
        data[0] += val;
        data[1] += val;
        data[2] += val;
        return *this;
    }

    int data[3];
};

class quad_t
{
public:
    quad_t() { data[0] = data[1] = data[2] = data[3] = 0; }
    quad_t(int v) { data[0] = data[1] = data[2] = data[3] = v; }
    quad_t(const quad_t &obj)
    { 
        data[0] = obj.data[0];
        data[1] = obj.data[1];
        data[2] = obj.data[2];
        data[3] = obj.data[3];
    }
    ~quad_t() { ; }

    quad_t operator = (const quad_t &obj)
    {
        data[0] = obj.data[0];
        data[1] = obj.data[1];
        data[2] = obj.data[2];
        data[3] = obj.data[3];
        return *this;
    }

    quad_t operator + (int val)
    {
        data[0] += val;
        data[1] += val;
        data[2] += val;
        data[3] += val;
        return *this;
    }

    int data[4];
};

// ****************************************************************************
// Function: write_multi_bov
//
// Purpose: 
//   Writes out a multiblock BOV dataset.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 20 15:47:31 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
write_multi_bov()
{
    int ncomps = 1;
    float *data = new float[20 * 30 * 40 * ncomps];
    char filename[100];

    // Write the BOV files.
    int findex = 0;
    for(int k = 0; k < 2; ++k)
    {
        int start_k = k * 40;
        for(int j = 0; j < 2; ++j)
        {
            int start_j = j * 30;
            for(int i = 0; i < 2; ++i)
            {
                int start_i = i * 20;

                for(int kk = 0; kk < 40; ++kk)
                {
                    float k_val = start_k + kk;
                    float dZ = 40. - k_val;
                    float dZ2 = dZ * dZ;
                    for(int jj = 0; jj < 30; ++jj)
                    {
                        float j_val = start_j + jj;
                        float dY = 30. - j_val;
                        float dY2 = dY * dY;
                        for(int ii = 0; ii < 20; ++ii)
                        {
                            unsigned int index = (kk*20*30 + jj*20 + ii)*ncomps;
                            float dX = 20. - (start_i + ii);
                            data[index] = sqrt(dX*dX + dY2 + dZ2);
                        }
                    }
                }

                snprintf(filename, 100, "multi%04d.values", findex);
                write_BOV_data(filename, data, 20 * 30 * 40 * ncomps);

                findex++;
            }
        }
    }

    // Write the BOV header.
    FILE *fp = fopen("multi.bov", "wt");
    fprintf(fp, "TIME: 0\n");
    fprintf(fp, "DATA_FILE: multi%%04d.values\n");
    fprintf(fp, "DATA_SIZE: 40 60 80\n");
    fprintf(fp, "DATA_FORMAT: FLOAT\n");
    fprintf(fp, "DATA_COMPONENTS: %d\n", ncomps);
    fprintf(fp, "VARIABLE: vec\n");
#ifdef WORDS_BIGENDIAN
    fprintf(fp, "DATA_ENDIAN: BIG\n");
#else
    fprintf(fp, "DATA_ENDIAN: LITTLE\n");
#endif
    fprintf(fp, "CENTERING: zonal\n");
    fprintf(fp, "BRICK_ORIGIN: 0. 0. 0.\n");
    fprintf(fp, "BRICK_SIZE: 20. 30. 40.\n");
    fprintf(fp, "DATA_BRICKLETS: 20 30 40\n");
    fclose(fp);

    delete [] data;
}

// ****************************************************************************
// Purpose: This program writes out a couple of different BOV files.
// 
// Programmer: Brad Whitlock
// Creation:   Fri Mar 17 16:48:31 PST 2006
//
// Modifications:
//   Brad Whitlock, Thu May 4 12:21:18 PDT 2006
//   I made it write out int and double data also.
//
//   Brad Whitlock, Fri Oct 20 15:48:01 PST 2006
//   I made it write out a multiblock dataset.
//
// ****************************************************************************

int
main(int argc, char *argv[])
{
    int *idata = new int[nels + offset];
    write_BOV_types("INT", "INT", 1, idata, false);
    delete [] idata;

    float *fdata = new float[nels + offset];
    write_BOV_types("FLOAT", "FLOAT", 1, fdata, false);
    delete [] fdata;

    double *ddata = new double[nels + offset];
    write_BOV_types("DOUBLE", "DOUBLE", 1, ddata, false);
    delete [] ddata;

    complex_t *cdata = new complex_t[nels + offset];
    write_BOV_types("COMPLEX", "DOUBLE", 2, cdata, false);
    delete [] cdata;

    triple_t *tdata = new triple_t[nels + offset];
    write_BOV_types("TRIPLE", "INT", 3, tdata, false);
    delete [] tdata;

    quad_t *qdata = new quad_t[nels + offset];
    write_BOV_types("QUAD", "INT", 4, qdata, false);
    delete [] qdata;

    unsigned char *ucdata = new unsigned char[nels + offset];
    write_BOV_types("BYTE", "BYTE", 1, ucdata, true);
    delete [] ucdata;

    // Write out a multiblock dataset.
    write_multi_bov();

    return 0;
}
