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

#include <stdio.h>
#include <silo.h>
#include <math.h>
#include <visitstream.h>
#include <vector>
#include <string>

// ****************************************************************************
// File: tire.C
//
// Purpose:
//   This program creates a multiblock silo database that looks like a tire.
//   The tire has three materials: rubber, cord, steel and a few variables.
//   The most interesting thing about this database is its geometry and that
//   it can be decomposed into subsets that look like something real.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 21 13:50:00 PST 2002
//
// Modifications:
//   
// ****************************************************************************

//
// Mesh dimensions
//

#define ISTEPS 10
#define JSTEPS 30
#define KSTEPS 10

//
// Bezier patch control meshes
//

float cm1[4][4][2] = {
{{0,0}, {1,-1}, {2,-2}, {3,-3}},
{{1,1}, {2,0}, {3,-1}, {6,4}},
{{1,20}, {2,21}, {3,22}, {4,23}},
{{0,22}, {1,23}, {2,24}, {3,25}}
};

float cm2[4][4][2] = {
{{0,22}, {1,23}, {2,24}, {3,25}},
{{-2,26}, {-1,26}, {1,26}, {2,28}},
{{-3,21}, {-4,24}, {-3,27}, {0,28}},
{{-6,24}, {-6,27}, {-6, 28}, {-3,28}}
};

float cm3[4][4][2] = {
{{-15,-2}, {-15,-4}, {-15,-5}, {-15,-7}},
{{-7,-2}, {-6,-4}, {-3,-5}, {0,-7}},
{{-2,-2}, {-2,-3}, {-1,-5}, {2,-5}},
{{0,0}, {1,-1}, {2,-2}, {3,-3}},
};

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
//   Keeps track of mixed material information
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 21 13:47:42 PST 2002
//
// Modifications:
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

    void AddMixed(int siloZone, int *matNumbers, double *matVf, int nMats)
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

    int GetMixedSize() const { return _array_index; };

    void WriteMaterial(DBfile *db, const char *matvarname, const char *meshName, int nx, int ny, int nz)
    {
        int i, mdims[3] = {nx,ny,nz};

        /* Create a 1..nTotalMaterials material number array. */
        int *allmats = new int[matNames.size()];
        for(i = 0; i < matNames.size(); ++i)
            allmats[i] = i + 1;

        DBoptlist *optList = DBMakeOptlist(2);

        // Add material names.
        char **matnames = new char *[4];
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
                          DB_DOUBLE, optList);
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
                mix_vf   = new double[new_size];
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
    double *mix_vf;
    int    *mix_next;
    int    *matlist;
    int    _array_size;
    int    _array_index;
    int    _array_growth;
    std::vector<std::string> matNames;
};

// ****************************************************************************
// Class: MeshArray
//
// Purpose:
//   Creates a 2D curvilinear mesh using a Bezier patch for the geometry
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 21 13:47:07 PST 2002
//
// Modifications:
//   Eric Brugger, Wed Mar 26 09:32:46 PST 2003
//   I added an index range check to SetX and SetY to get around a compiler
//   bug with g++ 2.96 on Linux with optimization.
//   
// ****************************************************************************

class MeshArray
{
public:
    MeshArray(int X, int Y, float cmesh[4][4][2])
    {
        int nels = X * Y;
        xdim = X;
        ydim = Y;
        coordsX = new float[nels];
        coordsY = new float[nels];
        EvalMesh(cmesh);
    }

    ~MeshArray()
    {
        delete [] coordsX;
        delete [] coordsY;
    }

    int XDim() const { return xdim; };
    int YDim() const { return ydim; };

    float GetX(int x, int y) const
    {
        return coordsX[y*xdim + x];
    }

    float GetY(int x, int y) const
    {
        return coordsY[y*xdim + x];
    }

    void WriteMesh(DBfile *db, const char *name)
    {
         float *coords[2] = {coordsX, coordsY};
         int dims[2] = {xdim, ydim};
         DBPutQuadmesh(db, (char *)name, NULL, coords, dims, 2, DB_FLOAT, DB_NONCOLLINEAR, NULL);
    }

    void SetX(int x, int y, float val)
    {
        // Range check gets around a compiler bug with g++ 2.96 on Linux
        // with optimization.
        if ((y*xdim + x) >= (xdim * ydim))
            cerr << "MeshArray::GetX out of range index reference." << endl;
        coordsX[y*xdim + x] = val;
    }

    void SetY(int x, int y, float val)
    {
        // Range check gets around a compiler bug with g++ 2.96 on Linux
        // with optimization.
        if ((y*xdim + x) >= (xdim * ydim))
            cerr << "MeshArray::GetX out of range index reference." << endl;
        coordsY[y*xdim + x] = val;
    }

private:
    float EvalBezier(float t, float c0, float c1, float c2, float c3)
    {
        float t2 = t * t;
        float t3 = t * t2;
        float omt = 1. - t;
        float omt2 = omt * omt;
        float omt3 = omt * omt2;

        float ret = c0*omt3 + 3.*c1*omt2*t +  3.*c2*omt*t2 + c3*t3;
        return ret;
    }

    void EvalMesh(float cm[4][4][2])
    {
        int istep = xdim;
        int jstep = ydim;
        float i, j;
        float delta_i = 1. / float(istep - 1);
        float delta_j = 1. / float(jstep - 1);
        int ii = 0, jj = 0;
        for(ii = 0, i = 0.; ii < istep; ++ii, i += delta_i)
        {
            float A[2], B[2], C[2], D[2];
            A[0] = EvalBezier(i, cm[0][0][0], cm[0][1][0], cm[0][2][0], cm[0][3][0]);
            A[1] = EvalBezier(i, cm[0][0][1], cm[0][1][1], cm[0][2][1], cm[0][3][1]);

            B[0] = EvalBezier(i, cm[1][0][0], cm[1][1][0], cm[1][2][0], cm[1][3][0]);
            B[1] = EvalBezier(i, cm[1][0][1], cm[1][1][1], cm[1][2][1], cm[1][3][1]);

            C[0] = EvalBezier(i, cm[2][0][0], cm[2][1][0], cm[2][2][0], cm[2][3][0]);
            C[1] = EvalBezier(i, cm[2][0][1], cm[2][1][1], cm[2][2][1], cm[2][3][1]);

            D[0] = EvalBezier(i, cm[3][0][0], cm[3][1][0], cm[3][2][0], cm[3][3][0]);
            D[1] = EvalBezier(i, cm[3][0][1], cm[3][1][1], cm[3][2][1], cm[3][3][1]);

            for(jj = 0, j = 0.; jj < jstep; ++jj, j += delta_j)
            {
                SetX(ii, jj, EvalBezier(j, A[0], B[0], C[0], D[0]));
                SetY(ii, jj, EvalBezier(j, A[1], B[1], C[1], D[1]));
            }
        }
    }

    int xdim;
    int ydim;
    float *coordsX;
    float *coordsY;
};


// ****************************************************************************
// Class: MeshArray3D
//
// Purpose:
//   Creates a 3D curvilinear mesh.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 21 13:46:44 PST 2002
//
// Modifications:
//   
// ****************************************************************************

class MeshArray3D
{
    class MeshData
    {
    public:
        MeshData(const std::string &n, int nx, int ny, int nz, bool node) : name(n)
        {
            xdim = nx;
            ydim = ny;
            zdim = nz;
            int ndata;
            if(node)
               ndata = xdim * ydim * zdim;
            else
               ndata = (xdim-1) * (ydim-1) * (zdim-1);
            data = new float[ndata];
            nodal = node;
        }

        ~MeshData()
        {
            delete [] data;
        }

        void SetZonalValue(int x, int y, int z, float val)
        {
            data[z*((ydim-1)*(xdim-1)) + y*(xdim-1) + x] = val;
        }

        void SetNodalValue(int x, int y, int z, float val)
        {
            data[z*(ydim*xdim) + y*xdim + x] = val;
        }

        int xdim, ydim, zdim;
        bool nodal;
        float *data;
        std::string name;
    };

    typedef std::vector<MeshData *> MeshDataVector;

public:
    MeshArray3D(const MeshArray &ma, int Z, void (*transform)(float *, float)) : data()
    {
        xdim = ma.XDim();
        ydim = ma.YDim();
        zdim = Z;
        int nels = xdim * ydim * Z;
        coordX = new float[nels];
        coordY = new float[nels];
        coordZ = new float[nels];

        // Allocate storage for the material cells.
        mats.AllocClean((xdim-1) * (ydim-1) * (zdim-1));

        // Transform the mesh.
        for(int k = 0; k < zdim; ++k)
        {
            float t = float(k) / float(zdim-1);
            for(int i = 0; i < xdim; ++i)
                for(int j = 0; j < ydim; ++j)
                {
                    float pt[3];
                    pt[0] = ma.GetX(i, j);
                    pt[1] = ma.GetY(i, j);
                    pt[2] = 0.;
                    (*transform)(pt, t);
                    SetX(i, j, k, pt[0]);
                    SetY(i, j, k, pt[1]);
                    SetZ(i, j, k, pt[2]);
                }
         }
    }

    ~MeshArray3D()
    {
        delete [] coordX;
        delete [] coordY;
        delete [] coordZ;

        for(int i = 0; i < data.size(); ++i)
            delete data[i];
    }

    int XDim() const { return xdim; };
    int YDim() const { return ydim; };
    int ZDim() const { return zdim; };

    void WriteMesh(DBfile *db, const char *name)
    {
         // Write the meshes.
         float *coords[3] = {coordX, coordY, coordZ};
         int dims[3] = {xdim, ydim, zdim};
         DBPutQuadmesh(db, (char *)name, NULL, coords, dims, 3, DB_FLOAT, DB_NONCOLLINEAR, NULL);

         // Write the mesh data
         for(int i = 0; i < data.size(); ++i)
         {
             MeshData *d = data[i];
             if(d->nodal)
             {
                 int ndims[] = {xdim, ydim, zdim};
                 DBPutQuadvar1(db, (char *)d->name.c_str(), (char *)name, d->data,
                               ndims, 3, NULL, 0, DB_FLOAT, DB_NODECENT, NULL);
             }
             else
             {
                 int zdims[] = {xdim-1, ydim-1, zdim-1};
                 DBPutQuadvar1(db, (char *)d->name.c_str(), (char *)name, d->data,
                               zdims, 3, NULL, 0, DB_FLOAT, DB_ZONECENT, NULL);
             }
         }

         // Write the material data
         mats.WriteMaterial(db, "mat1", name, xdim-1, ydim-1, zdim-1);
    }

    float GetX(int x, int y, int z) const
    {
        return coordX[z*(ydim*xdim) + y*xdim + x];
    }

    float GetY(int x, int y, int z) const
    {
        return coordY[z*(ydim*xdim) + y*xdim + x];
    }

    float GetZ(int x, int y, int z) const
    {
        return coordZ[z*(ydim*xdim) + y*xdim + x];
    }

    void SetX(int x, int y, int z, float val)
    {
        coordX[z*(ydim*xdim) + y*xdim + x] = val;
    }

    void SetY(int x, int y, int z, float val)
    {
        coordY[z*(ydim*xdim) + y*xdim + x] = val;
    }

    void SetZ(int x, int y, int z, float val)
    {
        coordZ[z*(ydim*xdim) + y*xdim + x] = val;
    }

    void CreateZonalData(const char *name, float (*zonal)(int,int,int, MeshArray3D *))
    {
        int ndata = (xdim-1) * (ydim-1) * (zdim-1);
        MeshData *m = new MeshData(name, xdim, ydim, zdim, false);
        data.push_back(m);

        // Create the data.
        for(int i = 0; i < xdim-1; ++i)
            for(int j = 0; j < ydim-1; ++j)
                for(int k = 0; k < zdim-1; ++k)
                    m->SetZonalValue(i,j,k, (*zonal)(i,j,k, this));
    }

    void CreateNodalData(const char *name, float (*nodal)(float *, MeshArray3D *))
    {
        MeshData *m = new MeshData(name, xdim, ydim, zdim, true);
        data.push_back(m);

        // Create the data.
        float *fptr = m->data;
        for(int i = 0; i < xdim; ++i)
            for(int j = 0; j < ydim; ++j)
                for(int k = 0; k < zdim; ++k)
                {
                    float pt[3];
                    pt[0] = GetX(i,j,k);
                    pt[1] = GetY(i,j,k);
                    pt[2] = GetZ(i,j,k);
                    m->SetNodalValue(i, j, k, (*nodal)(pt, this));
                }
    }

    void AddMaterial(const char *matname)
    {
        mats.AddMaterial(matname);
    }

    void CreateMaterialData(void (*createmat)(int,int,int, int *, double*, int *, MeshArray3D *))
    {
        int nMats = 1;
        int matnos[100];
        double matVf[100];

        // Create the data.
        for(int i = 0; i < xdim-1; ++i)
            for(int j = 0; j < ydim-1; ++j)
                for(int k = 0; k < zdim-1; ++k)
                {
                    int zoneid = k*((ydim-1)*(xdim-1)) + j*(xdim-1) + i;
                    (*createmat)(i,j,k, matnos, matVf, &nMats, this);
                    if(nMats > 1)
                        mats.AddMixed(zoneid, matnos, matVf, nMats);
                    else
                        mats.AddClean(zoneid, matnos[0]);
                }
    }

    void IJKFromZone(int zoneid, int &i, int &j, int &k)
    {
        k = zoneid / (ydim*xdim);
        int tmp = zoneid - (k *(ydim * xdim));
        j = tmp / xdim;
        i = tmp % xdim;
    }
private:
    int xdim;
    int ydim;
    int zdim;
    float *coordX;
    float *coordY;
    float *coordZ;
    MeshDataVector data;
    MaterialList mats;
};

//
// Geometry, Data, and Material creation callbacks
//

void
extrude(float *pt, float t)
{
    pt[2] = t * 20;
}

void
revolve1(float *pt, float t)
{
    // Revolve the point around X axis.
    float theta = (3.14159 / 2.) * t;
    float r = fabs(pt[1]);
    pt[1] = r * cos(theta);
    pt[2] = r * sin(theta);
}

void
revolve2(float *pt, float t)
{
    // Revolve the point around X axis.
    float theta = (3.14159 / 2.) * t + (3.14159 / 2.);
    float r = fabs(pt[1]);
    pt[1] = r * cos(theta);
    pt[2] = r * sin(theta);
}

void
revolve3(float *pt, float t)
{
    // Revolve the point around X axis.
    float theta = (3.14159 / 2.) * t + (2. * 3.14159 / 2.);
    float r = fabs(pt[1]);
    pt[1] = r * cos(theta);
    pt[2] = r * sin(theta);
}

void
revolve4(float *pt, float t)
{
    // Revolve the point around X axis.
    float theta = (3.14159 / 2.) * t + (3. * 3.14159 / 2.);
    float r = fabs(pt[1]);
    pt[1] = r * cos(theta);
    pt[2] = r * sin(theta);
}

float
CreatePressure(int i, int j, int k, MeshArray3D *m)
{
    return m->GetX(i,j,k);
}

float
CreateSteelBelted(int i, int j, int k, MeshArray3D *m)
{
    float val = 0.;

    if((i<m->XDim()-1) && (j<m->YDim()-1) && (k<m->ZDim()-1))
    {
        if(i>0 && j > 0 && (i%3)==0 && (j%3)==0)
            val = 10.;
    }

    return val;
}

float
CreateTemp(float *pt, MeshArray3D *)
{
    return float(sqrt(pt[0]*pt[0] + pt[1]*pt[1] + pt[2]*pt[2]));
}

void
CreateMaterial(int i, int j, int k, int *matnos, double *matVf, int *nMats,
    MeshArray3D *m)
{
    matnos[0] = 1;
    matVf[0] = 1.;
    *nMats = 1;

    if((i<m->XDim()-1) && (j<m->YDim()-1) && (k<m->ZDim()-1))
    {
        if(i>0 && j > 0 && (i%3)==0 && (j%3)==0)
        {
            if(j >= JSTEPS*2 && j < JSTEPS*4)
                matnos[0] = 2;
            else
                matnos[0] = 3;
            matVf[0] = 1.;

            *nMats = 1;
        }
    }
}

// ****************************************************************************
// Function: main
//
// Purpose:
//   This is the main function for the creation of the tire database.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 21 13:45:40 PST 2002
//
// Modifications:
//   
// ****************************************************************************

int
main(int argc, char *argv[])
{
    DBfile *db;
    int i, driver = DB_PDB;

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

    db = DBCreate("tire.silo", DB_CLOBBER, DB_LOCAL, "Fake tire dataset", driver);

    MeshArray A(ISTEPS, JSTEPS, cm3);
    MeshArray B(ISTEPS, JSTEPS, cm1);
    MeshArray C(ISTEPS, JSTEPS, cm2);

    // Mess some of A's coordinates to create treads
    int maxj = JSTEPS/2;
    for(int j = 0; j < maxj; j+=2)
    {
        float t = float(j) / float(maxj-1);
        float offset = 0.2*(1.-t) + 0.5*t;
        A.SetY(ISTEPS-1, j, A.GetY(ISTEPS-1, j)+offset);
    }

    // Glue them into one mesh and revolve them.
    MeshArray D(ISTEPS, JSTEPS * 6 - 5, cm3);
    float yaxis = 50.;
    for(int ii = 0; ii < ISTEPS; ++ii)
    {
        int jj, jindex = 0;

        for(jj = JSTEPS-1; jj > 0; --jj, ++jindex)
        {
             D.SetX(ii, jindex, -(C.GetX(ii, jj) + 15));
             D.SetY(ii, jindex, C.GetY(ii, jj) - yaxis);
        }
        for(jj = JSTEPS-1; jj > 0; --jj, ++jindex)
        {
             D.SetX(ii, jindex, -(B.GetX(ii, jj) + 15));
             D.SetY(ii, jindex, B.GetY(ii, jj) - yaxis);
        }
        for(jj = JSTEPS-1; jj > 0; --jj, ++jindex)
        {
             D.SetX(ii, jindex, -(A.GetX(ii, jj) + 15));
             D.SetY(ii, jindex, A.GetY(ii, jj) - yaxis);
        }


        for(jj = 0; jj < JSTEPS-1; ++jj, ++jindex)
        {
             D.SetX(ii, jindex, A.GetX(ii, jj) + 15);
             D.SetY(ii, jindex, A.GetY(ii, jj) - yaxis);
        }
        for(jj = 0; jj < JSTEPS-1; ++jj, ++jindex)
        {
             D.SetX(ii, jindex, B.GetX(ii, jj) + 15);
             D.SetY(ii, jindex, B.GetY(ii, jj) - yaxis);
        }
        for(jj = 0; jj < JSTEPS; ++jj, ++jindex)
        {
             D.SetX(ii, jindex, C.GetX(ii, jj) + 15);
             D.SetY(ii, jindex, C.GetY(ii, jj) - yaxis);
        }
    }

    MeshArray3D E(D, KSTEPS, revolve1);
    MeshArray3D F(D, KSTEPS, revolve2);
    MeshArray3D G(D, KSTEPS, revolve3);
    MeshArray3D H(D, KSTEPS, revolve4);
    MeshArray3D *meshes[4] = {&E, &F, &G, &H};

    char *meshnames[] = {"section0", "section1", "section2", "section3"};

    for(int i = 0; i < 4; ++i)
    {
        meshes[i]->CreateZonalData("pressure", CreatePressure);
        meshes[i]->CreateZonalData("sb", CreateSteelBelted);
        meshes[i]->CreateNodalData("temperature", CreateTemp);

        meshes[i]->AddMaterial("Rubber");
        meshes[i]->AddMaterial("Steel");
        meshes[i]->AddMaterial("Cord");
        meshes[i]->CreateMaterialData(CreateMaterial);

        char domain[20];
        sprintf(domain, "domain%d", i);
        DBMkDir(db, domain);
        DBSetDir(db, domain);
        meshes[i]->WriteMesh(db, meshnames[i]);
        DBSetDir(db, "..");
    }

    // Create a multimesh based on the different sections
    char *meshnames2[] = {"domain0/section0", "domain1/section1",
                          "domain2/section2", "domain3/section3"};
    int meshtypes[4] = {DB_QUAD_CURV,DB_QUAD_CURV,DB_QUAD_CURV,DB_QUAD_CURV};
    DBPutMultimesh(db, "tire", 4, meshnames2, meshtypes, NULL);
  
    // Create multivars
    char *varnames[] = {"domain0/pressure", "domain1/pressure", 
                        "domain2/pressure", "domain3/pressure"};
    int vartypes[] = {DB_QUADVAR,DB_QUADVAR,DB_QUADVAR,DB_QUADVAR};
    DBPutMultivar(db, "pressure", 4, varnames, vartypes, NULL);

    char *varnames2[] = {"domain0/temperature", "domain1/temperature", 
                        "domain2/temperature", "domain3/temperature"};
    DBPutMultivar(db, "temperature", 4, varnames2, vartypes, NULL);

    char *varnames3[] = {"domain0/sb", "domain1/sb", 
                         "domain2/sb", "domain3/sb"};
    DBPutMultivar(db, "sb", 4, varnames3, vartypes, NULL);


    char *matnames[] = {"domain0/mat1", "domain1/mat1", 
                         "domain2/mat1", "domain3/mat1"};
    DBPutMultimat(db, "Materials", 4, matnames, NULL);

    DBClose(db);


    return 0;
}
