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
#include <string.h>
#include <visitstream.h>
#include <string>
#include <vector>

#include "silo.h"

#undef NX
#define NX 100
#undef NY
#define NY 10
#undef NZ
#define NZ 15
#undef NT
#define NT 70

#undef DX
#define DX 10.
#undef DY
#define DY 0.5
#undef DZ
#define DZ 5.
#undef T0
#define T0 -2.
#undef DT
#define DT 14.

#define A  1.

int driver = DB_PDB;

void WriteFile(char *, double t, double t_minus_1, int cycle,
               bool, bool, bool);

// ****************************************************************************
// Function: main
//
// Purpose:
//   main function for the program
//
// Notes:      
//
// Programmer: Eric Brugger
// Creation:   2002
//
// Modifications:
//   Brad Whitlock, Mon May 12 17:53:38 PST 2003
//   I added a varaible that only gets written at some timesteps.
//
//   Eric Brugger, Thu Apr  8 09:00:27 PDT 2004
//   I added the creation of a second wave data set with a time varying SIL.
//
// ****************************************************************************

int
main(int argc, char *argv[])
{
    int       i;
    double    time, time_minus_1;
    int       cycle;

    for (i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-driver") == 0)
        {
            i++;

            if (strcmp(argv[i], "DB_HDF5") == 0)
            {
                driver = DB_HDF5;
            }
            else if (strcmp(argv[i], "DB_PDB") == 0)
            {
                driver = DB_PDB;
            }
            else
            {
               fprintf(stderr,"Uncrecognized driver name \"%s\"\n",
                   argv[i]);
            }
        }
    }

    ofstream ofile("wave.visit");

    int start(int(NT * 0.25f));
    int end(int(NT * 0.75f));

    //
    // Write the normal wave dataset.
    //
    for (i = 0; i < NT + 1; i++)
    {
        time = T0 + ((double)i / (double)NT) * DT;
        time_minus_1 = T0 + ((double)(i-1) / (double)NT) * DT;
        cycle = i * 10;
        bool writeTransient = (i >= start && i < end);
        char filename[1024];
        sprintf(filename, "wave%.4d.silo", cycle);
        WriteFile (filename, time, time_minus_1, cycle, writeTransient,
                   false, false);
        ofile << filename << endl;
    }

    //
    // Write the wave dataset with a time varying SIL.
    //
    ofstream ofile2("wave_tv.visit");

    int start2(int(NT * 0.1f));
    int end2(int(NT * 0.2f));

    for (i = 0; i < NT + 1; i++)
    {
        time = T0 + ((double)i / (double)NT) * DT;
        time_minus_1 = T0 + ((double)(i-1) / (double)NT) * DT;
        cycle = i * 10;
        bool writeTransientVar = (i >= start && i < end);
        bool writeTransientMesh = (i == 2);
        bool writeTransientMat  = (i >= start2 && i < end2);
        char filename[1024];
        sprintf(filename, "wave_tv%.4d.silo", cycle);
        WriteFile (filename, time, time_minus_1, cycle, writeTransientVar,
                   writeTransientMesh, writeTransientMat);
        ofile2 << filename << endl;
    }

    return 0;
}

//
// Template function for memory reallocation.
//

template <class T>
T *remake(T *ptr, int oldsize, int size)
{
    T *retval = new T[size];
    T *iptr = retval;
    for(int i = 0; i < oldsize; ++i)
        *iptr++ = ptr[i];
    delete [] ptr;
    return retval;
}

// ****************************************************************************
// Class: MaterialList
//
// Purpose:
//   Keeps track of mixed material information.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 21 13:53:35 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Mar 17 17:01:16 PST 2004
//   I fixed an off by one error.
//
// ****************************************************************************

class MaterialList
{
public:
    MaterialList() : matNames()
    {
        have_mixed = false;
        mix_zone = NULL;
        mix_mat = NULL;
        mix_vf = NULL;
        mix_next = NULL;
        matlist = NULL;

        // initialize private members.
        _array_size = 0;
        _array_index = 1;
        _array_growth = 1000;
    }

    ~MaterialList()
    {
       delete [] matlist;
       if (have_mixed)
       {
           delete [] mix_zone;   
           delete [] mix_mat;    
           delete [] mix_vf;      
           delete [] mix_next;    
       }
    }

    void AddMaterial(const std::string &mat)
    {
        matNames.push_back(mat);
    }

    void AddClean(int siloZone, int matNumber)
    {
        matlist[siloZone] = matNumber;
    }

    void AddMixed(int siloZone, int *matNumbers, float *matVf, int nMats)
    {
        int i;

        /* Grow the arrays if they will not fit nMats materials. */
        Resize(nMats);

        /* Record the mixed zone as a negative offset into the mix arrays. */
        matlist[siloZone] = -_array_index;

        /* Update the mix arrays. */
        for(i = 0; i < nMats; ++i)
        {
            int index = _array_index - 1;

            mix_zone[index] = siloZone;
            mix_mat[index]  = matNumbers[i];
            mix_vf[index]   = matVf[i];

            if(i < nMats - 1)
                mix_next[index] = index + 2;
            else
                mix_next[index] = 0;

            ++(_array_index);
        }

        /* indicate that we have mixed materials. */
        have_mixed = true;
    }

    void AllocClean(int nZones)
    {
        matlist = new int[nZones];
    }

    int GetMixedSize() const { return _array_index - 1; };

    void WriteMaterial(DBfile *db, const char *matvarname, const char *meshName, int nx, int ny, int nz)
    {
        int i, mdims[3] = {nx,ny,nz};

        /* Create a 1..nTotalMaterials material number array. */
        int *allmats = new int[matNames.size()];
        for(i = 0; i < matNames.size(); ++i)
            allmats[i] = i + 1;

        DBoptlist *optList = DBMakeOptlist(2);

        // Add material names.
        char **matnames = new char *[matNames.size()];
        for(i = 0; i < matNames.size(); ++i)
            matnames[i] = (char *)matNames[i].c_str();
        DBAddOption(optList, DBOPT_MATNAMES, matnames);

        if (have_mixed)
        {
            DBPutMaterial(db, (char *)matvarname, (char *)meshName,
                          matNames.size(), allmats,
                          matlist, mdims, 3, mix_next,
                          mix_mat, mix_zone,
                          (float*)mix_vf , GetMixedSize(),
                          DB_FLOAT, optList);
        }
        else
        {         
            DBPutMaterial(db, (char *)matvarname, (char *)meshName,
                          matNames.size(), allmats,
                          matlist, mdims, 3, NULL,
                          NULL, NULL, NULL , 0,
                          DB_INT, optList);
        }

        DBFreeOptlist(optList);
        delete [] matnames;
        delete [] allmats;
    }
private:
    void Resize(int nMats)
    { 
        if(_array_index + nMats >= _array_size)
        {
            int new_size = _array_size + _array_growth;

            if(_array_size == 0)
            {
                /* Reallocate arrays in large increments. */
                mix_zone = new int[new_size];
                mix_mat  = new int[new_size];
                mix_vf   = new float[new_size];
                mix_next = new int[new_size];
            }
            else
            {
                /* Reallocate arrays in large increments. */
                mix_zone = remake(mix_zone, _array_size, new_size);
                mix_mat  = remake(mix_mat, _array_size,  new_size);
                mix_vf   = remake(mix_vf, _array_size, new_size);
                mix_next = remake(mix_next, _array_size, new_size);
            }

            _array_size = new_size;
        }
    }

    int    have_mixed;
    int    *mix_zone;
    int    *mix_mat;
    float  *mix_vf;
    int    *mix_next;
    int    *matlist;
    int    _array_size;
    int    _array_index;
    int    _array_growth;
    std::vector<std::string> matNames;
};

void CreateMaterial(MaterialList &mat, int cycle, bool writeTransientMat);

// ****************************************************************************
// Modifications:
//   Brad Whitlock, Tue Jan 21 13:41:11 PST 2003
//   I added a material to the mesh.
//
//   Hank Childs, Thu Apr 10 09:24:00 PDT 2003
//   I added a vector variable using a defvar.
//
//   Brad Whitlock, Mon May 12 17:47:46 PST 2003
//   I added a variable that only exists for a subset of the timesteps.
//
//   Eric Brugger, Thu Apr  8 09:00:27 PDT 2004
//   I added the filename as an argument.  I added a point mesh and additional
//   materials that only exists for a subset of the timesteps.
//
//   Mark C. Miller, Mon Apr 25 21:35:54 PDT 2005
//   Added chars, shorts and ints variables for cycle 0000
//
// ****************************************************************************

void
WriteFile(char *filename, double time, double time_minus_1, int cycle,
    bool writeTransientVar, bool writeTransientMesh, bool writeTransientMat)
{
    int       i, j, k;
    char      *coordnames[3]={"x", "y", "z"};
    float     *coords[3];
    float     *coords2[3];
    float     xcoord[(NX+1)*(NY+1)*(NZ+1)];
    float     ycoord[(NX+1)*(NY+1)*(NZ+1)];
    float     zcoord[(NX+1)*(NY+1)*(NZ+1)];
    float     xcoord2[(NX+1)*(NZ+1)];
    float     ycoord2[(NX+1)*(NZ+1)];
    float     zcoord2[(NX+1)*(NZ+1)];
    float     u[(NX+1)*(NY+1)*(NZ+1)];
    float     v[(NX+1)*(NY+1)*(NZ+1)];
    float     w[(NX+1)*(NY+1)*(NZ+1)];
    char      cbuf[(NX+1)*(NY+1)*(NZ+1)];
    short     sbuf[(NX+1)*(NY+1)*(NZ+1)];
    int       ibuf[(NX+1)*(NY+1)*(NZ+1)];
    float     var[(NX+1)*(NY+1)*(NZ+1)];
    float     transient[(NX+1)*(NY+1)*(NZ+1)];
    int       dims[3];
    int       ndims;
    int       nels;
    float     widths[3];
    DBfile    *dbfile;
    DBoptlist *optList;
    char letter;

    ndims = 3;
    dims[0] = NX + 1;
    dims[1] = NY + 1;
    dims[2] = NZ + 1;
    nels = (NX + 1) * (NZ + 1);
    widths[0] = DX;
    widths[1] = DY;
    widths[2] = DZ;
     
    /*
     * Build the mesh and a variable.
     */
    letter = 'a';
    for (i = 0; i < NX + 1; i++)
    {
        for (j = 0; j < NY + 1; j++)
        {
            for (k = 0; k < NZ + 1; k++)
            {
                double    x1, y1, z1;
                double    dy, dy_minus_1;
  
                x1 = ((float)i / (float)NX) * widths[0];
                y1 = ((float)j / (float)NY) * widths[1];
                dy = (A * A * A) / (A + (x1 - time) * (x1 - time));
                dy_minus_1 = 
                 (A * A * A) / (A + (x1 - time_minus_1) * (x1 - time_minus_1));
                z1 = ((float)k / (float)NZ) * widths[2];

                int nodeIndex = i+j*(NX+1)+k*(NX+1)*(NY+1);
                xcoord[nodeIndex] = x1;
                ycoord[nodeIndex] = y1 + dy;
                zcoord[nodeIndex] = z1;

                var[nodeIndex] = dy;
                u[nodeIndex] = 0.;
                v[nodeIndex] = dy - dy_minus_1;
                w[nodeIndex] = 0.;
                cbuf[nodeIndex] = letter;
                sbuf[nodeIndex] = (short)j;
                ibuf[nodeIndex] = k;

                if(writeTransientVar)
                {
                    float t = float(i) / float(NX);
                    float X = t * DX;
                    float Y = cos(t * 4. * M_PI) + DZ;
                    float Z = sin(t * 4. * M_PI) + DZ / 2.;
                    float dX = X - xcoord[nodeIndex];
                    float dY = Y - ycoord[nodeIndex];
                    float dZ = Z - zcoord[nodeIndex];
                    transient[nodeIndex] = float(sqrt(dX*dX + dY*dY + dZ*dZ));
                }
                letter++;
                if (letter == 'z'+1)
                    letter = 'a';
            }
        }
    }

    coords[0] = xcoord;
    coords[1] = ycoord;
    coords[2] = zcoord;

    /*
     * Create a point mesh.
     */
    for (i = 0; i < NX + 1; i++)
    {
        for (k = 0; k < NZ + 1; k++)
        {
            int nodeIndex  = i+NY*(NX+1)+k*(NX+1)*(NY+1);
            int nodeIndex2 = i+k*(NX+1);

            xcoord2[nodeIndex2] = xcoord[nodeIndex];
            ycoord2[nodeIndex2] = ycoord[nodeIndex];
            zcoord2[nodeIndex2] = zcoord[nodeIndex];
        }
    }

    coords2[0] = xcoord2;
    coords2[1] = ycoord2;
    coords2[2] = zcoord2;

    /*
     * Create a material for the file.
     */
    MaterialList mat;
    CreateMaterial(mat, cycle, writeTransientMat);

    /*
     * Create a file that contains the mesh and variables.
     */
    dbfile = DBCreate(filename, 0, DB_LOCAL, "The Wave", driver);

    optList = DBMakeOptlist(10);
    DBAddOption(optList, DBOPT_DTIME, &time);
    DBAddOption(optList, DBOPT_CYCLE, &cycle);
    
    DBPutQuadmesh(dbfile, "quadmesh", coordnames, coords, dims, ndims,
                  DB_FLOAT, DB_NONCOLLINEAR, optList);

    if(writeTransientMesh)
    {
        DBPutPointmesh(dbfile, "pointmesh", ndims, coords2, nels, DB_FLOAT,
                       optList);
    }
    if(cycle == 0)
    {
        int doLabel = 1;
        DBoptlist *optList1 = DBMakeOptlist(5);
        DBAddOption(optList1, DBOPT_DTIME, &time);
        DBAddOption(optList1, DBOPT_CYCLE, &cycle);
        DBAddOption(optList1, DBOPT_ASCII_LABEL, &doLabel);

        DBPutQuadvar1(dbfile, "chars", "quadmesh", (float*)cbuf, dims, ndims, NULL,
                      0, DB_CHAR, DB_NODECENT, optList1);

        DBFreeOptlist(optList1);

        DBPutQuadvar1(dbfile, "shorts", "quadmesh", (float*)sbuf, dims, ndims, NULL,
                      0, DB_SHORT, DB_NODECENT, optList);
        DBPutQuadvar1(dbfile, "ints", "quadmesh", (float*)ibuf, dims, ndims, NULL,
                      0, DB_INT, DB_NODECENT, optList);
    }

    DBPutQuadvar1(dbfile, "pressure", "quadmesh", var, dims, ndims, NULL,
                  0, DB_FLOAT, DB_NODECENT, optList);

    if(writeTransientVar)
    {
        DBPutQuadvar1(dbfile, "transient", "quadmesh", transient, dims, ndims,
                      NULL, 0, DB_FLOAT, DB_NODECENT, optList);
    }

    mat.WriteMaterial(dbfile, "Material", "quadmesh", NX, NY, NZ);

    /*
     * Write out the components of the vector and make a defvar for it.
     */
    DBPutQuadvar1(dbfile, "u", "quadmesh", u, dims, ndims, NULL,
                  0, DB_FLOAT, DB_NODECENT, optList);
    DBPutQuadvar1(dbfile, "v", "quadmesh", v, dims, ndims, NULL,
                  0, DB_FLOAT, DB_NODECENT, optList);
    DBPutQuadvar1(dbfile, "w", "quadmesh", w, dims, ndims, NULL,
                  0, DB_FLOAT, DB_NODECENT, optList);
    char *defvars = "direction vector {u,v,w}";
    int amt = strlen(defvars) + 1;
    DBWrite(dbfile, "_meshtv_defvars", defvars, &amt, 1, DB_CHAR);

    DBFreeOptlist(optList);

    DBClose(dbfile);
}

float Distance(float pt[2], float origin[2])
{
    float dX = pt[0] - origin[0];
    float dZ = pt[1] - origin[1];
    return sqrt(dX*dX + dZ*dZ);
}

// ****************************************************************************
// Function: CreateMaterial
//
// Purpose: 
//   Sets up the materials based on the time.
//
// Arguments:
//   mat   : The material list object.
//   cycle : The cycle number for the time.
//   writeTransientMat : Flag indicating if two additional materials should
//                       be included.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 22 09:54:37 PDT 2003
//
// Modifications:
//   Eric Brugger, Thu Apr  8 09:00:27 PDT 2004
//   I added the ability to include two additional materials if
//   writeTransientMat is true.
//   
// ****************************************************************************

void
CreateMaterial(MaterialList &mat, int cycle, bool writeTransientMat)
{
    mat.AllocClean((NX)*(NY)*(NZ));
    mat.AddMaterial("barrier");
    mat.AddMaterial("water");
    mat.AddMaterial("dirt");

    if (writeTransientMat)
    {
        mat.AddMaterial("firstblock");
        mat.AddMaterial("secondblock");
    }

    float t = (float(cycle) / 10.) / float(NT + 1);

    float WIDTH  = DZ;
    float LENGTH = DX;

    float dX = float(DX) / float(NX);
    float dZ = float(DZ) / float(NZ);

    float cellDist = sqrt(dX*dX + dZ*dZ);
    float RADIUS = WIDTH * 0.25;
    float LARGE_RADIUS = RADIUS + cellDist;
    float SIDE_CLEARANCE = RADIUS * 1.25;

    float origin[] = {WIDTH * 0.5, LENGTH * 0.5};

    for (int i = 0; i < NX; i++)
    {
        for (int j = 0; j < NY ; j++)
        {
            for (int k = 0; k < NZ; k++)
            {
                int zoneid = k*(NY*NX) + j*NX + i;
                float x = ((float)i / (float)NX) * DX;
                float z = ((float)k / (float)NZ) * DZ;
                float ptMid[] = {z + 0.5 * dZ, x + 0.5 * dX};

                if (writeTransientMat && x > 8.0)
                {
                    if (x > 9.0)
                        mat.AddClean(zoneid, 5);
                    else
                        mat.AddClean(zoneid, 4);
                }
                else if(ptMid[0] < (origin[0] - SIDE_CLEARANCE) ||
                   ptMid[0] > (origin[0] + SIDE_CLEARANCE))
                {
                    mat.AddClean(zoneid, 3);
                }
                else
                {
                    float rad = Distance(ptMid, origin);
                    if(rad < LARGE_RADIUS)
                    {
                        // Figure the mix with the barrier material
                        int mat1count = 0;
                        const int SUBDIV = 10;
                        for(int ii = 0; ii < SUBDIV; ++ii)
                        {
                            float xSub = x + float(ii)/float(SUBDIV-1) * dX;
                            for(int jj = 0; jj < SUBDIV; ++jj)
                            {
                                float zSub = z + float(jj)/float(SUBDIV-1) * dZ;
                                float pt[] = {zSub, xSub};
                                if(Distance(pt, origin) < RADIUS)
                                    ++mat1count;
                            }
                        }

                        if(mat1count == SUBDIV * SUBDIV)
                        {
                            mat.AddClean(zoneid, 1);
                        }
                        else if(mat1count == 0)
                        {
                            int matno = (x < t * DX) ? 2 : 3;
                            mat.AddClean(zoneid, matno);
                        }
                        else
                        {
                            float matvf[2];
                            matvf[0] = float(mat1count) / float(SUBDIV * SUBDIV);
                            matvf[1] = 1. - matvf[0];
                            int matnos[2];
                            matnos[0] = 1;
                            matnos[1] = (x < t * DX) ? 2 : 3;
                            mat.AddMixed(zoneid, matnos, matvf, 2);
                        }
                    }
                    else if(x < t * DX)
                        mat.AddClean(zoneid, 2);
                    else
                        mat.AddClean(zoneid, 3);
                }
            }
        }
    }
}
